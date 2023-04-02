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
#ifndef DDSI_XQOS_H
#define DDSI_XQOS_H

#include "dds/ddsc/dds_public_qosdefs.h"
#include "dds/ddsi/ddsi_log.h"
#include "dds/ddsi/ddsi_protocol.h"
#include "dds/ddsrt/retcode.h"
#include "dds/ddsrt/time.h"
#include "dds/features.h"

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @struct ddsi_typeinfo
 * @brief 未定义的结构体，用于后续扩展 (An undefined structure for future expansion)
 */
struct ddsi_typeinfo;

/**
 * @typedef ddsi_octetseq_t
 * @brief 字节序列类型别名 (Alias for byte sequence type)
 */

/**
 * @struct ddsi_octetseq
 * @brief 字节序列结构体 (Byte sequence structure)
 */
typedef struct ddsi_octetseq {
  uint32_t length;       ///< 序列长度 (Sequence length)
  unsigned char* value;  ///< 序列值指针 (Pointer to the sequence values)
} ddsi_octetseq_t;

/**
 * @typedef dds_userdata_qospolicy_t
 * @brief 用户数据 QoS 策略类型别名 (Alias for user data QoS policy type)
 */
typedef ddsi_octetseq_t dds_userdata_qospolicy_t;

/**
 * @typedef dds_topicdata_qospolicy_t
 * @brief 主题数据 QoS 策略类型别名 (Alias for topic data QoS policy type)
 */
typedef ddsi_octetseq_t dds_topicdata_qospolicy_t;

/**
 * @typedef dds_groupdata_qospolicy_t
 * @brief 组数据 QoS 策略类型别名 (Alias for group data QoS policy type)
 */
typedef ddsi_octetseq_t dds_groupdata_qospolicy_t;

/**
 * @struct dds_property
 * @brief DDS 属性结构体 (DDS property structure)
 */
typedef struct dds_property {
  /**
   * @brief 是否传播属性
   * 如果为 'false'，则整个结构不应发送。
   * 它必须是结构中的第一个变量，因为它在序列化器中映射到 XbPROP。
   * (Whether to propagate the property.
   * If 'false', the entire structure should not be sent.
   * It must be the first variable in the structure because it is mapped to XbPROP in the
   * serializer.)
   */
  unsigned char propagate;
  char* name;   ///< 属性名称 (Property name)
  char* value;  ///< 属性值 (Property value)
} dds_property_t;

/**
 * @struct dds_propertyseq
 * @brief DDS 属性序列结构体 (DDS property sequence structure)
 */
typedef struct dds_propertyseq {
  uint32_t n;             ///< 序列中属性的数量 (Number of properties in the sequence)
  dds_property_t* props;  ///< 属性指针数组 (Array of pointers to properties)
} dds_propertyseq_t;

/**
 * @struct dds_binaryproperty
 * @brief DDS 二进制属性结构体 (DDS binary property structure)
 */
typedef struct dds_binaryproperty {
  /**
   * @brief 是否传播二进制属性
   * 如果为 'false'，则整个结构不应发送。
   * 它必须是结构中的第一个变量，因为它在序列化器中映射到 XbPROP。
   * (Whether to propagate the binary property.
   * If 'false', the entire structure should not be sent.
   * It must be the first variable in the structure because it is mapped to XbPROP in the
   * serializer.)
   */
  unsigned char propagate;
  char* name;             ///< 二进制属性名称 (Binary property name)
  ddsi_octetseq_t value;  ///< 二进制属性值 (Binary property value)
} dds_binaryproperty_t;

/**
 * @struct dds_binarypropertyseq
 * @brief DDS 二进制属性序列结构体 (DDS binary property sequence structure)
 */
typedef struct dds_binarypropertyseq {
  uint32_t n;  ///< 序列中二进制属性的数量 (Number of binary properties in the sequence)
  dds_binaryproperty_t* props;  ///< 二进制属性指针数组 (Array of pointers to binary properties)
} dds_binarypropertyseq_t;

/**
 * @brief dds_property_qospolicy 结构体 (dds_property_qospolicy struct)
 * @param value 属性序列 (Property sequence)
 * @param binary_value 二进制属性序列 (Binary property sequence)
 */
