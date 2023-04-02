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
/* TODO: do we really need to expose all of this as an API? maybe some, but all? */

/** @file
 *
 * @brief DDS C Implementation API
 *
 * This header file defines the public API for all kinds of things in the
 * Eclipse Cyclone DDS C language binding.
 */
#ifndef DDS_IMPL_H
#define DDS_IMPL_H

#include <stdbool.h>
#include <stdint.h>

#include "dds/ddsc/dds_opcodes.h"
#include "dds/ddsc/dds_public_alloc.h"
#include "dds/ddsrt/align.h"
#include "dds/export.h"
#include "dds/features.h"

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @defgroup implementation (Public Implementation Details)
 * @ingroup dds
 * 杂项类型和函数，需要公开，因为它们在 IDL 编译器的输出中，但不是直接使用。
 * Miscellaneous types and functions that are required to be public, since they are
 * in the output of the IDL compiler, but are not intended for direct use.
 */

/**
 * @ingroup implementation
 * @brief Datastructure of a Sequence type
 * 用于存储字节序列的容器。这种类型的通用模型也用于 IDL 输出，
 * 其中 uint8_t * _buffer 被所包含的适当子类型替换。
 * Container for a sequence of bytes. The general model of this type is also used in IDL output,
 * where the uint8_t * _buffer is replaced by the appropriate subtype of what is contained.
 */
typedef struct dds_sequence {
  uint32_t _maximum; /**< 分配给 _buffer 的空间 */
                     /**< Allocated space in _buffer */
  uint32_t _length;  /**< _buffer 中使用的空间 */
                     /**< Used space in _buffer */
  uint8_t* _buffer;  /**< 字节序列 */
                     /**< Sequence of bytes */
  bool _release;     /**< CycloneDDS _free 方法是否应释放包含的缓冲区。
                      如果您将自己分配的 _buffer 放入其中，请将此设置为 false 以避免
                      CycloneDDS 在其上调用 free()。*/
                     /**< Whether a CycloneDDS _free method should free the contained buffer.
                      if you put in your own allocated _buffer set this to false to avoid
                      CycloneDDS calling free() on it. */
} dds_sequence_t;

/**
 * @ingroup implementation
 * @brief Key Descriptor
 * 用于描述类型中命名键字段与结构开始处的偏移量。
 * Used to describe a named key field in a type with the offset from the start of a struct.
 */
typedef struct dds_key_descriptor {
  const char* m_name; /**< 键字段名称 */
                      /**< name of keyfield */
  uint32_t m_offset;  /**< 指针偏移量 */
                      /**< offset from pointer */
  uint32_t m_idx;     /**< 类型的第 m_idx 个键 */
                      /**< m_idx'th key of type */
} dds_key_descriptor_t;

/**
 * @defgroup topic_definition (主题定义 Topic Definition)
 * @ingroup implementation
 * 主题定义由 IDL 编译器输出，并具有实现私有的定义。API 上公开的唯一内容是指向
 * "dds_topic_descriptor_t" 结构类型的指针。 Topic definitions are output by the IDL compiler and
 * have an implementation-private definition. The only thing exposed on the API is a pointer to the
 * "dds_topic_descriptor_t" struct type.
 */

/**
 * @ingroup topic_definition
 * @brief 用于保存序列化类型信息的简单大小字节容器 (Simple sized byte container to hold serialized
 * type info) 保存类型的 XTypes 信息（TypeInformation，TypeMapping） Holds XTypes information
 * (TypeInformation, TypeMapping) for a type
 */
struct dds_type_meta_ser {
  unsigned char* data; /**< 数据指针 data pointer */
  uint32_t sz;         /**< 数据大小 data size */
};

/**
 * @anchor DDS_DATA_REPRESENTATION_XCDR1
 * @ingroup topic_definition
 * @brief 数据表示 XCDR1 (Data representation XCDR1)
 * 类型可以使用 XCDR1 表示
 * Type can be represented using XCDR1
 */
#define DDS_DATA_REPRESENTATION_XCDR1 0

/**
 * @anchor DDS_DATA_REPRESENTATION_XML
 * @ingroup topic_definition
 * @brief 数据表示 XML (Data representation XML)
 * 类型可以使用 XML 表示
 * Type can be represented using XML
 */
#define DDS_DATA_REPRESENTATION_XML 1

/**
 * @anchor DDS_DATA_REPRESENTATION_XCDR2
 * @ingroup topic_definition
 * @brief 数据表示 XCDR2 (Data representation XCDR2)
 * 类型可以使用 XCDR2 表示
 * Type can be represented using XCDR2
 */
