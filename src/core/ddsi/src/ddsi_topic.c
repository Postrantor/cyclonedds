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
#include <stddef.h>
#include <string.h>

#include "dds/dds.h"
#include "dds/ddsi/ddsi_builtin_topic_if.h"
#include "dds/ddsi/ddsi_domaingv.h"
#include "dds/ddsi/ddsi_proxy_participant.h"
#include "dds/ddsrt/heap.h"
#include "dds/ddsrt/md5.h"
#include "ddsi__discovery.h"
#include "ddsi__entity.h"
#include "ddsi__entity_index.h"
#include "ddsi__gc.h"
#include "ddsi__misc.h"
#include "ddsi__participant.h"
#include "ddsi__topic.h"
#include "ddsi__typelib.h"
#include "ddsi__vendor.h"
#include "ddsi__xmsg.h"
#include "ddsi__xqos.h"

#ifdef DDS_HAS_TOPIC_DISCOVERY

/**
 * @brief 在锁定状态下引用主题定义
 * @param[in] gv 指向ddsi_domaingv结构体的指针
 * @param[in] sertype 指向ddsi_sertype结构体的指针
 * @param[in] type_id 指向ddsi_typeid_t类型的指针
 * @param[in] qos 指向dds_qos结构体的指针
 * @param[out] is_new 布尔值指针，表示是否为新创建的主题定义
 * @return 返回一个指向ddsi_topic_definition结构体的指针
 *
 * @brief Reference a topic definition under locked state
 * @param[in] gv Pointer to ddsi_domaingv structure
 * @param[in] sertype Pointer to ddsi_sertype structure
 * @param[in] type_id Pointer to ddsi_typeid_t type
 * @param[in] qos Pointer to dds_qos structure
 * @param[out] is_new Boolean pointer indicating whether it's a newly created topic definition
 * @return Returns a pointer to the ddsi_topic_definition structure
 */
static struct ddsi_topic_definition *ref_topic_definition_locked(struct ddsi_domaingv *gv,
                                                                 const struct ddsi_sertype *sertype,
                                                                 const ddsi_typeid_t *type_id,
                                                                 struct dds_qos *qos,
                                                                 bool *is_new);

/**
 * @brief 引用主题定义
 * @param[in] gv 指向ddsi_domaingv结构体的指针
 * @param[in] sertype 指向ddsi_sertype结构体的指针
 * @param[in] type_id 指向ddsi_typeid_t类型的指针
 * @param[in] qos 指向dds_qos结构体的指针
 * @param[out] is_new 布尔值指针，表示是否为新创建的主题定义
 * @return 返回一个指向ddsi_topic_definition结构体的指针
 *
 * @brief Reference a topic definition
 * @param[in] gv Pointer to ddsi_domaingv structure
 * @param[in] sertype Pointer to ddsi_sertype structure
 * @param[in] type_id Pointer to ddsi_typeid_t type
 * @param[in] qos Pointer to dds_qos structure
 * @param[out] is_new Boolean pointer indicating whether it's a newly created topic definition
 * @return Returns a pointer to the ddsi_topic_definition structure
 */
static struct ddsi_topic_definition *ref_topic_definition(struct ddsi_domaingv *gv,
                                                          const struct ddsi_sertype *sertype,
                                                          const ddsi_typeid_t *type_id,
                                                          struct dds_qos *qos,
                                                          bool *is_new);

/**
 * @brief 在锁定状态下取消引用主题定义
 * @param[in] tpd 指向ddsi_topic_definition结构体的指针
 * @param[in] timestamp 时间戳
 *
 * @brief Unreference a topic definition under locked state
 * @param[in] tpd Pointer to ddsi_topic_definition structure
 * @param[in] timestamp Timestamp
 */
static void unref_topic_definition_locked(struct ddsi_topic_definition *tpd,
                                          ddsrt_wctime_t timestamp);

/**
 * @brief 取消引用主题定义
 * @param[in] gv 指向ddsi_domaingv结构体的指针
 * @param[in] tpd 指向ddsi_topic_definition结构体的指针
 * @param[in] timestamp 时间戳
 *
 * @brief Unreference a topic definition
 * @param[in] gv Pointer to ddsi_domaingv structure
 * @param[in] tpd Pointer to ddsi_topic_definition structure
 * @param[in] timestamp Timestamp
 */
static void unref_topic_definition(struct ddsi_domaingv *gv,
                                   struct ddsi_topic_definition *tpd,
                                   ddsrt_wctime_t timestamp);

/**
 * @brief 创建新的主题定义
 * @param[in] gv 指向ddsi_domaingv结构体的指针
 * @param[in] type 指向ddsi_sertype结构体的指针
 * @param[in] qos 指向dds_qos结构体的指针
 * @return 返回一个指向ddsi_topic_definition结构体的指针
 *
 * @brief Create a new topic definition
 * @param[in] gv Pointer to ddsi_domaingv structure
 * @param[in] type Pointer to ddsi_sertype structure
 * @param[in] qos Pointer to dds_qos structure
 * @return Returns a pointer to the ddsi_topic_definition structure
 */
static struct ddsi_topic_definition *new_topic_definition(struct ddsi_domaingv *gv,
                                                          const struct ddsi_sertype *type,
                                                          const struct dds_qos *qos);

#endif /* DDS_HAS_TOPIC_DISCOVERY */

/**
 * @brief gc_proxy_tp 结构体定义
 */
struct gc_proxy_tp {
  struct ddsi_proxy_participant *proxypp; /**< 指向ddsi_proxy_participant结构体的指针 */
  struct ddsi_proxy_topic *proxytp;       /**< 指向ddsi_proxy_topic结构体的指针 */
  ddsrt_wctime_t timestamp;               /**< 时间戳 */
};

/**
 * @brief gc_tpd 结构体定义
 */
struct gc_tpd {
  struct ddsi_topic_definition *tpd; /**< 指向ddsi_topic_definition结构体的指针 */
  ddsrt_wctime_t timestamp;          /**< 时间戳 */
};

/**
 * @brief 判断是否为内置主题
 * @param[in] id 实体ID
 * @param[in] vendorid 厂商ID
 * @return 返回1表示是内置主题，返回0表示不是
 *
 * @brief Check if it's a built-in topic
 * @param[in] id Entity ID
 * @param[in] vendorid Vendor ID
 * @return Returns 1 if it's a built-in topic, 0 otherwise
 */
int ddsi_is_builtin_topic(ddsi_entityid_t id, ddsi_vendorid_t vendorid) {
  return ddsi_is_builtin_entityid(id, vendorid) && ddsi_is_topic_entityid(id);
}

/**
 * @brief 判断实体ID是否为主题实体ID
 * @param[in] id 实体ID
 * @return 返回1表示是主题实体ID，返回0表示不是
 *
 * @brief Check if the entity ID is a topic entity ID
 * @param[in] id Entity ID
 * @return Returns 1 if it's a topic entity ID, 0 otherwise
 */
int ddsi_is_topic_entityid(ddsi_entityid_t id) {
  switch (id.u & DDSI_ENTITYID_KIND_MASK) {
    case DDSI_ENTITYID_KIND_CYCLONE_TOPIC_BUILTIN:
    case DDSI_ENTITYID_KIND_CYCLONE_TOPIC_USER:
      return 1;
    default:
      return 0;
  }
}

#ifdef DDS_HAS_TOPIC_DISCOVERY