typedef struct dds_property_qospolicy {
  dds_propertyseq_t value;               ///< 属性序列 (Property sequence)
  dds_binarypropertyseq_t binary_value;  ///< 二进制属性序列 (Binary property sequence)
} dds_property_qospolicy_t;

/**
 * @brief dds_durability_qospolicy 结构体 (dds_durability_qospolicy struct)
 * @param kind 持久化类型 (Durability kind)
 */
typedef struct dds_durability_qospolicy {
  dds_durability_kind_t kind;  ///< 持久化类型 (Durability kind)
} dds_durability_qospolicy_t;

/**
 * @brief dds_history_qospolicy 结构体 (dds_history_qospolicy struct)
 * @param kind 历史记录类型 (History kind)
 * @param depth 历史记录深度 (History depth)
 */
typedef struct dds_history_qospolicy {
  dds_history_kind_t kind;  ///< 历史记录类型 (History kind)
  int32_t depth;            ///< 历史记录深度 (History depth)
} dds_history_qospolicy_t;

/**
 * @brief dds_resource_limits_qospolicy 结构体 (dds_resource_limits_qospolicy struct)
 * @param max_samples 最大样本数 (Max samples)
 * @param max_instances 最大实例数 (Max instances)
 * @param max_samples_per_instance 每个实例的最大样本数 (Max samples per instance)
 */
typedef struct dds_resource_limits_qospolicy {
  int32_t max_samples;               ///< 最大样本数 (Max samples)
  int32_t max_instances;             ///< 最大实例数 (Max instances)
  int32_t max_samples_per_instance;  ///< 每个实例的最大样本数 (Max samples per instance)
} dds_resource_limits_qospolicy_t;

/**
 * @brief dds_durability_service_qospolicy 结构体 (dds_durability_service_qospolicy struct)
 * @param service_cleanup_delay 服务清理延迟 (Service cleanup delay)
 * @param history 历史记录策略 (History policy)
 * @param resource_limits 资源限制策略 (Resource limits policy)
 */
typedef struct dds_durability_service_qospolicy {
  dds_duration_t service_cleanup_delay;             ///< 服务清理延迟 (Service cleanup delay)
  dds_history_qospolicy_t history;                  ///< 历史记录策略 (History policy)
  dds_resource_limits_qospolicy_t resource_limits;  ///< 资源限制策略 (Resource limits policy)
} dds_durability_service_qospolicy_t;

/**
 * @brief dds_presentation_qospolicy 结构体 (dds_presentation_qospolicy struct)
 * @param access_scope 访问范围类型 (Access scope kind)
 * @param coherent_access 连贯访问标志 (Coherent access flag)
 * @param ordered_access 有序访问标志 (Ordered access flag)
 */
typedef struct dds_presentation_qospolicy {
  dds_presentation_access_scope_kind_t access_scope;  ///< 访问范围类型 (Access scope kind)
  unsigned char coherent_access;                      ///< 连贯访问标志 (Coherent access flag)
  unsigned char ordered_access;                       ///< 有序访问标志 (Ordered access flag)
} dds_presentation_qospolicy_t;

/**
 * @brief dds_deadline_qospolicy 结构体 (dds_deadline_qospolicy struct)
 * @param deadline 截止时间 (Deadline duration)
 */
typedef struct dds_deadline_qospolicy {
  dds_duration_t deadline;  ///< 截止时间 (Deadline duration)
} dds_deadline_qospolicy_t;

/**
 * @brief dds_latency_budget_qospolicy 结构体 (dds_latency_budget_qospolicy struct)
 * @param duration 延迟预算时长 (Latency budget duration)
 */
typedef struct dds_latency_budget_qospolicy {
  dds_duration_t duration;  ///< 延迟预算时长 (Latency budget duration)
} dds_latency_budget_qospolicy_t;

/**
 * @brief dds_ownership_qospolicy 结构体 (dds_ownership_qospolicy struct)
 * @param kind 所有权类型 (Ownership kind)
 */
typedef struct dds_ownership_qospolicy {
  dds_ownership_kind_t kind;  ///< 所有权类型 (Ownership kind)
} dds_ownership_qospolicy_t;

/**
 * @brief dds_ownership_strength_qospolicy 结构体 (dds_ownership_strength_qospolicy struct)
 * @param value 所有权强度值 (Ownership strength value)
 */
