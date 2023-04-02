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
#ifndef DDS_H
#define DDS_H

/**
 * @file
 * @brief Eclipse Cyclone DDS C 头文件 (Eclipse Cyclone DDS C header)
 * 包含了 Cyclone DDS C 库的主要头文件，包含您需要的所有 DDS 应用程序内容。
 * (Main header of the Cyclone DDS C library, containing everything you need
 * for your DDS application.)
 */

/**
 * @defgroup dds (DDS 功能) (DDS Functionality)
 */
/**
 * @defgroup deprecated (已弃用功能) (Deprecated functionality)
 */

// 如果定义了 __cplusplus，则将 restrict 宏定义为空
// (If defined(__cplusplus), define the restrict macro as empty)
#if defined(__cplusplus)
#define restrict
#endif

// 包含 dds_basic_types.h 头文件，提供基本类型定义
// (Include dds_basic_types.h header file, which provides basic type definitions)
#include "dds/ddsc/dds_basic_types.h"

// 包含 export.h 头文件，提供导出宏定义
// (Include export.h header file, which provides export macro definitions)
#include "dds/export.h"

// 包含 features.h 头文件，提供特性相关定义
// (Include features.h header file, which provides feature-related definitions)
#include "dds/features.h"

/**
 * @brief Cyclone DDS 公共头文件 (Cyclone DDS Public Header Files)
 * 子组件 (Sub components)
 */
// 包含用于分配内存的公共函数 (Includes public functions for memory allocation)
#include "dds/ddsc/dds_public_alloc.h"
// 包含用于处理错误的公共函数 (Includes public functions for error handling)
#include "dds/ddsc/dds_public_error.h"
// 包含实现公共接口的函数 (Includes functions implementing the public interface)
#include "dds/ddsc/dds_public_impl.h"
// 包含用于处理监听器的公共函数 (Includes public functions for listener handling)
#include "dds/ddsc/dds_public_listener.h"
// 包含用于处理服务质量（QoS）的公共函数 (Includes public functions for Quality of Service (QoS)
// handling)
#include "dds/ddsc/dds_public_qos.h"
// 包含用于处理状态的公共函数 (Includes public functions for status handling)
#include "dds/ddsc/dds_public_status.h"
// 包含日志功能 (Includes logging functionality)
#include "dds/ddsrt/log.h"
// 包含返回代码定义 (Includes return code definitions)
#include "dds/ddsrt/retcode.h"
// 包含时间处理功能 (Includes time handling functionality)
#include "dds/ddsrt/time.h"

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @brief DDS 类型标识符 (XTypes)
 * @ingroup dds
 * DOC_TODO
 *
 * @brief DDS Type Identifier (XTypes)
 * @ingroup dds
 * DOC_TODO
 */
typedef struct ddsi_typeid dds_typeid_t;

/**
 * @brief DDS 类型信息 (XTypes)
 * @ingroup dds
 * DOC_TODO
 *
 * @brief DDS Type Information (XTypes)
 * @ingroup dds
 * DOC_TODO
 */
typedef struct ddsi_typeinfo dds_typeinfo_t;

/**
 * @brief DDS 类型对象 (XTypes)
 * @ingroup dds
 * DOC_TODO
 *
 * @brief DDS Type Object (XTypes)
 * @ingroup dds
 * DOC_TODO
 */
typedef struct ddsi_typeobj dds_typeobj_t;

/**
 * @brief 读取器历史缓存
 * @ingroup dds
 * DOC_TODO
 *
 * @brief Reader History Cache
 * @ingroup dds
 * DOC_TODO
 */
struct dds_rhc;

/**
 * @brief DDSI 参数列表
 * @ingroup dds
 * DOC_TODO
 *
 * @brief DDSI parameter list
 * @ingroup dds
 * DOC_TODO
 */
struct ddsi_plist;

/**
 * @anchor ddsi_sertype
 * @brief DDSI 序列化类型
 * @ingroup dds
 * DOC_TODO
 *
 * @anchor ddsi_sertype
 * @brief DDSI sertype
 * @ingroup dds
 * DOC_TODO
 */
struct ddsi_sertype;

/**
 * @anchor ddsi_serdata
 * @brief DDSI 序列化数据
 * @ingroup dds
 * DOC_TODO
 *
 * @anchor ddsi_serdata
 * @brief DDSI Serdata
 * @ingroup dds
 * DOC_TODO
 */
struct ddsi_serdata;

/**
 * @brief DDSI 配置
 * @ingroup dds
 * DOC_TODO
 *
 * @brief DDSI Config
 * @ingroup dds
 * DOC_TODO
 */
struct ddsi_config;

/**
 * @brief 表示库使用 ddsi_sertype 而不是 ddsi_sertopic
 * @ingroup dds
 *
 * @brief Indicates that the library uses ddsi_sertype instead of ddsi_sertopic
 * @ingroup dds
 */
#define DDS_HAS_DDSI_SERTYPE 1

/**
 * @defgroup builtintopic (内置主题支持)
 * @ingroup dds
 *
 * @defgroup builtintopic (Builtin Topic Support)
 * @ingroup dds
 */
/**
 * @defgroup builtintopic_constants (常量)
 * @ingroup builtintopic
 * @brief 方便地引用内置主题的常量
 *
 * @defgroup builtintopic_constants (Constants)
 * @ingroup builtintopic
 * @brief Convenience constants for referring to builtin topics
 */
/**
 * @def DDS_BUILTIN_TOPIC_DCPSPARTICIPANT
 * @ingroup builtintopic_constants
 * 内置主题 DcpsParticipant 的伪 dds_topic_t。此主题的样本为
 * @ref dds_builtintopic_participant 结构体。
 *
 * @def DDS_BUILTIN_TOPIC_DCPSPARTICIPANT
 * @ingroup builtintopic_constants
 * Pseudo dds_topic_t for the builtin topic DcpsParticipant. Samples from this topic are
 * @ref dds_builtintopic_participant structs.
 */
/**
 * @def DDS_BUILTIN_TOPIC_DCPSTOPIC
 * @ingroup builtintopic_constants
 * 内置主题 DcpsTopic 的伪 dds_topic_t。此主题的样本为
 * @ref dds_builtintopic_topic 结构体。请注意，只有在 cmake 构建中指定了
 * ENABLE_TOPIC_DISCOVERY 时，这才有效。
 *
 * @def DDS_BUILTIN_TOPIC_DCPSTOPIC
 * @ingroup builtintopic_constants
 * Pseudo dds_topic_t for the builtin topic DcpsTopic. Samples from this topic are
 * @ref dds_builtintopic_topic structs. Note that this only works if you have specified
 * ENABLE_TOPIC_DISCOVERY in your cmake build.
 */
/**
 * @def DDS_BUILTIN_TOPIC_DCPSPUBLICATION
 * @ingroup builtintopic_constants
 * 内置主题 DcpsPublication 的伪 dds_topic_t。此主题的样本为
 * @ref dds_builtintopic_endpoint 结构体。
 *
 * @def DDS_BUILTIN_TOPIC_DCPSPUBLICATION
 * @ingroup builtintopic_constants
 * Pseudo dds_topic_t for the builtin topic DcpsPublication. Samples from this topic are
 * @ref dds_builtintopic_endpoint structs.
 */
/**
 * @def DDS_BUILTIN_TOPIC_DCPSSUBSCRIPTION
 * @ingroup builtintopic_constants
 * 内置主题 DcpsSubscription 的伪 dds_topic_t。此主题的样本为
 * @ref dds_builtintopic_endpoint 结构体。
 *
 * @def DDS_BUILTIN_TOPIC_DCPSSUBSCRIPTION
 * @ingroup builtintopic_constants
 * Pseudo dds_topic_t for the builtin topic DcpsSubscription. Samples from this topic are
 * @ref dds_builtintopic_endpoint structs.
 */

#define DDS_BUILTIN_TOPIC_DCPSPARTICIPANT ((dds_entity_t)(DDS_MIN_PSEUDO_HANDLE + 1))
#define DDS_BUILTIN_TOPIC_DCPSTOPIC ((dds_entity_t)(DDS_MIN_PSEUDO_HANDLE + 2))
#define DDS_BUILTIN_TOPIC_DCPSPUBLICATION ((dds_entity_t)(DDS_MIN_PSEUDO_HANDLE + 3))
#define DDS_BUILTIN_TOPIC_DCPSSUBSCRIPTION ((dds_entity_t)(DDS_MIN_PSEUDO_HANDLE + 4))

/**
 * @ingroup DOC_TODO
 * 特殊句柄，表示实体强制 dds_data_allocator 在堆上分配内存
 * Special handle representing the entity which forces the dds_data_allocator to allocate on heap
 */
#define DDS_DATA_ALLOCATOR_ALLOC_ON_HEAP ((dds_entity_t)(DDS_MIN_PSEUDO_HANDLE + 257))

/**
 * @defgroup entity_status (Entity Status)
 * @ingroup entity
 * 所有实体都有一组“状态条件”
 * All entities have a set of "status conditions"
 * （遵循 DCPS 规范），读取窥视，获取读取和重置（类似于读取器上的读取和获取操作）。
 * “掩码”允许仅对状态的子集进行操作。 (following the DCPS spec), read peeks, take reads & resets
 * (analogously to read & take operations on reader). The "mask" allows operating only on a subset
 * of the statuses. 根据 DCPS 规范启用状态。 Enabled status analogously to DCPS spec.
 * @{
 */
/**
 * @brief 这些标识符用于生成位移后的标识符。
 * These identifiers are used to generate the bitshifted identifiers.
 * 通过使用位标志而不是这些 ID，构建状态掩码的过程简化为使用简单的二进制 OR 操作。
 * By using bitflags instead of these IDs the process of building status masks is
 * simplified to using simple binary OR operations.
 * DOC_TODO 修复引用
 * DOC_TODO fix the refs
 */
typedef enum dds_status_id {
  DDS_INCONSISTENT_TOPIC_STATUS_ID,         /**< 参见 @ref DDS_INCONSISTENT_TOPIC_STATUS */
                                            /**< See @ref DDS_INCONSISTENT_TOPIC_STATUS */
  DDS_OFFERED_DEADLINE_MISSED_STATUS_ID,    /**< 参见 @ref DDS_OFFERED_DEADLINE_MISSED_STATUS */
                                            /**< See @ref DDS_OFFERED_DEADLINE_MISSED_STATUS */
  DDS_REQUESTED_DEADLINE_MISSED_STATUS_ID,  /**< 参见 @ref DDS_REQUESTED_DEADLINE_MISSED_STATUS */
                                            /**< See @ref DDS_REQUESTED_DEADLINE_MISSED_STATUS */
  DDS_OFFERED_INCOMPATIBLE_QOS_STATUS_ID,   /**< 参见 @ref DDS_OFFERED_INCOMPATIBLE_QOS_STATUS */
                                            /**< See @ref DDS_OFFERED_INCOMPATIBLE_QOS_STATUS */
  DDS_REQUESTED_INCOMPATIBLE_QOS_STATUS_ID, /**< 参见 @ref DDS_REQUESTED_INCOMPATIBLE_QOS_STATUS */
                                            /**< See @ref DDS_REQUESTED_INCOMPATIBLE_QOS_STATUS */
  DDS_SAMPLE_LOST_STATUS_ID,                /**< 参见 @ref DDS_SAMPLE_LOST_STATUS */
                                            /**< See @ref DDS_SAMPLE_LOST_STATUS */
  DDS_SAMPLE_REJECTED_STATUS_ID,            /**< 参见 @ref DDS_SAMPLE_REJECTED_STATUS */
                                            /**< See @ref DDS_SAMPLE_REJECTED_STATUS */
  DDS_DATA_ON_READERS_STATUS_ID,            /**< 参见 @ref DDS_DATA_ON_READERS_STATUS */
                                            /**< See @ref DDS_DATA_ON_READERS_STATUS */
  DDS_DATA_AVAILABLE_STATUS_ID,             /**< 参见 @ref DDS_DATA_AVAILABLE_STATUS */
                                            /**< See @ref DDS_DATA_AVAILABLE_STATUS */
  DDS_LIVELINESS_LOST_STATUS_ID,            /**< 参见 @ref DDS_LIVELINESS_LOST_STATUS */
                                            /**< See @ref DDS_LIVELINESS_LOST_STATUS */
  DDS_LIVELINESS_CHANGED_STATUS_ID,         /**< 参见 @ref DDS_LIVELINESS_CHANGED_STATUS */
                                            /**< See @ref DDS_LIVELINESS_CHANGED_STATUS */
  DDS_PUBLICATION_MATCHED_STATUS_ID,        /**< 参见 @ref DDS_PUBLICATION_MATCHED_STATUS */
                                            /**< See @ref DDS_PUBLICATION_MATCHED_STATUS */
  DDS_SUBSCRIPTION_MATCHED_STATUS_ID        /**< 参见 @ref DDS_SUBSCRIPTION_MATCHED_STATUS */
                                            /**< See @ref DDS_SUBSCRIPTION_MATCHED_STATUS */
} dds_status_id_t;

/** 辅助值，表示状态掩码中可以设置的最高位。 */
/** Helper value to indicate the highest bit that can be set in a status mask. */
#define DDS_STATUS_ID_MAX (DDS_SUBSCRIPTION_MATCHED_STATUS_ID)

/**
 * @anchor DDS_INCONSISTENT_TOPIC_STATUS
 * 存在具有相同名称但具有不同特征的其他主题。
 * Another topic exists with the same name but with different characteristics.
 */
#define DDS_INCONSISTENT_TOPIC_STATUS (1u << DDS_INCONSISTENT_TOPIC_STATUS_ID)
/**
 * @anchor DDS_OFFERED_DEADLINE_MISSED_STATUS
 * 写入器通过其截止日期 QoS 策略承诺的截止日期未得到尊重，针对特定实例。
 * The deadline that the writer has committed through its deadline QoS policy was not respected for
 * a specific instance. */
#define DDS_OFFERED_DEADLINE_MISSED_STATUS (1u << DDS_OFFERED_DEADLINE_MISSED_STATUS_ID)
/**
 * @anchor DDS_REQUESTED_DEADLINE_MISSED_STATUS
 * 阅读器通过其截止日期 QoS 策略期望的截止日期未得到尊重，针对特定实例。
 * The deadline that the reader was expecting through its deadline QoS policy was not respected for
 * a specific instance. */
#define DDS_REQUESTED_DEADLINE_MISSED_STATUS (1u << DDS_REQUESTED_DEADLINE_MISSED_STATUS_ID)
/**
 * @anchor DDS_OFFERED_INCOMPATIBLE_QOS_STATUS
 * QoS 策略设置与请求的内容不兼容。
 * A QoS policy setting was incompatible with what was requested. */
#define DDS_OFFERED_INCOMPATIBLE_QOS_STATUS (1u << DDS_OFFERED_INCOMPATIBLE_QOS_STATUS_ID)
/**
 * @anchor DDS_REQUESTED_INCOMPATIBLE_QOS_STATUS
 * QoS 策略设置与提供的内容不兼容。
 * A QoS policy setting was incompatible with what is offered. */
#define DDS_REQUESTED_INCOMPATIBLE_QOS_STATUS (1u << DDS_REQUESTED_INCOMPATIBLE_QOS_STATUS_ID)
/**
 * @anchor DDS_SAMPLE_LOST_STATUS
 * 丢失了一个样本（从未收到）。
 * A sample has been lost (never received). */
#define DDS_SAMPLE_LOST_STATUS (1u << DDS_SAMPLE_LOST_STATUS_ID)
/**
 * @anchor DDS_SAMPLE_REJECTED_STATUS
 * 拒绝了一个（已接收）样本。
 * A (received) sample has been rejected. */
#define DDS_SAMPLE_REJECTED_STATUS (1u << DDS_SAMPLE_REJECTED_STATUS_ID)
/**
 * @anchor DDS_DATA_ON_READERS_STATUS
 * 订阅者的一些数据读取器中有新信息。
 * New information is available in some of the data readers of a subscriber. */
#define DDS_DATA_ON_READERS_STATUS (1u << DDS_DATA_ON_READERS_STATUS_ID)
/**
 * @anchor DDS_DATA_AVAILABLE_STATUS
 * 数据读取器中有新信息。
 * New information is available in a data reader. */
#define DDS_DATA_AVAILABLE_STATUS (1u << DDS_DATA_AVAILABLE_STATUS_ID)
/**
 * @anchor DDS_LIVELINESS_LOST_STATUS
 * 未遵守 DDS_DataWriter 通过其活跃度 QoS 策略承诺的活跃度；因此，读者将认为写入器不再“活着”。
 * The liveliness that the DDS_DataWriter has committed through its liveliness QoS policy was not
 * respected; thus readers will consider the writer as no longer "alive". */
#define DDS_LIVELINESS_LOST_STATUS (1u << DDS_LIVELINESS_LOST_STATUS_ID)
/**
 * @anchor DDS_LIVELINESS_CHANGED_STATUS
 * 通过读者阅读的实例的一个或多个编写者的活跃度发生了变化。一些编写者已经变得“活着”或“不活着”。
 * The liveliness of one or more writers, that were writing instances read through the readers has
 * changed. Some writers have become "alive" or "not alive". */
#define DDS_LIVELINESS_CHANGED_STATUS (1u << DDS_LIVELINESS_CHANGED_STATUS_ID)
/**
 * @anchor DDS_PUBLICATION_MATCHED_STATUS
 * 编写者找到了匹配主题并具有兼容 QoS 的读者。
 * The writer has found a reader that matches the topic and has a compatible QoS. */
#define DDS_PUBLICATION_MATCHED_STATUS (1u << DDS_PUBLICATION_MATCHED_STATUS_ID)
/**
 * @anchor DDS_SUBSCRIPTION_MATCHED_STATUS
 * @brief 读者已找到与主题匹配且具有兼容QoS的写者。 (The reader has found a writer that matches the
 * topic and has a compatible QoS.)
 */
#define DDS_SUBSCRIPTION_MATCHED_STATUS (1u << DDS_SUBSCRIPTION_MATCHED_STATUS_ID)
// 使用位移操作符，将1左移DDS_SUBSCRIPTION_MATCHED_STATUS_ID位，用于表示订阅匹配状态。
// (Use the bit shift operator to shift 1 left by DDS_SUBSCRIPTION_MATCHED_STATUS_ID bits,
// representing the subscription matched status.)
/** @}*/  // 结束实体状态组。 (end group entity_status)

/**
 * @defgroup subscription (订阅)
 * @ingroup dds
 * DOC_TODO 这里包含了有关订阅数据的定义。
 */

/**
 * @defgroup subdata (数据访问)
 * @ingroup subscription
 * 从DDS读取的每个样本都带有一些元数据，您可以查看并过滤这些元数据。
 * @{
 */

/** 数据值的读取状态 */
typedef enum dds_sample_state {
  DDS_SST_READ = DDS_READ_SAMPLE_STATE,        /**<DataReader已通过read访问过该样本 */
  DDS_SST_NOT_READ = DDS_NOT_READ_SAMPLE_STATE /**<DataReader之前未访问过该样本 */
} dds_sample_state_t;

/** 实例相对于样本的视图状态 */
typedef enum dds_view_state {
  /** 当实例处于活动状态时，DataReader首次访问样本 */
  DDS_VST_NEW = DDS_NEW_VIEW_STATE,
  /** DataReader之前访问过该样本 */
  DDS_VST_OLD = DDS_NOT_NEW_VIEW_STATE
} dds_view_state_t;

/** 定义实例的状态 */
typedef enum dds_instance_state {
  /** 从活动的数据写入器接收到实例的样本 */
  DDS_IST_ALIVE = DDS_ALIVE_INSTANCE_STATE,
  /** 实例被数据写入器显式处理 */
  DDS_IST_NOT_ALIVE_DISPOSED = DDS_NOT_ALIVE_DISPOSED_INSTANCE_STATE,
  /** 数据读取器声明实例不活动，因为没有活动的数据写入器在写入该实例 */
  DDS_IST_NOT_ALIVE_NO_WRITERS = DDS_NOT_ALIVE_NO_WRITERS_INSTANCE_STATE
} dds_instance_state_t;

/** 包含关联数据值的信息 */
typedef struct dds_sample_info {
  /** 样本状态 */
  dds_sample_state_t sample_state;
  /** 视图状态 */
  dds_view_state_t view_state;
  /** 实例状态 */
  dds_instance_state_t instance_state;
  /** 表示是否有与样本关联的数据
   *  - true，表示数据有效
   *  - false，表示数据无效，没有可读取的数据 */
  bool valid_data;
  /** 数据实例写入时的时间戳 */
  dds_time_t source_timestamp;
  /** 数据实例的句柄 */
  dds_instance_handle_t instance_handle;
  /** 发布者的句柄 */
  dds_instance_handle_t publication_handle;
  /** 实例状态从NOT_ALIVE_DISPOSED变为ALIVE的计数 */
  uint32_t disposed_generation_count;
  /** 实例状态从NOT_ALIVE_NO_WRITERS变为ALIVE的计数 */
  uint32_t no_writers_generation_count;
  /** 集合中当前样本后面相同实例的样本数量 */
  uint32_t sample_rank;
  /** 返回集合中样本与相同实例最近样本之间的代差 */
  uint32_t generation_rank;
  /** read/take调用时样本与相同实例最近样本之间的代差 */
  uint32_t absolute_generation_rank;
} dds_sample_info_t;
/** @}*/  // 结束组 subdata

/**
 * @brief GUID在任何内置主题样本中的结构。
 * @ingroup builtintopic
 */
typedef struct dds_builtintopic_guid {
  uint8_t v[16]; /**< 16字节唯一标识符 */
} dds_builtintopic_guid_t;

/**
 * @brief GUID在任何内置主题样本中的结构。
 * @ingroup builtintopic
 * @ref dds_builtintopic_guid_t 对于大家通常称之为GUID的东西来说，这是一个有点奇怪的名字，
 * 所以让我们尝试使用更合逻辑的一个。
 */
typedef struct dds_builtintopic_guid dds_guid_t;

/**
 * @brief Builtin topic DcpsParticipant的样本结构。
 * @ingroup builtintopic
 */
typedef struct dds_builtintopic_participant {
  dds_guid_t key; /**< 网络上唯一标识参与者的GUID */
  dds_qos_t* qos; /**< 参与者的QoS */
} dds_builtintopic_participant_t;

/**
 * @brief 在 Builtin topic DcpsTopic 中的键结构。
 * @brief Structure of a key in the Builtin topic DcpsTopic.
 * @ingroup builtintopic
 */
typedef struct dds_builtintopic_topic_key {
  unsigned char d[16]; /**< 16字节唯一标识符 */
                       /**< 16-byte unique identifier */
} dds_builtintopic_topic_key_t;

/**
 * @brief Builtin topic DcpsTopic 的样本结构。
 * @brief Sample structure of the Builtin topic DcpsTopic.
 * @ingroup builtintopic
 */
typedef struct dds_builtintopic_topic {
  dds_builtintopic_topic_key_t key; /**< 网络上唯一标识主题的 GUID */
  /**< The GUID that uniquely identifies the topic on the network */
  char* topic_name; /**< 可能是 unicode 的主题名称。*/
                    /**< The name of the topic, potentially unicode. */
  char* type_name;  /**< 可能是 unicode 的类型名称。*/
                    /**< The name of the type, potentially unicode. */
  dds_qos_t* qos;   /**< 主题的 QoS */
                    /**< The QoS of the topic */
} dds_builtintopic_topic_t;

/**
 * @brief Builtin topic DcpsPublication 和 DcpsSubscription 的样本结构。
 * @brief Sample structure of the Builtin topic DcpsPublication and DcpsSubscription.
 * @ingroup builtintopic
 */
typedef struct dds_builtintopic_endpoint {
  dds_guid_t key;             /**< 网络上唯一标识端点的 GUID */
                              /**< The GUID that uniquely identifies the endpoint on the network */
  dds_guid_t participant_key; /**< 该端点所属参与者的 GUID。*/
                              /**< The GUID of the participant this endpoint belongs to. */
  dds_instance_handle_t participant_instance_handle; /**< 参与者分配给此端点的实例句柄。*/
                                                     /**< The instance handle the participant
                                                       assigned to this enpoint. */
  char* topic_name;                                  /**< 可能是 unicode 的主题名称。*/
  /**< The name of the topic, potentially unicode. */
  char* type_name; /**< 可能是 unicode 的类型名称。*/
                   /**< The name of the type, potentially unicode. */
  dds_qos_t* qos;  /**< 端点的 QoS */
                   /**< The QoS of the endpoint */
} dds_builtintopic_endpoint_t;

/**
 * @defgroup entity (实体)
 * @ingroup dds
 * @brief 库中的每个 DDS 对象都是一个实体。
 * @brief Every DDS object in the library is an Entity.
 * 所有实体都由进程私有句柄表示，其中一个调用可在创建时禁用实体。
 * An entity is created enabled by default.
 * 注意：目前不支持禁用创建。
 * Note: disabled creation is currently not supported.
 */

/**
 * @brief 启用实体。
 * @ingroup entity
 * @component generic_entity
 *
 * @note 目前尚不支持延迟实体启用 (CHAM-96)。
 *
 * 此操作启用 dds_entity_t。创建的 dds_entity_t 对象可以以启用或禁用状态开始。
 * This operation enables the dds_entity_t. Created dds_entity_t objects can start in
 * either an enabled or disabled state.
 * 这由给定实体的相应父实体上的 entityfactory 策略的值控制。
 * This is controlled by the value of the
 * entityfactory policy on the corresponding parent entity for the given
 * entity.
 * 启用的实体在创建时立即激活，这意味着它们的不可变 QoS 设置将无法再更改。
 * Enabled entities are immediately activated at creation time meaning
 * all their immutable QoS settings can no longer be changed.
 * 禁用的实体尚未激活，因此仍可以更改其不可变的 QoS 设置。
 * Disabled Entities are not
 * yet activated, so it is still possible to change their immutable QoS settings. However,
 * 一旦激活，不可变的 QoS 设置将无法再更改。
 * once activated the immutable QoS settings can no longer be changed.
 * 创建禁用实体是有意义的，当 DDS_Entity 的创建者还不知道要应用哪些 QoS 设置时，
 * thus allowing another piece of code
 * to set the QoS later on.
 *
 * 默认设置为 DDS_EntityFactoryQosPolicy，这样默认情况下，
 * entities are created in an enabled state so that it is not necessary to explicitly call
 * dds_enable on newly-created entities.
 *
 * dds_enable 操作无论执行多少次都会产生相同的结果。在已启用的 DDS_Entity 上调用 dds_enable 将返回
 * DDS_RETCODE_OK 并且没有任何影响。 The dds_enable operation produces the same results no matter
 * how many times it is performed. Calling dds_enable on an already enabled DDS_Entity returns
 * DDS_RETCODE_OK and has no effect.
 *
 * 如果实体尚未启用，可以对其调用的唯一操作是：设置、获取或复制 QosPolicy
 * 设置的操作，设置（或获取）侦听器的操作，获取状态的操作以及 dds_get_status_changes
 * 操作（尽管禁用实体的状态永远不会更改）。其他操作将返回错误 DDS_RETCODE_NOT_ENABLED。 If an Entity
 * has not yet been enabled, the only operations that can be invoked on it are: the ones to set, get
 * or copy the QosPolicy settings, the ones that set (or get) the Listener, the ones that get the
 * Status and the dds_get_status_changes operation (although the status of a disabled entity never
 * changes). Other operations will return the error DDS_RETCODE_NOT_ENABLED.
 *
 * 与禁用的父级创建的实体将无论 entityfactory 策略的设置如何都被创建为禁用。
 * Entities created with a parent that is disabled, are created disabled regardless of
 * the setting of the entityfactory policy.
 *
 * 如果 entityfactory 策略将 autoenable_created_entities 设置为 TRUE，则在父级上的 dds_enable
 * 操作将自动启用使用父级创建的所有子实体。 If the entityfactory policy has
 * autoenable_created_entities set to TRUE, the dds_enable operation on the parent will
 * automatically enable all child entities created with the parent.
 *
 * 与实体关联的侦听器在实体启用之前不会被调用。与未启用的实体关联的条件是“非活动”的，即触发值为
 * FALSE。 The Listeners associated with an Entity are not called until the Entity is enabled.
 * Conditions associated with an Entity that is not enabled are "inactive", that is, have a
 * trigger_value which is FALSE.
 *
 * @param[in]  entity  要启用的实体。
 * @param[in]  entity  The entity to enable.
 *
 * @returns 表示成功或失败的 dds_return_t。
 * @returns A dds_return_t indicating success or failure.
 *
 * @retval DDS_RETCODE_OK
 *             成功将侦听器复制到指定的侦听器参数中。
 *             The listeners of to the entity have been successfully been copied
 *             into the specified listener parameter.
 * @retval DDS_RETCODE_ERROR
 *             发生内部错误。
 *             An internal error has occurred.
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *             在不适当的对象上调用操作。
 *             The operation is invoked on an inappropriate object.
 * @retval DDS_RETCODE_ALREADY_DELETED
 *             实体已被删除。
 *             The entity has already been deleted.
 * @retval DDS_RETCODE_PRECONDITION_NOT_MET
 *             给定实体的父实体未启用。
 *             The parent of the given Entity is not enabled.
 */
DDS_EXPORT dds_return_t dds_enable(dds_entity_t entity);

/*
    所有实体都由进程私有句柄表示，通过一个调用删除实体及其逻辑包含的所有实体。
    也就是说，它相当于 DCPS API 中的 delete_contained_entities 和 delete_xxx 的组合。
*/
/**
 * @brief 删除给定实体。Delete the given entity.
 * @ingroup 实体 entity
 * @component 通用实体 generic_entity
 *
 * 此操作将删除给定实体。同时还会自动删除所有子实体、子实体的子实体等实体。
 * This operation will delete the given entity. It will also automatically
 * delete all its children, childrens' children, etc entities.
 *
 * @param[in]  entity  要删除的实体。Entity to delete.
 *
 * @returns 表示成功或失败的 dds_return_t。A dds_return_t indicating success or failure.
 *
 * @retval DDS_RETCODE_OK
 *             实体及其子实体（递归）已删除。The entity and its children (recursive are deleted).
 * @retval DDS_RETCODE_ERROR
 *             发生内部错误。An internal error has occurred.
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *             在不适当的对象上调用了操作。The operation is invoked on an inappropriate object.
 * @retval DDS_RETCODE_ALREADY_DELETED
 *             实体已被删除。The entity has already been deleted.
 */
/* TODO: 链接到通用 dds 实体关系文档。Link to generic dds entity relations documentation. */
DDS_EXPORT dds_return_t dds_delete(dds_entity_t entity);

/**
 * @brief 获取实体发布者。Get entity publisher.
 * @ingroup 实体 entity
 * @component 实体关系 entity_relations
 *
 * 此操作返回给定实体所属的发布者。
 * 例如，它将返回在创建 DataWriter 时使用的 Publisher（当这里提供了该 DataWriter）。
 * This operation returns the publisher to which the given entity belongs.
 * For instance, it will return the Publisher that was used when
 * creating a DataWriter (when that DataWriter was provided here).
 *
 * @param[in]  writer  从中获取其发布者的实体。Entity from which to get its publisher.
 *
 * @returns 有效实体或错误代码。A valid entity or an error code.
 *
 * @retval >0
 *             有效的发布者句柄。A valid publisher handle.
 * @retval DDS_RETCODE_ERROR
 *             发生内部错误。An internal error has occurred.
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *             在不适当的对象上调用了操作。The operation is invoked on an inappropriate object.
 * @retval DDS_RETCODE_ALREADY_DELETED
 *             实体已被删除。The entity has already been deleted.
 */
DDS_EXPORT dds_entity_t dds_get_publisher(dds_entity_t writer);

/**
 * @brief 获取实体订阅者。Get entity subscriber.
 * @ingroup 实体 entity
 * @component 实体关系 entity_relations
 *
 * 此操作返回给定实体所属的订阅者。
 * 例如，它将返回在创建 DataReader 时使用的 Subscriber（当这里提供了该 DataReader）。
 * This operation returns the subscriber to which the given entity belongs.
 * For instance, it will return the Subscriber that was used when
 * creating a DataReader (when that DataReader was provided here).
 *
 * @param[in]  entity  从中获取其订阅者的实体。Entity from which to get its subscriber.
 *
 * @returns 有效订阅者句柄或错误代码。A valid subscriber handle or an error code.
 *
 * @retval >0
 *             有效的订阅者句柄。A valid subscriber handle.
 * @retval DDS_RETCODE_ERROR
 *             发生内部错误。An internal error has occurred.
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *             在不适当的对象上调用了操作。The operation is invoked on an inappropriate object.
 * @retval DDS_RETCODE_ALREADY_DELETED
 *             实体已被删除。The entity has already been deleted.
 * DOC_TODO: 链接到通用 dds 实体关系文档。Link to generic dds entity relations documentation.
 */
DDS_EXPORT dds_entity_t dds_get_subscriber(dds_entity_t entity);

