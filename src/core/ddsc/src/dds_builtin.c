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
#include "dds/ddsi/ddsi_domaingv.h"
#include "dds/ddsi/ddsi_endpoint.h"
#include "dds/ddsi/ddsi_entity.h"
#include "dds/ddsi/ddsi_plist.h"
#include "dds/ddsi/ddsi_qosmatch.h"
#include "dds/ddsi/ddsi_thread.h"
#include "dds/ddsi/ddsi_tkmap.h"
#include "dds/ddsi/ddsi_topic.h"
#include "dds/ddsrt/string.h"
#include "dds__builtin.h"
#include "dds__domain.h"
#include "dds__entity.h"
#include "dds__init.h"
#include "dds__participant.h"
#include "dds__qos.h"
#include "dds__serdata_builtintopic.h"
#include "dds__subscriber.h"
#include "dds__topic.h"
#include "dds__types.h"
#include "dds__whc_builtintopic.h"
#include "dds__write.h"
#include "dds__writer.h"
#include <assert.h>
#include <string.h>

/**
 * @brief 创建内置的QoS对象
 *
 * 该函数创建一个内置的QoS对象，并设置一些默认的QoS策略。
 *
 * @return 返回创建的dds_qos_t指针
 */
dds_qos_t *dds__create_builtin_qos(void)
{
  // 定义内置分区名
  const char *partition = "__BUILT-IN PARTITION__";

  // 创建一个新的QoS对象
  dds_qos_t *qos = dds_create_qos();

  // 设置Durability QoS策略为TRANSIENT_LOCAL
  dds_qset_durability(qos, DDS_DURABILITY_TRANSIENT_LOCAL);

  // 设置Presentation QoS策略，使用TOPIC模式，不进行分组访问和排序
  dds_qset_presentation(qos, DDS_PRESENTATION_TOPIC, false, false);

  // 设置Reliability QoS策略为RELIABLE，并设置最大传输延迟为100毫秒
  dds_qset_reliability(qos, DDS_RELIABILITY_RELIABLE, DDS_MSECS(100));

  // 设置Partition QoS策略，使用内置分区名
  dds_qset_partition(qos, 1, &partition);

  // 将缺失的QoS策略与ddsi_default_qos_topic合并
  ddsi_xqos_mergein_missing(qos, &ddsi_default_qos_topic, DDS_TOPIC_QOS_MASK);

  // 设置Data Representation QoS策略为XCDR1
  dds_qset_data_representation(qos, 1, (dds_data_representation_id_t[]){DDS_DATA_REPRESENTATION_XCDR1});

  // 返回创建的QoS对象
  return qos;
}

/**
 * @file
 * @brief 内置主题列表和获取内置主题名称与类型名的函数实现
 */

// 内置主题列表，包含伪句柄、名称和类型名
static const struct
{
  dds_entity_t pseudo_handle; ///< 伪句柄
  const char *name;           ///< 主题名称
  const char *typename;       ///< 类型名
} builtin_topic_list[] = {
    {DDS_BUILTIN_TOPIC_DCPSPARTICIPANT, DDS_BUILTIN_TOPIC_PARTICIPANT_NAME, "org::eclipse::cyclonedds::builtin::DCPSParticipant"},
    {DDS_BUILTIN_TOPIC_DCPSTOPIC, DDS_BUILTIN_TOPIC_TOPIC_NAME, "org::eclipse::cyclonedds::builtin::DCPSTopic"},
    {DDS_BUILTIN_TOPIC_DCPSPUBLICATION, DDS_BUILTIN_TOPIC_PUBLICATION_NAME, "org::eclipse::cyclonedds::builtin::DCPSPublication"},
    {DDS_BUILTIN_TOPIC_DCPSSUBSCRIPTION, DDS_BUILTIN_TOPIC_SUBSCRIPTION_NAME, "org::eclipse::cyclonedds::builtin::DCPSSubscription"}};

/**
 * @brief 获取内置主题的名称和类型名
 *
 * @param[in]  pseudo_handle  伪句柄
 * @param[out] name           指向名称的指针（可选）
 * @param[out] typename       指向类型名的指针（可选）
 * @return 成功时返回0，失败时返回错误代码
 */
