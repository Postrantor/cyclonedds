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
#ifndef DDSI_LIFESPAN_H
#define DDSI_LIFESPAN_H

#include "dds/ddsi/ddsi_domaingv.h"
#include "dds/ddsrt/fibheap.h"
#include "dds/ddsrt/time.h"

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @brief 定义一个回调函数类型，用于处理过期的样本。
 *        Define a callback function type for handling expired samples.
 */
typedef ddsrt_mtime_t (*ddsi_sample_expired_cb_t)(void* hc, ddsrt_mtime_t tnow);

/**
 * @brief 生命周期管理结构体。
 *        Lifespan administration structure.
 */
struct ddsi_lifespan_adm {
  ddsrt_fibheap_t ls_exp_heap; /**< 堆，用于存储过期的样本（生命周期）。
                                    Heap for storing expired samples (lifespan). */
  struct ddsi_xevent* evt;     /**< 触发具有最早到期时间的样本的事件。
                                    Event that triggers for sample with earliest expiration. */
  ddsi_sample_expired_cb_t
      sample_expired_cb;       /**< 过期样本的回调函数；此回调可以使用
                                    ddsi_lifespan_next_expired_locked 获取下一个过期样本。
                                    Callback for expired sample; this callback can use
                                    ddsi_lifespan_next_expired_locked to get the next expired sample. */
  size_t fh_offset;            /**< 生命周期管理元素在 whc 或 rhc 中的偏移量。
                                    Offset of lifespan_adm element in whc or rhc. */
  size_t fhn_offset; /**< 生命周期管理节点元素在 whc 或 rhc 节点（样本）中的偏移量。
                          Offset of lifespan_fhnode element in whc or rhc node (sample). */
};

/**
 * @brief 生命周期管理节点结构体。
 *        Lifespan administration node structure.
 */
struct ddsi_lifespan_fhnode {
  ddsrt_fibheap_node_t heapnode; /**< 堆节点。
                                      Heap node. */
  ddsrt_mtime_t t_expire;        /**< 过期时间。
                                     Expiration time. */
};

/**
 * @brief 初始化生命周期管理结构体。
 *        Initialize the lifespan administration structure.
 *
 * @param[in] gv                全局变量指针。
 *                              Pointer to global variables.
 * @param[out] lifespan_adm     生命周期管理结构体指针。
 *                              Pointer to lifespan administration structure.
 * @param[in] fh_offset         生命周期管理元素在 whc 或 rhc 中的偏移量。
 *                              Offset of lifespan_adm element in whc or rhc.
 * @param[in] fh_node_offset    生命周期管理节点元素在 whc 或 rhc 节点（样本）中的偏移量。
 *                              Offset of lifespan_fhnode element in whc or rhc node (sample).
 * @param[in] sample_expired_cb 过期样本的回调函数。
 *                              Callback function for expired samples.
 */
void ddsi_lifespan_init(const struct ddsi_domaingv* gv,
                        struct ddsi_lifespan_adm* lifespan_adm,
                        size_t fh_offset,
                        size_t fh_node_offset,
                        ddsi_sample_expired_cb_t sample_expired_cb);

/**
 * @brief 清理生命周期管理结构体。
 *        Clean up the lifespan administration structure.
 *
 * @param[in] lifespan_adm 生命周期管理结构体指针。
 *                          Pointer to lifespan administration structure.
 */
void ddsi_lifespan_fini(const struct ddsi_lifespan_adm* lifespan_adm);

/**
 * @brief 获取下一个过期的样本。
 *        Get the next expired sample.
 *
 * @param[in]  lifespan_adm 生命周期管理结构体指针。
 *                           Pointer to lifespan administration structure.
 * @param[in]  tnow         当前时间。
 *                           Current time.
 * @param[out] sample       过期样本的指针。
 *                           Pointer to the expired sample.
 * @return 过期时间。
 *         Expiration time.
 */
ddsrt_mtime_t ddsi_lifespan_next_expired_locked(const struct ddsi_lifespan_adm* lifespan_adm,
                                                ddsrt_mtime_t tnow,
                                                void** sample);

/**
 * @brief 注册实际样本。
 *        Register a real sample.
 *
 * @param[in,out] lifespan_adm 生命周期管理结构体指针。
 *                              Pointer to lifespan administration structure.
 * @param[in]     node         生命周期管理节点指针。
 *                              Pointer to lifespan administration node.
 */
void ddsi_lifespan_register_sample_real(struct ddsi_lifespan_adm* lifespan_adm,
                                        struct ddsi_lifespan_fhnode* node);

/**
 * @brief 取消注册实际样本。
 *        Unregister a real sample.
 *
 * @param[in,out] lifespan_adm 生命周期管理结构体指针。
 *                              Pointer to lifespan administration structure.
 * @param[in]     node         生命周期管理节点指针。
 *                              Pointer to lifespan administration node.
 */
void ddsi_lifespan_unregister_sample_real(struct ddsi_lifespan_adm* lifespan_adm,
                                          struct ddsi_lifespan_fhnode* node);

/**
 * @brief 注册锁定的样本。
 *        Register a locked sample.
 *
 * @param[in,out] lifespan_adm 生命周期管理结构体指针。
 *                              Pointer to lifespan administration structure.
 * @param[in]     node         生命周期管理节点指针。
 *                              Pointer to lifespan administration node.
 */
inline void ddsi_lifespan_register_sample_locked(struct ddsi_lifespan_adm* lifespan_adm,
                                                 struct ddsi_lifespan_fhnode* node) {
  if (node->t_expire.v != DDS_NEVER) ddsi_lifespan_register_sample_real(lifespan_adm, node);
}

/**
 * @brief 取消注册锁定的样本。
 *        Unregister a locked sample.
 *
 * @param[in,out] lifespan_adm 生命周期管理结构体指针。
 *                              Pointer to lifespan administration structure.
 * @param[in]     node         生命周期管理节点指针。
 *                              Pointer to lifespan administration node.
 */
inline void ddsi_lifespan_unregister_sample_locked(struct ddsi_lifespan_adm* lifespan_adm,
                                                   struct ddsi_lifespan_fhnode* node) {
  if (node->t_expire.v != DDS_NEVER) ddsi_lifespan_unregister_sample_real(lifespan_adm, node);
}

#if defined(__cplusplus)
}
#endif

#endif /* DDSI_LIFESPAN_H */
