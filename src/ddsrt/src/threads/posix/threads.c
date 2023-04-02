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

/* _GNU_SOURCE is required for pthread_getname_np and pthread_setname_np. */
#define _GNU_SOURCE
#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <unistd.h>

#include "dds/ddsrt/heap.h"
#include "dds/ddsrt/log.h"
#include "dds/ddsrt/misc.h"
#include "dds/ddsrt/static_assert.h"
#include "dds/ddsrt/string.h"
#include "dds/ddsrt/types.h"
#include "threads_priv.h"

/* 定义一个结构体，用于存储线程的上下文信息。
 * Define a structure to store the context information of a thread.
 */
typedef struct {
  char *name;                      // 线程名。Thread name.
  ddsrt_thread_routine_t routine;  // 线程执行的函数。The function executed by the thread.
  void *arg;                       // 函数的参数。The argument of the function.
} thread_context_t;

// 根据不同的操作系统平台，包含相应的头文件和定义线程名的最大长度。
// Include corresponding header files and define the maximum length of thread names based on
// different operating system platforms.
#if defined(__linux)
#include <dirent.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#define MAXTHREADNAMESIZE \
  (15) /* 16字节，包括空终止字节。16 bytes, including null-terminating byte. */
#elif defined(__APPLE__)
#include <mach/mach_init.h>
#include <mach/task.h>
#include <mach/task_info.h>
#include <mach/thread_info.h> /* MAXTHREADNAMESIZE */
#include <mach/vm_map.h>
#elif defined(__sun)
#define MAXTHREADNAMESIZE (31)
#elif defined(__FreeBSD__)
/* 需要使用pthread_get_name_np和pthread_set_name_np函数。Required for pthread_get_name_np and
 * pthread_set_name_np. */
#include <pthread_np.h>
#include <sys/thr.h>
#define MAXTHREADNAMESIZE (MAXCOMLEN)
#elif defined(__VXWORKS__)
#include <taskLib.h>
/* VX_TASK_NAME_LENGTH是线程名的最大字节数，不包括空终止字节。
 * VX_TASK_NAME_LENGTH is the maximum number of bytes, excluding null-terminating byte, for a thread
 * name.
 */
#define MAXTHREADNAMESIZE (VX_TASK_NAME_LENGTH)
#elif defined(__QNXNTO__)
#include <pthread.h>
#include <sys/neutrino.h>
#define MAXTHREADNAMESIZE (_NTO_THREAD_NAME_MAX - 1)
#endif /* __APPLE__ */

/**
 * @brief 获取线程名称 (Get the thread name)
 *
 * @param[out] str 用于存储线程名称的缓冲区 (Buffer to store the thread name)
 * @param[in] size 缓冲区大小 (Size of the buffer)
 * @return 返回实际写入缓冲区的字符数（不包括空终止符）(Returns the number of characters actually
 * written to the buffer, excluding the null terminator)
 */
