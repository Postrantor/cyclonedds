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

#include <assert.h>
#include <string.h>

#include "dds/ddsi/ddsi_thread.h"
#include "dds/ddsrt/heap.h"
#include "dds/ddsrt/hopscotch.h"
#include "dds/ddsrt/random.h"
#include "dds/ddsrt/sync.h"
#include "dds/ddsrt/time.h"
#include "dds__handles.h"
#include "dds__types.h"

#define HDL_REFCOUNT_MASK (0x03fff000u)
#define HDL_REFCOUNT_UNIT (0x00001000u)
#define HDL_REFCOUNT_SHIFT 12
#define HDL_PINCOUNT_MASK (0x00000fffu)

/*
除主题之外的"常规"实体：
  - create 创建它
  - delete 立即删除它及其子实体
  - 显式域：引导复杂性需要额外的保护措施

除主题之外的隐式实体（pub，sub，domain，cyclonedds）：
  - 作为创建writer/reader/participant的结果而"自发地"创建
  - 删除最后一个子实体会导致它消失
  - 将显式删除视为对"常规"实体的删除
  - domain, cyclonedds: 引导复杂性需要额外的保护措施

主题：
  - create 创建它
  - 从不拥有子实体（因此句柄的cnt_flags可以具有不同的含义）
  - 读者、写者使其继续存在
  - 在不存在读者/写者时推迟删除
  -
如果处于"延迟删除"状态，则尝试删除它将失败（或者应该简单地返回确定同时什么都不做？），其他操作继续进行，例如，监听器保持有用

内置主题：
  - 主题的隐式变体
*/

/**
 * @def MAX_HANDLES
 * @brief 最大句柄数定义
 *
 * 最大句柄数为 INT32_MAX - 1，但由于分配器依赖于随机生成器来查找空闲句柄，
 * 因此在 dds_handle_create 中花费的时间会随着句柄数量的增加而增加。16M 句柄似乎足够使用，
 * 并使分配新句柄的可能成本更加合理。
 */
#define MAX_HANDLES (INT32_MAX / 128)

/**
 * @struct dds_handle_server
 * @brief 定义名为 "dds_handle_server" 的结构体，包含四个成员变量
 */
struct dds_handle_server {
  struct ddsrt_hh *ht;  ///< @brief ht 是指向哈希表的指针
  size_t count;         ///< @brief count 是哈希表中元素的总数
  ddsrt_mutex_t lock;   ///< @brief lock 是用于同步的互斥锁
  ddsrt_cond_t cond;    ///< @brief cond 是用于同步的条件变量
};

// 声明一个名为 "handles" 的静态全局变量，类型为 "dds_handle_server"
static struct dds_handle_server handles;

/**
 * @brief 定义名为 "handle_hash" 的函数，接受一个 const void 指针作为参数并返回一个无符号整数
 * @param va 传入的 const void 指针
 * @return 返回用作哈希键的句柄
 */
static uint32_t handle_hash(const void *va) {
  // 将 void 指针强制转换为名为 "a" 的结构体指针
  const struct dds_handle_link *a = va;
  /* 句柄已经是伪随机数，所以没有必要再次进行哈希 */
  return (uint32_t)a->hdl;  // 返回句柄作为哈希键
}

/**
 * @brief 定义名为 "handle_equal" 的函数，接受两个 const void 指针作为参数并返回一个整数
 * @param va 传入的第一个 const void 指针
 * @param vb 传入的第二个 const void 指针
 * @return 如果 a 和 b 的句柄匹配，则返回 1；如果不匹配，则返回 0
 */
static int handle_equal(const void *va, const void *vb) {
  // 将 void 指针强制转换为名为 "a" 和 "b" 的结构体指针
  const struct dds_handle_link *a = va;
  const struct dds_handle_link *b = vb;

  // 比较 a 和 b 的句柄，如果匹配则返回 1，否则返回 0
  return a->hdl == b->hdl;
}

/**
 * @brief 初始化句柄服务器
 *
 * 在持有 ddsrt 的单例互斥锁的情况下调用（请参阅 dds_init/fini）
 *
 * @return 返回码，表示操作成功
 */
