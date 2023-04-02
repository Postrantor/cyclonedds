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
#include <string.h>

#include "dds/ddsi/ddsi_endpoint.h"
#include "dds/ddsi/ddsi_entity.h"
#include "dds/ddsi/ddsi_pmd.h"
#include "dds/ddsi/ddsi_sertype.h"
#include "dds/ddsi/ddsi_transmit.h"
#include "dds/ddsi/ddsi_xqos.h"
#include "dds/ddsrt/heap.h"
#include "dds/ddsrt/log.h"
#include "dds/version.h"
#include "dds__builtin.h"
#include "dds__entity.h"
#include "dds__listener.h"
#include "dds__qos.h"
#include "dds__reader.h"
#include "dds__subscriber.h" // for non-materialized DATA_ON_READERS
#include "dds__topic.h"
#include "dds__write.h"
#include "dds__writer.h"

/**
 * @brief 从dds_handle_link结构体中获取dds_entity指针
 *
 * @param hdllink 指向dds_handle_link结构体的指针
 * @return 返回指向dds_entity的指针
 */
extern inline dds_entity *dds_entity_from_handle_link(struct dds_handle_link *hdllink);

/**
 * @brief 判断dds_entity是否启用
 *
 * @param e 指向dds_entity的常量指针
 * @return 如果启用则返回true，否则返回false
 */
extern inline bool dds_entity_is_enabled(const dds_entity *e);

/**
 * @brief 重置dds_entity的状态
 *
 * @param e 指向dds_entity的指针
 * @param t 要重置的状态掩码
 */
extern inline void dds_entity_status_reset(dds_entity *e, status_mask_t t);

/**
 * @brief 重置dds_entity的状态，并返回旧的状态值
 *
 * @param e 指向dds_entity的指针
 * @param t 要重置的状态掩码
 * @return 返回重置前的状态值
 */
extern inline uint32_t dds_entity_status_reset_ov(dds_entity *e, status_mask_t t);

/**
 * @brief 获取dds_entity的类型
 *
 * @param e 指向dds_entity的常量指针
 * @return 返回dds_entity的类型
 */
extern inline dds_entity_kind_t dds_entity_kind(const dds_entity *e);

/**
 * @file
 * @brief 该文件定义了一个名为 dds_entity_deriver_table 的数组，用于存储不同类型的 DDS 实体派生器。
 */

// 定义一个常量结构体指针数组，用于存储不同类型的 DDS 实体派生器
const struct dds_entity_deriver *dds_entity_deriver_table[] = {
    [DDS_KIND_TOPIC] = &dds_entity_deriver_topic,               ///< 主题实体派生器
    [DDS_KIND_PARTICIPANT] = &dds_entity_deriver_participant,   ///< 参与者实体派生器
    [DDS_KIND_READER] = &dds_entity_deriver_reader,             ///< 读取器实体派生器
    [DDS_KIND_WRITER] = &dds_entity_deriver_writer,             ///< 写入器实体派生器
    [DDS_KIND_SUBSCRIBER] = &dds_entity_deriver_subscriber,     ///< 订阅者实体派生器
    [DDS_KIND_PUBLISHER] = &dds_entity_deriver_publisher,       ///< 发布者实体派生器
    [DDS_KIND_COND_READ] = &dds_entity_deriver_readcondition,   ///< 读取条件实体派生器
    [DDS_KIND_COND_QUERY] = &dds_entity_deriver_readcondition,  ///< 查询条件实体派生器
    [DDS_KIND_COND_GUARD] = &dds_entity_deriver_guardcondition, ///< 保护条件实体派生器
    [DDS_KIND_WAITSET] = &dds_entity_deriver_waitset,           ///< 等待集实体派生器
    [DDS_KIND_DOMAIN] = &dds_entity_deriver_domain,             ///< 域实体派生器
    [DDS_KIND_CYCLONEDDS] = &dds_entity_deriver_cyclonedds      ///< CycloneDDS 实体派生器
};

/**
 * @brief 虚拟中断函数
 *
 * @param[in] e 指向实体结构的指针
 */
void dds_entity_deriver_dummy_interrupt(struct dds_entity *e)
{
  (void)e; // 忽略未使用的参数
}

/**
 * @brief 虚拟关闭函数
 *
 * @param[in] e 指向实体结构的指针
 */
void dds_entity_deriver_dummy_close(struct dds_entity *e)
{
  (void)e; // 忽略未使用的参数
}

/**
 * @brief 虚拟删除函数
 *
 * @param[in] e 指向实体结构的指针
 * @return 返回操作结果，这里总是返回DDS_RETCODE_OK
 */
dds_return_t dds_entity_deriver_dummy_delete(struct dds_entity *e)
{
  (void)e; // 忽略未使用的参数
  return DDS_RETCODE_OK;
}

/**
 * @brief 虚拟设置QoS函数
 *
 * @param[in] e 指向实体结构的指针
 * @param[in] qos 指向QoS结构的指针
 * @param[in] enabled 布尔值，表示是否启用
 * @return 返回操作结果，这里总是返回DDS_RETCODE_ILLEGAL_OPERATION
 */
dds_return_t dds_entity_deriver_dummy_set_qos(struct dds_entity *e, const dds_qos_t *qos, bool enabled)
{
  (void)e;       // 忽略未使用的参数
  (void)qos;     // 忽略未使用的参数
  (void)enabled; // 忽略未使用的参数
  return DDS_RETCODE_ILLEGAL_OPERATION;
}

/**
 * @brief 虚拟验证状态函数
 *
 * @param[in] mask 状态掩码
 * @return 返回操作结果，这里总是返回DDS_RETCODE_ILLEGAL_OPERATION
 */
dds_return_t dds_entity_deriver_dummy_validate_status(uint32_t mask)
{
  (void)mask; // 忽略未使用的参数
  return DDS_RETCODE_ILLEGAL_OPERATION;
}

/**
 * @brief 虚拟创建统计信息函数
 *
 * @param[in] e 指向实体结构的指针
 * @return 返回指向统计信息结构的指针，这里总是返回NULL
 */
struct dds_statistics *dds_entity_deriver_dummy_create_statistics(const struct dds_entity *e)
{
  (void)e; // 忽略未使用的参数
  return NULL;
}

/**
 * @brief 虚拟刷新统计信息函数
 *
 * @param[in] e 指向实体结构的指针
 * @param[in] s 指向统计信息结构的指针
 */
void dds_entity_deriver_dummy_refresh_statistics(const struct dds_entity *e, struct dds_statistics *s)
{
  (void)e; // 忽略未使用的参数
  (void)s; // 忽略未使用的参数
}

extern inline void dds_entity_deriver_interrupt(struct dds_entity *e);
extern inline void dds_entity_deriver_close(struct dds_entity *e);
extern inline dds_return_t dds_entity_deriver_delete(struct dds_entity *e);
extern inline dds_return_t dds_entity_deriver_set_qos(struct dds_entity *e, const dds_qos_t *qos, bool enabled);
extern inline dds_return_t dds_entity_deriver_validate_status(struct dds_entity *e, uint32_t mask);
extern inline bool dds_entity_supports_set_qos(struct dds_entity *e);
extern inline bool dds_entity_supports_validate_status(struct dds_entity *e);
extern inline struct dds_statistics *dds_entity_deriver_create_statistics(const struct dds_entity *e);
extern inline void dds_entity_deriver_refresh_statistics(const struct dds_entity *e, struct dds_statistics *s);

/**
 * @brief 比较两个实例句柄的函数
 *
 * @param[in] va 第一个实例句柄的指针
 * @param[in] vb 第二个实例句柄的指针
 * @return int 返回0表示相等，返回-1表示a小于b，返回1表示a大于b
 */
static int compare_instance_handle(const void *va, const void *vb)
{
  // 将输入参数转换为dds_instance_handle_t类型的指针
  const dds_instance_handle_t *a = va;
  const dds_instance_handle_t *b = vb;

  // 比较两个实例句柄的值并返回结果
  return (*a == *b) ? 0 : (*a < *b) ? -1
                                    : 1;
}

// 定义dds_entity_children_td变量
const ddsrt_avl_treedef_t dds_entity_children_td = DDSRT_AVL_TREEDEF_INITIALIZER(offsetof(struct dds_entity, m_avlnode_child), offsetof(struct dds_entity, m_iid), compare_instance_handle, 0);

// 声明dds_entity_observers_signal_delete函数
static void dds_entity_observers_signal_delete(dds_entity *observed);

// 声明dds_delete_impl函数
static dds_return_t dds_delete_impl(dds_entity_t entity, enum delete_impl_state delstate);

// 声明really_delete_pinned_closed_locked函数
static dds_return_t really_delete_pinned_closed_locked(struct dds_entity *e, enum delete_impl_state delstate);

/**
 * @brief 判断实体是否为内置主题
 *
 * @param[in] entity 要判断的实体指针
 * @return bool 返回true表示是内置主题，返回false表示不是内置主题
 */
static bool entity_is_builtin_topic(const struct dds_entity *entity)
{
  // 判断实体类型是否为DDS_KIND_TOPIC
  if (dds_entity_kind(entity) != DDS_KIND_TOPIC)
    return false;
  else
  {
    // 将实体转换为dds_topic类型的指针
    const dds_topic *tp = (dds_topic *)entity;

    // 判断主题是否为内置主题并返回结果
    return ddsi_builtintopic_is_builtintopic(&tp->m_entity.m_domain->btif, tp->m_stype);
  }
}

/**
 * @brief 增加实体引用计数（线程安全）
 *
 * @param[in] e 要增加引用计数的实体指针
 */
void dds_entity_add_ref_locked(dds_entity *e)
{
  // 增加实体的引用计数
  dds_handle_add_ref(&e->m_hdllink);
}

/**
 * @brief 释放实体引用 (Drop the reference of a DDS entity)
 * @param e 指向要释放引用的实体的指针 (Pointer to the entity whose reference is to be dropped)
 */
void dds_entity_drop_ref(dds_entity *e)
{
  // 如果成功释放实体引用 (If successfully dropped the entity reference)
  if (dds_handle_drop_ref(&e->m_hdllink))
  {
    // 删除实体并检查返回值 (Delete the entity and check the return value)
    dds_return_t ret = dds_delete_impl(e->m_hdllink.hdl, DIS_EXPLICIT);
    // 断言返回值为 DDS_RETCODE_OK (Assert that the return value is DDS_RETCODE_OK)
    assert(ret == DDS_RETCODE_OK);
    // 忽略返回值 (Ignore the return value)
    (void)ret;
  }
}