#define DDS_DATA_REPRESENTATION_XCDR2 2

/**
 * @anchor DDS_DATA_REPRESENTATION_FLAG_XCDR1
 * @ingroup topic_definition
 * @brief 数据表示 XCDR1 标志 (Data representation XCDR1 flag)
 * 类型可以使用 XCDR1 表示，预先移位
 * Type can be represented using XCDR1, preshifted
 */
#define DDS_DATA_REPRESENTATION_FLAG_XCDR1 (1u << DDS_DATA_REPRESENTATION_XCDR1)

/**
 * @anchor DDS_DATA_REPRESENTATION_FLAG_XML
 * @ingroup topic_definition
 * @brief 数据表示 XML 标志 (Data representation XML flag)
 * 类型可以使用 XML 表示，预先移位
 * Type can be represented using XML, preshifted
 */
#define DDS_DATA_REPRESENTATION_FLAG_XML (1u << DDS_DATA_REPRESENTATION_XML)

/**
 * @anchor DDS_DATA_REPRESENTATION_FLAG_XCDR2
 * @ingroup topic_definition
 * @brief 数据表示 XCDR2 标志 (Data representation XCDR2 flag)
 * 类型可以使用 XCDR2 表示，预先移位
 * Type can be represented using XCDR2, preshifted
 */
#define DDS_DATA_REPRESENTATION_FLAG_XCDR2 (1u << DDS_DATA_REPRESENTATION_XCDR2)

/**
 * @anchor DDS_DATA_REPRESENTATION_RESTRICT_DEFAULT
 * @ingroup topic_definition
 * @brief 默认数据表示标志，XCDR1 和 XCDR2 标志 (Default datarepresentation flag, XCDR1 and XCDR2
 * flags)
 */
#define DDS_DATA_REPRESENTATION_RESTRICT_DEFAULT \
  (DDS_DATA_REPRESENTATION_FLAG_XCDR1 | DDS_DATA_REPRESENTATION_FLAG_XCDR2)

/**
 * @brief 主题描述符 (Topic Descriptor)
 * @ingroup topic_definition
 * @warning 不稳定/私有 API (Unstable/Private API)
 * 包含关于类型的所有元信息，通常由 IDL 编译器生成 (Contains all meta information about a type,
 * usually produced by the IDL compiler)
 * 由于此类型不是为公共消费而设计的，因此可以在不发出警告的情况下更改 (Since this type is not
 * intended for public consumption it can change without warning)
 */
typedef struct dds_topic_descriptor {
  const uint32_t m_size;    /**< 主题类型的大小 (Size of topic type) */
  const uint32_t m_align;   /**< 主题类型的对齐 (Alignment of topic type) */
  const uint32_t m_flagset; /**< 标志 (Flags) */
  const uint32_t m_nkeys;   /**< 键的数量（可以为0）(Number of keys (can be 0)) */
  const char* m_typename;   /**< 类型名称 (Type name) */
  const dds_key_descriptor_t*
      m_keys; /**< 键描述符（如果 m_nkeys 为 0，则为 NULL）(Key descriptors (NULL iff m_nkeys 0)) */
  const uint32_t m_nops; /**< m_ops 中的操作数 (Number of ops in m_ops) */
  const uint32_t* m_ops; /**< 编组元数据 (Marshalling meta data) */
  const char* m_meta;    /**< XML 主题描述元数据 (XML topic description meta data) */
  struct dds_type_meta_ser
      type_information;  /**< XCDR2 序列化的 TypeInformation，仅在设置了
                            标志 DDS_TOPIC_XTYPES_METADATA 时存在 (XCDR2 serialized TypeInformation,
                            only present if  flag DDS_TOPIC_XTYPES_METADATA is set) */
  struct dds_type_meta_ser
      type_mapping; /**< XCDR2 序列化的 TypeMapping：将类型 ID 映射到类型对象和最小完整类型 ID，
                       仅在设置了标志 DDS_TOPIC_XTYPES_METADATA 时存在 (XCDR2 serialized
                       TypeMapping: maps type-id to type object and minimal to complete type id,
                       only present if flag DDS_TOPIC_XTYPES_METADATA is set) */
  const uint32_t
      restrict_data_representation; /**< 此主题的顶级类型允许的数据表示限制，
                                       仅在设置了标志 DDS_TOPIC_RESTRICT_DATA_REPRESENTATION 时存在
                                       (restrictions on the data representations allowed for the
                                       top-level type for this topic, only present if flag
                                       DDS_TOPIC_RESTRICT_DATA_REPRESENTATION) */
} dds_topic_descriptor_t;

