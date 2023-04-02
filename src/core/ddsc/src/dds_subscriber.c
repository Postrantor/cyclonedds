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
#include <string.h>

#include "dds/ddsi/ddsi_domaingv.h"
#include "dds/ddsi/ddsi_entity.h"
#include "dds/ddsi/ddsi_iid.h"
#include "dds/ddsrt/heap.h"
#include "dds/version.h"
#include "dds__listener.h"
#include "dds__participant.h"
#include "dds__qos.h"
#include "dds__subscriber.h"

DECL_ENTITY_LOCK_UNLOCK(dds_subscriber)

#define DDS_SUBSCRIBER_STATUS_MASK (DDS_DATA_ON_READERS_STATUS)

/**
 * @brief 设置订阅者的QoS参数
 *
 * @param[in] e 实体指针
 * @param[in] qos QoS参数指针
 * @param[in] enabled 是否启用
 * @return 返回DDS_RETCODE_OK
 */
static dds_return_t dds_subscriber_qos_set(dds_entity* e, const dds_qos_t* qos, bool enabled) {
  /* 注意: e->m_qos仍然是旧的，以允许在此处失败 */
  (void)e;
  (void)qos;
  (void)enabled;
  return DDS_RETCODE_OK;
}

/**
 * @brief 验证订阅者状态
 *
 * @param[in] mask 状态掩码
 * @return 返回DDS_RETCODE_BAD_PARAMETER或DDS_RETCODE_OK
 */
static dds_return_t dds_subscriber_status_validate(uint32_t mask) {
  return (mask & ~DDS_SUBSCRIBER_STATUS_MASK) ? DDS_RETCODE_BAD_PARAMETER : DDS_RETCODE_OK;
}

/* 订阅者实体派生结构 */
const struct dds_entity_deriver dds_entity_deriver_subscriber = {
    .interrupt = dds_entity_deriver_dummy_interrupt,
    .close = dds_entity_deriver_dummy_close,
    .delete = dds_entity_deriver_dummy_delete,
    .set_qos = dds_subscriber_qos_set,
    .validate_status = dds_subscriber_status_validate,
    .create_statistics = dds_entity_deriver_dummy_create_statistics,
    .refresh_statistics = dds_entity_deriver_dummy_refresh_statistics};

/**
 * @brief 创建订阅者实体
 *
 * @param[in] participant 参与者指针
 * @param[in] implicit 是否隐式创建
 * @param[in] qos QoS参数指针
 * @param[in] listener 监听器指针
 * @return 返回订阅者实体
 */
dds_entity_t dds__create_subscriber_l(dds_participant* participant,
                                      bool implicit,
                                      const dds_qos_t* qos,
                                      const dds_listener_t* listener) {
  /* 必须持有参与者实体锁 */
  dds_subscriber* sub;
  dds_entity_t subscriber;
  dds_return_t ret;
  dds_qos_t* new_qos;

  new_qos = dds_create_qos();
  if (qos) ddsi_xqos_mergein_missing(new_qos, qos, DDS_SUBSCRIBER_QOS_MASK);
  ddsi_xqos_mergein_missing(new_qos, &ddsi_default_qos_publisher_subscriber, ~(uint64_t)0);
  dds_apply_entity_naming(new_qos, participant->m_entity.m_qos,
                          &participant->m_entity.m_domain->gv);

  if ((ret = ddsi_xqos_valid(&participant->m_entity.m_domain->gv.logconfig, new_qos)) !=
      DDS_RETCODE_OK) {
    dds_delete_qos(new_qos);
    return ret;
  }

  sub = dds_alloc(sizeof(*sub));
  subscriber = dds_entity_init(&sub->m_entity, &participant->m_entity, DDS_KIND_SUBSCRIBER,
                               implicit, true, new_qos, listener, DDS_SUBSCRIBER_STATUS_MASK);
  sub->m_entity.m_iid = ddsi_iid_gen();
  sub->materialize_data_on_readers = 0;
  dds_entity_register_child(&participant->m_entity, &sub->m_entity);
  dds_entity_init_complete(&sub->m_entity);
  return subscriber;
}

