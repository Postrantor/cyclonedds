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
#ifndef DDS__ENTITY_H
#define DDS__ENTITY_H

#include "dds/ddsi/ddsi_thread.h"
#include "dds/ddsrt/countargs.h"
#include "dds/export.h"
#include "dds__types.h"

#if defined(__cplusplus)
extern "C" {
#endif

/** @component generic_entity */
/**
 * @brief 初始化实体 (Initialize the entity)
 *
 * @param[in] e        要初始化的实体指针 (Pointer to the entity to be initialized)
 * @param[in] parent   父实体指针 (Pointer to the parent entity)
 * @param[in] kind     实体类型 (Entity kind)
 * @param[in] implicit 是否为隐式实体 (Whether it is an implicit entity)
 * @param[in] user_access 用户是否有访问权限 (Whether the user has access permission)
 * @param[in] qos      质量服务设置 (Quality of Service settings)
 * @param[in] listener 监听器 (Listener)
 * @param[in] mask     状态掩码 (Status mask)
 *
 * @return 初始化后的实体 (Initialized entity)
 */
dds_entity_t dds_entity_init(dds_entity *e,
                             dds_entity *parent,
                             dds_entity_kind_t kind,
                             bool implicit,
                             bool user_access,
                             dds_qos_t *qos,
                             const dds_listener_t *listener,
                             status_mask_t mask);

/** @component generic_entity */
/**
 * @brief 完成实体初始化 (Complete the entity initialization)
 *
 * @param[in] entity 实体指针 (Pointer to the entity)
 */
void dds_entity_init_complete(dds_entity *entity);

/** @component generic_entity */
/**
 * @brief 注册子实体 (Register a child entity)
 *
 * @param[in] parent 父实体指针 (Pointer to the parent entity)
 * @param[in] child  子实体指针 (Pointer to the child entity)
 */
void dds_entity_register_child(dds_entity *parent, dds_entity *child);

/** @component generic_entity */
/**
 * @brief 对实体进行加锁并增加引用计数 (Lock the entity and increase the reference count)
 *
 * @param[in] e 实体指针 (Pointer to the entity)
 */
void dds_entity_add_ref_locked(dds_entity *e);

/** @component generic_entity */
/**
 * @brief 减少实体的引用计数 (Decrease the reference count of the entity)
 *
 * @param[in] e 实体指针 (Pointer to the entity)
 */
void dds_entity_drop_ref(dds_entity *e);

/** @component generic_entity */
/**
 * @brief 解除实体的锁定并减少引用计数 (Unpin the entity and decrease the reference count)
 *
 * @param[in] e 实体指针 (Pointer to the entity)
 */
void dds_entity_unpin_and_drop_ref(dds_entity *e);

#define DEFINE_ENTITY_LOCK_UNLOCK(type_, kind_, component_)                                                   \
  /** @component component_ */                                                                                \
  /**                                                                                                         \
   * @brief 锁定实体并返回类型为 type_ 的实体指针 (Lock the entity and return a pointer to an  \
   * entity of type type_)                                                                                    \
   *                                                                                                          \
   * @param[in] hdl 实体句柄 (Entity handle)                                                              \
   * @param[out] x  类型为 type_ 的实体指针 (Pointer to an entity of type type_)                      \
   *                                                                                                          \
   * @return 成功时返回 DDS_RETCODE_OK，失败时返回错误代码 (Returns DDS_RETCODE_OK on success, \
   * error code otherwise)                                                                                    \
   */                                                                                                         \
  inline dds_return_t type_##_lock(dds_entity_t hdl, type_ **x) {                                             \
    dds_return_t rc;                                                                                          \
    dds_entity *e;                                                                                            \
    if ((rc = dds_entity_lock(hdl, kind_, &e)) < 0) return rc;                                                \
    *x = (type_ *)e;                                                                                          \
    return DDS_RETCODE_OK;                                                                                    \
  }                                                                                                           \
  /** @component component_ */                                                                                \
  /**                                                                                                         \
   * @brief 解锁类型为 type_ 的实体 (Unlock an entity of type type_)                                  \
   *                                                                                                          \
   * @param[in] x 类型为 type_ 的实体指针 (Pointer to an entity of type type_)                        \
   */                                                                                                         \
  inline void type_##_unlock(type_ *x) { dds_entity_unlock(&x->m_entity); }
