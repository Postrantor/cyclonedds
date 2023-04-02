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
#include <ctype.h>
#include <string.h>

#include "dds/cdr/dds_cdrstream.h"
#include "dds/ddsc/dds_internal_api.h"
#include "dds/ddsi/ddsi_domaingv.h"
#include "dds/ddsi/ddsi_endpoint.h"
#include "dds/ddsi/ddsi_entity.h"
#include "dds/ddsi/ddsi_entity_index.h"
#include "dds/ddsi/ddsi_iid.h"
#include "dds/ddsi/ddsi_plist.h"
#include "dds/ddsi/ddsi_security_omg.h"
#include "dds/ddsi/ddsi_sertype.h"
#include "dds/ddsi/ddsi_thread.h"
#include "dds/ddsi/ddsi_typebuilder.h"
#include "dds/ddsrt/atomics.h"
#include "dds/ddsrt/heap.h"
#include "dds/ddsrt/hopscotch.h"
#include "dds/ddsrt/misc.h"
#include "dds/ddsrt/string.h"
#include "dds__builtin.h"
#include "dds__domain.h"
#include "dds__get_status.h"
#include "dds__init.h"
#include "dds__listener.h"
#include "dds__participant.h"
#include "dds__qos.h"
#include "dds__serdata_builtintopic.h"
#include "dds__serdata_default.h"
#include "dds__topic.h"
// 声明实体锁和解锁函数（针对dds_topic）
DECL_ENTITY_LOCK_UNLOCK(dds_topic)

// 定义DDS_TOPIC_STATUS_MASK，用于表示不一致主题状态
#define DDS_TOPIC_STATUS_MASK (DDS_INCONSISTENT_TOPIC_STATUS)

// 定义结构体topic_sertype_node，包含avl节点、引用计数和ddsi_sertype指针
struct topic_sertype_node {
  ddsrt_avl_node_t avlnode;       // AVL树节点
  uint32_t refc;                  // 引用计数
  const struct ddsi_sertype *st;  // 指向ddsi_sertype的指针
};

// 函数原型声明：检查名称是否有效
static bool is_valid_name(const char *name) ddsrt_nonnull_all;

// 实现is_valid_name函数，检查给定的名称是否有效
static bool is_valid_name(const char *name) {
  /* DDS规范没有明确说明什么是有效的名称。
   * 根据https://github.com/eclipse-cyclonedds/cyclonedds/pull/1426
   *  首先要求isprint为true且不是<space>*?[]"'，然后逐步支持UTF-8
   */
  const char *invalid = "*?[]\"'#$";  // 定义无效字符集

  if (name[0] == '\0')                // 如果名称为空字符串，则返回false
    return false;

  for (size_t i = 0; name[i]; i++)  // 遍历名称中的每个字符
    if ((!(isprint((unsigned char)name[i]))) || (isspace((unsigned char)name[i])) ||
        (strchr(invalid, name[i]) != NULL))
      return false;  // 如果字符不可打印、为空格或在无效字符集中，则返回false
  return true;       // 所有字符都有效，返回true
}

// 实现dds_topic_status_validate函数，验证主题状态掩码是否有效
static dds_return_t dds_topic_status_validate(uint32_t mask) {
  // 如果mask与~DDS_TOPIC_STATUS_MASK进行按位与操作的结果不为0，则返回DDS_RETCODE_BAD_PARAMETER，否则返回DDS_RETCODE_OK
  return (mask & ~DDS_TOPIC_STATUS_MASK) ? DDS_RETCODE_BAD_PARAMETER : DDS_RETCODE_OK;
}

// 如果定义了DDS_HAS_TOPIC_DISCOVERY
#ifdef DDS_HAS_TOPIC_DISCOVERY

// 实现topic_guid_map_refc_impl函数，根据dds_ktopic和ddsi_sertype引用或取消引用ktopic_type_guid
static struct ktopic_type_guid *topic_guid_map_refc_impl(const struct dds_ktopic *ktp,
                                                         const struct ddsi_sertype *sertype,
                                                         bool unref) {
  struct ktopic_type_guid *m = NULL;  // 初始化ktopic_type_guid指针为NULL
  ddsi_typeid_t *type_id =
      ddsi_sertype_typeid(sertype, DDSI_TYPEID_KIND_COMPLETE);  // 获取sertype的完整类型ID
  if (ddsi_typeid_is_none(type_id))                             // 如果类型ID为空
    goto no_typeid;                                             // 跳转到no_typeid标签
  struct ktopic_type_guid templ = {.type_id =
                                       type_id};  // 创建ktopic_type_guid模板，设置type_id字段
  m = ddsrt_hh_lookup(ktp->topic_guid_map,
                      &templ);  // 在ktp的topic_guid_map中查找模板对应的ktopic_type_guid
  assert(m != NULL);            // 断言找到的ktopic_type_guid不为NULL
  if (unref)                    // 如果需要取消引用
    m->refc--;                  // 引用计数减1
  else                          // 否则
    m->refc++;                  // 引用计数加1

no_typeid:                      // no_typeid标签
  if (type_id != NULL)          // 如果类型ID不为空
  {
    ddsi_typeid_fini(type_id);  // 销毁类型ID
    ddsrt_free(type_id);        // 释放类型ID内存
  }
  return m;                     // 返回ktopic_type_guid指针
}

// 当前不需要此函数，但对称性参数建议它应该存在，编译器不同意
#if 0
static void topic_guid_map_ref (const struct dds_ktopic * ktp, const struct ddsi_sertype *sertype)
{
  (void) topic_guid_map_refc_impl (ktp, sertype, false);
}
#endif

// 实现topic_guid_map_unref函数，根据dds_ktopic和ddsi_sertype取消引用ktopic_type_guid
static void topic_guid_map_unref(struct ddsi_domaingv *const gv,
                                 const struct dds_ktopic *ktp,
                                 const struct ddsi_sertype *sertype) {
  struct ktopic_type_guid *m = topic_guid_map_refc_impl(
      ktp, sertype, true);  // 调用topic_guid_map_refc_impl函数，取消引用ktopic_type_guid
  if (m == NULL)            // 如果ktopic_type_guid为空
    return;                 // 直接返回

  if (m->refc == 0)         // 如果引用计数为0
  {
    ddsrt_hh_remove_present(ktp->topic_guid_map, m);  // 从ktp的topic_guid_map中移除ktopic_type_guid
    ddsi_thread_state_awake(ddsi_lookup_thread_state(), gv);  // 唤醒线程状态
    (void)ddsi_delete_topic(gv, &m->guid);                    // 删除主题
    ddsi_thread_state_asleep(ddsi_lookup_thread_state());     // 线程进入休眠状态
    ddsi_typeid_fini(m->type_id);                             // 销毁类型ID
    ddsrt_free(m->type_id);                                   // 释放类型ID内存
    dds_free(m);                                              // 释放ktopic_type_guid内存
  }
}

#endif /* DDS_HAS_TOPIC_DISCOVERY */
// 主题状态更改回调处理程序。支持INCONSISTENT_TOPIC状态（主题上唯一定义的状态）。
// 在可以检测到不一致的主题定义之前，此处理程序无关紧要，因此在添加主题发现功能之前无关紧要。
#if 0
static void dds_topic_status_cb (struct dds_topic *tp)
{
  struct dds_listener const * const lst = &tp->m_entity.m_listener; // 获取主题实体的监听器

  // 对观察者锁进行加锁
  ddsrt_mutex_lock (&tp->m_entity.m_observers_lock);
  // 当回调计数大于0时，等待观察者条件变量
  while (tp->m_entity.m_cb_count > 0)
    ddsrt_cond_wait (&tp->m_entity.m_observers_cond, &tp->m_entity.m_observers_lock);
  tp->m_entity.m_cb_count++; // 回调计数加1

  // 更新不一致主题状态的总计数和总计数变化
  tp->m_inconsistent_topic_status.total_count++;
  tp->m_inconsistent_topic_status.total_count_change++;
  // 如果设置了on_inconsistent_topic监听器
  if (lst->on_inconsistent_topic)
  {
    // 解锁观察者锁
    ddsrt_mutex_unlock (&tp->m_entity.m_observers_lock);
    // 调用实体监听器处理不一致主题状态
    dds_entity_invoke_listener (&tp->m_entity, DDS_INCONSISTENT_TOPIC_STATUS_ID, &tp->m_inconsistent_topic_status);
    // 重新加锁观察者锁
    ddsrt_mutex_lock (&tp->m_entity.m_observers_lock);
    // 将不一致主题状态的总计数变化重置为0
    tp->m_inconsistent_topic_status.total_count_change = 0;
  }

  // 设置实体状态为DDS_INCONSISTENT_TOPIC_STATUS
  dds_entity_status_set (&tp->m_entity, DDS_INCONSISTENT_TOPIC_STATUS);
  tp->m_entity.m_cb_count--; // 回调计数减1
  // 广播观察者条件变量
  ddsrt_cond_broadcast (&tp->m_entity.m_observers_cond);
  // 解锁观察者锁
  ddsrt_mutex_unlock (&tp->m_entity.m_observers_lock);
}
#endif

