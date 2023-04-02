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
#ifndef DDSI__ENTITY_INDEX_H
#define DDSI__ENTITY_INDEX_H

#include "dds/ddsi/ddsi_endpoint.h"
#include "dds/ddsi/ddsi_entity.h"
#include "dds/ddsi/ddsi_entity_index.h"
#include "dds/ddsi/ddsi_proxy_endpoint.h"
#include "dds/ddsi/ddsi_topic.h"
#include "dds/ddsrt/hopscotch.h"
#include "ddsi__thread.h"

#if defined(__cplusplus)
extern "C" {
#endif

struct ddsi_entity_index;
struct ddsi_guid;
struct ddsi_domaingv;

/**
 * @brief 匹配实体范围键结构体 (Match entities range key structure)
 */
struct ddsi_match_entities_range_key {
  /**
   * @brief 实体联合体，包含多种实体类型 (Entity union, containing various entity types)
   */
  union {
#ifdef DDS_HAS_TOPIC_DISCOVERY
    struct ddsi_topic tp;                    ///< 主题实体 (Topic entity)
#endif
    struct ddsi_writer wr;                   ///< 写入者实体 (Writer entity)
    struct ddsi_reader rd;                   ///< 读取者实体 (Reader entity)
    struct ddsi_entity_common e;             ///< 通用实体 (Common entity)
    struct ddsi_generic_proxy_endpoint gpe;  ///< 通用代理端点实体 (Generic proxy endpoint entity)
  } entity;
  struct dds_qos xqos;                       ///< QoS 结构体 (QoS structure)
#ifdef DDS_HAS_TOPIC_DISCOVERY
  struct ddsi_topic_definition tpdef;        ///< 主题定义结构体 (Topic definition structure)
#endif
};

/**
 * @brief 枚举参与者实体结构体 (Enumeration participant entity structure)
 */
struct ddsi_entity_enum_participant {
  struct ddsi_entity_enum st;  ///< 实体枚举结构体 (Entity enumeration structure)
};

/**
 * @brief 枚举写入者实体结构体 (Enumeration writer entity structure)
 */
struct ddsi_entity_enum_writer {
  struct ddsi_entity_enum st;  ///< 实体枚举结构体 (Entity enumeration structure)
};

/**
 * @brief 枚举读取者实体结构体 (Enumeration reader entity structure)
 */
struct ddsi_entity_enum_reader {
  struct ddsi_entity_enum st;  ///< 实体枚举结构体 (Entity enumeration structure)
};

/**
 * @brief 枚举代理参与者实体结构体 (Enumeration proxy participant entity structure)
 */
struct ddsi_entity_enum_proxy_participant {
  struct ddsi_entity_enum st;  ///< 实体枚举结构体 (Entity enumeration structure)
};

/**
 * @brief 枚举代理写入者实体结构体 (Enumeration proxy writer entity structure)
 */
struct ddsi_entity_enum_proxy_writer {
  struct ddsi_entity_enum st;  ///< 实体枚举结构体 (Entity enumeration structure)
};

/**
 * @brief 枚举代理读取者实体结构体 (Enumeration proxy reader entity structure)
 */
struct ddsi_entity_enum_proxy_reader {
  struct ddsi_entity_enum st;  ///< 实体枚举结构体 (Entity enumeration structure)
};

/** @component entity_index */

/**
 * @brief 创建一个新的实体索引对象 (Create a new entity index object)
 * @param gv 指向 ddsi_domaingv 结构体的指针 (Pointer to the ddsi_domaingv structure)
 * @return 返回创建的实体索引对象指针 (Returns the pointer to the created entity index object)
 */
struct ddsi_entity_index* ddsi_entity_index_new(struct ddsi_domaingv* gv) ddsrt_nonnull_all;

/**
 * @brief 释放实体索引对象 (Free the entity index object)
 * @param ei 指向 ddsi_entity_index 结构体的指针 (Pointer to the ddsi_entity_index structure)
 */
void ddsi_entity_index_free(struct ddsi_entity_index* ei) ddsrt_nonnull_all;

/**
 * @brief 向实体索引中插入参与者 GUID (Insert participant GUID into the entity index)
 * @param ei 指向 ddsi_entity_index 结构体的指针 (Pointer to the ddsi_entity_index structure)
 * @param pp 指向 ddsi_participant 结构体的指针 (Pointer to the ddsi_participant structure)
 */
void ddsi_entidx_insert_participant_guid(struct ddsi_entity_index* ei,
                                         struct ddsi_participant* pp) ddsrt_nonnull_all;

/**
 * @brief 向实体索引中插入代理参与者 GUID (Insert proxy participant GUID into the entity index)
 * @param ei 指向 ddsi_entity_index 结构体的指针 (Pointer to the ddsi_entity_index structure)
 * @param proxypp 指向 ddsi_proxy_participant 结构体的指针 (Pointer to the ddsi_proxy_participant
 * structure)
 */
void ddsi_entidx_insert_proxy_participant_guid(
    struct ddsi_entity_index* ei, struct ddsi_proxy_participant* proxypp) ddsrt_nonnull_all;

/**
 * @brief 向实体索引中插入写入者 GUID (Insert writer GUID into the entity index)
 * @param ei 指向 ddsi_entity_index 结构体的指针 (Pointer to the ddsi_entity_index structure)
 * @param wr 指向 ddsi_writer 结构体的指针 (Pointer to the ddsi_writer structure)
 */
void ddsi_entidx_insert_writer_guid(struct ddsi_entity_index* ei,
                                    struct ddsi_writer* wr) ddsrt_nonnull_all;

/**
 * @brief 向实体索引中插入读取者 GUID (Insert reader GUID into the entity index)
 * @param ei 指向 ddsi_entity_index 结构体的指针 (Pointer to the ddsi_entity_index structure)
 * @param rd 指向 ddsi_reader 结构体的指针 (Pointer to the ddsi_reader structure)
 */
void ddsi_entidx_insert_reader_guid(struct ddsi_entity_index* ei,
                                    struct ddsi_reader* rd) ddsrt_nonnull_all;

/**
 * @brief 向实体索引中插入代理写入者 GUID (Insert proxy writer GUID into the entity index)
 * @param ei 指向 ddsi_entity_index 结构体的指针 (Pointer to the ddsi_entity_index structure)
 * @param pwr 指向 ddsi_proxy_writer 结构体的指针 (Pointer to the ddsi_proxy_writer structure)
 */
void ddsi_entidx_insert_proxy_writer_guid(struct ddsi_entity_index* ei,
                                          struct ddsi_proxy_writer* pwr) ddsrt_nonnull_all;

/**
 * @brief 向实体索引中插入代理读取者 GUID (Insert proxy reader GUID into the entity index)
 * @param ei 指向 ddsi_entity_index 结构体的指针 (Pointer to the ddsi_entity_index structure)
 * @param prd 指向 ddsi_proxy_reader 结构体的指针 (Pointer to the ddsi_proxy_reader structure)
 */
void ddsi_entidx_insert_proxy_reader_guid(struct ddsi_entity_index* ei,
                                          struct ddsi_proxy_reader* prd) ddsrt_nonnull_all;

/**
 * @brief 从实体索引中移除参与者 GUID (Remove participant GUID from the entity index)
 * @param ei 指向 ddsi_entity_index 结构体的指针 (Pointer to the ddsi_entity_index structure)
 * @param pp 指向 ddsi_participant 结构体的指针 (Pointer to the ddsi_participant structure)
 */
void ddsi_entidx_remove_participant_guid(struct ddsi_entity_index* ei,
                                         struct ddsi_participant* pp) ddsrt_nonnull_all;

/**
 * @brief 从实体索引中移除代理参与者 GUID (Remove proxy participant GUID from the entity index)
 * @param ei 指向 ddsi_entity_index 结构体的指针 (Pointer to the ddsi_entity_index structure)
 * @param proxypp 指向 ddsi_proxy_participant 结构体的指针 (Pointer to the ddsi_proxy_participant
 * structure)
 */
void ddsi_entidx_remove_proxy_participant_guid(
    struct ddsi_entity_index* ei, struct ddsi_proxy_participant* proxypp) ddsrt_nonnull_all;

/**
 * @brief 从实体索引中移除一个写入者（writer）实体
 * Remove a writer entity from the entity index
 *
 * @param[in] ei 实体索引指针 Pointer to the entity index
 * @param[in] wr 要移除的写入者实体指针 Pointer to the writer entity to be removed
 */
void ddsi_entidx_remove_writer_guid(struct ddsi_entity_index* ei,
                                    struct ddsi_writer* wr) ddsrt_nonnull_all;

/**
 * @brief 从实体索引中移除一个读取者（reader）实体
 * Remove a reader entity from the entity index
 *
 * @param[in] ei 实体索引指针 Pointer to the entity index
 * @param[in] rd 要移除的读取者实体指针 Pointer to the reader entity to be removed
 */
void ddsi_entidx_remove_reader_guid(struct ddsi_entity_index* ei,
                                    struct ddsi_reader* rd) ddsrt_nonnull_all;

/**
 * @brief 从实体索引中移除一个代理写入者（proxy writer）实体
 * Remove a proxy writer entity from the entity index
 *
 * @param[in] ei 实体索引指针 Pointer to the entity index
 * @param[in] pwr 要移除的代理写入者实体指针 Pointer to the proxy writer entity to be removed
 */
void ddsi_entidx_remove_proxy_writer_guid(struct ddsi_entity_index* ei,
                                          struct ddsi_proxy_writer* pwr) ddsrt_nonnull_all;

/**
 * @brief 从实体索引中移除一个代理读取者（proxy reader）实体
 * Remove a proxy reader entity from the entity index
 *
 * @param[in] ei 实体索引指针 Pointer to the entity index
 * @param[in] prd 要移除的代理读取者实体指针 Pointer to the proxy reader entity to be removed
 */
void ddsi_entidx_remove_proxy_reader_guid(struct ddsi_entity_index* ei,
                                          struct ddsi_proxy_reader* prd) ddsrt_nonnull_all;

/**
 * @brief 初始化一个特定主题的实体枚举器
 * Initialize an entity enumerator for a specific topic
 *
 * @param[out] st 实体枚举器指针 Pointer to the entity enumerator
 * @param[in] gh 实体索引指针 Pointer to the entity index
 * @param[in] kind 实体类型 Entity kind
 * @param[in] topic 主题名称 Topic name
 * @param[in] max 匹配实体范围键的最大值 Maximum value of match entities range key
 */
void ddsi_entidx_enum_init_topic(struct ddsi_entity_enum* st,
                                 const struct ddsi_entity_index* gh,
                                 enum ddsi_entity_kind kind,
                                 const char* topic,
                                 struct ddsi_match_entities_range_key* max) ddsrt_nonnull_all;

/**
 * @brief 使用前缀初始化一个特定主题的实体枚举器
 * Initialize an entity enumerator for a specific topic with a prefix
 *
 * @param[out] st 实体枚举器指针 Pointer to the entity enumerator
 * @param[in] ei 实体索引指针 Pointer to the entity index
 * @param[in] kind 实体类型 Entity kind
 * @param[in] topic 主题名称 Topic name
 * @param[in] prefix GUID前缀 GUID prefix
 * @param[in] max 匹配实体范围键的最大值 Maximum value of match entities range key
 */
void ddsi_entidx_enum_init_topic_w_prefix(struct ddsi_entity_enum* st,
                                          const struct ddsi_entity_index* ei,
                                          enum ddsi_entity_kind kind,
                                          const char* topic,
                                          const ddsi_guid_prefix_t* prefix,
                                          struct ddsi_match_entities_range_key* max)
    ddsrt_nonnull_all;

/**
 * @brief 获取下一个匹配实体，直到达到最大范围键值
 * Get the next matching entity until the maximum range key value is reached
 *
 * @param[in] st 实体枚举器指针 Pointer to the entity enumerator
 * @param[in] max 匹配实体范围键的最大值 Maximum value of match entities range key
 * @return 返回下一个匹配实体的指针 Pointer to the next matching entity
 */
void* ddsi_entidx_enum_next_max(struct ddsi_entity_enum* st,
                                const struct ddsi_match_entities_range_key* max) ddsrt_nonnull_all;

/**
 * @brief 初始化写入者实体枚举器
 * Initialize a writer entity enumerator
 *
 * @param[out] st 写入者实体枚举器指针 Pointer to the writer entity enumerator
 * @param[in] ei 实体索引指针 Pointer to the entity index
 */
void ddsi_entidx_enum_writer_init(struct ddsi_entity_enum_writer* st,
                                  const struct ddsi_entity_index* ei) ddsrt_nonnull_all;

/**
 * @brief 初始化读取者实体枚举器
 * Initialize a reader entity enumerator
 *
 * @param[out] st 读取者实体枚举器指针 Pointer to the reader entity enumerator
 * @param[in] ei 实体索引指针 Pointer to the entity index
 */
void ddsi_entidx_enum_reader_init(struct ddsi_entity_enum_reader* st,
                                  const struct ddsi_entity_index* ei) ddsrt_nonnull_all;

/** @component entity_index */

/**
 * @brief 初始化代理写入器枚举结构体 (Initialize the proxy writer enumeration structure)
 *
 * @param[out] st 代理写入器枚举结构体指针 (Pointer to the proxy writer enumeration structure)
 * @param[in] ei 实体索引指针 (Pointer to the entity index)
 */
void ddsi_entidx_enum_proxy_writer_init(struct ddsi_entity_enum_proxy_writer* st,
                                        const struct ddsi_entity_index* ei) ddsrt_nonnull_all;

/**
 * @brief 初始化代理读取器枚举结构体 (Initialize the proxy reader enumeration structure)
 *
 * @param[out] st 代理读取器枚举结构体指针 (Pointer to the proxy reader enumeration structure)
 * @param[in] ei 实体索引指针 (Pointer to the entity index)
 */
void ddsi_entidx_enum_proxy_reader_init(struct ddsi_entity_enum_proxy_reader* st,
                                        const struct ddsi_entity_index* ei) ddsrt_nonnull_all;

/**
 * @brief 初始化参与者枚举结构体 (Initialize the participant enumeration structure)
 *
 * @param[out] st 参与者枚举结构体指针 (Pointer to the participant enumeration structure)
 * @param[in] ei 实体索引指针 (Pointer to the entity index)
 */
void ddsi_entidx_enum_participant_init(struct ddsi_entity_enum_participant* st,
                                       const struct ddsi_entity_index* ei) ddsrt_nonnull_all;

/**
 * @brief 初始化代理参与者枚举结构体 (Initialize the proxy participant enumeration structure)
 *
 * @param[out] st 代理参与者枚举结构体指针 (Pointer to the proxy participant enumeration structure)
 * @param[in] ei 实体索引指针 (Pointer to the entity index)
 */
void ddsi_entidx_enum_proxy_participant_init(struct ddsi_entity_enum_proxy_participant* st,
                                             const struct ddsi_entity_index* ei) ddsrt_nonnull_all;

/**
 * @brief 获取下一个写入器实体 (Get the next writer entity)
 *
 * @param[in,out] st 写入器枚举结构体指针 (Pointer to the writer enumeration structure)
 * @return 返回下一个写入器实体指针 (Return pointer to the next writer entity)
 */
struct ddsi_writer* ddsi_entidx_enum_writer_next(struct ddsi_entity_enum_writer* st)
    ddsrt_nonnull_all;

/**
 * @brief 获取下一个读取器实体 (Get the next reader entity)
 *
 * @param[in,out] st 读取器枚举结构体指针 (Pointer to the reader enumeration structure)
 * @return 返回下一个读取器实体指针 (Return pointer to the next reader entity)
 */
struct ddsi_reader* ddsi_entidx_enum_reader_next(struct ddsi_entity_enum_reader* st)
    ddsrt_nonnull_all;

/**
 * @brief 获取下一个代理写入器实体 (Get the next proxy writer entity)
 *
 * @param[in,out] st 代理写入器枚举结构体指针 (Pointer to the proxy writer enumeration structure)
 * @return 返回下一个代理写入器实体指针 (Return pointer to the next proxy writer entity)
 */
struct ddsi_proxy_writer* ddsi_entidx_enum_proxy_writer_next(
    struct ddsi_entity_enum_proxy_writer* st) ddsrt_nonnull_all;

/**
 * @brief 获取下一个代理读取器实体 (Get the next proxy reader entity)
 *
 * @param[in,out] st 代理读取器枚举结构体指针 (Pointer to the proxy reader enumeration structure)
 * @return 返回下一个代理读取器实体指针 (Return pointer to the next proxy reader entity)
 */
struct ddsi_proxy_reader* ddsi_entidx_enum_proxy_reader_next(
    struct ddsi_entity_enum_proxy_reader* st) ddsrt_nonnull_all;

/**
 * @brief 获取下一个参与者实体 (Get the next participant entity)
 *
 * @param[in,out] st 参与者枚举结构体指针 (Pointer to the participant enumeration structure)
 * @return 返回下一个参与者实体指针 (Return pointer to the next participant entity)
 */
struct ddsi_participant* ddsi_entidx_enum_participant_next(struct ddsi_entity_enum_participant* st)
    ddsrt_nonnull_all;

/**
 * @brief 获取下一个代理参与者实体 (Get the next proxy participant entity)
 *
 * @param[in,out] st 代理参与者枚举结构体指针 (Pointer to the proxy participant enumeration
 * structure)
 * @return 返回下一个代理参与者实体指针 (Return pointer to the next proxy participant entity)
 */
struct ddsi_proxy_participant* ddsi_entidx_enum_proxy_participant_next(
    struct ddsi_entity_enum_proxy_participant* st) ddsrt_nonnull_all;

/**
 * @brief 结束写入器枚举 (Finalize writer enumeration)
 *
 * @param[in,out] st 写入器枚举结构体指针 (Pointer to the writer enumeration structure)
 */
void ddsi_entidx_enum_writer_fini(struct ddsi_entity_enum_writer* st) ddsrt_nonnull_all;

/**
 * @brief 结束读取器枚举 (Finalize reader enumeration)
 *
 * @param[in,out] st 读取器枚举结构体指针 (Pointer to the reader enumeration structure)
 */
void ddsi_entidx_enum_reader_fini(struct ddsi_entity_enum_reader* st) ddsrt_nonnull_all;

/** @component entity_index */
// 结束代理写入器枚举 (Finalize proxy writer enumeration)
void ddsi_entidx_enum_proxy_writer_fini(struct ddsi_entity_enum_proxy_writer* st) ddsrt_nonnull_all;
// 结束代理读取器枚举 (Finalize proxy reader enumeration)
void ddsi_entidx_enum_proxy_reader_fini(struct ddsi_entity_enum_proxy_reader* st) ddsrt_nonnull_all;
// 结束参与者枚举 (Finalize participant enumeration)
void ddsi_entidx_enum_participant_fini(struct ddsi_entity_enum_participant* st) ddsrt_nonnull_all;
// 结束代理参与者枚举 (Finalize proxy participant enumeration)
void ddsi_entidx_enum_proxy_participant_fini(struct ddsi_entity_enum_proxy_participant* st)
    ddsrt_nonnull_all;

#ifdef DDS_HAS_TOPIC_DISCOVERY

/** @component entity_index */
// 将主题插入到实体索引中 (Insert topic into entity index)
void ddsi_entidx_insert_topic_guid(struct ddsi_entity_index* ei,
                                   struct ddsi_topic* tp) ddsrt_nonnull_all;
// 从实体索引中移除主题 (Remove topic from entity index)
void ddsi_entidx_remove_topic_guid(struct ddsi_entity_index* ei,
                                   struct ddsi_topic* tp) ddsrt_nonnull_all;

// 主题实体枚举结构 (Topic entity enumeration structure)
struct ddsi_entity_enum_topic {
  struct ddsi_entity_enum st;
};

/** @component entity_index */
// 初始化主题实体枚举 (Initialize topic entity enumeration)
void ddsi_entidx_enum_topic_init(struct ddsi_entity_enum_topic* st,
                                 const struct ddsi_entity_index* ei) ddsrt_nonnull_all;
// 获取下一个主题实体枚举 (Get next topic entity enumeration)
struct ddsi_topic* ddsi_entidx_enum_topic_next(struct ddsi_entity_enum_topic* st) ddsrt_nonnull_all;
// 结束主题实体枚举 (Finalize topic entity enumeration)
void ddsi_entidx_enum_topic_fini(struct ddsi_entity_enum_topic* st) ddsrt_nonnull_all;

#endif

#endif /* DDS_HAS_TOPIC_DISCOVERY */

#if defined(__cplusplus)
}
#endif

#endif /* DDSI__ENTITY_INDEX_H */
