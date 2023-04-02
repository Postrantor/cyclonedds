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
#ifndef DDS__TOPIC_H
#define DDS__TOPIC_H

#include "dds__entity.h"
#include "dds__types.h"

#if defined(__cplusplus)
extern "C" {
#endif

// 定义实体锁定和解锁的宏（用于DDS主题）
DEFINE_ENTITY_LOCK_UNLOCK(dds_topic, DDS_KIND_TOPIC, topic)

/**
 * @brief 将主题与原始句柄关联
 * @component topic
 * @param handle 实体句柄
 * @param from_user 是否来自用户
 * @param tp 返回的dds_topic指针
 * @return dds_return_t 操作结果
 */
dds_return_t dds_topic_pin_with_origin(dds_entity_t handle,
                                       bool from_user,
                                       struct dds_topic** tp) ddsrt_nonnull_all;

/**
 * @brief 将主题与句柄关联
 * @component topic
 * @param handle 实体句柄
 * @param tp 返回的dds_topic指针
 * @return dds_return_t 操作结果
 */
dds_return_t dds_topic_pin(dds_entity_t handle, struct dds_topic** tp) ddsrt_nonnull_all;

/**
 * @brief 解除主题与句柄的关联
 * @component topic
 * @param tp 要解除关联的dds_topic指针
 */
void dds_topic_unpin(struct dds_topic* tp) ddsrt_nonnull_all;

/**
 * @brief 延迟设置主题的QoS
 * @component topic
 * @param tp 要设置QoS的dds_topic指针
 */
void dds_topic_defer_set_qos(struct dds_topic* tp) ddsrt_nonnull_all;

/**
 * @brief 允许设置主题的QoS
 * @component topic
 * @param tp 要设置QoS的dds_topic指针
 */
void dds_topic_allow_set_qos(struct dds_topic* tp) ddsrt_nonnull_all;

/**
 * @brief 创建主题实现
 * @component topic
 * @param participant 参与者实体
 * @param name 主题名称
 * @param allow_dcps 是否允许DCPS
 * @param sertype 序列化类型指针
 * @param qos QoS设置
 * @param listener 监听器设置
 * @param is_builtin 是否为内置主题
 * @return dds_entity_t 创建的主题实体
 */
dds_entity_t dds_create_topic_impl(dds_entity_t participant,
                                   const char* name,
                                   bool allow_dcps,
                                   struct ddsi_sertype** sertype,
                                   const dds_qos_t* qos,
                                   const dds_listener_t* listener,
                                   bool is_builtin);

#if defined(__cplusplus)
}
#endif

#endif /* DDS__TOPIC_H */