dds_return_t dds__get_builtin_topic_name_typename(dds_entity_t pseudo_handle, const char **name, const char **typename)
{
  const char *n;
  const char *tn;

  // 避免搜索（主要是因为我们可以）
  DDSRT_STATIC_ASSERT(DDS_BUILTIN_TOPIC_DCPSTOPIC == DDS_BUILTIN_TOPIC_DCPSPARTICIPANT + 1 &&
                      DDS_BUILTIN_TOPIC_DCPSPUBLICATION == DDS_BUILTIN_TOPIC_DCPSTOPIC + 1 &&
                      DDS_BUILTIN_TOPIC_DCPSSUBSCRIPTION == DDS_BUILTIN_TOPIC_DCPSPUBLICATION + 1);

  switch (pseudo_handle)
  {
  case DDS_BUILTIN_TOPIC_DCPSPARTICIPANT:
  case DDS_BUILTIN_TOPIC_DCPSTOPIC:
  case DDS_BUILTIN_TOPIC_DCPSPUBLICATION:
  case DDS_BUILTIN_TOPIC_DCPSSUBSCRIPTION:
  {
    dds_entity_t idx = pseudo_handle - DDS_BUILTIN_TOPIC_DCPSPARTICIPANT;
    n = builtin_topic_list[idx].name;
    tn = builtin_topic_list[idx].typename;
    break;
  }
  default:
  {
    // 没有断言：这也被一些API调用使用
    return DDS_RETCODE_BAD_PARAMETER;
  }
  }

  if (name)
    *name = n;
  if (typename)
    *typename = tn;

  return 0;
}

/**
 * @brief 通过类型名获取内置主题的伪句柄
 *
 * @param[in] typename 类型名称
 * @return 内置主题的伪句柄，如果找不到则返回 DDS_RETCODE_BAD_PARAMETER
 */
dds_entity_t dds__get_builtin_topic_pseudo_handle_from_typename(const char *typename)
{
  // 遍历内置主题列表
  for (size_t i = 0; i < sizeof(builtin_topic_list) / sizeof(builtin_topic_list[0]); i++)
  {
    // 如果找到匹配的类型名
    if (strcmp(typename, builtin_topic_list[i].typename) == 0)
      // 返回对应的伪句柄
      return builtin_topic_list[i].pseudo_handle;
  }
  // 如果没有找到匹配的类型名，返回错误码
  return DDS_RETCODE_BAD_PARAMETER;
}

/**
 * @brief 获取内置主题
 *
 * @param[in] entity 实体
 * @param[in] topic 主题
 * @return 成功时返回内置主题，失败时返回错误码
 */
dds_entity_t dds__get_builtin_topic(dds_entity_t entity, dds_entity_t topic)
{
  dds_entity_t tp;
  dds_return_t rc;
  dds_entity *e;
  dds_participant *par;

  // 尝试锁定实体
  if ((rc = dds_entity_pin(entity, &e)) < 0)
    return rc;
  // 获取参与者
  if ((par = dds_entity_participant(e)) == NULL)
  {
    // 解锁实体并返回错误码
    dds_entity_unpin(e);
    return DDS_RETCODE_ILLEGAL_OPERATION;
  }

  const char *topic_name;
  struct ddsi_sertype *sertype;
  // 获取内置主题的名称和类型名
  (void)dds__get_builtin_topic_name_typename(topic, &topic_name, NULL);
  // 根据主题选择对应的序列化类型
  switch (topic)
  {
  case DDS_BUILTIN_TOPIC_DCPSPARTICIPANT:
    sertype = e->m_domain->builtin_participant_type;
    break;
#ifdef DDS_HAS_TOPIC_DISCOVERY
  case DDS_BUILTIN_TOPIC_DCPSTOPIC:
    sertype = e->m_domain->builtin_topic_type;
    break;
#endif
  case DDS_BUILTIN_TOPIC_DCPSPUBLICATION:
    sertype = e->m_domain->builtin_writer_type;
    break;
  case DDS_BUILTIN_TOPIC_DCPSSUBSCRIPTION:
    sertype = e->m_domain->builtin_reader_type;
    break;
  default:
    assert(0);
    // 解锁实体并返回错误码
    dds_entity_unpin(e);
    return DDS_RETCODE_BAD_PARAMETER;
  }

  // 创建内置 QoS
  dds_qos_t *qos = dds__create_builtin_qos();
  // 创建主题实现
  if ((tp = dds_create_topic_impl(par->m_entity.m_hdllink.hdl, topic_name, true, &sertype, qos, NULL, true)) > 0)
  {
    /* 保持内置序列化类型的所有权，因为它们会被重用，这些序列化类型的生命周期与域绑定 */
    ddsi_sertype_ref(sertype);
  }
  // 删除 QoS
  dds_delete_qos(qos);
  // 解锁实体
  dds_entity_unpin(e);
  return tp;
}

