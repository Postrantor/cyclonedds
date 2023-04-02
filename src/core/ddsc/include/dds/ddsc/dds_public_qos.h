// commit openai

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
 * @defgroup qos (DDS C QoS API)
 * @ingroup dds
 *
 * 此处定义了 Eclipse Cyclone DDS C 语言绑定中的 QoS 和策略的公共 API。
 * This defines the public API of QoS and Policies in the
 * Eclipse Cyclone DDS C language binding.
 */
#ifndef DDS_QOS_H
#define DDS_QOS_H

// 导入 "dds/ddsc/dds_public_qosdefs.h" 头文件，包含 QoS 定义和相关数据结构。
// Include "dds/ddsc/dds_public_qosdefs.h" header file, which contains QoS definitions and related
// data structures.
#include "dds/ddsc/dds_public_qosdefs.h"

// 导入 "dds/export.h" 头文件，包含导出宏定义，用于控制符号可见性。
// Include "dds/export.h" header file, which contains export macro definitions for controlling
// symbol visibility.
#include "dds/export.h"

/**
 * @anchor DDS_HAS_PROPERTY_LIST_QOS
 * @ingroup qos
 * @brief 是否支持此版本中的 "property list" QoS 设置。如果支持，
 * 则特殊处理 "dds.sec." 属性，防止在不支持 DDS 安全性的实现中意外创建非安全参与者。
 * Whether or not the "property list" QoS setting is supported in this version. If it is,
 * the "dds.sec." properties are treated specially, preventing the accidental creation of
 * a non-secure participant by an implementation built without support for DDS Security.
 */
#define DDS_HAS_PROPERTY_LIST_QOS 1

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @ingroup qos
 * @component qos_obj
 * @brief 为默认的QoS策略分配内存并进行初始化 (Allocate memory and initialize default QoS-policies)
 *
 * @returns - 指向已初始化的dds_qos_t结构的指针，如果不成功则返回NULL (Pointer to the initialized
 * dds_qos_t structure, NULL if unsuccessful)
 */
DDS_EXPORT
dds_qos_t* dds_create_qos(void);

/**
 * @ingroup qos
 * @component qos_obj
 * @brief 删除分配给QoS策略结构的内存 (Delete memory allocated to QoS-policies structure)
 *
 * @param[in] qos - 指向dds_qos_t结构的指针 (Pointer to dds_qos_t structure)
 */
DDS_EXPORT void dds_delete_qos(dds_qos_t* __restrict qos);

/**
 * @ingroup qos
 * @component qos_obj
 * @brief 将QoS策略结构重置为默认值 (Reset a QoS-policies structure to default values)
 *
 * @param[in,out] qos - 指向dds_qos_t结构的指针 (Pointer to the dds_qos_t structure)
 */
DDS_EXPORT void dds_reset_qos(dds_qos_t* __restrict qos);

/**
 * @ingroup qos
 * @component qos_obj
 * @brief 将所有QoS策略从一个结构复制到另一个结构 (Copy all QoS-policies from one structure to
 * another)
 *
 * @param[in,out] dst - 指向目标dds_qos_t结构的指针 (Pointer to the destination dds_qos_t structure)
 * @param[in] src - 指向源dds_qos_t结构的指针 (Pointer to the source dds_qos_t structure)
 *
 * @returns - 表示成功或失败的返回代码 (Return-code indicating success or failure)
 */
DDS_EXPORT dds_return_t dds_copy_qos(dds_qos_t* __restrict dst, const dds_qos_t* __restrict src);

/**
 * @ingroup qos
 * @component qos_obj
 * @brief 将所有QoS策略从一个结构复制到另一个结构，除非已经设置 (Copy all QoS-policies from one
 * structure to another, unless already set)
 *
 * 从src复制策略到dst，除非src已经将策略设置为非默认值 (Policies are copied from src to dst, unless
 * src already has the policy set to a non-default value)
 *
 * @param[in,out] dst - 指向目标qos结构的指针 (Pointer to the destination qos structure)
 * @param[in] src - 指向源qos结构的指针 (Pointer to the source qos structure)
 */
DDS_EXPORT void dds_merge_qos(dds_qos_t* __restrict dst, const dds_qos_t* __restrict src);

/**
 * @ingroup qos
 * @component qos_obj
 * @brief 将所有QoS策略从一个结构复制到另一个结构，除非已经设置 (Copy all QoS-policies from one
 * structure to another, unless already set)
 *
 * 从src复制策略到dst，除非src已经将策略设置为非默认值 (Policies are copied from src to dst, unless
 * src already has the policy set to a non-default value)
 *
 * @param[in,out] a - 指向目标qos结构的指针 (Pointer to the destination qos structure)
 * @param[in] b - 指向源qos结构的指针 (Pointer to the source qos structure)
 *
 * @returns 复制是否成功 (whether the copy was successful)
 */
DDS_EXPORT bool dds_qos_equal(const dds_qos_t* __restrict a, const dds_qos_t* __restrict b);

/**
 * @defgroup qos_setters (Qos Setters)
 * @ingroup qos
 * @component qos_obj
 */

/**
 * @ingroup qos_setters
 * @component qos_obj
 * @brief 设置 qos 结构的 userdata。
 * @brief Set the userdata of a qos structure.
 *
 * @param[in,out] qos - 存储 userdata 的 dds_qos_t 结构的指针
 * @param[in,out] qos - Pointer to a dds_qos_t structure that will store the userdata
 * @param[in] value - 指向 userdata 的指针
 * @param[in] value - Pointer to the userdata
 * @param[in] sz - 存储在 value 中的 userdata 的大小
 * @param[in] sz - Size of userdata stored in value
 */
DDS_EXPORT void dds_qset_userdata(dds_qos_t* __restrict qos,
                                  const void* __restrict value,
                                  size_t sz);

/**
 * @ingroup qos_setters
 * @component qos_obj
 * @brief 设置 qos 结构的 topicdata。
 * @brief Set the topicdata of a qos structure.
 *
 * @param[in,out] qos - 存储 topicdata 的 dds_qos_t 结构的指针
 * @param[in,out] qos - Pointer to a dds_qos_t structure that will store the topicdata
 * @param[in] value - 指向 topicdata 的指针
 * @param[in] value - Pointer to the topicdata
 * @param[in] sz - 存储在 value 中的 topicdata 的大小
 * @param[in] sz - Size of the topicdata stored in value
 */
DDS_EXPORT void dds_qset_topicdata(dds_qos_t* __restrict qos,
                                   const void* __restrict value,
                                   size_t sz);

/**
 * @ingroup qos_setters
 * @component qos_obj
 * @brief 设置 qos 结构的 groupdata。
 * @brief Set the groupdata of a qos structure.
 *
 * @param[in,out] qos - 存储 groupdata 的 dds_qos_t 结构的指针
 * @param[in,out] qos - Pointer to a dds_qos_t structure that will store the groupdata
 * @param[in] value - 指向 group data 的指针
 * @param[in] value - Pointer to the group data
 * @param[in] sz - 存储在 value 中的 groupdata 的大小
 * @param[in] sz - Size of groupdata stored in value
 */
DDS_EXPORT void dds_qset_groupdata(dds_qos_t* __restrict qos,
                                   const void* __restrict value,
                                   size_t sz);