/**
 * @brief 获取实体数据读取器。Get entity datareader.
 * @ingroup 实体 entity
 * @component 实体关系 entity_relations
 *
 * 此操作返回给定实体所属的数据读取器。
 * 例如，它将返回在创建 ReadCondition 时使用的 DataReader（当这里提供了该 ReadCondition）。
 * This operation returns the datareader to which the given entity belongs.
 * For instance, it will return the DataReader that was used when
 * creating a ReadCondition (when that ReadCondition was provided here).
 *
 * @param[in]  entity  从中获取其数据读取器的实体。Entity from which to get its datareader.
 *
 * @returns 有效读取器句柄或错误代码。A valid reader handle or an error code.
 *
 * @retval >0
 *             有效的读取器句柄。A valid reader handle.
 * @retval DDS_RETCODE_ERROR
 *             发生内部错误。An internal error has occurred.
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *             在不适当的对象上调用了操作。The operation is invoked on an inappropriate object.
 * @retval DDS_RETCODE_ALREADY_DELETED
 *             实体已被删除。The entity has already been deleted.
 * DOC_TODO: 链接到通用 dds 实体关系文档。Link to generic dds entity relations documentation.
 */
DDS_EXPORT dds_entity_t dds_get_datareader(dds_entity_t entity);

/**
 * @defgroup condition (Conditions 条件)
 * @ingroup dds
 * @brief Conditions allow you to express conditional interest in samples,
 * to be used in read/take operations or attach to Waitsets.
 * @brief 条件允许您表达对样本的条件性兴趣，
 * 用于读取/获取操作或附加到 Waitsets。
 */

/**
 * @brief Get the mask of a condition. 获取条件的掩码。
 * @ingroup condition
 * @component entity_status
 *
 * This operation returns the mask that was used to create the given
 * condition.
 * 此操作返回用于创建给定条件的掩码。
 *
 * @param[in]  condition  Read or Query condition that has a mask. 具有掩码的读取或查询条件。
 * @param[out] mask       Where to store the mask of the condition. 存储条件掩码的位置。
 *
 * @returns A dds_return_t indicating success or failure. 表示成功或失败的 dds_return_t。
 *
 * @retval DDS_RETCODE_OK
 *             Success (given mask is set). 成功（设置了给定掩码）。
 * @retval DDS_RETCODE_ERROR
 *             An internal error has occurred. 发生内部错误。
 * @retval DDS_RETCODE_BAD_PARAMETER
 *             The mask arg is NULL. 掩码参数为 NULL。
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *             The operation is invoked on an inappropriate object. 操作在不适当的对象上调用。
 * @retval DDS_RETCODE_ALREADY_DELETED
 *             The entity has already been deleted. 实体已被删除。
 */
DDS_EXPORT dds_return_t dds_get_mask(dds_entity_t condition, uint32_t* mask);

/**
 * @brief Returns the instance handle that represents the entity. 返回表示实体的实例句柄。
 * @ingroup entity
 * @component generic_entity
 *
 * @param[in]   entity  Entity of which to get the instance handle. 要获取实例句柄的实体。
 * @param[out]  ihdl    Pointer to dds_instance_handle_t. 指向 dds_instance_handle_t 的指针。
 *
 * @returns A dds_return_t indicating success or failure. 表示成功或失败的 dds_return_t。
 *
 * @retval DDS_RETCODE_OK
 *             Success. 成功。
 * @retval DDS_RETCODE_ERROR
 *             An internal error has occurred. 发生内部错误。
 * DOC_TODO: Check list of return codes is complete. 检查返回代码列表是否完整。
 * */
DDS_EXPORT dds_return_t dds_get_instance_handle(dds_entity_t entity, dds_instance_handle_t* ihdl);

/**
 * @brief Returns the GUID that represents the entity in the network,
 * and therefore only supports participants, readers and writers.
 * @brief 返回在网络中表示实体的 GUID，
 * 因此仅支持参与者、读取器和写入器。
 * @ingroup entity
 * @component generic_entity
 *
 * @param[in]   entity  Entity of which to get the instance handle. 要获取实例句柄的实体。
 * @param[out]  guid    Where to store the GUID. 存储 GUID 的位置。
 *
 * @returns A dds_return_t indicating success or failure. 表示成功或失败的 dds_return_t。
 *
 * @retval DDS_RETCODE_OK
 *             Success. 成功。
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *             The operation is invoked on an inappropriate object. 操作在不适当的对象上调用。
 * @retval DDS_RETCODE_ERROR
 *             An internal error has occurred. 发生内部错误。
 *
 * DOC_TODO: Check list of return codes is complete. 检查返回代码列表是否完整。
 */
DDS_EXPORT dds_return_t dds_get_guid(dds_entity_t entity, dds_guid_t* guid);

/**
 * @brief Read the status set for the entity 读取实体的状态集
 * @ingroup entity_status
 * @component entity_status
 *
 * This operation reads the status(es) set for the entity based on
 * the enabled status and mask set. It does not clear the read status(es).
 * 此操作基于启用的状态和掩码集读取实体的状态集。它不会清除已读状态。
 *
 * @param[in]  entity  Entity on which the status has to be read. 必须读取状态的实体。
 * @param[out] status  Returns the status set on the entity, based on the enabled status.
 * 返回基于启用状态的实体上设置的状态。
 * @param[in]  mask    Filter the status condition to be read, 0 means all statuses
 * 要读取的状态条件过滤器，0 表示所有状态
 *
 * @returns A dds_return_t indicating success or failure. 表示成功或失败的 dds_return_t。
 *
 * @retval DDS_RETCODE_OK
 *             Success. 成功。
 * @retval DDS_RETCODE_BAD_PARAMETER
 *             The entity parameter is not a valid parameter, status is a null pointer or
 *             mask has bits set outside the status range.
 *             实体参数无效，状态为空指针或
 *             掩码在状态范围之外设置了位。
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *             The operation is invoked on an inappropriate object or mask has status
 *             bits set that are undefined for the type of entity.
 *             操作在不适当的对象上调用或掩码具有状态
 *             对于实体类型未定义的位集。
 * @retval DDS_RETCODE_ALREADY_DELETED
 *             The entity has already been deleted. 实体已被删除。
 */
DDS_EXPORT dds_return_t dds_read_status(dds_entity_t entity, uint32_t* status, uint32_t mask);

/**
 * @brief Read the status set for the entity 读取实体的状态集
 * @ingroup entity_status
 * @component entity_status
 *
 * This operation reads the status(es) set for the entity based on the enabled
 * status and mask set. It clears the status set after reading.
 * 此操作基于启用的状态和掩码集读取实体的状态集。读取后清除状态集。
 *
 * @param[in]  entity  Entity on which the status has to be read. 必须读取状态的实体。
 * @param[out] status  Returns the status set on the entity, based on the enabled status.
 * 返回基于启用状态的实体上设置的状态。
 * @param[in]  mask    Filter the status condition to be read, 0 means all statuses
 * 要读取的状态条件过滤器，0 表示所有状态
 *
 * @returns A dds_return_t indicating success or failure. 表示成功或失败的 dds_return_t。
 *
 * @retval DDS_RETCODE_OK
 *             Success. 成功。
 * @retval DDS_RETCODE_BAD_PARAMETER
 *             The entity parameter is not a valid parameter, status is a null pointer or
 *             mask has bits set outside the status range.
 *             实体参数无效，状态为空指针或
 *             掩码在状态范围之外设置了位。
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *             The operation is invoked on an inappropriate object or mask has status
 *             bits set that are undefined for the type of entity.
 *             操作在不适当的对象上调用或掩码具有状态
 *             对于实体类型未定义的位集。
 * @retval DDS_RETCODE_ALREADY_DELETED
 *             The entity has already been deleted. 实体已删除
 */
DDS_EXPORT dds_return_t dds_take_status(dds_entity_t entity, uint32_t* status, uint32_t mask);

/**
 * @brief 获取更改后的状态
 * @ingroup entity_status
 * @component entity_status
 *
 * 此操作返回自上次读取以来的状态更改。
 *
 * @param[in]  entity  读取状态的实体。
 * @param[out] status  返回触发的状态集。
 *
 * @returns 表示成功或失败的 dds_return_t。
 *
 * @retval DDS_RETCODE_OK
 *             成功。
 * @retval DDS_RETCODE_BAD_PARAMETER
 *             实体参数无效。
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *             在不适当的对象上调用操作。
 * @retval DDS_RETCODE_ALREADY_DELETED
 *             实体已被删除。
 */
DDS_EXPORT dds_return_t dds_get_status_changes(dds_entity_t entity, uint32_t* status);

/**
 * @anchor dds_get_status_mask
 * @brief 获取实体上启用的状态
 * @ingroup entity_status
 * @component entity_status
 *
 * 此操作返回实体上启用的状态
 *
 * @param[in]  entity  获取状态的实体。
 * @param[out] mask    实体上设置的启用状态掩码。
 *
 * @returns 表示成功或失败的 dds_return_t。
 *
 * @retval DDS_RETCODE_OK
 *             成功。
 * @retval DDS_RETCODE_BAD_PARAMETER
 *             实体参数无效。
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *             在不适当的对象上调用操作。
 * @retval DDS_RETCODE_ALREADY_DELETED
 *             实体已被删除。
 */
DDS_EXPORT dds_return_t dds_get_status_mask(dds_entity_t entity, uint32_t* mask);

/**
 * @anchor dds_set_status_mask
 * @brief 设置实体上启用的状态
 * @ingroup entity_status
 * @component entity_status
 *
 * 此操作根据设置的掩码启用状态
 *
 * @param[in]  entity  启用状态的实体。
 * @param[in]  mask    指示要启用的状态的状态值。
 *
 * @returns 表示成功或失败的 dds_return_t。
 *
 * @retval DDS_RETCODE_OK
 *             成功。
 * @retval DDS_RETCODE_BAD_PARAMETER
 *             实体参数无效。
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *             在不适当的对象上调用操作。
 * @retval DDS_RETCODE_ALREADY_DELETED
 *             实体已被删除。
 */
DDS_EXPORT dds_return_t dds_set_status_mask(dds_entity_t entity, uint32_t mask);

/**
 * @defgroup entity_qos (实体 QoS)
 * @ingroup entity
 * @brief 几乎所有实体都在其上定义了 get/set qos 操作，
 * 再次遵循 DCPS 规范。但与 DCPS 规范不同，qos_t 中的
 * "present" 字段允许初始化一个要设置的 QoS
 * 并将其传递给 set_qos。
 */

/**
 * @brief 获取实体 QoS 策略。
 * @ingroup entity_qos
 * @component entity_qos
 *
 * 此操作允许访问实体的现有 QoS 策略集。
 *
 * @param[in]  entity  要获取 qos 的实体。
 * @param[out] qos     返回设置策略的 qos 结构指针。
 *
 * @returns 表示成功或失败的 dds_return_t。QoS 对象将至少具有
 * 实体相关的所有 QoS，并且相应的 dds_qget_... 将返回 true。
 *
 * @retval DDS_RETCODE_OK
 *             成功将实体上应用的现有 QoS 策略值集复制到指定的 qos 参数中。
 * @retval DDS_RETCODE_ERROR
 *             发生内部错误。
 * @retval DDS_RETCODE_BAD_PARAMETER
 *             qos 参数为 NULL。
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *             在不适当的对象上调用操作。
 * @retval DDS_RETCODE_ALREADY_DELETED
 *             实体已被删除。
 *
 * DOC_TODO: 链接到通用 QoS 信息文档。
 */
DDS_EXPORT dds_return_t dds_get_qos(dds_entity_t entity, dds_qos_t* qos);

/**
 * @brief 设置实体QoS策略。
 * @ingroup entity_qos
 * @component entity_qos
 *
 * 此操作替换实体的现有Qos策略设置集。参数qos必须包含经过自我一致性检查的Qos策略设置结构。
 * @brief Set entity QoS policies.
 * @ingroup entity_qos
 * @component entity_qos
 *
 * This operation replaces the existing set of Qos Policy settings for an
 * entity. The parameter qos must contain the struct with the QosPolicy
 * settings which is checked for self-consistency.
 *
 * qos参数指定的Qos策略设置集将应用于现有QoS之上，替换先前设置的任何策略的值（前提是操作返回DDS_RETCODE_OK）。
 * The set of QosPolicy settings specified by the qos parameter are applied on
 * top of the existing QoS, replacing the values of any policies previously set
 * (provided, the operation returned DDS_RETCODE_OK).
 *
 * 实体启用时，并非所有策略都可以更改。
 * Not all policies are changeable when the entity is enabled.
 *
 * @note 目前只有延迟预算和所有权强度是可更改的QoS，可以设置。
 * @note Currently only Latency Budget and Ownership Strength are changeable QoS
 *       that can be set.
 *
 * @param[in]  entity  要获取qos的实体。
 * @param[in]  qos     提供策略的qos结构的指针。
 * @param[in]  entity  Entity from which to get qos.
 * @param[in]  qos     Pointer to the qos structure that provides the policies.
 *
 * @returns 表示成功或失败的dds_return_t。
 * @returns A dds_return_t indicating success or failure.
 *
 * @retval DDS_RETCODE_OK
 *             新的QoS策略已设置。
 * @retval DDS_RETCODE_OK
 *             The new QoS policies are set.
 * @retval DDS_RETCODE_ERROR
 *             发生内部错误。
 * @retval DDS_RETCODE_ERROR
 *             An internal error has occurred.
 * @retval DDS_RETCODE_BAD_PARAMETER
 *             qos参数为NULL。
 * @retval DDS_RETCODE_BAD_PARAMETER
 *             The qos parameter is NULL.
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *             在不适当的对象上调用操作。
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *             The operation is invoked on an inappropriate object.
 * @retval DDS_RETCODE_ALREADY_DELETED
 *             实体已被删除。
 * @retval DDS_RETCODE_ALREADY_DELETED
 *             The entity has already been deleted.
 * @retval DDS_RETCODE_IMMUTABLE_POLICY
 *             实体已启用，QoS的一个或多个策略是不可变的。
 * @retval DDS_RETCODE_IMMUTABLE_POLICY
 *             The entity is enabled and one or more of the policies of the QoS
 *             are immutable.
 * @retval DDS_RETCODE_INCONSISTENT_POLICY
 *             QoS中的一些策略彼此不一致。
 * @retval DDS_RETCODE_INCONSISTENT_POLICY
 *             A few policies within the QoS are not consistent with each other.
 *
 * DOC_TODO: 链接到通用QoS信息文档。
 * DOC_TODO: Link to generic QoS information documentation.
 */
DDS_EXPORT dds_return_t dds_set_qos(dds_entity_t entity, const dds_qos_t* qos);

/**
 * @defgroup entity_listener (实体监听器)
 * @ingroup entity
 * @brief 获取或设置与实体关联的监听器，提供的监听器类型必须与实体类型匹配。
 * @defgroup entity_listener (Entity Listener)
 * @ingroup entity
 * @brief Get or set listener associated with an entity,
 * type of listener provided much match type of entity.
 */

/**
 * @brief 获取实体监听器。
 * @ingroup entity_listener
 * @component entity_listener
 *
 * 此操作允许访问附加到实体的现有监听器。
 * @brief Get entity listeners.
 * @ingroup entity_listener
 * @component entity_listener
 *
 * This operation allows access to the existing listeners attached to
 * the entity.
 *
 * @param[in]  entity   要获取监听器的实体。
 * @param[out] listener 返回监听器回调集的监听器结构指针。
 * @param[in]  entity   Entity on which to get the listeners.
 * @param[out] listener Pointer to the listener structure that returns the
 *                      set of listener callbacks.
 *
 * @returns 表示成功或失败的dds_return_t。
 * @returns A dds_return_t indicating success or failure.
 *
 * @retval DDS_RETCODE_OK
 *             实体的监听器已成功复制到指定的监听器参数中。
 * @retval DDS_RETCODE_OK
 *             The listeners of to the entity have been successfully been
 *             copied into the specified listener parameter.
 * @retval DDS_RETCODE_ERROR
 *             发生内部错误。
 * @retval DDS_RETCODE_ERROR
 *             An internal error has occurred.
 * @retval DDS_RETCODE_BAD_PARAMETER
 *             监听器参数为NULL。
 * @retval DDS_RETCODE_BAD_PARAMETER
 *             The listener parameter is NULL.
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *             在不适当的对象上调用操作。
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *             The operation is invoked on an inappropriate object.
 * @retval DDS_RETCODE_ALREADY_DELETED
 *             实体已被删除。
 * @retval DDS_RETCODE_ALREADY_DELETED
 *             The entity has already been deleted.
 *
 * DOC_TODO: 链接到（通用）监听器和状态信息。
 * DOC_TODO: Link to (generic) Listener and status information.
 */
DDS_EXPORT dds_return_t dds_get_listener(dds_entity_t entity, dds_listener_t* listener);

/**
 * @brief 设置实体监听器。
 * @ingroup entity_listener
 * @component entity_listener
 *
 * 此操作将 dds_listener_t 附加到 dds_entity_t。每个实体只能附加一个
 * 监听器。如果已经附加了一个监听器，此操作将用新的监听器替换它。换句话说，
 * 所有相关的回调都被替换（可能为 NULL）。
 *
 * Set entity listeners.
 * This operation attaches a dds_listener_t to the dds_entity_t. Only one
 * Listener can be attached to each Entity. If a Listener was already
 * attached, this operation will replace it with the new one. In other
 * words, all related callbacks are replaced (possibly with NULL).
 *
 * 当 listener 参数为 NULL 时，将删除实体上可能设置的所有监听器回调。
 *
 * When listener parameter is NULL, all listener callbacks that were possibly
 * set on the Entity will be removed.
 *
 * @note 并非所有的监听器回调都与所有实体相关。
 *
 * @note Not all listener callbacks are related to all entities.
 *
 * ## 通信状态
 * 对于每个通信状态，StatusChangedFlag 标志最初设置为
 * FALSE。当该纯通信状态发生变化时，它变为 TRUE。对于在掩码中激活的每个纯通信状态，
 * 调用关联的监听器回调，并将通信状态重置
 * 为 FALSE，因为监听器隐式访问作为参数传递给该操作的状态。
 * 在调用监听器之前重置状态，因此如果应用程序从监听器内部调用
 * get_<status_name>，它将看到状态已经重置。
 *
 * ## Communication Status
 * For each communication status, the StatusChangedFlag flag is initially set to
 * FALSE. It becomes TRUE whenever that plain communication status changes. For
 * each plain communication status activated in the mask, the associated
 * Listener callback is invoked and the communication status is reset
 * to FALSE, as the listener implicitly accesses the status which is passed as a
 * parameter to that operation.
 * The status is reset prior to calling the listener, so if the application calls
 * the get_<status_name> from inside the listener it will see the
 * status already reset.
 *
 * ## 状态传播
 * 如果监听器中的相关回调未设置，则递归调用父实体的监听器，直到找到并调用已设置适当回调的监听器。这允许应用程序在包含的发布者的监听器中设置（例如）默认行为，并在需要时设置
 * DataWriter
 * 特定行为。如果发布者的监听器中也没有设置回调，则通信状态将传播到包含的域参与者的域参与者的监听器。如果域参与者的监听器中也没有设置回调，则将设置通信状态标志，从而可能触发
 * WaitSet。
 *
 * ## Status Propagation
 * In case a related callback within the Listener is not set, the Listener of
 * the Parent entity is called recursively, until a Listener with the appropriate
 * callback set has been found and called. This allows the application to set
 * (for instance) a default behaviour in the Listener of the containing Publisher
 * and a DataWriter specific behaviour when needed. In case the callback is not
 * set in the Publishers' Listener either, the communication status will be
 * propagated to the Listener of the DomainParticipant of the containing
 * DomainParticipant. In case the callback is not set in the DomainParticipants'
 * Listener either, the Communication Status flag will be set, resulting in a
 * possible WaitSet trigger.
 *
 * @param[in]  entity    要获取监听器的实体。
 * @param[in]  listener  包含一组监听器回调（可能为 NULL）的监听器结构的指针。
 *
 * @returns 表示成功或失败的 dds_return_t。
 *
 * @retval DDS_RETCODE_OK
 *             实体的监听器已成功复制到指定的监听器参数中。
 * @retval DDS_RETCODE_ERROR
 *             发生内部错误。
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *             在不适当的对象上调用操作。
 * @retval DDS_RETCODE_ALREADY_DELETED
 *             实体已被删除。
 * DOC_TODO: 链接到（通用）监听器和状态信息。
 *
 * @param[in]  entity    Entity on which to get the listeners.
 * @param[in]  listener  Pointer to the listener structure that contains the
 *                       set of listener callbacks (maybe NULL).
 *
 * @returns A dds_return_t indicating success or failure.
 *
 * @retval DDS_RETCODE_OK
 *             The listeners of to the entity have been successfully been
 *             copied into the specified listener parameter.
 * @retval DDS_RETCODE_ERROR
 *             An internal error has occurred.
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *             The operation is invoked on an inappropriate object.
 * @retval DDS_RETCODE_ALREADY_DELETED
 *             The entity has already been deleted.
 * DOC_TODO: Link to (generic) Listener and status information.
 */
DDS_EXPORT dds_return_t dds_set_listener(dds_entity_t entity, const dds_listener_t* listener);

/*
    创建各种实体的函数。创建订阅者或发布者是可选的：如果将读取器作为参与者的后代创建，
    就好像为该读取器专门创建了一个订阅者。

    QoS 默认值是 DDS 规范的默认值，但继承规则不同：

      * 发布者和订阅者从参与者 QoS 继承
      * 读取器和写入器始终从主题 QoS 继承
      * "qos" 参数中的 QoS 覆盖继承的值
  */

/**
 * @defgroup domain (域)
 * @ingroup DDS
 */

/**
 * @defgroup domain_participant (域参与者)
 * @ingroup domain
 */

/**
   * @brief 在域中创建一个新的 DDS 参与者实例
   * @ingroup domain_participant
   * @component participant
   *
   * 如果设置了域（不是 DDS_DOMAIN_DEFAULT），那么它必须匹配，否则将返回错误状态。
   * 目前只能通过提供配置文件来配置单个域。
   * 如果没有配置文件，将配置默认域为 0。
   *
   *
   * @param[in]  domain 要在其中创建参与者的域（可以是 DDS_DOMAIN_DEFAULT）。
   DDS_DOMAIN_DEFAULT 用于使用配置中的域。
   * @param[in]  qos 要设置在新参与者上的 QoS（可以为 NULL）。
   * @param[in]  listener 与新参与者关联的任何监听器函数（可以为 NULL）。

   * @returns 有效的参与者句柄或错误代码。
   *
   * @retval >0
   *             有效的参与者句柄。
   * @retval DDS_RETCODE_NOT_ALLOWED_BY_SECURITY
   *             指定了无效的 DDS 安全配置（无论
   *             是缺少还是不正确的条目、过期证书，
   *             或与安全设置和
   *             实现相关的其他内容）。
   * @retval DDS_RETCODE_PRECONDITION_NOT_MET
   *             QoS 中指定了一些安全属性，但 Cyclone
   *             构建不包括对 DDS 安全的支持。
   * @retval DDS_RETCODE_OUT_OF_RESOURCES
   *             某些资源限制（最大参与者、内存、句柄、
   *             等等。）阻止了参与者的创建。
   * @retval DDS_RETCODE_ERROR
   *             "CYCLONEDDS_URI" 环境变量列出了不存在的
   *             或无效的配置文件，或包含无效的嵌入式
   *             配置项；或发生了未指定的内部错误。
   */
DDS_EXPORT dds_entity_t dds_create_participant(const dds_domainid_t domain,
                                               const dds_qos_t* qos,
                                               const dds_listener_t* listener);

/**
 * @brief 使用给定配置创建域
 * @ingroup domain
 * @component domain
 *
 * 显式基于作为字符串传递的配置创建域。
 *
 * 如果已经存在具有给定域 ID 的域，则不会创建它。
 * 这可能是由 dds_create_participant() 隐式创建的。
 *
 * 请注意，给定的 domain_id 总是优先于
 * 配置。
 *
 * | domain_id | domain id in config | result                        |
 * |:----------|:--------------------|:------------------------------|
 * | n         | any (or absent)     | n, config is used             |
 * | n         | m == n              | n, config is used             |
 * | n         | m != n              | n, config is ignored: default |
 *
 * 配置模型：
 *  -# @code{xml}
 *     <CycloneDDS>
 *        <Domain id="X">...</Domain>
 *        <!-- <Domain .../> -->
 *      </CycloneDDS>
 *      @endcode
 *      其中 ... 是今天可以在 CycloneDDS 的子级中设置的所有内容
 *      除了 id
 *  -# @code{xml}
 *     <CycloneDDS>
 *        <Domain><Id>X</Id></Domain>
 *        <!-- more things here ... -->
 *     </CycloneDDS>
 *     @endcode
 *     旧形式，域 ID 必须是文件中第一个具有
 *     值的元素（如果以前没有设置过任何内容，警告就足够了
 *     良好）
 *
 * 使用 NULL 或 "" 作为配置将创建具有默认设置的域。
 *
 *
 * @param[in]  domain 要创建的域。DEFAULT_DOMAIN 不允许。
 * @param[in]  config 包含表示配置的文件名和/或 XML 片段的配置字符串。
 *
 * @returns 有效的实体句柄或错误代码。
 *
 * @retval DDS_RETCODE_BAD_PARAMETER
 *             域 id 或 configfile 参数的值非法。
 * @retval DDS_RETCODE_PRECONDITION_NOT_MET
 *             域已经存在，不能再次创建。
 * @retval DDS_RETCODE_ERROR
 *             发生了内部错误。
 */
DDS_EXPORT dds_entity_t dds_create_domain(const dds_domainid_t domain, const char* config);

/**
 * @brief 创建具有给定配置的域，该配置以初始化器形式指定（不稳定接口）
 *       (Creates a domain with a given configuration, specified as an
 *        initializer (unstable interface))
 * @ingroup domain
 * @component domain
 * @unstable
 *
 * 为了基于作为原始初始化器传递的配置显式创建域，而不是作为XML字符串。这允许绕过XML解析，
 * 但将初始化紧密耦合到实现中。请参阅dds/ddsi/ddsi_config.h:ddsi_config_init_default，
 * 了解如何初始化默认配置。
 * (To explicitly create a domain based on a configuration passed as a raw
 *  initializer rather than as an XML string. This allows bypassing the XML
 *  parsing, but tightly couples the initializing to implementation.  See
 *  dds/ddsi/ddsi_config.h:ddsi_config_init_default for a way to initialize
 *  the default configuration.)
 *
 * 如果已经存在具有给定域ID的域，则不会创建它。
 * 这可能是由dds_create_participant()隐式创建的。
 * (It will not be created if a domain with the given domain id already exists.
 *  This could have been created implicitly by a dds_create_participant().)
 *
 * 请注意，给定的domain_id始终优先于配置。
 * (Please be aware that the given domain_id always takes precedence over the
 *  configuration.)
 *
 * @param[in]  domain 要创建的域。DEFAULT_DOMAIN是不允许的。
 *            (The domain to be created. DEFAULT_DOMAIN is not allowed.)
 * @param[in]  config 配置初始化器。config中的任何指针的生命周期必须至少与域的生命周期相同。
 *            (A configuration initializer.  The lifetime of any pointers
 *             in config must be at least that of the lifetime of the domain.)
 *
 * @returns 有效的实体句柄或错误代码。
 *          (A valid entity handle or an error code.)
 *
 * @retval DDS_RETCODE_BAD_PARAMETER
 *             域ID的非法值或config参数为NULL。
 *             (Illegal value for domain id or the config parameter is NULL.)
 * @retval DDS_RETCODE_PRECONDITION_NOT_MET
 *             该域已经存在，无法再次创建。
 *             (The domain already existed and cannot be created again.)
 * @retval DDS_RETCODE_ERROR
 *             发生内部错误。
 *             (An internal error has occurred.)
 */
DDS_EXPORT dds_entity_t dds_create_domain_with_rawconfig(const dds_domainid_t domain,
                                                         const struct ddsi_config* config);

/**
 * @brief 获取实体的父对象 (Get entity parent).
 * @ingroup 实体组 (entity)
 * @component 实体关系组件 (entity_relations)
 *
 * 此操作返回给定实体所属的父对象。
 * 例如，当创建一个发布者时，它将返回用于创建该发布者的参与者（当提供该发布者时）。
 * (This operation returns the parent to which the given entity belongs.
 * For instance, it will return the Participant that was used when
 * creating a Publisher (when that Publisher was provided here).)
 *
 * 当使用参与者创建读取器或写入器时，订阅者或发布者会隐式创建。
 * 此函数将返回隐式父对象而不是使用的参与者。
 * (When a reader or a writer are created with a participant, then a
 * subscriber or publisher are created implicitly.
 * This function will return the implicit parent and not the used
 * participant.)
 *
 * @param[in]  entity  要获取其父对象的实体 (Entity from which to get its parent).
 *
 * @returns 有效的实体句柄或错误代码 (A valid entity handle or an error code).
 *
 * @retval >0
 *             有效的实体句柄 (A valid entity handle).
 * @retval DDS_ENTITY_NIL
 *             使用参与者调用 (Called with a participant).
 * @retval DDS_RETCODE_ERROR
 *             发生内部错误 (An internal error has occurred).
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *             在不适当的对象上调用操作 (The operation is invoked on an inappropriate object).
 * @retval DDS_RETCODE_ALREADY_DELETED
 *             实体已被删除 (The entity has already been deleted).
 * DOC_TODO: 链接到通用dds实体关系文档 (Link to generic dds entity relations documentation).
 */
DDS_EXPORT dds_entity_t dds_get_parent(dds_entity_t entity);

/**
 * @brief 获取实体参与者。
 * @ingroup entity
 * @component entity_relations
 *
 * 此操作返回给定实体所属的参与者。
 * 例如，当使用创建 DataWriter 的 Publisher 时，它将返回在创建该 Publisher 时使用的
 * Participant（当提供了该 DataWriter 时）。
 *
 * DOC_TODO: 链接到通用 dds 实体关系文档。
 *
 * @param[in]  entity  要获取其参与者的实体。
 *
 * @returns 有效实体或错误代码。
 *
 * @retval >0
 *             有效的参与者句柄。
 * @retval DDS_RETCODE_ERROR
 *             发生内部错误。
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *             在不适当的对象上调用操作。
 * @retval DDS_RETCODE_ALREADY_DELETED
 *             实体已被删除。
 */
/**
 * @brief Get entity participant.
 * @ingroup entity
 * @component entity_relations
 *
 * This operation returns the participant to which the given entity belongs.
 * For instance, it will return the Participant that was used when
 * creating a Publisher that was used to create a DataWriter (when that
 * DataWriter was provided here).
 *
 * DOC_TODO: Link to generic dds entity relations documentation.
 *
 * @param[in]  entity  Entity from which to get its participant.
 *
 * @returns A valid entity or an error code.
 *
 * @retval >0
 *             A valid participant handle.
 * @retval DDS_RETCODE_ERROR
 *             An internal error has occurred.
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *             The operation is invoked on an inappropriate object.
 * @retval DDS_RETCODE_ALREADY_DELETED
 *             The entity has already been deleted.
 */
DDS_EXPORT dds_entity_t dds_get_participant(dds_entity_t entity);

/**
 * @brief 获取实体的子对象.
 * @ingroup entity
 * @component entity_relations
 *
 * 此操作返回实体包含的子对象.
 * 例如，它将返回用于创建这些实体的参与者（当提供该参与者时）的所有主题、发布者和订阅者.
 * (This operation returns the children that the entity contains.
 * For instance, it will return all the Topics, Publishers and Subscribers
 * of the Participant that was used to create those entities (when that
 * Participant is provided here).)
 *
 * 此函数接受一个预分配的列表以放入子对象，并返回找到的子对象数量.
 * 可能给定的列表大小与找到的子对象数量不同. 如果 找到的子对象较少，则列表中的最后几个条目保持不变.
 * 当找到更多的子对象时，仍然只有 'size' 数量的条目插入到列表中， 但仍返回找到的子对象的完整计数.
 * 在后一种情况下返回哪些子对象是未定义的. (This functions takes a pre-allocated list to put the
 * children in and will return the number of found children. It is possible that the given size of
 * the list is not the same as the number of found children. If less children are found, then the
 * last few entries in the list are untouched. When more children are found, then only 'size' number
 * of entries are inserted into the list, but still complete count of the found children is
 * returned. Which children are returned in the latter case is undefined.)
 *
 * 当提供 NULL 作为列表和 0 作为大小时，您可以使用此方法获取子对象的数量而无需预先分配列表.
 * (When supplying NULL as list and 0 as size, you can use this to acquire
 * the number of children without having to pre-allocate a list.)
 *
 * 当使用参与者创建读取器或写入器时，订阅者或发布者将隐式创建.
 * 在参与者上使用此函数时，它将返回隐式订阅者和/或发布者，而不是相关的读取器/写入器.
 * (When a reader or a writer are created with a participant, then a
 * subscriber or publisher are created implicitly.
 * When used on the participant, this function will return the implicit
 * subscriber and/or publisher and not the related reader/writer.)
 *
 * @param[in]  entity   从中获取其子对象的实体.
 * @param[out] children 包含找到的子对象的预分配数组.
 * @param[in]  size     预分配子对象列表的大小.
 * ( @param[in]  entity   Entity from which to get its children.
 *   @param[out] children Pre-allocated array to contain the found children.
 *   @param[in]  size     Size of the pre-allocated children's list.)
 *
 * @returns 子对象的数量或错误代码.
 * (@returns Number of children or an error code.)
 *
 * @retval >=0
 *             找到的子对象数量（可能大于 'size'）.
 * @retval DDS_RETCODE_ERROR
 *             发生内部错误.
 * @retval DDS_RETCODE_BAD_PARAMETER
 *             children 参数为 NULL，同时提供了大小.
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *             在不适当的对象上调用操作.
 * @retval DDS_RETCODE_ALREADY_DELETED
 *             实体已被删除.
 * ( @retval >=0
 *             Number of found children (can be larger than 'size').
 *   @retval DDS_RETCODE_ERROR
 *             An internal error has occurred.
 *   @retval DDS_RETCODE_BAD_PARAMETER
 *             The children parameter is NULL, while a size is provided.
 *   @retval DDS_RETCODE_ILLEGAL_OPERATION
 *             The operation is invoked on an inappropriate object.
 *   @retval DDS_RETCODE_ALREADY_DELETED
 *             The entity has already been deleted.)
 */
