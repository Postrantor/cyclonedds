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

#include "dds/dds.h"
#include "dds__listener.h"

/**
 * @brief 创建并初始化一个dds_listener_t结构
 *
 * @param[in] arg 传递给监听器回调函数的参数
 * @return dds_listener_t* 返回创建的监听器指针
 */
dds_listener_t *dds_create_listener(void *arg)
{
  dds_listener_t *l = dds_alloc(sizeof(*l)); // 分配内存空间给监听器
  dds_reset_listener(l);                     // 重置监听器
  l->on_inconsistent_topic_arg = arg;
  l->on_liveliness_lost_arg = arg;
  l->on_offered_deadline_missed_arg = arg;
  l->on_offered_incompatible_qos_arg = arg;
  l->on_data_on_readers_arg = arg;
  l->on_sample_lost_arg = arg;
  l->on_data_available_arg = arg;
  l->on_sample_rejected_arg = arg;
  l->on_liveliness_changed_arg = arg;
  l->on_requested_deadline_missed_arg = arg;
  l->on_requested_incompatible_qos_arg = arg;
  l->on_publication_matched_arg = arg;
  l->on_subscription_matched_arg = arg;
  return l; // 返回创建的监听器指针
}

/**
 * @brief 删除并释放dds_listener_t结构
 *
 * @param[in] listener 要删除的监听器指针
 */
void dds_delete_listener(dds_listener_t *__restrict listener)
{
  dds_free(listener); // 释放监听器占用的内存空间
}

/**
 * @brief 重置dds_listener_t结构的所有回调函数和相关参数
 *
 * @param[in] listener 要重置的监听器指针
 */
void dds_reset_listener(dds_listener_t *__restrict listener)
{
  if (listener) // 如果监听器指针非空
  {
    dds_listener_t *const l = listener;
    l->inherited = 0;
    l->reset_on_invoke = 0;
    l->on_data_available = 0;
    l->on_data_on_readers = 0;
    l->on_inconsistent_topic = 0;
    l->on_liveliness_changed = 0;
    l->on_liveliness_lost = 0;
    l->on_offered_deadline_missed = 0;
    l->on_offered_incompatible_qos = 0;
    l->on_publication_matched = 0;
    l->on_requested_deadline_missed = 0;
    l->on_requested_incompatible_qos = 0;
    l->on_sample_lost = 0;
    l->on_sample_rejected = 0;
    l->on_subscription_matched = 0;
  }
}

/**
 * @brief 复制监听器
 *
 * @param[out] dst 目标监听器指针
 * @param[in] src 源监听器指针
 */
void dds_copy_listener(dds_listener_t *__restrict dst, const dds_listener_t *__restrict src)
{
  // 如果目标和源监听器都不为空，则复制源监听器到目标监听器
  if (dst && src)
    *dst = *src;
}

/**
 * @brief 合并监听器
 *
 * @param[in] inherited 继承的状态
 * @param[in] dst 目标回调函数指针
 * @param[in] src 源回调函数指针
 * @return bool 返回是否需要合并
 */
static bool dds_combine_listener_merge(uint32_t inherited, void (*dst)(void), void (*src)(void))
{
  (void)inherited;
  // 如果目标回调函数为空且源回调函数不为空，则需要合并
  return dst == 0 && src != 0;
}

/**
 * @brief 覆盖继承的监听器
 *
 * @param[in] inherited 继承的状态
 * @param[in] dst 目标回调函数指针
 * @param[in] src 源回调函数指针
 * @return bool 返回是否覆盖继承
 */
static bool dds_combine_listener_override_inherited(uint32_t inherited, void (*dst)(void), void (*src)(void))
{
  (void)dst;
  (void)src;
  // 返回继承的状态
  return inherited;
}

/**
 * @brief 复制位
 *
 * @param[in] a 第一个整数
 * @param[in] b 第二个整数
 * @param[in] mask 掩码
 * @return uint32_t 返回复制后的位
 */
static uint32_t copy_bits(uint32_t a, uint32_t b, uint32_t mask)
{
  // 使用掩码从b中复制位到a
  return (a & ~mask) | (b & mask);
}

/**
 * @brief 合并重置调用
 *
 * @param[in] dst 目标监听器指针
 * @param[in] src 源监听器指针
 * @param[in] status 状态
 * @return uint32_t 返回合并后的重置调用
 */
