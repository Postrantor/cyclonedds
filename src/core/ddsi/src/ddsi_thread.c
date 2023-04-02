/*
 * Copyright(c) 2006 to 2022 ZettaScale Technology and others
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v. 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0, or the Eclipse Distribution License
 * v. 1.0 which is available at
 * http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * SPDX-License-Identifier: EPL-2.0 OR BSD-3-Clause
 */
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "dds/ddsi/ddsi_domaingv.h"
#include "dds/ddsi/ddsi_log.h"
#include "dds/ddsi/ddsi_threadmon.h"
#include "dds/ddsrt/cdtors.h"
#include "dds/ddsrt/heap.h"
#include "dds/ddsrt/log.h"
#include "dds/ddsrt/misc.h"
#include "dds/ddsrt/string.h"
#include "dds/ddsrt/sync.h"
#include "dds/ddsrt/threads.h"
#include "ddsi__sysdeps.h"
#include "ddsi__thread.h"

/* 定义 ddsi_thread_states 结构体变量 thread_states
 * Define the ddsi_thread_states structure variable thread_states */
struct ddsi_thread_states thread_states;

/* 定义线程局部变量 tsd_thread_state，类型为 struct ddsi_thread_state *
 * Define thread-local variable tsd_thread_state of type struct ddsi_thread_state * */
ddsrt_thread_local struct ddsi_thread_state *tsd_thread_state;

/* 声明内联函数 ddsi_vtime_awake_p，判断 vtime 是否处于唤醒状态
 * Declare inline function ddsi_vtime_awake_p to determine if vtime is in awake state */
extern inline bool ddsi_vtime_awake_p(ddsi_vtime_t vtime);

/* 声明内联函数 ddsi_vtime_asleep_p，判断 vtime 是否处于休眠状态
 * Declare inline function ddsi_vtime_asleep_p to determine if vtime is in asleep state */
extern inline bool ddsi_vtime_asleep_p(ddsi_vtime_t vtime);

/* 声明内联函数 ddsi_vtime_gt，比较两个 vtime 的大小
 * Declare inline function ddsi_vtime_gt to compare the size of two vtimes */
extern inline bool ddsi_vtime_gt(ddsi_vtime_t vtime1, ddsi_vtime_t vtime0);

/* 声明内联函数 ddsi_lookup_thread_state，查找并返回当前线程的状态
 * Declare inline function ddsi_lookup_thread_state to find and return the current thread's state */
DDS_EXPORT extern inline struct ddsi_thread_state *ddsi_lookup_thread_state(void);

/* 声明内联函数 ddsi_thread_is_asleep，判断当前线程是否处于休眠状态
 * Declare inline function ddsi_thread_is_asleep to determine if the current thread is in asleep
 * state */
extern inline bool ddsi_thread_is_asleep(void);

/* 声明内联函数 ddsi_thread_is_awake，判断当前线程是否处于唤醒状态
 * Declare inline function ddsi_thread_is_awake to determine if the current thread is in awake state
 */
extern inline bool ddsi_thread_is_awake(void);

/* 声明内联函数 ddsi_thread_state_asleep，将指定线程状态设置为休眠状态
 * Declare inline function ddsi_thread_state_asleep to set the specified thread state to asleep
 * state */
extern inline void ddsi_thread_state_asleep(struct ddsi_thread_state *thrst);

/* 声明内联函数 ddsi_thread_state_awake，将指定线程状态设置为唤醒状态，并关联给定的 domaingv 结构体
 * Declare inline function ddsi_thread_state_awake to set the specified thread state to awake state
 * and associate it with the given domaingv structure */
extern inline void ddsi_thread_state_awake(struct ddsi_thread_state *thrst,
                                           const struct ddsi_domaingv *gv);

/* 声明内联函数 ddsi_thread_state_awake_domain_ok，将指定线程状态设置为唤醒状态（不检查域）
 * Declare inline function ddsi_thread_state_awake_domain_ok to set the specified thread state to
 * awake state (without checking domain) */
extern inline void ddsi_thread_state_awake_domain_ok(struct ddsi_thread_state *thrst);

/* 声明内联函数 ddsi_thread_state_awake_fixed_domain，将指定线程状态设置为唤醒状态（固定域）
 * Declare inline function ddsi_thread_state_awake_fixed_domain to set the specified thread state to
 * awake state (fixed domain) */
extern inline void ddsi_thread_state_awake_fixed_domain(struct ddsi_thread_state *thrst);

/* 声明内联函数
 * ddsi_thread_state_awake_to_awake_no_nest，将指定线程状态从唤醒状态切换到另一个唤醒状态（无嵌套）
 * Declare inline function ddsi_thread_state_awake_to_awake_no_nest to switch the specified thread
 * state from awake state to another awake state (no nesting) */
extern inline void ddsi_thread_state_awake_to_awake_no_nest(struct ddsi_thread_state *thrst);

/* 定义静态函数 init_thread_state，用于初始化线程状态
 * Define static function init_thread_state for initializing thread state */
static struct ddsi_thread_state *init_thread_state(const char *tname,
                                                   const struct ddsi_domaingv *gv,
                                                   enum ddsi_thread_state_kind state);

/* 定义静态函数 reap_thread_state，用于回收线程状态
 * Define static function reap_thread_state for reaping thread state */
