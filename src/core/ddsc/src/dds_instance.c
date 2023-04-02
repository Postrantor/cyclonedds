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
#include <assert.h>
#include <string.h>

#include "dds/dds.h"
#include "dds/ddsc/dds_rhc.h"
#include "dds/ddsi/ddsi_domaingv.h"
#include "dds/ddsi/ddsi_endpoint.h"
#include "dds/ddsi/ddsi_entity.h"
#include "dds/ddsi/ddsi_serdata.h"
#include "dds/ddsi/ddsi_thread.h"
#include "dds/ddsi/ddsi_tkmap.h"
#include "dds__entity.h"
#include "dds__write.h"
#include "dds__writer.h"

// dds_writedispose函数，将数据写入writer并且标记为dispose状态
dds_return_t dds_writedispose(dds_entity_t writer, const void *data)
{
  // 调用dds_writedispose_ts函数，并传入writer、data和当前时间戳
  return dds_writedispose_ts(writer, data, dds_time());
}

// dds_dispose函数，将数据标记为dispose状态
dds_return_t dds_dispose(dds_entity_t writer, const void *data)
{
  // 调用dds_dispose_ts函数，并传入writer、data和当前时间戳
  return dds_dispose_ts(writer, data, dds_time());
}

// dds_dispose_ih函数，将指定instance handle的数据标记为dispose状态
dds_return_t dds_dispose_ih(dds_entity_t writer, dds_instance_handle_t handle)
{
  // 调用dds_dispose_ih_ts函数，并传入writer、handle和当前时间戳
  return dds_dispose_ih_ts(writer, handle, dds_time());
}

// dds_instance_find函数，查找指定writer中与给定数据匹配的实例
static struct ddsi_tkmap_instance *dds_instance_find(const dds_writer *writer, const void *data, const bool create)
{
  // 根据writer的类型和SDK_KEY创建一个dds序列化数据对象
  struct ddsi_serdata *sd = ddsi_serdata_from_sample(writer->m_wr->type, SDK_KEY, data);
  // 如果序列化数据对象为空，则返回NULL
  if (sd == NULL)
    return NULL;
  // 在writer所在的domain中查找与序列化数据对象匹配的实例
  struct ddsi_tkmap_instance *inst = ddsi_tkmap_find(writer->m_entity.m_domain->gv.m_tkmap, sd, create);
  // 释放dds序列化数据对象
  ddsi_serdata_unref(sd);
  // 返回查找到的实例
  return inst;
}

// dds_instance_remove函数，从指定writer中删除指定数据或instance handle对应的实例
static void dds_instance_remove(struct dds_domain *dom, const dds_writer *writer, const void *data, dds_instance_handle_t handle)
{
  // 定义一个dds实例对象指针
  struct ddsi_tkmap_instance *inst;
  // 如果传入的handle不为空，则根据handle在domain中查找对应的实例
  if (handle != DDS_HANDLE_NIL)
    inst = ddsi_tkmap_find_by_id(dom->gv.m_tkmap, handle);
  // 如果传入的handle为空，则调用dds_instance_find函数查找与给定数据匹配的实例
  else
  {
    assert(data);
    inst = dds_instance_find(writer, data, false);
  }
  // 如果找到了实例，则将其从tkmap中移除
  if (inst)
  {
    ddsi_tkmap_instance_unref(dom->gv.m_tkmap, inst);
  }
}
dds_return_t dds_register_instance(dds_entity_t writer, dds_instance_handle_t *handle, const void *data)
{
  // 声明一个dds_writer类型的指针变量wr，用于存储writer实体的信息
  dds_writer *wr;
  dds_return_t ret;

  // 检查参数是否有误
  if (data == NULL || handle == NULL)
    return DDS_RETCODE_BAD_PARAMETER;

  // 获取writer实体的信息
  if ((ret = dds_writer_lock(writer, &wr)) != DDS_RETCODE_OK)
    return ret;

  // 获取当前线程的状态
  struct ddsi_thread_state *const thrst = ddsi_lookup_thread_state();
  // 唤醒当前线程，并设置全局变量gv
  ddsi_thread_state_awake(thrst, &wr->m_entity.m_domain->gv);
  // 根据data参数查找实例，如果不存在则新建一个实例
  struct ddsi_tkmap_instance *const inst = dds_instance_find(wr, data, true);
  // 如果实例不存在，返回参数错误
  if (inst == NULL)
    ret = DDS_RETCODE_BAD_PARAMETER;
  else
  {
    // 将实例的标识符赋值给handle参数
    *handle = inst->m_iid;
    ret = DDS_RETCODE_OK;
  }
  // 线程休眠
  ddsi_thread_state_asleep(thrst);
  // 释放writer实体的信息
  dds_writer_unlock(wr);
  return ret;
}

