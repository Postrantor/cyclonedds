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
#include <assert.h>
#include <string.h>

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

// 声明实体锁和解锁的宏
DECL_ENTITY_LOCK_UNLOCK(dds_reader)

// 定义DDS阅读器状态掩码
#define DDS_READER_STATUS_MASK             \
  (DDS_SAMPLE_REJECTED_STATUS |            \
   DDS_LIVELINESS_CHANGED_STATUS |         \
   DDS_REQUESTED_DEADLINE_MISSED_STATUS |  \
   DDS_REQUESTED_INCOMPATIBLE_QOS_STATUS | \
   DDS_DATA_AVAILABLE_STATUS |             \
   DDS_SAMPLE_LOST_STATUS |                \
   DDS_SUBSCRIPTION_MATCHED_STATUS)

// 声明一个静态函数 dds_reader_close，用于关闭 dds_reader 实体
static void dds_reader_close(dds_entity *e) ddsrt_nonnull_all;

// 定义 dds_reader_close 函数，关闭 dds_reader 实体
static void dds_reader_close(dds_entity *e)
{
  // 将 e 转换为 dds_reader 类型的指针
  struct dds_reader *const rd = (struct dds_reader *)e;
  // 断言 rd->m_rd 不为空
  assert(rd->m_rd != NULL);

#ifdef DDS_HAS_SHM
  if (rd->m_iox_sub)
  {
    // 等待使用此读取器的 iceoryx 订阅者的任何运行回调
    dds_shm_monitor_detach_reader(&rd->m_entity.m_domain->m_shm_monitor, rd);
    // 从现在开始，将不再对此读取器运行回调
  }
#endif

  // 唤醒线程状态
  ddsi_thread_state_awake(ddsi_lookup_thread_state(), &e->m_domain->gv);
  // 删除读取器
  (void)ddsi_delete_reader(&e->m_domain->gv, &e->m_guid);
  // 线程进入休眠状态
  ddsi_thread_state_asleep(ddsi_lookup_thread_state());

  // 锁定实体互斥锁
  ddsrt_mutex_lock(&e->m_mutex);
  // 当 rd->m_rd 不为空时，等待条件变量
  while (rd->m_rd != NULL)
    ddsrt_cond_wait(&e->m_cond, &e->m_mutex);
  // 解锁实体互斥锁
  ddsrt_mutex_unlock(&e->m_mutex);
}

// 声明一个静态函数 dds_reader_delete，用于删除 dds_reader 实体
static dds_return_t dds_reader_delete(dds_entity *e) ddsrt_nonnull_all;
// 定义静态函数 dds_reader_delete，用于删除数据读取器实体
static dds_return_t dds_reader_delete(dds_entity *e)
{
  // 将 e 转换为 dds_reader 类型的指针
  dds_reader *const rd = (dds_reader *)e;

  // 如果存在 m_loan
  if (rd->m_loan)
  {
    // 分配内存空间给 ptrs
    void **ptrs = ddsrt_malloc(rd->m_loan_size * sizeof(*ptrs));
    // 重新分配样本空间
    ddsi_sertype_realloc_samples(ptrs, rd->m_topic->m_stype, rd->m_loan, rd->m_loan_size, rd->m_loan_size);
    // 释放样本空间
    ddsi_sertype_free_samples(rd->m_topic->m_stype, ptrs, rd->m_loan_size, DDS_FREE_ALL);
    // 释放 ptrs 指向的内存空间
    ddsrt_free(ptrs);
  }

  // 唤醒线程状态
  ddsi_thread_state_awake(ddsi_lookup_thread_state(), &e->m_domain->gv);
  // 释放读者历史缓存
  dds_rhc_free(rd->m_rhc);
  // 线程进入休眠状态
  ddsi_thread_state_asleep(ddsi_lookup_thread_state());

#ifdef DDS_HAS_SHM
  if (rd->m_iox_sub)
  {
    // 在不再使用读者缓存之后，删除操作必须在最后进行
    // 因为需要互斥锁和使用 iceoryx 订阅者释放数据
    DDS_CLOG(DDS_LC_SHM, &e->m_domain->gv.logconfig, "Release iceoryx's subscriber\n");
    iox_sub_deinit(rd->m_iox_sub);
    iox_sub_context_fini(&rd->m_iox_sub_context);
  }
#endif

  // 减少主题实体的引用计数
  dds_entity_drop_ref(&rd->m_topic->m_entity);
  // 返回操作成功的返回码
  return DDS_RETCODE_OK;
}

// 定义静态函数 validate_reader_qos，用于验证读取器的 QoS 设置
static dds_return_t validate_reader_qos(const dds_qos_t *rqos)
{
#ifndef DDS_HAS_DEADLINE_MISSED
  if (rqos != NULL && (rqos->present & DDSI_QP_DEADLINE) && rqos->deadline.deadline != DDS_INFINITY)
    return DDS_RETCODE_BAD_PARAMETER;
#else
  DDSRT_UNUSED_ARG(rqos);
#endif
  return DDS_RETCODE_OK;
}
// 设置读取器的QoS（Quality of Service，服务质量）
static dds_return_t dds_reader_qos_set(dds_entity *e, const dds_qos_t *qos, bool enabled)
{
  // 注意：e->m_qos仍然是旧的QoS，以允许在此处失败
  dds_return_t ret;
  // 验证读取器的QoS是否有效
  if ((ret = validate_reader_qos(qos)) != DDS_RETCODE_OK)
    return ret;
  // 如果启用了新的QoS设置
  if (enabled)
  {
    struct ddsi_reader *rd;
    ddsi_thread_state_awake(ddsi_lookup_thread_state(), &e->m_domain->gv);
    // 查找并更新读取器的QoS
    if ((rd = ddsi_entidx_lookup_reader_guid(e->m_domain->gv.entity_index, &e->m_guid)) != NULL)
      ddsi_update_reader_qos(rd, qos);
    ddsi_thread_state_asleep(ddsi_lookup_thread_state());
  }
  return DDS_RETCODE_OK;
}

// 验证读取器状态掩码是否有效
static dds_return_t dds_reader_status_validate(uint32_t mask)
{
  return (mask & ~DDS_READER_STATUS_MASK) ? DDS_RETCODE_BAD_PARAMETER : DDS_RETCODE_OK;
}