static void reap_thread_state(struct ddsi_thread_state *thrst, bool in_ddsi_thread_states_fini);

/* 静态断言，检查 ddsi_thread_state 枚举值的顺序是否正确
 * Static assertion to check if the order of ddsi_thread_state enumeration values is correct */
DDSRT_STATIC_ASSERT(DDSI_THREAD_STATE_ZERO == 0 &&
                    DDSI_THREAD_STATE_ZERO < DDSI_THREAD_STATE_STOPPED &&
                    DDSI_THREAD_STATE_STOPPED < DDSI_THREAD_STATE_INIT &&
                    DDSI_THREAD_STATE_INIT < DDSI_THREAD_STATE_LAZILY_CREATED &&
                    DDSI_THREAD_STATE_INIT < DDSI_THREAD_STATE_ALIVE);

#if DDSI_THREAD_DEBUG
#include <execinfo.h>

/**
 * @brief 记录线程的虚拟时间 (Record the virtual time of a thread)
 *
 * @param[in] thrst 指向 ddsi_thread_state 结构体的指针 (Pointer to the ddsi_thread_state structure)
 */
void ddsi_thread_vtime_trace(struct ddsi_thread_state *thrst) {
  // 增加线程状态中的堆栈索引，并在达到最大值时将其重置为0
  // (Increment the stack index in the thread state and reset it to 0 when it reaches the maximum
  // value)
  if (++thrst->stks_idx == DDSI_THREAD_NSTACKS) thrst->stks_idx = 0;

  // 获取当前堆栈索引 (Get the current stack index)
  const int i = thrst->stks_idx;

  // 使用 backtrace 函数获取当前线程的调用堆栈信息，并将其存储在 thrst->stks[i] 中
  // (Use the backtrace function to get the call stack information of the current thread and store
  // it in thrst->stks[i])
  thrst->stks_depth[i] = backtrace(thrst->stks[i], DDSI_THREAD_STACKDEPTH);
}
#endif

/**
 * @brief 为指定大小的内存分配对齐到缓存行的空间。
 * Allocate memory of specified size aligned to a cache line.
 *
 * @param size 要分配的内存大小。The size of the memory to be allocated.
 * @return 返回一个指向已分配内存的指针。Returns a pointer to the allocated memory.
 */
static void *ddsrt_malloc_aligned_cacheline(size_t size) {
  // 这会浪费一些空间，但我们只使用它一次，而且它不是大量的内存，只是稍微超过了一个缓存行。
  // This wastes some space, but we use it only once and it isn't a huge amount of memory, just a
  // little over a cache line.
  const uintptr_t clm1 = DDSI_CACHE_LINE_SIZE - 1;
  uintptr_t ptrA;
  void **pptr;
  void *ptr;

  // 分配足够大的内存以容纳所需大小、缓存行大小和一个额外的void指针。
  // Allocate memory large enough to hold the required size, cache line size, and an additional void
  // pointer.
  ptr = ddsrt_malloc(size + DDSI_CACHE_LINE_SIZE + sizeof(void *));

  // 计算对齐到缓存行的地址。
  // Calculate the address aligned to the cache line.
  ptrA = ((uintptr_t)ptr + sizeof(void *) + clm1) & ~clm1;

  // 将原始指针存储在对齐地址之前的位置。
  // Store the original pointer just before the aligned address.
  pptr = (void **)ptrA;
  pptr[-1] = ptr;

  // 返回对齐的地址。
  // Return the aligned address.
  return (void *)ptrA;
}

/**
 * @brief 释放由ddsrt_malloc_aligned_cacheline分配的内存。
 * Free memory allocated by ddsrt_malloc_aligned_cacheline.
 *
 * @param ptr 指向要释放的内存的指针。Pointer to the memory to be freed.
 */
static void ddsrt_free_aligned(void *ptr) {
  if (ptr) {
    void **pptr = ptr;
    // 释放原始指针所指向的内存。
    // Free the memory pointed to by the original pointer.
    ddsrt_free(pptr[-1]);
  }
}

/**
 * @brief 初始化ddsi线程状态。
 * Initialize ddsi thread states.
 */
void ddsi_thread_states_init(void) {
  // 在持有ddsrt单例互斥锁的情况下调用（请参见dds_init/fini）。仍然活着的应用程序线程可能会导致thread_states保持活动状态，因为这些线程缓存了地址，我们必须重新使用旧数组。
  // Called with ddsrt's singleton mutex held (see dds_init/fini). Application threads remaining
  // alive can result in thread_states remaining alive, and as those thread cache the address, we
  // must then re-use the old array.
  if (ddsrt_atomic_ldvoidp(&thread_states.thread_states_head) == NULL) {
    struct ddsi_thread_states_list *tslist;
    ddsrt_mutex_init(&thread_states.lock);

    // 分配对齐到缓存行的内存。
    // Allocate memory aligned to a cache line.
    tslist = ddsrt_malloc_aligned_cacheline(sizeof(*tslist));
    tslist->next = NULL;
    tslist->nthreads = DDSI_THREAD_STATE_BATCH;

    // 将线程状态数组清零。
    // Zero the thread state array.
    memset(tslist->thrst, 0, sizeof(tslist->thrst));

    // 设置线程状态头指针。
    // Set the thread states head pointer.
    ddsrt_atomic_stvoidp(&thread_states.thread_states_head, tslist);
  }

  // 这个线程应该与以前的地址相同，或者从未在过去拥有一个插槽。此外，如果这个线程还没有分配一个插槽，那么为它分配一个插槽（严格来说不是必需的，但它最终会得到一个，而这使得它更加明确）。
  // This thread should be at the same address as before, or never have had a slot in the past.
  // Also, allocate a slot for this thread if it didn't have one yet (not strictly required, but
  // it'll get one eventually anyway, and this makes it rather more clear).
#ifndef NDEBUG
  struct ddsi_thread_state *const ts0 = tsd_thread_state;
#endif
  struct ddsi_thread_state *const thrst = ddsi_lookup_thread_state_real();

  // 断言当前线程状态为空或与之前的线程状态相同。
  // Assert that the current thread state is either NULL or the same as the previous thread state.
  assert(ts0 == NULL || ts0 == thrst);
  (void)thrst;
}