dds_return_t dds_handle_server_init(void) {
  // 如果句柄哈希表为空
  if (handles.ht == NULL) {
    // 创建哈希表
    handles.ht = ddsrt_hh_new(128, handle_hash, handle_equal);
    // 初始化句柄计数
    handles.count = 0;
    // 初始化互斥锁
    ddsrt_mutex_init(&handles.lock);
    // 初始化条件变量
    ddsrt_cond_init(&handles.cond);
  }
  // 返回码，表示操作成功
  return DDS_RETCODE_OK;
}

/**
 * @brief 终止句柄服务器
 *
 * 在持有 ddsrt 的单例互斥锁的情况下调用（请参阅 dds_init/fini）
 */
void dds_handle_server_fini(void) {
  // 如果哈希表不为空
  if (handles.ht != NULL) {
#ifndef NDEBUG  // 如果在非 Debug 模式下编译
    struct ddsrt_hh_iter it;
    // 遍历哈希表中的元素
    for (struct dds_handle_link *link = ddsrt_hh_iter_first(handles.ht, &it); link != NULL;
         link = ddsrt_hh_iter_next(&it)) {
      // 读取标志位并返回其值
      uint32_t cf = ddsrt_atomic_ld32(&link->cnt_flags);
      // 输出错误信息
      DDS_ERROR("handle %" PRId32 " pin %" PRIu32 " refc %" PRIu32 "%s%s%s\n", link->hdl,
                cf & HDL_PINCOUNT_MASK, (cf & HDL_REFCOUNT_MASK) >> HDL_REFCOUNT_SHIFT,
                cf & HDL_FLAG_PENDING ? " pending" : "", cf & HDL_FLAG_CLOSING ? " closing" : "",
                cf & HDL_FLAG_DELETE_DEFERRED ? " delete-deferred" : "");
    }
    // 断言哈希表中是否存在元素
    assert(ddsrt_hh_iter_first(handles.ht, &it) == NULL);
#endif
    // 释放哈希表的内存
    ddsrt_hh_free(handles.ht);
    // 销毁条件变量
    ddsrt_cond_destroy(&handles.cond);
    // 销毁互斥锁
    ddsrt_mutex_destroy(&handles.lock);
    // 将哈希表置为空
    handles.ht = NULL;
  }
}

// 该函数将一个元素添加到哈希表中
static bool hhadd(struct ddsrt_hh *ht, void *elem) { return ddsrt_hh_add(ht, elem); }

/*
 * 该函数用于创建一个新的 DDS 句柄。
 *
 * 参数说明：
 * @link: 新句柄链接，包括关联数据
 * @implicit: 是否为隐式句柄
 * @allow_children: 句柄是否允许有子句柄
 * @user_access: 句柄是否可由用户访问
 *
 * 返回值：
 * 返回新句柄。如果无法创建句柄，则返回错误代码 DDS_RETCODE_OUT_OF_RESOURCES。
 */
dds_handle_t dds_handle_create(struct dds_handle_link *link,
                               bool implicit,
                               bool allow_children,
                               bool user_access) {
  dds_handle_t ret;
  // 加锁保证线程安全
  ddsrt_mutex_lock(&handles.lock);
  if (handles.count == MAX_HANDLES)  // 检查是否超过最大句柄数
  {
    // 解锁并返回错误代码
    ddsrt_mutex_unlock(&handles.lock);
    ret = DDS_RETCODE_OUT_OF_RESOURCES;
  } else {
    handles.count++;  // 原子性地增加句柄计数器
    // 创建新句柄
    ret = dds_handle_create_int(link, implicit, allow_children, user_access);
    // 解锁
    ddsrt_mutex_unlock(&handles.lock);
    assert(ret > 0);  // 确保返回值大于0（正常情况下 DDS 句柄是正整数）
  }
  return ret;
}

/*
 * 该函数将用于创建 DDS 句柄的内部实现。
 *
 * 参数说明：
 * @link: 新句柄链接，包括关联数据
 * @implicit: 是否为隐式句柄
 * @refc_counts_children: 句柄是否允许有子句柄进行引用计数
 * @user_access: 句柄是否可由用户访问
 *
 * 返回值：
 * 返回新句柄。如果无法创建句柄，则返回错误代码 DDS_RETCODE_OUT_OF_RESOURCES。
 */