// 进入数据可用回调监听器独占访问
static void data_avail_cb_enter_listener_exclusive_access(dds_entity *e)
{
  // 假设在进入时持有 e->m_observers_lock
  // 可能解锁并重新锁定 e->m_observers_lock
  // 之后 e->m_listener 是稳定的
  e->m_cb_pending_count++;
  while (e->m_cb_count > 0)
    ddsrt_cond_wait(&e->m_observers_cond, &e->m_observers_lock);
  e->m_cb_count++;
}

// 离开数据可用回调监听器独占访问
static void data_avail_cb_leave_listener_exclusive_access(dds_entity *e)
{
  // 假设在进入时持有 e->m_observers_lock
  e->m_cb_count--;
  e->m_cb_pending_count--;
  ddsrt_cond_broadcast(&e->m_observers_cond);
}
// 定义数据可用回调函数，进入监听器独占访问
static void data_avail_cb_enter_listener_exclusive_access(dds_entity *e)
{
  // 假设在进入时已经持有 e->m_observers_lock
  // 可能会解锁并重新锁定 e->m_observers_lock
  // 之后 e->m_listener 是稳定的
  e->m_cb_pending_count++;
  while (e->m_cb_count > 0)
    ddsrt_cond_wait(&e->m_observers_cond, &e->m_observers_lock);
  e->m_cb_count++;
}

// 定义数据可用回调函数，离开监听器独占访问
static void data_avail_cb_leave_listener_exclusive_access(dds_entity *e)
{
  // 假设在进入时已经持有 e->m_observers_lock
  e->m_cb_count--;
  e->m_cb_pending_count--;
  ddsrt_cond_broadcast(&e->m_observers_cond);
}

// 定义数据可用回调函数，调用 Data On Readers 监听器
static void data_avail_cb_invoke_dor(dds_entity *sub, const struct dds_listener *lst)
{
  // 假设在进入时已经持有 sub->m_observers_lock
  // 解锁并重新锁定 sub->m_observers_lock
  data_avail_cb_enter_listener_exclusive_access(sub);
  ddsrt_mutex_unlock(&sub->m_observers_lock);
  lst->on_data_on_readers(sub->m_hdllink.hdl, lst->on_data_on_readers_arg);
  ddsrt_mutex_lock(&sub->m_observers_lock);
  data_avail_cb_leave_listener_exclusive_access(sub);
}

// 定义数据可用回调函数，设置状态
static uint32_t data_avail_cb_set_status(dds_entity *rd, uint32_t status_and_mask)
{
  uint32_t ret = 0;
  if (dds_entity_status_set(rd, DDS_DATA_AVAILABLE_STATUS))
    ret |= DDS_DATA_AVAILABLE_STATUS;
  if (status_and_mask & (DDS_DATA_ON_READERS_STATUS << SAM_ENABLED_SHIFT))
  {
    if (dds_entity_status_set(rd->m_parent, DDS_DATA_ON_READERS_STATUS))
      ret |= DDS_DATA_ON_READERS_STATUS;
  }
  return ret;
}

