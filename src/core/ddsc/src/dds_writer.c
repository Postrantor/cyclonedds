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

#include "dds/cdr/dds_cdrstream.h"
#include "dds/dds.h"
#include "dds/ddsi/ddsi_domaingv.h"
#include "dds/ddsi/ddsi_endpoint.h"
#include "dds/ddsi/ddsi_entity.h"
#include "dds/ddsi/ddsi_entity_index.h"
#include "dds/ddsi/ddsi_security_omg.h"
#include "dds/ddsi/ddsi_statistics.h"
#include "dds/ddsi/ddsi_thread.h"
#include "dds/ddsi/ddsi_tkmap.h"
#include "dds/ddsi/ddsi_xmsg.h"
#include "dds/ddsrt/heap.h"
#include "dds/ddsrt/io.h"
#include "dds/ddsrt/static_assert.h"
#include "dds/version.h"
#include "dds__data_allocator.h"
#include "dds__get_status.h"
#include "dds__init.h"
#include "dds__listener.h"
#include "dds__publisher.h"
#include "dds__qos.h"
#include "dds__statistics.h"
#include "dds__topic.h"
#include "dds__whc.h"
#include "dds__writer.h"

#ifdef DDS_HAS_SHM
#include "dds__shm_qos.h"
#endif

/** @brief 定义实体锁和解锁的宏 */
DECL_ENTITY_LOCK_UNLOCK(dds_writer)

/** @brief 定义 writer 状态掩码 */
#define DDS_WRITER_STATUS_MASK                                       \
  (DDS_LIVELINESS_LOST_STATUS | DDS_OFFERED_DEADLINE_MISSED_STATUS | \
   DDS_OFFERED_INCOMPATIBLE_QOS_STATUS | DDS_PUBLICATION_MATCHED_STATUS)

/**
 * @brief 验证 writer 状态掩码
 *
 * @param[in] mask 要验证的状态掩码
 * @return dds_return_t 返回操作结果状态码
 */
static dds_return_t dds_writer_status_validate(uint32_t mask)
{
  return (mask & ~DDS_WRITER_STATUS_MASK) ? DDS_RETCODE_BAD_PARAMETER : DDS_RETCODE_OK;
}

/**
 * @brief 更新提供的截止日期未满足的状态
 *
 * @param[out] st 提供的截止日期未满足状态结构体指针
 * @param[in] data 状态回调数据结构体指针
 */
static void update_offered_deadline_missed(
  struct dds_offered_deadline_missed_status * __restrict st, const ddsi_status_cb_data_t * data)
{
  st->last_instance_handle = data->handle;
  uint64_t tmp = (uint64_t)data->extra + (uint64_t)st->total_count;
  st->total_count = tmp > UINT32_MAX ? UINT32_MAX : (uint32_t)tmp;
  // 总是递增 st->total_count_change，然后复制到 *lst 是
  // 比最小工作量稍微多一点，但这保证了在启用监听器后
  // 一些事件发生时的正确值
  //
  // （所有这些都是相同的推理）
  int64_t tmp2 = (int64_t)data->extra + (int64_t)st->total_count_change;
  st->total_count_change = tmp2 > INT32_MAX   ? INT32_MAX
                           : tmp2 < INT32_MIN ? INT32_MIN
                                              : (int32_t)tmp2;
}

/**
 * @brief 更新提供的不兼容 QoS 状态
 *
 * @param[out] st 提供的不兼容 QoS 状态结构体指针
 * @param[in] data 状态回调数据结构体指针
 */
static void update_offered_incompatible_qos(
  struct dds_offered_incompatible_qos_status * __restrict st, const ddsi_status_cb_data_t * data)
{
  st->last_policy_id = data->extra;
  st->total_count++;
  st->total_count_change++;
}
/**
 * @brief 更新活跃度丢失状态
 *
 * @param[out] st 活跃度丢失状态结构体指针
 * @param[in] data 状态回调数据结构体指针
 */