/* TODO: 链接到通用 dds 实体关系文档. */
/* TODO: Link to generic dds entity relations documentation. */
DDS_EXPORT dds_return_t dds_get_children(dds_entity_t entity, dds_entity_t* children, size_t size);

/**
 * @brief 获取实体所附加的域ID。 (Get the domain id to which this entity is attached.)
 * @ingroup 实体 (entity)
 * @component 实体关系 (entity_relations)
 *
 * 在创建参与者实体时，它将附加到某个域。 (When creating a participant entity, it is attached to a
 * certain domain.) 所有子级（如发布者）和子级的子级（如数据读取器）等也附加到该域。 (All the
 * children (like Publishers) and childrens' children (like DataReaders), etc are also attached to
 * that domain.)
 *
 * 当在该层次结构中的任何实体上调用此函数时，将返回原始域ID。 (This function will return the
 * original domain ID when called on any of the entities within that hierarchy.)
 * 对于未与域关联的实体，id 设置为 DDS_DOMAIN_DEFAULT。 (For entities not associated with a domain,
 * the id is set to DDS_DOMAIN_DEFAULT.)
 *
 * @param[in]  entity   从中获取其子级的实体。 (Entity from which to get its children.)
 * @param[out] id       放置域ID的指针。 (Pointer to put the domain ID in.)
 *
 * @returns 表示成功或失败的 dds_return_t。 (A dds_return_t indicating success or failure.)
 *
 * @retval DDS_RETCODE_OK
 *             返回了域ID。 (Domain ID was returned.)
 * @retval DDS_RETCODE_ERROR
 *             发生了内部错误。 (An internal error has occurred.)
 * @retval DDS_RETCODE_BAD_PARAMETER
 *             id 参数为 NULL。 (The id parameter is NULL.)
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *             在不适当的对象上调用了操作。 (The operation is invoked on an inappropriate object.)
 * @retval DDS_RETCODE_ALREADY_DELETED
 *             该实体已被删除。 (The entity has already been deleted.)
 */
DDS_EXPORT dds_return_t dds_get_domainid(dds_entity_t entity, dds_domainid_t* id);

/**
 * @brief 获取域中的参与者。
 * @ingroup domain
 * @component participant
 *
 * 此操作获取在域上创建的参与者，并返回找到的参与者数量。
 *
 * Get participants of a domain.
 * This operation acquires the participants created on a domain and returns
 * the number of found participants.
 *
 * 该函数接受一个带有预分配参与者列表大小的域ID，将返回找到的参与者数量。可能给定的列表大小与找到的参与者数量不同。
 * 如果找到的参与者较少，则数组中的最后几个条目保持不变。如果找到更多的参与者并且数组太小，则返回的参与者是未定义的。
 *
 * This function takes a domain id with the size of pre-allocated participant's
 * list in and will return the number of found participants. It is possible that
 * the given size of the list is not the same as the number of found participants.
 * If less participants are found, then the last few entries in an array stay
 * untouched. If more participants are found and the array is too small, then the
 * participants returned are undefined.
 *
 * @param[in]  domain_id    域ID。
 * @param[out] participants 域的参与者。
 * @param[in]  size         预分配参与者列表的大小。
 *
 * @param[in]  domain_id    The domain id.
 * @param[out] participants The participant for domain.
 * @param[in]  size         Size of the pre-allocated participant's list.
 *
 * @returns 找到的参与者数量或错误代码。
 * @returns Number of participants found or and error code.
 *
 * @retval >0
 *             找到的参与者数量。
 * @retval DDS_RETCODE_ERROR
 *             发生内部错误。
 * @retval DDS_RETCODE_BAD_PARAMETER
 *             参与者参数为 NULL，同时提供了大小。
 *
 * @retval >0
 *             Number of participants found.
 * @retval DDS_RETCODE_ERROR
 *             An internal error has occurred.
 * @retval DDS_RETCODE_BAD_PARAMETER
 *             The participant parameter is NULL, while a size is provided.
 */
DDS_EXPORT dds_return_t dds_lookup_participant(dds_domainid_t domain_id,
                                               dds_entity_t* participants,
                                               size_t size);

/**
 * @defgroup topic (Topic)
 * @ingroup dds
 */

/**
 * @brief 创建一个具有默认类型处理的新主题。
 * @brief Creates a new topic with default type handling.
 * @ingroup topic
 * @component topic
 *
 * 主题的类型名称来自生成的描述符。主题匹配是基于主题名称和类型名称的组合进行的。
 * The type name for the topic is taken from the generated descriptor. Topic
 * matching is done on a combination of topic name and type name.
 * 每次成功调用 dds_create_topic 都会创建一个新的主题实体，与同名的其他所有主题共享相同的 QoS 设置。
 * Each successful call to dds_create_topic creates a new topic entity sharing the same QoS
 * settings with all other topics of the same name.
 *
 * @param[in]  participant  要在其上创建主题的参与者。
 * @param[in]  participant  Participant on which to create the topic.
 * @param[in]  descriptor   一个 IDL 生成的主题描述符。
 * @param[in]  descriptor   An IDL generated topic descriptor.
 * @param[in]  name         主题的名称。
 * @param[in]  name         Name of the topic.
 * @param[in]  qos          设置在新主题上的 QoS（可以为 NULL）。
 * @param[in]  qos          QoS to set on the new topic (can be NULL).
 * @param[in]  listener     与新主题关联的任何侦听器函数（可以为 NULL）。
 * @param[in]  listener     Any listener functions associated with the new topic (can be NULL).
 *
 * @returns 一个有效的、唯一的主题句柄或错误代码。
 * @returns A valid, unique topic handle or an error code.
 *
 * @retval >=0
 *             一个有效的唯一主题句柄。
 * @retval >=0
 *             A valid unique topic handle.
 * @retval DDS_RETCODE_BAD_PARAMETER
 *             参与者、描述符、名称或 QoS 无效。
 * @retval DDS_RETCODE_BAD_PARAMETER
 *             Either participant, descriptor, name or qos is invalid.
 * @retval DDS_RETCODE_INCONSISTENT_POLICY
 *             QoS 与现有主题的 QoS 不匹配。
 * @retval DDS_RETCODE_INCONSISTENT_POLICY
 *             QoS mismatch between qos and an existing topic's QoS.
 * @retval DDS_RETCODE_PRECONDITION_NOT_MET
 *             描述符中的类型名称与预先存在的主题的类型名称不匹配。
 * @retval DDS_RETCODE_PRECONDITION_NOT_MET
 *             Mismatch between type name in descriptor and pre-existing
 *             topic's type name.
 */
DDS_EXPORT dds_entity_t dds_create_topic(dds_entity_t participant,
                                         const dds_topic_descriptor_t* descriptor,
                                         const char* name,
                                         const dds_qos_t* qos,
                                         const dds_listener_t* listener);

/**
 * @brief 表示库定义了 dds_create_topic_sertype 函数
 * @ingroup topic
 * 引入此函数是为了帮助从 sertopic 切换到 sertype。如果您使用的是
 * 现代 CycloneDDS 版本，您将不需要这个。
 *
 * @brief Indicates that the library defines the dds_create_topic_sertype function
 * @ingroup topic
 * Introduced to help with the change from sertopic to sertype. If you are using
 * a modern CycloneDDS version you will not need this.
 */
#define DDS_HAS_CREATE_TOPIC_SERTYPE 1

/**
 * @brief 使用提供的类型处理创建一个新主题。
 * @ingroup topic
 * @component topic
 *
 * 类型名来自提供的 "sertype" 对象。类型匹配是基于主题名称和类型名称的组合进行的。
 * 每次成功调用 dds_create_topic 都会创建一个与同名其他主题共享相同 QoS 设置的新主题实体。
 *
 * @brief Creates a new topic with provided type handling.
 * @ingroup topic
 * @component topic
 *
 * The name for the type is taken from the provided "sertype" object. Type
 * matching is done on a combination of topic name and type name. Each successful
 * call to dds_create_topic creates a new topic entity sharing the same QoS
 * settings with all other topics of the same name.
 *
 * 如果此函数返回一个有效句柄，提供的 sertype 的所有权将移交给 Cyclone。
 * 在返回时，调用者在 sertype 参数中获得一个指向实际被主题使用的 sertype 的指针。
 * 这可以是提供的 sertype（如果此 sertype 尚未在域中知道），或者已经在域中知道的 sertype。
 *
 * In case this function returns a valid handle, the ownership of the provided
 * sertype is handed over to Cyclone. On return, the caller gets in the sertype parameter a
 * pointer to the sertype that is actually used by the topic. This can be the provided sertype
 * (if this sertype was not yet known in the domain), or a sertype thas was
 * already known in the domain.
 *
 * @param[in]     participant  要在其上创建主题的参与者。
 * @param[in]     name         主题名称
 * @param[in,out] sertype      类型的内部描述。返回时，sertype 参数设置为实际被主题使用的 sertype。
 * @param[in]     qos          设置在新主题上的 QoS（可以为 NULL）。
 * @param[in]     listener     与新主题关联的任何侦听器函数（可以为 NULL）。
 * @param[in]     sedp_plist   被忽略（应为 NULL，将来可能会强制执行）。
 *
 * @returns 有效的、唯一的主题句柄或错误代码。当且仅当有效句柄时，域接管提供的 serdata 的所有权。
 *
 * @retval >=0
 *             有效的唯一主题句柄。
 * @retval DDS_RETCODE_BAD_PARAMETER
 *             参与者、描述符、名称或 qos 无效。
 * @retval DDS_RETCODE_INCONSISTENT_POLICY
 *             qos 与现有主题的 QoS 不匹配。
 * @retval DDS_RETCODE_PRECONDITION_NOT_MET
 *             sertype 中的类型名称与预先存在的主题的类型名称不匹配。
 */
DDS_EXPORT dds_entity_t dds_create_topic_sertype(dds_entity_t participant,
                                                 const char* name,
                                                 struct ddsi_sertype** sertype,
                                                 const dds_qos_t* qos,
                                                 const dds_listener_t* listener,
                                                 const struct ddsi_plist* sedp_plist);

/**
 * @brief 根据主题名称和类型信息查找本地创建的或发现的远程主题
 * @brief Finds a locally created or discovered remote topic by topic name and type information
 * @ingroup topic
 * @component topic
 *
 * 根据主题名称和类型查找本地创建的主题或发现的远程主题。如果找不到主题，此函数将等待
 * 提供的超时时间内使主题变为可用。
 * Finds a locally created topic or a discovered remote topic based on the topic
 * name and type. In case the topic is not found, this function will wait for
 * the topic to become available until the provided time out.
 *
 * 当使用范围 DDS_FIND_SCOPE_LOCAL_DOMAIN 时，如果类型未解析，则不会发送请求
 * 在网络上解析类型。这也适用于依赖类型：在提供的类型的依赖项未解析的情况下，当使用 LOCAL_DOMAIN
 * 范围时， 不会发送请求以解析类型。 When using the scope DDS_FIND_SCOPE_LOCAL_DOMAIN, there will be
 * no requests sent over the network for resolving the type in case it is unresolved. This also
 * applies to dependent types: in case a dependency of the provided type is unresolved, no requests
 * will be sent for resolving the type when using LOCAL_DOMAIN scope.
 *
 * 如果范围是 DDS_FIND_SCOPE_GLOBAL，则对于未解析的类型（或依赖项）
 * 将发送类型查找请求。
 * In case the scope is DDS_FIND_SCOPE_GLOBAL, for unresolved types (or dependencies)
 * a type lookup request will be sent.
 *
 * 如果未提供类型信息，并且存在多个（已发现的）具有提供名称的主题，
 * 将返回具有该名称的任意主题。在这种情况下，最好读取 DCPSTopic 数据并使用该数据
 * 获取所需的主题元数据。
 * In case no type information is provided and multiple (discovered) topics exist
 * with the provided name, an arbitrary topic with that name will be returned.
 * In this scenario, it would be better to read DCPSTopic data and use that to
 * get the required topic meta-data.
 *
 * 应使用 dds_delete 释放返回的主题。
 * The returned topic should be released with dds_delete.
 *
 * @param[in]  scope        用于查找主题的范围。如果构建中未启用主题发现，则不能使用 SCOPE_GLOBAL。
 * @param[in]  participant  找到的主题将在其中创建的参与者的句柄
 * @param[in]  name         要查找的主题的名称。
 * @param[in]  type_info    要查找的主题的类型信息。可选的，如果构建中未启用主题发现，则不应提供。
 * @param[in]  timeout      等待主题变为可用的超时时间
 *
 * @returns 有效的主题句柄或错误代码。
 *
 * @retval >0
 *             有效的主题句柄。
 * @retval 0
 *             不存在此名称的主题
 * @retval DDS_RETCODE_BAD_PARAMETER
 *             参与者或类型信息无效。
 * @retval DDS_RETCODE_PRECONDITION_NOT_MET
 *             无法找到提供的类型。
 */
DDS_EXPORT dds_entity_t dds_find_topic(dds_find_scope_t scope,
                                       dds_entity_t participant,
                                       const char* name,
                                       const dds_typeinfo_t* type_info,
                                       dds_duration_t timeout);

/**
 * @component topic
 * @deprecated Finds a locally created or discovered remote topic by topic name
 * @ingroup deprecated
 * 使用 @ref dds_find_topic 代替。
 * Use @ref dds_find_topic instead.
 *
 * 根据主题名称查找本地创建的主题或发现的远程主题。如果没有找到主题，此函数将等待主题可用，直到提供的超时时间。
 * Finds a locally created topic or a discovered remote topic based on the topic
 * name. In case the topic is not found, this function will wait for the topic
 * to become available until the provided time out.
 *
 * 如果提供的名称存在多个（已发现）主题，此函数将随机返回其中一个主题。调用者可以决定读取 DCPSTopic
 * 数据并选择一个主题定义来创建主题。 In case multiple (discovered) topics exist with the provided
 * name, this function will return randomly one of these topic. The caller can decide to read
 * DCPSTopic data and select one of the topic definitions to create the topic.
 *
 * 返回的主题应使用 dds_delete 释放。
 * The returned topic should be released with dds_delete.
 *
 * @param[in]  scope        用于查找主题的范围
 * @param[in]  scope        The scope used to find the topic
 * @param[in]  participant  找到的主题将在其中创建的参与者的句柄
 * @param[in]  participant  The handle of the participant the found topic will be created in
 * @param[in]  name         要查找的主题的名称。
 * @param[in]  name         The name of the topic to find.
 * @param[in]  timeout      等待主题可用的超时时间
 * @param[in]  timeout      The timeout for waiting for the topic to become available
 *
 * @returns 有效的主题句柄或错误代码。
 * @returns A valid topic handle or an error code.
 *
 * @retval >0
 *             有效的主题句柄。
 * @retval >0
 *             A valid topic handle.
 * @retval 0
 *             此名称的主题尚不存在
 * @retval 0
 *             No topic of this name existed yet
 * @retval DDS_RETCODE_BAD_PARAMETER
 *             参与者句柄或范围无效
 * @retval DDS_RETCODE_BAD_PARAMETER
 *             Participant handle or scope invalid
 */
DDS_DEPRECATED_EXPORT dds_entity_t dds_find_topic_scoped(dds_find_scope_t scope,
                                                         dds_entity_t participant,
                                                         const char* name,
                                                         dds_duration_t timeout);

/**
 * @ingroup topic
 * @component topic
 * @brief 创建提供的 type_info 的主题描述符 (Creates topic descriptor for the provided type_info)
 *
 * @param[in]  scope        用于查找类型的范围：DDS_FIND_SCOPE_LOCAL_DOMAIN 或
 * DDS_FIND_SCOPE_GLOBAL。 如果使用 DDS_FIND_SCOPE_GLOBAL，则会向其他节点发送类型查找请求。 (The
 * scope used to find the type: DDS_FIND_SCOPE_LOCAL_DOMAIN or DDS_FIND_SCOPE_GLOBAL. In case
 * DDS_FIND_SCOPE_GLOBAL is used, a type lookup request will be sent to other nodes.)
 * @param[in]  participant  参与者的句柄。(The handle of the participant.)
 * @param[in]  type_info    要查找的主题的类型（dds_typeinfo_t）。
 *                          (The type (dds_typeinfo_t) of the topic to find.)
 * @param[in]  timeout      等待类型变为可用的超时时间。
 *                          (The timeout for waiting for the type to become available)
 * @param[out] descriptor - 指向将分配和填充的 dds_topic_descriptor_t
 * 指针的指针。要释放为此描述符分配的内存，请使用 dds_delete_topic_descriptor。 (Pointer to a
 * dds_topic_descriptor_t pointer that will be allocated and populated. To free allocated memory for
 * this descriptor, use dds_delete_topic_descriptor.)
 *
 * @returns 表示成功或失败的 dds_return_t。（A dds_return_t indicating success or failure.）
 *
 * @retval DDS_RETCODE_OK
 *             主题描述符已成功创建。(The topic descriptor has been succesfully created.)
 * @retval DDS_RETCODE_BAD_PARAMETER
 *             未提供 Type_info 或描述符参数，实体无效（不是参与者）或范围无效。
 *             (Type_info or descriptor parameter not provided, invalid entity (not a participant)
 * or scope invalid.)
 * @retval DDS_RETCODE_PRECONDITION_NOT_MET
 *             未找到参与者或 type_id。(The participant or the type_id was not found.)
 * @retval DDS_RETCODE_TIMEOUT
 *             在提供的超时时间内未解析类型。(Type was not resolved within the provided timeout)
 * @retval DDS_RETCODE_UNSUPPORTED
 *             Cyclone DDS 构建没有类型发现（参见 DDS_HAS_TYPE_DISCOVERY）。
 *             (Cyclone DDS built without type discovery
 *             (cf. DDS_HAS_TYPE_DISCOVERY))
 */
DDS_EXPORT dds_return_t dds_create_topic_descriptor(dds_find_scope_t scope,
                                                    dds_entity_t participant,
                                                    const dds_typeinfo_t* type_info,
                                                    dds_duration_t timeout,
                                                    dds_topic_descriptor_t** descriptor);

/**
 * @ingroup topic
 * @component topic
 * @brief 删除分配给提供的主题描述符的内存 (Delete memory allocated to the provided topic
 * descriptor)
 *
 * @param[in] descriptor - 指向 dds_topic_descriptor_t 的指针 (Pointer to a dds_topic_descriptor_t)
 *
 * @returns 表示成功或失败的 dds_return_t。 (A dds_return_t indicating success or failure.)
 *
 * @retval DDS_RETCODE_OK
 *             主题描述符已成功删除。 (The topic descriptor has been succesfully deleted.)
 * @retval DDS_RETCODE_BAD_PARAMETER
 *             未提供描述符 (No descriptor provided)
 * @retval DDS_RETCODE_UNSUPPORTED
 *             Cyclone DDS 构建时没有类型发现 (Cyclone DDS built without type discovery)
 *             (参见 DDS_HAS_TYPE_DISCOVERY) (cf. DDS_HAS_TYPE_DISCOVERY)
 */
DDS_EXPORT dds_return_t dds_delete_topic_descriptor(dds_topic_descriptor_t* descriptor);

/**
 * @brief 返回给定主题的名称。 (Returns the name of a given topic.)
 * @ingroup topic
 * @component topic
 *
 * @param[in]  topic  主题。 (The topic.)
 * @param[out] name   用于写入主题名称的缓冲区。 (Buffer to write the topic name to.)
 * @param[in]  size   缓冲区中可用的字节数。 (Number of bytes available in the buffer.)
 *
 * @returns 表示成功或失败的 dds_return_t。 (A dds_return_t indicating success or failure.)
 *
 * @return 主题名称的实际长度（如果返回值 >= size，则名称被截断）或错误。 (Actual length of topic
 * name (name is truncated if return value >= size) or error.)
 */
DDS_EXPORT dds_return_t dds_get_name(dds_entity_t topic, char* name, size_t size);

/**
 * @brief 返回给定主题的类型名称。 (Returns the type name of a given topic.)
 * @ingroup topic
 * @component topic
 *
 * @param[in]  topic  主题。 (The topic.)
 * @param[out] name   用于写入主题类型名称的缓冲区。 (Buffer to write the topic type name to.)
 * @param[in]  size   缓冲区中可用的字节数。 (Number of bytes available in the buffer.)
 *
 * @returns 表示成功或失败的 dds_return_t。 (A dds_return_t indicating success or failure.)
 *
 * @return 类型名称的实际长度（如果返回值 >= size，则名称被截断）或错误。 (Actual length of type
 * name (name is truncated if return value >= size) or error.)
 */
DDS_EXPORT dds_return_t dds_get_type_name(dds_entity_t topic, char* name, size_t size);

/**
 * @defgroup topic_filter (主题过滤器 Topic filters)
 * @ingroup topic
 * 主题过滤器函数 (Topic filter functions).
 * @warning 不稳定 API 的一部分：不能保证向后兼容性。
 *          (part of the Unstable API: no guarantee that any
 *          of this will be maintained for backwards compatibility.)
 *
 * 当在写调用中进行过滤时，Sampleinfo 全为零（即，使用过滤主题创建的 writer），
 * 否则它会尽可能正确地填充给定上下文和固定的剩余部分：
 * (Sampleinfo is all zero when filtering in a write call (i.e., writer created
 * using a filtered topic, which one perhaps shouldn't be doing), otherwise it
 * has as much filled in correctly as is possible given the context and the rest
 * fixed:)
 *    - sample_state         DDS_SST_NOT_READ;
 *    - publication_handle   设置为 writer 的实例句柄 (set to writer's instance handle)
 *    - source_timestamp     设置为样本的源时间戳 (set to source timestamp of sample)
 *    - ranks                0
 *    - valid_data           true
 *    - instance_handle      如果样本匹配现有实例，则设置为现有实例的实例句柄，
 *                           否则设置为通过过滤器的实例句柄
 *                           (set to instance handle of existing instance if the
 *                           sample matches an existing instance, otherwise to what
 *                           the instance handle will be if it passes the filter)
 *    - view_state           如果被过滤的样本匹配现有实例，则设置为实例视图状态，
 *                           如果不是，则为 NEW
 *                           (set to instance view state if sample being filtered
 *                           matches an existing instance, NEW if not)
 *    - instance_state       如果被过滤的样本匹配现有实例，则设置为实例状态，
 *                           如果不是，则为 NEW
 *                           (set to instance state if sample being filtered
 *                           matches an existing instance, NEW if not)
 *    - generation counts    如果样本匹配现有实例实例，则设置为实例的生成计数，
 *                           如果不是，则为 0
 *                           (set to instance's generation counts if the sample
 *                           matches an existing instance instance, 0 if not)
 */

/**
 * @anchor dds_topic_filter_sample_fn
 * @brief 仅需要查看样本的主题过滤函数。 (Topic filter function that only needs to look at the
 * sample.)
 * @ingroup topic_filter
 * @warning 不稳定的 API (Unstable API)
 * @unstable
 *
 * @param[in] sample 指向样本数据的指针。 (Pointer to the sample data.)
 * @return 如果样本通过过滤器，则返回 true，否则返回 false。 (Returns true if the sample passes the
 * filter, false otherwise.)
 */
typedef bool (*dds_topic_filter_sample_fn)(const void* sample);

/**
 * @anchor dds_topic_filter_sample_arg_fn
 * @brief 仅需要查看样本和自定义参数的主题过滤函数。 (Topic filter function that only needs to look
 * at the sample and a custom argument.)
 * @ingroup topic_filter
 * @warning 不稳定的 API (Unstable API)
 *
 * @param[in] sample 指向样本数据的指针。 (Pointer to the sample data.)
 * @param[in,out] arg 自定义参数。 (Custom argument.)
 * @return 如果样本通过过滤器，则返回 true，否则返回 false。 (Returns true if the sample passes the
 * filter, false otherwise.)
 */
typedef bool (*dds_topic_filter_sample_arg_fn)(const void* sample, void* arg);

/**
 * @anchor dds_topic_filter_sampleinfo_arg_fn
 * @brief 仅需要查看样本信息和自定义参数的主题过滤函数。 (Topic filter function that only needs to
 * look at the sampleinfo and a custom argument.)
 * @ingroup topic_filter
 * @warning 不稳定的 API (Unstable API)
 *
 * @param[in] sampleinfo 指向样本信息的指针。 (Pointer to the sample information.)
 * @param[in,out] arg 自定义参数。 (Custom argument.)
 * @return 如果样本信息通过过滤器，则返回 true，否则返回 false。 (Returns true if the sample
 * information passes the filter, false otherwise.)
 */
typedef bool (*dds_topic_filter_sampleinfo_arg_fn)(const dds_sample_info_t* sampleinfo, void* arg);

/**
 * @anchor dds_topic_filter_sample_sampleinfo_arg_fn
 * @brief 需要查看样本、样本信息和自定义参数的主题过滤函数。 (Topic filter function that needs to
 * look at the sample, the sampleinfo and a custom argument.)
 * @ingroup topic_filter
 * @warning 不稳定的 API (Unstable API)
 *
 * @param[in] sample 指向样本数据的指针。 (Pointer to the sample data.)
 * @param[in] sampleinfo 指向样本信息的指针。 (Pointer to the sample information.)
 * @param[in,out] arg 自定义参数。 (Custom argument.)
 * @return 如果样本和样本信息通过过滤器，则返回 true，否则返回 false。 (Returns true if the sample
 * and sample information pass the filter, false otherwise.)
 */
typedef bool (*dds_topic_filter_sample_sampleinfo_arg_fn)(const void* sample,
                                                          const dds_sample_info_t* sampleinfo,
                                                          void* arg);

/**
 * @anchor dds_topic_filter_fn
 * @brief 参见 \ref dds_topic_filter_sample_fn (See \ref dds_topic_filter_sample_fn)
 * @ingroup topic_filter
 * @warning 不稳定的 API (Unstable API)
 */
typedef dds_topic_filter_sample_fn dds_topic_filter_fn;

/**
 * @anchor dds_topic_filter_arg_fn
 * @brief 参见 \ref dds_topic_filter_sample_arg_fn (See \ref dds_topic_filter_sample_arg_fn)
 * @ingroup topic_filter
 * @warning 不稳定的 API (Unstable API)
 */
typedef dds_topic_filter_sample_arg_fn dds_topic_filter_arg_fn;

/**
 * @brief 主题过滤器模式 (Topic filter mode)
 * @ingroup topic_filter
 * @warning 不稳定的 API (Unstable API)
 */
enum dds_topic_filter_mode {
  DDS_TOPIC_FILTER_NONE,   /**< 用于重置主题过滤器 (Can be used to reset topic filter) */
  DDS_TOPIC_FILTER_SAMPLE, /**< 与 \ref dds_topic_filter_sample_fn 一起使用 (Use with \ref
                              dds_topic_filter_sample_fn) */
  DDS_TOPIC_FILTER_SAMPLE_ARG, /**< 与 \ref dds_topic_filter_sample_arg_fn 一起使用 (Use with \ref
                                  dds_topic_filter_sample_arg_fn) */
  DDS_TOPIC_FILTER_SAMPLEINFO_ARG, /**< 与 \ref dds_topic_filter_sampleinfo_arg_fn 一起使用 (Use
                                      with \ref dds_topic_filter_sampleinfo_arg_fn) */
  DDS_TOPIC_FILTER_SAMPLE_SAMPLEINFO_ARG, /**< 与 \ref
                                             dds_topic_filter_sample_sampleinfo_arg_fn 一起使用 (Use
                                             with \ref dds_topic_filter_sample_sampleinfo_arg_fn) */
};

/**
 * @brief 所有过滤器函数类型的联合 (Union of all filter function types)
 * @ingroup topic_filter
 * @warning 不稳定的 API (Unstable API)
 */
union dds_topic_filter_function_union {
  dds_topic_filter_sample_fn
      sample; /**< 与模式 dds_topic_filter_mode::DDS_TOPIC_FILTER_SAMPLE 一起使用 (Use with mode
                 dds_topic_filter_mode::DDS_TOPIC_FILTER_SAMPLE) */
  dds_topic_filter_sample_arg_fn
      sample_arg; /**< 与模式 dds_topic_filter_mode::DDS_TOPIC_FILTER_SAMPLE_ARG 一起使用 (Use with
                     mode dds_topic_filter_mode::DDS_TOPIC_FILTER_SAMPLE_ARG) */
  dds_topic_filter_sampleinfo_arg_fn
      sampleinfo_arg; /**< 与模式 dds_topic_filter_mode::DDS_TOPIC_FILTER_SAMPLEINFO_ARG 一起使用
                         (Use with mode dds_topic_filter_mode::DDS_TOPIC_FILTER_SAMPLEINFO_ARG) */
  dds_topic_filter_sample_sampleinfo_arg_fn
      sample_sampleinfo_arg; /**< 与模式
                                dds_topic_filter_mode::DDS_TOPIC_FILTER_SAMPLE_SAMPLEINFO_ARG
                                一起使用 (Use with mode
                                dds_topic_filter_mode::DDS_TOPIC_FILTER_SAMPLE_SAMPLEINFO_ARG) */
};

/**
 * @brief 全主题过滤器容器 (Full topic filter container)
 * @ingroup topic_filter
 * @warning 不稳定的API (Unstable API)
 */
struct dds_topic_filter {
  enum dds_topic_filter_mode mode;         /**< 提供一个模式 (Provide a mode) */
  union dds_topic_filter_function_union f; /**< 提供一个过滤函数 (Provide a filter function) */
  void* arg; /**< 提供一个参数，可以为NULL (Provide an argument, can be NULL) */
};

/**
 * @anchor dds_set_topic_filter_and_arg
 * @brief 在主题上设置过滤器和过滤器参数 (Sets a filter and filter argument on a topic)
 * @ingroup topic_filter
 * @component topic
 * @warning 不稳定的API (Unstable API)
 * 将被读者的适当过滤替代 (To be replaced by proper filtering on readers)
 *
 * 与使用此主题的读者/作者读取/写入数据的线程安全性不同 (Not thread-safe with respect to data being
 * read/written using readers/writers using this topic)
 * 确保创建特定于要过滤的读者的主题实体，然后设置过滤函数，然后再创建读者 (Be sure to create a topic
 * entity specific to the reader you want to filter, then set the filter function, and only then
 * create the reader) 除非您知道没有并发写入，否则不要更改它 (And don't change it unless you know
 * there are no concurrent writes)
 *
 * @param[in]  topic   设置内容过滤器的主题 (The topic on which the content filter is set)
 * @param[in]  filter  用于过滤主题样本的过滤函数 (The filter function used to filter topic samples)
 * @param[in]  arg     过滤函数的参数 (Argument for the filter function)
 *
 * @returns 表示成功或失败的dds_return_t (A dds_return_t indicating success or failure)
 *
 * @retval DDS_RETCODE_OK  成功设置过滤器 (Filter set successfully)
 * @retval DDS_RETCODE_BAD_PARAMETER  主题句柄无效 (The topic handle is invalid)
 */
DDS_EXPORT dds_return_t dds_set_topic_filter_and_arg(dds_entity_t topic,
                                                     dds_topic_filter_arg_fn filter,
                                                     void* arg);

/**
 * @anchor dds_set_topic_filter_extended
 * @brief 设置主题上的过滤器和过滤器参数。(Sets a filter and filter argument on a topic.)
 * @ingroup topic_filter
 * @component topic
 * @warning 不稳定的API (Unstable API)
 * 将被读者的正确过滤所取代。 (To be replaced by proper filtering on readers.)
 *
 * 与使用此主题的读/写数据的线程不安全。 (Not thread-safe with respect to data being read/written
 * using readers/writers using this topic.)
 * 请确保为您要过滤的读者创建一个特定的主题实体，然后设置过滤器函数，然后再创建读者。 (Be sure to
 * create a topic entity specific to the reader you want to filter, then set the filter function,
 * and only then create the reader.) 除非您知道没有并发写入，否则不要更改它。 (And don't change it
 * unless you know there are no concurrent writes.)
 *
 * @param[in]  topic   设置内容过滤器的主题。 (The topic on which the content filter is set.)
 * @param[in]  filter  过滤器规范。 (The filter specification.)
 *
 * @returns 指示成功或失败的dds_return_t。 (A dds_return_t indicating success or failure.)
 *
 * @retval DDS_RETCODE_OK  过滤器设置成功 (Filter set successfully)
 * @retval DDS_RETCODE_BAD_PARAMETER  主题句柄无效 (The topic handle is invalid)
 */
