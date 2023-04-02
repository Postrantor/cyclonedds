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

/**
 * @defgroup dcps_status (DDS C Communication Status API)
 * @ingroup dds
 * This defines the public API of the Communication Status in the
 * Eclipse Cyclone DDS C language binding. Listeners are implemented
 * as structs containing callback functions that take listener status types
 * as arguments.
 */
#ifndef DDS_STATUS_H
#define DDS_STATUS_H

#include "dds/export.h"

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @ingroup dcps_status
 * @brief DCPS_Status_OfferedDeadlineMissed
 * DOC_TODO
 *
 * @param total_count 总数（Total count of missed deadlines）
 * @param total_count_change 自上次读取后的总数变化（Change in total count since last read）
 * @param last_instance_handle 最后一个未满足期限的实例句柄（Instance handle of the last deadline
 * missed）
 */
typedef struct dds_offered_deadline_missed_status {
  uint32_t total_count; /**< 总数（Total count of missed deadlines） */
  int32_t
      total_count_change; /**< 自上次读取后的总数变化（Change in total count since last read） */
  dds_instance_handle_t last_instance_handle; /**< 最后一个未满足期限的实例句柄（Instance handle of
                                                 the last deadline missed） */
} dds_offered_deadline_missed_status_t;

/**
 * @ingroup dcps_status
 * @brief DCPS_Status_OfferedIncompatibleQoS
 * DOC_TODO
 *
 * @param total_count 总不兼容 QoS 策略数（Total count of incompatible QoS policies）
 * @param total_count_change 自上次读取后的总数变化（Change in total count since last read）
 * @param last_policy_id 最后一个不兼容的 QoS 策略 ID（ID of the last incompatible QoS policy）
 */
typedef struct dds_offered_incompatible_qos_status {
  uint32_t total_count; /**< 总不兼容 QoS 策略数（Total count of incompatible QoS policies） */
  int32_t
      total_count_change; /**< 自上次读取后的总数变化（Change in total count since last read） */
  uint32_t
      last_policy_id; /**< 最后一个不兼容的 QoS 策略 ID（ID of the last incompatible QoS policy） */
} dds_offered_incompatible_qos_status_t;

/**
 * @ingroup dcps_status
 * @brief DCPS_Status_PublicationMatched
 * DOC_TODO
 *
 * @param total_count 总匹配的订阅者数（Total count of matched subscribers）
 * @param total_count_change 自上次读取后的总数变化（Change in total count since last read）
 * @param current_count 当前匹配的订阅者数（Current count of matched subscribers）
 * @param current_count_change 自上次读取后的当前数变化（Change in current count since last read）
 * @param last_subscription_handle 最后一个匹配的订阅者句柄（Handle of the last matched subscriber）
 */
typedef struct dds_publication_matched_status {
  uint32_t total_count; /**< 总匹配的订阅者数（Total count of matched subscribers） */
  int32_t
      total_count_change; /**< 自上次读取后的总数变化（Change in total count since last read） */
  uint32_t current_count; /**< 当前匹配的订阅者数（Current count of matched subscribers） */
  int32_t current_count_change; /**< 自上次读取后的当前数变化（Change in current count since last
                                   read） */
  dds_instance_handle_t last_subscription_handle; /**< 最后一个匹配的订阅者句柄（Handle of the last
                                                     matched subscriber） */
} dds_publication_matched_status_t;

/**
 * @ingroup dcps_status
 * @brief DCPS_Status_LivelinessLost
 * DOC_TODO
 *
 * @param total_count 总失去活跃度的次数（Total count of lost liveliness）
 * @param total_count_change 自上次读取后的总数变化（Change in total count since last read）
 */
typedef struct dds_liveliness_lost_status {
  uint32_t total_count; /**< 总失去活跃度的次数（Total count of lost liveliness） */
  int32_t
      total_count_change; /**< 自上次读取后的总数变化（Change in total count since last read） */
} dds_liveliness_lost_status_t;