// 实现dds_topic_pin_with_origin函数，根据实体句柄和来源锁定主题
dds_return_t dds_topic_pin_with_origin(dds_entity_t handle, bool from_user, struct dds_topic **tp) {
  struct dds_entity *e;
  dds_return_t ret;
  // 使用给定的实体句柄和来源锁定实体
  if ((ret = dds_entity_pin_with_origin(handle, from_user, &e)) < 0)
    return ret;  // 如果锁定失败，返回错误代码
  // 检查实体类型是否为DDS_KIND_TOPIC
  if (dds_entity_kind(e) != DDS_KIND_TOPIC) {
    dds_entity_unpin(e);                   // 解锁实体
    return DDS_RETCODE_ILLEGAL_OPERATION;  // 返回非法操作错误代码
  }
  *tp = (struct dds_topic *)e;  // 将实体转换为dds_topic并存储到输出参数tp中
  return DDS_RETCODE_OK;        // 返回成功
}

// 实现dds_topic_pin函数，根据实体句柄锁定主题
dds_return_t dds_topic_pin(dds_entity_t handle, struct dds_topic **tp) {
  return dds_topic_pin_with_origin(handle, true,
                                   tp);  // 调用dds_topic_pin_with_origin函数，来源设置为true
}

// 实现dds_topic_unpin函数，解锁给定的dds_topic
void dds_topic_unpin(struct dds_topic *tp) {
  dds_entity_unpin(&tp->m_entity);  // 解锁主题实体
}

// 实现dds_topic_defer_set_qos函数，延迟设置主题的QoS
void dds_topic_defer_set_qos(struct dds_topic *tp) {
  struct dds_ktopic *const ktp = tp->m_ktopic;  // 获取主题的dds_ktopic
  struct dds_participant *const pp = dds_entity_participant(&tp->m_entity);  // 获取主题实体的参与者
  ddsrt_mutex_lock(&pp->m_entity.m_mutex);    // 对参与者实体加锁
  ++ktp->defer_set_qos;                       // 增加延迟设置QoS计数
  ddsrt_mutex_unlock(&pp->m_entity.m_mutex);  // 解锁参与者实体
}

// 实现dds_topic_allow_set_qos函数，允许设置主题的QoS
void dds_topic_allow_set_qos(struct dds_topic *tp) {
  struct dds_ktopic *const ktp = tp->m_ktopic;  // 获取主题的dds_ktopic
  struct dds_participant *const pp = dds_entity_participant(&tp->m_entity);  // 获取主题实体的参与者
  ddsrt_mutex_lock(&pp->m_entity.m_mutex);  // 对参与者实体加锁
  assert(ktp->defer_set_qos > 0);           // 断言延迟设置QoS计数大于0
  if (--ktp->defer_set_qos == 0)            // 减少延迟设置QoS计数，如果计数变为0
    ddsrt_cond_broadcast(&pp->m_entity.m_cond);  // 广播参与者实体的条件变量
  ddsrt_mutex_unlock(&pp->m_entity.m_mutex);     // 解锁参与者实体
}
// 减少 ktopic 的引用计数，如果引用计数为 0，则删除 ktopic
static void ktopic_unref(dds_participant *const pp, struct dds_ktopic *const ktp) {
  // 如果 ktopic 的引用计数减少到 0
  if (--ktp->refc == 0) {
    // 从参与者的 ktopics 树中删除 ktopic
    ddsrt_avl_delete(&participant_ktopics_treedef, &pp->m_ktopics, ktp);
    // 删除 ktopic 的 QoS
    dds_delete_qos(ktp->qos);
    // 释放 ktopic 名称的内存
    dds_free(ktp->name);
#ifdef DDS_HAS_TOPIC_DISCOVERY
    // 如果启用了主题发现功能，释放 ktopic 的 topic_guid_map
    ddsrt_hh_free(ktp->topic_guid_map);
#endif
    // 释放 ktopic 结构的内存
    dds_free(ktp);
  }
}

// 声明 dds_topic_close 函数，该函数用于关闭主题实体
static void dds_topic_close(dds_entity *e) ddsrt_nonnull_all;

// 实现 dds_topic_close 函数
static void dds_topic_close(dds_entity *e) {
  // 将 e 转换为 dds_topic 类型的指针
  struct dds_topic *const tp = (dds_topic *)e;
  // 获取 ktopic 指针
  struct dds_ktopic *const ktp = tp->m_ktopic;
  // 确保 e 的父实体类型为 DDS_KIND_PARTICIPANT
  assert(dds_entity_kind(e->m_parent) == DDS_KIND_PARTICIPANT);
  // 将 e 的父实体转换为 dds_participant 类型的指针
  dds_participant *const pp = (dds_participant *)e->m_parent;
#ifdef DDS_HAS_TYPE_DISCOVERY
  // 如果启用了类型发现功能，取消引用 sertype
  ddsi_type_unref_sertype(&e->m_domain->gv, tp->m_stype);
#endif
  // 释放主题名称的内存
  dds_free(tp->m_name);

  // 锁定参与者实体的互斥锁
  ddsrt_mutex_lock(&pp->m_entity.m_mutex);

#ifdef DDS_HAS_TOPIC_DISCOVERY
  // 如果启用了主题发现功能，取消引用 topic_guid_map
  topic_guid_map_unref(&e->m_domain->gv, ktp, tp->m_stype);
#endif
  // 减少 ktopic 的引用计数
  ktopic_unref(pp, ktp);
  // 解锁参与者实体的互斥锁
  ddsrt_mutex_unlock(&pp->m_entity.m_mutex);
  // 取消引用 sertype
  ddsi_sertype_unref(tp->m_stype);
}

// 设置主题实体的 QoS
static dds_return_t dds_topic_qos_set(dds_entity *e, const dds_qos_t *qos, bool enabled) {
  // 注意：此时 e->m_qos 仍然是旧的 QoS，以便在此处失败时进行处理
#ifdef DDS_HAS_TOPIC_DISCOVERY
  // 如果启用了主题发现功能
  if (enabled) {
    // 将 e 转换为 dds_topic 类型的指针
    struct dds_topic *tp = (struct dds_topic *)e;
    // 获取 ktopic 指针
    struct dds_ktopic *const ktp = tp->m_ktopic;
    // 唤醒线程状态
    ddsi_thread_state_awake(ddsi_lookup_thread_state(), &e->m_domain->gv);
    // 定义迭代器
    struct ddsrt_hh_iter it;
    // 父实体 pp 已锁定并保护 ktp->topic_guid_map
    // 遍历 ktopic 的 topic_guid_map
    for (struct ktopic_type_guid *obj = ddsrt_hh_iter_first(ktp->topic_guid_map, &it); obj;
         obj = ddsrt_hh_iter_next(&it)) {
      // 定义 ddsi_topic 指针
      struct ddsi_topic *ddsi_tp;
      // 如果能够根据 GUID 查找到 ddsi_topic
      if ((ddsi_tp = ddsi_entidx_lookup_topic_guid(e->m_domain->gv.entity_index, &obj->guid)) !=
          NULL)
        // 更新 ddsi_topic 的 QoS
        ddsi_update_topic_qos(ddsi_tp, qos);
    }
    // 使线程状态进入休眠
    ddsi_thread_state_asleep(ddsi_lookup_thread_state());
  }
#else
  // 如果未启用主题发现功能，不执行任何操作
  (void)e;
  (void)qos;
  (void)enabled;
#endif
  // 返回成功代码
  return DDS_RETCODE_OK;
}

/**
 * 检查给定的 QoS 和 ktopic 的 QoS 是否相同或兼容。
 *
 * @param qos  要检查的 QoS 指针。
 * @param ktp  包含要比较的 QoS 的 ktopic 结构指针。
 * @return     如果给定的 QoS 和 ktopic 的 QoS 相同或兼容，则返回 true，否则返回 false。
 */
static bool dupdef_qos_ok(const dds_qos_t *qos, const dds_ktopic *ktp) {
  // 如果 qos 为 NULL 与 ktp->qos 为 NULL 的情况不一致，则返回 false
  if ((qos == NULL) != (ktp->qos == NULL)) return false;
  // 如果 qos 为 NULL，则说明两者都为 NULL，返回 true
  else if (qos == NULL)
    return true;
  // 否则，比较两个 QoS 是否相等，并返回结果
  else
    return dds_qos_equal(ktp->qos, qos);
}

