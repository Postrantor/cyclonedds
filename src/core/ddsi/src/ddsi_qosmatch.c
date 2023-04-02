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
#include "dds/ddsi/ddsi_qosmatch.h"

#include <assert.h>
#include <string.h>

#include "dds/dds.h"
#include "dds/ddsi/ddsi_domaingv.h"
#include "dds/ddsi/ddsi_entity.h"
#include "dds/ddsi/ddsi_xqos.h"
#include "dds/features.h"
#include "ddsi__misc.h"
#include "ddsi__typelib.h"
#include "ddsi__typelookup.h"

/**
 * @brief 判断分区字符串是否包含通配符 (Check if the partition string contains a wildcard)
 *
 * @param str 分区字符串 (Partition string)
 * @return 如果包含通配符返回非零值，否则返回0 (Non-zero value if it contains a wildcard, 0
 * otherwise)
 */
static int is_wildcard_partition(const char *str) {
  /* 检查字符串中是否有 '*' 或 '?' 通配符 (Check if the string contains '*' or '?' wildcards) */
  return strchr(str, '*') || strchr(str, '?');
}

/**
 * @brief 判断分区名称是否与模式匹配 (Check if the partition name matches the pattern)
 *
 * @param pat 可能包含通配符的模式字符串 (Pattern string that may contain wildcards)
 * @param name 不包含通配符的分区名称 (Partition name without wildcards)
 * @return 如果匹配返回1，否则返回0 (1 if matched, 0 otherwise)
 */
static int partition_patmatch_p(const char *pat, const char *name) {
  /* pat 可能是一个通配符表达式，name 必须不是 (pat may be a wildcard expression, name must not be)
   */

  /* 如果 pat 中没有通配符，则必须等于 name (If there is no wildcard in pat, it must equal name) */
  if (!is_wildcard_partition(pat))
    return (strcmp(pat, name) == 0);
  else if (is_wildcard_partition(name))
    /* (我们知道: pat 中有通配符) => name 中有通配符 => 不匹配 ((we know: wildcard in pat) =>
     * wildcard in name => no match) */
    return 0;
  else
    /* 使用 ddsi_patmatch 函数进行模式匹配 (Use the ddsi_patmatch function for pattern matching) */
    return ddsi_patmatch(pat, name);
}

/**
 * @brief 检查分区是否匹配默认值 (Check if partitions match the default value)
 *
 * @param[in] x 分区的 QoS 设置 (QoS settings of the partition)
 * @return int 如果匹配返回 1，否则返回 0 (Returns 1 if it matches, otherwise returns 0)
 */
static int partitions_match_default(const dds_qos_t *x) {
  // 检查分区是否存在或者为空 (Check if the partition is present or empty)
  if (!(x->present & DDSI_QP_PARTITION) || x->partition.n == 0) return 1;

  // 遍历分区字符串，检查它们是否匹配空字符串 (Iterate through partition strings and check if they
  // match the empty string)
  for (uint32_t i = 0; i < x->partition.n; i++)
    if (partition_patmatch_p(x->partition.strs[i], "")) return 1;

  return 0;
}

/**
 * @brief 检查两个分区的 QoS 是否匹配 (Check if the QoS of two partitions match)
 *
 * @param[in] a 第一个分区的 QoS 设置 (QoS settings of the first partition)
 * @param[in] b 第二个分区的 QoS 设置 (QoS settings of the second partition)
 * @return int 如果匹配返回 1，否则返回 0 (Returns 1 if they match, otherwise returns 0)
 */
static int partitions_match_p(const dds_qos_t *a, const dds_qos_t *b) {
  // 检查第一个分区是否匹配默认值 (Check if the first partition matches the default value)
  if (!(a->present & DDSI_QP_PARTITION) || a->partition.n == 0) return partitions_match_default(b);
  // 检查第二个分区是否匹配默认值 (Check if the second partition matches the default value)
  else if (!(b->present & DDSI_QP_PARTITION) || b->partition.n == 0)
    return partitions_match_default(a);
  else {
    // 遍历两个分区的字符串，检查它们是否互相匹配 (Iterate through strings of both partitions and
    // check if they match each other)
    for (uint32_t i = 0; i < a->partition.n; i++)
      for (uint32_t j = 0; j < b->partition.n; j++) {
        if (partition_patmatch_p(a->partition.strs[i], b->partition.strs[j]) ||
            partition_patmatch_p(b->partition.strs[j], a->partition.strs[i]))
          return 1;
      }
    return 0;
  }
}

