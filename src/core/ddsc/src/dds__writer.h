/*
 * Copyright(c) 2006 to 2021 ZettaScale Technology and others
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v. 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0, or the Eclipse Distribution License
 * v. 1.0 which is available at
 * http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * SPDX-License-Identifier: EPL-2.0 OR BSD-3-Clause
 */
#ifndef DDS__WRITER_H
#define DDS__WRITER_H

#include "dds__entity.h"

#if defined(__cplusplus)
extern "C"
{
#endif

  DEFINE_ENTITY_LOCK_UNLOCK(dds_writer, DDS_KIND_WRITER, writer)

  struct ddsi_status_cb_data;

  /**
   * @brief DDS写入器状态回调函数
   * @param entity 指向实体的指针
   * @param data 指向ddsi_status_cb_data结构的指针，包含状态信息
   * @component writer
   */
  void dds_writer_status_cb(void *entity, const struct ddsi_status_cb_data *data);

  /**
   * @brief 返回DDS写入器的借用缓冲区
   * @param writer 指向dds_writer的指针
   * @param buf 指向缓冲区指针的指针
   * @param bufsz 缓冲区大小（int32_t类型）
   * @return 返回dds_return_t类型的结果
   * @component writer
   */
  dds_return_t dds_return_writer_loan(dds_writer *writer, void **buf, int32_t bufsz) ddsrt_nonnull_all;

  /**
   * @brief 等待DDS写入器的确认消息
   * @param wr 指向dds_writer的指针
   * @param rdguid 指向读取器GUID的指针
   * @param abstimeout 绝对超时时间（dds_time_t类型）
   * @return 返回dds_return_t类型的结果
   * @component writer
   */
  dds_return_t dds__ddsi_writer_wait_for_acks(struct dds_writer *wr, ddsi_guid_t *rdguid, dds_time_t abstimeout);

#if defined(__cplusplus)
}
#endif

#endif /* DDS__WRITER_H */