size_t ddsrt_thread_getname(char *str, size_t size) {
#ifdef MAXTHREADNAMESIZE
  // 定义一个缓冲区，大小为最大线程名大小加1 (Define a buffer with a size of maximum thread name
  // size plus 1)
  char buf[MAXTHREADNAMESIZE + 1] = "";
#endif
  // 初始化计数器为0 (Initialize the counter to 0)
  size_t cnt = 0;

  // 断言：str 不为空 (Assert: str is not NULL)
  assert(str != NULL);
  // 断言：size 大于0 (Assert: size is greater than 0)
  assert(size > 0);

#if defined(__linux)
  /* 在 Linux 上，线程名限制为16字节，缓冲区应该留有空间。
     为了可移植性，prctl 优先于 pthread_getname_np。例如 musl libc。*/
  // 使用 prctl 函数获取线程名 (Use prctl function to get the thread name)
  (void)prctl(PR_GET_NAME, (unsigned long)buf, 0UL, 0UL, 0UL);
  // 将 buf 中的线程名复制到 str，并返回实际写入的字符数 (Copy the thread name in buf to str and
  // return the number of characters actually written)
  cnt = ddsrt_strlcpy(str, buf, size);
#elif defined(__APPLE__)
  /* APPLE 上的 pthread_getname_np 使用 strlcpy 复制线程名，但
     不返回（已经）写入的字节数。使用中间缓冲区。*/
  // 使用 pthread_getname_np 函数获取线程名 (Use pthread_getname_np function to get the thread name)
  (void)pthread_getname_np(pthread_self(), buf, sizeof(buf));
  // 将 buf 中的线程名复制到 str，并返回实际写入的字符数 (Copy the thread name in buf to str and
  // return the number of characters actually written)
  cnt = ddsrt_strlcpy(str, buf, size);
#elif defined(__FreeBSD__)
  // 使用 pthread_get_name_np 函数获取线程名 (Use pthread_get_name_np function to get the thread
  // name)
  (void)pthread_get_name_np(pthread_self(), buf, sizeof(buf));
  // 将 buf 中的线程名复制到 str，并返回实际写入的字符数 (Copy the thread name in buf to str and
  // return the number of characters actually written)
  cnt = ddsrt_strlcpy(str, buf, size);
#elif defined(__sun)
#if !(__SunOS_5_6 || __SunOS_5_7 || __SunOS_5_8 || __SunOS_5_9 || __SunOS_5_10)
  // 使用 pthread_getname_np 函数获取线程名 (Use pthread_getname_np function to get the thread name)
  (void)pthread_getname_np(pthread_self(), buf, sizeof(buf));
#else
  // 将缓冲区的第一个字符设置为空 (Set the first character of the buffer to null)
  buf[0] = 0;
#endif
  // 将 buf 中的线程名复制到 str，并返回实际写入的字符数 (Copy the thread name in buf to str and
  // return the number of characters actually written)
  cnt = ddsrt_strlcpy(str, buf, size);
#elif defined(__VXWORKS__)
  {
    char *ptr;
    /* VxWorks 不支持通过 POSIX 线程 API 检索任务名，但任务 API
       通过 taskName 提供了它。*/
    /* 不要释放 taskName 返回的指针。详见 src/wind/taskInfo.c。*/
    // 使用 taskName 函数获取任务名 (Use taskName function to get the task name)
    ptr = taskName(taskIdSelf());
    if (ptr == NULL) {
      ptr = buf;
    }
    // 将 ptr 中的线程名复制到 str，并返回实际写入的字符数 (Copy the thread name in ptr to str and
    // return the number of characters actually written)
    cnt = ddsrt_strlcpy(str, ptr, size);
  }
#elif defined(__QNXNTO__)
  // 使用 pthread_getname_np 函数获取线程名 (Use pthread_getname_np function to get the thread name)
  (void)pthread_getname_np(pthread_self(), buf, sizeof(buf));
  // 将 buf 中的线程名复制到 str，并返回实际写入的字符数 (Copy the thread name in buf to str and
  // return the number of characters actually written)
  cnt = ddsrt_strlcpy(str, buf, size);
#endif

  /* 如果不支持线程名查找或线程名为空，则使用线程标识符作为备用。*/
  // 如果 cnt 为0，表示没有获取到线程名 (If cnt is 0, it means that the thread name was not
  // obtained)
  if (cnt == 0) {
    // 获取线程 ID (Get the thread ID)
    ddsrt_tid_t tid = ddsrt_gettid();
    // 将线程 ID 写入 str，并返回实际写入的字符数 (Write the thread ID to str and return the number
    // of characters actually written)
    cnt = (size_t)snprintf(str, size, "%" PRIdTID, tid);
  }

  // 返回实际写入的字符数 (Return the number of characters actually written)
  return cnt;
}

/**
 * @brief 设置线程名称 (Set the thread name)
 *
 * @param[in] name 线程名称 (Thread name)
 */
void ddsrt_thread_setname(const char *__restrict name) {
  // 断言：确保名称不为空 (Assert: Ensure the name is not NULL)
  assert(name != NULL);

#if defined(__linux)
  // Linux下线程名称限制为16个字节。如果名称超过限制，返回ERANGE，因此静默截断。
  // (Thread names are limited to 16 bytes on Linux. ERANGE is returned if the
  // name exceeds the limit, so silently truncate.)
  char buf[MAXTHREADNAMESIZE + 1] = "";
  (void)ddsrt_strlcpy(buf, name, sizeof(buf));
  (void)pthread_setname_np(pthread_self(), buf);
#elif defined(__APPLE__)
  // 在苹果系统中设置线程名 (Set the thread name on Apple systems)
  (void)pthread_setname_np(name);
#elif defined(__FreeBSD__)
  // 在FreeBSD系统中设置线程名 (Set the thread name on FreeBSD systems)
  (void)pthread_set_name_np(pthread_self(), name);
#elif defined(__sun)
  // Solaris下线程名称限制为31个字节。超出的字节将被静默截断。
  // (Thread names are limited to 31 bytes on Solaris. Excess bytes are
  // silently truncated.)
#if !(__SunOS_5_6 || __SunOS_5_7 || __SunOS_5_8 || __SunOS_5_9 || __SunOS_5_10)
  (void)pthread_setname_np(pthread_self(), name);
#endif
#elif defined(__QNXNTO__)
  // 在QNX系统中设置线程名 (Set the thread name on QNX systems)
  (void)pthread_setname_np(pthread_self(), name);
#else
  // VxWorks不支持在任务创建后设置任务名称。可以通过pthread_attr_setname设置任务名称。
  // (VxWorks does not support the task name to be set after a task is created.
  // Setting the name of a task can be done through pthread_attr_setname.)
#warning "ddsrt_thread_setname is not supported"
#endif
}