#ifdef DDS_HAS_TYPE_DISCOVERY

/**
 * @brief 检查端点类型是否已解析 (Check if the endpoint type is resolved)
 *
 * @param[in] gv        域全局变量指针 (Pointer to domain global variables)
 * @param[in] type_name 类型名称 (Type name)
 * @param[in] type_pair 类型对，包含最小和完整类型 (Type pair, including minimal and complete types)
 * @param[out] req_lookup 是否需要查询类型的布尔值指针 (Pointer to a boolean value indicating
 * whether a type lookup is required)
 * @param[in] entity    实体名称 (Entity name)
 * @return uint32_t     返回类型解析状态 (Return the type resolution status)
 */
/*
`ddsrt_nonnull((1, 2, 3))` 是一个函数属性，它表示该函数的第1、2和3个参数不能为空（即不能为
`NULL`）。这是一种编译时检查，用于确保在调用此函数时不会传递无效的空指针。这有助于提高代码的健壮性并减少潜在的运行时错误。

这个属性紧跟在函数原型之后，但在函数实现之前。因此，它不需要花括号
`{}`。实际上，它只是一个修饰符，告诉编译器如何处理这个函数。
*/
static uint32_t is_endpoint_type_resolved(struct ddsi_domaingv *gv,
                                          char *type_name,
                                          const ddsi_type_pair_t *type_pair,
                                          bool *req_lookup,
                                          const char *entity) ddsrt_nonnull((1, 2, 3));

static uint32_t is_endpoint_type_resolved(struct ddsi_domaingv *gv,
                                          char *type_name,
                                          const ddsi_type_pair_t *type_pair,
                                          bool *req_lookup,
                                          const char *entity) {
  // 断言：确保 type_pair 不为空 (Assert: Ensure type_pair is not null)
  assert(type_pair);

  // 锁定类型库互斥锁 (Lock the typelib mutex)
  ddsrt_mutex_lock(&gv->typelib_lock);

  // 检查最小和完整类型是否已解析 (Check if minimal and complete types are resolved)
  bool min_resolved = ddsi_type_resolved_locked(gv, type_pair->minimal, DDSI_TYPE_INCLUDE_DEPS),
       compl_resolved = ddsi_type_resolved_locked(gv, type_pair->complete, DDSI_TYPE_INCLUDE_DEPS);

  // 如果最小和完整类型都未解析 (If both minimal and complete types are not resolved)
  if (!min_resolved && !compl_resolved) {
    struct ddsi_typeid_str str;
    const ddsi_typeid_t *tid_m = ddsi_type_pair_minimal_id(type_pair),
                        *tid_c = ddsi_type_pair_complete_id(type_pair);
    // 记录未解析的类型信息 (Log the unresolved type information)
    GVTRACE("unresolved %s type %s ", entity, type_name);
    if (tid_m) GVTRACE("min %s", ddsi_make_typeid_str(&str, tid_m));
    if (tid_c) GVTRACE("compl %s", ddsi_make_typeid_str(&str, tid_c));
    GVTRACE("\n");

    /* 在释放端点 QoS 锁之后再请求未解析的类型，
       因此只需设置一个布尔值，表示需要进行类型查找 */
    /* Defer requesting the unresolved type until after the endpoint QoS lock
       has been released, so just set a bool value indicating that a type
       lookup is required */
    if (req_lookup != NULL) *req_lookup = true;

    // 解锁类型库互斥锁 (Unlock the typelib mutex)
    ddsrt_mutex_unlock(&gv->typelib_lock);

    // 返回无类型状态 (Return no type status)
    return DDS_XTypes_TK_NONE;
  }

  // 解锁类型库互斥锁 (Unlock the typelib mutex)
  ddsrt_mutex_unlock(&gv->typelib_lock);

  // 如果最小和完整类型都已解析，返回双重状态 (If both minimal and complete types are resolved,
  // return both status)
  if (min_resolved && compl_resolved) return DDS_XTypes_EK_BOTH;

  // 返回已解析的类型状态 (Return the resolved type status)
  return compl_resolved ? DDS_XTypes_EK_COMPLETE : DDS_XTypes_EK_MINIMAL;
}

