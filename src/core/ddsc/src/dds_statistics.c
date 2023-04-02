/*
 * Copyright(c) 2020 ZettaScale Technology and others
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

#include "dds/ddsc/dds_statistics.h"
#include "dds/ddsrt/heap.h"
#include "dds/ddsrt/log.h"
#include "dds__entity.h"
#include "dds__statistics.h"
// 定义一个名为dds_alloc_statistics的函数，接收两个参数：一个指向dds_entity结构体的指针e和一个指向dds_stat_descriptor结构体的指针d
struct dds_statistics *dds_alloc_statistics(const struct dds_entity *e, const struct dds_stat_descriptor *d)
{
  // 分配内存空间给dds_statistics结构体，并根据d->count计算所需的额外空间
  struct dds_statistics *s = ddsrt_malloc(sizeof(*s) + d->count * sizeof(s->kv[0]));
  // 将e->m_hdllink.hdl赋值给s->entity
  s->entity = e->m_hdllink.hdl;
  // 将e->m_iid赋值给s->opaque
  s->opaque = e->m_iid;
  // 将0赋值给s->time
  s->time = 0;
  // 将d->count赋值给s->count
  s->count = d->count;
  // 使用memset函数将s->kv数组的前d->count * sizeof(s->kv[0])个字节设置为0
  memset(s->kv, 0, d->count * sizeof(s->kv[0]));
  // 使用for循环遍历s->kv数组
  for (size_t i = 0; i < s->count; i++)
  {
    // 将d->kv[i].kind赋值给s->kv[i].kind
    s->kv[i].kind = d->kv[i].kind;
    // 将d->kv[i].name赋值给s->kv[i].name
    s->kv[i].name = d->kv[i].name;
  }
  // 返回指向dds_statistics结构体的指针s
  return s;
}

// 定义一个名为dds_create_statistics的函数，接收一个dds_entity_t类型的参数entity
struct dds_statistics *dds_create_statistics(dds_entity_t entity)
{
  // 声明两个指针变量：一个指向dds_entity结构体的e和一个指向dds_statistics结构体的s
  dds_entity *e;
  struct dds_statistics *s;
  // 如果无法获取到实体，则返回NULL
  if (dds_entity_pin(entity, &e) != DDS_RETCODE_OK)
    return NULL;
  // 查找线程状态
  struct ddsi_thread_state *const thrst = ddsi_lookup_thread_state();
  // 唤醒线程状态
  ddsi_thread_state_awake(thrst, &e->m_domain->gv);
  // 创建统计信息
  if ((s = dds_entity_deriver_create_statistics(e)) != NULL)
    dds_entity_deriver_refresh_statistics(e, s);
  // 线程进入休眠状态
  ddsi_thread_state_asleep(thrst);
  // 解除实体引用
  dds_entity_unpin(e);
  // 返回指向dds_statistics结构体的指针s
  return s;
}

// 定义一个名为dds_refresh_statistics的函数，接收一个指向dds_statistics结构体的指针stat
dds_return_t dds_refresh_statistics(struct dds_statistics *stat)
{
  // 声明一个dds_return_t类型的变量rc和一个指向dds_entity结构体的指针e
  dds_return_t rc;
  dds_entity *e;
  // 如果stat为NULL，则返回错误代码DDS_RETCODE_BAD_PARAMETER
  if (stat == NULL)
    return DDS_RETCODE_BAD_PARAMETER;
  // 如果无法获取到实体，则返回错误代码rc
  if ((rc = dds_entity_pin(stat->entity, &e)) != DDS_RETCODE_OK)
    return rc;
  // 如果stat->opaque与e->m_iid不相等，则解除实体引用并返回错误代码DDS_RETCODE_BAD_PARAMETER
  if (stat->opaque != e->m_iid)
  {
    dds_entity_unpin(e);
    return DDS_RETCODE_BAD_PARAMETER;
  }
  // 查找线程状态
  struct ddsi_thread_state *const thrst = ddsi_lookup_thread_state();
  // 唤醒线程状态
  ddsi_thread_state_awake(thrst, &e->m_domain->gv);
  // 更新统计信息的时间戳
  stat->time = dds_time();
  // 刷新统计信息
  dds_entity_deriver_refresh_statistics(e, stat);
  // 线程进入休眠状态
  ddsi_thread_state_asleep(thrst);
  // 解除实体引用
  dds_entity_unpin(e);
  // 返回成功代码DDS_RETCODE_OK
  return DDS_RETCODE_OK;
}

// 定义一个名为dds_lookup_statistic的函数，接收两个参数：一个指向dds_statistics结构体的指针stat和一个字符串指针name
const struct dds_stat_keyvalue *dds_lookup_statistic(const struct dds_statistics *stat, const char *name)
{
  // 如果stat为NULL，则返回NULL
  if (stat == NULL)
    return NULL;
  // 使用for循环遍历stat->kv数组
  for (size_t i = 0; i < stat->count; i++)
    // 如果找到与name相同的名称，则返回指向该元素的指针
    if (strcmp(stat->kv[i].name, name) == 0)
      return &stat->kv[i];
  // 如果没有找到匹配的名称，则返回NULL
  return NULL;
}

// 定义一个名为dds_delete_statistics的函数，接收一个指向dds_statistics结构体的指针stat
void dds_delete_statistics(struct dds_statistics *stat)
{
  // 释放stat指向的内存空间
  ddsrt_free(stat);
}