// 定义数据可用回调函数，触发等待集合
// 触发数据可用回调的等待集函数
static void data_avail_cb_trigger_waitsets(dds_entity *rd, uint32_t signal)
{
  // 如果信号为0，则直接返回
  if (signal == 0)
    return;

  // 如果信号包含DDS_DATA_ON_READERS_STATUS
  if (signal & DDS_DATA_ON_READERS_STATUS)
  {
    // 获取读取器的父实体（订阅者）
    dds_entity *const sub = rd->m_parent;
    // 锁定订阅者的观察者锁
    ddsrt_mutex_lock(&sub->m_observers_lock);
    // 获取订阅者状态和掩码值
    const uint32_t sm = ddsrt_atomic_ld32(&sub->m_status.m_status_and_mask);
    // 如果状态和掩码值中包含DDS_DATA_ON_READERS_STATUS
    if ((sm & (sm >> SAM_ENABLED_SHIFT)) & DDS_DATA_ON_READERS_STATUS)
      // 向订阅者的观察者发送DDS_DATA_ON_READERS_STATUS信号
      dds_entity_observers_signal(sub, DDS_DATA_ON_READERS_STATUS);
    // 解锁订阅者的观察者锁
    ddsrt_mutex_unlock(&sub->m_observers_lock);
  }
  // 如果信号包含DDS_DATA_AVAILABLE_STATUS
  if (signal & DDS_DATA_AVAILABLE_STATUS)
  {
    // 获取读取器状态和掩码值
    const uint32_t sm = ddsrt_atomic_ld32(&rd->m_status.m_status_and_mask);
    // 如果状态和掩码值中包含DDS_DATA_AVAILABLE_STATUS
    if ((sm & (sm >> SAM_ENABLED_SHIFT)) & DDS_DATA_AVAILABLE_STATUS)
      // 向读取器的观察者发送DDS_DATA_AVAILABLE_STATUS信号
      dds_entity_observers_signal(rd, DDS_DATA_AVAILABLE_STATUS);
  }
}
void dds_reader_data_available_cb(struct dds_reader *rd)
{
  // DATA_AVAILABLE在两个方面是特殊的：首先，它应该首先尝试祖先上的DATA_ON_READERS，
  // 如果没有消耗，则在订阅者上设置状态；其次，它是唯一一个对开销真正重要的状态。
  // 否则，它与dds_reader_status_cb非常相似。
  struct dds_listener const *const lst = &rd->m_entity.m_listener;
  uint32_t signal = 0;

  ddsrt_mutex_lock(&rd->m_entity.m_observers_lock);
  const uint32_t status_and_mask = ddsrt_atomic_ld32(&rd->m_entity.m_status.m_status_and_mask);
  if (lst->on_data_on_readers == 0 && lst->on_data_available == 0)
    signal = data_avail_cb_set_status(&rd->m_entity, status_and_mask);
  else
  {
    // “锁定”监听器对象，以便我们可以在不持有m_observers_lock的情况下查看“lst”
    data_avail_cb_enter_listener_exclusive_access(&rd->m_entity);
    // 判断 lst 的 on_data_on_readers 是否为真
    if (lst->on_data_on_readers)
    {
      // 获取 rd 实体的父实体 sub
      dds_entity *const sub = rd->m_entity.m_parent;
      // 解锁 rd 实体的观察者锁
      ddsrt_mutex_unlock(&rd->m_entity.m_observers_lock);
      // 锁定 sub 实体的观察者锁
      ddsrt_mutex_lock(&sub->m_observers_lock);
      // 如果 reset_on_invoke 不包含 DDS_DATA_ON_READERS_STATUS 标志
      if (!(lst->reset_on_invoke & DDS_DATA_ON_READERS_STATUS))
        // 设置 rd 实体的状态，并返回是否需要触发信号
        signal = data_avail_cb_set_status(&rd->m_entity, status_and_mask);
      // 调用 data_avail_cb_invoke_dor 函数处理 sub 和 lst
      data_avail_cb_invoke_dor(sub, lst);
      // 解锁 sub 实体的观察者锁
      ddsrt_mutex_unlock(&sub->m_observers_lock);
      // 重新锁定 rd 实体的观察者锁
      ddsrt_mutex_lock(&rd->m_entity.m_observers_lock);
    }
    else
    {
      // 断言 rd 实体的监听器的 on_data_available 为真
      assert(rd->m_entity.m_listener.on_data_available);
      // 如果 reset_on_invoke 不包含 DDS_DATA_AVAILABLE_STATUS 标志
      if (!(lst->reset_on_invoke & DDS_DATA_AVAILABLE_STATUS))
        // 设置 rd 实体的状态，并返回是否需要触发信号
        signal = data_avail_cb_set_status(&rd->m_entity, status_and_mask);
      // 解锁 rd 实体的观察者锁
      ddsrt_mutex_unlock(&rd->m_entity.m_observers_lock);
      // 调用 lst 的 on_data_available 函数处理 rd 实体的句柄和参数
      lst->on_data_available(rd->m_entity.m_hdllink.hdl, lst->on_data_available_arg);
      // 重新锁定 rd 实体的观察者锁
      ddsrt_mutex_lock(&rd->m_entity.m_observers_lock);
    }
    // 离开监听器独占访问状态
    data_avail_cb_leave_listener_exclusive_access(&rd->m_entity);
  }
  // 触发等待集合的信号
  data_avail_cb_trigger_waitsets(&rd->m_entity, signal);
  // 解锁 rd 实体的观察者锁
  ddsrt_mutex_unlock(&rd->m_entity.m_observers_lock);
}
// 更新请求的截止时间未满足状态
static void update_requested_deadline_missed(struct dds_requested_deadline_missed_status *__restrict st, const ddsi_status_cb_data_t *data)
{
  // 设置最后一个实例句柄
  st->last_instance_handle = data->handle;
  // 计算总数并处理溢出情况
  uint64_t tmp = (uint64_t)data->extra + (uint64_t)st->total_count;
  st->total_count = tmp > UINT32_MAX ? UINT32_MAX : (uint32_t)tmp;
  // 始终递增st->total_count_change，然后复制到*lst
  // 这比最小工作量要多一点，但是这样可以保证在启用监听器后也能得到正确的值
  //
  // （对所有这些都有相同的推理）
  int64_t tmp2 = (int64_t)data->extra + (int64_t)st->total_count_change;
  st->total_count_change = tmp2 > INT32_MAX ? INT32_MAX : tmp2 < INT32_MIN ? INT32_MIN
                                                                           : (int32_t)tmp2;
}

// 更新请求的不兼容QoS状态
static void update_requested_incompatible_qos(struct dds_requested_incompatible_qos_status *__restrict st, const ddsi_status_cb_data_t *data)
{
  // 设置最后一个策略ID
  st->last_policy_id = data->extra;
  // 总数递增
  st->total_count++;
  // 总数变化递增
  st->total_count_change++;
}

// 更新丢失的样本状态
static void update_sample_lost(struct dds_sample_lost_status *__restrict st, const ddsi_status_cb_data_t *data)
{
  (void)data;
  // 总数递增
  st->total_count++;
  // 总数变化递增
  st->total_count_change++;
}
// 更新被拒绝的样本状态
static void update_sample_rejected(struct dds_sample_rejected_status *__restrict st, const ddsi_status_cb_data_t *data)
{
  // 设置最后一次拒绝的原因
  st->last_reason = data->extra;
  // 设置最后一次拒绝的实例句柄
  st->last_instance_handle = data->handle;
  // 增加总计数器
  st->total_count++;
  // 增加总计数变化
  st->total_count_change++;
}