/**
 * @ingroup dcps_status
 * @brief DCPS_Status_SubscriptionMatched
 * DOC_TODO
 *
 * @param total_count 总匹配的发布者数（Total count of matched publishers）
 * @param total_count_change 自上次读取后的总数变化（Change in total count since last read）
 * @param current_count 当前匹配的发布者数（Current count of matched publishers）
 * @param current_count_change 自上次读取后的当前数变化（Change in current count since last read）
 * @param last_publication_handle 最后一个匹配的发布者句柄（Handle of the last matched publisher）
 */
typedef struct dds_subscription_matched_status {
  uint32_t total_count; /**< 总匹配的发布者数（Total count of matched publishers） */
  int32_t
      total_count_change; /**< 自上次读取后的总数变化（Change in total count since last read） */
  uint32_t current_count; /**< 当前匹配的发布者数（Current count of matched publishers） */
  int32_t current_count_change; /**< 自上次读取后的当前数变化（Change in current count since last
                                   read） */
  dds_instance_handle_t last_publication_handle; /**< 最后一个匹配的发布者句柄（Handle of the last
                                                    matched publisher） */
} dds_subscription_matched_status_t;

/**
 * @ingroup dcps_status
 * @brief Rejected Status
 * DOC_TODO
 */
// 拒绝状态 (Rejected Status)
typedef enum {
  DDS_NOT_REJECTED,                /**< 不被拒绝 (Not Rejected) */
  DDS_REJECTED_BY_INSTANCES_LIMIT, /**< 被实例限制拒绝 (Rejected by Instances Limit) */
  DDS_REJECTED_BY_SAMPLES_LIMIT,   /**< 被样本限制拒绝 (Rejected by Samples Limit) */
  DDS_REJECTED_BY_SAMPLES_PER_INSTANCE_LIMIT /**< 被每个实例的样本限制拒绝 (Rejected by Samples per
                                                Instance Limit) */
} dds_sample_rejected_status_kind;

/**
 * @ingroup dcps_status
 * @brief DCPS_Status_SampleRejected
 * DOC_TODO
 */
// 样本拒绝状态 (Sample Rejected Status)
typedef struct dds_sample_rejected_status {
  uint32_t total_count;                        /**< 总计数 (Total Count) */
  int32_t total_count_change;                  /**< 总计数变化 (Total Count Change) */
  dds_sample_rejected_status_kind last_reason; /**< 最后一个原因 (Last Reason) */
  dds_instance_handle_t last_instance_handle; /**< 最后一个实例句柄 (Last Instance Handle) */
} dds_sample_rejected_status_t;

/**
 * @ingroup dcps_status
 * @brief DCPS_Status_LivelinessChanged
 * DOC_TODO
 */
// 活跃度改变状态 (Liveliness Changed Status)
typedef struct dds_liveliness_changed_status {
  uint32_t alive_count;           /**< 存活计数 (Alive Count) */
  uint32_t not_alive_count;       /**< 非存活计数 (Not Alive Count) */
  int32_t alive_count_change;     /**< 存活计数变化 (Alive Count Change) */
  int32_t not_alive_count_change; /**< 非存活计数变化 (Not Alive Count Change) */
  dds_instance_handle_t last_publication_handle; /**< 最后一个发布句柄 (Last Publication Handle) */
} dds_liveliness_changed_status_t;

/**
 * @ingroup dcps_status
 * @brief DCPS_Status_RequestedDeadlineMissed
 * DOC_TODO
 */
// 请求的截止日期未满足状态 (Requested Deadline Missed Status)
typedef struct dds_requested_deadline_missed_status {
  uint32_t total_count;                       /**< 总计数 (Total Count) */
  int32_t total_count_change;                 /**< 总计数变化 (Total Count Change) */
  dds_instance_handle_t last_instance_handle; /**< 最后一个实例句柄 (Last Instance Handle) */
} dds_requested_deadline_missed_status_t;