/**
 * @ingroup qos_setters
 * @component qos_obj
 * @brief 设置 qos 结构的 durability 策略。
 * @brief Set the durability policy of a qos structure.
 *
 * @param[in,out] qos - 存储策略的 dds_qos_t 结构的指针
 * @param[in,out] qos - Pointer to a dds_qos_t structure that will store the policy
 * @param[in] kind - Durability 类型值
 * @param[in] kind - Durability kind value
 */
DDS_EXPORT void dds_qset_durability(dds_qos_t* __restrict qos, dds_durability_kind_t kind);

/**
 * @ingroup qos_setters
 * @component qos_obj
 * @brief 设置 qos 结构的 history 策略。
 * @brief Set the history policy of a qos structure.
 *
 * 注意，depth 仅对 keep last 有关。如果要为 keep all 设置有限的历史，请使用
 * dds_qset_resource_limits()。 Note that depth is only relevant for keep last. If you want limited
 * history for keep all, use dds_qset_resource_limits().
 *
 * @param[in,out] qos - 存储策略的 dds_qos_t 结构的指针
 * @param[in,out] qos - Pointer to a dds_qos_t structure that will store the policy
 * @param[in] kind - History 类型值
 * @param[in] kind - History kind value
 * @param[in] depth - 历史深度值
 * @param[in] depth - History depth value
 */
DDS_EXPORT void dds_qset_history(dds_qos_t* __restrict qos, dds_history_kind_t kind, int32_t depth);

/**
 * @ingroup qos_setters
 * @component qos_obj
 * @brief 设置 qos 结构的资源限制策略。
 * @brief Set the resource limits policy of a qos structure.
 *
 * @param[in,out] qos - 存储策略的 dds_qos_t 结构的指针
 * @param[in,out] qos - Pointer to a dds_qos_t structure that will store the policy
 * @param[in] max_samples - 样本资源限制值
 * @param[in] max_samples - Number of samples resource-limit value
 * @param[in] max_instances - 实例资源限制值
 * @param[in] max_instances - Number of instances resource-limit value
 * @param[in] max_samples_per_instance - 每个实例的样本资源限制值
 * @param[in] max_samples_per_instance - Number of samples per instance resource-limit value
 */
DDS_EXPORT void dds_qset_resource_limits(dds_qos_t* __restrict qos,
                                         int32_t max_samples,
                                         int32_t max_instances,
                                         int32_t max_samples_per_instance);

/**
 * @ingroup qos_setters
 * @component qos_obj
 * @brief 设置 qos 结构的 presentation 策略。
 * @brief Set the presentation policy of a qos structure.
 *
 * @param[in,out] qos - 存储策略的 dds_qos_t 结构的指针
 * @param[in,out] qos - Pointer to a dds_qos_t structure that will store the policy
 * @param[in] access_scope - 访问范围类型
 * @param[in] access_scope - Access-scope kind
 * @param[in] coherent_access - 一致访问启用值
 * @param[in] coherent_access - Coherent access enable value
 * @param[in] ordered_access - 有序访问启用值
 * @param[in] ordered_access - Ordered access enable value
 */
DDS_EXPORT void dds_qset_presentation(dds_qos_t* __restrict qos,
                                      dds_presentation_access_scope_kind_t access_scope,
                                      bool coherent_access,
                                      bool ordered_access);

/**
 * @ingroup qos_setters
 * @component qos_obj
 * @brief 设置 qos 结构的生命周期策略。 (Set the lifespan policy of a qos structure.)
 *
 * @param[in,out] qos - 将存储策略的 dds_qos_t 结构的指针。 (Pointer to a dds_qos_t structure that
 * will store the policy)
 * @param[in] lifespan - 生命周期持续时间（相对于样本源时间戳的过期时间）。(Lifespan duration
 * (expiration time relative to source timestamp of a sample))
 */
DDS_EXPORT void dds_qset_lifespan(dds_qos_t* __restrict qos, dds_duration_t lifespan);

/**
 * @ingroup qos_setters
 * @component qos_obj
 * @brief 设置 qos 结构的 deadline 策略。 (Set the deadline policy of a qos structure.)
 *
 * @param[in,out] qos - 将存储策略的 dds_qos_t 结构的指针。 (Pointer to a dds_qos_t structure that
 * will store the policy)
 * @param[in] deadline - Deadline 持续时间。 (Deadline duration)
 */
DDS_EXPORT void dds_qset_deadline(dds_qos_t* __restrict qos, dds_duration_t deadline);

/**
 * @ingroup qos_setters
 * @component qos_obj
 * @brief 设置 qos 结构的延迟预算策略。 (Set the latency-budget policy of a qos structure.)
 *
 * @param[in,out] qos - 将存储策略的 dds_qos_t 结构的指针。 (Pointer to a dds_qos_t structure that
 * will store the policy)
 * @param[in] duration - 延迟预算持续时间。 (Latency budget duration)
 */
DDS_EXPORT void dds_qset_latency_budget(dds_qos_t* __restrict qos, dds_duration_t duration);

/**
 * @ingroup qos_setters
 * @component qos_obj
 * @brief 设置 qos 结构的所有权策略。 (Set the ownership policy of a qos structure.)
 *
 * @param[in,out] qos - 将存储策略的 dds_qos_t 结构的指针。 (Pointer to a dds_qos_t structure that
 * will store the policy)
 * @param[in] kind - 所有权类型。 (Ownership kind)
 */
DDS_EXPORT void dds_qset_ownership(dds_qos_t* __restrict qos, dds_ownership_kind_t kind);

/**
 * @ingroup qos_setters
 * @component qos_obj
 * @brief 设置 qos 结构的所有权强度策略。 (Set the ownership strength policy of a qos structure.)
 *
 * @param[in,out] qos - 将存储策略的 dds_qos_t 结构的指针。 (Pointer to a dds_qos_t structure that
 * will store the policy)
 * @param[in] value - 所有权强度值。 (Ownership strength value)
 */
DDS_EXPORT void dds_qset_ownership_strength(dds_qos_t* __restrict qos, int32_t value);

/**
 * @ingroup qos_setters
 * @component qos_obj
 * @brief 设置 qos 结构的活跃度策略。 (Set the liveliness policy of a qos structure.)
 *
 * @param[in,out] qos - 将存储策略的 dds_qos_t 结构的指针。 (Pointer to a dds_qos_t structure that
 * will store the policy)
 * @param[in] kind - 活跃度类型。 (Liveliness kind)
 * @param[in] lease_duration - 租约持续时间。 (Lease duration)
 */
DDS_EXPORT void dds_qset_liveliness(dds_qos_t* __restrict qos,
                                    dds_liveliness_kind_t kind,
                                    dds_duration_t lease_duration);

/**
 * @ingroup qos_setters
 * @component qos_obj
 * @brief 设置 qos 结构的基于时间的过滤策略。 (Set the time-based filter policy of a qos structure.)
 *
 * @param[in,out] qos - 将存储策略的 dds_qos_t 结构的指针。 (Pointer to a dds_qos_t structure that
 * will store the policy)
 * @param[in] minimum_separation - 实例样本传递之间的最短持续时间。 (Minimum duration between sample
 * delivery for an instance)
 */
DDS_EXPORT void dds_qset_time_based_filter(dds_qos_t* __restrict qos,
                                           dds_duration_t minimum_separation);