/**
 * @brief 解除实体引用并释放引用 (Unpin and drop the reference of a DDS entity)
 * @param e 指向要解除引用并释放引用的实体的指针 (Pointer to the entity whose reference is to be unpinned and dropped)
 */
void dds_entity_unpin_and_drop_ref(dds_entity *e)
{
  // 如果成功解除实体引用并释放引用 (If successfully unpinned and dropped the entity reference)
  if (dds_handle_unpin_and_drop_ref(&e->m_hdllink))
  {
    // 删除实体并检查返回值 (Delete the entity and check the return value)
    dds_return_t ret = dds_delete_impl(e->m_hdllink.hdl, DIS_EXPLICIT);
    // 断言返回值为 DDS_RETCODE_OK (Assert that the return value is DDS_RETCODE_OK)
    assert(ret == DDS_RETCODE_OK);
    // 忽略返回值 (Ignore the return value)
    (void)ret;
  }
}

/**
 * @brief entity_has_status 函数用于检查实体是否具有状态。
 *
 * @param e 指向 dds_entity 结构的指针
 * @return 如果实体具有状态，则返回 true，否则返回 false
 */
static bool entity_has_status(const dds_entity *e)
{
  switch (e->m_kind)
  {
  case DDS_KIND_TOPIC:
  case DDS_KIND_READER:
  case DDS_KIND_WRITER:
  case DDS_KIND_PUBLISHER:
  case DDS_KIND_SUBSCRIBER:
  case DDS_KIND_PARTICIPANT:
    return true;
  case DDS_KIND_COND_READ:
  case DDS_KIND_COND_QUERY:
  case DDS_KIND_COND_GUARD:
  case DDS_KIND_WAITSET:
  case DDS_KIND_DOMAIN:
  case DDS_KIND_CYCLONEDDS:
    break;
  case DDS_KIND_DONTCARE:
    abort();
    break;
  }
  return false;
}

/**
 * @brief entity_may_have_children 函数用于检查实体是否可能具有子实体。
 *
 * @param e 指向 dds_entity 结构的指针
 * @return 如果实体可能具有子实体，则返回 true，否则返回 false
 */
static bool entity_may_have_children(const dds_entity *e)
{
  switch (e->m_kind)
  {
  case DDS_KIND_TOPIC:
    return false;
  case DDS_KIND_READER:
  case DDS_KIND_WRITER:
  case DDS_KIND_PUBLISHER:
  case DDS_KIND_SUBSCRIBER:
  case DDS_KIND_PARTICIPANT:
  case DDS_KIND_COND_READ:
  case DDS_KIND_COND_QUERY:
  case DDS_KIND_COND_GUARD:
  case DDS_KIND_WAITSET:
  case DDS_KIND_DOMAIN:
  case DDS_KIND_CYCLONEDDS:
    break;
  case DDS_KIND_DONTCARE:
    abort();
    break;
  }
  return true;
}

#ifndef NDEBUG
/**
 * @brief 判断实体类型是否具有QoS属性
 *
 * @param kind 实体类型枚举值
 * @return 如果实体类型具有QoS属性，则返回true，否则返回false
 */
static bool entity_kind_has_qos(dds_entity_kind_t kind)
{
  // 根据实体类型进行判断
  switch (kind)
  {
  // 如果是以下类型，具有QoS属性，返回true
  case DDS_KIND_READER:
  case DDS_KIND_WRITER:
  case DDS_KIND_PUBLISHER:
  case DDS_KIND_SUBSCRIBER:
  case DDS_KIND_PARTICIPANT:
    return true;

  // 如果是以下类型，不具有QoS属性，跳出switch语句
  case DDS_KIND_TOPIC:
  case DDS_KIND_COND_READ:
  case DDS_KIND_COND_QUERY:
  case DDS_KIND_COND_GUARD:
  case DDS_KIND_WAITSET:
  case DDS_KIND_DOMAIN:
  case DDS_KIND_CYCLONEDDS:
    break;

  // 如果是DDS_KIND_DONTCARE类型，调用abort()函数终止程序
  case DDS_KIND_DONTCARE:
    abort();
    break;
  }

  // 其他情况，返回false
  return false;
}
#endif

/**
 * @brief 初始化实体对象
 *
 * 该函数用于初始化一个实体对象，包括设置实体类型、父实体、QoS策略等。
 *
 * @param[out] e          要初始化的实体对象指针
 * @param[in]  parent      父实体对象指针，如果没有父实体则为NULL
 * @param[in]  kind        实体类型，例如DDS_KIND_CYCLONEDDS、DDS_KIND_READER等
 * @param[in]  implicit    是否为隐式实体
 * @param[in]  user_access 是否允许用户访问
 * @param[in]  qos         QoS策略指针，如果不需要QoS策略则为NULL
 * @param[in]  listener    监听器指针，如果不需要监听器则为NULL
 * @param[in]  mask        状态掩码
 * @return 返回实体句柄，如果创建失败则返回负值
 */
dds_entity_t dds_entity_init(dds_entity *e, dds_entity *parent, dds_entity_kind_t kind, bool implicit, bool user_access, dds_qos_t *qos, const dds_listener_t *listener, status_mask_t mask)
{
  dds_handle_t handle;

  /* CycloneDDS是层次结构的根 */
  assert((kind == DDS_KIND_CYCLONEDDS) == (parent == NULL));
  assert(entity_kind_has_qos(kind) == (qos != NULL));
  assert(e);

  e->m_kind = kind;          // 设置实体类型
  e->m_qos = qos;            // 设置QoS策略
  e->m_cb_count = 0;         // 初始化回调计数器
  e->m_cb_pending_count = 0; // 初始化待处理回调计数器
  e->m_observers = NULL;     // 初始化观察者列表

  /* TODO: CHAM-96: 实现实体的动态启用。 */
  e->m_flags |= DDS_ENTITY_ENABLED; // 设置实体启用标志
  if (implicit)
    e->m_flags |= DDS_ENTITY_IMPLICIT; // 设置实体隐式标志

  /* 根据实体类型设置状态使能 */
  assert(kind != DDS_KIND_READER || (mask & DDS_DATA_ON_READERS_STATUS) == 0);
  if (entity_has_status(e))
    ddsrt_atomic_st32(&e->m_status.m_status_and_mask, (uint32_t)mask << SAM_ENABLED_SHIFT);
  else
    ddsrt_atomic_st32(&e->m_status.m_trigger, 0);

  ddsrt_mutex_init(&e->m_mutex);          // 初始化互斥锁
  ddsrt_mutex_init(&e->m_observers_lock); // 初始化观察者锁
  ddsrt_cond_init(&e->m_cond);            // 初始化条件变量
  ddsrt_cond_init(&e->m_observers_cond);  // 初始化观察者条件变量

  if (parent)
  {
    e->m_parent = parent;           // 设置父实体
    e->m_domain = parent->m_domain; // 设置域
  }
  else
  {
    assert(kind == DDS_KIND_CYCLONEDDS);
    e->m_parent = NULL; // 清除父实体
    e->m_domain = NULL; // 清除域
  }
  ddsrt_avl_init(&dds_entity_children_td, &e->m_children); // 初始化子实体列表

  dds_reset_listener(&e->m_listener); // 重置监听器
  if (listener)
    dds_merge_listener(&e->m_listener, listener); // 合并监听器

  /* 特殊情况：DataReader上不存在on_data_on_readers事件。 */
  if (kind == DDS_KIND_READER)
    e->m_listener.on_data_on_readers = 0;

  if (parent)
  {
    ddsrt_mutex_lock(&parent->m_observers_lock);
    dds_inherit_listener(&e->m_listener, &parent->m_listener); // 继承父实体的监听器
    ddsrt_mutex_unlock(&parent->m_observers_lock);
  }

  if (kind == DDS_KIND_CYCLONEDDS)
  {
    if ((handle = dds_handle_register_special(&e->m_hdllink, implicit, true, DDS_CYCLONEDDS_HANDLE)) <= 0)
      return (dds_entity_t)handle;
  }
  else
  {
    /* 对于主题，refc计数读/写者；对于所有其他实体，它计数子实体（只要主题不能有子实体，我们就可以摆脱这个问题） */
    if ((handle = dds_handle_create(&e->m_hdllink, implicit, entity_may_have_children(e), user_access)) <= 0)
      return (dds_entity_t)handle;
  }

  /* 将dds_handle_t直接用作dds_entity_t。 */
  return (dds_entity_t)handle;
}

/**
 * @brief 完成dds_entity的初始化。
 *
 * 此函数将取消挂起实体的m_hdllink。
 *
 * @param[in] entity 要完成初始化的dds_entity指针。
 */
void dds_entity_init_complete(dds_entity *entity)
{
  // 取消挂起实体的m_hdllink
  dds_handle_unpend(&entity->m_hdllink);
}

/**
 * @brief 向父实体注册子实体。
 *
 * 确保父实体在其refc中跟踪子实体，否则无法添加子实体。
 *
 * @param[in] parent 父实体指针。
 * @param[in] child 子实体指针。
 */
void dds_entity_register_child(dds_entity *parent, dds_entity *child)
{
  // 父实体必须在其refc中跟踪子实体，否则无法添加子实体
  assert(ddsrt_atomic_ld32(&parent->m_hdllink.cnt_flags) & HDL_FLAG_ALLOW_CHILDREN);
  assert(child->m_iid != 0);
  assert(ddsrt_avl_lookup(&dds_entity_children_td, &parent->m_children, &child->m_iid) == NULL);

  // 将子实体插入到父实体的子实体列表中
  ddsrt_avl_insert(&dds_entity_children_td, &parent->m_children, child);

  // 增加父实体的引用计数
  dds_entity_add_ref_locked(parent);
}

/**
 * @brief 获取下一个符合条件的子实体。
 *
 * 遍历子实体列表，返回符合allowed_kinds条件的下一个子实体。
 *
 * @param[in] remaining_children 剩余子实体列表。
 * @param[in] allowed_kinds 允许的子实体类型。
 * @param[in,out] cursor 游标，用于遍历子实体列表。
 * @return 符合条件的下一个子实体指针，如果没有找到则返回NULL。
 */