/* TOPIC -------------------------------------------------------- */

/**
 * @brief 创建一个新的主题 (Create a new topic)
 *
 * @param[out] tp_out 主题指针的输出参数 (Output parameter for the topic pointer)
 * @param[in,out] tpguid 输入参与者GUID，输出主题GUID (Input participant GUID, output topic GUID)
 * @param[in] pp 参与者实例 (Participant instance)
 * @param[in] topic_name 主题名称 (Topic name)
 * @param[in] sertype 序列化类型 (Serialization type)
 * @param[in] xqos 质量服务设置 (Quality of Service settings)
 * @param[in] is_builtin 是否为内置主题 (Whether it is a built-in topic)
 * @param[out] new_topic_def 新主题定义的输出参数 (Output parameter for the new topic definition)
 * @return dds_return_t 成功返回0，否则返回错误代码 (Returns 0 on success, otherwise returns an
 * error code)
 */
dds_return_t ddsi_new_topic(struct ddsi_topic **tp_out,
                            struct ddsi_guid *tpguid,
                            struct ddsi_participant *pp,
                            const char *topic_name,
                            const struct ddsi_sertype *sertype,
                            const struct dds_qos *xqos,
                            bool is_builtin,
                            bool *new_topic_def) {
  // 定义返回值变量 (Define return value variable)
  dds_return_t rc;
  // 获取当前时间戳 (Get the current timestamp)
  ddsrt_wctime_t timestamp = ddsrt_time_wallclock();
  // 获取参与者所属的域全局变量 (Get the domain global variables to which the participant belongs)
  struct ddsi_domaingv *gv = pp->e.gv;
  // 设置主题GUID的前缀为参与者GUID的前缀 (Set the prefix of the topic GUID to the prefix of the
  // participant GUID)
  tpguid->prefix = pp->e.guid.prefix;
  // 分配实体ID (Allocate entity ID)
  if ((rc =
           ddsi_participant_allocate_entityid(&tpguid->entityid,
                                              (is_builtin ? DDSI_ENTITYID_KIND_CYCLONE_TOPIC_BUILTIN
                                                          : DDSI_ENTITYID_KIND_CYCLONE_TOPIC_USER) |
                                                  DDSI_ENTITYID_SOURCE_VENDOR,
                                              pp)) < 0)
    return rc;
  // 确保查找到的主题GUID不存在 (Ensure that the found topic GUID does not exist)
  assert(ddsi_entidx_lookup_topic_guid(gv->entity_index, tpguid) == NULL);

  // 为新主题分配内存空间 (Allocate memory space for the new topic)
  struct ddsi_topic *tp = ddsrt_malloc(sizeof(*tp));
  // 如果提供了tp_out参数，则将新主题指针赋值给它 (If the tp_out parameter is provided, assign the
  // new topic pointer to it)
  if (tp_out) *tp_out = tp;
  // 初始化实体通用部分 (Initialize the common part of the entity)
  ddsi_entity_common_init(&tp->e, gv, tpguid, DDSI_EK_TOPIC, timestamp, DDSI_VENDORID_ECLIPSE,
                          pp->e.onlylocal);
  // 引用参与者并设置主题的参与者指针 (Reference the participant and set the participant pointer of
  // the topic)
  tp->pp = ddsi_ref_participant(pp, &tp->e.guid);

  // 复制QoS，合并默认值 (Copy QoS, merging in defaults)
  struct dds_qos *tp_qos = ddsrt_malloc(sizeof(*tp_qos));
  ddsi_xqos_copy(tp_qos, xqos);
  ddsi_xqos_mergein_missing(tp_qos, &ddsi_default_qos_topic, ~(uint64_t)0);
  // 确保QoS未设置别名 (Ensure that the QoS has not set an alias)
  assert(tp_qos->aliased == 0);

  // 在QoS中设置主题名称、类型名称和类型信息 (Set topic name, type name and type information in QoS)
  tp_qos->present |= DDSI_QP_TYPE_INFORMATION;
  tp_qos->type_information = ddsi_sertype_typeinfo(sertype);
  // 确保类型信息存在 (Ensure that the type information exists)
  assert(tp_qos->type_information);
  ddsi_set_topic_type_name(tp_qos, topic_name, sertype->type_name);

  // 如果启用了发现日志，则记录主题和QoS信息 (If discovery logging is enabled, log the topic and QoS
  // information)
  if (gv->logconfig.c.mask & DDS_LC_DISCOVERY) {
    ELOGDISC(tp, "TOPIC " PGUIDFMT " QOS={", PGUID(tp->e.guid));
    ddsi_xqos_log(DDS_LC_DISCOVERY, &gv->logconfig, tp_qos);
    ELOGDISC(tp, "}\n");
  }
  // 引用主题定义并设置新主题的定义 (Reference the topic definition and set the definition of the
  // new topic)
  tp->definition = ref_topic_definition(
      gv, sertype, ddsi_typeinfo_complete_typeid(tp_qos->type_information), tp_qos, new_topic_def);
  // 确保主题定义存在 (Ensure that the topic definition exists)
  assert(tp->definition);
  // 如果是新的主题定义，则写入内置主题 (If it is a new topic definition, write the built-in topic)
  if (new_topic_def)
    ddsi_builtintopic_write_topic(gv->builtin_topic_interface, tp->definition, timestamp, true);
  // 清理QoS并释放内存空间 (Clean up QoS and free memory space)
  ddsi_xqos_fini(tp_qos);
  ddsrt_free(tp_qos);

  // 对实体进行加锁 (Lock the entity)
  ddsrt_mutex_lock(&tp->e.lock);
  // 将主题插入到实体索引中 (Insert the topic into the entity index)
  ddsi_entidx_insert_topic_guid(gv->entity_index, tp);
  // 写入SEDP主题 (Write the SEDP topic)
  (void)ddsi_sedp_write_topic(tp, true);
  // 解锁实体 (Unlock the entity)
  ddsrt_mutex_unlock(&tp->e.lock);
  // 返回成功 (Return success)
  return 0;
}

/**
 * @brief 更新主题的QoS设置
 *
 * 该函数用于更新给定主题的QoS设置。在更新过程中，不会创建新的主题实例。
 *
 * @param[in] tp 要更新QoS设置的主题指针
 * @param[in] xqos 新的QoS设置
 */