/**
 * \brief 包装线程启动例程 (Wrap thread start routine)
 *
 * os_startRoutineWrapper 封装了一个线程的启动例程。
 * 在调用用户例程之前，它会在线程上下文中设置线程名称。
 * 使用 pthread_getspecific，可以为不同目的检索名称。
 */
static void *os_startRoutineWrapper(void *threadContext) {
  // 定义一个指向 thread_context_t 结构体的指针，并将传入的参数赋值给它
  // (Define a pointer to the thread_context_t structure and assign the passed argument to it)
  thread_context_t *context = threadContext;
  uintptr_t resultValue;

  // 设置线程名称 (Set the thread name)
  ddsrt_thread_setname(context->name);

  // 调用用户例程 (Call the user routine)
  resultValue = context->routine(context->arg);

  // 释放线程上下文资源，参数是 os_procCreate 的调用者的责任
  // (Free the thread context resources, arguments are the responsibility of the caller of
  // os_procCreate)
  ddsrt_free(context->name);
  ddsrt_free(context);

#if defined(__VXWORKS__) && !defined(_WRS_KERNEL)
  struct sched_param sched_param;
  int max, policy = 0;

  // VxWorks 6.x RTP 模式中 pthread_join 存在已知问题。
  //
  // WindRiver: 当 pthread_join 返回时，并不总是表示线程已经结束。
  // 如果调用 pthread_join 的线程优先级高于当前正在终止的线程，
  // 那么 pthread_join 可能在 pthread_exit 完成之前返回。这与
  // POSIX 规范冲突，该规范要求 pthread_join 只有在线程真正终止时才返回。
  // WindRiver 支持建议的解决方法是在交还信号量之前提高要终止的线程（任务）的优先级，
  // 以确保线程在 pthread_join 返回之前退出。
  //
  // 此错误已提交给 WindRiver，TSR 编号为 815826。

  // 请注意，此处可能引发的任何错误都不是致命的，因为线程可能已经退出。
  // (Note that any possible errors raised here are not terminal since the thread may have exited at
  // this point anyway.)
  if (pthread_getschedparam(thread.v, &policy, &sched_param) == 0) {
    max = sched_get_priority_max(policy);
    if (max != -1) {
      (void)pthread_setschedprio(thread.v, max);
    }
  }
#endif

  // 返回用户例程的结果 (Return the result of the user routine)
  return (void *)resultValue;
}

/**
 * @brief 创建一个新的线程 (Create a new thread)
 *
 * @param[out] threadptr 指向线程句柄的指针 (Pointer to the thread handle)
 * @param[in] name 线程的名称 (Name of the thread)
 * @param[in] threadAttr 线程属性 (Thread attributes)
 * @param[in] start_routine 线程的启动函数 (Thread's start function)
 * @param[in] arg 传递给启动函数的参数 (Argument passed to the start function)
 * @return dds_return_t 成功时返回 DDS_RETCODE_OK，失败时返回相应的错误代码 (Returns DDS_RETCODE_OK
 * on success, appropriate error code on failure)
 */