static dds_entity *get_next_child(ddsrt_avl_tree_t *remaining_children, uint32_t allowed_kinds, uint64_t *cursor)
{
  ddsrt_avl_iter_t it;

  // 遍历子实体列表
  for (dds_entity *e = ddsrt_avl_iter_succ(&dds_entity_children_td, remaining_children, &it, cursor); e != NULL; e = ddsrt_avl_iter_next(&it))
  {
    dds_entity_kind_t kind = dds_entity_kind(e);

    // 检查子实体是否符合allowed_kinds条件
    if ((1u << (uint32_t)kind) & allowed_kinds)
      return e;
  }

  // 如果没有找到符合条件的子实体，则返回NULL
  return NULL;
}

/**
 * @brief 删除符合条件的子实体。
 *
 * 删除父实体中符合allowed_kinds条件的所有子实体。
 *
 * @param[in] parent 父实体指针。
 * @param[in] allowed_kinds 允许删除的子实体类型。
 */
static void delete_children(struct dds_entity *parent, uint32_t allowed_kinds)
{
  dds_entity *child;
  dds_return_t ret;
  uint64_t cursor = 0;

  // 锁定父实体的互斥锁
  ddsrt_mutex_lock(&parent->m_mutex);

  // 遍历子实体列表，删除符合条件的子实体
  while ((child = get_next_child(&parent->m_children, allowed_kinds, &cursor)) != NULL)
  {
    dds_entity_t child_handle = child->m_hdllink.hdl;
    cursor = child->m_iid;

    // 子实体将从parent->m_children列表中移除自己
    ddsrt_mutex_unlock(&parent->m_mutex);
    ret = dds_delete_impl(child_handle, DIS_FROM_PARENT);
    assert(ret == DDS_RETCODE_OK || ret == DDS_RETCODE_BAD_PARAMETER);
    (void)ret;
    ddsrt_mutex_lock(&parent->m_mutex);

    // 如果dds_delete并行删除子实体失败，则等待它完成
    if (ddsrt_avl_lookup(&dds_entity_children_td, &parent->m_children, &cursor) != NULL)
      ddsrt_cond_wait(&parent->m_cond, &parent->m_mutex);
  }

  // 解锁父实体的互斥锁
  ddsrt_mutex_unlock(&parent->m_mutex);
}

#define TRACE_DELETE 0 /* FIXME: use DDS_LOG for this */
#if TRACE_DELETE

/**
 * @brief 根据实体类型返回对应的字符串表示
 *
 * @param kind 实体类型，dds_entity_kind_t 枚举值
 * @return 对应实体类型的字符串表示
 */
static const char *entity_kindstr(dds_entity_kind_t kind)
{
  switch (kind)
  {
  case DDS_KIND_TOPIC:
    return "topic"; // 主题类型
  case DDS_KIND_READER:
    return "reader"; // 读取器类型
  case DDS_KIND_WRITER:
    return "writer"; // 写入器类型
  case DDS_KIND_PUBLISHER:
    return "publisher"; // 发布者类型
  case DDS_KIND_SUBSCRIBER:
    return "subscriber"; // 订阅者类型
  case DDS_KIND_PARTICIPANT:
    return "participant"; // 参与者类型
  case DDS_KIND_COND_READ:
    return "readcond"; // 读取条件类型
  case DDS_KIND_COND_QUERY:
    return "querycond"; // 查询条件类型
  case DDS_KIND_COND_GUARD:
    return "guardcond"; // 守护条件类型
  case DDS_KIND_WAITSET:
    return "waitset"; // 等待集类型
  case DDS_KIND_DOMAIN:
    return "domain"; // 域类型
  case DDS_KIND_CYCLONEDDS:
    return "cyclonedds"; // CycloneDDS 类型
  case DDS_KIND_DONTCARE:
    break; // 不关心的类型
  }
  return "UNDEF"; // 未定义类型
}

/**
 * @brief 打印删除操作的相关信息
 *
 * @param e 指向 dds_entity 的指针
 * @param delstate 删除操作的状态，delete_impl_state 枚举值
 * @param iid 实例句柄
 */
static void print_delete(const dds_entity *e, enum delete_impl_state delstate, dds_instance_handle_t iid)
{
  if (e)
  {
    unsigned cm = ddsrt_atomic_ld32(&e->m_hdllink.cnt_flags);
    printf("delete(%p, delstate %s, iid %" PRIx64 "): %s%s %d pin %u refc %u %s %s\n",
           (void *)e, (delstate == DIS_IMPLICIT) ? "implicit" : (delstate == DIS_EXPLICIT) ? "explicit"
                                                                                           : "from_parent",
           iid,
           entity_kindstr(e->m_kind), (e->m_flags & DDS_ENTITY_IMPLICIT) ? " [implicit]" : "",
           e->m_hdllink.hdl, cm & 0xfff, (cm >> 12) & 0x7fff, (cm & 0x80000000) ? "closed" : "open",
           ddsrt_avl_is_empty(&e->m_children) ? "childless" : "has-children");
  }
  else
  {
    printf("delete(%p, delstate %s, handle %" PRId64 "): pin failed\n",
           (void *)e, (delstate == DIS_IMPLICIT) ? "implicit" : (delstate == DIS_EXPLICIT) ? "explicit"
                                                                                           : "from_parent",
           iid);
  }
}
#endif

/**
 * @brief 删除实体
 *
 * @param entity 要删除的实体
 * @return dds_return_t 返回操作结果
 */
dds_return_t dds_delete(dds_entity_t entity)
{
  // 调用实现函数，传入用户状态
  return dds_delete_impl(entity, DIS_USER);
}

/**
 * @brief 在释放实体之前执行最终的反初始化操作
 *
 * @param e 指向要处理的实体的指针
 */
void dds_entity_final_deinit_before_free(dds_entity *e)
{
  // 删除实体的QoS设置
  dds_delete_qos(e->m_qos);
  // 销毁条件变量
  ddsrt_cond_destroy(&e->m_cond);
  // 销毁观察者条件变量
  ddsrt_cond_destroy(&e->m_observers_cond);
  // 销毁互斥锁
  ddsrt_mutex_destroy(&e->m_mutex);
  // 销毁观察者互斥锁
  ddsrt_mutex_destroy(&e->m_observers_lock);
}

/**
 * @brief 实现删除实体的功能
 *
 * @param entity 要删除的实体
 * @param delstate 删除实现状态
 * @return dds_return_t 返回操作结果
 */
static dds_return_t dds_delete_impl(dds_entity_t entity, enum delete_impl_state delstate)
{
  dds_entity *e;
  dds_return_t ret;
  // 尝试为删除操作锁定实体
  if ((ret = dds_entity_pin_for_delete(entity, (delstate != DIS_IMPLICIT), (delstate == DIS_USER), &e)) == DDS_RETCODE_OK)
    // 如果锁定成功，执行删除操作
    return dds_delete_impl_pinned(e, delstate);
  else if (ret == DDS_RETCODE_TRY_AGAIN) /* non-child refs exist */
    // 如果存在非子引用，返回成功
    return DDS_RETCODE_OK;
  else
  {
#if TRACE_DELETE
    // 如果启用了跟踪删除，打印删除信息
    print_delete(NULL, delstate, (uint64_t)entity);
#endif
    // 返回操作结果
    return ret;
  }
}

/**
 * @brief 删除一个被固定的dds_entity实例。
 *
 * 这个函数会删除一个被固定的dds_entity实例，同时处理多线程情况下的竞争问题。
 *
 * @param[in] e         要删除的dds_entity实例指针。
 * @param[in] delstate  删除状态枚举值。
 * @return 返回dds_return_t类型的结果，表示操作成功或失败。
 */
dds_return_t dds_delete_impl_pinned(dds_entity *e, enum delete_impl_state delstate)
{
  // 多个线程可能在尝试删除或者锁定该实例，因此需要加锁保护
  ddsrt_mutex_lock(&e->m_mutex);

#if TRACE_DELETE
  // 如果启用了TRACE_DELETE宏，打印删除信息
  print_delete(e, delstate, e->m_iid);
#endif

  // 如果另一个线程正在尝试删除该实例，它将在持有m_mutex的情况下设置CLOSING标志，
  // 此时我们应该退出。
  assert(dds_handle_is_closed(&e->m_hdllink));

  // 真正执行删除操作，并解锁
  return really_delete_pinned_closed_locked(e, delstate);
}

/**
 * @brief 真正删除实体的函数，确保实体已关闭且锁定。
 *
 * @param[in] e        要删除的实体指针
 * @param[in] delstate 删除状态，表示是从父实体还是其他方式触发的删除
 * @return 返回 dds_return_t 类型的结果，成功时为 DDS_RETCODE_OK
 */