static dds_handle_t dds_handle_create_int(struct dds_handle_link *link,
                                          bool implicit,
                                          bool refc_counts_children,
                                          bool user_access) {
  uint32_t flags = HDL_FLAG_PENDING;                          // 初始化标志
  flags |= implicit ? HDL_FLAG_IMPLICIT : HDL_REFCOUNT_UNIT;  // 设置隐式或显式句柄标志
  flags |= refc_counts_children ? HDL_FLAG_ALLOW_CHILDREN
                                : 0;  // 设置句柄是否允许有子句柄进行引用计数的标志
  flags |= user_access ? 0 : HDL_FLAG_NO_USER_ACCESS;  // 设置句柄是否可由用户访问的标志
  ddsrt_atomic_st32(&link->cnt_flags, flags | 1u);     // 将状态和计数初始化为1
  do {
    do {
      link->hdl = (int32_t)(ddsrt_random() & INT32_MAX);  // 生成一个伪随机句柄号
    } while (link->hdl == 0 ||
             link->hdl >= DDS_MIN_PSEUDO_HANDLE);  // 确保随机句柄号不为0且不小于最小值
  } while (!hhadd(handles.ht, link));  // 将新的句柄链接加入哈希表中，直到成功为止
  return link->hdl;                    // 返回新句柄
}

dds_return_t dds_handle_register_special(struct dds_handle_link *link,
                                         bool implicit,
                                         bool allow_children,
                                         dds_handle_t handle) {
  dds_return_t ret;                    // 定义返回值变量ret
  if (handle <= 0)                     // 如果参数handle小于或等于0
    return DDS_RETCODE_BAD_PARAMETER;  // 返回“错误参数”的错误代码

  ddsrt_mutex_lock(
      &handles.lock);  // 使用API函数ddsrt_mutex_lock()锁定互斥对象（在此为handles.lock）

  if (handles.count == MAX_HANDLES)     // 如果已经储存的句柄数量已经达到最大限制
  {
    ddsrt_mutex_unlock(&handles.lock);  // 解锁互斥对象
    ret = DDS_RETCODE_OUT_OF_RESOURCES;  // 返回“资源不足”的错误代码
  } else                                 // 否则
  {
    handles.count++;                     // 句柄数量加一
    ddsrt_atomic_st32(&link->cnt_flags, HDL_FLAG_PENDING |
                                            (implicit ? HDL_FLAG_IMPLICIT : HDL_REFCOUNT_UNIT) |
                                            (allow_children ? HDL_FLAG_ALLOW_CHILDREN : 0) | 1u);
    // 设置link->cnt_flags，包括三个标志位：HDL_FLAG_PENDING(表示该句柄未被处理完)、implicit(表示是否是隐含的)、allow_children(表示是否允许有下级结构成员)和一个无符号整型常量1u。
    link->hdl = handle;                 // 将handle赋值给link指针所指向的句柄

    if (hhadd(handles.ht, link))        // 如果在句柄表中添加成功
      ret = handle;                     // 返回句柄
    else
      ret = DDS_RETCODE_BAD_PARAMETER;  // 否则返回“错误参数”的错误代码

    ddsrt_mutex_unlock(&handles.lock);  // 解锁互斥对象
    assert(ret > 0);  // 检查返回的句柄是否大于0，若不是，则终止程序并打印错误信息。
  }
  return ret;  // 返回ret变量（即句柄）
}

// 定义一个函数，函数名为dds_handle_unpend，参数类型为指向struct dds_handle_link结构体的指针link
void dds_handle_unpend(struct dds_handle_link *link) {
  // 如果定义了NDEBUG宏，则执行以下代码
#ifndef NDEBUG
  // 将指向link中cnt_flags成员的无符号32位整型指针cf初始化为cnt_flags的值
  uint32_t cf = ddsrt_atomic_ld32(&link->cnt_flags);
  // 确保HDL_FLAG_PENDING位被设置
  assert((cf & HDL_FLAG_PENDING));
  // 确保HDL_FLAG_DELETE_DEFERRED位没有被设置
  assert(!(cf & HDL_FLAG_DELETE_DEFERRED));
  // 确保HDL_FLAG_CLOSING位没有被设置
  assert(!(cf & HDL_FLAG_CLOSING));
  // 确保HDL_REFCOUNT_MASK位中的值大于或等于HDL_REFCOUNT_UNIT的值或者HDL_FLAG_IMPLICIT位被设置
  assert((cf & HDL_REFCOUNT_MASK) >= HDL_REFCOUNT_UNIT || (cf & HDL_FLAG_IMPLICIT));
  // 确保HDL_PINCOUNT_MASK位中的值大于或等于1
  assert((cf & HDL_PINCOUNT_MASK) >= 1u);
#endif
  // 将cnt_flags中的HDL_FLAG_PENDING位清空
  ddsrt_atomic_and32(&link->cnt_flags, ~HDL_FLAG_PENDING);
  // 调用dds_handle_unpin函数，传入link作为参数
  dds_handle_unpin(link);
}

