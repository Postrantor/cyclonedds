/*
 * Copyright(c) 2021 ZettaScale Technology and others
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v. 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0, or the Eclipse Distribution License
 * v. 1.0 which is available at
 * http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * SPDX-License-Identifier: EPL-2.0 OR BSD-3-Clause
 */
#ifndef DDS__DATA_ALLOCATOR_H
#define DDS__DATA_ALLOCATOR_H

#include "dds/ddsc/dds_data_allocator.h"
#include "dds/ddsrt/static_assert.h"
#include "dds/ddsrt/sync.h"

#if defined(__cplusplus)
extern "C" {
#endif

#ifdef DDS_HAS_SHM

#include "iceoryx_binding_c/publisher.h"
#include "iceoryx_binding_c/subscriber.h"

/**
 * @brief dds_iox_allocator_kind 枚举类型定义了不同的分配器类型。
 *        (The dds_iox_allocator_kind enumeration defines different types of allocators.)
 */
typedef enum dds_iox_allocator_kind {
  DDS_IOX_ALLOCATOR_KIND_FINI,      /**< 分配器已完成。 (Allocator is finished.) */
  DDS_IOX_ALLOCATOR_KIND_NONE,      /**< 使用堆。 (Use heap.) */
  DDS_IOX_ALLOCATOR_KIND_PUBLISHER, /**< 发布者分配器。 (Publisher allocator.) */
  DDS_IOX_ALLOCATOR_KIND_SUBSCRIBER /**< 订阅者分配器。 (Subscriber allocator.) */
} dds_iox_allocator_kind_t;

/**
 * @brief dds_iox_allocator 结构体定义了一个分配器，包含分配器类型、引用和互斥锁。
 *        (The dds_iox_allocator structure defines an allocator with a type, reference, and mutex.)
 */
typedef struct dds_iox_allocator {
  enum dds_iox_allocator_kind kind; /**< 分配器类型。 (Allocator type.) */
  union {
    iox_pub_t pub;                  /**< 发布者引用。 (Publisher reference.) */
    iox_sub_t sub;                  /**< 订阅者引用。 (Subscriber reference.) */
  } ref;
  ddsrt_mutex_t mutex;              /**< 互斥锁。 (Mutex.) */
} dds_iox_allocator_t;

DDSRT_STATIC_ASSERT(sizeof(dds_iox_allocator_t) <= sizeof(dds_data_allocator_t));

#endif  // DDS_HAS_SHM

struct dds_writer;
struct dds_reader;

/**
 * @brief 初始化写入器数据分配器。
 *        (Initialize the writer data allocator.)
 *
 * @param[in] wr 指向 dds_writer 结构体的指针。 (Pointer to the dds_writer structure.)
 * @param[out] data_allocator 指向 dds_data_allocator_t 结构体的指针。 (Pointer to the
 * dds_data_allocator_t structure.)
 * @return 成功返回 DDS_RETCODE_OK，否则返回错误代码。 (Returns DDS_RETCODE_OK on success, otherwise
 * returns an error code.)
 */
dds_return_t dds__writer_data_allocator_init(
    const struct dds_writer* wr, dds_data_allocator_t* data_allocator) ddsrt_nonnull_all;

/**
 * @brief 释放写入器数据分配器。
 *        (Finalize the writer data allocator.)
 *
 * @param[in] wr 指向 dds_writer 结构体的指针。 (Pointer to the dds_writer structure.)
 * @param[out] data_allocator 指向 dds_data_allocator_t 结构体的指针。 (Pointer to the
 * dds_data_allocator_t structure.)
 * @return 成功返回 DDS_RETCODE_OK，否则返回错误代码。 (Returns DDS_RETCODE_OK on success, otherwise
 * returns an error code.)
 */
dds_return_t dds__writer_data_allocator_fini(
    const struct dds_writer* wr, dds_data_allocator_t* data_allocator) ddsrt_nonnull_all;

/**
 * @brief 初始化读取器数据分配器。
 *        (Initialize the reader data allocator.)
 *
 * @param[in] wr 指向 dds_reader 结构体的指针。 (Pointer to the dds_reader structure.)
 * @param[out] data_allocator 指向 dds_data_allocator_t 结构体的指针。 (Pointer to the
 * dds_data_allocator_t structure.)
 * @return 成功返回 DDS_RETCODE_OK，否则返回错误代码。 (Returns DDS_RETCODE_OK on success, otherwise
 * returns an error code.)
 */
dds_return_t dds__reader_data_allocator_init(
    const struct dds_reader* wr, dds_data_allocator_t* data_allocator) ddsrt_nonnull_all;

/**
 * @brief 释放读取器数据分配器。
 *        (Finalize the reader data allocator.)
 *
 * @param[in] wr 指向 dds_reader 结构体的指针。 (Pointer to the dds_reader structure.)
 * @param[out] data_allocator 指向 dds_data_allocator_t 结构体的指针。 (Pointer to the
 * dds_data_allocator_t structure.)
 * @return 成功返回 DDS_RETCODE_OK，否则返回错误代码。 (Returns DDS_RETCODE_OK on success, otherwise
 * returns an error code.)
 */
dds_return_t dds__reader_data_allocator_fini(
    const struct dds_reader* wr, dds_data_allocator_t* data_allocator) ddsrt_nonnull_all;

#if defined(__cplusplus)
}
#endif

#endif /* DDS__DATA_ALLOCATOR_H */