/**
 * @defgroup reading_masks 读取掩码 (Reading Masks)
 * @ingroup conditions 条件 (Conditions)
 * 用于读取条件、读取和获取的掩码：这里只有一个掩码，
 * 它结合了样本、视图和实例状态。
 */

/**
 * @anchor DDS_READ_SAMPLE_STATE
 * @ingroup reading_masks
 * @brief 已经通过 read/take 操作返回过一次的样本 (Samples that were already returned once by a
 * read/take operation)
 */
#define DDS_READ_SAMPLE_STATE 1u

/**
 * @anchor DDS_NOT_READ_SAMPLE_STATE
 * @ingroup reading_masks
 * @brief 尚未通过 read/take 操作返回的样本 (Samples that have not been returned by a read/take
 * operation yet)
 */
#define DDS_NOT_READ_SAMPLE_STATE 2u

/**
 * @anchor DDS_ANY_SAMPLE_STATE
 * @ingroup reading_masks
 * @brief 样本 \ref DDS_READ_SAMPLE_STATE 或 \ref DDS_NOT_READ_SAMPLE_STATE (Samples \ref
 * DDS_READ_SAMPLE_STATE or \ref DDS_NOT_READ_SAMPLE_STATE)
 */
#define DDS_ANY_SAMPLE_STATE (1u | 2u)

/**
 * @anchor DDS_NEW_VIEW_STATE
 * @ingroup reading_masks
 * @brief 属于新实例（唯一键值）的样本 (Samples that belong to a new instance (unique key value))
 */
#define DDS_NEW_VIEW_STATE 4u

/**
 * @anchor DDS_NOT_NEW_VIEW_STATE
 * @ingroup reading_masks
 * @brief 属于现有实例（先前接收到的键值）的样本 (Samples that belong to an existing instance
 * (previously received key value))
 */
#define DDS_NOT_NEW_VIEW_STATE 8u

/**
 * @anchor DDS_ANY_VIEW_STATE
 * @ingroup reading_masks
 * @brief 样本 \ref DDS_NEW_VIEW_STATE 或 \ref DDS_NOT_NEW_VIEW_STATE (Samples \ref
 * DDS_NEW_VIEW_STATE or \ref DDS_NOT_NEW_VIEW_STATE)
 */
#define DDS_ANY_VIEW_STATE (4u | 8u)

/**
 * @anchor DDS_ALIVE_INSTANCE_STATE
 * @ingroup reading_masks
 * @brief 属于写操作的样本 (Samples that belong to a write)
 */
#define DDS_ALIVE_INSTANCE_STATE 16u

/**
 * @anchor DDS_NOT_ALIVE_DISPOSED_INSTANCE_STATE
 * @ingroup reading_masks
 * @brief 属于（写入）处理的样本 (Samples that belong to a (write)dispose)
 */
#define DDS_NOT_ALIVE_DISPOSED_INSTANCE_STATE 32u

/**
 * @anchor DDS_NOT_ALIVE_NO_WRITERS_INSTANCE_STATE
 * @ingroup reading_masks
 * @brief 属于已消失的写入器的样本 (Samples that belong a writer that is gone)
 */
#define DDS_NOT_ALIVE_NO_WRITERS_INSTANCE_STATE 64u

/**
 * @anchor DDS_ANY_INSTANCE_STATE
 * @ingroup reading_masks
 * @brief Samples \ref DDS_ALIVE_INSTANCE_STATE, \ref DDS_NOT_ALIVE_DISPOSED_INSTANCE_STATE or \ref
 * DDS_NOT_ALIVE_NO_WRITERS_INSTANCE_STATE
 *
 * 中文: 采样 \ref DDS_ALIVE_INSTANCE_STATE, \ref DDS_NOT_ALIVE_DISPOSED_INSTANCE_STATE 或 \ref
 * DDS_NOT_ALIVE_NO_WRITERS_INSTANCE_STATE
 */
#define DDS_ANY_INSTANCE_STATE (16u | 32u | 64u)

/**
 * @anchor DDS_ANY_STATE
 * @ingroup reading_masks
 * @brief Any and all samples
 * Equivalen to \ref DDS_ANY_SAMPLE_STATE | \ref DDS_ANY_VIEW_STATE | \ref DDS_ANY_INSTANCE_STATE
 *
 * 中文: 所有的样本
 * 等价于 \ref DDS_ANY_SAMPLE_STATE | \ref DDS_ANY_VIEW_STATE | \ref DDS_ANY_INSTANCE_STATE
 */
#define DDS_ANY_STATE (DDS_ANY_SAMPLE_STATE | DDS_ANY_VIEW_STATE | DDS_ANY_INSTANCE_STATE)

