/*
 * Copyright(c) 2006 to 2019 ZettaScale Technology and others
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v. 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0, or the Eclipse Distribution License
 * v. 1.0 which is available at
 * http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * SPDX-License-Identifier: EPL-2.0 OR BSD-3-Clause
 */
#ifndef DDSRT_SYNC_H
#define DDSRT_SYNC_H

#include <stdbool.h>

#include "dds/ddsrt/attributes.h"
#include "dds/ddsrt/retcode.h"
#include "dds/ddsrt/time.h"

#if DDSRT_WITH_FREERTOS
#include "dds/ddsrt/sync/freertos.h"
#elif _WIN32
#include "dds/ddsrt/sync/windows.h"
#elif __SunOS_5_6
#include "dds/ddsrt/sync/solaris2.6.h"
#else
#include "dds/ddsrt/sync/posix.h"
#endif

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @brief 初始化一个互斥锁。 (Initialize a mutex.)
 *
 * @param[in]  mutex  要初始化的互斥锁。 (Mutex to initialize.)
 */
DDS_EXPORT void ddsrt_mutex_init(ddsrt_mutex_t* mutex) ddsrt_nonnull_all;

/**
 * @brief 销毁一个互斥锁。 (Destroy a mutex.)
 *
 * @param[in]  mutex  要销毁的互斥锁。 (Mutex to destroy.)
 */
DDS_EXPORT void ddsrt_mutex_destroy(ddsrt_mutex_t* mutex) ddsrt_nonnull_all;

/**
 * @brief 获取一个互斥锁。 (Acquire a mutex.)
 *
 * @param[in]  mutex  要获取的互斥锁。 (Mutex to acquire.)
 */
DDS_EXPORT void ddsrt_mutex_lock(ddsrt_mutex_t* mutex) ddsrt_nonnull_all;

/**
 * @brief 如果尚未获取，则获取一个互斥锁。 (Acquire a mutex if it is not already acquired.)
 *
 * @param[in]  mutex  要获取的互斥锁。 (Mutex to acquire.)
 *
 * @returns 如果成功获取了互斥锁，则返回 true；否则返回 false。 (true if the mutex was acquired,
 * false otherwise.)
 */
DDS_EXPORT bool ddsrt_mutex_trylock(ddsrt_mutex_t* mutex)
    ddsrt_nonnull_all ddsrt_attribute_warn_unused_result;

/**
 * @brief 释放已获取的互斥锁。 (Release an acquired mutex.)
 *
 * @param[in]  mutex  要释放的互斥锁。 (Mutex to release.)
 */
DDS_EXPORT void ddsrt_mutex_unlock(ddsrt_mutex_t* mutex) ddsrt_nonnull_all;

/**
 * @brief 初始化一个条件变量。 (Initialize a condition variable.)
 *
 * @param[in]  cond  要初始化的条件变量。 (Condition variable to initialize.)
 */
DDS_EXPORT void ddsrt_cond_init(ddsrt_cond_t* cond) ddsrt_nonnull_all;

/**
 * @brief 销毁条件变量 (Destroy a condition variable)
 *
 * @param[in]  cond  要销毁的条件变量 (Condition variable to destroy)
 */
DDS_EXPORT void ddsrt_cond_destroy(ddsrt_cond_t* cond) ddsrt_nonnull_all;

/**
 * @brief 等待条件变量被触发 (Wait for a condition variable to be signalled)
 *
 * @param[in]  cond   需要阻塞的条件变量 (Condition variable to block on)
 * @param[in]  mutex  与条件变量关联的互斥锁 (Mutex to associate with condition variable)
 *
 * @pre 调用线程必须持有由 @mutex 指定的互斥锁 (The calling thread must hold the mutex specified by
 * @mutex)
 *
 * @post 调用线程将持有由 @mutex 指定的互斥锁 (The calling thread will hold the mutex specified by
 * @mutex)
 */
DDS_EXPORT void ddsrt_cond_wait(ddsrt_cond_t* cond, ddsrt_mutex_t* mutex) ddsrt_nonnull_all;