/**
 * @ingroup dcps_status
 * @brief DCPS_Status_RequestedIncompatibleQoS
 * DOC_TODO
 */
// 请求的不兼容 QoS 状态 (Requested Incompatible QoS Status)
typedef struct dds_requested_incompatible_qos_status {
  uint32_t total_count;       /**< 总计数 (Total Count) */
  int32_t total_count_change; /**< 总计数变化 (Total Count Change) */
  uint32_t last_policy_id;    /**< 最后一个策略 ID (Last Policy ID) */
} dds_requested_incompatible_qos_status_t;

/**
 * @ingroup dcps_status
 * @brief DCPS_Status_SampleLost
 * DOC_TODO
 */
// 样本丢失状态 (Sample Lost Status)
typedef struct dds_sample_lost_status {
  uint32_t total_count;       /**< 总计数 (Total Count) */
  int32_t total_count_change; /**< 总计数变化 (Total Count Change) */
} dds_sample_lost_status_t;

/**
 * @ingroup dcps_status
 * @brief DCPS_Status_InconsistentTopic
 * DOC_TODO
 */
// 不一致主题状态结构体 (Inconsistent Topic Status Structure)
typedef struct dds_inconsistent_topic_status {
  uint32_t total_count;       /**< 总计数 (Total count) */
  int32_t total_count_change; /**< 总计数变化 (Total count change) */
} dds_inconsistent_topic_status_t;

/**
 * @defgroup dcps_status_getters (DCPS Status Getters)
 * @ingroup dcps_status
 * get_<status> APIs return the status of an entity and resets the status
 */

/**
 * @ingroup dcps_status_getters
 * @component entity_status
 * @brief Get INCONSISTENT_TOPIC status
 *
 * 获取与 INCONSISTENT_TOPIC 对应的状态值并重置状态。
 * 只有在为实体启用状态时才能获取该值。
 * 允许使用 NULL 值作为状态，当启用状态时，它将重置触发器值。
 *
 * This operation gets the status value corresponding to INCONSISTENT_TOPIC
 * and reset the status. The value can be obtained, only if the status is enabled for an entity.
 * NULL value for status is allowed and it will reset the trigger value when status is enabled.
 *
 * @param[in]  topic  要获取状态的实体 (The entity to get the status)
 * @param[out] status 指向 @ref dds_inconsistent_topic_status_t 的指针以获取状态 (The pointer to
 * @ref dds_inconsistent_topic_status_t to get the status)
 *
 * @returns  0 - 成功 (Success)
 * @returns <0 - 失败 (Failure)
 *
 * @retval DDS_RETCODE_ERROR
 *                  发生内部错误 (An internal error has occurred.)
 * @retval DDS_RETCODE_BAD_PARAMETER
 *                  给定参数之一无效 (One of the given arguments is not valid.)
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *                  在不适当的对象上调用操作 (The operation is invoked on an inappropriate object.)
 * @retval DDS_RETCODE_ALREADY_DELETED
 *                  实体已被删除 (The entity has already been deleted.)
 */
DDS_EXPORT dds_return_t dds_get_inconsistent_topic_status(dds_entity_t topic,
                                                          dds_inconsistent_topic_status_t* status);

/**
 * @ingroup dcps_status_getters
 * @component entity_status
 * @brief Get PUBLICATION_MATCHED status
 *
 * 获取与 PUBLICATION_MATCHED 对应的状态值并重置状态。
 * 只有在为实体启用状态时才能获取该值。
 * 允许使用 NULL 值作为状态，当启用状态时，它将重置触发器值。
 *
 * This operation gets the status value corresponding to PUBLICATION_MATCHED
 * and reset the status. The value can be obtained, only if the status is enabled for an entity.
 * NULL value for status is allowed and it will reset the trigger value when status is enabled.
 *
 * @param[in]  writer  要获取状态的实体 (The entity to get the status)
 * @param[out] status  指向 @ref dds_publication_matched_status_t 的指针以获取状态 (The pointer to
 * @ref dds_publication_matched_status_t to get the status)
 *
 * @returns  0 - 成功 (Success)
 * @returns <0 - 失败 (Failure)
 *
 * @retval DDS_RETCODE_ERROR
 *                  发生内部错误 (An internal error has occurred.)
 * @retval DDS_RETCODE_BAD_PARAMETER
 *                  给定参数之一无效 (One of the given arguments is not valid.)
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *                  在不适当的对象上调用操作 (The operation is invoked on an inappropriate object.)
 * @retval DDS_RETCODE_ALREADY_DELETED
 *                  实体已被删除 (The entity has already been deleted.)
 */
