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
#ifndef DDSI_DELIVER_LOCALLY_H
#define DDSI_DELIVER_LOCALLY_H

#include <stdbool.h>
#include <stdint.h>

#include "dds/ddsi/ddsi_guid.h"
#include "dds/ddsrt/avl.h"
#include "dds/ddsrt/retcode.h"
#include "dds/export.h"

#if defined(__cplusplus)
extern "C" {
#endif

struct ddsi_domaingv;
struct ddsi_tkmap_instance;
struct ddsi_sertype;
struct ddsi_serdata;
struct ddsi_entity_index;
struct ddsi_reader;
struct ddsi_entity_common;
struct ddsi_writer_info;
struct ddsi_local_reader_ary;

/**
 * @brief 生成序列化数据的函数指针类型
 * (Function pointer type for creating serialized data)
 *
 * @param[out] tk 指向 ddsi_tkmap_instance 的指针的指针
 *                (Pointer to a pointer to a ddsi_tkmap_instance)
 * @param[in] gv 指向 ddsi_domaingv 结构体的指针
 *               (Pointer to a ddsi_domaingv structure)
 * @param[in] type 指向常量 ddsi_sertype 结构体的指针
 *                 (Pointer to a constant ddsi_sertype structure)
 * @param[in] vsourceinfo 指向源信息的指针
 *                        (Pointer to source information)
 * @return 返回指向 ddsi_serdata 结构体的指针
 *         (Returns a pointer to a ddsi_serdata structure)
 */
typedef struct ddsi_serdata* (*deliver_locally_makesample_t)(struct ddsi_tkmap_instance** tk,
                                                             struct ddsi_domaingv* gv,
                                                             struct ddsi_sertype const* const type,
                                                             void* vsourceinfo);

/**
 * @brief 获取第一个本地读取器的函数指针类型
 * (Function pointer type for getting the first local reader)
 *
 * @param[in] entity_index 指向 ddsi_entity_index 结构体的指针
 *                          (Pointer to a ddsi_entity_index structure)
 * @param[in] source_entity 指向 ddsi_entity_common 结构体的指针
 *                           (Pointer to a ddsi_entity_common structure)
 * @param[out] it 指向 ddsrt_avl_iter_t 结构体的指针
 *                (Pointer to a ddsrt_avl_iter_t structure)
 * @return 返回指向 ddsi_reader 结构体的指针
 *         (Returns a pointer to a ddsi_reader structure)
 */
typedef struct ddsi_reader* (*deliver_locally_first_reader_t)(
    struct ddsi_entity_index* entity_index,
    struct ddsi_entity_common* source_entity,
    ddsrt_avl_iter_t* it);

/**
 * @brief 获取下一个本地读取器的函数指针类型
 * (Function pointer type for getting the next local reader)
 *
 * @param[in] entity_index 指向 ddsi_entity_index 结构体的指针
 *                          (Pointer to a ddsi_entity_index structure)
 * @param[out] it 指向 ddsrt_avl_iter_t 结构体的指针
 *                (Pointer to a ddsrt_avl_iter_t structure)
 * @return 返回指向 ddsi_reader 结构体的指针
 *         (Returns a pointer to a ddsi_reader structure)
 */
typedef struct ddsi_reader* (*deliver_locally_next_reader_t)(struct ddsi_entity_index* entity_index,
                                                             ddsrt_avl_iter_t* it);

/**
 * @brief 快速路径失败时的处理函数指针类型
 * (Function pointer type for handling failure on fastpath)
 *
 * @param[in] source_entity 指向 ddsi_entity_common 结构体的指针
 *                           (Pointer to a ddsi_entity_common structure)
 * @param[in] source_entity_locked 源实体锁定状态
 *                                 (Source entity locked state)
 * @param[in] fastpath_rdary 指向 ddsi_local_reader_ary 结构体的指针
 *                            (Pointer to a ddsi_local_reader_ary structure)
 * @param[in] vsourceinfo 指向源信息的指针
 *                        (Pointer to source information)
 * @return 返回 dds_return_t 类型的结果
 *         (Returns a dds_return_t type result)
 */
typedef dds_return_t (*deliver_locally_on_failure_fastpath_t)(
    struct ddsi_entity_common* source_entity,
    bool source_entity_locked,
    struct ddsi_local_reader_ary* fastpath_rdary,
    void* vsourceinfo);

/**
 * @brief 本地传递操作结构体
 * (Local delivery operations structure)
 */
struct ddsi_deliver_locally_ops {
  deliver_locally_makesample_t makesample;
  deliver_locally_first_reader_t first_reader;
  deliver_locally_next_reader_t next_reader;
  deliver_locally_on_failure_fastpath_t on_failure_fastpath;
};

/**
 * @brief 将数据传递给所有同步的本地读取器
 * (Deliver data to all in-sync local readers)
 *
 * @param[in] gv 指向 ddsi_domaingv 结构体的指针
 *               (Pointer to a ddsi_domaingv structure)
 * @param[in] source_entity 指向 ddsi_entity_common 结构体的指针
 *                           (Pointer to a ddsi_entity_common structure)
 * @param[in] source_entity_locked 源实体锁定状态
 *                                 (Source entity locked state)
 * @param[in] fastpath_rdary 指向 ddsi_local_reader_ary 结构体的指针
 *                            (Pointer to a ddsi_local_reader_ary structure)
 * @param[in] wrinfo 指向 ddsi_writer_info 结构体的指针
 *                   (Pointer to a ddsi_writer_info structure)
 * @param[in] ops 指向 ddsi_deliver_locally_ops 结构体的指针
 *                (Pointer to a ddsi_deliver_locally_ops structure)
 * @param[in] vsourceinfo 指向源信息的指针
 *                        (Pointer to source information)
 * @return 返回 dds_return_t 类型的结果
 *         (Returns a dds_return_t type result)
 */
dds_return_t ddsi_deliver_locally_allinsync(struct ddsi_domaingv* gv,
                                            struct ddsi_entity_common* source_entity,
                                            bool source_entity_locked,
                                            struct ddsi_local_reader_ary* fastpath_rdary,
                                            const struct ddsi_writer_info* wrinfo,
                                            const struct ddsi_deliver_locally_ops* __restrict ops,
                                            void* vsourceinfo);

#if defined(__cplusplus)
}
#endif

#endif /* DDSI_DELIVER_LOCALLY_H */