DDS_EXPORT dds_return_t dds_set_topic_filter_extended(dds_entity_t topic,
                                                      const struct dds_topic_filter* filter);

/**
 * @brief 获取主题的过滤器。 (Gets the filter for a topic.)
 * @ingroup topic_filter
 * @component topic
 * @warning 不稳定的API (Unstable API)
 *
 * 将被读者的正确过滤所取代。 (To be replaced by proper filtering on readers.)
 *
 * @param[in]  topic  要获取过滤器的主题。 (The topic from which to get the filter.)
 * @param[out] fn     主题过滤器函数（fn可以为NULL）。 (The topic filter function (fn may be NULL).)
 * @param[out] arg    过滤器函数参数（arg可以为NULL）。 (Filter function argument (arg may be
 * NULL).)
 *
 * @retval DDS_RETCODE_OK  过滤器设置成功 (Filter set successfully)
 * @retval DDS_RETCODE_PRECONDITION_NOT_MET  过滤器不是“none”或“sample_arg” (Filter is not of "none"
 * or "sample_arg")
 * @retval DDS_RETCODE_BAD_PARAMETER  主题句柄无效 (The topic handle is invalid)
 */
DDS_EXPORT dds_return_t dds_get_topic_filter_and_arg(dds_entity_t topic,
                                                     dds_topic_filter_arg_fn* fn,
                                                     void** arg);

/**
 * @brief 获取主题的过滤器 (Gets the filter for a topic.)
 * @ingroup topic_filter
 * @component topic
 * @warning 不稳定的API (Unstable API)
 *
 * 将被替换为读者上的适当过滤 (To be replaced by proper filtering on readers)
 *
 * @param[in]  topic  要获取过滤器的主题 (The topic from which to get the filter.)
 * @param[out] filter 主题过滤器规范 (The topic filter specification.)
 *
 * @retval DDS_RETCODE_OK  过滤器设置成功 (Filter set successfully)
 * @retval DDS_RETCODE_BAD_PARAMETER  主题句柄无效 (The topic handle is invalid)
 */
DDS_EXPORT dds_return_t dds_get_topic_filter_extended(dds_entity_t topic,
                                                      struct dds_topic_filter* filter);

/**
 * @defgroup subscriber (订阅者) (Subscriber)
 * @ingroup subscription
 * DOC_TODO 订阅者是一个DDS实体 (The Subscriber is a DDS Entity)
 */

/**
 * @brief 创建一个新的DDS订阅者实例 (Creates a new instance of a DDS subscriber)
 * @ingroup subscriber
 * @component subscriber
 *
 * @param[in]  participant 创建订阅者的参与者 (The participant on which the subscriber is being
 * created.)
 * @param[in]  qos         设置在新订阅者上的QoS (可以为NULL) (The QoS to set on the new subscriber
 * (can be NULL).)
 * @param[in]  listener    与新订阅者关联的任何侦听器功能 (可以为NULL) (Any listener functions
 * associated with the new subscriber (can be NULL).)
 *
 * @returns 有效的订阅者句柄或错误代码 (A valid subscriber handle or an error code.)
 *
 * @retval >0
 *             有效的订阅者句柄 (A valid subscriber handle.)
 * @retval DDS_RETCODE_ERROR
 *             发生内部错误 (An internal error has occurred.)
 * @retval DDS_RETCODE_BAD_PARAMETER
 *             参数之一无效 (One of the parameters is invalid.)
 */
DDS_EXPORT dds_entity_t dds_create_subscriber(dds_entity_t participant,
                                              const dds_qos_t* qos,
                                              const dds_listener_t* listener);

/**
 * @defgroup publication (Publication)
 * @ingroup dds
 * DOC_TODO This contains the definitions regarding publication of data.
 */

/**
 * @defgroup publisher (Publisher)
 * @ingroup publication
 * DOC_TODO The Publisher is a DDS Entity
 */

/**
 * @brief 创建一个新的DDS发布者实例 (Creates a new instance of a DDS publisher)
 * @ingroup publisher
 * @component publisher
 *
 * @param[in]  participant 要为其创建发布者的参与者 (The participant to create a publisher for.)
 * @param[in]  qos         设置在新发布者上的QoS（可以为NULL）(The QoS to set on the new publisher
 * (can be NULL).)
 * @param[in]  listener    与新发布者关联的任何监听器函数（可以为NULL）(Any listener functions
 * associated with the new publisher (can be NULL).)
 *
 * @returns 有效的发布者句柄或错误代码 (A valid publisher handle or an error code.)
 *
 * @retval >0
 *            有效的发布者句柄 (A valid publisher handle.)
 * @retval DDS_RETCODE_ERROR
 *            发生内部错误 (An internal error has occurred.)
 */
/* TODO: Check list of error codes is complete. */
DDS_EXPORT dds_entity_t dds_create_publisher(dds_entity_t participant,
                                             const dds_qos_t* qos,
                                             const dds_listener_t* listener);

/**
 * @brief 暂停发布者的发布 (Suspends the publications of the Publisher)
 * @ingroup publisher
 * @component publisher
 *
 * 此操作是对服务的提示，以便通过例如收集对DDS编写器的修改，然后对其进行批处理来优化其性能。服务无需使用提示。
 * (This operation is a hint to the Service so it can optimize its performance by e.g., collecting
 * modifications to DDS writers and then batching them. The Service is not required to use the
 * hint.)
 *
 * 此操作的每次调用都必须与相应的调用 @see dds_resume 匹配，表示修改集已完成。
 * (Every invocation of this operation must be matched by a corresponding call to @see dds_resume
 * indicating that the set of modifications has completed.)
 *
 * @param[in]  publisher 将暂停所有发布的发布者 (The publisher for which all publications will be
 * suspended.)
 *
 * @returns 表示成功或失败的dds_return_t (A dds_return_t indicating success or failure.)
 *
 * @retval DDS_RETCODE_OK
 *             成功暂停发布 (Publications suspended successfully.)
 * @retval DDS_RETCODE_BAD_PARAMETER
 *             pub参数不是有效的发布者 (The pub parameter is not a valid publisher.)
 * @retval DDS_RETCODE_UNSUPPORTED
 *             操作不受支持 (Operation is not supported.)
 */
DDS_EXPORT dds_return_t dds_suspend(dds_entity_t publisher);

/**
 * @brief 恢复发布者的发布操作
 * @ingroup publisher
 * @component publisher
 *
 * 此操作是向服务提供的提示，表明应用程序已完成由之前的 dds_suspend()
 * 启动的更改。服务无需使用该提示。
 *
 * 调用 resume_publications 必须与先前对 @see suspend_publications 的调用相匹配。
 *
 * @param[in]  publisher 将恢复所有发布操作的发布者。
 *
 * @returns 表示成功或失败的 dds_return_t。
 *
 * @retval DDS_RETCODE_OK
 *             成功恢复发布。
 * @retval DDS_RETCODE_BAD_PARAMETER
 *             pub 参数不是有效的发布者。
 * @retval DDS_RETCODE_PRECONDITION_NOT_MET
 *             没有先前匹配的 dds_suspend()。
 * @retval DDS_RETCODE_UNSUPPORTED
 *             操作不受支持。
 */
// Resumes the publications of the Publisher
DDS_EXPORT dds_return_t dds_resume(dds_entity_t publisher);

/**
 * @brief 在发布者或写入者中等待最长持续时间的超时以获取 acks 数据。
 * @ingroup publication
 * @component publisher
 *
 * 此操作阻止调用线程，直到发布者或写入者编写的所有数据都被所有匹配的可靠读取器实体确认，或者超时参数指定的持续时间过去，以先发生者为准。
 *
 * @param[in]  publisher_or_writer 必须等待其确认的发布者或写入者
 * @param[in]  timeout             在超时之前等待确认的时间长度
 *
 * @returns 表示成功或失败的 dds_return_t。
 *
 * @retval DDS_RETCODE_OK
 *             在超时内成功接收到所有确认。
 * @retval DDS_RETCODE_BAD_PARAMETER
 *             publisher_or_writer 不是有效的发布者或写入者。
 * @retval DDS_RETCODE_TIMEOUT
 *             在从可靠读取器实体接收到所有确认之前，超时过期。
 * @retval DDS_RETCODE_UNSUPPORTED
 *             操作不受支持。
 */
// Waits at most for the duration timeout for acks for data in the publisher or writer.
DDS_EXPORT dds_return_t dds_wait_for_acks(dds_entity_t publisher_or_writer, dds_duration_t timeout);

/**
 * @defgroup reader (阅读器)
 * @ingroup subscription (订阅)
 * DOC_TODO 阅读器是一个DDS实体 (The reader is a DDS Entity)
 */

/**
 * @brief 创建一个新的DDS阅读器实例。 (Creates a new instance of a DDS reader.)
 * @ingroup reader (阅读器)
 * @component reader (阅读器组件)
 *
 * 当使用参与者创建阅读器时，会自动创建一个隐式订阅者。
 * 当删除创建的阅读器时，此隐式订阅者将被自动删除。
 * (When a participant is used to create a reader, an implicit subscriber is created.
 * This implicit subscriber will be deleted automatically when the created reader
 * is deleted.)
 *
 * @param[in]  participant_or_subscriber 正在创建的阅读器的参与者或订阅者。(The participant or
 * subscriber on which the reader is being created.)
 * @param[in]  topic                     要读取的主题。(The topic to read.)
 * @param[in]  qos                       设置在新阅读器上的QoS（可以为NULL）。(The QoS to set on the
 * new reader (can be NULL).)
 * @param[in]  listener                  与新阅读器关联的任何侦听器函数（可以为NULL）。(Any listener
 * functions associated with the new reader (can be NULL).)
 *
 * @returns 有效的阅读器句柄或错误代码。 (A valid reader handle or an error code.)
 *
 * @retval >0
 *            有效的阅读器句柄。 (A valid reader handle.)
 * @retval DDS_RETCODE_ERROR
 *            发生内部错误。 (An internal error occurred.)
 *
 * DOC_TODO: 完整的错误代码列表 (Complete list of error codes)
 */
DDS_EXPORT dds_entity_t dds_create_reader(dds_entity_t participant_or_subscriber,
                                          dds_entity_t topic,
                                          const dds_qos_t* qos,
                                          const dds_listener_t* listener);

/**
 * @brief 创建一个具有自定义历史缓存的 DDS 读取器实例。
 * @brief Creates a new instance of a DDS reader with a custom history cache.
 * @ingroup reader
 * @component reader
 *
 * 当使用参与者创建读取器时，将创建一个隐式订阅者。
 * When a participant is used to create a reader, an implicit subscriber is created.
 * 删除创建的读取器时，此隐式订阅者将自动删除。
 * This implicit subscriber will be deleted automatically when the created reader is deleted.
 *
 * @param[in]  participant_or_subscriber 正在创建读取器的参与者或订阅者。
 * @param[in]  participant_or_subscriber The participant or subscriber on which the reader is being
 * created.
 * @param[in]  topic                     要读取的主题。
 * @param[in]  topic                     The topic to read.
 * @param[in]  qos                       设置在新读取器上的 QoS（可以为 NULL）。
 * @param[in]  qos                       The QoS to set on the new reader (can be NULL).
 * @param[in]  listener                  与新读取器关联的任何监听器函数（可以为 NULL）。
 * @param[in]  listener                  Any listener functions associated with the new reader (can
 * be NULL).
 * @param[in]  rhc                       要使用的读取器历史缓存，读取器成为所有者。
 * @param[in]  rhc                       Reader history cache to use, reader becomes the owner.
 *
 * @returns 有效的读取器句柄或错误代码。
 * @returns A valid reader handle or an error code.
 *
 * @retval >0
 *            有效的读取器句柄。
 * @retval >0
 *            A valid reader handle.
 * @retval DDS_RETCODE_ERROR
 *            发生内部错误。
 * @retval DDS_RETCODE_ERROR
 *            An internal error occurred.
 *
 * DOC_TODO: 完整的错误代码列表
 * DOC_TODO: Complete list of error codes
 */
DDS_EXPORT dds_entity_t dds_create_reader_rhc(
    dds_entity_t participant_or_subscriber,  // 正在创建读取器的参与者或订阅者
    dds_entity_t topic,                      // 要读取的主题
    const dds_qos_t* qos,                    // 设置在新读取器上的 QoS（可以为 NULL）
    const dds_listener_t* listener,  // 与新读取器关联的任何监听器函数（可以为 NULL）
    struct dds_rhc* rhc);            // 要使用的读取器历史缓存，读取器成为所有者

/**
 * @brief 等待读取器接收所有历史数据 (Wait until reader receives all historic data)
 * @ingroup reader
 * @component reader
 *
 * 该操作会阻塞调用线程，直到接收到所有的"历史"数据，或者超过 max_wait
 * 参数指定的持续时间，以先发生的为准。 返回值为0表示接收到了所有的"历史"数据；返回值为 TIMEOUT
 * 表示在接收到所有数据之前已经超时。 (The operation blocks the calling thread until either all
 * "historical" data is received, or else the duration specified by the max_wait parameter elapses,
 * whichever happens first. A return value of 0 indicates that all the "historical" data was
 * received; a return value of TIMEOUT indicates that max_wait elapsed before all the data was
 * received.)
 *
 * @param[in]  reader    要等待历史数据的读取器 (The reader on which to wait for historical data.)
 * @param[in]  max_wait  在超时之前等待历史数据的时间长度 (How long to wait for historical data
 * before time out.)
 *
 * @returns 状态，成功为0，超时为 TIMEOUT，错误为负值 (a status, 0 on success, TIMEOUT on timeout or
 * a negative value to indicate error.)
 *
 * DOC_TODO: Complete list of error codes
 */
DDS_EXPORT dds_return_t dds_reader_wait_for_historical_data(dds_entity_t reader,
                                                            dds_duration_t max_wait);

/**
 * @defgroup writer (Writer)
 * @ingroup publication
 * DOC_TODO The writer is a DDS Entity
 */

/**
 * @brief 创建一个新的 DDS 写入器实例 (Creates a new instance of a DDS writer.)
 * @ingroup writer
 * @component writer
 *
 * 当参与者用于创建写入器时，会自动创建一个隐式发布者。
 * 删除创建的写入器时，此隐式发布者也将被自动删除。
 * (When a participant is used to create a writer, an implicit publisher is created.
 * This implicit publisher will be deleted automatically when the created writer
 * is deleted.)
 *
 * @param[in]  participant_or_publisher 正在创建写入器的参与者或发布者 (The participant or publisher
 * on which the writer is being created.)
 * @param[in]  topic 要写入的主题 (The topic to write.)
 * @param[in]  qos 设置在新写入器上的 QoS（可以为 NULL）(The QoS to set on the new writer (can be
 * NULL).)
 * @param[in]  listener 与新写入器关联的任何监听器函数（可以为 NULL）(Any listener functions
 * associated with the new writer (can be NULL).)
 *
 * @returns 有效的写入器句柄或错误代码 (A valid writer handle or an error code.)
 *
 * @returns >0
 *              有效的写入器句柄 (A valid writer handle.)
 * @returns DDS_RETCODE_ERROR
 *              发生内部错误 (An internal error occurred.)
 *
 * DOC_TODO: Complete list of error codes
 */
DDS_EXPORT dds_entity_t dds_create_writer(dds_entity_t participant_or_publisher,
                                          dds_entity_t topic,
                                          const dds_qos_t* qos,
                                          const dds_listener_t* listener);

/**
 * @defgroup writing (写入数据)
 * @ingroup writer
 * 写入数据（及其变体）是简单明了的。第一组
 * 等同于将 -1 传递给 "timestamp" 的第二组，
 * 意味着，替换为调用 time() 的结果。处理
 * 和注销操作需要主题类型的对象，但
 * 只接触关键字段；其余部分可能未定义。
 */

/**
 * @brief 注册一个实例
 * @ingroup writing
 * @component data_instance
 *
 * 此操作向数据写入器注册具有键值的实例，并
 * 返回可用于连续写入和处理操作的实例句柄。当句柄未分配时，函数将返回
 * 错误，并且句柄将不被触及。
 *
 * @param[in]  writer  要与实例关联的写入器。
 * @param[out] handle  实例句柄。
 * @param[in]  data    带有键值的实例。
 *
 * @returns 表示成功或失败的 dds_return_t。
 *
 * @retval DDS_RETCODE_OK
 *            操作成功。
 * @retval DDS_RETCODE_BAD_PARAMETER
 *            给定参数之一无效。
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *            在不适当的对象上调用操作。
 */
DDS_EXPORT dds_return_t dds_register_instance(dds_entity_t writer,
                                              dds_instance_handle_t* handle,
                                              const void* data);

/**
 * @brief 通过实例取消注册一个实例 (Unregisters an instance by instance)
 * @ingroup 写入 (writing)
 * @component 数据实例 (data_instance)
 *
 * 此操作逆转了 register instance
 * 的操作，删除了有关实例的所有信息，并从数据写入器中取消注册具有键值的实例。 (This operation
 * reverses the action of register instance, removes all information regarding the instance and
 * unregisters an instance with a key value from the data writer.)
 *
 * @param[in]  writer  与实例关联的写入器。(The writer to which instance is associated.)
 * @param[in]  data    具有键值的实例。(The instance with the key value.)
 *
 * @returns 表示成功或失败的 dds_return_t。(A dds_return_t indicating success or failure.)
 *
 * @retval DDS_RETCODE_OK
 *             操作成功。(The operation was successful.)
 * @retval DDS_RETCODE_BAD_PARAMETER
 *             给定参数之一无效。(One of the given arguments is not valid.)
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *             在不适当的对象上调用操作。(The operation is invoked on an inappropriate object.)
 */
DDS_EXPORT dds_return_t dds_unregister_instance(dds_entity_t writer, const void* data);

/**
 * @brief 通过实例句柄取消注册一个实例 (Unregisters an instance by instance handle)
 * @ingroup 写入 (writing)
 * @component 数据实例 (data_instance)
 *
 * 此操作取消注册由给定类型实例句柄的键字段标识的实例。
 * (This operation unregisters the instance which is identified by the key fields of the given
 * typed instance handle.)
 *
 * @param[in]  writer  与实例关联的写入器。(The writer to which instance is associated.)
 * @param[in]  handle  实例句柄。(The instance handle.)
 *
 * @returns 表示成功或失败的 dds_return_t。(A dds_return_t indicating success or failure.)
 *
 * @retval DDS_RETCODE_OK
 *             操作成功。(The operation was successful.)
 * @retval DDS_RETCODE_BAD_PARAMETER
 *             给定参数之一无效。(One of the given arguments is not valid.)
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *             在不适当的对象上调用操作。(The operation is invoked on an inappropriate object.)
 */
DDS_EXPORT dds_return_t dds_unregister_instance_ih(dds_entity_t writer,
                                                   dds_instance_handle_t handle);

/**
 * @brief 通过实例和时间戳取消注册实例 (Unregisters an instance by instance with timestamp)
 * @ingroup 写入 (writing)
 * @component 数据实例 (data_instance)
 *
 * 此操作与 register instance
 * 操作相反，删除有关实例的所有信息，并从数据写入器中取消注册具有键值的实例。它还为时间戳显式提供了一个值。
 * (This operation reverses the action of register instance, removes all information regarding
 * the instance and unregisters an instance with a key value from the data writer. It also
 * provides a value for the timestamp explicitly.)
 *
 * @param[in]  writer    与实例关联的写入器 (The writer to which instance is associated)
 * @param[in]  data      具有键值的实例 (The instance with the key value)
 * @param[in]  timestamp 注册时使用的时间戳 (The timestamp used at registration)
 *
 * @returns 表示成功或失败的 dds_return_t (A dds_return_t indicating success or failure)
 *
 * @retval DDS_RETCODE_OK
 *             操作成功 (The operation was successful)
 * @retval DDS_RETCODE_BAD_PARAMETER
 *             给定参数之一无效 (One of the given arguments is not valid)
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *             在不适当的对象上调用操作 (The operation is invoked on an inappropriate object)
 */
DDS_EXPORT dds_return_t dds_unregister_instance_ts(dds_entity_t writer,
                                                   const void* data,
                                                   dds_time_t timestamp);

/**
 * @brief 通过实例句柄和时间戳取消注册实例 (Unregisters an instance by instance handle with
 * timestamp)
 * @ingroup 写入 (writing)
 * @component 数据实例 (data_instance)
 *
 * 此操作通过句柄取消注册具有键值的实例。可以从实例句柄中识别实例。如果将未注册的键 ID
 * 作为实例数据传递，则会记录错误，但不会将其标记为返回值。 (This operation unregisters an instance
 * with a key value from the handle. Instance can be identified from instance handle. If an
 * unregistered key ID is passed as an instance data, an error is logged and not flagged as return
 * value.)
 *
 * @param[in]  writer    与实例关联的写入器 (The writer to which instance is associated)
 * @param[in]  handle    实例句柄 (The instance handle)
 * @param[in]  timestamp 注册时使用的时间戳 (The timestamp used at registration)
 *
 * @returns 表示成功或失败的 dds_return_t (A dds_return_t indicating success or failure)
 *
 * @retval DDS_RETCODE_OK
 *             操作成功 (The operation was successful)
 * @retval DDS_RETCODE_BAD_PARAMETER
 *             给定参数之一无效 (One of the given arguments is not valid)
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *             在不适当的对象上调用操作 (The operation is invoked on an inappropriate object)
 */
DDS_EXPORT dds_return_t dds_unregister_instance_ih_ts(dds_entity_t writer,
                                                      dds_instance_handle_t handle,
                                                      dds_time_t timestamp);

/**
 * @brief 该操作用于修改和处理数据实例。
 * @brief This operation modifies and disposes a data instance.
 * @ingroup writing
 * @component write_data
 *
 * 该操作要求数据分发服务修改实例并将其标记为删除。存储在每个连接的读取器中的实例及其相应样本的副本（以及依赖于QoS策略设置的瞬态和持久存储中）将通过将其dds_instance_state_t设置为DDS_IST_NOT_ALIVE_DISPOSED来修改并标记为删除。
 * This operation requests the Data Distribution Service to modify the instance and
 * mark it for deletion. Copies of the instance and its corresponding samples, which are
 * stored in every connected reader and, dependent on the QoS policy settings (also in
 * the Transient and Persistent stores) will be modified and marked for deletion by
 * setting their dds_instance_state_t to DDS_IST_NOT_ALIVE_DISPOSED.
 *
 * @par 阻塞
 * @par Blocking
 * 如果历史记录QoS策略设置为DDS_HISTORY_KEEP_ALL，则在写入器上的dds_writedispose操作可能会阻塞，因为如果修改导致数据丢失，因为资源限制QoS策略中指定的限制之一将被超过。如果通信写入器和读取器的可靠性Qos策略的同步属性值设置为true，则写入器将等待所有同步读取器确认数据。在这种情况下，可靠性QoS策略的max_blocking_time属性配置了dds_writedispose操作可能阻塞的最长时间。
 * If the history QoS policy is set to DDS_HISTORY_KEEP_ALL, the
 * dds_writedispose operation on the writer may block if the modification
 * would cause data to be lost because one of the limits, specified in the
 * resource_limits QoS policy, to be exceeded. In case the synchronous
 * attribute value of the reliability Qos policy is set to true for
 * communicating writers and readers then the writer will wait until
 * all synchronous readers have acknowledged the data. Under these
 * circumstances, the max_blocking_time attribute of the reliability
 * QoS policy configures the maximum time the dds_writedispose operation
 * may block.
 * 如果在写入器能够存储修改而不超过限制并且收到所有预期确认之前，max_blocking_time已过，则dds_writedispose操作将失败并返回DDS_RETCODE_TIMEOUT。
 * If max_blocking_time elapses before the writer is able to store the
 * modification without exceeding the limits and all expected acknowledgements
 * are received, the dds_writedispose operation will fail and returns
 * DDS_RETCODE_TIMEOUT.
 *
 * @param[in]  writer 要从中处理数据实例的写入器。
 * @param[in]  writer The writer to dispose the data instance from.
 * @param[in]  data   要写入和处理的数据。
 * @param[in]  data   The data to be written and disposed.
 *
 * @returns 表示成功或失败的dds_return_t。
 * @returns A dds_return_t indicating success or failure.
 *
 * @retval DDS_RETCODE_OK
 *             样本已写入，实例已标记为删除。
 * @retval DDS_RETCODE_OK
 *             The sample is written and the instance is marked for deletion.
 * @retval DDS_RETCODE_ERROR
 *             发生内部错误。
 * @retval DDS_RETCODE_ERROR
 *             An internal error has occurred.
 * @retval DDS_RETCODE_BAD_PARAMETER
 *             至少有一个参数无效。
 * @retval DDS_RETCODE_BAD_PARAMETER
 *             At least one of the arguments is invalid.
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *             该操作在不合适的对象上调用。
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *             The operation is invoked on an inappropriate object.
 * @retval DDS_RETCODE_ALREADY_DELETED
 *             实体已被删除。
 * @retval DDS_RETCODE_ALREADY_DELETED
 *             The entity has already been deleted.
 * @retval DDS_RETCODE_TIMEOUT
 *             当前操作溢出了可靠性QoS策略、历史记录QoS策略和资源限制QoS策略组合指定的可用资源，或者当前操作正在等待同步读取器对数据传送进行确认。这导致了此操作的阻塞，而在可靠性QoS策略的max_blocking_time过去之前无法解决。
 * @retval DDS_RETCODE_TIMEOUT
 *             Either the current action overflowed the available resources as
 *             specified by the combination of the reliability QoS policy,
 *             history QoS policy and resource_limits QoS policy, or the
 *             current action was waiting for data delivery acknowledgement
 *             by synchronous readers. This caused blocking of this operation,
 *             which could not be resolved before max_blocking_time of the
 *             reliability QoS policy elapsed.
 */
DDS_EXPORT dds_return_t dds_writedispose(dds_entity_t writer, const void* data);

/**
 * @brief 这个操作用特定的时间戳修改和处理数据实例。
 *        This operation modifies and disposes a data instance with a specific
 *        timestamp.
 * @ingroup 写入
 *          writing
 * @component 写入数据
 *            write_data
 *
 * 这个操作执行与 dds_writedispose() 相同的功能，只是应用程序提供了连接到读取器对象的
 * source_timestamp 的值。这个时间戳对于解释 destination_order QoS 策略非常重要。 This operation
 * performs the same functions as dds_writedispose() except that the application provides the value
 * for the source_timestamp that is made available to connected reader objects. This timestamp is
 * important for the interpretation of the destination_order QoS policy.
 *
 * @param[in]  writer    要从中处理数据实例的写入器。
 *                       The writer to dispose the data instance from.
 * @param[in]  data      要写入和处理的数据。
 *                       The data to be written and disposed.
 * @param[in]  timestamp 用作源时间戳的时间戳。
 *                       The timestamp used as source timestamp.
 *
 * @returns 指示成功或失败的 dds_return_t。
 *          A dds_return_t indicating success or failure.
 *
 * @retval DDS_RETCODE_OK
 *             样本已写入，并将实例标记为删除。
 *             The sample is written and the instance is marked for deletion.
 * @retval DDS_RETCODE_ERROR
 *             发生内部错误。
 *             An internal error has occurred.
 * @retval DDS_RETCODE_BAD_PARAMETER
 *             至少有一个参数无效。
 *             At least one of the arguments is invalid.
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *             该操作在不适当的对象上调用。
 *             The operation is invoked on an inappropriate object.
 * @retval DDS_RETCODE_ALREADY_DELETED
 *             实体已被删除。
 *             The entity has already been deleted.
 * @retval DDS_RETCODE_TIMEOUT
 *             当前操作溢出了可靠性 QoS 策略、历史 QoS 策略和资源限制 QoS
 * 策略组合指定的可用资源，或者当前操作正在等待同步读取器的数据传送确认。这导致了此操作的阻塞，在可靠性
 * QoS 策略的 max_blocking_time 过去之前无法解决。 Either the current action overflowed the
 * available resources as specified by the combination of the reliability QoS policy, history QoS
 * policy and resource_limits QoS policy, or the current action was waiting for data delivery
 * acknowledgement by synchronous readers. This caused blocking of this operation, which could not
 * be resolved before max_blocking_time of the reliability QoS policy elapsed.
 */
DDS_EXPORT dds_return_t dds_writedispose_ts(dds_entity_t writer,
                                            const void* data,
                                            dds_time_t timestamp);

/**
 * @brief 该操作通过数据样本来标识并处理一个实例。
 * @ingroup writing
 * @component write_data
 *
 * @brief This operation disposes an instance, identified by the data sample.
 *
 * 此操作要求数据分发服务修改实例并将其标记为删除。存储在每个连接的读取器中的实例及其相应的样本副本，
 * 取决于QoS策略设置（也包括Transient和Persistent存储），将通过将其dds_instance_state_t设置为
 * DDS_IST_NOT_ALIVE_DISPOSED来进行修改和标记删除。
 *
 * This operation requests the Data Distribution Service to modify the instance and
 * mark it for deletion. Copies of the instance and its corresponding samples, which are
 * stored in every connected reader and, dependent on the QoS policy settings (also in
 * the Transient and Persistent stores) will be modified and marked for deletion by
 * setting their dds_instance_state_t to DDS_IST_NOT_ALIVE_DISPOSED.
 *
 * @par 阻塞
 * 如果历史记录QoS策略设置为DDS_HISTORY_KEEP_ALL，则如果修改会导致数据丢失，因为超过了资源限制QoS策略中指定的限制之一，
 * 则在写入器上的dds_writedispose操作可能会阻塞。如果通信写入器和读取器的可靠性Qos策略的同步属性值设置为true，
 * 则写入器将等待所有同步读取器确认数据。在这些情况下，可靠性QoS策略的max_blocking_time属性配置dds_writedispose操作
 * 可能阻塞的最长时间。
 *
 * @par Blocking
 * If the history QoS policy is set to DDS_HISTORY_KEEP_ALL, the
 * dds_writedispose operation on the writer may block if the modification
 * would cause data to be lost because one of the limits, specified in the
 * resource_limits QoS policy, to be exceeded. In case the synchronous
 * attribute value of the reliability Qos policy is set to true for
 * communicating writers and readers then the writer will wait until
 * all synchronous readers have acknowledged the data. Under these
 * circumstances, the max_blocking_time attribute of the reliability
 * QoS policy configures the maximum time the dds_writedispose operation
 * may block.
 *
 * 如果在写入器能够存储修改而不超过限制以及接收到所有预期确认之前，max_blocking_time耗尽，
 * 则dds_writedispose操作将失败并返回DDS_RETCODE_TIMEOUT。
 *
 * If max_blocking_time elapses before the writer is able to store the
 * modification without exceeding the limits and all expected acknowledgements
 * are received, the dds_writedispose operation will fail and returns
 * DDS_RETCODE_TIMEOUT.
 *
 * @param[in]  writer 要从中处理数据实例的写入器。
 * @param[in]  data   标识要处理的实例的数据样本。
 *
 * @param[in]  writer The writer to dispose the data instance from.
 * @param[in]  data   The data sample that identifies the instance
 *                    to be disposed.
 *
 * @returns 表示成功或失败的dds_return_t。
 *
 * @returns A dds_return_t indicating success or failure.
 *
 * @retval DDS_RETCODE_OK
 *             样本已写入，实例被标记为删除。
 * @retval DDS_RETCODE_OK
 *             The sample is written and the instance is marked for deletion.
 * @retval DDS_RETCODE_ERROR
 *             发生内部错误。
 * @retval DDS_RETCODE_ERROR
 *             An internal error has occurred.
 * @retval DDS_RETCODE_BAD_PARAMETER
 *             至少有一个参数无效。
 * @retval DDS_RETCODE_BAD_PARAMETER
 *             At least one of the arguments is invalid.
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *             该操作在不合适的对象上调用。
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *             The operation is invoked on an inappropriate object.
 * @retval DDS_RETCODE_ALREADY_DELETED
 *             实体已经被删除。
 * @retval DDS_RETCODE_ALREADY_DELETED
 *             The entity has already been deleted.
 * @retval DDS_RETCODE_TIMEOUT
 *             当前操作溢出了可靠性QoS策略、历史记录QoS策略和资源限制QoS策略组合所指定的可用资源，
 *             或者当前操作正在等待同步读取器对数据传送的确认。这导致了此操作的阻塞，而在可靠性QoS策略的
 *             max_blocking_time耗尽之前无法解决。
 * @retval DDS_RETCODE_TIMEOUT
 *             Either the current action overflowed the available resources as
 *             specified by the combination of the reliability QoS policy,
 *             history QoS policy and resource_limits QoS policy, or the
 *             current action was waiting for data delivery acknowledgement
 *             by synchronous readers. This caused blocking of this operation,
 *             which could not be resolved before max_blocking_time of the
 *             reliability QoS policy elapsed.
 */
