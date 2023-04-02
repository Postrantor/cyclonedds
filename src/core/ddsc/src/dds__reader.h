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
#ifndef DDS__READER_H
#define DDS__READER_H

#include "dds__entity.h"
#include "dds__types.h"

#if defined(__cplusplus)
extern "C"
{
#endif
  /**
   * @brief DDSI状态回调数据结构
   *
   * 该结构用于存储DDSI状态回调的相关数据。
   */
  struct ddsi_status_cb_data;

  /**
   * @component reader
   * @brief DDS读取器状态回调函数
   *
   * 该函数用于处理DDS读取器的状态回调事件。
   *
   * @param entity [in] 指向实体的指针，表示要处理状态回调的实体
   * @param data [in] 指向ddsi_status_cb_data结构的指针，表示状态回调的相关数据
   */
  void dds_reader_status_cb(void *entity, const struct ddsi_status_cb_data *data);

  /**
   * @component reader
   * @brief 返回DDS读取器借用的缓冲区
   *
   * 该函数用于将借用的缓冲区返回给DDS读取器。
   *
   * @param rd [in] 指向dds_reader的指针，表示要返回借用缓冲区的读取器
   * @param buf [in,out] 指向void类型的二级指针，表示要返回的缓冲区指针
   * @param bufsz [in] int32_t类型，表示缓冲区的大小
   *
   * @return 返回dds_return_t类型，表示操作结果
   */
  dds_return_t dds_return_reader_loan(dds_reader *rd, void **buf, int32_t bufsz);

#define DDS_READ_WITHOUT_LOCK (0xFFFFFFED)

  /**
   * @component reader
   * @brief 锁定读取器缓存中的样本数量
   *
   * 返回读取缓存中的样本数量，并锁定读取器缓存以确保样本内容不发生变化。由于缓存被锁定，
   * 你应该能够在不先锁定的情况下进行读取/获取操作。这是通过将DDS_READ_WITHOUT_LOCK值作为maxs传递给read/take调用来实现的。
   * 然后，read/take将不会锁定但仍然会解锁。
   *
   * 用于支持C++中的LENGTH_UNLIMITED。
   *
   * @param entity [in] dds_entity_t类型，表示读取器实体
   * @return 返回uint32_t类型，表示样本数量
   */
  DDS_EXPORT uint32_t dds_reader_lock_samples(dds_entity_t entity);

  DEFINE_ENTITY_LOCK_UNLOCK(dds_reader, DDS_KIND_READER, reader)

#if defined(__cplusplus)
}
#endif

#endif // DDS__READER_H