// 定义一个函数，函数名为dds_handle_delete，参数类型为指向struct dds_handle_link结构体的指针link
int32_t dds_handle_delete(struct dds_handle_link *link) {
  // 如果定义了NDEBUG宏，则执行以下代码
#ifndef NDEBUG
  // 将指向link中cnt_flags成员的无符号32位整型指针cf初始化为cnt_flags的值
  uint32_t cf = ddsrt_atomic_ld32(&link->cnt_flags);
  // 如果HDL_FLAG_PENDING位没有被设置
  if (!(cf & HDL_FLAG_PENDING)) {
    // 确保HDL_FLAG_CLOSING位已经被设置
    assert(cf & HDL_FLAG_CLOSING);
    // 确保HDL_REFCOUNT_MASK位中的值等于0
    assert((cf & HDL_REFCOUNT_MASK) == 0u);
  }
  // 确保HDL_PINCOUNT_MASK位中的值等于1
  assert((cf & HDL_PINCOUNT_MASK) == 1u);
#endif
  // 获取锁
  ddsrt_mutex_lock(&handles.lock);
  // 从哈希表中移除当前链表link
  ddsrt_hh_remove_present(handles.ht, link);
  // 确保handles.count的值大于0
  assert(handles.count > 0);
  // 将count减去1
  handles.count--;
  // 释放锁
  ddsrt_mutex_unlock(&handles.lock);
  // 返回DDS_RETCODE_OK
  return DDS_RETCODE_OK;
}

// 定义一个静态函数，返回类型为int32_t，函数名为dds_handle_pin_int，参数列表包括hdl、delta、from_user和link
static int32_t dds_handle_pin_int(dds_handle_t hdl,
                                  uint32_t delta,
                                  bool from_user,
                                  struct dds_handle_link **link) {
  // 定义一个结构体变量dummy，其中成员.hdl的值为传入的参数hdl
  struct dds_handle_link dummy = {.hdl = hdl};
  int32_t rc;
  /*在此处检查初始化是有意义的：任何操作（create_participant之外）的第一件事情就是调用dds_handle_pin操作，
  因此在这里检查库是否被初始化有助于避免程序崩溃（如果有人忘记创建一个参与者或者允许程序在未成功创建参与者之后继续运行）。
  可以检查句柄是否大于0，但这会捕获更少的错误而没有任何优势。*/
  if (handles.ht == NULL) return DDS_RETCODE_PRECONDITION_NOT_MET;

  // 使用ddsrt_mutex_lock函数对handles.lock加锁
  ddsrt_mutex_lock(&handles.lock);

  // 通过dds_handle_lookup函数查找关联link并将结果赋值给*link
  *link = ddsrt_hh_lookup(handles.ht, &dummy);

  // 判断*link是否为空
  if (*link == NULL)
    rc = DDS_RETCODE_BAD_PARAMETER;
  else {
    uint32_t cf;
    // 假定成功；如果该对象实际上正在被删除，则退出
    rc = DDS_RETCODE_OK;

    // 执行循环语句，检查(*link)->cnt_flags的值
    do {
      cf = ddsrt_atomic_ld32(&(*link)->cnt_flags);
      if (cf & (HDL_FLAG_CLOSING | HDL_FLAG_PENDING | HDL_FLAG_NO_USER_ACCESS)) {
        if (cf & (HDL_FLAG_CLOSING | HDL_FLAG_PENDING)) {
          rc = DDS_RETCODE_BAD_PARAMETER;
          break;
        } else if (from_user) {
          rc = DDS_RETCODE_BAD_PARAMETER;
          break;
        }
      }
    } while (!ddsrt_atomic_cas32(&(*link)->cnt_flags, cf, cf + delta));
  }

  // 使用ddsrt_mutex_unlock函数对handles.lock解锁
  ddsrt_mutex_unlock(&handles.lock);

  // 返回rc值
  return rc;
}
// dds_handle_pin函数，接受dds_handle_t类型参数hdl和struct dds_handle_link**类型参数link。
// 返回一个int32_t类型值。
int32_t dds_handle_pin(dds_handle_t hdl, struct dds_handle_link **link) {
  // 调用dds_handle_pin_int函数，并将1u、true和link参数传递给它。
  return dds_handle_pin_int(hdl, 1u, true, link);
}

