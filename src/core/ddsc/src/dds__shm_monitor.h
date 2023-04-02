/*
 * Copyright(c) 2021 Apex.AI Inc. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v. 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0, or the Eclipse Distribution License
 * v. 1.0 which is available at
 * http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * SPDX-License-Identifier: EPL-2.0 OR BSD-3-Clause
 */
#ifndef DDS__SHM_MONITOR_H
#define DDS__SHM_MONITOR_H

#include "dds/ddsi/ddsi_shm_transport.h"
#include "dds/ddsrt/sync.h"
#include "dds/ddsrt/threads.h"
#include "iceoryx_binding_c/listener.h"
#include "iceoryx_binding_c/subscriber.h"

#if defined(__cplusplus)
extern "C" {
#endif

// ICEORYX_TODO: the iceoryx listener has a maximum number of subscribers that can be registered but
// this can only be queried at runtime 目前，iceoryx监听器在运行时只能查询到最大订阅者数量 currently
// it is hardcoded to be 128 events in the iceoryx C binding 当前，在iceoryx
// C绑定中硬编码为128个事件 and we need one registration slot for the wake up trigger
// 我们需要一个注册槽用于唤醒触发器
#define SHM_MAX_NUMBER_OF_READERS 127

// 定义dds_reader结构体
struct dds_reader;

// 定义shm_monitor结构体
struct shm_monitor;

// 定义共享内存监视器状态枚举
enum shm_monitor_states {
  // 共享内存监视器未运行状态
  SHM_MONITOR_NOT_RUNNING = 0,
  // 共享内存监视器运行状态
  SHM_MONITOR_RUNNING = 1
};

/// @brief 用于监控与负责响应通过共享内存接收到的数据的内部线程的共享内存通信的抽象
struct shm_monitor {
  ddsrt_mutex_t m_lock;       ///< @brief 互斥锁，用于保护共享资源
  iox_listener_t m_listener;  ///< @brief 监听器，用于监听共享内存中的数据

  /// @brief 用户触发器，用于在等待时唤醒（例如：终止操作）
  iox_user_trigger_t m_wakeup_trigger;

  uint32_t m_number_of_attached_readers;  ///< @brief 已连接的读取器数量
  uint32_t m_state;  ///< @brief 当前状态，用于表示共享内存监控器的状态
};

typedef struct shm_monitor shm_monitor_t;

/**
 * @brief 初始化共享内存监控器
 * @component shm_monitor
 *
 * @param monitor 自身实例指针
 */
void dds_shm_monitor_init(shm_monitor_t* monitor);

/**
 * @brief 删除共享内存监控器
 * @component shm_monitor
 *
 * @param monitor 自身实例指针
 */
void dds_shm_monitor_destroy(shm_monitor_t* monitor);

/**
 * @brief 唤醒内部监听器并禁用由于接收到的数据而执行的监听器回调
 * @component shm_monitor
 *
 * @param monitor 自身实例指针
 * @return dds_return_t 返回操作结果
 */
dds_return_t dds_shm_monitor_wake_and_disable(shm_monitor_t* monitor);

/**
 * @brief 唤醒内部监听器并启用由于接收到的数据而执行的监听器回调
 * @component shm_monitor
 *
 * @param monitor 自身实例指针
 * @return dds_return_t 返回操作结果
 */
dds_return_t dds_shm_monitor_wake_and_enable(shm_monitor_t* monitor);

/**
 * @brief 附加一个新的读取器
 * @component shm_monitor
 *
 * @param monitor 自身实例指针
 * @param reader 要附加的读取器
 * @return dds_return_t 返回操作结果
 */
dds_return_t dds_shm_monitor_attach_reader(shm_monitor_t* monitor, struct dds_reader* reader);

/**
 * @brief 分离一个读取器
 * @component shm_monitor
 *
 * @param monitor 自身实例指针
 * @param reader 要分离的读取器
 * @return dds_return_t 返回操作结果
 */
dds_return_t dds_shm_monitor_detach_reader(shm_monitor_t* monitor, struct dds_reader* reader);

// ICEORYX_TODO: 阐明读取器的生命周期，因为在 dds_reader_delete 调用中它们是分离的，所以应该没问题

#if defined(__cplusplus)
}
#endif

#endif /* DDS__SHM_MONITOR_H */