void ddsi_update_topic_qos(struct ddsi_topic *tp, const dds_qos_t *xqos) {
  // 更新主题QoS，这意味着替换主题定义，但不会产生新的主题实例
  // Update the topic QoS, which means replacing the topic definition for a topic,
  // does not result in a new topic in the context of the find topic api.

  struct ddsi_domaingv *gv = tp->e.gv;
  ddsrt_mutex_lock(&tp->e.lock);
  ddsrt_mutex_lock(&tp->e.qos_lock);
  struct ddsi_topic_definition *tpd = tp->definition;
  uint64_t mask =
      ddsi_xqos_delta(tpd->xqos, xqos,
                      DDSI_QP_CHANGEABLE_MASK & ~(DDSI_QP_RXO_MASK | DDSI_QP_PARTITION)) &
      xqos->present;
  GVLOGDISC("ddsi_update_topic_qos " PGUIDFMT " delta=%" PRIu64 " QOS={", PGUID(tp->e.guid), mask);
  ddsi_xqos_log(DDS_LC_DISCOVERY, &gv->logconfig, xqos);
  GVLOGDISC("}\n");
  if (mask == 0) {
    ddsrt_mutex_unlock(&tp->e.qos_lock);
    ddsrt_mutex_unlock(&tp->e.lock);
    return;  // 没有变化，或者不支持的变化
  }

  bool new_tpd = false;
  dds_qos_t *newqos = dds_create_qos();
  ddsi_xqos_mergein_missing(newqos, xqos, mask);
  ddsi_xqos_mergein_missing(newqos, tpd->xqos, ~(uint64_t)0);
  ddsrt_mutex_lock(&gv->topic_defs_lock);
  tp->definition = ref_topic_definition_locked(gv, NULL, ddsi_type_pair_complete_id(tpd->type_pair),
                                               newqos, &new_tpd);
  assert(tp->definition);
  unref_topic_definition_locked(tpd, ddsrt_time_wallclock());
  ddsrt_mutex_unlock(&gv->topic_defs_lock);
  if (new_tpd)
    ddsi_builtintopic_write_topic(gv->builtin_topic_interface, tp->definition,
                                  ddsrt_time_wallclock(), true);
  ddsrt_mutex_unlock(&tp->e.qos_lock);
  (void)ddsi_sedp_write_topic(tp, true);
  ddsrt_mutex_unlock(&tp->e.lock);
  dds_delete_qos(newqos);
}

/**
 * @brief 删除主题的垃圾收集函数
 *
 * 该函数用于删除给定的主题实例，并释放相关资源。
 *
 * @param[in] gcreq 垃圾收集请求指针
 */
static void gc_delete_topic(struct ddsi_gcreq *gcreq) {
  struct ddsi_topic *tp = gcreq->arg;
  ELOGDISC(tp, "gc_delete_topic (%p, " PGUIDFMT ")\n", (void *)gcreq, PGUID(tp->e.guid));
  ddsi_gcreq_free(gcreq);
  if (!ddsi_is_builtin_entityid(tp->e.guid.entityid, DDSI_VENDORID_ECLIPSE))
    (void)ddsi_sedp_write_topic(tp, false);
  ddsi_entity_common_fini(&tp->e);
  unref_topic_definition(tp->e.gv, tp->definition, ddsrt_time_wallclock());
  ddsi_unref_participant(tp->pp, &tp->e.guid);
  ddsrt_free(tp);
}

/**
 * @brief 删除主题的垃圾回收请求 (Garbage collection request for deleting a topic)
 * @param tp 要删除的主题指针 (Pointer to the topic to be deleted)
 * @return 总是返回0 (Always returns 0)
 */
static int gcreq_topic(struct ddsi_topic *tp) {
  // 创建一个新的垃圾回收请求，将 gc_delete_topic 函数作为回调函数 (Create a new garbage collection
  // request with gc_delete_topic as the callback function)
  struct ddsi_gcreq *gcreq = ddsi_gcreq_new(tp->e.gv->gcreq_queue, gc_delete_topic);

  // 将要删除的主题设置为垃圾回收请求的参数 (Set the topic to be deleted as the argument of the
  // garbage collection request)
  gcreq->arg = tp;

  // 将垃圾回收请求加入队列 (Enqueue the garbage collection request)
  ddsi_gcreq_enqueue(gcreq);

  return 0;
}

/**
 * @brief 删除指定 GUID 的主题 (Delete a topic with the specified GUID)
 * @param gv 域全局变量指针 (Pointer to domain global variables)
 * @param guid 要删除的主题的 GUID (GUID of the topic to be deleted)
 * @return 成功时返回0，否则返回 DDS_RETCODE_BAD_PARAMETER (Returns 0 on success, otherwise returns
 * DDS_RETCODE_BAD_PARAMETER)
 */
dds_return_t ddsi_delete_topic(struct ddsi_domaingv *gv, const struct ddsi_guid *guid) {
  struct ddsi_topic *tp;

  // 确保 GUID 的实体 ID 是一个主题 (Ensure the entity ID of the GUID is a topic)
  assert(ddsi_is_topic_entityid(guid->entityid));

  // 在实体索引中查找指定 GUID 的主题 (Lookup the topic with the specified GUID in the entity index)
  if ((tp = ddsi_entidx_lookup_topic_guid(gv->entity_index, guid)) == NULL) {
    // 如果找不到主题，打印警告并返回错误代码 (If the topic is not found, print a warning and return
    // an error code)
    GVLOGDISC("ddsi_delete_topic (guid " PGUIDFMT ") - unknown guid\n", PGUID(*guid));
    return DDS_RETCODE_BAD_PARAMETER;
  }

  // 打印删除主题的调试信息 (Print debug information for deleting the topic)
  GVLOGDISC("ddsi_delete_topic (guid " PGUIDFMT ") ...\n", PGUID(*guid));

  // 从实体索引中移除主题 (Remove the topic from the entity index)
  ddsi_entidx_remove_topic_guid(gv->entity_index, tp);

  // 调用垃圾回收请求以删除主题 (Invoke garbage collection request to delete the topic)
  gcreq_topic(tp);

  return 0;
}

/* TOPIC DEFINITION ---------------------------------------------- */

/**
 * @brief 删除主题定义的垃圾回收函数 (Garbage collection function for deleting topic definition)
 *
 * @param[in] gcreq 垃圾回收请求指针 (Pointer to garbage collection request)
 */
static void gc_delete_topic_definition(struct ddsi_gcreq *gcreq) {
  // 获取垃圾回收数据 (Get garbage collection data)
  struct gc_tpd *gcdata = gcreq->arg;
  // 获取主题定义 (Get topic definition)
  struct ddsi_topic_definition *tpd = gcdata->tpd;
  // 获取域全局变量 (Get domain global variables)
  struct ddsi_domaingv *gv = tpd->gv;

  // 记录日志 (Log the message)
  GVLOGDISC("gcreq_delete_topic_definition(%p)\n", (void *)gcreq);

  // 写入内置主题 (Write built-in topic)
  ddsi_builtintopic_write_topic(gv->builtin_topic_interface, tpd, gcdata->timestamp, false);

  // 如果存在类型对 (If type pair exists)
  if (tpd->type_pair) {
    // 取消引用最小类型 (Unreference minimal type)
    ddsi_type_unref(gv, tpd->type_pair->minimal);
    // 取消引用完整类型 (Unreference complete type)
    ddsi_type_unref(gv, tpd->type_pair->complete);
    // 释放类型对内存 (Free type pair memory)
    ddsrt_free(tpd->type_pair);
  }

  // 清理主题质量属性 (Clean up topic quality of service attributes)
  ddsi_xqos_fini(tpd->xqos);
  // 释放主题质量属性内存 (Free topic quality of service memory)
  ddsrt_free(tpd->xqos);
  // 释放主题定义内存 (Free topic definition memory)
  ddsrt_free(tpd);
  // 释放垃圾回收数据内存 (Free garbage collection data memory)
  ddsrt_free(gcdata);
  // 释放垃圾回收请求内存 (Free garbage collection request memory)
  ddsi_gcreq_free(gcreq);
}

/**
 * @brief 创建一个用于删除主题定义的垃圾回收请求 (Create a garbage collection request for deleting a
 * topic definition)
 *
 * @param[in] tpd 主题定义指针 (Pointer to topic definition)
 * @param[in] timestamp 时间戳 (Timestamp)
 * @return int 返回值为0 (Return value is 0)
 */