dds_return_t dds_unregister_instance(dds_entity_t writer, const void *data)
{
  // 调用dds_unregister_instance_ts函数，将时间参数设置为dds_time()
  return dds_unregister_instance_ts(writer, data, dds_time());
}

dds_return_t dds_unregister_instance_ih(dds_entity_t writer, dds_instance_handle_t handle)
{
  // 调用dds_unregister_instance_ih_ts函数，将时间参数设置为dds_time()
  return dds_unregister_instance_ih_ts(writer, handle, dds_time());
}
dds_return_t dds_unregister_instance_ts(dds_entity_t writer, const void *data, dds_time_t timestamp)
{
  dds_return_t ret;                                   // 定义返回值变量
  bool autodispose = true;                            // 定义自动销毁标志变量并初始化为true
  dds_write_action action = DDS_WR_ACTION_UNREGISTER; // 定义写入操作类型变量并初始化为DDS_WR_ACTION_UNREGISTER
  dds_writer *wr;                                     // 定义dds_writer类型指针变量

  if (data == NULL || timestamp < 0) // 如果数据为空或时间戳小于0，则返回错误码DDS_RETCODE_BAD_PARAMETER
    return DDS_RETCODE_BAD_PARAMETER;

  if ((ret = dds_writer_lock(writer, &wr)) != DDS_RETCODE_OK) // 如果获取锁失败，则返回相应的错误码
    return ret;

  if (wr->m_entity.m_qos)                                                   // 如果writer的QoS存在
    (void)dds_qget_writer_data_lifecycle(wr->m_entity.m_qos, &autodispose); // 获取writer的QoS中的自动销毁标志

  struct ddsi_thread_state *const thrst = ddsi_lookup_thread_state(); // 获取当前线程状态
  ddsi_thread_state_awake(thrst, &wr->m_entity.m_domain->gv);         // 唤醒当前线程状态
  if (autodispose)                                                    // 如果自动销毁标志为true
  {
    dds_instance_remove(wr->m_entity.m_domain, wr, data, DDS_HANDLE_NIL); // 从实例列表中移除该实例
    action |= DDS_WR_DISPOSE_BIT;                                         // 设置写入操作类型为DDS_WR_ACTION_UNREGISTER | DDS_WR_DISPOSE_BIT
  }
  ret = dds_write_impl(wr, data, timestamp, action); // 执行写入操作
  ddsi_thread_state_asleep(thrst);                   // 使当前线程状态休眠
  dds_writer_unlock(wr);                             // 解锁writer
  return ret;                                        // 返回执行结果
}
dds_return_t dds_unregister_instance_ih_ts(dds_entity_t writer, dds_instance_handle_t handle, dds_time_t timestamp)
{
  dds_return_t ret = DDS_RETCODE_OK;                  // 定义返回值变量ret并初始化为DDS_RETCODE_OK
  bool autodispose = true;                            // 定义自动销毁标志autodispose并初始化为true
  dds_write_action action = DDS_WR_ACTION_UNREGISTER; // 定义写入操作action并初始化为DDS_WR_ACTION_UNREGISTER
  dds_writer *wr;                                     // 定义指向dds_writer类型的指针wr
  struct ddsi_tkmap_instance *tk;                     // 定义指向ddsi_tkmap_instance类型的指针tk

  if ((ret = dds_writer_lock(writer, &wr)) != DDS_RETCODE_OK) // 如果dds_writer锁定失败，则返回错误码ret
    return ret;

  if (wr->m_entity.m_qos)                                                   // 如果dds_writer的实体有QoS
    (void)dds_qget_writer_data_lifecycle(wr->m_entity.m_qos, &autodispose); // 获取数据生命周期，并将结果存储在autodispose中

  struct ddsi_thread_state *const thrst = ddsi_lookup_thread_state(); // 获取当前线程状态
  ddsi_thread_state_awake(thrst, &wr->m_entity.m_domain->gv);         // 唤醒线程状态
  if (autodispose)                                                    // 如果自动销毁标志为真
  {
    dds_instance_remove(wr->m_entity.m_domain, wr, NULL, handle); // 从实例列表中删除实例
    action |= DDS_WR_DISPOSE_BIT;                                 // 设置写入操作为DDS_WR_DISPOSE_BIT
  }
  if ((tk = ddsi_tkmap_find_by_id(wr->m_entity.m_domain->gv.m_tkmap, handle)) == NULL) // 如果找不到指定的实例句柄
    ret = DDS_RETCODE_PRECONDITION_NOT_MET;                                            // 返回错误码DDS_RETCODE_PRECONDITION_NOT_MET
  else
  {
    struct ddsi_sertype *tp = wr->m_topic->m_stype;                       // 获取dds_writer的主题类型
    void *sample = ddsi_sertype_alloc_sample(tp);                         // 分配一个样本
    ddsi_serdata_untyped_to_sample(tp, tk->m_sample, sample, NULL, NULL); // 将序列化数据转换为样本数据
    ddsi_tkmap_instance_unref(wr->m_entity.m_domain->gv.m_tkmap, tk);     // 减少实例引用计数
    ret = dds_write_impl(wr, sample, timestamp, action);                  // 写入数据
    ddsi_sertype_free_sample(tp, sample, DDS_FREE_ALL);                   // 释放样本内存
  }
  ddsi_thread_state_asleep(thrst); // 线程状态设置为睡眠
  dds_writer_unlock(wr);           // 解锁dds_writer
  return ret;                      // 返回ret
}
// 定义函数 dds_writedispose_ts，参数为 writer、data 和 timestamp
dds_return_t dds_writedispose_ts(dds_entity_t writer, const void *data, dds_time_t timestamp)
{
  dds_return_t ret;
  dds_writer *wr;

  // 如果获取 writer 的锁失败，则返回错误码
  if ((ret = dds_writer_lock(writer, &wr)) != DDS_RETCODE_OK)
    return ret;

  // 获取当前线程状态
  struct ddsi_thread_state *const thrst = ddsi_lookup_thread_state();
  // 唤醒当前线程
  ddsi_thread_state_awake(thrst, &wr->m_entity.m_domain->gv);
  // 调用 dds_write_impl 函数进行写入操作，并指定写入类型为 DDS_WR_ACTION_WRITE_DISPOSE
  if ((ret = dds_write_impl(wr, data, timestamp, DDS_WR_ACTION_WRITE_DISPOSE)) == DDS_RETCODE_OK)
    // 如果写入成功，则从实例列表中移除该实例
    dds_instance_remove(wr->m_entity.m_domain, wr, data, DDS_HANDLE_NIL);
  // 使当前线程进入睡眠状态
  ddsi_thread_state_asleep(thrst);
  // 释放 writer 的锁
  dds_writer_unlock(wr);
  // 返回操作结果
  return ret;
}

