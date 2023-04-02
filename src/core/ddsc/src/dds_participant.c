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

// 定义一个宏，用于实现实体的锁定和解锁操作
DECL_ENTITY_LOCK_UNLOCK(dds_participant)

// 定义一个常量，表示参与者状态掩码
#define DDS_PARTICIPANT_STATUS_MASK (0u)
// 比较两个话题名称的函数
static int cmp_ktopic_name(const void *a, const void *b)
{
  return strcmp(a, b);
}

// 定义一个 AVL 树，用于存储参与者的话题
const ddsrt_avl_treedef_t participant_ktopics_treedef = DDSRT_AVL_TREEDEF_INITIALIZER_INDKEY(offsetof(struct dds_ktopic, pp_ktopics_avlnode), offsetof(struct dds_ktopic, name), cmp_ktopic_name, 0);

// 验证参与者状态掩码是否合法
static dds_return_t dds_participant_status_validate(uint32_t mask)
{
  return (mask & ~DDS_PARTICIPANT_STATUS_MASK) ? DDS_RETCODE_BAD_PARAMETER : DDS_RETCODE_OK;
}

// 删除参与者实体
static dds_return_t dds_participant_delete(dds_entity *e)
{
  dds_return_t ret;
  // 确保实体类型为参与者
  assert(dds_entity_kind(e) == DDS_KIND_PARTICIPANT);

  // 确保参与者的所有话题都已经被删除
  assert(ddsrt_avl_is_empty(&((struct dds_participant *)e)->m_ktopics));

  // 唤醒线程状态，以便删除参与者实体
  ddsi_thread_state_awake(ddsi_lookup_thread_state(), &e->m_domain->gv);
  if ((ret = ddsi_delete_participant(&e->m_domain->gv, &e->m_guid)) < 0)
    DDS_CERROR(&e->m_domain->gv.logconfig, "dds_participant_delete: internal error %" PRId32 "\n", ret);
  // 休眠线程状态
  ddsi_thread_state_asleep(ddsi_lookup_thread_state());
  return DDS_RETCODE_OK;
}

// 设置参与者实体的 QoS
static dds_return_t dds_participant_qos_set(dds_entity *e, const dds_qos_t *qos, bool enabled)
{
  // 注意：e->m_qos 仍然是旧的 QoS，以便在此处失败
  if (enabled)
  {
    struct ddsi_participant *pp;
    // 唤醒线程状态，以便设置参与者实体的 QoS
    ddsi_thread_state_awake(ddsi_lookup_thread_state(), &e->m_domain->gv);
    if ((pp = ddsi_entidx_lookup_participant_guid(e->m_domain->gv.entity_index, &e->m_guid)) != NULL)
    {
      ddsi_plist_t plist;
      ddsi_plist_init_empty(&plist);
      plist.qos.present = plist.qos.aliased = qos->present;
      plist.qos = *qos;
      ddsi_update_participant_plist(pp, &plist);
    }
    // 休眠线程状态
    ddsi_thread_state_asleep(ddsi_lookup_thread_state());
  }
  return DDS_RETCODE_OK;
}
// 定义了一个名为dds_entity_deriver_participant的结构体，其中包含了一些函数指针，用于操作DDS实体
const struct dds_entity_deriver dds_entity_deriver_participant = {
    .interrupt = dds_entity_deriver_dummy_interrupt,                  // 中断处理函数
    .close = dds_entity_deriver_dummy_close,                          // 关闭函数
    .delete = dds_participant_delete,                                 // 删除函数
    .set_qos = dds_participant_qos_set,                               // 设置QoS函数
    .validate_status = dds_participant_status_validate,               // 验证状态函数
    .create_statistics = dds_entity_deriver_dummy_create_statistics,  // 创建统计信息函数
    .refresh_statistics = dds_entity_deriver_dummy_refresh_statistics // 刷新统计信息函数
};