// 定义一个 dds_entity_deriver 结构体，用于实现主题实体的相关操作
const struct dds_entity_deriver dds_entity_deriver_topic = {
    .interrupt = dds_entity_deriver_dummy_interrupt,  // 中断操作：使用空操作（dummy）函数
    .close = dds_topic_close,                         // 关闭操作：关闭主题实体
    .delete = dds_entity_deriver_dummy_delete,  // 删除操作：使用空操作（dummy）函数
    .set_qos = dds_topic_qos_set,               // 设置 QoS 操作：设置主题实体的 QoS
    .validate_status = dds_topic_status_validate,  // 验证状态操作：验证主题实体的状态
    .create_statistics =
        dds_entity_deriver_dummy_create_statistics,  // 创建统计信息操作：使用空操作（dummy）函数
    .refresh_statistics =
        dds_entity_deriver_dummy_refresh_statistics};  // 刷新统计信息操作：使用空操作（dummy）函数

/**
 * @brief 检查参与者中是否存在具有相同名称的 ktopic，
 *        如果存在，检查其 QoS 是否匹配。
 *
 * ktopics 集合存储在参与者中，受参与者的互斥锁保护，
 * 这些 ktopics 的内部状态（包括 QoS）也受该互斥锁保护。
 *
 * @param[out] ktp_out    如果调用成功，则为匹配的 ktopic，如果不存在具有此名称的 ktopic，则为 NULL
 * @param[in]  pp         固定且已锁定的参与者
 * @param[in]  name       要查找的主题名称
 * @param[in]  new_qos    新主题的 QoS（可以为 NULL）
 *
 * @returns 成功 + ktopic，成功 + NULL 或错误。
 *
 * @retval DDS_RETCODE_OK
 *             ktp_out 是 NULL（首次尝试创建此主题），或
 *             匹配的 ktopic 实体
 * @retval DDS_RETCODE_INCONSISTENT_POLICY
 *             存在具有不同 QoS 的 ktopic
 * @retval DDS_RETCODE_PRECONDITION_NOT_MET
 *             存在具有不同类型名称的 ktopic
 */
static dds_return_t lookup_and_check_ktopic(struct dds_ktopic **ktp_out,
                                            dds_participant *pp,
                                            const char *name,
                                            const dds_qos_t *new_qos) {
  // 获取参与者实体所属域的全局变量结构
  struct ddsi_domaingv *const gv = &pp->m_entity.m_domain->gv;

  // 定义一个指向 dds_ktopic 结构的指针
  struct dds_ktopic *ktp;

  // 在参与者的 ktopics 中查找具有给定名称的 ktopic
  if ((ktp = *ktp_out = ddsrt_avl_lookup(&participant_ktopics_treedef, &pp->m_ktopics, name)) ==
      NULL) {
    // 如果没有找到具有该名称的 ktopic，则输出跟踪信息并返回 DDS_RETCODE_OK
    GVTRACE("lookup_and_check_ktopic_may_unlock_pp: no such ktopic\n");
    return DDS_RETCODE_OK;
  }
  // 如果找到的 ktopic 的 QoS 与给定的 new_qos 不匹配
  else if (!dupdef_qos_ok(new_qos, ktp)) {
    // 输出跟踪信息并返回 DDS_RETCODE_INCONSISTENT_POLICY
    GVTRACE("lookup_and_check_ktopic_may_unlock_pp: ktp %p qos mismatch\n", (void *)ktp);
    return DDS_RETCODE_INCONSISTENT_POLICY;
  } else {
    // 如果找到的 ktopic 可以重用，则输出跟踪信息并返回 DDS_RETCODE_OK
    GVTRACE("lookup_and_check_ktopic_may_unlock_pp: ktp %p reuse\n", (void *)ktp);
    return DDS_RETCODE_OK;
  }
}

/**
 * @brief 在已锁定的参与者中创建主题。
 *
 * @param[in]  pp          已锁定的参与者
 * @param[in]  ktp         要关联的 ktopic
 * @param[in]  builtin      是否为内置主题
 * @param[in]  topic_name   主题名称
 * @param[in]  sertype      序列化类型
 * @param[in]  listener     监听器
 *
 * @returns 创建的主题实体句柄。
 */
static dds_entity_t create_topic_pp_locked(struct dds_participant *pp,
                                           struct dds_ktopic *ktp,
                                           bool builtin,
                                           const char *topic_name,
                                           struct ddsi_sertype *sertype,
                                           const dds_listener_t *listener) {
  // 定义一个实体句柄变量
  dds_entity_t hdl;

  // 分配一个 dds_topic 结构的内存空间
  dds_topic *tp = dds_alloc(sizeof(*tp));

  // 初始化 tp 实体，设置其父实体为参与者实体，实体类型为 DDS_KIND_TOPIC
  // 如果是内置主题，则不允许应用程序删除
  hdl = dds_entity_init(&tp->m_entity, &pp->m_entity, DDS_KIND_TOPIC, builtin, !builtin, NULL,
                        listener, DDS_TOPIC_STATUS_MASK);

  // 为 tp 实体生成一个唯一的实例标识符
  tp->m_entity.m_iid = ddsi_iid_gen();

  // 将 tp 实体注册为参与者实体的子实体
  dds_entity_register_child(&pp->m_entity, &tp->m_entity);

  // 设置 tp 的 ktopic、名称和序列化类型
  tp->m_ktopic = ktp;
  tp->m_name = dds_string_dup(topic_name);
  tp->m_stype = sertype;

  // 完成 tp 实体的初始化
  dds_entity_init_complete(&tp->m_entity);

  // 返回创建的主题实体句柄
  return hdl;
}
// 如果定义了DDS_HAS_TOPIC_DISCOVERY宏
#ifdef DDS_HAS_TOPIC_DISCOVERY

// 注册主题类型以进行发现的静态函数
static bool register_topic_type_for_discovery(struct ddsi_domaingv *const gv,
                                              dds_participant *const pp,
                                              dds_ktopic *const ktp,
                                              bool is_builtin,
                                              struct ddsi_sertype *const sertype) {
  bool new_topic_def = false;

  // 创建或引用一个ktopic-sertype元数据条目。哈希表以
  // 完整的xtypes类型ID作为键；对于具有类型信息的本地和已发现主题，
  // 最小和完整类型标识符始终设置
  ddsi_typeid_t *type_id = ddsi_sertype_typeid(sertype, DDSI_TYPEID_KIND_COMPLETE);
  if (ddsi_typeid_is_none(type_id)) goto free_typeid;

  struct ktopic_type_guid templ = {.type_id = type_id}, *m;
  if ((m = ddsrt_hh_lookup(ktp->topic_guid_map, &templ))) {
    m->refc++;
    goto free_typeid;
  } else {
    // 使用sertype的完整类型标识符作为
    // 键，将新创建的ddsi主题实体的引用添加到ktopic-type-guid条目中
    ddsi_thread_state_awake(ddsi_lookup_thread_state(), gv);
    const struct ddsi_guid *pp_guid = dds_entity_participant_guid(&pp->m_entity);
    struct ddsi_participant *pp_ddsi =
        ddsi_entidx_lookup_participant_guid(gv->entity_index, pp_guid);

    m = dds_alloc(sizeof(*m));
    m->type_id = type_id;
    type_id = NULL;  // ktopic_type_guid获取type_id的所有权
    m->refc = 1;
    dds_return_t rc = ddsi_new_topic(&m->tp, &m->guid, pp_ddsi, ktp->name, sertype, ktp->qos,
                                     is_builtin, &new_topic_def);
    assert(rc == DDS_RETCODE_OK);  // FIXME: 至少可以是资源不足
    (void)rc;
    ddsrt_hh_add_absent(ktp->topic_guid_map, m);
    ddsi_thread_state_asleep(ddsi_lookup_thread_state());
  }
free_typeid:
  if (type_id != NULL) {
    ddsi_typeid_fini(type_id);
    ddsrt_free(type_id);
  }
  return new_topic_def;
}

// 比较两个ktopic_type_guid是否相等的静态函数
static int ktopic_type_guid_equal(const void *ktp_guid_a, const void *ktp_guid_b) {
  struct ktopic_type_guid *a = (struct ktopic_type_guid *)ktp_guid_a;
  struct ktopic_type_guid *b = (struct ktopic_type_guid *)ktp_guid_b;
  return !ddsi_typeid_compare(a->type_id, b->type_id);
}

// 计算ktopic_type_guid哈希值的静态函数
static uint32_t ktopic_type_guid_hash(const void *ktp_guid) {
  uint32_t hash32;
  struct ktopic_type_guid *x = (struct ktopic_type_guid *)ktp_guid;
  DDS_XTypes_EquivalenceHash hash;
  ddsi_typeid_get_equivalence_hash(x->type_id, &hash);
  memcpy(&hash32, hash, sizeof(hash32));
  return hash32;
}