typedef struct dds_ownership_strength_qospolicy {
  int32_t value;  ///< 所有权强度值 (Ownership strength value)
} dds_ownership_strength_qospolicy_t;

/**
 * @brief dds_liveliness_qospolicy 结构体 (dds_liveliness_qospolicy struct)
 * @param kind 活跃度类型 (Liveliness kind)
 * @param lease_duration 租期时长 (Lease duration)
 */
typedef struct dds_liveliness_qospolicy {
  dds_liveliness_kind_t kind;     ///< 活跃度类型 (Liveliness kind)
  dds_duration_t lease_duration;  ///< 租期时长 (Lease duration)
} dds_liveliness_qospolicy_t;

/**
 * @brief dds_time_based_filter_qospolicy 结构体 (dds_time_based_filter_qospolicy struct)
 * @param minimum_separation 最小分离时间 (Minimum separation duration)
 */
typedef struct dds_time_based_filter_qospolicy {
  dds_duration_t minimum_separation;  ///< 最小分离时间 (Minimum separation duration)
} dds_time_based_filter_qospolicy_t;

/**
 * @brief 字符串序列结构体 (String sequence structure)
 */
typedef struct ddsi_stringseq {
  uint32_t n;   ///< 序列中字符串的数量 (Number of strings in the sequence)
  char** strs;  ///< 字符串指针数组 (Array of string pointers)
} ddsi_stringseq_t;

/**
 * @brief 分区 QoS 策略类型定义 (Partition QoS policy type definition)
 */
typedef ddsi_stringseq_t dds_partition_qospolicy_t;

/**
 * @brief 可靠性 QoS 策略结构体 (Reliability QoS policy structure)
 */
typedef struct dds_reliability_qospolicy {
  dds_reliability_kind_t kind;       ///< 可靠性种类 (Reliability kind)
  dds_duration_t max_blocking_time;  ///< 最大阻塞时间 (Maximum blocking time)
} dds_reliability_qospolicy_t;

/**
 * @brief 外部可靠性种类枚举 (External reliability kind enumeration)
 */
typedef enum dds_external_reliability_kind {
  DDS_EXTERNAL_RELIABILITY_BEST_EFFORT = 1,  ///< 最大努力传输 (Best-effort delivery)
  DDS_EXTERNAL_RELIABILITY_RELIABLE = 2      ///< 可靠传输 (Reliable delivery)
} dds_external_reliability_kind_t;

/**
 * @brief 传输优先级 QoS 策略结构体 (Transport priority QoS policy structure)
 */
typedef struct dds_transport_priority_qospolicy {
  int32_t value;  ///< 优先级值 (Priority value)
} dds_transport_priority_qospolicy_t;

/**
 * @brief 生命周期 QoS 策略结构体 (Lifespan QoS policy structure)
 */
typedef struct dds_lifespan_qospolicy {
  dds_duration_t duration;  ///< 生命周期持续时间 (Lifespan duration)
} dds_lifespan_qospolicy_t;

/**
 * @brief 目标顺序 QoS 策略结构体 (Destination order QoS policy structure)
 */
typedef struct dds_destination_order_qospolicy {
  dds_destination_order_kind_t kind;  ///< 目标顺序种类 (Destination order kind)
} dds_destination_order_qospolicy_t;

/**
 * @brief 实体工厂 QoS 策略结构体 (Entity factory QoS policy structure)
 */
typedef struct dds_entity_factory_qospolicy {
  unsigned char autoenable_created_entities;  ///< 是否自动启用创建的实体 (Whether to auto-enable
                                              ///< created entities)
} dds_entity_factory_qospolicy_t;

/**
 * @brief 写入器数据生命周期 QoS 策略结构体 (Writer data lifecycle QoS policy structure)
 */
typedef struct dds_writer_data_lifecycle_qospolicy {
  unsigned char autodispose_unregistered_instances;  ///< 是否自动处理未注册的实例 (Whether to
                                                     ///< auto-dispose unregistered instances)
} dds_writer_data_lifecycle_qospolicy_t;

/**
 * @brief 读取器数据生命周期 QoS 策略结构体 (Reader data lifecycle QoS policy structure)
 */