/**
 * @brief 检查给定的 QoS 是否具有资源限制。
 *
 * @param[in] qos 要检查的 dds_qos_t 结构指针。
 * @return 如果存在资源限制，则返回 true，否则返回 false。
 */
static bool qos_has_resource_limits(const dds_qos_t *qos)
{
  // 断言 QoS 中包含资源限制
  assert(qos->present & DDSI_QP_RESOURCE_LIMITS);

  // 检查资源限制是否不受限制，如果任何一个资源限制不是无限制的，则返回 true
  return (qos->resource_limits.max_samples != DDS_LENGTH_UNLIMITED ||
          qos->resource_limits.max_instances != DDS_LENGTH_UNLIMITED ||
          qos->resource_limits.max_samples_per_instance != DDS_LENGTH_UNLIMITED);
}

/**
 * @brief 验证内置读取器的 QoS 设置是否有效。
 *
 * @param[in] dom 域指针。
 * @param[in] topic 主题实体。
 * @param[in] qos 要验证的 dds_qos_t 结构指针。
 * @return 如果 QoS 设置有效，则返回 true，否则返回 false。
 */
bool dds__validate_builtin_reader_qos(const dds_domain *dom, dds_entity_t topic, const dds_qos_t *qos)
{
  if (qos == NULL)
    // 默认 QoS 继承自主题，因此默认情况下有效
    return true;
  else
  {
    /* 如果内置主题的写入者具有资源限制，为避免失败的写操作带来的复杂问题，
       禁止创建与内置主题写入者匹配的读取器 */
    struct ddsi_local_orphan_writer *bwr;
    switch (topic)
    {
    case DDS_BUILTIN_TOPIC_DCPSPARTICIPANT:
      bwr = dom->builtintopic_writer_participant;
      break;
#ifdef DDS_HAS_TOPIC_DISCOVERY
    case DDS_BUILTIN_TOPIC_DCPSTOPIC:
      bwr = dom->builtintopic_writer_topics;
      break;
#endif
    case DDS_BUILTIN_TOPIC_DCPSPUBLICATION:
      bwr = dom->builtintopic_writer_publications;
      break;
    case DDS_BUILTIN_TOPIC_DCPSSUBSCRIPTION:
      bwr = dom->builtintopic_writer_subscriptions;
      break;
    default:
      assert(0);
      return false;
    }

    /* FIXME: DDSI 级别的读取器、写入者在 QoS 中具有主题和类型名称，
       但 DDSC 级别的没有，这在比较完整的 QoS 对象时会导致自动不匹配。
       在此处，两者在构建时具有相同的主题，因此在比较中忽略它们可以使事情正常工作。
       有一天应该解决这个差异。 */
    const uint64_t qmask = ~(DDSI_QP_TOPIC_NAME | DDSI_QP_TYPE_NAME | DDSI_QP_TYPE_INFORMATION);
    dds_qos_policy_id_t dummy;
#ifdef DDS_HAS_TYPE_DISCOVERY
    return ddsi_qos_match_mask_p(bwr->wr.e.gv, qos, bwr->wr.xqos, qmask, &dummy, NULL, NULL, NULL, NULL) && !qos_has_resource_limits(qos);
#else
    return ddsi_qos_match_mask_p(bwr->wr.e.gv, qos, bwr->wr.xqos, qmask, &dummy) && !qos_has_resource_limits(qos);
#endif
  }
}

/**
 * @brief 创建内置订阅者实体
 *
 * @param[in] participant 指向dds_participant结构体的指针
 * @return 返回创建的内置订阅者实体
 */
static dds_entity_t dds__create_builtin_subscriber(dds_participant *participant)
{
  // 创建内置QoS对象
  dds_qos_t *qos = dds__create_builtin_qos();
  // 使用内置QoS创建订阅者实体
  dds_entity_t sub = dds__create_subscriber_l(participant, false, qos, NULL);
  // 删除QoS对象
  dds_delete_qos(qos);
  // 返回创建的订阅者实体
  return sub;
}

