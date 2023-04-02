/*
 * Copyright(c) 2006 to 2020 ZettaScale Technology and others
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v. 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0, or the Eclipse Distribution License
 * v. 1.0 which is available at
 * http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * SPDX-License-Identifier: EPL-2.0 OR BSD-3-Clause
 */
#ifndef DDS__WHC_H
#define DDS__WHC_H

#include "dds/ddsi/ddsi_whc.h"

#if defined(__cplusplus)
extern "C"
{
#endif

  struct ddsi_domaingv;
  struct whc_writer_info;
  struct dds_writer;

  /**
   * @brief 创建一个新的写历史缓存 (WHC)
   *
   * 此函数用于为指定的域和写入者信息创建一个新的写历史缓存 (WHC)。
   *
   * @param gv     指向 ddsi_domaingv 结构的指针，该结构包含域全局变量信息
   * @param wrinfo 指向 whc_writer_info 结构的指针，该结构包含写入者信息
   *
   * @return 返回一个指向新创建的 ddsi_whc 结构的指针
   */
  struct ddsi_whc *dds_whc_new(struct ddsi_domaingv *gv, const struct whc_writer_info *wrinfo);

  /**
   * @brief 从给定的写入者和 QoS 设置中创建 whc_writer_info 结构
   *
   * 此函数用于根据提供的写入者和 QoS 设置创建一个新的 whc_writer_info 结构。
   *
   * @param wr  指向 dds_writer 结构的指针，该结构包含写入者信息
   * @param qos 指向 dds_qos_t 结构的指针，该结构包含 QoS 设置
   *
   * @return 返回一个指向新创建的 whc_writer_info 结构的指针
   */
  struct whc_writer_info *dds_whc_make_wrinfo(struct dds_writer *wr, const dds_qos_t *qos);

  /**
   * @brief 释放 whc_writer_info 结构的内存
   *
   * 此函数用于释放给定的 whc_writer_info 结构所占用的内存。
   *
   * @param wrinfo 指向要释放的 whc_writer_info 结构的指针
   */
  void dds_whc_free_wrinfo(struct whc_writer_info *);

#if defined(__cplusplus)
}
#endif

#endif /* DDS__WHC_H */
