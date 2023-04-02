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

/* Feature macros:

   - SSM: support for source-specific multicast
     requires: NETWORK_PARTIITONS
     also requires platform support; SSM is silently disabled if the
     platform doesn't support it

   - IPV6: support for IPV6
     requires: platform support (which itself is not part of DDSI)

   - NETWORK_PARTITIONS: support for multiple network partitions

*/
/* 特性宏:

   - SSM: 支持源特定多播 (Source-Specific Multicast)
     要求: NETWORK_PARTIITONS
     还需要平台支持; 如果平台不支持, SSM 将被静默禁用

   - IPV6: 支持 IPV6
     要求: 平台支持 (这本身不是 DDSI 的一部分)

   - NETWORK_PARTITIONS: 支持多个网络分区

*/
#include "dds/features.h"

#ifdef DDS_HAS_SSM
#ifndef DDS_HAS_NETWORK_PARTITIONS
#error "SSM requires NETWORK_PARTITIONS"
#endif

#include "dds/ddsrt/sockets.h"
#ifndef DDSRT_HAVE_SSM
#error "DDSRT_HAVE_SSM should be defined"
#elif !DDSRT_HAVE_SSM
#undef DDS_HAS_SSM
#endif
#endif
