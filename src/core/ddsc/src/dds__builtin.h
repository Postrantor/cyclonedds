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
#ifndef DDS__BUILTIN_H
#define DDS__BUILTIN_H

#include "dds/ddsi/ddsi_builtin_topic_if.h"
#include "dds__types.h"

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @brief 获取内置主题伪句柄的名称和类型名 (Get name and typename based from a builtin-topic
 * pseudo-handle)
 * @component 内置主题 (builtin_topic)
 *
 * @param pseudo_handle 内置主题伪句柄 (built-in topic pseudo handle)
 * @param name          获取内置主题的名称 (gets the name of the built-in topic)
 * @param typename      获取内置主题的类型名 (gets the type name of the built-in topic)
 * @return 如果伪句柄无效，则返回 DDS_RETCODE_BAD_PARAMETER (DDS_RETCODE_BAD_PARAMETER if
 * pseudo_handle is invalid)
 */
dds_return_t dds__get_builtin_topic_name_typename(dds_entity_t pseudo_handle,
                                                  const char** name,
                                                  const char** typename);

/**
 * @brief 根据给定的类型名返回伪句柄 (Returns the pseudo handle for the given typename)
 * @component 内置主题 (builtin_topic)
 *
 * @return 如果类型名不是内置主题之一，则返回 DDS_RETCODE_BAD_PARAMETER (DDS_RETCODE_BAD_PARAMETER
 * if typename isn't one of the built-in topics)
 */
dds_entity_t dds__get_builtin_topic_pseudo_handle_from_typename(const char* typename);

/**
 * @brief 获取与主题 'id' 相关的相关参与者中的实际主题 (Get actual topic in related participant
 * related to topic 'id'.)
 * @component 内置主题 (builtin_topic)
 *
 * @param e         从中获取内置主题的实体 (entity to get the built-in topic from)
 * @param topic     用于获取实际内置主题的伪句柄 (pseudo handle to get the actual built-in topic
 * for)
 * @returns 内置主题实体 (the built-in topic entity)
 */
dds_entity_t dds__get_builtin_topic(dds_entity_t e, dds_entity_t topic);

/**
 * @brief 为内置主题 QoS 构建 QoS 对象 (Constructs the QoS object for a built-in topic QoS)
 * @component 内置主题 (builtin_topic)
 *
 * @returns 内置主题的 qos (the qos for the built-in topic)
 */
dds_qos_t* dds__create_builtin_qos(void);

/**
 * @brief 相关参与者中的订阅者单例 (Subscriber singleton within related participant.)
 * @component 内置主题 (builtin_topic)
 *
 * @param e 从中获取参与者的实体，从该参与者中检索内置订阅者 (entity to get the participant from,
 * from which the built-in subscriber will be retrieved)
 * @returns 订阅者实体 (the subscriber entity)
 */
dds_entity_t dds__get_builtin_subscriber(dds_entity_t e);

/**
 * @brief 检查读取器 QoS 是否适用于内置主题 TOPIC (Checks whether the reader QoS is valid for use
 * with built-in topic TOPIC)
 * @component 内置主题 (builtin_topic)
 *
 * @param dom 域 (the domain)
 * @param topic 内置主题的伪句柄 (pseudo handle of the built-in topic)
 * @param qos 要检查有效性的 qos (qos to check validity for)
 * @returns 如果 qos 有效，则返回 true (true iff the qos is valid)
 */
bool dds__validate_builtin_reader_qos(const dds_domain* dom,
                                      dds_entity_t topic,
                                      const dds_qos_t* qos);

/** @component 内置主题 (builtin_topic) */
void dds__builtin_init(struct dds_domain* dom);

/** @component 内置主题 (builtin_topic) */
void dds__builtin_fini(struct dds_domain* dom);

struct ddsi_entity_common;
struct ddsi_proxy_topic;

/** @component 内置主题 (builtin_topic) */
struct ddsi_serdata* dds__builtin_make_sample_endpoint(const struct ddsi_entity_common* e,
                                                       ddsrt_wctime_t timestamp,
                                                       bool alive);

/** @component 内置主题 (builtin_topic) */
struct ddsi_serdata* dds__builtin_make_sample_topic(const struct ddsi_entity_common* e,
                                                    ddsrt_wctime_t timestamp,
                                                    bool alive);

/** @component 内置主题 (builtin_topic) */
struct ddsi_serdata* dds__builtin_make_sample_proxy_topic(const struct ddsi_proxy_topic* proxytp,
                                                          ddsrt_wctime_t timestamp,
                                                          bool alive);

#if defined(__cplusplus)
}
#endif

#endif /* DDS__BUILTIN_H */
