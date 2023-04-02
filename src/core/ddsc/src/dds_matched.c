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

/**
 * @brief 获取与给定数据写入者匹配的订阅者实例句柄列表。
 *
 * @param[in] writer 数据写入者实体。
 * @param[out] rds 订阅者实例句柄数组，可以为NULL。
 * @param[in] nrds rds数组的大小，如果rds为NULL，则应为0。
 * @return 成功时返回DDS_RETCODE_OK，否则返回相应的错误代码。
 */
dds_return_t dds_get_matched_subscriptions(dds_entity_t writer,
                                           dds_instance_handle_t* rds,
                                           size_t nrds) {
  dds_writer* wr;
  dds_return_t rc;

  // 检查参数是否有效
  if ((rds != NULL && (nrds == 0 || nrds > INT32_MAX)) || (rds == NULL && nrds != 0))
    return DDS_RETCODE_BAD_PARAMETER;

  // 尝试锁定写入者实体
  if ((rc = dds_writer_lock(writer, &wr)) != DDS_RETCODE_OK)
    return rc;
  else {
    // 获取匹配的订阅者实例句柄列表
    rc = ddsi_writer_get_matched_subscriptions(wr->m_wr, rds, nrds);
    // 解锁写入者实体
    dds_writer_unlock(wr);
    return rc;
  }
}

/**
 * @brief 获取与给定数据读取者匹配的发布者实例句柄列表。
 *
 * @param[in] reader 数据读取者实体。
 * @param[out] wrs 发布者实例句柄数组，可以为NULL。
 * @param[in] nwrs wrs数组的大小，如果wrs为NULL，则应为0。
 * @return 成功时返回DDS_RETCODE_OK，否则返回相应的错误代码。
 */
dds_return_t dds_get_matched_publications(dds_entity_t reader,
                                          dds_instance_handle_t* wrs,
                                          size_t nwrs) {
  dds_reader* rd;
  dds_return_t rc;

  // 检查参数是否有效
  if ((wrs != NULL && (nwrs == 0 || nwrs > INT32_MAX)) || (wrs == NULL && nwrs != 0))
    return DDS_RETCODE_BAD_PARAMETER;

  // 尝试锁定读取者实体
  if ((rc = dds_reader_lock(reader, &rd)) != DDS_RETCODE_OK)
    return rc;
  else {
    // 获取匹配的发布者实例句柄列表
    rc = ddsi_reader_get_matched_publications(rd->m_rd, wrs, nwrs);
    // 解锁读取者实体
    dds_reader_unlock(rd);
    return rc;
  }
}

/**
 * @brief 创建一个内置主题端点对象
 *
 * @param guid 指向GUID的指针，用于设置内置主题端点的key
 * @param ppguid 指向参与者GUID的指针，用于设置内置主题端点的participant_key
 * @param ppiid 参与者实例句柄，用于设置内置主题端点的participant_instance_handle
 * @param qos 指向QoS的指针，用于设置内置主题端点的QoS属性
 * @return 返回创建的内置主题端点对象指针
 */
static dds_builtintopic_endpoint_t* make_builtintopic_endpoint(const ddsi_guid_t* guid,
                                                               const ddsi_guid_t* ppguid,
                                                               dds_instance_handle_t ppiid,
                                                               const dds_qos_t* qos) {
  dds_builtintopic_endpoint_t* ep;
  ddsi_guid_t tmp;
  ep = dds_alloc(sizeof(*ep));              // 分配内存空间给内置主题端点对象
  tmp = ddsi_hton_guid(*guid);              // 将GUID转换为网络字节序
  memcpy(&ep->key, &tmp, sizeof(ep->key));  // 复制转换后的GUID到内置主题端点的key
  ep->participant_instance_handle = ppiid;  // 设置参与者实例句柄
  tmp = ddsi_hton_guid(*ppguid);            // 将参与者GUID转换为网络字节序
  memcpy(&ep->participant_key, &tmp,
         sizeof(ep->participant_key));  // 复制转换后的参与者GUID到内置主题端点的participant_key
  ep->qos = dds_create_qos();  // 创建一个新的QoS对象
  ddsi_xqos_mergein_missing(ep->qos, qos,
                            ~(DDSI_QP_TOPIC_NAME | DDSI_QP_TYPE_NAME));  // 合并缺失的QoS属性
  ep->topic_name = dds_string_dup(qos->topic_name);                      // 复制主题名称
  ep->type_name = dds_string_dup(qos->type_name);                        // 复制类型名称
  return ep;  // 返回创建的内置主题端点对象指针
}

/**
 * @brief 获取匹配的订阅数据
 *
 * @param writer 写实体
 * @param ih 实例句柄
 * @return 返回匹配的订阅数据，如果没有找到匹配的订阅数据，则返回NULL
 */