static int gcreq_topic_definition(struct ddsi_topic_definition *tpd, ddsrt_wctime_t timestamp) {
  // 创建垃圾回收请求 (Create garbage collection request)
  struct ddsi_gcreq *gcreq = ddsi_gcreq_new(tpd->gv->gcreq_queue, gc_delete_topic_definition);
  // 分配垃圾回收数据内存 (Allocate garbage collection data memory)
  struct gc_tpd *gcdata = ddsrt_malloc(sizeof(*gcdata));
  // 设置主题定义和时间戳 (Set topic definition and timestamp)
  gcdata->tpd = tpd;
  gcdata->timestamp = timestamp;
  // 设置垃圾回收请求参数 (Set garbage collection request argument)
  gcreq->arg = gcdata;
  // 将垃圾回收请求加入队列 (Enqueue garbage collection request)
  ddsi_gcreq_enqueue(gcreq);
  return 0;
}

/**
 * @brief 在锁定状态下删除主题定义 (Delete topic definition in locked state)
 *
 * @param[in] tpd 主题定义指针 (Pointer to topic definition)
 * @param[in] timestamp 时间戳 (Timestamp)
 */
static void delete_topic_definition_locked(struct ddsi_topic_definition *tpd,
                                           ddsrt_wctime_t timestamp) {
  // 获取域全局变量 (Get domain global variables)
  struct ddsi_domaingv *gv = tpd->gv;

  // 记录日志 (Log the message)
  GVLOGDISC("delete_topic_definition_locked (%p) ", tpd);

  // 从哈希表中移除主题定义 (Remove topic definition from hash table)
  ddsrt_hh_remove_present(gv->topic_defs, tpd);

  // 记录日志 (Log the message)
  GVLOGDISC("- deleting\n");

  // 创建垃圾回收请求 (Create garbage collection request)
  gcreq_topic_definition(tpd, timestamp);
}

/**
 * @brief 计算主题定义的哈希值 (Calculate the hash value of the topic definition)
 *
 * @param[in] tpd 主题定义指针 (Pointer to topic definition)
 * @return uint32_t 哈希值 (Hash value)
 */
uint32_t ddsi_topic_definition_hash(const struct ddsi_topic_definition *tpd) {
  // 断言主题定义不为空 (Assert that topic definition is not NULL)
  assert(tpd != NULL);
  // 返回哈希值 (Return hash value)
  return *(uint32_t *)tpd->key;
}

/**
 * @brief 设置 DDSI 主题定义的哈希值 (Set the hash value for the DDSI topic definition)
 *
 * @param[in,out] tpd 指向 ddsi_topic_definition 结构体的指针 (Pointer to the ddsi_topic_definition
 * structure)
 */
static void set_ddsi_topic_definition_hash(struct ddsi_topic_definition *tpd) {
  // 获取完整的类型标识符 (Get the complete type identifier)
  const ddsi_typeid_t *tid_complete = ddsi_type_pair_complete_id(tpd->type_pair);

  // 确保类型标识符不为空 (Ensure the type identifier is not empty)
  assert(!ddsi_typeid_is_none(tid_complete));

  // 确保 QoS 不为空 (Ensure QoS is not empty)
  assert(tpd->xqos != NULL);

  // 初始化 MD5 状态 (Initialize MD5 state)
  ddsrt_md5_state_t md5st;
  ddsrt_md5_init(&md5st);

  // 将类型 ID 添加到密钥中 (Add type ID to the key)
  unsigned char *buf = NULL;
  uint32_t sz = 0;
  ddsi_typeid_ser(tid_complete, &buf, &sz);

  // 确保序列化后的大小和缓冲区不为空 (Ensure serialized size and buffer are not empty)
  assert(sz && buf);

  // 将序列化的类型 ID 添加到 MD5 哈希中 (Append the serialized type ID to the MD5 hash)
  ddsrt_md5_append(&md5st, (ddsrt_md5_byte_t *)buf, sz);
  ddsrt_free(buf);

  // 将序列化的 QoS 作为密钥的一部分添加 (Add serialized QoS as part of the key)
  // QoS 的 type_information 字段不包括在内，因为该字段可能包含依赖类型 ID 列表，
  // 因此对于相等的类型定义可能会有所不同
  // (The type_information field of the QoS is not included, as this field may contain a list of
  // dependent type IDs and therefore may be different for equal type definitions)
  struct ddsi_xmsg *mqos =
      ddsi_xmsg_new(tpd->gv->xmsgpool, &ddsi_nullguid, NULL, 0, DDSI_XMSG_KIND_DATA);
  ddsi_xqos_addtomsg(mqos, tpd->xqos, ~(DDSI_QP_TYPE_INFORMATION), DDSI_PLIST_CONTEXT_TOPIC);

  // 获取序列化后的 QoS 大小和内容 (Get serialized QoS size and content)
  size_t sqos_sz;
  void *sqos = ddsi_xmsg_payload(&sqos_sz, mqos);

  // 确保序列化后的 QoS 大小不超过 UINT32_MAX (Ensure serialized QoS size does not exceed
  // UINT32_MAX)
  assert(sqos_sz <= UINT32_MAX);

  // 将序列化的 QoS 添加到 MD5 哈希中 (Append the serialized QoS to the MD5 hash)
  ddsrt_md5_append(&md5st, (ddsrt_md5_byte_t *)sqos, (uint32_t)sqos_sz);
  ddsi_xmsg_free(mqos);

  // 完成 MD5 计算并将结果存储在主题定义的 key 中 (Finish the MD5 computation and store the result
  // in the topic definition's key)
  ddsrt_md5_finish(&md5st, (ddsrt_md5_byte_t *)&tpd->key);
}

/**
 * @brief 在锁定状态下引用主题定义
 * @param[in] gv 指向ddsi_domaingv结构的指针，表示Cyclone DDS域全局变量
 * @param[in] sertype 指向ddsi_sertype结构的指针，表示序列化类型
 * @param[in] type_id 指向ddsi_typeid_t结构的指针，表示类型标识符
 * @param[in] qos 指向dds_qos结构的指针，表示质量保证设置
 * @param[out] is_new 布尔值指针，表示是否为新创建的主题定义
 * @return 返回指向ddsi_topic_definition结构的指针
 *
 * @brief Reference a topic definition while locked
 * @param[in] gv Pointer to ddsi_domaingv structure, representing Cyclone DDS domain global
 * variables
 * @param[in] sertype Pointer to ddsi_sertype structure, representing the serialization type
 * @param[in] type_id Pointer to ddsi_typeid_t structure, representing the type identifier
 * @param[in] qos Pointer to dds_qos structure, representing quality of service settings
 * @param[out] is_new Boolean pointer, indicating whether it is a newly created topic definition
 * @return Returns a pointer to the ddsi_topic_definition structure
 */