DDS_EXPORT dds_return_t
dds_get_publication_matched_status(dds_entity_t writer, dds_publication_matched_status_t* status);

/**
 * @ingroup dcps_status_getters
 * @component entity_status
 * @brief 获取 LIVELINESS_LOST 状态 (Get LIVELINESS_LOST status)
 *
 * 此操作获取与 LIVELINESS_LOST 对应的状态值，并重置状态。
 * 只有在为实体启用状态时，才能获得该值。
 * 允许使用 NULL 值作为状态，并且在启用状态时将重置触发器值。
 * (This operation gets the status value corresponding to LIVELINESS_LOST
 * and reset the status. The value can be obtained, only if the status is enabled for an entity.
 * NULL value for status is allowed and it will reset the trigger value when status is enabled.)
 *
 * @param[in]  writer  要获取状态的实体 (The entity to get the status)
 * @param[out] status  指向 @ref dds_liveliness_lost_status_t 的指针以获取状态 (The pointer to @ref
 * dds_liveliness_lost_status_t to get the status)
 *
 * @returns  0 - 成功 (Success)
 * @returns <0 - 失败 (Failure)
 *
 * @retval DDS_RETCODE_ERROR
 *                  发生内部错误 (An internal error has occurred.)
 * @retval DDS_RETCODE_BAD_PARAMETER
 *                  给定参数之一无效 (One of the given arguments is not valid.)
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *                  在不合适的对象上调用了操作 (The operation is invoked on an inappropriate
 * object.)
 * @retval DDS_RETCODE_ALREADY_DELETED
 *                  实体已被删除 (The entity has already been deleted.)
 */
DDS_EXPORT dds_return_t dds_get_liveliness_lost_status(dds_entity_t writer,
                                                       dds_liveliness_lost_status_t* status);

/**
 * @ingroup dcps_status_getters
 * @component entity_status
 * @brief 获取 OFFERED_DEADLINE_MISSED 状态 (Get OFFERED_DEADLINE_MISSED status)
 *
 * 此操作获取与 OFFERED_DEADLINE_MISSED 对应的状态值，并重置状态。
 * 只有在为实体启用状态时，才能获得该值。
 * 允许使用 NULL 值作为状态，并且在启用状态时将重置触发器值。
 * (This operation gets the status value corresponding to OFFERED_DEADLINE_MISSED
 * and reset the status. The value can be obtained, only if the status is enabled for an entity.
 * NULL value for status is allowed and it will reset the trigger value when status is enabled.)
 *
 * @param[in]  writer  要获取状态的实体 (The entity to get the status)
 * @param[out] status  指向 @ref dds_offered_deadline_missed_status_t 的指针以获取状态 (The pointer
 * to @ref dds_offered_deadline_missed_status_t to get the status)
 *
 * @returns  0 - 成功 (Success)
 * @returns <0 - 失败 (Failure)
 *
 * @retval DDS_RETCODE_ERROR
 *                  发生内部错误 (An internal error has occurred.)
 * @retval DDS_RETCODE_BAD_PARAMETER
 *                  给定参数之一无效 (One of the given arguments is not valid.)
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *                  在不合适的对象上调用了操作 (The operation is invoked on an inappropriate
 * object.)
 * @retval DDS_RETCODE_ALREADY_DELETED
 *                  实体已被删除 (The entity has already been deleted.)
 */
