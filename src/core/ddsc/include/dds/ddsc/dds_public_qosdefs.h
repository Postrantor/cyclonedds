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

#ifndef DDS_QOSDEFS_H
#define DDS_QOSDEFS_H

#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @anchor DDS_LENGTH_UNLIMITED
 * @ingroup qos
 * @brief 用于在 dds_qset_resource_limits() 中表示无限长度 (Used for indicating unlimited length in
 * dds_qset_resource_limits())
 */
#define DDS_LENGTH_UNLIMITED -1

/**
 * @brief Qos 政策 ID (Qos Policy IDs)
 * @ingroup internal
 * 用于内部标记 QoS 政策类型 (Used internally to mark the QoS policy type)
 */
typedef enum dds_qos_policy_id {
  DDS_INVALID_QOS_POLICY_ID,      /**< 无效政策 (Invalid Policy) */
  DDS_USERDATA_QOS_POLICY_ID,     /**< 用户数据政策 dds_qset_userdata() (Userdata policy
                                     dds_qset_userdata()) */
  DDS_DURABILITY_QOS_POLICY_ID,   /**< 持久性政策 dds_qset_durability() (Durability policy
                                     dds_qset_durability()) */
  DDS_PRESENTATION_QOS_POLICY_ID, /**< 展示政策 dds_qset_presentation() (Presentation policy
                                     dds_qset_presentation()) */
  DDS_DEADLINE_QOS_POLICY_ID,     /**< 最后期限政策 dds_qset_deadline() (Deadline policy
                                     dds_qset_deadline()) */
  DDS_LATENCYBUDGET_QOS_POLICY_ID, /**< 延迟预算政策 dds_qset_latency_budget() (LatencyBudget policy
                                      dds_qset_latency_budget()) */
  DDS_OWNERSHIP_QOS_POLICY_ID,         /**< 所有权政策 dds_qset_ownership() (Ownership policy
                                          dds_qset_ownership()) */
  DDS_OWNERSHIPSTRENGTH_QOS_POLICY_ID, /**< 所有权强度政策 dds_qset_ownership_strength()
                                          (OwnershipStrength policy dds_qset_ownership_strength())
                                        */
  DDS_LIVELINESS_QOS_POLICY_ID,        /**< 活跃性政策 dds_qset_liveliness() (Liveliness policy
                                          dds_qset_liveliness()) */
  DDS_TIMEBASEDFILTER_QOS_POLICY_ID,   /**< 基于时间过滤器政策 dds_qset_time_based_filter()
                                          (TimeBasedFilter policy dds_qset_time_based_filter()) */
  DDS_PARTITION_QOS_POLICY_ID,         /**< 分区政策 dds_qset_partition() (Partition policy
                                          dds_qset_partition()) */
  DDS_RELIABILITY_QOS_POLICY_ID, /**< 可靠性政策 dds_qset_reliability() (Reliability policy
                                    dds_qset_reliability()) */
  DDS_DESTINATIONORDER_QOS_POLICY_ID, /**< 目的地顺序政策 dds_qset_destination_order()
                                         (DestinationOrder policy dds_qset_destination_order()) */
  DDS_HISTORY_QOS_POLICY_ID, /**< 历史政策 dds_qset_history() (History policy dds_qset_history()) */
  DDS_RESOURCELIMITS_QOS_POLICY_ID, /**< 资源限制政策 dds_qset_resource_limits() (ResourceLimits
                                       policy dds_qset_resource_limits()) */
  DDS_ENTITYFACTORY_QOS_POLICY_ID, /**< 实体工厂政策 (EntityFactory policy) */
  DDS_WRITERDATALIFECYCLE_QOS_POLICY_ID, /**< 写入数据生命周期政策 dds_qset_writer_data_lifecycle()
                                            (WriterDataLifecycle policy
                                            dds_qset_writer_data_lifecycle()) */
  DDS_READERDATALIFECYCLE_QOS_POLICY_ID, /**< 读取数据生命周期政策 dds_qset_reader_data_lifecycle()
                                            (ReaderDataLifecycle policy
                                            dds_qset_reader_data_lifecycle()) */
  DDS_TOPICDATA_QOS_POLICY_ID, /**< 主题数据政策 dds_qset_topicdata() (Topicdata policy
                                  dds_qset_topicdata()) */
  DDS_GROUPDATA_QOS_POLICY_ID, /**< 组数据政策 dds_qset_groupdata() (Groupdata policy
                                  dds_qset_groupdata()) */
  DDS_TRANSPORTPRIORITY_QOS_POLICY_ID, /**< 传输优先级政策 dds_qset_transport_priority()
                                          (TransportPriority policy dds_qset_transport_priority())
                                        */
  DDS_LIFESPAN_QOS_POLICY_ID,          /**< 生命周期政策 dds_qset_lifespan() (Livespan policy
                                          dds_qset_lifespan()) */
  DDS_DURABILITYSERVICE_QOS_POLICY_ID, /**< 持久性服务政策 dds_qset_durability_service()
                                          (DurabilityService policy dds_qset_durability_service())
                                        */
  DDS_PROPERTY_QOS_POLICY_ID,          /**< 属性政策 dds_qset_property() (Property policy
                                          dds_qset_property()) */
  DDS_TYPE_CONSISTENCY_ENFORCEMENT_QOS_POLICY_ID, /**< 类型一致性强制政策
                                                     dds_qset_type_consistency_enforcements()
                                                     (TypeConsistencyEnforcement policy
                                                     dds_qset_type_consistency_enforcements()) */
  DDS_DATA_REPRESENTATION_QOS_POLICY_ID /**< 数据表示政策 dds_qset_data_representation()
                                           (DataRepresentation policy
                                           dds_qset_data_representation()) */
} dds_qos_policy_id_t;

