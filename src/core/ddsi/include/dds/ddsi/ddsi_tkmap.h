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
#ifndef _DDS_TKMAP_H_
#define _DDS_TKMAP_H_

#include "dds/ddsrt/atomics.h"

#if defined(__cplusplus)
extern "C" {
#endif

// 定义 ddsi_tkmap 结构体
// Define the ddsi_tkmap structure
struct ddsi_tkmap;

// 定义 ddsi_serdata 结构体
// Define the ddsi_serdata structure
struct ddsi_serdata;

// 定义 dds_topic 结构体
// Define the dds_topic structure
struct dds_topic;

// 定义 ddsi_domaingv 结构体
// Define the ddsi_domaingv structure
struct ddsi_domaingv;

// 定义 ddsi_tkmap_instance 结构体
// Define the ddsi_tkmap_instance structure
struct ddsi_tkmap_instance {
  struct ddsi_serdata* m_sample;  // 存储序列化数据的指针
                                  // Pointer to store serialized data
  uint64_t m_iid;                 // 实例标识符
                                  // Instance identifier
  ddsrt_atomic_uint32_t m_refc;   // 原子引用计数器
                                  // Atomic reference counter
};

/**
 * @brief 创建一个新的 ddsi_tkmap 对象
 * @param gv 指向 ddsi_domaingv 结构体的指针
 * @return 返回创建的 ddsi_tkmap 对象的指针
 *
 * @brief Create a new ddsi_tkmap object
 * @param gv Pointer to the ddsi_domaingv structure
 * @return Returns a pointer to the created ddsi_tkmap object
 */
struct ddsi_tkmap* ddsi_tkmap_new(struct ddsi_domaingv* gv);

/**
 * @brief 释放 ddsi_tkmap 对象
 * @param tkmap 指向 ddsi_tkmap 结构体的指针
 *
 * @brief Free the ddsi_tkmap object
 * @param tkmap Pointer to the ddsi_tkmap structure
 */
void ddsi_tkmap_free(struct ddsi_tkmap* tkmap);

/**
 * @brief 增加 ddsi_tkmap_instance 的引用计数
 * @param tk 指向 ddsi_tkmap_instance 结构体的指针
 *
 * @brief Increase the reference count of the ddsi_tkmap_instance
 * @param tk Pointer to the ddsi_tkmap_instance structure
 */
void ddsi_tkmap_instance_ref(struct ddsi_tkmap_instance* tk);

/**
 * @brief 查找与给定序列化数据对应的实例标识符
 * @param tkmap 指向 ddsi_tkmap 结构体的指针
 * @param serdata 指向 ddsi_serdata 结构体的指针
 * @return 返回查找到的实例标识符
 *
 * @brief Lookup the instance identifier corresponding to the given serialized data
 * @param tkmap Pointer to the ddsi_tkmap structure
 * @param serdata Pointer to the ddsi_serdata structure
 * @return Returns the found instance identifier
 */
uint64_t ddsi_tkmap_lookup(struct ddsi_tkmap* tkmap, const struct ddsi_serdata* serdata);

/**
 * @brief 在 ddsi_tkmap 中查找或创建与给定序列化数据对应的 ddsi_tkmap_instance
 * @param map 指向 ddsi_tkmap 结构体的指针
 * @param sd 指向 ddsi_serdata 结构体的指针
 * @param create 如果为 true，则在未找到对应实例时创建新实例
 * @return 返回查找或创建的 ddsi_tkmap_instance 的指针
 *
 * @brief Find or create a ddsi_tkmap_instance in the ddsi_tkmap corresponding to the given
 * serialized data
 * @param map Pointer to the ddsi_tkmap structure
 * @param sd Pointer to the ddsi_serdata structure
 * @param create If true, create a new instance if not found
 * @return Returns a pointer to the found or created ddsi_tkmap_instance
 */
struct ddsi_tkmap_instance* ddsi_tkmap_find(struct ddsi_tkmap* map,
                                            struct ddsi_serdata* sd,
                                            const bool create);

/**
 * @brief 通过实例标识符在 ddsi_tkmap 中查找 ddsi_tkmap_instance
 * @param map 指向 ddsi_tkmap 结构体的指针
 * @param iid 实例标识符
 * @return 返回查找到的 ddsi_tkmap_instance 的指针
 *
 * @brief Find a ddsi_tkmap_instance in the ddsi_tkmap by instance identifier
 * @param map Pointer to the ddsi_tkmap structure
 * @param iid Instance identifier
 * @return Returns a pointer to the found ddsi_tkmap_instance
 */
struct ddsi_tkmap_instance* ddsi_tkmap_find_by_id(struct ddsi_tkmap* map, uint64_t iid);

/**
 * @brief 查找与给定序列化数据对应的 ddsi_tkmap_instance 并增加其引用计数
 * @param map 指向 ddsi_tkmap 结构体的指针
 * @param sd 指向 ddsi_serdata 结构体的指针
 * @return 返回查找到的并已增加引用计数的 ddsi_tkmap_instance 的指针
 *
 * @brief Lookup the ddsi_tkmap_instance corresponding to the given serialized data and increase its
 * reference count
 * @param map Pointer to the ddsi_tkmap structure
 * @param sd Pointer to the ddsi_serdata structure
 * @return Returns a pointer to the found ddsi_tkmap_instance with increased reference count
 */
struct ddsi_tkmap_instance* ddsi_tkmap_lookup_instance_ref(struct ddsi_tkmap* map,
                                                           struct ddsi_serdata* sd);

/**
 * @brief 减少 ddsi_tkmap_instance 的引用计数，并在引用计数为零时释放资源
 * @param map 指向 ddsi_tkmap 结构体的指针
 * @param tk 指向 ddsi_tkmap_instance 结构体的指针
 *
 * @brief Decrease the reference count of the ddsi_tkmap_instance and release resources when the
 * reference count is zero
 * @param map Pointer to the ddsi_tkmap structure
 * @param tk Pointer to the ddsi_tkmap_instance structure
 */
void ddsi_tkmap_instance_unref(struct ddsi_tkmap* map, struct ddsi_tkmap_instance* tk);

#if defined(__cplusplus)
}
#endif
#endif