// dds_handle_pin_with_origin函数，接受dds_handle_t类型参数hdl、bool类型参数from_user
// 和struct dds_handle_link**类型参数link。
// 返回一个int32_t类型值。
int32_t dds_handle_pin_with_origin(dds_handle_t hdl,
                                   bool from_user,
                                   struct dds_handle_link **link) {
  // 调用dds_handle_pin_int函数，并将1u、from_user和link参数传递给它。
  return dds_handle_pin_int(hdl, 1u, from_user, link);
}

// dds_handle_pin_for_delete函数，接受dds_handle_t类型参数hdl、bool类型参数explicit、from_user
// 和struct dds_handle_link**类型参数link。
// 返回一个int32_t类型值。
int32_t dds_handle_pin_for_delete(dds_handle_t hdl,
                                  bool explicit,
                                  bool from_user,
                                  struct dds_handle_link **link) {
  // 创建一个dds_handle_link类型的变量dummy，并设置它的hdl成员为参数hdl的值。
  struct dds_handle_link dummy = {.hdl = hdl};
  int32_t rc;

  // 检查handles.ht是否为NULL，如果是则返回DDS_RETCODE_PRECONDITION_NOT_MET。
  if (handles.ht == NULL) return DDS_RETCODE_PRECONDITION_NOT_MET;

  // 加锁，以保证访问链表时不受到其他线程的影响。
  ddsrt_mutex_lock(&handles.lock);

  // 在句柄链接列表中查找与dummy匹配的句柄链接。将结果赋值给参数link。
  *link = ddsrt_hh_lookup(handles.ht, &dummy);

  // 如果link是NULL，则返回DDS_RETCODE_BAD_PARAMETER。
  if (*link == NULL)
    rc = DDS_RETCODE_BAD_PARAMETER;
  else {
    uint32_t cf, cf1;

    // 假设操作成功；如果对象正在删除过程中，则退出操作。
    do {
      // 读取cnt_flags值并将其保存在cf变量中。
      cf = ddsrt_atomic_ld32(&(*link)->cnt_flags);

      // 如果from_user为真且(cf &
      // HDL_FLAG_NO_USER_ACCESS)为真，则用户无法删除该句柄，直接返回DDS_RETCODE_BAD_PARAMETER。
      if (from_user && (cf & (HDL_FLAG_NO_USER_ACCESS))) {
        rc = DDS_RETCODE_BAD_PARAMETER;
        break;
      }
      // 如果(cf & (HDL_FLAG_CLOSING |
      // HDL_FLAG_PENDING))为真，则已经有其他线程在关闭或等待关闭该句柄，直接返回DDS_RETCODE_BAD_PARAMETER。
      else if (cf & (HDL_FLAG_CLOSING | HDL_FLAG_PENDING)) {
        rc = DDS_RETCODE_BAD_PARAMETER;
        break;
      }
      // 如果(cf &
      // HDL_FLAG_DELETE_DEFERRED)为真，则已经有其他线程正在删除该句柄，但是因为该句柄仍有未关闭的对象所以此次删除操作被推迟。如果(cf
      // & HDL_REFCOUNT_MASK)为真，则返回DDS_RETCODE_ALREADY_DELETED；否则，将cf +
      // 1，设置HDL_FLAG_CLOSING位，并将结果保存在cf1中。
      else if (cf & HDL_FLAG_DELETE_DEFERRED) {
        assert(!(cf & HDL_FLAG_ALLOW_CHILDREN));
        if (cf & HDL_REFCOUNT_MASK) {
          rc = DDS_RETCODE_ALREADY_DELETED;
          break;
        } else {
          cf1 = (cf + 1u) | HDL_FLAG_CLOSING;
        }
      }
      // 如果explicit为真，则表明这是一次显式的调用dds_delete函数来删除该句柄本身或其父级。如果(cf &
      // HDL_FLAG_IMPLICIT)为真，则说明该句柄是由库自动创建的、使用的，而非用户创建的，因此不需要引用
      else if (explicit)  // 如果是显式调用
      {
        /* 显式调用dds_delete（可以是应用程序或父项删除其子项）*/
        if (cf & HDL_FLAG_IMPLICIT)  // 如果实体隐式，那么句柄不保留引用计数
        {
          cf1 = (cf + 1u) | HDL_FLAG_CLOSING;    // 设置标记表明实体正在关闭
        } else                                   // 否则
        {
          assert((cf & HDL_REFCOUNT_MASK) > 0);  // 断言是否大于0
          if ((cf & HDL_REFCOUNT_MASK) ==
              HDL_REFCOUNT_UNIT)  // 如果引用计数为1，表示最后一个引用正在关闭
          {
            cf1 = (cf - HDL_REFCOUNT_UNIT + 1u) |
                  HDL_FLAG_CLOSING;  // 先将对实体的引用计数加1并设置标志，在给cf1赋值
          } else if (!(cf &
                       HDL_FLAG_ALLOW_CHILDREN))  // 如果refcnt中不包含子项，则表示实体的关闭被推迟
          {
            cf1 =
                (cf - HDL_REFCOUNT_UNIT) | HDL_FLAG_DELETE_DEFERRED;  // 设置标记表明实体关闭被推迟
          } else  // 实体是显式的，因此句柄持有引用，refc仅计算子项，所以这不是我们关心的东西
          {
            cf1 = (cf - HDL_REFCOUNT_UNIT + 1u) |
                  HDL_FLAG_CLOSING;  // 先将对实体的引用计数加1并设置标志，在给cf1赋值
          }
        }
      } else  // 否则为隐式调用dds_delete（子项调用其父项上的删除）
      {
        if (cf & HDL_FLAG_IMPLICIT)  // 如果实体是隐式的，则其句柄具有引用计数
        {
          assert((cf & HDL_REFCOUNT_MASK) > 0);  // 断言是否大于0
          if ((cf & HDL_REFCOUNT_MASK) ==
              HDL_REFCOUNT_UNIT)  // 如果引用计数为1，表示最后一个引用正在关闭
          {
            cf1 = (cf - HDL_REFCOUNT_UNIT + 1u) |
                  HDL_FLAG_CLOSING;  // 先将对实体的引用计数加1并设置标志，在给cf1赋值
          } else if (!(cf &
                       HDL_FLAG_ALLOW_CHILDREN))  // 如果refcnt中不包含子项，则表示实体的关闭被推迟
          {
            cf1 =
                (cf - HDL_REFCOUNT_UNIT) | HDL_FLAG_DELETE_DEFERRED;  // 设置标记表明实体关闭被推迟
          } else                                                      // 否则
          {
            cf1 = (cf - HDL_REFCOUNT_UNIT);    // 仅减少子项引用计数一次
          }
        } else                                 // 子项不能删除显式父项
        {
          rc = DDS_RETCODE_ILLEGAL_OPERATION;  // 操作非法
          break;
        }
      }

      rc =
          ((cf1 & HDL_REFCOUNT_MASK) == 0 || (cf1 & HDL_FLAG_ALLOW_CHILDREN))
              ? DDS_RETCODE_OK
              : DDS_RETCODE_TRY_AGAIN;  // 如果新的引用计数为零或存在子项，则返回DDS_RETCODE_OK，否则返回DDS_RETCODE_TRY_AGAIN
    } while (!ddsrt_atomic_cas32(&(*link)->cnt_flags, cf, cf1));
  }
  ddsrt_mutex_unlock(&handles.lock);
  return rc;
}

