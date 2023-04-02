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
/**
 * @file threads.h
 * @brief Thread management and creation.
 *
 * Platform independent interface for managing and creating execution threads.
 */
#ifndef DDSRT_THREADS_H
#define DDSRT_THREADS_H

#include <stdbool.h>
#include <stdint.h>

#include "dds/config.h"
#include "dds/ddsrt/attributes.h"
#include "dds/ddsrt/retcode.h"
#include "dds/ddsrt/sched.h"
#include "dds/export.h"

#if DDSRT_WITH_FREERTOS
#include "dds/ddsrt/threads/freertos.h"
#elif _WIN32
#include "dds/ddsrt/threads/windows.h"
#else
#include "dds/ddsrt/threads/posix.h"
#endif

#if defined(__cplusplus)
extern "C" {
#endif

#if defined(_MSC_VER) || __MINGW__
/* 使用 __declspec(thread) 在 Windows 版本之前的线程局部存储
   Vista 和 Server 2008 在 DLL 中工作，如果它们绑定到可执行文件，
   它不适用于使用 LoadLibrary 加载的库。*/
/* Thread-local storage using __declspec(thread) on Windows versions before
   Vista and Server 2008 works in DLLs if they are bound to the executable,
   it does not work if the library is loaded using LoadLibrary. */
#define ddsrt_thread_local __declspec(thread)
#elif defined(__GNUC__) || (defined(__clang__) && __clang_major__ >= 2)
/* GCC 自版本 3.3 起支持 x86 的线程局部存储。Clang
   自版本 2.0 起支持线程局部存储。*/
/* GCC supports Thread-local storage for x86 since version 3.3. Clang
   supports Thread-local storage since version 2.0. */
/* VxWorks 7 支持 GCC 和 DIAB 的 __thread，较早的版本可能也支持，
   但尚未验证。*/
/* VxWorks 7 supports __thread for both GCC and DIAB, older versions may
   support it as well, but that is not verified. */
#define ddsrt_thread_local __thread
#elif defined(__SUNPRO_C) || defined(__SUNPRO_CC)
// 在 Solaris Studio 编译器中定义线程局部存储
// Define thread-local storage in Solaris Studio compiler
#define ddsrt_thread_local __thread
#else
// 如果不支持线程局部存储，则报错
// Error if thread-local storage is not supported
#error "Thread-local storage is not supported"
#endif

/**
 * @brief 定义线程创建时调用的线程例程。(Definition for a thread routine invoked on thread create.)
 */
typedef uint32_t (*ddsrt_thread_routine_t)(void *);

/**
 * @brief 线程属性定义。(Definition of the thread attributes)
 */
typedef struct {
  /** 指定调度类。(Specifies the scheduling class) */
  ddsrt_sched_t schedClass;
  /** 指定线程优先级。(Specifies the thread priority) */
  int32_t schedPriority;
  /** 指定线程堆栈大小。(Specifies the thread stack size) */
  uint32_t stackSize;
} ddsrt_threadattr_t;

/**
 * @brief 将线程属性初始化为平台默认值。(Initialize thread attributes to platform defaults.)
 *
 * @param[out] attr 要初始化的线程属性。(Thread attributes to be initialized.)
 */
DDS_EXPORT void ddsrt_threadattr_init(ddsrt_threadattr_t *attr) ddsrt_nonnull_all;

/**
 * @brief 创建一个新线程。(Create a new thread.)
 *
 * 创建一个与调用线程并发执行的控制线程。新线程应用函数 start_routine，并将 arg 作为第一个参数传递。
 * (Creates a new thread of control that executes concurrently with the calling thread. The new
 * thread applies the function start_routine passing it arg as first argument.)
 *
 * 新线程通过从 start_routine 函数返回来终止。创建的线程由返回的 threadId 标识。
 * (The new thread terminates by returning from the start_routine function. The created thread is
 * identified by the returned threadId.)
 *
 * @param[out]  thread         存储线程 ID 的位置。(Location where thread id is stored.)
 * @param[in]   name           分配给创建的线程的名称。(Name assigned to created thread.)
 * @param[in]   attr           用于创建线程的属性。(Attributes to create thread with.)
 * @param[in]   start_routine  在创建的线程中执行的函数。(Function to execute in created thread.)
 * @param[in]   arg            传递给 @start_routine 的参数。(Argument passed to @start_routine.)
 *
 * @returns 指示成功或失败的 dds_return_t。(A dds_return_t indicating success or failure.)
 *
 * @retval DDS_RETCODE_OK
 *             线程创建成功。(Thread successfully created.)
 * @retval DDS_RETCODE_ERROR
 *             无法创建线程。(Thread could not be created.)
 */
DDS_EXPORT dds_return_t ddsrt_thread_create(ddsrt_thread_t *thread,
                                            const char *name,
                                            const ddsrt_threadattr_t *attr,
                                            ddsrt_thread_routine_t start_routine,
                                            void *arg) ddsrt_nonnull((1, 2, 3, 4));

/**
 * @brief 获取给定线程ID的整数表示。 (Retrieve integer representation of the given thread id.)
 *
 * @returns 当前线程的整数表示。 (The integer representation of the current thread.)
 */
DDS_EXPORT ddsrt_tid_t ddsrt_gettid(void);

/**
 * @brief 获取给定线程ID的整数表示。 (Retrieve integer representation of the given thread id.)
 *
 * @param[in] thread 要获取整数表示的线程。 (The thread to get the integer representation for.)
 * @returns 给定线程的整数表示。 (The integer representation of the given thread.)
 */
DDS_EXPORT ddsrt_tid_t ddsrt_gettid_for_thread(ddsrt_thread_t thread);

/**
 * @brief 返回调用线程的线程ID。 (Return thread ID of the calling thread.)
 *
 * @returns 调用线程的线程ID。 (Thread ID of the calling thread.)
 */
DDS_EXPORT ddsrt_thread_t ddsrt_thread_self(void);

/**
 * @brief 比较线程标识符。 (Compare thread identifiers.)
 *
 * @param[in] t1 第一个要比较的线程ID。 (First thread id to compare.)
 * @param[in] t2 第二个要比较的线程ID。 (Second thread id to compare.)
 * @returns 如果线程ID匹配，则为true，否则为false。 (true if thread ids match, otherwise false.)
 */
DDS_EXPORT bool ddsrt_thread_equal(ddsrt_thread_t t1, ddsrt_thread_t t2);

/**
 * @brief 等待指定线程的终止。 (Wait for termination of the specified thread.)
 *
 * 如果指定的线程仍在运行，则等待其终止，否则立即返回。 (If the specified thread is still running,
 * wait for its termination else return immediately.) 在thread_result中返回线程的退出状态。 (In
 * thread_result it returns the exit status of the thread.)
 * 如果为@thread_result传递NULL，则不返回退出状态，但ddsrt_thread_join仍然等待线程终止。 (If NULL is
 * passed for @thread_result, no exit status is returned, but ddsrt_thread_join still waits for the
 * thread to terminate.)
 *
 * @param[in]   thread         要等待的线程ID。 (Id of thread to wait for.)
 * @param[out]  thread_result  存储线程结果的位置。 (Location where thread result is stored.)
 *
 * @returns 指示成功或失败的dds_return_t。 (A dds_return_t indicating success or failure.)
 *
 * @retval DDS_RETCODE_OK
 *             目标线程已终止。 (Target thread terminated.)
 * @retval DDS_RETCODE_ERROR
 *             等待线程终止时发生错误。 (An error occurred while waiting for the thread to
 * terminate.)
 */
DDS_EXPORT dds_return_t ddsrt_thread_join(ddsrt_thread_t thread, uint32_t *thread_result);

/**
 * @brief 获取当前线程的名称。 (Get name of current thread.)
 *
 * @param[in]  name  将名称复制到的缓冲区。 (Buffer where the name is copied to.)
 * @param[in]  size  缓冲区中可用的字节数。 (Number of bytes available in the buffer.)
 *
 * @returns
 * 写入的字节数（不包括空终止字节）。如果缓冲区不足够大，名称将被截断，并返回本应写入的字节数。 (The
 * number of bytes (excluding the null terminating bytes) that are written. If the buffer is not
 * sufficiently large enough, the name is truncated and the number of bytes that would have been
 *          written is returned.)
 */
DDS_EXPORT size_t ddsrt_thread_getname(char *__restrict name, size_t size);

/**
 * @brief 设置当前线程的名称。 (Set name of current thread.)
 *
 * 将当前线程的名称设置为 @name。如果名称长于平台最大值，则静默截断。
 * (Set name of the current thread to @name. If the name is longer than the
 * platform maximum, it is silently truncated.)
 *
 * @param[in]  name  当前线程的名称。 (Name for current thread.)
 */
#if DDSRT_HAVE_THREAD_SETNAME
DDS_EXPORT void ddsrt_thread_setname(const char *__restrict name);
#endif

#if DDSRT_HAVE_THREAD_LIST
/**
 * @brief 获取调用进程中的线程列表 (Get a list of threads in the calling process)
 *
 * @param[out]  tids    要填充线程标识符的大小为 size 的数组，如果 size 为 0，则可以为 NULL
 *                      (Array of size elements to be filled with thread
 *                      identifiers, may be NULL if size is 0)
 * @param[in]   size    tids 数组的大小；允许为 0
 *                      (The size of the tids array; 0 is allowed)
 *
 * @returns 一个 dds_return_t，表示进程中的线程数或失败时的错误代码。
 *          (A dds_return_t indicating the number of threads in the process
 *          or an error code on failure.)
 *
 * @retval > 0
 *             进程中的线程数，可能大于 size
 *             tids[0 .. (return - 1)] 是有效的
 *             (Number of threads in the process, may be larger than size
 *             tids[0 .. (return - 1)] are valid)
 * @retval DDS_RETCODE_ERROR
 *             出现错误，tids 的内容未定义
 *             (Something went wrong, contents of tids is undefined)
 * @retval DDS_RETCODE_UNSUPPORTED
 *             平台不支持
 *             (Not supported on the platform)
 */
DDS_EXPORT dds_return_t ddsrt_thread_list(ddsrt_thread_list_id_t *__restrict tids, size_t size);

/**
 * @brief 获取指定线程（在调用进程中）的名称 (Get the name of the specified thread (in the calling
 * process))
 *
 * @param[in]   tid     要查找名称的线程标识符 (Thread identifier for which the name is sought)
 * @param[out]  name
 * 成功返回时填充线程名（或合成的名称）；如果实际名称比name可以容纳的长度更长，则静默截断； 如果size
 * > 0，始终以0结尾 (Filled with the thread name (or a synthesized one) on successful return; name
 * is silently truncated if the actual name is longer than name can hold; always 0-terminated if
 * size > 0)
 * @param[in]   size    可分配给name的字节数，允许size为0，尽管有些无用 (Number of bytes of name
 * that may be assigned, size is 0 is allowed, though somewhat useless)
 *
 * @returns 表示成功或失败的dds_return_t。 (A dds_return_t indicating success or failure.)
 *
 * @retval DDS_RETCODE_OK
 *             可能截断的名称作为以空字符结尾的字符串返回到name中（提供size > 0）。(Possibly
 * truncated name is returned as a null-terminated string in name (provided size > 0).)
 * @retval DDS_RETCODE_NOT_FOUND
 *             未找到线程；name的内容保持不变 (Thread not found; the contents of name is unchanged)
 * @retval DDS_RETCODE_ERROR
 *             未指定的失败，name的内容未定义 (Unspecified failure, the contents of name is
 * undefined)
 * @retval DDS_RETCODE_UNSUPPORTED
 *             平台不支持 (Not supported on the platform)
 */
DDS_EXPORT dds_return_t ddsrt_thread_getname_anythread(ddsrt_thread_list_id_t tid,
                                                       char *__restrict name,
                                                       size_t size);
#endif

/**
 * @brief 将清理处理程序推送到清理堆栈上 (Push cleanup handler onto the cleanup stack)
 *
 * 将清理处理程序推送到调用线程的清理堆栈顶部。当线程退出时，将从线程的清理堆栈中弹出清理处理程序，并使用指定的参数调用它。
 * (Push a cleanup handler onto the top of the calling thread's cleanup
 * stack. The cleanup handler will be popped of the thread's cleanup stack
 * and invoked with the specified argument when the thread exits.)
 *
 * @param[in]  routine  要推送到线程清理堆栈上的清理处理程序。 (Cleanup handler to push onto the
 * thread cleanup stack.)
 * @param[in]  arg      将传递给清理处理程序的参数。 (Argument that will be passed to the cleanup
 * handler.)
 */
DDS_EXPORT dds_return_t ddsrt_thread_cleanup_push(void (*routine)(void *), void *arg);

/**
 * @brief 从清理堆栈顶部弹出清理处理程序 (Pop cleanup handler from the top of the cleanup stack)
 *
 * 从调用线程的清理堆栈顶部删除例程，并选择性地调用它（如果执行为非零）。(Remove routine at the top
 * of the calling thread's cleanup stack and optionally invoke it (if execute is non-zero).)
 */
DDS_EXPORT dds_return_t ddsrt_thread_cleanup_pop(int execute);

/**
 * @brief 初始化线程内部。 (Initialize thread internals.)
 *
 * 用于未使用@ddsrt_create_thread创建的线程初始化内部。默认情况下，初始化是自动完成的。
 * (Initialize internals for threads not created with @ddsrt_create_thread. By
 * default initialization is done automatically.)
 */
DDS_EXPORT void ddsrt_thread_init(uint32_t reason);

/**
 * @brief 释放线程资源并执行清理处理程序。 (Free thread resources and execute cleanup handlers.)
 *
 * 支持它的平台会自动释放当前线程所占用的资源，并调用任何已注册的清理例程。
 * (Platforms that support it, automatically free resources claimed by the current thread and call
 * any registered cleanup routines.) 仅需要在不支持线程析构函数的平台上调用此函数， (This function
 * only needs to be called on platforms that do not support thread destructors) 并且仅适用于未使用
 * @ddsrt_thread_create 创建的线程。 (and only for threads that were not created with
 * @ddsrt_thread_create.)
 *
 * @param[in] reason 原因代码，用于指示为什么要释放线程资源。 (Reason code indicating why the thread
 * resources are being freed.)
 */
DDS_EXPORT void ddsrt_thread_fini(uint32_t reason);

#if defined(__cplusplus)
}
#endif

#endif /* DDSRT_THREADS_H */