typedef struct dds_reader_data_lifecycle_qospolicy {
  dds_duration_t autopurge_nowriter_samples_delay;  ///< 无写入器样本自动清除延迟 (Auto-purge delay
                                                    ///< for no-writer samples)
  dds_duration_t autopurge_disposed_samples_delay;  ///< 废弃样本自动清除延迟 (Auto-purge delay for
                                                    ///< disposed samples)
} dds_reader_data_lifecycle_qospolicy_t;

/**
 * @brief 读取器生命周期 QoS 策略结构体 (Reader lifespan QoS policy structure)
 */
typedef struct dds_reader_lifespan_qospolicy {
  unsigned char use_lifespan;  ///< 是否使用生命周期 (Whether to use lifespan)
  dds_duration_t duration;     ///< 生命周期持续时间 (Lifespan duration)
} dds_reader_lifespan_qospolicy_t;

/**
 * @brief 忽略本地 QoS 策略结构体 (Ignore local QoS policy structure)
 */
typedef struct dds_ignorelocal_qospolicy {
  dds_ignorelocal_kind_t value;  ///< 忽略本地种类值 (Ignore local kind value)
} dds_ignorelocal_qospolicy_t;

/**
 * @brief 类型一致性强制策略 (Type Consistency Enforcement Policy)
 * @param kind 类型一致性种类 (Type Consistency Kind)
 * @param ignore_sequence_bounds 是否忽略序列边界 (Ignore Sequence Bounds)
 * @param ignore_string_bounds 是否忽略字符串边界 (Ignore String Bounds)
 * @param ignore_member_names 是否忽略成员名称 (Ignore Member Names)
 * @param prevent_type_widening 是否防止类型扩展 (Prevent Type Widening)
 * @param force_type_validation 是否强制类型验证 (Force Type Validation)
 */
typedef struct dds_type_consistency_enforcement_qospolicy {
  dds_type_consistency_kind_t kind;  ///< 类型一致性种类 (Type Consistency Kind)
  bool ignore_sequence_bounds;       ///< 是否忽略序列边界 (Ignore Sequence Bounds)
  bool ignore_string_bounds;         ///< 是否忽略字符串边界 (Ignore String Bounds)
  bool ignore_member_names;          ///< 是否忽略成员名称 (Ignore Member Names)
  bool prevent_type_widening;        ///< 是否防止类型扩展 (Prevent Type Widening)
  bool force_type_validation;        ///< 是否强制类型验证 (Force Type Validation)
} dds_type_consistency_enforcement_qospolicy_t;

/**
 * @brief 定位器掩码 (Locator Mask)
 */
typedef uint32_t dds_locator_mask_t;

/**
 * @brief 数据表示 ID 序列 (Data Representation ID Sequence)
 * @param n 序列中的元素数量 (Number of elements in the sequence)
 * @param ids 数据表示 ID 数组指针 (Pointer to the array of Data Representation IDs)
 */
typedef struct dds_data_representation_id_seq {
  uint32_t n;  ///< 序列中的元素数量 (Number of elements in the sequence)
  dds_data_representation_id_t*
      ids;     ///< 数据表示 ID 数组指针 (Pointer to the array of Data Representation IDs)
} dds_data_representation_id_seq_t;

/**
 * @brief 数据表示质量服务策略 (Data Representation QoS Policy)
 * @param value 数据表示 ID 序列值 (Data Representation ID Sequence Value)
 */
typedef struct dds_data_representation_qospolicy {
  dds_data_representation_id_seq_t
      value;  ///< 数据表示 ID 序列值 (Data Representation ID Sequence Value)
} dds_data_representation_qospolicy_t;

/***/