/**
 * @brief QoS 数据类型 (QoS datatype)
 * @ingroup qos
 * QoS 结构是不透明的 (QoS structure is opaque)
 */
typedef struct dds_qos dds_qos_t;

/**
 * @brief 持久性 QoS：适用于 Topic、DataReader 和 DataWriter (Durability QoS: Applies to Topic,
 * DataReader, DataWriter)
 * @ingroup qos
 */
typedef enum dds_durability_kind {
  DDS_DURABILITY_VOLATILE,        /**< 易失性持久性 (Volatile durability) */
  DDS_DURABILITY_TRANSIENT_LOCAL, /**< 本地瞬态持久性 (Transient Local durability) */
  DDS_DURABILITY_TRANSIENT,       /**< 瞬态持久性 (Transient durability) */
  DDS_DURABILITY_PERSISTENT       /**< 持久性持久性 (Persistent durability) */
} dds_durability_kind_t;

/**
 * @brief 历史记录 QoS：适用于 Topic、DataReader 和 DataWriter (History QoS: Applies to Topic,
 * DataReader, DataWriter)
 * @ingroup qos
 */
typedef enum dds_history_kind {
  DDS_HISTORY_KEEP_LAST, /**< 保留最后的历史记录 (Keep Last history) */
  DDS_HISTORY_KEEP_ALL   /**< 保留所有历史记录 (Keep All history) */
} dds_history_kind_t;

/**
 * @brief 所有权 QoS：适用于 Topic、DataReader 和 DataWriter (Ownership QoS: Applies to Topic,
 * DataReader, DataWriter)
 * @ingroup qos
 */
typedef enum dds_ownership_kind {
  DDS_OWNERSHIP_SHARED,   /**< 共享所有权 (Shared Ownership) */
  DDS_OWNERSHIP_EXCLUSIVE /**< 独占所有权 (Exclusive Ownership) */
} dds_ownership_kind_t;