#endif /* DDS_HAS_TYPE_DISCOVERY */

/**
 * @brief 判断数据表示是否匹配 (Check if data representations match)
 *
 * @param[in] rd_qos 读者的 QoS 设置 (Reader's QoS settings)
 * @param[in] wr_qos 写者的 QoS 设置 (Writer's QoS settings)
 * @return 匹配返回 1，不匹配返回 0 (Return 1 if matched, 0 otherwise)
 */
static int data_representation_match_p(const dds_qos_t *rd_qos, const dds_qos_t *wr_qos) {
  // 确保读者的 QoS 设置包含数据表示 (Ensure reader's QoS settings contain data representation)
  assert(rd_qos->present & DDSI_QP_DATA_REPRESENTATION);
  // 确保读者的数据表示值大于 0 (Ensure reader's data representation value is greater than 0)
  assert(rd_qos->data_representation.value.n > 0);
  // 确保写者的 QoS 设置包含数据表示 (Ensure writer's QoS settings contain data representation)
  assert(wr_qos->present & DDSI_QP_DATA_REPRESENTATION);
  // 确保写者的数据表示值大于 0 (Ensure writer's data representation value is greater than 0)
  assert(wr_qos->data_representation.value.n > 0);

  /* 对于写者，仅使用第一个表示标识符并忽略 1..n（规范 7.6.3.1.1）
   * (For the writer, only use the first representation identifier and ignore 1..n (spec 7.6.3.1.1))
   */
  for (uint32_t i = 0; i < rd_qos->data_representation.value.n; i++)
    if (rd_qos->data_representation.value.ids[i] == wr_qos->data_representation.value.ids[0])
      return 1;
  return 0;
}

#ifdef DDS_HAS_TYPE_DISCOVERY
/**
 * @brief 检查类型对是否具有 ID (Check if type pair has an ID)
 *
 * @param[in] pair 类型对指针 (Pointer to the type pair)
 * @return 如果类型对具有 ID，则返回 true，否则返回 false (Return true if type pair has an ID, false
 * otherwise)
 */
static bool type_pair_has_id(const ddsi_type_pair_t *pair) {
  // 类型对存在并且至少有一个（最小或完整）是真的 (Type pair exists and at least one (minimal or
  // complete) is true)
  return pair && (pair->minimal || pair->complete);
}
#endif

/**
 * @brief 检查读写器的 QoS 是否匹配，同时考虑给定的掩码和类型对。
 *
 * Check if the QoS of a reader and a writer match, taking into account the given mask and type
pairs.
 *
 * @param[in] gv 域全局变量指针。Pointer to domain global variables.
 * @param[in] rd_qos 读者的 QoS 设置。QoS settings of the reader.
 * @param[in] wr_qos 写者的 QoS 设置。QoS settings of the writer.
 * @param[in] mask 要检查的 QoS 策略的掩码。Mask of QoS policies to check.
 * @param[out] reason 如果不匹配，返回导致不匹配的 QoS 策略 ID。If not matching, returns the QoS
policy ID causing the mismatch. #ifdef DDS_HAS_TYPE_DISCOVERY
 * @param[in] rd_type_pair 读者的类型对。Type pair of the reader.
 * @param[in] wr_type_pair 写者的类型对。Type pair of the writer.
 * @param[out] rd_typeid_req_lookup 读者是否需要查找类型 ID。Whether the reader needs to lookup the
type ID.
 * @param[out] wr_typeid_req_lookup 写者是否需要查找类型 ID。Whether the writer needs to lookup the
type ID. #endif
 * @return 如果 QoS 匹配，则返回 true；否则返回 false。Returns true if QoS match, false otherwise.
 */