DDS_EXPORT dds_return_t dds_dispose(dds_entity_t writer, const void* data);

/**
 * @brief 该操作使用特定的时间戳处理由数据样本标识的实例。
 *        This operation disposes an instance with a specific timestamp, identified by the data
 *        sample.
 * @ingroup writing
 * @component write_data
 *
 * 该操作执行与 dds_dispose() 相同的功能，只是应用程序提供了连接到读取器对象的源时间戳的值。
 * This operation performs the same functions as dds_dispose() except that
 * the application provides the value for the source_timestamp that is made
 * available to connected reader objects. This timestamp is important for the
 * interpretation of the destination_order QoS policy.
 *
 * @param[in]  writer    要从中处理数据实例的写入器。
 *                       The writer to dispose the data instance from.
 * @param[in]  data      标识要处理的实例的数据样本。
 *                       The data sample that identifies the instance
 *                       to be disposed.
 * @param[in]  timestamp 用作源时间戳的时间戳。
 *                       The timestamp used as source timestamp.
 *
 * @returns 表示成功或失败的 dds_return_t。
 *          A dds_return_t indicating success or failure.
 *
 * @retval DDS_RETCODE_OK
 *             样本已写入，实例被标记为删除。
 *             The sample is written and the instance is marked for deletion
 * @retval DDS_RETCODE_ERROR
 *             发生内部错误。
 *             An internal error has occurred
 * @retval DDS_RETCODE_BAD_PARAMETER
 *             至少有一个参数无效。
 *             At least one of the arguments is invalid
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *             该操作在不适当的对象上调用。
 *             The operation is invoked on an inappropriate object
 * @retval DDS_RETCODE_ALREADY_DELETED
 *             实体已被删除。
 *             The entity has already been deleted
 * @retval DDS_RETCODE_TIMEOUT
 *             当前操作溢出了可靠性 QoS 策略、历史 QoS 策略和资源限制 QoS 策略组合指定的可用资源，
 *             或者当前操作正在等待同步读取器的数据传输确认。这导致此操作阻塞，无法在可靠性 QoS
 *             策略的 max_blocking_time 消耗之前解决。
 *             Either the current action overflowed the available resources as
 *             specified by the combination of the reliability QoS policy,
 *             history QoS policy and resource_limits QoS policy, or the
 *             current action was waiting for data delivery acknowledgment
 *             by synchronous readers. This caused blocking of this operation,
 *             which could not be resolved before max_blocking_time of the
 *             reliability QoS policy elapsed.
 */
DDS_EXPORT dds_return_t dds_dispose_ts(dds_entity_t writer, const void* data, dds_time_t timestamp);

/**
 * @brief 此操作通过实例句柄处理一个实例。
 * @ingroup 写入
 * @component 写入数据
 *
 * 该操作请求数据分发服务修改实例并将其标记为删除。实例及其对应的样本副本，
 * 存储在每个连接的读取器中，并根据QoS策略设置（也包括在Transient和Persistent存储中）进行修改，
 * 并通过将dds_instance_state_t设置为DDS_IST_NOT_ALIVE_DISPOSED来标记为删除。
 *
 * @brief This operation disposes an instance, identified by the instance handle.
 * @ingroup writing
 * @component write_data
 *
 * This operation requests the Data Distribution Service to modify the instance and
 * mark it for deletion. Copies of the instance and its corresponding samples, which are
 * stored in every connected reader and, dependent on the QoS policy settings (also in
 * the Transient and Persistent stores) will be modified and marked for deletion by
 * setting their dds_instance_state_t to DDS_IST_NOT_ALIVE_DISPOSED.
 *
 * @par 实例句柄
 * 给定的实例句柄必须与dds_register_instance操作、dds_register_instance_ts或dds_lookup_instance返回的值相对应。
 * 如果没有对应关系，那么操作的结果是不确定的。
 *
 * @par Instance Handle
 * The given instance handle must correspond to the value that was returned by either
 * the dds_register_instance operation, dds_register_instance_ts or dds_lookup_instance.
 * If there is no correspondence, then the result of the operation is unspecified.
 *
 * @param[in]  writer 要从中处理数据实例的写入器。
 * @param[in]  handle 用于标识实例的句柄。
 *
 * @param[in]  writer The writer to dispose the data instance from.
 * @param[in]  handle The handle to identify an instance.
 *
 * @returns 表示成功或失败的dds_return_t。
 * @returns A dds_return_t indicating success or failure.
 *
 * @retval DDS_RETCODE_OK
 *             样本已写入，实例被标记为删除。
 * @retval DDS_RETCODE_OK
 *             The sample is written and the instance is marked for deletion.
 * @retval DDS_RETCODE_ERROR
 *             发生内部错误。
 * @retval DDS_RETCODE_ERROR
 *             An internal error has occurred.
 * @retval DDS_RETCODE_BAD_PARAMETER
 *             至少有一个参数无效。
 * @retval DDS_RETCODE_BAD_PARAMETER
 *             At least one of the arguments is invalid.
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *             该操作在不适当的对象上调用。
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *             The operation is invoked on an inappropriate object.
 * @retval DDS_RETCODE_ALREADY_DELETED
 *             实体已经被删除。
 * @retval DDS_RETCODE_ALREADY_DELETED
 *             The entity has already been deleted.
 * @retval DDS_RETCODE_PRECONDITION_NOT_MET
 *             实例句柄尚未在此写入器中注册。
 * @retval DDS_RETCODE_PRECONDITION_NOT_MET
 *             The instance handle has not been registered with this writer.
 */
DDS_EXPORT dds_return_t dds_dispose_ih(dds_entity_t writer, dds_instance_handle_t handle);

/**
 * @brief 该操作使用特定的时间戳处理由实例句柄标识的实例。
 *        This operation disposes an instance with a specific timestamp, identified by the instance
 * handle.
 * @ingroup writing
 * @component write_data
 *
 * 该操作执行与 dds_dispose_ih()
 * 相同的功能，只是应用程序提供了连接到读取器对象的源时间戳的值。这个时间戳对于目标顺序 QoS
 * 策略的解释非常重要。 This operation performs the same functions as dds_dispose_ih() except that
 * the application provides the value for the source_timestamp that is made
 * available to connected reader objects. This timestamp is important for the
 * interpretation of the destination_order QoS policy.
 *
 * @param[in]  writer    要从中处理数据实例的写入器。
 *                       The writer to dispose the data instance from.
 * @param[in]  handle    用于标识实例的句柄。
 *                       The handle to identify an instance.
 * @param[in]  timestamp 用作源时间戳的时间戳。
 *                       The timestamp used as source timestamp.
 *
 * @returns 表示成功或失败的 dds_return_t。
 *          A dds_return_t indicating success or failure.
 *
 * @retval DDS_RETCODE_OK
 *             样本已写入，实例被标记为删除。
 *             The sample is written and the instance is marked for deletion.
 * @retval DDS_RETCODE_ERROR
 *             发生内部错误。
 *             An internal error has occurred.
 * @retval DDS_RETCODE_BAD_PARAMETER
 *             至少有一个参数无效。
 *             At least one of the arguments is invalid.
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *             操作在不适当的对象上调用。
 *             The operation is invoked on an inappropriate object.
 * @retval DDS_RETCODE_ALREADY_DELETED
 *             实体已被删除。
 *             The entity has already been deleted.
 * @retval DDS_RETCODE_PRECONDITION_NOT_MET
 *             实例句柄尚未在此写入器中注册。
 *             The instance handle has not been registered with this writer.
 */
DDS_EXPORT dds_return_t dds_dispose_ih_ts(dds_entity_t writer,
                                          dds_instance_handle_t handle,
                                          dds_time_t timestamp);

/**
 * @brief 写入数据实例的值
 *        Write the value of a data instance
 * @ingroup writing
 * @component write_data
 *
 * 使用此 API，源时间戳的值将由服务自动提供给数据读取器。
 * With this API, the value of the source timestamp is automatically made
 * available to the data reader by the service.
 *
 * @param[in]  writer 写入器实体。
 *                   The writer entity.
 * @param[in]  data 要写入的值。
 *                 Value to be written.
 *
 * @returns 表示成功或失败的 dds_return_t。
 *          dds_return_t indicating success or failure.
 */
DDS_EXPORT dds_return_t dds_write(dds_entity_t writer, const void* data);

/**
 * @brief 清空一个 writer 的批量写入缓存 (Flush a writer's batched writes)
 * @ingroup 写入 (writing)
 * @component 写入数据 (write_data)
 *
 * 当使用 WriteBatch 模式时，您可以手动将小的写入操作批量处理为较大的数据包以提高网络效率。
 * 通常的 dds_write() 调用将不再自动决定何时发送数据，而是通过这个函数手动完成。
 *
 * DOC_TODO 检查关于此功能如何工作的假设是否正确
 *
 * @param[in]  writer 写入实体 (The writer entity).
 */
DDS_EXPORT void dds_write_flush(dds_entity_t writer);

/**
 * @brief 将数据实例的序列化值写入 (Write a serialized value of a data instance)
 * @ingroup 写入 (writing)
 * @component 写入数据 (write_data)
 *
 * 此调用使 writer 将 serdata 参数中提供的序列化值写入。Timestamp 和 statusinfo
 * 字段分别设置为当前时间和 0（表示常规写入）。
 *
 * @param[in]  writer 写入实体 (The writer entity).
 * @param[in]  serdata 要写入的序列化值 (Serialized value to be written).
 *
 * @returns 表示成功或失败的 dds_return_t (A dds_return_t indicating success or failure).
 *
 * @retval DDS_RETCODE_OK
 *             写入者成功地写入了序列化值 (The writer successfully wrote the serialized value).
 * @retval DDS_RETCODE_ERROR
 *             发生内部错误 (An internal error has occurred).
 * @retval DDS_RETCODE_BAD_PARAMETER
 *             给定的参数无效 (One of the given arguments is not valid).
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *             在不适当的对象上调用了操作 (The operation is invoked on an inappropriate object).
 * @retval DDS_RETCODE_ALREADY_DELETED
 *             实体已被删除 (The entity has already been deleted).
 * @retval DDS_RETCODE_TIMEOUT
 *             在指定的 max_blocking_time 内，写入者未能可靠地写入序列化值 (The writer failed to
 * write the serialized value reliably within the specified max_blocking_time).
 */
DDS_EXPORT dds_return_t dds_writecdr(dds_entity_t writer, struct ddsi_serdata* serdata);

/**
 * @brief 写入数据实例的序列化值 (Write a serialized value of a data instance)
 * @ingroup 写入 (writing)
 * @component 写入数据 (write_data)
 *
 * 此调用使写入器将提供的serdata参数中的序列化值写入。时间戳和状态信息保持不变。
 * (This call causes the writer to write the serialized value that is provided
 * in the serdata argument. Timestamp and statusinfo are used as is.)
 *
 * @param[in]  writer 写入器实体 (The writer entity.)
 * @param[in]  serdata 要写入的序列化值 (Serialized value to be written.)
 *
 * @returns 表示成功或失败的dds_return_t (A dds_return_t indicating success or failure.)
 *
 * @retval DDS_RETCODE_OK
 *             写入器成功写入序列化值 (The writer successfully wrote the serialized value.)
 * @retval DDS_RETCODE_ERROR
 *             发生内部错误 (An internal error has occurred.)
 * @retval DDS_RETCODE_BAD_PARAMETER
 *             给定参数之一无效 (One of the given arguments is not valid.)
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *             在不适当的对象上调用操作 (The operation is invoked on an inappropriate object.)
 * @retval DDS_RETCODE_ALREADY_DELETED
 *             实体已被删除 (The entity has already been deleted.)
 * @retval DDS_RETCODE_TIMEOUT
 *             写入器未能在指定的最大阻塞时间内可靠地写入序列化值
 *             (The writer failed to write the serialized value reliably within the specified
 * max_blocking_time.)
 */
DDS_EXPORT dds_return_t dds_forwardcdr(dds_entity_t writer, struct ddsi_serdata* serdata);

/**
 * @brief 写入数据实例的值以及传递的源时间戳 (Write the value of a data instance along with the
 * source timestamp passed.)
 * @ingroup 写入 (writing)
 * @component 写入数据 (write_data)
 *
 * @param[in]  writer 写入器实体 (The writer entity.)
 * @param[in]  data 要写入的值 (Value to be written.)
 * @param[in]  timestamp 源时间戳 (Source timestamp.)
 *
 * @returns 表示成功或失败的dds_return_t (A dds_return_t indicating success or failure.)
 */
DDS_EXPORT dds_return_t dds_write_ts(dds_entity_t writer, const void* data, dds_time_t timestamp);

/**
 * @defgroup readcondition (ReadCondition)
 * @ingroup condition
 */
/**
 * @defgroup querycondition (QueryCondition)
 * @ingroup condition
 */
/**
 * @defgroup guardcondition (GuardCondition)
 * @ingroup condition
 */

/**
 * @brief 创建与给定读取器关联的readcondition。
 * @brief Creates a readcondition associated to the given reader.
 * @ingroup readcondition
 * @component data_query
 *
 * readcondition允许通过掩码指定在数据读取器历史记录中感兴趣的样本。
 * The readcondition allows specifying which samples are of interest in
 * a data reader's history, by means of a mask.
 * 掩码与dds_sample_state_t、dds_view_state_t和dds_instance_state_t的标志进行或操作。
 * The mask is or'd with the flags that are dds_sample_state_t, dds_view_state_t and
 * dds_instance_state_t.
 *
 * 根据设置的掩码值，当读取器上有可用数据时，readcondition会被触发。
 * Based on the mask value set, the readcondition gets triggered when
 * data is available on the reader.
 *
 * Waitsets允许等待一组任意实体上的某个事件。
 * Waitsets allow waiting for an event on some of any set of entities.
 * 这意味着当具有与给定掩码匹配的状态的数据位于读取器历史记录中时，readcondition可用于唤醒waitset。
 * This means that the readcondition can be used to wake up a waitset when
 * data is in the reader history with states that matches the given mask.
 *
 * @note 父读取器及其所有关联的条件（无论是readconditions还是queryconditions）共享相同的资源。
 * @note The parent reader and every of its associated conditions (whether
 *       they are readconditions or queryconditions) share the same resources.
 *       这意味着这些实体中的一个读取或获取数据时，数据的状态将自动更改为其他实体。
 *       This means that one of these entities reads or takes data, the states
 *       of the data will change for other entities automatically. For instance,
 *       如果一个实体读取了一个样本，那么对于所有关联的读取器/条件，样本状态都会变为“已读”。
 *       if one reads a sample, then the sample state will become 'read' for all
 *       associated reader/conditions.
 * 或者，如果一个实体获取了一个样本，那么它对任何其他关联的读取器/条件都不可用。 Or if one takes a
 * sample, then it's not available to any other associated reader/condition.
 *
 * @param[in]  reader  要关联条件的读取器。
 * @param[in]  reader  Reader to associate the condition to.
 * @param[in]  mask    兴趣（dds_sample_state_t|dds_view_state_t|dds_instance_state_t）。
 * @param[in]  mask    Interest (dds_sample_state_t|dds_view_state_t|dds_instance_state_t).
 *
 * @returns 有效的条件句柄或错误代码。
 * @returns A valid condition handle or an error code.
 *
 * @retval >0
 *             有效的条件句柄
 * @retval >0
 *             A valid condition handle
 * @retval DDS_RETCODE_ERROR
 *             发生内部错误。
 * @retval DDS_RETCODE_ERROR
 *             An internal error has occurred.
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *             操作在不适当的对象上调用。
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *             The operation is invoked on an inappropriate object.
 * @retval DDS_RETCODE_ALREADY_DELETED
 *             实体已被删除。
 * @retval DDS_RETCODE_ALREADY_DELETED
 *             The entity has already been deleted.
 */
DDS_EXPORT dds_entity_t dds_create_readcondition(dds_entity_t reader, uint32_t mask);

/**
 * @brief 查询条件过滤器函数签名 (Function signature for a querycondition filter)
 * @ingroup querycondition
 */
typedef bool (*dds_querycondition_filter_fn)(const void* sample);

/**
 * @brief 为给定的读取器创建一个查询条件 (Creates a queryondition associated to the given reader.)
 * @ingroup querycondition
 * @component data_query
 *
 * 查询条件允许通过掩码和过滤器指定数据读取器历史中感兴趣的样本。(The queryondition allows
 * specifying which samples are of interest in a data reader's history, by means of a mask and a
 * filter. The mask is or'd with the flags that are dds_sample_state_t, dds_view_state_t and
 * dds_instance_state_t.)
 *
 * 根据设置的掩码值和与过滤器匹配的数据，当读取器上有可用数据时，查询条件将被触发。(Based on the
 * mask value set and data that matches the filter, the querycondition gets triggered when data is
 * available on the reader.)
 *
 * 等待集允许在任意一组实体上等待某个事件。(Waitsets allow waiting for an event on some of any set
 * of entities. This means that the querycondition can be used to wake up a waitset when data is in
 * the reader history with states that matches the given mask and filter.)
 *
 * @note 父读取器及其所有关联的条件（无论是读取条件还是查询条件）共享相同的资源。(The parent reader
 * and every of its associated conditions (whether they are readconditions or queryconditions) share
 * the same resources. This means that one of these entities reads or takes data, the states of the
 * data will change for other entities automatically. For instance, if one reads a sample, then the
 * sample state will become 'read' for all associated reader/conditions. Or if one takes a sample,
 * then it's not available to any other associated reader/condition.)
 *
 * @param[in]  reader  要关联条件的读取器 (Reader to associate the condition to.)
 * @param[in]  mask    兴趣掩码 (Interest
 * (dds_sample_state_t|dds_view_state_t|dds_instance_state_t).)
 * @param[in]  filter  应用程序可以使用的回调以过滤特定样本 (Callback that the application can use
 * to filter specific samples.)
 *
 * @returns 有效的条件句柄或错误代码 (A valid condition handle or an error code)
 *
 * @retval >=0
 *             有效的条件句柄 (A valid condition handle.)
 * @retval DDS_RETCODE_ERROR
 *             发生内部错误 (An internal error has occurred.)
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *             操作在不适当的对象上调用 (The operation is invoked on an inappropriate object.)
 * @retval DDS_RETCODE_ALREADY_DELETED
 *             实体已被删除 (The entity has already been deleted.)
 */
DDS_EXPORT dds_entity_t dds_create_querycondition(dds_entity_t reader,
                                                  uint32_t mask,
                                                  dds_querycondition_filter_fn filter);

/**
 * @brief 创建一个守卫条件。 (Creates a guardcondition.)
 * @ingroup guardcondition
 * @component guard_condition
 *
 * Waitsets 允许在一组实体中的某些或任意实体上等待事件。
 * 这意味着当读者历史中的数据状态与给定掩码匹配时，可以使用守卫条件唤醒 waitset。
 * (Waitsets allow waiting for an event on some of any set of entities.
 * This means that the guardcondition can be used to wake up a waitset when
 * data is in the reader history with states that matches the given mask.)
 *
 * @param[in]   participant  要在其上创建守卫条件的参与者。 (Participant on which to create the
 * guardcondition.)
 *
 * @returns 有效的条件句柄或错误代码。 (A valid condition handle or an error code.)
 *
 * @retval >0
 *             有效的条件句柄 (A valid condition handle)
 * @retval DDS_RETCODE_ERROR
 *             发生内部错误。 (An internal error has occurred.)
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *             在不适当的对象上调用操作。 (The operation is invoked on an inappropriate object.)
 * @retval DDS_RETCODE_ALREADY_DELETED
 *             实体已被删除。 (The entity has already been deleted.)
 */
DDS_EXPORT dds_entity_t dds_create_guardcondition(dds_entity_t participant);

/**
 * @brief 设置守卫条件的触发状态。 (Sets the trigger status of a guardcondition.)
 * @ingroup guardcondition
 * @component guard_condition
 *
 * @param[in]   guardcond  要设置触发状态的守卫条件。 (Guard condition to set the trigger status
 * of.)
 * @param[in]   triggered  要设置的触发状态。 (The triggered status to set.)
 *
 * @retval DDS_RETCODE_OK
 *             操作成功 (Operation successful)
 * @retval DDS_RETCODE_ERROR
 *             发生内部错误。 (An internal error has occurred.)
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *             在不适当的对象上调用操作。 (The operation is invoked on an inappropriate object.)
 * @retval DDS_RETCODE_ALREADY_DELETED
 *             实体已被删除。 (The entity has already been deleted.)
 */
DDS_EXPORT dds_return_t dds_set_guardcondition(dds_entity_t guardcond, bool triggered);

/**
 * @brief 读取守卫条件的触发状态。 (Reads the trigger status of a guardcondition.)
 * @ingroup guardcondition
 * @component guard_condition
 *
 * @param[in]   guardcond  要读取触发状态的守卫条件。 (Guard condition to read the trigger status
 * of.)
 * @param[out]  triggered  从守卫条件中读取的触发状态。 (The triggered status read from the guard
 * condition.)
 *
 * @retval DDS_RETCODE_OK
 *             操作成功 (Operation successful)
 * @retval DDS_RETCODE_ERROR
 *             发生内部错误。 (An internal error has occurred.)
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *             在不适当的对象上调用操作。 (The operation is invoked on an inappropriate object.)
 * @retval DDS_RETCODE_ALREADY_DELETED
 *             实体已被删除。 (The entity has already been deleted.)
 */
DDS_EXPORT dds_return_t dds_read_guardcondition(dds_entity_t guardcond, bool* triggered);

/**
 * @brief 读取并重置 guardcondition 的触发状态。 (Reads and resets the trigger status of a
 * guardcondition.)
 * @ingroup guardcondition
 * @component guard_condition
 *
 * @param[in]   guardcond  要读取和重置触发状态的 Guard condition。 (Guard condition to read and
 * reset the trigger status of.)
 * @param[out]  triggered  从 Guard condition 读取的触发状态。 (The triggered status read from the
 * guard condition.)
 *
 * @retval DDS_RETCODE_OK
 *             操作成功 (Operation successful)
 * @retval DDS_RETCODE_ERROR
 *             发生内部错误 (An internal error has occurred.)
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *             在不适当的对象上调用操作 (The operation is invoked on an inappropriate object.)
 * @retval DDS_RETCODE_ALREADY_DELETED
 *             实体已被删除 (The entity has already been deleted.)
 */
DDS_EXPORT dds_return_t dds_take_guardcondition(dds_entity_t guardcond, bool* triggered);

/**
 * @defgroup waitset (WaitSet)
 * @ingroup dds
 */

/**
 * @brief Waitset 附件参数。 (Waitset attachment argument.)
 * @ingroup waitset
 *
 * 附加到 waitset 的每个实体都可以伴随这样一个附件参数。
 * 当 waitset 等待由于实体触发而解除阻塞时，返回的数组将填充与触发实体相关的这些附件参数。
 * (Every entity that is attached to the waitset can be accompanied by such
 * an attachment argument. When the waitset wait is unblocked because of an
 * entity that triggered, then the returning array will be populated with
 * these attachment arguments that are related to the triggered entity.)
 */
typedef intptr_t dds_attach_t;

/**
 * @brief 创建 waitset 并分配所需资源 (Create a waitset and allocate the resources required)
 * @ingroup waitset
 * @component waitset
 *
 * WaitSet 对象允许应用程序等待，直到附加实体的一个或多个条件评估为 TRUE 或超时过期。
 * (A WaitSet object allows an application to wait until one or more of the
 * conditions of the attached entities evaluates to TRUE or until the timeout
 * expires.)
 *
 * @param[in]  participant  包含的域参与者。 (Domain participant which the WaitSet contains.)
 *
 * @returns 有效的 waitset 句柄或错误代码。 (A valid waitset handle or an error code.)
 *
 * @retval >=0
 *             有效的 waitset 句柄。 (A valid waitset handle.)
 * @retval DDS_RETCODE_ERROR
 *             发生内部错误。 (An internal error has occurred.)
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *             在不适当的对象上调用操作。 (The operation is invoked on an inappropriate object.)
 * @retval DDS_RETCODE_ALREADY_DELETED
 *             实体已被删除。 (The entity has already been deleted.)
 */
DDS_EXPORT dds_entity_t dds_create_waitset(dds_entity_t participant);

/**
 * @brief 获取先前附加的实体。 (Acquire previously attached entities.)
 * @ingroup waitset
 * @component waitset
 *
 * 此函数接受一个预分配的列表来存放实体，并返回找到的实体数量。
 * (This function takes a pre-allocated list to put the entities in and will return the number of
 * found entities.) 给定的列表大小可能与找到的实体数量不同。 (It is possible that the given size of
 * the list is not the same as the number of found entities.)
 * 如果找到的实体较少，则列表中的最后几个条目保持不变。
 * (If less entities are found, then the last few entries in the list are untouched.)
 * 当找到更多的实体时，仅将 'size' 数量的条目插入列表，但仍返回找到的实体的完整计数。
 * (When more entities are found, then only 'size' number of entries are inserted into the list, but
 * still the complete count of the found entities is returned.)
 * 在后一种情况下返回哪些实体是未定义的。(Which entities are returned in the latter case is
 * undefined.)
 *
 * @param[in]  waitset  从其中获取其附加实体的 Waitset。(Waitset from which to get its attached
 * entities.)
 * @param[out] entities 包含找到的实体的预分配数组。(Pre-allocated array to contain the found
 * entities.)
 * @param[in]  size     预分配实体列表的大小。(Size of the pre-allocated entities' list.)
 *
 * @returns 返回具有子项数量或错误代码的 dds_return_t。(A dds_return_t with the number of children
 * or an error code.)
 *
 * @retval >=0
 *             找到的子项数量（可以大于 'size'）(Number of children found (can be larger than
 * 'size').)
 * @retval DDS_RETCODE_ERROR
 *             发生内部错误。(An internal error has occurred.)
 * @retval DDS_RETCODE_BAD_PARAMETER
 *             实体参数为 NULL，同时提供了大小。(The entities parameter is NULL, while a size is
 * provided.)
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *             该操作在不适当的对象上调用。(The operation is invoked on an inappropriate object.)
 * @retval DDS_RETCODE_ALREADY_DELETED
 *             Waitset 已被删除。(The waitset has already been deleted.)
 */
DDS_EXPORT dds_return_t dds_waitset_get_entities(dds_entity_t waitset,
                                                 dds_entity_t* entities,
                                                 size_t size);

/**
 * @brief 将实体附加到 WaitSet。 (This operation attaches an Entity to the WaitSet.)
 * @ingroup waitset
 * @component waitset
 *
 * 此操作将实体附加到 WaitSet。 当没有附加的实体被触发时，dds_waitset_wait() 将阻塞。
 * (This operation attaches an Entity to the WaitSet. The dds_waitset_wait() will block when none of
 * the attached entities are triggered.)
 * 对于每个实体，“触发”（dds_triggered()）并不意味着相同的含义：
 * ('Triggered' (dds_triggered()) doesn't mean the same for every entity:)
 *  - Reader/Writer/Publisher/Subscriber/Topic/Participant
 *      - 当它们的状态发生变化时，这些实体会被触发。(These are triggered when their status changed.)
 *  - WaitSet
 *      - 当应用程序将触发值设置为 true 时触发。 直到应用程序将触发值设置为
 * false（dds_waitset_set_trigger()），它将保持触发状态。 (Triggered when trigger value was set to
 * true by the application. It stays triggered until application sets the trigger value to false
 * (dds_waitset_set_trigger()).) 这可以用于唤醒 waitset
 * 的其他原因（例如终止）而不是“正常”的状态更改（如新数据）。 (This can be used to wake up a waitset
 * for different reasons (f.i. termination) than the 'normal' status change (like new data).)
 *  - ReadCondition/QueryCondition
 *      - 当与 Condition 匹配的相关 Reader 上有可用数据时触发。
 *        (Triggered when data is available on the related Reader that matches the Condition.)
 *
 * 多个实体可以附加到单个 waitset。特定实体可以附加到多个 waitset。
 * (Multiple entities can be attached to a single waitset. A particular entity can be attached to
 * multiple waitsets.) 但是，特定实体不能多次附加到特定 waitset。 (However, a particular entity
 * cannot be attached to a particular waitset multiple times.)
 *
 * @param[in]  waitset  将给定实体附加到的 waitset。(The waitset to attach the given entity to.)
 * @param[in]  entity   要附加的实体。(The entity to attach.)
 * @param[in]  x        当 waitset 等待由给定实体触发时提供的 Blob。(Blob that will be supplied when
 * the waitset wait is triggered by the given entity.)
 *
 * @returns 指示成功或失败的 dds_return_t。(A dds_return_t indicating success or failure.)
 *
 * @retval DDS_RETCODE_OK
 *             实体已附加。(Entity attached.)
 * @retval DDS_RETCODE_ERROR
 *             发生内部错误。(An internal error has occurred.)
 * @retval DDS_RETCODE_BAD_PARAMETER
 *             给定的 waitset 或实体无效。(The given waitset or entity are not valid.)
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *             该操作在不适当的对象上调用。(The operation is invoked on an inappropriate object.)
 * @retval DDS_RETCODE_ALREADY_DELETED
 *             Waitset 已被删除。(The waitset has already been deleted.)
 * @retval DDS_RETCODE_PRECONDITION_NOT_MET
 *             实体已经附加了。(The entity was already attached.)
 */
DDS_EXPORT dds_return_t dds_waitset_attach(dds_entity_t waitset,
                                           dds_entity_t entity,
                                           dds_attach_t x);

/**
 * @brief 此操作将实体从 WaitSet 中分离。
 * @brief This operation detaches an Entity from the WaitSet.
 * @ingroup waitset
 * @component waitset
 *
 * @param[in]  waitset  要从中分离给定实体的 waitset。
 * @param[in]  waitset  The waitset to detach the given entity from.
 * @param[in]  entity   要分离的实体。
 * @param[in]  entity   The entity to detach.
 *
 * @returns 表示成功或失败的 dds_return_t。
 * @returns A dds_return_t indicating success or failure.
 *
 * @retval DDS_RETCODE_OK
 *             实体已分离。
 * @retval DDS_RETCODE_OK
 *             Entity detached.
 * @retval DDS_RETCODE_ERROR
 *             发生内部错误。
 * @retval DDS_RETCODE_ERROR
 *             An internal error has occurred.
 * @retval DDS_RETCODE_BAD_PARAMETER
 *             给定的 waitset 或实体无效。
 * @retval DDS_RETCODE_BAD_PARAMETER
 *             The given waitset or entity are not valid.
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *             在不适当的对象上调用操作。
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *             The operation is invoked on an inappropriate object.
 * @retval DDS_RETCODE_ALREADY_DELETED
 *             waitset 已被删除。
 * @retval DDS_RETCODE_ALREADY_DELETED
 *             The waitset has already been deleted.
 * @retval DDS_RETCODE_PRECONDITION_NOT_MET
 *             实体未连接。
 * @retval DDS_RETCODE_PRECONDITION_NOT_MET
 *             The entity is not attached.
 */
DDS_EXPORT dds_return_t dds_waitset_detach(dds_entity_t waitset, dds_entity_t entity);

/**
 * @brief 设置与 waitset 关联的触发器值。
 * @brief Sets the trigger_value associated with a waitset.
 * @ingroup waitset
 * @component waitset
 *
 * 当 waitset 附加到其自身并且触发器值设置为“true”时，waitset 将像其他附加实体的状态更改一样唤醒。
 * When the waitset is attached to itself and the trigger value is
 * set to 'true', then the waitset will wake up just like with an
 * other status change of the attached entities.
 *
 * 这可用于强制唤醒 waitset，例如应用程序希望关闭时。因此，当触发器值为 true 时，waitset
 * 将唤醒或根本不等待。 This can be used to forcefully wake up a waitset, for instance when the
 * application wants to shut down. So, when the trigger value is true, the waitset will wake up or
 * not wait at all.
 *
 * 触发器值将保持为 true，直到应用程序再次故意将其设置为 false。
 * The trigger value will remain true until the application sets it
 * false again deliberately.
 *
 * @param[in]  waitset  要在其上设置触发器值的 waitset。
 * @param[in]  waitset  The waitset to set the trigger value on.
 * @param[in]  trigger  要设置的触发器值。
 * @param[in]  trigger  The trigger value to set.
 *
 * @returns 表示成功或失败的 dds_return_t。
 * @returns A dds_return_t indicating success or failure.
 *
 * @retval DDS_RETCODE_OK
 *             触发器值已设置。
 * @retval DDS_RETCODE_OK
 *             Trigger value set.
 * @retval DDS_RETCODE_ERROR
 *             发生内部错误。
 * @retval DDS_RETCODE_ERROR
 *             An internal error has occurred.
 * @retval DDS_RETCODE_BAD_PARAMETER
 *             给定的 waitset 无效。
 * @retval DDS_RETCODE_BAD_PARAMETER
 *             The given waitset is not valid.
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *             在不适当的对象上调用操作。
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *             The operation is invoked on an inappropriate object.
 * @retval DDS_RETCODE_ALREADY_DELETED
 *             waitset 已被删除。
 * @retval DDS_RETCODE_ALREADY_DELETED
 *             The waitset has already been deleted.
 */