dds_return_t ddsrt_thread_create(ddsrt_thread_t *threadptr,
                                 const char *name,
                                 const ddsrt_threadattr_t *threadAttr,
                                 uint32_t (*start_routine)(void *),
                                 void *arg) {
  pthread_attr_t attr;  // 定义 POSIX 线程属性变量 (Define POSIX thread attribute variable)
  thread_context_t *ctx;
  ddsrt_threadattr_t tattr;  // 定义本地线程属性变量 (Define local thread attribute variable)
  int result, create_ret;
  sigset_t set, oset;

  assert(threadptr !=
         NULL);  // 断言线程句柄指针不为空 (Assert that the thread handle pointer is not NULL)
  assert(name != NULL);  // 断言线程名称不为空 (Assert that the thread name is not NULL)
  assert(threadAttr !=
         NULL);  // 断言线程属性不为空 (Assert that the thread attributes are not NULL)
  assert(start_routine !=
         NULL);  // 断言线程启动函数不为空 (Assert that the thread start function is not NULL)
  tattr = *threadAttr;  // 复制线程属性到本地变量 (Copy thread attributes to local variable)

  if (pthread_attr_init(&attr) != 0)
    return DDS_RETCODE_ERROR;  // 初始化 POSIX 线程属性，如果失败则返回错误代码 (Initialize POSIX
                               // thread attributes, return error code if failed)

#if defined(__VXWORKS__)
  /* pthread_setname_np is not available on VxWorks. Use pthread_attr_setname
     instead (proprietary VxWorks extension). */
  (void)pthread_attr_setname(&attr,
                             name);  // 在 VxWorks 上设置线程名称 (Set thread name on VxWorks)
#endif

  if (pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM) !=
          0 ||  // 设置线程范围为系统级 (Set thread scope to system level)
      pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE) !=
          0)  // 设置线程的分离状态为可连接 (Set thread's detach state to joinable)
    goto err;

  if (tattr.stackSize != 0) {  // 如果堆栈大小非零 (If stack size is non-zero)
#ifdef PTHREAD_STACK_MIN
    if (tattr.stackSize <
        (uint32_t)PTHREAD_STACK_MIN)  // 如果堆栈大小小于最小值，则设置为最小值 (If stack size is
                                      // less than minimum, set it to minimum)
      tattr.stackSize = (uint32_t)PTHREAD_STACK_MIN;
#endif
    if ((result = pthread_attr_setstacksize(&attr, tattr.stackSize)) !=
        0) {  // 设置线程堆栈大小，如果失败则输出错误信息 (Set thread stack size, output error
              // message if failed)
      DDS_ERROR("ddsrt_thread_create(%s): pthread_attr_setstacksize(%" PRIu32
                ") failed with error %d\n",
                name, tattr.stackSize, result);
      goto err;
    }
  }

  /**
   * @brief 根据线程属性 tattr 设置线程优先级和调度策略
   * @param[in] tattr 线程属性结构体
   * @param[in] name 线程名称
   * @return 无返回值
   *
   * @details Based on the thread attributes tattr, set the thread priority and scheduling policy.
   * @param[in] tattr Thread attribute structure
   * @param[in] name Thread name
   * @return No return value
   */
  if (tattr.schedClass == DDSRT_SCHED_DEFAULT) {
    if (tattr.schedPriority != 0) {
      /* 如果调用者没有设置类别，他也不能尝试设置优先级，我们通过期望一个0来近似实现。
         FIXME: 应该将此作为配置验证的一部分进行 */
      /* If caller doesn't set the class, he must not try to set the priority, which we
         approximate by expecting a 0. FIXME: should do this as part of config validation */
      DDS_ERROR("ddsrt_thread_create(%s): schedClass DEFAULT but priority != 0 is unsupported\n",
                name);
      goto err;
    }
  } else {
    int policy;
    struct sched_param sched_param;
    // 获取当前线程的调度参数
    // Get the scheduling parameters of the current thread
    if ((result = pthread_getschedparam(pthread_self(), &policy, &sched_param) != 0) != 0) {
      DDS_ERROR("ddsrt_thread_create(%s): pthread_attr_getschedparam(self) failed with error %d\n",
                name, result);
      goto err;
    }
    switch (tattr.schedClass) {
      case DDSRT_SCHED_DEFAULT:
        assert(0);
        break;
      case DDSRT_SCHED_REALTIME:
        // 设置实时调度策略
        // Set real-time scheduling policy
        policy = SCHED_FIFO;
        break;
      case DDSRT_SCHED_TIMESHARE:
        // 设置时间共享调度策略
        // Set time-sharing scheduling policy
        policy = SCHED_OTHER;
        break;
    }
    // 设置线程属性的调度策略
    // Set the scheduling policy of the thread attribute
    if ((result = pthread_attr_setschedpolicy(&attr, policy)) != 0) {
      DDS_ERROR("ddsrt_thread_create(%s): pthread_attr_setschedpolicy(%d) failed with error %d\n",
                name, policy, result);
      goto err;
    }
    // 设置线程属性的优先级
    // Set the priority of the thread attribute
    sched_param.sched_priority = tattr.schedPriority;
    if ((result = pthread_attr_setschedparam(&attr, &sched_param)) != 0) {
      DDS_ERROR(
          "ddsrt_thread_create(%s): pthread_attr_setschedparam(priority = %d) failed with error "
          "%d\n",
          name, tattr.schedPriority, result);
      goto err;
    }
    // 设置线程属性的继承调度属性
    // Set the inheritance scheduling attribute of the thread attribute
    if ((result = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED)) != 0) {
      DDS_ERROR(
          "ddsrt_thread_create(%s): pthread_attr_setinheritsched(EXPLICIT) failed with error %d\n",
          name, result);
      goto err;
    }
  }

  /* 构建上下文结构并启动线程 (Construct context structure & start thread) */
  ctx = ddsrt_malloc(sizeof(thread_context_t));
  ctx->name = ddsrt_strdup(name);  // 复制线程名字 (Duplicate the thread name)
  ctx->routine = start_routine;    // 设置线程的启动函数 (Set the thread's start routine)
  ctx->arg = arg;                  // 设置线程的参数 (Set the thread's argument)

  /* 在我们自己的线程中阻止信号传递（SIGXCPU
     除外，因为我们有一种方法可以转储堆栈跟踪，但这应该得到改进） (Block signal delivery in our own
     threads, except for SIGXCPU to allow stack trace dumping) */
  sigfillset(&set);
