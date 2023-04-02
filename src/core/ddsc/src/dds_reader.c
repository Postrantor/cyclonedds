/*
 * Copyright(c) 2006 to 2022 ZettaScale Technology and others
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v. 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0, or the Eclipse Distribution License
 * v. 1.0 which is available at
 * http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * SPDX-License-Identifier: EPL-2.0 OR BSD-3-Clause
 */

#include <assert.h>
#include <string.h>

#include "dds/dds.h"
#include "dds/ddsc/dds_rhc.h"
#include "dds/ddsi/ddsi_domaingv.h"
#include "dds/ddsi/ddsi_endpoint.h"
#include "dds/ddsi/ddsi_endpoint_match.h"
#include "dds/ddsi/ddsi_entity.h"
#include "dds/ddsi/ddsi_entity_index.h"
#include "dds/ddsi/ddsi_security_omg.h"
#include "dds/ddsi/ddsi_sertype.h"
#include "dds/ddsi/ddsi_statistics.h"
#include "dds/ddsi/ddsi_thread.h"
#include "dds/ddsrt/heap.h"
#include "dds/ddsrt/io.h"
#include "dds/ddsrt/static_assert.h"
#include "dds/version.h"
#include "dds__builtin.h"
#include "dds__data_allocator.h"
#include "dds__get_status.h"
#include "dds__init.h"
#include "dds__listener.h"
#include "dds__participant.h"
#include "dds__qos.h"
#include "dds__reader.h"
#include "dds__rhc_default.h"
#include "dds__statistics.h"
#include "dds__subscriber.h"
#include "dds__topic.h"

#ifdef DDS_HAS_SHM
#include "dds/ddsi/ddsi_shm_transport.h"
#include "dds/ddsi/ddsi_tkmap.h"
#include "dds/ddsrt/md5.h"
#include "dds/ddsrt/sync.h"
#include "dds/ddsrt/threads.h"
#include "dds__shm_monitor.h"
#include "dds__shm_qos.h"
#include "iceoryx_binding_c/wait_set.h"
#endif

/**
 * @file
 * @brief Cyclone DDS Reader 相关实现
 */

/**
 * @def DDS_READER_STATUS_MASK
 * @brief 读取器状态掩码，用于表示可用的读取器状态
 */
#define DDS_READER_STATUS_MASK                                                    \
  (DDS_SAMPLE_REJECTED_STATUS | DDS_LIVELINESS_CHANGED_STATUS |                   \
   DDS_REQUESTED_DEADLINE_MISSED_STATUS | DDS_REQUESTED_INCOMPATIBLE_QOS_STATUS | \
   DDS_DATA_AVAILABLE_STATUS | DDS_SAMPLE_LOST_STATUS | DDS_SUBSCRIPTION_MATCHED_STATUS)

/**
 * @brief 关闭 dds_reader 实体
 *
 * @param[in] e 指向要关闭的 dds_entity 的指针
 */
static void dds_reader_close(dds_entity *e) ddsrt_nonnull_all;

/**
 * @brief 关闭 dds_reader 实体的实现
 *
 * @param[in] e 指向要关闭的 dds_entity 的指针
 */
static void dds_reader_close(dds_entity *e) {
  // 将 e 转换为 dds_reader 类型的指针
  struct dds_reader *const rd = (struct dds_reader *)e;

  // 确保 rd->m_rd 不为空
  assert(rd->m_rd != NULL);

#ifdef DDS_HAS_SHM
  // 如果启用了共享内存支持
  if (rd->m_iox_sub) {
    // 等待使用此读取器的 iceoryx 订阅者的任何运行回调
    dds_shm_monitor_detach_reader(&rd->m_entity.m_domain->m_shm_monitor, rd);

    // 从现在开始，此读取器上将不再运行回调
  }
#endif
}

/**
 * @brief 唤醒线程状态并删除读取器
 *
 * @param[in] e 读取器实体指针
 * @return 返回操作结果，成功返回 DDS_RETCODE_OK
 */
static dds_return_t dds_reader_delete(dds_entity *e) ddsrt_nonnull_all;

static dds_return_t dds_reader_delete(dds_entity *e) {
  // 将实体指针转换为读取器指针
  dds_reader *const rd = (dds_reader *)e;

  // 如果存在内存分配，则释放相关资源
  if (rd->m_loan) {
    void **ptrs = ddsrt_malloc(rd->m_loan_size * sizeof(*ptrs));
    ddsi_sertype_realloc_samples(ptrs, rd->m_topic->m_stype, rd->m_loan, rd->m_loan_size,
                                 rd->m_loan_size);
    ddsi_sertype_free_samples(rd->m_topic->m_stype, ptrs, rd->m_loan_size, DDS_FREE_ALL);
    ddsrt_free(ptrs);
  }

  // 唤醒线程状态
  ddsi_thread_state_awake(ddsi_lookup_thread_state(), &e->m_domain->gv);
  // 释放读取器的缓存
  dds_rhc_free(rd->m_rhc);
  // 线程进入休眠状态
  ddsi_thread_state_asleep(ddsi_lookup_thread_state());

#ifdef DDS_HAS_SHM
  // 如果使用共享内存，则释放相关资源
  if (rd->m_iox_sub) {
    // 删除操作必须在读取器缓存不再使用后进行
    // 因为需要互斥量并且需要使用 iceoryx 订阅者释放数据
    DDS_CLOG(DDS_LC_SHM, &e->m_domain->gv.logconfig, "Release iceoryx's subscriber\n");
    iox_sub_deinit(rd->m_iox_sub);
    iox_sub_context_fini(&rd->m_iox_sub_context);
  }
#endif

  // 减少主题实体的引用计数
  dds_entity_drop_ref(&rd->m_topic->m_entity);
  return DDS_RETCODE_OK;
}

/**
 * @brief 验证读取器的QoS设置是否有效。
 *
 * @param[in] rqos 读取器的QoS设置指针。
 * @return 如果验证成功，则返回DDS_RETCODE_OK；否则，返回DDS_RETCODE_BAD_PARAMETER。
 */
static dds_return_t validate_reader_qos(const dds_qos_t *rqos) {
#ifndef DDS_HAS_DEADLINE_MISSED
  // 如果rqos不为空且包含DEADLINE，并且deadline不等于DDS_INFINITY，则返回错误参数。
  if (rqos != NULL && (rqos->present & DDSI_QP_DEADLINE) && rqos->deadline.deadline != DDS_INFINITY)
    return DDS_RETCODE_BAD_PARAMETER;
#else
  // 如果定义了DDS_HAS_DEADLINE_MISSED，则忽略rqos参数。
  DDSRT_UNUSED_ARG(rqos);
#endif
  // 返回成功状态。
  return DDS_RETCODE_OK;
}

/**
 * @brief 设置读取器的QoS。
 *
 * @param[in] e 读取器实体指针。
 * @param[in] qos 读取器的QoS设置指针。
 * @param[in] enabled 是否启用QoS设置。
 * @return 如果设置成功，则返回DDS_RETCODE_OK；否则，返回其他错误代码。
 */
