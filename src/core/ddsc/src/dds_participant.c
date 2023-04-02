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

#include "dds/ddsi/ddsi_domaingv.h"
#include "dds/ddsi/ddsi_entity.h"
#include "dds/ddsi/ddsi_entity_index.h"
#include "dds/ddsi/ddsi_participant.h"
#include "dds/ddsi/ddsi_plist.h"
#include "dds/ddsi/ddsi_proxy_endpoint.h"
#include "dds/ddsi/ddsi_thread.h"
#include "dds/ddsrt/cdtors.h"
#include "dds/ddsrt/environ.h"
#include "dds/version.h"
#include "dds__builtin.h"
#include "dds__domain.h"
#include "dds__init.h"
#include "dds__participant.h"
#include "dds__qos.h"

/** @file
 *  Cyclone DDS相关的C代码示例，包括实体锁定/解锁、参与者状态验证和删除等功能。
 */

// 声明实体锁定/解锁函数
DECL_ENTITY_LOCK_UNLOCK(dds_participant)

// 定义参与者状态掩码
#define DDS_PARTICIPANT_STATUS_MASK (0u)

/**
 * @brief 比较两个主题名称的字符串
 *
 * @param[in] a 第一个主题名称
 * @param[in] b 第二个主题名称
 * @return 字符串比较结果
 */
static int cmp_ktopic_name(const void *a, const void *b) { return strcmp(a, b); }

// 初始化参与者主题树定义
const ddsrt_avl_treedef_t participant_ktopics_treedef =
    DDSRT_AVL_TREEDEF_INITIALIZER_INDKEY(offsetof(struct dds_ktopic, pp_ktopics_avlnode),
                                         offsetof(struct dds_ktopic, name),
                                         cmp_ktopic_name,
                                         0);

/**
 * @brief 验证参与者状态掩码是否有效
 *
 * @param[in] mask 参与者状态掩码
 * @return 验证结果，如果有效返回DDS_RETCODE_OK，否则返回DDS_RETCODE_BAD_PARAMETER
 */
static dds_return_t dds_participant_status_validate(uint32_t mask) {
  return (mask & ~DDS_PARTICIPANT_STATUS_MASK) ? DDS_RETCODE_BAD_PARAMETER : DDS_RETCODE_OK;
}

// 声明参与者删除函数
static dds_return_t dds_participant_delete(dds_entity *e) ddsrt_nonnull_all;

/**
 * @brief 删除参与者实体 (Delete a participant entity)
 *
 * @param[in] e 指向要删除的参与者实体的指针 (Pointer to the participant entity to be deleted)
 *
 * @return 返回操作结果，成功时返回 DDS_RETCODE_OK (Returns the operation result, returns
 * DDS_RETCODE_OK on success)
 */
static dds_return_t dds_participant_delete(dds_entity *e) {
  // 定义返回值变量 (Define return value variable)
  dds_return_t ret;

  // 断言实体类型为参与者 (Assert that the entity kind is participant)
  assert(dds_entity_kind(e) == DDS_KIND_PARTICIPANT);

  // ktopics 和 topics 是子节点，因此在我们到达这里之前，它们必须已经全部被删除
  // (ktopics & topics are children and therefore must all have been deleted by the time we get
  // here)
  assert(ddsrt_avl_is_empty(&((struct dds_participant *)e)->m_ktopics));

  // 唤醒线程状态 (Wake up thread state)
  ddsi_thread_state_awake(ddsi_lookup_thread_state(), &e->m_domain->gv);

  // 尝试删除参与者，如果失败则记录错误日志
  // (Attempt to delete participant, if failed then log an error message)
  if ((ret = ddsi_delete_participant(&e->m_domain->gv, &e->m_guid)) < 0)
    DDS_CERROR(&e->m_domain->gv.logconfig, "dds_participant_delete: internal error %" PRId32 "\n",
               ret);

  // 线程进入休眠状态 (Put the thread to sleep)
  ddsi_thread_state_asleep(ddsi_lookup_thread_state());

  // 返回操作成功的结果代码 (Return the success result code)
  return DDS_RETCODE_OK;
}

/**
 * @brief 设置参与者的QoS（Quality of Service，服务质量）属性。
 *
 * @param[in] e       指向dds_entity结构体的指针，表示要设置QoS属性的参与者实体。
 * @param[in] qos     指向dds_qos_t结构体的指针，包含要设置的QoS属性值。
 * @param[in] enabled
 * 布尔值，表示是否启用QoS设置。如果为true，则应用QoS设置；如果为false，则不进行任何操作。
 *
 * @return 返回dds_return_t类型的结果代码。如果成功设置QoS属性，则返回DDS_RETCODE_OK。
 */
