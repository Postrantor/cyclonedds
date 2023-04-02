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
#ifndef DDSI__RHC_H
#define DDSI__RHC_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "dds/ddsi/ddsi_guid.h"
#include "dds/ddsi/ddsi_rhc.h"
#include "dds/ddsrt/time.h"
#include "dds/export.h"
#include "dds/features.h"

#if defined(__cplusplus)
extern "C" {
#endif

struct dds_qos;

/** @component rhc_if */
/**
 * @brief 注销写入器 (Unregister a writer)
 * @param[in] rhc 读者历史缓存指针 (Pointer to the reader history cache)
 * @param[in] wrinfo 写入器信息指针 (Pointer to the writer info)
 */
inline void ddsi_rhc_unregister_wr(struct ddsi_rhc* __restrict rhc,
                                   const struct ddsi_writer_info* __restrict wrinfo) {
  // 调用读者历史缓存操作的注销写入器函数 (Call the unregister writer function of the reader history
  // cache operations)
  rhc->ops->unregister_wr(rhc, wrinfo);
}

/** @component rhc_if */
/**
 * @brief 放弃所有权 (Relinquish ownership)
 * @param[in] rhc 读者历史缓存指针 (Pointer to the reader history cache)
 * @param[in] wr_iid 写入器实例ID (Writer instance ID)
 */
inline void ddsi_rhc_relinquish_ownership(struct ddsi_rhc* __restrict rhc, const uint64_t wr_iid) {
  // 调用读者历史缓存操作的放弃所有权函数 (Call the relinquish ownership function of the reader
  // history cache operations)
  rhc->ops->relinquish_ownership(rhc, wr_iid);
}

/** @component rhc_if */
/**
 * @brief 设置QoS (Set QoS)
 * @param[in] rhc 读者历史缓存指针 (Pointer to the reader history cache)
 * @param[in] qos 指向DDS QoS结构的指针 (Pointer to the DDS QoS structure)
 */
inline void ddsi_rhc_set_qos(struct ddsi_rhc* rhc, const struct dds_qos* qos) {
  // 调用读者历史缓存操作的设置QoS函数 (Call the set QoS function of the reader history cache
  // operations)
  rhc->ops->set_qos(rhc, qos);
}

/** @component rhc_if */
/**
 * @brief 释放读者历史缓存 (Free reader history cache)
 * @param[in] rhc 读者历史缓存指针 (Pointer to the reader history cache)
 */
inline void ddsi_rhc_free(struct ddsi_rhc* rhc) {
  // 调用读者历史缓存操作的释放函数 (Call the free function of the reader history cache operations)
  rhc->ops->free(rhc);
}

#if defined(__cplusplus)
}
#endif

#endif /* DDSI__RHC_H */