DDS_EXPORT dds_return_t dds_get_offered_deadline_missed_status(
    dds_entity_t writer, dds_offered_deadline_missed_status_t* status);

/**
 * @ingroup dcps_status_getters
 * @component entity_status
 * @brief 获取 OFFERED_INCOMPATIBLE_QOS 状态 (Get OFFERED_INCOMPATIBLE_QOS status)
 *
 * 此操作获取与 OFFERED_INCOMPATIBLE_QOS 对应的状态值，并重置状态。
 * 只有实体的状态启用时，才能获得该值。
 * 允许使用 NULL 值作为状态，当状态启用时，它将重置触发值。
 *
 * @param[in]  writer  要获取状态的写入器实体 (The writer entity to get the status)
 * @param[out] status  指向 @ref dds_offered_incompatible_qos_status_t 的指针，以获取状态 (The
 * pointer to @ref dds_offered_incompatible_qos_status_t to get the status)
 *
 * @returns  0 - 成功 (Success)
 * @returns <0 - 失败 (Failure)
 *
 * @retval DDS_RETCODE_ERROR
 *                  发生内部错误 (An internal error has occurred.)
 * @retval DDS_RETCODE_BAD_PARAMETER
 *                  给定参数之一无效 (One of the given arguments is not valid.)
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *                  在不合适的对象上调用了操作 (The operation is invoked on an inappropriate
 * object.)
 * @retval DDS_RETCODE_ALREADY_DELETED
 *                  实体已被删除 (The entity has already been deleted.)
 */
DDS_EXPORT dds_return_t dds_get_offered_incompatible_qos_status(
    dds_entity_t writer, dds_offered_incompatible_qos_status_t* status);

/**
 * @ingroup dcps_status_getters
 * @component entity_status
 * @brief 获取 SUBSCRIPTION_MATCHED 状态 (Get SUBSCRIPTION_MATCHED status)
 *
 * 此操作获取与 SUBSCRIPTION_MATCHED 对应的状态值，并重置状态。
 * 只有实体的状态启用时，才能获得该值。
 * 允许使用 NULL 值作为状态，当状态启用时，它将重置触发值。
 *
 * @param[in]  reader  要获取状态的读取器实体 (The reader entity to get the status)
 * @param[out] status  指向 @ref dds_subscription_matched_status_t 的指针，以获取状态 (The pointer
 * to @ref dds_subscription_matched_status_t to get the status)
 *
 * @returns  0 - 成功 (Success)
 * @returns <0 - 失败 (Failure)
 *
 * @retval DDS_RETCODE_ERROR
 *                  发生内部错误 (An internal error has occurred.)
 * @retval DDS_RETCODE_BAD_PARAMETER
 *                  给定参数之一无效 (One of the given arguments is not valid.)
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *                  在不合适的对象上调用了操作 (The operation is invoked on an inappropriate
 * object.)
 * @retval DDS_RETCODE_ALREADY_DELETED
 *                  实体已被删除 (The entity has already been deleted.)
 */
DDS_EXPORT dds_return_t
dds_get_subscription_matched_status(dds_entity_t reader, dds_subscription_matched_status_t* status);