#ifdef __APPLE__
  DDSRT_WARNING_GNUC_OFF(sign - conversion)
#endif
  sigdelset(&set, SIGXCPU);  // 从信号集中删除 SIGXCPU (Remove SIGXCPU from the signal set)
#ifdef __APPLE__
  DDSRT_WARNING_GNUC_ON(sign - conversion)
#endif
  sigprocmask(SIG_BLOCK, &set, &oset);  // 阻塞信号集中的信号 (Block signals in the signal set)
  if ((create_ret = pthread_create(&threadptr->v, &attr, os_startRoutineWrapper, ctx)) != 0) {
    DDS_ERROR("os_threadCreate(%s): pthread_create failed with error %d\n", name, create_ret);
    goto err_create;
  }
  sigprocmask(SIG_SETMASK, &oset, NULL);  // 恢复原始信号掩码 (Restore original signal mask)
  pthread_attr_destroy(&attr);  // 销毁线程属性对象 (Destroy the thread attribute object)
  return DDS_RETCODE_OK;

err_create:
  ddsrt_free(ctx->name);  // 释放线程名字内存 (Free the memory of the thread name)
  ddsrt_free(ctx);        // 释放上下文结构内存 (Free the memory of the context structure)
err:
  pthread_attr_destroy(&attr);  // 销毁线程属性对象 (Destroy the thread attribute object)
  return DDS_RETCODE_ERROR;
}

/* 获取线程 ID 的函数 (Function to get the thread ID) */
ddsrt_tid_t ddsrt_gettid(void) {
  ddsrt_tid_t tid;

#if defined(__linux)
  tid = syscall(
      SYS_gettid);  // Linux 系统获取线程 ID 的方法 (Method to get the thread ID on Linux systems)
#elif defined(__FreeBSD__) && (__FreeBSD__ >= 9)
  /* FreeBSD >= 9.0 */
  tid = pthread_getthreadid_np();  // FreeBSD 系统获取线程 ID 的方法 (Method to get the thread ID on
                                   // FreeBSD systems)
#elif defined(__APPLE__) && \
    !(defined(__MAC_OS_X_VERSION_MIN_REQUIRED) && __MAC_OS_X_VERSION_MIN_REQUIRED < 1060)
  /* macOS >= 10.6 */
  pthread_threadid_np(
      NULL, &tid);  // macOS 系统获取线程 ID 的方法 (Method to get the thread ID on macOS systems)
#elif defined(__VXWORKS__)
  tid = taskIdSelf();  // VxWorks 系统获取线程 ID 的方法 (Method to get the thread ID on VxWorks
                       // systems)
#else
  tid = (uintmax_t)((uintptr_t)pthread_self());  // 其他系统获取线程 ID 的方法 (Method to get the
                                                 // thread ID on other systems)
#endif

  return tid;
}

/**
 * @brief 获取指定线程的线程ID (Get the thread ID of the specified thread)
 * @param[in] thread 要获取线程ID的线程 (The thread for which to get the thread ID)
 * @return 返回线程ID (Returns the thread ID)
 */
ddsrt_tid_t ddsrt_gettid_for_thread(ddsrt_thread_t thread) {
  // 将线程结构体中的值转换为线程ID并返回 (Convert the value in the thread structure to a thread ID
  // and return it)
  return (ddsrt_tid_t)thread.v;
}

/**
 * @brief 获取当前线程的线程ID (Get the thread ID of the current thread)
 * @return 返回当前线程的线程ID (Returns the thread ID of the current thread)
 */
ddsrt_thread_t ddsrt_thread_self(void) {
  // 定义一个线程ID变量，并将当前线程的线程ID存储在其中 (Define a thread ID variable and store the
  // current thread's thread ID in it)
  ddsrt_thread_t id = {.v = pthread_self()};
  // 返回当前线程的线程ID (Return the thread ID of the current thread)
  return id;
}

/**
 * @brief 比较两个线程是否相等 (Compare whether two threads are equal)
 * @param[in] a 第一个线程 (The first thread)
 * @param[in] b 第二个线程 (The second thread)
 * @return 如果两个线程相等，返回true；否则返回false (If the two threads are equal, return true;
 * otherwise return false)
 */
bool ddsrt_thread_equal(ddsrt_thread_t a, ddsrt_thread_t b) {
  // 使用pthread_equal比较两个线程是否相等，并返回结果 (Use pthread_equal to compare whether the two
  // threads are equal and return the result)
  return (pthread_equal(a.v, b.v) != 0);
}