static dds_return_t dds_reader_qos_set(dds_entity *e, const dds_qos_t *qos, bool enabled) {
  // 注意：e->m_qos仍然是旧的，以允许此处失败。
  dds_return_t ret;
  // 验证读取器的QoS设置。
  if ((ret = validate_reader_qos(qos)) != DDS_RETCODE_OK) return ret;
  // 如果启用了QoS设置。
  if (enabled) {
    struct ddsi_reader *rd;
    // 唤醒线程状态。
    ddsi_thread_state_awake(ddsi_lookup_thread_state(), &e->m_domain->gv);
    // 查找读取器实体。
    if ((rd = ddsi_entidx_lookup_reader_guid(e->m_domain->gv.entity_index, &e->m_guid)) != NULL)
      // 更新读取器的QoS设置。
      ddsi_update_reader_qos(rd, qos);
    // 设置线程状态为睡眠。
    ddsi_thread_state_asleep(ddsi_lookup_thread_state());
  }
  // 返回成功状态。
  return DDS_RETCODE_OK;
}

/**
 * @brief 验证读取器状态掩码是否有效。
 *
 * @param[in] mask 读取器状态掩码。
 * @return 如果验证成功，则返回DDS_RETCODE_OK；否则，返回DDS_RETCODE_BAD_PARAMETER。
 */
static dds_return_t dds_reader_status_validate(uint32_t mask) {
  return (mask & ~DDS_READER_STATUS_MASK) ? DDS_RETCODE_BAD_PARAMETER : DDS_RETCODE_OK;
}

/**
 * @brief 在进入监听器独占访问之前，触发数据可用回调。
 *
 * @param[in] e 读取器实体指针。
 */
static void data_avail_cb_enter_listener_exclusive_access(dds_entity *e) {
  // 假设在进入时已经持有e->m_observers_lock。
  // 可能解锁并重新锁定e->m_observers_lock。
  // 之后e->m_listener将保持稳定。
  e->m_cb_pending_count++;
  while (e->m_cb_count > 0) ddsrt_cond_wait(&e->m_observers_cond, &e->m_observers_lock);
  e->m_cb_count++;
}

/**
 * @brief 离开监听器独占访问的回调函数
 *
 * 假设在进入时已经持有 e->m_observers_lock。
 *
 * @param[in] e 实体指针
 */
static void data_avail_cb_leave_listener_exclusive_access(dds_entity *e) {
  // 减少回调计数
  e->m_cb_count--;
  // 减少待处理回调计数
  e->m_cb_pending_count--;
  // 广播观察者条件变量
  ddsrt_cond_broadcast(&e->m_observers_cond);
}

/**
 * @brief 调用数据可用回调函数
 *
 * 假设在进入时已经持有 sub->m_observers_lock。
 * 解锁并重新锁定 sub->m_observers_lock。
 *
 * @param[in] sub 订阅者实体指针
 * @param[in] lst 监听器结构指针
 */
static void data_avail_cb_invoke_dor(dds_entity *sub, const struct dds_listener *lst) {
  // 进入监听器独占访问
  data_avail_cb_enter_listener_exclusive_access(sub);
  // 解锁观察者互斥锁
  ddsrt_mutex_unlock(&sub->m_observers_lock);
  // 调用 on_data_on_readers 回调函数
  lst->on_data_on_readers(sub->m_hdllink.hdl, lst->on_data_on_readers_arg);
  // 锁定观察者互斥锁
  ddsrt_mutex_lock(&sub->m_observers_lock);
  // 离开监听器独占访问
  data_avail_cb_leave_listener_exclusive_access(sub);
}

/**
 * @brief 设置数据可用回调状态
 *
 * @param[in] rd 数据读取实体指针
 * @param[in] status_and_mask 状态和掩码值
 * @return uint32_t 返回设置的状态值
 */
static uint32_t data_avail_cb_set_status(dds_entity *rd, uint32_t status_and_mask) {
  uint32_t ret = 0;
  // 设置数据可用状态
  if (dds_entity_status_set(rd, DDS_DATA_AVAILABLE_STATUS)) ret |= DDS_DATA_AVAILABLE_STATUS;
  // 检查并设置数据读取者状态
  if (status_and_mask & (DDS_DATA_ON_READERS_STATUS << SAM_ENABLED_SHIFT)) {
    if (dds_entity_status_set(rd->m_parent, DDS_DATA_ON_READERS_STATUS))
      ret |= DDS_DATA_ON_READERS_STATUS;
  }
  return ret;
}

/**
 * @brief 触发等待集合的数据可用回调
 *
 * @param[in] rd 数据读取实体指针
 * @param[in] signal 触发信号
 */
static void data_avail_cb_trigger_waitsets(dds_entity *rd, uint32_t signal) {
  // 如果信号为0，则返回
  if (signal == 0) return;

  // 处理数据读取者状态信号
  if (signal & DDS_DATA_ON_READERS_STATUS) {
    dds_entity *const sub = rd->m_parent;
    // 锁定观察者互斥锁
    ddsrt_mutex_lock(&sub->m_observers_lock);
    // 获取状态和掩码值
    const uint32_t sm = ddsrt_atomic_ld32(&sub->m_status.m_status_and_mask);
    // 检查并触发数据读取者状态信号
    if ((sm & (sm >> SAM_ENABLED_SHIFT)) & DDS_DATA_ON_READERS_STATUS)
      dds_entity_observers_signal(sub, DDS_DATA_ON_READERS_STATUS);
    // 解锁观察者互斥锁
    ddsrt_mutex_unlock(&sub->m_observers_lock);
  }
  // 处理数据可用状态信号
  if (signal & DDS_DATA_AVAILABLE_STATUS) {
    // 获取状态和掩码值
    const uint32_t sm = ddsrt_atomic_ld32(&rd->m_status.m_status_and_mask);
    // 检查并触发数据可用状态信号
    if ((sm & (sm >> SAM_ENABLED_SHIFT)) & DDS_DATA_AVAILABLE_STATUS)
      dds_entity_observers_signal(rd, DDS_DATA_AVAILABLE_STATUS);
  }
}

/**
 * @brief 数据可用回调函数，当数据可用时触发。
 *
 * DATA_AVAILABLE 有两个特殊之处：首先，它应该首先尝试在祖先行上的 DATA_ON_READERS，
 * 如果没有消耗，则设置订阅者的状态；其次，它是唯一一个需要考虑开销的。
 * 否则，它与 dds_reader_status_cb 非常相似。
 *
 * @param rd 指向 dds_reader 结构体的指针
 */
