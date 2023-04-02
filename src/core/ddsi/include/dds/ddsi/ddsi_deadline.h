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
#ifndef DDSI_DEADLINE_H
#define DDSI_DEADLINE_H

#include "dds/ddsi/ddsi_domaingv.h"
#include "dds/ddsi/ddsi_xevent.h"
#include "dds/ddsrt/circlist.h"
#include "dds/ddsrt/time.h"

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @brief 定义一个函数指针类型，用于处理deadline missed的回调函数。
 *        Define a function pointer type for handling deadline missed callback functions.
 */
typedef ddsrt_mtime_t (*deadline_missed_cb_t)(void* hc, ddsrt_mtime_t tnow);

/**
 * @brief ddsi_deadline_adm 结构体，用于管理 deadline missed 事件。
 *        The ddsi_deadline_adm structure for managing deadline missed events.
 */
struct ddsi_deadline_adm {
  struct ddsrt_circlist list;              /**< 链表，用于存储 deadline missed 的实例。
                                                Linked list for storing instances with deadline missed. */
  struct ddsi_xevent* evt;                 /**< 当实例的 deadline 过期时触发的 xevent。
                                                Xevent that triggers when the deadline expires for an instance. */
  deadline_missed_cb_t deadline_missed_cb; /**< 处理 deadline missed 的回调函数；
                                                使用 ddsi_deadline_next_missed_locked 可获取下一个
                                                missed deadline 的实例。
                                                Callback for deadline missed; this callback can use
                                                ddsi_deadline_next_missed_locked to get the next
                                              instance that has a missed deadline. */
  size_t list_offset; /**< deadline_adm 元素在 whc 或 rhc 中的偏移量。
                           Offset of deadline_adm element in whc or rhc. */
  size_t elem_offset; /**< deadline_elem 元素在 whc 或 rhc 实例中的偏移量。
                           Offset of deadline_elem element in whc or rhc instance. */
  dds_duration_t dur; /**< deadline 持续时间。Deadline duration. */
};

/**
 * @brief deadline_elem 结构体，用于存储与 deadline 相关的信息。
 *        The deadline_elem structure for storing information related to deadlines.
 */
struct deadline_elem {
  struct ddsrt_circlist_elem e; /**< 链表元素。List element. */
  ddsrt_mtime_t t_deadline;     /**< deadline 时间。Deadline time. */
  ddsrt_mtime_t t_last_update;  /**< 上次更新时间。Last update time. */
  uint32_t deadlines_missed;    /**< 错过的 deadline 数量。Number of deadlines missed. */
};

/**
 * @brief 初始化 ddsi_deadline_adm 结构体。
 *        Initialize the ddsi_deadline_adm structure.
 *
 * @param gv Domain global variables
 * @param deadline_adm Pointer to the deadline administration structure
 * @param list_offset Offset of the deadline_adm element in whc or rhc
 * @param elem_offset Offset of the deadline_elem element in whc or rhc instance
 * @param deadline_missed_cb Callback function for handling deadline missed events
 */
void ddsi_deadline_init(const struct ddsi_domaingv* gv,
                        struct ddsi_deadline_adm* deadline_adm,
                        size_t list_offset,
                        size_t elem_offset,
                        deadline_missed_cb_t deadline_missed_cb);

/**
 * @brief 停止 ddsi_deadline_adm 的处理。
 *        Stop processing for the ddsi_deadline_adm.
 *
 * @param deadline_adm Pointer to the deadline administration structure
 */
void ddsi_deadline_stop(const struct ddsi_deadline_adm* deadline_adm);

/**
 * @brief 清除 ddsi_deadline_adm 结构体中的数据。
 *        Clear data in the ddsi_deadline_adm structure.
 *
 * @param deadline_adm Pointer to the deadline administration structure
 */
void ddsi_deadline_clear(struct ddsi_deadline_adm* deadline_adm);

/**
 * @brief 释放 ddsi_deadline_adm 结构体的资源。
 *        Release resources for the ddsi_deadline_adm structure.
 *
 * @param deadline_adm Pointer to the deadline administration structure
 */
void ddsi_deadline_fini(const struct ddsi_deadline_adm* deadline_adm);

/**
 * @brief 获取下一个 missed deadline 的实例（需要在锁定状态下调用）。
 *        Get the next instance with a missed deadline (must be called while locked).
 *
 * @param deadline_adm Pointer to the deadline administration structure
 * @param tnow Current time
 * @param[out] instance Pointer to the instance with the next missed deadline
 * @return 下一个 missed deadline 的时间。The time of the next missed deadline.
 */
ddsrt_mtime_t ddsi_deadline_next_missed_locked(struct ddsi_deadline_adm* deadline_adm,
                                               ddsrt_mtime_t tnow,
                                               void** instance);

/**
 * @component deadline_qos
 * @brief 注册实例的真实处理函数 (Real handling function for registering instances)
 *
 * @param[in] deadline_adm 指向 ddsi_deadline_adm 结构体的指针 (Pointer to the ddsi_deadline_adm
 * structure)
 * @param[in] elem 指向 deadline_elem 结构体的指针 (Pointer to the deadline_elem structure)
 * @param[in] tprev 上一次更新时间 (Previous update time)
 * @param[in] tnow 当前时间 (Current time)
 */