// 如果没有定义DDS_HAS_TOPIC_DISCOVERY宏
#else
// 以下是带有详细中文注释的代码：

static bool register_topic_type_for_discovery(
    struct ddsi_domaingv *const gv,  // 参数1：指向ddsi_domaingv结构体的指针，表示域全局变量
    dds_participant *const pp,  // 参数2：指向dds_participant结构体的指针，表示参与者
    dds_ktopic *const ktp,      // 参数3：指向dds_ktopic结构体的指针，表示内核主题
    bool is_builtin,            // 参数4：布尔值，表示是否为内置主题
    struct ddsi_sertype *const sertype  // 参数5：指向ddsi_sertype结构体的指针，表示序列化类型
) {
  (void)gv;                             // 忽略gv参数
  (void)pp;                             // 忽略pp参数
  (void)ktp;                            // 忽略ktp参数
  (void)is_builtin;                     // 忽略is_builtin参数
  (void)sertype;                        // 忽略sertype参数
  return false;                         // 返回false
}

#endif /* DDS_HAS_TOPIC_DISCOVERY */
// 以下是带有详细中文注释的代码：

dds_entity_t dds_create_topic_impl(
    dds_entity_t participant,               // 参数1：参与者实体
    const char *name,                       // 参数2：主题名称
    bool allow_dcps,                        // 参数3：是否允许DCPS前缀
    struct ddsi_sertype **sertype,          // 参数4：指向序列化类型指针的指针
    const dds_qos_t *qos,                   // 参数5：指向QoS结构体的指针
    const dds_listener_t *listener,         // 参数6：指向监听器结构体的指针
    bool is_builtin)                        // 参数7：布尔值，表示是否为内置主题
{
  dds_return_t rc = 0;                      // 定义返回值变量并初始化为0
  dds_participant *pp;                      // 定义参与者指针
  dds_qos_t *new_qos = NULL;                // 定义新QoS指针并初始化为NULL
  dds_entity_t hdl;                         // 定义实体句柄
  struct ddsi_sertype *sertype_registered;  // 定义已注册的序列化类型指针

  if (sertype == NULL || *sertype == NULL || name == NULL || !is_valid_name(name))
    return DDS_RETCODE_BAD_PARAMETER;  // 检查参数有效性，如果无效则返回错误码
  if (!allow_dcps && strncmp(name, "DCPS", 4) == 0)
    return DDS_RETCODE_BAD_PARAMETER;  // 如果不允许DCPS前缀且名称以"DCPS"开头，则返回错误码

  {
    dds_entity *par_ent;
    if ((rc = dds_entity_pin(participant, &par_ent)) < 0)
      return rc;                  // 尝试锁定参与者实体，如果失败则返回错误码
    if (dds_entity_kind(par_ent) != DDS_KIND_PARTICIPANT) {
      dds_entity_unpin(par_ent);  // 如果实体类型不是参与者，则解锁实体并返回错误码
      return DDS_RETCODE_ILLEGAL_OPERATION;
    }
    pp = (struct dds_participant *)par_ent;  // 将实体转换为参与者指针
  }

#ifdef DDS_HAS_TYPE_DISCOVERY
  // 确保在类型信息存在时，顶层类型的最小和完整类型标识符都存在
  ddsi_typeinfo_t *type_info = ddsi_sertype_typeinfo(*sertype);  // 参数1：序列化类型指针
  if (type_info != NULL) {
    if (ddsi_typeid_is_none(ddsi_typeinfo_minimal_typeid(type_info)) ||
        ddsi_typeid_is_none(ddsi_typeinfo_complete_typeid(type_info)))
      rc = DDS_RETCODE_BAD_PARAMETER;
    ddsi_typeinfo_fini(type_info);
    ddsrt_free(type_info);
    if (rc < 0) {
      dds_entity_unpin(&pp->m_entity);
      return rc;
    }
  }
#endif

  new_qos = dds_create_qos();                                            // 创建新QoS
  if (qos) ddsi_xqos_mergein_missing(new_qos, qos, DDS_TOPIC_QOS_MASK);  // 合并缺失的QoS设置

  // 根据实体类型处理DDS规范的默认设置
  if ((rc = dds_ensure_valid_data_representation(new_qos, (*sertype)->allowed_data_representation,
                                                 true)) != 0)
    goto error;

  struct ddsi_domaingv *const gv = &pp->m_entity.m_domain->gv;            // 获取域全局变量
  if ((rc = ddsi_xqos_valid(&gv->logconfig, new_qos)) != DDS_RETCODE_OK)  // 检查新QoS是否有效
    goto error;

  // 检查安全设置是否允许创建主题
  if (!ddsi_omg_security_check_create_topic(&pp->m_entity.m_domain->gv, &pp->m_entity.m_guid, name,
                                            new_qos)) {
    rc = DDS_RETCODE_NOT_ALLOWED_BY_SECURITY;
    goto error;
  }

  // 检查是否允许创建主题；ktp返回时已锁定（受pp锁保护）
  // 因此我们可以确保它不会消失且其QoS不能更改
  GVTRACE("dds_create_topic_impl (pp %p " PGUIDFMT " sertype %p reg?%s refc %" PRIu32 " %s/%s)\n",
          (void *)pp, PGUID(pp->m_entity.m_guid), (void *)(*sertype),
          (ddsrt_atomic_ld32(&(*sertype)->flags_refc) & DDSI_SERTYPE_REGISTERED) ? "yes" : "no",
          ddsrt_atomic_ld32(&(*sertype)->flags_refc) & DDSI_SERTYPE_REFC_MASK, name,
          (*sertype)->type_name);
  ddsrt_mutex_lock(&pp->m_entity.m_mutex);  // 锁定参与者实体互斥锁

  struct dds_ktopic *ktp;
  // 查找并检查内核主题
  if ((rc = lookup_and_check_ktopic(&ktp, pp, name, new_qos)) != DDS_RETCODE_OK) {
    GVTRACE("dds_create_topic_impl: failed after compatibility check: %s\n", dds_strretcode(rc));
    ddsrt_mutex_unlock(&pp->m_entity.m_mutex);  // 解锁参与者实体互斥锁
    goto error;
  }

  // 如果内核主题尚不存在，则创建一个；否则引用现有的并删除不需要的“new_qos”
  if (ktp == NULL) {
    ktp = dds_alloc(sizeof(*ktp));     // 分配内存
    ktp->refc = 1;                     // 设置引用计数为1
    ktp->defer_set_qos = 0;            // 设置延迟设置QoS标志为0
    ktp->qos = new_qos;                // 设置QoS
    ktp->name = dds_string_dup(name);  // 复制主题名称
#ifdef DDS_HAS_TOPIC_DISCOVERY
    ktp->topic_guid_map =
        ddsrt_hh_new(1, ktopic_type_guid_hash, ktopic_type_guid_equal);  // 创建主题GUID映射
#endif
    ddsrt_avl_insert(&participant_ktopics_treedef, &pp->m_ktopics,
                     ktp);  // 将ktp插入参与者的内核主题树
    GVTRACE("create_and_lock_ktopic: ktp %p\n", (void *)ktp);
  } else {
    ktp->refc++;              // 增加引用计数
    dds_delete_qos(new_qos);  // 删除不需要的new_qos
  }
  /* sertype: 如果可能的话，重用之前注册过的类型，否则注册这个新类型 */
  {
    // 锁定 gv->sertypes_lock 互斥量
    ddsrt_mutex_lock(&gv->sertypes_lock);
    // 在已锁定的情况下查找已注册的 sertype
    if ((sertype_registered = ddsi_sertype_lookup_locked(gv, *sertype)) != NULL)
      // 如果找到了已注册的 sertype，则重用它并打印跟踪信息
      GVTRACE("dds_create_topic_impl: reuse sertype %p\n", (void *)sertype_registered);
    else {
      // 否则，注册新的 sertype 并打印跟踪信息
      GVTRACE("dds_create_topic_impl: register new sertype %p\n", (void *)(*sertype));
      ddsi_sertype_register_locked(gv, *sertype);
      sertype_registered = *sertype;
    }
    // 解锁 gv->sertypes_lock 互斥量
    ddsrt_mutex_unlock(&gv->sertypes_lock);
  }

#ifdef DDS_HAS_TYPE_DISCOVERY
  struct ddsi_type *type;
  // 检查类型引用是否有效
  if ((rc = ddsi_type_ref_local(gv, &type, sertype_registered, DDSI_TYPEID_KIND_MINIMAL)) !=
          DDS_RETCODE_OK ||
      ddsi_type_ref_local(gv, NULL, sertype_registered, DDSI_TYPEID_KIND_COMPLETE) !=
          DDS_RETCODE_OK) {
    if (rc == DDS_RETCODE_OK)
      // 如果类型引用无效，取消对该类型的引用
      ddsi_type_unref(gv, type);
    // 取消对 sertype 的引用
    ddsi_sertype_unref(*sertype);
    // 取消对 ktopic 的引用
    ktopic_unref(pp, ktp);
    // 解锁 pp->m_entity.m_mutex 互斥量
    ddsrt_mutex_unlock(&pp->m_entity.m_mutex);
    // 打印跟踪信息，表示类型无效
    GVTRACE("dds_create_topic_impl: invalid type\n");
    rc = DDS_RETCODE_BAD_PARAMETER;
    goto error_typeref;
  }
#endif

  /* 创建一个引用 ktopic 和 sertype_registered 的主题 */
  hdl = create_topic_pp_locked(pp, ktp, (sertype_registered->ops == &ddsi_sertype_ops_builtintopic),
                               name, sertype_registered, listener);
  // 取消对 *sertype 的引用
  ddsi_sertype_unref(*sertype);
  *sertype = sertype_registered;

  // 为发现过程注册主题类型
  const bool new_topic_def =
      register_topic_type_for_discovery(gv, pp, ktp, is_builtin, sertype_registered);
  // 解锁 pp->m_entity.m_mutex 互斥量
  ddsrt_mutex_unlock(&pp->m_entity.m_mutex);

  if (new_topic_def) {
    // 锁定 gv->new_topic_lock 互斥量
    ddsrt_mutex_lock(&gv->new_topic_lock);
    // 增加新主题版本
    gv->new_topic_version++;
    // 广播新主题条件变量
    ddsrt_cond_broadcast(&gv->new_topic_cond);
    // 解锁 gv->new_topic_lock 互斥量
    ddsrt_mutex_unlock(&gv->new_topic_lock);
  }

  // 取消对 pp->m_entity 的引用
  dds_entity_unpin(&pp->m_entity);
  // 打印跟踪信息，表示创建了新主题
  GVTRACE("dds_create_topic_impl: new topic %" PRId32 "\n", hdl);
  return hdl;

error:
  // 删除新的 QoS
  dds_delete_qos(new_qos);
#ifdef DDS_HAS_TYPE_DISCOVERY
error_typeref:
#endif
  // 取消对 pp->m_entity 的引用
  dds_entity_unpin(&pp->m_entity);
  return rc;
}
// 创建一个具有指定参与者、名称、序列化类型、QoS和监听器的主题实体
dds_entity_t dds_create_topic_sertype(dds_entity_t participant,        // 参与者实体
                                      const char *name,                // 主题名称
                                      struct ddsi_sertype **sertype,   // 序列化类型
                                      const dds_qos_t *qos,            // QoS设置
                                      const dds_listener_t *listener,  // 监听器
                                      const ddsi_plist_t *sedp_plist)  // SEDP属性列表
{
  (void)sedp_plist;
  return dds_create_topic_impl(participant, name, false, sertype, qos, listener, false);
}