void dds_reader_data_available_cb(struct dds_reader *rd) {
  // 获取实体的监听器对象
  struct dds_listener const *const lst = &rd->m_entity.m_listener;
  uint32_t signal = 0;

  // 对实体的观察者锁进行加锁
  ddsrt_mutex_lock(&rd->m_entity.m_observers_lock);
  const uint32_t status_and_mask = ddsrt_atomic_ld32(&rd->m_entity.m_status.m_status_and_mask);

  // 如果 on_data_on_readers 和 on_data_available 都为 0，则设置实体的状态
  if (lst->on_data_on_readers == 0 && lst->on_data_available == 0)
    signal = data_avail_cb_set_status(&rd->m_entity, status_and_mask);
  else {
    // 锁定监听器对象，以便在不持有 m_observers_lock 的情况下查看 "lst"
    data_avail_cb_enter_listener_exclusive_access(&rd->m_entity);

    // 如果存在 on_data_on_readers 回调
    if (lst->on_data_on_readers) {
      dds_entity *const sub = rd->m_entity.m_parent;
      ddsrt_mutex_unlock(&rd->m_entity.m_observers_lock);
      ddsrt_mutex_lock(&sub->m_observers_lock);

      // 如果不重置状态，则设置实体的状态
      if (!(lst->reset_on_invoke & DDS_DATA_ON_READERS_STATUS))
        signal = data_avail_cb_set_status(&rd->m_entity, status_and_mask);

      // 调用 on_data_on_readers 回调
      data_avail_cb_invoke_dor(sub, lst);

      ddsrt_mutex_unlock(&sub->m_observers_lock);
      ddsrt_mutex_lock(&rd->m_entity.m_observers_lock);
    } else {
      assert(rd->m_entity.m_listener.on_data_available);

      // 如果不重置状态，则设置实体的状态
      if (!(lst->reset_on_invoke & DDS_DATA_AVAILABLE_STATUS))
        signal = data_avail_cb_set_status(&rd->m_entity, status_and_mask);

      ddsrt_mutex_unlock(&rd->m_entity.m_observers_lock);

      // 调用 on_data_available 回调
      lst->on_data_available(rd->m_entity.m_hdllink.hdl, lst->on_data_available_arg);

      ddsrt_mutex_lock(&rd->m_entity.m_observers_lock);
    }

    // 离开监听器独占访问
    data_avail_cb_leave_listener_exclusive_access(&rd->m_entity);
  }

  // 触发等待集合
  data_avail_cb_trigger_waitsets(&rd->m_entity, signal);

  // 解锁实体的观察者锁
  ddsrt_mutex_unlock(&rd->m_entity.m_observers_lock);
}

/**
 * @brief 更新请求的截止时间未满足状态
 *
 * @param[out] st 请求的截止时间未满足状态指针
 * @param[in] data 状态回调数据指针
 */
static void update_requested_deadline_missed(
    struct dds_requested_deadline_missed_status *__restrict st, const ddsi_status_cb_data_t *data) {
  // 设置最后一个实例句柄
  st->last_instance_handle = data->handle;

  // 计算总数并更新，确保不超过 UINT32_MAX
  uint64_t tmp = (uint64_t)data->extra + (uint64_t)st->total_count;
  st->total_count = tmp > UINT32_MAX ? UINT32_MAX : (uint32_t)tmp;

  // 增加 total_count_change 的值，并确保其在 INT32_MIN 和 INT32_MAX 之间
  int64_t tmp2 = (int64_t)data->extra + (int64_t)st->total_count_change;
  st->total_count_change = tmp2 > INT32_MAX   ? INT32_MAX
                           : tmp2 < INT32_MIN ? INT32_MIN
                                              : (int32_t)tmp2;
}

/**
 * @brief 更新请求的不兼容 QoS 状态
 *
 * @param[out] st 请求的不兼容 QoS 状态指针
 * @param[in] data 状态回调数据指针
 */
static void update_requested_incompatible_qos(
    struct dds_requested_incompatible_qos_status *__restrict st,
    const ddsi_status_cb_data_t *data) {
  // 设置最后一个策略 ID
  st->last_policy_id = data->extra;

  // 增加总数和变化计数
  st->total_count++;
  st->total_count_change++;
}

/**
 * @brief 更新样本丢失状态
 *
 * @param[out] st 样本丢失状态指针
 * @param[in] data 状态回调数据指针
 */
static void update_sample_lost(struct dds_sample_lost_status *__restrict st,
                               const ddsi_status_cb_data_t *data) {
  (void)data;

  // 增加总数和变化计数
  st->total_count++;
  st->total_count_change++;
}

/**
 * @brief 更新样本拒绝状态
 *
 * @param[out] st 样本拒绝状态指针
 * @param[in] data 状态回调数据指针
 */
static void update_sample_rejected(struct dds_sample_rejected_status *__restrict st,
                                   const ddsi_status_cb_data_t *data) {
  // 设置最后一个原因和实例句柄
  st->last_reason = data->extra;
  st->last_instance_handle = data->handle;

  // 增加总数和变化计数
  st->total_count++;
  st->total_count_change++;
}

/**
 * @brief 更新生命周期改变状态 (Update liveliness changed status)
 *
 * @param[in,out] st 生命周期改变状态指针 (Pointer to the liveliness changed status)
 * @param[in] data 状态回调数据 (Status callback data)
 */
static void update_liveliness_changed(struct dds_liveliness_changed_status *__restrict st,
                                      const ddsi_status_cb_data_t *data) {
  // 静态断言，确保枚举值的顺序正确 (Static assert to ensure correct order of enum values)
  DDSRT_STATIC_ASSERT(
      (uint32_t)DDSI_LIVELINESS_CHANGED_ADD_ALIVE == 0 &&
      DDSI_LIVELINESS_CHANGED_ADD_ALIVE < DDSI_LIVELINESS_CHANGED_ADD_NOT_ALIVE &&
      DDSI_LIVELINESS_CHANGED_ADD_NOT_ALIVE < DDSI_LIVELINESS_CHANGED_REMOVE_NOT_ALIVE &&
      DDSI_LIVELINESS_CHANGED_REMOVE_NOT_ALIVE < DDSI_LIVELINESS_CHANGED_REMOVE_ALIVE &&
      DDSI_LIVELINESS_CHANGED_REMOVE_ALIVE < DDSI_LIVELINESS_CHANGED_ALIVE_TO_NOT_ALIVE &&
      DDSI_LIVELINESS_CHANGED_ALIVE_TO_NOT_ALIVE < DDSI_LIVELINESS_CHANGED_NOT_ALIVE_TO_ALIVE &&
      (uint32_t)DDSI_LIVELINESS_CHANGED_NOT_ALIVE_TO_ALIVE < UINT32_MAX);

  // 断言额外数据在有效范围内 (Assert that extra data is within valid range)
  assert(data->extra <= (uint32_t)DDSI_LIVELINESS_CHANGED_NOT_ALIVE_TO_ALIVE);

  // 设置最后一个出版物句柄 (Set last publication handle)
  st->last_publication_handle = data->handle;

  // 根据额外数据更新生命周期状态 (Update liveliness status based on extra data)
  switch ((enum ddsi_liveliness_changed_data_extra)data->extra) {
    case DDSI_LIVELINESS_CHANGED_ADD_ALIVE:
      st->alive_count++;
      st->alive_count_change++;
      break;
    case DDSI_LIVELINESS_CHANGED_ADD_NOT_ALIVE:
      st->not_alive_count++;
      st->not_alive_count_change++;
      break;
    case DDSI_LIVELINESS_CHANGED_REMOVE_NOT_ALIVE:
      st->not_alive_count--;
      st->not_alive_count_change--;
      break;
    case DDSI_LIVELINESS_CHANGED_REMOVE_ALIVE:
      st->alive_count--;
      st->alive_count_change--;
      break;
    case DDSI_LIVELINESS_CHANGED_ALIVE_TO_NOT_ALIVE:
      st->alive_count--;
      st->alive_count_change--;
      st->not_alive_count++;
      st->not_alive_count_change++;
      break;
    case DDSI_LIVELINESS_CHANGED_NOT_ALIVE_TO_ALIVE:
      st->not_alive_count--;
      st->not_alive_count_change--;
      st->alive_count++;
      st->alive_count_change++;
      break;
  }
}

/**
 * @brief 更新订阅匹配状态 (Update subscription matched status)
 *
 * @param[in,out] st 订阅匹配状态指针 (Pointer to the subscription matched status)
 * @param[in] data 状态回调数据 (Status callback data)
 */