// 定义函数 dds_dispose_impl，参数为 wr、data、handle 和 timestamp
static dds_return_t dds_dispose_impl(dds_writer *wr, const void *data, dds_instance_handle_t handle, dds_time_t timestamp) ddsrt_nonnull_all;

// 实现函数 dds_dispose_impl，参数为 wr、data、handle 和 timestamp
static dds_return_t dds_dispose_impl(dds_writer *wr, const void *data, dds_instance_handle_t handle, dds_time_t timestamp)
{
  dds_return_t ret;
  // 断言当前线程处于唤醒状态
  assert(ddsi_thread_is_awake());
  // 调用 dds_write_impl 函数进行写入操作，并指定写入类型为 DDS_WR_ACTION_DISPOSE
  if ((ret = dds_write_impl(wr, data, timestamp, DDS_WR_ACTION_DISPOSE)) == DDS_RETCODE_OK)
    // 如果写入成功，则从实例列表中移除该实例
    dds_instance_remove(wr->m_entity.m_domain, wr, data, handle);
  // 返回操作结果
  return ret;
}

// 定义函数 dds_dispose_ts，参数为 writer、data 和 timestamp
dds_return_t dds_dispose_ts(dds_entity_t writer, const void *data, dds_time_t timestamp)
{
  dds_return_t ret;
  dds_writer *wr;

  // 如果 data 为空，则返回错误码
  if (data == NULL)
    return DDS_RETCODE_BAD_PARAMETER;

  // 如果获取 writer 的锁失败，则返回错误码
  if ((ret = dds_writer_lock(writer, &wr)) != DDS_RETCODE_OK)
    return ret;

  // 获取当前线程状态
  struct ddsi_thread_state *const thrst = ddsi_lookup_thread_state();
  // 唤醒当前线程
  ddsi_thread_state_awake(thrst, &wr->m_entity.m_domain->gv);
  // 调用 dds_dispose_impl 函数进行释放操作
  ret = dds_dispose_impl(wr, data, DDS_HANDLE_NIL, timestamp);
  // 使当前线程进入睡眠状态
  ddsi_thread_state_asleep(thrst);
  // 释放 writer 的锁
  dds_writer_unlock(wr);
  // 返回操作结果
  return ret;
}