/**
 * @brief Liveliness QoS: 应用于 Topic, DataReader, DataWriter
 *        Liveliness QoS: Applies to Topic, DataReader, DataWriter
 * @ingroup qos
 */
typedef enum dds_liveliness_kind {
  DDS_LIVELINESS_AUTOMATIC,             /**< 自动活跃度 Automatic liveliness */
  DDS_LIVELINESS_MANUAL_BY_PARTICIPANT, /**< 参与者手动活跃度 Manual by Participant liveliness */
  DDS_LIVELINESS_MANUAL_BY_TOPIC        /**< 主题手动活跃度 Manual by Topic liveliness */
} dds_liveliness_kind_t;

/**
 * @brief Reliability QoS: 应用于 Topic, DataReader, DataWriter
 *        Reliability QoS: Applies to Topic, DataReader, DataWriter
 * @ingroup qos
 */
typedef enum dds_reliability_kind {
  DDS_RELIABILITY_BEST_EFFORT, /**< 尽力而为可靠性 Best Effort reliability */
  DDS_RELIABILITY_RELIABLE     /**< 可靠的可靠性 Reliable reliability */
} dds_reliability_kind_t;

/**
 * @brief DestinationOrder QoS: 应用于 Topic, DataReader, DataWriter
 *        DestinationOrder QoS: Applies to Topic, DataReader, DataWriter
 * @ingroup qos
 */
typedef enum dds_destination_order_kind {
  DDS_DESTINATIONORDER_BY_RECEPTION_TIMESTAMP, /**< 按接收时间戳排序 order by reception timestamp */
  DDS_DESTINATIONORDER_BY_SOURCE_TIMESTAMP /**< 按源时间戳排序 order by source timestamp */
} dds_destination_order_kind_t;

/**
 * @brief Presentation QoS: 应用于 Publisher, Subscriber
 *        Presentation QoS: Applies to Publisher, Subscriber
 * @ingroup qos
 */
typedef enum dds_presentation_access_scope_kind {
  DDS_PRESENTATION_INSTANCE, /**< 每个实例的表示范围 presentation scope per instance */
  DDS_PRESENTATION_TOPIC,    /**< 每个主题的表示范围 presentation scope per topic */
  DDS_PRESENTATION_GROUP     /**< 每个组的表示范围 presentation scope per group */
} dds_presentation_access_scope_kind_t;

/**
 * @brief 忽略本地 QoS：适用于 DataReader 和 DataWriter
 *        Ignore-local QoS: Applies to DataReader, DataWriter
 * @ingroup qos
 */
typedef enum dds_ignorelocal_kind {
  DDS_IGNORELOCAL_NONE,        /**< 不忽略本地数据 Don't ignore local data */
  DDS_IGNORELOCAL_PARTICIPANT, /**< 忽略来自同一参与者的本地数据 Ignore local data from same
                                  participant */
  DDS_IGNORELOCAL_PROCESS /**< 忽略来自同一进程的本地数据 Ignore local data from same process */
} dds_ignorelocal_kind_t;

/**
 * @brief 类型一致性 QoS：适用于 DataReader 和 DataWriter
 *        Type-consistency QoS: Applies to DataReader, DataWriter
 * @ingroup qos
 */
typedef enum dds_type_consistency_kind {
  DDS_TYPE_CONSISTENCY_DISALLOW_TYPE_COERCION, /**< 不允许类型强制转换 Do not allow type coercion */
  DDS_TYPE_CONSISTENCY_ALLOW_TYPE_COERCION /**< 允许类型强制转换 Allow type coercion */
} dds_type_consistency_kind_t;

/**
 * @brief 数据表示 QoS：适用于 Topic，DataReader 和 DataWriter
 *        Data Representation QoS: Applies to Topic, DataReader, DataWriter
 * @ingroup qos
 */
typedef int16_t dds_data_representation_id_t;

#if defined(__cplusplus)
}
#endif
#endif
