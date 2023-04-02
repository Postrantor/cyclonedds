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
#ifndef DDSI_GC_H
#define DDSI_GC_H

#include "dds/export.h"

#if defined(__cplusplus)
extern "C" {
#endif

struct ddsi_gcreq;
struct ddsi_gcreq_queue;

// 定义一个函数指针类型，用于回调函数
// Define a function pointer type for callback functions
typedef void (*ddsi_gcreq_cb_t)(struct ddsi_gcreq* gcreq);

/**
 * @brief 创建一个新的ddsi_gcreq对象
 * @param[in] gcreq_queue 与垃圾收集请求关联的队列
 * @param[in] cb 回调函数，用于处理垃圾收集请求
 * @return 返回一个新创建的ddsi_gcreq对象
 *
 * @brief Create a new ddsi_gcreq object
 * @param[in] gcreq_queue The queue associated with the garbage collection request
 * @param[in] cb Callback function to handle the garbage collection request
 * @return A newly created ddsi_gcreq object
 */
DDS_EXPORT struct ddsi_gcreq* ddsi_gcreq_new(struct ddsi_gcreq_queue* gcreq_queue,
                                             ddsi_gcreq_cb_t cb);

/**
 * @brief 释放一个ddsi_gcreq对象
 * @param[in] gcreq 要释放的ddsi_gcreq对象
 *
 * @brief Free a ddsi_gcreq object
 * @param[in] gcreq The ddsi_gcreq object to be freed
 */
DDS_EXPORT void ddsi_gcreq_free(struct ddsi_gcreq* gcreq);

/**
 * @brief 将ddsi_gcreq对象加入到队列中
 * @param[in] gcreq 要加入队列的ddsi_gcreq对象
 *
 * @brief Enqueue a ddsi_gcreq object into the queue
 * @param[in] gcreq The ddsi_gcreq object to be enqueued
 */
DDS_EXPORT void ddsi_gcreq_enqueue(struct ddsi_gcreq* gcreq);

/**
 * @brief 获取ddsi_gcreq对象的参数
 * @param[in] gcreq 要获取参数的ddsi_gcreq对象
 * @return 返回ddsi_gcreq对象的参数
 *
 * @brief Get the argument of a ddsi_gcreq object
 * @param[in] gcreq The ddsi_gcreq object to get the argument from
 * @return The argument of the ddsi_gcreq object
 */
DDS_EXPORT void* ddsi_gcreq_get_arg(struct ddsi_gcreq* gcreq);

/**
 * @brief 设置ddsi_gcreq对象的参数
 * @param[in] gcreq 要设置参数的ddsi_gcreq对象
 * @param[in] arg 要设置的参数值
 *
 * @brief Set the argument of a ddsi_gcreq object
 * @param[in] gcreq The ddsi_gcreq object to set the argument for
 * @param[in] arg The argument value to set
 */
DDS_EXPORT void ddsi_gcreq_set_arg(struct ddsi_gcreq* gcreq, void* arg);

#if defined(__cplusplus)
}
#endif

#endif /* DDSI_GC_H */