/* Qos Present bit indices */
/*
- `((uint64_t)1 << 13)` 是一个位操作表达式，它将整数 `1` 转换为 `uint64_t` 类型后，向左移动 `13`
位。结果是一个二进制数，其中第 `14` 位为 `1`，其余位为 `0`。 (The `((uint64_t)1 << 13)` is a bitwise
operation expression that takes the integer `1`, casts it to `uint64_t` type, and then shifts it
left by `13` positions. The result is a binary number with the 14th bit set to `1` and all other
bits set to `0`.)
*/
#define DDSI_QP_TOPIC_NAME ((uint64_t)1 << 0)
#define DDSI_QP_TYPE_NAME ((uint64_t)1 << 1)
#define DDSI_QP_PRESENTATION ((uint64_t)1 << 2)
#define DDSI_QP_PARTITION ((uint64_t)1 << 3)
#define DDSI_QP_GROUP_DATA ((uint64_t)1 << 4)
#define DDSI_QP_TOPIC_DATA ((uint64_t)1 << 5)
#define DDSI_QP_DURABILITY ((uint64_t)1 << 6)
#define DDSI_QP_DURABILITY_SERVICE ((uint64_t)1 << 7)
#define DDSI_QP_DEADLINE ((uint64_t)1 << 8)
#define DDSI_QP_LATENCY_BUDGET ((uint64_t)1 << 9)
#define DDSI_QP_LIVELINESS ((uint64_t)1 << 10)
#define DDSI_QP_RELIABILITY ((uint64_t)1 << 11)
#define DDSI_QP_DESTINATION_ORDER ((uint64_t)1 << 12)
#define DDSI_QP_HISTORY ((uint64_t)1 << 13)
#define DDSI_QP_RESOURCE_LIMITS ((uint64_t)1 << 14)
#define DDSI_QP_TRANSPORT_PRIORITY ((uint64_t)1 << 15)
#define DDSI_QP_LIFESPAN ((uint64_t)1 << 16)
#define DDSI_QP_USER_DATA ((uint64_t)1 << 17)
#define DDSI_QP_OWNERSHIP ((uint64_t)1 << 18)
#define DDSI_QP_OWNERSHIP_STRENGTH ((uint64_t)1 << 19)
#define DDSI_QP_TIME_BASED_FILTER ((uint64_t)1 << 20)
#define DDSI_QP_ADLINK_WRITER_DATA_LIFECYCLE ((uint64_t)1 << 21)
#define DDSI_QP_ADLINK_READER_DATA_LIFECYCLE ((uint64_t)1 << 22)
#define DDSI_QP_ADLINK_READER_LIFESPAN ((uint64_t)1 << 24)
// available: ((uint64_t)1 << 25)
#define DDSI_QP_ADLINK_ENTITY_FACTORY ((uint64_t)1 << 27)
#define DDSI_QP_CYCLONE_IGNORELOCAL ((uint64_t)1 << 30)
#define DDSI_QP_PROPERTY_LIST ((uint64_t)1 << 31)
#define DDSI_QP_TYPE_CONSISTENCY_ENFORCEMENT ((uint64_t)1 << 32)
#define DDSI_QP_TYPE_INFORMATION ((uint64_t)1 << 33)
#define DDSI_QP_LOCATOR_MASK ((uint64_t)1 << 34)
#define DDSI_QP_DATA_REPRESENTATION ((uint64_t)1 << 35)
#define DDSI_QP_ENTITY_NAME ((uint64_t)1 << 36)

/**
 * @brief Partition QoS is not RxO according to the specification (DDS 1.2,
 * section 7.1.3), but communication will not take place unless it
 * matches. Same for topic and type. Relaxed qos matching is a bit of
 * a weird one, but it affects matching, so ...
 *
 * 根据规范（DDS 1.2，第7.1.3节），分区QoS不是RxO，但除非匹配，否则不会进行通信。
 * 主题和类型也是如此。宽松的qos匹配有点奇怪，但它会影响匹配，所以...
 */

/*
"RxO" 是 "Receivability by Others"（可被其他人接收）的缩写。在 DDS（Data Distribution
Service，数据分发服务）中，它表示与接收方相关的一组 QoS（Quality of
Service，服务质量）策略。这些策略决定了一个发布者（Publisher）的数据是否能被一个订阅者（Subscriber）接收。

在给定的代码片段中，`DDSI_QP_RXO_MASK` 定义了与接收方相关的 QoS
策略掩码。这意味着当进行匹配时，这些策略需要在发布者和订阅者之间相互匹配，以便它们可以成功地通信。尽管根据规范（DDS
1.2，第7.1.3节），分区 QoS 不是 RxO，但除非匹配，否则不会进行通信。主题和类型也是如此。
*/