/**
 * @brief 等待指定线程结束并获取其返回值 (Wait for the specified thread to end and get its return
 * value)
 * @param[in] thread 要等待的线程 (The thread to wait for)
 * @param[out] thread_result 存储线程返回值的指针，如果为NULL，则忽略返回值 (Pointer to store the
 * thread return value; if NULL, ignore the return value)
 * @return 如果成功，返回DDS_RETCODE_OK；否则返回DDS_RETCODE_ERROR (If successful, return
 * DDS_RETCODE_OK; otherwise return DDS_RETCODE_ERROR)
 */
dds_return_t ddsrt_thread_join(ddsrt_thread_t thread, uint32_t *thread_result) {
  int err;
  void *vthread_result;

  // 检查线程是否有效 (Check if the thread is valid)
  assert(thread.v);

  // 使用pthread_join等待指定线程结束，并获取其返回值 (Use pthread_join to wait for the specified
  // thread to end and get its return value)
  if ((err = pthread_join(thread.v, &vthread_result)) != 0) {
    // 如果出错，输出错误信息并返回错误代码 (If an error occurs, output the error message and return
    // the error code)
    DDS_ERROR("pthread_join(0x%" PRIxMAX ") failed with error %d\n",
              (uintmax_t)((uintptr_t)thread.v), err);
    return DDS_RETCODE_ERROR;
  }

  // 如果提供了用于存储线程返回值的指针，则将返回值存储在其中 (If a pointer is provided to store the
  // thread return value, store the return value in it)
  if (thread_result) *thread_result = (uint32_t)((uintptr_t)vthread_result);
  // 返回成功代码 (Return the success code)
  return DDS_RETCODE_OK;
}

#if defined __linux
/**
 * @brief 获取当前进程的线程列表 (Get the list of threads for the current process)
 * @param[out] tids 线程 ID 数组的指针 (Pointer to an array of thread IDs)
 * @param[in] size 线程 ID 数组的大小 (Size of the thread ID array)
 * @return 成功时返回线程数量，失败时返回错误代码 (On success, returns the number of threads; on
 * failure, returns an error code)
 */
dds_return_t ddsrt_thread_list(ddsrt_thread_list_id_t *__restrict tids, size_t size) {
  DIR *dir;
  struct dirent *de;
  // 打开 /proc/self/task 目录 (Open the /proc/self/task directory)
  if ((dir = opendir("/proc/self/task")) == NULL) return DDS_RETCODE_ERROR;
  dds_return_t n = 0;
  // 遍历目录中的条目 (Iterate through the entries in the directory)
  while ((de = readdir(dir)) != NULL) {
    // 跳过 . 和 .. 目录 (Skip the . and .. directories)
    if (de->d_name[0] == '.' &&
        (de->d_name[1] == 0 || (de->d_name[1] == '.' && de->d_name[2] == 0)))
      continue;
    int pos;
    long tid;
    // 尝试解析条目名称为线程 ID (Try to parse the entry name as a thread ID)
    if (sscanf(de->d_name, "%ld%n", &tid, &pos) != 1 || de->d_name[pos] != 0) {
      n = DDS_RETCODE_ERROR;
      break;
    }
    // 如果数组还有空间，将线程 ID 添加到数组中 (If there is still space in the array, add the
    // thread ID to the array)
    if ((size_t)n < size) tids[n] = (ddsrt_thread_list_id_t)tid;
    n++;
  }
  closedir(dir);
  // 如果没有线程，说明出现了严重错误 (If there were no threads, something must've gone badly wrong)
  return (n == 0) ? DDS_RETCODE_ERROR : n;
}

/**
 * @brief 获取指定线程的名称 (Get the name of the specified thread)
 * @param[in] tid 要查询的线程 ID (Thread ID to query)
 * @param[out] name 线程名称缓冲区的指针 (Pointer to the thread name buffer)
 * @param[in] size 线程名称缓冲区的大小 (Size of the thread name buffer)
 * @return 成功时返回 DDS_RETCODE_OK，失败时返回错误代码 (On success, returns DDS_RETCODE_OK; on
 * failure, returns an error code)
 */
