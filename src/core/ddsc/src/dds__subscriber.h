/*
 * Copyright(c) 2006 to 2021 ZettaScale Technology and others
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v. 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0, or the Eclipse Distribution License
 * v. 1.0 which is available at
 * http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * SPDX-License-Identifier: EPL-2.0 OR BSD-3-Clause
 */
#ifndef DDS__SUBSCRIBER_H
#define DDS__SUBSCRIBER_H

#include "dds/dds.h"
#include "dds__entity.h"

#if defined (__cplusplus)
extern "C" {
#endif

DEFINE_ENTITY_LOCK_UNLOCK(dds_subscriber, DDS_KIND_SUBSCRIBER, subscriber)

/**
 * @brief 创建一个持有参与者实体锁的订阅者 (Creates a subscriber with participant entity-lock held)
 * @component 订阅者组件 (subscriber component)
 *
 * @param participant 父参与者 (the parent participant)
 * @param implicit 表示是否隐式创建 (indicates if implicitly created)
 * @param qos 存储在订阅者中的qos对象 (qos object that is stored in the subscriber)
 * @param listener 存储在订阅者中的监听器对象 (listener object that is stored in the subscriber)
 * @return dds_entity_t 类型的实体 (dds_entity_t type entity)
 */
dds_entity_t dds__create_subscriber_l(struct dds_participant *participant, bool implicit, const dds_qos_t *qos, const dds_listener_t *listener);

/**
 * @brief 开始一致性操作 (Begin coherent operation)
 * @component 订阅者组件 (subscriber component)
 *
 * @param e 实体类型参数 (Entity type parameter)
 * @return dds_return_t 类型的返回值 (dds_return_t type return value)
 */
dds_return_t dds_subscriber_begin_coherent (dds_entity_t e);

/**
 * @brief 结束一致性操作 (End coherent operation)
 * @component 订阅者组件 (subscriber component)
 *
 * @param e 实体类型参数 (Entity type parameter)
 * @return dds_return_t 类型的返回值 (dds_return_t type return value)
 */
dds_return_t dds_subscriber_end_coherent (dds_entity_t e);

/**
 * @brief 计算锁定的读取器上的数据 (Compute data on locked readers)
 * @component 订阅者组件 (subscriber component)
 *
 * @param sub 订阅者指针 (Pointer to subscriber)
 * @return 布尔值，表示是否计算成功 (Boolean value indicating whether the calculation was successful)
 */
bool dds_subscriber_compute_data_on_readers_locked (dds_subscriber *sub);

/**
 * @brief 调整读取器上的数据实例化 (Adjust materialize data on readers)
 * @component 订阅者组件 (subscriber component)
 *
 * @param sub 订阅者指针 (Pointer to subscriber)
 * @param materialization_needed 是否需要实例化 (Whether materialization is needed)
 */
void dds_subscriber_adjust_materialize_data_on_readers (dds_subscriber *sub, bool materialization_needed) ddsrt_nonnull_all;

#if defined (__cplusplus)
}
#endif

#endif /* DDS__SUBSCRIBER_H */
