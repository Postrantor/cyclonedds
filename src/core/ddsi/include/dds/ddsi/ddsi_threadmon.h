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
#ifndef DDSI_THREADMON_H
#define DDSI_THREADMON_H

#if defined(__cplusplus)
extern "C" {
#endif

struct ddsi_threadmon;
struct ddsi_domaingv;

/**
 * @brief 创建一个新的线程监视器对象 (Create a new thread monitor object)
 * @param liveliness_monitoring_interval 活跃度监控间隔，以纳秒为单位 (Liveliness monitoring
 * interval in nanoseconds)
 * @param noprogress_log_stacktraces 如果为 true，则在没有进展时记录堆栈跟踪 (If true, log stack
 * traces when no progress is made)
 * @return 返回创建的线程监视器对象指针 (Returns a pointer to the created thread monitor object)
 */
struct ddsi_threadmon* ddsi_threadmon_new(int64_t liveliness_monitoring_interval,
                                          bool noprogress_log_stacktraces);

/**
 * @brief 启动线程监视器 (Start the thread monitor)
 * @param sl 线程监视器对象指针 (Pointer to the thread monitor object)
 * @param name 线程名称 (Thread name)
 * @return 返回操作结果，成功返回 DDS_RETCODE_OK (Returns the operation result, success returns
 * DDS_RETCODE_OK)
 */
dds_return_t ddsi_threadmon_start(struct ddsi_threadmon* sl, const char* name);

/**
 * @brief 注册域到线程监视器 (Register domain to the thread monitor)
 * @param sl 线程监视器对象指针 (Pointer to the thread monitor object)
 * @param gv 域全局变量结构体指针 (Pointer to the domain global variables structure)
 */
void ddsi_threadmon_register_domain(struct ddsi_threadmon* sl, const struct ddsi_domaingv* gv);

/**
 * @brief 从线程监视器中注销域 (Unregister domain from the thread monitor)
 * @param sl 线程监视器对象指针 (Pointer to the thread monitor object)
 * @param gv 域全局变量结构体指针 (Pointer to the domain global variables structure)
 */
void ddsi_threadmon_unregister_domain(struct ddsi_threadmon* sl, const struct ddsi_domaingv* gv);

/**
 * @brief 停止线程监视器 (Stop the thread monitor)
 * @param sl 线程监视器对象指针 (Pointer to the thread monitor object)
 */
void ddsi_threadmon_stop(struct ddsi_threadmon* sl);

/**
 * @brief 释放线程监视器对象 (Free the thread monitor object)
 * @param sl 线程监视器对象指针 (Pointer to the thread monitor object)
 */
void ddsi_threadmon_free(struct ddsi_threadmon* sl);

#if defined(__cplusplus)
}
#endif

#endif /* DDSI_THREADMON_H */
