/*
 * Copyright(c) 2006 to 2019 ZettaScale Technology and others
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v. 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0, or the Eclipse Distribution License
 * v. 1.0 which is available at
 * http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * SPDX-License-Identifier: EPL-2.0 OR BSD-3-Clause
 */
#ifndef DDSI_LAT_ESTIM_H
#define DDSI_LAT_ESTIM_H

#include "dds/ddsi/ddsi_log.h"

#if defined(__cplusplus)
extern "C" {
#endif

#define DDSI_LAT_ESTIM_MEDIAN_WINSZ 7

/**
 * @struct ddsi_lat_estim
 * @brief 结构体用于估计 DDSI 延迟 (Structure for estimating DDSI latency)
 *
 * 该结构体包含了中值滤波和简单的 alpha 滤波，用于平滑延迟数据。
 * (This structure contains median filtering and simple alpha filtering for smoothing latency data.)
 */
struct ddsi_lat_estim {
  /**
   * @brief 窗口索引 (Window index)
   *
   * 用于跟踪当前窗口位置的整数索引。 (An integer index for tracking the current window position.)
   */
  int index;

  /**
   * @brief 中值滤波窗口 (Median filtering window)
   *
   * 一个大小为 DDSI_LAT_ESTIM_MEDIAN_WINSZ 的浮点数组，用于存储延迟数据进行中值滤波。
   * (A float array of size DDSI_LAT_ESTIM_MEDIAN_WINSZ for storing latency data for median
   * filtering.)
   */
  float window[DDSI_LAT_ESTIM_MEDIAN_WINSZ];

  /**
   * @brief 平滑后的延迟值 (Smoothed latency value)
   *
   * 经过简单的 alpha 滤波处理后的平滑延迟值。 (The smoothed latency value after simple alpha
   * filtering.)
   */
  float smoothed;
};

#if defined(__cplusplus)
}
#endif

#endif /* DDSI_LAT_ESTIM_H */
