/*
 * Copyright(c) 2006 to 2020 ZettaScale Technology and others
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v. 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0, or the Eclipse Distribution License
 * v. 1.0 which is available at
 * http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * SPDX-License-Identifier: EPL-2.0 OR BSD-3-Clause
 */
#ifndef DDSI_ENTITY_INDEX_H
#define DDSI_ENTITY_INDEX_H

#include "dds/ddsi/ddsi_endpoint.h"
#include "dds/ddsi/ddsi_entity.h"
#include "dds/ddsi/ddsi_proxy_endpoint.h"
#include "dds/ddsi/ddsi_thread.h"
#include "dds/ddsi/ddsi_topic.h"

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @struct ddsi_entity_index
 * @brief 用于存储实体索引的结构体 (Structure for storing entity index)
 */
struct ddsi_entity_index;

/**
 * @struct ddsi_guid
 * @brief 用于存储全局唯一标识符 (GUID) 的结构体 (Structure for storing Globally Unique Identifier
 * (GUID))
 */
struct ddsi_guid;

/**
 * @struct ddsi_domaingv
 * @brief 用于存储域全局变量的结构体 (Structure for storing domain global variables)
 */
struct ddsi_domaingv;

/**
 * @struct ddsi_entity_enum
 * @brief 用于枚举 DDS 实体的结构体 (Structure for enumerating DDS entities)
 */
struct ddsi_entity_enum {
  struct ddsi_entity_index* entidx; /**< 指向实体索引的指针 (Pointer to the entity index) */
  enum ddsi_entity_kind kind;       /**< 实体类型 (Entity kind) */
  struct ddsi_entity_common*
      cur; /**< 当前实体的通用信息 (Common information of the current entity) */

#ifndef NDEBUG
  ddsi_vtime_t vtime; /**< 验证时间戳 (Validation timestamp) */
#endif
};

/* 读者和写者都在 GUID- 和 GID-键表中。如果
   它们在基于 GID 的表中，它们也在基于 GUID 的表中，
   但不是反过来，原因有两个：

   - 首先，有些读者和写者没有 GID
     （内置端点，虚构的瞬态数据读取器），

   - 其次，它们首先插入到基于 GUID 的表中，然后
     插入到基于 GID 的表中。

   GID 仅用于与 OpenSplice 内核的接口，
   所有内部状态和协议处理都使用 GUID 进行。
   所以这意味着，例如，正在删除的写者会变得
   对网络读者不可见，稍微早于在协议处理中消失，
   或者说写者可能在协议级别存在，稍微早于网络读者可以使用它
   来传输数据。 */

/* Readers & writers are both in a GUID- and in a GID-keyed table. If
   they are in the GID-based one, they are also in the GUID-based one,
   but not the way around, for two reasons:

   - firstly, there are readers & writers that do not have a GID
     (built-in endpoints, fictitious transient data readers),

   - secondly, they are inserted first in the GUID-keyed one, and then
     in the GID-keyed one.

   The GID is used solely for the interface with the OpenSplice
   kernel, all internal state and protocol handling is done using the
   GUID. So all this means is that, e.g., a writer being deleted
   becomes invisible to the network reader slightly before it
   disappears in the protocol handling, or that a writer might exist
   at the protocol level slightly before the network reader can use it
   to transmit data. */

/** @component entity_index */
// 函数 ddsi_entidx_lookup_guid_untyped 的声明，用于在实体索引中查找未分类的 GUID
// Declaration of function ddsi_entidx_lookup_guid_untyped, used to lookup untyped GUID in the
// entity index
void* ddsi_entidx_lookup_guid_untyped(const struct ddsi_entity_index* ei,
                                      const struct ddsi_guid* guid) ddsrt_nonnull_all;

/** @component entity_index */
// 函数 ddsi_entidx_lookup_guid 的声明，用于在实体索引中查找具有特定类型的 GUID
// Declaration of function ddsi_entidx_lookup_guid, used to lookup GUID with specific type in the
// entity index
void* ddsi_entidx_lookup_guid(const struct ddsi_entity_index* ei,
                              const struct ddsi_guid* guid,
                              enum ddsi_entity_kind kind) ddsrt_nonnull_all;

/** @component entity_index */
// 函数 ddsi_entidx_lookup_participant_guid 的声明，用于在实体索引中查找参与者的 GUID
// Declaration of function ddsi_entidx_lookup_participant_guid, used to lookup participant's GUID in
// the entity index
struct ddsi_participant* ddsi_entidx_lookup_participant_guid(
    const struct ddsi_entity_index* ei, const struct ddsi_guid* guid) ddsrt_nonnull_all;