bool ddsi_qos_match_mask_p(struct ddsi_domaingv *gv,
                           const dds_qos_t *rd_qos,
                           const dds_qos_t *wr_qos,
                           uint64_t mask,
                           dds_qos_policy_id_t *reason
#ifdef DDS_HAS_TYPE_DISCOVERY
                           ,
                           const struct ddsi_type_pair *rd_type_pair,
                           const struct ddsi_type_pair *wr_type_pair,
                           bool *rd_typeid_req_lookup,
                           bool *wr_typeid_req_lookup
#endif
) {
  // 忽略未使用的参数。Ignore unused argument.
  DDSRT_UNUSED_ARG(gv);

#ifndef NDEBUG
  // 必须具有的 QoS 策略掩码。Mask of QoS policies that must be present.
  uint64_t musthave = (DDSI_QP_RXO_MASK | DDSI_QP_PARTITION | DDSI_QP_TOPIC_NAME |
                       DDSI_QP_TYPE_NAME | DDSI_QP_DATA_REPRESENTATION) &
                      mask;
  // 断言读者和写者的 QoS 设置包含必须具有的策略。Assert that the reader and writer QoS settings
  // contain the required policies.
  assert((rd_qos->present & musthave) == musthave);
  assert((wr_qos->present & musthave) == musthave);
#endif

#ifdef DDS_HAS_TYPE_DISCOVERY
  // 初始化类型 ID 请求查找标志
  // Initialize the type ID request lookup flags
  if (rd_typeid_req_lookup != NULL) *rd_typeid_req_lookup = false;
  if (wr_typeid_req_lookup != NULL) *wr_typeid_req_lookup = false;
#endif

  // 计算 QoS 的掩码
  // Calculate the QoS mask
  uint32_t mask = rd_qos->present & wr_qos->present;

  // 初始化原因为无效的 QoS 政策 ID
  // Initialize the reason as an invalid QoS policy ID
  *reason = DDS_INVALID_QOS_POLICY_ID;

  // 检查主题名称是否匹配
  // Check if topic names match
  if ((mask & DDSI_QP_TOPIC_NAME) && strcmp(rd_qos->topic_name, wr_qos->topic_name) != 0)
    return false;

  // 检查可靠性 QoS 政策是否兼容
  // Check if reliability QoS policy is compatible
  if ((mask & DDSI_QP_RELIABILITY) && rd_qos->reliability.kind > wr_qos->reliability.kind) {
    *reason = DDS_RELIABILITY_QOS_POLICY_ID;
    return false;
  }

  // 检查持久性 QoS 政策是否兼容
  // Check if durability QoS policy is compatible
  if ((mask & DDSI_QP_DURABILITY) && rd_qos->durability.kind > wr_qos->durability.kind) {
    *reason = DDS_DURABILITY_QOS_POLICY_ID;
    return false;
  }

  // 检查演示 QoS 政策的访问范围是否兼容
  // Check if presentation QoS policy's access scope is compatible
  if ((mask & DDSI_QP_PRESENTATION) &&
      rd_qos->presentation.access_scope > wr_qos->presentation.access_scope) {
    *reason = DDS_PRESENTATION_QOS_POLICY_ID;
    return false;
  }

  // 检查演示 QoS 政策的一致访问是否兼容
  // Check if presentation QoS policy's coherent access is compatible
  if ((mask & DDSI_QP_PRESENTATION) &&
      rd_qos->presentation.coherent_access > wr_qos->presentation.coherent_access) {
    *reason = DDS_PRESENTATION_QOS_POLICY_ID;
    return false;
  }

  // 检查演示 QoS 政策的有序访问是否兼容
  // Check if presentation QoS policy's ordered access is compatible
  if ((mask & DDSI_QP_PRESENTATION) &&
      rd_qos->presentation.ordered_access > wr_qos->presentation.ordered_access) {
    *reason = DDS_PRESENTATION_QOS_POLICY_ID;
    return false;
  }

  // 检查截止日期 QoS 政策是否兼容
  // Check if deadline QoS policy is compatible
  if ((mask & DDSI_QP_DEADLINE) && rd_qos->deadline.deadline < wr_qos->deadline.deadline) {
    *reason = DDS_DEADLINE_QOS_POLICY_ID;
    return false;
  }

  // 检查延迟预算 QoS 政策是否兼容
  // Check if latency budget QoS policy is compatible
  if ((mask & DDSI_QP_LATENCY_BUDGET) &&
      rd_qos->latency_budget.duration < wr_qos->latency_budget.duration) {
    *reason = DDS_LATENCYBUDGET_QOS_POLICY_ID;
    return false;
  }

  // 检查所有权 QoS 政策是否兼容
  // Check if ownership QoS policy is compatible
  if ((mask & DDSI_QP_OWNERSHIP) && rd_qos->ownership.kind != wr_qos->ownership.kind) {
    *reason = DDS_OWNERSHIP_QOS_POLICY_ID;
    return false;
  }

  // 检查活跃度 QoS 政策是否兼容
  // Check if liveliness QoS policy is compatible
  if ((mask & DDSI_QP_LIVELINESS) && rd_qos->liveliness.kind > wr_qos->liveliness.kind) {
    *reason = DDS_LIVELINESS_QOS_POLICY_ID;
    return false;
  }

  // 检查活跃度 QoS 政策的租赁持续时间是否兼容
  // Check if liveliness QoS policy's lease duration is compatible
  if ((mask & DDSI_QP_LIVELINESS) &&
      rd_qos->liveliness.lease_duration < wr_qos->liveliness.lease_duration) {
    *reason = DDS_LIVELINESS_QOS_POLICY_ID;
    return false;
  }

  // 检查目标顺序 QoS 政策是否兼容
  // Check if destination order QoS policy is compatible
  if ((mask & DDSI_QP_DESTINATION_ORDER) &&
      rd_qos->destination_order.kind > wr_qos->destination_order.kind) {
    *reason = DDS_DESTINATIONORDER_QOS_POLICY_ID;
    return false;
  }

  // 检查分区 QoS 政策是否兼容
  // Check if partition QoS policy is compatible
  if ((mask & DDSI_QP_PARTITION) && !partitions_match_p(rd_qos, wr_qos)) {
    *reason = DDS_PARTITION_QOS_POLICY_ID;
    return false;
  }

  // 检查数据表示 QoS 政策是否兼容
  // Check if data representation QoS policy is compatible
  if ((mask & DDSI_QP_DATA_REPRESENTATION) && !data_representation_match_p(rd_qos, wr_qos)) {
    *reason = DDS_DATA_REPRESENTATION_QOS_POLICY_ID;
    return false;
  }

#ifdef DDS_HAS_TYPE_DISCOVERY
  // 如果读取器或写入器的类型对没有类型ID，则检查类型信息是否缺失 (If either the reader or writer
  // type pair does not have a type id, check for missing type info)
  if (!type_pair_has_id(rd_type_pair) || !type_pair_has_id(wr_type_pair)) {
    // 如果设置了“强制类型验证”，则在任何一端或两端都缺少类型信息时自动失败 (Automatic failure if
    // "force type validation" is set and type info is missing on either or both ends)
    if (rd_qos->type_consistency.force_type_validation) {
      *reason = DDS_TYPE_CONSISTENCY_ENFORCEMENT_QOS_POLICY_ID;
      return false;
    }
    // 如果读取器或写入器不提供类型ID，则查询类型名称 (XTypes spec 7.6.3.4.2) (If either the reader
    // or writer does not provide a type id, the type names are consulted)
    if ((mask & DDSI_QP_TYPE_NAME) && strcmp(rd_qos->type_name, wr_qos->type_name) != 0)
      return false;
  } else {
    // 初始化类型一致性强制策略 (Initialize the type consistency enforcement policy)
    dds_type_consistency_enforcement_qospolicy_t tce = {
        .kind = DDS_TYPE_CONSISTENCY_ALLOW_TYPE_COERCION,
        .ignore_sequence_bounds = true,
        .ignore_string_bounds = true,
        .ignore_member_names = false,
        .prevent_type_widening = false,
        .force_type_validation = false};
    // 获取读取器的类型一致性策略 (Get the type consistency policy of the reader)
    (void)dds_qget_type_consistency(rd_qos, &tce.kind, &tce.ignore_sequence_bounds,
                                    &tce.ignore_string_bounds, &tce.ignore_member_names,
                                    &tce.prevent_type_widening, &tce.force_type_validation);

    // 如果不允许类型强制，则比较类型ID (If type coercion is disallowed, compare the type ids)
    if (tce.kind == DDS_TYPE_CONSISTENCY_DISALLOW_TYPE_COERCION) {
      if (ddsi_typeid_compare(ddsi_type_pair_minimal_id(rd_type_pair),
                              ddsi_type_pair_minimal_id(wr_type_pair))) {
        *reason = DDS_TYPE_CONSISTENCY_ENFORCEMENT_QOS_POLICY_ID;
        return false;
      }
    } else {
      // 检查端点类型是否已解析 (Check if the endpoint types are resolved)
      uint32_t rd_resolved, wr_resolved;
      if (!(rd_resolved = is_endpoint_type_resolved(gv, rd_qos->type_name, rd_type_pair,
                                                    rd_typeid_req_lookup, "rd")) ||
          !(wr_resolved = is_endpoint_type_resolved(gv, wr_qos->type_name, wr_type_pair,
                                                    wr_typeid_req_lookup, "wr")))
        return false;

      // 检查类型是否可分配 (Check if the types are assignable)
      if (!ddsi_is_assignable_from(gv, rd_type_pair, rd_resolved, wr_type_pair, wr_resolved,
                                   &tce)) {
        *reason = DDS_TYPE_CONSISTENCY_ENFORCEMENT_QOS_POLICY_ID;
        return false;
      }
    }
  }
#else
  // 如果没有启用类型发现功能，仅比较类型名称 (If type discovery is not enabled, just compare the
  // type names)
  if ((mask & DDSI_QP_TYPE_NAME) && strcmp(rd_qos->type_name, wr_qos->type_name) != 0) return false;
#endif

  return true;
}

