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
#ifndef DDSI_RHC_H
#define DDSI_RHC_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "dds/ddsi/ddsi_guid.h"
#include "dds/ddsrt/time.h"
#include "dds/export.h"
#include "dds/features.h"

#if defined(__cplusplus)
extern "C" {
#endif

/** @struct dds_qos
 *  @brief QoS 结构体 (Quality of Service structure)
 */
struct dds_qos;

/** @struct ddsi_rhc
 *  @brief RHC 结构体 (Reader History Cache structure)
 */
struct ddsi_rhc;

/** @struct ddsi_tkmap_instance
 *  @brief TKMAP 实例结构体 (Topic Key Map Instance structure)
 */
struct ddsi_tkmap_instance;

/** @struct ddsi_serdata
 *  @brief 序列化数据结构体 (Serialized Data structure)
 */
struct ddsi_serdata;

/** @struct ddsi_writer_info
 *  @brief 写入器信息结构体 (Writer Information structure)
 */
struct ddsi_writer_info {
  ddsi_guid_t guid;           /**< GUID (Globally Unique Identifier) */
  bool auto_dispose;          /**< 自动处理标志 (Auto dispose flag) */
  int32_t ownership_strength; /**< 所有权强度 (Ownership strength) */
  uint64_t iid;               /**< 实例 ID (Instance ID) */
#ifdef DDS_HAS_LIFESPAN
  ddsrt_mtime_t lifespan_exp; /**< 生命周期过期时间 (Lifespan expiration time) */
#endif
};

/** @typedef ddsi_rhc_free_t
 *  @brief RHC 释放函数类型定义 (RHC free function type definition)
 */
typedef void (*ddsi_rhc_free_t)(struct ddsi_rhc* rhc);

/** @typedef ddsi_rhc_store_t
 *  @brief RHC 存储函数类型定义 (RHC store function type definition)
 */
typedef bool (*ddsi_rhc_store_t)(struct ddsi_rhc* __restrict rhc,
                                 const struct ddsi_writer_info* __restrict wrinfo,
                                 struct ddsi_serdata* __restrict sample,
                                 struct ddsi_tkmap_instance* __restrict tk);

/** @typedef ddsi_rhc_unregister_wr_t
 *  @brief RHC 注销写入器函数类型定义 (RHC unregister writer function type definition)
 */
typedef void (*ddsi_rhc_unregister_wr_t)(struct ddsi_rhc* __restrict rhc,
                                         const struct ddsi_writer_info* __restrict wrinfo);

/**
 * @typedef ddsi_rhc_relinquish_ownership_t
 * @brief RHC 放弃所有权函数类型定义 (RHC relinquish ownership function type definition)
 *
 * @param rhc 指向 ddsi_rhc 结构体的指针，用于表示可靠历史缓存 (Pointer to a ddsi_rhc structure,
 * representing the reliable history cache)
 * @param wr_iid 写入器实例的唯一标识符 (Unique identifier of the writer instance)
 */
typedef void (*ddsi_rhc_relinquish_ownership_t)(struct ddsi_rhc* __restrict rhc,
                                                const uint64_t wr_iid);

/** @typedef ddsi_rhc_set_qos_t
 *  @brief RHC 设置 QoS 函数类型定义 (RHC set QoS function type definition)
 */
typedef void (*ddsi_rhc_set_qos_t)(struct ddsi_rhc* rhc, const struct dds_qos* qos);

/** @struct ddsi_rhc_ops
 *  @brief RHC 操作结构体 (Reader History Cache operations structure)
 */
struct ddsi_rhc_ops {
  ddsi_rhc_store_t store;                 /**< 存储操作 (Store operation) */
  ddsi_rhc_unregister_wr_t unregister_wr; /**< 注销写入器操作 (Unregister writer operation) */
  ddsi_rhc_relinquish_ownership_t
      relinquish_ownership;   /**< 放弃所有权操作 (Relinquish ownership operation) */
  ddsi_rhc_set_qos_t set_qos; /**< 设置 QoS 操作 (Set QoS operation) */
  ddsi_rhc_free_t free;       /**< 释放操作 (Free operation) */
};

/** @struct ddsi_rhc
 *  @brief RHC 结构体定义 (Reader History Cache structure definition)
 */
struct ddsi_rhc {
  const struct ddsi_rhc_ops* ops; /**< 操作指针 (Operations pointer) */
};

/**
 * @component rhc_if
 * @brief 存储 RHC 数据的内联函数 (Inline function to store RHC data)
 *
 * @param[in] rhc RHC 结构体指针 (Pointer to RHC structure)
 * @param[in] wrinfo 写入器信息结构体指针 (Pointer to writer information structure)
 * @param[in] sample 序列化数据结构体指针 (Pointer to serialized data structure)
 * @param[in] tk TKMAP 实例结构体指针 (Pointer to Topic Key Map Instance structure)
 *
 * @return 存储成功返回 true，否则返回 false (Returns true if store is successful, otherwise returns
 * false)
 */
inline bool ddsi_rhc_store(struct ddsi_rhc* __restrict rhc,
                           const struct ddsi_writer_info* __restrict wrinfo,
                           struct ddsi_serdata* __restrict sample,
                           struct ddsi_tkmap_instance* __restrict tk) {
  return rhc->ops->store(rhc, wrinfo, sample, tk);
}

#if defined(__cplusplus)
}
#endif

#endif /* DDSI_RHC_H */