static dds_return_t really_delete_pinned_closed_locked(struct dds_entity *e, enum delete_impl_state delstate)
{
  dds_return_t ret;

  // 没有线程再固定它，不需要担心其他线程删除它
  // 但仍然可能有很多线程固定它，并尝试获取 m_mutex 来执行操作（包括创建子实体、附加到 waitsets 等）

  assert(dds_handle_is_closed(&e->m_hdllink));

  // 触发阻塞的线程（同时，删除 DDSI 读/写器以触发继续清理 -- 虽然这相当安全，因为 GUID 不会被快速重用，但它需要更新）
  dds_entity_deriver_interrupt(e);
  ddsrt_mutex_unlock(&e->m_mutex);

  // - 等待当前正在进行的监听器完成
  // - 重置所有监听器，以便不会发生新的监听器调用
  // - 等待所有挂起的监听器结束
  ddsrt_mutex_lock(&e->m_observers_lock);
  while (e->m_cb_pending_count > 0)
    ddsrt_cond_wait(&e->m_observers_cond, &e->m_observers_lock);
  dds_reset_listener(&e->m_listener);
  ddsrt_mutex_unlock(&e->m_observers_lock);

  // 等待所有其他线程取消固定实体
  dds_handle_close_wait(&e->m_hdllink);

  // 固定计数减少到 1，设置 CLOSING 标志：没有其他线程仍然涉及此实体的操作
  if (dds_entity_kind(e) == DDS_KIND_WAITSET)
  {
    // 这处理了 waitset 附加到自身的罕见和不赞成的情况。这些只能通过固定 waitset 来触发，
    // 所以这个调用应该是安全的，即使它对于从 RHC 代码深处发出信号的 read conditions 来说并不安全。
    // 此调用后列表为空，这将在 close() 后的“正常”调用中变为无操作。
    dds_entity_observers_signal_delete(e);
  }
  dds_entity_deriver_close(e);
  dds_entity_observers_signal_delete(e);

  // 递归删除子实体

  // 可能一个写入器/读取器具有对主题的最后引用。这意味着删除写入器可能导致主题被删除。
  // 当删除参与者的子实体时，这可能会导致问题：当主题是下一个要删除的子实体时，
  // 同时由于发布者->写入器的递归删除而已经在删除中。

  // 另一个问题是，当主题已经被删除，我们在这里第二次删除它之前，
  // 写入器/读取器还没有被删除，它们将具有悬空指针。

  // 为了规避问题。我们在第一个循环中忽略主题。
  DDSRT_STATIC_ASSERT((uint32_t)DDS_KIND_MAX < 32);
  static const uint32_t disallowed_kinds[] = {
      1u << (uint32_t)DDS_KIND_TOPIC,
      (uint32_t)0};
  for (size_t i = 0; i < sizeof(disallowed_kinds) / sizeof(disallowed_kinds[0]); i++)
  {
    delete_children(e, ~disallowed_kinds[i]);
  }

  // dds_handle_delete 将等待直到该句柄上的最后一个活动声明被释放。
  // 可能这个最后的释放将由在 close() 期间被踢出的线程完成。
  ret = dds_handle_delete(&e->m_hdllink);
  assert(ret == DDS_RETCODE_OK);
  (void)ret;

  // 从父实体中移除；如果它是隐式创建的、没有任何剩余子实体，
  // 并且我们不是因为删除父实体而来到这里，则安排删除父实体。
  dds_entity *parent_to_delete = NULL;
  if (e->m_parent != NULL)
  {
    struct dds_entity *const p = e->m_parent;

    ddsrt_mutex_lock(&p->m_mutex);
    assert(ddsrt_avl_lookup(&dds_entity_children_td, &p->m_children, &e->m_iid) != NULL);
    ddsrt_avl_delete(&dds_entity_children_td, &p->m_children, e);
    if (dds_handle_drop_childref_and_pin(&p->m_hdllink, delstate != DIS_FROM_PARENT))
    {
      dds_handle_close(&p->m_hdllink);
      assert(dds_handle_is_closed(&p->m_hdllink));
      assert(dds_handle_is_not_refd(&p->m_hdllink));
      assert(ddsrt_avl_is_empty(&p->m_children));
      parent_to_delete = p;
    }
    // 触发父实体，以防它在 delete 中等待
    ddsrt_cond_broadcast(&p->m_cond);
    ddsrt_mutex_unlock(&p->m_mutex);
  }

  // 在需要时执行特定的删除操作
  ret = dds_entity_deriver_delete(e);
  if (ret == DDS_RETCODE_NO_DATA)
  {
    // 引导及其逆过程总是棘手的业务，在这里也不例外：
    // 删除伪顶级对象会拆除所有应该保持存在的东西（如整个平台抽象），
    // 因此它必须是最后一个调用。因此，我们依赖它调用 "dds_entity_final_deinit_before_free" 并返回一个
    // 我们不传递给调用者的特殊错误代码。
    ret = DDS_RETCODE_OK;
  }
  else if (ret != DDS_RETCODE_OK)
  {
    if (parent_to_delete)
      dds_entity_unpin(parent_to_delete);
    return ret;
  }
  else
  {
    dds_entity_final_deinit_before_free(e);
    dds_free(e);
  }

  assert(ret == DDS_RETCODE_OK);
  (void)ret;
  return (parent_to_delete != NULL) ? dds_delete_impl_pinned(parent_to_delete, DIS_IMPLICIT) : DDS_RETCODE_OK;
}

/**
 * @brief 检查实体是否在给定根实体的范围内
 *
 * @param e 要检查的实体指针
 * @param root 根实体指针
 * @return 如果实体在根实体的范围内，则返回 true，否则返回 false
 */
bool dds_entity_in_scope(const dds_entity *e, const dds_entity *root)
{
  /* 实体不能是其自身的祖先 */
  while (e != NULL && e != root)
    e = e->m_parent;  // 获取实体的父实体
  return (e != NULL); // 如果实体不为空，则说明它在根实体的范围内
}

/**
 * @brief 获取实体的父实体
 *
 * @param entity 要获取父实体的实体
 * @return 父实体的句柄，如果没有父实体，则返回 0
 */
dds_entity_t dds_get_parent(dds_entity_t entity)
{
  dds_entity *e;
  dds_return_t rc;
  if ((rc = dds_entity_pin(entity, &e)) != DDS_RETCODE_OK) // 尝试锁定实体
    return rc;
  else
  {
    dds_entity_t hdl = e->m_parent ? e->m_parent->m_hdllink.hdl : 0; // 获取父实体的句柄
    dds_entity_unpin(e);                                             // 解锁实体
    return hdl;
  }
}

/**
 * @brief 获取实体的参与者
 *
 * @param e 要获取参与者的实体指针
 * @return 实体的参与者指针，如果没有找到参与者，则返回 NULL
 */
dds_participant *dds_entity_participant(const dds_entity *e)
{
  while (e && dds_entity_kind(e) != DDS_KIND_PARTICIPANT) // 遍历实体的祖先，直到找到参与者类型的实体
    e = e->m_parent;                                      // 获取实体的父实体
  return (dds_participant *)e;                            // 返回参与者实体指针
}

/**
 * @brief 获取实体参与者的 GUID
 *
 * @param e 要获取参与者 GUID 的实体指针
 * @return 实体参与者的 GUID 指针
 */
const ddsi_guid_t *dds_entity_participant_guid(const dds_entity *e)
{
  struct dds_participant const *const pp = dds_entity_participant(e); // 获取实体的参与者
  assert(pp != NULL);                                                 // 断言参与者不为空
  return &pp->m_entity.m_guid;                                        // 返回参与者的 GUID
}

/**
 * @brief 获取实体的参与者句柄
 *
 * @param entity 要获取参与者句柄的实体
 * @return 参与者的句柄，如果没有找到参与者，则返回 0
 */
dds_entity_t dds_get_participant(dds_entity_t entity)
{
  dds_entity *e;
  dds_return_t rc;
  if ((rc = dds_entity_pin(entity, &e)) != DDS_RETCODE_OK) // 尝试锁定实体
    return rc;
  else
  {
    dds_participant *par = dds_entity_participant(e);         // 获取实体的参与者
    dds_entity_t hdl = par ? par->m_entity.m_hdllink.hdl : 0; // 获取参与者的句柄
    dds_entity_unpin(e);                                      // 解锁实体
    return hdl;
  }
}

/**
 * @brief 获取给定实体的子实体列表
 *
 * @param entity    要查询的实体
 * @param children  子实体数组，用于存储查询到的子实体
 * @param size      子实体数组的大小
 * @return dds_return_t 返回操作结果，成功时返回子实体数量，失败时返回错误代码
 */
dds_return_t dds_get_children(dds_entity_t entity, dds_entity_t *children, size_t size)
{
  dds_entity *e;   // 定义一个实体指针 e
  dds_return_t rc; // 定义一个返回值变量 rc

  // 检查参数是否有效
  if ((children != NULL && (size == 0 || size > INT32_MAX)) || (children == NULL && size != 0))
    return DDS_RETCODE_BAD_PARAMETER;

  // 尝试锁定实体并获取实体指针
  if ((rc = dds_entity_pin(entity, &e)) != DDS_RETCODE_OK)
    return rc;
  else
  {
    ddsrt_avl_iter_t it;           // 定义一个 AVL 树迭代器
    size_t n = 0;                  // 初始化子实体计数器
    struct dds_entity *c;          // 定义一个子实体指针 c
    ddsrt_mutex_lock(&e->m_mutex); // 锁定实体互斥锁

    // 遍历实体的子实体树
    for (c = ddsrt_avl_iter_first(&dds_entity_children_td, &e->m_children, &it); c != NULL; c = ddsrt_avl_iter_next(&it))
    {
      struct dds_entity *tmp; // 定义一个临时实体指针 tmp

      // 尝试锁定子实体并获取实体指针，如果失败则跳过该子实体
      if (dds_entity_pin(c->m_hdllink.hdl, &tmp) == DDS_RETCODE_OK)
      {
        // 如果实体不是内置主题，则将其添加到子实体数组中
        if (!entity_is_builtin_topic(tmp))
        {
          if (n < size)
            children[n] = c->m_hdllink.hdl;
          n++;
        }
        dds_entity_unpin(tmp); // 解锁临时实体
      }
    }

    ddsrt_mutex_unlock(&e->m_mutex); // 解锁实体互斥锁
    dds_entity_unpin(e);             // 解锁实体
    assert(n <= INT32_MAX);          // 断言子实体数量不超过 INT32_MAX
    return (dds_return_t)n;          // 返回子实体数量
  }
}

/**
 * @brief 根据实体类型获取QoS掩码值
 *
 * @param kind 实体类型，例如：DDS_KIND_TOPIC、DDS_KIND_PARTICIPANT等
 * @return uint64_t 返回对应实体类型的QoS掩码值
 */
static uint64_t entity_kind_qos_mask(dds_entity_kind_t kind)
{
  // 使用switch语句根据实体类型选择相应的QoS掩码值
  switch (kind)
  {
  case DDS_KIND_TOPIC: // 主题类型
    return DDS_TOPIC_QOS_MASK;
  case DDS_KIND_PARTICIPANT: // 参与者类型
    return DDS_PARTICIPANT_QOS_MASK;
  case DDS_KIND_READER: // 读取器类型
    return DDS_READER_QOS_MASK;
  case DDS_KIND_WRITER: // 写入器类型
    return DDS_WRITER_QOS_MASK;
  case DDS_KIND_SUBSCRIBER: // 订阅者类型
    return DDS_SUBSCRIBER_QOS_MASK;
  case DDS_KIND_PUBLISHER: // 发布者类型
    return DDS_PUBLISHER_QOS_MASK;
  case DDS_KIND_DONTCARE:
  case DDS_KIND_COND_READ:
  case DDS_KIND_COND_QUERY:
  case DDS_KIND_COND_GUARD:
  case DDS_KIND_WAITSET:
  case DDS_KIND_DOMAIN:
  case DDS_KIND_CYCLONEDDS:
    break; // 其他类型不处理
  }
  return 0; // 默认返回0
}