DDS_EXPORT dds_return_t dds_waitset_set_trigger(dds_entity_t waitset, bool trigger);

/**
 * @brief 该操作允许应用程序线程等待附加到 WaitSet 的实体上的状态更改或其他触发器。
 *        This operation allows an application thread to wait for a status
 *        change or other trigger on (one of) the entities that are attached to
 *        the WaitSet.
 * @ingroup waitset
 * @component waitset
 *
 * dds_waitset_wait() 操作阻塞，直到已附加实体中的一些实体触发或 "reltimeout" 超时。
 * The dds_waitset_wait() operation blocks until some of the attached
 * entities have triggered or "reltimeout" has elapsed.
 * '触发'（dds_triggered()）对于每个实体并不意味着相同：
 * 'Triggered' (dds_triggered()) doesn't mean the same for every entity:
 *  - Reader/Writer/Publisher/Subscriber/Topic/Participant
 *      - 当它们的状态发生变化时，会触发这些实体。
 *      - These are triggered when their status changed.
 *  - WaitSet
 *      - 当应用程序将触发值设置为 true 时触发。在应用程序将触发值设置为
 *        false（dds_waitset_set_trigger()）之前，它将保持触发状态。这可以用于唤醒
 *        等待集合的不同原因（如终止），而不是“正常”的状态更改（如新数据）。
 *      - Triggered when trigger value was set to true by the application.
 *        It stays triggered until application sets the trigger value to
 *        false (dds_waitset_set_trigger()). This can be used to wake up a
 *        waitset for different reasons (f.i. termination) than the 'normal'
 *        status change (like new data).
 *  - ReadCondition/QueryCondition
 *      - 当与条件匹配的相关 Reader 上有可用数据时触发。
 *      - Triggered when data is available on the related Reader that matches
 *        the Condition.
 *
 * 此函数接受一个预分配的列表，将“xs”块放入其中（在附加相关实体期间提供），并返回
 * 触发实体的数量。给定列表的大小可能与触发实体的数量不同。如果触发的实体较少，
 * 则列表中的最后几个条目保持不变。当触发更多实体时，仅将“size”数量的条目插入到
 * 列表中，但仍返回触发实体的完整计数。在后一种情况下返回哪些“xs”块是未定义的。
 * This function takes a pre-allocated list to put the "xs" blobs in (that
 * were provided during the attach of the related entities) and will return
 * the number of triggered entities. It is possible that the given size
 * of the list is not the same as the number of triggered entities. If less
 * entities were triggered, then the last few entries in the list are
 * untouched. When more entities are triggered, then only 'size' number of
 * entries are inserted into the list, but still the complete count of the
 * triggered entities is returned. Which "xs" blobs are returned in the
 * latter case is undefined.
 *
 * 如果发生超时，返回值为 0。
 * In case of a time out, the return value is 0.
 *
 * 在应用程序被阻塞时删除等待集将导致错误代码（即“wait”返回的值小于 0）。
 * Deleting the waitset while the application is blocked results in an
 * error code (i.e. < 0) returned by "wait".
 *
 * 多个线程可以同时在单个等待集上阻塞；调用是完全独立的。
 * Multiple threads may block on a single waitset at the same time;
 * the calls are entirely independent.
 *
 * 空的等待集永远不会触发（即，在空的等待集上的 dds_waitset_wait 实际上相当于睡眠）。
 * An empty waitset never triggers (i.e., dds_waitset_wait on an empty
 * waitset is essentially equivalent to a sleep).
 *
 * “dds_waitset_wait_until”操作与“dds_waitset_wait”相同，只是它采用绝对超时。
 * The "dds_waitset_wait_until" operation is the same as the
 * "dds_waitset_wait" except that it takes an absolute timeout.
 *
 * @param[in]  waitset    要在其上设置触发值的等待集。
 *                        The waitset to set the trigger value on.
 * @param[out] xs         预分配的列表，用于存储在触发实体的附加过程中提供的“blobs”。
 *                        Pre-allocated list to store the 'blobs' that were
 *                        provided during the attach of the triggered entities.
 * @param[in]  nxs        预分配 blobs 列表的大小。
 *                        The size of the pre-allocated blobs list.
 * @param[in]  reltimeout 相对超时
 *                        Relative timeout
 *
 * @returns 带有触发实体数量或错误代码的 dds_return_t
 *          A dds_return_t with the number of entities triggered or an error code
 *
 * @retval >0
 *             触发的实体数量。
 *             Number of entities triggered.
 * @retval  0
 *             超时（没有触发任何实体）。
 *             Time out (no entities were triggered).
 * @retval DDS_RETCODE_ERROR
 *             发生内部错误。
 *             An internal error has occurred.
 * @retval DDS_RETCODE_BAD_PARAMETER
 *             给定的等待集无效。
 *             The given waitset is not valid.
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *             该操作在不适当的对象上调用。
 *             The operation is invoked on an inappropriate object.
 * @retval DDS_RETCODE_ALREADY_DELETED
 *             等待集已被删除。
 *             The waitset has already been deleted.
 */
DDS_EXPORT dds_return_t dds_waitset_wait(dds_entity_t waitset,
                                         dds_attach_t* xs,
                                         size_t nxs,
                                         dds_duration_t reltimeout);

/**
 * @brief 该操作允许应用程序线程等待附加到 WaitSet 的实体之一上的状态更改或其他触发器。
 *        This operation allows an application thread to wait for a status
 *        change or other trigger on (one of) the entities that are attached to
 *        the WaitSet.
 * @ingroup waitset
 * @component waitset
 *
 * dds_waitset_wait() 操作阻塞，直到某些附加实体触发或达到 "abstimeout"。
 * The dds_waitset_wait() operation blocks until some of the attached
 * entities have triggered or "abstimeout" has been reached.
 * '触发' (dds_triggered()) 对于每个实体并不意味着相同的含义：
 * 'Triggered' (dds_triggered()) doesn't mean the same for every entity:
 *  - Reader/Writer/Publisher/Subscriber/Topic/Participant
 *      - 当它们的状态发生变化时触发。
 *      - These are triggered when their status changed.
 *  - WaitSet
 *      - 当应用程序将触发值设置为 true 时触发。它保持触发状态，直到应用程序将触发值设置为
 *        false (dds_waitset_set_trigger())。这可以用于唤醒 waitset
 * 的不同原因（例如终止），而不是“正常”的 状态更改（如新数据）。
 *      - Triggered when trigger value was set to true by the application.
 *        It stays triggered until application sets the trigger value to
 *        false (dds_waitset_set_trigger()). This can be used to wake up a
 *        waitset for different reasons (e.g. termination) than the 'normal'
 *        status change (like new data).
 *  - ReadCondition/QueryCondition
 *      - 当相关 Reader 上可用的数据与 Condition 匹配时触发。
 *      - Triggered when data is available on the related Reader that matches
 *        the Condition.
 *
 * 此函数采用预分配的列表将 "xs" blobs
 * 放入（在附加相关实体期间提供），并返回触发实体的数量。可能给定的列表大小
 * 与触发实体的数量不同。如果触发的实体较少，则列表中的最后几个条目保持不变。当触发更多实体时，仅将“size”数量的
 * 条目插入列表，但仍返回触发实体的完整计数。在后一种情况下返回哪些 "xs" blobs 是未定义的。
 * This functions takes a pre-allocated list to put the "xs" blobs in (that
 * were provided during the attach of the related entities) and will return
 * the number of triggered entities. It is possible that the given size
 * of the list is not the same as the number of triggered entities. If fewer
 * entities were triggered, then the last few entries in the list are
 * untouched. When more entities are triggered, then only 'size' number of
 * entries are inserted into the list, but still the complete count of the
 * triggered entities is returned. Which "xs" blobs are returned in the
 * latter case is undefined.
 *
 * 如果发生超时，返回值为0。
 * In case of a time out, the return value is 0.
 *
 * 在应用程序被阻塞时删除 waitset 将导致错误代码（即 < 0）由 "wait" 返回。
 * Deleting the waitset while the application is blocked results in an
 * error code (i.e. < 0) returned by "wait".
 *
 * 多个线程可以同时在单个 waitset 上阻塞；调用是完全独立的。
 * Multiple threads may block on a single waitset at the same time;
 * the calls are entirely independent.
 *
 * 空的 waitset 永远不会触发（即，dds_waitset_wait 在空的 waitset 上本质上等同于睡眠）。
 * An empty waitset never triggers (i.e., dds_waitset_wait on an empty
 * waitset is essentially equivalent to a sleep).
 *
 * "dds_waitset_wait" 操作与 "dds_waitset_wait_until" 相同，只是它采用相对超时。
 * The "dds_waitset_wait" operation is the same as the
 * "dds_waitset_wait_until" except that it takes a relative timeout.
 *
 * "dds_waitset_wait" 操作与 "dds_wait" 相同，只是它采用绝对超时。
 * The "dds_waitset_wait" operation is the same as the "dds_wait"
 * except that it takes an absolute timeout.
 *
 * @param[in]  waitset    要设置触发值的 waitset。
 *                        The waitset to set the trigger value on.
 * @param[out] xs         预分配的列表，用于存储在触发实体的附加过程中提供的 'blobs'。
 *                        Pre-allocated list to store the 'blobs' that were
 *                        provided during the attach of the triggered entities.
 * @param[in]  nxs        预分配 blobs 列表的大小。
 *                        The size of the pre-allocated blobs list.
 * @param[in]  abstimeout 绝对超时
 *                        Absolute timeout
 *
 * @returns 返回 dds_return_t，其中包含触发实体的数量或错误代码。
 *          A dds_return_t with the number of entities triggered or an error code.
 *
 * @retval >0
 *             触发的实体数量。
 *             Number of entities triggered.
 * @retval  0
 *             超时（没有触发实体）。
 *             Time out (no entities were triggered).
 * @retval DDS_RETCODE_ERROR
 *             发生内部错误。
 *             An internal error has occurred.
 * @retval DDS_RETCODE_BAD_PARAMETER
 *             给定的 waitset 无效。
 *             The given waitset is not valid.
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *             在不适当的对象上调用操作。
 *             The operation is invoked on an inappropriate object.
 * @retval DDS_RETCODE_ALREADY_DELETED
 *             waitset 已被删除。
 *             The waitset has already been deleted.
 */
DDS_EXPORT dds_return_t dds_waitset_wait_until(dds_entity_t waitset,
                                               dds_attach_t* xs,
                                               size_t nxs,
                                               dds_time_t abstimeout);

/**
 * @defgroup reading (读取数据)
 * @ingroup reader
 * 有多种获取数据的方法，分为 "read" 和 "take" 的变体。
 * read/take 操作的返回值是返回元素的数量。"max_samples"
 * 应该具有相同的类型，因为这样无法返回超过 MAX_INT
 * 的值。X、Y、CX、CY 返回到各种过滤
 * 选项，请参阅 DCPS 规范。
 *
 * O ::= read | take
 *
 * X             => CX
 * (empty)          (empty)
 * _next_instance   instance_handle_t prev
 *
 * Y             => CY
 * (empty)          uint32_t mask
 * _cond            cond_t cond -- refers to a read condition (or query if implemented)
 */

/**
 * @brief 访问和读取来自数据读取器、读取条件或查询条件的相同类型的数据值集合和样本信息。
 * @ingroup reading
 * @component read_data
 *
 * 返回值提供关于已读取的样本数量的信息，该数量将
 * 小于等于 maxs。基于计数，当在 sample info 结构中设置 valid_data 位时，
 * 缓冲区将仅包含要读取的数据。
 * 数据值所需的缓冲区可以显式分配，也可以
 * 使用来自数据读取器的内存以防止复制。在后一种情况下，缓冲区和
 * 样本信息应在不再使用数据时返回。
 * 读取后的数据值将保留在缓冲区中，样本状态设置为 READ
 * 和 view_state 设置为 NOT_NEW。
 *
 * @param[in]  reader_or_condition 读取器、读取条件或查询条件实体。
 * @param[out] buf 一个指向要读取的数据样本的指针数组（指针可以为 NULL）。
 * @param[out] si 指向为每个数据值返回的 @ref dds_sample_info_t 数组的指针。
 * @param[in]  bufsz 提供的缓冲区大小。
 * @param[in]  maxs 要读取的最大样本数。
 *
 * @returns 一个 dds_return_t，表示已读取的样本数量或错误代码。
 *
 * @retval >=0
 *             已读取的样本数量。
 * @retval DDS_RETCODE_ERROR
 *             发生内部错误。
 * @retval DDS_RETCODE_BAD_PARAMETER
 *             给定参数之一无效。
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *             在不适当的对象上调用操作。
 * @retval DDS_RETCODE_ALREADY_DELETED
 *             实体已被删除。
 */
DDS_EXPORT dds_return_t dds_read(dds_entity_t reader_or_condition,
                                 void** buf,
                                 dds_sample_info_t* si,
                                 size_t bufsz,
                                 uint32_t maxs);

/**
 * @brief 访问和读取数据阅读器、读取条件或查询条件的借用样本。 (Access and read loaned samples of
 * data reader, readcondition or querycondition.)
 * @ingroup reading
 * @component read_data
 *
 * 在调用 dds_read_wl 函数并处理数据后，必须调用 dds_return_loan() 函数以可能释放内存。
 * (After dds_read_wl function is being called and the data has been handled, dds_return_loan()
 * function must be called to possibly free memory.)
 *
 * @param[in]  reader_or_condition 阅读器、读取条件或查询条件实体 (Reader, readcondition or
 * querycondition entity)
 * @param[out] buf 读取数据的指针数组 (An array of pointers to samples into which data is read
 * (pointers can be NULL))
 * @param[out] si 为每个数据值返回的 @ref dds_sample_info_t 数组的指针 (Pointer to an array of @ref
 * dds_sample_info_t returned for each data value)
 * @param[in]  maxs 要读取的最大样本数 (Maximum number of samples to read)
 *
 * @returns 返回一个包含已读取样本数量或错误代码的 dds_return_t (A dds_return_t with the number of
 * samples read or an error code)
 *
 * @retval >=0
 *             已读取的样本数。 (Number of samples read.)
 * @retval DDS_RETCODE_ERROR
 *             发生内部错误。 (An internal error has occurred.)
 * @retval DDS_RETCODE_BAD_PARAMETER
 *             给定参数之一无效。 (One of the given arguments is not valid.)
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *             在不适当的对象上调用操作。 (The operation is invoked on an inappropriate object.)
 * @retval DDS_RETCODE_ALREADY_DELETED
 *             实体已被删除。 (The entity has already been deleted.)
 */
DDS_EXPORT dds_return_t dds_read_wl(dds_entity_t reader_or_condition,
                                    void** buf,
                                    dds_sample_info_t* si,
                                    uint32_t maxs);

/**
 * @brief 根据掩码从数据阅读器、读取条件或查询条件中读取数据值和样本信息的集合。
 *        (Read the collection of data values and sample info from the data reader, readcondition
 *        or querycondition based on mask.)
 * @ingroup reading
 * @component read_data
 *
 * 使用读取条件或查询条件时，它们的掩码与给定掩码进行或运算。
 * (When using a readcondition or querycondition, their masks are or'd with the given mask.)
 *
 * @param[in]  reader_or_condition 阅读器、读取条件或查询条件实体。 (Reader, readcondition or
 * querycondition entity.)
 * @param[out] buf 读取数据的指针数组 (An array of pointers to samples into which data is read
 * (pointers can be NULL).)
 * @param[out] si 为每个数据值返回的 @ref dds_sample_info_t 数组的指针。 (Pointer to an array of
 * @ref dds_sample_info_t returned for each data value.)
 * @param[in]  bufsz 提供的缓冲区大小。 (The size of buffer provided.)
 * @param[in]  maxs 要读取的最大样本数。 (Maximum number of samples to read.)
 * @param[in]  mask 基于 dds_sample_state_t|dds_view_state_t|dds_instance_state_t 过滤数据。
 *                 (Filter the data based on
 *                  dds_sample_state_t|dds_view_state_t|dds_instance_state_t.)
 *
 * @returns 返回一个包含已读取样本数量或错误代码的 dds_return_t。 (A dds_return_t with the number of
 * samples read or an error code.)
 *
 * @retval >=0
 *             已读取的样本数。 (Number of samples read.)
 * @retval DDS_RETCODE_ERROR
 *             发生内部错误。 (An internal error has occurred.)
 * @retval DDS_RETCODE_BAD_PARAMETER
 *             给定参数之一无效。 (One of the given arguments is not valid.)
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *             在不适当的对象上调用操作。 (The operation is invoked on an inappropriate object.)
 * @retval DDS_RETCODE_ALREADY_DELETED
 *             实体已被删除。 (The entity has already been deleted.)
 */
DDS_EXPORT dds_return_t dds_read_mask(dds_entity_t reader_or_condition,
                                      void** buf,
                                      dds_sample_info_t* si,
                                      size_t bufsz,
                                      uint32_t maxs,
                                      uint32_t mask);

/**
 * @brief 访问和读取数据阅读器、readcondition或querycondition的借用样本，基于掩码
 *        Access and read loaned samples of data reader, readcondition
 *        or querycondition based on mask
 * @ingroup reading
 * @component read_data
 *
 * 使用readcondition或querycondition时，它们的掩码与给定的掩码进行或操作。
 * When using a readcondition or querycondition, their masks are or'd with the given mask.
 *
 * 调用dds_read_mask_wl函数并处理数据后，必须调用dds_return_loan()函数以可能释放内存
 * After dds_read_mask_wl function is being called and the data has been handled, dds_return_loan()
 * function must be called to possibly free memory
 *
 * @param[in]  reader_or_condition 阅读器、readcondition或querycondition实体。
 *                                  Reader, readcondition or querycondition entity.
 * @param[out] buf 一个指向样本的指针数组，其中读取数据（指针可以为NULL）。
 *                 An array of pointers to samples into which data is read (pointers can be NULL).
 * @param[out] si 指向为每个数据值返回的@ref dds_sample_info_t数组的指针。
 *                Pointer to an array of @ref dds_sample_info_t returned for each data value.
 * @param[in]  maxs 要读取的最大样本数。
 *                 Maximum number of samples to read.
 * @param[in]  mask 基于dds_sample_state_t|dds_view_state_t|dds_instance_state_t过滤数据。
 *                  Filter the data based on
 *                  dds_sample_state_t|dds_view_state_t|dds_instance_state_t.
 *
 * @returns 一个dds_return_t，其中包含已读取的样本数或错误代码。
 *          A dds_return_t with the number of samples read or an error code.
 *
 * @retval >=0
 *             已读取的样本数。
 *             Number of samples read.
 * @retval DDS_RETCODE_ERROR
 *             发生内部错误。
 *             An internal error has occurred.
 * @retval DDS_RETCODE_BAD_PARAMETER
 *             给定参数之一无效。
 *             One of the given arguments is not valid.
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *             在不适当的对象上调用操作。
 *             The operation is invoked on an inappropriate object.
 * @retval DDS_RETCODE_ALREADY_DELETED
 *             实体已被删除。
 *             The entity has already been deleted.
 */
DDS_EXPORT dds_return_t dds_read_mask_wl(dds_entity_t reader_or_condition,
                                         void** buf,
                                         dds_sample_info_t* si,
                                         uint32_t maxs,
                                         uint32_t mask);

/**
 * @brief 访问并从数据阅读器、readcondition或querycondition中读取相同类型的数据值集合和样本信息，
 *        并由提供的实例句柄进行复制。
 *        Access and read the collection of data values (of same type) and sample info from the
 *        data reader, readcondition or querycondition, coped by the provided instance handle.
 * @ingroup reading
 * @component read_data
 *
 * 此操作实现了与dds_read相同的功能，只是读取的数据限于提供的实例句柄。
 * This operation implements the same functionality as dds_read, except that only data scoped to
 * the provided instance handle is read.
 *
 * @param[in]  reader_or_condition 阅读器、readcondition或querycondition实体。
 *                                  Reader, readcondition or querycondition entity.
 * @param[out] buf 一个指向样本的指针数组，其中读取数据（指针可以为NULL）。
 *                 An array of pointers to samples into which data is read (pointers can be NULL).
 * @param[out] si 指向为每个数据值返回的@ref dds_sample_info_t数组的指针。
 *                Pointer to an array of @ref dds_sample_info_t returned for each data value.
 * @param[in]  bufsz 提供的缓冲区大小。
 *                  The size of buffer provided.
 * @param[in]  maxs 要读取的最大样本数。
 *                 Maximum number of samples to read.
 * @param[in]  handle 与要读取的样本相关的实例句柄。
 *                   Instance handle related to the samples to read.
 *
 * @returns 一个dds_return_t，其中包含已读取的样本数或错误代码。
 *          A dds_return_t with the number of samples read or an error code.
 *
 * @retval >=0
 *             已读取的样本数。
 *             Number of samples read.
 * @retval DDS_RETCODE_ERROR
 *             发生内部错误。
 *             An internal error has occurred.
 * @retval DDS_RETCODE_BAD_PARAMETER
 *             给定参数之一无效。
 *             One of the given arguments is not valid.
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *             在不适当的对象上调用操作。
 *             The operation is invoked on an inappropriate object.
 * @retval DDS_RETCODE_ALREADY_DELETED
 *             实体已被删除。
 *             The entity has already been deleted.
 * @retval DDS_RETCODE_PRECONDITION_NOT_MET
 *             实例句柄尚未在此阅读器中注册。
 *             The instance handle has not been registered with this reader.
 */
DDS_EXPORT dds_return_t dds_read_instance(dds_entity_t reader_or_condition,
                                          void** buf,
                                          dds_sample_info_t* si,
                                          size_t bufsz,
                                          uint32_t maxs,
                                          dds_instance_handle_t handle);

/**
 * @brief 访问和读取数据读取器、读取条件或查询条件的借用样本，范围由提供的实例句柄限定。
 *        Access and read loaned samples of data reader, readcondition or querycondition,
 *        scoped by the provided instance handle.
 * @ingroup reading
 * @component read_data
 *
 * 此操作实现了与 dds_read_wl 相同的功能，只是读取的数据仅限于提供的实例句柄。
 * This operation implements the same functionality as dds_read_wl, except that only data
 * scoped to the provided instance handle is read.
 *
 * @param[in]  reader_or_condition 读取器、读取条件或查询条件实体。
 *                                 Reader, readcondition or querycondition entity.
 * @param[out] buf 一个指向样本的指针数组，用于读取数据（指针可以为 NULL）。
 *                An array of pointers to samples into which data is read (pointers can be NULL).
 * @param[out] si 指向为每个数据值返回的 @ref dds_sample_info_t 数组的指针。
 *                Pointer to an array of @ref dds_sample_info_t returned for each data value.
 * @param[in]  maxs 要读取的最大样本数。
 *                 Maximum number of samples to read.
 * @param[in]  handle 与要读取的样本相关的实例句柄。
 *                    Instance handle related to the samples to read.
 *
 * @returns 返回具有已读取样本数或错误代码的 dds_return_t。
 *          A dds_return_t with the number of samples read or an error code.
 *
 * @retval >=0
 *             已读取的样本数。
 *             Number of samples read.
 * @retval DDS_RETCODE_ERROR
 *             发生内部错误。
 *             An internal error has occurred.
 * @retval DDS_RETCODE_BAD_PARAMETER
 *             给定参数之一无效。
 *             One of the given arguments is not valid.
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *             在不适当的对象上调用了操作。
 *             The operation is invoked on an inappropriate object.
 * @retval DDS_RETCODE_ALREADY_DELETED
 *             实体已被删除。
 *             The entity has already been deleted.
 * @retval DDS_RETCODE_PRECONDITION_NOT_MET
 *             实例句柄尚未在此读取器中注册。
 *             The instance handle has not been registered with this reader.
 */
DDS_EXPORT dds_return_t dds_read_instance_wl(dds_entity_t reader_or_condition,
                                             void** buf,
                                             dds_sample_info_t* si,
                                             uint32_t maxs,
                                             dds_instance_handle_t handle);

/**
 * @brief 根据掩码和提供的实例句柄范围从数据读取器、读取条件或查询条件中读取数据值和样本信息的集合。
 *        Read the collection of data values and sample info from the data reader, readcondition
 *        or querycondition based on mask and scoped by the provided instance handle.
 * @ingroup reading
 * @component read_data
 *
 * 此操作实现了与 dds_read_mask 相同的功能，只是读取的数据仅限于提供的实例句柄。
 * This operation implements the same functionality as dds_read_mask, except that only data
 * scoped to the provided instance handle is read.
 *
 * @param[in]  reader_or_condition 读取器、读取条件或查询条件实体。
 *                                 Reader, readcondition or querycondition entity.
 * @param[out] buf 一个指向样本的指针数组，用于读取数据（指针可以为 NULL）。
 *                An array of pointers to samples into which data is read (pointers can be NULL).
 * @param[out] si 指向为每个数据值返回的 @ref dds_sample_info_t 数组的指针。
 *                Pointer to an array of @ref dds_sample_info_t returned for each data value.
 * @param[in]  bufsz 提供的缓冲区大小。
 *                  The size of buffer provided.
 * @param[in]  maxs 要读取的最大样本数。
 *                 Maximum number of samples to read.
 * @param[in]  handle 与要读取的样本相关的实例句柄。
 *                    Instance handle related to the samples to read.
 * @param[in]  mask 根据 dds_sample_state_t|dds_view_state_t|dds_instance_state_t 过滤数据。
 *                 Filter the data based on
 *                 dds_sample_state_t|dds_view_state_t|dds_instance_state_t.
 *
 * @returns 返回具有已读取样本数或错误代码的 dds_return_t。
 *          A dds_return_t with the number of samples read or an error code.
 *
 * @retval >=0
 *             已读取的样本数。
 *             Number of samples read.
 * @retval DDS_RETCODE_ERROR
 *             发生内部错误。
 *             An internal error has occurred.
 * @retval DDS_RETCODE_BAD_PARAMETER
 *             给定参数之一无效。
 *             One of the given arguments is not valid.
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *             在不适当的对象上调用了操作。
 *             The operation is invoked on an inappropriate object.
 * @retval DDS_RETCODE_ALREADY_DELETED
 *             实体已被删除。
 *             The entity has already been deleted.
 * @retval DDS_RETCODE_PRECONDITION_NOT_MET
 *             实例句柄尚未在此读取器中注册。
 *             The instance handle has not been registered with this reader.
 */
DDS_EXPORT dds_return_t dds_read_instance_mask(dds_entity_t reader_or_condition,
                                               void** buf,
                                               dds_sample_info_t* si,
                                               size_t bufsz,
                                               uint32_t maxs,
                                               dds_instance_handle_t handle,
                                               uint32_t mask);

/**
 * @brief 访问并读取基于掩码的数据阅读器、readcondition或querycondition的借用样本，
 *        由提供的实例句柄限定范围。
 *        Access and read loaned samples of data reader, readcondition or
 *        querycondition based on mask, scoped by the provided instance handle.
 * @ingroup reading
 * @component read_data
 *
 * 此操作实现了与dds_read_mask_wl相同的功能，只是读取的数据仅限于提供的实例句柄。
 * This operation implements the same functionality as dds_read_mask_wl, except that
 * only data scoped to the provided instance handle is read.
 *
 * @param[in]  reader_or_condition 阅读器、readcondition或querycondition实体。
 *                                 Reader, readcondition or querycondition entity.
 * @param[out] buf 一个指向样本的指针数组，用于读取数据（指针可以为空）。
 *                An array of pointers to samples into which data is read (pointers can be NULL).
 * @param[out] si 指向为每个数据值返回的@ref dds_sample_info_t数组的指针。
 *                Pointer to an array of @ref dds_sample_info_t returned for each data value.
 * @param[in]  maxs 要读取的最大样本数。
 *                 Maximum number of samples to read.
 * @param[in]  handle 与要读取的样本相关的实例句柄。
 *                   Instance handle related to the samples to read.
 * @param[in]  mask 基于dds_sample_state_t|dds_view_state_t|dds_instance_state_t过滤数据。
 *                 Filter the data based on
 * dds_sample_state_t|dds_view_state_t|dds_instance_state_t.
 *
 * @returns 一个dds_return_t，其中包含已读取的样本数或错误代码。
 *          A dds_return_t with the number of samples read or an error code.
 *
 * @retval >=0
 *             已读取的样本数。
 *             Number of samples read.
 * @retval DDS_RETCODE_ERROR
 *             发生了内部错误。
 *             An internal error has occurred.
 * @retval DDS_RETCODE_BAD_PARAMETER
 *             给定的参数之一无效。
 *             One of the given arguments is not valid.
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *             在不合适的对象上调用了该操作。
 *             The operation is invoked on an inappropriate object.
 * @retval DDS_RETCODE_ALREADY_DELETED
 *             实体已被删除。
 *             The entity has already been deleted.
 * @retval DDS_RETCODE_PRECONDITION_NOT_MET
 *             实例句柄尚未在此阅读器中注册。
 *             The instance handle has not been registered with this reader.
 */
DDS_EXPORT dds_return_t dds_read_instance_mask_wl(dds_entity_t reader_or_condition,
                                                  void** buf,
                                                  dds_sample_info_t* si,
                                                  uint32_t maxs,
                                                  dds_instance_handle_t handle,
                                                  uint32_t mask);

/**
 * @brief 访问从数据阅读器、readcondition或querycondition获取的相同类型的数据值集合和样本信息。
 *        Access the collection of data values (of same type) and sample info from the
 *        data reader, readcondition or querycondition.
 * @ingroup reading
 * @component read_data
 *
 * 读取后的数据值将从DataReader中删除，不能再次“读取”或“获取”。
 * Data value once read is removed from the Data Reader cannot to
 * 'read' or 'taken' again.
 * 返回值提供关于已读取样本数的信息，该值将<= maxs。基于计数，只有在sample
 * info结构中设置了valid_data位时，缓冲区才包含要读取的数据。 Return value provides information
 * about number of samples read, which will be <= maxs. Based on the count, the buffer will contain
 * data to be read only when valid_data bit in sample info structure is set.
 * 数据值所需的缓冲区可以显式分配，也可以使用来自DataReader的内存以防止复制。在后一种情况下，当不再使用数据时，应返回缓冲区和sample_info。
 * The buffer required for data values, could be allocated explicitly or can
 * use the memory from data reader to prevent copy. In the latter case, buffer and
 * sample_info should be returned back, once it is no longer using the Data.
 *
 * @param[in]  reader_or_condition 阅读器、readcondition或querycondition实体。
 *                                 Reader, readcondition or querycondition entity.
 * @param[out] buf 一个指向样本的指针数组，用于读取数据（指针可以为空）。
 *                An array of pointers to samples into which data is read (pointers can be NULL).
 * @param[out] si 指向为每个数据值返回的@ref dds_sample_info_t数组的指针。
 *                Pointer to an array of @ref dds_sample_info_t returned for each data value.
 * @param[in]  bufsz 提供的缓冲区大小。
 *                 The size of buffer provided.
 * @param[in]  maxs 要读取的最大样本数。
 *                 Maximum number of samples to read.
 *
 * @returns 一个dds_return_t，其中包含已读取的样本数或错误代码。
 *          A dds_return_t with the number of samples read or an error code.
 *
 * @retval >=0
 *             已读取的样本数。
 *             Number of samples read.
 * @retval DDS_RETCODE_ERROR
 *             发生了内部错误。
 *             An internal error has occurred.
 * @retval DDS_RETCODE_BAD_PARAMETER
 *             给定的参数之一无效。
 *             One of the given arguments is not valid.
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *             在不合适的对象上调用了该操作。
 *             The operation is invoked on an inappropriate object.
 * @retval DDS_RETCODE_ALREADY_DELETED
 *             实体已被删除。
 *             The entity has already been deleted.
 */
DDS_EXPORT dds_return_t dds_take(dds_entity_t reader_or_condition,
                                 void** buf,
                                 dds_sample_info_t* si,
                                 size_t bufsz,
                                 uint32_t maxs);

/**
 * @brief 访问数据读取器、读取条件或查询条件的借用样本。 (Access loaned samples of data reader,
 * readcondition or querycondition.)
 * @ingroup reading
 * @component read_data
 *
 * 在调用 dds_take_wl 函数并处理数据后，必须调用 dds_return_loan() 函数以可能释放内存。
 * (After dds_take_wl function is being called and the data has been handled, dds_return_loan()
 * function must be called to possibly free memory)
 *
 * @param[in]  reader_or_condition 读取器、读取条件或查询条件实体。 (Reader, readcondition or
 * querycondition entity.)
 * @param[out] buf 一个指向样本的指针数组，其中读取数据（指针可以为 NULL）。(An array of pointers to
 * samples into which data is read (pointers can be NULL).)
 * @param[out] si 指向为每个数据值返回的 @ref dds_sample_info_t 数组的指针。 (Pointer to an array of
 * @ref dds_sample_info_t returned for each data value.)
 * @param[in]  maxs 要读取的最大样本数。 (Maximum number of samples to read.)
 *
 * @returns 返回一个 dds_return_t，其中包含已读取的样本数或错误代码。 (A dds_return_t with the
 * number of samples read or an error code.)
 *
 * @retval >=0
 *             已读取的样本数。 (Number of samples read.)
 * @retval DDS_RETCODE_ERROR
 *             发生了内部错误。 (An internal error has occurred.)
 * @retval DDS_RETCODE_BAD_PARAMETER
 *             给定的参数之一无效。 (One of the given arguments is not valid.)
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *             该操作在不合适的对象上调用。 (The operation is invoked on an inappropriate object.)
 * @retval DDS_RETCODE_ALREADY_DELETED
 *             实体已被删除。 (The entity has already been deleted.)
 */