// 该函数用于处理删除一个子引用并锁定它的父对象
// 参数 link：指向需要被删除子引用的 dds_handle_link 结构体的指针
// 参数 may_delete_parent：控制删除操作是否可能同时也删除父对象
bool dds_handle_drop_childref_and_pin(struct dds_handle_link *link, bool may_delete_parent) {
  // 初始化删除父对象的标志位
  bool del_parent = false;

  // 加锁，以保证在并发场景下修改计数器和标志位的原子性
  ddsrt_mutex_lock(&handles.lock);

  // 定义两个计数器，cf1 是 cf 减去 HDL_REFCOUNT_UNIT 的结果
  uint32_t cf, cf1;

  do {
    // 获取原子变量中的值（拿到状态和引用计数）
    cf = ddsrt_atomic_ld32(&link->cnt_flags);

    // 如果正在关闭或等待关闭，则子引用还未被移除
    if (cf & (HDL_FLAG_CLOSING | HDL_FLAG_PENDING)) {
      /* 只有一个线程能抢到锁, 所以只有它可以成功的做完这个任务；子引用仍将被移除 */
      assert((cf & HDL_REFCOUNT_MASK) > 0);
      cf1 = (cf - HDL_REFCOUNT_UNIT);
      del_parent = false;
    } else {
      // 如果是隐式父关系，则最后一个引用时删除父对象
      if (cf & HDL_FLAG_IMPLICIT) {
        /* 隐式父关系: 如果是最后一个引用且不需要锁定父节点，则删除父对象 */
        if ((cf & HDL_REFCOUNT_MASK) == HDL_REFCOUNT_UNIT && may_delete_parent) {
          cf1 = (cf - HDL_REFCOUNT_UNIT + 1u);
          del_parent = true;
        } else {
          assert((cf & HDL_REFCOUNT_MASK) > 0);
          cf1 = (cf - HDL_REFCOUNT_UNIT);
          del_parent = false;
        }
      } else {
        // 如果是显式父关系，则子不能删除父，子引用还未被移除
        assert((cf & HDL_REFCOUNT_MASK) > 0);
        cf1 = (cf - HDL_REFCOUNT_UNIT);
        del_parent = false;
      }
    }
  } while (!ddsrt_atomic_cas32(&link->cnt_flags, cf, cf1));

  // 解锁，以便其他并发访问修改计数器和标志位
  ddsrt_mutex_unlock(&handles.lock);

  // 返回删除父对象的标志位
  return del_parent;
}
// 定义一个函数，用于在给定的dds句柄上增加引用计数，并将句柄锁定到内存中以便使用，返回成功锁定的句柄。
// 参数：
//   hdl:    dds句柄
//   from_user: 标记是否来自用户的调用
//   link:   保存句柄链表节点的指针
// 返回值:
//   成功时返回已锁定的句柄；失败时返回0
int32_t dds_handle_pin_and_ref_with_origin(dds_handle_t hdl,
                                           bool from_user,
                                           struct dds_handle_link **link) {
  // 调用dds_handle_pin_int()函数并传递指定的参数
  return dds_handle_pin_int(hdl, HDL_REFCOUNT_UNIT + 1u, from_user, link);
}