static void update_subscription_matched(struct dds_subscription_matched_status *__restrict st,
                                        const ddsi_status_cb_data_t *data) {
  // 设置最后一个出版物句柄 (Set last publication handle)
  st->last_publication_handle = data->handle;

  // 根据是否添加更新订阅匹配状态 (Update subscription matched status based on whether it is added
  // or not)
  if (data->add) {
    st->total_count++;
    st->current_count++;
    st->total_count_change++;
    st->current_count_change++;
  } else {
    st->current_count--;
    st->current_count_change--;
  }
}

/*
在这个例子中，STATUS_CB_IMPL
宏可能有多个版本，每个版本接受不同数量或类型的参数。通过使用宏定义，我们可以根据传入的参数自动选择合适的实现，从而实现类似于函数重载的功能。

这种方法在 C 语言中是常见的，尤其是在处理不同类型或数量的参数时。
*/
/* 重置将所有内容（类型）设置为0，包括原因字段，请验证0是否正确 */
/* Reset sets everything (type) 0, including the reason field, verify that 0 is correct */
DDSRT_STATIC_ASSERT((int)DDS_NOT_REJECTED == 0);

/**
 * @brief 获取订阅匹配状态
 * @param reader 读取器实例
 * @param subscription_matched 订阅匹配状态
 * @param SUBSCRIPTION_MATCHED 状态类型
 * @param total_count_change 总计数变化
 * @param current_count_change 当前计数变化
 */
DDS_GET_STATUS(
    reader, subscription_matched, SUBSCRIPTION_MATCHED, total_count_change, current_count_change)

/**
 * @brief 获取生命周期改变状态
 * @param reader 读取器实例
 * @param liveliness_changed 生命周期改变状态
 * @param LIVELINESS_CHANGED 状态类型
 * @param alive_count_change 存活计数变化
 * @param not_alive_count_change 非存活计数变化
 */
DDS_GET_STATUS(
    reader, liveliness_changed, LIVELINESS_CHANGED, alive_count_change, not_alive_count_change)

/**
 * @brief 获取样本拒绝状态
 * @param reader 读取器实例
 * @param sample_rejected 样本拒绝状态
 * @param SAMPLE_REJECTED 状态类型
 * @param total_count_change 总计数变化
 */
DDS_GET_STATUS(reader, sample_rejected, SAMPLE_REJECTED, total_count_change)

/**
 * @brief 获取样本丢失状态
 * @param reader 读取器实例
 * @param sample_lost 样本丢失状态
 * @param SAMPLE_LOST 状态类型
 * @param total_count_change 总计数变化
 */
DDS_GET_STATUS(reader, sample_lost, SAMPLE_LOST, total_count_change)

/**
 * @brief 获取请求截止日期未满足状态
 * @param reader 读取器实例
 * @param requested_deadline_missed 请求截止日期未满足状态
 * @param REQUESTED_DEADLINE_MISSED 状态类型
 * @param total_count_change 总计数变化
 */
DDS_GET_STATUS(reader, requested_deadline_missed, REQUESTED_DEADLINE_MISSED, total_count_change)

/**
 * @brief 获取请求的不兼容QoS状态
 * @param reader 读取器实例
 * @param requested_incompatible_qos 请求的不兼容QoS状态
 * @param REQUESTED_INCOMPATIBLE_QOS 状态类型
 * @param total_count_change 总计数变化
 */
DDS_GET_STATUS(reader, requested_incompatible_qos, REQUESTED_INCOMPATIBLE_QOS, total_count_change)

/* 状态回调函数实现 */

/**
 * @brief 订阅匹配状态回调函数实现
 * @param reader 读取器实例
 * @param subscription_matched 订阅匹配状态
 * @param SUBSCRIPTION_MATCHED 状态类型
 * @param total_count_change 总计数变化
 * @param current_count_change 当前计数变化
 */
STATUS_CB_IMPL(
    reader, subscription_matched, SUBSCRIPTION_MATCHED, total_count_change, current_count_change)

/**
 * @brief 生命周期改变状态回调函数实现
 * @param reader 读取器实例
 * @param liveliness_changed 生命周期改变状态
 * @param LIVELINESS_CHANGED 状态类型
 * @param alive_count_change 存活计数变化
 * @param not_alive_count_change 非存活计数变化
 */
STATUS_CB_IMPL(
    reader, liveliness_changed, LIVELINESS_CHANGED, alive_count_change, not_alive_count_change)

/**
 * @brief 样本拒绝状态回调函数实现
 * @param reader 读取器实例
 * @param sample_rejected 样本拒绝状态
 * @param SAMPLE_REJECTED 状态类型
 * @param total_count_change 总计数变化
 */
STATUS_CB_IMPL(reader, sample_rejected, SAMPLE_REJECTED, total_count_change)

/**
 * @brief 样本丢失状态回调函数实现
 * @param reader 读取器实例
 * @param sample_lost 样本丢失状态
 * @param SAMPLE_LOST 状态类型
 * @param total_count_change 总计数变化
 */
STATUS_CB_IMPL(reader, sample_lost, SAMPLE_LOST, total_count_change)

/**
 * @brief 请求截止日期未满足状态回调函数实现
 * @param reader 读取器实例
 * @param requested_deadline_missed 请求截止日期未满足状态
 * @param REQUESTED_DEADLINE_MISSED 状态类型
 * @param total_count_change 总计数变化
 */
STATUS_CB_IMPL(reader, requested_deadline_missed, REQUESTED_DEADLINE_MISSED, total_count_change)

/**
 * @brief 请求的不兼容QoS状态回调函数实现
 * @param reader 读取器实例
 * @param requested_incompatible_qos 请求的不兼容QoS状态
 * @param REQUESTED_INCOMPATIBLE_QOS 状态类型
 * @param total_count_change 总计数变化
 */
STATUS_CB_IMPL(reader, requested_incompatible_qos, REQUESTED_INCOMPATIBLE_QOS, total_count_change)

/**
 * @brief DDS 读取器状态回调函数。
 *
 * 当读取器的状态发生变化时，此回调函数将被调用。
 *
 * @param[in] ventity 一个指向 dds_reader 结构体的指针。
 * @param[in] data 包含状态信息的 ddsi_status_cb_data_t 结构体指针。
 */