static struct ddsi_topic_definition *ref_topic_definition_locked(struct ddsi_domaingv *gv,
                                                                 const struct ddsi_sertype *sertype,
                                                                 const ddsi_typeid_t *type_id,
                                                                 struct dds_qos *qos,
                                                                 bool *is_new) {
  // 定义两个类型标识符指针：最小类型标识符和完整类型标识符
  // Define two type identifier pointers: minimal type identifier and complete type identifier
  const ddsi_typeid_t *type_id_minimal = NULL, *type_id_complete = NULL;

  // 判断类型标识符是否为最小类型标识符，并相应地设置指针
  // Determine if the type identifier is a minimal type identifier and set the pointer accordingly
  if (ddsi_typeid_is_minimal(type_id))
    type_id_minimal = type_id;
  else
    type_id_complete = type_id;

  // 初始化一个主题定义模板，包括质量保证设置、类型对和全局变量
  // Initialize a topic definition template, including quality of service settings, type pair, and
  // global variables
  struct ddsi_topic_definition templ = {
      .xqos = qos, .type_pair = ddsi_type_pair_init(type_id_minimal, type_id_complete), .gv = gv};

  // 设置主题定义模板的哈希值
  // Set the hash value of the topic definition template
  set_ddsi_topic_definition_hash(&templ);

  // 在全局变量的主题定义中查找模板
  // Look up the template in the global variable's topic definitions
  struct ddsi_topic_definition *tpd = ddsrt_hh_lookup(gv->topic_defs, &templ);

  // 释放类型对内存
  // Free the memory of the type pair
  ddsi_type_pair_free(templ.type_pair);

  // 如果找到了主题定义，则增加引用计数并设置is_new为false
  // If the topic definition is found, increase the reference count and set is_new to false
  if (tpd) {
    tpd->refc++;
    *is_new = false;
  } else {
    // 如果没有找到主题定义，则创建一个新的主题定义并设置is_new为true
    // If the topic definition is not found, create a new topic definition and set is_new to true
    tpd = new_topic_definition(gv, sertype, qos);
    if (tpd) *is_new = true;
  }

  // 返回主题定义指针
  // Return the topic definition pointer
  return tpd;
}

/**
 * @brief 引用主题定义
 * @param[in] gv 指向ddsi_domaingv结构的指针，表示Cyclone DDS域全局变量
 * @param[in] sertype 指向ddsi_sertype结构的指针，表示序列化类型
 * @param[in] type_id 指向ddsi_typeid_t结构的指针，表示类型标识符
 * @param[in] qos 指向dds_qos结构的指针，表示质量保证设置
 * @param[out] is_new 布尔值指针，表示是否为新创建的主题定义
 * @return 返回指向ddsi_topic_definition结构的指针
 *
 * @brief Reference a topic definition
 * @param[in] gv Pointer to ddsi_domaingv structure, representing Cyclone DDS domain global
 * variables
 * @param[in] sertype Pointer to ddsi_sertype structure, representing the serialization type
 * @param[in] type_id Pointer to ddsi_typeid_t structure, representing the type identifier
 * @param[in] qos Pointer to dds_qos structure, representing quality of service settings
 * @param[out] is_new Boolean pointer, indicating whether it is a newly created topic definition
 * @return Returns a pointer to the ddsi_topic_definition structure
 */
static struct ddsi_topic_definition *ref_topic_definition(struct ddsi_domaingv *gv,
                                                          const struct ddsi_sertype *sertype,
                                                          const ddsi_typeid_t *type_id,
                                                          struct dds_qos *qos,
                                                          bool *is_new) {
  // 锁定全局变量的主题定义锁
  // Lock the global variable's topic definitions lock
  ddsrt_mutex_lock(&gv->topic_defs_lock);

  // 在锁定状态下引用主题定义
  // Reference the topic definition while locked
  struct ddsi_topic_definition *tpd =
      ref_topic_definition_locked(gv, sertype, type_id, qos, is_new);

  // 解锁全局变量的主题定义锁
  // Unlock the global variable's topic definitions lock
  ddsrt_mutex_unlock(&gv->topic_defs_lock);

  // 返回主题定义指针
  // Return the topic definition pointer
  return tpd;
}

/**
 * @brief 在锁定状态下取消引用主题定义
 * @param[in] tpd 指向ddsi_topic_definition结构的指针，表示要取消引用的主题定义
 * @param[in] timestamp 表示时间戳的ddsrt_wctime_t结构
 *
 * @brief Unreference a topic definition while locked
 * @param[in] tpd Pointer to ddsi_topic_definition structure, representing the topic definition to
 * unreference
 * @param[in] timestamp ddsrt_wctime_t structure representing the timestamp
 */
static void unref_topic_definition_locked(struct ddsi_topic_definition *tpd,
                                          ddsrt_wctime_t timestamp) {
  // 减少引用计数，如果引用计数为0，则删除主题定义
  // Decrease the reference count, and if the reference count is 0, delete the topic definition
  if (!--tpd->refc) delete_topic_definition_locked(tpd, timestamp);
}

/**
 * @brief 减少主题定义的引用计数，并在适当的时候释放资源。
 *        Decrease the reference count of a topic definition and release resources when appropriate.
 *
 * @param[in] gv 指向域全局变量结构的指针。Pointer to the domain global variables structure.
 * @param[in] tpd 指向要取消引用的主题定义结构的指针。Pointer to the topic definition structure to
 * be unreferenced.
 * @param[in] timestamp 当前时间戳。Current wall-clock time.
 */
static void unref_topic_definition(struct ddsi_domaingv *gv,
                                   struct ddsi_topic_definition *tpd,
                                   ddsrt_wctime_t timestamp) {
  // 锁定主题定义互斥锁以确保线程安全。Lock the topic definitions mutex to ensure thread safety.
  // 对于多线程环境，这是必要的。This is necessary for multi-threaded environments.
  ddsrt_mutex_lock(&gv->topic_defs_lock);

  // 在已锁定的情况下减少主题定义的引用计数。Decrease the reference count of the topic definition
  // while locked.
  unref_topic_definition_locked(tpd, timestamp);

  // 解锁主题定义互斥锁。Unlock the topic definitions mutex.
  ddsrt_mutex_unlock(&gv->topic_defs_lock);
}

/**
 * @brief 比较两个主题定义是否相等。
 *        Compare two topic definitions for equality.
 *
 * @param[in] tpd_a 指向第一个主题定义结构的指针。Pointer to the first topic definition structure.
 * @param[in] tpd_b 指向第二个主题定义结构的指针。Pointer to the second topic definition structure.
 * @return 如果两个主题定义相等，则返回1，否则返回0。Returns 1 if the two topic definitions are
 * equal, 0 otherwise.
 */
int ddsi_topic_definition_equal(const struct ddsi_topic_definition *tpd_a,
                                const struct ddsi_topic_definition *tpd_b) {
  // 检查两个主题定义是否都不为NULL。Check if both topic definitions are not NULL.
  if (tpd_a != NULL && tpd_b != NULL) {
    // 主题定义的完整类型标识符和QoS应始终设置。The complete type identifier and qos should always
    // be set for a topic definition.
    assert(tpd_a->xqos != NULL && tpd_b->xqos != NULL);

    // 获取两个主题定义的完整类型标识符。Get the complete type identifiers for both topic
    // definitions.
    const ddsi_typeid_t *tid_a = ddsi_type_pair_complete_id(tpd_a->type_pair),
                        *tid_b = ddsi_type_pair_complete_id(tpd_b->type_pair);

    // 比较两个主题定义的类型标识符和QoS是否相等。Compare the type identifiers and QoS of the two
    // topic definitions for equality.
    return !ddsi_typeid_compare(tid_a, tid_b) &&
           !ddsi_xqos_delta(tpd_a->xqos, tpd_b->xqos, ~(DDSI_QP_TYPE_INFORMATION));
  }
  // 如果两个主题定义都为NULL或者只有一个为NULL，则返回它们是否相等。Return whether they are equal
  // if both topic definitions are NULL or only one is NULL.
  return tpd_a == tpd_b;
}