/**
 * @ingroup dcps_status_getters
 * @component entity_status
 * @brief Get LIVELINESS_CHANGED status
 *
 * 这个操作获取与LIVELINESS_CHANGED对应的状态值，并重置状态。只有在实体启用状态时，才能获得该值。
 * 允许使用NULL值作为状态，当启用状态时，它将重置触发值。
 *
 * This operation gets the status value corresponding to LIVELINESS_CHANGED
 * and reset the status. The value can be obtained, only if the status is enabled for an entity.
 * NULL value for status is allowed and it will reset the trigger value when status is enabled.
 *
 * @param[in]  reader  要获取状态的实体 (The entity to get the status)
 * @param[out] status  指向 @ref dds_liveliness_changed_status_t 的指针以获取状态 (The pointer to
 * @ref dds_liveliness_changed_status_t to get the status)
 *
 * @returns  0 - 成功 (Success)
 * @returns <0 - 失败 (Failure)
 *
 * @retval DDS_RETCODE_ERROR
 *                  发生了内部错误。 (An internal error has occurred.)
 * @retval DDS_RETCODE_BAD_PARAMETER
 *                  给定参数之一无效。 (One of the given arguments is not valid.)
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *                  在不适当的对象上调用了操作。 (The operation is invoked on an inappropriate
 * object.)
 * @retval DDS_RETCODE_ALREADY_DELETED
 *                  实体已被删除。 (The entity has already been deleted.)
 */
DDS_EXPORT dds_return_t dds_get_liveliness_changed_status(dds_entity_t reader,
                                                          dds_liveliness_changed_status_t* status);

/**
 * @ingroup dcps_status_getters
 * @component entity_status
 * @brief Get SAMPLE_REJECTED status
 *
 * 这个操作获取与SAMPLE_REJECTED对应的状态值，并重置状态。只有在实体启用状态时，才能获得该值。
 * 允许使用NULL值作为状态，当启用状态时，它将重置触发值。
 *
 * This operation gets the status value corresponding to SAMPLE_REJECTED
 * and reset the status. The value can be obtained, only if the status is enabled for an entity.
 * NULL value for status is allowed and it will reset the trigger value when status is enabled.
 *
 * @param[in]  reader  要获取状态的实体 (The entity to get the status)
 * @param[out] status  指向 @ref dds_sample_rejected_status_t 的指针以获取状态 (The pointer to @ref
 * dds_sample_rejected_status_t to get the status)
 *
 * @returns  0 - 成功 (Success)
 * @returns <0 - 失败 (Failure)
 *
 * @retval DDS_RETCODE_ERROR
 *                  发生了内部错误。 (An internal error has occurred.)
 * @retval DDS_RETCODE_BAD_PARAMETER
 *                  给定参数之一无效。 (One of the given arguments is not valid.)
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *                  在不适当的对象上调用了操作。 (The operation is invoked on an inappropriate
 * object.)
 * @retval DDS_RETCODE_ALREADY_DELETED
 *                  实体已被删除。 (The entity has already been deleted.)
 */
DDS_EXPORT dds_return_t dds_get_sample_rejected_status(dds_entity_t reader,
                                                       dds_sample_rejected_status_t* status);

/**
 * @ingroup dcps_status_getters
 * @component entity_status
 * @brief 获取 SAMPLE_LOST 状态 (Get SAMPLE_LOST status)
 *
 * 此操作获取与 SAMPLE_LOST 对应的状态值，并重置状态。只有在实体启用状态时，才能获得该值。
 * 允许使用 NULL 值作为状态，当启用状态时，它将重置触发值。
 * (This operation gets the status value corresponding to SAMPLE_LOST
 * and reset the status. The value can be obtained, only if the status is enabled for an entity.
 * NULL value for status is allowed and it will reset the trigger value when status is enabled.)
 *
 * @param[in]  reader  要获取状态的实体 (The entity to get the status)
 * @param[out] status  指向 @ref dds_sample_lost_status_t 的指针，以获取状态 (The pointer to @ref
 * dds_sample_lost_status_t to get the status)
 *
 * @returns 表示成功或失败的 dds_return_t (A dds_return_t indicating success or failure)
 *
 * @retval DDS_RETCODE_OK
 *            成功 (Success)
 * @retval DDS_RETCODE_ERROR
 *            发生内部错误 (An internal error has occurred.)
 * @retval DDS_RETCODE_BAD_PARAMETER
 *            给定参数之一无效 (One of the given arguments is not valid.)
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *            在不适当的对象上调用操作 (The operation is invoked on an inappropriate object.)
 * @retval DDS_RETCODE_ALREADY_DELETED
 *            实体已被删除 (The entity has already been deleted.)
 */
