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
#ifndef DDSI_THREAD_H
#define DDSI_THREAD_H

#include <assert.h>

#include "dds/ddsrt/atomics.h"
#include "dds/ddsrt/static_assert.h"
#include "dds/ddsrt/sync.h"
#include "dds/ddsrt/threads.h"
#include "dds/export.h"

#if defined(__cplusplus)
extern "C" {
#endif

struct ddsi_domaingv;
struct ddsi_thread_state;

/* 如果 DDSI_CACHE_LINE_SIZE 定义错误，事情不会出错，
   但是由于错误的缓存行共享，它们的运行速度会变慢。可以在运行时发现，
   但实际上对于大多数 CPU 来说是 64，对于某些 CPU 是 128。 */
/* Things don't go wrong if DDSI_CACHE_LINE_SIZE is defined incorrectly,
   they just run slower because of false cache-line sharing. It can be
   discovered at run-time, but in practice it's 64 for most CPUs and
   128 for some. */
#define DDSI_CACHE_LINE_SIZE 64

typedef uint32_t ddsi_vtime_t;
#define DDSI_VTIME_NEST_MASK 0xfu
#define DDSI_VTIME_TIME_MASK 0xfffffff0u
#define DDSI_VTIME_TIME_SHIFT 4

// ddsi_thread_state_kind 枚举定义了线程状态的类型
// The ddsi_thread_state_kind enum defines the types of thread states
enum ddsi_thread_state_kind {
  DDSI_THREAD_STATE_ZERO,           /* 已知为死亡状态 */
                                    /* known to be dead */
  DDSI_THREAD_STATE_STOPPED,        /* 内部线程，已停止但未回收 */
                                    /* internal thread, stopped-but-not-reaped */
  DDSI_THREAD_STATE_INIT,           /* 内部线程，正在初始化 */
                                    /* internal thread, initializing */
  DDSI_THREAD_STATE_LAZILY_CREATED, /* 应用程序使用它时懒惰地创建。
                                       如果线程终止，则回收，但如果在此线程尚未终止时关闭所有
                                       Cyclone，则不视为错误 */
                                    /* lazily created in response because an application used it.
                                       Reclaimed if the thread terminates, but not considered an
                                       error if all of Cyclone is shutdown while this thread hasn't
                                       terminated yet */
  DDSI_THREAD_STATE_ALIVE           /* 已知为活动状态 - 用于 Cyclone 内部线程 */
                                    /* known to be alive - for Cyclone internal threads */
};

/*
 * vtime 表示垃圾回收器和活跃度监控的进度。
 * (vtime indicates progress for the garbage collector and the liveliness monitoring.)
 *
 * vtime 在不使用原子操作的情况下更新：只有拥有线程更新它们，垃圾回收机制和活跃度监控只观察值
 * (vtime is updated without using atomic operations: only the owning thread updates them, and the
 * garbage collection mechanism and the liveliness monitoring only observe the value)
 *
 * 对于内部线程（即状态为 ALIVE 的线程），gv 是常量
 * (gv is constant for internal threads, i.e., for threads with state = ALIVE)
 * 除了线程活跃度监控之外，对于内部线程，gv 是非空的
 * (gv is non-NULL for internal threads except thread liveliness monitoring)
 *
 * DDSI_THREAD_DEBUG 启用一些可能不完全可移植的非常昂贵的调试内容（我曾经使用过它，也可以保留它）
 * (DDSI_THREAD_DEBUG enables some really costly debugging stuff that may not be fully portable (I
 * used it once, might as well keep it))
 */
#define DDSI_THREAD_DEBUG 0
#if DDSI_THREAD_DEBUG
#define DDSI_THREAD_NSTACKS 20
#define DDSI_THREAD_STACKDEPTH 10
// 定义线程基本调试信息结构
// (Define thread base debug information structure)
#define THREAD_BASE_DEBUG                                  \
  void *stks[DDSI_THREAD_NSTACKS][DDSI_THREAD_STACKDEPTH]; \
  int stks_depth[DDSI_THREAD_NSTACKS];                     \
  int stks_idx;

/** @component thread_support */
// 声明 ddsi_thread_vtime_trace 函数
// (Declare the ddsi_thread_vtime_trace function)
void ddsi_thread_vtime_trace(struct ddsi_thread_state *thrst);
#else /* DDSI_THREAD_DEBUG */
#define THREAD_BASE_DEBUG
// 如果未启用 DDSI_THREAD_DEBUG，将 ddsi_thread_vtime_trace 定义为空操作
// (If DDSI_THREAD_DEBUG is not enabled, define ddsi_thread_vtime_trace as a no-op)
#define ddsi_thread_vtime_trace(thrst) \
  do {                                 \
  } while (0)
#endif /* DDSI_THREAD_DEBUG */

// 定义线程基本结构
// (Define thread base structure)
#define THREAD_BASE                                                     \
  ddsrt_atomic_uint32_t vtime;                                          \
  enum ddsi_thread_state_kind state;                                    \
  ddsrt_atomic_voidp_t gv;                                              \
  ddsrt_thread_t tid;                                                   \
  uint32_t (*f)(void *arg);                                             \
  void *f_arg;                                                          \
  THREAD_BASE_DEBUG /* 注意：没有分号！(note: no semicolon!) */ \
      char name[24] /* 注意：没有分号！(note: no semicolon!) */

// 定义 ddsi_thread_state_base 结构
// (Define the ddsi_thread_state_base structure)
struct ddsi_thread_state_base {
  THREAD_BASE;
};

/* GCC具有一个很好的功能，允许指定所需的对齐方式: __attribute__ ((aligned (DDSI_CACHE_LINE_SIZE)))
   在这种情况下。 许多其他编译器也实现了它，但它绝不是一个标准功能。所以我们用老式的方法来做。 */
/* GCC has a nifty feature allowing the specification of the required
   alignment: __attribute__ ((aligned (DDSI_CACHE_LINE_SIZE))) in this
   case. Many other compilers implement it as well, but it is by no
   means a standard feature.  So we do it the old-fashioned way. */

// 定义 ddsi_thread_state 结构体
// Define the ddsi_thread_state structure
struct ddsi_thread_state {
  THREAD_BASE;
  char pad[DDSI_CACHE_LINE_SIZE *
               ((sizeof(struct ddsi_thread_state_base) + DDSI_CACHE_LINE_SIZE - 1) /
                DDSI_CACHE_LINE_SIZE) -
           sizeof(struct ddsi_thread_state_base)];
};
#undef THREAD_BASE

// 定义 ddsi_thread_states 结构体
// Define the ddsi_thread_states structure
struct ddsi_thread_states {
  ddsrt_mutex_t lock;                       // 互斥锁
  ddsrt_atomic_voidp_t thread_states_head;  // 原子操作的线程状态头指针
};

extern struct ddsi_thread_states thread_states;

// Windows上的thread_local不能（也不需要？）被导出
// thread_local cannot (and doesn't need to?) be exported on Windows
#if defined _WIN32 && !defined __MINGW32__
extern ddsrt_thread_local struct ddsi_thread_state *tsd_thread_state;
#else
DDS_EXPORT extern ddsrt_thread_local struct ddsi_thread_state *tsd_thread_state;
#endif

/** @component thread_support */
// 初始化线程状态
// Initialize thread states
void ddsi_thread_states_init(void);

/** @component thread_support */
// 结束线程状态
// Finalize thread states
bool ddsi_thread_states_fini(void);

/** @component thread_support */
/**
 * @brief 创建一个新的线程
 * @param [out] thrst 指向新创建的线程状态的指针
 * @param [in] gv 域全局变量的指针
 * @param [in] name 线程名称
 * @param [in] f 线程函数指针
 * @param [in] arg 传递给线程函数的参数
 * @return 成功时返回 DDS_RETCODE_OK，失败时返回错误代码
 */
/**
 * @brief Create a new thread
 * @param [out] thrst Pointer to the newly created thread state
 * @param [in] gv Pointer to the domain global variables
 * @param [in] name Thread name
 * @param [in] f Thread function pointer
 * @param [in] arg Argument passed to the thread function
 * @return Returns DDS_RETCODE_OK on success, error code on failure
 */
dds_return_t ddsi_create_thread(struct ddsi_thread_state **thrst,
                                const struct ddsi_domaingv *gv,
                                const char *name,
                                uint32_t (*f)(void *arg),
                                void *arg);

/** @component thread_support */

/**
 * @brief 加入线程 (Join a thread)
 *
 * @param[in] thrst 指向 ddsi_thread_state 结构的指针 (Pointer to a ddsi_thread_state structure)
 * @return dds_return_t 返回操作结果 (Return operation result)
 */
dds_return_t ddsi_join_thread(struct ddsi_thread_state *thrst);

/** @component thread_support */

/**
 * @brief 查找实际线程状态 (Look up real thread state)
 *
 * @return struct ddsi_thread_state* 返回指向 ddsi_thread_state 结构的指针 (Return pointer to a
 * ddsi_thread_state structure)
 */
DDS_EXPORT struct ddsi_thread_state *ddsi_lookup_thread_state_real(void);

/** @component thread_support */

/**
 * @brief 查找线程状态 (Look up thread state)
 *
 * @return struct ddsi_thread_state* 返回指向 ddsi_thread_state 结构的指针 (Return pointer to a
 * ddsi_thread_state structure)
 */
DDS_INLINE_EXPORT inline struct ddsi_thread_state *ddsi_lookup_thread_state(void) {
  // 获取线程状态 (Get thread state)
  struct ddsi_thread_state *thrst = tsd_thread_state;
  // 如果线程状态存在，则返回线程状态，否则查找实际线程状态 (If the thread state exists, return the
  // thread state, otherwise look up the real thread state)
  if (thrst)
    return thrst;
  else
    return ddsi_lookup_thread_state_real();
}

/** @component thread_support */

/**
 * @brief 判断虚拟时间是否唤醒 (Determine if virtual time is awake)
 *
 * @param[in] vtime 虚拟时间 (Virtual time)
 * @return bool 返回唤醒状态 (Return awake status)
 */
inline bool ddsi_vtime_awake_p(ddsi_vtime_t vtime) { return (vtime & DDSI_VTIME_NEST_MASK) != 0; }

/** @component thread_support */

/**
 * @brief 判断虚拟时间是否睡眠 (Determine if virtual time is asleep)
 *
 * @param[in] vtime 虚拟时间 (Virtual time)
 * @return bool 返回睡眠状态 (Return asleep status)
 */
inline bool ddsi_vtime_asleep_p(ddsi_vtime_t vtime) { return (vtime & DDSI_VTIME_NEST_MASK) == 0; }

/** @component thread_support */

/**
 * @brief 判断线程是否唤醒 (Determine if the thread is awake)
 *
 * @return bool 返回唤醒状态 (Return awake status)
 */
inline bool ddsi_thread_is_awake(void) {
  // 查找线程状态 (Look up thread state)
  struct ddsi_thread_state *thrst = ddsi_lookup_thread_state();
  // 加载虚拟时间 (Load virtual time)
  ddsi_vtime_t vt = ddsrt_atomic_ld32(&thrst->vtime);
  // 判断虚拟时间是否唤醒 (Determine if virtual time is awake)
  return ddsi_vtime_awake_p(vt);
}

/** @component thread_support */
/**
 * @brief 检查当前线程是否处于休眠状态 (Check if the current thread is in a sleep state)
 *
 * @return 如果线程处于休眠状态，则返回 true，否则返回 false (Returns true if the thread is in a
 * sleep state, false otherwise)
 */
inline bool ddsi_thread_is_asleep(void) {
  // 查找当前线程的状态 (Lookup the current thread's state)
  struct ddsi_thread_state *thrst = ddsi_lookup_thread_state();
  // 加载当前线程的虚拟时间 (Load the current thread's virtual time)
  ddsi_vtime_t vt = ddsrt_atomic_ld32(&thrst->vtime);
  // 判断当前线程是否处于休眠状态 (Determine if the current thread is in a sleep state)
  return ddsi_vtime_asleep_p(vt);
}

/** @component thread_support */
/**
 * @brief 将线程置于休眠状态 (Put the thread into a sleep state)
 *
 * @param[in] thrst 线程状态指针 (Pointer to the thread state)
 */
inline void ddsi_thread_state_asleep(struct ddsi_thread_state *thrst) {
  // 加载线程的虚拟时间 (Load the thread's virtual time)
  ddsi_vtime_t vt = ddsrt_atomic_ld32(&thrst->vtime);
  // 断言线程处于唤醒状态 (Assert that the thread is in an awake state)
  assert(ddsi_vtime_awake_p(vt));
  // 添加内存屏障以确保正确的内存顺序 (Add a memory barrier to ensure correct memory ordering)
  ddsrt_atomic_fence_rel();
  // 跟踪线程的虚拟时间 (Trace the thread's virtual time)
  ddsi_thread_vtime_trace(thrst);
  if ((vt & DDSI_VTIME_NEST_MASK) == 1)
    vt += (1u << DDSI_VTIME_TIME_SHIFT) - 1u;
  else
    vt -= 1u;
  // 更新线程的虚拟时间 (Update the thread's virtual time)
  ddsrt_atomic_st32(&thrst->vtime, vt);
}

/** @component thread_support */
/**
 * @brief 将线程从休眠状态唤醒 (Wake up the thread from a sleep state)
 *
 * @param[in] thrst 线程状态指针 (Pointer to the thread state)
 * @param[in] gv 域全局变量指针 (Pointer to the domain global variables)
 */
inline void ddsi_thread_state_awake(struct ddsi_thread_state *thrst,
                                    const struct ddsi_domaingv *gv) {
  // 加载线程的虚拟时间 (Load the thread's virtual time)
  ddsi_vtime_t vt = ddsrt_atomic_ld32(&thrst->vtime);
  // 断言嵌套计数小于最大值 (Assert that the nesting count is less than the maximum value)
  assert((vt & DDSI_VTIME_NEST_MASK) < DDSI_VTIME_NEST_MASK);
  // 断言 gv 不为 NULL (Assert that gv is not NULL)
  assert(gv != NULL);
  // 断言线程状态为 ALIVE 或 gv 与原子加载的 gv 相等 (Assert that the thread state is ALIVE or gv is
  // equal to the atomically loaded gv)
  assert(thrst->state != DDSI_THREAD_STATE_ALIVE || gv == ddsrt_atomic_ldvoidp(&thrst->gv));
  // 跟踪线程的虚拟时间 (Trace the thread's virtual time)
  ddsi_thread_vtime_trace(thrst);
  // 更新线程的域全局变量指针 (Update the thread's domain global variables pointer)
  ddsrt_atomic_stvoidp(&thrst->gv, (struct ddsi_domaingv *)gv);
  // 添加内存屏障以确保正确的内存顺序 (Add a memory barrier to ensure correct memory ordering)
  ddsrt_atomic_fence_stst();
  // 更新线程的虚拟时间 (Update the thread's virtual time)
  ddsrt_atomic_st32(&thrst->vtime, vt + 1u);
  // 添加内存屏障以确保正确的内存顺序 (Add a memory barrier to ensure correct memory ordering)
  ddsrt_atomic_fence_acq();
}

#if defined(__cplusplus)
}
#endif

#endif /* DDSI_THREAD_H */
