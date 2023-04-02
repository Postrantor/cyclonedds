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
/* TODO: do we really need to expose this as an API? */

/** @file
 *
 * @brief DDS C Allocation API
 *
 * 本头文件定义了 Eclipse Cyclone DDS C 语言绑定中分配便捷功能的公共 API。
 * This header file defines the public API of allocation convenience functions
 * in the Eclipse Cyclone DDS C language binding.
 */
#ifndef DDS_ALLOC_H
#define DDS_ALLOC_H

#include <stddef.h>

#include "dds/export.h"

#if defined(__cplusplus)
extern "C" {
#endif

struct dds_topic_descriptor;
struct dds_sequence;

// 默认的 CDR 流分配器
// Default CDR stream allocator
DDS_EXPORT extern const struct dds_cdrstream_allocator dds_cdrstream_default_allocator;

/**
 * @anchor DDS_FREE_KEY_BIT
 * @ingroup alloc
 * @brief 释放样本中所有 keyfields 的指令
 * @brief Instruction to free all keyfields in sample
 */
#define DDS_FREE_KEY_BIT 0x01

/**
 * @anchor DDS_FREE_CONTENTS_BIT
 * @ingroup alloc
 * @brief 释放样本中所有非 keyfields 的指令
 * @brief Instruction to free all non-keyfields in sample
 */
#define DDS_FREE_CONTENTS_BIT 0x02

/**
 * @anchor DDS_FREE_ALL_BIT
 * @ingroup alloc
 * @brief 释放外部样本的指令
 * @brief Instruction to free outer sample
 */
#define DDS_FREE_ALL_BIT 0x04

/**
 * @brief 释放操作类型
 * @brief Freeing operation type
 * @ingroup alloc
 * 要释放样本的哪个部分
 * What part of a sample to free
 */
typedef enum {
  DDS_FREE_ALL = DDS_FREE_KEY_BIT | DDS_FREE_CONTENTS_BIT |
                 DDS_FREE_ALL_BIT, /**< 释放完整样本 free full sample */
  DDS_FREE_CONTENTS = DDS_FREE_KEY_BIT |
                      DDS_FREE_CONTENTS_BIT, /**< 释放所有样本内容，但保留样本指针不变 free all
                                                sample contents, but leave sample pointer intact */
  DDS_FREE_KEY =
      DDS_FREE_KEY_BIT /**< 仅释放样本中的 keyfields free only the keyfields in a sample */
} dds_free_op_t;

/**
 * @brief DDS 分配器
 * @brief DDS Allocator
 * @ingroup alloc
 * C 风格分配器 API
 * C-Style allocator API
 */
typedef struct dds_allocator {
  void *(*malloc)(size_t size); /**< 类似于 C 中的 malloc behave like C malloc */
  void *(*realloc)(
      void *ptr,
      size_t size); /**< 类似于 C 中的 realloc，可以为空 behave like C realloc, may be null */
  void (*free)(void *ptr); /**< 类似于 C 中的 free behave like C free */
} dds_allocator_t;

/**
 * @brief 使用默认分配器执行 alloc()。
 * @brief Perform an alloc() with the default allocator.
 * @component memory_alloc
 *
 * @param[in] size 字节数
 * @param[in] size number of bytes
 * @returns 新指针或内存不足时返回 NULL
 * @returns new pointer or NULL if out of memory
 */
DDS_EXPORT void *dds_alloc(size_t size);

/**
 * @brief 使用默认分配器执行 realloc()。
 * @brief Perform a realloc() with the default allocator.
 * @component memory_alloc
 *
 * @param[in] ptr 先前分配的指针
 * @param[in] ptr previously alloc()'ed pointer
 * @param[in] size 新大小
 * @param[in] size new size
 * @return 新指针或内存不足时返回 NULL
 * @return new pointer or NULL if out of memory
 */
DDS_EXPORT void *dds_realloc(void *ptr, size_t size);

/**
 * @brief 使用默认分配器执行realloc()。将内存清零。
 * @brief Perform a realloc() with the default allocator. Zero out memory.
 * @component memory_alloc
 *
 * @param[in] ptr 先前分配的指针
 * @param[in] ptr previously alloc()'ed pointer
 * @param[in] size 新大小
 * @param[in] size new size
 * @return 新指针，如果内存不足则返回NULL
 * @return new pointer or NULL if out of memory
 */
DDS_EXPORT void *dds_realloc_zero(void *ptr, size_t size);

/**
 * @brief 对使用默认分配器分配的内存片段执行free()。
 * @brief Perform a free() on a memory fragment allocated with the default allocator.
 * @component memory_alloc
 *
 * @param[in] ptr 先前分配的指针
 * @param[in] ptr previously alloc()'ed pointer
 */
DDS_EXPORT void dds_free(void *ptr);

#ifndef DOXYGEN_SHOULD_SKIP_THIS
typedef void *(*dds_alloc_fn_t)(size_t);
typedef void *(*dds_realloc_fn_t)(void *, size_t);
typedef void (*dds_free_fn_t)(void *);
#endif  // DOXYGEN_SHOULD_SKIP_THIS

/**
 * @brief 分配一个带有大小的字符串，考虑到空终止符。
 * @brief Allocated a string with size, accounting for the null terminator.
 * @component memory_alloc
 *
 * @param[in] size 字符数
 * @param[in] size number of characters
 * @returns 新分配的字符串，如果内存不足则返回NULL
 * @returns newly allocated string or NULL if out of memory
 */
DDS_EXPORT char *dds_string_alloc(size_t size);

/**
 * @brief 复制一个空终止字符串
 * @brief Duplicate a null-terminated string
 * @component memory_alloc
 *
 * @param[in] str 要复制的字符串
 * @param[in] str string to duplicate
 * @returns 新分配的重复字符串，如果内存不足则返回NULL
 * @returns newly allocated duplicate string, or NULL if out of memory
 */
DDS_EXPORT char *dds_string_dup(const char *str);

/**
 * @brief 释放字符串，相当于dds_free
 * @brief Free a string, equivalent to dds_free
 * @component memory_alloc
 *
 * @param[in] str 要释放的字符串
 * @param[in] str string to free
 */
DDS_EXPORT void dds_string_free(char *str);

/**
 * @brief 根据\ref dds_free_op_t释放样本（部分）
 * @brief Free (parts of) a sample according to the \ref dds_free_op_t
 * @component memory_alloc
 *
 * @param[in] sample 要释放的样本
 * @param[in] sample sample to free
 * @param[in] desc 该样本创建的类型的主题描述符。
 * @param[in] desc topic descriptor of the type this sample was created from.
 * @param[in] op 要释放的样本的哪些部分。
 * @param[in] op Which parts of the sample to free.
 */
DDS_EXPORT void dds_sample_free(void *sample,
                                const struct dds_topic_descriptor *desc,
                                dds_free_op_t op);

#if defined(__cplusplus)
}
#endif
#endif