/**
 * @brief 获取内置主题的QoS设置
 *
 * @param qos 指向dds_qos_t结构的指针，用于存储获取到的QoS设置
 * @return dds_return_t 返回操作结果，成功时返回DDS_RETCODE_OK
 */
static dds_return_t dds_get_qos_builtin_topic(dds_qos_t *qos)
{
  // 在没有域引用的情况下获取内置主题的QoS设置可能会比较困难
  // 在这个领域进行一些改进可能是个好主意

  // 重置QoS设置
  dds_reset_qos(qos);

  // 创建内置QoS设置
  dds_qos_t *bq = dds__create_builtin_qos();

  // 合并缺失的QoS设置
  ddsi_xqos_mergein_missing(qos, bq, DDS_TOPIC_QOS_MASK);

  // 删除内置QoS设置
  dds_delete_qos(bq);

  // 返回操作成功
  return DDS_RETCODE_OK;
}

/**
 * @brief 获取实体的QoS（Quality of Service）设置。
 *
 * @param entity 实体标识符，可以是参与者、主题、发布者、订阅者或数据写入/读取器。
 * @param qos 用于存储获取到的QoS设置的指针。
 * @return 成功时返回DDS_RETCODE_OK，否则返回相应的错误代码。
 */
dds_return_t dds_get_qos(dds_entity_t entity, dds_qos_t *qos)
{
  // 定义实体指针和返回值变量
  dds_entity *e;
  dds_return_t ret;

  // 检查qos参数是否为空，如果为空则返回错误代码
  if (qos == NULL)
    return DDS_RETCODE_BAD_PARAMETER;

  // 尝试锁定实体，如果失败则返回错误代码
  if ((ret = dds_entity_lock(entity, DDS_KIND_DONTCARE, &e)) != DDS_RETCODE_OK)
  {
    // 如果实体是内置主题，则尝试获取内置主题的QoS设置
    if (dds__get_builtin_topic_name_typename(entity, NULL, NULL) == 0)
      return dds_get_qos_builtin_topic(qos);
    else
      return ret;
  }

  // 检查实体是否支持设置QoS，如果不支持则返回错误代码
  if (!dds_entity_supports_set_qos(e))
    ret = DDS_RETCODE_ILLEGAL_OPERATION;
  else
  {
    // 定义实体QoS指针
    dds_qos_t *entity_qos;
    // 如果实体类型不是主题，则直接获取实体的QoS设置
    if (dds_entity_kind(e) != DDS_KIND_TOPIC)
      entity_qos = e->m_qos;
    else
    {
      // 如果实体类型是主题，需要先锁定参与者实体，然后获取主题的QoS设置
      struct dds_topic *const tp = (dds_topic *)e;
      struct dds_participant *const pp = dds_entity_participant(e);
      ddsrt_mutex_lock(&pp->m_entity.m_mutex);
      entity_qos = tp->m_ktopic->qos;
      ddsrt_mutex_unlock(&pp->m_entity.m_mutex);
    }

    // 重置qos参数，并将实体的QoS设置合并到qos参数中
    dds_reset_qos(qos);
    ddsi_xqos_mergein_missing(qos, entity_qos, ~(DDSI_QP_TOPIC_NAME | DDSI_QP_TYPE_NAME | DDSI_QP_TYPE_INFORMATION));
    ret = DDS_RETCODE_OK;
  }
  // 解锁实体
  dds_entity_unlock(e);
  return ret;
}

/**
 * @brief 设置实体的QoS，同时处理实体是否启用和QoS更改的有效性。
 *
 * @param[in] e          实体指针
 * @param[in,out] e_qos_ptr 实体QoS指针的指针，用于存储新的QoS设置
 * @param[in] qos        新的QoS设置
 * @param[in] mask       QoS掩码，用于确定哪些QoS设置需要合并
 * @param[in] logcfg     日志配置指针
 * @return dds_return_t  返回操作结果，成功时返回DDS_RETCODE_OK
 */
static dds_return_t dds_set_qos_locked_raw(dds_entity *e, dds_qos_t **e_qos_ptr, const dds_qos_t *qos, uint64_t mask, const struct ddsrt_log_cfg *logcfg)
{
  // 检查实体是否已启用
  const bool enabled = ((e->m_flags & DDS_ENTITY_ENABLED) != 0);
  dds_return_t ret;

  /* 对于主题，只有一个QoS，尽管可以有多个定义和sertypes（用于多语言支持） */
  dds_qos_t *newqos = dds_create_qos();
  ddsi_xqos_mergein_missing(newqos, qos, mask);
  ddsi_xqos_mergein_missing(newqos, *e_qos_ptr, ~(uint64_t)0);
  if ((ret = ddsi_xqos_valid(logcfg, newqos)) != DDS_RETCODE_OK)
  {
    // 无效或不一致的QoS设置
    goto error_or_nochange;
  }
  else if (!enabled)
  {
    // 实体未启用时，可以自由更改QoS
  }
  else
  {
    const uint64_t delta = ddsi_xqos_delta(*e_qos_ptr, newqos, ~(uint64_t)0);
    if (delta == 0)
    {
      // 新设置与旧设置相同
      goto error_or_nochange;
    }
    else if (delta & ~DDSI_QP_CHANGEABLE_MASK)
    {
      // 根据规范，不是所有QoS都可以更改
      ret = DDS_RETCODE_IMMUTABLE_POLICY;
      goto error_or_nochange;
    }
    else if (delta & (DDSI_QP_RXO_MASK | DDSI_QP_PARTITION))
    {
      /* Cyclone尚不支持更改影响匹配的QoS。重新进行匹配很容易，但后果非常奇怪。
         例如，如果一个transient-local写入者在其分区QoS设置为A时发布了数据，然后将其分区更改为B，
         那么B中的读取者是否应该获得最初在A中发布的数据？

         其他RxO QoS设置也可以做同样的事情，例如延迟预算设置。这很奇怪，因此在解决这些问题之前，
         最好不要让用户陷入这些陷阱和困境。
       */
      ret = DDS_RETCODE_UNSUPPORTED;
      goto error_or_nochange;
    }
  }

  assert(ret == DDS_RETCODE_OK);
  if ((ret = dds_entity_deriver_set_qos(e, newqos, enabled)) != DDS_RETCODE_OK)
    goto error_or_nochange;
  else
  {
    dds_delete_qos(*e_qos_ptr);
    *e_qos_ptr = newqos;
  }
  return DDS_RETCODE_OK;

error_or_nochange:
  dds_delete_qos(newqos);
  return ret;
}

/**
 * @brief 设置实体的QoS属性（已锁定）
 *
 * 此函数用于设置给定实体的QoS属性。如果实体类型不是DDS_KIND_TOPIC，将直接调用dds_set_qos_locked_raw。
 * 如果实体类型是DDS_KIND_TOPIC，则需要先确保实体已启用，并对其进行特殊处理。
 *
 * @param[in] e      要设置QoS属性的实体指针
 * @param[in] qos    包含新QoS属性的dds_qos_t结构指针
 * @param[in] mask   用于指示要更改的QoS属性的掩码
 * @return           成功时返回DDS_RETCODE_OK，否则返回相应的错误代码
 */
static dds_return_t dds_set_qos_locked_impl(dds_entity *e, const dds_qos_t *qos, uint64_t mask)
{
  // 获取日志配置
  const struct ddsrt_log_cfg *logcfg = &e->m_domain->gv.logconfig;

  // 获取实体类型
  dds_entity_kind_t kind = dds_entity_kind(e);

  // 如果实体类型不是DDS_KIND_TOPIC，直接调用dds_set_qos_locked_raw
  if (kind != DDS_KIND_TOPIC)
  {
    return dds_set_qos_locked_raw(e, &e->m_qos, qos, mask, logcfg);
  }
  else
  {
    // 对于DDS_KIND_TOPIC类型的实体，需要先确保实体已启用
    assert(e->m_flags & DDS_ENTITY_ENABLED);

    // 定义相关变量
    struct dds_topic *const tp = (struct dds_topic *)e;
    struct dds_participant *const pp = dds_entity_participant(e);
    struct dds_ktopic *const ktp = tp->m_ktopic;
    dds_return_t rc;

    // 锁定实体互斥锁
    ddsrt_mutex_lock(&pp->m_entity.m_mutex);

    // 等待ktp的defer_set_qos为0
    while (ktp->defer_set_qos != 0)
      ddsrt_cond_wait(&pp->m_entity.m_cond, &pp->m_entity.m_mutex);

    // 调用dds_set_qos_locked_raw设置QoS属性
    rc = dds_set_qos_locked_raw(e, &ktp->qos, qos, mask, logcfg);

    // 解锁实体互斥锁
    ddsrt_mutex_unlock(&pp->m_entity.m_mutex);

    // 返回结果
    return rc;
  }
}

/**
 * @brief 将发布者/订阅者的QoS设置推送到其子实体（读取器或写入器）。
 *
 * @param e 一个指向dds_entity结构的指针，表示要处理的发布者或订阅者实体。
 */