dds_return_t ddsrt_thread_getname_anythread(ddsrt_thread_list_id_t tid,
                                            char *__restrict name,
                                            size_t size) {
  char file[100];
  FILE *fp;
  int pos;
  // 构造 /proc/self/task/<tid>/stat 文件路径 (Construct the /proc/self/task/<tid>/stat file path)
  pos = snprintf(file, sizeof(file), "/proc/self/task/%lu/stat", (unsigned long)tid);
  if (pos < 0 || pos >= (int)sizeof(file)) return DDS_RETCODE_ERROR;
  // 打开文件 (Open the file)
  if ((fp = fopen(file, "r")) == NULL) return DDS_RETCODE_NOT_FOUND;
  int c;
  size_t namelen = 0, namepos = 0;
  // 查找左括号 (Find the left parenthesis)
  while ((c = fgetc(fp)) != EOF)
    if (c == '(') break;
  // 读取线程名称，直到遇到右括号 (Read the thread name until a right parenthesis is encountered)
  while ((c = fgetc(fp)) != EOF) {
    if (c == ')') namelen = namepos;
    if (namepos + 1 < size) name[namepos++] = (char)c;
  }
  fclose(fp);
  assert(size == 0 || namelen < size);
  // 添加字符串终止符 (Add the string terminator)
  if (size > 0) name[namelen] = 0;
  return DDS_RETCODE_OK;
}
#elif defined __APPLE__
DDSRT_STATIC_ASSERT(sizeof(ddsrt_thread_list_id_t) == sizeof(mach_port_t));

/**
 * @brief 获取当前进程的线程列表 (Get the list of threads for the current process)
 * @param[out] tids 线程 ID 数组的指针 (Pointer to an array of thread IDs)
 * @param[in] size 线程 ID 数组的大小 (Size of the thread ID array)
 * @return 成功时返回线程数量，失败时返回错误代码 (On success, returns the number of threads; on
 * failure, returns an error code)
 */
dds_return_t ddsrt_thread_list(ddsrt_thread_list_id_t *__restrict tids, size_t size) {
  thread_act_array_t tasks;
  mach_msg_type_number_t count;
  // 获取当前任务的线程列表 (Get the list of threads for the current task)
  if (task_threads(mach_task_self(), &tasks, &count) != KERN_SUCCESS) return DDS_RETCODE_ERROR;
  // 将 mach_port_t 类型的线程 ID 转换为 ddsrt_thread_list_id_t 类型，并存储到 tids 数组中 (Convert
  // the thread IDs from mach_port_t to ddsrt_thread_list_id_t and store them in the tids array)
  for (mach_msg_type_number_t i = 0; i < count && (size_t)i < size; i++)
    tids[i] = (ddsrt_thread_list_id_t)tasks[i];
  vm_deallocate(mach_task_self(), (vm_address_t)tasks, count * sizeof(thread_act_t));
  return (dds_return_t)count;
}

/**
 * @brief 获取指定线程的名称 (Get the name of the specified thread)
 * @param[in] tid 要查询的线程 ID (Thread ID to query)
 * @param[out] name 线程名称缓冲区的指针 (Pointer to the thread name buffer)
 * @param[in] size 线程名称缓冲区的大小 (Size of the thread name buffer)
 * @return 成功时返回 DDS_RETCODE_OK，失败时返回错误代码 (On success, returns DDS_RETCODE_OK; on
 * failure, returns an error code)
 */
dds_return_t ddsrt_thread_getname_anythread(ddsrt_thread_list_id_t tid,
                                            char *__restrict name,
                                            size_t size) {
  if (size > 0) {
    pthread_t pt = pthread_from_mach_thread_np((mach_port_t)tid);
    name[0] = '\0';
    // 尝试获取线程名称 (Try to get the thread name)
    if (pt == NULL || pthread_getname_np(pt, name, size) != 0 || name[0] == 0)
      snprintf(name, size, "task%" PRIu64, (uint64_t)tid);
  }
  return DDS_RETCODE_OK;
}
#endif

// 静态线程局部存储变量，用于存储清理函数的键
// Static thread-local storage variable for storing the cleanup function key
static pthread_key_t thread_cleanup_key;

// 用于确保线程初始化只执行一次的静态变量
// Static variable to ensure thread initialization is executed only once
static pthread_once_t thread_once = PTHREAD_ONCE_INIT;

// 声明一个线程清理的终止函数
// Declare a termination function for thread cleanup
static void thread_cleanup_fini(void *arg);

// 确保线程初始化只执行一次的函数
// Function to ensure thread initialization is executed only once
static void thread_init_once(void) {
  int err;

  // 创建一个键，关联到线程清理的终止函数
  // Create a key associated with the thread cleanup termination function
  err = pthread_key_create(&thread_cleanup_key, &thread_cleanup_fini);

  // 断言错误码为0，表示成功创建键
  // Assert that the error code is 0, indicating successful key creation
  assert(err == 0);

  // 忽略错误码（已经处理过）
  // Ignore the error code (already handled)
  (void)err;
}

// 初始化线程
// Initialize the thread
static void thread_init(void) {
  // 执行一次性线程初始化
  // Perform one-time thread initialization
  (void)pthread_once(&thread_once, &thread_init_once);
}