// 定义一个函数，用于增加指定句柄的引用计数
// 参数：
//   link：保存句柄链表节点的指针
void dds_handle_repin(struct dds_handle_link *link) {
  // 使用原子操作对cnt_flags变量进行增量处理
  uint32_t x = ddsrt_atomic_inc32_nv(&link->cnt_flags);
  (void)x;  // 防止编译器发出未使用变量的警告信息
}

// 定义一个函数，用于减少指定句柄的引用计数，并在必要时唤醒等待该句柄的线程
// 参数：
//   link: 保存句柄链表节点的指针
void dds_handle_unpin(struct dds_handle_link *link) {
  // 如果该句柄的状态为正在关闭，则检查pincount是否大于1，否则检查pincount是否大于等于1
#ifndef NDEBUG
  uint32_t cf = ddsrt_atomic_ld32(&link->cnt_flags);
  if (cf & HDL_FLAG_CLOSING)
    assert((cf & HDL_PINCOUNT_MASK) > 1u);
  else
    assert((cf & HDL_PINCOUNT_MASK) >= 1u);
#endif

  // 使用互斥锁对句柄进行加锁，以防止其他线程访问该句柄
  ddsrt_mutex_lock(&handles.lock);

  // 使用原子操作对cnt_flags变量进行减量处理，然后检查句柄的状态是否为“正在关闭”并且引用计数是否为1
  if ((ddsrt_atomic_dec32_nv(&link->cnt_flags) & (HDL_FLAG_CLOSING | HDL_PINCOUNT_MASK)) ==
      (HDL_FLAG_CLOSING | 1u)) {
    // 如果是，则发送一个唤醒信号，以便等待该句柄的线程可以恢复执行
    ddsrt_cond_broadcast(&handles.cond);
  }

  // 解锁句柄
  ddsrt_mutex_unlock(&handles.lock);
}

