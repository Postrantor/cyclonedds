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
#ifndef DDSI_TOPIC_H
#define DDSI_TOPIC_H

#include "dds/ddsi/ddsi_domaingv.h"
#include "dds/ddsi/ddsi_entity.h"
#include "dds/ddsi/ddsi_typelib.h"
#include "dds/export.h"
#include "dds/features.h"

#if defined(__cplusplus)
extern "C" {
#endif

/** @component ddsi_topic */
// 声明一个函数，判断主题是否为内置主题
// Declare a function to determine if the topic is a built-in topic
int ddsi_is_builtin_topic(ddsi_entityid_t id, ddsi_vendorid_t vendorid);

#ifdef DDS_HAS_TOPIC_DISCOVERY

// 声明一个代理参与者结构体
// Declare a proxy participant structure
struct ddsi_proxy_participant;

// 声明一个类型对结构体
// Declare a type pair structure
struct ddsi_type_pair;

// 声明一个 QoS 结构体
// Declare a QoS structure
struct dds_qos;

// 定义一个主题定义结构体
// Define a topic definition structure
struct ddsi_topic_definition {
  unsigned char key[16];            /* 主题定义的键（类型ID和QoS的MD5哈希值）
                                     /* Key for this topic definition (MD5 hash of the type_id and qos) */
  struct ddsi_type_pair* type_pair; /* 包含最小和完整类型的 ddsi_type 对象，
                                     /* Has a ddsi_type object for the minimal and complete type,
                                       其中包含 XTypes 类型标识符 */
  which contains the XTypes type identifiers* / struct dds_qos* xqos; /* 包含主题名称和类型名称 */
  /* Contains also the topic name and type name */
  uint32_t refc;
  struct ddsi_domaingv* gv;
};

// 定义一个 ddsi 主题结构体
// Define a ddsi topic structure
struct ddsi_topic {
  struct ddsi_entity_common e;
  struct ddsi_topic_definition* definition; /* 指向（共享的）主题定义的引用，受 e.qos_lock 保护 */
  /* Ref to (shared) topic definition, protected by e.qos_lock */
  struct ddsi_participant* pp; /* 参与者的反向引用 */
                               /* Backref to the participant */
};

// 定义一个代理主题结构体
// Define a proxy topic structure
struct ddsi_proxy_topic {
  ddsi_entityid_t entityid;
  struct ddsi_topic_definition* definition; /* 指向（共享的）主题定义的引用 */
                                            /* Ref to (shared) topic definition */
  ddsrt_wctime_t tupdate;                   /* 最后更新的时间戳 */
                                            /* Timestamp of last update */
  ddsi_seqno_t seq;                         /* 最近 SEDP 消息的序列号 */
                                            /* Sequence number of most recent SEDP message */
  ddsrt_avl_node_t avlnode;                 /* 在 proxypp->topics 中的条目 */
                                            /* Entry in proxypp->topics */
  unsigned deleted : 1;
};

/** @component ddsi_topic */
/**
 * @brief 创建一个新的主题 (Create a new topic)
 *
 * @param[out] tp_out        主题指针的输出参数 (Output parameter for the topic pointer)
 * @param[in]  tpguid        主题的全局唯一标识符 (The globally unique identifier for the topic)
 * @param[in]  pp            参与者指针 (Pointer to the participant)
 * @param[in]  topic_name    主题的名称 (The name of the topic)
 * @param[in]  type          主题的数据类型 (The data type of the topic)
 * @param[in]  xqos          主题的质量服务 (Quality of Service for the topic)
 * @param[in]  is_builtin    是否为内置主题 (Whether the topic is built-in or not)
 * @param[out] new_topic_def 新主题定义的输出参数 (Output parameter for the new topic definition)
 *
 * @returns 成功时返回 DDS_RETCODE_OK，失败时返回相应的错误代码 (Returns DDS_RETCODE_OK on success,
 * appropriate error code on failure)
 */
dds_return_t ddsi_new_topic(struct ddsi_topic** tp_out,
                            struct ddsi_guid* tpguid,
                            struct ddsi_participant* pp,
                            const char* topic_name,
                            const struct ddsi_sertype* type,
                            const struct dds_qos* xqos,
                            bool is_builtin,
                            bool* new_topic_def);

/** @component ddsi_topic */
/**
 * @brief 更新主题的质量服务 (Update the Quality of Service of a topic)
 *
 * @param[in,out] tp   要更新的主题指针 (Pointer to the topic to update)
 * @param[in]     xqos 新的质量服务设置 (New Quality of Service settings)
 */
void ddsi_update_topic_qos(struct ddsi_topic* tp, const dds_qos_t* xqos);

/** @component ddsi_topic */
/**
 * @brief 删除主题 (Delete a topic)
 *
 * @param[in] gv   域全局变量指针 (Pointer to the domain global variables)
 * @param[in] guid 要删除的主题的全局唯一标识符 (Globally unique identifier of the topic to delete)
 *
 * @returns 成功时返回 DDS_RETCODE_OK，失败时返回相应的错误代码 (Returns DDS_RETCODE_OK on success,
 * appropriate error code on failure)
 */
dds_return_t ddsi_delete_topic(struct ddsi_domaingv* gv, const struct ddsi_guid* guid);

/** @component ddsi_topic */
/**
 * @brief 查找主题定义 (Lookup a topic definition)
 *
 * @param[in]  gv         域全局变量指针 (Pointer to the domain global variables)
 * @param[in]  topic_name 要查找的主题的名称 (The name of the topic to lookup)
 * @param[in]  type_id    要查找的主题的类型标识符 (Type identifier of the topic to lookup)
 * @param[out] tpd        主题定义指针的输出参数 (Output parameter for the topic definition pointer)
 *
 * @returns 成功时返回 DDS_RETCODE_OK，失败时返回相应的错误代码 (Returns DDS_RETCODE_OK on success,
 * appropriate error code on failure)
 */
dds_return_t ddsi_lookup_topic_definition(struct ddsi_domaingv* gv,
                                          const char* topic_name,
                                          const ddsi_typeid_t* type_id,
                                          struct ddsi_topic_definition** tpd);

#endif /* DDS_HAS_TOPIC_DISCOVERY */

#if defined(__cplusplus)
}
#endif

#endif /* DDSI_TOPIC_H */
