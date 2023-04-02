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
#ifndef DDS__QOS_H
#define DDS__QOS_H

#include "dds/ddsi/ddsi_xqos.h"

#if defined(__cplusplus)
extern "C" {
#endif

/*
这段代码定义了一系列的QoS属性掩码，用于表示DDS实体（如主题、参与者、发布者、读取器、订阅者和写入器）的各种QoS属性。每个掩码都是由多个QoS属性组合而成的，用于表示该实体所支持的所有QoS属性。
*/

// 定义DDS_TOPIC_QOS_MASK，用于表示DDS主题的QoS属性掩码
#define DDS_TOPIC_QOS_MASK                                                                       \
  (DDSI_QP_TOPIC_DATA | DDSI_QP_DURABILITY | DDSI_QP_DURABILITY_SERVICE | DDSI_QP_DEADLINE |     \
   DDSI_QP_LATENCY_BUDGET | DDSI_QP_OWNERSHIP | DDSI_QP_LIVELINESS | DDSI_QP_RELIABILITY |       \
   DDSI_QP_TRANSPORT_PRIORITY | DDSI_QP_LIFESPAN | DDSI_QP_DESTINATION_ORDER | DDSI_QP_HISTORY | \
   DDSI_QP_RESOURCE_LIMITS | DDSI_QP_DATA_REPRESENTATION | DDSI_QP_ENTITY_NAME)

// 定义DDS_PARTICIPANT_QOS_MASK，用于表示DDS参与者的QoS属性掩码
#define DDS_PARTICIPANT_QOS_MASK                                                     \
  (DDSI_QP_USER_DATA | DDSI_QP_ADLINK_ENTITY_FACTORY | DDSI_QP_CYCLONE_IGNORELOCAL | \
   DDSI_QP_PROPERTY_LIST | DDSI_QP_LIVELINESS |                                      \
   DDSI_QP_ENTITY_NAME)  // liveliness is a Cyclone DDS special

// 定义DDS_PUBLISHER_QOS_MASK，用于表示DDS发布者的QoS属性掩码
#define DDS_PUBLISHER_QOS_MASK                                                                     \
  (DDSI_QP_PARTITION | DDSI_QP_PRESENTATION | DDSI_QP_GROUP_DATA | DDSI_QP_ADLINK_ENTITY_FACTORY | \
   DDSI_QP_CYCLONE_IGNORELOCAL | DDSI_QP_ENTITY_NAME)

// 定义DDS_READER_QOS_MASK，用于表示DDS读取器的QoS属性掩码
#define DDS_READER_QOS_MASK                                                                     \
  (DDSI_QP_USER_DATA | DDSI_QP_DURABILITY | DDSI_QP_DEADLINE | DDSI_QP_LATENCY_BUDGET |         \
   DDSI_QP_OWNERSHIP | DDSI_QP_LIVELINESS | DDSI_QP_TIME_BASED_FILTER | DDSI_QP_RELIABILITY |   \
   DDSI_QP_DESTINATION_ORDER | DDSI_QP_HISTORY | DDSI_QP_RESOURCE_LIMITS |                      \
   DDSI_QP_ADLINK_READER_DATA_LIFECYCLE | DDSI_QP_CYCLONE_IGNORELOCAL | DDSI_QP_PROPERTY_LIST | \
   DDSI_QP_TYPE_CONSISTENCY_ENFORCEMENT | DDSI_QP_DATA_REPRESENTATION | DDSI_QP_ENTITY_NAME)

// 定义DDS_SUBSCRIBER_QOS_MASK，用于表示DDS订阅者的QoS属性掩码
#define DDS_SUBSCRIBER_QOS_MASK                                                                    \
  (DDSI_QP_PARTITION | DDSI_QP_PRESENTATION | DDSI_QP_GROUP_DATA | DDSI_QP_ADLINK_ENTITY_FACTORY | \
   DDSI_QP_CYCLONE_IGNORELOCAL | DDSI_QP_ENTITY_NAME)

// 定义DDS_WRITER_QOS_MASK，用于表示DDS写入器的QoS属性掩码
#define DDS_WRITER_QOS_MASK                                                                       \
  (DDSI_QP_USER_DATA | DDSI_QP_DURABILITY | DDSI_QP_DURABILITY_SERVICE | DDSI_QP_DEADLINE |       \
   DDSI_QP_LATENCY_BUDGET | DDSI_QP_OWNERSHIP | DDSI_QP_OWNERSHIP_STRENGTH | DDSI_QP_LIVELINESS | \
   DDSI_QP_RELIABILITY | DDSI_QP_TRANSPORT_PRIORITY | DDSI_QP_LIFESPAN |                          \
   DDSI_QP_DESTINATION_ORDER | DDSI_QP_HISTORY | DDSI_QP_RESOURCE_LIMITS |                        \
   DDSI_QP_ADLINK_WRITER_DATA_LIFECYCLE | DDSI_QP_CYCLONE_IGNORELOCAL | DDSI_QP_PROPERTY_LIST |   \
   DDSI_QP_DATA_REPRESENTATION | DDSI_QP_ENTITY_NAME)

/*
  这段代码定义了两个函数：dds_ensure_valid_data_representation 和
  dds_apply_entity_naming。第一个函数用于确保给定的 QoS
  对象具有有效的数据表示属性，第二个函数用于将实体命名应用到给定的 QoS 对象上。
  */

/**
 * @brief 确保数据表示的有效性
 * @component qos_obj
 * @param[out] qos 指向要检查的dds_qos_t对象的指针
 * @param[in] allowed_data_representations 允许的数据表示的掩码
 * @param[in] topicqos 如果为true，则表示正在处理主题QoS；否则，表示正在处理其他实体的QoS
 * @return
 * 返回dds_return_t类型的结果，如果数据表示有效，则返回DDS_RETCODE_OK；否则，返回相应的错误代码
 */
dds_return_t dds_ensure_valid_data_representation(dds_qos_t* qos,
                                                  uint32_t allowed_data_representations,
                                                  bool topicqos);

/**
 * @brief 应用实体命名
 * @component qos_obj
 * @param[out] qos 指向要应用命名的dds_qos_t对象的指针
 * @param[in] parent_qos
 * 可选参数，指向父实体的dds_qos_t对象的指针，如果提供了此参数，则会将父实体的命名应用到子实体上
 * @param[in] gv 指向ddsi_domaingv结构的指针，包含与实体相关的全局变量
 */
void dds_apply_entity_naming(dds_qos_t* qos,
                             /* optional */ dds_qos_t* parent_qos,
                             struct ddsi_domaingv* gv);

#if defined(__cplusplus)
}
#endif
#endif /* DDS__QOS_H */