// 将清理函数压入线程清理栈中
// Push the cleanup function onto the thread cleanup stack
dds_return_t ddsrt_thread_cleanup_push(void (*routine)(void *), void *arg) {
  int err;
  thread_cleanup_t *prev, *tail;

  // 断言清理函数不为空
  // Assert that the cleanup function is not NULL
  assert(routine != NULL);

  // 初始化线程
  // Initialize the thread
  thread_init();

  // 分配内存给尾部节点
  // Allocate memory for the tail node
  if ((tail = ddsrt_calloc(1, sizeof(*tail))) != NULL) {
    // 获取当前线程关联的清理函数链表
    // Get the cleanup function list associated with the current thread
    prev = pthread_getspecific(thread_cleanup_key);

    // 设置新节点的前一个节点为之前的节点
    // Set the new node's previous node to the previous node
    tail->prev = prev;

    // 设置新节点的清理函数和参数
    // Set the new node's cleanup function and argument
    tail->routine = routine;
    tail->arg = arg;

    // 将新节点设置为当前线程关联的清理函数链表
    // Set the new node as the cleanup function list associated with the current thread
    if ((err = pthread_setspecific(thread_cleanup_key, tail)) != 0) {
      // 断言错误码不等于EINVAL（表示键无效）
      // Assert that the error code is not EINVAL (indicating an invalid key)
      assert(err != EINVAL);

      // 返回资源不足的错误码
      // Return an out of resources error code
      return DDS_RETCODE_OUT_OF_RESOURCES;
    }

    // 返回成功的错误码
    // Return a successful error code
    return DDS_RETCODE_OK;
  }

  // 返回资源不足的错误码
  // Return an out of resources error code
  return DDS_RETCODE_OUT_OF_RESOURCES;
}

/**
 * @brief 清理线程并弹出栈顶的清理函数。
 *        Clean up the thread and pop the top cleanup function.
 *
 * @param[in] execute 是否执行清理函数。1为执行，0为不执行。
 *                    Whether to execute the cleanup function. 1 for execution, 0 for not.
 * @return dds_return_t 返回操作结果状态码。
 *                     Return the operation result status code.
 */
dds_return_t ddsrt_thread_cleanup_pop(int execute) {
  int err;
  thread_cleanup_t *tail;

  // 初始化线程。Initialize the thread.
  thread_init();

  // 获取线程特定数据。Get the thread-specific data.
  if ((tail = pthread_getspecific(thread_cleanup_key)) != NULL) {
    // 设置线程特定数据。Set the thread-specific data.
    if ((err = pthread_setspecific(thread_cleanup_key, tail->prev)) != 0) {
      assert(err != EINVAL);
      // 返回资源不足错误。Return out of resources error.
      return DDS_RETCODE_OUT_OF_RESOURCES;
    }

    // 如果需要执行清理函数，则执行。If execution is required, execute the cleanup function.
    if (execute) {
      tail->routine(tail->arg);
    }

    // 释放内存。Free memory.
    ddsrt_free(tail);
  }

  // 返回操作成功。Return operation success.
  return DDS_RETCODE_OK;
}

/**
 * @brief 线程清理函数。
 *        Thread cleanup function.
 *
 * @param[in] arg 清理函数参数。
 *                Cleanup function argument.
 */
static void thread_cleanup_fini(void *arg) {
  thread_cleanup_t *tail, *prev;

  tail = (thread_cleanup_t *)arg;

  // 遍历并执行清理函数。Traverse and execute cleanup functions.
  while (tail != NULL) {
    prev = tail->prev;
    assert(tail->routine != NULL);
    tail->routine(tail->arg);

    // 释放内存。Free memory.
    ddsrt_free(tail);
    tail = prev;
  }

  /* 如果作为析构函数调用，则线程特定值已经与thread_cleanup_key关联。
     If invoked as a destructor, the thread-specific value associated with thread_cleanup_key will
     already be nullified. */
}

/**
 * @brief 初始化线程。
 *        Initialize the thread.
 *
 * @param[in] reason 线程初始化原因。
 *                   Thread initialization reason.
 */
void ddsrt_thread_init(uint32_t reason) {
  (void)reason;
  thread_init();
}

/**
 * @brief 结束线程。
 *        Terminate the thread.
 *
 * @param[in] reason 线程结束原因。
 *                   Thread termination reason.
 */
void ddsrt_thread_fini(uint32_t reason) {
  thread_cleanup_t *tail;

  (void)reason;
  thread_init();

  // 获取线程特定数据。Get the thread-specific data.
  if ((tail = pthread_getspecific(thread_cleanup_key)) != NULL) {
    // 执行线程清理函数。Execute the thread cleanup function.
    thread_cleanup_fini(tail);

    // 设置线程特定数据为空。Set the thread-specific data to NULL.
    (void)pthread_setspecific(thread_cleanup_key, NULL);
  }
}