// 更新活跃度变化状态
static void update_liveliness_changed(struct dds_liveliness_changed_status *__restrict st, const ddsi_status_cb_data_t *data)
{
  // 静态断言，确保枚举值的顺序和范围正确
  DDSRT_STATIC_ASSERT((uint32_t)DDSI_LIVELINESS_CHANGED_ADD_ALIVE == 0 &&
                      DDSI_LIVELINESS_CHANGED_ADD_ALIVE < DDSI_LIVELINESS_CHANGED_ADD_NOT_ALIVE &&
                      DDSI_LIVELINESS_CHANGED_ADD_NOT_ALIVE < DDSI_LIVELINESS_CHANGED_REMOVE_NOT_ALIVE &&
                      DDSI_LIVELINESS_CHANGED_REMOVE_NOT_ALIVE < DDSI_LIVELINESS_CHANGED_REMOVE_ALIVE &&
                      DDSI_LIVELINESS_CHANGED_REMOVE_ALIVE < DDSI_LIVELINESS_CHANGED_ALIVE_TO_NOT_ALIVE &&
                      DDSI_LIVELINESS_CHANGED_ALIVE_TO_NOT_ALIVE < DDSI_LIVELINESS_CHANGED_NOT_ALIVE_TO_ALIVE &&
                      (uint32_t)DDSI_LIVELINESS_CHANGED_NOT_ALIVE_TO_ALIVE < UINT32_MAX);
  // 断言，确保 data->extra 的值在有效范围内
  assert(data->extra <= (uint32_t)DDSI_LIVELINESS_CHANGED_NOT_ALIVE_TO_ALIVE);
  // 设置最后一次出版的句柄
  st->last_publication_handle = data->handle;
  // 根据 data->extra 的值更新活跃度计数器
  switch ((enum ddsi_liveliness_changed_data_extra)data->extra)
  {
  case DDSI_LIVELINESS_CHANGED_ADD_ALIVE:
    // 增加活跃计数器和活跃计数变化
    st->alive_count++;
    st->alive_count_change++;
    break;
  case DDSI_LIVELINESS_CHANGED_ADD_NOT_ALIVE:
    // 增加非活跃计数器和非活跃计数变化
    st->not_alive_count++;
    st->not_alive_count_change++;
    break;
  case DDSI_LIVELINESS_CHANGED_REMOVE_NOT_ALIVE:
    // 减少非活跃计数器和非活跃计数变化
    st->not_alive_count--;
    st->not_alive_count_change--;
    break;
  case DDSI_LIVELINESS_CHANGED_REMOVE_ALIVE:
    // 减少活跃计数器和活跃计数变化
    st->alive_count--;
    st->alive_count_change--;
    break;
  case DDSI_LIVELINESS_CHANGED_ALIVE_TO_NOT_ALIVE:
    // 将活跃状态转为非活跃状态，更新计数器和计数变化
    st->alive_count--;
    st->alive_count_change--;
    st->not_alive_count++;
    st->not_alive_count_change++;
    break;
  case DDSI_LIVELINESS_CHANGED_NOT_ALIVE_TO_ALIVE:
    // 将非活跃状态转为活跃状态，更新计数器和计数变化
    st->not_alive_count--;
    st->not_alive_count_change--;
    st->alive_count++;
    st->alive_count_change++;
    break;
  }
}
// 更新订阅匹配状态
static void update_subscription_matched(struct dds_subscription_matched_status *__restrict st, const ddsi_status_cb_data_t *data)
{
  // 设置最后一个出版物句柄
  st->last_publication_handle = data->handle;
  // 如果需要添加
  if (data->add)
  {
    // 更新各种计数器
    st->total_count++;
    st->current_count++;
    st->total_count_change++;
    st->current_count_change++;
  }
  else
  {
    // 更新当前计数器
    st->current_count--;
    st->current_count_change--;
  }
}

// 确保未被拒绝的值为0
/* Reset sets everything (type) 0, including the reason field, verify that 0 is correct */
DDSRT_STATIC_ASSERT((int)DDS_NOT_REJECTED == 0);

// 获取不同状态的宏定义
DDS_GET_STATUS(reader, subscription_matched, SUBSCRIPTION_MATCHED, total_count_change, current_count_change)
DDS_GET_STATUS(reader, liveliness_changed, LIVELINESS_CHANGED, alive_count_change, not_alive_count_change)
DDS_GET_STATUS(reader, sample_rejected, SAMPLE_REJECTED, total_count_change)
DDS_GET_STATUS(reader, sample_lost, SAMPLE_LOST, total_count_change)
DDS_GET_STATUS(reader, requested_deadline_missed, REQUESTED_DEADLINE_MISSED, total_count_change)
DDS_GET_STATUS(reader, requested_incompatible_qos, REQUESTED_INCOMPATIBLE_QOS, total_count_change)

// 实现不同状态的回调函数
STATUS_CB_IMPL(reader, subscription_matched, SUBSCRIPTION_MATCHED, total_count_change, current_count_change)
STATUS_CB_IMPL(reader, liveliness_changed, LIVELINESS_CHANGED, alive_count_change, not_alive_count_change)
STATUS_CB_IMPL(reader, sample_rejected, SAMPLE_REJECTED, total_count_change)
STATUS_CB_IMPL(reader, sample_lost, SAMPLE_LOST, total_count_change)
STATUS_CB_IMPL(reader, requested_deadline_missed, REQUESTED_DEADLINE_MISSED, total_count_change)
STATUS_CB_IMPL(reader, requested_incompatible_qos, REQUESTED_INCOMPATIBLE_QOS, total_count_change)