/**
 * @brief 创建一个订阅者实体
 *
 * @param participant 参与者实体
 * @param qos 服务质量配置
 * @param listener 监听器
 * @return 返回创建的订阅者实体
 */
dds_entity_t dds_create_subscriber(dds_entity_t participant,
                                   const dds_qos_t* qos,
                                   const dds_listener_t* listener) {
  // 定义参与者指针
  dds_participant* par;
  // 定义实体句柄
  dds_entity_t hdl;
  // 定义返回值
  dds_return_t ret;

  // 尝试锁定参与者
  if ((ret = dds_participant_lock(participant, &par)) != DDS_RETCODE_OK) return ret;

  // 创建订阅者实体
  hdl = dds__create_subscriber_l(par, false, qos, listener);
  // 解锁参与者
  dds_participant_unlock(par);

  return hdl;
}

/**
 * @brief 通知订阅者的读取器
 *
 * @param subscriber 订阅者实体
 * @return 返回操作结果
 */
dds_return_t dds_notify_readers(dds_entity_t subscriber) {
  // 定义订阅者指针
  dds_subscriber* sub;
  // 定义返回值
  dds_return_t ret;

  // 尝试锁定订阅者
  if ((ret = dds_subscriber_lock(subscriber, &sub)) != DDS_RETCODE_OK) return ret;

  // 解锁订阅者
  dds_subscriber_unlock(sub);

  return DDS_RETCODE_UNSUPPORTED;
}

/**
 * @brief 开始一致性订阅操作
 *
 * @param e 订阅者实体
 * @return 返回操作结果
 */
dds_return_t dds_subscriber_begin_coherent(dds_entity_t e) {
  return dds_generic_unimplemented_operation(e, DDS_KIND_SUBSCRIBER);
}

/**
 * @brief 结束一致性订阅操作
 *
 * @param e 订阅者实体
 * @return 返回操作结果
 */
dds_return_t dds_subscriber_end_coherent(dds_entity_t e) {
  return dds_generic_unimplemented_operation(e, DDS_KIND_SUBSCRIBER);
}

/**
 * @brief 计算锁定的订阅者上的读取器数据
 *
 * @param sub 订阅者指针
 * @return 返回是否有可用数据
 */
bool dds_subscriber_compute_data_on_readers_locked(dds_subscriber* sub) {
  // sub->m_entity.m_mutex 必须被锁定
  ddsrt_avl_iter_t it;

  // 当某个读取器设置了 DATA_AVAILABLE 时返回 true 并不是正确的行为
  // 因为它在订阅者的读取器上首次 read/take 操作时并没有重置 DATA_ON_READERS 状态。
  // 在实践中，这似乎不太可能成为问题：
  //
  // - 如果使用监听器，为什么要查看状态？
  // - 如果使用 waitset，因为它是物化的，所以它是精确的
  //
  // 所以这就剩下了轮询 DATA_ON_READERS。此外，它在 Cyclone 中没有添加任何功能，
  // 因为目前还没有实现组排序/一致性和 get_datareaders()。
  //
  // 解决这个问题的一种可能方法是在订阅者中添加一个“虚拟时钟”（只是一个整数，
  // 在所有相关操作上原子更新），并在每个读取器中记录最后一次更新 DATA_AVAILABLE 的虚拟“时间戳”。

  // 遍历订阅者的子实体
  for (dds_entity* rd =
           ddsrt_avl_iter_first(&dds_entity_children_td, &sub->m_entity.m_children, &it);
       rd; rd = ddsrt_avl_iter_next(&it)) {
    // 获取状态和掩码
    const uint32_t sm = ddsrt_atomic_ld32(&rd->m_status.m_status_and_mask);
    // 如果设置了 DATA_AVAILABLE 状态，则返回 true
    if (sm & DDS_DATA_AVAILABLE_STATUS) return true;
  }

  return false;
}

/**
 * @brief 调整订阅者的数据实例化状态，并在需要时将其传播到读取器。
 *
 * @param sub 指向dds_subscriber结构的指针。
 * @param materialization_needed 布尔值，表示是否需要数据实例化。
 */
