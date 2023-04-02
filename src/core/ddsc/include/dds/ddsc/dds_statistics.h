/*
 * Copyright(c) 2020 ZettaScale Technology and others
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v. 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0, or the Eclipse Distribution License
 * v. 1.0 which is available at
 * http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * SPDX-License-Identifier: EPL-2.0 OR BSD-3-Clause
 */

#ifndef DDS_STATISTICS_H
#define DDS_STATISTICS_H

/**
 * @defgroup statistics (DDS Statistics)
 * @ingroup dds
 * @warning Unstable API
 *
 * A quick-and-dirty provisional interface
 */

#include "dds/dds.h"
#include "dds/ddsrt/attributes.h"
#include "dds/export.h"

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @brief 统计值类型 (Kind of statistical value)
 * @ingroup statistics
 */
enum dds_stat_kind {
  DDS_STAT_KIND_UINT32, /**< 值是一个32位无符号整数 (value is a 32-bit unsigned integer) */
  DDS_STAT_KIND_UINT64, /**< 值是一个64位无符号整数 (value is a 64-bit unsigned integer) */
  DDS_STAT_KIND_LENGTHTIME /**< 值是积分(length(t) dt) (value is integral(length(t) dt)) */
};

/**
 * @brief 键值统计条目 (KeyValue statistics entry)
 * @ingroup statistics
 */
struct dds_stat_keyvalue {
  const char* name;        /**< 名称，库拥有内存 (name, memory owned by library) */
  enum dds_stat_kind kind; /**< 值类型 (value type) */
  union {
    uint32_t
        u32; /**< 若 kind == DDS_STAT_KIND_UINT32 时使用 (used if kind == DDS_STAT_KIND_UINT32) */
    uint64_t
        u64; /**< 若 kind == DDS_STAT_KIND_UINT64 时使用 (used if kind == DDS_STAT_KIND_UINT64) */
    uint64_t lengthtime; /**< 若 kind == DDS_STAT_KIND_LENGTHTIME 时使用 (used if kind ==
                            DDS_STAT_KIND_LENGTHTIME) */
  } u;                   /**< 值 (value) */
};

/**
 * @brief 统计容器 (Statistics container)
 * @ingroup statistics
 */
struct dds_statistics {
  dds_entity_t
      entity; /**< 应用此值集的实体句柄 (handle of entity to which this set of values applies) */
  uint64_t opaque; /**< 内部数据 (internal data) */
  dds_time_t time; /**< 最近一次调用 dds_refresh_statistics() 的时间戳 (time stamp of latest call to
                      dds_refresh_statistics()) */
  size_t count;                  /**< 键值对数量 (number of key-value pairs) */
  struct dds_stat_keyvalue kv[]; /**< 数据 (data) */
};

/**
 * @brief 为实体分配一个新的统计对象 (Allocate a new statistics object for entity)
 * @ingroup statistics
 * @component statistics
 * @unstable
 *
 * 为指定实体分配并填充一个新分配的 `struct dds_statistics`。
 * (This allocates and populates a newly allocated `struct dds_statistics` for the specified
 * entity.)
 *
 * @param[in] entity       实体句柄 (the handle of the entity)
 *
 * @returns 新分配且填充了数据的统计结构，如果实体无效或不支持任何统计信息，则返回 NULL。
 * (a newly allocated and populated statistics structure or NULL if entity is invalid or doesn't
 * support any statistics.)
 */
DDS_EXPORT struct dds_statistics* dds_create_statistics(dds_entity_t entity);

/**
 * @brief 使用当前值更新先前创建的统计结构 (Update a previously created statistics structure with
 * current values)
 * @ingroup statistics
 * @component statistics
 * @unstable
 *
 * 只有时间戳和值（以及“opaque”）可能发生变化。键集合和值类型不会改变。
 * (Only the time stamp and the values (and "opaque") may change. The set of keys and the types of
 * the values do not change.)
 *
 * @param[in,out] stat     要更新值的统计结构 (statistics structure to update the values of)
 *
 * @returns 成功或错误指示 (success or an error indication)
 *
 * @retval DDS_RETCODE_OK
 *    数据已成功更新 (the data was successfully updated)
 * @retval DDS_RETCODE_BAD_PARAMETER
 *    stats 是空指针，或引用的实体不存在 (stats is a null pointer or the referenced entity no longer
 * exists)
 * @retval DDS_RETCODE_PRECONDITION_NOT_MET
 *    库已取消初始化 (library was deinitialized)
 */
DDS_EXPORT dds_return_t dds_refresh_statistics(struct dds_statistics* stat);

/**
 * @brief 释放先前创建的统计对象 (Free a previously created statistics object)
 * @ingroup statistics
 * @component statistics
 * @unstable
 *
 * 释放统计对象。传递空指针是无操作。即使引用的实体不再存在，操作也会成功。
 * (This frees the statistics object. Passing a null pointer is a no-op. The operation succeeds also
 * if the referenced entity no longer exists.)
 *
 * @param[in] stat         要释放的统计对象 (statistics object to free)
 */
DDS_EXPORT void dds_delete_statistics(struct dds_statistics* stat);

/**
 * @brief 按名称查找特定值 (Lookup a specific value by name)
 * @ingroup statistics
 * @component statistics
 * @unstable
 *
 * 在 `stat` 中的键列表中查找指定名称，如果存在，则返回键值对的地址；如果不存在，则返回空指针。如果
 * `stat` 是空指针，则返回空指针。 (This looks up the specified name in the list of keys in `stat`
 * and returns the address of the key-value pair if present, a null pointer if not. If `stat` is a
 * null pointer, it returns a null pointer.)
 *
 * @param[in] stat         要在其中查找名称的统计对象（或 NULL）(statistics object to lookup a name
 * in (or NULL))
 * @param[in] name         要查找的名称 (name to look for)
 *
 * @returns `stat` 内部的键值对地址，如果 `stat` 为 NULL 或 `name` 与 `stat` 中的键不匹配，则返回
 * NULL。 (The address of the key-value pair inside `stat`, or NULL if `stat` is NULL or `name` does
 * not match a key in `stat.)
 */
DDS_EXPORT const struct dds_stat_keyvalue* dds_lookup_statistic(const struct dds_statistics* stat,
                                                                const char* name)
    ddsrt_nonnull((2));

#if defined(__cplusplus)
}
#endif
#endif