static void update_liveliness_lost(
  struct dds_liveliness_lost_status * __restrict st, const ddsi_status_cb_data_t * data)
{
  (void)data;
  st->total_count++;
  st->total_count_change++;
}

/**
 * @brief 更新出版物匹配状态
 *
 * @param[out] st 出版物匹配状态结构体指针
 * @param[in] data 状态回调数据结构体指针
 */
static void update_publication_matched(
  struct dds_publication_matched_status * __restrict st, const ddsi_status_cb_data_t * data)
{
  st->last_subscription_handle = data->handle;
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

/** 定义获取 writer 状态的宏 */
DDS_GET_STATUS(
  writer, publication_matched, PUBLICATION_MATCHED, total_count_change, current_count_change)
DDS_GET_STATUS(writer, liveliness_lost, LIVELINESS_LOST, total_count_change)
DDS_GET_STATUS(writer, offered_deadline_missed, OFFERED_DEADLINE_MISSED, total_count_change)
DDS_GET_STATUS(writer, offered_incompatible_qos, OFFERED_INCOMPATIBLE_QOS, total_count_change)

/** 定义实现 writer 状态回调的宏 */
STATUS_CB_IMPL(
  writer, publication_matched, PUBLICATION_MATCHED, total_count_change, current_count_change)
STATUS_CB_IMPL(writer, liveliness_lost, LIVELINESS_LOST, total_count_change)
STATUS_CB_IMPL(writer, offered_deadline_missed, OFFERED_DEADLINE_MISSED, total_count_change)
STATUS_CB_IMPL(writer, offered_incompatible_qos, OFFERED_INCOMPATIBLE_QOS, total_count_change)
/**
 * @brief DDS写入器状态回调函数
 *
 * @param[in] entity 指向dds_writer实体的指针
 * @param[in] data 包含状态回调数据的结构体指针
 */
void dds_writer_status_cb(void * entity, const struct ddsi_status_cb_data * data)
{
  // 将传入的实体指针转换为dds_writer类型的指针
  dds_writer * const wr = entity;

  /* 当data为NULL时，表示DDSI读取器已被删除。 */
  if (data == NULL) {
    /* 在创建过程中释放初始声明。这将表明现在可以进行进一步的API删除操作。 */
    ddsrt_mutex_lock(&wr->m_entity.m_mutex);
    wr->m_wr = NULL;
    ddsrt_cond_broadcast(&wr->m_entity.m_cond);
    ddsrt_mutex_unlock(&wr->m_entity.m_mutex);
    return;
  }

  /* FIXME: 如果没有设置监听器，为什么要等待？ */
  ddsrt_mutex_lock(&wr->m_entity.m_observers_lock);
  wr->m_entity.m_cb_pending_count++;
  while (wr->m_entity.m_cb_count > 0)
    ddsrt_cond_wait(&wr->m_entity.m_observers_cond, &wr->m_entity.m_observers_lock);
  wr->m_entity.m_cb_count++;

  // 获取状态ID
  const enum dds_status_id status_id = (enum dds_status_id)data->raw_status_id;
  switch (status_id) {
    case DDS_OFFERED_DEADLINE_MISSED_STATUS_ID:
      status_cb_offered_deadline_missed(wr, data);
      break;
    case DDS_LIVELINESS_LOST_STATUS_ID:
      status_cb_liveliness_lost(wr, data);
      break;
    case DDS_OFFERED_INCOMPATIBLE_QOS_STATUS_ID:
      status_cb_offered_incompatible_qos(wr, data);
      break;
    case DDS_PUBLICATION_MATCHED_STATUS_ID:
      status_cb_publication_matched(wr, data);
      break;
    case DDS_DATA_AVAILABLE_STATUS_ID:
    case DDS_INCONSISTENT_TOPIC_STATUS_ID:
    case DDS_SAMPLE_LOST_STATUS_ID:
    case DDS_DATA_ON_READERS_STATUS_ID:
    case DDS_SAMPLE_REJECTED_STATUS_ID:
    case DDS_LIVELINESS_CHANGED_STATUS_ID:
    case DDS_SUBSCRIPTION_MATCHED_STATUS_ID:
    case DDS_REQUESTED_DEADLINE_MISSED_STATUS_ID:
    case DDS_REQUESTED_INCOMPATIBLE_QOS_STATUS_ID:
      assert(0);
  }

  wr->m_entity.m_cb_count--;
  wr->m_entity.m_cb_pending_count--;
  ddsrt_cond_broadcast(&wr->m_entity.m_observers_cond);
  ddsrt_mutex_unlock(&wr->m_entity.m_observers_lock);
}

/**
 * @brief 中断DDS写入器
 *
 * @param[in] e 指向dds_entity实体的指针
 */
static void dds_writer_interrupt(dds_entity * e) ddsrt_nonnull_all;

static void dds_writer_interrupt(dds_entity * e)
{
  // 获取实体所属域的domaingv结构体指针
  struct ddsi_domaingv * const gv = &e->m_domain->gv;
  // 唤醒DDSI线程状态
  ddsi_thread_state_awake(ddsi_lookup_thread_state(), gv);
  // 解除对节流写入器的阻塞
  ddsi_unblock_throttled_writer(gv, &e->m_guid);
  // 使DDSI线程状态进入休眠
  ddsi_thread_state_asleep(ddsi_lookup_thread_state());
}
/**
 * @brief 关闭DDS写入器实例
 *
 * @param[in] e 指向dds_entity的指针
 */
static void dds_writer_close(dds_entity * e) ddsrt_nonnull_all;

static void dds_writer_close(dds_entity * e)
{
  // 将 e 转换为 dds_writer 结构体指针
  struct dds_writer * const wr = (struct dds_writer *)e;
  // 获取域全局变量指针
  struct ddsi_domaingv * const gv = &e->m_domain->gv;
  // 查找当前线程状态
  struct ddsi_thread_state * const thrst = ddsi_lookup_thread_state();
  // 唤醒线程状态
  ddsi_thread_state_awake(thrst, gv);
  // 发送数据包
  ddsi_xpack_send(wr->m_xp, false);
  // 删除写入器
  (void)ddsi_delete_writer(gv, &e->m_guid);
  // 线程进入休眠状态
  ddsi_thread_state_asleep(thrst);

  // 锁定互斥锁
  ddsrt_mutex_lock(&e->m_mutex);
  // 等待条件变量，直到写入器为空
  while (wr->m_wr != NULL) ddsrt_cond_wait(&e->m_cond, &e->m_mutex);
  // 解锁互斥锁
  ddsrt_mutex_unlock(&e->m_mutex);
}

/**
 * @brief 删除DDS写入器实例
 *
 * @param[in] e 指向dds_entity的指针
 * @return 成功返回DDS_RETCODE_OK，否则返回错误代码
 */
static dds_return_t dds_writer_delete(dds_entity * e) ddsrt_nonnull_all;

static dds_return_t dds_writer_delete(dds_entity * e)
{
  // 将 e 转换为 dds_writer 结构体指针
  dds_writer * const wr = (dds_writer *)e;
#ifdef DDS_HAS_SHM
  if (wr->m_iox_pub) {
    // 释放 iceoryx 的发布者
    DDS_CLOG(DDS_LC_SHM, &e->m_domain->gv.logconfig, "Release iceoryx's publisher\n");
    iox_pub_stop_offer(wr->m_iox_pub);
    iox_pub_deinit(wr->m_iox_pub);
  }
#endif
  /* FIXME: not freeing WHC here because it is owned by the DDSI entity */
  // 唤醒线程状态
  ddsi_thread_state_awake(ddsi_lookup_thread_state(), &e->m_domain->gv);
  // 释放数据包
  ddsi_xpack_free(wr->m_xp);
  // 线程进入休眠状态
  ddsi_thread_state_asleep(ddsi_lookup_thread_state());
  // 减少主题引用计数
  dds_entity_drop_ref(&wr->m_topic->m_entity);
  return DDS_RETCODE_OK;
}

/**
 * @brief 验证写入器的QoS设置
 *
 * @param[in] wqos 指向dds_qos_t的指针
 * @return 成功返回DDS_RETCODE_OK，否则返回错误代码
 */
static dds_return_t validate_writer_qos(const dds_qos_t * wqos)
{
#ifndef DDS_HAS_LIFESPAN
  if (wqos != NULL && (wqos->present & DDSI_QP_LIFESPAN) && wqos->lifespan.duration != DDS_INFINITY)
    return DDS_RETCODE_BAD_PARAMETER;
#endif
#ifndef DDS_HAS_DEADLINE_MISSED
  if (wqos != NULL && (wqos->present & DDSI_QP_DEADLINE) && wqos->deadline.deadline != DDS_INFINITY)
    return DDS_RETCODE_BAD_PARAMETER;
#endif
#if defined(DDS_HAS_LIFESPAN) && defined(DDS_HAS_DEADLINE_MISSED)
  DDSRT_UNUSED_ARG(wqos);
#endif
  return DDS_RETCODE_OK;
}

/**
 * @brief 设置写入器的QoS参数
 *
 * @param e      [in] 指向dds_entity结构体的指针
 * @param qos    [in] 指向dds_qos_t结构体的指针，包含QoS设置
 * @param enabled [in] 是否启用QoS设置更新
 * @return dds_return_t 返回操作结果，成功返回DDS_RETCODE_OK
 */
static dds_return_t dds_writer_qos_set(dds_entity * e, const dds_qos_t * qos, bool enabled)
{
  // 注意：e->m_qos仍然是旧的QoS设置，以防止此处失败
  dds_return_t ret;
  // 验证写入器QoS设置是否有效
  if ((ret = validate_writer_qos(qos)) != DDS_RETCODE_OK) return ret;
  // 如果启用了QoS设置更新
  if (enabled) {
    struct ddsi_writer * wr;
    // 唤醒线程状态
    ddsi_thread_state_awake(ddsi_lookup_thread_state(), &e->m_domain->gv);
    // 查找写入器实体
    if ((wr = ddsi_entidx_lookup_writer_guid(e->m_domain->gv.entity_index, &e->m_guid)) != NULL)
      // 更新写入器的QoS设置
      ddsi_update_writer_qos(wr, qos);
    // 使线程进入休眠状态
    ddsi_thread_state_asleep(ddsi_lookup_thread_state());
  }
  return DDS_RETCODE_OK;
}

// 写入器统计信息键值描述符数组
static const struct dds_stat_keyvalue_descriptor dds_writer_statistics_kv[] = {
  {"rexmit_bytes", DDS_STAT_KIND_UINT64},    // 重传字节数
  {"throttle_count", DDS_STAT_KIND_UINT32},  // 节流计数
  {"time_throttle", DDS_STAT_KIND_UINT64},   // 节流时间
  {"time_rexmit", DDS_STAT_KIND_UINT64}};    // 重传时间

// 写入器统计信息描述符结构体
static const struct dds_stat_descriptor dds_writer_statistics_desc = {
  .count = sizeof(dds_writer_statistics_kv) / sizeof(dds_writer_statistics_kv[0]),
  .kv = dds_writer_statistics_kv};

/**
 * @brief 创建写入器的统计信息结构体
 *
 * @param entity [in] 指向dds_entity结构体的指针
 * @return struct dds_statistics* 返回创建的dds_statistics结构体指针
 */
static struct dds_statistics * dds_writer_create_statistics(const struct dds_entity * entity)
{
  return dds_alloc_statistics(entity, &dds_writer_statistics_desc);
}
/**
 * @brief 刷新写入器的统计信息
 *
 * @param entity [in] 指向dds_entity结构体的指针
 * @param stat   [in] 指向dds_statistics结构体的指针，用于存储刷新后的统计信息
 */
static void dds_writer_refresh_statistics(
  const struct dds_entity * entity, struct dds_statistics * stat)
{
  // 将实体转换为dds_writer类型
  const struct dds_writer * wr = (const struct dds_writer *)entity;
  // 如果写入器存在，则获取写入器的统计信息
  if (wr->m_wr)
    ddsi_get_writer_stats(
      wr->m_wr, &stat->kv[0].u.u64, &stat->kv[1].u.u32, &stat->kv[2].u.u64, &stat->kv[3].u.u64);
}

// 写入器实体派生操作结构体
const struct dds_entity_deriver dds_entity_deriver_writer = {
  .interrupt = dds_writer_interrupt,                   // 中断写入器操作
  .close = dds_writer_close,                           // 关闭写入器
  .delete = dds_writer_delete,                         // 删除写入器
  .set_qos = dds_writer_qos_set,                       // 设置写入器QoS参数
  .validate_status = dds_writer_status_validate,       // 验证写入器状态
  .create_statistics = dds_writer_create_statistics,   // 创建写入器统计信息
  .refresh_statistics = dds_writer_refresh_statistics  // 刷新写入器统计信息
};
#ifdef DDS_HAS_SHM
/**
 * @brief 创建 iox_pub_options_t 结构体实例
 *
 * 根据给定的 dds_qos_t 参数创建一个 iox_pub_options_t 结构体实例。
 *
 * @param qos 指向 dds_qos_t 结构体的指针，包含了 QoS（Quality of Service）相关信息
 * @return 返回一个初始化完成的 iox_pub_options_t 结构体实例
 */
static iox_pub_options_t create_iox_pub_options(const dds_qos_t * qos)
{
  // 定义一个 iox_pub_options_t 结构体变量
  iox_pub_options_t opts;

  // 初始化 iox_pub_options_t 结构体变量
  iox_pub_options_init(&opts);

  // 判断 qos 中的 durability 属性是否为 DDS_DURABILITY_VOLATILE
  if (qos->durability.kind == DDS_DURABILITY_VOLATILE) {
    // 如果是 DDS_DURABILITY_VOLATILE，则将 historyCapacity 设置为 0
    opts.historyCapacity = 0;
  } else {
    // 如果不是 DDS_DURABILITY_VOLATILE（即 Transient Local 或更强）
    // 判断 qos 中的 durability_service 的 history 属性是否为 DDS_HISTORY_KEEP_LAST
    if (qos->durability_service.history.kind == DDS_HISTORY_KEEP_LAST) {
      // 如果是 DDS_HISTORY_KEEP_LAST，则将 historyCapacity 设置为 qos 中的 durability_service 的 history 的 depth 值
      opts.historyCapacity = (uint64_t)qos->durability_service.history.depth;
    } else {
      // 如果不是 DDS_HISTORY_KEEP_LAST，则将 historyCapacity 设置为 0
      opts.historyCapacity = 0;
    }
  }

  // 返回初始化完成的 iox_pub_options_t 结构体实例
  return opts;
}
#endif
/**
 * @brief 创建一个DDS写入器实例。
 *
 * @param participant_or_publisher 参与者或发布者实体，用于创建写入器。
 * @param topic 话题实体，写入器将在此话题上发布数据。
 * @param qos 写入器的质量服务设置。如果为NULL，则使用默认值。
 * @param listener 写入器的监听器。如果为NULL，则不使用监听器。
 * @return 成功时返回新创建的写入器实体，失败时返回错误代码。
 */
dds_entity_t dds_create_writer(
  dds_entity_t participant_or_publisher, dds_entity_t topic, const dds_qos_t * qos,
  const dds_listener_t * listener)
{
  dds_return_t rc;
  dds_qos_t * wqos;
  dds_publisher * pub = NULL;
  dds_topic * tp;
  dds_entity_t publisher;
  struct whc_writer_info * wrinfo;
  bool created_implicit_pub = false;

  // 锁定参与者或发布者实体
  {
    dds_entity * p_or_p;
    if (
      (rc = dds_entity_lock(participant_or_publisher, DDS_KIND_DONTCARE, &p_or_p)) !=
      DDS_RETCODE_OK)
      return rc;
    switch (dds_entity_kind(p_or_p)) {
      case DDS_KIND_PUBLISHER:
        publisher = participant_or_publisher;
        pub = (dds_publisher *)p_or_p;
        break;
      case DDS_KIND_PARTICIPANT:
        publisher = dds__create_publisher_l((dds_participant *)p_or_p, true, qos, NULL);
        dds_entity_unlock(p_or_p);
        if ((rc = dds_publisher_lock(publisher, &pub)) < 0) return rc;
        created_implicit_pub = true;
        break;
      default:
        dds_entity_unlock(p_or_p);
        return DDS_RETCODE_ILLEGAL_OPERATION;
    }
  }

  // 锁定话题实体
  if ((rc = dds_topic_pin(topic, &tp)) != DDS_RETCODE_OK) goto err_pin_topic;
  assert(tp->m_stype);
  if (dds_entity_participant(&pub->m_entity) != dds_entity_participant(&tp->m_entity)) {
    rc = DDS_RETCODE_BAD_PARAMETER;
    goto err_pp_mismatch;
  }

  // 延迟设置话题的QoS，直到写入器创建并注册完毕
  dds_topic_defer_set_qos(tp);

  // 合并话题和发布者的QoS
  struct ddsi_domaingv * gv = &pub->m_entity.m_domain->gv;
  wqos = dds_create_qos();
  if (qos) ddsi_xqos_mergein_missing(wqos, qos, DDS_WRITER_QOS_MASK);
  if (pub->m_entity.m_qos)
    ddsi_xqos_mergein_missing(wqos, pub->m_entity.m_qos, ~DDSI_QP_ENTITY_NAME);
  if (tp->m_ktopic->qos)
    ddsi_xqos_mergein_missing(
      wqos, tp->m_ktopic->qos, (DDS_WRITER_QOS_MASK | DDSI_QP_TOPIC_DATA) & ~DDSI_QP_ENTITY_NAME);
  ddsi_xqos_mergein_missing(wqos, &ddsi_default_qos_writer, ~DDSI_QP_DATA_REPRESENTATION);
  dds_apply_entity_naming(wqos, pub->m_entity.m_qos, gv);

  // 确保数据表示有效
  if (
    (rc = dds_ensure_valid_data_representation(
       wqos, tp->m_stype->allowed_data_representation, false)) != 0)
    goto err_data_repr;

  // 验证QoS设置是否有效
  if (
    (rc = ddsi_xqos_valid(&gv->logconfig, wqos)) < 0 ||
    (rc = validate_writer_qos(wqos)) != DDS_RETCODE_OK)
    goto err_bad_qos;

  assert(wqos->present & DDSI_QP_DATA_REPRESENTATION && wqos->data_representation.value.n > 0);
  dds_data_representation_id_t data_representation = wqos->data_representation.value.ids[0];

  // 唤醒线程状态
  ddsi_thread_state_awake(ddsi_lookup_thread_state(), gv);
  const struct ddsi_guid * ppguid = dds_entity_participant_guid(&pub->m_entity);
  struct ddsi_participant * pp = ddsi_entidx_lookup_participant_guid(gv->entity_index, ppguid);
  assert(pp != NULL);

#ifdef DDS_HAS_SECURITY
  // 检查DDS安全性是否启用
  if (ddsi_omg_participant_is_secure(pp)) {
    // 请求访问控制安全插件以获取创建写入器权限
    if (!ddsi_omg_security_check_create_writer(pp, gv->config.domainId, tp->m_name, wqos)) {
      rc = DDS_RETCODE_NOT_ALLOWED_BY_SECURITY;
      goto err_not_allowed;
    }
  }
#endif

  // 配置异步模式
  bool async_mode = (wqos->latency_budget.duration > 0);

  // 创建写入器
  struct dds_writer * const wr = dds_alloc(sizeof(*wr));
  const dds_entity_t writer = dds_entity_init(
    &wr->m_entity, &pub->m_entity, DDS_KIND_WRITER, false, true, wqos, listener,
    DDS_WRITER_STATUS_MASK);
  wr->m_topic = tp;
  dds_entity_add_ref_locked(&tp->m_entity);
  wr->m_xp = ddsi_xpack_new(gv, async_mode);
  wrinfo = dds_whc_make_wrinfo(wr, wqos);
  wr->m_whc = dds_whc_new(gv, wrinfo);
  dds_whc_free_wrinfo(wrinfo);
  wr->whc_batch = gv->config.whc_batch;

#ifdef DDS_HAS_SHM
  assert(wqos->present & DDSI_QP_LOCATOR_MASK);
  if (!(gv->config.enable_shm && dds_shm_compatible_qos_and_topic(wqos, tp, true)))
    wqos->ignore_locator_type |= DDSI_LOCATOR_KIND_SHEM;
#endif

  struct ddsi_sertype * sertype = ddsi_sertype_derive_sertype(
    tp->m_stype, data_representation,
    wqos->present & DDSI_QP_TYPE_CONSISTENCY_ENFORCEMENT ? wqos->type_consistency
                                                         : ddsi_default_qos_topic.type_consistency);
  if (!sertype) sertype = tp->m_stype;

  rc = ddsi_new_writer(
    &wr->m_wr, &wr->m_entity.m_guid, NULL, pp, tp->m_name, sertype, wqos, wr->m_whc,
    dds_writer_status_cb, wr);
  assert(rc == DDS_RETCODE_OK);
  ddsi_thread_state_asleep(ddsi_lookup_thread_state());

#ifdef DDS_HAS_SHM
  if (wr->m_wr->has_iceoryx) {
    DDS_CLOG(
      DDS_LC_SHM, &wr->m_entity.m_domain->gv.logconfig,
      "Writer's topic name will be DDS:Cyclone:%s\n", wr->m_topic->m_name);
    iox_pub_options_t opts = create_iox_pub_options(wqos);

    char * part_topic = dds_shm_partition_topic(wqos, wr->m_topic);
    assert(part_topic != NULL);
    wr->m_iox_pub = iox_pub_init(
      &(iox_pub_storage_t){0}, gv->config.iceoryx_service, wr->m_topic->m_stype->type_name,
      part_topic, &opts);
    ddsrt_free(part_topic);
    memset(wr->m_iox_pub_loans, 0, sizeof(wr->m_iox_pub_loans));
  }
#endif

  wr->m_entity.m_iid = ddsi_get_entity_instanceid(&wr->m_entity.m_domain->gv, &wr->m_entity.m_guid);
  dds_entity_register_child(&pub->m_entity, &wr->m_entity);

  dds_entity_init_complete(&wr->m_entity);

  dds_topic_allow_set_qos(tp);
  dds_topic_unpin(tp);
  dds_publisher_unlock(pub);

  // 如果异步模式启用且线程尚未启动，则启动异步线程
  ddsrt_mutex_lock(&gv->sendq_running_lock);
  if (async_mode && !gv->sendq_running) {
    ddsi_xpack_sendq_init(gv);
    ddsi_xpack_sendq_start(gv);
  }
  ddsrt_mutex_unlock(&gv->sendq_running_lock);
  return writer;

#ifdef DDS_HAS_SECURITY
err_not_allowed:
  ddsi_thread_state_asleep(ddsi_lookup_thread_state());
#endif
err_bad_qos:
err_data_repr:
  dds_delete_qos(wqos);
  dds_topic_allow_set_qos(tp);
err_pp_mismatch:
  dds_topic_unpin(tp);
err_pin_topic:
  dds_publisher_unlock(pub);
  if (created_implicit_pub) (void)dds_delete(publisher);
  return rc;
}
/**
 * @brief 获取与给定写入者关联的发布者实体
 *
 * @param writer 写入者实体
 * @return 成功时返回发布者实体，失败时返回错误代码
 */
dds_entity_t dds_get_publisher(dds_entity_t writer)
{
  dds_entity * e;   // 定义一个指向实体的指针
  dds_return_t rc;  // 定义一个返回值变量
  // 尝试获取实体并检查返回值
  if ((rc = dds_entity_pin(writer, &e)) != DDS_RETCODE_OK)
    return rc;
  else {
    dds_entity_t pubh;  // 定义一个发布者实体变量
    // 检查实体类型是否为写入者
    if (dds_entity_kind(e) != DDS_KIND_WRITER)
      pubh = DDS_RETCODE_ILLEGAL_OPERATION;
    else {
      // 断言实体的父实体类型为发布者
      assert(dds_entity_kind(e->m_parent) == DDS_KIND_PUBLISHER);
      // 获取父实体的句柄
      pubh = e->m_parent->m_hdllink.hdl;
    }
    // 解除实体引用
    dds_entity_unpin(e);
    return pubh;
  }
}

/**
 * @brief 初始化数据分配器
 *
 * @param wr 指向dds_writer的指针
 * @param data_allocator 指向dds_data_allocator_t的指针
 * @return 返回操作结果
 */
dds_return_t dds__writer_data_allocator_init(
  const dds_writer * wr, dds_data_allocator_t * data_allocator)
{
#ifdef DDS_HAS_SHM
  dds_iox_allocator_t * d = (dds_iox_allocator_t *)data_allocator->opaque.bytes;
  ddsrt_mutex_init(&d->mutex);
  if (NULL != wr->m_iox_pub) {
    d->kind = DDS_IOX_ALLOCATOR_KIND_PUBLISHER;
    d->ref.pub = wr->m_iox_pub;
  } else {
    d->kind = DDS_IOX_ALLOCATOR_KIND_NONE;
  }
  return DDS_RETCODE_OK;
#else
  (void)wr;
  (void)data_allocator;
  return DDS_RETCODE_OK;
#endif
}

/**
 * @brief 结束数据分配器
 *
 * @param wr 指向dds_writer的指针
 * @param data_allocator 指向dds_data_allocator_t的指针
 * @return 返回操作结果
 */
dds_return_t dds__writer_data_allocator_fini(
  const dds_writer * wr, dds_data_allocator_t * data_allocator)
{
#ifdef DDS_HAS_SHM
  dds_iox_allocator_t * d = (dds_iox_allocator_t *)data_allocator->opaque.bytes;
  ddsrt_mutex_destroy(&d->mutex);
  d->kind = DDS_IOX_ALLOCATOR_KIND_FINI;
#else
  (void)data_allocator;
#endif
  (void)wr;
  return DDS_RETCODE_OK;
}

/**
 * @brief 等待写入者的确认
 *
 * @param wr 指向dds_writer的指针
 * @param rdguid 指向ddsi_guid_t的指针
 * @param abstimeout 绝对超时时间
 * @return 返回操作结果
 */
dds_return_t dds__ddsi_writer_wait_for_acks(
  struct dds_writer * wr, ddsi_guid_t * rdguid, dds_time_t abstimeout)
{
  // 在写入者的生命周期内，m_wr保持不变，只有在删除时才会被清除
  if (wr->m_wr == NULL)
    return DDS_RETCODE_OK;
  else
    return ddsi_writer_wait_for_acks(wr->m_wr, rdguid, abstimeout);
}