// 使用给定的参与者、主题描述符、名称、QoS和监听器创建一个主题实体
dds_entity_t dds_create_topic(dds_entity_t participant,            // 参与者实体
                              const dds_topic_descriptor_t *desc,  // 主题描述符
                              const char *name,                    // 主题名称
                              const dds_qos_t *qos,                // QoS设置
                              const dds_listener_t *listener)      // 监听器
{
  dds_entity_t hdl;
  struct dds_entity *ppent;
  dds_return_t ret;

  // 检查输入参数是否有效
  if (desc == NULL || name == NULL) return DDS_RETCODE_BAD_PARAMETER;

  // 尝试获取参与者实体
  if ((ret = dds_entity_pin(participant, &ppent)) < 0) return ret;

  // 创建并合并QoS设置
  dds_qos_t *tpqos = dds_create_qos();
  if (qos) ddsi_xqos_mergein_missing(tpqos, qos, DDS_TOPIC_QOS_MASK);

  // 检查QoS中的数据表示与主题类型的可扩展性是否兼容
  uint32_t allowed_repr = desc->m_flagset & DDS_TOPIC_RESTRICT_DATA_REPRESENTATION
                              ? desc->restrict_data_representation
                              : DDS_DATA_REPRESENTATION_RESTRICT_DEFAULT;
  uint16_t min_xcdrv = dds_stream_minimum_xcdr_version(desc->m_ops);
  if (min_xcdrv == DDSI_RTPS_CDR_ENC_VERSION_2) allowed_repr &= ~DDS_DATA_REPRESENTATION_FLAG_XCDR1;
  if ((hdl = dds_ensure_valid_data_representation(tpqos, allowed_repr, true)) != 0)
    goto err_data_repr;

  assert(tpqos->present & DDSI_QP_DATA_REPRESENTATION && tpqos->data_representation.value.n > 0);
  dds_data_representation_id_t data_representation = tpqos->data_representation.value.ids[0];

  // 初始化默认序列化类型
  struct dds_sertype_default *st = ddsrt_malloc(sizeof(*st));
  if ((hdl = dds_sertype_default_init(ppent->m_domain, st, desc, min_xcdrv, data_representation)) <
      0) {
    ddsrt_free(st);
    goto err_st_init;
  }

  // 创建主题实体并处理错误
  struct ddsi_sertype *st_tmp = &st->c;
  hdl = dds_create_topic_impl(participant, name, false, &st_tmp, tpqos, listener, false);
  if (hdl < 0) ddsi_sertype_unref(st_tmp);
err_data_repr:
err_st_init:
  // 清理资源并返回句柄
  dds_delete_qos(tpqos);
  dds_entity_unpin(ppent);
  return hdl;
}
// 如果给定的子实体是一个与指定名称和类型信息匹配的主题，则返回该主题，否则返回NULL
static dds_topic *pin_if_matching_topic(dds_entity *const e_pp_child,      // 子实体
                                        const char *name,                  // 主题名称
                                        const ddsi_typeinfo_t *type_info)  // 类型信息
{
  // 当我们持有pp->m_entity.m_mutex时，e_pp_child不会消失，因此我们可以在尝试固定它之前跳过非主题实体。
  // 这使得这种方式比dds_topic_pin更便宜。
  if (dds_entity_kind(e_pp_child) != DDS_KIND_TOPIC) return NULL;

  // 固定以确保子实体未关闭（即仍可访问）
  struct dds_entity *x;
  if (dds_entity_pin(e_pp_child->m_hdllink.hdl, &x) != DDS_RETCODE_OK) return NULL;

  struct dds_topic *const tp = (struct dds_topic *)e_pp_child;
  // 检查主题名称是否匹配
  if (!strcmp(tp->m_ktopic->name, name)) {
#ifdef DDS_HAS_TYPE_DISCOVERY
    /* 如果没有提供类型信息，则返回具有指定名称的任何（第一个）主题。
       如果设置了类型信息，则主题的类型信息应与其匹配 */
    ddsi_typeinfo_t *topic_type_info = ddsi_sertype_typeinfo(tp->m_stype);
    bool ti_match = !ddsi_typeinfo_present(type_info) ||
                    (ddsi_typeinfo_present(topic_type_info) &&
                     ddsi_typeinfo_equal(topic_type_info, type_info, DDSI_TYPE_IGNORE_DEPS));
    ddsi_typeinfo_fini(topic_type_info);
    ddsrt_free(topic_type_info);
    if (ti_match) return tp;
#else
    (void)type_info;
    return tp;
#endif
  }

  // 如果主题不匹配，则取消固定
  dds_entity_unpin(x);
  return NULL;
}
// 以下代码是用C语言编写的
static dds_entity_t find_local_topic_pp(dds_participant *pp,
                                        const char *name,
                                        const ddsi_typeinfo_t *type_info,
                                        dds_participant *pp_topic) {
  // 入口条件：
  // - pp 和 pp_topic 已固定，没有锁定
  // - pp 和 pp_topic 可能相同，也可能不同
  // - pp_topic 可能包含匹配的 ktopic（无论 pp 和 pp_topic 是否相同）
  // - 并行创建、删除、设置主题和 ktopics 的 qos 可能正在进行
  //
  // dds_create_topic 在找到 sertype 和 qos 后会做 "正确的事情"，所以如果我们可以在 pp 中找到
  // 匹配的主题，我们克隆它的 qos（以防止并发调用 set_qos）并
  // 增加其 sertype 的引用计数。然后，我们可以随意调用 dds_create_topic_sertype。
  struct dds_topic *tp = NULL;
  ddsrt_avl_iter_t it;

  ddsrt_mutex_lock(&pp->m_entity.m_mutex);
  for (dds_entity *e_pp_child =
           ddsrt_avl_iter_first(&dds_entity_children_td, &pp->m_entity.m_children, &it);
       e_pp_child != NULL; e_pp_child = ddsrt_avl_iter_next(&it)) {
    // 固定主题有两个目的：检查其句柄是否已关闭
    // 并在我们解锁参与者后保持其活动状态。
    tp = pin_if_matching_topic(e_pp_child, name, type_info);
    if (tp != NULL) break;
  }

  if (tp == NULL) {
    ddsrt_mutex_unlock(&pp->m_entity.m_mutex);
    return 0;
  } else {
    // QoS 更改受参与者锁保护（请参阅 dds_set_qos_locked_impl）
    // 我们当前为主题的参与者持有该锁
    dds_qos_t *qos = ddsi_xqos_dup(tp->m_ktopic->qos);
    ddsrt_mutex_unlock(&pp->m_entity.m_mutex);

    // 固定 tp 后，其 sertype 将保持注册状态。dds_create_topic_sertype
    // 接受传递给它的引用并返回实际使用的（未计数的）指针
    // 因此，如果我们增加 tp->m_stype 的引用计数，我们
    // 可以将其传递到 dds_create_topic_sertype 并确保它使用它，只有
    // 在失败时才需要删除引用。（例如，当应用程序同时调用 dds_delete on pp_topic 时，可能会失败。）
    struct ddsi_sertype *sertype = ddsi_sertype_ref(tp->m_stype);
    const dds_entity_t hdl =
        dds_create_topic_sertype(pp_topic->m_entity.m_hdllink.hdl, name, &sertype, qos, NULL, NULL);
    if (hdl < 0) ddsi_sertype_unref(sertype);
    dds_delete_qos(qos);

#ifndef NDEBUG
    if (hdl > 0) {
      dds_topic *new_topic;
      // 同时在随机句柄上调用 dds_delete 可能导致固定新创建的主题失败
      if (dds_topic_pin(hdl, &new_topic) == DDS_RETCODE_OK) {
        if (pp == pp_topic)
          assert(tp->m_ktopic == new_topic->m_ktopic);
        else
          assert(tp->m_ktopic != new_topic->m_ktopic);
        assert(tp->m_stype == new_topic->m_stype);
        dds_topic_unpin(new_topic);
      }
    }

    // 必须在取消固定 tp 之前，否则（1）如果 dds_create_topic_sertype 失败，
    // sertype 可能已从表中删除，且（2）当我们到达此处时，tp 可能已被删除，
    // 因此它将不再一定包含在 sertype 的引用计数中。
    {
      // 定义一个指向 ddsi_domaingv 结构的常量指针 gv，并将其初始化为 pp_topic 的 m_entity 的
      // m_domain 的 gv 成员的地址
      struct ddsi_domaingv *const gv = &pp_topic->m_entity.m_domain->gv;

      // 锁定 gv 的 sertypes_lock 互斥锁，以确保对 sertypes 的访问是线程安全的
      ddsrt_mutex_lock(&gv->sertypes_lock);

      // 使用断言检查 gv 的 sertypes 中是否存在 sertype，如果不存在，则程序会终止
      assert(ddsrt_hh_lookup(gv->sertypes, sertype) == sertype);

      // 解锁 gv 的 sertypes_lock 互斥锁，允许其他线程访问 sertypes
      ddsrt_mutex_unlock(&gv->sertypes_lock);

      // 从 sertype 的 flags_refc 原子变量中加载一个 uint32_t 类型的值
      const uint32_t sertype_flags_refc = ddsrt_atomic_ld32(&sertype->flags_refc);

      // 使用断言检查 sertype 是否已注册，如果未注册，则程序会终止
      assert(sertype_flags_refc & DDSI_SERTYPE_REGISTERED);

      // 使用断言检查 sertype 的引用计数是否大于等于 1（当 hdl < 0 时）或 2（当 hdl >= 0
      // 时），如果不满足条件，则程序会终止
      assert((sertype_flags_refc & DDSI_SERTYPE_REFC_MASK) >= (hdl < 0 ? 1u : 2u));
    }

#endif

    dds_topic_unpin(tp);
    return hdl;
  }
}
// 定义一个静态函数 find_local_topic_impl，用于在指定范围内查找本地主题
static dds_return_t find_local_topic_impl(dds_find_scope_t scope,
                                          dds_participant *pp_topic,
                                          const char *name,
                                          const ddsi_typeinfo_t *type_info) {
  // 函数入口：pp_topic 已经被锁定，没有持有任何锁

  // 定义两个 dds_entity 指针 e_pp 和 e_domain_child
  dds_entity *e_pp, *e_domain_child;
  // 定义一个 dds_instance_handle_t 类型的变量 last_iid，并初始化为 0
  dds_instance_handle_t last_iid = 0;

  // 如果查找范围是参与者范围，则调用 find_local_topic_pp 函数进行查找
  if (scope == DDS_FIND_SCOPE_PARTICIPANT)
    return find_local_topic_pp(pp_topic, name, type_info, pp_topic);
  // 获取参与者所属的域
  dds_domain *dom = pp_topic->m_entity.m_domain;
  // 对域实体的互斥锁进行加锁
  ddsrt_mutex_lock(&dom->m_entity.m_mutex);
  // 使用 while 循环遍历域的子实体
  while ((e_domain_child = ddsrt_avl_lookup_succ(&dds_entity_children_td, &dom->m_entity.m_children,
                                                 &last_iid)) != NULL) {
    // 更新 last_iid 的值
    last_iid = e_domain_child->m_iid;
    // 如果当前子实体不是参与者类型，则跳过本次循环
    if (dds_entity_kind(e_domain_child) != DDS_KIND_PARTICIPANT) continue;

    // 尝试锁定当前子实体，如果失败则跳过本次循环
    if (dds_entity_pin(e_domain_child->m_hdllink.hdl, &e_pp) != DDS_RETCODE_OK) continue;

    // 将 e_domain_child 转换为 dds_participant 类型的指针 pp
    dds_participant *pp = (dds_participant *)e_domain_child;
    // 解锁域实体的互斥锁
    ddsrt_mutex_unlock(&dom->m_entity.m_mutex);
    // 调用 find_local_topic_pp 函数查找本地主题
    dds_entity_t hdl = find_local_topic_pp(pp, name, type_info, pp_topic);
    // 解锁当前子实体
    dds_entity_unpin(e_pp);
    // 如果找到了本地主题，则返回其句柄
    if (hdl != 0) return hdl;
    // 再次加锁域实体的互斥锁
    ddsrt_mutex_lock(&dom->m_entity.m_mutex);
  }
  // 解锁域实体的互斥锁
  ddsrt_mutex_unlock(&dom->m_entity.m_mutex);
  // 如果没有找到本地主题，则返回 0
  return 0;
}

