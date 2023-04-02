/*
 * Copyright(c) 2019 to 2022 ZettaScale Technology and others
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

#include "dds/dds.h"
#include "dds/ddsi/ddsi_domaingv.h"
#include "dds/ddsi/ddsi_endpoint.h"
#include "dds/ddsi/ddsi_endpoint_match.h"
#include "dds/ddsi/ddsi_entity.h"
#include "dds/ddsi/ddsi_entity_index.h"
#include "dds/ddsi/ddsi_participant.h"
#include "dds/ddsi/ddsi_proxy_participant.h"
#include "dds/ddsi/ddsi_thread.h"
#include "dds/ddsrt/heap.h"
#include "dds/ddsrt/string.h"
#include "dds/version.h"
#include "dds__reader.h"
#include "dds__topic.h"
#include "dds__writer.h"

// 获取与写入器匹配的订阅者实例句柄
dds_return_t dds_get_matched_subscriptions(dds_entity_t writer, dds_instance_handle_t *rds, size_t nrds)
{
  dds_writer *wr;
  dds_return_t rc;
  // 检查参数是否合法
  if ((rds != NULL && (nrds == 0 || nrds > INT32_MAX)) || (rds == NULL && nrds != 0))
    return DDS_RETCODE_BAD_PARAMETER;
  // 锁定写入器
  if ((rc = dds_writer_lock(writer, &wr)) != DDS_RETCODE_OK)
    return rc;
  else
  {
    // 获取与写入器匹配的订阅者实例句柄
    rc = ddsi_writer_get_matched_subscriptions(wr->m_wr, rds, nrds);
    // 解锁写入器
    dds_writer_unlock(wr);
    return rc;
  }
}

// 获取与阅读器匹配的发布者实例句柄
dds_return_t dds_get_matched_publications(dds_entity_t reader, dds_instance_handle_t *wrs, size_t nwrs)
{
  dds_reader *rd;
  dds_return_t rc;
  // 检查参数是否合法
  if ((wrs != NULL && (nwrs == 0 || nwrs > INT32_MAX)) || (wrs == NULL && nwrs != 0))
    return DDS_RETCODE_BAD_PARAMETER;
  // 锁定阅读器
  if ((rc = dds_reader_lock(reader, &rd)) != DDS_RETCODE_OK)
    return rc;
  else
  {
    // 获取与阅读器匹配的发布者实例句柄
    rc = ddsi_reader_get_matched_publications(rd->m_rd, wrs, nwrs);
    // 解锁阅读器
    dds_reader_unlock(rd);
    return rc;
  }
}

// 创建内置主题端点
static dds_builtintopic_endpoint_t *make_builtintopic_endpoint(const ddsi_guid_t *guid, const ddsi_guid_t *ppguid, dds_instance_handle_t ppiid, const dds_qos_t *qos)
{
  dds_builtintopic_endpoint_t ep;
  ddsi_guid_t tmp; // 分配内存 ep = dds_alloc(sizeof(ep));
  // 将GUID转换为网络字节序
  tmp = ddsi_hton_guid(guid);
  memcpy(&ep->key, &tmp, sizeof(ep->key)); // 设置参与者实例句柄 ep->participant_instance_handle = ppiid; // 将参与者GUID转换为网络字节序 tmp = ddsi_hton_guid(ppguid);
  memcpy(&ep->participant_key, &tmp, sizeof(ep->participant_key));
  // 创建QoS
  ep->qos = dds_create_qos();
  // 将QoS合并到内置主题端点的QoS中
  ddsi_xqos_mergein_missing(ep->qos, qos, ~(DDSI_QP_TOPIC_NAME | DDSI_QP_TYPE_NAME));
  // 复制主题名称和类型名称
  ep->topic_name = dds_string_dup(qos->topic_name);
  ep->type_name = dds_string_dup(qos->type_name);
  return ep;
}

// 获取与写入器匹配的订阅者数据
dds_builtintopic_endpoint_t *dds_get_matched_subscription_data(dds_entity_t writer, dds_instance_handle_t ih)
{
  dds_writer *wr;
  // 锁定写入器
  if (dds_writer_lock(writer, &wr))
    return NULL;

  dds_builtintopic_endpoint_t *ret = NULL;
  struct ddsi_entity_common *rdc;
  struct dds_qos *rdqos;
  struct ddsi_entity_common *ppc;

  // 确保线程在使用DDSI实体指针时处于“唤醒”状态
  struct ddsi_domaingv *const gv = &wr->m_entity.m_domain->gv;
  ddsi_thread_state_awake(ddsi_lookup_thread_state(), gv);
  // 查找与写入器匹配的订阅者
  if (ddsi_writer_find_matched_reader(wr->m_wr, ih, &rdc, &rdqos, &ppc))
    // 创建内置主题端点
    ret = make_builtintopic_endpoint(&rdc->guid, &ppc->guid, ppc->iid, rdqos);
  // 线程恢复“休眠”状态
  ddsi_thread_state_asleep(ddsi_lookup_thread_state());

  // 解锁写入器
  dds_writer_unlock(wr);
  return ret;
}
// 获取匹配的发布者数据
dds_builtintopic_endpoint_t *dds_get_matched_publication_data(dds_entity_t reader, dds_instance_handle_t ih)
{
  dds_reader *rd;
  // 锁定读取器
  if (dds_reader_lock(reader, &rd))
    return NULL;

  dds_builtintopic_endpoint_t *ret = NULL;
  struct ddsi_entity_common *wrc;
  struct dds_qos *wrqos;
  struct ddsi_entity_common *ppc;

  // 线程在使用DDSI实体指针时必须“唤醒”
  struct ddsi_domaingv *const gv = &rd->m_entity.m_domain->gv;
  ddsi_thread_state_awake(ddsi_lookup_thread_state(), gv);
  // 查找匹配的写入器
  if (ddsi_reader_find_matched_writer(rd->m_rd, ih, &wrc, &wrqos, &ppc))
    // 创建内置主题端点
    ret = make_builtintopic_endpoint(&wrc->guid, &ppc->guid, ppc->iid, wrqos);
  // 线程在使用DDSI实体指针时必须“休眠”
  ddsi_thread_state_asleep(ddsi_lookup_thread_state());

  // 解锁读取器
  dds_reader_unlock(rd);
  return ret;
}

#ifdef DDS_HAS_TYPE_DISCOVERY
/**
@brief 获取内置主题的端点类型信息
@param builtintopic_endpoint 内置主题端点
@param type_info 类型信息
@return dds_return_t 返回代码
*/
dds_return_t dds_builtintopic_get_endpoint_type_info(dds_builtintopic_endpoint_t *builtintopic_endpoint, const dds_typeinfo_t **type_info)
{
  // 检查参数是否为空
  if (builtintopic_endpoint == NULL || type_info == NULL)
    return DDS_RETCODE_BAD_PARAMETER;
  // 检查内置主题端点的QoS是否存在类型信息
  if (builtintopic_endpoint->qos && builtintopic_endpoint->qos->present & DDSI_QP_TYPE_INFORMATION)
    // 如果存在，则将类型信息赋值给type_info
    *type_info = builtintopic_endpoint->qos->type_information;
  else
    // 如果不存在，则将type_info赋值为NULL
    *type_info = NULL;
  // 返回执行结果
  return DDS_RETCODE_OK;
}
#endif

