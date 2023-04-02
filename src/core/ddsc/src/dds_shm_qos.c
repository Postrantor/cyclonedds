/*
 * Copyright(c) 2023 ZettaScale Technology and others
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

#include "dds/ddsi/ddsi_sertype.h"
#include "dds/ddsrt/heap.h"
#include "dds__qos.h"
#include "dds__shm_qos.h"
#include "dds__topic.h"

/**
 * @brief 判断字符串是否包含通配符
 *
 * @param str 要检查的字符串
 * @return 如果字符串包含 '*' 或 '?'，则返回 true，否则返回 false
 */
static bool is_wildcard_partition(const char* str) { return strchr(str, '*') || strchr(str, '?'); }

#define QOS_CHECK_FIELDS                                                              \
  (DDSI_QP_LIVELINESS | DDSI_QP_DEADLINE | DDSI_QP_RELIABILITY | DDSI_QP_DURABILITY | \
   DDSI_QP_HISTORY)

/**
 * @brief 检查 QoS 和主题是否兼容共享内存
 *
 * @param qos 要检查的 QoS 结构体
 * @param tp 要检查的主题结构体
 * @param check_durability_service 是否检查持久性服务
 * @return 如果 QoS 和主题兼容共享内存，则返回 true，否则返回 false
 */
bool dds_shm_compatible_qos_and_topic(const struct dds_qos* qos,
                                      const struct dds_topic* tp,
                                      bool check_durability_service) {
  // 检查必要条件：固定大小的数据类型或者支持序列化到共享内存
  if (!tp->m_stype->fixed_size &&
      (!tp->m_stype->ops->get_serialized_size || !tp->m_stype->ops->serialize_into)) {
    return false;
  }

  // 历史记录类型必须为 DDS_HISTORY_KEEP_LAST
  if (qos->history.kind != DDS_HISTORY_KEEP_LAST) {
    return false;
  }

  // 持久性类型必须为 DDS_DURABILITY_VOLATILE 或 DDS_DURABILITY_TRANSIENT_LOCAL
  if (!(qos->durability.kind == DDS_DURABILITY_VOLATILE ||
        qos->durability.kind == DDS_DURABILITY_TRANSIENT_LOCAL)) {
    return false;
  }

  // 如果需要检查持久性服务，我们无法支持所需的历史记录
  if (check_durability_service && qos->durability.kind == DDS_DURABILITY_TRANSIENT_LOCAL &&
      qos->durability_service.history.kind == DDS_HISTORY_KEEP_LAST &&
      qos->durability_service.history.depth > (int32_t)iox_cfg_max_publisher_history()) {
    return false;
  }

  // 忽略本地设置必须为 DDS_IGNORELOCAL_NONE
  if (qos->ignorelocal.value != DDS_IGNORELOCAL_NONE) {
    return false;
  }

  // 只允许默认分区或一个非通配符分区
  if (qos->partition.n > 1 ||
      (qos->partition.n == 1 && is_wildcard_partition(qos->partition.strs[0]))) {
    return false;
  }

  // 检查 QoS 设置是否满足要求
  return (QOS_CHECK_FIELDS == (qos->present & QOS_CHECK_FIELDS) &&
          DDS_LIVELINESS_AUTOMATIC == qos->liveliness.kind &&
          DDS_INFINITY == qos->deadline.deadline);
}

/**
 * @brief 根据 QoS 和主题生成共享内存分区主题字符串
 *
 * @param qos 要处理的 QoS 结构体
 * @param tp 要处理的主题结构体
 * @return 生成的共享内存分区主题字符串，需要调用者释放内存
 */
char* dds_shm_partition_topic(const struct dds_qos* qos, const struct dds_topic* tp) {
  // 初始化分区为空字符串
  const char* partition = "";
  // 如果 QoS 中存在分区设置
  if (qos->present & DDSI_QP_PARTITION) {
    // 确保分区数量不超过1
    assert(qos->partition.n <= 1);
    // 如果有一个分区，则获取分区字符串
    if (qos->partition.n == 1) partition = qos->partition.strs[0];
  }
  // 确保分区字符串不为空
  assert(partition);
  // 确保分区字符串中没有通配符
  assert(!is_wildcard_partition(partition));

  // 计算组合字符串长度，允许转义点（使用 C 风格的反斜杠）
  size_t size = 1 + strlen(tp->m_name) + 1;  // 点和终止0
  for (char const* src = partition; *src; src++) {
    if (*src == '\\' || *src == '.') size++;
    size++;
  }
  // 分配组合字符串所需的内存空间
  char* combined = ddsrt_malloc(size);
  // 如果内存分配失败，返回 NULL
  if (combined == NULL) return NULL;
  // 初始化目标指针
  char* dst = combined;
  // 遍历分区字符串，处理转义字符
  for (char const* src = partition; *src; src++) {
    if (*src == '\\' || *src == '.') *dst++ = '\\';
    *dst++ = *src;
  }
  // 添加点分隔符
  *dst++ = '.';
  // 复制主题名称到组合字符串
  strcpy(dst, tp->m_name);
  // 返回生成的共享内存分区主题字符串
  return combined;
}
