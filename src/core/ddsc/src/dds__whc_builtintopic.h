/*
 * Copyright(c) 2006 to 2020 ZettaScale Technology and others
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v. 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0, or the Eclipse Distribution License
 * v. 1.0 which is available at
 * http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * SPDX-License-Identifier: EPL-2.0 OR BSD-3-Clause
 */
#ifndef DDS__WHC_BUILTINTOPIC_H
#define DDS__WHC_BUILTINTOPIC_H

#include "dds/ddsi/ddsi_whc.h"
#include "dds__serdata_builtintopic.h"

#if defined(__cplusplus)
extern "C"
{
#endif

  /**
   * @brief 创建一个新的内置主题的写历史缓存 (WHC)
   *
   * 此函数用于为指定的实体类型创建一个新的内置主题的写历史缓存 (WHC)。
   *
   * @param entity_kind 内置主题实体类型，可以是以下之一：
   *                    - DDSI_SER_BUILTIN_TOPIC_ENTITY_KIND_PARTICIPANT
   *                    - DDSI_SER_BUILTIN_TOPIC_ENTITY_KIND_READER
   *                    - DDSI_SER_BUILTIN_TOPIC_ENTITY_KIND_WRITER
   * @param entidx      指向 ddsi_entity_index 结构的指针，该结构包含实体索引信息
   *
   * @return 返回一个指向新创建的 ddsi_whc 结构的指针
   */
  struct ddsi_whc *dds_builtintopic_whc_new(enum ddsi_sertype_builtintopic_entity_kind entity_kind, const struct ddsi_entity_index *entidx);

#if defined(__cplusplus)
}
#endif

#endif /* DDS__WHC_BUILTINTOPIC_H */