static uint32_t combine_reset_on_invoke(const dds_listener_t *dst, const dds_listener_t *src, uint32_t status)
{
  // 使用状态作为掩码，从源监听器复制重置调用到目标监听器
  return copy_bits(dst->reset_on_invoke, src->reset_on_invoke, status);
}

/**
 * @brief 组合监听器
 *
 * @param[in] op 操作函数指针
 * @param[out] dst 目标监听器指针
 * @param[in] src 源监听器指针
 */
static void dds_combine_listener(bool (*op)(uint32_t inherited, void (*)(void), void (*)(void)), dds_listener_t *__restrict dst, const dds_listener_t *__restrict src)
{
#define C(NAME_, name_)                                                                                              \
  do                                                                                                                 \
  {                                                                                                                  \
    if (op(dst->inherited & DDS_##NAME_##_STATUS, (void (*)(void))dst->on_##name_, (void (*)(void))src->on_##name_)) \
    {                                                                                                                \
      dst->inherited |= DDS_##NAME_##_STATUS;                                                                        \
      dst->reset_on_invoke = combine_reset_on_invoke(dst, src, DDS_##NAME_##_STATUS);                                \
      dst->on_##name_ = src->on_##name_;                                                                             \
      dst->on_##name_##_arg = src->on_##name_##_arg;                                                                 \
    }                                                                                                                \
  } while (0)
  // 遍历所有状态，根据操作函数决定是否合并监听器
  C(DATA_AVAILABLE, data_available);
  C(DATA_ON_READERS, data_on_readers);
  C(INCONSISTENT_TOPIC, inconsistent_topic);
  C(LIVELINESS_CHANGED, liveliness_changed);
  C(LIVELINESS_LOST, liveliness_lost);
  C(OFFERED_DEADLINE_MISSED, offered_deadline_missed);
  C(OFFERED_INCOMPATIBLE_QOS, offered_incompatible_qos);
  C(PUBLICATION_MATCHED, publication_matched);
  C(REQUESTED_DEADLINE_MISSED, requested_deadline_missed);
  C(REQUESTED_INCOMPATIBLE_QOS, requested_incompatible_qos);
  C(SAMPLE_LOST, sample_lost);
  C(SAMPLE_REJECTED, sample_rejected);
  C(SUBSCRIPTION_MATCHED, subscription_matched);
#undef C
}
/**
 * @brief 重写继承的监听器
 * @param dst 目标监听器
 * @param src 源监听器
 */
void dds_override_inherited_listener(dds_listener_t *__restrict dst, const dds_listener_t *__restrict src)
{
  // 如果目标和源监听器都不为空
  if (dst && src)
    // 结合两个监听器，使用覆盖继承的方式
    dds_combine_listener(dds_combine_listener_override_inherited, dst, src);
}

/**
 * @brief 继承监听器
 * @param dst 目标监听器
 * @param src 源监听器
 */
void dds_inherit_listener(dds_listener_t *__restrict dst, const dds_listener_t *__restrict src)
{
  // 如果目标和源监听器都不为空
  if (dst && src)
    // 结合两个监听器，使用合并的方式
    dds_combine_listener(dds_combine_listener_merge, dst, src);
}

/**
 * @brief 合并监听器
 * @param dst 目标监听器
 * @param src 源监听器
 */
void dds_merge_listener(dds_listener_t *__restrict dst, const dds_listener_t *__restrict src)
{
  // 如果目标和源监听器都不为空
  if (dst && src)
  {
    // 获取目标监听器的继承属性
    uint32_t inherited = dst->inherited;
    // 结合两个监听器，使用合并的方式
    dds_combine_listener(dds_combine_listener_merge, dst, src);
    // 设置目标监听器的继承属性
    dst->inherited = inherited;
  }
}

/**
 * @brief 设置监听器参数的宏定义
 * @param NAME_ 名称
 * @param name_ 小写名称
 */
#define DDS_SET_LISTENER_ARG(NAME_, name_)                                                                                                \
  /**                                                                                                                                     \
   * @brief 设置监听器参数                                                                                                         \
   * @param listener 监听器                                                                                                            \
   * @param callback 回调函数                                                                                                         \
   * @param arg 参数                                                                                                                    \
   * @param reset_on_invoke 是否在调用时重置                                                                                      \
   * @return 返回操作结果                                                                                                           \
   */                                                                                                                                     \
  dds_return_t dds_lset_##name_##_arg(dds_listener_t *__restrict listener, dds_on_##name_##_fn callback, void *arg, bool reset_on_invoke) \
  {                                                                                                                                       \
// 如果监听器为空，返回错误参数
if (listener == NULL)
  return DDS_RETCODE_BAD_PARAMETER;                                                                                         // 设置监听器的重置属性
listener->reset_on_invoke = copy_bits(listener->reset_on_invoke, reset_on_invoke ? ~(uint32_t)0 : 0, DDS_##NAME_##_STATUS); // 设置监听器的回调函数
listener->on_##name_ = callback;                                                                                            // 设置监听器的参数
listener->on_##name_##_arg = arg;                                                                                           // 返回操作成功
return DDS_RETCODE_OK;
}
// 使用宏定义设置各种监听器参数
DDS_SET_LISTENER_ARG(DATA_AVAILABLE, data_available)
DDS_SET_LISTENER_ARG(DATA_ON_READERS, data_on_readers)
DDS_SET_LISTENER_ARG(INCONSISTENT_TOPIC, inconsistent_topic)
DDS_SET_LISTENER_ARG(LIVELINESS_CHANGED, liveliness_changed)
DDS_SET_LISTENER_ARG(LIVELINESS_LOST, liveliness_lost)
DDS_SET_LISTENER_ARG(OFFERED_DEADLINE_MISSED, offered_deadline_missed)
DDS_SET_LISTENER_ARG(OFFERED_INCOMPATIBLE_QOS, offered_incompatible_qos)
DDS_SET_LISTENER_ARG(PUBLICATION_MATCHED, publication_matched)
DDS_SET_LISTENER_ARG(REQUESTED_DEADLINE_MISSED, requested_deadline_missed)
DDS_SET_LISTENER_ARG(REQUESTED_INCOMPATIBLE_QOS, requested_incompatible_qos)
DDS_SET_LISTENER_ARG(SAMPLE_LOST, sample_lost)
DDS_SET_LISTENER_ARG(SAMPLE_REJECTED, sample_rejected)
DDS_SET_LISTENER_ARG(SUBSCRIPTION_MATCHED, subscription_matched)
// 取消宏定义
#undef DDS_SET_LISTENER_ARG
/**
 * @brief 定义一个宏，用于设置监听器的回调函数。
 * @param NAME_ 事件名称（大写）
 * @param name_ 事件名称（小写）
 */
#define DDS_SET_LISTENER(NAME_, name_)                                                     \
  /**                                                                                      \
   * @brief 设置监听器的回调函数。                                              \
   * @param listener 监听器指针                                                       \
   * @param callback 回调函数指针                                                    \
   */                                                                                      \
  void dds_lset_##name_(dds_listener_t *__restrict listener, dds_on_##name_##_fn callback) \
  {                                                                                        \
    if (listener)                                                                          \
      (void)dds_lset_##name_##_arg(listener, callback, listener->on_##name_##_arg, true);  \
  }

// 使用宏为各种事件设置监听器
DDS_SET_LISTENER(DATA_AVAILABLE, data_available)
DDS_SET_LISTENER(DATA_ON_READERS, data_on_readers)
DDS_SET_LISTENER(INCONSISTENT_TOPIC, inconsistent_topic)
DDS_SET_LISTENER(LIVELINESS_CHANGED, liveliness_changed)
DDS_SET_LISTENER(LIVELINESS_LOST, liveliness_lost)
DDS_SET_LISTENER(OFFERED_DEADLINE_MISSED, offered_deadline_missed)
DDS_SET_LISTENER(OFFERED_INCOMPATIBLE_QOS, offered_incompatible_qos)
DDS_SET_LISTENER(PUBLICATION_MATCHED, publication_matched)
DDS_SET_LISTENER(REQUESTED_DEADLINE_MISSED, requested_deadline_missed)
DDS_SET_LISTENER(REQUESTED_INCOMPATIBLE_QOS, requested_incompatible_qos)
DDS_SET_LISTENER(SAMPLE_LOST, sample_lost)
DDS_SET_LISTENER(SAMPLE_REJECTED, sample_rejected)
DDS_SET_LISTENER(SUBSCRIPTION_MATCHED, subscription_matched)

// 取消宏定义
#undef DDS_SET_LISTENER

/**
 * @brief 定义一个宏，用于获取监听器的回调函数和参数。
 * @param NAME_ 事件名称（大写）
 * @param name_ 事件名称（小写）
 */
#define DDS_GET_LISTENER_ARG(NAME_, name_)                                                                                                         \
  /**                                                                                                                                              \
   * @brief 获取监听器的回调函数和参数。                                                                                             \
   * @param listener 监听器指针                                                                                                               \
   * @param callback 回调函数指针                                                                                                            \
   * @param arg 参数指针                                                                                                                       \
   * @param reset_on_invoke 是否在调用后重置标志指针                                                                                   \
   * @return 返回操作结果                                                                                                                    \
   */                                                                                                                                              \
  dds_return_t dds_lget_##name_##_arg(const dds_listener_t *__restrict listener, dds_on_##name_##_fn *callback, void **arg, bool *reset_on_invoke) \
  {                                                                                                                                                \
    if (listener == NULL)                                                                                                                          \
      return DDS_RETCODE_BAD_PARAMETER;                                                                                                            \
    if (callback)                                                                                                                                  \
      *callback = listener->on_##name_;                                                                                                            \
    if (arg)                                                                                                                                       \
      *arg = listener->on_##name_##_arg;                                                                                                           \
    if (reset_on_invoke)                                                                                                                           \
      *reset_on_invoke = (listener->reset_on_invoke & DDS_##NAME_##_STATUS) != 0;                                                                  \
    return DDS_RETCODE_OK;                                                                                                                         \
  }

// 使用宏为各种事件获取监听器参数
DDS_GET_LISTENER_ARG(DATA_AVAILABLE, data_available)
DDS_GET_LISTENER_ARG(DATA_ON_READERS, data_on_readers)
DDS_GET_LISTENER_ARG(INCONSISTENT_TOPIC, inconsistent_topic)
DDS_GET_LISTENER_ARG(LIVELINESS_CHANGED, liveliness_changed)
DDS_GET_LISTENER_ARG(LIVELINESS_LOST, liveliness_lost)
DDS_GET_LISTENER_ARG(OFFERED_DEADLINE_MISSED, offered_deadline_missed)
DDS_GET_LISTENER_ARG(OFFERED_INCOMPATIBLE_QOS, offered_incompatible_qos)
DDS_GET_LISTENER_ARG(PUBLICATION_MATCHED, publication_matched)
DDS_GET_LISTENER_ARG(REQUESTED_DEADLINE_MISSED, requested_deadline_missed)
DDS_GET_LISTENER_ARG(REQUESTED_INCOMPATIBLE_QOS, requested_incompatible_qos)
DDS_GET_LISTENER_ARG(SAMPLE_LOST, sample_lost)
DDS_GET_LISTENER_ARG(SAMPLE_REJECTED, sample_rejected)
DDS_GET_LISTENER_ARG(SUBSCRIPTION_MATCHED, subscription_matched)

// 取消宏定义
#undef DDS_GET_LISTENER_ARG

/**
 * @brief 定义一个宏，用于获取监听器的回调函数。
 * @param name_ 事件名称（小写）
 */
#define DDS_GET_LISTENER(name_)                                                                   \
  /**                                                                                             \
   * @brief 获取监听器的回调函数。                                                     \
   * @param listener 监听器指针                                                              \
   * @param callback 回调函数指针                                                           \
   */                                                                                             \
  void dds_lget_##name_(const dds_listener_t *__restrict listener, dds_on_##name_##_fn *callback) \
  {                                                                                               \
    (void)dds_lget_##name_##_arg(listener, callback, NULL, NULL);                                 \
  }

// 使用宏为各种事件获取监听器
DDS_GET_LISTENER(data_available)
DDS_GET_LISTENER(data_on_readers)
DDS_GET_LISTENER(inconsistent_topic)
DDS_GET_LISTENER(liveliness_changed)
DDS_GET_LISTENER(liveliness_lost)
DDS_GET_LISTENER(offered_deadline_missed)
DDS_GET_LISTENER(offered_incompatible_qos)
DDS_GET_LISTENER(publication_matched)
DDS_GET_LISTENER(requested_deadline_missed)
DDS_GET_LISTENER(requested_incompatible_qos)
DDS_GET_LISTENER(sample_lost)
DDS_GET_LISTENER(sample_rejected)
DDS_GET_LISTENER(subscription_matched)

// 取消宏定义
#undef DDS_GET_LISTENER