void dds_reader_status_cb(void *ventity, const ddsi_status_cb_data_t *data) {
  // 将传入的 void 指针转换为 dds_reader 指针
  dds_reader *const rd = ventity;

  // 当 data 为 NULL 时，表示 DDSI 读取器已被删除
  if (data == NULL) {
    // 释放在创建过程中进行的初始声明，这将表示现在可以进行进一步的 API 删除操作
    ddsrt_mutex_lock(&rd->m_entity.m_mutex);
    rd->m_rd = NULL;
    ddsrt_cond_broadcast(&rd->m_entity.m_cond);
    ddsrt_mutex_unlock(&rd->m_entity.m_mutex);
    return;
  }

  // 序列化监听器调用。这样做的好处是在释放 m_observers_lock 的同时，
  // 可以安全地递增和/或重置计数器和 "change" 计数器，并在监听器调用期间保持稳定
  ddsrt_mutex_lock(&rd->m_entity.m_observers_lock);
  rd->m_entity.m_cb_pending_count++;
  while (rd->m_entity.m_cb_count > 0)
    ddsrt_cond_wait(&rd->m_entity.m_observers_cond, &rd->m_entity.m_observers_lock);
  rd->m_entity.m_cb_count++;

  // 获取状态 ID
  const enum dds_status_id status_id = (enum dds_status_id)data->raw_status_id;
  switch (status_id) {
    case DDS_REQUESTED_DEADLINE_MISSED_STATUS_ID:
      status_cb_requested_deadline_missed(rd, data);
      break;
    case DDS_REQUESTED_INCOMPATIBLE_QOS_STATUS_ID:
      status_cb_requested_incompatible_qos(rd, data);
      break;
    case DDS_SAMPLE_LOST_STATUS_ID:
      status_cb_sample_lost(rd, data);
      break;
    case DDS_SAMPLE_REJECTED_STATUS_ID:
      status_cb_sample_rejected(rd, data);
      break;
    case DDS_LIVELINESS_CHANGED_STATUS_ID:
      status_cb_liveliness_changed(rd, data);
      break;
    case DDS_SUBSCRIPTION_MATCHED_STATUS_ID:
      status_cb_subscription_matched(rd, data);
      break;
    case DDS_DATA_ON_READERS_STATUS_ID:
    case DDS_DATA_AVAILABLE_STATUS_ID:
    case DDS_INCONSISTENT_TOPIC_STATUS_ID:
    case DDS_LIVELINESS_LOST_STATUS_ID:
    case DDS_PUBLICATION_MATCHED_STATUS_ID:
    case DDS_OFFERED_DEADLINE_MISSED_STATUS_ID:
    case DDS_OFFERED_INCOMPATIBLE_QOS_STATUS_ID:
      assert(0);
  }

  // 更新回调计数器
  rd->m_entity.m_cb_count--;
  rd->m_entity.m_cb_pending_count--;

  // 发送广播并解锁
  ddsrt_cond_broadcast(&rd->m_entity.m_observers_cond);
  ddsrt_mutex_unlock(&rd->m_entity.m_observers_lock);
}

/**
 * @brief dds_stat_keyvalue_descriptor 结构体数组，用于描述读取器统计信息的键值对。
 */
static const struct dds_stat_keyvalue_descriptor dds_reader_statistics_kv[] = {
    {"discarded_bytes", DDS_STAT_KIND_UINT64}};

/**
 * @brief dds_stat_descriptor 结构体实例，用于描述读取器统计信息。
 */
static const struct dds_stat_descriptor dds_reader_statistics_desc = {
    .count = sizeof(dds_reader_statistics_kv) / sizeof(dds_reader_statistics_kv[0]),
    .kv = dds_reader_statistics_kv};

/**
 * @brief 创建一个 dds_statistics 实例，用于存储读取器的统计信息。
 *
 * @param entity 指向 dds_entity 的指针。
 * @return 返回创建的 dds_statistics 实例的指针。
 */
static struct dds_statistics *dds_reader_create_statistics(const struct dds_entity *entity) {
  return dds_alloc_statistics(entity, &dds_reader_statistics_desc);
}

/**
 * @brief 刷新读取器的统计信息。
 *
 * @param entity 指向 dds_entity 的指针。
 * @param stat 指向 dds_statistics 的指针，用于存储刷新后的统计信息。
 */
static void dds_reader_refresh_statistics(const struct dds_entity *entity,
                                          struct dds_statistics *stat) {
  const struct dds_reader *rd = (const struct dds_reader *)entity;
  if (rd->m_rd) ddsi_get_reader_stats(rd->m_rd, &stat->kv[0].u.u64);
}

/**
 * @brief dds_entity_deriver 结构体实例，包含了与读取器相关的操作函数。
 */
const struct dds_entity_deriver dds_entity_deriver_reader = {
    .interrupt = dds_entity_deriver_dummy_interrupt,
    .close = dds_reader_close,
    .delete = dds_reader_delete,
    .set_qos = dds_reader_qos_set,
    .validate_status = dds_reader_status_validate,
    .create_statistics = dds_reader_create_statistics,
    .refresh_statistics = dds_reader_refresh_statistics};

#ifdef DDS_HAS_SHM
/**
 * @brief 创建一个iox_sub_options_t结构体，用于配置Cyclone DDS的共享内存订阅者选项。
 *
 * @param[in] qos 指向dds_qos_t结构体的指针，包含了QoS策略信息。
 * @return 返回一个已配置的iox_sub_options_t结构体。
 */
static iox_sub_options_t create_iox_sub_options(const dds_qos_t *qos) {
  // 初始化iox_sub_options_t结构体
  iox_sub_options_t opts;
  iox_sub_options_init(&opts);

  // 获取最大订阅者队列容量
  const uint32_t max_sub_queue_capacity = iox_cfg_max_subscriber_queue_capacity();

  // 注意：在接收到history.depth个样本之后，我们可能会丢失数据（如果我们没有足够快地从iceoryx队列中获取它们并将它们移动到读取器历史缓存中），
  // 但这对于volatile来说是有效的行为。然而，如果数据尽可能快地发布，队列填充得非常快，这可能导致不希望出现的行为。
  // 注意：如果历史深度大于队列容量，我们仍然使用共享内存，但相应地限制queueCapacity（否则iceoryx会发出警告并自行限制）

  if ((uint32_t)qos->history.depth <= max_sub_queue_capacity) {
    opts.queueCapacity = (uint64_t)qos->history.depth;
  } else {
    opts.queueCapacity = max_sub_queue_capacity;
  }

  // 对于BEST EFFORT，DDS要求不接收历史数据（无论持久性如何）
  if (qos->reliability.kind == DDS_RELIABILITY_BEST_EFFORT ||
      qos->durability.kind == DDS_DURABILITY_VOLATILE) {
    opts.historyRequest = 0;
  } else {
    // TRANSIENT LOCAL和更强的策略
    opts.historyRequest = (uint64_t)qos->history.depth;
    // 如果发布者不支持历史数据，它将不会被iceoryx连接
    opts.requirePublisherHistorySupport = true;
  }

  return opts;
}
#endif

/**
 * @brief 创建一个数据读取器实例。
 *
 * @param[in] participant_or_subscriber 参与者或订阅者实体
 * @param[in] topic 主题实体
 * @param[in] qos 读取器的质量服务设置
 * @param[in] listener 读取器的监听器设置
 * @param[in] rhc 读取器的缓存，如果为NULL，则使用默认缓存
 * @return 成功时返回创建的读取器实体，失败时返回错误代码
 */
