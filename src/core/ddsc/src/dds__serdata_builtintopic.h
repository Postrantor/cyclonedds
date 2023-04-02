/*
 * Copyright(c) 2006 to 2021 ZettaScale Technology and others
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v. 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0, or the Eclipse Distribution License
 * v. 1.0 which is available at
 * http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * SPDX-License-Identifier: EPL-2.0 OR BSD-3-Clause
 */
#ifndef DDS__SERDATA_BUILTINTYPE_H
#define DDS__SERDATA_BUILTINTYPE_H

#include "dds/dds.h"
#include "dds/ddsi/ddsi_serdata.h"
#include "dds/ddsi/ddsi_sertype.h"
#include "dds/ddsi/ddsi_xqos.h"

#if defined(__cplusplus)
extern "C"
{
#endif

  struct ddsi_entity_common;
  struct ddsi_topic_definition;

  // 定义内置主题实体类型枚举
  enum ddsi_sertype_builtintopic_entity_kind
  {
    DSBT_PARTICIPANT, // 参与者
    DSBT_TOPIC,       // 主题
    DSBT_READER,      // 读取器
    DSBT_WRITER       // 写入器
  };

  // 定义内置主题序列化数据结构
  struct ddsi_serdata_builtintopic
  {
    struct ddsi_serdata c; // 序列化数据通用部分
    union
    {
      unsigned char raw[16]; // 原始字节表示的GUID
      ddsi_guid_t guid;      // GUID结构
    } key;
    dds_qos_t xqos; // QoS策略
  };

  // 定义参与者内置主题序列化数据结构
  struct ddsi_serdata_builtintopic_participant
  {
    struct ddsi_serdata_builtintopic common; // 内置主题序列化数据通用部分
    dds_instance_handle_t pphandle;          // 参与者实例句柄
  };

#ifdef DDS_HAS_TOPIC_DISCOVERY
  // 定义主题内置主题序列化数据结构（仅在启用主题发现时可用）
  struct ddsi_serdata_builtintopic_topic
  {
    struct ddsi_serdata_builtintopic common; // 内置主题序列化数据通用部分
  };
#endif

  // 定义端点内置主题序列化数据结构
  struct ddsi_serdata_builtintopic_endpoint
  {
    struct ddsi_serdata_builtintopic common; // 内置主题序列化数据通用部分
    dds_instance_handle_t pphandle;          // 端点实例句柄
  };

  // 定义内置主题序列化类型结构
  struct ddsi_sertype_builtintopic
  {
    struct ddsi_sertype c;                                  // 序列化类型通用部分
    enum ddsi_sertype_builtintopic_entity_kind entity_kind; // 实体类型
  };

  // 声明内置主题序列化类型操作集
  extern const struct ddsi_sertype_ops ddsi_sertype_ops_builtintopic;
  // 声明内置主题序列化数据操作集
  extern const struct ddsi_serdata_ops ddsi_serdata_ops_builtintopic;

  /**
   * @brief 创建新的内置主题序列化类型
   * @param entity_kind 实体类型（参与者、主题、读取器或写入器）
   * @param typename 类型名称
   * @return 返回创建的内置主题序列化类型指针
   */
  /** @component typesupport_builtin */
  struct ddsi_sertype *dds_new_sertype_builtintopic(enum ddsi_sertype_builtintopic_entity_kind entity_kind, const char *typename);

  /**
   * @brief 从端点创建内置主题序列化数据
   * @param tpcmn 序列化类型通用部分指针
   * @param guid GUID指针
   * @param entity 实体通用部分指针
   * @param kind 序列化数据种类
   * @return 返回创建的内置主题序列化数据指针
   */
  /** @component typesupport_builtin */
  struct ddsi_serdata *dds_serdata_builtin_from_endpoint(const struct ddsi_sertype *tpcmn, const ddsi_guid_t *guid, struct ddsi_entity_common *entity, enum ddsi_serdata_kind kind);

#ifdef DDS_HAS_TOPIC_DISCOVERY
  // 声明一个名为ddsi_serdata_ops_builtintopic_topic的结构体常量，该结构体用于序列化操作
  extern const struct ddsi_serdata_ops ddsi_serdata_ops_builtintopic_topic;

  /**
   * @brief 创建一个新的内置主题类型对象
   * @component typesupport_builtin
   * @param entity_kind 内置主题实体类型枚举值
   * @param typename 类型名称字符串
   * @return 返回一个指向新创建的内置主题类型对象的指针
   */
  struct ddsi_sertype *dds_new_sertype_builtintopic_topic(enum ddsi_sertype_builtintopic_entity_kind entity_kind, const char *typename);

  /**
   * @brief 从主题定义中创建一个内置序列化数据对象
   * @component typesupport_builtin
   * @param tpcmn 指向通用主题类型的指针
   * @param key 指向内置主题键的指针
   * @param tpd 指向主题定义结构体的指针
   * @param kind 序列化数据类型枚举值
   * @return 返回一个指向新创建的内置序列化数据对象的指针
   */
  struct ddsi_serdata *dds_serdata_builtin_from_topic_definition(const struct ddsi_sertype *tpcmn, const dds_builtintopic_topic_key_t *key, const struct ddsi_topic_definition *tpd, enum ddsi_serdata_kind kind);

#endif /* DDS_HAS_TOPIC_DISCOVERY */

#if defined(__cplusplus)
}
#endif

#endif /* DDS__SERDATA_BUILTINTYPE_H */
