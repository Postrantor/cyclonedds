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

/**
 * @defgroup listener (Listener API)
 * @ingroup dds
 *
 * This defines the public API of listeners in the
 * Eclipse Cyclone DDS C language binding.
 */
#ifndef _DDS_PUBLIC_LISTENER_H_
#define _DDS_PUBLIC_LISTENER_H_

#include "dds/ddsc/dds_public_impl.h"
#include "dds/ddsc/dds_public_status.h"
#include "dds/export.h"

#if defined(__cplusplus)
extern "C" {
#endif

/* Listener callbacks */
// 监听器回调

// 不一致主题回调函数
// Inconsistent topic callback function
typedef void (*dds_on_inconsistent_topic_fn)(dds_entity_t topic,
                                             const dds_inconsistent_topic_status_t status,
                                             void* arg);
// 生存期丢失回调函数
// Liveliness lost callback function
typedef void (*dds_on_liveliness_lost_fn)(dds_entity_t writer,
                                          const dds_liveliness_lost_status_t status,
                                          void* arg);
// 提供的截止日期错过回调函数
// Offered deadline missed callback function
typedef void (*dds_on_offered_deadline_missed_fn)(dds_entity_t writer,
                                                  const dds_offered_deadline_missed_status_t status,
                                                  void* arg);
// 提供的不兼容QoS回调函数
// Offered incompatible QoS callback function
typedef void (*dds_on_offered_incompatible_qos_fn)(
    dds_entity_t writer, const dds_offered_incompatible_qos_status_t status, void* arg);
// 数据在读取器上的回调函数
// Data on readers callback function
typedef void (*dds_on_data_on_readers_fn)(dds_entity_t subscriber, void* arg);
// 样本丢失回调函数
// Sample lost callback function
typedef void (*dds_on_sample_lost_fn)(dds_entity_t reader,
                                      const dds_sample_lost_status_t status,
                                      void* arg);
// 数据可用回调函数
// Data available callback function
typedef void (*dds_on_data_available_fn)(dds_entity_t reader, void* arg);
// 样本拒绝回调函数
// Sample rejected callback function
typedef void (*dds_on_sample_rejected_fn)(dds_entity_t reader,
                                          const dds_sample_rejected_status_t status,
                                          void* arg);
// 生存期更改回调函数
// Liveliness changed callback function
typedef void (*dds_on_liveliness_changed_fn)(dds_entity_t reader,
                                             const dds_liveliness_changed_status_t status,
                                             void* arg);
// 请求的截止日期错过回调函数
// Requested deadline missed callback function
typedef void (*dds_on_requested_deadline_missed_fn)(
    dds_entity_t reader, const dds_requested_deadline_missed_status_t status, void* arg);
// 请求的不兼容QoS回调函数
// Requested incompatible QoS callback function
typedef void (*dds_on_requested_incompatible_qos_fn)(
    dds_entity_t reader, const dds_requested_incompatible_qos_status_t status, void* arg);
// 出版匹配回调函数
// Publication matched callback function
typedef void (*dds_on_publication_matched_fn)(dds_entity_t writer,
                                              const dds_publication_matched_status_t status,
                                              void* arg);
// 订阅匹配回调函数
// Subscription matched callback function
typedef void (*dds_on_subscription_matched_fn)(dds_entity_t reader,
                                               const dds_subscription_matched_status_t status,
                                               void* arg);

/**
 * @anchor DDS_LUNSET
 * @ingroup internal
 * @brief Default initial value (nullptr) for listener functions.
 */
// 监听器函数的默认初始值（空指针）
#define DDS_LUNSET 0

/**
 * @brief DDS Listener struct (opaque)
 * @ingroup listener
 */
// DDS监听器结构体（不透明）
struct dds_listener;

/**
 * @brief DDS Listener type (opaque)
 * @ingroup listener
 */
// DDS监听器类型（不透明）
typedef struct dds_listener dds_listener_t;

/**
 * @ingroup listener
 * @component listener_obj
 * @brief Allocate memory and initializes to default values (@ref DDS_LUNSET) of a listener
 *
 * @param[in] arg optional pointer that will be passed on to the listener callbacks
 *
 * @returns Returns a pointer to the allocated memory for dds_listener_t structure.
 */
// 为监听器分配内存并初始化为默认值（@ref DDS_LUNSET）
// 参数：arg 可选指针，将传递给监听器回调函数
// 返回值：返回指向dds_listener_t结构的分配内存的指针
DDS_EXPORT dds_listener_t* dds_create_listener(void* arg);

/**
 * @ingroup listener
 * @component listener_obj
 * @brief Delete the memory allocated to listener structure
 *
 * @param[in] listener pointer to the listener struct to delete
 */
// 删除分配给监听器结构的内存
// 参数：listener 指向要删除的监听器结构的指针
DDS_EXPORT void dds_delete_listener(dds_listener_t* __restrict listener);

/**
 * @ingroup listener
 * @component listener_obj
 * @brief Reset the listener structure contents to @ref DDS_LUNSET
 *
 * @param[in,out] listener pointer to the listener struct to reset
 */
// 将监听器结构内容重置为 @ref DDS_LUNSET
// 参数：listener 指向要重置的监听器结构的指针
DDS_EXPORT void dds_reset_listener(dds_listener_t* __restrict listener);

/**
 * @ingroup listener
 * @component listener_obj
 * @brief 复制监听器回调从源到目标 (Copy the listener callbacks from source to destination)
 *
 * @param[in,out] dst 指向要复制内容的目标监听器结构的指针 (The pointer to the destination listener
 * structure, where the content is to be copied)
 * @param[in] src 要复制的源监听器结构的指针 (The pointer to the source listener structure to be
 * copied)
 */
DDS_EXPORT void dds_copy_listener(dds_listener_t* __restrict dst,
                                  const dds_listener_t* __restrict src);

/**
 * @ingroup listener
 * @component listener_obj
 * @brief 从源复制监听器回调到目标，除非已经设置 (Copy the listener callbacks from source to
 * destination, unless already set)
 *
 * 在 @p dst 中已经设置的任何监听器回调（包括 NULL）都将被跳过，只有设置为 DDS_LUNSET 的那些才会从
 * @p src 中复制。 (Any listener callbacks already set in @p dst (including NULL) are skipped, only
 * those set to DDS_LUNSET are copied from @p src.)
 *
 * @param[in,out] dst 指向要合并内容的目标监听器结构的指针 (The pointer to the destination listener
 * structure, where the content is merged)
 * @param[in] src 要复制的源监听器结构的指针 (The pointer to the source listener structure to be
 * copied)
 */
DDS_EXPORT void dds_merge_listener(dds_listener_t* __restrict dst,
                                   const dds_listener_t* __restrict src);

/************************************************************************************************
 *  Setters
 ************************************************************************************************/

/**
 * @defgroup listener_setters (设置器) (Setters)
 * @ingroup listener
 */

/**
 * @ingroup listener_setters
 * @component listener_obj
 * @brief 在监听器结构中设置 data_available 回调和参数。(Set the data_available callback and
 * argument in the listener structure.)
 *
 * @param[in,out] listener 要更新的监听器结构 (listener structure to update)
 * @param[in] callback 要设置的回调或空指针 (the callback to set or a null pointer)
 * @param[in] arg 传递给回调函数的未解释的回调参数 (callback argument that is passed uninterpreted
 * to the callback function)
 * @param[in] reset_on_invoke 当监听器回调被调用时，是否清除状态 (whether or not the status should
 * be cleared when the listener callback is invoked)
 *
 * @retval DDS_RETCODE_OK 成功 (success)
 * @retval DDS_RETCODE_BAD_PARAMETER 监听器是空指针 (listener is a null pointer)
 */
DDS_EXPORT dds_return_t dds_lset_data_available_arg(dds_listener_t* __restrict listener,
                                                    dds_on_data_available_fn callback,
                                                    void* arg,
                                                    bool reset_on_invoke);

/**
 * @ingroup listener_setters
 * @component listener_obj
 * @brief 在监听器结构中设置 data_on_readers 回调和参数。(Set the data_on_readers callback and
 * argument in the listener structure.)
 *
 * @param[in,out] listener 要更新的监听器结构 (listener structure to update)
 * @param[in] callback 要设置的回调或空指针 (the callback to set or a null pointer)
 * @param[in] arg 传递给回调函数的未解释的回调参数 (callback argument that is passed uninterpreted
 * to the callback function)
 * @param[in] reset_on_invoke 当监听器回调被调用时，是否清除状态 (whether or not the status should
 * be cleared when the listener callback is invoked)
 *
 * @retval DDS_RETCODE_OK 成功 (success)
 * @retval DDS_RETCODE_BAD_PARAMETER 监听器是空指针 (listener is a null pointer)
 */
DDS_EXPORT dds_return_t dds_lset_data_on_readers_arg(dds_listener_t* __restrict listener,
                                                     dds_on_data_on_readers_fn callback,
                                                     void* arg,
                                                     bool reset_on_invoke);

/**
 * @ingroup listener_setters
 * @component listener_obj
 * @brief 在监听器结构中设置 inconsistent_topic 回调和参数。(Set the inconsistent_topic callback and
 * argument in the listener structure.)
 *
 * @param[in,out] listener 要更新的监听器结构 (listener structure to update)
 * @param[in] callback 要设置的回调或空指针 (the callback to set or a null pointer)
 * @param[in] arg 传递给回调函数的未解释的回调参数 (callback argument that is passed uninterpreted
 * to the callback function)
 * @param[in] reset_on_invoke 当监听器回调被调用时，是否清除状态 (whether or not the status should
 * be cleared when the listener callback is invoked)
 *
 * @retval DDS_RETCODE_OK 成功 (success)
 * @retval DDS_RETCODE_BAD_PARAMETER 监听器是空指针 (listener is a null pointer)
 */
DDS_EXPORT dds_return_t dds_lset_inconsistent_topic_arg(dds_listener_t* __restrict listener,
                                                        dds_on_inconsistent_topic_fn callback,
                                                        void* arg,
                                                        bool reset_on_invoke);

/**
 * @ingroup listener_setters
 * @component listener_obj
 * @brief 在监听器结构中设置 liveliness_changed 回调和参数。(Set the liveliness_changed callback and
 * argument in the listener structure.)
 *
 * @param[in,out] listener 要更新的监听器结构 (listener structure to update)
 * @param[in] callback 要设置的回调或空指针 (the callback to set or a null pointer)
 * @param[in] arg 传递给回调函数的未解释的回调参数 (callback argument that is passed uninterpreted
 * to the callback function)
 * @param[in] reset_on_invoke 当监听器回调被调用时，是否清除状态 (whether or not the status should
 * be cleared when the listener callback is invoked)
 *
 * @retval DDS_RETCODE_OK 成功 (success)
 * @retval DDS_RETCODE_BAD_PARAMETER 监听器是空指针 (listener is a null pointer)
 */
DDS_EXPORT dds_return_t dds_lset_liveliness_changed_arg(dds_listener_t* __restrict listener,
                                                        dds_on_liveliness_changed_fn callback,
                                                        void* arg,
                                                        bool reset_on_invoke);

/**
 * @ingroup listener_setters
 * @component listener_obj
 * @brief 在监听器结构中设置 liveliness_lost 回调和参数。(Set the liveliness_lost callback and
 * argument in the listener structure.)
 *
 * @param[in,out] listener 要更新的监听器结构 (listener structure to update)
 * @param[in] callback 要设置的回调或空指针 (the callback to set or a null pointer)
 * @param[in] arg 传递给回调函数的未解释的回调参数 (callback argument that is passed uninterpreted
 * to the callback function)
 * @param[in] reset_on_invoke 当监听器回调被调用时，是否清除状态 (whether or not the status should
 * be cleared when the listener callback is invoked)
 *
 * @retval DDS_RETCODE_OK 成功 (success)
 * @retval DDS_RETCODE_BAD_PARAMETER 监听器是空指针 (listener is a null pointer)
 */
DDS_EXPORT dds_return_t dds_lset_liveliness_lost_arg(dds_listener_t* __restrict listener,
                                                     dds_on_liveliness_lost_fn callback,
                                                     void* arg,
                                                     bool reset_on_invoke);

/**
 * @ingroup listener_setters
 * @component listener_obj
 * @brief 设置listener结构中的offered_deadline_missed回调函数和参数。
 * Set the offered_deadline_missed callback and argument in the listener structure.
 *
 * @param[in,out] listener 要更新的监听器结构
 * listener structure to update
 * @param[in] callback 要设置的回调函数或空指针
 * the callback to set or a null pointer
 * @param[in] arg 传递给回调函数的未解释的回调参数
 * callback argument that is passed uninterpreted to the callback function
 * @param[in] reset_on_invoke 在调用监听器回调时是否应清除状态
 * whether or not the status should be cleared when the listener callback
 * is invoked
 *
 * @retval DDS_RETCODE_OK 成功
 * success
 * @retval DDS_RETCODE_BAD_PARAMETER 监听器是空指针
 * listener is a null pointer
 */
DDS_EXPORT dds_return_t
dds_lset_offered_deadline_missed_arg(dds_listener_t* __restrict listener,
                                     dds_on_offered_deadline_missed_fn callback,
                                     void* arg,
                                     bool reset_on_invoke);

/**
 * @ingroup listener_setters
 * @component listener_obj
 * @brief 设置listener结构中的offered_incompatible_qos回调函数和参数。
 * Set the offered_incompatible_qos callback and argument in the listener structure.
 *
 * @param[in,out] listener 要更新的监听器结构
 * listener structure to update
 * @param[in] callback 要设置的回调函数或空指针
 * the callback to set or a null pointer
 * @param[in] arg 传递给回调函数的未解释的回调参数
 * callback argument that is passed uninterpreted to the callback function
 * @param[in] reset_on_invoke 在调用监听器回调时是否应清除状态
 * whether or not the status should be cleared when the listener callback
 * is invoked
 *
 * @retval DDS_RETCODE_OK 成功
 * success
 * @retval DDS_RETCODE_BAD_PARAMETER 监听器是空指针
 * listener is a null pointer
 */
DDS_EXPORT dds_return_t
dds_lset_offered_incompatible_qos_arg(dds_listener_t* __restrict listener,
                                      dds_on_offered_incompatible_qos_fn callback,
                                      void* arg,
                                      bool reset_on_invoke);

/**
 * @ingroup listener_setters
 * @component listener_obj
 * @brief 设置listener结构中的publication_matched回调函数和参数。
 * Set the publication_matched callback and argument in the listener structure.
 *
 * @param[in,out] listener 要更新的监听器结构
 * listener structure to update
 * @param[in] callback 要设置的回调函数或空指针
 * the callback to set or a null pointer
 * @param[in] arg 传递给回调函数的未解释的回调参数
 * callback argument that is passed uninterpreted to the callback function
 * @param[in] reset_on_invoke 在调用监听器回调时是否应清除状态
 * whether or not the status should be cleared when the listener callback
 * is invoked
 *
 * @retval DDS_RETCODE_OK 成功
 * success
 * @retval DDS_RETCODE_BAD_PARAMETER 监听器是空指针
 * listener is a null pointer
 */
DDS_EXPORT dds_return_t dds_lset_publication_matched_arg(dds_listener_t* __restrict listener,
                                                         dds_on_publication_matched_fn callback,
                                                         void* arg,
                                                         bool reset_on_invoke);

/**
 * @ingroup listener_setters
 * @component listener_obj
 * @brief 设置listener结构中的requested_deadline_missed回调函数和参数。
 * Set the requested_deadline_missed callback and argument in the listener structure.
 *
 * @param[in,out] listener 要更新的监听器结构
 * listener structure to update
 * @param[in] callback 要设置的回调函数或空指针
 * the callback to set or a null pointer
 * @param[in] arg 传递给回调函数的未解释的回调参数
 * callback argument that is passed uninterpreted to the callback function
 * @param[in] reset_on_invoke 在调用监听器回调时是否应清除状态
 * whether or not the status should be cleared when the listener callback
 * is invoked
 *
 * @retval DDS_RETCODE_OK 成功
 * success
 * @retval DDS_RETCODE_BAD_PARAMETER 监听器是空指针
 * listener is a null pointer
 */
DDS_EXPORT dds_return_t
dds_lset_requested_deadline_missed_arg(dds_listener_t* __restrict listener,
                                       dds_on_requested_deadline_missed_fn callback,
                                       void* arg,
                                       bool reset_on_invoke);

/**
 * @ingroup listener_setters
 * @component listener_obj
 * @brief 设置listener结构中的requested_incompatible_qos回调函数和参数。
 * Set the requested_incompatible_qos callback and argument in the listener structure.
 *
 * @param[in,out] listener 要更新的监听器结构
 * listener structure to update
 * @param[in] callback 要设置的回调函数或空指针
 * the callback to set or a null pointer
 * @param[in] arg 传递给回调函数的未解释的回调参数
 * callback argument that is passed uninterpreted to the callback function
 * @param[in] reset_on_invoke 在调用监听器回调时是否应清除状态
 * whether or not the status should be cleared when the listener callback
 * is invoked
 *
 * @retval DDS_RETCODE_OK 成功
 * success
 * @retval DDS_RETCODE_BAD_PARAMETER 监听器是空指针
 * listener is a null pointer
 */
DDS_EXPORT dds_return_t
dds_lset_requested_incompatible_qos_arg(dds_listener_t* __restrict listener,
                                        dds_on_requested_incompatible_qos_fn callback,
                                        void* arg,
                                        bool reset_on_invoke);

/**
 * @ingroup listener_setters
 * @component listener_obj
 * @brief 设置listener结构中的sample_lost回调函数和参数。
 * Set the sample_lost callback and argument in the listener structure.
 *
 * @param[in,out] listener 要更新的监听器结构
 * listener structure to update
 * @param[in] callback 要设置的回调函数或空指针
 * the callback to set or a null pointer
 * @param[in] arg 传递给回调函数的未解释的回调参数
 * callback argument that is passed uninterpreted to the callback function
 * @param[in] reset_on_invoke 在调用监听器回调时是否应清除状态
 * whether or not the status should be cleared when the listener callback
 * is invoked
 *
 * @retval DDS_RETCODE_OK 成功
 * success
 * @retval DDS_RETCODE_BAD_PARAMETER 监听器是空指针
 * listener is a null pointer
 */
DDS_EXPORT dds_return_t dds_lset_sample_lost_arg(dds_listener_t* __restrict listener,
                                                 dds_on_sample_lost_fn callback,
                                                 void* arg,
                                                 bool reset_on_invoke);

/**
 * @ingroup listener_setters
 * @component listener_obj
 * @brief 设置listener结构中的sample_rejected回调函数和参数。
 * Set the sample_rejected callback and argument in the listener structure.
 *
 * @param[in,out] listener 要更新的监听器结构
 * listener structure to update
 * @param[in] callback 要设置的回调函数或空指针
 * the callback to set or a null pointer
 * @param[in] arg 传递给回调函数的未解释的回调参数
 * callback argument that is passed uninterpreted to the callback function
 * @param[in] reset_on_invoke 在调用监听器回调时是否应清除状态
 * whether or not the status should be cleared when the listener callback
 * is invoked
 *
 * @retval DDS_RETCODE_OK 成功
 * success
 * @retval DDS_RETCODE_BAD_PARAMETER 监听器是空指针
 * listener is a null pointer
 */
DDS_EXPORT dds_return_t dds_lset_sample_rejected_arg(dds_listener_t* __restrict listener,
                                                     dds_on_sample_rejected_fn callback,
                                                     void* arg,
                                                     bool reset_on_invoke);

/**
 * @ingroup listener_setters
 * @component listener_obj
 * @brief 设置监听器结构中的 subscription_matched 回调和参数。
 * @brief Set the subscription_matched callback and argument in the listener structure.
 *
 * @param[in,out] listener 要更新的监听器结构
 * @param[in,out] listener listener structure to update
 * @param[in] callback 要设置的回调或空指针
 * @param[in] callback the callback to set or a null pointer
 * @param[in] arg 传递给回调函数的未解释的回调参数
 * @param[in] arg callback argument that is passed uninterpreted to the callback function
 * @param[in] reset_on_invoke 当监听器回调被调用时，是否清除状态
 * @param[in] reset_on_invoke whether or not the status should be cleared when the listener callback
 * is invoked
 *
 * @retval DDS_RETCODE_OK 成功
 * @retval DDS_RETCODE_OK success
 * @retval DDS_RETCODE_BAD_PARAMETER 监听器是空指针
 * @retval DDS_RETCODE_BAD_PARAMETER listener is a null pointer
 */
DDS_EXPORT dds_return_t dds_lset_subscription_matched_arg(dds_listener_t* __restrict listener,
                                                          dds_on_subscription_matched_fn callback,
                                                          void* arg,
                                                          bool reset_on_invoke);

/**
 * @ingroup listener_setters
 * @component listener_obj
 * @brief 在监听器结构中设置 inconsistent_topic 回调。
 * @brief Set the inconsistent_topic callback in the listener structure.
 *
 * 等同于使用 dds_create_listener() 中传递的参数调用 @ref dds_lset_inconsistent_topic_arg，
 * 并将 reset_on_invoke 设置为 true，然后丢弃结果。
 * Equivalent to calling @ref dds_lset_inconsistent_topic_arg with arg set to the argument passed in
 * dds_create_listener() and reset_on_invoke to true, and throwing away the result.
 *
 * @param[in,out] listener 要更新的监听器结构
 * @param[in,out] listener listener structure to update
 * @param[in] callback 要设置的回调或空指针
 * @param[in] callback the callback to set or a null pointer
 */
DDS_EXPORT void dds_lset_inconsistent_topic(dds_listener_t* __restrict listener,
                                            dds_on_inconsistent_topic_fn callback);

/**
 * @ingroup listener_setters
 * @component listener_obj
 * @brief 在监听器结构中设置 liveliness_lost 回调。
 * @brief Set the liveliness_lost callback in the listener structure.
 *
 * 等同于使用 dds_create_listener() 中传递的参数调用 @ref dds_lset_liveliness_lost_arg，
 * 并将 reset_on_invoke 设置为 true，然后丢弃结果。
 * Equivalent to calling @ref dds_lset_liveliness_lost_arg with arg set to the argument passed in
 * dds_create_listener() and reset_on_invoke to true, and throwing away the result.
 *
 * @param[in,out] listener 要更新的监听器结构
 * @param[in,out] listener listener structure to update
 * @param[in] callback 要设置的回调或空指针
 * @param[in] callback the callback to set or a null pointer
 */
DDS_EXPORT void dds_lset_liveliness_lost(dds_listener_t* __restrict listener,
                                         dds_on_liveliness_lost_fn callback);

/**
 * @ingroup listener_setters
 * @component listener_obj
 * @brief 在监听器结构中设置 offered_deadline_missed 回调。
 * @brief Set the offered_deadline_missed callback in the listener structure.
 *
 * 等同于使用 dds_create_listener() 中传递的参数调用 @ref dds_lset_offered_deadline_missed_arg，
 * 并将 reset_on_invoke 设置为 true，然后丢弃结果。
 * Equivalent to calling @ref dds_lset_offered_deadline_missed_arg with arg set to the argument
 * passed in dds_create_listener() and reset_on_invoke to true, and throwing away the result.
 *
 * @param[in,out] listener 要更新的监听器结构
 * @param[in,out] listener listener structure to update
 * @param[in] callback 要设置的回调或空指针
 * @param[in] callback the callback to set or a null pointer
 */
DDS_EXPORT void dds_lset_offered_deadline_missed(dds_listener_t* __restrict listener,
                                                 dds_on_offered_deadline_missed_fn callback);

/**
 * @ingroup listener_setters
 * @component listener_obj
 * @brief 在监听器结构中设置 offered_incompatible_qos 回调。
 * @brief Set the offered_incompatible_qos callback in the listener structure.
 *
 * 等同于使用 dds_create_listener() 中传递的参数调用 @ref dds_lset_offered_incompatible_qos_arg，
 * 并将 reset_on_invoke 设置为 true，然后丢弃结果。
 * Equivalent to calling @ref dds_lset_offered_incompatible_qos_arg with arg set to the argument
 * passed in dds_create_listener() and reset_on_invoke to true, and throwing away the result.
 *
 * @param[in,out] listener 要更新的监听器结构
 * @param[in,out] listener listener structure to update
 * @param[in] callback 要设置的回调或空指针
 * @param[in] callback the callback to set or a null pointer
 */
DDS_EXPORT void dds_lset_offered_incompatible_qos(dds_listener_t* __restrict listener,
                                                  dds_on_offered_incompatible_qos_fn callback);

/**
 * @ingroup listener_setters
 * @component listener_obj
 * @brief 在监听器结构中设置 data_on_readers 回调。
 * @brief Set the data_on_readers callback in the listener structure.
 *
 * 等同于使用 dds_create_listener() 中传递的参数调用 @ref dds_lset_data_on_readers_arg，
 * 并将 reset_on_invoke 设置为 true，然后丢弃结果。
 * Equivalent to calling @ref dds_lset_data_on_readers_arg with arg set to the argument passed in
 * dds_create_listener() and reset_on_invoke to true, and throwing away the result.
 *
 * @param[in,out] listener 要更新的监听器结构
 * @param[in,out] listener listener structure to update
 * @param[in] callback 要设置的回调或空指针
 * @param[in] callback the callback to set or a null pointer
 */
DDS_EXPORT void dds_lset_data_on_readers(dds_listener_t* __restrict listener,
                                         dds_on_data_on_readers_fn callback);

/**
 * @ingroup listener_setters
 * @component listener_obj
 * @brief 设置 listener 结构中的 sample_lost 回调。 (Set the sample_lost callback in the listener
 * structure.)
 *
 * 等同于使用 dds_create_listener() 中传递的参数将 arg 设置为 true 的 reset_on_invoke 调用 @ref
 * dds_lset_sample_lost_arg ，并丢弃结果。 (Equivalent to calling @ref dds_lset_sample_lost_arg with
 * arg set to the argument passed in dds_create_listener() and reset_on_invoke to true, and throwing
 * away the result.)
 *
 * @param[in,out] listener 要更新的监听器结构 (listener structure to update)
 * @param[in] callback 要设置的回调或空指针 (the callback to set or a null pointer)
 */
DDS_EXPORT void dds_lset_sample_lost(dds_listener_t* __restrict listener,
                                     dds_on_sample_lost_fn callback);

/**
 * @ingroup listener_setters
 * @component listener_obj
 * @brief 设置 listener 结构中的 data_available 回调。 (Set the data_available callback in the
 * listener structure.)
 *
 * 等同于使用 dds_create_listener() 中传递的参数将 arg 设置为 true 的 reset_on_invoke 调用 @ref
 * dds_lset_data_available_arg ，并丢弃结果。 (Equivalent to calling @ref
 * dds_lset_data_available_arg with arg set to the argument passed in dds_create_listener() and
 * reset_on_invoke to true, and throwing away the result.)
 *
 * @param[in,out] listener 要更新的监听器结构 (listener structure to update)
 * @param[in] callback 要设置的回调或空指针 (the callback to set or a null pointer)
 */
DDS_EXPORT void dds_lset_data_available(dds_listener_t* __restrict listener,
                                        dds_on_data_available_fn callback);

/**
 * @ingroup listener_setters
 * @component listener_obj
 * @brief 设置 listener 结构中的 sample_rejected 回调。 (Set the sample_rejected callback in the
 * listener structure.)
 *
 * 等同于使用 dds_create_listener() 中传递的参数将 arg 设置为 true 的 reset_on_invoke 调用 @ref
 * dds_lset_sample_rejected_arg ，并丢弃结果。 (Equivalent to calling @ref
 * dds_lset_sample_rejected_arg with arg set to the argument passed in dds_create_listener() and
 * reset_on_invoke to true, and throwing away the result.)
 *
 * @param[in,out] listener 要更新的监听器结构 (listener structure to update)
 * @param[in] callback 要设置的回调或空指针 (the callback to set or a null pointer)
 */
DDS_EXPORT void dds_lset_sample_rejected(dds_listener_t* __restrict listener,
                                         dds_on_sample_rejected_fn callback);

/**
 * @ingroup listener_setters
 * @component listener_obj
 * @brief 设置 listener 结构中的 liveliness_changed 回调。 (Set the liveliness_changed callback in
 * the listener structure.)
 *
 * 等同于使用 dds_create_listener() 中传递的参数将 arg 设置为 true 的 reset_on_invoke 调用 @ref
 * dds_lset_liveliness_changed_arg ，并丢弃结果。 (Equivalent to calling @ref
 * dds_lset_liveliness_changed_arg with arg set to the argument passed in dds_create_listener() and
 * reset_on_invoke to true, and throwing away the result.)
 *
 * @param[in,out] listener 要更新的监听器结构 (listener structure to update)
 * @param[in] callback 要设置的回调或空指针 (the callback to set or a null pointer)
 */
DDS_EXPORT void dds_lset_liveliness_changed(dds_listener_t* __restrict listener,
                                            dds_on_liveliness_changed_fn callback);

/**
 * @ingroup listener_setters
 * @component listener_obj
 * @brief 设置 listener 结构中的 requested_deadline_missed 回调。 (Set the requested_deadline_missed
 * callback in the listener structure.)
 *
 * 等同于使用 dds_create_listener() 中传递的参数将 arg 设置为 true 的 reset_on_invoke 调用 @ref
 * dds_lset_requested_deadline_missed_arg ，并丢弃结果。 (Equivalent to calling @ref
 * dds_lset_requested_deadline_missed_arg with arg set to the argument passed in
 * dds_create_listener() and reset_on_invoke to true, and throwing away the result.)
 *
 * @param[in,out] listener 要更新的监听器结构 (listener structure to update)
 * @param[in] callback 要设置的回调或空指针 (the callback to set or a null pointer)
 */
DDS_EXPORT void dds_lset_requested_deadline_missed(dds_listener_t* __restrict listener,
                                                   dds_on_requested_deadline_missed_fn callback);

/**
 * @ingroup listener_setters
 * @component listener_obj
 * @brief 设置监听器结构中的 requested_incompatible_qos 回调。
 * @brief Set the requested_incompatible_qos callback in the listener structure.
 *
 * 等效于以 dds_create_listener() 中传入的参数 arg 和 reset_on_invoke 设为 true 调用 @ref
 * dds_lset_requested_incompatible_qos_arg ，并丢弃结果。 Equivalent to calling @ref
 * dds_lset_requested_incompatible_qos_arg with arg set to the argument passed in
 * dds_create_listener() and reset_on_invoke to true, and throwing away the result.
 *
 * @param[in,out] listener 要更新的监听器结构
 * @param[in,out] listener listener structure to update
 * @param[in] callback 要设置的回调或空指针
 * @param[in] callback the callback to set or a null pointer
 */
DDS_EXPORT void dds_lset_requested_incompatible_qos(dds_listener_t* __restrict listener,
                                                    dds_on_requested_incompatible_qos_fn callback);

/**
 * @ingroup listener_setters
 * @component listener_obj
 * @brief 设置监听器结构中的 publication_matched 回调。
 * @brief Set the publication_matched callback in the listener structure.
 *
 * 等效于以 dds_create_listener() 中传入的参数 arg 和 reset_on_invoke 设为 true 调用 @ref
 * dds_lset_publication_matched_arg ，并丢弃结果。 Equivalent to calling @ref
 * dds_lset_publication_matched_arg with arg set to the argument passed in dds_create_listener() and
 * reset_on_invoke to true, and throwing away the result.
 *
 * @param[in,out] listener 要更新的监听器结构
 * @param[in,out] listener listener structure to update
 * @param[in] callback 要设置的回调或空指针
 * @param[in] callback the callback to set or a null pointer
 */
DDS_EXPORT void dds_lset_publication_matched(dds_listener_t* __restrict listener,
                                             dds_on_publication_matched_fn callback);

/**
 * @ingroup listener_setters
 * @component listener_obj
 * @brief 设置监听器结构中的 subscription_matched 回调。
 * @brief Set the subscription_matched callback in the listener structure.
 *
 * 等效于以 dds_create_listener() 中传入的参数 arg 和 reset_on_invoke 设为 true 调用 @ref
 * dds_lset_subscription_matched_arg ，并丢弃结果。 Equivalent to calling @ref
 * dds_lset_subscription_matched_arg with arg set to the argument passed in dds_create_listener()
 * and reset_on_invoke to true, and throwing away the result.
 *
 * @param[in,out] listener 要更新的监听器结构
 * @param[in,out] listener listener structure to update
 * @param[in] callback 要设置的回调或空指针
 * @param[in] callback the callback to set or a null pointer
 */
DDS_EXPORT void dds_lset_subscription_matched(dds_listener_t* __restrict listener,
                                              dds_on_subscription_matched_fn callback);

/************************************************************************************************
 *  Getters
 ************************************************************************************************/

/**
 * @defgroup listener_getters (Getters)
 * @ingroup listener
 */

/**
 * @ingroup listener_getters
 * @component listener_obj
 * @brief 从监听器结构中获取 data_available 回调。
 * @brief Get the data_available callback from the listener structure.
 *
 * @param[in] listener 要从中检索回调的监听器结构的指针
 * @param[in] listener The pointer to the listener structure, where the callback will be retrieved
 * from
 * @param[out] callback 回调函数；可能是空指针
 * @param[out] callback Callback function; may be a null pointer
 * @param[out] arg 回调参数指针；可能是空指针
 * @param[out] arg Callback argument pointer; may be a null pointer
 * @param[out] reset_on_invoke 监听器调用是否重置状态；可能是空指针
 * @param[out] reset_on_invoke Whether the status is reset by listener invocation; may be a null
 * pointer
 *
 * @retval DDS_RETCODE_OK if successful
 * @retval DDS_RETCODE_BAD_PARAMETER listener is a null pointer
 */
DDS_EXPORT dds_return_t dds_lget_data_available_arg(const dds_listener_t* __restrict listener,
                                                    dds_on_data_available_fn* callback,
                                                    void** arg,
                                                    bool* reset_on_invoke);

/**
 * @ingroup listener_getters
 * @component listener_obj
 * @brief 从监听器结构中获取 data_on_readers 回调。
 * @brief Get the data_on_readers callback from the listener structure.
 *
 * @param[in] listener 要从中检索回调的监听器结构的指针
 * @param[in] listener The pointer to the listener structure, where the callback will be retrieved
 * from
 * @param[out] callback 回调函数；可能是空指针
 * @param[out] callback Callback function; may be a null pointer
 * @param[out] arg 回调参数指针；可能是空指针
 * @param[out] arg Callback argument pointer; may be a null pointer
 * @param[out] reset_on_invoke 监听器调用是否重置状态；可能是空指针
 * @param[out] reset_on_invoke Whether the status is reset by listener invocation; may be a null
 * pointer
 *
 * @retval DDS_RETCODE_OK if successful
 * @retval DDS_RETCODE_BAD_PARAMETER listener is a null pointer
 */
DDS_EXPORT dds_return_t dds_lget_data_on_readers_arg(const dds_listener_t* __restrict listener,
                                                     dds_on_data_on_readers_fn* callback,
                                                     void** arg,
                                                     bool* reset_on_invoke);

/**
 * @ingroup listener_getters
 * @component listener_obj
 * @brief 从监听器结构中获取 inconsistent_topic 回调。
 * @brief Get the inconsistent_topic callback from the listener structure.
 *
 * @param[in] listener 要从中检索回调的监听器结构的指针
 * @param[in] listener The pointer to the listener structure, where the callback will be retrieved
 * from
 * @param[out] callback 回调函数；可能是空指针
 * @param[out] callback Callback function; may be a null pointer
 * @param[out] arg 回调参数指针；可能是空指针
 * @param[out] arg Callback argument pointer; may be a null pointer
 * @param[out] reset_on_invoke 监听器调用是否重置状态；可能是空指针
 * @param[out] reset_on_invoke Whether the status is reset by listener invocation; may be a null
 * pointer
 *
 * @retval DDS_RETCODE_OK if successful
 * @retval DDS_RETCODE_BAD_PARAMETER listener is a null pointer
 */
DDS_EXPORT dds_return_t dds_lget_inconsistent_topic_arg(const dds_listener_t* __restrict listener,
                                                        dds_on_inconsistent_topic_fn* callback,
                                                        void** arg,
                                                        bool* reset_on_invoke);

/**
 * @ingroup listener_getters
 * @component listener_obj
 * @brief 从监听器结构中获取 liveliness_changed 回调。
 * @brief Get the liveliness_changed callback from the listener structure.
 *
 * @param[in] listener 要从中检索回调的监听器结构的指针
 * @param[in] listener The pointer to the listener structure, where the callback will be retrieved
 * from
 * @param[out] callback 回调函数；可能是空指针
 * @param[out] callback Callback function; may be a null pointer
 * @param[out] arg 回调参数指针；可能是空指针
 * @param[out] arg Callback argument pointer; may be a null pointer
 * @param[out] reset_on_invoke 监听器调用是否重置状态；可能是空指针
 * @param[out] reset_on_invoke Whether the status is reset by listener invocation; may be a null
 * pointer
 *
 * @retval DDS_RETCODE_OK if successful
 * @retval DDS_RETCODE_BAD_PARAMETER listener is a null pointer
 */
DDS_EXPORT dds_return_t dds_lget_liveliness_changed_arg(const dds_listener_t* __restrict listener,
                                                        dds_on_liveliness_changed_fn* callback,
                                                        void** arg,
                                                        bool* reset_on_invoke);

/**
 * @ingroup listener_getters
 * @component listener_obj
 * @brief 从监听器结构中获取 liveliness_lost 回调。 (Get the liveliness_lost callback from the
 * listener structure.)
 *
 * @param[in] listener 指向要从中检索回调的监听器结构的指针 (The pointer to the listener structure,
 * where the callback will be retrieved from)
 * @param[out] callback 回调函数；可能是空指针 (Callback function; may be a null pointer)
 * @param[out] arg 回调参数指针；可能是空指针 (Callback argument pointer; may be a null pointer)
 * @param[out] reset_on_invoke 是否通过监听器调用重置状态；可能是空指针 (Whether the status is reset
 * by listener invocation; may be a null pointer)
 *
 * @retval DDS_RETCODE_OK if successful
 * @retval DDS_RETCODE_BAD_PARAMETER listener is a null pointer
 */
DDS_EXPORT dds_return_t dds_lget_liveliness_lost_arg(const dds_listener_t* __restrict listener,
                                                     dds_on_liveliness_lost_fn* callback,
                                                     void** arg,
                                                     bool* reset_on_invoke);

/**
 * @ingroup listener_getters
 * @component listener_obj
 * @brief 从监听器结构中获取 offered_deadline_missed 回调。 (Get the offered_deadline_missed
 * callback from the listener structure.)
 *
 * @param[in] listener 指向要从中检索回调的监听器结构的指针 (The pointer to the listener structure,
 * where the callback will be retrieved from)
 * @param[out] callback 回调函数；可能是空指针 (Callback function; may be a null pointer)
 * @param[out] arg 回调参数指针；可能是空指针 (Callback argument pointer; may be a null pointer)
 * @param[out] reset_on_invoke 是否通过监听器调用重置状态；可能是空指针 (Whether the status is reset
 * by listener invocation; may be a null pointer)
 *
 * @retval DDS_RETCODE_OK if successful
 * @retval DDS_RETCODE_BAD_PARAMETER listener is a null pointer
 */
DDS_EXPORT dds_return_t
dds_lget_offered_deadline_missed_arg(const dds_listener_t* __restrict listener,
                                     dds_on_offered_deadline_missed_fn* callback,
                                     void** arg,
                                     bool* reset_on_invoke);

/**
 * @ingroup listener_getters
 * @component listener_obj
 * @brief 从监听器结构中获取 offered_incompatible_qos 回调。 (Get the offered_incompatible_qos
 * callback from the listener structure.)
 *
 * @param[in] listener 指向要从中检索回调的监听器结构的指针 (The pointer to the listener structure,
 * where the callback will be retrieved from)
 * @param[out] callback 回调函数；可能是空指针 (Callback function; may be a null pointer)
 * @param[out] arg 回调参数指针；可能是空指针 (Callback argument pointer; may be a null pointer)
 * @param[out] reset_on_invoke 是否通过监听器调用重置状态；可能是空指针 (Whether the status is reset
 * by listener invocation; may be a null pointer)
 *
 * @retval DDS_RETCODE_OK if successful
 * @retval DDS_RETCODE_BAD_PARAMETER listener is a null pointer
 */
DDS_EXPORT dds_return_t
dds_lget_offered_incompatible_qos_arg(const dds_listener_t* __restrict listener,
                                      dds_on_offered_incompatible_qos_fn* callback,
                                      void** arg,
                                      bool* reset_on_invoke);

/**
 * @ingroup listener_getters
 * @component listener_obj
 * @brief 从监听器结构中获取 publication_matched 回调。 (Get the publication_matched callback from
 * the listener structure.)
 *
 * @param[in] listener 指向要从中检索回调的监听器结构的指针 (The pointer to the listener structure,
 * where the callback will be retrieved from)
 * @param[out] callback 回调函数；可能是空指针 (Callback function; may be a null pointer)
 * @param[out] arg 回调参数指针；可能是空指针 (Callback argument pointer; may be a null pointer)
 * @param[out] reset_on_invoke 是否通过监听器调用重置状态；可能是空指针 (Whether the status is reset
 * by listener invocation; may be a null pointer)
 *
 * @retval DDS_RETCODE_OK if successful
 * @retval DDS_RETCODE_BAD_PARAMETER listener is a null pointer
 */
DDS_EXPORT dds_return_t dds_lget_publication_matched_arg(const dds_listener_t* __restrict listener,
                                                         dds_on_publication_matched_fn* callback,
                                                         void** arg,
                                                         bool* reset_on_invoke);

/**
 * @ingroup listener_getters
 * @component listener_obj
 * @brief 从监听器结构中获取 subscription_matched 回调。 (Get the subscription_matched callback from
 * the listener structure.)
 *
 * @param[in] listener 指向要从中检索回调的监听器结构的指针 (The pointer to the listener structure,
 * where the callback will be retrieved from)
 * @param[out] callback 回调函数；可能是空指针 (Callback function; may be a null pointer)
 * @param[out] arg 回调参数指针；可能是空指针 (Callback argument pointer; may be a null pointer)
 * @param[out] reset_on_invoke 是否通过监听器调用重置状态；可能是空指针 (Whether the status is reset
 * by listener invocation; may be a null pointer)
 *
 * @retval DDS_RETCODE_OK if successful
 * @retval DDS_RETCODE_BAD_PARAMETER listener is a null pointer
 */
DDS_EXPORT dds_return_t
dds_lget_requested_deadline_missed_arg(const dds_listener_t* __restrict listener,
                                       dds_on_requested_deadline_missed_fn* callback,
                                       void** arg,
                                       bool* reset_on_invoke);

/**
 * @ingroup listener_getters
 * @component listener_obj
 * @brief 从监听器结构中获取 requested_incompatible_qos 回调。 (Get the requested_incompatible_qos
 * callback from the listener structure.)
 *
 * @param[in] listener 指向要从中检索回调的监听器结构的指针 (The pointer to the listener structure,
 * where the callback will be retrieved from)
 * @param[out] callback 回调函数；可能是空指针 (Callback function; may be a null pointer)
 * @param[out] arg 回调参数指针；可能是空指针 (Callback argument pointer; may be a null pointer)
 * @param[out] reset_on_invoke 是否通过监听器调用重置状态；可能是空指针 (Whether the status is reset
 * by listener invocation; may be a null pointer)
 *
 * @retval DDS_RETCODE_OK if successful
 * @retval DDS_RETCODE_BAD_PARAMETER listener is a null pointer
 */
DDS_EXPORT dds_return_t
dds_lget_requested_incompatible_qos_arg(const dds_listener_t* __restrict listener,
                                        dds_on_requested_incompatible_qos_fn* callback,
                                        void** arg,
                                        bool* reset_on_invoke);

/**
 * @ingroup listener_getters
 * @component listener_obj
 * @brief 从监听器结构中获取 sample_lost 回调。 (Get the sample_lost callback from the listener
 * structure.)
 *
 * @param[in] listener 需要检索回调的监听器结构指针 (The pointer to the listener structure, where
 * the callback will be retrieved from)
 * @param[out] callback 回调函数；可以是空指针 (Callback function; may be a null pointer)
 * @param[out] arg 回调参数指针；可以是空指针 (Callback argument pointer; may be a null pointer)
 * @param[out] reset_on_invoke 监听器调用是否重置状态；可以是空指针 (Whether the status is reset by
 * listener invocation; may be a null pointer)
 *
 * @retval DDS_RETCODE_OK if successful
 * @retval DDS_RETCODE_BAD_PARAMETER listener is a null pointer
 */
DDS_EXPORT dds_return_t dds_lget_sample_lost_arg(const dds_listener_t* __restrict listener,
                                                 dds_on_sample_lost_fn* callback,
                                                 void** arg,
                                                 bool* reset_on_invoke);

/**
 * @ingroup listener_getters
 * @component listener_obj
 * @brief 从监听器结构中获取 sample_rejected 回调。 (Get the sample_rejected callback from the
 * listener structure.)
 *
 * @param[in] listener 需要检索回调的监听器结构指针 (The pointer to the listener structure, where
 * the callback will be retrieved from)
 * @param[out] callback 回调函数；可以是空指针 (Callback function; may be a null pointer)
 * @param[out] arg 回调参数指针；可以是空指针 (Callback argument pointer; may be a null pointer)
 * @param[out] reset_on_invoke 监听器调用是否重置状态；可以是空指针 (Whether the status is reset by
 * listener invocation; may be a null pointer)
 *
 * @retval DDS_RETCODE_OK if successful
 * @retval DDS_RETCODE_BAD_PARAMETER listener is a null pointer
 */
DDS_EXPORT dds_return_t dds_lget_sample_rejected_arg(const dds_listener_t* __restrict listener,
                                                     dds_on_sample_rejected_fn* callback,
                                                     void** arg,
                                                     bool* reset_on_invoke);

/**
 * @ingroup listener_getters
 * @component listener_obj
 * @brief 从监听器结构中获取 subscription_matched 回调。 (Get the subscription_matched callback from
 * the listener structure.)
 *
 * @param[in] listener 需要检索回调的监听器结构指针 (The pointer to the listener structure, where
 * the callback will be retrieved from)
 * @param[out] callback 回调函数；可以是空指针 (Callback function; may be a null pointer)
 * @param[out] arg 回调参数指针；可以是空指针 (Callback argument pointer; may be a null pointer)
 * @param[out] reset_on_invoke 监听器调用是否重置状态；可以是空指针 (Whether the status is reset by
 * listener invocation; may be a null pointer)
 *
 * @retval DDS_RETCODE_OK if successful
 * @retval DDS_RETCODE_BAD_PARAMETER listener is a null pointer
 */
DDS_EXPORT dds_return_t dds_lget_subscription_matched_arg(const dds_listener_t* __restrict listener,
                                                          dds_on_subscription_matched_fn* callback,
                                                          void** arg,
                                                          bool* reset_on_invoke);

/**
 * @ingroup listener_getters
 * @component listener_obj
 * @brief 从监听器结构中获取 inconsistent_topic 回调。 (Get the inconsistent_topic callback from the
 * listener structure.)
 *
 * 等同于将 arg 和 reset_on_invoke 设置为空指针并丢弃结果的 dds_lget_inconsistent_topic_arg 调用。
 * (Equivalent to calling @ref dds_lget_inconsistent_topic_arg with arg and reset_on_invoke set to a
 * null pointer and throwing away the result.)
 *
 * @param[in] listener 需要检索回调的监听器结构指针 (The pointer to the listener structure, where
 * the callback will be retrieved from)
 * @param[out] callback 回调函数；可以是空指针 (Callback function; may be a null pointer)
 */
DDS_EXPORT void dds_lget_inconsistent_topic(const dds_listener_t* __restrict listener,
                                            dds_on_inconsistent_topic_fn* callback);

/**
 * @ingroup listener_getters
 * @component listener_obj
 * @brief 从监听器结构中获取 liveliness_lost 回调。 (Get the liveliness_lost callback from the
 * listener structure.)
 *
 * 等同于将 arg 和 reset_on_invoke 设置为空指针并丢弃结果的 dds_lget_liveliness_lost_arg 调用。
 * (Equivalent to calling @ref dds_lget_liveliness_lost_arg with arg and reset_on_invoke set to a
 * null pointer and throwing away the result.)
 *
 * @param[in] listener 需要检索回调的监听器结构指针 (The pointer to the listener structure, where
 * the callback will be retrieved from)
 * @param[out] callback 回调函数；可以是空指针 (Callback function; may be a null pointer)
 */
DDS_EXPORT void dds_lget_liveliness_lost(const dds_listener_t* __restrict listener,
                                         dds_on_liveliness_lost_fn* callback);

/**
 * @ingroup listener_getters
 * @component listener_obj
 * @brief 从监听器结构中获取 offered_deadline_missed 回调。 (Get the offered_deadline_missed
 * callback from the listener structure.)
 *
 * 等同于将 arg 和 reset_on_invoke 设置为空指针并丢弃结果的 dds_lget_offered_deadline_missed_arg
 * 调用。 (Equivalent to calling @ref dds_lget_offered_deadline_missed_arg with arg and
 * reset_on_invoke set to a null pointer and throwing away the result.)
 *
 * @param[in] listener 需要检索回调的监听器结构指针 (The pointer to the listener structure, where
 * the callback will be retrieved from)
 * @param[out] callback 回调函数；可以是空指针 (Callback function; may be a null pointer)
 */
DDS_EXPORT void dds_lget_offered_deadline_missed(const dds_listener_t* __restrict listener,
                                                 dds_on_offered_deadline_missed_fn* callback);

/**
 * @ingroup listener_getters
 * @component listener_obj
 * @brief 从监听器结构中获取 offered_incompatible_qos 回调。 (Get the offered_incompatible_qos
 * callback from the listener structure.)
 *
 * 等同于将 arg 和 reset_on_invoke 设置为空指针并丢弃结果的 dds_lget_offered_incompatible_qos_arg
 * 调用。 (Equivalent to calling @ref dds_lget_offered_incompatible_qos_arg with arg and
 * reset_on_invoke set to a null pointer and throwing away the result.)
 *
 * @param[in] listener 需要检索回调的监听器结构指针 (The pointer to the listener structure, where
 * the callback will be retrieved from)
 * @param[out] callback 回调函数；可以是空指针 (Callback function; may be a null pointer)
 */
DDS_EXPORT void dds_lget_offered_incompatible_qos(const dds_listener_t* __restrict listener,
                                                  dds_on_offered_incompatible_qos_fn* callback);

/**
 * @ingroup listener_getters
 * @component listener_obj
 * @brief 从监听器结构中获取 data_on_readers 回调。 (Get the data_on_readers callback from the
 * listener structure.)
 *
 * 等同于将 arg 和 reset_on_invoke 设置为空指针并丢弃结果的 dds_lget_data_on_readers_arg 调用。
 * (Equivalent to calling @ref dds_lget_data_on_readers_arg with arg and reset_on_invoke set to a
 * null pointer and throwing away the result.)
 *
 * @param[in] listener 需要检索回调的监听器结构指针 (The pointer to the listener structure, where
 * the callback will be retrieved from)
 * @param[out] callback 回调函数；可以是空指针 (Callback function; may be a null pointer)
 */
DDS_EXPORT void dds_lget_data_on_readers(const dds_listener_t* __restrict listener,
                                         dds_on_data_on_readers_fn* callback);

/**
 * @ingroup listener_getters
 * @component listener_obj
 * @brief Get the sample_lost callback from the listener structure.
 *
 * Equivalent to calling @ref dds_lget_sample_lost_arg with arg and reset_on_invoke set to a null
 * pointer and throwing away the result.
 *
 * @param[in] listener The pointer to the listener structure, where the callback will be retrieved
 * from
 * @param[out] callback Callback function; may be a null pointer
 */
// 从监听器结构中获取 sample_lost 回调。
// 等同于用空指针调用 dds_lget_sample_lost_arg，并将结果丢弃。
DDS_EXPORT void dds_lget_sample_lost(const dds_listener_t* __restrict listener,
                                     dds_on_sample_lost_fn* callback);

/**
 * @ingroup listener_getters
 * @component listener_obj
 * @brief Get the data_available callback from the listener structure.
 *
 * Equivalent to calling @ref dds_lget_data_available_arg with arg and reset_on_invoke set to a null
 * pointer and throwing away the result.
 *
 * @param[in] listener The pointer to the listener structure, where the callback will be retrieved
 * from
 * @param[out] callback Callback function; may be a null pointer
 */
// 从监听器结构中获取 data_available 回调。
// 等同于用空指针调用 dds_lget_data_available_arg，并将结果丢弃。
DDS_EXPORT void dds_lget_data_available(const dds_listener_t* __restrict listener,
                                        dds_on_data_available_fn* callback);

/**
 * @ingroup listener_getters
 * @component listener_obj
 * @brief Get the sample_rejected callback from the listener structure.
 *
 * Equivalent to calling @ref dds_lget_sample_rejected_arg with arg and reset_on_invoke set to a
 * null pointer and throwing away the result.
 *
 * @param[in] listener The pointer to the listener structure, where the callback will be retrieved
 * from
 * @param[out] callback Callback function; may be a null pointer
 */
// 从监听器结构中获取 sample_rejected 回调。
// 等同于用空指针调用 dds_lget_sample_rejected_arg，并将结果丢弃。
DDS_EXPORT void dds_lget_sample_rejected(const dds_listener_t* __restrict listener,
                                         dds_on_sample_rejected_fn* callback);

/**
 * @ingroup listener_getters
 * @component listener_obj
 * @brief Get the liveliness_changed callback from the listener structure.
 *
 * Equivalent to calling @ref dds_lget_liveliness_changed_arg with arg and reset_on_invoke set to a
 * null pointer and throwing away the result.
 *
 * @param[in] listener The pointer to the listener structure, where the callback will be retrieved
 * from
 * @param[out] callback Callback function; may be a null pointer
 */
// 从监听器结构中获取 liveliness_changed 回调。
// 等同于用空指针调用 dds_lget_liveliness_changed_arg，并将结果丢弃。
DDS_EXPORT void dds_lget_liveliness_changed(const dds_listener_t* __restrict listener,
                                            dds_on_liveliness_changed_fn* callback);

/**
 * @ingroup listener_getters
 * @component listener_obj
 * @brief Get the requested_deadline_missed callback from the listener structure.
 *
 * Equivalent to calling @ref dds_lget_requested_deadline_missed_arg with arg and reset_on_invoke
 * set to a null pointer and throwing away the result.
 *
 * @param[in] listener The pointer to the listener structure, where the callback will be retrieved
 * from
 * @param[out] callback Callback function; may be a null pointer
 */
// 从监听器结构中获取 requested_deadline_missed 回调。
// 等同于用空指针调用 dds_lget_requested_deadline_missed_arg，并将结果丢弃。
DDS_EXPORT void dds_lget_requested_deadline_missed(const dds_listener_t* __restrict listener,
                                                   dds_on_requested_deadline_missed_fn* callback);

/**
 * @ingroup listener_getters
 * @component listener_obj
 * @brief Get the requested_incompatible_qos callback from the listener structure.
 *
 * Equivalent to calling @ref dds_lget_requested_incompatible_qos_arg with arg and reset_on_invoke
 * set to a null pointer and throwing away the result.
 *
 * @param[in] listener The pointer to the listener structure, where the callback will be retrieved
 * from
 * @param[out] callback Callback function; may be a null pointer
 */
// 从监听器结构中获取 requested_incompatible_qos 回调。
// 等同于用空指针调用 dds_lget_requested_incompatible_qos_arg，并将结果丢弃。
DDS_EXPORT void dds_lget_requested_incompatible_qos(const dds_listener_t* __restrict listener,
                                                    dds_on_requested_incompatible_qos_fn* callback);

/**
 * @ingroup listener_getters
 * @component listener_obj
 * @brief Get the publication_matched callback from the listener structure.
 *
 * Equivalent to calling @ref dds_lget_publication_matched_arg with arg and reset_on_invoke set to a
 * null pointer and throwing away the result.
 *
 * @param[in] listener The pointer to the listener structure, where the callback will be retrieved
 * from
 * @param[out] callback Callback function; may be a null pointer
 */
// 从监听器结构中获取 publication_matched 回调。
// 等同于用空指针调用 dds_lget_publication_matched_arg，并将结果丢弃。
DDS_EXPORT void dds_lget_publication_matched(const dds_listener_t* __restrict listener,
                                             dds_on_publication_matched_fn* callback);

/**
 * @ingroup listener_getters
 * @component listener_obj
 * @brief Get the subscription_matched callback from the listener structure.
 *
 * Equivalent to calling @ref dds_lget_subscription_matched_arg with arg and reset_on_invoke set to
 * a null pointer and throwing away the result.
 *
 * @param[in] listener The pointer to the listener structure, where the callback will be retrieved
 * from
 * @param[out] callback Callback function; may be a null pointer
 */
// 从监听器结构中获取 subscription_matched 回调。
// 等同于用空指针调用 dds_lget_subscription_matched_arg，并将结果丢弃。
DDS_EXPORT void dds_lget_subscription_matched(const dds_listener_t* __restrict listener,
                                              dds_on_subscription_matched_fn* callback);

#if defined(__cplusplus)
}
#endif

#endif /*_DDS_PUBLIC_LISTENER_H_*/