static dds_entity_t dds_create_reader_int(dds_entity_t participant_or_subscriber,
                                          dds_entity_t topic,
                                          const dds_qos_t *qos,
                                          const dds_listener_t *listener,
                                          struct dds_rhc *rhc) {
  dds_qos_t *rqos;
  dds_subscriber *sub = NULL;
  dds_entity_t subscriber;
  dds_topic *tp;
  dds_return_t rc;
  dds_entity_t pseudo_topic = 0;
  bool created_implicit_sub = false;

  // 根据主题类型进行处理
  switch (topic) {
    case DDS_BUILTIN_TOPIC_DCPSTOPIC:
#ifndef DDS_HAS_TOPIC_DISCOVERY
      return DDS_RETCODE_UNSUPPORTED;
#endif
    case DDS_BUILTIN_TOPIC_DCPSPARTICIPANT:
    case DDS_BUILTIN_TOPIC_DCPSPUBLICATION:
    case DDS_BUILTIN_TOPIC_DCPSSUBSCRIPTION:
      /* 将提供的伪主题转换为真实主题 */
      pseudo_topic = topic;
      if ((subscriber = dds__get_builtin_subscriber(participant_or_subscriber)) < 0)
        return subscriber;
      if ((rc = dds_subscriber_lock(subscriber, &sub)) != DDS_RETCODE_OK) return rc;
      topic = dds__get_builtin_topic(subscriber, topic);
      break;

    default: {
      dds_entity *p_or_s;
      if ((rc = dds_entity_lock(participant_or_subscriber, DDS_KIND_DONTCARE, &p_or_s)) !=
          DDS_RETCODE_OK)
        return rc;
      switch (dds_entity_kind(p_or_s)) {
        case DDS_KIND_SUBSCRIBER:
          subscriber = participant_or_subscriber;
          sub = (dds_subscriber *)p_or_s;
          break;
        case DDS_KIND_PARTICIPANT:
          created_implicit_sub = true;
          subscriber = dds__create_subscriber_l((dds_participant *)p_or_s, true, qos, NULL);
          dds_entity_unlock(p_or_s);
          if ((rc = dds_subscriber_lock(subscriber, &sub)) < 0) return rc;
          break;
        default:
          dds_entity_unlock(p_or_s);
          return DDS_RETCODE_ILLEGAL_OPERATION;
      }
      break;
    }
  }

  /* 如果pseudo_topic不为0，表示主题并非来自应用程序，我们允许将其固定，尽管它被标记为NO_USER_ACCESS
   */
  if ((rc = dds_topic_pin_with_origin(topic, pseudo_topic ? false : true, &tp)) < 0)
    goto err_pin_topic;
  assert(tp->m_stype);
  if (dds_entity_participant(&sub->m_entity) != dds_entity_participant(&tp->m_entity)) {
    rc = DDS_RETCODE_BAD_PARAMETER;
    goto err_pp_mismatch;
  }

  /* 在创建和注册读取器之前，防止在主题上设置qos：我们不能允许在创建读取器之前发生TOPIC_DATA更改，
     因为那样的更改将不会在发现/内置主题中发布。

     不要保持参与者（它保护主题的QoS）锁定，因为这可能导致死锁
     在订阅匹配监听器中创建读取器/写入器的应用程序（无论监听器中的限制是否合理，
     它过去曾经起作用，所以不能随意破坏）。 */
  dds_topic_defer_set_qos(tp);

  /* 合并主题和订阅者的qos，dds_copy_qos仅在传递空参数时失败，但在这里不是这种情况 */
  struct ddsi_domaingv *gv = &sub->m_entity.m_domain->gv;
  rqos = dds_create_qos();
  if (qos) ddsi_xqos_mergein_missing(rqos, qos, DDS_READER_QOS_MASK);
  if (sub->m_entity.m_qos)
    ddsi_xqos_mergein_missing(rqos, sub->m_entity.m_qos, ~DDSI_QP_ENTITY_NAME);
  if (tp->m_ktopic->qos)
    ddsi_xqos_mergein_missing(rqos, tp->m_ktopic->qos,
                              (DDS_READER_QOS_MASK | DDSI_QP_TOPIC_DATA) & ~DDSI_QP_ENTITY_NAME);
  ddsi_xqos_mergein_missing(rqos, &ddsi_default_qos_reader, ~DDSI_QP_DATA_REPRESENTATION);
  dds_apply_entity_naming(rqos, sub->m_entity.m_qos, gv);

  if ((rc = dds_ensure_valid_data_representation(rqos, tp->m_stype->allowed_data_representation,
                                                 false)) != 0)
    goto err_data_repr;

  if ((rc = ddsi_xqos_valid(&gv->logconfig, rqos)) < 0 ||
      (rc = validate_reader_qos(rqos)) != DDS_RETCODE_OK)
    goto err_bad_qos;

  /* 内置主题需要额外的检查：我们不希望在内置主题上遇到资源限制，这是一个不必要的复杂性 */
  if (pseudo_topic &&
      !dds__validate_builtin_reader_qos(tp->m_entity.m_domain, pseudo_topic, rqos)) {
    rc = DDS_RETCODE_INCONSISTENT_POLICY;
    goto err_bad_qos;
  }

  ddsi_thread_state_awake(ddsi_lookup_thread_state(), gv);
  const struct ddsi_guid *ppguid = dds_entity_participant_guid(&sub->m_entity);
  struct ddsi_participant *pp = ddsi_entidx_lookup_participant_guid(gv->entity_index, ppguid);

  /* 在删除参与者时，在删除DDSI参与者之前，会先移除子句柄（包括订阅者）。
     因此，在此处，在订阅者锁定内，我们可以断言参与者存在。 */
  assert(pp != NULL);

#ifdef DDS_HAS_SECURITY
  /* 检查是否启用了DDS安全性 */
  if (ddsi_omg_participant_is_secure(pp)) {
    /* 向访问控制安全插件请求创建读取器权限 */
    if (!ddsi_omg_security_check_create_reader(pp, gv->config.domainId, tp->m_name, rqos)) {
      rc = DDS_RETCODE_NOT_ALLOWED_BY_SECURITY;
      ddsi_thread_state_asleep(ddsi_lookup_thread_state());
      goto err_bad_qos;
    }
  }
#endif

  /* 创建读取器和关联的读取缓存（如果未由调用者提供） */
  struct dds_reader *const rd = dds_alloc(sizeof(*rd));
  const dds_entity_t reader = dds_entity_init(&rd->m_entity, &sub->m_entity, DDS_KIND_READER, false,
                                              true, rqos, listener, DDS_READER_STATUS_MASK);
  // 假设DATA_ON_READERS在订阅者中实现：
  // - 在将其添加到订阅者的子项之前，不会将更改传播到此读取器
  // - 一旦调用`new_reader`，数据就可以到达，需要在实现时引发DATA_ON_READERS
  // - 如果实际上没有实现，那么在订阅者上设置DATA_ON_READERS也没问题
  ddsrt_atomic_or32(&rd->m_entity.m_status.m_status_and_mask,
                    DDS_DATA_ON_READERS_STATUS << SAM_ENABLED_SHIFT);
  rd->m_sample_rejected_status.last_reason = DDS_NOT_REJECTED;
  rd->m_topic = tp;
  rd->m_rhc = rhc ? rhc : dds_rhc_default_new(rd, tp->m_stype);
  if (dds_rhc_associate(rd->m_rhc, rd, tp->m_stype, rd->m_entity.m_domain->gv.m_tkmap) < 0) {
    /* FIXME: 参见create_querycond，需要能够撤消entity_init */
    abort();
  }
  dds_entity_add_ref_locked(&tp->m_entity);

  /* FIXME: 监听器可能来得太早...应根据监听器设置掩码
     然后原子地设置监听器，将掩码保存到挂起的集合并清除它；
     然后调用挂起集中的那些监听器 */
  dds_entity_init_complete(&rd->m_entity);

#ifdef DDS_HAS_SHM
  assert(rqos->present & DDSI_QP_LOCATOR_MASK);
  if (!(gv->config.enable_shm && dds_shm_compatible_qos_and_topic(rqos, tp, true)))
    rqos->ignore_locator_type |= DDSI_LOCATOR_KIND_SHEM;
#endif

  /* 读取器从主题获取sertype，因为读取器使用的serdata函数不是特定于数据表示的
     （可以从cdr头部检索表示） */
  rc = ddsi_new_reader(&rd->m_rd, &rd->m_entity.m_guid, NULL, pp, tp->m_name, tp->m_stype, rqos,
                       &rd->m_rhc->common.rhc, dds_reader_status_cb, rd);
  assert(rc == DDS_RETCODE_OK); /* FIXME: 至少可以是资源不足 */
  ddsi_thread_state_asleep(ddsi_lookup_thread_state());

#ifdef DDS_HAS_SHM
  if (rd->m_rd->has_iceoryx) {
    DDS_CLOG(DDS_LC_SHM, &rd->m_entity.m_domain->gv.logconfig,
             "Reader's topic name will be DDS:Cyclone:%s\n", rd->m_topic->m_name);

    iox_sub_context_init(&rd->m_iox_sub_context);

    iox_sub_options_t opts = create_iox_sub_options(rqos);

    // 快速hack以使分区工作；使用*标记分隔分区名称和主题名称
    // 因为我们已经知道分区不能再包含*了
    char *part_topic = dds_shm_partition_topic(rqos, rd->m_topic);
    assert(part_topic != NULL);
    rd->m_iox_sub = iox_sub_init(&(iox_sub_storage_t){0}, gv->config.iceoryx_service,
                                 rd->m_topic->m_stype->type_name, part_topic, &opts);
    ddsrt_free(part_topic);

    // NB: 由于iceoryx结构的存储范例更改
    // 我们现在在m_iox_sub之前有一个指针8字节
    // 我们使用此地址来存储上下文指针。
    iox_sub_context_t **context = iox_sub_context_ptr(rd->m_iox_sub);
    *context = &rd->m_iox_sub_context;

    rc = dds_shm_monitor_attach_reader(&rd->m_entity.m_domain->m_shm_monitor, rd);

    if (rc != DDS_RETCODE_OK) {
      // 如果无法附加到监听器（因为我们将无法获取数据），我们将失败
      iox_sub_deinit(rd->m_iox_sub);
      rd->m_iox_sub = NULL;
      DDS_CLOG(DDS_LC_WARNING | DDS_LC_SHM, &rd->m_entity.m_domain->gv.logconfig,
               "Failed to attach iox subscriber to iox listener\n");
      // FIXME: 我们需要清理到现在为止创建的所有内容。
      //        目前只有部分清理，we need to extend it.
      goto err_bad_qos;
    }

    // those are set once and never changed
    // they are used to access reader and monitor from the callback when data is received
    rd->m_iox_sub_context.monitor = &rd->m_entity.m_domain->m_shm_monitor;
    rd->m_iox_sub_context.parent_reader = rd;
  }
#endif

  /** @brief 为函数添加参数列表的说明，并逐行添加详细的中文注释
   *
   * @param rd 读取器实体指针
   * @param sub 订阅者实体指针
   * @param tp 主题实体指针
   * @param rqos 读取器QoS设置指针
   * @param subscriber 订阅者实例
   * @return reader 创建成功返回读取器实例，否则返回错误代码
   */
  rd->m_entity.m_iid = ddsi_get_entity_instanceid(
      &rd->m_entity.m_domain->gv, &rd->m_entity.m_guid);  // 获取实体实例ID并赋值给读取器实体
  dds_entity_register_child(&sub->m_entity, &rd->m_entity);  // 将读取器实体注册为订阅者实体的子实体

  // 在将读取器包含在订阅者的子实体中之后，订阅者将开始传播是否实现了data_on_readers。
  // 这里没有考虑到悲观地将其设置为已实现的情况，也没有考虑到在`dds_entity_register_child`之前实际上已经实现但不再实现的竞争情况。
  ddsrt_mutex_lock(&rd->m_entity.m_observers_lock);   // 锁定读取器实体的观察者锁
  ddsrt_mutex_lock(&sub->m_entity.m_observers_lock);  // 锁定订阅者实体的观察者锁
  if (sub->materialize_data_on_readers == 0)          // 如果订阅者的data_on_readers未实现
    ddsrt_atomic_and32(&rd->m_entity.m_status.m_status_and_mask,
                       ~(uint32_t)(DDS_DATA_ON_READERS_STATUS
                                   << SAM_ENABLED_SHIFT));  // 更新读取器实体的状态和掩码
  ddsrt_mutex_unlock(&sub->m_entity.m_observers_lock);      // 解锁订阅者实体的观察者锁
  ddsrt_mutex_unlock(&rd->m_entity.m_observers_lock);       // 解锁读取器实体的观察者锁

  dds_topic_allow_set_qos(tp);                              // 允许设置主题QoS
  dds_topic_unpin(tp);                                      // 取消固定主题实例
  dds_subscriber_unlock(sub);                               // 解锁订阅者实例
  return reader;                                            // 返回创建的读取器实例

err_bad_qos:
err_data_repr:
  dds_delete_qos(rqos);                                    // 删除错误的读取器QoS设置
  dds_topic_allow_set_qos(tp);                             // 允许设置主题QoS
err_pp_mismatch:
  dds_topic_unpin(tp);                                     // 取消固定主题实例
err_pin_topic:
  dds_subscriber_unlock(sub);                              // 解锁订阅者实例
  if (created_implicit_sub) (void)dds_delete(subscriber);  // 如果创建了隐式订阅者，则删除订阅者实例
  return rc;                                               // 返回错误代码
}