static void pushdown_pubsub_qos(dds_entity *e)
{
  // 声明一个指向dds_entity结构的指针，用于存储子实体
  struct dds_entity *c;
  // 初始化最后一个实例句柄为0
  dds_instance_handle_t last_iid = 0;
  // 对实体e的互斥锁进行加锁操作
  ddsrt_mutex_lock(&e->m_mutex);
  // 在实体e的子实体树中查找下一个子实体，并将其赋值给c
  while ((c = ddsrt_avl_lookup_succ(&dds_entity_children_td, &e->m_children, &last_iid)) != NULL)
  {
    // 声明一个指向dds_entity结构的指针，用于存储临时实体
    struct dds_entity *x;
    // 更新最后一个实例句柄为当前子实体的实例句柄
    last_iid = c->m_iid;
    // 如果成功锁定子实体，则继续执行
    if (dds_entity_pin(c->m_hdllink.hdl, &x) == DDS_RETCODE_OK)
    {
      // 断言x和c指向相同的实体
      assert(x == c);
      // 断言子实体c的类型为DDS_KIND_READER或DDS_KIND_WRITER
      assert(dds_entity_kind(c) == DDS_KIND_READER || dds_entity_kind(c) == DDS_KIND_WRITER);
      // 解锁实体e的互斥锁，避免在持有父实体锁的情况下锁定子实体
      ddsrt_mutex_unlock(&e->m_mutex);

      // 对子实体c的互斥锁进行加锁操作
      ddsrt_mutex_lock(&c->m_mutex);
      // 重新对实体e的互斥锁进行加锁操作
      ddsrt_mutex_lock(&e->m_mutex);
      // 将实体e的QoS设置应用到子实体c上
      dds_set_qos_locked_impl(c, e->m_qos, DDSI_QP_GROUP_DATA | DDSI_QP_PARTITION);
      // 解锁子实体c的互斥锁
      ddsrt_mutex_unlock(&c->m_mutex);
      // 解除子实体c的锁定状态
      dds_entity_unpin(c);
    }
  }
  // 解锁实体e的互斥锁
  ddsrt_mutex_unlock(&e->m_mutex);
}

static void pushdown_topic_qos(dds_entity *e, struct dds_ktopic *ktp)
{
  /* on input: both entities claimed but no mutexes held */
  enum
  {
    NOP,
    PROP,
    CHANGE
  } todo;
  switch (dds_entity_kind(e))
  {
  case DDS_KIND_READER:
  {
    dds_reader *rd = (dds_reader *)e;
    todo = (rd->m_topic->m_ktopic == ktp) ? CHANGE : NOP;
    break;
  }
  case DDS_KIND_WRITER:
  {
    dds_writer *wr = (dds_writer *)e;
    todo = (wr->m_topic->m_ktopic == ktp) ? CHANGE : NOP;
    break;
  }
  default:
  {
    todo = PROP;
    break;
  }
  }
  switch (todo)
  {
  case NOP:
    break;
  case CHANGE:
  {
    /* may lock topic while holding reader/writer lock */
    struct dds_participant *const pp = dds_entity_participant(e);
    ddsrt_mutex_lock(&e->m_mutex);
    ddsrt_mutex_lock(&pp->m_entity.m_mutex);
    dds_set_qos_locked_impl(e, ktp->qos, DDSI_QP_TOPIC_DATA);
    ddsrt_mutex_unlock(&pp->m_entity.m_mutex);
    ddsrt_mutex_unlock(&e->m_mutex);
    break;
  }
  case PROP:
  {
    struct dds_entity *c;
    dds_instance_handle_t last_iid = 0;
    ddsrt_mutex_lock(&e->m_mutex);
    while ((c = ddsrt_avl_lookup_succ(&dds_entity_children_td, &e->m_children, &last_iid)) != NULL)
    {
      struct dds_entity *x;
      last_iid = c->m_iid;
      if (dds_entity_pin(c->m_hdllink.hdl, &x) == DDS_RETCODE_OK)
      {
        assert(x == c);
        /* see dds_get_children for why "c" remains valid despite unlocking m_mutex */
        ddsrt_mutex_unlock(&e->m_mutex);
        pushdown_topic_qos(c, ktp);
        ddsrt_mutex_lock(&e->m_mutex);
        dds_entity_unpin(c);
      }
    }
    ddsrt_mutex_unlock(&e->m_mutex);
    break;
  }
  }
}

dds_return_t dds_set_qos(dds_entity_t entity, const dds_qos_t *qos)
{
  dds_entity *e;
  dds_return_t ret;
  if (qos == NULL)
    return DDS_RETCODE_BAD_PARAMETER;
  if ((ret = dds_entity_pin(entity, &e)) < 0)
    return ret;

  const dds_entity_kind_t kind = dds_entity_kind(e);
  if (!dds_entity_supports_set_qos(e))
  {
    dds_entity_unpin(e);
    return DDS_RETCODE_ILLEGAL_OPERATION;
  }

  ddsrt_mutex_lock(&e->m_mutex);
  ret = dds_set_qos_locked_impl(e, qos, entity_kind_qos_mask(kind));
  ddsrt_mutex_unlock(&e->m_mutex);
  if (ret < 0)
  {
    dds_entity_unpin(e);
    return ret;
  }

  switch (dds_entity_kind(e))
  {
  case DDS_KIND_TOPIC:
  {
    dds_entity *pp;
    assert(dds_entity_kind(e->m_parent) == DDS_KIND_PARTICIPANT);
    if (dds_entity_pin(e->m_parent->m_hdllink.hdl, &pp) == DDS_RETCODE_OK)
    {
      struct dds_topic *tp = (struct dds_topic *)e;
      pushdown_topic_qos(pp, tp->m_ktopic);
      dds_entity_unpin(pp);
    }
    break;
  }
  case DDS_KIND_PUBLISHER:
  case DDS_KIND_SUBSCRIBER:
  {
    pushdown_pubsub_qos(e);
    break;
  }
  default:
  {
    break;
  }
  }

  dds_entity_unpin(e);
  return 0;
}

dds_return_t dds_get_listener(dds_entity_t entity, dds_listener_t *listener)
{
  dds_entity *e;
  dds_return_t ret;
  if (listener == NULL)
    return DDS_RETCODE_BAD_PARAMETER;
  else if ((ret = dds_entity_pin(entity, &e)) != DDS_RETCODE_OK)
    return ret;
  else
  {
    ddsrt_mutex_lock(&e->m_observers_lock);
    dds_copy_listener(listener, &e->m_listener);
    ddsrt_mutex_unlock(&e->m_observers_lock);
    dds_entity_unpin(e);
    return DDS_RETCODE_OK;
  }
}

static void pushdown_listener(dds_entity *e)
{
  /* Note: e is claimed, no mutexes held */
  struct dds_entity *c;
  dds_instance_handle_t last_iid = 0;
  ddsrt_mutex_lock(&e->m_mutex);
  while ((c = ddsrt_avl_lookup_succ(&dds_entity_children_td, &e->m_children, &last_iid)) != NULL)
  {
    struct dds_entity *x;
    last_iid = c->m_iid;
    if (dds_entity_pin(c->m_hdllink.hdl, &x) == DDS_RETCODE_OK)
    {
      ddsrt_mutex_unlock(&e->m_mutex);

      ddsrt_mutex_lock(&c->m_observers_lock);
      while (c->m_cb_pending_count > 0)
        ddsrt_cond_wait(&c->m_observers_cond, &c->m_observers_lock);

      ddsrt_mutex_lock(&e->m_observers_lock);
      dds_override_inherited_listener(&c->m_listener, &e->m_listener);
      ddsrt_mutex_unlock(&e->m_observers_lock);

      ddsrt_mutex_unlock(&c->m_observers_lock);

      pushdown_listener(c);

      ddsrt_mutex_lock(&e->m_mutex);
      dds_entity_unpin(c);
    }
  }
  ddsrt_mutex_unlock(&e->m_mutex);
}

dds_return_t dds_set_listener(dds_entity_t entity, const dds_listener_t *listener)
{
  dds_entity *e, *x;
  dds_return_t rc;

  if ((rc = dds_entity_pin(entity, &e)) != DDS_RETCODE_OK)
    return rc;

  ddsrt_mutex_lock(&e->m_observers_lock);
  while (e->m_cb_pending_count > 0)
    ddsrt_cond_wait(&e->m_observers_cond, &e->m_observers_lock);

  /* new listener is constructed by combining "listener" with the ancestral listeners;
     the new set of listeners is then pushed down into the descendant entities, overriding
     the ones they originally inherited from */
  dds_reset_listener(&e->m_listener);
  if (listener)
    dds_merge_listener(&e->m_listener, listener);

  /* Special case: the on_data_on_readers event doesn't exist on DataReaders. */
  if (dds_entity_kind(e) == DDS_KIND_READER)
    e->m_listener.on_data_on_readers = 0;

  x = e;
  while (dds_entity_kind(x) != DDS_KIND_CYCLONEDDS)
  {
    x = x->m_parent;
    ddsrt_mutex_lock(&x->m_observers_lock);
    dds_inherit_listener(&e->m_listener, &x->m_listener);
    ddsrt_mutex_unlock(&x->m_observers_lock);
  }

  ddsrt_mutex_unlock(&e->m_observers_lock);
  pushdown_listener(e);
  dds_entity_unpin(e);
  return DDS_RETCODE_OK;
}

dds_return_t dds_enable(dds_entity_t entity)
{
  dds_entity *e;
  dds_return_t rc;

  if ((rc = dds_entity_lock(entity, DDS_KIND_DONTCARE, &e)) != DDS_RETCODE_OK)
    return rc;

  if ((e->m_flags & DDS_ENTITY_ENABLED) == 0)
  {
    /* TODO: Really enable. */
    e->m_flags |= DDS_ENTITY_ENABLED;
    DDS_CERROR(&e->m_domain->gv.logconfig, "Delayed entity enabling is not supported\n");
  }
  dds_entity_unlock(e);
  return DDS_RETCODE_OK;
}

dds_return_t dds_get_status_mask(dds_entity_t entity, uint32_t *mask)
{
  dds_entity *e;
  dds_return_t ret;

  if (mask == NULL)
    return DDS_RETCODE_BAD_PARAMETER;

  if ((ret = dds_entity_pin(entity, &e)) != DDS_RETCODE_OK)
    return ret;

  if (!dds_entity_supports_validate_status(e))
    ret = DDS_RETCODE_ILLEGAL_OPERATION;
  else
  {
    assert(entity_has_status(e));
    *mask = ddsrt_atomic_ld32(&e->m_status.m_status_and_mask) >> SAM_ENABLED_SHIFT;
    // don't leak abuse of DATA_ON_READERS in readers' status masks
    if (dds_entity_kind(e) == DDS_KIND_READER)
      *mask &= ~(uint32_t)DDS_DATA_ON_READERS_STATUS;
    ret = DDS_RETCODE_OK;
  }
  dds_entity_unpin(e);
  return ret;
}