// 释放 DDS 内置主题的端点
void dds_builtintopic_free_endpoint(dds_builtintopic_endpoint_t *builtintopic_endpoint)
{
  // 删除端点的 QoS
  dds_delete_qos(builtintopic_endpoint->qos);
  // 释放端点的主题名称
  ddsrt_free(builtintopic_endpoint->topic_name);
  // 释放端点的类型名称
  ddsrt_free(builtintopic_endpoint->type_name);
  // 释放端点
  ddsrt_free(builtintopic_endpoint);
}

// 释放 DDS 内置主题的主题
void dds_builtintopic_free_topic(dds_builtintopic_topic_t *builtintopic_topic)
{
  // 删除主题的 QoS
  dds_delete_qos(builtintopic_topic->qos);
  // 释放主题的名称
  ddsrt_free(builtintopic_topic->topic_name);
  // 释放主题的类型名称
  ddsrt_free(builtintopic_topic->type_name);
  // 释放主题
  ddsrt_free(builtintopic_topic);
}

// 释放 DDS 内置主题的参与者
void dds_builtintopic_free_participant(dds_builtintopic_participant_t *builtintopic_participant)
{
  // 删除参与者的 QoS
  dds_delete_qos(builtintopic_participant->qos);
  // 释放参与者
  ddsrt_free(builtintopic_participant);
}