/**
 * @brief 结束并清理线程状态 (Finalize and clean up thread states)
 *
 * @return 如果所有其他线程已经停止并且资源被释放，则返回 true，否则返回 false (true if all other
 * threads have been stopped and resources have been released, false otherwise)
 */
bool ddsi_thread_states_fini(void) {
  // 当前调用线程是关闭所有操作的线程，因此它肯定不会（或者说不应该）再需要其线程状态了。
  // 清理它以便在所有其他线程都已经停止的情况下，我们可以释放所有资源。
  // (The calling thread is the one shutting everything down, so it certainly won't (well,
  // shouldn't) need its slot anymore. Clean it up so that if all other threads happen to have been
  // stopped already, we can release all resources.)
  struct ddsi_thread_state *thrst = ddsi_lookup_thread_state();
  assert(ddsi_vtime_asleep_p(ddsrt_atomic_ld32(&thrst->vtime)));
  reap_thread_state(thrst, true);
  tsd_thread_state = NULL;

  // 一些在某个时刻需要线程状态的应用程序线程可能仍然存在。
  // 对于这些线程，当线程终止时将调用清理例程。
  // 这应该重写为不依赖于这个全局变量，并且每个线程拥有自己的位状态，例如链接在一个列表中以便 GC
  // 访问它。 在那之前，如果仍然有用户，我们不能释放这些资源。 (Some applications threads that, at
  // some point, required a thread state, may still be around. Of those, the cleanup routine is
  // invoked when the thread terminates. This should be rewritten to not rely on this global thing
  // and with each thread owning its own bit state, e.g., linked together in a list to give the GC
  // access to it. Until then, we can't release these resources if there are still users.)
  uint32_t others = 0;
  ddsrt_mutex_lock(&thread_states.lock);
  for (struct ddsi_thread_states_list *cur =
           ddsrt_atomic_ldvoidp(&thread_states.thread_states_head);
       cur; cur = cur->next) {
    for (uint32_t i = 0; i < DDSI_THREAD_STATE_BATCH; i++) {
      switch (cur->thrst[i].state) {
        case DDSI_THREAD_STATE_ZERO:
          break;
        case DDSI_THREAD_STATE_LAZILY_CREATED:
          others++;
          break;
        case DDSI_THREAD_STATE_STOPPED:
        case DDSI_THREAD_STATE_INIT:
        case DDSI_THREAD_STATE_ALIVE:
          assert(0);
      }
    }
  }
  ddsrt_mutex_unlock(&thread_states.lock);
  if (others == 0) {
    // 没有其他活动线程，无需担心原子性
    // (no other threads active, no need to worry about atomicity)
    ddsrt_mutex_destroy(&thread_states.lock);
    struct ddsi_thread_states_list *head = ddsrt_atomic_ldvoidp(&thread_states.thread_states_head);
    ddsrt_atomic_stvoidp(&thread_states.thread_states_head, NULL);
    while (head) {
      struct ddsi_thread_states_list *next = head->next;
      ddsrt_free_aligned(head);
      head = next;
    }
    return true;
  } else {
    return false;
  }
}

/**
 * @brief 查找指定线程ID的线程状态 (Find the thread state for a given thread ID)
 *
 * @param[in] tid 要查找的线程ID (The thread ID to search for)
 * @return 指向找到的线程状态的指针，如果未找到则返回NULL (Pointer to the found thread state, or
 * NULL if not found)
 */
static struct ddsi_thread_state *find_thread_state(ddsrt_thread_t tid) {
  // 如果线程状态链表不为空 (If the thread states list is not empty)
  if (ddsrt_atomic_ldvoidp(&thread_states.thread_states_head)) {
    // 加锁以保护线程状态链表 (Lock to protect the thread states list)
    ddsrt_mutex_lock(&thread_states.lock);
    // 遍历线程状态链表 (Iterate through the thread states list)
    for (struct ddsi_thread_states_list *cur =
             ddsrt_atomic_ldvoidp(&thread_states.thread_states_head);
         cur; cur = cur->next) {
      // 遍历当前线程状态列表中的每个线程状态 (Iterate through each thread state in the current
      // list)
      for (uint32_t i = 0; i < DDSI_THREAD_STATE_BATCH; i++) {
        // 如果找到匹配的线程ID (If a matching thread ID is found)
        if (cur->thrst[i].state > DDSI_THREAD_STATE_INIT &&
            ddsrt_thread_equal(cur->thrst[i].tid, tid)) {
          // 解锁并返回找到的线程状态 (Unlock and return the found thread state)
          ddsrt_mutex_unlock(&thread_states.lock);
          return &cur->thrst[i];
        }
      }
    }
    // 解锁线程状态链表 (Unlock the thread states list)
    ddsrt_mutex_unlock(&thread_states.lock);
  }
  // 如果未找到线程状态，返回NULL (Return NULL if no thread state was found)
  return NULL;
}