// 读者状态回调函数
void dds_reader_status_cb(void *ventity, const ddsi_status_cb_data_t *data)
{
  // 定义读者实体
  dds_reader *const rd = ventity;

  // 当数据为NULL时，表示DDSI读者已被删除
  if (data == NULL)
  {
    // 释放创建过程中的初始声明，允许进一步的API删除操作
    ddsrt_mutex_lock(&rd->m_entity.m_mutex);
    rd->m_rd = NULL;
    ddsrt_cond_broadcast(&rd->m_entity.m_cond);
    ddsrt_mutex_unlock(&rd->m_entity.m_mutex);
    return;
  }

  // 序列化监听器调用
  ddsrt_mutex_lock(&rd->m_entity.m_observers_lock);
  rd->m_entity.m_cb_pending_count++;
  while (rd->m_entity.m_cb_count > 0)
    ddsrt_cond_wait(&rd->m_entity.m_observers_cond, &rd->m_entity.m_observers_lock);
  rd->m_entity.m_cb_count++;

  // 获取状态ID
  const enum dds_status_id status_id = (enum dds_status_id)data->raw_status_id;
  // 根据状态ID执行相应的回调函数
  switch (status_id)
  {
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
  // 其他未处理的状态ID
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
  ddsrt_cond_broadcast(&rd->m_entity.m_observers_cond);
  ddsrt_mutex_unlock(&rd->m_entity.m_observers_lock);
}
// 定义一个名为dds_reader_statistics_kv的静态常量结构体数组，包含一个键值对
static const struct dds_stat_keyvalue_descriptor dds_reader_statistics_kv[] = {
    {"discarded_bytes", DDS_STAT_KIND_UINT64}};

// 定义一个名为dds_reader_statistics_desc的静态常量结构体，包含两个成员：count和kv
static const struct dds_stat_descriptor dds_reader_statistics_desc = {
    .count = sizeof(dds_reader_statistics_kv) / sizeof(dds_reader_statistics_kv[0]),
    .kv = dds_reader_statistics_kv};

// 定义一个名为dds_reader_create_statistics的静态函数，输入参数为一个指向dds_entity结构体的指针
static struct dds_statistics *dds_reader_create_statistics(const struct dds_entity *entity)
{
  // 调用dds_alloc_statistics函数并返回结果
  return dds_alloc_statistics(entity, &dds_reader_statistics_desc);
}

// 定义一个名为dds_reader_refresh_statistics的静态函数，输入参数为一个指向dds_entity结构体的指针和一个指向dds_statistics结构体的指针
static void dds_reader_refresh_statistics(const struct dds_entity *entity, struct dds_statistics *stat)
{
  // 将输入的dds_entity结构体指针强制转换为指向dds_reader结构体的指针
  const struct dds_reader *rd = (const struct dds_reader *)entity;
  // 如果rd->m_rd不为空，则调用ddsi_get_reader_stats函数更新stat->kv[0].u.u64的值
  if (rd->m_rd)
    ddsi_get_reader_stats(rd->m_rd, &stat->kv[0].u.u64);
}

// 定义一个名为dds_entity_deriver_reader的静态常量结构体，包含多个函数指针成员
const struct dds_entity_deriver dds_entity_deriver_reader = {
    .interrupt = dds_entity_deriver_dummy_interrupt,
    .close = dds_reader_close,
    .delete = dds_reader_delete,
    .set_qos = dds_reader_qos_set,
    .validate_status = dds_reader_status_validate,
    .create_statistics = dds_reader_create_statistics,
    .refresh_statistics = dds_reader_refresh_statistics};
// 如果定义了DDS_HAS_SHM宏
#ifdef DDS_HAS_SHM
// 创建iox_sub_options_t结构体的函数，参数为dds_qos_t指针
static iox_sub_options_t create_iox_sub_options(const dds_qos_t *qos)
{
  // 定义一个iox_sub_options_t类型的变量opts
  iox_sub_options_t opts;
  // 初始化opts
  iox_sub_options_init(&opts);

  // 获取最大的订阅者队列容量
  const uint32_t max_sub_queue_capacity = iox_cfg_max_subscriber_queue_capacity();

  // 注意：当接收到history.depth个样本后，我们可能会丢失数据（如果我们没有足够快地从iceoryx队列中获取它们并将它们移动到读取器历史缓存中），
  // 但这对于volatile来说是有效的行为。然而，如果数据尽可能快地发布，队列可能会很快填满，这可能导致不希望出现的行为。
  // 注意：如果历史深度大于队列容量，我们仍然使用共享内存，但相应地限制queueCapacity（否则iceoryx会发出警告并自行限制）

  if ((uint32_t)qos->history.depth <= max_sub_queue_capacity)
  {
    opts.queueCapacity = (uint64_t)qos->history.depth;
  }
  else
  {
    opts.queueCapacity = max_sub_queue_capacity;
  }

  // 对于BEST EFFORT，DDS要求不接收历史数据（无论持久性如何）
  if (qos->reliability.kind == DDS_RELIABILITY_BEST_EFFORT ||
      qos->durability.kind == DDS_DURABILITY_VOLATILE)
  {
    opts.historyRequest = 0;
  }
  else
  {
    // 对于TRANSIENT LOCAL和更强的持久性
    opts.historyRequest = (uint64_t)qos->history.depth;
    // 如果发布者不支持历史数据，它将不会被iceoryx连接
    opts.requirePublisherHistorySupport = true;
  }

  // 返回opts结构体
  return opts;
}
#endif
// 定义一个静态函数 dds_create_reader_int，用于创建数据读取器
static dds_entity_t dds_create_reader_int(
    dds_entity_t participant_or_subscriber, // 参与者或订阅者实体
    dds_entity_t topic,                     // 主题实体
    const dds_qos_t *qos,                   // 服务质量配置
    const dds_listener_t *listener,         // 监听器
    struct dds_rhc *rhc)                    // 资源历史缓存
{
  // 声明变量
  dds_qos_t *rqos;
  dds_subscriber *sub = NULL;
  dds_entity_t subscriber;
  dds_topic *tp;
  dds_return_t rc;
  dds_entity_t pseudo_topic = 0;
  bool created_implicit_sub = false;

  // 根据主题类型进行处理
  switch (topic)
  {
  case DDS_BUILTIN_TOPIC_DCPSTOPIC:
#ifndef DDS_HAS_TOPIC_DISCOVERY
    // 如果不支持主题发现，则返回不支持错误码
    return DDS_RETCODE_UNSUPPORTED;
#endif
  case DDS_BUILTIN_TOPIC_DCPSPARTICIPANT:
  case DDS_BUILTIN_TOPIC_DCPSPUBLICATION:
  case DDS_BUILTIN_TOPIC_DCPSSUBSCRIPTION:
    // 将提供的伪主题转换为真实主题
    // 将传入的topic赋值给pseudo_topic
    pseudo_topic = topic;
    // 获取内置订阅者，如果获取失败则返回错误码
    if ((subscriber = dds__get_builtin_subscriber(participant_or_subscriber)) < 0)
      return subscriber;
    // 对订阅者进行加锁操作，如果加锁失败则返回错误码
    if ((rc = dds_subscriber_lock(subscriber, &sub)) != DDS_RETCODE_OK)
      return rc;
    // 获取内置主题，并将其赋值给topic变量
    topic = dds__get_builtin_topic(subscriber, topic);
    break;

  default:
  {
    // 定义一个dds_entity类型的指针p_or_s
    dds_entity *p_or_s;
    // 对实体进行加锁操作，如果加锁失败则返回错误码
    if ((rc = dds_entity_lock(participant_or_subscriber, DDS_KIND_DONTCARE, &p_or_s)) != DDS_RETCODE_OK)
      return rc;
    // 根据实体类型进行不同的处理
    switch (dds_entity_kind(p_or_s))
    {
    case DDS_KIND_SUBSCRIBER:
      // 如果实体类型为订阅者，则将participant_or_subscriber赋值给subscriber，并将p_or_s强制转换为dds_subscriber类型后赋值给sub
      subscriber = participant_or_subscriber;
      sub = (dds_subscriber *)p_or_s;
      break;
    case DDS_KIND_PARTICIPANT:
      // 如果实体类型为参与者，则创建一个隐式订阅者并将其赋值给subscriber
      created_implicit_sub = true;
      subscriber = dds__create_subscriber_l((dds_participant *)p_or_s, true, qos, NULL);
      // 解锁实体
      dds_entity_unlock(p_or_s);
      // 对新创建的订阅者进行加锁操作，如果加锁失败则返回错误码
      if ((rc = dds_subscriber_lock(subscriber, &sub)) < 0)
        return rc;
      break;
    default:
      // 如果实体类型不是订阅者或参与者，则解锁实体并返回非法操作错误码
      dds_entity_unlock(p_or_s);
      return DDS_RETCODE_ILLEGAL_OPERATION;
    }
    break;
  }
  }
}
// 如果 pseudo_topic 不等于0，表示主题不是由应用程序产生的，我们允许将其固定，
// 即使它被标记为 NO_USER_ACCESS
if ((rc = dds_topic_pin_with_origin(topic, pseudo_topic ? false : true, &tp)) < 0)
  goto err_pin_topic;
assert(tp->m_stype);

// 检查订阅者和主题的参与者是否相同，如果不同则返回错误代码
if (dds_entity_participant(&sub->m_entity) != dds_entity_participant(&tp->m_entity))
{
  rc = DDS_RETCODE_BAD_PARAMETER;
  goto err_pp_mismatch;
}

// 在创建并注册读取器之前，阻止对主题进行 set_qos 操作：我们不能允许在创建读取器之前发生 TOPIC_DATA 更改，
// 因为这样的更改将不会在发现/内置主题中发布。
//
// 不要保持参与者（保护主题的 QoS）锁定，因为这可能导致死锁，例如在订阅匹配监听器中创建读取器/写入器的应用程序。
dds_topic_defer_set_qos(tp);

// 合并主题和订阅者的 qos，dds_copy_qos 只有在传递空参数时才会失败，
// 但在这里不是这种情况
struct ddsi_domaingv *gv = &sub->m_entity.m_domain->gv;
rqos = dds_create_qos();
if (qos)
  ddsi_xqos_mergein_missing(rqos, qos, DDS_READER_QOS_MASK);
if (sub->m_entity.m_qos)
  ddsi_xqos_mergein_missing(rqos, sub->m_entity.m_qos, ~DDSI_QP_ENTITY_NAME);
if (tp->m_ktopic->qos)
  ddsi_xqos_mergein_missing(rqos, tp->m_ktopic->qos, (DDS_READER_QOS_MASK | DDSI_QP_TOPIC_DATA) & ~DDSI_QP_ENTITY_NAME);
ddsi_xqos_mergein_missing(rqos, &ddsi_default_qos_reader, ~DDSI_QP_DATA_REPRESENTATION);
dds_apply_entity_naming(rqos, sub->m_entity.m_qos, gv);

// 确保数据表示有效
if ((rc = dds_ensure_valid_data_representation(rqos, tp->m_stype->allowed_data_representation, false)) != 0)
  goto err_data_repr;

// 检查 QoS 是否有效
if ((rc = ddsi_xqos_valid(&gv->logconfig, rqos)) < 0 || (rc = validate_reader_qos(rqos)) != DDS_RETCODE_OK)
  goto err_bad_qos;

// 对于内置主题需要进行额外的检查：我们不希望在内置主题上遇到资源限制，这是一个不必要的复杂性
if (pseudo_topic && !dds__validate_builtin_reader_qos(tp->m_entity.m_domain, pseudo_topic, rqos))
{
  rc = DDS_RETCODE_INCONSISTENT_POLICY;
  goto err_bad_qos;
}

// 唤醒当前线程状态
ddsi_thread_state_awake(ddsi_lookup_thread_state(), gv);
// 获取参与者（participant）的GUID
const struct ddsi_guid *ppguid = dds_entity_participant_guid(&sub->m_entity);
// 根据GUID查找对应的参与者实体
struct ddsi_participant *pp = ddsi_entidx_lookup_participant_guid(gv->entity_index, ppguid);

// 当删除一个参与者时，子句柄（包括订阅者）会在删除DDSI参与者之前被移除。
// 因此，在此时，在订阅者锁内，我们可以断言参与者存在。
assert(pp != NULL);

#ifdef DDS_HAS_SECURITY
// 检查DDS安全功能是否启用
if (ddsi_omg_participant_is_secure(pp))
{
  // 向访问控制安全插件请求创建读取器权限
  if (!ddsi_omg_security_check_create_reader(pp, gv->config.domainId, tp->m_name, rqos))
  {
    // 如果没有权限，则返回错误代码
    rc = DDS_RETCODE_NOT_ALLOWED_BY_SECURITY;
    // 使当前线程进入休眠状态
    ddsi_thread_state_asleep(ddsi_lookup_thread_state());
    // 跳转到错误处理部分
    goto err_bad_qos;
  }
}
#endif

// 创建读取器和关联的读取缓存（如果调用者没有提供）
struct dds_reader *const rd = dds_alloc(sizeof(*rd));
// 初始化读取器实体，并将其添加到订阅者的子实体中
const dds_entity_t reader = dds_entity_init(&rd->m_entity, &sub->m_entity, DDS_KIND_READER, false, true, rqos, listener, DDS_READER_STATUS_MASK);
// 假设 DATA_ON_READERS 在订阅者中实现：
// - 对它的更改在将此读取器添加到订阅者的子节点之后才会传播到此读取器
// - 一旦调用 `new_reader`，数据就可以到达，需要在实现时提高 DATA_ON_READERS
// - 如果实际上没有实现 DATA_ON_READERS，则在订阅者上设置 DATA_ON_READERS 没有问题
ddsrt_atomic_or32(&rd->m_entity.m_status.m_status_and_mask, DDS_DATA_ON_READERS_STATUS << SAM_ENABLED_SHIFT);
// 设置最后拒绝原因为未拒绝
rd->m_sample_rejected_status.last_reason = DDS_NOT_REJECTED;
// 设置主题
rd->m_topic = tp;
// 设置读取缓存
rd->m_rhc = rhc ? rhc : dds_rhc_default_new(rd, tp->m_stype);
// 关联读取缓存
if (dds_rhc_associate(rd->m_rhc, rd, tp->m_stype, rd->m_entity.m_domain->gv.m_tkmap) < 0)
{
  // 如果关联失败，需要撤销实体初始化
  abort();
}
// 增加主题实体的引用计数
dds_entity_add_ref_locked(&tp->m_entity);

// 设置监听器可能过早，应根据监听器设置掩码
// 然后原子地设置监听器，将掩码保存到待处理集合并清除它；
// 然后调用处于待处理集合中的那些监听器
dds_entity_init_complete(&rd->m_entity);

#ifdef DDS_HAS_SHM
// 检查共享内存是否兼容
assert(rqos->present &DDSI_QP_LOCATOR_MASK);
if (!(gv->config.enable_shm && dds_shm_compatible_qos_and_topic(rqos, tp, true)))
  rqos->ignore_locator_type |= DDSI_LOCATOR_KIND_SHEM;
#endif

// 读取器从主题获取序列化类型，因为读取器使用的序列化数据函数
// 不是特定于数据表示的（可以从 cdr 头部检索表示）
rc = ddsi_new_reader(&rd->m_rd, &rd->m_entity.m_guid, NULL, pp, tp->m_name, tp->m_stype, rqos, &rd->m_rhc->common.rhc, dds_reader_status_cb, rd);
// 检查返回值是否正确（至少可能是资源不足）
assert(rc == DDS_RETCODE_OK);
// 将线程状态设置为休眠
ddsi_thread_state_asleep(ddsi_lookup_thread_state());
// 如果定义了DDS_HAS_SHM宏
#ifdef DDS_HAS_SHM
// 如果读取器的m_rd成员变量中的has_iceoryx为真
if (rd->m_rd->has_iceoryx)
{
  // 输出日志，显示读取器的主题名称
  DDS_CLOG(DDS_LC_SHM, &rd->m_entity.m_domain->gv.logconfig, "Reader's topic name will be DDS:Cyclone:%s\n", rd->m_topic->m_name);

  // 初始化iox子上下文
  iox_sub_context_init(&rd->m_iox_sub_context);

  // 创建iox订阅者选项
  iox_sub_options_t opts = create_iox_sub_options(rqos);

  // 快速hack，使分区工作；使用*标记分隔分区名称和主题名称
  // 因为我们已经知道分区不能再包含*了
  char *part_topic = dds_shm_partition_topic(rqos, rd->m_topic);
  assert(part_topic != NULL);
  // 初始化iox订阅者
  rd->m_iox_sub = iox_sub_init(&(iox_sub_storage_t){0}, gv->config.iceoryx_service, rd->m_topic->m_stype->type_name, part_topic, &opts);
  // 释放part_topic内存
  ddsrt_free(part_topic);

  // 注意：由于iceoryx结构的存储范式发生了变化
  // 我们现在在m_iox_sub之前有一个指针8字节
  // 我们使用这个地址来存储指向上下文的指针。
  iox_sub_context_t **context = iox_sub_context_ptr(rd->m_iox_sub);
  *context = &rd->m_iox_sub_context;

  // 将读取器附加到共享内存监视器
  rc = dds_shm_monitor_attach_reader(&rd->m_entity.m_domain->m_shm_monitor, rd);

  // 如果返回码不是DDS_RETCODE_OK
  if (rc != DDS_RETCODE_OK)
  {
    // 如果无法附加到监听器，我们将失败（因为我们将无法获取数据）
    iox_sub_deinit(rd->m_iox_sub);
    rd->m_iox_sub = NULL;
    // 输出警告日志
    DDS_CLOG(DDS_LC_WARNING | DDS_LC_SHM,
             &rd->m_entity.m_domain->gv.logconfig,
             "Failed to attach iox subscriber to iox listener\n");
    // FIXME: 我们需要清理到现在为止创建的所有内容。
    //        目前只有部分清理，我们需要扩展它。
    goto err_bad_qos;
  }

  // 这些值设置一次，永远不会改变
  // 当接收到数据时，它们用于从回调访问读取器和监视器
  rd->m_iox_sub_context.monitor = &rd->m_entity.m_domain->m_shm_monitor;
  rd->m_iox_sub_context.parent_reader = rd;
}
#endif
// 为 rd->m_entity.m_iid 设置实体实例 ID，使用 ddsi_get_entity_instanceid 函数从域和 GUID 中获取
rd->m_entity.m_iid = ddsi_get_entity_instanceid(&rd->m_entity.m_domain->gv, &rd->m_entity.m_guid);

// 在订阅者的子实体中注册读取器实体
dds_entity_register_child(&sub->m_entity, &rd->m_entity);

// 在包含读取器作为订阅者子实体之后，订阅者将开始传播 data_on_readers 是否实现。
// 这里没有考虑到悲观地将其设置为已实现的情况，也没有考虑到在 `dds_entity_register_child` 之前实际上已经实现但不再实现的竞争情况。
ddsrt_mutex_lock(&rd->m_entity.m_observers_lock);
ddsrt_mutex_lock(&sub->m_entity.m_observers_lock);

// 如果订阅者的 materialize_data_on_readers 为 0，则更新读取器实体的状态和掩码
if (sub->materialize_data_on_readers == 0)
  ddsrt_atomic_and32(&rd->m_entity.m_status.m_status_and_mask, ~(uint32_t)(DDS_DATA_ON_READERS_STATUS << SAM_ENABLED_SHIFT));

// 解锁互斥锁
ddsrt_mutex_unlock(&sub->m_entity.m_observers_lock);
ddsrt_mutex_unlock(&rd->m_entity.m_observers_lock);

// 允许设置主题的 QoS
dds_topic_allow_set_qos(tp);

// 取消固定主题
dds_topic_unpin(tp);

// 解锁订阅者
dds_subscriber_unlock(sub);

// 返回读取器实体
return reader;

// 错误处理部分
err_bad_qos : err_data_repr : dds_delete_qos(rqos);
dds_topic_allow_set_qos(tp);
err_pp_mismatch : dds_topic_unpin(tp);
err_pin_topic : dds_subscriber_unlock(sub);
if (created_implicit_sub)
  (void)dds_delete(subscriber);
return rc;
}