/** @component entity_index */
// 函数 ddsi_entidx_lookup_writer_guid 的声明，用于在实体索引中查找写者的 GUID
// Declaration of function ddsi_entidx_lookup_writer_guid, used to lookup writer's GUID in the
// entity index
struct ddsi_writer* ddsi_entidx_lookup_writer_guid(const struct ddsi_entity_index* ei,
                                                   const struct ddsi_guid* guid) ddsrt_nonnull_all;

/** @component entity_index */
// 函数 ddsi_entidx_lookup_reader_guid 的声明，用于在实体索引中查找读者的 GUID
// Declaration of function ddsi_entidx_lookup_reader_guid, used to lookup reader's GUID in the
// entity index
struct ddsi_reader* ddsi_entidx_lookup_reader_guid(const struct ddsi_entity_index* ei,
                                                   const struct ddsi_guid* guid) ddsrt_nonnull_all;

/** @component entity_index */
// 函数 ddsi_entidx_lookup_proxy_participant_guid 的声明，用于在实体索引中查找代理参与者的 GUID
// Declaration of function ddsi_entidx_lookup_proxy_participant_guid, used to lookup proxy
// participant's GUID in the entity index
struct ddsi_proxy_participant* ddsi_entidx_lookup_proxy_participant_guid(
    const struct ddsi_entity_index* ei, const struct ddsi_guid* guid) ddsrt_nonnull_all;

/** @component entity_index */
// 查找具有给定 GUID 的代理写入器 (Lookup a proxy writer with the given GUID)
// @param[in] ei 实体索引指针 (Pointer to the entity index)
// @param[in] guid 要查找的代理写入器的 GUID (GUID of the proxy writer to lookup)
// @return 返回找到的代理写入器，如果未找到，则返回 NULL (Returns the found proxy writer, or NULL if
// not found)
struct ddsi_proxy_writer* ddsi_entidx_lookup_proxy_writer_guid(
    const struct ddsi_entity_index* ei, const struct ddsi_guid* guid) ddsrt_nonnull_all;

/** @component entity_index */
// 查找具有给定 GUID 的代理读取器 (Lookup a proxy reader with the given GUID)
// @param[in] ei 实体索引指针 (Pointer to the entity index)
// @param[in] guid 要查找的代理读取器的 GUID (GUID of the proxy reader to lookup)
// @return 返回找到的代理读取器，如果未找到，则返回 NULL (Returns the found proxy reader, or NULL if
// not found)
struct ddsi_proxy_reader* ddsi_entidx_lookup_proxy_reader_guid(
    const struct ddsi_entity_index* ei, const struct ddsi_guid* guid) ddsrt_nonnull_all;

/* Enumeration of entries in the hash table:

   - "next" visits at least all entries that were in the hash table at
     the time of calling init and that have not subsequently been
     removed;

   - "next" may visit an entry more than once, but will do so only
     because of rare events (i.e., resize or so);

   - the order in which entries are visited is arbitrary;

   - the caller must call init() before it may call next(); it must
     call fini() before it may call init() again. */

/** @component entity_index */
// 初始化实体枚举 (Initialize the entity enumeration)
// @param[out] st 实体枚举指针 (Pointer to the entity enumeration)
// @param[in] ei 实体索引指针 (Pointer to the entity index)
// @param[in] kind 要枚举的实体类型 (The type of entities to enumerate)
void ddsi_entidx_enum_init(struct ddsi_entity_enum* st,
                           const struct ddsi_entity_index* ei,
                           enum ddsi_entity_kind kind) ddsrt_nonnull_all;

/** @component entity_index */
// 获取下一个实体 (Get the next entity)
// @param[in,out] st 实体枚举指针 (Pointer to the entity enumeration)
// @return 返回找到的实体，如果没有更多实体，则返回 NULL (Returns the found entity, or NULL if no
// more entities)
void* ddsi_entidx_enum_next(struct ddsi_entity_enum* st) ddsrt_nonnull_all;

/** @component entity_index */
// 结束实体枚举 (Finalize the entity enumeration)
// @param[in,out] st 实体枚举指针 (Pointer to the entity enumeration)
void ddsi_entidx_enum_fini(struct ddsi_entity_enum* st) ddsrt_nonnull_all;

#ifdef DDS_HAS_TOPIC_DISCOVERY
/** @component entity_index */
// 通过 GUID 查找主题 (Lookup a topic by its GUID)
// @param[in] ei 实体索引指针 (Pointer to the entity index)
// @param[in] guid 要查找的主题的 GUID (GUID of the topic to lookup)
// @return 返回找到的主题，如果未找到，则返回 NULL (Returns the found topic, or NULL if not found)
struct ddsi_topic* ddsi_entidx_lookup_topic_guid(const struct ddsi_entity_index* ei,
                                                 const struct ddsi_guid* guid);
#endif

#if defined(__cplusplus)
}
#endif

#endif /* DDSI_ENTITY_INDEX_H */
