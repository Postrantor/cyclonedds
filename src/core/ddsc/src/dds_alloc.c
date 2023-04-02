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
#include <assert.h>
#include <string.h>

#include "dds/cdr/dds_cdrstream.h"
#include "dds/ddsrt/heap.h"

// 定义一个静态的dds_allocator_t结构体，包含内存分配、重新分配和释放函数
static dds_allocator_t dds_allocator_fns = {ddsrt_malloc, ddsrt_realloc, ddsrt_free};

// 定义一个默认的dds_cdrstream_allocator结构体，包含内存分配、重新分配和释放函数
const struct dds_cdrstream_allocator dds_cdrstream_default_allocator = {ddsrt_malloc, ddsrt_realloc,
                                                                        ddsrt_free};

/**
 * @brief 分配内存并初始化为0
 *
 * @param size 要分配的内存大小
 * @return 返回分配的内存指针
 */
void *dds_alloc(size_t size) {
  // 使用分配函数进行内存分配
  void *ret = (dds_allocator_fns.malloc)(size);
  if (ret == NULL) {
    // 如果分配失败，输出致命错误信息
    DDS_FATAL("dds_alloc");
  } else {
    // 将分配的内存初始化为0
    memset(ret, 0, size);
  }
  return ret;
}

/**
 * @brief 重新分配内存
 *
 * @param ptr 指向原始内存的指针
 * @param size 新的内存大小
 * @return 返回重新分配的内存指针
 */
void *dds_realloc(void *ptr, size_t size) {
  // 使用重新分配函数进行内存重新分配
  void *ret = (dds_allocator_fns.realloc)(ptr, size);
  if (ret == NULL)
    // 如果重新分配失败，输出致命错误信息
    DDS_FATAL("dds_realloc");
  return ret;
}

/**
 * @brief 重新分配内存并将新分配的部分初始化为0
 *
 * @param ptr 指向原始内存的指针
 * @param size 新的内存大小
 * @return 返回重新分配的内存指针
 */
void *dds_realloc_zero(void *ptr, size_t size) {
  // 使用dds_realloc函数进行内存重新分配
  void *ret = dds_realloc(ptr, size);
  if (ret) {
    // 将新分配的部分初始化为0
    memset(ret, 0, size);
  }
  return ret;
}
/**
 * @brief 释放内存空间
 *
 * @param ptr 指向要释放的内存空间的指针
 */
void dds_free(void *ptr) {
  // 如果指针不为空，则调用dds_allocator_fns.free释放内存
  if (ptr) (dds_allocator_fns.free)(ptr);
}

/**
 * @brief 分配字符串内存空间
 *
 * @param size 字符串大小（不包括空字符）
 * @return char* 返回分配的字符串内存空间的指针
 */
char *dds_string_alloc(size_t size) {
  // 调用dds_alloc为字符串分配size+1个字节的内存空间，并将结果转换为char类型指针
  return (char *)dds_alloc(size + 1);
}

/**
 * @brief 复制字符串
 *
 * @param str 指向源字符串的指针
 * @return char* 返回复制后的新字符串的指针
 */
char *dds_string_dup(const char *str) {
  char *ret = NULL;
  // 如果源字符串指针不为空
  if (str) {
    // 计算源字符串的长度（包括空字符）
    size_t sz = strlen(str) + 1;
    // 为新字符串分配内存空间
    ret = dds_alloc(sz);
    // 将源字符串内容复制到新字符串内存空间中
    memcpy(ret, str, sz);
  }
  // 返回新字符串的指针
  return ret;
}

/**
 * @brief 释放字符串内存空间
 *
 * @param str 指向要释放的字符串内存空间的指针
 */
void dds_string_free(char *str) {
  // 调用dds_free释放字符串内存空间
  dds_free(str);
}

/**
 * @brief 释放样本键值内存空间
 *
 * @param vsample 指向样本的指针
 * @param desc 指向dds_topic_descriptor结构体的指针
 */
static void dds_sample_free_key(void *vsample, const struct dds_topic_descriptor *desc) {
  char *sample = vsample;
  // 遍历所有键值
  for (uint32_t i = 0; i < desc->m_nkeys; i++) {
    const uint32_t *op = desc->m_ops + desc->m_keys[i].m_offset;
    // 如果操作类型为DDS_OP_VAL_STR，则释放对应的内存空间
    if (DDS_OP_TYPE(*op) == DDS_OP_VAL_STR) dds_free(*(char **)(sample + op[1]));
  }
}

/**
 * @brief 释放样本内存空间
 *
 * @param sample 指向样本的指针
 * @param desc 指向dds_topic_descriptor结构体的指针
 * @param op 释放操作类型（dds_free_op_t枚举）
 */
void dds_sample_free(void *sample, const struct dds_topic_descriptor *desc, dds_free_op_t op) {
  // 确保dds_topic_descriptor指针不为空
  assert(desc);

  // 如果样本指针不为空
  if (sample) {
    // 如果操作类型包含DDS_FREE_CONTENTS_BIT，则释放样本内容内存空间
    if (op & DDS_FREE_CONTENTS_BIT)
      dds_stream_free_sample(sample, &dds_cdrstream_default_allocator, desc->m_ops);
    // 如果操作类型包含DDS_FREE_KEY_BIT，则释放样本键值内存空间
    else if (op & DDS_FREE_KEY_BIT)
      dds_sample_free_key(sample, desc);

    // 如果操作类型包含DDS_FREE_ALL_BIT，则释放整个样本内存空间
    if (op & DDS_FREE_ALL_BIT) dds_free(sample);
  }
}