/**
 * @brief 清理线程状态 (Clean up the thread state)
 *
 * @param[in] data 传递给清理函数的数据 (Data passed to the cleanup function)
 */
static void cleanup_thread_state(void *data) {
  // 查找当前线程的线程状态 (Find the thread state for the current thread)
  struct ddsi_thread_state *thrst = find_thread_state(ddsrt_thread_self());
  (void)data;
  // 如果找到线程状态 (If a thread state is found)
  if (thrst) {
    // 检查线程状态是否正确 (Check that the thread state is correct)
    assert(thrst->state == DDSI_THREAD_STATE_LAZILY_CREATED);
    assert(ddsi_vtime_asleep_p(ddsrt_atomic_ld32(&thrst->vtime)));
    // 回收线程状态 (Reap the thread state)
    reap_thread_state(thrst, false);
  }
  // 结束ddsrt (Finalize ddsrt)
  ddsrt_fini();
}

/**
 * @brief 懒惰地创建线程状态 (Lazily create the thread state)
 *
 * @param[in] self 当前线程ID (The current thread ID)
 * @return 指向新创建的线程状态的指针 (Pointer to the newly created thread state)
 */
static struct ddsi_thread_state *lazy_create_thread_state(ddsrt_thread_t self) {
  // 此情况仅适用于未使用create_thread创建的线程，即应用程序线程 (This situation only arises for
  // threads not created using create_thread, aka application threads)
  struct ddsi_thread_state *thrst;
  char name[128];
  // 获取线程名称 (Get the thread name)
  ddsrt_thread_getname(name, sizeof(name));
  // 加锁以保护线程状态链表 (Lock to protect the thread states list)
  ddsrt_mutex_lock(&thread_states.lock);
  // 初始化线程状态 (Initialize the thread state)
  if ((thrst = init_thread_state(name, NULL, DDSI_THREAD_STATE_LAZILY_CREATED)) != NULL) {
    // 初始化ddsrt (Initialize ddsrt)
    ddsrt_init();
    // 设置线程ID (Set the thread ID)
    thrst->tid = self;
    // 记录应用程序线程启动日志 (Log the start of the application thread)
    DDS_LOG(DDS_LC_TRACE, "started application thread %s\n", name);
    // 注册清理函数 (Register the cleanup function)
    ddsrt_thread_cleanup_push(&cleanup_thread_state, NULL);
  }
  // 解锁线程状态链表 (Unlock the thread states list)
  ddsrt_mutex_unlock(&thread_states.lock);
  // 返回新创建的线程状态 (Return the newly created thread state)
  return thrst;
}

/**
 * @brief 查找当前线程的 ddsi_thread_state 结构体实例。
 * @details 如果当前线程没有关联的 ddsi_thread_state 实例，将会创建一个新的实例并返回。
 * @return 返回当前线程关联的 ddsi_thread_state 结构体指针。
 *
 * @brief Look up the ddsi_thread_state structure instance for the current thread.
 * @details If the current thread does not have an associated ddsi_thread_state instance, a new
 * instance will be created and returned.
 * @return Returns a pointer to the ddsi_thread_state structure associated with the current thread.
 */
struct ddsi_thread_state *ddsi_lookup_thread_state_real(void) {
  // 获取当前线程关联的 ddsi_thread_state 实例
  // Get the associated ddsi_thread_state instance for the current thread
  struct ddsi_thread_state *thrst = tsd_thread_state;

  // 如果当前线程没有关联的 ddsi_thread_state 实例
  // If the current thread does not have an associated ddsi_thread_state instance
  if (thrst == NULL) {
    // 获取当前线程的标识符
    // Get the identifier of the current thread
    ddsrt_thread_t self = ddsrt_thread_self();

    // 查找与当前线程关联的 ddsi_thread_state 实例，如果不存在则创建一个新的实例
    // Find the ddsi_thread_state instance associated with the current thread, create a new one if
    // it doesn't exist
    if ((thrst = find_thread_state(self)) == NULL) thrst = lazy_create_thread_state(self);

    // 将新创建的 ddsi_thread_state 实例与当前线程关联
    // Associate the newly created ddsi_thread_state instance with the current thread
    tsd_thread_state = thrst;
  }

  // 确保找到了与当前线程关联的 ddsi_thread_state 实例
  // Ensure that the associated ddsi_thread_state instance for the current thread is found
  assert(thrst != NULL);

  // 返回找到的 ddsi_thread_state 实例
  // Return the found ddsi_thread_state instance
  return thrst;
}