/**
 * @brief 获取内置订阅者实体
 *
 * @param[in] e 实体标识符
 * @return 返回内置订阅者实体，如果失败则返回错误代码
 */
dds_entity_t dds__get_builtin_subscriber(dds_entity_t e)
{
  dds_entity_t sub;
  dds_return_t ret;
  dds_entity_t pp;
  dds_participant *p;

  // 获取参与者实体
  if ((pp = dds_get_participant(e)) <= 0)
    return pp;
  // 锁定参与者实体
  if ((ret = dds_participant_lock(pp, &p)) != DDS_RETCODE_OK)
    return ret;

  // 如果内置订阅者不存在，则创建一个
  if (p->m_builtin_subscriber <= 0)
  {
    p->m_builtin_subscriber = dds__create_builtin_subscriber(p);
  }
  // 获取内置订阅者实体
  sub = p->m_builtin_subscriber;
  // 解锁参与者实体
  dds_participant_unlock(p);
  // 返回内置订阅者实体
  return sub;
}

/**
 * @brief 判断是否为内置主题
 *
 * @param[in] tp 指向ddsi_sertype结构体的指针
 * @param[in] vdomain 未使用的参数
 * @return 如果是内置主题则返回true，否则返回false
 */
static bool dds__builtin_is_builtintopic(const struct ddsi_sertype *tp, void *vdomain)
{
  (void)vdomain;
  return tp->ops == &ddsi_sertype_ops_builtintopic;
}

/**
 * @brief 判断GUID是否可见
 *
 * @param[in] guid 指向ddsi_guid_t结构体的指针
 * @param[in] vendorid 供应商ID
 * @param[in] vdomain 未使用的参数
 * @return 如果GUID可见则返回true，否则返回false
 */
static bool dds__builtin_is_visible(const ddsi_guid_t *guid, ddsi_vendorid_t vendorid, void *vdomain)
{
  (void)vdomain;
  if (ddsi_is_builtin_endpoint(guid->entityid, vendorid) || ddsi_is_builtin_topic(guid->entityid, vendorid))
    return false;
  return true;
}

/**
 * @brief 获取指定GUID的tkmap实例
 *
 * @param[in] guid      实体的GUID
 * @param[in] vdomain   指向dds_domain结构体的指针
 * @return 返回对应的ddsi_tkmap_instance指针
 */
static struct ddsi_tkmap_instance *dds__builtin_get_tkmap_entry(const struct ddsi_guid *guid, void *vdomain)
{
  // 将void指针转换为dds_domain指针
  struct dds_domain *domain = vdomain;

  // 创建一个内置主题的序列化数据，用于查找tkmap实例
  struct ddsi_serdata *sd = dds_serdata_builtin_from_endpoint(domain->builtin_participant_type, guid, NULL, SDK_KEY);

  // 在tkmap中查找对应的实例
  struct ddsi_tkmap_instance *tk = ddsi_tkmap_find(domain->gv.m_tkmap, sd, true);

  // 减少serdata的引用计数
  ddsi_serdata_unref(sd);

  // 返回找到的tkmap实例
  return tk;
}

/**
 * @brief 为给定的实体创建一个内置样本
 *
 * @param[in] e         指向ddsi_entity_common结构体的指针
 * @param[in] timestamp 写入时间戳
 * @param[in] alive     实体是否存活
 * @return 返回创建的ddsi_serdata指针
 */
struct ddsi_serdata *dds__builtin_make_sample_endpoint(const struct ddsi_entity_common *e, ddsrt_wctime_t timestamp, bool alive)
{
  // 初始化dom变量，避免C类型系统导致的gcc警告
  struct dds_domain *dom = e->gv->builtin_topic_interface->arg;
  struct ddsi_sertype *type = NULL;

  // 根据实体类型选择相应的内置类型
  switch (e->kind)
  {
  case DDSI_EK_PARTICIPANT:
  case DDSI_EK_PROXY_PARTICIPANT:
    type = dom->builtin_participant_type;
    break;
  case DDSI_EK_WRITER:
  case DDSI_EK_PROXY_WRITER:
    type = dom->builtin_writer_type;
    break;
  case DDSI_EK_READER:
  case DDSI_EK_PROXY_READER:
    type = dom->builtin_reader_type;
    break;
  default:
    abort();
    break;
  }

