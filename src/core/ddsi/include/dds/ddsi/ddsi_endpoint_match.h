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
#ifndef DDSI_ENDPOINT_MATCH_H
#define DDSI_ENDPOINT_MATCH_H

#include "dds/ddsi/ddsi_lat_estim.h"
#include "dds/ddsrt/avl.h"
#include "dds/export.h"
#include "dds/features.h"

#if defined(__cplusplus)
extern "C" {
#endif

struct ddsi_writer;
struct ddsi_reader;
struct ddsi_entity_common;
struct dds_qos;

/**
 * @component endpoint_matching
 * @brief 获取与给定写入者匹配的订阅列表 (Get the list of matched subscriptions for the given
 * writer)
 *
 * @param[in] wr 写入者实例指针 (Pointer to the writer instance)
 * @param[out] rds 匹配的订阅实体句柄数组 (Array of matched subscription entity handles)
 * @param[in] nrds 数组大小 (Size of the array)
 * @return 成功返回 DDS_RETCODE_OK，否则返回相应的错误代码 (Returns DDS_RETCODE_OK on success,
 * otherwise returns corresponding error code)
 */
dds_return_t ddsi_writer_get_matched_subscriptions(struct ddsi_writer* wr,
                                                   uint64_t* rds,
                                                   size_t nrds);

/**
 * @component endpoint_matching
 * @brief 获取与给定读取者匹配的发布列表 (Get the list of matched publications for the given reader)
 *
 * @param[in] rd 读取者实例指针 (Pointer to the reader instance)
 * @param[out] wrs 匹配的发布实体句柄数组 (Array of matched publication entity handles)
 * @param[in] nwrs 数组大小 (Size of the array)
 * @return 成功返回 DDS_RETCODE_OK，否则返回相应的错误代码 (Returns DDS_RETCODE_OK on success,
 * otherwise returns corresponding error code)
 */
dds_return_t ddsi_reader_get_matched_publications(struct ddsi_reader* rd,
                                                  uint64_t* wrs,
                                                  size_t nwrs);

/**
 * @component endpoint_matching
 * @brief 在给定写入者的匹配订阅列表中查找指定的读取者 (Find the specified reader in the list of
 * matched subscriptions for the given writer)
 *
 * @param[in] wr 写入者实例指针 (Pointer to the writer instance)
 * @param[in] ih 要查找的读取者实体句柄 (Entity handle of the reader to find)
 * @param[out] rdc 匹配的读取者公共实体指针 (Pointer to the matched reader common entity)
 * @param[out] rdqos 匹配的读取者 QoS 设置指针 (Pointer to the matched reader QoS settings)
 * @param[out] ppc 与匹配的读取者关联的参与者公共实体指针 (Pointer to the participant common entity
 * associated with the matched reader)
 * @return 如果找到匹配的读取者，返回 true，否则返回 false (Returns true if the matched reader is
 * found, otherwise returns false)
 */
bool ddsi_writer_find_matched_reader(struct ddsi_writer* wr,
                                     uint64_t ih,
                                     struct ddsi_entity_common** rdc,
                                     struct dds_qos** rdqos,
                                     struct ddsi_entity_common** ppc);

/**
 * @component endpoint_matching
 * @brief 在给定读取者的匹配发布列表中查找指定的写入者 (Find the specified writer in the list of
 * matched publications for the given reader)
 *
 * @param[in] rd 读取者实例指针 (Pointer to the reader instance)
 * @param[in] ih 要查找的写入者实体句柄 (Entity handle of the writer to find)
 * @param[out] wrc 匹配的写入者公共实体指针 (Pointer to the matched writer common entity)
 * @param[out] wrqos 匹配的写入者 QoS 设置指针 (Pointer to the matched writer QoS settings)
 * @param[out] ppc 与匹配的写入者关联的参与者公共实体指针 (Pointer to the participant common entity
 * associated with the matched writer)
 * @return 如果找到匹配的写入者，返回 true，否则返回 false (Returns true if the matched writer is
 * found, otherwise returns false)
 */
bool ddsi_reader_find_matched_writer(struct ddsi_reader* rd,
                                     uint64_t ih,
                                     struct ddsi_entity_common** wrc,
                                     struct dds_qos** wrqos,
                                     struct ddsi_entity_common** ppc);

#if defined(__cplusplus)
}
#endif

#endif /* DDSI_ENDPOINT_MATCH_H */