/**
 * @brief 创建一个数据读取器 (Create a data reader)
 *
 * @param participant_or_subscriber 参与者或订阅者实体 (Participant or subscriber entity)
 * @param topic 主题实体 (Topic entity)
 * @param qos 服务质量设置 (Quality of Service settings)
 * @param listener 监听器 (Listener)
 * @return 成功时返回创建的读取器实体，失败时返回错误代码 (Returns the created reader entity on
 * success, error code on failure)
 */
dds_entity_t dds_create_reader(dds_entity_t participant_or_subscriber,
                               dds_entity_t topic,
                               const dds_qos_t *qos,
                               const dds_listener_t *listener) {
  // 调用内部函数创建读取器 (Call internal function to create reader)
  return dds_create_reader_int(participant_or_subscriber, topic, qos, listener, NULL);
}

/**
 * @brief 创建一个带有资源历史缓存的数据读取器 (Create a data reader with resource history cache)
 *
 * @param participant_or_subscriber 参与者或订阅者实体 (Participant or subscriber entity)
 * @param topic 主题实体 (Topic entity)
 * @param qos 服务质量设置 (Quality of Service settings)
 * @param listener 监听器 (Listener)
 * @param rhc 资源历史缓存 (Resource history cache)
 * @return 成功时返回创建的读取器实体，失败时返回错误代码 (Returns the created reader entity on
 * success, error code on failure)
 */
dds_entity_t dds_create_reader_rhc(dds_entity_t participant_or_subscriber,
                                   dds_entity_t topic,
                                   const dds_qos_t *qos,
                                   const dds_listener_t *listener,
                                   struct dds_rhc *rhc) {
  // 检查资源历史缓存是否为空 (Check if resource history cache is NULL)
  if (rhc == NULL) return DDS_RETCODE_BAD_PARAMETER;

  // 调用内部函数创建带有资源历史缓存的读取器 (Call internal function to create reader with resource
  // history cache)
  return dds_create_reader_int(participant_or_subscriber, topic, qos, listener, rhc);
}

/**
 * @brief 锁定数据读取器中的样本 (Lock samples in the data reader)
 *
 * @param reader 读取器实体 (Reader entity)
 * @return 返回锁定的样本数量 (Returns the number of locked samples)
 */