  // 确保类型不为空
  assert(type != NULL);

  // 为给定的实体创建一个内置样本
  struct ddsi_serdata *serdata = dds_serdata_builtin_from_endpoint(type, &e->guid, (struct ddsi_entity_common *)e, alive ? SDK_DATA : SDK_KEY);

  // 设置样本的时间戳和状态信息
  serdata->timestamp = timestamp;
  serdata->statusinfo = alive ? 0 : DDSI_STATUSINFO_DISPOSE | DDSI_STATUSINFO_UNREGISTER;

  // 返回创建的样本
  return serdata;
}

#ifdef DDS_HAS_TOPIC_DISCOVERY

/**
 * @brief 为内置主题创建序列化数据样本的实现函数
 *
 * @param[in] tpd        主题定义指针
 * @param[in] timestamp  时间戳
 * @param[in] alive      样本是否存活
 * @return 返回序列化数据样本指针
 */
static struct ddsi_serdata *dds__builtin_make_sample_topic_impl(const struct ddsi_topic_definition *tpd, ddsrt_wctime_t timestamp, bool alive)
{
  // 获取域对象
  struct dds_domain *dom = tpd->gv->builtin_topic_interface->arg;
  // 获取内置主题类型
  struct ddsi_sertype *type = dom->builtin_topic_type;
  // 从主题定义创建序列化数据
  struct ddsi_serdata *serdata = dds_serdata_builtin_from_topic_definition(type, (dds_builtintopic_topic_key_t *)&tpd->key, tpd, alive ? SDK_DATA : SDK_KEY);
  // 设置序列化数据的时间戳
  serdata->timestamp = timestamp;
  // 设置序列化数据的状态信息
  serdata->statusinfo = alive ? 0 : DDSI_STATUSINFO_DISPOSE | DDSI_STATUSINFO_UNREGISTER;
  // 返回序列化数据指针
  return serdata;
}

/**
 * @brief 为内置主题创建序列化数据样本
 *
 * @param[in] e          实体通用结构指针
 * @param[in] timestamp  时间戳
 * @param[in] alive      样本是否存活
 * @return 返回序列化数据样本指针
 */
struct ddsi_serdata *dds__builtin_make_sample_topic(const struct ddsi_entity_common *e, ddsrt_wctime_t timestamp, bool alive)
{
  // 获取主题指针
  struct ddsi_topic *tp = (struct ddsi_topic *)e;
  // 锁定QoS锁
  ddsrt_mutex_lock(&tp->e.qos_lock);
  // 调用实现函数创建序列化数据样本
  struct ddsi_serdata *sd = dds__builtin_make_sample_topic_impl(tp->definition, timestamp, alive);
  // 解锁QoS锁
  ddsrt_mutex_unlock(&tp->e.qos_lock);
  // 返回序列化数据样本指针
  return sd;
}

/**
 * @brief 为代理内置主题创建序列化数据样本
 *
 * @param[in] proxytp    代理主题指针
 * @param[in] timestamp  时间戳
 * @param[in] alive      样本是否存活
 * @return 返回序列化数据样本指针
 */
struct ddsi_serdata *dds__builtin_make_sample_proxy_topic(const struct ddsi_proxy_topic *proxytp, ddsrt_wctime_t timestamp, bool alive)
{
  // 调用实现函数创建序列化数据样本
  return dds__builtin_make_sample_topic_impl(proxytp->definition, timestamp, alive);
}

#endif /* DDS_HAS_TOPIC_DISCOVERY  */

/**
 * @brief 写入内置端点信息
 *
 * @param[in] e         实体通用结构指针
 * @param[in] timestamp 写入操作的时间戳
 * @param[in] alive     实体是否存活
 * @param[in] vdomain   域指针
 */