void dds_subscriber_adjust_materialize_data_on_readers(dds_subscriber* sub,
                                                       bool materialization_needed) {
  // 没有锁定，sub已固定
  bool propagate = false;
  ddsrt_mutex_lock(&sub->m_entity.m_observers_lock);  // 锁定观察者锁

  if (materialization_needed)                         // 如果需要数据实例化
  {
    // FIXME: 确实无需传播，如果标志已设置？
    if (sub->materialize_data_on_readers++ == 0) propagate = true;  // 需要传播
  } else {
    assert((sub->materialize_data_on_readers & DDS_SUB_MATERIALIZE_DATA_ON_READERS_MASK) > 0);
    if (--sub->materialize_data_on_readers == 0) {
      sub->materialize_data_on_readers &= ~DDS_SUB_MATERIALIZE_DATA_ON_READERS_FLAG;
      propagate = true;  // 需要传播
    }
  }
  ddsrt_mutex_unlock(&sub->m_entity.m_observers_lock);  // 解锁观察者锁

  // 锁定实体互斥锁，用于遍历读取器，顺序为m_mutex，然后是m_observers_lock
  ddsrt_mutex_lock(&sub->m_entity.m_mutex);

  if (propagate)  // 如果需要传播
  {
    // 将数据实例化状态传播到读取器，并在有任何具有DATA_AVAILABLE设置的读取器时设置DATA_ON_READERS
    // 无需触发等待集，因为这在附加之前完成
    dds_instance_handle_t last_iid = 0;
    dds_entity* rd;
    while ((rd = ddsrt_avl_lookup_succ(&dds_entity_children_td, &sub->m_entity.m_children,
                                       &last_iid)) != NULL) {
      last_iid = rd->m_iid;
      dds_entity* x;
      if (dds_entity_pin(rd->m_hdllink.hdl, &x) < 0) continue;
      if (x == rd)                                          // FIXME: 这是否可能不是真的？
      {
        ddsrt_mutex_unlock(&sub->m_entity.m_mutex);         // 解锁实体互斥锁

        ddsrt_mutex_lock(&x->m_observers_lock);             // 锁定读取器的观察者锁
        ddsrt_mutex_lock(&sub->m_entity.m_observers_lock);  // 再次锁定订阅者的观察者锁
        if (sub->materialize_data_on_readers)
          ddsrt_atomic_or32(&x->m_status.m_status_and_mask,
                            DDS_DATA_ON_READERS_STATUS << SAM_ENABLED_SHIFT);
        else
          ddsrt_atomic_and32(&x->m_status.m_status_and_mask,
                             ~(uint32_t)(DDS_DATA_ON_READERS_STATUS << SAM_ENABLED_SHIFT));
        ddsrt_mutex_unlock(&sub->m_entity.m_observers_lock);  // 解锁订阅者的观察者锁
        ddsrt_mutex_unlock(&x->m_observers_lock);             // 解锁读取器的观察者锁

        ddsrt_mutex_lock(&sub->m_entity.m_mutex);             // 再次锁定实体互斥锁
      }
      dds_entity_unpin(x);
    }
  }

  // 设置/清除DATA_ON_READERS - 触发等待集没有意义，因为在将其附加到等待集之前就已经实例化了
  ddsrt_mutex_lock(&sub->m_entity.m_observers_lock);
  if (dds_subscriber_compute_data_on_readers_locked(sub))
    ddsrt_atomic_or32(&sub->m_entity.m_status.m_status_and_mask, DDS_DATA_ON_READERS_STATUS);
  else
    dds_entity_status_reset(&sub->m_entity, DDS_DATA_ON_READERS_STATUS);
  if ((sub->materialize_data_on_readers & DDS_SUB_MATERIALIZE_DATA_ON_READERS_MASK) != 0)
    sub->materialize_data_on_readers |= DDS_SUB_MATERIALIZE_DATA_ON_READERS_FLAG;
  ddsrt_mutex_unlock(&sub->m_entity.m_observers_lock);  // 解锁观察者锁
  ddsrt_mutex_unlock(&sub->m_entity.m_mutex);           // 解锁实体互斥锁
}