/**
 * @brief 创建并启动一个新线程，该线程将执行给定的函数。
 * @param ptr 指向 ddsi_thread_state 结构体实例的指针。
 * @return 返回执行给定函数后的返回值。
 *
 * @brief Create and start a new thread that will execute the given function.
 * @param ptr A pointer to the ddsi_thread_state structure instance.
 * @return Returns the return value after executing the given function.
 */
static uint32_t create_thread_wrapper(void *ptr) {
  // 将传入的参数转换为 ddsi_thread_state 结构体指针
  // Convert the passed argument to a ddsi_thread_state structure pointer
  struct ddsi_thread_state *const thrst = ptr;

  // 获取与当前线程关联的 ddsi_domaingv 实例
  // Get the associated ddsi_domaingv instance for the current thread
  struct ddsi_domaingv const *const gv = ddsrt_atomic_ldvoidp(&thrst->gv);

  // 如果存在关联的 ddsi_domaingv 实例，则记录线程启动信息
  // If there is an associated ddsi_domaingv instance, log the thread start information
  if (gv) GVTRACE("started new thread %" PRIdTID ": %s\n", ddsrt_gettid(), thrst->name);

  // 确保当前线程状态为初始化状态
  // Ensure that the current thread state is in the initialization state
  assert(thrst->state == DDSI_THREAD_STATE_INIT);

  // 将当前线程关联的 ddsi_thread_state 实例设置为传入的实例
  // Set the associated ddsi_thread_state instance for the current thread to the passed instance
  tsd_thread_state = thrst;

  // 加锁以确保线程状态的同步
  // Lock to ensure synchronization of thread states
  ddsrt_mutex_lock(&thread_states.lock);

  // 设置当前线程状态为活动状态
  // Set the current thread state to active state
  thrst->state = DDSI_THREAD_STATE_ALIVE;

  // 解锁以释放线程状态的同步
  // Unlock to release synchronization of thread states
  ddsrt_mutex_unlock(&thread_states.lock);

  // 执行给定的函数并获取返回值
  // Execute the given function and get the return value
  const uint32_t ret = thrst->f(thrst->f_arg);

  // 加锁以确保线程状态的同步
  // Lock to ensure synchronization of thread states
  ddsrt_mutex_lock(&thread_states.lock);

  // 设置当前线程状态为停止状态
  // Set the current thread state to stopped state
  thrst->state = DDSI_THREAD_STATE_STOPPED;

  // 解锁以释放线程状态的同步
  // Unlock to release synchronization of thread states
  ddsrt_mutex_unlock(&thread_states.lock);

  // 将当前线程关联的 ddsi_thread_state 实例设置为 NULL
  // Set the associated ddsi_thread_state instance for the current thread to NULL
  tsd_thread_state = NULL;

  // 返回执行给定函数后的返回值
  // Return the return value after executing the given function
  return ret;
}

/**
 * @brief 根据给定的线程名称查找与之关联的 ddsi_config_thread_properties_listelem 结构体实例。
 * @param config 指向 ddsi_config 结构体实例的指针。
 * @param name 要查找的线程名称。
 * @return 返回找到的 ddsi_config_thread_properties_listelem 结构体实例，如果没有找到则返回 NULL。
 *
 * @brief Look up the associated ddsi_config_thread_properties_listelem structure instance based on
 * the given thread name.
 * @param config A pointer to the ddsi_config structure instance.
 * @param name The thread name to look up.
 * @return Returns the found ddsi_config_thread_properties_listelem structure instance, or NULL if
 * not found.
 */
const struct ddsi_config_thread_properties_listelem *ddsi_lookup_thread_properties(
    const struct ddsi_config *config, const char *name) {
  const struct ddsi_config_thread_properties_listelem *e;

  // 遍历配置中的线程属性列表，查找与给定线程名称匹配的属性实例
  // Traverse the thread properties list in the configuration to find the property instance that
  // matches the given thread name
  for (e = config->thread_properties; e != NULL; e = e->next)
    if (strcmp(e->name, name) == 0) break;

  // 返回找到的 ddsi_config_thread_properties_listelem 结构体实例，如果没有找到则返回 NULL
  // Return the found ddsi_config_thread_properties_listelem structure instance, or NULL if not
  // found
  return e;
}

/**
 * @brief 增加线程状态列表的容量。
 * @details 为线程状态列表分配新的内存空间，并将其添加到列表中。
 * @return 返回一个可用的 ddsi_thread_state 结构体指针，如果分配失败则返回 NULL。
 *
 * @brief Increase the capacity of the thread states list.
 * @details Allocate new memory space for the thread states list and add it to the list.
 * @return Returns an available ddsi_thread_state structure pointer, or NULL if allocation fails.
 */
static struct ddsi_thread_state *grow_thread_states(void) {
  struct ddsi_thread_states_list *x;

  // 为线程状态列表分配新的内存空间
  // Allocate new memory space for the thread states list
  if ((x = ddsrt_malloc_aligned_cacheline(sizeof(*x))) == NULL) return NULL;

  // 将新分配的内存空间初始化为 0
  // Initialize the newly allocated memory space to 0
  memset(x->thrst, 0, sizeof(x->thrst));

  // 将新分配的内存空间添加到线程状态列表中
  // Add the newly allocated memory space to the thread states list
  do {
    x->next = ddsrt_atomic_ldvoidp(&thread_states.thread_states_head);
    x->nthreads = DDSI_THREAD_STATE_BATCH + x->next->nthreads;
  } while (!ddsrt_atomic_casvoidp(&thread_states.thread_states_head, x->next, x));
  return &x->thrst[0];
}