DDS_EXPORT dds_return_t dds_get_sample_lost_status(dds_entity_t reader,
                                                   dds_sample_lost_status_t* status);

/**
 * @ingroup dcps_status_getters
 * @component entity_status
 * @brief 获取 REQUESTED_DEADLINE_MISSED 状态 (Get REQUESTED_DEADLINE_MISSED status)
 *
 * 此操作获取与 REQUESTED_DEADLINE_MISSED
 * 对应的状态值，并重置状态。只有在实体启用状态时，才能获得该值。 允许使用 NULL
 * 值作为状态，当启用状态时，它将重置触发值。 (This operation gets the status value corresponding to
 * REQUESTED_DEADLINE_MISSED and reset the status. The value can be obtained, only if the status is
 * enabled for an entity. NULL value for status is allowed and it will reset the trigger value when
 * status is enabled.)
 *
 * @param[in]  reader  要获取状态的实体 (The entity to get the status)
 * @param[out] status  指向 @ref dds_requested_deadline_missed_status_t 的指针，以获取状态 (The
 * pointer to @ref dds_requested_deadline_missed_status_t to get the status)
 *
 * @returns 表示成功或失败的 dds_return_t (A dds_return_t indicating success or failure)
 *
 * @retval DDS_RETCODE_OK
 *            成功 (Success)
 * @retval DDS_RETCODE_ERROR
 *            发生内部错误 (An internal error has occurred.)
 * @retval DDS_RETCODE_BAD_PARAMETER
 *            给定参数之一无效 (One of the given arguments is not valid.)
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *            在不适当的对象上调用操作 (The operation is invoked on an inappropriate object.)
 * @retval DDS_RETCODE_ALREADY_DELETED
 *            实体已被删除 (The entity has already been deleted.)
 */
DDS_EXPORT dds_return_t dds_get_requested_deadline_missed_status(
    dds_entity_t reader, dds_requested_deadline_missed_status_t* status);

/**
 * @ingroup dcps_status_getters
 * @component entity_status
 * @brief 获取 REQUESTED_INCOMPATIBLE_QOS 状态 (Get REQUESTED_INCOMPATIBLE_QOS status)
 *
 * 此操作获取与 REQUESTED_INCOMPATIBLE_QOS 对应的状态值，并重置状态。
 * 只有在为实体启用状态时，才能获得该值。
 * 允许使用 NULL 值作为状态，当启用状态时，它将重置触发值。
 * (This operation gets the status value corresponding to REQUESTED_INCOMPATIBLE_QOS
 * and reset the status. The value can be obtained, only if the status is enabled for an entity.
 * NULL value for status is allowed and it will reset the trigger value when status is enabled.)
 *
 * @param[in]  reader  要获取状态的实体 (The entity to get the status)
 * @param[out] status  指向 @ref dds_requested_incompatible_qos_status_t 的指针，以获取状态 (The
 * pointer to @ref dds_requested_incompatible_qos_status_t to get the status)
 *
 * @returns 表示成功或失败的 dds_return_t (A dds_return_t indicating success or failure)
 *
 * @retval DDS_RETCODE_OK
 *            成功 (Success)
 * @retval DDS_RETCODE_ERROR
 *            发生内部错误 (An internal error has occurred.)
 * @retval DDS_RETCODE_BAD_PARAMETER
 *            给定参数之一无效 (One of the given arguments is not valid.)
 * @retval DDS_RETCODE_ILLEGAL_OPERATION
 *            在不适当的对象上调用了操作 (The operation is invoked on an inappropriate object.)
 * @retval DDS_RETCODE_ALREADY_DELETED
 *            实体已被删除 (The entity has already been deleted.)
 */
DDS_EXPORT dds_return_t dds_get_requested_incompatible_qos_status(
    dds_entity_t reader, dds_requested_incompatible_qos_status_t* status);

#if defined(__cplusplus)
}
#endif
#endif