/**
 * @brief 创建一个新的主题定义 (Create a new topic definition)
 *
 * @param[in] gv  Cyclone DDS 全局变量 (Cyclone DDS global variables)
 * @param[in] type  主题的序列化类型 (Serialization type of the topic)
 * @param[in] qos  主题的质量服务 (Quality of Service for the topic)
 *
 * @return 成功时返回新创建的主题定义，失败时返回 NULL (Returns the newly created topic definition
 * on success, NULL on failure)
 */
static struct ddsi_topic_definition *new_topic_definition(struct ddsi_domaingv *gv,
                                                          const struct ddsi_sertype *type,
                                                          const struct dds_qos *qos) {
  dds_return_t ret;
  // 确保 QoS 中存在主题名和类型名 (Ensure that topic name and type name are present in QoS)
  assert((qos->present & (DDSI_QP_TOPIC_NAME | DDSI_QP_TYPE_NAME)) ==
         (DDSI_QP_TOPIC_NAME | DDSI_QP_TYPE_NAME));

  // 为主题定义分配内存 (Allocate memory for topic definition)
  struct ddsi_topic_definition *tpd = ddsrt_malloc(sizeof(*tpd));
  if (!tpd) goto err;

  // 复制 QoS 设置 (Duplicate QoS settings)
  tpd->xqos = ddsi_xqos_dup(qos);
  tpd->refc = 1;
  tpd->gv = gv;

  // 为类型对分配内存 (Allocate memory for type pair)
  tpd->type_pair = ddsrt_malloc(sizeof(*tpd->type_pair));
  if (!tpd->type_pair) {
    ddsi_xqos_fini(tpd->xqos);
    ddsrt_free(tpd);
    tpd = NULL;
    goto err;
  }

  if (type != NULL) {
    // 此处不应失败，因为此处使用的 sertype 已经在 typelib 中
    // 类型从 dds_create_topic_impl 中引用 (This shouldn't fail, because the sertype used here is
    // already in the typelib as the types are referenced from dds_create_topic_impl)
    ret = ddsi_type_ref_local(gv, &tpd->type_pair->minimal, type, DDSI_TYPEID_KIND_MINIMAL);
    assert(ret == DDS_RETCODE_OK);
    ret = ddsi_type_ref_local(gv, &tpd->type_pair->complete, type, DDSI_TYPEID_KIND_COMPLETE);
    assert(ret == DDS_RETCODE_OK);
    (void)ret;
  } else {
    // 确保 QoS 中存在类型信息 (Ensure that type information is present in QoS)
    assert(qos->present & DDSI_QP_TYPE_INFORMATION);

    // 获取代理类型引用 (Get proxy type references)
    if ((ret = ddsi_type_ref_proxy(gv, &tpd->type_pair->minimal, qos->type_information,
                                   DDSI_TYPEID_KIND_MINIMAL, NULL)) != DDS_RETCODE_OK ||
        ddsi_type_ref_proxy(gv, &tpd->type_pair->complete, qos->type_information,
                            DDSI_TYPEID_KIND_COMPLETE, NULL) != DDS_RETCODE_OK) {
      if (ret == DDS_RETCODE_OK) ddsi_type_unref(gv, tpd->type_pair->minimal);
      ddsi_xqos_fini(tpd->xqos);
      ddsrt_free(tpd->type_pair);
      ddsrt_free(tpd);
      tpd = NULL;
      goto err;
    }
  }

  // 设置主题定义的哈希值 (Set the hash value for the topic definition)
  set_ddsi_topic_definition_hash(tpd);

  // 如果启用了发现日志，则记录主题定义信息 (Log topic definition information if discovery logging
  // is enabled)
  if (gv->logconfig.c.mask & DDS_LC_DISCOVERY) {
    GVLOGDISC(" topic-definition 0x%p: key 0x", tpd);
    for (size_t i = 0; i < sizeof(tpd->key); i++) GVLOGDISC("%02x", tpd->key[i]);
    GVLOGDISC(" QOS={");
    ddsi_xqos_log(DDS_LC_DISCOVERY, &gv->logconfig, tpd->xqos);
    GVLOGDISC("}\n");
  }

  // 将新创建的主题定义添加到哈希表中 (Add the newly created topic definition to the hash table)
  ddsrt_hh_add_absent(gv->topic_defs, tpd);
err:
  return tpd;
}

/**
 * @brief 查找主题定义 (Lookup topic definition)
 *
 * @param[in] gv        域全局变量指针 (Pointer to domain global variables)
 * @param[in] topic_name 主题名称 (Topic name)
 * @param[in] type_id    类型标识符 (Type identifier)
 * @param[out] tpd       存储找到的主题定义的指针 (Pointer to store the found topic definition)
 *
 * @return dds_return_t  返回操作结果状态码 (Return operation result status code)
 */
dds_return_t ddsi_lookup_topic_definition(struct ddsi_domaingv *gv,
                                          const char *topic_name,
                                          const ddsi_typeid_t *type_id,
                                          struct ddsi_topic_definition **tpd) {
  // 断言：tpd 不为 NULL (Assert: tpd is not NULL)
  assert(tpd != NULL);

  // 定义迭代器和返回值 (Define iterator and return value)
  struct ddsrt_hh_iter it;
  dds_return_t ret = DDS_RETCODE_OK;

  // 初始化 tpd 为空 (Initialize tpd as NULL)
  *tpd = NULL;

  // 锁定主题定义互斥锁 (Lock the topic definitions mutex)
  ddsrt_mutex_lock(&gv->topic_defs_lock);

  // 遍历主题定义 (Iterate through topic definitions)
  for (struct ddsi_topic_definition *tpd1 = ddsrt_hh_iter_first(gv->topic_defs, &it); tpd1;
       tpd1 = ddsrt_hh_iter_next(&it)) {
    // 检查主题名称和类型ID是否匹配 (Check if topic name and type ID match)
    if (!strcmp(tpd1->xqos->topic_name, topic_name) &&
        (ddsi_typeid_is_none(type_id) ||
         ((tpd1->xqos->present & DDSI_QP_TYPE_INFORMATION) &&
          !ddsi_typeid_compare(type_id,
                               ddsi_typeinfo_complete_typeid(tpd1->xqos->type_information))))) {
      // 设置找到的主题定义 (Set the found topic definition)
      *tpd = tpd1;
      break;
    }
  }

  // 解锁主题定义互斥锁 (Unlock the topic definitions mutex)
  ddsrt_mutex_unlock(&gv->topic_defs_lock);

  // 返回操作结果状态码 (Return operation result status code)
  return ret;
}

/* PROXY-TOPIC --------------------------------------------------- */

/**
 * @brief 查找代理主题 (Lookup a proxy topic)
 *
 * @param[in] proxypp 代理参与者指针 (Pointer to the proxy participant)
 * @param[in] guid GUID 指针 (Pointer to the GUID)
 * @return 返回查找到的代理主题指针，如果未找到则返回 NULL (Returns pointer to the found proxy
 * topic, or NULL if not found)
 */