DDS_EXPORT dds_return_t dds_take_wl(dds_entity_t reader_or_condition,
                                    void** buf,
                                    dds_sample_info_t* si,
                                    uint32_t maxs);

/**
 * @brief 根据掩码从数据读取器、读取条件或查询条件中获取数据值（相同类型）和样本信息的集合。
 *        (Take the collection of data values (of same type) and sample info from the
 *        data reader, readcondition or querycondition based on mask)
 * @ingroup reading
 * @component read_data
 *
 * 使用读取条件或查询条件时，它们的掩码与给定掩码进行或运算。 (When using a readcondition or
 * querycondition, their masks are or'd with the given mask.)
 *
 * @param[in]  reader_or_condition 读取器、读取条件或查询条件实体。 (Reader, readcondition or
 * querycondition entity.)
 * @param[out] buf 一个指向样本的指针数组，其中读取数据（指针可以为 NULL）。(An array of pointers to
 * samples into which data is read (pointers can be NULL).)
 * @param[out] si 指向为每个数据值返回的 @ref dds_sample_info_t 数组的指针。 (Pointer to an array of
 * @ref dds_sample_info_t returned for each data value.)
 * @param[in]  bufsz 提供的缓冲区大小。 (The size of buffer provided.)
 * @param[in]  maxs 要读取的最大样本数。 (Maximum number of samples to read.)
 * @param[in]  mask 基于 dds_sample_state_t|dds_view_state_t|dds_instance_state_t 过滤数据。 (Filter
 * the data based on dds_sample_state_t|dds_view_state_t|dds_instance_state_t.)
 *
 * @returns 返回一个 dds_return_t，其中包含已读取的样本数或错误代码。 (A dds_return_t with the
 * number of samples read or an error code.)
 *
 * @retval >=0
 *             已读取的样本数。 (Number of samples read.)
 * @retval DDS_RETCODE_ERROR
 *             发生了内部错误。 (An internal error has occurred.)
 * @retval DDS_RETCODE_BAD_PARAMETER
 *             给定的参数之一无效。 (One of the given arguments is not valid.)
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *             该操作在不合适的对象上调用。 (The operation is invoked on an inappropriate object.)
 * @retval DDS_RETCODE_ALREADY_DELETED
 *             实体已被删除。 (The entity has already been deleted.)
 */
DDS_EXPORT dds_return_t dds_take_mask(dds_entity_t reader_or_condition,
                                      void** buf,
                                      dds_sample_info_t* si,
                                      size_t bufsz,
                                      uint32_t maxs,
                                      uint32_t mask);

/**
 * @brief  访问基于掩码的数据阅读器、readcondition或querycondition的借用样本。
 *         Access loaned samples of data reader, readcondition or querycondition based on mask.
 * @ingroup reading
 * @component read_data
 *
 * 当使用readcondition或querycondition时，它们的掩码与给定掩码进行或操作。
 * When using a readcondition or querycondition, their masks are or'd with the given mask.
 *
 * 在调用dds_take_mask_wl函数并处理数据后，必须调用dds_return_loan()函数以可能释放内存。
 * After dds_take_mask_wl function is being called and the data has been handled, dds_return_loan()
 * function must be called to possibly free memory.
 *
 * @param[in]  reader_or_condition 阅读器、readcondition或querycondition实体。
 *                                 Reader, readcondition or querycondition entity.
 * @param[out] buf 一个指向样本的指针数组，其中读取数据（指针可以为NULL）。
 *                An array of pointers to samples into which data is read (pointers can be NULL).
 * @param[out] si 为每个数据值返回的dds_sample_info_t数组的指针。
 *                Pointer to an array of @ref dds_sample_info_t returned for each data value.
 * @param[in]  maxs 要读取的最大样本数。
 *                 Maximum number of samples to read.
 * @param[in]  mask 基于dds_sample_state_t|dds_view_state_t|dds_instance_state_t过滤数据。
 *                 Filter the data based on
 *                 dds_sample_state_t|dds_view_state_t|dds_instance_state_t.
 *
 * @returns 一个dds_return_t，其中包含读取的样本数或错误代码。
 *          A dds_return_t with the number of samples read or an error code.
 *
 * @retval >=0
 *             读取的样本数。
 *             Number of samples read.
 * @retval DDS_RETCODE_ERROR
 *             发生内部错误。
 *             An internal error has occurred.
 * @retval DDS_RETCODE_BAD_PARAMETER
 *             给定参数之一无效。
 *             One of the given arguments is not valid.
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *             在不适当的对象上调用操作。
 *             The operation is invoked on an inappropriate object.
 * @retval DDS_RETCODE_ALREADY_DELETED
 *             实体已被删除。
 *             The entity has already been deleted.
 */
DDS_EXPORT dds_return_t dds_take_mask_wl(dds_entity_t reader_or_condition,
                                         void** buf,
                                         dds_sample_info_t* si,
                                         uint32_t maxs,
                                         uint32_t mask);

/**
 * @anchor DDS_HAS_READCDR
 * @ingroup reading
 * @brief Set when function dds_has_readcdr is defined.
 *
 * 当定义函数 dds_has_readcdr 时设置。
 */
#define DDS_HAS_READCDR 1

/**
 * @brief Access the collection of serialized data values (of same type) and
 *        sample info from the data reader, readcondition or querycondition.
 * @ingroup reading
 * @component read_data
 *
 * 此调用从数据读取器、读取条件或查询条件访问序列化数据，并使其可供应用程序使用。序列化数据通过 @ref
 * ddsi_serdata 结构提供。返回的样本标记为已读。
 *
 * This call accesses the serialized data from the data reader, readcondition or
 * querycondition and makes it available to the application. The serialized data
 * is made available through @ref ddsi_serdata structures. Returned samples are
 * marked as READ.
 *
 * Return value provides information about the number of samples read, which will
 * be <= maxs. Based on the count, the buffer will contain serialized data to be
 * read only when valid_data bit in sample info structure is set.
 * The buffer required for data values, could be allocated explicitly or can
 * use the memory from data reader to prevent copy. In the latter case, buffer and
 * sample_info should be returned back, once it is no longer using the data.
 *
 * 返回值提供关于读取的样本数量的信息，该数量将小于等于 maxs。基于计数，当样本信息结构中的
 * valid_data 位设置时，缓冲区将包含要读取的序列化数据。
 * 数据值所需的缓冲区可以显式分配，也可以使用数据读取器的内存以防止复制。在后一种情况下，一旦不再使用数据，应返回缓冲区和
 * sample_info。
 *
 * @param[in]  reader_or_condition Reader, readcondition or querycondition entity.
 *                                读取器、读取条件或查询条件实体。
 * @param[out] buf An array of pointers to @ref ddsi_serdata structures that contain
 *                 the serialized data. The pointers can be NULL.
 *                 包含序列化数据的 @ref ddsi_serdata 结构的指针数组。指针可以为 NULL。
 * @param[in]  maxs Maximum number of samples to read.
 *                 要读取的最大样本数量。
 * @param[out] si Pointer to an array of @ref dds_sample_info_t returned for each data value.
 *                指向为每个数据值返回的 @ref dds_sample_info_t 数组的指针。
 * @param[in]  mask Filter the data based on
 *                  dds_sample_state_t|dds_view_state_t|dds_instance_state_t.
 *                  根据 dds_sample_state_t|dds_view_state_t|dds_instance_state_t 过滤数据。
 *
 * @returns A dds_return_t with the number of samples read or an error code.
 *          带有已读取样本数量或错误代码的 dds_return_t。
 *
 * @retval >=0
 *             Number of samples read.
 *             已读取的样本数量。
 * @retval DDS_RETCODE_ERROR
 *             An internal error has occurred.
 *             发生内部错误。
 * @retval DDS_RETCODE_BAD_PARAMETER
 *             One of the given arguments is not valid.
 *             给定参数之一无效。
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *             The operation is invoked on an inappropriate object.
 *             该操作在不适当的对象上调用。
 * @retval DDS_RETCODE_ALREADY_DELETED
 *             The entity has already been deleted.
 *             实体已被删除。
 * @retval DDS_RETCODE_PRECONDITION_NOT_MET
 *             The precondition for this operation is not met.
 *             此操作的前提条件未满足。
 */
DDS_EXPORT dds_return_t dds_readcdr(dds_entity_t reader_or_condition,
                                    struct ddsi_serdata** buf,
                                    uint32_t maxs,
                                    dds_sample_info_t* si,
                                    uint32_t mask);

/**
 * @brief
 * 通过提供的实例句柄，从数据读取器、读取条件或查询条件访问相同类型的序列化数据值集合和样本信息。
 *        Access the collection of serialized data values (of same type) and
 *        sample info from the data reader, readcondition or querycondition
 *        scoped by the provided instance handle.
 * @ingroup reading
 * @component read_data
 *
 * 这个操作实现了与 dds_read_instance_wl 相同的功能，只是现在样本以序列化形式存在。
 * This operation implements the same functionality as dds_read_instance_wl, except that
 * samples are now in their serialized form. The serialized data is made available through
 * @ref ddsi_serdata structures. Returned samples are marked as READ.
 *
 * 返回值提供关于读取的样本数量的信息，该数量将小于等于 maxs。基于计数，仅当样本信息结构中设置了
 * valid_data 位时，缓冲区才包含要读取的序列化数据。 Return value provides information about the
 * number of samples read, which will be <= maxs. Based on the count, the buffer will contain
 * serialized data to be read only when valid_data bit in sample info structure is set.
 * 数据值所需的缓冲区可以显式分配，也可以使用数据读取器的内存以防止复制。在后一种情况下，一旦不再使用数据，应返回缓冲区和
 * sample_info。 The buffer required for data values, could be allocated explicitly or can use the
 * memory from data reader to prevent copy. In the latter case, buffer and sample_info should be
 * returned back, once it is no longer using the data.
 *
 * @param[in]  reader_or_condition Reader, readcondition or querycondition entity.
 * @param[out] buf An array of pointers to @ref ddsi_serdata structures that contain
 *                 the serialized data. The pointers can be NULL.
 * @param[in]  maxs Maximum number of samples to read.
 * @param[out] si Pointer to an array of @ref dds_sample_info_t returned for each data value.
 * @param[in]  handle Instance handle related to the samples to read.
 * @param[in]  mask Filter the data based on
 * dds_sample_state_t|dds_view_state_t|dds_instance_state_t.
 *
 * @returns A dds_return_t with the number of samples read or an error code.
 *
 * @retval >=0
 *             Number of samples read.
 * @retval DDS_RETCODE_ERROR
 *             An internal error has occurred.
 * @retval DDS_RETCODE_BAD_PARAMETER
 *             One of the given arguments is not valid.
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *             The operation is invoked on an inappropriate object.
 * @retval DDS_RETCODE_ALREADY_DELETED
 *             The entity has already been deleted.
 * @retval DDS_RETCODE_PRECONDITION_NOT_MET
 *             The instance handle has not been registered with this reader.
 */
DDS_EXPORT dds_return_t dds_readcdr_instance(dds_entity_t reader_or_condition,
                                             struct ddsi_serdata** buf,
                                             uint32_t maxs,
                                             dds_sample_info_t* si,
                                             dds_instance_handle_t handle,
                                             uint32_t mask);

/**
 * @brief 从数据读取器、读取条件或查询条件访问序列化数据值（相同类型）的集合和样本信息。
 *        Access the collection of serialized data values (of same type) and
 *        sample info from the data reader, readcondition or querycondition.
 * @ingroup reading
 * @component read_data
 *
 * 此调用从数据读取器、读取条件或查询条件访问序列化数据，并使其可供应用程序使用。序列化数据通过 @ref
 * ddsi_serdata 结构提供。一旦读取数据，它将从读取器中删除，不能再次“读取”或“获取”。 This call
 * accesses the serialized data from the data reader, readcondition or querycondition and makes it
 * available to the application. The serialized data is made available through @ref ddsi_serdata
 * structures. Once read the data is removed from the reader and cannot be 'read' or 'taken' again.
 *
 * 返回值提供关于读取的样本数量的信息，该数量将小于等于 maxs。基于计数，仅当样本信息结构中设置了
 * valid_data 位时，缓冲区才包含要读取的序列化数据。 Return value provides information about the
 * number of samples read, which will be <= maxs. Based on the count, the buffer will contain
 * serialized data to be read only when valid_data bit in sample info structure is set.
 * 数据值所需的缓冲区可以显式分配，也可以使用数据读取器的内存以防止复制。在后一种情况下，一旦不再使用数据，应返回缓冲区和
 * sample_info。 The buffer required for data values, could be allocated explicitly or can use the
 * memory from data reader to prevent copy. In the latter case, buffer and sample_info should be
 * returned back, once it is no longer using the data.
 *
 * @param[in]  reader_or_condition Reader, readcondition or querycondition entity.
 * @param[out] buf An array of pointers to @ref ddsi_serdata structures that contain
 *                 the serialized data. The pointers can be NULL.
 * @param[in]  maxs Maximum number of samples to read.
 * @param[out] si Pointer to an array of @ref dds_sample_info_t returned for each data value.
 * @param[in]  mask Filter the data based on
 * dds_sample_state_t|dds_view_state_t|dds_instance_state_t.
 *
 * @returns A dds_return_t with the number of samples read or an error code.
 *
 * @retval >=0
 *             Number of samples read.
 * @retval DDS_RETCODE_ERROR
 *             An internal error has occurred.
 * @retval DDS_RETCODE_BAD_PARAMETER
 *             One of the given arguments is not valid.
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *             The operation is invoked on an inappropriate object.
 * @retval DDS_RETCODE_ALREADY_DELETED
 *             The entity has already been deleted.
 * @retval DDS_RETCODE_PRECONDITION_NOT_MET
 *             The precondition for this operation is not met.
 */
DDS_EXPORT dds_return_t dds_takecdr(dds_entity_t reader_or_condition,
                                    struct ddsi_serdata** buf,
                                    uint32_t maxs,
                                    dds_sample_info_t* si,
                                    uint32_t mask);

/**
 * @brief 访问数据读取器、读取条件或查询条件中的序列化数据值（相同类型）集合，
 *        并通过提供的实例句柄进行范围限定。
 *        Access the collection of serialized data values (of same type) and
 *        sample info from the data reader, readcondition or querycondition
 *        scoped by the provided instance handle.
 * @ingroup reading
 * @component read_data
 *
 * 该操作实现了与 dds_take_instance_wl 相同的功能，只是现在样本以序列化形式存在。
 * This operation implements the same functionality as dds_take_instance_wl, except that
 * samples are now in their serialized form. The serialized data is made available through
 * @ref ddsi_serdata structures. Returned samples are marked as READ.
 *
 * 返回值提供关于已读取的样本数量的信息，这将小于等于
 * maxs。基于计数，当有效数据位在样本信息结构中设置时，缓冲区将包含要读取的序列化数据。 Return value
 * provides information about the number of samples read, which will be <= maxs. Based on the count,
 * the buffer will contain serialized data to be read only when valid_data bit in sample info
 * structure is set.
 * 数据值所需的缓冲区可以显式分配，也可以使用数据读取器的内存以防止复制。在后一种情况下，当不再使用数据时，应返回缓冲区和
 * sample_info。 The buffer required for data values, could be allocated explicitly or can use the
 * memory from data reader to prevent copy. In the latter case, buffer and sample_info should be
 * returned back, once it is no longer using the data.
 *
 * @param[in]  reader_or_condition 读取器、读取条件或查询条件实体。
 *                                 Reader, readcondition or querycondition entity.
 * @param[out] buf 包含序列化数据的 @ref ddsi_serdata 结构的指针数组。指针可以为 NULL。
 *                 An array of pointers to @ref ddsi_serdata structures that contain
 *                 the serialized data. The pointers can be NULL.
 * @param[in]  maxs 要读取的最大样本数。
 *                 Maximum number of samples to read.
 * @param[out] si 为每个数据值返回的 @ref dds_sample_info_t 数组的指针。
 *                Pointer to an array of @ref dds_sample_info_t returned for each data value.
 * @param[in]  handle 与要读取的样本相关的实例句柄。
 *                    Instance handle related to the samples to read.
 * @param[in]  mask 基于 dds_sample_state_t|dds_view_state_t|dds_instance_state_t 过滤数据。
 *                  Filter the data based on
 *                  dds_sample_state_t|dds_view_state_t|dds_instance_state_t.
 *
 * @returns 一个带有已读取样本数量或错误代码的 dds_return_t。
 *          A dds_return_t with the number of samples read or an error code.
 *
 * @retval >=0
 *             已读取的样本数。
 *             Number of samples read.
 * @retval DDS_RETCODE_ERROR
 *             发生了内部错误。
 *             An internal error has occurred.
 * @retval DDS_RETCODE_BAD_PARAMETER
 *             给定的参数之一无效。
 *             One of the given arguments is not valid.
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *             该操作在不适当的对象上调用。
 *             The operation is invoked on an inappropriate object.
 * @retval DDS_RETCODE_ALREADY_DELETED
 *             实体已被删除。
 *             The entity has already been deleted.
 * @retval DDS_RETCODE_PRECONDITION_NOT_MET
 *             实例句柄尚未在此读取器中注册。
 *             The instance handle has not been registered with this reader.
 */
DDS_EXPORT dds_return_t dds_takecdr_instance(dds_entity_t reader_or_condition,
                                             struct ddsi_serdata** buf,
                                             uint32_t maxs,
                                             dds_sample_info_t* si,
                                             dds_instance_handle_t handle,
                                             uint32_t mask);

/**
 * @brief
 * 访问数据读取器、读取条件或查询条件中的数据值（相同类型）和样本信息集合，但仅限于给定的实例句柄范围。
 *        Access the collection of data values (of same type) and sample info from the
 *        data reader, readcondition or querycondition but scoped by the given
 *        instance handle.
 * @ingroup reading
 * @component read_data
 *
 * 此操作实现了与 dds_take 相同的功能，只是仅获取提供的实例句柄范围内的数据。
 * This operation implements the same functionality as dds_take, except that only data
 * scoped to the provided instance handle is taken.
 *
 * @param[in]  reader_or_condition 读取器、读取条件或查询条件实体。
 *                                 Reader, readcondition or querycondition entity.
 * @param[out] buf 用于读取数据的样本指针数组（指针可以为 NULL）。
 *                An array of pointers to samples into which data is read (pointers can be NULL).
 * @param[out] si 返回每个数据值的 @ref dds_sample_info_t 数组的指针。
 *                Pointer to an array of @ref dds_sample_info_t returned for each data value.
 * @param[in]  bufsz 提供的缓冲区大小。
 *                  The size of buffer provided.
 * @param[in]  maxs 要读取的最大样本数。
 *                 Maximum number of samples to read.
 * @param[in]  handle 与要读取的样本相关的实例句柄。
 *                   Instance handle related to the samples to read.
 *
 * @returns 返回一个 dds_return_t，其中包含已读取的样本数或错误代码。
 *          A dds_return_t with the number of samples read or an error code.
 *
 * @retval >=0
 *             已读取的样本数。
 *             Number of samples read.
 * @retval DDS_RETCODE_ERROR
 *             发生内部错误。
 *             An internal error has occurred.
 * @retval DDS_RETCODE_BAD_PARAMETER
 *             给定参数之一无效。
 *             One of the given arguments is not valid.
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *             该操作在不适当的对象上调用。
 *             The operation is invoked on an inappropriate object.
 * @retval DDS_RETCODE_ALREADY_DELETED
 *             实体已被删除。
 *             The entity has already been deleted.
 * @retval DDS_RETCODE_PRECONDITION_NOT_MET
 *             实例句柄尚未在此读取器中注册。
 *             The instance handle has not been registered with this reader.
 */
DDS_EXPORT dds_return_t dds_take_instance(dds_entity_t reader_or_condition,
                                          void** buf,
                                          dds_sample_info_t* si,
                                          size_t bufsz,
                                          uint32_t maxs,
                                          dds_instance_handle_t handle);

/**
 * @brief 访问数据读取器、读取条件或查询条件的借用样本，范围由给定的实例句柄限定。
 *        Access loaned samples of data reader, readcondition or querycondition,
 *        scoped by the given instance handle.
 * @ingroup reading
 * @component read_data
 *
 * 此操作实现了与 dds_take_wl 相同的功能，只是读取的数据仅限于提供的实例句柄的范围。
 * This operation implements the same functionality as dds_take_wl, except that
 * only data scoped to the provided instance handle is read.
 *
 * @param[in]  reader_or_condition 读取器、读取条件或查询条件实体。
 *                                 Reader, readcondition or querycondition entity.
 * @param[out] buf 一个指向样本的指针数组，其中读取数据（指针可以为 NULL）。
 *                An array of pointers to samples into which data is read (pointers can be NULL).
 * @param[out] si 指向为每个数据值返回的 @ref dds_sample_info_t 数组的指针。
 *                Pointer to an array of @ref dds_sample_info_t returned for each data value.
 * @param[in]  maxs 要读取的最大样本数。
 *                 Maximum number of samples to read.
 * @param[in]  handle 与要读取的样本相关的实例句柄。
 *                   Instance handle related to the samples to read.
 *
 * @returns 一个 dds_return_t，包含已读取的样本数量或错误代码。
 *          A dds_return_t with the number of samples read or an error code.
 *
 * @retval >=0
 *             已读取的样本数量。
 *             Number of samples read.
 * @retval DDS_RETCODE_ERROR
 *             发生了内部错误。
 *             An internal error has occurred.
 * @retval DDS_RETCODE_BAD_PARAMETER
 *             给定的参数之一无效。
 *             One of the given arguments is not valid.
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *             该操作在不适当的对象上调用。
 *             The operation is invoked on an inappropriate object.
 * @retval DDS_RETCODE_ALREADY_DELETED
 *             该实体已被删除。
 *             The entity has already been deleted.
 * @retval DDS_RETCODE_PRECONDITION_NOT_MET
 *             实例句柄尚未在此读取器中注册。
 *             The instance handle has not been registered with this reader.
 */
DDS_EXPORT dds_return_t dds_take_instance_wl(dds_entity_t reader_or_condition,
                                             void** buf,
                                             dds_sample_info_t* si,
                                             uint32_t maxs,
                                             dds_instance_handle_t handle);

/**
 * @brief
 * 从数据读取器、读取条件或查询条件中获取相同类型的数据值集合和样本信息，基于掩码并限定给定实例句柄。
 *        Take the collection of data values (of same type) and sample info from the
 *        data reader, readcondition or querycondition based on mask and scoped
 *        by the given instance handle.
 * @ingroup reading
 * @component read_data
 *
 * 此操作实现了与 dds_take_mask 相同的功能，只是读取的数据仅限于提供的实例句柄。
 * This operation implements the same functionality as dds_take_mask, except that only
 * data scoped to the provided instance handle is read.
 *
 * @param[in]  reader_or_condition 读取器、读取条件或查询条件实体。
 *                                 Reader, readcondition or querycondition entity.
 * @param[out] buf 一个指向样本的指针数组，用于读取数据（指针可以为 NULL）。
 *                 An array of pointers to samples into which data is read (pointers can be NULL).
 * @param[out] si 返回每个数据值的 @ref dds_sample_info_t 数组的指针。
 *                Pointer to an array of @ref dds_sample_info_t returned for each data value.
 * @param[in]  bufsz 提供的缓冲区大小。
 *                  The size of buffer provided.
 * @param[in]  maxs 要读取的最大样本数。
 *                 Maximum number of samples to read.
 * @param[in]  handle 与要读取的样本相关的实例句柄。
 *                   Instance handle related to the samples to read.
 * @param[in]  mask 基于 dds_sample_state_t|dds_view_state_t|dds_instance_state_t 过滤数据。
 *                 Filter the data based on
 *                 dds_sample_state_t|dds_view_state_t|dds_instance_state_t.
 *
 * @returns 返回一个包含读取的样本数或错误代码的 dds_return_t。
 *          A dds_return_t with the number of samples read or an error code.
 *
 * @retval >=0
 *             读取的样本数。
 *             Number of samples read.
 * @retval DDS_RETCODE_ERROR
 *             发生了内部错误。
 *             An internal error has occurred.
 * @retval DDS_RETCODE_BAD_PARAMETER
 *             给定的参数之一无效。
 *             One of the given arguments is not valid.
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *             在不适当的对象上调用操作。
 *             The operation is invoked on an inappropriate object.
 * @retval DDS_RETCODE_ALREADY_DELETED
 *             实体已被删除。
 *             The entity has already been deleted.
 * @retval DDS_RETCODE_PRECONDITION_NOT_MET
 *             实例句柄尚未在此读取器中注册。
 *             The instance handle has not been registered with this reader.
 */
DDS_EXPORT dds_return_t dds_take_instance_mask(
    dds_entity_t reader_or_condition,  // 读取器、读取条件或查询条件实体。Reader, readcondition or
                                       // querycondition entity.
    void** buf,  // 指向样本的指针数组，用于读取数据。An array of pointers to samples into which
                 // data is read.
    dds_sample_info_t* si,  // 返回每个数据值的样本信息数组的指针。Pointer to an array of sample
                            // info returned for each data value.
    size_t bufsz,   // 提供的缓冲区大小。The size of buffer provided.
    uint32_t maxs,  // 要读取的最大样本数。Maximum number of samples to read.
    dds_instance_handle_t
        handle,  // 与要读取的样本相关的实例句柄。Instance handle related to the samples to read.
    uint32_t mask);  // 基于状态过滤数据。Filter the data based on states.

/**
 * @brief  基于掩码和给定的实例句柄范围，访问数据读取器、读取条件或查询条件的借用样本。
 *         Access loaned samples of data reader, readcondition or querycondition based
 *         on mask and scoped by the given intance handle.
 * @ingroup reading
 * @component read_data
 *
 * 此操作实现了与 dds_take_mask_wl 相同的功能，只是读取的数据仅限于提供的实例句柄范围。
 * This operation implements the same functionality as dds_take_mask_wl, except that
 * only data scoped to the provided instance handle is read.
 *
 * @param[in]  reader_or_condition Reader, readcondition or querycondition entity.
 *                                读取器、读取条件或查询条件实体。
 * @param[out] buf An array of pointers to samples into which data is read (pointers can be NULL).
 *                一个指向样本的指针数组，其中读取数据（指针可以为NULL）。
 * @param[out] si Pointer to an array of @ref dds_sample_info_t returned for each data value.
 *                指向为每个数据值返回的 @ref dds_sample_info_t 数组的指针。
 * @param[in]  maxs Maximum number of samples to read.
 *                 要读取的最大样本数。
 * @param[in]  handle Instance handle related to the samples to read.
 *                    与要读取的样本相关的实例句柄。
 * @param[in]  mask Filter the data based on
 *                  dds_sample_state_t|dds_view_state_t|dds_instance_state_t.
 *                  基于 dds_sample_state_t|dds_view_state_t|dds_instance_state_t 过滤数据。
 *
 * @returns A dds_return_t with the number of samples or an error code.
 *          返回一个包含样本数量或错误代码的 dds_return_t。
 *
 * @retval >= 0
 *             Number of samples read.
 *             读取的样本数。
 * @retval DDS_RETCODE_ERROR
 *             An internal error has occurred.
 *             发生了内部错误。
 * @retval DDS_RETCODE_BAD_PARAMETER
 *             One of the given arguments is not valid.
 *             给定参数之一无效。
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *             The operation is invoked on an inappropriate object
 *             在不适当的对象上调用操作。
 * @retval DDS_RETCODE_ALREADY_DELETED
 *             The entity has already been deleted.
 *             实体已被删除。
 * @retval DDS_RETCODE_PRECONDITION_NOT_MET
 *             The instance handle has not been registered with this reader.
 *             实例句柄尚未在此读取器中注册。
 */
DDS_EXPORT dds_return_t dds_take_instance_mask_wl(dds_entity_t reader_or_condition,
                                                  void** buf,
                                                  dds_sample_info_t* si,
                                                  uint32_t maxs,
                                                  dds_instance_handle_t handle,
                                                  uint32_t mask);

/**
 * @brief 读取、复制和移除实体的状态集 (Read, copy and remove the status set for the entity)
 * @ingroup reading
 * @component read_data
 *
 * 此操作从数据阅读器中复制下一个未访问过的数据值和相应的样本信息，并将其删除。作为实体，只接受阅读器。
 * (This operation copies the next, non-previously accessed
 * data value and corresponding sample info and removes from
 * the data reader. As an entity, only reader is accepted.)
 *
 * read/take next 函数返回单个样本。返回的样本具有 NOT_READ 的样本状态，ANY_VIEW_STATE 的视图状态和
 * ANY_INSTANCE_STATE 的实例状态。 (The read/take next functions return a single sample. The
 * returned sample has a sample state of NOT_READ, a view state of ANY_VIEW_STATE and an instance
 * state of ANY_INSTANCE_STATE.)
 *
 * @param[in]  reader 阅读器实体 (The reader entity)
 * @param[out] buf 读取数据的指针数组 (An array of pointers to samples into which data is read
 * (pointers can be NULL))
 * @param[out] si 返回数据值的 dds_sample_info_t 指针 (The pointer to @ref dds_sample_info_t
 * returned for a data value)
 *
 * @returns 表示成功或失败的 dds_return_t (A dds_return_t indicating success or failure)
 *
 * @retval DDS_RETCODE_OK
 *             操作成功 (The operation was successful)
 * @retval DDS_RETCODE_BAD_PARAMETER
 *             实体参数无效 (The entity parameter is not a valid parameter)
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *             在不适当的对象上调用操作 (The operation is invoked on an inappropriate object)
 * @retval DDS_RETCODE_ALREADY_DELETED
 *             实体已被删除 (The entity has already been deleted)
 */
DDS_EXPORT dds_return_t dds_take_next(dds_entity_t reader, void** buf, dds_sample_info_t* si);

/**
 * @brief 读取、复制并移除实体的状态集 (Read, copy and remove the status set for the entity)
 * @ingroup reading
 * @component read_data
 *
 * 此操作从数据阅读器中复制下一个未访问过的数据值和相应的样本信息，并将其删除。作为实体，只接受读取器。
 * (This operation copies the next, non-previously accessed
 * data value and corresponding sample info and removes from
 * the data reader. As an entity, only reader is accepted.)
 *
 * read/take next 函数返回单个样本。返回的样本具有 NOT_READ 的样本状态，ANY_VIEW_STATE 的视图状态和
 * ANY_INSTANCE_STATE 的实例状态。 (The read/take next functions return a single sample. The
 * returned sample has a sample state of NOT_READ, a view state of ANY_VIEW_STATE and an instance
 * state of ANY_INSTANCE_STATE.)
 *
 * 在调用 dds_take_next_wl 函数并处理数据后，必须调用 dds_return_loan() 函数以释放内存。
 * (After dds_take_next_wl function is being called and the data has been handled,
 * dds_return_loan() function must be called to possibly free memory.)
 *
 * @param[in]  reader 阅读器实体 (The reader entity.)
 * @param[out] buf 读取数据的指针数组 (An array of pointers to samples into which data is read
 * (pointers can be NULL).)
 * @param[out] si 返回数据值的 @ref dds_sample_info_t 指针 (The pointer to @ref dds_sample_info_t
 * returned for a data value.)
 *
 * @returns 表示成功或失败的 dds_return_t (A dds_return_t indicating success or failure.)
 *
 * @retval DDS_RETCODE_OK
 *             操作成功 (The operation was successful.)
 * @retval DDS_RETCODE_BAD_PARAMETER
 *             实体参数无效 (The entity parameter is not a valid parameter.)
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *             在不适当的对象上调用操作 (The operation is invoked on an inappropriate object.)
 * @retval DDS_RETCODE_ALREADY_DELETED
 *             实体已被删除 (The entity has already been deleted.)
 */
DDS_EXPORT dds_return_t dds_take_next_wl(dds_entity_t reader, void** buf, dds_sample_info_t* si);

/**
 * @brief 读取并复制实体的状态集 (Read and copy the status set for the entity)
 * @ingroup reading
 * @component read_data
 *
 * 此操作复制下一个未访问过的数据值和相应的样本信息。作为实体，只接受读取器。(This operation copies
 * the next, non-previously accessed data value and corresponding sample info. As an entity, only
 * reader is accepted.)
 *
 * read/take next 函数返回单个样本。返回的样本具有 NOT_READ 的样本状态、ANY_VIEW_STATE 的视图状态和
 * ANY_INSTANCE_STATE 的实例状态。 (The read/take next functions return a single sample. The
 * returned sample has a sample state of NOT_READ, a view state of ANY_VIEW_STATE and an instance
 * state of ANY_INSTANCE_STATE.)
 *
 * @param[in]  reader 读取器实体 (The reader entity)
 * @param[out] buf 读取数据的指针数组 (An array of pointers to samples into which data is read
 * (pointers can be NULL))
 * @param[out] si 为数据值返回的 @ref dds_sample_info_t 指针 (The pointer to @ref dds_sample_info_t
 * returned for a data value)
 *
 * @returns 表示成功或失败的 dds_return_t (A dds_return_t indicating success or failure)
 *
 * @retval DDS_RETCODE_OK
 *             操作成功 (The operation was successful)
 * @retval DDS_RETCODE_BAD_PARAMETER
 *             实体参数无效 (The entity parameter is not a valid parameter)
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *             在不适当的对象上调用操作 (The operation is invoked on an inappropriate object)
 * @retval DDS_RETCODE_ALREADY_DELETED
 *             实体已被删除 (The entity has already been deleted)
 */
