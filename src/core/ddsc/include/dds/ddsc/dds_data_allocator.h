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

#ifndef DDS_DATA_ALLOCATOR_H
#define DDS_DATA_ALLOCATOR_H

/**
 * @defgroup data_allocator (Data Allocator)
 * @ingroup dds
 * A quick-and-dirty provisional interface
 */

#include "dds/dds.h"
#include "dds/ddsrt/attributes.h"
#include "dds/export.h"

#if defined(__cplusplus)
extern "C" {
#endif

// macOS 的互斥锁需要相当多的空间，但这还不足以使此系统依赖
// (macOS' mutexes require quite a lot of space, but it is not quite enough to make this
// system-dependent) DDS_DATA_ALLOCATOR_MAX_SIZE：用于定义数据分配器最大大小的宏。此宏计算为 12 乘以
// void 指针的大小，以确保在 macOS
// 上有足够的空间来存储互斥锁。尽管如此，这个大小仍然不足以使整个系统依赖于特定平台。
#define DDS_DATA_ALLOCATOR_MAX_SIZE (12 * sizeof(void *))

/**
 * @ingroup data_allocator
 * @brief 数据分配器结构 (Data Allocator structure)
 * 包含给定实体的数据分配器的内部详细信息 (Contains internal details about the data allocator for a
 * given entity)
 */
typedef struct dds_data_allocator {
  dds_entity_t entity; /**< 与此分配器关联的实体 (to which entity this allocator is attached) */
  union {
    unsigned char bytes[DDS_DATA_ALLOCATOR_MAX_SIZE]; /**< 内部详细信息 (internal details) */
    void *align_ptr;                                  /**< 内部详细信息 (internal details) */
    uint64_t align_u64;                               /**< 内部详细信息 (internal details) */
  } opaque;                                           /**< 内部详细信息 (internal details) */
} dds_data_allocator_t;

/**
 * @ingroup data_allocator
 * @component data_alloc
 * @brief 初始化一个用于在读取器/写入器上下文中执行分配/释放的对象 (Initialize an object for
 * performing allocations/frees in the context of a reader/writer)
 *
 * 如果没有更好的可用选项，操作将回退到标准堆分配。 (The operation will fall back to standard heap
 * allocation if nothing better is available.)
 *
 * @param[in] entity 实体句柄 (the handle of the entity)
 * @param[out] data_allocator 要初始化的不透明分配器对象 (opaque allocator object to initialize)
 *
 * @returns 成功或通用错误指示 (success or a generic error indication)
 *
 * @retval DDS_RETCODE_OK
 *    分配器对象已成功初始化 (the allocator object was successfully initialized)
 * @retval DDS_RETCODE_BAD_PARAMETER
 *    实体无效，data_allocator为空指针 (entity is invalid, data_allocator is a null pointer)
 * @retval DDS_RETCODE_PRECONDITION_NOT_MET
 *    Cyclone DDS尚未初始化 (Cyclone DDS is not initialized)
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *    此实体不支持操作 (operation not supported on this entity)
 */
DDS_EXPORT dds_return_t dds_data_allocator_init(dds_entity_t entity,
                                                dds_data_allocator_t *data_allocator);

/**
 * @ingroup data_allocator
 * @component data_alloc
 * @brief 初始化一个用于在堆上执行标准分配/释放的对象 (Initialize an object for performing standard
 * allocations/frees on the heap)
 *
 * @param[out] data_allocator 要初始化的不透明分配器对象 (opaque allocator object to initialize)
 *
 * @returns 成功或通用错误指示 (success or a generic error indication)
 *
 * @retval DDS_RETCODE_OK
 *    分配器对象成功初始化 (the allocator object was successfully initialized)
 * @retval DDS_RETCODE_BAD_PARAMETER
 *    实体无效，data_allocator 是空指针 (entity is invalid, data_allocator is a null pointer)
 * @retval DDS_RETCODE_PRECONDITION_NOT_MET
 *    Cyclone DDS 未初始化 (Cyclone DDS is not initialized)
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *    此实体不支持操作 (operation not supported on this entity)
 */
DDS_EXPORT dds_return_t dds_data_allocator_init_heap(dds_data_allocator_t *data_allocator);

/**
 * @ingroup data_allocator
 * @component data_alloc
 * @brief 最终确定先前初始化的分配器对象 (Finalize a previously initialized allocator object)
 *
 * @param[in,out] data_allocator 要完成的对象 (object to finalize)
 *
 * @returns 成功或错误指示 (success or an error indication)
 *
 * @retval DDS_RETCODE_OK
 *    数据已成功完成 (the data was successfully finalized)
 * @retval DDS_RETCODE_BAD_PARAMETER
 *    data_allocator 不引用有效实体 (data_allocator does not reference a valid entity)
 * @retval DDS_RETCODE_PRECONDITION_NOT_MET
 *    Cyclone DDS 未初始化 (Cyclone DDS is not initialized)
 */
DDS_EXPORT dds_return_t dds_data_allocator_fini(dds_data_allocator_t *data_allocator);

/**
 * @ingroup data_allocator
 * @component data_alloc
 * @brief 使用给定的分配器分配内存 (Allocate memory using the given allocator)
 *
 * @param[in,out] data_allocator 初始化的分配器对象 (initialized allocator object)
 * @param[in] size 以合适的对齐方式分配的最小字节数 (minimum number of bytes to allocate with
 * suitable alignment)
 *
 * @returns 指向至少请求大小的未别名、未初始化内存的指针，或 NULL (a pointer to unaliased,
 * uninitialized memory of at least the requested size, or NULL)
 */
DDS_EXPORT void *dds_data_allocator_alloc(dds_data_allocator_t *data_allocator, size_t size)
    ddsrt_attribute_warn_unused_result ddsrt_attribute_malloc;

/**
 * @ingroup data_allocator
 * @component data_alloc
 * @brief 释放使用给定分配器的内存 (Release memory using the given allocator)
 *
 * @param[in,out] data_allocator 初始化的分配器对象 (initialized allocator object)
 * @param[in] ptr 需要释放的内存 (memory to free)
 *
 * @returns 成功或错误指示 (success or an error indication)
 *
 * @retval DDS_RETCODE_OK
 *    内存成功释放 (the memory was successfully released)
 * @retval DDS_RETCODE_BAD_PARAMETER
 *    data_allocator 不引用有效实体 (data_allocator does not reference a valid entity)
 *  @retval DDS_RETCODE_PRECONDITION_NOT_MET
 *    dds_data_allocator 已经完成 (dds_data_allocator already finalized)
 */
DDS_EXPORT dds_return_t dds_data_allocator_free(dds_data_allocator_t *data_allocator, void *ptr);

#if defined(__cplusplus)
}
#endif
#endif