void ddsi_deadline_register_instance_real(struct ddsi_deadline_adm* deadline_adm,
                                          struct deadline_elem* elem,
                                          ddsrt_mtime_t tprev,
                                          ddsrt_mtime_t tnow);

/**
 * @component deadline_qos
 * @brief 注销实例的真实处理函数 (Real handling function for unregistering instances)
 *
 * @param[in] deadline_adm 指向 ddsi_deadline_adm 结构体的指针 (Pointer to the ddsi_deadline_adm
 * structure)
 * @param[in] elem 指向 deadline_elem 结构体的指针 (Pointer to the deadline_elem structure)
 */
void ddsi_deadline_unregister_instance_real(struct ddsi_deadline_adm* deadline_adm,
                                            struct deadline_elem* elem);

/**
 * @component deadline_qos
 * @brief 更新实例的真实处理函数 (Real handling function for renewing instances)
 *
 * @param[in] deadline_adm 指向 ddsi_deadline_adm 结构体的指针 (Pointer to the ddsi_deadline_adm
 * structure)
 * @param[in] elem 指向 deadline_elem 结构体的指针 (Pointer to the deadline_elem structure)
 */
void ddsi_deadline_renew_instance_real(struct ddsi_deadline_adm* deadline_adm,
                                       struct deadline_elem* elem);

/**
 * @component deadline_qos
 * @brief 在锁定状态下注册实例 (Register instance in locked state)
 *
 * @param[in] deadline_adm 指向 ddsi_deadline_adm 结构体的指针 (Pointer to the ddsi_deadline_adm
 * structure)
 * @param[in] elem 指向 deadline_elem 结构体的指针 (Pointer to the deadline_elem structure)
 * @param[in] tnow 当前时间 (Current time)
 */
inline void ddsi_deadline_register_instance_locked(struct ddsi_deadline_adm* deadline_adm,
                                                   struct deadline_elem* elem,
                                                   ddsrt_mtime_t tnow) {
  // 如果持续时间不是无穷大 (If the duration is not infinity)
  if (deadline_adm->dur != DDS_INFINITY)
    // 调用真实处理函数 (Call the real handling function)
    ddsi_deadline_register_instance_real(deadline_adm, elem, tnow, tnow);
}

/**
 * @component deadline_qos
 * @brief 在锁定状态下重新注册实例 (Reregister instance in locked state)
 *
 * @param[in] deadline_adm 指向 ddsi_deadline_adm 结构体的指针 (Pointer to the ddsi_deadline_adm
 * structure)
 * @param[in] elem 指向 deadline_elem 结构体的指针 (Pointer to the deadline_elem structure)
 * @param[in] tnow 当前时间 (Current time)
 */
inline void ddsi_deadline_reregister_instance_locked(struct ddsi_deadline_adm* deadline_adm,
                                                     struct deadline_elem* elem,
                                                     ddsrt_mtime_t tnow) {
  // 如果持续时间不是无穷大 (If the duration is not infinity)
  if (deadline_adm->dur != DDS_INFINITY)
    // 调用真实处理函数 (Call the real handling function)
    ddsi_deadline_register_instance_real(deadline_adm, elem, elem->t_last_update, tnow);
}

/**
 * @component deadline_qos
 * @brief 在锁定状态下注销实例 (Unregister instance in locked state)
 *
 * @param[in] deadline_adm 指向 ddsi_deadline_adm 结构体的指针 (Pointer to the ddsi_deadline_adm
 * structure)
 * @param[in] elem 指向 deadline_elem 结构体的指针 (Pointer to the deadline_elem structure)
 */
inline void ddsi_deadline_unregister_instance_locked(struct ddsi_deadline_adm* deadline_adm,
                                                     struct deadline_elem* elem) {
  // 如果持续时间不是无穷大 (If the duration is not infinity)
  if (deadline_adm->dur != DDS_INFINITY) {
    // 断言截止时间不是永远 (Assert that the deadline is not never)
    assert(elem->t_deadline.v != DDS_NEVER);
    // 调用真实处理函数 (Call the real handling function)
    ddsi_deadline_unregister_instance_real(deadline_adm, elem);
  }
}

/**
 * @component deadline_qos
 * @brief 在锁定状态下更新实例 (Renew instance in locked state)
 *
 * @param[in] deadline_adm 指向 ddsi_deadline_adm 结构体的指针 (Pointer to the ddsi_deadline_adm
 * structure)
 * @param[in] elem 指向 deadline_elem 结构体的指针 (Pointer to the deadline_elem structure)
 */
inline void ddsi_deadline_renew_instance_locked(struct ddsi_deadline_adm* deadline_adm,
                                                struct deadline_elem* elem) {
  // 如果持续时间不是无穷大 (If the duration is not infinity)
  if (deadline_adm->dur != DDS_INFINITY) {
    // 断言截止时间不是永远 (Assert that the deadline is not never)
    assert(elem->t_deadline.v != DDS_NEVER);
    // 调用真实处理函数 (Call the real handling function)
    ddsi_deadline_renew_instance_real(deadline_adm, elem);
  }
}

#if defined(__cplusplus)
}
#endif

#endif /* DDSI_DEADLINE_H */