DDS_EXPORT dds_return_t dds_read_next(dds_entity_t reader, void** buf, dds_sample_info_t* si);

/**
 * @brief 读取并复制已借出样本的状态集 (Read and copy the status set for the loaned sample)
 * @ingroup reading
 * @component read_data
 *
 * 此操作将复制下一个未访问过的数据值和相应的借出样本信息。作为实体，只接受阅读器。 (This operation
 * copies the next, non-previously accessed data value and corresponding loaned sample info. As an
 * entity, only reader is accepted.)
 *
 * read/take next 函数返回单个样本。返回的样本具有 NOT_READ 的样本状态、ANY_VIEW_STATE 的视图状态和
 * ANY_INSTANCE_STATE 的实例状态。 (The read/take next functions return a single sample. The
 * returned sample has a sample state of NOT_READ, a view state of ANY_VIEW_STATE and an instance
 * state of ANY_INSTANCE_STATE.)
 *
 * 在调用 dds_read_next_wl 函数并处理数据后，必须调用 dds_return_loan() 函数以可能释放内存。 (After
 * dds_read_next_wl function is being called and the data has been handled, dds_return_loan()
 * function must be called to possibly free memory.)
 *
 * @param[in]  reader 读者实体 (The reader entity)
 * @param[out] buf 读取数据的指针数组 (An array of pointers to samples into which data is read
 * (pointers can be NULL))
 * @param[out] si 返回数据值的 @ref dds_sample_info_t 指针 (The pointer to @ref dds_sample_info_t
 * returned for a data value)
 *
 * @returns 表示成功或失败的 dds_return_t (A dds_return_t indicating success or failure)
 *
 * @retval DDS_RETCODE_OK
 *             操作成功 (The operation was successful)
 * @retval DDS_RETCODE_BAD_PARAMETER
 *             实体参数无效 (The entity parameter is not a valid parameter)
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *             在不适当的对象上调用操作 (The operation is invoked on an inappropriate object)
 * @retval DDS_RETCODE_ALREADY_DELETED
 *             实体已被删除 (The entity has already been deleted)
 */
DDS_EXPORT dds_return_t dds_read_next_wl(dds_entity_t reader, void** buf, dds_sample_info_t* si);

/**
 * @defgroup loan (Loans API)
 * @ingroup dds
 */

/**
 * @brief Return loaned samples to a reader or writer
 * @ingroup loan
 * @component read_data
 *
 * 用于释放由读取/获取操作返回的样本缓冲区（reader-loan）
 * 或者，在启用共享内存的情况下，由 loan_sample 操作返回的样本缓冲区（writer-loan）。
 * Used to release sample buffers returned by a read/take operation (a reader-loan)
 * or, in case shared memory is enabled, of the loan_sample operation (a writer-loan).
 *
 * 当应用程序向 reader-loan 提供一个空缓冲区时，内存由 DDS 分配和管理。
 * 通过调用 dds_return_loan()，释放 reader-loan，以便在后续的读取/获取操作中重用缓冲区。
 * 当提供条件时，将查找条件所属的读取器。
 * When the application provides an empty buffer to a reader-loan, memory is allocated and
 * managed by DDS. By calling dds_return_loan(), the reader-loan is released so that the buffer
 * can be reused during a successive read/take operation. When a condition is provided, the
 * reader to which the condition belongs is looked up.
 *
 * Writer-loans 通常在写入已借用的样本时隐式释放，但您可以通过调用 return_loan() 操作提前取消
 * writer-loan。 对于 writer loans，buf
 * 中所有成功返回的条目都被覆盖为空指针。任何失败都会导致它中止，可能在 buf 的中途。 Writer-loans
 * are normally released implicitly when writing a loaned sample, but you can cancel a writer-loan
 * prematurely by invoking the return_loan() operation. For writer loans, buf is overwritten with
 * null pointers for all successfully returned entries. Any failure causes it to abort, possibly
 * midway through buf.
 *
 * @param[in] entity 所属贷款的实体。
 * @param[in] entity The entity that the loan belongs to.
 * @param[in,out] buf 一个包含（指向）样本的数组，其中部分或全部将设置为 null 指针。
 * @param[in,out] buf An array of (pointers to) samples, some or all of which will be set to null
 * pointers.
 * @param[in] bufsz 存储在 buf 中的（指向）样本的数量。
 * @param[in] bufsz The number of (pointers to) samples stored in buf.
 *
 * @returns 表示成功或失败的 dds_return_t
 * @returns A dds_return_t indicating success or failure
 * @retval DDS_RETCODE_OK
 *             - 操作成功；对于 writer loan，buf 中的所有条目都设置为 null
 *             - this specifically includes cases where bufsz <= 0 while entity is valid
 * @retval DDS_RETCODE_BAD_PARAMETER
 *             - 实体参数无效
 *             - buf 为空，或 bufsz > 0 且 buf[0] = null
 *             - （对于 writer loans）buf[0 <= i < bufsz] 为空；操作中止，所有 buf[j <
 * i] = null 返回
 * @retval DDS_RETCODE_PRECONDITION_NOT_MET
 *             - （对于 reader loans）buf 已经返回（不能保证检测到）
 *             - （对于 writer loans）buf[0 <= i < bufsz] 不对应未完成的贷款，
 * 所有 buf[j < i] = null 返回
 * @retval DDS_RETCODE_UNSUPPORTED
 *             - （对于 writer loans）在不支持贷款的写入器上调用。
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *             - 在不适当的对象上调用操作。
 */
DDS_EXPORT dds_return_t dds_return_loan(dds_entity_t entity, void** buf, int32_t bufsz);

/**
 * @defgroup instance_handle (实例句柄)
 * @ingroup dds
 * 实例句柄 <=> 键值映射。
 * 与读取数据相同的处理方式。
 * 输出时，仅设置键值。
 * @code{c}
 * T x = { ... };
 * T y;
 * dds_instance_handle_t ih;
 * ih = dds_lookup_instance (e, &x);
 * dds_instance_get_key (e, ih, &y);
 * @endcode
 */

/**
 * @brief 此操作接收一个样本并返回用于后续操作的实例句柄。
 * This operation takes a sample and returns an instance handle to be used for subsequent
 * operations.
 * @ingroup instance_handle
 * @component data_instance
 *
 * @param[in]  entity 读者或写者实体。Reader or Writer entity.
 * @param[in]  data   设置了关键字段的样本。Sample with key fields set.
 *
 * @returns 如果无法从键中找到实例，则返回实例句柄或DDS_HANDLE_NIL。
 *          instance handle or DDS_HANDLE_NIL if instance could not be found from key.
 */
DDS_EXPORT dds_instance_handle_t dds_lookup_instance(dds_entity_t entity, const void* data);

/**
 * @brief 此操作接收一个实例句柄并返回与其对应的键值。
 * This operation takes an instance handle and return a key-value corresponding to it.
 * @ingroup instance_handle
 * @component data_instance
 *
 * @param[in]  entity 读者、写者、读条件或查询条件实体。Reader, writer, readcondition or
 * querycondition entity.
 * @param[in]  inst   实例句柄。Instance handle.
 * @param[out] data   指向一个实例的指针，将返回与实例句柄对应的键ID，应忽略实例中的样本。
 *                    pointer to an instance, to which the key ID corresponding to the instance
 *                    handle will be returned, the sample in the instance should be ignored.
 *
 * @returns 表示成功或失败的dds_return_t。
 *          A dds_return_t indicating success or failure.
 *
 * @retval DDS_RETCODE_OK
 *             操作成功。The operation was successful.
 * @retval DDS_RETCODE_BAD_PARAMETER
 *             参数之一无效或主题不存在。One of the parameters was invalid or the topic does not
 * exist.
 * @retval DDS_RETCODE_ERROR
 *             发生内部错误。An internal error has occurred.
 *
 * DOC_TODO: 检查返回代码的完整性
 *           Check return codes for completeness
 */
DDS_EXPORT dds_return_t dds_instance_get_key(dds_entity_t entity,
                                             dds_instance_handle_t inst,
                                             void* data);

/**
 * @brief 开始连贯发布或在订阅者中开始访问连贯集 (Begin coherent publishing or begin accessing a
 * coherent set in a subscriber)
 * @ingroup publication
 * @component coherent_sets
 *
 * 在 Writer 或 Reader 上调用的行为就像在其父 Publisher 或 Subscriber 上分别调用了
 * dds_begin_coherent。 (Invoking on a Writer or Reader behaves as if dds_begin_coherent was invoked
 * on its parent Publisher or Subscriber respectively.)
 *
 * @param[in]  entity 准备进行连贯访问的实体 (The entity that is prepared for coherent access.)
 *
 * @returns 表示成功或失败的 dds_return_t (A dds_return_t indicating success or failure.)
 *
 * @retval DDS_RETCODE_OK
 *             操作成功 (The operation was successful.)
 * @retval DDS_RETCODE_ERROR
 *             发生内部错误 (An internal error has occurred.)
 * @retval DDS_RETCODE_BAD_PARAMETER
 *             提供的实体无效或不受支持 (The provided entity is invalid or not supported.)
 */
DDS_EXPORT dds_return_t dds_begin_coherent(dds_entity_t entity);

/**
 * @brief 结束连贯发布或在订阅者中结束访问连贯集 (End coherent publishing or end accessing a
 * coherent set in a subscriber)
 * @ingroup publication
 * @component coherent_sets
 *
 * 在 Writer 或 Reader 上调用的行为就像在其父 Publisher 或 Subscriber 上分别调用了
 * dds_end_coherent。 (Invoking on a Writer or Reader behaves as if dds_end_coherent was invoked on
 * its parent Publisher or Subscriber respectively.)
 *
 * @param[in] entity 连贯访问已完成的实体 (The entity on which coherent access is finished.)
 *
 * @returns 表示成功或失败的 dds_return_t (A dds_return_t indicating success or failure.)
 *
 * @retval DDS_RETCODE_OK
 *             操作成功 (The operation was successful.)
 * @retval DDS_RETCODE_BAD_PARAMETER
 *             提供的实体无效或不受支持 (The provided entity is invalid or not supported.)
 */
DDS_EXPORT dds_return_t dds_end_coherent(dds_entity_t entity);

/**
 * @brief 在包含的读取器上触发 DATA_AVAILABLE 事件 (Trigger DATA_AVAILABLE event on contained
 * readers)
 * @ingroup subscriber
 * @component subscriber
 *
 * 将 DATA_AVAILABLE 事件广播给当前具有新数据可用的此订阅者拥有的所有读取器。
 * (The DATA_AVAILABLE event is broadcast to all readers owned by this subscriber that currently
 * have new data available.)
 * 分别调用附加到相应读取器的任何 on_data_available 监听器回调。
 * (Any on_data_available listener callbacks attached to respective
 * readers are invoked.)
 *
 * @param[in] subscriber 有效的订阅者句柄 (A valid subscriber handle.)
 *
 * @returns 表示成功或失败的 dds_return_t (A dds_return_t indicating success or failure.)
 *
 * @retval DDS_RETCODE_OK
 *             操作成功 (The operation was successful.)
 * @retval DDS_RETCODE_BAD_PARAMETER
 *             提供的订阅者无效 (The provided subscriber is invalid.)
 */
DDS_EXPORT dds_return_t dds_notify_readers(dds_entity_t subscriber);

/**
 * @brief 检查实体是否触发了其中一个启用的状态。
 * @brief Checks whether the entity has one of its enabled statuses triggered.
 * @ingroup entity
 * @component entity_status
 *
 * @param[in]  entity  要检查触发状态的实体。
 * @param[in]  entity  Entity for which to check for triggered status.
 *
 * @returns 表示成功或失败的 dds_return_t。
 * @returns A dds_return_t indicating success or failure.
 *
 * @retval DDS_RETCODE_OK
 *             操作成功。
 *             The operation was successful.
 * @retval DDS_RETCODE_BAD_PARAMETER
 *             实体参数无效。
 *             The entity parameter is not a valid parameter.
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *             在不适当的对象上调用操作。
 *             The operation is invoked on an inappropriate object.
 * @retval DDS_RETCODE_ALREADY_DELETED
 *             实体已被删除。
 *             The entity has already been deleted.
 */
DDS_EXPORT dds_return_t dds_triggered(dds_entity_t entity);

/**
 * @brief 获取主题
 * @brief Get the topic
 * @ingroup entity
 * @component entity_relations
 *
 * 当使用读取器、写入器、读取条件或查询条件完成函数调用时，此操作返回一个主题（句柄）。
 * This operation returns a topic (handle) when the function call is done
 * with reader, writer, read condition or query condition. For instance, it
 * will return the topic when it is used for creating the reader or writer.
 * 对于条件，它返回用于创建读取器的主题，该读取器用于创建条件。
 * For the conditions, it returns the topic that is used for creating the reader
 * which was used to create the condition.
 *
 * @param[in] entity 实体。
 * @param[in] entity The entity.
 *
 * @returns 表示成功或失败的 dds_return_t。
 * @returns A dds_return_t indicating success or failure.
 *
 * @retval DDS_RETCODE_OK
 *             操作成功。
 *             The operation was successful.
 * @retval DDS_RETCODE_BAD_PARAMETER
 *             实体参数无效。
 *             The entity parameter is not a valid parameter.
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *             在不适当的对象上调用操作。
 *             The operation is invoked on an inappropriate object.
 * @retval DDS_RETCODE_ALREADY_DELETED
 *             实体已被删除。
 *             The entity has already been deleted.
 */
DDS_EXPORT dds_entity_t dds_get_topic(dds_entity_t entity);

/**
 * @brief 获取与写入器匹配的数据读取器的实例句柄
 * @brief Get instance handles of the data readers matching a writer
 * @ingroup builtintopic
 * @component writer
 *
 * 此操作使用与写入器匹配的数据读取器的实例句柄填充提供的数组。
 * This operation fills the provided array with the instance handles
 * of the data readers that match the writer.  On successful output,
 * "rds"设置的条目数是返回值和"nrds"值的最小值。
 * the number of entries of "rds" set is the minimum of the return
 * value and the value of "nrds".
 *
 * @param[in] writer   写入器。
 * @param[in] writer   The writer.
 * @param[in] rds      要填充的数组。
 * @param[in] rds      The array to be filled.
 * @param[in] nrds     rds 数组的大小，最多填充前 nrds 个条目。rds = NULL 和 nrds = 0
 * 是确定匹配读取器数量的有效方法，但与依赖匹配发布状态相比效率较低。
 * @param[in] nrds     The size of the rds array, at most the first
 *                     nrds entries will be filled.  rds = NULL and nrds = 0
 *                     is a valid way of determining the number of matched
 *                     readers, but inefficient compared to relying on the
 *                     matched publication status.
 *
 * @returns 表示匹配读取器数量或失败的
 * dds_return_t。如果有更多匹配的读取器而数组无法容纳，则返回值可能大于 nrds。
 * @returns A dds_return_t indicating the number of matched readers
 *             or failure.  The return value may be larger than nrds
 *             if there are more matching readers than the array can
 *             hold.
 *
 * @retval >=0
 *             匹配读取器的数量。
 *             The number of matching readers.
 * @retval DDS_RETCODE_BAD_PARAMETER
 *             实体参数无效，或者 rds = NULL 和 nrds > 0。
 *             The entity parameter is not valid or rds = NULL and
 *             nrds > 0.
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *             在不适当的对象上调用操作。
 *             The operation is invoked on an inappropriate object.
 */
DDS_EXPORT dds_return_t dds_get_matched_subscriptions(dds_entity_t writer,
                                                      dds_instance_handle_t* rds,
                                                      size_t nrds);

/**
 * @brief 获取与提供的写入器匹配的读取器的描述 (Get a description of a reader matched with the
 * provided writer)
 * @ingroup builtintopic
 * @component writer
 *
 * 此操作在与指定写入器匹配的读取器集中查找读取器实例句柄，如果找到，则返回新分配的 DCPSSubscription
 * 内置主题样本， 否则返回 NULL。调用者负责释放分配的内存，例如使用 dds_builtintopic_free_endpoint。
 * (This operation looks up the reader instance handle in the set of
 * readers matched with the specified writer, returning a freshly
 * allocated sample of the DCPSSubscription built-in topic if found,
 * and NULL if not.  The caller is responsible for freeing the
 * memory allocated, e.g. using dds_builtintopic_free_endpoint.)
 *
 * 此操作类似于在 DCPSSubscription 内置主题的读取器上执行给定实例句柄的读取，
 * 但此操作还会根据提供的写入器是否匹配读取器进行过滤。
 * (This operation is similar to performing a read of the given
 * instance handle on a reader of the DCPSSubscription built-in
 * topic, but this operation additionally filters on whether the
 * reader is matched by the provided writer.)
 *
 * @param[in] writer   写入器 (The writer)
 * @param[in] ih       读取器的实例句柄 (The instance handle of a reader)
 *
 * @returns 新分配的样本，其中包含有关读取器的信息，或者对于任何类型的失败，返回 NULL 指针。
 * (A newly allocated sample containing the information on the
 *             reader, or a NULL pointer for any kind of failure.)
 *
 * @retval != NULL
 *             请求的数据 (The requested data)
 * @retval NULL
 *             写入器无效或 ih 不是匹配读取器的实例句柄 (The writer is not valid or ih is not an
 * instance handle of a matched reader)
 */
DDS_EXPORT dds_builtintopic_endpoint_t* dds_get_matched_subscription_data(dds_entity_t writer,
                                                                          dds_instance_handle_t ih);

/**
 * @brief 获取与读取器匹配的数据写入器的实例句柄 (Get instance handles of the data writers matching
 * a reader)
 * @ingroup builtintopic
 * @component reader
 *
 * 此操作使用与读取器匹配的数据写入器的实例句柄填充提供的数组。在成功输出时，
 * "wrs" 设置的条目数是返回值和 "nwrs" 值的最小值。
 * (This operation fills the provided array with the instance handles
 * of the data writers that match the reader.  On successful output,
 * the number of entries of "wrs" set is the minimum of the return
 * value and the value of "nwrs".)
 *
 * @param[in] reader   读取器 (The reader)
 * @param[in] wrs      要填充的数组 (The array to be filled)
 * @param[in] nwrs     wrs 数组的大小，最多填充前 nwrs 个条目。wrs = NULL 和 wrds = 0
 *             是确定匹配读取器数量的有效方法，但与依赖匹配发布状态相比效率低。
 *             (The size of the wrs array, at most the first
 *             nwrs entries will be filled.  wrs = NULL and wrds = 0
 *             is a valid way of determining the number of matched
 *             readers, but inefficient compared to relying on the
 *             matched publication status.)
 *
 * @returns 表示匹配写入器数量或失败的 dds_return_t。如果有更多匹配的写入器而数组无法容纳，
 *             则返回值可能大于 nwrs。
 *             (A dds_return_t indicating the number of matched writers
 *             or failure.  The return value may be larger than nwrs
 *             if there are more matching writers than the array can
 *             hold.)
 *
 * @retval >=0
 *             匹配写入器的数量 (The number of matching writers)
 * @retval DDS_RETCODE_BAD_PARAMETER
 *             实体参数无效或 wrs = NULL 和 nwrs > 0。
 *             (The entity parameter is not valid or wrs = NULL and
 *             nwrs > 0)
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *             在不适当的对象上调用操作 (The operation is invoked on an inappropriate object)
 */
DDS_EXPORT dds_return_t dds_get_matched_publications(dds_entity_t reader,
                                                     dds_instance_handle_t* wrs,
                                                     size_t nwrs);

/**
 * @brief 获取与提供的读取器匹配的写入器的描述 (Get a description of a writer matched with the
 * provided reader)
 * @ingroup builtintopic
 * @component reader
 *
 * 此操作在与指定读取器匹配的写入器集合中查找写入器实例句柄，如果找到，则返回新分配的
 * DCPSPublication 内置主题样本， 否则返回 NULL。调用者负责释放分配的内存，例如使用
 * dds_builtintopic_free_endpoint。 (This operation looks up the writer instance handle in the set
 * of writers matched with the specified reader, returning a freshly allocated sample of the
 * DCPSPublication built-in topic if found, and NULL if not.  The caller is responsible for freeing
 * the memory allocated, e.g. using dds_builtintopic_free_endpoint.)
 *
 * 此操作类似于在 DCPSPublication 内置主题的读取器上执行给定实例句柄的读取，
 * 但此操作还会根据提供的读取器是否匹配写入器进行过滤。
 * (This operation is similar to performing a read of the given
 * instance handle on a reader of the DCPSPublication built-in
 * topic, but this operation additionally filters on whether the
 * writer is matched by the provided reader.)
 *
 * @param[in] reader   读取器 (The reader)
 * @param[in] ih       写入器的实例句柄 (The instance handle of a writer)
 *
 * @returns 包含有关写入器信息的新分配的样本，或者在任何类型的故障时返回 NULL 指针。
 *          (A newly allocated sample containing the information on the
 *          writer, or a NULL pointer for any kind of failure.)
 *
 * @retval != NULL
 *             请求的数据 (The requested data)
 * @retval NULL
 *             读取器无效或 ih 不是匹配写入器的实例句柄。
 *             (The reader is not valid or ih is not an instance handle
 *             of a matched writer.)
 */
DDS_EXPORT dds_builtintopic_endpoint_t* dds_get_matched_publication_data(dds_entity_t reader,
                                                                         dds_instance_handle_t ih);

#ifdef DDS_HAS_TYPE_DISCOVERY
/**
 * @brief 从通过 dds_get_matched_subscription_data 或
 *        dds_get_matched_publication_data 检索的端点信息中获取类型信息
 *        (Gets the type information from endpoint information that was
 *        retrieved by dds_get_matched_subscription_data or
 *        dds_get_matched_publication_data)
 * @ingroup builtintopic
 * @component builtin_topic
 *
 * @param[in] builtintopic_endpoint  内置主题端点结构 (The builtintopic endpoint struct)
 * @param[out] type_info             类型信息，如果成功，将由此函数分配。
 *                                   (Type information that will be allocated by this function in
 *                                   case of success.)
 *
 * @returns 表示成功或失败的 dds_return_t。 (A dds_return_t indicating success or failure.)
 *
 * @retval DDS_RETCODE_OK
 *             操作成功 (The operation was successful)
 * @retval DDS_RETCODE_BAD_PARAMETER
 *             一个或多个参数无效 (One or more parameters are invalid)
 */
DDS_EXPORT dds_return_t dds_builtintopic_get_endpoint_type_info(
    dds_builtintopic_endpoint_t* builtintopic_endpoint, const dds_typeinfo_t** type_info);
#endif

/**
 * @brief 释放由 dds_get_matched_subscription_data 或 dds_get_matched_publication_data
 * 检索的端点信息 (Free the endpoint information that was retrieved by
 * dds_get_matched_subscription_data or dds_get_matched_publication_data)
 * @ingroup builtintopic
 * @component builtin_topic
 *
 * 此操作释放 dds_builtintopic_endpoint_t 结构中字段的内存，并释放结构本身。(This operation
 * deallocates the memory of the fields in a dds_builtintopic_endpoint_t struct and deallocates the
 * struct itself.)
 *
 * @param[in] builtintopic_endpoint  内置主题端点结构 (The builtintopic endpoint struct)
 */
DDS_EXPORT void dds_builtintopic_free_endpoint(dds_builtintopic_endpoint_t* builtintopic_endpoint);

/**
 * @brief 释放提供的主题信息 (Free the provided topic information)
 * @ingroup builtintopic
 * @component builtin_topic
 *
 * 此操作释放 dds_builtintopic_topic_t 结构中字段的内存，并释放结构本身。(This operation deallocates
 * the memory of the fields in a dds_builtintopic_topic_t struct and deallocates the struct itself.)
 *
 * @param[in] builtintopic_topic  内置主题结构 (The builtintopic topic struct)
 */
DDS_EXPORT void dds_builtintopic_free_topic(dds_builtintopic_topic_t* builtintopic_topic);

/**
 * @brief 释放提供的参与者信息 (Free the provided participant information)
 * @ingroup builtintopic
 * @component builtin_topic
 *
 * 此操作释放 dds_builtintopic_participant_t 结构中字段的内存，并释放结构本身。(This operation
 * deallocates the memory of the fields in a dds_builtintopic_participant_t struct and deallocates
 * the struct itself.)
 *
 * @param[in] builtintopic_participant  内置主题参与者结构 (The builtintopic participant struct)
 */
DDS_EXPORT void dds_builtintopic_free_participant(
    dds_builtintopic_participant_t* builtintopic_participant);

/**
 * @brief 此操作手动声明写入器或域参与者的活跃性 (This operation manually asserts the liveliness of
 * a writer or domain participant.)
 * @ingroup entity
 * @component participant
 *
 * 此操作手动声明写入器或域参与者的活跃性。这与 Liveliness QoS
 * 策略结合使用，以指示实体仍然活动。只有在 QoS 中的活跃性类型为
 * DDS_LIVELINESS_MANUAL_BY_PARTICIPANT 或 DDS_LIVELINESS_MANUAL_BY_TOPIC
 * 时，才需要使用此操作。(This operation manually asserts the liveliness of a writer or domain
 * participant. This is used in combination with the Liveliness QoS policy to indicate that the
 * entity remains active. This operation need only be used if the liveliness kind in the QoS is
 * either DDS_LIVELINESS_MANUAL_BY_PARTICIPANT or DDS_LIVELINESS_MANUAL_BY_TOPIC.)
 *
 * @param[in] entity  域参与者或写入器 (A domain participant or writer)
 *
 * @returns 指示成功或失败的 dds_return_t。(A dds_return_t indicating success or failure.)
 *
 * @retval DDS_RETCODE_OK
 *             操作成功。(The operation was successful.)
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *             在不适当的对象上调用操作。(The operation is invoked on an inappropriate object.)
 */
DDS_EXPORT dds_return_t dds_assert_liveliness(dds_entity_t entity);

/**
 * @defgroup internal (内部)
 * @ingroup dds
 */

/**
 * @defgroup testing (测试工具)
 * @ingroup internal
 */

/**
 *
 * @brief 这个操作允许临时使域的网络堆栈变得失聪和/或哑巴。
 * @ingroup testing
 * @component domain
 * @warning 不稳定的API，用于测试
 * @unstable
 *
 * 这是一个用于测试和其他特殊用途的支持函数，可能会发生变化。
 *
 * @param[in] entity  域实体或绑定到域的实体，如
 *                    参与者、读取器或写入器。
 * @param[in] deaf    网络堆栈是否应该假装失聪并
 *                    忽略任何传入的数据包。
 * @param[in] mute    网络堆栈是否应该假装哑巴并
 *                    丢弃通常会传递给操作系统内核进行传输的任何出站数据包。
 * @param[in] reset_after  小于INFINITY的任何值都会导致它在reset_after ns过去后
 *                    设置deaf = mute = false。
 *                    这是通过为适当时间安排事件并忘记其他事情来完成的。这些事件不受
 *                    随后调用此函数的影响。
 *
 * @returns 表示成功或失败的dds_return_t。
 *
 * @retval DDS_RETCODE_OK
 *             操作成功。
 * @retval DDS_RETCODE_BAD_PARAMETER
 *             实体参数无效。
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *             在不适当的对象上调用操作。
 */
DDS_EXPORT dds_return_t dds_domain_set_deafmute(dds_entity_t entity,
                                                bool deaf,
                                                bool mute,
                                                dds_duration_t reset_after);

/**
 * @defgroup xtypes (XTypes)
 * @ingroup dds
 *
 * CycloneDDS支持XTypes，但除了新的IDL构造之外，大部分功能都在幕后进行。然而，还添加了一些API功能，允许在运行时检查类型。在C中使用它并不是很方便，但像Python这样的动态语言可以很好地利用它。
 */

/**
 * @brief 此函数解析提供的类型标识符的类型，
 * 例如，可以从端点或主题发现数据中获取。
 * @ingroup xtypes
 * @component type_metadata
 *
 * @param[in]   entity              域实体或绑定到域的实体，如
 *                                  参与者、读取器或写入器。
 * @param[in]   type_id             类型标识符
 * @param[in]   timeout             等待请求的类型信息可用的超时时间
 * @param[out]  type_obj            类型信息，如果类型未解析，则保持不变
 *
 *
 * @returns 表示成功或失败的dds_return_t。
 *
 * @retval DDS_RETCODE_OK
 *             操作成功。
 * @retval DDS_RETCODE_BAD_PARAMETER
 *             实体参数无效，未提供类型标识符或类型名称
 *             或sertype输出参数为NULL
 * @retval DDS_RETCODE_NOT_FOUND
 *             未找到具有提供的type_id和type_name的类型
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *             在不适当的对象上调用操作。
 * @retval DDS_RETCODE_UNSUPPORTED
 *             Cyclone DDS构建时没有类型发现
 *             (参见DDS_HAS_TYPE_DISCOVERY)
 */
DDS_EXPORT dds_return_t dds_get_typeobj(dds_entity_t entity,
                                        const dds_typeid_t* type_id,
                                        dds_duration_t timeout,
                                        dds_typeobj_t** type_obj);

/**
 * @brief 释放使用 dds_get_typeobj 获取的类型对象 (Free the type object that was retrieved using
 * dds_get_typeobj)
 * @ingroup xtypes
 * @component type_metadata
 *
 * @param[in]  type_obj     类型对象 (The type object)
 *
 *
 * @returns 表示成功或失败的 dds_return_t (A dds_return_t indicating success or failure)
 *
 * @retval DDS_RETCODE_OK
 *             操作成功 (The operation was successful)
 * @retval DDS_RETCODE_BAD_PARAMETER
 *             type_obj 参数为 NULL (The type_obj parameter is NULL)
 * @retval DDS_RETCODE_UNSUPPORTED
 *             Cyclone DDS 构建时未启用类型发现 (Cyclone DDS built without type discovery)
 *             (参见 DDS_HAS_TYPE_DISCOVERY) (cf. DDS_HAS_TYPE_DISCOVERY)
 */
DDS_EXPORT dds_return_t dds_free_typeobj(dds_typeobj_t* type_obj);

/**
 * @brief 此函数从提供的主题、读取器或写入器中获取类型信息 (This function gets the type information
 * from the provided topic, reader or writer)
 * @ingroup xtypes
 * @component type_metadata
 *
 * @param[in]   entity          主题/读取器/写入器实体 (A topic/reader/writer entity)
 * @param[out]  type_info       类型信息，如果返回代码表示失败，则不会更改 (The type information,
 * untouched if returncode indicates failure)
 *
 *
 * @returns 表示成功或失败的 dds_return_t (A dds_return_t indicating success or failure)
 *
 * @retval DDS_RETCODE_OK
 *             操作成功 (The operation was successful)
 * @retval DDS_RETCODE_BAD_PARAMETER
 *             type_info 参数为 null (The type_info parameter is null)
 * @retval DDS_RETCODE_NOT_FOUND
 *             实体未设置类型信息 (The entity does not have type information set)
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *             在不适当的对象上调用操作 (The operation is invoked on an inappropriate object)
 * @retval DDS_RETCODE_UNSUPPORTED
 *             Cyclone DDS 构建时未启用类型发现 (Cyclone DDS built without type discovery)
 *             (参见 DDS_HAS_TYPE_DISCOVERY) (cf. DDS_HAS_TYPE_DISCOVERY)
 */
DDS_EXPORT dds_return_t dds_get_typeinfo(dds_entity_t entity, dds_typeinfo_t** type_info);

/**
 * @brief 释放使用 dds_get_typeinfo 获取的类型信息 (Free the type information that was retrieved
 * using dds_get_typeinfo)
 * @ingroup xtypes
 * @component type_metadata
 *
 * @param[in]  type_info     类型信息 (The type information)
 *
 *
 * @returns 表示成功或失败的 dds_return_t (A dds_return_t indicating success or failure)
 *
 * @retval DDS_RETCODE_OK
 *             操作成功 (The operation was successful)
 * @retval DDS_RETCODE_BAD_PARAMETER
 *             type_info 参数为 NULL (The type_info parameter is NULL)
 * @retval DDS_RETCODE_UNSUPPORTED
 *             Cyclone DDS 构建时未启用类型发现 (Cyclone DDS built without type discovery)
 *             (参见 DDS_HAS_TYPE_DISCOVERY) (cf. DDS_HAS_TYPE_DISCOVERY)
 */
DDS_EXPORT dds_return_t dds_free_typeinfo(dds_typeinfo_t* type_info);

#if defined(__cplusplus)
}
#endif
#endif /* DDS_H */