// 如果定义了 DDS_HAS_TOPIC_DISCOVERY
#ifdef DDS_HAS_TOPIC_DISCOVERY

// 定义一个静态函数 find_remote_topic_impl，用于在指定范围内查找远程主题
static dds_entity_t find_remote_topic_impl(dds_participant *pp_topic,
                                           const char *name,
                                           const dds_typeinfo_t *type_info,
                                           dds_duration_t timeout) {
  // 函数入口：pp_topic 已经被锁定，没有持有任何锁

  // 定义一个 dds_entity_t 类型的变量 ret
  dds_entity_t ret;
  // 定义一个 ddsi_topic_definition 指针 tpd
  struct ddsi_topic_definition *tpd;
  // 获取参与者所属域的全局变量指针 gv
  struct ddsi_domaingv *gv = &pp_topic->m_entity.m_domain->gv;
  // 获取 type_info 对应的类型标识符 type_id
  const struct ddsi_typeid *type_id = ddsi_typeinfo_complete_typeid(type_info);
  // 定义一个 ddsi_type 指针 resolved_type，并初始化为 NULL
  struct ddsi_type *resolved_type = NULL;

  // 调用 ddsi_lookup_topic_definition 函数查找主题定义
  if ((ret = ddsi_lookup_topic_definition(gv, name, type_id, &tpd)) != DDS_RETCODE_OK) return ret;
  // 如果没有找到主题定义，则返回 DDS_RETCODE_OK
  if (tpd == NULL) return DDS_RETCODE_OK;

  // 调用 ddsi_wait_for_type_resolved 函数等待类型解析完成
  if ((ret = ddsi_wait_for_type_resolved(gv, type_id, timeout, &resolved_type,
                                         DDSI_TYPE_INCLUDE_DEPS, DDSI_TYPE_SEND_REQUEST)) !=
      DDS_RETCODE_OK)
    return ret;
  // 断言：tpd 的完整类型与 resolved_type 相同
  assert(!ddsi_type_compare(tpd->type_pair->complete, resolved_type));
  // 断言：tpd 的完整类型已经解析完成，包括依赖项
  assert(ddsi_type_resolved(gv, tpd->type_pair->complete, DDSI_TYPE_INCLUDE_DEPS));

  // 分配一个 dds_topic_descriptor_t 类型的指针 desc
  dds_topic_descriptor_t *desc = ddsrt_malloc(sizeof(*desc));
  // 调用 ddsi_topic_descriptor_from_type 函数从类型中获取主题描述符
  if ((ret = ddsi_topic_descriptor_from_type(gv, desc, tpd->type_pair->complete))) goto err_desc;
  // 调用 dds_create_topic 函数创建主题
  ret = dds_create_topic(pp_topic->m_entity.m_hdllink.hdl, desc, name, tpd->xqos, NULL);
  // 调用 ddsi_topic_descriptor_fini 函数释放主题描述符资源
  ddsi_topic_descriptor_fini(desc);
  // 如果 resolved_type 不为 NULL，则释放其资源
  if (resolved_type) ddsi_type_unref(gv, resolved_type);
// 错误处理标签：释放 desc 指针分配的内存
err_desc:
  ddsrt_free(desc);
  // 返回结果
  return ret;
}