/**
 * @ingroup qos_setters
 * @component qos_obj
 * @brief 设置 qos 结构的分区策略。 (Set the partition policy of a qos structure.)
 *
 * @param[in,out] qos - 将存储策略的 dds_qos_t 结构的指针。 (Pointer to a dds_qos_t structure that
 * will store the policy)
 * @param[in] n - 存储在 ps 中的分区数。 (Number of partitions stored in ps)
 * @param[in] ps - 存储分区名称的字符串指针。 (Pointer to string(s) storing partition name(s))
 */
DDS_EXPORT void dds_qset_partition(dds_qos_t* __restrict qos,
                                   uint32_t n,
                                   const char** __restrict ps);

/**
 * @ingroup qos_setters
 * @component qos_obj
 * @brief 设置 qos 结构的分区策略为单个名称的便捷函数。名称可以是空指针。 (Convenience function to
 * set the partition policy of a qos structure to a single name. Name may be a null pointer.)
 *
 * @param[in,out] qos - 将存储策略的 dds_qos_t 结构的指针。 (Pointer to a dds_qos_t structure that
 * will store the policy)
 * @param[in] name - 指向名称的指针。 (Pointer to the name)
 */
DDS_EXPORT void dds_qset_partition1(dds_qos_t* __restrict qos, const char* __restrict name);

/**
 * @ingroup qos_setters
 * @component qos_obj
 * @brief 设置 qos 结构的可靠性策略 (Set the reliability policy of a qos structure)
 *
 * @param[in,out] qos - 指向将存储策略的 dds_qos_t 结构的指针 (Pointer to a dds_qos_t structure that
 * will store the policy)
 * @param[in] kind - 可靠性类型 (Reliability kind)
 * @param[in] max_blocking_time - 当 kind
 * 为可靠时应用的最大阻塞持续时间。这是写入器在其历史记录已满时将阻塞多长时间。 (Max blocking
 * duration applied when kind is reliable. This is how long the writer will block when its history
 * is full.)
 */
DDS_EXPORT void dds_qset_reliability(dds_qos_t* __restrict qos,
                                     dds_reliability_kind_t kind,
                                     dds_duration_t max_blocking_time);

/**
 * @ingroup qos_setters
 * @component qos_obj
 * @brief 设置 qos 结构的传输优先级策略 (Set the transport-priority policy of a qos structure)
 *
 * @param[in,out] qos - 指向将存储策略的 dds_qos_t 结构的指针 (Pointer to a dds_qos_t structure that
 * will store the policy)
 * @param[in] value - 优先级值 (Priority value)
 */
DDS_EXPORT void dds_qset_transport_priority(dds_qos_t* __restrict qos, int32_t value);

/**
 * @ingroup qos_setters
 * @component qos_obj
 * @brief 设置 qos 结构的目标顺序策略 (Set the destination-order policy of a qos structure)
 *
 * @param[in,out] qos - 指向将存储策略的 dds_qos_t 结构的指针 (Pointer to a dds_qos_t structure that
 * will store the policy)
 * @param[in] kind - 目标顺序类型 (Destination-order kind)
 */
DDS_EXPORT void dds_qset_destination_order(dds_qos_t* __restrict qos,
                                           dds_destination_order_kind_t kind);

/**
 * @ingroup qos_setters
 * @component qos_obj
 * @brief 设置 qos 结构的写入器数据生命周期策略 (Set the writer data-lifecycle policy of a qos
 * structure)
 *
 * @param[in,out] qos - 指向将存储策略的 dds_qos_t 结构的指针 (Pointer to a dds_qos_t structure that
 * will store the policy)
 * @param[in] autodispose - 自动处理未注册的实例 (Automatic disposal of unregistered instances)
 */
DDS_EXPORT void dds_qset_writer_data_lifecycle(dds_qos_t* __restrict qos, bool autodispose);

/**
 * @ingroup qos_setters
 * @component qos_obj
 * @brief 设置 qos 结构的读取器数据生命周期策略 (Set the reader data-lifecycle policy of a qos
 * structure)
 *
 * @param[in,out] qos - 指向将存储策略的 dds_qos_t 结构的指针 (Pointer to a dds_qos_t structure that
 * will store the policy)
 * @param[in] autopurge_nowriter_samples_delay - 从无写入器状态的实例中清除样本的延迟 (Delay for
 * purging of samples from instances in a no-writers state)
 * @param[in] autopurge_disposed_samples_delay - 从处理过的实例中清除样本的延迟 (Delay for purging
 * of samples from disposed instances)
 */
DDS_EXPORT void dds_qset_reader_data_lifecycle(dds_qos_t* __restrict qos,
                                               dds_duration_t autopurge_nowriter_samples_delay,
                                               dds_duration_t autopurge_disposed_samples_delay);

/**
 * @ingroup qos_setters
 * @component qos_obj
 * @brief 设置 qos 结构的持久性服务策略 (Set the durability-service policy of a qos structure)
 *
 * @param[in,out] qos - 指向将存储策略的 dds_qos_t 结构的指针 (Pointer to a dds_qos_t structure that
 * will store the policy)
 * @param[in] service_cleanup_delay - 从持久性服务中清除被遗弃实例的延迟 (Delay for purging of
 * abandoned instances from the durability service)
 * @param[in] history_kind - 持久性服务应用的历史策略类型 (History policy kind applied by the
 * durability service)
 * @param[in] history_depth - 持久性服务应用的历史策略深度 (History policy depth applied by the
 * durability service)
 * @param[in] max_samples - 持久性服务应用的样本资源限制策略数量 (Number of samples resource-limit
 * policy applied by the durability service)
 * @param[in] max_instances - 持久性服务应用的实例资源限制策略数量 (Number of instances
 * resource-limit policy applied by the durability service)
 * @param[in] max_samples_per_instance - 持久性服务应用的每个实例资源限制策略样本数量 (Number of
 * samples per instance resource-limit policy applied by the durability service)
 */
DDS_EXPORT void dds_qset_durability_service(dds_qos_t* __restrict qos,
                                            dds_duration_t service_cleanup_delay,
                                            dds_history_kind_t history_kind,
                                            int32_t history_depth,
                                            int32_t max_samples,
                                            int32_t max_instances,
                                            int32_t max_samples_per_instance);

/**
 * @ingroup qos_setters
 * @component qos_obj
 * @brief 设置 qos 结构的忽略本地策略 (Set the ignore-local policy of a qos structure)
 *
 * @param[in,out] qos - 指向将存储策略的 dds_qos_t 结构的指针 (Pointer to a dds_qos_t structure that
 * will store the policy)
 * @param[in] ignore - 如果同一参与者拥有的读取器和写入器应被忽略，则为 true (True if readers and
 * writers owned by the same participant should be ignored)
 */
DDS_EXPORT void dds_qset_ignorelocal(dds_qos_t* __restrict qos, dds_ignorelocal_kind_t ignore);

/**
 * @ingroup qos_setters
 * @component qos_obj
 * @brief 在 qos 结构中存储具有提供的名称和字符串值的属性。 (Stores a property with the provided
 * name and string value in a qos structure.)
 *
 * 如果 qos
 * 结构中已存在具有提供的名称的属性，则使用提供的字符串值覆盖此条目的值。如果存在多个具有提供的名称的属性，则仅更新这些属性中第一个的值。
 * (In the case a property with the provided name already exists in the qos structure,
 * the value for this entry is overwritten with the provided string value. If more than
 * one property with the provided name exists, only the value of the first of these
 * properties is updated.)
 *
 * @param[in,out] qos - 指向将存储属性的 dds_qos_t 结构的指针 (Pointer to a dds_qos_t structure that
 * will store the property)
 * @param[in] name - 属性名称的指针 (Pointer to name of the property)
 * @param[in] value - 将存储的（以空结尾）字符串的指针 (Pointer to a (null-terminated) string that
 * will be stored)
 */