/**
 * @anchor DDS_DOMAIN_DEFAULT
 * @ingroup domain
 * @brief Select the default domain
 *
 * 中文: 选择默认域
 */
#define DDS_DOMAIN_DEFAULT ((uint32_t)0xffffffffu)

// The following block will be skipped by Doxygen.
// 下面的代码块将被 Doxygen 跳过。
#ifndef DOXYGEN_SHOULD_SKIP_THIS
#define DDS_HANDLE_NIL 0  // Define a constant for an invalid handle. 定义一个无效句柄的常量。
#define DDS_ENTITY_NIL 0  // Define a constant for an invalid entity. 定义一个无效实体的常量。
#endif

/**
 * @brief DDS 实体类型常量 (DDS Entity Kind constants)
 * @ingroup internal
 * @warning 不稳定/私有 API (Unstable/Private API)
 * 用于整个库中，表示实体的类型。(Used throughout the library to indicate what entity is what.)
 */
typedef enum dds_entity_kind {
  DDS_KIND_DONTCARE,    /**< 获取任意实体 (Retrieving any entity) */
  DDS_KIND_TOPIC,       /**< 主题实体 (Topic entity) */
  DDS_KIND_PARTICIPANT, /**< 域参与者实体 (Domain Participant entity) */
  DDS_KIND_READER,      /**< 读取器实体 (Reader entity) */
  DDS_KIND_WRITER,      /**< 写入器实体 (Writer entity) */
  DDS_KIND_SUBSCRIBER,  /**< 订阅者实体 (Subscriber entity) */
  DDS_KIND_PUBLISHER,   /**< 发布者实体 (Publisher entity) */
  DDS_KIND_COND_READ,   /**< 读取条件实体 (ReadCondition entity) */
  DDS_KIND_COND_QUERY,  /**< 查询条件实体 (QueryCondition entity) */
  DDS_KIND_COND_GUARD,  /**< 守护条件实体 (GuardCondition entity) */
  DDS_KIND_WAITSET,     /**< 等待集实体 (WaitSet entity) */
  DDS_KIND_DOMAIN,      /**< 域实体 (Domain entity) */
  DDS_KIND_CYCLONEDDS   /**< CycloneDDS 库实体 (CycloneDDS library entity) */
} dds_entity_kind_t;

/**
 * @anchor DDS_KIND_MAX
 * @ingroup internal
 * @brief 最大实体类型，用于循环。 (Max entity kind, used for loops.)
 */
#define DDS_KIND_MAX DDS_KIND_CYCLONEDDS

/**
 * @ingroup internal
 * @warning 私有 API (Private API)
 * @brief 实例句柄在幕后是 uint64_t 类型 (Instance handles are uint64_t behind the scenes)
 */
typedef uint64_t dds_instance_handle_t;

/**
 * @ingroup domain
 * @brief Domain IDs are 32 bit unsigned integers.
 */
typedef uint32_t dds_domainid_t;

/**
 * @ingroup topic
 * @brief Scope for dds_find_topic()
 */
typedef enum dds_find_scope {
  DDS_FIND_SCOPE_GLOBAL, /**< 在 CycloneDDS 所知范围内的任何地方定位主题 (locate the topic anywhere
                            CycloneDDS knows about) */
  DDS_FIND_SCOPE_LOCAL_DOMAIN, /**< 在域边界内本地定位主题 (locate the topic locally within domain
                                  boundaries) */
  DDS_FIND_SCOPE_PARTICIPANT /**< 在当前参与者内定位主题 (locate the topic within the current
                                participant) */
} dds_find_scope_t;

/**
 * @ingroup builtintopic
 * @brief Type identifier kind for getting endpoint type identifier
 */
typedef enum dds_typeid_kind {
  DDS_TYPEID_MINIMAL, /**< XTypes 最小类型 ID (XTypes Minimal Type ID) */
  DDS_TYPEID_COMPLETE /**< XTypes 完整类型 ID (XTypes Complete Type ID) */
} dds_typeid_kind_t;

/**
 * @brief 启用或禁用写入批处理。 (Enable or disable write batching.)
 * @component domain
 *
 * 覆盖默认配置设置的写入批处理（Internal/WriteBatch）。 (Overrides default configuration setting
 * for write batching (Internal/WriteBatch).)
 *
 * @param[in] enable 为所有写入器启用或禁用写入批处理。 (Enables or disables write batching for all
 * writers.)
 */
DDS_EXPORT void dds_write_set_batch(bool enable);

#if defined(__cplusplus)
}
#endif
#endif