/**
 * @brief 获取一个可用的线程槽 (Get an available thread slot)
 *
 * @return 返回一个可用的线程状态指针，如果没有找到则返回NULL并增长线程状态列表 (Return a pointer to
 * an available thread state, or NULL and grow the thread states list if not found)
 */
static struct ddsi_thread_state *get_available_thread_slot() {
  // 声明一个指向ddsi_thread_states_list结构体的指针cur (Declare a pointer to a
  // ddsi_thread_states_list structure named cur)
  struct ddsi_thread_states_list *cur;
  // 声明一个32位无符号整数i (Declare a 32-bit unsigned integer i)
  uint32_t i;
  // 遍历线程状态列表，查找一个可用的线程槽 (Traverse the thread states list to find an available
  // thread slot)
  for (cur = ddsrt_atomic_ldvoidp(&thread_states.thread_states_head); cur; cur = cur->next)
    for (i = 0; i < DDSI_THREAD_STATE_BATCH; i++)
      if (cur->thrst[i].state == DDSI_THREAD_STATE_ZERO) return &cur->thrst[i];
  // 如果没有找到可用的线程槽，则增长线程状态列表 (If no available thread slot is found, grow the
  // thread states list)
  return grow_thread_states();
}

/**
 * @brief 初始化线程状态 (Initialize thread state)
 *
 * @param[in] tname 线程名 (Thread name)
 * @param[in] gv 指向ddsi_domaingv结构体的指针 (Pointer to a ddsi_domaingv structure)
 * @param[in] state 线程状态 (Thread state)
 * @return 返回一个初始化后的线程状态指针，如果没有可用的线程槽则返回NULL (Return a pointer to an
 * initialized thread state, or NULL if no available thread slot)
 */
static struct ddsi_thread_state *init_thread_state(const char *tname,
                                                   const struct ddsi_domaingv *gv,
                                                   enum ddsi_thread_state_kind state) {
  // 获取一个可用的线程槽 (Get an available thread slot)
  struct ddsi_thread_state *const thrst = get_available_thread_slot();
  // 如果没有找到可用的线程槽，则返回NULL (If no available thread slot is found, return NULL)
  if (thrst == NULL) return thrst;

  // 断言线程虚拟时间处于睡眠状态 (Assert that the thread virtual time is in sleep state)
  assert(ddsi_vtime_asleep_p(ddsrt_atomic_ld32(&thrst->vtime)));
  // 设置线程状态的ddsi_domaingv字段 (Set the ddsi_domaingv field of the thread state)
  ddsrt_atomic_stvoidp(&thrst->gv, (struct ddsi_domaingv *)gv);
  // 复制线程名到线程状态的name字段 (Copy the thread name to the name field of the thread state)
  (void)ddsrt_strlcpy(thrst->name, tname, sizeof(thrst->name));
  // 设置线程状态的state字段 (Set the state field of the thread state)
  thrst->state = state;
  // 返回初始化后的线程状态指针 (Return the pointer to the initialized thread state)
  return thrst;
}

/**
 * @brief 创建线程的内部实现 (Internal implementation of creating a thread)
 *
 * @param[out] ts1_out 返回创建的线程状态指针 (Return the created thread state pointer)
 * @param[in] gv 指向ddsi_domaingv结构体的指针 (Pointer to a ddsi_domaingv structure)
 * @param[in] tprops 线程属性 (Thread properties)
 * @param[in] name 线程名 (Thread name)
 * @param[in] f 线程函数指针 (Thread function pointer)
 * @param[in] arg 线程函数参数 (Thread function argument)
 * @return 返回创建线程的结果，成功返回DDS_RETCODE_OK，失败返回DDS_RETCODE_ERROR (Return the result
 * of creating the thread, success returns DDS_RETCODE_OK, failure returns DDS_RETCODE_ERROR)
 */