static dds_return_t dds_participant_qos_set(dds_entity *e, const dds_qos_t *qos, bool enabled) {
  // 注意：e->m_qos仍然是旧的QoS设置，以允许在此处失败
  if (enabled) {
    // 定义一个ddsi_participant类型的指针pp
    struct ddsi_participant *pp;

    // 唤醒当前线程，并获取实体所在域的全局变量
    ddsi_thread_state_awake(ddsi_lookup_thread_state(), &e->m_domain->gv);

    // 查找参与者实体的GUID，并将其赋值给pp
    if ((pp = ddsi_entidx_lookup_participant_guid(e->m_domain->gv.entity_index, &e->m_guid)) !=
        NULL) {
      // 定义并初始化一个ddsi_plist_t类型的变量plist
      ddsi_plist_t plist;
      ddsi_plist_init_empty(&plist);

      // 将qos的present属性赋值给plist.qos.present和plist.qos.aliased
      plist.qos.present = plist.qos.aliased = qos->present;

      // 将qos结构体的内容复制到plist.qos中
      plist.qos = *qos;

      // 更新参与者实体的QoS设置
      ddsi_update_participant_plist(pp, &plist);
    }

    // 使当前线程进入休眠状态
    ddsi_thread_state_asleep(ddsi_lookup_thread_state());
  }

  // 返回操作成功的结果代码
  return DDS_RETCODE_OK;
}

/** @file
 *  Cyclone DDS 相关的 C 代码，包括实体派生器和参与者创建函数。
 */

/**
 * 实体派生器结构体，用于定义参与者实体的操作。
 */
const struct dds_entity_deriver dds_entity_deriver_participant = {
    .interrupt = dds_entity_deriver_dummy_interrupt,                   ///< 中断操作
    .close = dds_entity_deriver_dummy_close,                           ///< 关闭操作
    .delete = dds_participant_delete,                                  ///< 删除操作
    .set_qos = dds_participant_qos_set,                                ///< 设置 QoS 操作
    .validate_status = dds_participant_status_validate,                ///< 验证状态操作
    .create_statistics = dds_entity_deriver_dummy_create_statistics,   ///< 创建统计信息操作
    .refresh_statistics = dds_entity_deriver_dummy_refresh_statistics  ///< 刷新统计信息操作
};

/**
 * 创建参与者实体。
 *
 * @param[in] domain    域 ID
 * @param[in] qos       参与者的 QoS 设置
 * @param[in] listener  参与者的监听器
 * @return 成功时返回参与者实体的句柄，失败时返回错误代码
 */