#define DECL_ENTITY_LOCK_UNLOCK(type_)                                  \
  extern inline dds_return_t type_##_lock(dds_entity_t hdl, type_ **x); \
  extern inline void type_##_unlock(type_ *x);

/** @component generic_entity */
/**
 * @brief 从句柄链接获取实体指针 (Get the entity pointer from the handle link)
 *
 * @param[in] hdllink 句柄链接 (Handle link)
 *
 * @return 实体指针 (Entity pointer)
 */
inline dds_entity *dds_entity_from_handle_link(struct dds_handle_link *hdllink) {
  return (dds_entity *)((char *)hdllink - offsetof(struct dds_entity, m_hdllink));
}

/** @component generic_entity */
/**
 * @brief 检查实体是否已启用 (Check if the entity is enabled)
 *
 * @param[in] e 实体指针 (Pointer to the entity)
 *
 * @return 如果实体已启用，则返回 true，否则返回 false (Returns true if the entity is enabled, false
 * otherwise)
 */
inline bool dds_entity_is_enabled(const dds_entity *e) {
  return (e->m_flags & DDS_ENTITY_ENABLED) != 0;
}

/** @component generic_entity */

/**
 * @brief 设置实体状态
 * @param[in] e 指向要设置状态的实体的指针
 * @param[in] t 要设置的状态掩码
 * @return 返回操作是否成功
 *
 * @brief Set entity status
 * @param[in] e Pointer to the entity whose status is to be set
 * @param[in] t Status mask to be set
 * @return Returns whether the operation was successful
 */
bool dds_entity_status_set(dds_entity *e, status_mask_t t) ddsrt_attribute_warn_unused_result;

/** @component generic_entity */

/**
 * @brief 重置实体状态
 * @param[in] e 指向要重置状态的实体的指针
 * @param[in] t 要重置的状态掩码
 *
 * @brief Reset entity status
 * @param[in] e Pointer to the entity whose status is to be reset
 * @param[in] t Status mask to be reset
 */
inline void dds_entity_status_reset(dds_entity *e, status_mask_t t) {
  ddsrt_atomic_and32(&e->m_status.m_status_and_mask, SAM_ENABLED_MASK | (status_mask_t)~t);
}

/** @component generic_entity */

/**
 * @brief 重置实体状态并返回旧值
 * @param[in] e 指向要重置状态的实体的指针
 * @param[in] t 要重置的状态掩码
 * @return 返回重置前的状态掩码
 *
 * @brief Reset entity status and return old value
 * @param[in] e Pointer to the entity whose status is to be reset
 * @param[in] t Status mask to be reset
 * @return Returns the status mask before resetting
 */
inline uint32_t dds_entity_status_reset_ov(dds_entity *e, status_mask_t t) {
  return ddsrt_atomic_and32_ov(&e->m_status.m_status_and_mask,
                               SAM_ENABLED_MASK | (status_mask_t)~t);
}

/** @component generic_entity */

/**
 * @brief 获取实体类型
 * @param[in] e 指向要获取类型的实体的指针
 * @return 返回实体类型
 *
 * @brief Get entity kind
 * @param[in] e Pointer to the entity whose kind is to be obtained
 * @return Returns the entity kind
 */
inline dds_entity_kind_t dds_entity_kind(const dds_entity *e) { return e->m_kind; }

/** @component generic_entity */

/**
 * @brief 发送观察者信号
 * @param[in] observed 被观察的实体
 * @param[in] status 状态掩码
 *
 * @brief Signal observers
 * @param[in] observed The observed entity
 * @param[in] status Status mask
 */
void dds_entity_observers_signal(dds_entity *observed, uint32_t status);

/** @component generic_entity */

/**
 * @brief 发送实体状态信号
 * @param[in] e 指向要发送状态信号的实体的指针
 * @param[in] status 状态掩码
 *
 * @brief Signal entity status
 * @param[in] e Pointer to the entity whose status signal is to be sent
 * @param[in] status Status mask
 */
void dds_entity_status_signal(dds_entity *e, uint32_t status);