DDS_EXPORT void dds_qset_prop(dds_qos_t* __restrict qos, const char* name, const char* value);

/**
 * @ingroup qos_setters
 * @component qos_obj
 * @brief 从 qos 结构中删除具有提供的名称的属性。 (Removes the property with the provided name from
 * a qos structure.)
 *
 * 如果存在具有此名称的多个属性，则仅删除第一个属性。
 * (In case more than one property exists with this name, only the first property
 * is removed.)
 *
 * @param[in,out] qos - 包含属性的 dds_qos_t 结构的指针 (Pointer to a dds_qos_t structure that
 * contains the property)
 * @param[in] name - 属性名称的指针 (Pointer to name of the property)
 */
DDS_EXPORT void dds_qunset_prop(dds_qos_t* __restrict qos, const char* name);

/**
 * @ingroup qos_setters
 * @component qos_obj
 * @brief 将提供的二进制数据存储为 qos 结构中的属性 (Stores the provided binary data as a property
 * in a qos structure)
 *
 * 如果 qos
 * 结构中已存在具有提供的名称的属性，则使用提供的数据覆盖此条目的值。如果存在多个具有提供的名称的属性，则仅更新这些属性中第一个的值。
 * (In the case a property with the provided name already exists in the qos structure,
 * the value for this entry is overwritten with the provided data. If more than one
 * property with the provided name exists, only the value of the first of these
 * properties is updated.)
 *
 * @param[in,out] qos - 指向将存储属性的 dds_qos_t 结构的指针 (Pointer to a dds_qos_t structure that
 * will store the property)
 * @param[in] name - 属性名称的指针 (Pointer to name of the property)
 * @param[in] value - 将存储在属性中的数据的指针 (Pointer to data to be stored in the property)
 * @param[in] sz - 数据的大小 (Size of the data)
 */
DDS_EXPORT void dds_qset_bprop(dds_qos_t* __restrict qos,
                               const char* name,
                               const void* value,
                               const size_t sz);

/**
 * @ingroup qos_setters
 * @component qos_obj
 * @brief 从 qos 结构中删除具有提供的名称的二进制属性 (Removes the binary property with the provided
 * name from a qos structure)
 *
 * 如果具有此名称的多个二进制属性存在，则仅删除第一个二进制属性。
 * (In case more than one binary property exists with this name, only the first binary
 * property is removed.)
 *
 * @param[in,out] qos - 包含二进制属性的 dds_qos_t 结构的指针 (Pointer to a dds_qos_t structure that
 * contains the binary property)
 * @param[in] name - 属性名称的指针 (Pointer to name of the property)
 */
DDS_EXPORT void dds_qunset_bprop(dds_qos_t* __restrict qos, const char* name);

/**
 * @ingroup qos_setters
 * @component qos_obj
 * @brief 设置 qos 结构的类型一致性强制策略 (Set the type consistency enforcement policy of a qos
 * structure)
 *
 * @param[in,out] qos - 将存储策略的 dds_qos_t 结构的指针 (Pointer to a dds_qos_t structure that
 * will store the policy)
 * @param[in] kind - 类型一致性策略类型 (Type consistency policy kind)
 * @param[in] ignore_sequence_bounds - 在类型可分配性检查中忽略序列边界 (Ignore sequence bounds in
 * type assignability checking)
 * @param[in] ignore_string_bounds - 在类型可分配性检查中忽略字符串边界 (Ignore string bounds in
 * type assignability checking)
 * @param[in] ignore_member_names - 在类型可分配性检查中忽略成员名称 (Ignore member names in type
 * assignability checking)
 * @param[in] prevent_type_widening - 在类型可分配性检查中防止类型扩展 (Prevent type widening in
 * type assignability checking)
 * @param[in] force_type_validation - 在可分配性检查中强制类型验证 (Force type validation in
 * assignability checking)
 */
DDS_EXPORT void dds_qset_type_consistency(dds_qos_t* __restrict qos,
                                          dds_type_consistency_kind_t kind,
                                          bool ignore_sequence_bounds,
                                          bool ignore_string_bounds,
                                          bool ignore_member_names,
                                          bool prevent_type_widening,
                                          bool force_type_validation);

/**
 * @ingroup qos_setters
 * @component qos_obj
 * @brief 设置 qos 结构的数据表示 (Set the data representation of a qos structure)
 *
 * @param[in,out] qos    - 将存储策略的 dds_qos_t 结构的指针 (Pointer to a dds_qos_t structure that
 * will store the policy)
 * @param[in]     n      - 数据表示值的数量 (Number of data representation values)
 * @param[in]     values - 数据表示值 (Data representation values)
 */
DDS_EXPORT void dds_qset_data_representation(dds_qos_t* __restrict qos,
                                             uint32_t n,
                                             const dds_data_representation_id_t* values);

/**
 * @ingroup qos_setters
 * @component qos_obj
 * @brief 设置实体名称 (Set the entity name)
 *
 * 使用此 QoS
 * 初始化参与者、发布者、订阅者、读取器或写入器时，它将采用此处设置的名称。此名称在发现过程中可见，可用于使工具中的网络有意义。
 * (When using this QoS to initialize a participant, publisher, subscriber, reader or writer
 * it will take the name set here. This name is visible over discovery and can be used
 * to make sense of network in tooling.)
 *
 * @param[in,out] qos - 将存储实体名称的 dds_qos_t 结构的指针 (Pointer to a dds_qos_t structure that
 * will store the entity name)
 * @param[in] name - 要设置的实体名称的指针 (Pointer to the entity name to set)
 */
DDS_EXPORT void dds_qset_entity_name(dds_qos_t* __restrict qos, const char* name);

/**
 * @defgroup qos_getters (QoS 获取器)
 * @ingroup qos
 * @component qos_obj
 */

/**
 * @ingroup qos_getters
 * @component qos_obj
 * @brief 从 qos 结构中获取用户数据 (Get the userdata from a qos structure)
 *
 * @param[in] qos - 存储策略的 dds_qos_t 结构的指针 (Pointer to a dds_qos_t structure storing the
 * policy)
 * @param[in,out] value - 将存储用户数据的指针。如果 sz = 0，则为空指针，否则为指向分配了 sz+1
 * 字节的缓冲区的指针，其中最后一个字节始终为 0 (Pointer that will store the userdata. If sz = 0,
 * then a null pointer, else it is a pointer to an allocated buffer of sz+1 bytes where the last
 * byte is always 0)
 * @param[in,out] sz - 将存储用户数据大小的指针 (Pointer that will store the size of userdata)
 *
 * @returns - 如果任何参数无效或 qos 不在 qos 对象中，则返回 false (false iff any of the arguments
 * is invalid or the qos is not present in the qos object)
 */
DDS_EXPORT bool dds_qget_userdata(const dds_qos_t* __restrict qos, void** value, size_t* sz);

