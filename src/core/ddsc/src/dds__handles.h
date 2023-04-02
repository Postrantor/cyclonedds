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
#ifndef DDS__HANDLES_H
#define DDS__HANDLES_H

#include "dds/dds.h"
#include "dds/ddsrt/atomics.h"
#include "dds/ddsrt/retcode.h"
#include "dds/ddsrt/time.h"

#if defined(__cplusplus)
extern "C" {
#endif

struct dds_entity;

/**
 * @brief 短暂的工作思考。
 *
 * 一个句柄将与一个相关对象一起创建。如果您想通过互斥锁来保护该对象，
 * 那么这是您的责任，而不是 handleservers' 的责任。
 *
 * 句柄服务器保持一个“活动声明”计数。每次声明一个句柄时，计数会增加，
 * 当句柄释放时，计数会减少。只有在没有更多活动声明时，句柄才能被删除。
 * 您可以使用此功能确保没有人再使用句柄，这应该意味着没有人再使用相关对象。
 * 因此，当您可以删除句柄时，您可以安全地删除相关对象。
 *
 * 为了防止新的声明（例如，当您想要删除句柄时），您可以关闭句柄。
 * 这不会删除 handleserver 中的任何信息，它只是阻止新的声明。
 * 实际上，删除操作将释放 handleserver 内部内存。
 *
 * 目前，在句柄服务器中有一个全局锁，用于每个 API 调用。
 * 也许内部可以改进以避免需要这样做...
 */

// 定义 dds_handle_t 类型
typedef int32_t dds_handle_t;

/**
 * @brief 句柄链接类型。
 *
 * 查找句柄应该非常快速。
 * 但为了提高性能，可以获取一个 handlelink。
 * 这可以被 handleserver 用来绕过查找并直接访问与句柄相关的信息。
 *
 * 几乎每个 handleserver 函数都支持 handlelink。您不必提供链接。
 * 当它为 NULL 时，将执行正常的查找。
 *
 * 在删除相关句柄后，此 handlelink 将失效，之后不应再使用。
 */

// 关闭和已关闭可以合并，但有两个方法可以强制先调用 close()，
// 然后调用 close_wait()，然后调用 delete()。
#define HDL_FLAG_CLOSING (0x80000000u)          ///< 标志：正在关闭
#define HDL_FLAG_DELETE_DEFERRED (0x40000000u)  ///< 标志：延迟删除
#define HDL_FLAG_PENDING (0x20000000u)          ///< 标志：待处理
#define HDL_FLAG_IMPLICIT (0x10000000u)         ///< 标志：隐式
#define HDL_FLAG_ALLOW_CHILDREN (0x08000000u)  ///< 标志：允许子对象（引用计数包括子对象）
#define HDL_FLAG_NO_USER_ACCESS (0x04000000u)  ///< 标志：禁止用户访问

/*
  这段代码包含了一些与句柄服务器相关的函数声明。dds_handle_server_init 和 dds_handle_server_fini
  分别用于初始化和销毁句柄服务器单例。dds_handle_create
  函数用于创建一个新的句柄，并与用户数据关联。dds_handle_register_special
  函数用于注册特定句柄。dds_handle_unpend 函数用于取消挂起的句柄操作。dds_handle_close_wait
  函数用于关闭句柄，所有信息保持不变，只有新的请求将失败。
  */

/**
 * @struct dds_handle_link
 * @brief 一个包含句柄和原子计数器的结构体，用于链接操作。
 */
struct dds_handle_link {
  /**
   * @var dds_handle_t hdl
   * @brief 句柄类型，用于表示唯一的实例标识符。
   */
  dds_handle_t hdl;

  /**
   * @var ddsrt_atomic_uint32_t cnt_flags
   * @brief 原子无符号32位整数类型，用于存储计数器和标志位。
   */
  ddsrt_atomic_uint32_t cnt_flags;
};

/**
 * @brief 初始化句柄服务器单例。
 * @component handles
 *
 * @return dds_return_t 返回初始化结果
 */
dds_return_t dds_handle_server_init(void);

/**
 * @brief 销毁句柄服务器单例。
 * @component handles
 *
 * 当 fini() 被调用的次数与 init() 相同时，句柄服务器将被销毁。
 */
void dds_handle_server_fini(void);

/**
 * @component handles
 *
 * 此函数创建一个包含给定类型的新句柄，并与用户数据关联。
 *
 * 必须提供一个 kind 值 != 0，以确保不会创建 0 句柄。它还应该适合 DDSRT_HANDLE_KIND_MASK。
 * 换句话说，如果 ((kind & ~DDSRT_HANDLE_KIND_MASK != 0) || (kind & DDSRT_HANDLE_KIND_MASK ==
 * 0))，句柄创建将失败。
 *
 * 它必须采取一些巧妙的方法来确保删除的句柄在删除后不会很快重新发行。
 *
 * @param link           指向 dds_handle_link 结构体的指针
 * @param implicit       是否隐式创建句柄
 * @param allow_children 是否允许子句柄
 * @param user_access    是否允许用户访问
 *
 * @return dds_handle_t  如果返回值为正，则表示有效句柄；否则返回负句柄。
 */
dds_handle_t dds_handle_create(struct dds_handle_link* link,
                               bool implicit,
                               bool allow_children,
                               bool user_access);

/**
 * @brief 注册特定句柄。
 * @component handles
 *
 * @param link           指向 dds_handle_link 结构体的指针
 * @param implicit       是否隐式创建句柄
 * @param allow_children 是否允许子句柄
 * @param handle         要注册的句柄
 *
 * @return dds_return_t  返回注册结果
 */
dds_return_t dds_handle_register_special(struct dds_handle_link* link,
                                         bool implicit,
                                         bool allow_children,
                                         dds_handle_t handle);

/**
 * @component handles
 *
 * 取消挂起的句柄操作。
 *
 * @param link 指向 dds_handle_link 结构体的指针
 */
void dds_handle_unpend(struct dds_handle_link* link);

/**
 * @component handles
 *
 * 此函数将关闭句柄。所有信息保持不变，只有新的请求将失败。
 *
 * 对已关闭的句柄调用此函数无效。
 *
 * @param link 指向 dds_handle_link 结构体的指针
 */
void dds_handle_close_wait(struct dds_handle_link* link);

/**
 * @component handles
 *
 * 这将从服务器管理中删除与句柄相关的信息，以释放空间。
 *
 * 这是一个隐式的 close()。
 *
 * 当没有更多活动声明时，它将删除信息。必要时，它会阻塞等待所有可能的声明被释放。
 *
 * @param link 指向 dds_handle_link 结构的指针
 * @return 返回 int32_t 类型的结果
 */
int32_t dds_handle_delete(struct dds_handle_link* link);

/**
 * @component handles
 *
 * 如果给定一个有效的句柄，该句柄与 kind 匹配且未关闭，则将提供相关 arg，并增加 claims 计数。
 *
 * 成功时返回 OK。
 *
 * @param hdl 句柄
 * @param entity 指向 dds_handle_link 结构的指针的指针
 * @return 返回 int32_t 类型的结果
 */
int32_t dds_handle_pin(dds_handle_t hdl, struct dds_handle_link** entity);

/**
 * @component handles
 *
 * @param hdl 句柄
 * @param from_user 布尔值，表示是否来自用户
 * @param entity 指向 dds_handle_link 结构的指针的指针
 * @return 返回 int32_t 类型的结果
 */
int32_t dds_handle_pin_with_origin(dds_handle_t hdl,
                                   bool from_user,
                                   struct dds_handle_link** entity);

/**
 * @component handles
 *
 * @param hdl 句柄
 * @param from_user 布尔值，表示是否来自用户
 * @param entity 指向 dds_handle_link 结构的指针的指针
 * @return 返回 int32_t 类型的结果
 */
int32_t dds_handle_pin_and_ref_with_origin(dds_handle_t hdl,
                                           bool from_user,
                                           struct dds_handle_link** entity);

/**
 * @component handles
 *
 * @param link 指向 dds_handle_link 结构的指针
 */
void dds_handle_repin(struct dds_handle_link* link);

/**
 * @component handles
 *
 * 减少活动 claims 计数。
 *
 * @param link 指向 dds_handle_link 结构的指针
 */
void dds_handle_unpin(struct dds_handle_link* link);

/**
 * @component handles
 *
 * @param hdl 句柄
 * @param explicit 布尔值，表示是否显式删除
 * @param from_user 布尔值，表示是否来自用户
 * @param link 指向 dds_handle_link 结构的指针的指针
 * @return 返回 int32_t 类型的结果
 */
int32_t dds_handle_pin_for_delete(dds_handle_t hdl,
                                  bool explicit,
                                  bool from_user,
                                  struct dds_handle_link** link);

/**
 * @component handles
 *
 * @param link 指向 dds_handle_link 结构的指针
 * @param may_delete_parent 布尔值，表示是否可以删除父对象
 * @return 返回布尔值
 */
bool dds_handle_drop_childref_and_pin(struct dds_handle_link* link, bool may_delete_parent);

/**
 * @component handles
 *
 * @param link 指向 dds_handle_link 结构的指针
 */
void dds_handle_add_ref(struct dds_handle_link* link);

/**
 * @component handles
 *
 * @param link 指向 dds_handle_link 结构的指针
 * @return 返回布尔值
 */
bool dds_handle_drop_ref(struct dds_handle_link* link);

/**
 * @component handles
 *
 * @param link 指向 dds_handle_link 结构的指针
 * @return 返回布尔值
 */
bool dds_handle_close(struct dds_handle_link* link);

/**
 * @component handles
 *
 * @param link 指向 dds_handle_link 结构的指针
 * @return 返回布尔值
 */
bool dds_handle_unpin_and_drop_ref(struct dds_handle_link* link);

/**
 * @brief 检查句柄是否已关闭。
 * @component handles
 *
 * 当您已经声明了一个句柄，并且可能在另一个线程中尝试删除该句柄时（例如，在等待某个操作时），此功能非常有用。
 * 现在，您可以中断进程并释放句柄，从而使删除成为可能。
 */

// 内联函数：检查给定的 dds_handle_link 是否已关闭
inline bool dds_handle_is_closed(struct dds_handle_link* link) {
  // 返回 link 的 cnt_flags 是否具有 HDL_FLAG_CLOSING 标志位
  return (ddsrt_atomic_ld32(&link->cnt_flags) & HDL_FLAG_CLOSING) != 0;
}

/** @component handles */
// 函数：检查给定的 dds_handle_link 是否未被引用
bool dds_handle_is_not_refd(struct dds_handle_link* link);

#if defined(__cplusplus)
}
#endif

#endif /* DDS__HANDLES_H */