/**
 * @brief 检查两个 QoS 是否匹配 (Check if two QoS match)
 *
 * @param[in] gv 域全局变量指针 (Pointer to domain global variables)
 * @param[in] rd_qos 读者的 QoS (Reader's QoS)
 * @param[in] wr_qos 写者的 QoS (Writer's QoS)
 * @param[out] reason 如果不匹配，返回不匹配的原因 (If not matching, return the reason for not
matching) #ifdef DDS_HAS_TYPE_DISCOVERY
 * @param[in] rd_type_pair 读者的类型对 (Reader's type pair)
 * @param[in] wr_type_pair 写者的类型对 (Writer's type pair)
 * @param[out] rd_typeid_req_lookup 读者类型 ID 需要查询 (Reader type ID needs lookup)
 * @param[out] wr_typeid_req_lookup 写者类型 ID 需要查询 (Writer type ID needs lookup)
#endif
 * @return 如果匹配返回 true，否则返回 false (Return true if matching, otherwise return false)
 */
bool ddsi_qos_match_p(struct ddsi_domaingv *gv,
                      const dds_qos_t *rd_qos,
                      const dds_qos_t *wr_qos,
                      dds_qos_policy_id_t *reason
#ifdef DDS_HAS_TYPE_DISCOVERY
                      ,
                      const struct ddsi_type_pair *rd_type_pair,
                      const struct ddsi_type_pair *wr_type_pair,
                      bool *rd_typeid_req_lookup,
                      bool *wr_typeid_req_lookup
#endif
) {
  // 创建一个临时的 dummy 变量用于存储不匹配的原因 (Create a temporary dummy variable to store the
  // reason for not matching)
  dds_qos_policy_id_t dummy;

#ifdef DDS_HAS_TYPE_DISCOVERY
  // 调用 ddsi_qos_match_mask_p 函数并传入所有参数 (Call the ddsi_qos_match_mask_p function and pass
  // in all parameters)
  return ddsi_qos_match_mask_p(gv, rd_qos, wr_qos, ~(uint64_t)0, reason ? reason : &dummy,
                               rd_type_pair, wr_type_pair, rd_typeid_req_lookup,
                               wr_typeid_req_lookup);
#else
  // 如果没有类型发现，调用 ddsi_qos_match_mask_p 函数并传入部分参数 (If there is no type discovery,
  // call the ddsi_qos_match_mask_p function and pass in some parameters)
  return ddsi_qos_match_mask_p(gv, rd_qos, wr_qos, ~(uint64_t)0, reason ? reason : &dummy);
#endif
}