dds_return_t dds_dispose_ih_ts(dds_entity_t writer, dds_instance_handle_t handle, dds_time_t timestamp)
{
  dds_return_t ret; // 定义返回值变量
  dds_writer *wr;   // 定义写入器指针

  if ((ret = dds_writer_lock(writer, &wr)) != DDS_RETCODE_OK) // 锁定写入器
    return ret;                                               // 如果锁定失败，返回错误码

  struct ddsi_tkmap_instance *tk;                                                      // 定义实例映射指针
  struct ddsi_thread_state *const thrst = ddsi_lookup_thread_state();                  // 获取线程状态
  ddsi_thread_state_awake(thrst, &wr->m_entity.m_domain->gv);                          // 唤醒线程
  if ((tk = ddsi_tkmap_find_by_id(wr->m_entity.m_domain->gv.m_tkmap, handle)) == NULL) // 通过实例句柄查找实例映射
    ret = DDS_RETCODE_PRECONDITION_NOT_MET;                                            // 如果查找失败，返回错误码
  else
  {
    const struct ddsi_sertype *tp = wr->m_wr->type;                       // 获取写入器类型
    void *sample = ddsi_sertype_alloc_sample(tp);                         // 分配样本内存
    ddsi_serdata_untyped_to_sample(tp, tk->m_sample, sample, NULL, NULL); // 将序列化数据转换为样本数据
    ddsi_tkmap_instance_unref(wr->m_entity.m_domain->gv.m_tkmap, tk);     // 释放实例映射
    ret = dds_dispose_impl(wr, sample, handle, timestamp);                // 调用实际的释放函数
    ddsi_sertype_free_sample(tp, sample, DDS_FREE_ALL);                   // 释放样本内存
  }
  ddsi_thread_state_asleep(thrst); // 休眠线程
  dds_writer_unlock(wr);           // 解锁写入器
  return ret;                      // 返回结果
}