dds_return_t dds_set_status_mask(dds_entity_t entity, uint32_t mask)
{
  dds_entity *e;
  dds_return_t ret;

  if ((mask & ~SAM_STATUS_MASK) != 0)
    return DDS_RETCODE_BAD_PARAMETER;

  /* Lock rather than pin so this is can be done atomically with respect to dds_delete */
  if ((ret = dds_entity_lock(entity, DDS_KIND_DONTCARE, &e)) != DDS_RETCODE_OK)
    return ret;

  if (dds_handle_is_closed(&e->m_hdllink))
  {
    dds_entity_unlock(e);
    return DDS_RETCODE_PRECONDITION_NOT_MET;
  }

  if ((ret = dds_entity_deriver_validate_status(e, mask)) == DDS_RETCODE_OK)
  {
    assert(entity_has_status(e));
    ddsrt_mutex_lock(&e->m_observers_lock);
    while (e->m_cb_pending_count > 0)
      ddsrt_cond_wait(&e->m_observers_cond, &e->m_observers_lock);

    // readers: don't touch DATA_ON_READERS_STATUS in mask
    if (dds_entity_kind(e) == DDS_KIND_READER)
      mask |= DDS_DATA_ON_READERS_STATUS;
    uint32_t old, new;
    do
    {
      old = ddsrt_atomic_ld32(&e->m_status.m_status_and_mask);
      assert(!(old & DDS_DATA_ON_READERS_STATUS) || dds_entity_kind(e) != DDS_KIND_READER);
      new = (mask << SAM_ENABLED_SHIFT) | (old & SAM_STATUS_MASK);
    } while (!ddsrt_atomic_cas32(&e->m_status.m_status_and_mask, old, new));
    ddsrt_mutex_unlock(&e->m_observers_lock);
  }
  dds_entity_unlock(e);
  return ret;
}

static dds_return_t dds_readtake_status(dds_entity_t entity, uint32_t *status, uint32_t mask, bool reset)
{
  dds_entity *e;
  dds_return_t ret;

  if (status == NULL)
    return DDS_RETCODE_BAD_PARAMETER;
  if ((mask & ~SAM_STATUS_MASK) != 0)
    return DDS_RETCODE_BAD_PARAMETER;

  if ((ret = dds_entity_lock(entity, DDS_KIND_DONTCARE, &e)) != DDS_RETCODE_OK)
    return ret;

  if ((ret = dds_entity_deriver_validate_status(e, mask)) == DDS_RETCODE_OK)
  {
    uint32_t s;
    assert(entity_has_status(e));
    if (mask == 0)
      mask = SAM_STATUS_MASK;
    if (reset)
      s = ddsrt_atomic_and32_ov(&e->m_status.m_status_and_mask, ~mask) & mask;
    else
      s = ddsrt_atomic_ld32(&e->m_status.m_status_and_mask) & mask;

    // non-materialized DATA_ON_READERS requires a fix-up
    if (dds_entity_kind(e) == DDS_KIND_SUBSCRIBER)
    {
      dds_subscriber *const sub = (dds_subscriber *)e;
      ddsrt_mutex_lock(&sub->m_entity.m_observers_lock);
      if (!(sub->materialize_data_on_readers & DDS_SUB_MATERIALIZE_DATA_ON_READERS_FLAG))
      {
        if (dds_subscriber_compute_data_on_readers_locked(sub))
          s |= DDS_DATA_ON_READERS_STATUS;
        else
          s &= ~(uint32_t)DDS_DATA_ON_READERS_STATUS;
      }
      ddsrt_mutex_unlock(&sub->m_entity.m_observers_lock);
    }

    *status = s;
  }
  dds_entity_unlock(e);
  return ret;
}

dds_return_t dds_read_status(dds_entity_t entity, uint32_t *status, uint32_t mask)
{
  return dds_readtake_status(entity, status, mask, false);
}

dds_return_t dds_take_status(dds_entity_t entity, uint32_t *status, uint32_t mask)
{
  return dds_readtake_status(entity, status, mask, true);
}

dds_return_t dds_get_status_changes(dds_entity_t entity, uint32_t *status)
{
  return dds_read_status(entity, status, 0);
}

dds_return_t dds_get_domainid(dds_entity_t entity, dds_domainid_t *id)
{
  dds_entity *e;
  dds_return_t rc;

  if (id == NULL)
    return DDS_RETCODE_BAD_PARAMETER;

  if ((rc = dds_entity_pin(entity, &e)) != DDS_RETCODE_OK)
    return rc;

  *id = e->m_domain ? e->m_domain->m_id : DDS_DOMAIN_DEFAULT;
  dds_entity_unpin(e);
  return DDS_RETCODE_OK;
}

dds_return_t dds_get_instance_handle(dds_entity_t entity, dds_instance_handle_t *ihdl)
{
  dds_entity *e;
  dds_return_t ret;

  if (ihdl == NULL)
    return DDS_RETCODE_BAD_PARAMETER;

  if ((ret = dds_entity_pin(entity, &e)) != DDS_RETCODE_OK)
    return ret;
  *ihdl = e->m_iid;
  dds_entity_unpin(e);
  return ret;
}

dds_return_t dds_get_guid(dds_entity_t entity, dds_guid_t *guid)
{
  dds_entity *e;
  dds_return_t ret;

  if (guid == NULL)
    return DDS_RETCODE_BAD_PARAMETER;

  if ((ret = dds_entity_pin(entity, &e)) != DDS_RETCODE_OK)
    return ret;
  switch (dds_entity_kind(e))
  {
  case DDS_KIND_PARTICIPANT:
  case DDS_KIND_READER:
  case DDS_KIND_WRITER:
  case DDS_KIND_TOPIC:
  {
    DDSRT_STATIC_ASSERT(sizeof(dds_guid_t) == sizeof(ddsi_guid_t));
    ddsi_guid_t tmp = ddsi_ntoh_guid(e->m_guid);
    memcpy(guid, &tmp, sizeof(*guid));
    ret = DDS_RETCODE_OK;
    break;
  }
  default:
  {
    ret = DDS_RETCODE_ILLEGAL_OPERATION;
    break;
  }
  }
  dds_entity_unpin(e);
  return ret;
}

dds_return_t dds_entity_pin_with_origin(dds_entity_t hdl, bool from_user, dds_entity **eptr)
{
  dds_return_t hres;
  struct dds_handle_link *hdllink;
  if ((hres = dds_handle_pin_with_origin(hdl, from_user, &hdllink)) < 0)
    return hres;
  else
  {
    *eptr = dds_entity_from_handle_link(hdllink);
    return DDS_RETCODE_OK;
  }
}

dds_return_t dds_entity_pin(dds_entity_t hdl, dds_entity **eptr)
{
  return dds_entity_pin_with_origin(hdl, true, eptr);
}

dds_return_t dds_entity_pin_for_delete(dds_entity_t hdl, bool explicit, bool from_user, dds_entity **eptr)
{
  dds_return_t hres;
  struct dds_handle_link *hdllink;
  if ((hres = dds_handle_pin_for_delete(hdl, explicit, from_user, &hdllink)) < 0)
    return hres;
  else
  {
    *eptr = dds_entity_from_handle_link(hdllink);
    return DDS_RETCODE_OK;
  }
}

void dds_entity_unpin(dds_entity *e)
{
  dds_handle_unpin(&e->m_hdllink);
}

dds_return_t dds_entity_lock(dds_entity_t hdl, dds_entity_kind_t kind, dds_entity **eptr)
{
  dds_return_t hres;
  dds_entity *e;
  if ((hres = dds_entity_pin(hdl, &e)) < 0)
    return hres;
  else
  {
    if (dds_entity_kind(e) != kind && kind != DDS_KIND_DONTCARE)
    {
      dds_entity_unpin(e);
      return DDS_RETCODE_ILLEGAL_OPERATION;
    }

    ddsrt_mutex_lock(&e->m_mutex);
    *eptr = e;
    return DDS_RETCODE_OK;
  }
}

void dds_entity_unlock(dds_entity *e)
{
  ddsrt_mutex_unlock(&e->m_mutex);
  dds_entity_unpin(e);
}

dds_return_t dds_triggered(dds_entity_t entity)
{
  dds_entity *e;
  dds_return_t ret;

  if ((ret = dds_entity_lock(entity, DDS_KIND_DONTCARE, &e)) != DDS_RETCODE_OK)
    return ret;
  if (!entity_has_status(e))
    ret = (ddsrt_atomic_ld32(&e->m_status.m_trigger) != 0);
  else
  {
    const uint32_t sm = ddsrt_atomic_ld32(&e->m_status.m_status_and_mask);
    ret = ((sm & (sm >> SAM_ENABLED_SHIFT)) != 0);
  }
  dds_entity_unlock(e);
  return ret;
}

static bool in_observer_list_p(const struct dds_entity *observed, const dds_waitset *observer)
{
  dds_entity_observer *cur;
  for (cur = observed->m_observers; cur != NULL; cur = cur->m_next)
    if (cur->m_observer == observer)
      return true;
  return false;
}

dds_return_t dds_entity_observer_register(dds_entity *observed, dds_waitset *observer, dds_entity_callback_t cb, dds_entity_attach_callback_t attach_cb, void *attach_arg, dds_entity_delete_callback_t delete_cb)
{
  dds_return_t rc;
  assert(observed);
  ddsrt_mutex_lock(&observed->m_observers_lock);
  if (in_observer_list_p(observed, observer))
    rc = DDS_RETCODE_PRECONDITION_NOT_MET;
  else if (!attach_cb(observer, observed, attach_arg))
    rc = DDS_RETCODE_BAD_PARAMETER;
  else
  {
    dds_entity_observer *o = ddsrt_malloc(sizeof(dds_entity_observer));
    o->m_cb = cb;
    o->m_delete_cb = delete_cb;
    o->m_observer = observer;
    o->m_next = observed->m_observers;
    observed->m_observers = o;
    rc = DDS_RETCODE_OK;
  }
  ddsrt_mutex_unlock(&observed->m_observers_lock);
  return rc;
}

