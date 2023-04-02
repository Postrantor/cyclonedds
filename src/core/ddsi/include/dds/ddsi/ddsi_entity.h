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
#ifndef DDSI_ENTITY_H
#define DDSI_ENTITY_H

#include "dds/ddsi/ddsi_guid.h"
#include "dds/ddsi/ddsi_protocol.h"
#include "dds/ddsrt/atomics.h"
#include "dds/ddsrt/avl.h"
#include "dds/ddsrt/sync.h"
#include "dds/export.h"
#include "dds/features.h"

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @brief ddsi_rsample_info 结构体声明
 */
struct ddsi_rsample_info;

/**
 * @brief ddsi_rdata 结构体声明
 */
struct ddsi_rdata;

/**
 * @brief ddsi_tkmap_instance 结构体声明
 */
struct ddsi_tkmap_instance;

/**
 * @brief ddsi_local_reader_ary 结构体声明
 */
struct ddsi_local_reader_ary;

/**
 * @brief 枚举 ddsi_entity_kind，表示实体类型
 */
enum ddsi_entity_kind {
  DDSI_EK_PARTICIPANT,       /**< 参与者 */
  DDSI_EK_PROXY_PARTICIPANT, /**< 代理参与者 */
  DDSI_EK_TOPIC,             /**< 主题 */
  DDSI_EK_WRITER,            /**< 写入者 */
  DDSI_EK_PROXY_WRITER,      /**< 代理写入者 */
  DDSI_EK_READER,            /**< 读取者 */
  DDSI_EK_PROXY_READER       /**< 代理读取者 */
};

/**
 * @brief ddsi_status_cb_data 结构体，用于存储状态回调数据
 */
typedef struct ddsi_status_cb_data {
  int raw_status_id; /**< 原始状态ID */
  uint32_t extra;    /**< 额外信息 */
  uint64_t handle;   /**< 句柄 */
  bool add;          /**< 是否添加 */
} ddsi_status_cb_data_t;

/**
 * @brief 状态回调函数类型定义
 *
 * @param entity 实体指针
 * @param data 状态回调数据指针
 */
typedef void (*ddsi_status_cb_t)(void* entity, const ddsi_status_cb_data_t* data);

/**
 * @brief ddsi_type_pair 结构体，用于存储类型对
 */
typedef struct ddsi_type_pair
#ifdef DDS_HAS_TYPE_DISCOVERY
{
  struct ddsi_type* minimal;  /**< 最小类型 */
  struct ddsi_type* complete; /**< 完整类型 */
}
#endif
ddsi_type_pair_t;

/**
 * @brief ddsi_entity_common 结构体定义了一个通用的实体对象，包含了实体的基本信息。
 * The ddsi_entity_common structure defines a generic entity object, containing basic information
 * about the entity.
 */
struct ddsi_entity_common {
  enum ddsi_entity_kind kind; /**< 实体类型 (Entity type) */
  ddsi_guid_t guid; /**< 实体的全局唯一标识符 (Global unique identifier of the entity) */
  ddsrt_wctime_t tupdate; /**< 最后更新时间戳 (Timestamp of last update) */
  uint64_t iid;           /**< 实例 ID (Instance ID) */
  struct ddsi_tkmap_instance*
      tk;             /**< 实体的主题键映射实例 (Topic key map instance of the entity) */
  ddsrt_mutex_t lock; /**< 互斥锁，用于同步访问实体 (Mutex for synchronized access to the entity) */
  bool onlylocal;           /**< 是否仅为本地实体 (Whether it is only a local entity) */
  struct ddsi_domaingv* gv; /**< 实体所属的域 (Domain the entity belongs to) */
  ddsrt_avl_node_t all_entities_avlnode; /**< AVL 树节点，用于存储所有实体 (AVL tree node for
                                            storing all entities) */

  /**
   * QoS 更改总是锁定实体本身，并在操作 QoS 时额外获取 qos_lock（在实体锁的范围内）。
   * 因此，任何需要在不获取实体锁的情况下读取 QoS 的线程仍然可以这样做。
   * QoS changes always lock the entity itself, and additionally acquire qos_lock while manipulating
   * the QoS (within the scope of the entity lock). So any thread that needs to read the QoS without
   * acquiring the entity's lock can still do so.
   */
  ddsrt_mutex_t qos_lock; /**< QoS 锁 (QoS lock) */
};

/**
 * @brief ddsi_local_reader_ary 结构体定义了一个本地读者数组，用于高效地传递数据。
 * The ddsi_local_reader_ary structure defines a local reader array for efficient data delivery.
 */
struct ddsi_local_reader_ary {
  ddsrt_mutex_t rdary_lock; /**< 读者数组的互斥锁 (Mutex for the reader array) */
  unsigned valid : 1; /**< 有效标志，直到（代理）写入器被删除为止始终为 true；!valid => !fastpath_ok
                         (Validity flag, always true until (proxy-)writer is being deleted; !valid
                         => !fastpath_ok) */
  unsigned fastpath_ok : 1; /**< 快速路径标志，如果不 ok，则回退到使用 GUID (Fast path flag, if not
                               ok, fall back to using GUIDs) */
  uint32_t n_readers; /**< 读者数量 (Number of readers) */
  struct ddsi_reader** rdary; /**< 高效传递的读者数组，以空指针结尾，按主题分组 (Reader array for
                                 efficient delivery, null-pointer terminated, grouped by topic) */
};

/**
 * @brief 将无符号整数转换为实体 ID
 * Convert an unsigned integer to an entity ID
 * @param u 无符号整数 (Unsigned integer)
 * @return 实体 ID (Entity ID)
 */
/** @component ddsi_generic_entity */
ddsi_entityid_t ddsi_to_entityid(unsigned u);

/**
 * @brief 获取实体的厂商 ID
 * Get the vendor ID of the entity
 * @param e 指向 ddsi_entity_common 结构体的指针 (Pointer to a ddsi_entity_common structure)
 * @return 厂商 ID (Vendor ID)
 */
/** @component ddsi_generic_entity */
ddsi_vendorid_t ddsi_get_entity_vendorid(const struct ddsi_entity_common* e);

/**
 * @brief 获取实体的实例 ID
 * Get the instance ID of the entity
 * @param gv 指向 ddsi_domaingv 结构体的指针 (Pointer to a ddsi_domaingv structure)
 * @param guid 指向 ddsi_guid 结构体的指针 (Pointer to a ddsi_guid structure)
 * @return 实例 ID (Instance ID)
 */
/** @component ddsi_generic_entity */
uint64_t ddsi_get_entity_instanceid(const struct ddsi_domaingv* gv, const struct ddsi_guid* guid);

#if defined(__cplusplus)
}
#endif

#endif /* DDSI_ENTITY_H */