#endif /* DDS_HAS_TOPIC_DISCOVERY */
// 定义一个静态函数 dds_find_topic_impl，用于在指定范围内查找主题
static dds_entity_t dds_find_topic_impl(dds_find_scope_t scope,
                                        dds_entity_t participant,
                                        const char *name,
                                        const dds_typeinfo_t *type_info,
                                        dds_duration_t timeout) {
  // 定义一个 dds_entity_t 类型的变量 hdl
  dds_entity_t hdl;
  // 定义一个 dds_return_t 类型的变量 ret
  dds_return_t ret;
  // 定义一个 dds_entity 指针 e

#ifndef DDS_HAS_TOPIC_DISCOVERY
  // 如果没有定义 DDS_HAS_TOPIC_DISCOVERY，并且查找范围是全局范围，则返回错误参数
  if (scope == DDS_FIND_SCOPE_GLOBAL) return DDS_RETCODE_BAD_PARAMETER;
#endif

  // 如果 name 为空或者不是有效的名称，则返回错误参数
  if (name == NULL || !is_valid_name(name)) return DDS_RETCODE_BAD_PARAMETER;
  // 尝试锁定参与者实体，如果失败则返回错误码
  if ((ret = dds_entity_pin(participant, &e)) < 0) return ret;
  // 如果实体类型不是参与者类型，则解锁实体并返回错误参数
  if (e->m_kind != DDS_KIND_PARTICIPANT) {
    dds_entity_unpin(e);
    return DDS_RETCODE_BAD_PARAMETER;
  }
  // 将 e 转换为 dds_participant 类型的指针 pp_topic
  dds_participant *pp_topic = (dds_participant *)e;
  // 获取实体所属域的全局变量指针 gv
  struct ddsi_domaingv *gv = &e->m_domain->gv;
  // 获取当前时间 tnow
  const dds_time_t tnow = dds_time();
  // 计算绝对超时时间 abstimeout
  const dds_time_t abstimeout = (DDS_INFINITY - timeout <= tnow) ? DDS_NEVER : (tnow + timeout);
  // 使用 do-while 循环进行查找操作
  do {
    // 加锁新主题互斥锁
    ddsrt_mutex_lock(&gv->new_topic_lock);
    // 获取新主题版本号 tv
    uint32_t tv = gv->new_topic_version;
    // 解锁新主题互斥锁
    ddsrt_mutex_unlock(&gv->new_topic_lock);
    // 调用 find_local_topic_impl 函数查找本地主题
    if ((hdl = find_local_topic_impl(scope, pp_topic, name, type_info)) == 0 &&
        scope == DDS_FIND_SCOPE_GLOBAL) {
#ifdef DDS_HAS_TOPIC_DISCOVERY
      // 如果定义了 DDS_HAS_TOPIC_DISCOVERY，则调用 find_remote_topic_impl 函数查找远程主题
      hdl = find_remote_topic_impl(pp_topic, name, type_info, timeout);
#endif
    }
    // 如果没有找到主题，并且超时时间大于 0，则等待新主题出现或者超时
    if (hdl == 0 && timeout > 0) {
      // 加锁新主题互斥锁
      ddsrt_mutex_lock(&gv->new_topic_lock);
      // 使用 while 循环等待新主题出现或者超时
      while (hdl != DDS_RETCODE_TIMEOUT && gv->new_topic_version == tv) {
        // 如果条件变量等待超时，则设置 hdl 为 DDS_RETCODE_TIMEOUT
        if (!ddsrt_cond_waituntil(&gv->new_topic_cond, &gv->new_topic_lock, abstimeout))
          hdl = DDS_RETCODE_TIMEOUT;
      }
      // 解锁新主题互斥锁
      ddsrt_mutex_unlock(&gv->new_topic_lock);
    }
    // 当 hdl 为 0 且超时时间大于 0 时，继续循环查找
  } while (hdl == 0 && timeout > 0);
  // 解锁实体
  dds_entity_unpin(e);
  // 返回查找到的主题句柄
  return hdl;
}

// dds_find_topic 函数
dds_entity_t dds_find_topic(dds_find_scope_t scope,           // 查找范围
                            dds_entity_t participant,         // 参与者实体
                            const char *name,                 // 主题名称
                            const dds_typeinfo_t *type_info,  // 类型信息
                            dds_duration_t timeout            // 超时时间
) {
#ifdef DDS_HAS_TOPIC_DISCOVERY
  if (type_info && !ddsi_typeinfo_valid(type_info)) return DDS_RETCODE_BAD_PARAMETER;
#else
  if (type_info != NULL) return DDS_RETCODE_BAD_PARAMETER;
#endif
  return dds_find_topic_impl(scope, participant, name, type_info, timeout);
}

// dds_find_topic_scoped 函数
dds_entity_t dds_find_topic_scoped(dds_find_scope_t scope,    // 查找范围
                                   dds_entity_t participant,  // 参与者实体
                                   const char *name,          // 主题名称
                                   dds_duration_t timeout     // 超时时间
) {
  return dds_find_topic_impl(scope, participant, name, NULL, timeout);
}

// dds_set_topic_filter_extended 函数
dds_return_t dds_set_topic_filter_extended(dds_entity_t topic,                    // 主题实体
                                           const struct dds_topic_filter *filter  // 过滤器结构
) {
  struct dds_topic_filter f;
  dds_topic *t;
  dds_return_t rc;

  if (filter == NULL) return DDS_RETCODE_BAD_PARAMETER;

  f = *filter;

  {
    bool valid = false;
    switch (f.mode) {
      case DDS_TOPIC_FILTER_NONE:
        // 如果 mode = NONE，将 function 和 argument 视为输入时不关心的内容
        // 但在内部表示中将它们设为 null 指针
        f.f.sample = 0;
        f.arg = NULL;
        valid = true;
        break;
      case DDS_TOPIC_FILTER_SAMPLE:
        f.arg = NULL;
        /* falls through */
      case DDS_TOPIC_FILTER_SAMPLEINFO_ARG:
      case DDS_TOPIC_FILTER_SAMPLE_ARG:
      case DDS_TOPIC_FILTER_SAMPLE_SAMPLEINFO_ARG:
        // 可以安全地使用任何函数指针
        valid = (filter->f.sample != 0);
        break;
    }
    if (!valid) {
      // 只有在调用者传递了错误的 mode 参数时才可能发生
      return DDS_RETCODE_BAD_PARAMETER;
    }
  }

  if ((rc = dds_topic_lock(topic, &t)) != DDS_RETCODE_OK) return rc;
  t->m_filter = f;
  dds_topic_unlock(t);
  return DDS_RETCODE_OK;
}
// 设置主题过滤器和参数
dds_return_t dds_set_topic_filter_and_arg(dds_entity_t topic,
                                          dds_topic_filter_arg_fn filter,
                                          void *arg) {
  // 定义一个过滤器结构体并初始化
  struct dds_topic_filter f = {.mode = filter ? DDS_TOPIC_FILTER_SAMPLE_ARG : DDS_TOPIC_FILTER_NONE,
                               .arg = arg,
                               .f = {.sample_arg = filter}};
  // 调用扩展设置主题过滤器函数
  return dds_set_topic_filter_extended(topic, &f);
}

// 获取扩展主题过滤器
dds_return_t dds_get_topic_filter_extended(dds_entity_t topic, struct dds_topic_filter *filter) {
  dds_return_t rc;
  dds_topic *t;
  // 检查过滤器是否为空
  if (filter == NULL) return DDS_RETCODE_BAD_PARAMETER;
  // 锁定主题
  if ((rc = dds_topic_lock(topic, &t)) != DDS_RETCODE_OK) return rc;
  // 获取过滤器
  *filter = t->m_filter;
  // 解锁主题
  dds_topic_unlock(t);
  return rc;
}