// 定义 DDSI_QP_RXO_MASK，表示与接收方相关的 QoS 策略掩码
// Define DDSI_QP_RXO_MASK, which represents the mask of QoS policies related to the receiver
#define DDSI_QP_RXO_MASK                                                                      \
  (DDSI_QP_DURABILITY | DDSI_QP_PRESENTATION | DDSI_QP_DEADLINE | DDSI_QP_LATENCY_BUDGET |    \
   DDSI_QP_OWNERSHIP | DDSI_QP_LIVELINESS | DDSI_QP_RELIABILITY | DDSI_QP_DESTINATION_ORDER | \
   DDSI_QP_DATA_REPRESENTATION)

// 定义 DDSI_QP_CHANGEABLE_MASK，表示可更改的 QoS 策略掩码
// Define DDSI_QP_CHANGEABLE_MASK, which represents the mask of changeable QoS policies
#define DDSI_QP_CHANGEABLE_MASK                                                      \
  (DDSI_QP_USER_DATA | DDSI_QP_TOPIC_DATA | DDSI_QP_GROUP_DATA | DDSI_QP_DEADLINE |  \
   DDSI_QP_LATENCY_BUDGET | DDSI_QP_OWNERSHIP_STRENGTH | DDSI_QP_TIME_BASED_FILTER | \
   DDSI_QP_PARTITION | DDSI_QP_TRANSPORT_PRIORITY | DDSI_QP_LIFESPAN |               \
   DDSI_QP_ADLINK_ENTITY_FACTORY | DDSI_QP_ADLINK_WRITER_DATA_LIFECYCLE |            \
   DDSI_QP_ADLINK_READER_DATA_LIFECYCLE)

// 定义 DDSI_QP_UNRECOGNIZED_INCOMPATIBLE_MASK，表示不兼容的未识别 QoS 策略掩码
// Define DDSI_QP_UNRECOGNIZED_INCOMPATIBLE_MASK, which represents the mask of unrecognized
// incompatible QoS policies
#define DDSI_QP_UNRECOGNIZED_INCOMPATIBLE_MASK ((uint64_t)0)

/* readers & writers have an extended qos, hence why it is a separate
   type */
struct dds_qos {
  /* Entries present, for sparse QoS（存在的条目，用于稀疏QoS） */
  uint64_t present;
  uint64_t aliased;

  /*v---- in ...Qos
     v--- in ...BuiltinTopicData
      v-- mapped in DDSI
       v- reader/writer/publisher/subscriber/participant specific */
  /*      Extras: */
  /* xx */ char* topic_name;
  /* xx */ char* type_name;
  /* xx */ char* entity_name;
#ifdef DDS_HAS_TYPE_DISCOVERY
  /* xx */ struct ddsi_typeinfo* type_information;
#endif
  /*      PublisherQos, SubscriberQos: */
  /*xxx */ dds_presentation_qospolicy_t presentation;
  /*xxx */ dds_partition_qospolicy_t partition;
  /*xxx */ dds_groupdata_qospolicy_t group_data;
  /*x xX*/ dds_entity_factory_qospolicy_t entity_factory;
  /*      TopicQos: */
  /*xxx */ dds_topicdata_qospolicy_t topic_data;
  /*      DataWriterQos, DataReaderQos: */
  /*xxx */ dds_durability_qospolicy_t durability;
  /*xxx */ dds_durability_service_qospolicy_t durability_service;
  /*xxx */ dds_deadline_qospolicy_t deadline;
  /*xxx */ dds_latency_budget_qospolicy_t latency_budget;
  /*xxx */ dds_liveliness_qospolicy_t liveliness;
  /*xxx */ dds_reliability_qospolicy_t reliability;
  /*xxx */ dds_destination_order_qospolicy_t destination_order;
  /*x x */ dds_history_qospolicy_t history;
  /*x x */ dds_resource_limits_qospolicy_t resource_limits;
  /*x x */ dds_transport_priority_qospolicy_t transport_priority;
  /*xxx */ dds_lifespan_qospolicy_t lifespan;
  /*xxx */ dds_userdata_qospolicy_t user_data;
  /*xxx */ dds_ownership_qospolicy_t ownership;
  /*xxxW*/ dds_ownership_strength_qospolicy_t ownership_strength;
  /*xxxR*/ dds_time_based_filter_qospolicy_t time_based_filter;
  /*x  W*/ dds_writer_data_lifecycle_qospolicy_t writer_data_lifecycle;
  /*x xR*/ dds_reader_data_lifecycle_qospolicy_t reader_data_lifecycle;
  /*x xR*/ dds_reader_lifespan_qospolicy_t reader_lifespan;
  /* x  */ dds_ignorelocal_qospolicy_t ignorelocal;
  /*xxx */ dds_property_qospolicy_t property;
  /*xxxR*/ dds_type_consistency_enforcement_qospolicy_t type_consistency;
  /*xxxX*/ dds_locator_mask_t ignore_locator_type;
  /*xxx */ dds_data_representation_qospolicy_t data_representation;
};