// 创建读取器实体的函数，接受参与者或订阅者、主题、QoS 和监听器作为参数
dds_entity_t dds_create_reader(dds_entity_t participant_or_subscriber, dds_entity_t topic, const dds_qos_t *qos, const dds_listener_t *listener)
{
  return dds_create_reader_int(participant_or_subscriber, topic, qos, listener, NULL);
}

// 创建带有 RHC 的读取器实体的函数，接受参与者或订阅者、主题、QoS、监听器和 RHC 作为参数
dds_entity_t dds_create_reader_rhc(dds_entity_t participant_or_subscriber, dds_entity_t topic, const dds_qos_t *qos, const dds_listener_t *listener, struct dds_rhc *rhc)
{
  // 如果 RHC 为空，则返回错误代码
  if (rhc == NULL)
    return DDS_RETCODE_BAD_PARAMETER;

  // 调用内部创建读取器实体的函数
  return dds_create_reader_int(participant_or_subscriber, topic, qos, listener, rhc);
}
// dds_reader_lock_samples 函数用于锁定 reader 中的样本
uint32_t dds_reader_lock_samples(dds_entity_t reader)
{
  // 定义一个 dds_reader 类型的指针 rd
  dds_reader *rd;
  // 定义一个 uint32_t 类型的变量 n
  uint32_t n;
  // 如果锁定 reader 失败，返回 0
  if (dds_reader_lock(reader, &rd) != DDS_RETCODE_OK)
    return 0;
  // 锁定 rd 的样本，并将结果赋值给 n
  n = dds_rhc_lock_samples(rd->m_rhc);
  // 解锁 rd
  dds_reader_unlock(rd);
  // 返回 n
  return n;
}