static dds_return_t create_thread_int(
    struct ddsi_thread_state **ts1_out,
    const struct ddsi_domaingv *gv,
    struct ddsi_config_thread_properties_listelem const *const tprops,
    const char *name,
    uint32_t (*f)(void *arg),
    void *arg) {
  // 声明一个ddsrt_threadattr_t结构体变量tattr (Declare a ddsrt_threadattr_t structure variable
  // named tattr)
  ddsrt_threadattr_t tattr;
  // 声明一个指向ddsi_thread_state结构体的指针thrst (Declare a pointer to a ddsi_thread_state
  // structure named thrst)
  struct ddsi_thread_state *thrst;
  // 锁定线程状态列表 (Lock the thread states list)
  ddsrt_mutex_lock(&thread_states.lock);

  // 初始化线程状态 (Initialize the thread state)
  thrst = *ts1_out = init_thread_state(name, gv, DDSI_THREAD_STATE_INIT);
  // 如果初始化失败，则跳转到fatal标签处理 (If initialization fails, jump to the fatal label for
  // processing)
  if (thrst == NULL) goto fatal;

  // 设置线程状态的函数指针和参数 (Set the function pointer and argument of the thread state)
  thrst->f = f;
  thrst->f_arg = arg;
  // 初始化线程属性 (Initialize the thread attributes)
  ddsrt_threadattr_init(&tattr);
  // 如果提供了线程属性，则设置线程属性值 (If thread properties are provided, set the thread
  // attribute values)
  if (tprops != NULL) {
    if (!tprops->schedule_priority.isdefault) tattr.schedPriority = tprops->schedule_priority.value;
    tattr.schedClass = tprops->sched_class; /* explicit default value in the enum */
    if (!tprops->stack_size.isdefault) tattr.stackSize = tprops->stack_size.value;
  }
  // 如果提供了ddsi_domaingv结构体指针，则打印线程创建信息 (If a ddsi_domaingv structure pointer is
  // provided, print the thread creation information)
  if (gv) {
    GVTRACE("create_thread: %s: class %d priority %" PRId32 " stack %" PRIu32 "\n", name,
            (int)tattr.schedClass, tattr.schedPriority, tattr.stackSize);
  }

  // 创建线程，如果失败则跳转到fatal标签处理 (Create the thread, if it fails, jump to the fatal
  // label for processing)
  if (ddsrt_thread_create(&thrst->tid, name, &tattr, &create_thread_wrapper, thrst) !=
      DDS_RETCODE_OK) {
    thrst->state = DDSI_THREAD_STATE_ZERO;
    DDS_FATAL("create_thread: %s: ddsrt_thread_create failed\n", name);
    goto fatal;
  }
  // 解锁线程状态列表并返回成功 (Unlock the thread states list and return success)
  ddsrt_mutex_unlock(&thread_states.lock);
  return DDS_RETCODE_OK;
fatal:
  // 解锁线程状态列表，设置输出参数为NULL，并终止程序 (Unlock the thread states list, set the output
  // parameter to NULL, and abort the program)
  ddsrt_mutex_unlock(&thread_states.lock);
  *ts1_out = NULL;
  abort();
  return DDS_RETCODE_ERROR;
}

/**
 * @brief 创建一个新线程并设置其属性
 *        Create a new thread and set its properties
 *
 * @param[out] thrst 指向新创建的线程状态的指针的指针
 *                  Pointer to the pointer of the newly created thread state
 * @param[in] tprops 线程属性列表元素的指针，用于设置新线程的属性
 *                   Pointer to the thread properties list element, used to set the properties of
 * the new thread
 * @param[in] name 新线程的名称
 *                 Name of the new thread
 * @param[in] f 新线程要执行的函数指针
 *              Function pointer that the new thread will execute
 * @param[in] arg 传递给新线程的参数
 *                Argument passed to the new thread
 * @return 成功时返回 DDS_RETCODE_OK，否则返回错误代码
 *         Returns DDS_RETCODE_OK on success, otherwise returns an error code
 */
dds_return_t ddsi_create_thread_with_properties(
    struct ddsi_thread_state **thrst,
    struct ddsi_config_thread_properties_listelem const *const tprops,
    const char *name,
    uint32_t (*f)(void *arg),
    void *arg) {
  return create_thread_int(thrst, NULL, tprops, name, f, arg);
}

/**
 * @brief 创建一个新线程
 *        Create a new thread
 *
 * @param[out] thrst 指向新创建的线程状态的指针的指针
 *                  Pointer to the pointer of the newly created thread state
 * @param[in] gv 域全局变量的指针
 *               Pointer to the domain global variables
 * @param[in] name 新线程的名称
 *                 Name of the new thread
 * @param[in] f 新线程要执行的函数指针
 *              Function pointer that the new thread will execute
 * @param[in] arg 传递给新线程的参数
 *                Argument passed to the new thread
 * @return 成功时返回 DDS_RETCODE_OK，否则返回错误代码
 *         Returns DDS_RETCODE_OK on success, otherwise returns an error code
 */
dds_return_t ddsi_create_thread(struct ddsi_thread_state **thrst,
                                const struct ddsi_domaingv *gv,
                                const char *name,
                                uint32_t (*f)(void *arg),
                                void *arg) {
  struct ddsi_config_thread_properties_listelem const *const tprops =
      ddsi_lookup_thread_properties(&gv->config, name);
  return create_thread_int(thrst, gv, tprops, name, f, arg);
}

/**
 * @brief 回收线程状态
 *        Reap thread state
 *
 * @param[in] thrst 要回收的线程状态的指针
 *                  Pointer to the thread state to be reaped
 * @param[in] in_ddsi_thread_states_fini 标识是否在 ddsi_thread_states_fini 函数中调用此函数
 *                                       Indicates whether this function is called within the
 * ddsi_thread_states_fini function
 */