struct ddsi_proxy_topic *ddsi_lookup_proxy_topic(struct ddsi_proxy_participant *proxypp,
                                                 const ddsi_guid_t *guid) {
  assert(proxypp != NULL);  // 确保代理参与者不为空 (Ensure proxy participant is not NULL)
  ddsrt_mutex_lock(&proxypp->e.lock);  // 锁定互斥量 (Lock the mutex)
  struct ddsi_proxy_topic *ptp = ddsrt_avl_lookup(
      &ddsi_proxypp_proxytp_treedef, &proxypp->topics,
      &guid->entityid);  // 在 AVL 树中查找代理主题 (Lookup the proxy topic in the AVL tree)
  ddsrt_mutex_unlock(&proxypp->e.lock);  // 解锁互斥量 (Unlock the mutex)
  return ptp;  // 返回查找到的代理主题 (Return the found proxy topic)
}

/**
 * @brief 创建新的代理主题 (Create a new proxy topic)
 *
 * @param[in] proxypp 代理参与者指针 (Pointer to the proxy participant)
 * @param[in] seq 序列号 (Sequence number)
 * @param[in] guid GUID 指针 (Pointer to the GUID)
 * @param[in] type_id_minimal 最小类型 ID (Minimal type ID)
 * @param[in] type_id_complete 完整类型 ID (Complete type ID)
 * @param[in] qos 服务质量指针 (Pointer to the Quality of Service)
 * @param[in] timestamp 时间戳 (Timestamp)
 * @return 返回操作结果代码 (Returns operation result code)
 */
dds_return_t ddsi_new_proxy_topic(struct ddsi_proxy_participant *proxypp,
                                  ddsi_seqno_t seq,
                                  const ddsi_guid_t *guid,
                                  const ddsi_typeid_t *type_id_minimal,
                                  const ddsi_typeid_t *type_id_complete,
                                  struct dds_qos *qos,
                                  ddsrt_wctime_t timestamp) {
  assert(proxypp != NULL);  // 确保代理参与者不为空 (Ensure proxy participant is not NULL)
  struct ddsi_domaingv *gv = proxypp->e.gv;  // 获取域全局变量 (Get the domain global variables)
  bool new_tpd = false;
  struct ddsi_topic_definition *tpd = NULL;
  if (!ddsi_typeid_is_none(type_id_complete))
    tpd = ref_topic_definition(gv, NULL, type_id_complete, qos,
                               &new_tpd);  // 使用完整类型 ID 引用主题定义 (Reference topic
                                           // definition using complete type ID)
  else if (!ddsi_typeid_is_none(type_id_minimal))
    tpd = ref_topic_definition(gv, NULL, type_id_minimal, qos,
                               &new_tpd);  // 使用最小类型 ID 引用主题定义 (Reference topic
                                           // definition using minimal type ID)
  if (tpd == NULL) return DDS_RETCODE_BAD_PARAMETER;
#ifndef NDEBUG
  bool found_proxytp =
      ddsi_lookup_proxy_topic(proxypp, guid);  // 查找代理主题 (Lookup the proxy topic)
  assert(!found_proxytp);  // 确保未找到代理主题 (Ensure the proxy topic is not found)
#endif
  struct ddsi_proxy_topic *proxytp =
      ddsrt_malloc(sizeof(*proxytp));  // 分配内存给代理主题 (Allocate memory for the proxy topic)
  proxytp->entityid = guid->entityid;  // 设置实体 ID (Set the entity ID)
  proxytp->definition = tpd;           // 设置主题定义 (Set the topic definition)
  proxytp->seq = seq;                  // 设置序列号 (Set the sequence number)
  proxytp->tupdate = timestamp;        // 设置时间戳 (Set the timestamp)
  proxytp->deleted = 0;  // 初始化删除标记为 0 (Initialize the deleted flag to 0)
  ddsrt_mutex_lock(&proxypp->e.lock);  // 锁定互斥量 (Lock the mutex)
  ddsrt_avl_insert(&ddsi_proxypp_proxytp_treedef, &proxypp->topics,
                   proxytp);  // 将代理主题插入 AVL 树 (Insert the proxy topic into the AVL tree)
  ddsrt_mutex_unlock(&proxypp->e.lock);  // 解锁互斥量 (Unlock the mutex)
  if (new_tpd) {
    ddsi_builtintopic_write_topic(gv->builtin_topic_interface, tpd, timestamp,
                                  true);    // 写入内置主题 (Write the built-in topic)
    ddsrt_mutex_lock(&gv->new_topic_lock);  // 锁定新主题互斥量 (Lock the new topic mutex)
    gv->new_topic_version++;  // 增加新主题版本 (Increment the new topic version)
    ddsrt_cond_broadcast(
        &gv->new_topic_cond);  // 广播新主题条件变量 (Broadcast the new topic condition variable)
    ddsrt_mutex_unlock(&gv->new_topic_lock);  // 解锁新主题互斥量 (Unlock the new topic mutex)
  }

  return DDS_RETCODE_OK;  // 返回操作成功代码 (Return operation success code)
}

/**
 * @brief 更新代理主题信息
 * @param[in] proxypp 代理参与者实例指针
 * @param[in,out] proxytp 代理主题实例指针
 * @param[in] seq 序列号
 * @param[in] xqos 新的QoS设置
 * @param[in] timestamp 时间戳
 *
 * @details Update the proxy topic information.
 * @param[in] proxypp Pointer to the proxy participant instance
 * @param[in,out] proxytp Pointer to the proxy topic instance
 * @param[in] seq Sequence number
 * @param[in] xqos New QoS settings
 * @param[in] timestamp Timestamp
 */