// 定义一个函数，用于增加指定句柄的引用计数
// 参数：
//   link: 保存句柄链表节点的指针
void dds_handle_add_ref(struct dds_handle_link *link) {
  // 使用原子加法操作增加引用计数
  ddsrt_atomic_add32(&link->cnt_flags, HDL_REFCOUNT_UNIT);
}

bool dds_handle_drop_ref(struct dds_handle_link *link) {
  // 读取原子变量中的值
  uint32_t old, new;
  do {
    old = ddsrt_atomic_ld32(&link->cnt_flags);
    // 检查引用计数是否大于0
    assert((old & HDL_REFCOUNT_MASK) > 0);
    // 将引用计数减1
    new = old - HDL_REFCOUNT_UNIT;
  } while (!ddsrt_atomic_cas32(&link->cnt_flags, old, new));
  // 加锁
  ddsrt_mutex_lock(&handles.lock);
  // 检查标志位和引用计数是否符合要求
  if ((new &(HDL_FLAG_CLOSING | HDL_PINCOUNT_MASK)) == (HDL_FLAG_CLOSING | 1u)) {
    // 广播信号
    ddsrt_cond_broadcast(&handles.cond);
  }
  // 解锁
  ddsrt_mutex_unlock(&handles.lock);
  // 检查引用计数是否为0
  return ((new &HDL_REFCOUNT_MASK) == 0);
}

bool dds_handle_unpin_and_drop_ref(struct dds_handle_link *link) {
  // 读取原子变量中的值
  uint32_t old, new;
  do {
    old = ddsrt_atomic_ld32(&link->cnt_flags);
    // 检查引用计数是否大于0
    assert((old & HDL_REFCOUNT_MASK) > 0);
    // 检查固定计数是否大于0
    assert((old & HDL_PINCOUNT_MASK) > 0);
    // 将引用计数和固定计数分别减1
    new = old - HDL_REFCOUNT_UNIT - 1u;
  } while (!ddsrt_atomic_cas32(&link->cnt_flags, old, new));
  // 加锁
  ddsrt_mutex_lock(&handles.lock);
  // 检查标志位和引用计数是否符合要求
  if ((new &(HDL_FLAG_CLOSING | HDL_PINCOUNT_MASK)) == (HDL_FLAG_CLOSING | 1u)) {
    // 广播信号
    ddsrt_cond_broadcast(&handles.cond);
  }
  // 解锁
  ddsrt_mutex_unlock(&handles.lock);
  // 检查引用计数是否为0
  return ((new &HDL_REFCOUNT_MASK) == 0);
}

bool dds_handle_close(struct dds_handle_link *link) {
  // 将标志位置为关闭状态
  uint32_t old = ddsrt_atomic_or32_ov(&link->cnt_flags, HDL_FLAG_CLOSING);
  // 检查引用计数是否为0
  return (old & HDL_REFCOUNT_MASK) == 0;
}

void dds_handle_close_wait(struct dds_handle_link *link) {
// 如果不是调试模式，则加载link->cnt_flags的值
#ifndef NDEBUG
  uint32_t cf = ddsrt_atomic_ld32(&link->cnt_flags);
  // 断言cf中包含HDL_FLAG_CLOSING标志
  assert((cf & HDL_FLAG_CLOSING));
  // 断言cf中的HDL_PINCOUNT_MASK大于等于1
  assert((cf & HDL_PINCOUNT_MASK) >= 1u);
#endif
  // 加锁
  ddsrt_mutex_lock(&handles.lock);
  // 循环检查link->cnt_flags中的HDL_PINCOUNT_MASK是否等于1
  while ((ddsrt_atomic_ld32(&link->cnt_flags) & HDL_PINCOUNT_MASK) != 1u)
    // 等待条件变量handles.cond，并且保持handles.lock的锁定
    ddsrt_cond_wait(&handles.cond, &handles.lock);
  // 只有一个线程可以在给定的句柄上调用close_wait
  ddsrt_mutex_unlock(&handles.lock);
}

// 检查link->cnt_flags中的HDL_REFCOUNT_MASK是否等于0
bool dds_handle_is_not_refd(struct dds_handle_link *link) {
  return ((ddsrt_atomic_ld32(&link->cnt_flags) & HDL_REFCOUNT_MASK) == 0);
}

// 检查link->cnt_flags中的HDL_FLAG_CLOSED标志是否被设置
extern inline bool dds_handle_is_closed(struct dds_handle_link *link);