static void reap_thread_state(struct ddsi_thread_state *thrst, bool in_ddsi_thread_states_fini) {
  ddsrt_mutex_lock(&thread_states.lock);
  switch (thrst->state) {
    case DDSI_THREAD_STATE_INIT:
    case DDSI_THREAD_STATE_STOPPED:
    case DDSI_THREAD_STATE_LAZILY_CREATED:
      thrst->state = DDSI_THREAD_STATE_ZERO;
      break;
    case DDSI_THREAD_STATE_ZERO:
      // 尝试两次回收已死亡的线程并不是一件好事，而且通常不会发生。
      // 在 Windows 上，当一个 C++ 进程在离开 main 时仅保持 guard conditions 和 waitsets 存活时，
      // 可能会在全局析构函数中删除它们。在 Windows 上，这些全局析构函数很奇怪：它们在所有其他线程被
      // Windows 杀死之后以及调用线程的线程终止例程之后运行。
      //
      // 这意味着 ddsi_thread_states_fini() 可能看不到自己的线程仍然存活。
      // 这也意味着您不能依赖全局析构函数来关闭 Cyclone 在 Windows 上。
      // Trying to reap a deceased thread twice is not a good thing and it
      // doesn't normally happen. On Windows, however, a C++ process that
      // has only guard conditions and waitsets alive when it leaves main
      // may end up deleting those in a global destructor. Those global
      // destructors on Windows are weird: they run after all other threads
      // have been killed by Windows and after the thread finalization
      // routine for the calling thread has been called.
      //
      // That means ddsi_thread_states_fini() may not see its own thread as
      // alive anymore. It also means that you cannot ever rely on global
      // destructors to shut down Cyclone in Windows.
#ifdef _WIN32
      assert(in_ddsi_thread_states_fini);
#else
      assert(0);
#endif
      (void)in_ddsi_thread_states_fini;
      break;
    case DDSI_THREAD_STATE_ALIVE:
      assert(0);
  }
  ddsrt_mutex_unlock(&thread_states.lock);
}

/**
 * @brief 加入线程并等待其结束 (Join a thread and wait for it to finish)
 *
 * @param[in] thrst 指向 ddsi_thread_state 结构体的指针 (Pointer to the ddsi_thread_state structure)
 * @return 成功时返回 DDS_RETCODE_OK，失败时返回错误代码 (Returns DDS_RETCODE_OK on success, error
 * code on failure)
 */
dds_return_t ddsi_join_thread(struct ddsi_thread_state *thrst) {
  dds_return_t ret;  // 定义返回值变量 (Define return value variable)

  // 对 thread_states.lock 上锁 (Lock the thread_states.lock)
  ddsrt_mutex_lock(&thread_states.lock);

  // 根据 thrst 的状态进行处理 (Process according to the state of thrst)
  switch (thrst->state) {
    case DDSI_THREAD_STATE_INIT:
    case DDSI_THREAD_STATE_STOPPED:
    case DDSI_THREAD_STATE_ALIVE:
      break;
    case DDSI_THREAD_STATE_ZERO:
    case DDSI_THREAD_STATE_LAZILY_CREATED:
      assert(0);  // 断言，检查状态是否有效 (Assert, check if the state is valid)
  }

  // 解锁 thread_states.lock (Unlock the thread_states.lock)
  ddsrt_mutex_unlock(&thread_states.lock);

  // 等待线程结束 (Wait for the thread to finish)
  ret = ddsrt_thread_join(thrst->tid, NULL);

  // 断言，检查虚拟时间是否处于睡眠状态 (Assert, check if virtual time is in sleep state)
  assert(ddsi_vtime_asleep_p(ddsrt_atomic_ld32(&thrst->vtime)));

  // 回收线程状态资源 (Reap thread state resources)
  reap_thread_state(thrst, false);

  return ret;  // 返回结果 (Return the result)
}

/**
 * @brief 记录所有线程的堆栈跟踪信息 (Log stack traces of all threads)
 *
 * @param[in] logcfg 指向 ddsrt_log_cfg 结构体的指针，用于配置日志记录 (Pointer to the ddsrt_log_cfg
 * structure for configuring log recording)
 * @param[in] gv 指向 ddsi_domaingv 结构体的指针，用于筛选特定域的线程 (Pointer to the ddsi_domaingv
 * structure for filtering threads in a specific domain)
 */
void ddsi_log_stack_traces(const struct ddsrt_log_cfg *logcfg, const struct ddsi_domaingv *gv) {
  // 遍历线程状态列表 (Traverse the thread states list)
  for (struct ddsi_thread_states_list *cur =
           ddsrt_atomic_ldvoidp(&thread_states.thread_states_head);
       cur; cur = cur->next) {
    // 遍历每个批次中的线程状态 (Traverse the thread states in each batch)
    for (uint32_t i = 0; i < DDSI_THREAD_STATE_BATCH; i++) {
      struct ddsi_thread_state *const thrst = &cur->thrst[i];
      // 如果线程状态大于 DDSI_THREAD_STATE_INIT 且所属域与 gv 匹配，则记录堆栈跟踪信息 (If the
      // thread state is greater than DDSI_THREAD_STATE_INIT and the domain matches gv, log the
      // stack trace)
      if (thrst->state > DDSI_THREAD_STATE_INIT &&
          (gv == NULL || ddsrt_atomic_ldvoidp(&thrst->gv) == gv)) {
        /* 存在一个竞态条件，可能导致我们使用无效的线程 ID 调用 log_stacktrace
           （甚至可能是与新创建的线程映射的线程 ID，在这种情况下，它实际上并不相关！）
           但这是一个可选的调试功能，因此没有必要避免它。 */
        /* There's a race condition here that may cause us to call log_stacktrace with an invalid
           thread id (or even with a thread id mapping to a newly created thread that isn't really
           relevant in this context!) but this is an optional debug feature, so it's not worth the
           bother to avoid it. */
        ddsi_log_stacktrace(logcfg, thrst->name, thrst->tid);
      }
    }
  }
}