static void dds__builtin_write_endpoint(const struct ddsi_entity_common *e, ddsrt_wctime_t timestamp, bool alive, void *vdomain)
{
  // 将void指针转换为dds_domain指针
  struct dds_domain *dom = vdomain;

  // 检查实体是否可见
  if (dds__builtin_is_visible(&e->guid, ddsi_get_entity_vendorid(e), dom))
  {
    // 初始化bwr以避免C类型系统引起的gcc警告
    struct ddsi_local_orphan_writer *bwr = NULL;

    // 创建内置样本端点
    struct ddsi_serdata *serdata = dds__builtin_make_sample_endpoint(e, timestamp, alive);

    // 断言实体的主题不为空
    assert(e->tk != NULL);

    // 根据实体类型选择相应的内置主题写入器
    switch (e->kind)
    {
    case DDSI_EK_PARTICIPANT:
    case DDSI_EK_PROXY_PARTICIPANT:
      bwr = dom->builtintopic_writer_participant;
      break;
    case DDSI_EK_WRITER:
    case DDSI_EK_PROXY_WRITER:
      bwr = dom->builtintopic_writer_publications;
      break;
    case DDSI_EK_READER:
    case DDSI_EK_PROXY_READER:
      bwr = dom->builtintopic_writer_subscriptions;
      break;
    default:
      // 遇到未知类型时中止程序
      abort();
      break;
    }

    // 使用内置主题写入器将序列化数据写入本地孤立实体
    dds_writecdr_local_orphan_impl(bwr, serdata);
  }
}

#ifdef DDS_HAS_TOPIC_DISCOVERY
/**
 * @brief 写入内置主题数据
 *
 * @param[in] tpd        主题定义指针
 * @param[in] timestamp  写入操作的时间戳
 * @param[in] alive      标识主题是否存活
 * @param[in] vdomain    域指针
 */
static void dds__builtin_write_topic(const struct ddsi_topic_definition *tpd, ddsrt_wctime_t timestamp, bool alive, void *vdomain)
{
  struct dds_domain *dom = vdomain;                                                          // 将void类型的vdomain转换为dds_domain结构体指针
  struct ddsi_local_orphan_writer *bwr = dom->builtintopic_writer_topics;                    // 获取内置主题写入器
  struct ddsi_serdata *serdata = dds__builtin_make_sample_topic_impl(tpd, timestamp, alive); // 创建序列化数据样本
  dds_writecdr_local_orphan_impl(bwr, serdata);                                              // 将序列化数据样本写入内置主题写入器
}
#endif

/**
 * @brief 取消引用内置类型
 *
 * @param[in] dom  域指针
 */
static void unref_builtin_types(struct dds_domain *dom)
{
  ddsi_sertype_unref(dom->builtin_participant_type); // 取消引用内置参与者类型
#ifdef DDS_HAS_TOPIC_DISCOVERY
  ddsi_sertype_unref(dom->builtin_topic_type); // 取消引用内置主题类型
#endif
  ddsi_sertype_unref(dom->builtin_reader_type); // 取消引用内置读取器类型
  ddsi_sertype_unref(dom->builtin_writer_type); // 取消引用内置写入器类型
}

/**
 * @brief 初始化内置主题和实体的函数
 *
 * 该函数用于初始化内置主题和实体，包括创建QoS、设置内置主题接口函数、注册内置类型等。
 *
 * @param[in] dom 指向dds_domain结构体的指针
 */
