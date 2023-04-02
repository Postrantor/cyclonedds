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
#ifndef DDS__STATISTICS_H
#define DDS__STATISTICS_H

#include "dds/ddsc/dds_statistics.h"

#if defined(__cplusplus)
extern "C" {
#endif

// 定义一个结构体，表示键值对描述符
struct dds_stat_keyvalue_descriptor {
  const char* name;         // 键值对的名称
  enum dds_stat_kind kind;  // 键值对的类型（枚举）
};

// 定义一个结构体，表示统计描述符
struct dds_stat_descriptor {
  size_t count;                                   // 描述符中键值对的数量
  const struct dds_stat_keyvalue_descriptor* kv;  // 指向键值对描述符数组的指针
};

/**
 * @brief 分配一个dds_statistics结构体实例
 * @component statistics
 *
 * @param[in] e 指向dds_entity实例的指针
 * @param[in] d 指向dds_stat_descriptor实例的指针
 *
 * @return 返回一个已分配的dds_statistics实例的指针
 */
struct dds_statistics* dds_alloc_statistics(const struct dds_entity* e,
                                            const struct dds_stat_descriptor* d);

#if defined(__cplusplus)
}
#endif
#endif /* DDS__STATISTICS_H */