/**
 * @ingroup qos_getters
 * @component qos_obj
 * @brief 从 qos 结构中获取 topicdata (Get the topicdata from a qos structure)
 *
 * @param[in] qos - 指向存储策略的 dds_qos_t 结构的指针 (Pointer to a dds_qos_t structure storing
 * the policy)
 * @param[in,out] value - 将存储 topicdata 的指针。如果 sz = 0，则为空指针，
 * 否则它是一个指向分配了 sz+1 字节缓冲区的指针，其中最后一个字节始终为 0
 * (Pointer that will store the topicdata. If sz = 0, then a null pointer,
 * else it is a pointer to an allocated buffer of sz+1 bytes where the last byte is always 0)
 * @param[in,out] sz - 将存储 topicdata 大小的指针 (Pointer that will store the size of topicdata)
 *
 * @returns 如果任何参数无效或 qos 不在 qos 对象中，则返回 false (false iff any of the arguments is
 * invalid or the qos is not present in the qos object)
 */
DDS_EXPORT bool dds_qget_topicdata(const dds_qos_t* __restrict qos, void** value, size_t* sz);

/**
 * @ingroup qos_getters
 * @component qos_obj
 * @brief 从 qos 结构中获取 groupdata (Get the groupdata from a qos structure)
 *
 * @param[in] qos - 指向存储策略的 dds_qos_t 结构的指针 (Pointer to a dds_qos_t structure storing
 * the policy)
 * @param[in,out] value - 将存储 groupdata 的指针。如果 sz = 0，则为空指针，
 * 否则它是一个指向分配了 sz+1 字节缓冲区的指针，其中最后一个字节始终为 0
 * (Pointer that will store the groupdata. If sz = 0, then a null pointer,
 * else it is a pointer to an allocated buffer of sz+1 bytes where the last byte is always 0)
 * @param[in,out] sz - 将存储 groupdata 大小的指针 (Pointer that will store the size of groupdata)
 *
 * @returns 如果任何参数无效或 qos 不在 qos 对象中，则返回 false (false iff any of the arguments is
 * invalid or the qos is not present in the qos object)
 */
DDS_EXPORT bool dds_qget_groupdata(const dds_qos_t* __restrict qos, void** value, size_t* sz);

/**
 * @ingroup qos_getters
 * @component qos_obj
 * @brief 从 qos 结构中获取 durability 策略 (Get the durability policy from a qos structure)
 *
 * @param[in] qos - 指向存储策略的 dds_qos_t 结构的指针 (Pointer to a dds_qos_t structure storing
 * the policy)
 * @param[in,out] kind - 将存储 durability 类型的指针 (Pointer that will store the durability kind)
 *
 * @returns 如果任何参数无效或 qos 不在 qos 对象中，则返回 false (false iff any of the arguments is
 * invalid or the qos is not present in the qos object)
 */
DDS_EXPORT bool dds_qget_durability(const dds_qos_t* __restrict qos, dds_durability_kind_t* kind);

/**
 * @ingroup qos_getters
 * @component qos_obj
 * @brief 从 qos 结构中获取 history 策略 (Get the history policy from a qos structure)
 *
 * @param[in] qos - 指向存储策略的 dds_qos_t 结构的指针 (Pointer to a dds_qos_t structure storing
 * the policy)
 * @param[in,out] kind - 将存储 history 类型的指针（可选）(Pointer that will store the history kind
 * (optional))
 * @param[in,out] depth - 将存储 history 深度的指针（可选）(Pointer that will store the history
 * depth (optional))
 *
 * @returns 如果任何参数无效或 qos 不在 qos 对象中，则返回 false (false iff any of the arguments is
 * invalid or the qos is not present in the qos object)
 */
DDS_EXPORT bool dds_qget_history(const dds_qos_t* __restrict qos,
                                 dds_history_kind_t* kind,
                                 int32_t* depth);

/**
 * @ingroup qos_getters
 * @component qos_obj
 * @brief 从 qos 结构中获取 resource-limits 策略 (Get the resource-limits policy from a qos
 * structure)
 *
 * @param[in] qos - 指向存储策略的 dds_qos_t 结构的指针 (Pointer to a dds_qos_t structure storing
 * the policy)
 * @param[in,out] max_samples - 将存储样本资源限制数量的指针（可选）
 * (Pointer that will store the number of samples resource-limit (optional))
 * @param[in,out] max_instances - 将存储实例资源限制数量的指针（可选）
 * (Pointer that will store the number of instances resource-limit (optional))
 * @param[in,out] max_samples_per_instance - 将存储每个实例的样本资源限制数量的指针（可选）
 * (Pointer that will store the number of samples per instance resource-limit (optional))
 *
 * @returns 如果任何参数无效或 qos 不在 qos 对象中，则返回 false (false iff any of the arguments is
 * invalid or the qos is not present in the qos object)
 */
DDS_EXPORT bool dds_qget_resource_limits(const dds_qos_t* __restrict qos,
                                         int32_t* max_samples,
                                         int32_t* max_instances,
                                         int32_t* max_samples_per_instance);

/**
 * @ingroup qos_getters
 * @component qos_obj
 * @brief 从 qos 结构中获取 presentation 策略 (Get the presentation policy from a qos structure)
 *
 * @param[in] qos - 指向存储策略的 dds_qos_t 结构的指针 (Pointer to a dds_qos_t structure storing
 * the policy)
 * @param[in,out] access_scope - 将存储访问范围类型的指针（可选）(Pointer that will store access
 * scope kind (optional))
 * @param[in,out] coherent_access - 将存储连贯访问启用值的指针（可选）(Pointer that will store
 * coherent access enable value (optional))
 * @param[in,out] ordered_access - 将存储有序访问启用值的指针（可选）(Pointer that will store
 * orderede access enable value (optional))
 *
 * @returns 如果任何参数无效或 qos 不在 qos 对象中，则返回 false (false iff any of the arguments is
 * invalid or the qos is not present in the qos object)
 */
DDS_EXPORT bool dds_qget_presentation(const dds_qos_t* __restrict qos,
                                      dds_presentation_access_scope_kind_t* access_scope,
                                      bool* coherent_access,
                                      bool* ordered_access);

/**
 * @ingroup qos_getters
 * @component qos_obj
 * @brief 从 qos 结构中获取生命周期策略 (Get the lifespan policy from a qos structure)
 *
 * @param[in] qos - 指向存储策略的 dds_qos_t 结构的指针 (Pointer to a dds_qos_t structure storing
 * the policy)
 * @param[in,out] lifespan - 将存储生命周期持续时间的指针 (Pointer that will store lifespan
 * duration)
 *
 * @returns - 如果任何参数无效或 qos 不在 qos 对象中，则返回 false (false iff any of the arguments
 * is invalid or the qos is not present in the qos object)
 */
DDS_EXPORT bool dds_qget_lifespan(const dds_qos_t* __restrict qos, dds_duration_t* lifespan);

/**
 * @ingroup qos_getters
 * @component qos_obj
 * @brief 从 qos 结构中获取 deadline 策略 (Get the deadline policy from a qos structure)
 *
 * @param[in] qos - 指向存储策略的 dds_qos_t 结构的指针 (Pointer to a dds_qos_t structure storing
 * the policy)
 * @param[in,out] deadline - 将存储 deadline 持续时间的指针 (Pointer that will store deadline
 * duration)
 *
 * @returns - 如果任何参数无效或 qos 不在 qos 对象中，则返回 false (false iff any of the arguments
 * is invalid or the qos is not present in the qos object)
 */
DDS_EXPORT bool dds_qget_deadline(const dds_qos_t* __restrict qos, dds_duration_t* deadline);

