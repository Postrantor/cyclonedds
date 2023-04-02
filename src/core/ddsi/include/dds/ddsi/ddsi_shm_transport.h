/*
 * Copyright(c) 2021 ZettaScale Technology and others
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v. 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0, or the Eclipse Distribution License
 * v. 1.0 which is available at
 * http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * SPDX-License-Identifier: EPL-2.0 OR BSD-3-Clause
 */
#ifndef DDS_SHM__TRANSPORT_H
#define DDS_SHM__TRANSPORT_H

#include "dds/ddsi/ddsi_config.h"
#include "dds/ddsi/ddsi_keyhash.h"
#include "dds/ddsi/ddsi_protocol.h" /* for, e.g., ddsi_rtps_submessage_kind_t */
#include "dds/ddsi/ddsi_tran.h"
#include "dds/ddsrt/sync.h"
#include "dds/export.h"
#include "dds/features.h"
#include "iceoryx_binding_c/chunk.h"
#include "iceoryx_binding_c/config.h"
#include "iceoryx_binding_c/publisher.h"
#include "iceoryx_binding_c/subscriber.h"

#if defined(__cplusplus)
extern "C" {
#endif

// 定义一个枚举类型，表示共享内存中的数据状态（Define an enumeration type to represent the data
// state in shared memory）
typedef enum {
  IOX_CHUNK_UNINITIALIZED,      // 数据块未初始化（Data chunk uninitialized）
  IOX_CHUNK_CONTAINS_RAW_DATA,  // 数据块包含原始数据（Data chunk contains raw data）
  IOX_CHUNK_CONTAINS_SERIALIZED_DATA  // 数据块包含序列化数据（Data chunk contains serialized data）
} iox_shm_data_state_t;

// 定义一个结构体，表示 Iceoryx 的头部信息（Define a structure representing the header information
// of Iceoryx）
struct iceoryx_header {
  struct ddsi_guid guid;                // 全局唯一标识符（Global Unique Identifier）
  dds_time_t tstamp;                    // 时间戳（Timestamp）
  uint32_t statusinfo;                  // 状态信息（Status information）
  uint32_t data_size;                   // 数据大小（Data size）
  unsigned char data_kind;              // 数据类型（Data kind）
  ddsi_keyhash_t keyhash;               // 键哈希（Key hash）
  iox_shm_data_state_t shm_data_state;  // 共享内存数据状态（Shared memory data state）
};

// 为 iceoryx_header 结构体定义一个类型别名（Define a type alias for the iceoryx_header structure）
typedef struct iceoryx_header iceoryx_header_t;

// 前向声明一个 dds_reader 结构体（Forward declaration of a dds_reader structure）
struct dds_reader;

// 前向声明一个 shm_monitor 结构体（Forward declaration of a shm_monitor structure）
struct shm_monitor;

// 定义一个结构体，表示 Iceoryx 订阅者上下文（Define a structure representing the Iceoryx subscriber
// context）
typedef struct {
  ddsrt_mutex_t mutex;  // 互斥锁，用于同步访问（Mutex for synchronized access）
  struct shm_monitor* monitor;  // 指向共享内存监视器的指针（Pointer to shared memory monitor）
  struct dds_reader* parent_reader;  // 指向父读取器的指针（Pointer to parent reader）
} iox_sub_context_t;

/** @component iceoryx_support */
// 获取订阅者上下文指针 (Get the subscriber context pointer)
iox_sub_context_t** iox_sub_context_ptr(iox_sub_t sub);

/** @component iceoryx_support */
// 初始化订阅者上下文 (Initialize the subscriber context)
void iox_sub_context_init(iox_sub_context_t* context);

/** @component iceoryx_support */
// 销毁订阅者上下文 (Finalize the subscriber context)
void iox_sub_context_fini(iox_sub_context_t* context);

/**
 * @brief 为单个订阅者加锁和解锁 (Lock and unlock for individual subscribers)
 * @component iceoryx_support
 */
void shm_lock_iox_sub(iox_sub_t sub);

/** @component iceoryx_support */
// 为单个订阅者解锁 (Unlock for individual subscribers)
void shm_unlock_iox_sub(iox_sub_t sub);

/** @component iceoryx_support */
// 释放 Iceoryx chunk (Free the Iceoryx chunk)
DDS_EXPORT void free_iox_chunk(iox_sub_t* iox_sub, void** iox_chunk);

/** @component iceoryx_support */
// 从 Iceoryx chunk 中获取 header (Get the Iceoryx header from the chunk)
DDS_EXPORT iceoryx_header_t* iceoryx_header_from_chunk(const void* iox_chunk);

/** @component iceoryx_support */
// 设置共享内存日志级别 (Set the shared memory log level)
void shm_set_loglevel(enum ddsi_shm_loglevel);

/** @component iceoryx_support */
// 创建一个共享内存块 (Create a shared memory chunk)
void* shm_create_chunk(iox_pub_t iox_pub, size_t size);

/** @component iceoryx_support */
// 设置共享内存块的数据状态 (Set the data state of the shared memory chunk)
DDS_EXPORT void shm_set_data_state(void* iox_chunk, iox_shm_data_state_t data_state);

/** @component iceoryx_support */
// 获取共享内存块的数据状态 (Get the data state of the shared memory chunk)
iox_shm_data_state_t shm_get_data_state(void* iox_chunk);

#if defined(__cplusplus)
}
#endif

#endif  // DDS_SHM__TRANSPORT_H
