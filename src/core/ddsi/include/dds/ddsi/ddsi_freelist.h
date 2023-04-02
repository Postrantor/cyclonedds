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
#ifndef DDSI_FREELIST_H
#define DDSI_FREELIST_H

#include "dds/ddsrt/atomics.h"
#include "dds/ddsrt/sync.h"

#if defined(__cplusplus)
extern "C" {
#endif

#define DDSI_FREELIST_NONE 1
#define DDSI_FREELIST_ATOMIC_LIFO 2
#define DDSI_FREELIST_DOUBLE 3

#define DDSI_FREELIST_TYPE DDSI_FREELIST_DOUBLE

#ifndef DDSI_FREELIST_TYPE
#if DDSRT_HAVE_ATOMIC_LIFO
#define DDSI_FREELIST_TYPE DDSI_FREELIST_ATOMIC_LIFO
#else
#define DDSI_FREELIST_TYPE DDSI_FREELIST_DOUBLE
#endif
#endif

#if DDSI_FREELIST_TYPE == DDSI_FREELIST_NONE

/**
 * @struct ddsi_freelist
 * @brief 一个简单的自由列表结构，用于管理内存块。 (A simple freelist structure for managing memory
 * blocks.)
 */
struct ddsi_freelist {
  char dummy; /**< 占位符，用于确保结构体不为空。 (Placeholder to ensure the structure is not
                 empty.) */
};

#elif DDSI_FREELIST_TYPE == DDSI_FREELIST_ATOMIC_LIFO

/**
 * @struct ddsi_freelist
 * @brief 一个简单的线程安全的自由列表结构，用于存储和回收固定大小的内存块。 (A simple thread-safe
 * free list structure for storing and recycling fixed-size memory blocks.)
 */
struct ddsi_freelist {
  ddsrt_atomic_lifo_t x;  ///< 原子操作的后进先出（LIFO）数据结构，用于存储空闲内存块。 (Atomic LIFO
                          ///< data structure for storing free memory blocks.)
  ddsrt_atomic_uint32_t count;  ///< 当前在自由列表中的内存块数量。 (The current number of memory
                                ///< blocks in the free list.)
  uint32_t max;  ///< 自由列表中允许的最大内存块数量。 (The maximum allowed number of memory blocks
                 ///< in the free list.)
  size_t linkoff;  ///< 内存块中链接到下一个内存块的偏移量。 (The offset in the memory block to link
                   ///< to the next memory block.)
};

#elif DDSI_FREELIST_TYPE == DDSI_FREELIST_DOUBLE

#define NN_FREELIST_NPAR 4
#define NN_FREELIST_NPAR_LG2 2
#define NN_FREELIST_MAGSIZE 256

/**
 * @struct ddsi_freelist_m
 * @brief 内存管理结构体，用于存储空闲内存块的信息 (Memory management structure for storing
 * information about free memory blocks)
 */
struct ddsi_freelist_m {
  void* x[NN_FREELIST_MAGSIZE]; /**< 存储空闲内存块指针的数组 (Array for storing pointers to free
                                   memory blocks) */
  void* next; /**< 指向下一个ddsi_freelist_m结构体的指针 (Pointer to the next ddsi_freelist_m
                 structure) */
};

/**
 * @struct ddsi_freelist1
 * @brief 单个并行链表中的内存管理结构体 (Memory management structure in a single parallel linked
 * list)
 */
struct ddsi_freelist1 {
  ddsrt_mutex_t lock; /**< 互斥锁，用于同步访问 (Mutex for synchronized access) */
  uint32_t
      count; /**< 当前链表中空闲内存块的数量 (Number of free memory blocks in the current list) */
  struct ddsi_freelist_m* m; /**< 指向ddsi_freelist_m结构体的指针，用于管理内存块 (Pointer to the
                                ddsi_freelist_m structure for managing memory blocks) */
};

/**
 * @struct ddsi_freelist
 * @brief 并行链表组成的内存管理结构体 (Memory management structure composed of parallel linked
 * lists)
 */
struct ddsi_freelist {
  struct ddsi_freelist1
      inner[NN_FREELIST_NPAR]; /**< 包含多个ddsi_freelist1结构体的数组 (Array containing multiple
                                  ddsi_freelist1 structures) */
  ddsrt_atomic_uint32_t cc; /**< 原子计数器，用于跟踪并行链表的访问次数 (Atomic counter for tracking
                               access to parallel linked lists) */
  ddsrt_mutex_t lock; /**< 互斥锁，用于同步访问mlist和emlist (Mutex for synchronized access to mlist
                         and emlist) */
  struct ddsi_freelist_m* mlist; /**< 指向ddsi_freelist_m结构体的指针，用于管理内存块 (Pointer to
                                    the ddsi_freelist_m structure for managing memory blocks) */
  struct ddsi_freelist_m*
      emlist; /**< 指向ddsi_freelist_m结构体的指针，用于管理额外的内存块 (Pointer to the
                 ddsi_freelist_m structure for managing additional memory blocks) */
  uint32_t count; /**< 总空闲内存块数量 (Total number of free memory blocks) */
  uint32_t max;   /**< 最大空闲内存块数量 (Maximum number of free memory blocks) */
  size_t linkoff; /**< 链表偏移量，用于定位下一个内存块 (Linked list offset for locating the next
                     memory block) */
};

#endif

/**
 * @component ddsi_freelist
 * @brief 初始化dds_freelist结构体
 * @param[in] fl 指向dds_freelist结构体的指针
 * @param[in] max freelist中允许的最大元素数量
 * @param[in] linkoff 链接偏移量
 */
// Initialize a ddsi_freelist structure
void ddsi_freelist_init(struct ddsi_freelist* fl, uint32_t max, size_t linkoff);

/**
 * @component ddsi_freelist
 * @brief 释放dds_freelist结构体
 * @param[in] fl 指向dds_freelist结构体的指针
 * @param[in] free 用于释放元素的函数指针
 */
// Finalize and release a ddsi_freelist structure
void ddsi_freelist_fini(struct ddsi_freelist* fl, void (*free)(void* elem));

/**
 * @component ddsi_freelist
 * @brief 将元素添加到freelist中
 * @param[in] fl 指向dds_freelist结构体的指针
 * @param[in] elem 要添加的元素的指针
 * @return 如果成功添加元素，返回true；否则返回false
 */
// Push an element onto the freelist
bool ddsi_freelist_push(struct ddsi_freelist* fl, void* elem);

/**
 * @component ddsi_freelist
 * @brief 将多个元素一次性添加到freelist中
 * @param[in] fl 指向dds_freelist结构体的指针
 * @param[in] first 要添加的第一个元素的指针
 * @param[in] last 要添加的最后一个元素的指针
 * @param[in] n 要添加的元素数量
 * @return 返回添加到freelist中的第一个元素的指针
 */
// Push multiple elements onto the freelist at once
void* ddsi_freelist_pushmany(struct ddsi_freelist* fl, void* first, void* last, uint32_t n);

/**
 * @component ddsi_freelist
 * @brief 从freelist中弹出一个元素
 * @param[in] fl 指向dds_freelist结构体的指针
 * @return 返回从freelist中弹出的元素的指针
 */
// Pop an element from the freelist
void* ddsi_freelist_pop(struct ddsi_freelist* fl);

#if defined(__cplusplus)
}
#endif

#endif /* DDSI_FREELIST_H */