dds_entity_t dds_create_participant(const dds_domainid_t domain,
                                    const dds_qos_t *qos,
                                    const dds_listener_t *listener) {
  dds_domain *dom;
  dds_entity_t ret;
  ddsi_guid_t guid;
  dds_participant *pp;
  ddsi_plist_t plist;
  dds_qos_t *new_qos = NULL;
  const char *config = "";

  /* 确保 DDS 实例已初始化。 */
  if ((ret = dds_init()) < 0) goto err_dds_init;

  (void)ddsrt_getenv("CYCLONEDDS_URI", &config);

  /* 创建域。 */
  if ((ret = dds_domain_create_internal(&dom, domain, true, config)) < 0) goto err_domain_create;

  /* 创建新的 QoS 设置。 */
  new_qos = dds_create_qos();
  if (qos != NULL) ddsi_xqos_mergein_missing(new_qos, qos, DDS_PARTICIPANT_QOS_MASK);
  ddsi_xqos_mergein_missing(new_qos, &dom->gv.default_local_xqos_pp, ~(uint64_t)0);
  dds_apply_entity_naming(new_qos, NULL, &dom->gv);

  /* 验证 QoS 设置。 */
  if ((ret = ddsi_xqos_valid(&dom->gv.logconfig, new_qos)) < 0) goto err_qos_validation;
  // 通用验证代码将检查租约持续时间，我们只需要检查种类是否符合要求
  if (new_qos->liveliness.kind != DDS_LIVELINESS_AUTOMATIC) {
    ret = DDS_RETCODE_BAD_PARAMETER;
    goto err_qos_validation;
  }

  // DDSI 层需要一个它将复制的 plist，DDS 层获取传入 entity_init 的 QoS 对象的所有权。
  // 这里我们必须将 QoS 复制到 plist 中
  ddsi_plist_init_empty(&plist);
  ddsi_xqos_mergein_missing(&plist.qos, new_qos, ~(uint64_t)0);

  ddsi_thread_state_awake(ddsi_lookup_thread_state(), &dom->gv);
  ret = ddsi_new_participant(&guid, &dom->gv, 0, &plist);
  ddsi_thread_state_asleep(ddsi_lookup_thread_state());
  ddsi_plist_fini(&plist);
  if (ret < 0) {
    ret = DDS_RETCODE_ERROR;
    goto err_new_participant;
  }

  /* 初始化参与者实体。 */
  pp = dds_alloc(sizeof(*pp));
  if ((ret = dds_entity_init(&pp->m_entity, &dom->m_entity, DDS_KIND_PARTICIPANT, false, true,
                             new_qos, listener, DDS_PARTICIPANT_STATUS_MASK)) < 0)
    goto err_entity_init;

  pp->m_entity.m_guid = guid;
  pp->m_entity.m_iid = ddsi_get_entity_instanceid(&dom->gv, &guid);
  pp->m_entity.m_domain = dom;
  pp->m_builtin_subscriber = 0;
  ddsrt_avl_init(&participant_ktopics_treedef, &pp->m_ktopics);

  /* 将参与者添加到范围中。 */
  ddsrt_mutex_lock(&dom->m_entity.m_mutex);
  dds_entity_register_child(&dom->m_entity, &pp->m_entity);
  ddsrt_mutex_unlock(&dom->m_entity.m_mutex);

  dds_entity_init_complete(&pp->m_entity);
  /* 删除临时额外引用的域和 dds_init。 */
  dds_entity_unpin_and_drop_ref(&dom->m_entity);
  dds_entity_unpin_and_drop_ref(&dds_global.m_entity);
  return ret;

err_entity_init:
  dds_free(pp);
err_new_participant:
err_qos_validation:
  dds_delete_qos(new_qos);
  dds_entity_unpin_and_drop_ref(&dom->m_entity);
err_domain_create:
  dds_entity_unpin_and_drop_ref(&dds_global.m_entity);
err_dds_init:
  return ret;
}

/**
 * @brief 查找参与者实体 (Lookup participant entities)
 *
 * @param domain_id 域ID (Domain ID)
 * @param participants 参与者实体数组指针 (Pointer to an array of participant entities)
 * @param size 参与者实体数组的大小 (Size of the participant entities array)
 * @return dds_return_t 返回操作结果 (Return operation result)
 */
dds_return_t dds_lookup_participant(dds_domainid_t domain_id,
                                    dds_entity_t *participants,
                                    size_t size) {
  dds_return_t ret;

  // 检查参数是否有效 (Check if parameters are valid)
  if ((participants != NULL && (size == 0 || size >= INT32_MAX)) ||
      (participants == NULL && size != 0))
    return DDS_RETCODE_BAD_PARAMETER;

  // 初始化参与者数组 (Initialize the participants array)
  if (participants) participants[0] = 0;

  // 初始化DDS (Initialize DDS)
  if ((ret = dds_init()) < 0) return ret;

  ret = 0;
  struct dds_domain *dom;
  // 锁定全局互斥锁 (Lock the global mutex)
  ddsrt_mutex_lock(&dds_global.m_mutex);
  // 查找域 (Find the domain)
  if ((dom = dds_domain_find_locked(domain_id)) != NULL) {
    ddsrt_avl_iter_t it;
    // 遍历实体子节点 (Iterate through entity children)
    for (dds_entity *e =
             ddsrt_avl_iter_first(&dds_entity_children_td, &dom->m_entity.m_children, &it);
         e != NULL; e = ddsrt_avl_iter_next(&it)) {
      // 如果数组未满，则将实体添加到参与者数组中 (If the array is not full, add the entity to the
      // participants array)
      if ((size_t)ret < size) participants[ret] = e->m_hdllink.hdl;
      ret++;
    }
  }
  // 解锁全局互斥锁 (Unlock the global mutex)
  ddsrt_mutex_unlock(&dds_global.m_mutex);
  // 取消引用并删除实体 (Unpin and drop reference to the entity)
  dds_entity_unpin_and_drop_ref(&dds_global.m_entity);
  return ret;
}