/**
 * @ingroup qos_getters
 * @component qos_obj
 * @brief 从 qos 结构中获取延迟预算策略 (Get the latency-budget policy from a qos structure)
 *
 * @param[in] qos - 指向存储策略的 dds_qos_t 结构的指针 (Pointer to a dds_qos_t structure storing
 * the policy)
 * @param[in,out] duration - 将存储延迟预算持续时间的指针 (Pointer that will store latency-budget
 * duration)
 *
 * @returns - 如果任何参数无效或 qos 不在 qos 对象中，则返回 false (false iff any of the arguments
 * is invalid or the qos is not present in the qos object)
 */
DDS_EXPORT bool dds_qget_latency_budget(const dds_qos_t* __restrict qos, dds_duration_t* duration);

/**
 * @ingroup qos_getters
 * @component qos_obj
 * @brief 从 qos 结构中获取所有权策略 (Get the ownership policy from a qos structure)
 *
 * @param[in] qos - 指向存储策略的 dds_qos_t 结构的指针 (Pointer to a dds_qos_t structure storing
 * the policy)
 * @param[in,out] kind - 将存储所有权类型的指针 (Pointer that will store the ownership kind)
 *
 * @returns - 如果任何参数无效或 qos 不在 qos 对象中，则返回 false (false iff any of the arguments
 * is invalid or the qos is not present in the qos object)
 */
DDS_EXPORT bool dds_qget_ownership(const dds_qos_t* __restrict qos, dds_ownership_kind_t* kind);

/**
 * @ingroup qos_getters
 * @component qos_obj
 * @brief 获取所有权强度 qos 策略 (Get the ownership strength qos policy)
 *
 * @param[in] qos - 指向存储策略的 dds_qos_t 结构的指针 (Pointer to a dds_qos_t structure storing
 * the policy)
 * @param[in,out] value - 将存储所有权强度值的指针 (Pointer that will store the ownership strength
 * value)
 *
 * @returns - 如果任何参数无效或 qos 不在 qos 对象中，则返回 false (false iff any of the arguments
 * is invalid or the qos is not present in the qos object)
 */
DDS_EXPORT bool dds_qget_ownership_strength(const dds_qos_t* __restrict qos, int32_t* value);

/**
 * @ingroup qos_getters
 * @component qos_obj
 * @brief 获取活跃度 qos 策略 (Get the liveliness qos policy)
 *
 * @param[in] qos - 指向存储策略的 dds_qos_t 结构的指针 (Pointer to a dds_qos_t structure storing
 * the policy)
 * @param[in,out] kind - 将存储活跃度类型的指针（可选）(Pointer that will store the liveliness kind
 * (optional))
 * @param[in,out] lease_duration - 将存储活跃度租约持续时间的指针（可选）(Pointer that will store
 * the liveliness lease duration (optional))
 *
 * @returns - 如果任何参数无效或 qos 不在 qos 对象中，则返回 false (false iff any of the arguments
 * is invalid or the qos is not present in the qos object)
 */
DDS_EXPORT bool dds_qget_liveliness(const dds_qos_t* __restrict qos,
                                    dds_liveliness_kind_t* kind,
                                    dds_duration_t* lease_duration);

/**
 * @ingroup qos_getters
 * @component qos_obj
 * @brief 获取基于时间的过滤器 qos 策略 (Get the time-based filter qos policy)
 *
 * @param[in] qos - 指向存储策略的 dds_qos_t 结构的指针 (Pointer to a dds_qos_t structure storing
 * the policy)
 * @param[in,out] minimum_separation - 将存储最小分离持续时间的指针（可选）(Pointer that will store
 * the minimum separation duration (optional))
 *
 * @returns - 如果任何参数无效或 qos 不在 qos 对象中，则返回 false (false iff any of the arguments
 * is invalid or the qos is not present in the qos object)
 */
DDS_EXPORT bool dds_qget_time_based_filter(const dds_qos_t* __restrict qos,
                                           dds_duration_t* minimum_separation);

/**
 * @ingroup qos_getters
 * @component qos_obj
 * @brief 获取分区 QoS 策略 (Get the partition qos policy)
 *
 * @param[in] qos - 指向存储策略的 dds_qos_t 结构的指针 (Pointer to a dds_qos_t structure storing
 * the policy)
 * @param[in,out] n - 将存储分区数量的指针（可选）(Pointer that will store the number of partitions
 * (optional))
 * @param[in,out] ps - 将存储包含分区名称的字符串的指针（可选）(Pointer that will store the
 * string(s) containing partition name(s) (optional))
 *
 * @returns - 如果任何参数无效或 qos 不在 qos 对象中，则返回 false (false iff any of the arguments
 * is invalid or the qos is not present in the qos object)
 */
DDS_EXPORT bool dds_qget_partition(const dds_qos_t* __restrict qos, uint32_t* n, char*** ps);

/**
 * @ingroup qos_getters
 * @component qos_obj
 * @brief 获取可靠性 QoS 策略 (Get the reliability qos policy)
 *
 * @param[in] qos - 指向存储策略的 dds_qos_t 结构的指针 (Pointer to a dds_qos_t structure storing
 * the policy)
 * @param[in,out] kind - 将存储可靠性类型的指针（可选）(Pointer that will store the reliability kind
 * (optional))
 * @param[in,out] max_blocking_time - 将存储可靠可靠性的最大阻塞时间的指针（可选）(Pointer that will
 * store the max blocking time for reliable reliability (optional))
 *
 * @returns - 如果任何参数无效或 qos 不在 qos 对象中，则返回 false (false iff any of the arguments
 * is invalid or the qos is not present in the qos object)
 */
DDS_EXPORT bool dds_qget_reliability(const dds_qos_t* __restrict qos,
                                     dds_reliability_kind_t* kind,
                                     dds_duration_t* max_blocking_time);

/**
 * @ingroup qos_getters
 * @component qos_obj
 * @brief 获取传输优先级 QoS 策略 (Get the transport priority qos policy)
 *
 * @param[in] qos - 指向存储策略的 dds_qos_t 结构的指针 (Pointer to a dds_qos_t structure storing
 * the policy)
 * @param[in,out] value - 将存储传输优先级值的指针 (Pointer that will store the transport priority
 * value)
 *
 * @returns - 如果任何参数无效或 qos 不在 qos 对象中，则返回 false (false iff any of the arguments
 * is invalid or the qos is not present in the qos object)
 */
DDS_EXPORT bool dds_qget_transport_priority(const dds_qos_t* __restrict qos, int32_t* value);

/**
 * @ingroup qos_getters
 * @component qos_obj
 * @brief 获取目标顺序 QoS 策略 (Get the destination-order qos policy)
 *
 * @param[in] qos - 指向存储策略的 dds_qos_t 结构的指针 (Pointer to a dds_qos_t structure storing
 * the policy)
 * @param[in,out] kind - 将存储目标顺序类型的指针 (Pointer that will store the destination-order
 * kind)
 *
 * @returns - 如果任何参数无效或 qos 不在 qos 对象中，则返回 false (false iff any of the arguments
 * is invalid or the qos is not present in the qos object)
 */
DDS_EXPORT bool dds_qget_destination_order(const dds_qos_t* __restrict qos,
                                           dds_destination_order_kind_t* kind);

