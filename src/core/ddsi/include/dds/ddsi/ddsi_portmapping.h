/*
 * Copyright(c) 2019 to 2020 ZettaScale Technology and others
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v. 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0, or the Eclipse Distribution License
 * v. 1.0 which is available at
 * http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * SPDX-License-Identifier: EPL-2.0 OR BSD-3-Clause
 */
#ifndef DDSI_PORTMAPPING_H
#define DDSI_PORTMAPPING_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @brief ddsi_portmapping 结构体，用于存储端口映射信息 (ddsi_portmapping structure for storing port
 * mapping information)
 *
 * @param base 基础端口号 (Base port number)
 * @param dg 数据广播组端口号 (Data broadcast group port number)
 * @param pg 参数广播组端口号 (Parameter broadcast group port number)
 * @param d0 用于特定目的的端口号 (Port number for specific purpose)
 * @param d1 用于特定目的的端口号 (Port number for specific purpose)
 * @param d2 用于特定目的的端口号 (Port number for specific purpose)
 * @param d3 用于特定目的的端口号 (Port number for specific purpose)
 */
struct ddsi_portmapping {
  uint32_t base;  ///< 基础端口号 (Base port number)
  uint32_t dg;    ///< 数据广播组端口号 (Data broadcast group port number)
  uint32_t pg;    ///< 参数广播组端口号 (Parameter broadcast group port number)
  uint32_t d0;    ///< 用于特定目的的端口号 (Port number for specific purpose)
  uint32_t d1;    ///< 用于特定目的的端口号 (Port number for specific purpose)
  uint32_t d2;    ///< 用于特定目的的端口号 (Port number for specific purpose)
  uint32_t d3;    ///< 用于特定目的的端口号 (Port number for specific purpose)
};

#if defined(__cplusplus)
}
#endif

#endif /* DDSI_PORTMAPPING_H */