dds_return_t dds_entity_observer_unregister(dds_entity *observed, dds_waitset *observer, bool invoke_delete_cb)
{
  dds_return_t rc;
  dds_entity_observer *prev, *idx;

  ddsrt_mutex_lock(&observed->m_observers_lock);
  prev = NULL;
  idx = observed->m_observers;
  while (idx != NULL && idx->m_observer != observer)
  {
    prev = idx;
    idx = idx->m_next;
  }
  if (idx == NULL)
    rc = DDS_RETCODE_PRECONDITION_NOT_MET;
  else
  {
    if (prev == NULL)
      observed->m_observers = idx->m_next;
    else
      prev->m_next = idx->m_next;
    if (invoke_delete_cb)
      idx->m_delete_cb(idx->m_observer, observed->m_hdllink.hdl);
    ddsrt_free(idx);
    rc = DDS_RETCODE_OK;
  }
  ddsrt_mutex_unlock(&observed->m_observers_lock);
  return rc;
}

void dds_entity_observers_signal(dds_entity *observed, uint32_t status)
{
  for (dds_entity_observer *idx = observed->m_observers; idx; idx = idx->m_next)
    idx->m_cb(idx->m_observer, observed->m_hdllink.hdl, status);
}

static void dds_entity_observers_signal_delete(dds_entity *observed)
{
  dds_entity_observer *idx;
  idx = observed->m_observers;
  while (idx != NULL)
  {
    dds_entity_observer *next = idx->m_next;
    idx->m_delete_cb(idx->m_observer, observed->m_hdllink.hdl);
    ddsrt_free(idx);
    idx = next;
  }
  observed->m_observers = NULL;
}

void dds_entity_status_signal(dds_entity *e, uint32_t status)
{
  ddsrt_mutex_lock(&e->m_observers_lock);
  dds_entity_observers_signal(e, status);
  ddsrt_mutex_unlock(&e->m_observers_lock);
}

bool dds_entity_status_set(dds_entity *e, status_mask_t status)
{
  // returns true if waitsets need triggering
  assert(entity_has_status(e));
  uint32_t old = ddsrt_atomic_or32_ov(&e->m_status.m_status_and_mask, status);
  if (old & status)
    return false; // already set, no need to trigger waitsets
  else if (!(status & (old >> SAM_ENABLED_SHIFT)))
    return false; // masked
  else
    return true;
}

dds_entity_t dds_get_topic(dds_entity_t entity)
{
  dds_return_t rc;
  dds_entity_t hdl;
  dds_entity *e;

  if ((rc = dds_entity_lock(entity, DDS_KIND_DONTCARE, &e)) != DDS_RETCODE_OK)
    return rc;
  switch (dds_entity_kind(e))
  {
  case DDS_KIND_READER:
  {
    dds_reader *rd = (dds_reader *)e;
    if ((hdl = dds__get_builtin_topic_pseudo_handle_from_typename(rd->m_topic->m_stype->type_name)) < 0)
      hdl = rd->m_topic->m_entity.m_hdllink.hdl;
    break;
  }
  case DDS_KIND_WRITER:
  {
    dds_writer *wr = (dds_writer *)e;
    assert(dds__get_builtin_topic_pseudo_handle_from_typename(wr->m_wr->type->type_name) < 0);
    hdl = wr->m_topic->m_entity.m_hdllink.hdl;
    break;
  }
  case DDS_KIND_COND_READ:
  case DDS_KIND_COND_QUERY:
  {
    assert(dds_entity_kind(e->m_parent) == DDS_KIND_READER);
    dds_reader *rd = (dds_reader *)e->m_parent;
    if ((hdl = dds__get_builtin_topic_pseudo_handle_from_typename(rd->m_topic->m_stype->type_name)) < 0)
      hdl = rd->m_topic->m_entity.m_hdllink.hdl;
    break;
  }
  default:
  {
    hdl = DDS_RETCODE_ILLEGAL_OPERATION;
    break;
  }
  }
  dds_entity_unlock(e);
  return hdl;
}

dds_return_t dds_generic_unimplemented_operation_manykinds(dds_entity_t handle, size_t nkinds, const dds_entity_kind_t *kinds)
{
  dds_entity *e;
  dds_return_t ret;
  if ((ret = dds_entity_pin(handle, &e)) != DDS_RETCODE_OK)
    return ret;
  else
  {
    const dds_entity_kind_t actual = dds_entity_kind(e);
    ret = DDS_RETCODE_ILLEGAL_OPERATION;
    for (size_t i = 0; i < nkinds; i++)
    {
      if (kinds[i] == actual)
      {
        /* If the handle happens to be for an entity of the right kind, return unsupported */
        ret = DDS_RETCODE_UNSUPPORTED;
        break;
      }
    }
    dds_entity_unpin(e);
    return ret;
  }
}

dds_return_t dds_generic_unimplemented_operation(dds_entity_t handle, dds_entity_kind_t kind)
{
  return dds_generic_unimplemented_operation_manykinds(handle, 1, &kind);
}

dds_return_t dds_assert_liveliness(dds_entity_t entity)
{
  dds_return_t rc;
  dds_entity *e, *ewr;

  if ((rc = dds_entity_pin(entity, &e)) != DDS_RETCODE_OK)
    return rc;
  switch (dds_entity_kind(e))
  {
  case DDS_KIND_PARTICIPANT:
  {
    ddsi_write_pmd_message_guid(&e->m_domain->gv, &e->m_guid, DDSI_PARTICIPANT_MESSAGE_DATA_KIND_MANUAL_LIVELINESS_UPDATE);
    break;
  }
  case DDS_KIND_WRITER:
  {
    if ((rc = dds_entity_lock(entity, DDS_KIND_WRITER, &ewr)) != DDS_RETCODE_OK)
      return rc;
    if ((rc = ddsi_write_hb_liveliness(&e->m_domain->gv, &e->m_guid, ((struct dds_writer *)ewr)->m_xp)) != DDS_RETCODE_OK)
      return rc;
    dds_entity_unlock(e);
    break;
  }
  default:
  {
    rc = DDS_RETCODE_ILLEGAL_OPERATION;
    break;
  }
  }
  dds_entity_unpin(e);
  return rc;
}

dds_return_t dds_return_loan(dds_entity_t entity, void **buf, int32_t bufsz)
{
  dds_entity *p_entity;
  dds_return_t ret;

  // bufsz <= 0 is accepted because it allows one to write:
  //
  // if (dds_return_loan(rd, buf, dds_take(rd, buf, ...)) < 0)
  //   abort();
  //
  // with abort only being called if there is a real problem.
  //
  // The wisdom of such code may be debatable, but it has been allowed for a long
  // time and changing it may well break existing application code.
  if (buf == NULL || (bufsz > 0 && buf[0] == NULL))
    return DDS_RETCODE_BAD_PARAMETER;

  if ((ret = dds_entity_pin(entity, &p_entity)) < 0)
    return ret;

  switch (dds_entity_kind(p_entity))
  {
  case DDS_KIND_READER:
  {
    dds_reader *rd = (dds_reader *)p_entity;
    ret = dds_return_reader_loan(rd, buf, bufsz);
    break;
  }
  case DDS_KIND_COND_READ:
  case DDS_KIND_COND_QUERY:
  {
    dds_readcond *rdcond = (dds_readcond *)p_entity;
    dds_reader *rd = (dds_reader *)rdcond->m_entity.m_parent;
    assert(dds_entity_kind(&rd->m_entity) == DDS_KIND_READER);
    ret = dds_return_reader_loan(rd, buf, bufsz);
    break;
  }
  case DDS_KIND_WRITER:
  {
    dds_writer *wr = (dds_writer *)p_entity;
    ret = dds_return_writer_loan(wr, buf, bufsz);
    break;
  }
  case DDS_KIND_DONTCARE:
  case DDS_KIND_CYCLONEDDS:
  case DDS_KIND_DOMAIN:
  case DDS_KIND_WAITSET:
  case DDS_KIND_COND_GUARD:
  case DDS_KIND_PARTICIPANT:
  case DDS_KIND_TOPIC:
  case DDS_KIND_PUBLISHER:
  case DDS_KIND_SUBSCRIBER:
  {
    ret = DDS_RETCODE_ILLEGAL_OPERATION;
    break;
  }
  }
  dds_entity_unpin(p_entity);
  return ret;
}

#ifdef DDS_HAS_TYPE_DISCOVERY

dds_return_t dds_get_typeinfo(dds_entity_t entity, dds_typeinfo_t **type_info)
{
  dds_return_t ret;
  dds_entity *e;

  if (!type_info)
    return DDS_RETCODE_BAD_PARAMETER;
  if ((ret = dds_entity_pin(entity, &e)) != DDS_RETCODE_OK)
    return ret;
  switch (dds_entity_kind(e))
  {
  case DDS_KIND_TOPIC:
  {
    struct dds_topic *const tp = (struct dds_topic *)e;
    if (!(*type_info = ddsi_sertype_typeinfo(tp->m_stype)))
      ret = DDS_RETCODE_NOT_FOUND;
    break;
  }
  case DDS_KIND_READER:
  {
    struct dds_reader *const rd = (struct dds_reader *)e;
    if (!(*type_info = ddsi_sertype_typeinfo(rd->m_rd->type)))
      ret = DDS_RETCODE_NOT_FOUND;
    break;
  }
  case DDS_KIND_WRITER:
  {
    struct dds_writer *const wr = (struct dds_writer *)e;
    if (!(*type_info = ddsi_sertype_typeinfo(wr->m_wr->type)))
      ret = DDS_RETCODE_NOT_FOUND;
    break;
  }
  default:
    ret = DDS_RETCODE_ILLEGAL_OPERATION;
    break;
  }
  dds_entity_unpin(e);
  return ret;
}

dds_return_t dds_free_typeinfo(dds_typeinfo_t *type_info)
{
  if (type_info == NULL)
    return DDS_RETCODE_BAD_PARAMETER;
  ddsi_typeinfo_fini(type_info);
  dds_free(type_info);
  return DDS_RETCODE_OK;
}

#else

dds_return_t dds_get_typeinfo(dds_entity_t entity, dds_typeinfo_t **type_info)
{
  (void)entity;
  (void)type_info;
  return DDS_RETCODE_UNSUPPORTED;
}

dds_return_t dds_free_typeinfo(dds_typeinfo_t *type_info)
{
  (void)type_info;
  return DDS_RETCODE_UNSUPPORTED;
}

#endif /* DDS_HAS_TYPE_DISCOVERY */