dds_builtintopic_endpoint_t* dds_get_matched_subscription_data(dds_entity_t writer,
                                                               dds_instance_handle_t ih) {
  dds_writer* wr;
  if (dds_writer_lock(writer, &wr))  // 锁定写实体
    return NULL;

  dds_builtintopic_endpoint_t* ret = NULL;
  struct ddsi_entity_common* rdc;
  struct dds_qos* rdqos;
  struct ddsi_entity_common* ppc;

  // 线程在使用DDSI实体指针时必须处于"awake"状态
  struct ddsi_domaingv* const gv = &wr->m_entity.m_domain->gv;
  ddsi_thread_state_awake(ddsi_lookup_thread_state(), gv);
  if (ddsi_writer_find_matched_reader(wr->m_wr, ih, &rdc, &rdqos, &ppc))  // 查找匹配的读实体
    ret = make_builtintopic_endpoint(&rdc->guid, &ppc->guid, ppc->iid,
                                     rdqos);             // 创建内置主题端点对象
  ddsi_thread_state_asleep(ddsi_lookup_thread_state());  // 线程进入"asleep"状态

  dds_writer_unlock(wr);                                 // 解锁写实体
  return ret;                                            // 返回匹配的订阅数据
}

/**
 * @brief 获取与指定的 DataReader 匹配的 Publication 的内置主题数据。
 *
 * @param[in] reader DataReader 实体的句柄。
 * @param[in] ih 要获取数据的实例句柄。
 * @return 成功时返回 dds_builtintopic_endpoint_t 指针，失败时返回 NULL。
 */
dds_builtintopic_endpoint_t* dds_get_matched_publication_data(dds_entity_t reader,
                                                              dds_instance_handle_t ih) {
  // 定义一个 dds_reader 类型的指针
  dds_reader* rd;

  // 尝试锁定 DataReader，如果失败则返回 NULL
  if (dds_reader_lock(reader, &rd)) return NULL;

  // 定义一个 dds_builtintopic_endpoint_t 类型的指针，并初始化为 NULL
  dds_builtintopic_endpoint_t* ret = NULL;

  // 定义一些需要用到的结构体指针
  struct ddsi_entity_common* wrc;
  struct dds_qos* wrqos;
  struct ddsi_entity_common* ppc;

  // 在使用 DDSI 实体指针时，线程必须处于 "awake" 状态
  struct ddsi_domaingv* const gv = &rd->m_entity.m_domain->gv;
  ddsi_thread_state_awake(ddsi_lookup_thread_state(), gv);

  // 查找匹配的写入者，并创建内置主题端点
  if (ddsi_reader_find_matched_writer(rd->m_rd, ih, &wrc, &wrqos, &ppc))
    ret = make_builtintopic_endpoint(&wrc->guid, &ppc->guid, ppc->iid, wrqos);

  // 将线程状态设置为 "asleep"
  ddsi_thread_state_asleep(ddsi_lookup_thread_state());

  // 解锁 DataReader
  dds_reader_unlock(rd);

  // 返回结果
  return ret;
}

#ifdef DDS_HAS_TYPE_DISCOVERY
/**
 * @brief 获取内置主题端点的类型信息。
 *
 * @param[in] builtintopic_endpoint 内置主题端点指针。
 * @param[out] type_info 类型信息的指针。
 * @return 成功时返回 DDS_RETCODE_OK，失败时返回相应的错误代码。
 */
dds_return_t dds_builtintopic_get_endpoint_type_info(
    dds_builtintopic_endpoint_t* builtintopic_endpoint, const dds_typeinfo_t** type_info) {
  // 检查输入参数是否有效
  if (builtintopic_endpoint == NULL || type_info == NULL) return DDS_RETCODE_BAD_PARAMETER;

  // 如果存在类型信息，则将其赋值给输出参数
  if (builtintopic_endpoint->qos && builtintopic_endpoint->qos->present & DDSI_QP_TYPE_INFORMATION)
    *type_info = builtintopic_endpoint->qos->type_information;
  else
    *type_info = NULL;

  // 返回操作结果
  return DDS_RETCODE_OK;
}
#endif

/**
 * @brief 释放内置主题端点资源
 *
 * @param[in] builtintopic_endpoint 指向dds_builtintopic_endpoint_t结构的指针
 */
void dds_builtintopic_free_endpoint(dds_builtintopic_endpoint_t* builtintopic_endpoint) {
  dds_delete_qos(builtintopic_endpoint->qos);     // 删除QoS设置
  ddsrt_free(builtintopic_endpoint->topic_name);  // 释放主题名称内存
  ddsrt_free(builtintopic_endpoint->type_name);   // 释放类型名称内存
  ddsrt_free(builtintopic_endpoint);              // 释放内置主题端点结构内存
}

/**
 * @brief 释放内置主题资源
 *
 * @param[in] builtintopic_topic 指向dds_builtintopic_topic_t结构的指针
 */
void dds_builtintopic_free_topic(dds_builtintopic_topic_t* builtintopic_topic) {
  dds_delete_qos(builtintopic_topic->qos);     // 删除QoS设置
  ddsrt_free(builtintopic_topic->topic_name);  // 释放主题名称内存
  ddsrt_free(builtintopic_topic->type_name);   // 释放类型名称内存
  ddsrt_free(builtintopic_topic);              // 释放内置主题结构内存
}

/**
 * @brief 释放内置参与者资源
 *
 * @param[in] builtintopic_participant 指向dds_builtintopic_participant_t结构的指针
 */
void dds_builtintopic_free_participant(dds_builtintopic_participant_t* builtintopic_participant) {
  dds_delete_qos(builtintopic_participant->qos);  // 删除QoS设置
  ddsrt_free(builtintopic_participant);           // 释放内置参与者结构内存
}