uint32_t dds_reader_lock_samples(dds_entity_t reader) {
  dds_reader *rd;
  uint32_t n;

  // 尝试锁定读取器 (Try to lock the reader)
  if (dds_reader_lock(reader, &rd) != DDS_RETCODE_OK) return 0;

  // 锁定资源历史缓存中的样本 (Lock samples in resource history cache)
  n = dds_rhc_lock_samples(rd->m_rhc);

  // 解锁读取器 (Unlock the reader)
  dds_reader_unlock(rd);

  // 返回锁定的样本数量 (Return the number of locked samples)
  return n;
}

/**
 * @brief 等待历史数据 (Wait for historical data)
 *
 * @param reader 读取器实体 (Reader entity)
 * @param max_wait 最大等待时间 (Maximum wait time)
 * @return 返回操作结果 (Returns the operation result)
 */
dds_return_t dds_reader_wait_for_historical_data(dds_entity_t reader, dds_duration_t max_wait) {
  dds_reader *rd;
  dds_return_t ret;

  // 忽略最大等待时间 (Ignore maximum wait time)
  (void)max_wait;

  // 尝试锁定读取器 (Try to lock the reader)
  if ((ret = dds_reader_lock(reader, &rd)) != DDS_RETCODE_OK) return ret;

  // 根据耐久性类型处理历史数据 (Handle historical data based on durability kind)
  switch (rd->m_entity.m_qos->durability.kind) {
    case DDS_DURABILITY_VOLATILE:
      ret = DDS_RETCODE_OK;
      break;
    case DDS_DURABILITY_TRANSIENT_LOCAL:
      break;
    case DDS_DURABILITY_TRANSIENT:
    case DDS_DURABILITY_PERSISTENT:
      break;
  }

  // 解锁读取器 (Unlock the reader)
  dds_reader_unlock(rd);

  // 返回操作结果 (Return the operation result)
  return ret;
}

/**
 * @brief 获取实体的订阅者 (Get the subscriber of an entity)
 *
 * @param[in] entity 要查询的实体 (The entity to query)
 * @return 订阅者实体，如果出错则返回错误代码 (The subscriber entity, or an error code if there is
 * an issue)
 */
dds_entity_t dds_get_subscriber(dds_entity_t entity) {
  dds_entity *e;     // 定义一个指向实体的指针 (Define a pointer to an entity)
  dds_return_t ret;  // 定义一个返回值变量 (Define a return value variable)

  // 尝试获取实体并检查返回值 (Try to get the entity and check the return value)
  if ((ret = dds_entity_pin(entity, &e)) != DDS_RETCODE_OK)
    return ret;  // 如果返回值不是成功，则直接返回错误代码 (If the return value is not success,
                 // return the error code directly)
  else {
    dds_entity_t subh;  // 定义一个订阅者实体变量 (Define a subscriber entity variable)

    // 根据实体类型进行处理 (Process according to the entity type)
    switch (dds_entity_kind(e)) {
      case DDS_KIND_READER:           // 如果实体类型是读者 (If the entity type is reader)
        assert(dds_entity_kind(e->m_parent) ==
               DDS_KIND_SUBSCRIBER);  // 断言其父实体类型为订阅者 (Assert that its parent entity
                                      // type is subscriber)
        subh = e->m_parent->m_hdllink.hdl;  // 获取订阅者实体 (Get the subscriber entity)
        break;
      case DDS_KIND_COND_READ:  // 如果实体类型是条件读 (If the entity type is conditional read)
      case DDS_KIND_COND_QUERY:  // 或者实体类型是条件查询 (Or the entity type is conditional query)
        assert(dds_entity_kind(e->m_parent) ==
               DDS_KIND_READER);  // 断言其父实体类型为读者 (Assert that its parent entity type is
                                  // reader)
        assert(dds_entity_kind(e->m_parent->m_parent) ==
               DDS_KIND_SUBSCRIBER);  // 断言其祖父实体类型为订阅者 (Assert that its grandparent
                                      // entity type is subscriber)
        subh = e->m_parent->m_parent->m_hdllink.hdl;  // 获取订阅者实体 (Get the subscriber entity)
        break;
      default:                                        // 其他情况 (Other cases)
        subh = DDS_RETCODE_ILLEGAL_OPERATION;  // 返回非法操作错误代码 (Return illegal operation
                                               // error code)
        break;
    }

    dds_entity_unpin(e);  // 解除实体的引用 (Unpin the entity)
    return subh;          // 返回订阅者实体 (Return the subscriber entity)
  }
}

/**
 * @brief 初始化数据分配器 (Initialize the data allocator)
 *
 * @param[in] rd 读取器指针 (Pointer to the reader)
 * @param[out] data_allocator 数据分配器指针 (Pointer to the data allocator)
 * @return 返回操作结果代码 (Return operation result code)
 */
dds_return_t dds__reader_data_allocator_init(const dds_reader *rd,
                                             dds_data_allocator_t *data_allocator) {
#ifdef DDS_HAS_SHM
  // 将data_allocator的opaque字节转换为dds_iox_allocator_t类型指针
  // (Cast the opaque bytes of data_allocator to a pointer of type dds_iox_allocator_t)
  dds_iox_allocator_t *d = (dds_iox_allocator_t *)data_allocator->opaque.bytes;

  // 初始化互斥锁 (Initialize the mutex)
  ddsrt_mutex_init(&d->mutex);

  // 判断读取器是否有m_iox_sub成员 (Check if the reader has an m_iox_sub member)
  if (NULL != rd->m_iox_sub) {
    // 设置分配器类型为订阅者 (Set the allocator kind to subscriber)
    d->kind = DDS_IOX_ALLOCATOR_KIND_SUBSCRIBER;

    // 设置订阅者引用 (Set the subscriber reference)
    d->ref.sub = rd->m_iox_sub;
  } else {
    // 设置分配器类型为无 (Set the allocator kind to none)
    d->kind = DDS_IOX_ALLOCATOR_KIND_NONE;
  }

  // 返回操作成功代码 (Return the operation success code)
  return DDS_RETCODE_OK;
#else
  // 忽略未使用的参数 (Ignore unused parameters)
  (void)rd;
  (void)data_allocator;

  // 返回操作成功代码 (Return the operation success code)
  return DDS_RETCODE_OK;
#endif
}

/**
 * @brief 结束数据分配器 (Finalize the data allocator)
 *
 * @param[in] rd 读取器指针 (Pointer to the reader)
 * @param[out] data_allocator 数据分配器指针 (Pointer to the data allocator)
 * @return 返回操作结果代码 (Return operation result code)
 */
dds_return_t dds__reader_data_allocator_fini(const dds_reader *rd,
                                             dds_data_allocator_t *data_allocator) {
#ifdef DDS_HAS_SHM
  // 将data_allocator的opaque字节转换为dds_iox_allocator_t类型指针
  // (Cast the opaque bytes of data_allocator to a pointer of type dds_iox_allocator_t)
  dds_iox_allocator_t *d = (dds_iox_allocator_t *)data_allocator->opaque.bytes;

  // 销毁互斥锁 (Destroy the mutex)
  ddsrt_mutex_destroy(&d->mutex);

  // 设置分配器类型为结束 (Set the allocator kind to finalize)
  d->kind = DDS_IOX_ALLOCATOR_KIND_FINI;
#else
  // 忽略未使用的参数 (Ignore unused parameters)
  (void)data_allocator;
#endif

  // 忽略未使用的参数 (Ignore unused parameters)
  (void)rd;

  // 返回操作成功代码 (Return the operation success code)
  return DDS_RETCODE_OK;
}