/**
 * @brief 在 @abstime 之前等待条件变量被触发 (Wait until @abstime for a condition variable to be
 * signalled)
 *
 * @param[in]  cond     需要阻塞的条件变量 (Condition variable to block on)
 * @param[in]  mutex    与条件变量关联的互斥锁 (Mutex to associate with condition variable)
 * @param[in]  abstime  自 UNIX 纪元以来的纳秒时间 (Time in nanoseconds since UNIX Epoch)
 *
 * @pre 调用线程必须持有由 @mutex 指定的互斥锁 (The calling thread must hold the mutex specified by
 * @mutex)
 *
 * @post 调用线程将持有由 @mutex 指定的互斥锁 (The calling thread will hold the mutex specified by
 * @mutex)
 *
 * @returns 如果在由 @abstime 指定的绝对时间之前条件变量未触发，则返回 false，否则返回 true (false
 * if the condition variable was not signalled before the absolute time specified by @abstime
 * passed, otherwise true)
 */
DDS_EXPORT bool ddsrt_cond_waituntil(ddsrt_cond_t* cond, ddsrt_mutex_t* mutex, dds_time_t abstime)
    ddsrt_nonnull((1, 2));

/**
 * @brief 在 @reltime 之前等待条件变量被触发 (Wait for @reltime for a condition variable to be
 * signalled)
 *
 * @param[in]  cond     需要阻塞的条件变量 (Condition variable to block on)
 * @param[in]  mutex    与条件变量关联的互斥锁 (Mutex to associate with condition variable)
 * @param[in]  reltime  自 UNIX 纪元以来的纳秒时间 (Time in nanoseconds since UNIX Epoch)
 *
 * @pre 调用线程必须持有由 @mutex 指定的互斥锁 (The calling thread must hold the mutex specified by
 * @mutex)
 *
 * @post 调用线程将持有由 @mutex 指定的互斥锁 (The calling thread will hold the mutex specified by
 * @mutex)
 *
 * @returns 如果在由 @reltime 指定的相对时间之前条件变量未触发，则返回 false，否则返回 true (false
 * if the condition variable was not signalled before the relative time specified by @reltime
 * passed, otherwise true)
 */
DDS_EXPORT bool ddsrt_cond_waitfor(ddsrt_cond_t* cond, ddsrt_mutex_t* mutex, dds_duration_t reltime)
    ddsrt_nonnull((1, 2));

/**
 * @brief Signal a condition variable and unblock at least one thread.
 *        发送信号到条件变量并至少解锁一个线程。
 *
 * @param[in]  cond  Condition variable to signal. 要发送信号的条件变量。
 *
 * @pre The mutex associated with the condition in general should be acquired
 *      by the calling thread before setting the condition state and
 *      signalling.
 *      通常情况下，在设置条件状态和发出信号之前，调用线程应该获取与条件关联的互斥锁。
 */
DDS_EXPORT void ddsrt_cond_signal(ddsrt_cond_t* cond) ddsrt_nonnull_all;

/**
 * @brief Signal a condition variable and unblock all threads.
 *        发送信号到条件变量并解锁所有线程。
 *
 * @param[in]  cond  Condition variable to signal. 要发送信号的条件变量。
 *
 * @pre The mutex associated with the condition in general should be acquired
 *      by the calling thread before setting the condition state and
 *      signalling
 *      通常情况下，在设置条件状态和发出信号之前，调用线程应该获取与条件关联的互斥锁。
 */
DDS_EXPORT void ddsrt_cond_broadcast(ddsrt_cond_t* cond) ddsrt_nonnull_all;

/**
 * @brief Initialize a read-write lock.
 *        初始化一个读写锁。
 *
 * @param[in]  rwlock  Read-write lock to initialize. 要初始化的读写锁。
 */
DDS_EXPORT void ddsrt_rwlock_init(ddsrt_rwlock_t* rwlock) ddsrt_nonnull_all;

