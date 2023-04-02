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
#include <ctype.h>
#include <stddef.h>
#include <string.h>

#include "dds/dds.h"
#include "dds/ddsi/ddsi_freelist.h"
#include "dds/ddsi/ddsi_iid.h"
#include "dds/ddsi/ddsi_sertype.h"
#include "dds/ddsrt/heap.h"
#include "dds/ddsrt/md5.h"
#include "dds__serdata_builtintopic.h"

// 定义一个新的内建主题序列化类型实现函数
static struct ddsi_sertype *new_sertype_builtintopic_impl(
    enum ddsi_sertype_builtintopic_entity_kind entity_kind,  // 实体类型枚举值
    const char *typename,                                    // 类型名称
    const struct ddsi_serdata_ops *serdata_ops)              // 序列化数据操作结构体指针
{
  // 分配内存并初始化内建主题序列化类型结构体
  struct ddsi_sertype_builtintopic *tp = ddsrt_malloc(sizeof(*tp));
  // 初始化序列化类型结构体
  ddsi_sertype_init(&tp->c, typename, &ddsi_sertype_ops_builtintopic, serdata_ops, false);
  // 设置实体类型
  tp->entity_kind = entity_kind;
  // 返回序列化类型结构体指针
  return &tp->c;
}

// 创建一个新的内建主题序列化类型
struct ddsi_sertype *dds_new_sertype_builtintopic(
    enum ddsi_sertype_builtintopic_entity_kind entity_kind, const char *typename) {
  // 调用实现函数并返回结果
  return new_sertype_builtintopic_impl(entity_kind, typename, &ddsi_serdata_ops_builtintopic);
}

// 释放内建序列化类型资源
static void sertype_builtin_free(struct ddsi_sertype *tp) {
  // 结束序列化类型
  ddsi_sertype_fini(tp);
  // 释放内存
  ddsrt_free(tp);
}

// 比较两个内建序列化类型是否相等
static bool sertype_builtin_equal(const struct ddsi_sertype *acmn,
                                  const struct ddsi_sertype *bcmn) {
  // 类型转换为内建主题序列化类型结构体指针
  const struct ddsi_sertype_builtintopic *a = (struct ddsi_sertype_builtintopic *)acmn;
  const struct ddsi_sertype_builtintopic *b = (struct ddsi_sertype_builtintopic *)bcmn;
  // 比较实体类型是否相等
  return a->entity_kind == b->entity_kind;
}

// 计算内建序列化类型的哈希值
static uint32_t sertype_builtin_hash(const struct ddsi_sertype *tpcmn) {
  // 类型转换为内建主题序列化类型结构体指针
  const struct ddsi_sertype_builtintopic *tp = (struct ddsi_sertype_builtintopic *)tpcmn;
  // 返回实体类型作为哈希值
  return (uint32_t)tp->entity_kind;
}

// 释放参与者资源
static void free_pp(void *vsample) {
  // 类型转换为内建主题参与者结构体指针
  dds_builtintopic_participant_t *sample = vsample;
  // 删除QoS并设置为空
  dds_delete_qos(sample->qos);
  sample->qos = NULL;
}
// 如果定义了DDS_HAS_TOPIC_DISCOVERY宏
#ifdef DDS_HAS_TOPIC_DISCOVERY

// 定义一个函数，用于创建新的内置主题类型的dds_sertype结构体实例
struct ddsi_sertype *dds_new_sertype_builtintopic_topic(
    enum ddsi_sertype_builtintopic_entity_kind entity_kind, const char *typename) {
  // 调用new_sertype_builtintopic_impl函数，并传入实体类型、类型名和序列化操作指针
  return new_sertype_builtintopic_impl(entity_kind, typename, &ddsi_serdata_ops_builtintopic_topic);
}

// 定义一个静态函数，用于释放主题资源
static void free_topic(void *vsample) {
  // 将void指针转换为dds_builtintopic_topic_t类型的指针
  dds_builtintopic_topic_t *sample = vsample;
  // 释放主题名称资源
  dds_free(sample->topic_name);
  // 释放类型名称资源
  dds_free(sample->type_name);
  // 删除QoS设置
  dds_delete_qos(sample->qos);
  // 将主题名称、类型名称和QoS设置置为空
  sample->topic_name = sample->type_name = NULL;
  sample->qos = NULL;
}

// 结束DDS_HAS_TOPIC_DISCOVERY宏定义
#endif /* DDS_HAS_TOPIC_DISCOVERY */

// 定义一个静态函数，用于释放端点资源
static void free_endpoint(void *vsample) {
  // 将void指针转换为dds_builtintopic_endpoint_t类型的指针
  dds_builtintopic_endpoint_t *sample = vsample;
  // 释放主题名称资源
  dds_free(sample->topic_name);
  // 释放类型名称资源
  dds_free(sample->type_name);
  // 删除QoS设置
  dds_delete_qos(sample->qos);
  // 将主题名称、类型名称和QoS设置置为空
  sample->topic_name = sample->type_name = NULL;
  sample->qos = NULL;
}