void ddsi_update_proxy_topic(struct ddsi_proxy_participant *proxypp,
                             struct ddsi_proxy_topic *proxytp,
                             ddsi_seqno_t seq,
                             struct dds_qos *xqos,
                             ddsrt_wctime_t timestamp) {
  // 锁定代理参与者实例以确保线程安全
  // Lock the proxy participant instance to ensure thread safety
  ddsrt_mutex_lock(&proxypp->e.lock);

  // 获取域全局变量指针
  // Get the domain global variable pointer
  struct ddsi_domaingv *gv = proxypp->e.gv;

  // 如果代理主题已被删除，解锁并返回
  // If the proxy topic is deleted, unlock and return
  if (proxytp->deleted) {
    GVLOGDISC(" deleting\n");
    ddsrt_mutex_unlock(&proxypp->e.lock);
    return;
  }

  // 如果序列号不是新的，解锁并返回
  // If the sequence number is not new, unlock and return
  if (seq <= proxytp->seq) {
    GVLOGDISC(" seqno not new\n");
    ddsrt_mutex_unlock(&proxypp->e.lock);
    return;
  }

  // 锁定主题定义锁以确保线程安全
  // Lock the topic definition lock to ensure thread safety
  ddsrt_mutex_lock(&gv->topic_defs_lock);

  // 获取当前代理主题定义
  // Get the current proxy topic definition
  struct ddsi_topic_definition *tpd0 = proxytp->definition;

  // 更新序列号和时间戳
  // Update the sequence number and timestamp
  proxytp->seq = seq;
  proxytp->tupdate = timestamp;

  // 计算QoS设置的变化掩码
  // Calculate the change mask for QoS settings
  uint64_t mask =
      ddsi_xqos_delta(tpd0->xqos, xqos,
                      DDSI_QP_CHANGEABLE_MASK & ~(DDSI_QP_RXO_MASK | DDSI_QP_PARTITION)) &
      xqos->present;

  // 打印变化的QoS设置
  // Print the changed QoS settings
  GVLOGDISC("ddsi_update_proxy_topic %" PRIx32 " delta=%" PRIu64 " QOS={", proxytp->entityid.u,
            mask);
  ddsi_xqos_log(DDS_LC_DISCOVERY, &gv->logconfig, xqos);
  GVLOGDISC("}\n");

  // 如果没有变化，解锁并返回
  // If there is no change, unlock and return
  if (mask == 0) {
    ddsrt_mutex_unlock(&gv->topic_defs_lock);
    ddsrt_mutex_unlock(&proxypp->e.lock);
    return; /* no change, or an as-yet unsupported one */
  }

  // 创建新的QoS设置并合并变化
  // Create a new QoS settings and merge the changes
  dds_qos_t *newqos = dds_create_qos();
  ddsi_xqos_mergein_missing(newqos, xqos, mask);
  ddsi_xqos_mergein_missing(newqos, tpd0->xqos, ~(uint64_t)0);

  // 获取新的主题定义
  // Get the new topic definition
  bool new_tpd = false;
  struct ddsi_topic_definition *tpd1 = ref_topic_definition_locked(
      gv, NULL, ddsi_type_pair_complete_id(tpd0->type_pair), newqos, &new_tpd);
  assert(tpd1);

  // 取消引用旧的主题定义，并更新代理主题定义
  // Unreference the old topic definition and update the proxy topic definition
  unref_topic_definition_locked(tpd0, timestamp);
  proxytp->definition = tpd1;

  // 解锁主题定义锁和代理参与者实例锁
  // Unlock the topic definition lock and the proxy participant instance lock
  ddsrt_mutex_unlock(&gv->topic_defs_lock);
  ddsrt_mutex_unlock(&proxypp->e.lock);

  // 删除新创建的QoS设置
  // Delete the newly created QoS settings
  dds_delete_qos(newqos);

  // 如果有新的主题定义，写入内置主题并更新版本号
  // If there is a new topic definition, write it to the built-in topic and update the version
  // number
  if (new_tpd) {
    ddsi_builtintopic_write_topic(gv->builtin_topic_interface, tpd1, timestamp, true);

    ddsrt_mutex_lock(&gv->new_topic_lock);
    gv->new_topic_version++;
    ddsrt_cond_broadcast(&gv->new_topic_cond);
    ddsrt_mutex_unlock(&gv->new_topic_lock);
  }
}

/**
 * @brief 删除代理主题的垃圾收集函数
 * @param[in] gcreq 垃圾收集请求指针
 *
 * @note 该函数会在垃圾收集队列中调用，以删除代理主题。
 */
// Delete proxy topic garbage collection function
// @param[in] gcreq pointer to the garbage collection request
//
// This function will be called in the garbage collection queue to delete the proxy topic.
static void gc_delete_proxy_topic(struct ddsi_gcreq *gcreq) {
  struct gc_proxy_tp *gcdata = gcreq->arg;

  // 锁定代理参与者实体
  // Lock the proxy participant entity
  ddsrt_mutex_lock(&gcdata->proxypp->e.lock);
  struct ddsi_domaingv *gv = gcdata->proxypp->e.gv;
  // 锁定主题定义
  // Lock the topic definitions
  ddsrt_mutex_lock(&gv->topic_defs_lock);
  struct ddsi_topic_definition *tpd = gcdata->proxytp->definition;
  GVLOGDISC("gc_delete_proxy_topic (%p)\n", (void *)gcdata->proxytp);
  // 从代理参与者的主题树中删除代理主题
  // Remove the proxy topic from the proxy participant's topic tree
  ddsrt_avl_delete(&ddsi_proxypp_proxytp_treedef, &gcdata->proxypp->topics, gcdata->proxytp);
  // 解除锁定的主题定义引用
  // Unreference the locked topic definition
  unref_topic_definition_locked(tpd, gcdata->timestamp);
  // 释放代理主题内存
  // Free the proxy topic memory
  ddsrt_free(gcdata->proxytp);
  // 解锁主题定义和代理参与者实体
  // Unlock the topic definitions and the proxy participant entity
  ddsrt_mutex_unlock(&gv->topic_defs_lock);
  ddsrt_mutex_unlock(&gcdata->proxypp->e.lock);
  // 释放垃圾收集数据内存
  // Free the garbage collection data memory
  ddsrt_free(gcdata);
  // 释放垃圾收集请求
  // Free the garbage collection request
  ddsi_gcreq_free(gcreq);
}

/**
 * @brief 创建代理主题的垃圾收集请求
 * @param[in] proxypp 代理参与者指针
 * @param[in] proxytp 代理主题指针
 * @param[in] timestamp 时间戳
 * @return 成功时返回0
 */
// Create garbage collection request for proxy topic
// @param[in] proxypp pointer to the proxy participant
// @param[in] proxytp pointer to the proxy topic
// @param[in] timestamp time stamp
// @return 0 on success
static int gcreq_proxy_topic(struct ddsi_proxy_participant *proxypp,
                             struct ddsi_proxy_topic *proxytp,
                             ddsrt_wctime_t timestamp) {
  // 创建垃圾收集请求
  // Create a garbage collection request
  struct ddsi_gcreq *gcreq =
      ddsi_gcreq_new(proxytp->definition->gv->gcreq_queue, gc_delete_proxy_topic);
  // 分配垃圾收集数据内存
  // Allocate memory for garbage collection data
  struct gc_proxy_tp *gcdata = ddsrt_malloc(sizeof(*gcdata));
  gcdata->proxypp = proxypp;
  gcdata->proxytp = proxytp;
  gcdata->timestamp = timestamp;
  gcreq->arg = gcdata;
  // 将垃圾收集请求加入队列
  // Enqueue the garbage collection request
  ddsi_gcreq_enqueue(gcreq);
  return 0;
}

/**
 * @brief 删除锁定的代理主题
 * @param[in] proxypp 代理参与者指针
 * @param[in] proxytp 代理主题指针
 * @param[in] timestamp 时间戳
 * @return 成功时返回DDS_RETCODE_OK，否则返回DDS_RETCODE_PRECONDITION_NOT_MET
 */
// Delete locked proxy topic
// @param[in] proxypp pointer to the proxy participant
// @param[in] proxytp pointer to the proxy topic
// @param[in] timestamp time stamp
// @return DDS_RETCODE_OK on success, otherwise DDS_RETCODE_PRECONDITION_NOT_MET
int ddsi_delete_proxy_topic_locked(struct ddsi_proxy_participant *proxypp,
                                   struct ddsi_proxy_topic *proxytp,
                                   ddsrt_wctime_t timestamp) {
  struct ddsi_domaingv *gv = proxypp->e.gv;
  GVLOGDISC("ddsi_delete_proxy_topic_locked (%p) ", proxypp);
  // 检查代理主题是否已删除
  // Check if the proxy topic is already deleted
  if (proxytp->deleted) return DDS_RETCODE_PRECONDITION_NOT_MET;
  proxytp->deleted = 1;
  // 创建代理主题的垃圾收集请求
  // Create garbage collection request for the proxy topic
  gcreq_proxy_topic(proxypp, proxytp, timestamp);
  return DDS_RETCODE_OK;
}

#endif
