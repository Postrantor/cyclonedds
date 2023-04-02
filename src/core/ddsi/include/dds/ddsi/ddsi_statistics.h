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
#ifndef _DDSI_STATISTICS_H_
#define _DDSI_STATISTICS_H_

#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif

// 定义 ddsi_reader 结构体
// Define the ddsi_reader structure
struct ddsi_reader;

// 定义 ddsi_writer 结构体
// Define the ddsi_writer structure
struct ddsi_writer;

/**
 * @brief 获取 ddsi_writer 的统计信息
 * @component ddsi_statistics
 * @param[in] wr 指向 ddsi_writer 结构体的指针
 * @param[out] rexmit_bytes 重传字节数
 * @param[out] throttle_count 节流计数
 * @param[out] time_throttled 节流时间
 * @param[out] time_retransmit 重传时间
 */
/**
 * @brief Get statistics of a ddsi_writer
 * @component ddsi_statistics
 * @param[in] wr Pointer to the ddsi_writer structure
 * @param[out] rexmit_bytes Number of bytes retransmitted
 * @param[out] throttle_count Throttle count
 * @param[out] time_throttled Time throttled
 * @param[out] time_retransmit Time spent on retransmission
 */
void ddsi_get_writer_stats(struct ddsi_writer* wr,
                           uint64_t* __restrict rexmit_bytes,
                           uint32_t* __restrict throttle_count,
                           uint64_t* __restrict time_throttled,
                           uint64_t* __restrict time_retransmit);

/**
 * @brief 获取 ddsi_reader 的统计信息
 * @component ddsi_statistics
 * @param[in] rd 指向 ddsi_reader 结构体的指针
 * @param[out] discarded_bytes 丢弃的字节数
 */
/**
 * @brief Get statistics of a ddsi_reader
 * @component ddsi_statistics
 * @param[in] rd Pointer to the ddsi_reader structure
 * @param[out] discarded_bytes Number of bytes discarded
 */
void ddsi_get_reader_stats(struct ddsi_reader* rd, uint64_t* __restrict discarded_bytes);

#if defined(__cplusplus)
}
#endif
#endif