dds_instance_handle_t dds_lookup_instance(dds_entity_t entity, const void *data)
{
  const struct ddsi_sertype *sertype; // 定义序列化类型指针
  struct ddsi_serdata *sd;            // 定义序列化数据指针
  dds_entity *w_or_r;                 // 定义实体指针

  if (data == NULL) // 如果数据为空，返回空句柄
    return DDS_HANDLE_NIL;

  if (dds_entity_lock(entity, DDS_KIND_DONTCARE, &w_or_r) < 0) // 锁定实体 return DDS_HANDLE_NIL; // 如果锁定失败，返回空句柄 switch (dds_entity_kind(w_or_r)) // 判断实体类型 { case DDS_KIND_WRITER: // 如果是写入器 sertype = ((dds_writer *)w_or_r)->m_wr->type; // 获取写入器类型
    break;
case DDS_KIND_READER: // 如果是读取器
  // FIXME: used for serdata_from_sample, so maybe this should take the derived sertype for a specific data-representation?
  sertype = ((dds_reader *)w_or_r)->m_topic->m_stype; // 获取读取器类型
  break;
default: // 如果是其他类型的实体，返回空句柄
  dds_entity_unlock(w_or_r);
  return DDS_HANDLE_NIL;
}

dds_instance_handle_t ih;                                            // 定义实例句柄
struct ddsi_thread_state *const thrst = ddsi_lookup_thread_state();  // 获取线程状态
ddsi_thread_state_awake(thrst, &w_or_r->m_domain->gv);               // 唤醒线程
if ((sd = ddsi_serdata_from_sample(sertype, SDK_KEY, data)) == NULL) // 将数据转换为序列化数据
  ih = DDS_HANDLE_NIL;                                               // 如果转换失败，返回空句柄
else
{
  ih = ddsi_tkmap_lookup(w_or_r->m_domain->gv.m_tkmap, sd); // 通过序列化数据查找实例句柄
  ddsi_serdata_unref(sd);                                   // 释放序列化数据
}
ddsi_thread_state_asleep(thrst); // 休眠线程
dds_entity_unlock(w_or_r);       // 解锁实体
return ih;                       // 返回实例句柄
}

// 获取实例的键值
dds_return_t dds_instance_get_key(dds_entity_t entity, dds_instance_handle_t ih, void *data)
{
  dds_return_t ret;
  const dds_topic *topic;
  struct ddsi_tkmap_instance *tk;
  dds_entity *e;

  // 检查参数是否合法
  if (data == NULL)
    return DDS_RETCODE_BAD_PARAMETER;

  // 锁定实体
  if ((ret = dds_entity_lock(entity, DDS_KIND_DONTCARE, &e)) < 0)
    return ret;

  // 根据实体类型获取主题
  switch (dds_entity_kind(e))
  {
  case DDS_KIND_WRITER:
    topic = ((dds_writer *)e)->m_topic;
    break;
  case DDS_KIND_READER:
    topic = ((dds_reader *)e)->m_topic;
    break;
  case DDS_KIND_COND_READ:
  case DDS_KIND_COND_QUERY:
    topic = ((dds_reader *)e->m_parent)->m_topic;
    break;
  default:
    dds_entity_unlock(e);
    return DDS_RETCODE_ILLEGAL_OPERATION;
  }

  // 获取线程状态
  struct ddsi_thread_state *const thrst = ddsi_lookup_thread_state();
  // 唤醒线程
  ddsi_thread_state_awake(thrst, &e->m_domain->gv);

  // 根据实例句柄查找实例
  if ((tk = ddsi_tkmap_find_by_id(e->m_domain->gv.m_tkmap, ih)) == NULL)
    ret = DDS_RETCODE_BAD_PARAMETER;
  else
  {
    // 使用主题中的序列化类型将序列化数据转换为样本数据
    ddsi_sertype_zero_sample(topic->m_stype, data);
    ddsi_serdata_untyped_to_sample(topic->m_stype, tk->m_sample, data, NULL, NULL);
    // 释放实例引用
    ddsi_tkmap_instance_unref(e->m_domain->gv.m_tkmap, tk);
    ret = DDS_RETCODE_OK;
  }

  // 休眠线程
  ddsi_thread_state_asleep(thrst);
  // 解锁实体
  dds_entity_unlock(e);
  return ret;
}