// 创建一个DDS参与者实体
dds_entity_t dds_create_participant(const dds_domainid_t domain, const dds_qos_t *qos, const dds_listener_t *listener)
{
  dds_domain *dom;           // DDS域
  dds_entity_t ret;          // 返回值
  ddsi_guid_t guid;          // GUID
  dds_participant *pp;       // DDS参与者
  ddsi_plist_t plist;        // 属性列表
  dds_qos_t *new_qos = NULL; // 新的QoS
  const char *config = "";   // 配置信息

  // 确保DDS实例已经初始化
  if ((ret = dds_init()) < 0)
    goto err_dds_init;

  // 获取环境变量CYCLONEDDS_URI的值
  (void)ddsrt_getenv("CYCLONEDDS_URI", &config);

  // 创建DDS域
  if ((ret = dds_domain_create_internal(&dom, domain, true, config)) < 0)
    goto err_domain_create;

  // 创建新的QoS
  new_qos = dds_create_qos();
  // 如果传入的QoS不为空，则将其与默认的本地QoS合并
  if (qos != NULL)
    ddsi_xqos_mergein_missing(new_qos, qos, DDS_PARTICIPANT_QOS_MASK);
  // 将默认的本地QoS合并到新的QoS中
  ddsi_xqos_mergein_missing(new_qos, &dom->gv.default_local_xqos_pp, ~(uint64_t)0);
  // 应用实体命名规则
  dds_apply_entity_naming(new_qos, NULL, &dom->gv);

  // 验证QoS是否有效
  if ((ret = ddsi_xqos_valid(&dom->gv.logconfig, new_qos)) < 0)
    goto err_qos_validation; // 检查活性状态的类型是否为自动 if (new_qos->liveliness.kind != DDS_LIVELINESS_AUTOMATIC)
  {
    ret = DDS_RETCODE_BAD_PARAMETER;
    goto err_qos_validation;
  }

  // DDSI层需要一个plist，它将被复制，DDS层接管传递给entity_init的QoS对象的所有权
  // 我们需要在这里将QoS复制到plist中
  ddsi_plist_init_empty(&plist);
  ddsi_xqos_mergein_missing(&plist.qos, new_qos, ~(uint64_t)0);

  // 唤醒线程状态并创建新的参与者
  ddsi_thread_state_awake(ddsi_lookup_thread_state(), &dom->gv);
  ret = ddsi_new_participant(&guid, &dom->gv, 0, &plist);
  ddsi_thread_state_asleep(ddsi_lookup_thread_state());

  // 清理plist
  ddsi_plist_fini(&plist);

  // 如果创建参与者失败，则跳转到错误处理
  if (ret < 0)
  {
    ret = DDS_RETCODE_ERROR;
    goto err_new_participant;
  }

  // 分配新的参与者实体
  pp = dds_alloc(sizeof(*pp));

  // 初始化实体并将其注册为子实体
  if ((ret = dds_entity_init(&pp->m_entity, &dom->m_entity, DDS_KIND_PARTICIPANT, false, true, new_qos, listener, DDS_PARTICIPANT_STATUS_MASK)) < 0)
    goto err_entity_init;

  // 设置实体的GUID和IID
  pp->m_entity.m_guid = guid;
  pp->m_entity.m_iid = ddsi_get_entity_instanceid(&dom->gv, &guid);

  // 设置实体的域和内置订阅者
  pp->m_entity.m_domain = dom;
  pp->m_builtin_subscriber = 0;

  // 初始化参与者的主题列表
  ddsrt_avl_init(&participant_ktopics_treedef, &pp->m_ktopics);

  // 将参与者添加到域的范围内
  ddsrt_mutex_lock(&dom->m_entity.m_mutex);
  dds_entity_register_child(&dom->m_entity, &pp->m_entity);
  ddrt_mutex_unlock(&dom->m_entity.m_mutex);

  // 完成实体初始化
  dds_entity_init_complete(&pp->m_entity);

  // 释放对域和dds_init的临时引用
  dds_entity_unpin_and_drop_ref(&dom->m_entity);
  dds_entity_unpin_and_drop_ref(&dds_global.m_entity);

  // 返回参与者实体
  return ret;

// 错误处理
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
  // 定义函数 dds_lookup_participant，接收 domain_id、participants 和 size 三个参数
  dds_return_t dds_lookup_participant(dds_domainid_t domain_id, dds_entity_t * participants, size_t size)
  {
    dds_return_t ret;

    // 检查参数是否合法
    if ((participants != NULL && (size == 0 || size >= INT32_MAX)) || (participants == NULL && size != 0))
      return DDS_RETCODE_BAD_PARAMETER;

    // 如果 participants 不为 NULL，则将第一个元素设置为 0
    if (participants)
      participants[0] = 0;

    // 初始化 DDS
    if ((ret = dds_init()) < 0)
      return ret;

    // 初始化 ret 为 0
    ret = 0;

    // 定义一个指向 dds_domain 结构体的指针 dom
    struct dds_domain *dom;

    // 加锁以访问全局变量 dds_global
    ddsrt_mutex_lock(&dds_global.m_mutex);

    // 在 dds_domain 中查找指定的 domain_id
    if ((dom = dds_domain_find_locked(domain_id)) != NULL)
    {
      // 遍历 domain_id 中的所有实体
      ddsrt_avl_iter_t it;
      for (dds_entity *e = ddsrt_avl_iter_first(&dds_entity_children_td, &dom->m_entity.m_children, &it); e != NULL; e = ddsrt_avl_iter_next(&it))
      {
        // 如果 ret 小于 size，则将实体的句柄存储在 participants 数组中
        if ((size_t)ret < size)
          participants[ret] = e->m_hdllink.hdl;
        ret++;
      }
    }

    // 解锁以访问全局变量 dds_global
    ddsrt_mutex_unlock(&dds_global.m_mutex);

    // 释放 dds_global.m_entity 的引用并将其从引用计数中删除
    dds_entity_unpin_and_drop_ref(&dds_global.m_entity);

    // 返回 ret
    return ret;
  }