// dds_reader_wait_for_historical_data 函数用于等待历史数据
dds_return_t dds_reader_wait_for_historical_data(dds_entity_t reader, dds_duration_t max_wait)
{
  // 定义一个 dds_reader 类型的指针 rd
  dds_reader *rd;
  // 定义一个 dds_return_t 类型的变量 ret
  dds_return_t ret;
  // 忽略 max_wait 参数
  (void)max_wait;
  // 如果锁定 reader 失败，返回错误码
  if ((ret = dds_reader_lock(reader, &rd)) != DDS_RETCODE_OK)
    return ret;
  // 根据 rd 的持久性类型进行处理
  switch (rd->m_entity.m_qos->durability.kind)
  {
  case DDS_DURABILITY_VOLATILE:
    ret = DDS_RETCODE_OK;
    break;
  case DDS_DURABILITY_TRANSIENT_LOCAL:
    break;
  case DDS_DURABILITY_TRANSIENT:
  case DDS_DURABILITY_PERSISTENT:
    break;
  }
  // 解锁 rd
  dds_reader_unlock(rd);
  // 返回 ret
  return ret;
}

// dds_get_subscriber 函数用于获取实体的订阅者
dds_entity_t dds_get_subscriber(dds_entity_t entity)
{
  // 定义一个 dds_entity 类型的指针 e
  dds_entity *e;
  // 定义一个 dds_return_t 类型的变量 ret
  dds_return_t ret;
  // 如果锁定实体失败，返回错误码
  if ((ret = dds_entity_pin(entity, &e)) != DDS_RETCODE_OK)
    return ret;
  else
  {
    // 定义一个 dds_entity_t 类型的变量 subh
    dds_entity_t subh;
    // 根据实体类型进行处理
    // 根据实体类型执行相应操作
    switch (dds_entity_kind(e))
    {
    // 如果实体类型为读取器
    case DDS_KIND_READER:
      // 断言实体的父级类型为订阅者
      assert(dds_entity_kind(e->m_parent) == DDS_KIND_SUBSCRIBER);
      // 获取父级实体的句柄
      subh = e->m_parent->m_hdllink.hdl;
      // 跳出 switch 语句
      break;

    // 如果实体类型为条件读取或条件查询
    case DDS_KIND_COND_READ:
    case DDS_KIND_COND_QUERY:
      // 断言实体的父级类型为读取器
      assert(dds_entity_kind(e->m_parent) == DDS_KIND_READER);
      // 断言实体的父级的父级类型为订阅者
      assert(dds_entity_kind(e->m_parent->m_parent) == DDS_KIND_SUBSCRIBER);
      // 获取父级实体的父级实体的句柄
      subh = e->m_parent->m_parent->m_hdllink.hdl;
      // 跳出 switch 语句
      break;

    // 其他情况
    default:
      // 设置返回代码为非法操作
      subh = DDS_RETCODE_ILLEGAL_OPERATION;
      // 跳出 switch 语句
      break;
    }
    // 解锁实体
    dds_entity_unpin(e);
    // 返回 subh
    return subh;
  }
}
// 初始化dds_reader的数据分配器
dds_return_t dds__reader_data_allocator_init(const dds_reader *rd, dds_data_allocator_t *data_allocator)
{
#ifdef DDS_HAS_SHM
  // 定义一个指向dds_iox_allocator_t类型的指针d，并将其指向data_allocator的opaque字节
  dds_iox_allocator_t *d = (dds_iox_allocator_t *)data_allocator->opaque.bytes;
  // 初始化互斥锁
  ddsrt_mutex_init(&d->mutex);
  // 如果rd的m_iox_sub成员不为空
  if (NULL != rd->m_iox_sub)
  {
    // 设置d的kind为DDS_IOX_ALLOCATOR_KIND_SUBSCRIBER
    d->kind = DDS_IOX_ALLOCATOR_KIND_SUBSCRIBER;
    // 将d的ref.sub设置为rd的m_iox_sub成员
    d->ref.sub = rd->m_iox_sub;
  }
  else
  {
    // 否则，设置d的kind为DDS_IOX_ALLOCATOR_KIND_NONE
    d->kind = DDS_IOX_ALLOCATOR_KIND_NONE;
  }
  // 返回DDS_RETCODE_OK
  return DDS_RETCODE_OK;
#else
  // 如果没有定义DDS_HAS_SHM，则忽略rd和data_allocator参数
  (void)rd;
  (void)data_allocator;
  // 返回DDS_RETCODE_OK
  return DDS_RETCODE_OK;
#endif
}

// 销毁dds_reader的数据分配器
dds_return_t dds__reader_data_allocator_fini(const dds_reader *rd, dds_data_allocator_t *data_allocator)
{
#ifdef DDS_HAS_SHM
  // 定义一个指向dds_iox_allocator_t类型的指针d，并将其指向data_allocator的opaque字节
  dds_iox_allocator_t *d = (dds_iox_allocator_t *)data_allocator->opaque.bytes;
  // 销毁互斥锁
  ddsrt_mutex_destroy(&d->mutex);
  // 设置d的kind为DDS_IOX_ALLOCATOR_KIND_FINI
  d->kind = DDS_IOX_ALLOCATOR_KIND_FINI;
#else
  // 如果没有定义DDS_HAS_SHM，则忽略data_allocator参数
  (void)data_allocator;
#endif
  // 忽略rd参数
  (void)rd;
  // 返回DDS_RETCODE_OK
  return DDS_RETCODE_OK;
}