DDS_EXPORT extern const dds_qos_t ddsi_default_qos_reader;
DDS_EXPORT extern const dds_qos_t ddsi_default_qos_writer;
DDS_EXPORT extern const dds_qos_t ddsi_default_qos_topic;
DDS_EXPORT extern const dds_qos_t ddsi_default_qos_publisher_subscriber;
DDS_EXPORT extern const dds_qos_t ddsi_default_qos_participant;

/**
 * @brief 初始化一个新的空dds_qos_t对象 (Initialize a new empty dds_qos_t as an empty object)
 * @component qos_handling
 *
 * 原则上，这只会清除 "present" 和 "aliased" 位掩码。调试构建还会将所有其他字节初始化为0x55。
 * (In principle, this only clears the "present" and "aliased" bitmasks. A debug build
 * additionally initializes all other bytes to 0x55.)
 *
 * @param[out] xqos  要初始化的qos对象 (qos object to be initialized)
 */
void ddsi_xqos_init_empty(dds_qos_t* xqos);

/**
 * @brief 将 "src" 复制到 "dst" (Copy "src" to "dst")
 * @component qos_handling
 *
 * @param[out]    dst     目标，任何内容都将被覆盖 (destination, any contents are overwritten)
 * @param[in]     src     源dds_qos_t (source dds_qos_t)
 */
void ddsi_xqos_copy(dds_qos_t* dst, const dds_qos_t* src);

/**
 * @brief 释放 "xqos" 所拥有的内存 (Free memory owned by "xqos")
 * @component qos_handling
 *
 * 根据设置的字段、它们的类型以及它们是否被标记为 "aliased"，dds_qos_t 可能拥有其他分配的内存块。
 * (A dds_qos_t may own other allocated blocks of memory, depending on which fields are
 * set, their types and whether they are marked as "aliased".)
 * 此函数释放 "xqos" 所拥有的任何此类内存，但不是 "xqos" 本身。
 * (This function releases any such memory owned by "xqos", but not "xqos" itself.)
 * 之后，"xqos" 的内容未定义，除非重新初始化它，否则不得再次使用。
 * (Afterward, the content of "xqos" is undefined and must not be used again without initialising
 * it.)
 *
 * @param[in] xqos   要释放内存的dds_qos_t (dds_qos_t for which to free memory)
 */
void ddsi_xqos_fini(dds_qos_t* xqos);

/**
 * @brief 根据规范中的验证规则检查xqos是否有效 (Check whether xqos is valid according to the
 * validation rules in the spec)
 * @component qos_handling
 *
 * 检查涉及各个字段的值以及少数字段组合。只检查设置的那些（默认值都是有效的），
 * (The checks concern the values for the individual fields as well as a few combinations
 * of fields. Only those that are set are checked (the defaults are all valid anyway),)
 * 如果需要检查字段组合并且某些但不是所有字段已指定，则使用缺失字段的默认值。
 * (and where a combination of fields must be checked and some but not all fields are
 * specified, it uses the defaults for the missing ones.)
 *
 * 根据指定的日志记录配置，将无效值记录为类别 "plist"。
 * (Invalid values get logged as category "plist" according to the specified logging
 * configuration.)
 *
 * @param[in] logcfg  日志记录配置 (logging configuration)
 * @param[in] xqos    要检查的qos对象 (qos object to check)
 *
 * @returns DDS_RETCODE_OK 或 DDS_RETCODE_BAD_PARAMETER (DDS_RETCODE_OK or
 * DDS_RETCODE_BAD_PARAMETER)
 */
dds_return_t ddsi_xqos_valid(const struct ddsrt_log_cfg* logcfg, const dds_qos_t* xqos);