// 获取实体类型对应的大小
static size_t get_size(enum ddsi_sertype_builtintopic_entity_kind entity_kind) {
  // 根据实体类型进行判断
  switch (entity_kind) {
    case DSBT_PARTICIPANT:
      // 参与者类型，返回对应结构体的大小
      return sizeof(dds_builtintopic_participant_t);
    case DSBT_TOPIC:
#ifdef DDS_HAS_TOPIC_DISCOVERY
      // 主题类型，返回对应结构体的大小（如果启用了主题发现）
      return sizeof(dds_builtintopic_topic_t);
#else
      // 否则跳出 switch 语句
      break;
#endif
    case DSBT_READER:
    case DSBT_WRITER:
      // 读写器类型，返回对应结构体的大小
      return sizeof(dds_builtintopic_endpoint_t);
  }
  // 如果没有匹配到任何类型，触发断言
  assert(0);
  // 返回 0
  return 0;
}

// 将样本置零
static void sertype_builtin_zero_samples(const struct ddsi_sertype *sertype_common,
                                         void *samples,
                                         size_t count) {
  // 类型转换为内建主题类型
  const struct ddsi_sertype_builtintopic *tp =
      (const struct ddsi_sertype_builtintopic *)sertype_common;
  // 获取实体类型对应的大小
  size_t size = get_size(tp->entity_kind);
  // 将 samples 的内存区域置零
  memset(samples, 0, size * count);
}

// 重新分配样本内存
static void sertype_builtin_realloc_samples(void **ptrs,
                                            const struct ddsi_sertype *sertype_common,
                                            void *old,
                                            size_t oldcount,
                                            size_t count) {
  // 类型转换为内建主题类型
  const struct ddsi_sertype_builtintopic *tp =
      (const struct ddsi_sertype_builtintopic *)sertype_common;
  // 获取实体类型对应的大小
  const size_t size = get_size(tp->entity_kind);
  // 根据新旧计数值重新分配内存
  char *new = (oldcount == count) ? old : dds_realloc(old, size * count);
  // 如果新内存存在且新计数值大于旧计数值，将新增部分置零
  if (new &&count > oldcount) memset(new + size *oldcount, 0, size * (count - oldcount));
  // 遍历新内存区域，设置指针数组
  for (size_t i = 0; i < count; i++) {
    void *ptr = (char *)new + i *size;
    ptrs[i] = ptr;
  }
}

// 释放内建序列化类型的样本内存
static void sertype_builtin_free_samples(const struct ddsi_sertype *sertype_common,
                                         void **ptrs,
                                         size_t count,
                                         dds_free_op_t op) {
  // 如果计数值大于0，执行释放操作
  if (count > 0) {
    // 类型转换为内建主题类型
    const struct ddsi_sertype_builtintopic *tp =
        (const struct ddsi_sertype_builtintopic *)sertype_common;
    // 获取实体类型对应的大小
    const size_t size = get_size(tp->entity_kind);
#ifndef NDEBUG
    // 断言检查指针数组中的元素是否按顺序排列
    for (size_t i = 0, off = 0; i < count; i++, off += size)
      assert((char *)ptrs[i] == (char *)ptrs[0] + off);
#endif
    // 如果需要释放内容
    if (op & DDS_FREE_CONTENTS_BIT) {
      void (*f)(void *) = 0;
      char *ptr = ptrs[0];
      // 根据实体类型选择释放函数
      switch (tp->entity_kind) {
        case DSBT_PARTICIPANT:
          f = free_pp;
          break;
        case DSBT_TOPIC:
#ifdef DDS_HAS_TOPIC_DISCOVERY
          f = free_topic;
#endif
          break;
        case DSBT_READER:
        case DSBT_WRITER:
          f = free_endpoint;
          break;
      }
      // 断言检查释放函数是否存在
      assert(f != 0);
      // 遍历指针数组，释放内容
      for (size_t i = 0; i < count; i++) {
        f(ptr);
        ptr += size;
      }
    }
    // 如果需要释放所有内存
    if (op & DDS_FREE_ALL_BIT) {
      dds_free(ptrs[0]);
    }
  }
}

// 内建主题序列化类型操作结构体定义
const struct ddsi_sertype_ops ddsi_sertype_ops_builtintopic = {
    .version = ddsi_sertype_v0,                          // 版本信息
    .arg = 0,                                            // 参数
    .equal = sertype_builtin_equal,                      // 判断相等的函数
    .hash = sertype_builtin_hash,                        // 计算哈希值的函数
    .free = sertype_builtin_free,                        // 释放内存的函数
    .zero_samples = sertype_builtin_zero_samples,        // 将样本置零的函数
    .realloc_samples = sertype_builtin_realloc_samples,  // 重新分配样本内存的函数
    .free_samples = sertype_builtin_free_samples,        // 释放样本内存的函数
    .type_id = 0,                                        // 类型ID
    .type_map = 0,                                       // 类型映射
    .type_info = 0,                                      // 类型信息
    .get_serialized_size = 0,                            // 获取序列化大小的函数
    .serialize_into = 0};                                // 序列化到目标内存的函数
