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
#ifndef DDSI_HBCONTROL_H
#define DDSI_HBCONTROL_H

#include "dds/ddsrt/time.h"
#include "dds/features.h"

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @struct ddsi_hbcontrol
 * @brief 心跳控制结构体 (Heartbeat control structure)
 *
 * 用于管理心跳发送和接收的相关时间戳和计数器。
 * (Used for managing timestamps and counters related to heartbeat sending and receiving.)
 */
struct ddsi_hbcontrol {
  /**
   * @brief 上次写操作的时间戳 (Timestamp of the last write operation)
   *
   * 记录上一次写操作发生的时间，以便在需要时触发心跳。
   * (Records the time when the last write operation occurred, so as to trigger heartbeats when
   * needed.)
   */
  ddsrt_mtime_t t_of_last_write;

  /**
   * @brief 上次心跳发送的时间戳 (Timestamp of the last heartbeat sent)
   *
   * 记录上一次心跳发送的时间，以便确定何时发送下一个心跳。
   * (Records the time when the last heartbeat was sent, in order to determine when to send the next
   * heartbeat.)
   */
  ddsrt_mtime_t t_of_last_hb;

  /**
   * @brief 上次确认心跳接收的时间戳 (Timestamp of the last acknowledged heartbeat received)
   *
   * 记录上一次收到确认心跳的时间，以便确定何时发送下一个确认心跳。
   * (Records the time when the last acknowledged heartbeat was received, in order to determine when
   * to send the next acknowledged heartbeat.)
   */
  ddsrt_mtime_t t_of_last_ackhb;

  /**
   * @brief 调度时间戳 (Scheduled timestamp)
   *
   * 记录下一个心跳或确认心跳的调度时间。
   * (Records the scheduled time for the next heartbeat or acknowledged heartbeat.)
   */
  ddsrt_mtime_t tsched;

  /**
   * @brief 自上次写操作以来的心跳计数 (Heartbeat count since the last write operation)
   *
   * 记录自上次写操作以来发送的心跳数量，用于跟踪和调整心跳频率。
   * (Records the number of heartbeats sent since the last write operation, for tracking and
   * adjusting the heartbeat frequency.)
   */
  uint32_t hbs_since_last_write;

  /**
   * @brief 最后一个数据包的ID (Last packet ID)
   *
   * 记录最后一个发送或接收的数据包的ID，用于确保数据包按顺序处理。
   * (Records the ID of the last sent or received packet, to ensure packets are processed in order.)
   */
  uint32_t last_packetid;
};

#if defined(__cplusplus)
}
#endif

#endif /* DDSI_HBCONTROL_H */
