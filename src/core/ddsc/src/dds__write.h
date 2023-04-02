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
#ifndef DDS__WRITE_H
#define DDS__WRITE_H

#if defined(__cplusplus)
extern "C"
{
#endif

#define DDS_WR_KEY_BIT 0x01
#define DDS_WR_DISPOSE_BIT 0x02
#define DDS_WR_UNREGISTER_BIT 0x04

  struct ddsi_serdata;

  // 定义一个枚举类型，表示不同的写操作
  typedef enum
  {
    DDS_WR_ACTION_WRITE = 0,                                          // 写操作
    DDS_WR_ACTION_WRITE_DISPOSE = DDS_WR_DISPOSE_BIT,                 // 写并处理操作
    DDS_WR_ACTION_DISPOSE = DDS_WR_KEY_BIT | DDS_WR_DISPOSE_BIT,      // 处理操作
    DDS_WR_ACTION_UNREGISTER = DDS_WR_KEY_BIT | DDS_WR_UNREGISTER_BIT // 注销操作
  } dds_write_action;

  /**
   * @brief 写入数据实现函数
   * @param wr 指向dds_writer的指针
   * @param data 要写入的数据的指针
   * @param tstamp 时间戳
   * @param action 写操作类型（dds_write_action枚举值）
   * @return 返回dds_return_t类型的结果
   * @component write_data
   */
  dds_return_t dds_write_impl(dds_writer *wr, const void *data, dds_time_t tstamp, dds_write_action action);

  /**
   * @brief 写入CDR数据实现函数
   * @param wr 指向dds_writer的指针
   * @param xp 指向ddsi_xpack的指针
   * @param d 指向ddsi_serdata的指针
   * @param flush 是否刷新缓冲区
   * @return 返回dds_return_t类型的结果
   * @component write_data
   */
  dds_return_t dds_writecdr_impl(dds_writer *wr, struct ddsi_xpack *xp, struct ddsi_serdata *d, bool flush);

  /**
   * @brief 写入本地孤立CDR数据实现函数
   * @param lowr 指向ddsi_local_orphan_writer的指针
   * @param d 指向ddsi_serdata的指针
   * @return 返回dds_return_t类型的结果
   * @component write_data
   */
  dds_return_t dds_writecdr_local_orphan_impl(struct ddsi_local_orphan_writer *lowr, struct ddsi_serdata *d);

#if defined(__cplusplus)
}
#endif
#endif /* DDS__WRITE_H */