// 获取主题过滤器和参数
dds_return_t dds_get_topic_filter_and_arg(dds_entity_t topic,
                                          dds_topic_filter_arg_fn *fn,
                                          void **arg) {
  struct dds_topic_filter f;
  dds_return_t rc;
  // 获取扩展主题过滤器
  if ((rc = dds_get_topic_filter_extended(topic, &f)) != DDS_RETCODE_OK) return rc;
  // 根据过滤器模式处理
  switch (f.mode) {
    case DDS_TOPIC_FILTER_NONE:
      assert(f.f.sample_arg == 0);
      /* fall through */
    case DDS_TOPIC_FILTER_SAMPLE_ARG:
      if (fn) *fn = f.f.sample_arg;
      if (arg) *arg = f.arg;
      break;
    case DDS_TOPIC_FILTER_SAMPLE:
    case DDS_TOPIC_FILTER_SAMPLEINFO_ARG:
    case DDS_TOPIC_FILTER_SAMPLE_SAMPLEINFO_ARG:
      rc = DDS_RETCODE_PRECONDITION_NOT_MET;
      break;
  }
  return rc;
}

// 获取主题名称
dds_return_t dds_get_name(dds_entity_t topic, char *name, size_t size) {
  dds_topic *t;
  dds_return_t ret;
  // 检查参数是否有效
  if (size <= 0 || name == NULL) return DDS_RETCODE_BAD_PARAMETER;
  name[0] = '\0';

  const char *bname;
  // 获取内置主题名称和类型名
  if (dds__get_builtin_topic_name_typename(topic, &bname, NULL) == 0)
    ret = (dds_return_t)ddsrt_strlcpy(name, bname, size);
  else if ((ret = dds_topic_pin(topic, &t)) == DDS_RETCODE_OK) {
    ret = (dds_return_t)ddsrt_strlcpy(name, t->m_name, size);
    dds_topic_unpin(t);
  }
  return ret;
}

// 获取主题类型名称
dds_return_t dds_get_type_name(dds_entity_t topic, char *name, size_t size) {
  dds_topic *t;
  dds_return_t ret;
  // 检查参数是否有效
  if (size <= 0 || name == NULL) return DDS_RETCODE_BAD_PARAMETER;
  name[0] = '\0';

  const char *bname;
  // 获取内置主题名称和类型名
  if (dds__get_builtin_topic_name_typename(topic, NULL, &bname) == 0)
    ret = (dds_return_t)ddsrt_strlcpy(name, bname, size);
  else if ((ret = dds_topic_pin(topic, &t)) == DDS_RETCODE_OK) {
    ret = (dds_return_t)ddsrt_strlcpy(name, t->m_stype->type_name, size);
    dds_topic_unpin(t);
  }
  return ret;
}
// 获取主题的状态信息
DDS_GET_STATUS(topic, inconsistent_topic, INCONSISTENT_TOPIC, total_count_change)

// 如果定义了DDS_HAS_TOPIC_DISCOVERY
#ifdef DDS_HAS_TOPIC_DISCOVERY

// 创建主题描述符函数
dds_return_t dds_create_topic_descriptor(
    dds_find_scope_t scope,              // 查找范围（全局或本地域）
    dds_entity_t participant,            // 参与者实体
    const dds_typeinfo_t *type_info,     // 类型信息指针
    dds_duration_t timeout,              // 超时时间
    dds_topic_descriptor_t **descriptor  // 主题描述符指针的指针
) {
  dds_return_t ret;

  // 检查参数是否有效
  if (scope != DDS_FIND_SCOPE_GLOBAL && scope != DDS_FIND_SCOPE_LOCAL_DOMAIN)
    return DDS_RETCODE_BAD_PARAMETER;
  if (type_info == NULL || descriptor == NULL) return DDS_RETCODE_BAD_PARAMETER;

  // 分配内存给描述符
  *descriptor = dds_alloc(sizeof(**descriptor));
  if (*descriptor == NULL) return DDS_RETCODE_OUT_OF_RESOURCES;

  dds_entity *e;
  // 尝试获取参与者实体
  if ((ret = dds_entity_pin(participant, &e)) < 0) goto err_pin;
  // 检查实体类型是否为参与者
  if (e->m_kind != DDS_KIND_PARTICIPANT) {
    ret = DDS_RETCODE_BAD_PARAMETER;
    goto err;
  }

  struct ddsi_domaingv *gv = &e->m_domain->gv;
  struct ddsi_type *type;
  // 等待类型解析完成
  if ((ret = ddsi_wait_for_type_resolved(
           gv, ddsi_typeinfo_complete_typeid(type_info), timeout, &type, DDSI_TYPE_INCLUDE_DEPS,
           scope == DDS_FIND_SCOPE_GLOBAL ? DDSI_TYPE_SEND_REQUEST : DDSI_TYPE_NO_REQUEST)))
    goto err;
  assert(type && ddsi_type_resolved(gv, type, DDSI_TYPE_INCLUDE_DEPS));
  // 从类型信息创建主题描述符
  ret = ddsi_topic_descriptor_from_type(gv, *descriptor, type);
  ddsi_type_unref(gv, type);

err:
  dds_entity_unpin(e);
err_pin:
  if (ret != DDS_RETCODE_OK) ddsrt_free(*descriptor);
  return ret;
}

// 删除主题描述符的函数
dds_return_t dds_delete_topic_descriptor(dds_topic_descriptor_t *descriptor) {
  // 检查参数是否有效
  if (!descriptor) return DDS_RETCODE_BAD_PARAMETER;
  // 结束主题描述符的使用
  ddsi_topic_descriptor_fini(descriptor);
  // 释放主题描述符所占用的内存
  dds_free(descriptor);
  // 返回操作成功的代码
  return DDS_RETCODE_OK;
}

#else

// 如果未定义DDS_HAS_TOPIC_DISCOVERY，返回不支持的错误代码
dds_return_t dds_create_topic_descriptor(dds_find_scope_t scope,
                                         dds_entity_t participant,
                                         const dds_typeinfo_t *type_info,
                                         dds_duration_t timeout,
                                         dds_topic_descriptor_t **descriptor) {
  (void)scope;
  (void)participant;
  (void)type_info;
  (void)timeout;
  (void)descriptor;
  return DDS_RETCODE_UNSUPPORTED;
}

dds_return_t dds_delete_topic_descriptor(dds_topic_descriptor_t *descriptor) {
  (void)descriptor;
  return DDS_RETCODE_UNSUPPORTED;
}

#endif /* DDS_HAS_TOPIC_DISCOVERY */

// 从主题描述符中获取cdr流描述信息的函数
void dds_cdrstream_desc_from_topic_desc(struct dds_cdrstream_desc *desc,  // cdr流描述指针
                                        const dds_topic_descriptor_t *topic_desc  // 主题描述符指针
) {
  // 将desc内存区域设置为0
  memset(desc, 0, sizeof(*desc));
  // 设置desc的size字段为topic_desc的m_size字段值
  desc->size = topic_desc->m_size;
  // 设置desc的align字段为topic_desc的m_align字段值
  desc->align = topic_desc->m_align;
  // 设置desc的flagset字段为topic_desc的m_flagset字段值
  desc->flagset = topic_desc->m_flagset;
  // 计算操作数，并设置desc的ops.nops字段
  desc->ops.nops = dds_stream_countops(topic_desc->m_ops, topic_desc->m_nkeys, topic_desc->m_keys);
  // 为desc的ops.ops字段分配内存
  desc->ops.ops = dds_alloc(desc->ops.nops * sizeof(*desc->ops.ops));
  // 将topic_desc的m_ops字段内容复制到desc的ops.ops字段
  memcpy(desc->ops.ops, topic_desc->m_ops, desc->ops.nops * sizeof(*desc->ops.ops));
  // 设置desc的keys.nkeys字段为topic_desc的m_nkeys字段值
  desc->keys.nkeys = topic_desc->m_nkeys;
  // 如果有键值
  if (desc->keys.nkeys > 0) {
    // 为desc的keys.keys字段分配内存
    desc->keys.keys = dds_alloc(desc->keys.nkeys * sizeof(*desc->keys.keys));
    // 遍历键值，设置desc的keys.keys字段内容
    for (uint32_t i = 0; i < desc->keys.nkeys; i++) {
      desc->keys.keys[i].ops_offs = topic_desc->m_keys[i].m_offset;
      desc->keys.keys[i].idx = topic_desc->m_keys[i].m_idx;
    }
  }
}
