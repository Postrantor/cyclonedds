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
#ifndef DDS__RHC_DEFAULT_H
#define DDS__RHC_DEFAULT_H

#include "dds/features.h"

#if defined(__cplusplus)
extern "C"
{
#endif

  struct dds_rhc;
  struct dds_reader;
  struct ddsi_sertype;
  struct ddsi_domaingv;
  struct dds_rhc_default;
  struct rhc_sample;

  /**
   * @component rhc
   * @brief 创建一个默认的DDS可靠性历史缓存（RHC）实例，并设置是否进行交叉检查
   *
   * @param reader [in] 指向dds_reader的指针，表示要创建RHC的读取器
   * @param gv [in] 指向ddsi_domaingv结构的指针
   * @param type [in] 指向ddsi_sertype结构的指针，表示序列化类型
   * @param xchecks [in] bool类型，表示是否进行交叉检查
   * @return 返回创建的dds_rhc指针
   */
  struct dds_rhc *dds_rhc_default_new_xchecks(dds_reader *reader, struct ddsi_domaingv *gv, const struct ddsi_sertype *type, bool xchecks);

  /**
   * @component rhc
   * @brief 创建一个默认的DDS可靠性历史缓存（RHC）实例
   *
   * @param reader [in] 指向dds_reader的指针，表示要创建RHC的读取器
   * @param type [in] 指向ddsi_sertype结构的指针，表示序列化类型
   * @return 返回创建的dds_rhc指针
   */
  struct dds_rhc *dds_rhc_default_new(struct dds_reader *reader, const struct ddsi_sertype *type);

#ifdef DDS_HAS_LIFESPAN
  /**
   * @component rhc
   * @brief 默认RHC样本过期回调函数
   *
   * @param hc [in] 指向RHC实例的指针
   * @param tnow [in] ddsrt_mtime_t类型，表示当前时间
   * @return 返回ddsrt_mtime_t类型，表示下一个过期时间
   */
  ddsrt_mtime_t dds_rhc_default_sample_expired_cb(void *hc, ddsrt_mtime_t tnow);
#endif

#ifdef DDS_HAS_DEADLINE_MISSED
  /**
   * @component rhc
   * @brief 默认RHC截止日期未达成回调函数
   *
   * @param hc [in] 指向RHC实例的指针
   * @param tnow [in] ddsrt_mtime_t类型，表示当前时间
   * @return 返回ddsrt_mtime_t类型，表示下一个截止日期未达成时间
   */
  ddsrt_mtime_t dds_rhc_default_deadline_missed_cb(void *hc, ddsrt_mtime_t tnow);
#endif

#if defined(__cplusplus)
}
#endif
#endif /* DDS__RHC_DEFAULT_H */