/**
 * @brief 用 "b" 中的选定条目扩展 "a"
 * @brief Extend "a" with selected entries present in "b"
 * @component qos_handling
 *
 * 这将把 "b" 中存在的、在 "mask" 中包含的且在 "a" 中缺失的条目复制到 "a"。它不会触及 "a"
 * 中已经存在的任何条目。 This copies into "a" any entries present in "b" that are included in
 * "mask" and missing in "a".  It doesn't touch any entries already present in "a". 将空的 "a"
 * 和所有位设置为 "mask" 等同于将 "b" 复制到 "a"；调用这个函数时，如果 "mask" 为
 * 0，则不复制任何内容。 Calling this on an empty "a" with all bits set in "mask" is equivalent to
 * copying "b" into "a"; calling this with "mask" 0 copies nothing.
 *
 * @param[in,out] a       要扩展的 dds_qos_t
 * @param[in,out] a       dds_qos_t to be extended
 * @param[in]     b       要从中复制条目的 dds_qos_t
 * @param[in]     b       dds_qos_t from which to copy entries
 * @param[in]     mask    要包含的掩码（如果设置了 DDSI_QP_X，则包括 X）
 * @param[in]     mask    which to include (if DDSI_QP_X is set, include X)
 */
void ddsi_xqos_mergein_missing(dds_qos_t* a, const dds_qos_t* b, uint64_t mask);

/**
 * @brief 确定 "x" 与 "y" 不同的条目集
 * @brief Determine the set of entries in which "x" differs from "y"
 * @component qos_handling
 *
 * 这将计算在 "x" 中设置但在 "y" 中未设置的条目，或者在 "x" 中未设置但在 "y" 中设置的条目，或者在
 * "x" 和 "y" 中都设置但具有不同值的条目。 This computes the entries set in "x" but not set in "y",
 * not set in "x" but set in "y", or set in both "x" and "y" but to a different value.
 * 它返回仅包含在 "mask" 中的此集合，即，如果 "mask" 中的位 X 清除，则结果中的位 X 将清除。
 * It returns this set reduced to only those included in "mask", that is, if bit X is clear in
 * "mask", bit X will be clear in the result.
 *
 * @param[in]  a         要比较的两个 plist 中的一个
 * @param[in]  a         one of the two plists to compare
 * @param[in]  b         要比较的另一个 plist
 * @param[in]  b         other plist to compare
 * @param[in]  mask      要比较的条目子集
 * @param[in]  mask      subset of entries to be compared
 *
 * @returns 差异的位掩码
 * @returns Bitmask of differences
 */
uint64_t ddsi_xqos_delta(const dds_qos_t* a, const dds_qos_t* b, uint64_t mask);

/**
 * @brief 如果不存在，则将属性 'name' 添加到 "xqos" 的属性中
 * @brief Add a property 'name' to the properties of "xqos" if it does not exists
 * @component qos_handling
 *
 * @param[in]  xqos        要添加属性的 qos 对象。
 * @param[in]  xqos        qos object to add property to.
 * @param[in]  propagate   是否传播（发射到线）属性
 * @param[in]  propagate   whether to propagate (emit to wire) the property
 * @param[in]  name        属性名称
 * @param[in]  name        property name
 * @param[in]  value       属性值
 * @param[in]  value       property value
 *
 * @returns 当且仅当 xqos 被修改时返回 true（属性尚不存在）
 * @returns true iff xqos was modified (property did not exist yet)
 */
bool ddsi_xqos_add_property_if_unset(dds_qos_t* xqos,
                                     bool propagate,
                                     const char* name,
                                     const char* value);

/**
 * @brief 复制 "src"
 * @brief Duplicate "src"
 * @component qos_handling
 *
 * @param[in]  src       要复制的 dds_qos_t
 * @param[in]  src       dds_qos_t to be duplicated
 *
 * @returns 包含 "src" 副本的新的（使用 ddsrt_malloc 分配的）dds_qos_t。
 * @returns a new (allocated using ddsrt_malloc) dds_qos_t containing a copy of "src".
 */
dds_qos_t* ddsi_xqos_dup(const dds_qos_t* src);

#if defined(__cplusplus)
}
#endif

#endif /* DDSI_XQOS_H */
