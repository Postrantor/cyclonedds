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
#ifndef DDSI__ACKNACK_H
#define DDSI__ACKNACK_H

#include <stdbool.h>
#include <stddef.h>

#include "dds/ddsrt/time.h"
#include "ddsi__protocol.h"
#include "ddsi__xevent.h"

#if defined(__cplusplus)
extern "C" {
#endif

// 声明结构体 ddsi_xevent
struct ddsi_xevent;
// 声明结构体 ddsi_pwr_rd_match
struct ddsi_pwr_rd_match;
// 声明结构体 ddsi_proxy_writer
struct ddsi_proxy_writer;

// 定义枚举类型 ddsi_add_acknack_result
enum ddsi_add_acknack_result {
  AANR_SUPPRESSED_ACK,   //!< 发送无操作：距离上次ACK的时间太短
  AANR_ACK,              //!< 发送ACK，没有需要NACK的内容
  AANR_SUPPRESSED_NACK,  //!< 即使有需要NACK的内容，也发送ACK
  AANR_NACK,             //!< 发送NACK，可能还会发送NACKFRAG
  AANR_NACKFRAG_ONLY     //!< 仅发送NACKFRAG
};

DDSRT_STATIC_ASSERT(
  (DDSI_SEQUENCE_NUMBER_SET_MAX_BITS % 32) == 0 && (DDSI_FRAGMENT_NUMBER_SET_MAX_BITS % 32) == 0);
// 定义结构体 ddsi_add_acknack_info
struct ddsi_add_acknack_info
{
  bool nack_sent_on_nackdelay; // NACK延迟后是否已发送NACK
#if ACK_REASON_IN_FLAGS
  uint8_t flags; // 标志位
#endif
  struct
  {
    struct ddsi_sequence_number_set_header set; // 序列号集合头部
    uint32_t bits[DDSI_FRAGMENT_NUMBER_SET_MAX_BITS / 32]; // 位数组
  } acknack;
  struct
  {
    ddsi_seqno_t seq; // 序列号
    struct ddsi_fragment_number_set_header set; // 片段号集合头部
    uint32_t bits[DDSI_FRAGMENT_NUMBER_SET_MAX_BITS / 32]; // 位数组
  } nackfrag;
};

/**
 * @component incoming_rtps
 * @brief 根据需要调度ACKNACK
 *
 * @param[in] ev                 ddsi_xevent结构体指针
 * @param[in] pwr                ddsi_proxy_writer结构体指针
 * @param[in] rwn                ddsi_pwr_rd_match结构体指针
 * @param[in] tnow               当前时间
 * @param[in] avoid_suppressed_nack 是否避免抑制NACK
 */
void ddsi_sched_acknack_if_needed(
  struct ddsi_xevent * ev, struct ddsi_proxy_writer * pwr, struct ddsi_pwr_rd_match * rwn,
  ddsrt_mtime_t tnow, bool avoid_suppressed_nack);

/**
 * @component incoming_rtps
 * @brief 创建并重新调度ACKNACK
 *
 * @param[in] ev                 ddsi_xevent结构体指针
 * @param[in] pwr                ddsi_proxy_writer结构体指针
 * @param[in] rwn                ddsi_pwr_rd_match结构体指针
 * @param[in] tnow               当前时间
 * @param[in] avoid_suppressed_nack 是否避免抑制NACK
 * @return                        返回ddsi_xmsg结构体指针
 */
struct ddsi_xmsg * ddsi_make_and_resched_acknack(
  struct ddsi_xevent * ev, struct ddsi_proxy_writer * pwr, struct ddsi_pwr_rd_match * rwn,
  ddsrt_mtime_t tnow, bool avoid_suppressed_nack);

#if defined(__cplusplus)
}
#endif

#endif /* DDSI__ACKNACK_H */
