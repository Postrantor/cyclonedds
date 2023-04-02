/*
 * Copyright(c) 2006 to 2019 ZettaScale Technology and others
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v. 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0, or the Eclipse Distribution License
 * v. 1.0 which is available at
 * http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * SPDX-License-Identifier: EPL-2.0 OR BSD-3-Clause
 */
#include "ddsi__rhc.h"

/**
 * @brief 释放读者历史缓存 (Free the reader history cache)
 *
 * @param rhc 一个指向 ddsi_rhc 结构的指针 (A pointer to a ddsi_rhc structure)
 */
extern inline void ddsi_rhc_free(struct ddsi_rhc* rhc);

/**
 * @brief 将样本存储到读者历史缓存中 (Store a sample in the reader history cache)
 *
 * @param rhc 一个指向 ddsi_rhc 结构的指针 (A pointer to a ddsi_rhc structure)
 * @param wrinfo 一个指向 ddsi_writer_info 结构的指针，包含与写入器相关的信息 (A pointer to a
 * ddsi_writer_info structure containing information related to the writer)
 * @param sample 一个指向 ddsi_serdata 结构的指针，表示要存储的样本 (A pointer to a ddsi_serdata
 * structure representing the sample to be stored)
 * @param tk 一个指向 ddsi_tkmap_instance 结构的指针，表示与样本关联的实例 (A pointer to a
 * ddsi_tkmap_instance structure representing the instance associated with the sample)
 * @return 如果成功存储样本，则返回 true，否则返回 false (Returns true if the sample was
 * successfully stored, false otherwise)
 */
extern inline bool ddsi_rhc_store(struct ddsi_rhc* __restrict rhc,
                                  const struct ddsi_writer_info* __restrict wrinfo,
                                  struct ddsi_serdata* __restrict sample,
                                  struct ddsi_tkmap_instance* __restrict tk);

/**
 * @brief 从读者历史缓存中注销写入器 (Unregister a writer from the reader history cache)
 *
 * @param rhc 一个指向 ddsi_rhc 结构的指针 (A pointer to a ddsi_rhc structure)
 * @param wrinfo 一个指向 ddsi_writer_info 结构的指针，包含与写入器相关的信息 (A pointer to a
 * ddsi_writer_info structure containing information related to the writer)
 */
extern inline void ddsi_rhc_unregister_wr(struct ddsi_rhc* __restrict rhc,
                                          const struct ddsi_writer_info* __restrict wrinfo);

/**
 * @brief 放弃读者历史缓存中的所有权 (Relinquish ownership in the reader history cache)
 *
 * @param rhc 一个指向 ddsi_rhc 结构的指针 (A pointer to a ddsi_rhc structure)
 * @param wr_iid 写入器实例的唯一标识符 (The unique identifier of the writer instance)
 */
extern inline void ddsi_rhc_relinquish_ownership(struct ddsi_rhc* __restrict rhc,
                                                 const uint64_t wr_iid);

/**
 * @brief 设置读者历史缓存的服务质量 (Set the Quality of Service for the reader history cache)
 *
 * @param rhc 一个指向 ddsi_rhc 结构的指针 (A pointer to a ddsi_rhc structure)
 * @param qos 一个指向 dds_qos 结构的指针，表示要设置的服务质量 (A pointer to a dds_qos structure
 * representing the Quality of Service to be set)
 */
extern inline void ddsi_rhc_set_qos(struct ddsi_rhc* rhc, const struct dds_qos* qos);