void dds__builtin_init(struct dds_domain *dom)
{
  // 创建内置QoS
  dds_qos_t *qos = dds__create_builtin_qos();

  // 设置内置主题接口函数
  dom->btif.arg = dom;
  dom->btif.builtintopic_get_tkmap_entry = dds__builtin_get_tkmap_entry;
  dom->btif.builtintopic_is_builtintopic = dds__builtin_is_builtintopic;
  dom->btif.builtintopic_is_visible = dds__builtin_is_visible;
  dom->btif.builtintopic_write_endpoint = dds__builtin_write_endpoint;
#ifdef DDS_HAS_TOPIC_DISCOVERY
  dom->btif.builtintopic_write_topic = dds__builtin_write_topic;
#endif
  dom->gv.builtin_topic_interface = &dom->btif;

  // 获取内置主题名称和类型名
  const char *typename;
  (void)dds__get_builtin_topic_name_typename(DDS_BUILTIN_TOPIC_DCPSPARTICIPANT, NULL, &typename);
  dom->builtin_participant_type = dds_new_sertype_builtintopic(DSBT_PARTICIPANT, typename);
#ifdef DDS_HAS_TOPIC_DISCOVERY
  (void)dds__get_builtin_topic_name_typename(DDS_BUILTIN_TOPIC_DCPSTOPIC, NULL, &typename);
  dom->builtin_topic_type = dds_new_sertype_builtintopic_topic(DSBT_TOPIC, typename);
#endif
  (void)dds__get_builtin_topic_name_typename(DDS_BUILTIN_TOPIC_DCPSSUBSCRIPTION, NULL, &typename);
  dom->builtin_reader_type = dds_new_sertype_builtintopic(DSBT_READER, typename);
  (void)dds__get_builtin_topic_name_typename(DDS_BUILTIN_TOPIC_DCPSPUBLICATION, NULL, &typename);
  dom->builtin_writer_type = dds_new_sertype_builtintopic(DSBT_WRITER, typename);

  // 注册内置类型
  ddsrt_mutex_lock(&dom->gv.sertypes_lock);
  ddsi_sertype_register_locked(&dom->gv, dom->builtin_participant_type);
#ifdef DDS_HAS_TOPIC_DISCOVERY
  ddsi_sertype_register_locked(&dom->gv, dom->builtin_topic_type);
#endif
  ddsi_sertype_register_locked(&dom->gv, dom->builtin_reader_type);
  ddsi_sertype_register_locked(&dom->gv, dom->builtin_writer_type);
  ddsrt_mutex_unlock(&dom->gv.sertypes_lock);

  // 唤醒线程状态
  ddsi_thread_state_awake(ddsi_lookup_thread_state(), &dom->gv);
  const struct ddsi_entity_index *gh = dom->gv.entity_index;
  dom->builtintopic_writer_participant = ddsi_new_local_orphan_writer(&dom->gv, ddsi_to_entityid(DDSI_ENTITYID_SPDP_BUILTIN_PARTICIPANT_WRITER), DDS_BUILTIN_TOPIC_PARTICIPANT_NAME, dom->builtin_participant_type, qos, dds_builtintopic_whc_new(DSBT_PARTICIPANT, gh));
#ifdef DDS_HAS_TOPIC_DISCOVERY
  dom->builtintopic_writer_topics = ddsi_new_local_orphan_writer(&dom->gv, ddsi_to_entityid(DDSI_ENTITYID_SEDP_BUILTIN_TOPIC_WRITER), DDS_BUILTIN_TOPIC_TOPIC_NAME, dom->builtin_topic_type, qos, dds_builtintopic_whc_new(DSBT_TOPIC, gh));
#endif
  dom->builtintopic_writer_publications = ddsi_new_local_orphan_writer(&dom->gv, ddsi_to_entityid(DDSI_ENTITYID_SEDP_BUILTIN_PUBLICATIONS_WRITER), DDS_BUILTIN_TOPIC_PUBLICATION_NAME, dom->builtin_writer_type, qos, dds_builtintopic_whc_new(DSBT_WRITER, gh));
  dom->builtintopic_writer_subscriptions = ddsi_new_local_orphan_writer(&dom->gv, ddsi_to_entityid(DDSI_ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_WRITER), DDS_BUILTIN_TOPIC_SUBSCRIPTION_NAME, dom->builtin_reader_type, qos, dds_builtintopic_whc_new(DSBT_READER, gh));
  // 睡眠线程状态
  ddsi_thread_state_asleep(ddsi_lookup_thread_state());

  // 删除QoS
  dds_delete_qos(qos);

  // 减少内置类型的引用计数
  unref_builtin_types(dom);
}

/**
 * @brief 释放DDS域内的内置主题资源
 *
 * @param dom 指向要处理的dds_domain结构体的指针
 */
void dds__builtin_fini(struct dds_domain *dom)
{
  // 不再为内置主题样本提供更多来源
  ddsi_thread_state_awake(ddsi_lookup_thread_state(), &dom->gv);

  // 删除参与者内置主题的本地孤立写入器
  ddsi_delete_local_orphan_writer(dom->builtintopic_writer_participant);

#ifdef DDS_HAS_TOPIC_DISCOVERY
  // 如果定义了DDS_HAS_TOPIC_DISCOVERY，删除主题发现内置主题的本地孤立写入器
  ddsi_delete_local_orphan_writer(dom->builtintopic_writer_topics);
#endif

  // 删除发布内置主题的本地孤立写入器
  ddsi_delete_local_orphan_writer(dom->builtintopic_writer_publications);

  // 删除订阅内置主题的本地孤立写入器
  ddsi_delete_local_orphan_writer(dom->builtintopic_writer_subscriptions);

  // 将线程状态设置为休眠
  ddsi_thread_state_asleep(ddsi_lookup_thread_state());

  // 取消对内置类型的引用
  unref_builtin_types(dom);
}