/**
 * @ingroup qos_getters
 * @component qos_obj
 * @brief 获取写入器数据生命周期 QoS 策略 (Get the writer data-lifecycle qos policy)
 *
 * @param[in] qos - 指向存储策略的 dds_qos_t 结构的指针 (Pointer to a dds_qos_t structure storing
 * the policy)
 * @param[in,out] autodispose - 将存储自动处理未注册实例启用值的指针 (Pointer that will store the
 * autodispose unregistered instances enable value)
 *
 * @returns - 如果任何参数无效或 qos 不在 qos 对象中，则返回 false (false iff any of the arguments
 * is invalid or the qos is not present in the qos object)
 */
DDS_EXPORT bool dds_qget_writer_data_lifecycle(const dds_qos_t* __restrict qos, bool* autodispose);

/**
 * @ingroup qos_getters
 * @component qos_obj
 * @brief 获取读取器数据生命周期 QoS 策略 (Get the reader data-lifecycle qos policy)
 *
 * @param[in] qos - 指向存储策略的 dds_qos_t 结构的指针 (Pointer to a dds_qos_t structure storing
 * the policy)
 * @param[in,out] autopurge_nowriter_samples_delay -
 * 将存储从无写入器状态的实例中自动清除样本的延迟的指针（可选）(Pointer that will store the delay
 * for auto-purging samples from instances in a no-writer state (optional))
 * @param[in,out] autopurge_disposed_samples_delay -
 * 将存储自动清除已处理实例的延迟的指针（可选）(Pointer that will store the delay for auto-purging
 * of disposed instances (optional))
 *
 * @returns - 如果任何参数无效或 qos 不在 qos 对象中，则返回 false (false iff any of the arguments
 * is invalid or the qos is not present in the qos object)
 */
DDS_EXPORT bool dds_qget_reader_data_lifecycle(const dds_qos_t* __restrict qos,
                                               dds_duration_t* autopurge_nowriter_samples_delay,
                                               dds_duration_t* autopurge_disposed_samples_delay);

/**
 * @ingroup qos_getters
 * @component qos_obj
 * @brief 获取耐用性服务 QoS 策略值 (Get the durability-service qos policy values)
 *
 * @param[in] qos - 指向存储策略的 dds_qos_t 结构的指针 (Pointer to a dds_qos_t structure storing
 * the policy)
 * @param[in,out]  service_cleanup_delay -
 * 将存储从耐用性服务中清除被遗弃实例的延迟的指针（可选）(Pointer that will store the delay for
 * purging of abandoned instances from the durability service (optional))
 * @param[in,out] history_kind - 将存储由耐用性服务应用的历史策略类型的指针（可选）(Pointer that
 * will store history policy kind applied by the durability service (optional))
 * @param[in,out] history_depth - 将存储由耐用性服务应用的历史策略深度的指针（可选）(Pointer that
 * will store history policy depth applied by the durability service (optional))
 * @param[in,out] max_samples - 将存储由耐用性服务应用的样本资源限制策略数量的指针（可选）(Pointer
 * that will store number of samples resource-limit policy applied by the durability service
 * (optional))
 * @param[in,out] max_instances - 将存储由耐用性服务应用的实例资源限制策略数量的指针（可选）(Pointer
 * that will store number of instances resource-limit policy applied by the durability service
 * (optional))
 * @param[in,out] max_samples_per_instance -
 * 将存储由耐用性服务应用的每个实例资源限制策略样本数量的指针（可选）(Pointer that will store number
 * of samples per instance resource-limit policy applied by the durability service (optional))
 *
 * @returns - 如果任何参数无效或 qos 不在 qos 对象中，则返回 false (false iff any of the arguments
 * is invalid or the qos is not present in the qos object)
 */
DDS_EXPORT bool dds_qget_durability_service(const dds_qos_t* __restrict qos,
                                            dds_duration_t* service_cleanup_delay,
                                            dds_history_kind_t* history_kind,
                                            int32_t* history_depth,
                                            int32_t* max_samples,
                                            int32_t* max_instances,
                                            int32_t* max_samples_per_instance);

/**
 * @ingroup qos_getters
 * @component qos_obj
 * @brief 获取 ignore-local qos 策略
 * @brief Get the ignore-local qos policy
 *
 * @param[in] qos - 指向存储策略的 dds_qos_t 结构的指针
 * @param[in] qos - Pointer to a dds_qos_t structure storing the policy
 * @param[in,out] ignore - 将存储是否忽略同一参与者拥有的读取器/写入器的指针（可选）
 * @param[in,out] ignore - Pointer that will store whether to ignore readers/writers owned by the
 * same participant (optional)
 *
 * @returns - 如果任何参数无效或 qos 对象中不存在 qos，则返回 false
 * @returns - false iff any of the arguments is invalid or the qos is not present in the qos object
 */
DDS_EXPORT bool dds_qget_ignorelocal(const dds_qos_t* __restrict qos,
                                     dds_ignorelocal_kind_t* ignore);

/**
 * @ingroup qos_getters
 * @component qos_obj
 * @brief 从 qos 结构中获取属性名称。
 * @brief Gets the names of the properties from a qos structure.
 *
 * @param[in,out] qos - 包含属性的 dds_qos_t 结构的指针
 * @param[in,out] qos - Pointer to a dds_qos_t structure that contains properties
 * @param[in,out] n - 返回的属性名称数量的指针（可选）
 * @param[in,out] n - Pointer to number of property names that are returned (optional)
 * @param[in,out] names -
 * 将存储包含属性名称的字符串的指针（可选）。此函数将为名称列表和包含名称的字符串分配内存；调用者获得分配内存的所有权
 * @param[in,out] names - Pointer that will store the string(s) containing property name(s)
 * (optional). This function will allocate the memory for the list of names and for the strings
 * containing the names; the caller gets ownership of the allocated memory
 *
 * @returns - 如果任何参数无效或 qos 对象中不存在 qos，则返回 false
 * @returns - false iff any of the arguments is invalid or the qos is not present in the qos object
 */
DDS_EXPORT bool dds_qget_propnames(const dds_qos_t* __restrict qos, uint32_t* n, char*** names);

/**
 * @ingroup qos_getters
 * @component qos_obj
 * @brief 从 qos 结构中获取具有提供名称的属性的值。
 * @brief Get the value of the property with the provided name from a qos structure.
 *
 * 如果存在多个具有此名称的属性，则返回第一个具有此名称的属性的值。
 * In case more than one property exists with this name, the value for the first
 * property with this name will be returned.
 *
 * @param[in,out] qos - 包含属性的 dds_qos_t 结构的指针
 * @param[in,out] qos - Pointer to a dds_qos_t structure that contains the property
 * @param[in] name - 属性名称的指针
 * @param[in] name - Pointer to name of the property
 * @param[in,out] value -
 * 将存储属性值的字符串的指针。存储字符串值的内存将由此函数分配，调用者获得分配内存的所有权
 * @param[in,out] value - Pointer to a string that will store the value of the property. The memory
 * for storing the string value will be allocated by this function and the caller gets ownership of
 * the allocated memory
 *
 * @returns - 如果任何参数无效，qos 对象中不存在 qos 或未找到具有提供名称的属性，则返回 false
 * @returns - false iff any of the arguments is invalid, the qos is not present in the qos object or
 * there was no property found with the provided name
 */
DDS_EXPORT bool dds_qget_prop(const dds_qos_t* __restrict qos, const char* name, char** value);