/**
 * @brief DDS 状态联合体
 *
 * 包含了多种 DDS 状态，用于在不同状态之间共享内存。
 */
union dds_status_union {
  dds_inconsistent_topic_status_t inconsistent_topic;            ///< 不一致主题状态
  dds_liveliness_changed_status_t liveliness_changed;            ///< 活跃度变化状态
  dds_liveliness_lost_status_t liveliness_lost;                  ///< 活跃度丢失状态
  dds_offered_deadline_missed_status_t offered_deadline_missed;  ///< 提供的截止日期错过状态
  dds_offered_incompatible_qos_status_t offered_incompatible_qos;  ///< 提供的不兼容 QoS 状态
  dds_publication_matched_status_t publication_matched;            ///< 出版匹配状态
  dds_requested_deadline_missed_status_t requested_deadline_missed;  ///< 请求的截止日期错过状态
  dds_requested_incompatible_qos_status_t requested_incompatible_qos;  ///< 请求的不兼容 QoS 状态
  dds_sample_lost_status_t sample_lost;                                ///< 样本丢失状态
  dds_sample_rejected_status_t sample_rejected;                        ///< 样本拒绝状态
  dds_subscription_matched_status_t subscription_matched;              ///< 订阅匹配状态
};

#define DDS_RESET_STATUS_FIELDS_1(ent_, status_, reset0_) \
  ((ent_)->m_##status_##_status.reset0_ = 0);
#define DDS_RESET_STATUS_FIELDS_2(ent_, status_, reset0_, reset1_) \
  ((ent_)->m_##status_##_status.reset0_ = 0);                      \
  ((ent_)->m_##status_##_status.reset1_ = 0);
#define DDS_RESET_STATUS_FIELDS_MSVC_WORKAROUND(x) x
#define DDS_RESET_STATUS_FIELDS_N1(n_, ent_, status_, ...) \
  DDS_RESET_STATUS_FIELDS_MSVC_WORKAROUND(DDS_RESET_STATUS_FIELDS_##n_(ent_, status_, __VA_ARGS__))
#define DDS_RESET_STATUS_FIELDS_N(n_, ent_, status_, ...) \
  DDS_RESET_STATUS_FIELDS_N1(n_, ent_, status_, __VA_ARGS__)

// 在遵循规范的模式下，不要设置掩码中的状态，即使在调用监听器之前重置状态的时间非常短。
// 唯一的原因是，在等待集合中对条件进行初始评估时，没有窗口可以观察到状态。
// 没有任何地方说这是必需的，但它使编写监听器/等待集触发器排序测试变得更加困难。
/**
 * @brief 宏定义实现状态回调函数的调用 (Macro definition for implementing the invocation of status
 * callback functions)
 *
 * @param entity_kind_ 实体类型 (Entity kind)
 * @param name_ 状态名称 (Status name)
 * @param NAME_ 大写状态名称 (Uppercase status name)
 * @param ... 可变参数列表，包含状态字段 (Variadic argument list, containing status fields)
 */
#define STATUS_CB_IMPL_INVOKE(entity_kind_, name_, NAME_, ...)                              \
  static bool status_cb_##name_##_invoke(dds_##entity_kind_ *const e) {                     \
    /* 获取监听器指针 (Get listener pointer) */                                      \
    struct dds_listener const *const listener = &e->m_entity.m_listener;                    \
    /* 定义状态联合体 (Define status union) */                                       \
    union dds_status_union lst;                                                             \
    /* 设置状态 (Set status) */                                                         \
    lst.name_ = e->m_##name_##_status;                                                      \
    /* 判断是否需要重置状态 (Check if status needs to be reset) */                \
    if (listener->reset_on_invoke & DDS_##NAME_##_STATUS) {                                 \
      signal = false;                                                                       \
      /* 重置状态字段 (Reset status fields) */                                        \
      DDS_RESET_STATUS_FIELDS_N(DDSRT_COUNT_ARGS(__VA_ARGS__), e, name_, __VA_ARGS__)       \
      /* 重置实体状态 (Reset entity status) */                                        \
      dds_entity_status_reset(&e->m_entity, DDS_##NAME_##_STATUS);                          \
    } else {                                                                                \
      /* 设置实体状态 (Set entity status) */                                          \
      signal = dds_entity_status_set(&e->m_entity, DDS_##NAME_##_STATUS);                   \
    }                                                                                       \
    /* 解锁观察者互斥锁 (Unlock observers mutex) */                                 \
    ddsrt_mutex_unlock(&e->m_entity.m_observers_lock);                                      \
    /* 调用回调函数 (Invoke callback function) */                                     \
    listener->on_##name_(e->m_entity.m_hdllink.hdl, lst.name_, listener->on_##name_##_arg); \
    /* 锁定观察者互斥锁 (Lock observers mutex) */                                   \
    ddsrt_mutex_lock(&e->m_entity.m_observers_lock);                                        \
    /* 判断是否需要返回 false (Check if returning false is needed) */               \
    if (!signal)                                                                            \
      return false;                                                                         \
    else {                                                                                  \
      /* 获取实体状态和掩码 (Get entity status and mask) */                        \
      const uint32_t sm = ddsrt_atomic_ld32(&e->m_entity.m_status.m_status_and_mask);       \
      /* 返回状态判断结果 (Return the result of status judgement) */                \
      return ((sm & (sm >> SAM_ENABLED_SHIFT)) & DDS_##NAME_##_STATUS) != 0;                \
    }                                                                                       \
  }

/**
 * @brief 定义状态回调函数实现 (Define the implementation of a status callback function)
 * 这段代码定义了一个宏STATUS_CB_IMPL，用于生成状态回调函数的实现。在这个宏中，首先调用STATUS_CB_IMPL_INVOKE宏来生成回调函数的调用部分。然后，定义一个静态状态回调函数status_cb_##name_，该函数接收两个参数：一个指向dds_##entity_kind_结构体的指针和一个指向ddsi_status_cb_data_t结构体的指针。
 * 在回调函数内部，首先获取实体的监听器，然后更新状态。接下来，定义一个信号标志，并根据是否设置了回调函数来设置实体状态或调用回调函数。最后，如果信号标志为真，则通知实体观察者。
 * @param entity_kind_ 实体类型（如：reader、writer等）(Entity kind, e.g., reader, writer, etc.)
 * @param name_ 状态名称（小写）(Status name in lowercase)
 * @param NAME_ 状态名称（大写）(Status name in uppercase)
 * @param ... 可变参数列表 (Variable argument list)
 */
#define STATUS_CB_IMPL(entity_kind_, name_, NAME_, ...)                                                         \
  STATUS_CB_IMPL_INVOKE(entity_kind_, name_, NAME_, __VA_ARGS__)                                                \
  /**                                                                                                           \
   * @brief 状态回调函数 (Status callback function)                                                       \
   *                                                                                                            \
   * @param e 指向dds_##entity_kind_结构体的指针 (Pointer to dds_##entity_kind_ structure)              \
   * @param data 指向ddsi_status_cb_data_t结构体的指针 (Pointer to ddsi_status_cb_data_t                \
   * structure)                                                                                                 \
   */                                                                                                           \
  static void status_cb_##name_(dds_##entity_kind_ *const e, const ddsi_status_cb_data_t *data) {               \
    /* 获取实体的监听器 (Get the entity's listener) */                                                  \
    struct dds_listener const *const listener = &e->m_entity.m_listener;                                        \
    /* 更新状态 (Update the status) */                                                                      \
    update_##name_(&e->m_##name_##_status, data);                                                               \
    /* 定义信号标志 (Define signal flag) */                                                               \
    bool signal;                                                                                                \
    /* 如果没有设置回调函数，则设置实体状态 (If no callback is set, set the entity status) */ \
    if (listener->on_##name_ == 0)                                                                              \
      signal = dds_entity_status_set(&e->m_entity, DDS_##NAME_##_STATUS);                                       \
    /* 否则，调用回调函数 (Otherwise, invoke the callback function) */                                 \
    else                                                                                                        \
      signal = status_cb_##name_##_invoke(e);                                                                   \
    /* 如果信号标志为真，则通知实体观察者 (If signal flag is true, notify entity observers) */ \
    if (signal) dds_entity_observers_signal(&e->m_entity, DDS_##NAME_##_STATUS);                                \
  }

/** @component generic_entity */
/**
 * @brief 获取实体的参与者 (Get the participant of the entity)
 *
 * @param[in] e 指向 dds_entity 的指针 (Pointer to the dds_entity)
 * @return 返回实体的参与者 (Returns the participant of the entity)
 */
dds_participant *dds_entity_participant(const dds_entity *e);

/** @component generic_entity */
/**
 * @brief 获取实体参与者的 GUID (Get the GUID of the entity participant)
 *
 * @param[in] e 指向 dds_entity 的指针 (Pointer to the dds_entity)
 * @return 返回实体参与者的 GUID (Returns the GUID of the entity participant)
 */
const ddsi_guid_t *dds_entity_participant_guid(const dds_entity *e);

/** @component generic_entity */
/**
 * @brief 在释放实体之前进行最终的反初始化操作 (Perform final deinitialization before freeing the
 * entity)
 *
 * @param[in,out] e 指向 dds_entity 的指针 (Pointer to the dds_entity)
 */
void dds_entity_final_deinit_before_free(dds_entity *e);

/** @component generic_entity */
/**
 * @brief 判断实体是否在给定根实体的范围内 (Determine if the entity is within the scope of the given
 * root entity)
 *
 * @param[in] e 指向 dds_entity 的指针 (Pointer to the dds_entity)
 * @param[in] root 指向根 dds_entity 的指针 (Pointer to the root dds_entity)
 * @return 如果实体在给定根实体的范围内，则返回 true，否则返回 false (Returns true if the entity is
 * within the scope of the given root entity, false otherwise)
 */
bool dds_entity_in_scope(const dds_entity *e, const dds_entity *root);

/**
 * @brief 删除实现状态枚举 (Delete implementation state enumeration)
 */
enum delete_impl_state {
  DIS_USER,     /**< 用户直接调用删除 (Delete invoked directly by application) */
  DIS_EXPLICIT, /**< 在此实体上进行显式删除 (Explicit delete on this entity) */
  DIS_FROM_PARENT, /**< 因为父实体被删除而调用 (Called because the parent is being deleted) */
  DIS_IMPLICIT /**< 从子实体调用；如果没有子实体，则隐式删除 (Called from child; delete if implicit
                  without children) */
};

/** @component generic_entity */
/**
 * @brief 删除固定的实体 (Delete a pinned entity)
 *
 * @param[in,out] e 指向 dds_entity 的指针 (Pointer to the dds_entity)
 * @param[in] delstate 删除实现状态 (Delete implementation state)
 * @return 返回操作结果 (Returns the operation result)
 */
dds_return_t dds_delete_impl_pinned(dds_entity *e, enum delete_impl_state delstate);

/** @component generic_entity */
/**
 * @brief 固定实体 (Pin an entity)
 *
 * @param[in] hdl 实体句柄 (Entity handle)
 * @param[out] eptr 指向 dds_entity 的指针的指针 (Pointer to a pointer to the dds_entity)
 * @return 返回操作结果 (Returns the operation result)
 */
dds_return_t dds_entity_pin(dds_entity_t hdl, dds_entity **eptr);

/** @component generic_entity */
/**
 * @brief 用原始状态固定实体 (Pin an entity with origin)
 *
 * @param[in] hdl 实体句柄 (Entity handle)
 * @param[in] from_user 是否来自用户 (Whether it's from user)
 * @param[out] eptr 指向 dds_entity 的指针的指针 (Pointer to a pointer to the dds_entity)
 * @return 返回操作结果 (Returns the operation result)
 */
dds_return_t dds_entity_pin_with_origin(dds_entity_t hdl, bool from_user, dds_entity **eptr);

/** @component generic_entity */
/**
 * @brief 为删除操作固定实体 (Pin an entity for delete operation)
 *
 * @param[in] hdl 实体句柄 (Entity handle)
 * @param[in] explicit 是否显式删除 (Whether it's explicit delete)
 * @param[in] from_user 是否来自用户 (Whether it's from user)
 * @param[out] eptr 指向 dds_entity 的指针的指针 (Pointer to a pointer to the dds_entity)
 * @return 返回操作结果 (Returns the operation result)
 */
dds_return_t dds_entity_pin_for_delete(dds_entity_t hdl,
                                       bool explicit,
                                       bool from_user,
                                       dds_entity **eptr);

/** @component generic_entity */

/**
 * @brief 解除实体的引用 (Unpin an entity)
 *
 * @param[in] e 要解除引用的实体指针 (Pointer to the entity to unpin)
 */
void dds_entity_unpin(dds_entity *e);

/** @component generic_entity */

/**
 * @brief 锁定具有特定类型的实体 (Lock an entity with a specific kind)
 *
 * @param[in] hdl 实体句柄 (Entity handle)
 * @param[in] kind 实体类型 (Entity kind)
 * @param[out] e 指向锁定实体的指针 (Pointer to the locked entity)
 * @return 成功时返回 DDS_RETCODE_OK，否则返回错误代码 (Returns DDS_RETCODE_OK on success, otherwise
 * returns an error code)
 */
dds_return_t dds_entity_lock(dds_entity_t hdl, dds_entity_kind_t kind, dds_entity **e);

/** @component generic_entity */

/**
 * @brief 解锁实体 (Unlock an entity)
 *
 * @param[in] e 要解锁的实体指针 (Pointer to the entity to unlock)
 */
void dds_entity_unlock(dds_entity *e);

/** @component generic_entity */

/**
 * @brief 注册观察者 (Register an observer)
 *
 * @param[in] observed 被观察的实体 (Observed entity)
 * @param[in] observer 观察者实体 (Observer entity)
 * @param[in] cb 回调函数 (Callback function)
 * @param[in] attach_cb 附加回调函数 (Attach callback function)
 * @param[in] attach_arg 附加回调函数的参数 (Argument for the attach callback function)
 * @param[in] delete_cb 删除回调函数 (Delete callback function)
 * @return 成功时返回 DDS_RETCODE_OK，否则返回错误代码 (Returns DDS_RETCODE_OK on success, otherwise
 * returns an error code)
 */
dds_return_t dds_entity_observer_register(dds_entity *observed,
                                          dds_waitset *observer,
                                          dds_entity_callback_t cb,
                                          dds_entity_attach_callback_t attach_cb,
                                          void *attach_arg,
                                          dds_entity_delete_callback_t delete_cb);

/** @component generic_entity */

/**
 * @brief 注销观察者 (Unregister an observer)
 *
 * @param[in] observed 被观察的实体 (Observed entity)
 * @param[in] observer 观察者实体 (Observer entity)
 * @param[in] invoke_delete_cb 是否调用删除回调函数 (Whether to invoke the delete callback function)
 * @return 成功时返回 DDS_RETCODE_OK，否则返回错误代码 (Returns DDS_RETCODE_OK on success, otherwise
 * returns an error code)
 */
dds_return_t dds_entity_observer_unregister(dds_entity *observed,
                                            dds_waitset *observer,
                                            bool invoke_delete_cb);

/** @component generic_entity */

/**
 * @brief 对多种类型的实体执行未实现的操作 (Perform unimplemented operation for many kinds of
 * entities)
 *
 * @param[in] handle 实体句柄 (Entity handle)
 * @param[in] nkinds 类型数量 (Number of kinds)
 * @param[in] kinds 实体类型数组 (Array of entity kinds)
 * @return 成功时返回 DDS_RETCODE_OK，否则返回错误代码 (Returns DDS_RETCODE_OK on success, otherwise
 * returns an error code)
 */
dds_return_t dds_generic_unimplemented_operation_manykinds(dds_entity_t handle,
                                                           size_t nkinds,
                                                           const dds_entity_kind_t *kinds);

/** @component generic_entity */

/**
 * @brief 对特定类型的实体执行未实现的操作 (Perform unimplemented operation for a specific kind of
 * entity)
 *
 * @param[in] handle 实体句柄 (Entity handle)
 * @param[in] kind 实体类型 (Entity kind)
 * @return 成功时返回 DDS_RETCODE_OK，否则返回错误代码 (Returns DDS_RETCODE_OK on success, otherwise
 * returns an error code)
 */
dds_return_t dds_generic_unimplemented_operation(dds_entity_t handle, dds_entity_kind_t kind);

#if defined(__cplusplus)
}
#endif
#endif /* DDS__ENTITY_H */