/**
 * @brief Destroy a read-write lock.
 *        销毁一个读写锁。
 *
 * @param[in]  rwlock  Read-write lock to destroy. 要销毁的读写锁。
 */
DDS_EXPORT void ddsrt_rwlock_destroy(ddsrt_rwlock_t* rwlock);

/**
 * @brief Acquire a read-write lock for reading.
 *        获取一个读写锁以进行读操作。
 *
 * @param[in]  rwlock  Read-write lock to acquire. 要获取的读写锁。
 *
 * @post Data related to the critical section must not be changed by the
 *       calling thread.
 *       调用线程不能更改与关键部分相关的数据。
 */
DDS_EXPORT void ddsrt_rwlock_read(ddsrt_rwlock_t* rwlock) ddsrt_nonnull_all;

/**
 * @brief Acquire a read-write lock for writing.
 *        获取一个读写锁以进行写操作。
 *
 * @param[in]  rwlock  Read-write lock to acquire. 要获取的读写锁。
 */
DDS_EXPORT void ddsrt_rwlock_write(ddsrt_rwlock_t* rwlock) ddsrt_nonnull_all;

/**
 * @brief 尝试获取读写锁以进行读取操作。
 *        Try to acquire a read-write lock for reading.
 *
 * 如果锁已被其他线程独占，立即返回。
 * Immediately return if the lock is already exclusively acquired by another thread.
 *
 * @param[in]  rwlock  要获取的读写锁。
 *                    Read-write lock to acquire.
 *
 * @post 调用线程不能更改与临界区相关的数据。
 *       Data related to the critical section must not be changed by the calling thread.
 *
 * @returns 如果获取了锁，则返回 true，否则返回 false。
 *          true if the lock was acquired, otherwise false.
 */
DDS_EXPORT bool ddsrt_rwlock_tryread(ddsrt_rwlock_t* rwlock)
    ddsrt_nonnull_all ddsrt_attribute_warn_unused_result;

/**
 * @brief 尝试获取读写锁以进行写入操作。
 *        Try to acquire a read-write lock for writing.
 *
 * 如果锁已被其他线程获取（无论是读还是写），立即返回。
 * Immediately return if the lock is already acquired, either for reading or writing, by another
 * thread.
 *
 * @param[in]  rwlock  要获取的读写锁。
 *                    Read-write lock to acquire.
 *
 * @returns 如果获取了锁，则返回 true，否则返回 false。
 *          true if the lock was acquired, otherwise false.
 */
DDS_EXPORT bool ddsrt_rwlock_trywrite(ddsrt_rwlock_t* rwlock)
    ddsrt_nonnull_all ddsrt_attribute_warn_unused_result;

/**
 * @brief 释放先前获取的读写锁。
 *        Release a previously acquired read-write lock.
 *
 * @param[in]  rwlock  要释放的读写锁。
 *                    Read-write lock to release.
 */
DDS_EXPORT void ddsrt_rwlock_unlock(ddsrt_rwlock_t* rwlock) ddsrt_nonnull_all;

/* 由 ddsrt_once 使用的初始化回调 */
/* Initialization callback used by ddsrt_once */
typedef void (*ddsrt_once_fn)(void);

/**
 * @brief 对于给定的控制，仅调用一次 init_fn。
 *        Invoke init_fn exactly once for a given control.
 *
 * 第一个调用此函数的线程将使用给定控制调用由 @init_fn
 * 指定的函数（无参数）。之后使用相同控制的调用将不会调用指定的函数。 The first thread to call this
 * function with a given control will call the function specified by @init_fn with no arguments. All
 * following calls with the same control will not call the specified function.
 *
 * @pre 控制参数已使用 DDSRT_ONCE_INIT 正确初始化。
 *      The control parameter is properly initialized with DDSRT_ONCE_INIT.
 */
DDS_EXPORT void ddsrt_once(ddsrt_once_t* control, ddsrt_once_fn init_fn);

#if defined(__cplusplus)
}
#endif

#endif /* DDSRT_SYNC_H */