/**
 * @ingroup qos_getters
 * @component qos_obj
 * @brief 从 qos 结构中获取二进制属性名称。
 * @brief Gets the names of the binary properties from a qos structure.
 *
 * @param[in,out] qos - 包含二进制属性的 dds_qos_t 结构的指针
 * @param[in,out] qos - Pointer to a dds_qos_t structure that contains binary properties
 * @param[in,out] n - 返回的二进制属性名称数量的指针（可选）
 * @param[in,out] n - Pointer to number of binary property names that are returned (optional)
 * @param[in,out] names -
 * 将存储包含二进制属性名称的字符串的指针（可选）。此函数将为名称列表和包含名称的字符串分配内存；调用者获得分配内存的所有权
 * @param[in,out] names - Pointer that will store the string(s) containing binary property name(s)
 * (optional). This function will allocate the memory for the list of names and for the strings
 * containing the names; the caller gets ownership of the allocated memory
 *
 * @returns - 如果任何参数无效或 qos 对象中不存在 qos，则返回 false
 * @returns - false iff any of the arguments is invalid or the qos is not present in the qos object
 */
DDS_EXPORT bool dds_qget_bpropnames(const dds_qos_t* __restrict qos, uint32_t* n, char*** names);

/**
 * @ingroup qos_getters
 * @component qos_obj
 * @brief 从 qos 结构中获取具有提供名称的二进制属性的值。
 * @brief Get the value of the binary property with the provided name from a qos structure.
 *
 * 如果存在多个具有此名称的二进制属性，则返回第一个具有此名称的二进制属性的值。
 * In case more than one binary property exists with this name, the value for the first
 * binary property with this name will be returned.
 *
 * @param[in,out] qos - 包含属性的 dds_qos_t 结构的指针
 * @param[in,out] qos - Pointer to a dds_qos_t structure that contains the property
 * @param[in] name - 二进制属性名称的指针
 * @param[in] name - Pointer to name of the binary property
 * @param[in,out] value - 将存储属性值的缓冲区的指针。如果 sz =
 * 0，则为空指针。存储值的内存将由此函数分配，调用者获得分配内存的所有权
 * @param[in,out] value - Pointer to a buffer that will store the value of the property. If sz = 0
 * then a NULL pointer. The memory for storing the value will be allocated by this function and the
 * caller gets ownership of the allocated memory
 * @param[in,out] sz - 将存储返回缓冲区大小的指针。
 * @param[in,out] sz - Pointer that will store the size of the returned buffer.
 *
 * @returns - 如果任何参数无效，qos 对象中不存在 qos 或未找到具有提供名称的二进制属性，则返回 false
 * @returns - false iff any of the arguments is invalid, the qos is not present in the qos object or
 * there was no binary property found with the provided name
 */
DDS_EXPORT bool dds_qget_bprop(const dds_qos_t* __restrict qos,
                               const char* name,
                               void** value,
                               size_t* sz);

/**
 * @ingroup qos_getters
 * @component qos_obj
 * @brief 获取类型一致性强制 qos 策略值。
 * @brief Get the type consistency enforcement qos policy values.
 *
 * @param[in] qos - 指向存储策略的 dds_qos_t 结构的指针
 * @param[in] qos - Pointer to a dds_qos_t structure storing the policy
 * @param[in,out] kind - 将存储类型一致性强制种类的指针（可选）
 * @param[in,out] kind - Pointer that will store the type consistency enforcement kind (optional)
 * @param[in,out] ignore_sequence_bounds -
 * 将存储在类型可分配性检查中忽略序列边界的布尔值的指针（可选）
 * @param[in,out] ignore_sequence_bounds - Pointer that will store the boolean value for ignoring
 * sequence bounds in type assignability checking (optional)
 * @param[in,out] ignore_string_bounds -
 * 将存储在类型可分配性检查中忽略字符串边界的布尔值的指针（可选）
 * @param[in,out] ignore_string_bounds - Pointer that will store the boolean value for ignoring
 * string bounds in type assignability checking (optional)
 * @param[in,out] ignore_member_names - 将存储在类型可分配性检查中忽略成员名称的布尔值的指针（可选）
 * @param[in,out] ignore_member_names - Pointer that will store the boolean value for ignoring
 * member names in type assignability checking (optional)
 * @param[in,out] prevent_type_widening -
 * 将存储在类型可分配性检查中防止类型扩展的布尔值的指针（可选）
 * @param[in,out] prevent_type_widening - Pointer that will store the boolean value to prevent type
 * widening in type assignability checking (optional)
 * @param[in,out] force_type_validation -
 * 将存储在类型可分配性检查中强制类型验证的布尔值的指针（可选）
 * @param[in,out] force_type_validation - Pointer that will store the boolean value to force type
 * validation in assignability checking (optional)
 *
 * @returns - false iff any of the arguments is invalid or the qos is not present in the qos object
 */
DDS_EXPORT bool dds_qget_type_consistency(const dds_qos_t* __restrict qos,
                                          dds_type_consistency_kind_t* kind,
                                          bool* ignore_sequence_bounds,
                                          bool* ignore_string_bounds,
                                          bool* ignore_member_names,
                                          bool* prevent_type_widening,
                                          bool* force_type_validation);

/**
 * @ingroup qos_getters
 * @component qos_obj
 * @brief 获取数据表示 qos 策略值。
 * @brief Get the data representation qos policy value.
 *
 * 返回提供的 QoS 对象中设置的数据表示值，并将值的数量存储在输出参数 'n' 中。如果提供了 'values'
 * 参数， 此函数将分配一个包含数据表示值的缓冲区，并将 'values'
 * 设置为指向此缓冲区。调用者有责任释放此缓冲区的内存。
 *
 * Returns the data representation values that are set in the provided QoS object
 * and stores the number of values in out parameter 'n'. In case the 'values' parameter
 * is provided, this function will allocate a buffer that contains the data representation
 * values, and set 'values' to point to this buffer. It is the responsibility of the caller
 * to free the memory of this buffer.
 *
 * @param[in] qos - 指向存储策略的 dds_qos_t 结构的指针
 * @param[in] qos - Pointer to a dds_qos_t structure storing the policy
 * @param[in,out] n - 将存储数据表示值数量的指针
 * @param[in,out] n - Pointer that will store the number of data representation values
 * @param[in,out] values - 将存储数据表示值的指针（可选）
 * @param[in,out] values - Pointer that will store the data representation values (optional)
 *
 * @returns - false iff any of the arguments is invalid or the qos is not present in the qos object
 */
DDS_EXPORT bool dds_qget_data_representation(const dds_qos_t* __restrict qos,
                                             uint32_t* n,
                                             dds_data_representation_id_t** values);

/**
 * @ingroup qos_getters
 * @component qos_obj
 * @brief 从 qos 结构中获取实体名称。
 * @brief Get the entity name from a qos structure
 *
 * @param[in] qos - 指向存储实体名称的 dds_qos_t 结构的指针
 * @param[in] qos - Pointer to a dds_qos_t structure storing the entity name
 * @param[in,out] name - 将存储返回的实体名称的字符串指针
 * @param[in,out] name - Pointer to a string that will store the returned entity name
 *
 * @returns - false iff any of the arguments is invalid or the qos is not present in the qos object
 *            或者无法分配存储名称的缓冲区。
 *            or if a buffer to store the name could not be allocated.
 */
DDS_EXPORT bool dds_qget_entity_name(const dds_qos_t* __restrict qos, char** name);

#if defined(__cplusplus)
}
#endif
#endif
