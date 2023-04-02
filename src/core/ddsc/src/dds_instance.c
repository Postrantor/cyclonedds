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

/**
 * @brief 写入并销毁数据。
 *
 * @param[in] writer 写实体的句柄。
 * @param[in] data 要写入并销毁的数据指针。
 * @return dds_return_t 操作结果。
 */
dds_return_t dds_writedispose(dds_entity_t writer, const void *data) {
  // 调用dds_writedispose_ts函数，传入当前时间作为时间戳参数
  return dds_writedispose_ts(writer, data, dds_time());
}

/**
 * @brief 销毁数据。
 *
 * @param[in] writer 写实体的句柄。
 * @param[in] data 要销毁的数据指针。
 * @return dds_return_t 操作结果。
 */
dds_return_t dds_dispose(dds_entity_t writer, const void *data) {
  // 调用dds_dispose_ts函数，传入当前时间作为时间戳参数
  return dds_dispose_ts(writer, data, dds_time());
}

/**
 * @brief 根据实例句柄销毁数据。
 *
 * @param[in] writer 写实体的句柄。
 * @param[in] handle 实例句柄。
 * @return dds_return_t 操作结果。
 */
dds_return_t dds_dispose_ih(dds_entity_t writer, dds_instance_handle_t handle) {
  // 调用dds_dispose_ih_ts函数，传入当前时间作为时间戳参数
  return dds_dispose_ih_ts(writer, handle, dds_time());
}

/**
 * @brief 查找或创建实例。
 *
 * @param[in] writer 写实体的指针。
 * @param[in] data 要查找或创建的数据指针。
 * @param[in] create 是否创建新实例（如果不存在）。
 * @return struct ddsi_tkmap_instance* 实例指针，如果未找到且不创建，则为NULL。
 */
static struct ddsi_tkmap_instance *dds_instance_find(const dds_writer *writer,
                                                     const void *data,
                                                     const bool create) {
  // 将样本数据转换为序列化数据
  struct ddsi_serdata *sd = ddsi_serdata_from_sample(writer->m_wr->type, SDK_KEY, data);

  // 如果序列化数据为空，返回NULL
  if (sd == NULL) return NULL;

  // 查找或创建实例
  struct ddsi_tkmap_instance *inst =
      ddsi_tkmap_find(writer->m_entity.m_domain->gv.m_tkmap, sd, create);

  // 取消对序列化数据的引用
  ddsi_serdata_unref(sd);

  // 返回实例指针
  return inst;
}

/**
 * @brief 移除实例
 *
 * @param[in] dom      域指针，用于获取全局变量
 * @param[in] writer   写入器指针，用于查找实例
 * @param[in] data     数据指针，用于查找实例
 * @param[in] handle   实例句柄，用于查找实例
 */
static void dds_instance_remove(struct dds_domain *dom,
                                const dds_writer *writer,
                                const void *data,
                                dds_instance_handle_t handle) {
  // 定义实例指针
  struct ddsi_tkmap_instance *inst;

  // 如果句柄不为空，则通过句柄查找实例
  if (handle != DDS_HANDLE_NIL)
    inst = ddsi_tkmap_find_by_id(dom->gv.m_tkmap, handle);
  else {
    // 否则，通过数据查找实例
    assert(data);
    inst = dds_instance_find(writer, data, false);
  }

  // 如果找到实例，则取消引用
  if (inst) {
    ddsi_tkmap_instance_unref(dom->gv.m_tkmap, inst);
  }
}

/**
 * @brief 注册实例
 *
 * @param[in]  writer  写入器实体
 * @param[out] handle  返回实例句柄
 * @param[in]  data    数据指针
 * @return 成功返回DDS_RETCODE_OK，失败返回相应错误码
 */
dds_return_t dds_register_instance(dds_entity_t writer,
                                   dds_instance_handle_t *handle,
                                   const void *data) {
  // 定义写入器指针和返回值
  dds_writer *wr;
  dds_return_t ret;

  // 检查输入参数是否为空
  if (data == NULL || handle == NULL) return DDS_RETCODE_BAD_PARAMETER;

  // 锁定写入器
  if ((ret = dds_writer_lock(writer, &wr)) != DDS_RETCODE_OK) return ret;

  // 查找线程状态
  struct ddsi_thread_state *const thrst = ddsi_lookup_thread_state();

  // 唤醒线程状态
  ddsi_thread_state_awake(thrst, &wr->m_entity.m_domain->gv);

  // 查找实例
  struct ddsi_tkmap_instance *const inst = dds_instance_find(wr, data, true);

  // 如果实例为空，返回错误码
  if (inst == NULL)
    ret = DDS_RETCODE_BAD_PARAMETER;
  else {
    // 否则，设置句柄并返回成功
    *handle = inst->m_iid;
    ret = DDS_RETCODE_OK;
  }

  // 线程进入休眠状态
  ddsi_thread_state_asleep(thrst);

  // 解锁写入器
  dds_writer_unlock(wr);

  // 返回结果
  return ret;
}

/**
 * @brief 取消注册实例
 *
 * @param[in] writer 写入器实体
 * @param[in] data 实例数据
 * @return dds_return_t 返回操作结果
 */
dds_return_t dds_unregister_instance(dds_entity_t writer, const void *data) {
  // 调用带时间戳的取消注册实例函数
  return dds_unregister_instance_ts(writer, data, dds_time());
}

/**
 * @brief 根据实例句柄取消注册实例
 *
 * @param[in] writer 写入器实体
 * @param[in] handle 实例句柄
 * @return dds_return_t 返回操作结果
 */
dds_return_t dds_unregister_instance_ih(dds_entity_t writer, dds_instance_handle_t handle) {
  // 调用带时间戳的根据实例句柄取消注册实例函数
  return dds_unregister_instance_ih_ts(writer, handle, dds_time());
}

/**
 * @brief 带时间戳的取消注册实例
 *
 * @param[in] writer 写入器实体
 * @param[in] data 实例数据
 * @param[in] timestamp 时间戳
 * @return dds_return_t 返回操作结果
 */
dds_return_t dds_unregister_instance_ts(dds_entity_t writer,
                                        const void *data,
                                        dds_time_t timestamp) {
  dds_return_t ret;
  bool autodispose = true;
  dds_write_action action = DDS_WR_ACTION_UNREGISTER;
  dds_writer *wr;

  // 检查输入参数是否有效
  if (data == NULL || timestamp < 0) return DDS_RETCODE_BAD_PARAMETER;

  // 锁定写入器
  if ((ret = dds_writer_lock(writer, &wr)) != DDS_RETCODE_OK) return ret;

  // 获取写入器的数据生命周期
  if (wr->m_entity.m_qos) (void)dds_qget_writer_data_lifecycle(wr->m_entity.m_qos, &autodispose);

  // 查找线程状态并唤醒
  struct ddsi_thread_state *const thrst = ddsi_lookup_thread_state();
  ddsi_thread_state_awake(thrst, &wr->m_entity.m_domain->gv);

  // 如果自动处理，则移除实例并设置操作位
  if (autodispose) {
    dds_instance_remove(wr->m_entity.m_domain, wr, data, DDS_HANDLE_NIL);
    action |= DDS_WR_DISPOSE_BIT;
  }

  // 调用写入实现函数
  ret = dds_write_impl(wr, data, timestamp, action);

  // 设置线程状态为休眠
  ddsi_thread_state_asleep(thrst);

  // 解锁写入器
  dds_writer_unlock(wr);

  // 返回操作结果
  return ret;
}

/**
 * @brief 取消注册实例句柄并设置时间戳
 *
 * 该函数用于取消注册给定的实例句柄，并根据提供的时间戳进行操作。
 *
 * @param[in] writer       写入器实体标识符
 * @param[in] handle       实例句柄
 * @param[in] timestamp    时间戳
 * @return dds_return_t    返回操作结果，成功时返回 DDS_RETCODE_OK
 */
dds_return_t dds_unregister_instance_ih_ts(dds_entity_t writer,
                                           dds_instance_handle_t handle,
                                           dds_time_t timestamp) {
  // 定义返回值，默认为 DDS_RETCODE_OK
  dds_return_t ret = DDS_RETCODE_OK;
  // 定义自动处理标志，默认为 true
  bool autodispose = true;
  // 定义写入操作，默认为取消注册
  dds_write_action action = DDS_WR_ACTION_UNREGISTER;
  // 定义写入器指针
  dds_writer *wr;
  // 定义实例映射结构指针
  struct ddsi_tkmap_instance *tk;

  // 尝试锁定写入器，如果失败则返回错误码
  if ((ret = dds_writer_lock(writer, &wr)) != DDS_RETCODE_OK) return ret;

  // 如果存在 QoS 设置，则获取写入器数据生命周期配置
  if (wr->m_entity.m_qos) (void)dds_qget_writer_data_lifecycle(wr->m_entity.m_qos, &autodispose);

  // 查找线程状态并唤醒
  struct ddsi_thread_state *const thrst = ddsi_lookup_thread_state();
  ddsi_thread_state_awake(thrst, &wr->m_entity.m_domain->gv);

  // 如果自动处理标志为 true，则移除实例并设置操作位
  if (autodispose) {
    dds_instance_remove(wr->m_entity.m_domain, wr, NULL, handle);
    action |= DDS_WR_DISPOSE_BIT;
  }

  // 根据实例句柄查找映射，如果未找到则返回错误码
  if ((tk = ddsi_tkmap_find_by_id(wr->m_entity.m_domain->gv.m_tkmap, handle)) == NULL)
    ret = DDS_RETCODE_PRECONDITION_NOT_MET;
  else {
    // 获取序列化类型和样本
    struct ddsi_sertype *tp = wr->m_topic->m_stype;
    void *sample = ddsi_sertype_alloc_sample(tp);
    ddsi_serdata_untyped_to_sample(tp, tk->m_sample, sample, NULL, NULL);
    // 取消实例映射的引用
    ddsi_tkmap_instance_unref(wr->m_entity.m_domain->gv.m_tkmap, tk);
    // 调用写入实现函数
    ret = dds_write_impl(wr, sample, timestamp, action);
    // 释放样本内存
    ddsi_sertype_free_sample(tp, sample, DDS_FREE_ALL);
  }

  // 线程进入休眠状态
  ddsi_thread_state_asleep(thrst);
  // 解锁写入器
  dds_writer_unlock(wr);
  // 返回操作结果
  return ret;
}

/**
 * @brief 写入并销毁数据的时间戳版本
 *
 * @param[in] writer    写实体的句柄
 * @param[in] data      要写入和销毁的数据指针
 * @param[in] timestamp 时间戳
 *
 * @return 成功时返回 DDS_RETCODE_OK，否则返回错误代码
 */
dds_return_t dds_writedispose_ts(dds_entity_t writer, const void *data, dds_time_t timestamp) {
  dds_return_t ret;
  dds_writer *wr;

  // 锁定写实体
  if ((ret = dds_writer_lock(writer, &wr)) != DDS_RETCODE_OK) return ret;

  // 查找线程状态
  struct ddsi_thread_state *const thrst = ddsi_lookup_thread_state();
  // 唤醒线程状态
  ddsi_thread_state_awake(thrst, &wr->m_entity.m_domain->gv);
  // 实现写入并销毁操作
  if ((ret = dds_write_impl(wr, data, timestamp, DDS_WR_ACTION_WRITE_DISPOSE)) == DDS_RETCODE_OK)
    // 移除实例
    dds_instance_remove(wr->m_entity.m_domain, wr, data, DDS_HANDLE_NIL);
  // 线程进入休眠状态
  ddsi_thread_state_asleep(thrst);
  // 解锁写实体
  dds_writer_unlock(wr);
  return ret;
}

// 声明一个静态函数 dds_dispose_impl
static dds_return_t dds_dispose_impl(dds_writer *wr,
                                     const void *data,
                                     dds_instance_handle_t handle,
                                     dds_time_t timestamp) ddsrt_nonnull_all;

/**
 * @brief 实现销毁操作
 *
 * @param[in] wr        写实体指针
 * @param[in] data      要销毁的数据指针
 * @param[in] handle    实例句柄
 * @param[in] timestamp 时间戳
 *
 * @return 成功时返回 DDS_RETCODE_OK，否则返回错误代码
 */
static dds_return_t dds_dispose_impl(dds_writer *wr,
                                     const void *data,
                                     dds_instance_handle_t handle,
                                     dds_time_t timestamp) {
  dds_return_t ret;
  // 确保线程处于唤醒状态
  assert(ddsi_thread_is_awake());
  // 实现销毁操作
  if ((ret = dds_write_impl(wr, data, timestamp, DDS_WR_ACTION_DISPOSE)) == DDS_RETCODE_OK)
    // 移除实例
    dds_instance_remove(wr->m_entity.m_domain, wr, data, handle);
  return ret;
}

/**
 * @brief 以给定的时间戳销毁数据样本。
 *
 * @param[in] writer     写入实体的句柄。
 * @param[in] data       要销毁的数据样本。
 * @param[in] timestamp  销毁操作的时间戳。
 *
 * @return 成功时返回 DDS_RETCODE_OK，否则返回相应的错误代码。
 */
dds_return_t dds_dispose_ts(dds_entity_t writer, const void *data, dds_time_t timestamp) {
  dds_return_t ret;
  dds_writer *wr;

  // 检查输入参数是否有效
  if (data == NULL) return DDS_RETCODE_BAD_PARAMETER;

  // 尝试锁定写入实体
  if ((ret = dds_writer_lock(writer, &wr)) != DDS_RETCODE_OK) return ret;

  struct ddsi_thread_state *const thrst = ddsi_lookup_thread_state();
  // 唤醒线程状态
  ddsi_thread_state_awake(thrst, &wr->m_entity.m_domain->gv);
  // 执行销毁操作
  ret = dds_dispose_impl(wr, data, DDS_HANDLE_NIL, timestamp);
  // 设置线程状态为休眠
  ddsi_thread_state_asleep(thrst);
  // 解锁写入实体
  dds_writer_unlock(wr);
  return ret;
}

/**
 * @brief 以给定的时间戳和实例句柄销毁数据样本。
 *
 * @param[in] writer     写入实体的句柄。
 * @param[in] handle     要销毁的数据样本的实例句柄。
 * @param[in] timestamp  销毁操作的时间戳。
 *
 * @return 成功时返回 DDS_RETCODE_OK，否则返回相应的错误代码。
 */
dds_return_t dds_dispose_ih_ts(dds_entity_t writer,
                               dds_instance_handle_t handle,
                               dds_time_t timestamp) {
  dds_return_t ret;
  dds_writer *wr;

  // 尝试锁定写入实体
  if ((ret = dds_writer_lock(writer, &wr)) != DDS_RETCODE_OK) return ret;

  struct ddsi_tkmap_instance *tk;
  struct ddsi_thread_state *const thrst = ddsi_lookup_thread_state();
  // 唤醒线程状态
  ddsi_thread_state_awake(thrst, &wr->m_entity.m_domain->gv);
  // 根据实例句柄查找映射实例
  if ((tk = ddsi_tkmap_find_by_id(wr->m_entity.m_domain->gv.m_tkmap, handle)) == NULL)
    ret = DDS_RETCODE_PRECONDITION_NOT_MET;
  else {
    const struct ddsi_sertype *tp = wr->m_wr->type;
    // 分配样本内存
    void *sample = ddsi_sertype_alloc_sample(tp);
    // 将未类型化的序列化数据转换为样本
    ddsi_serdata_untyped_to_sample(tp, tk->m_sample, sample, NULL, NULL);
    // 减少映射实例的引用计数
    ddsi_tkmap_instance_unref(wr->m_entity.m_domain->gv.m_tkmap, tk);
    // 执行销毁操作
    ret = dds_dispose_impl(wr, sample, handle, timestamp);
    // 释放样本内存
    ddsi_sertype_free_sample(tp, sample, DDS_FREE_ALL);
  }
  // 设置线程状态为休眠
  ddsi_thread_state_asleep(thrst);
  // 解锁写入实体
  dds_writer_unlock(wr);
  return ret;
}

/**
 * @brief 查找实例句柄 (Lookup instance handle)
 *
 * @param[in] entity 实体对象，可以是读取器或写入器 (Entity object, can be a reader or writer)
 * @param[in] data 数据指针，用于查找实例句柄 (Data pointer for looking up the instance handle)
 * @return 返回实例句柄，如果失败则返回 DDS_HANDLE_NIL (Returns the instance handle, or
 * DDS_HANDLE_NIL on failure)
 */
dds_instance_handle_t dds_lookup_instance(dds_entity_t entity, const void *data) {
  // 定义序列化类型和序列化数据结构
  const struct ddsi_sertype *sertype;
  struct ddsi_serdata *sd;
  dds_entity *w_or_r;

  // 如果数据为空，则返回 DDS_HANDLE_NIL
  if (data == NULL) return DDS_HANDLE_NIL;

  // 锁定实体对象
  if (dds_entity_lock(entity, DDS_KIND_DONTCARE, &w_or_r) < 0) return DDS_HANDLE_NIL;
  switch (dds_entity_kind(w_or_r)) {
    case DDS_KIND_WRITER:
      // 获取写入器的序列化类型
      sertype = ((dds_writer *)w_or_r)->m_wr->type;
      break;
    case DDS_KIND_READER:
      // 获取读取器的序列化类型
      // FIXME: used for serdata_from_sample, so maybe this should take the derived sertype for a
      // specific data-representation?
      sertype = ((dds_reader *)w_or_r)->m_topic->m_stype;
      break;
    default:
      // 解锁实体对象并返回 DDS_HANDLE_NIL
      dds_entity_unlock(w_or_r);
      return DDS_HANDLE_NIL;
  }

  // 定义实例句柄
  dds_instance_handle_t ih;
  // 查找线程状态
  struct ddsi_thread_state *const thrst = ddsi_lookup_thread_state();
  // 唤醒线程状态
  ddsi_thread_state_awake(thrst, &w_or_r->m_domain->gv);
  // 从样本中获取序列化数据
  if ((sd = ddsi_serdata_from_sample(sertype, SDK_KEY, data)) == NULL)
    // 如果序列化数据为空，则返回 DDS_HANDLE_NIL
    ih = DDS_HANDLE_NIL;
  else {
    // 查找实例句柄
    ih = ddsi_tkmap_lookup(w_or_r->m_domain->gv.m_tkmap, sd);
    // 取消引用序列化数据
    ddsi_serdata_unref(sd);
  }
  // 线程状态进入睡眠
  ddsi_thread_state_asleep(thrst);
  // 解锁实体对象
  dds_entity_unlock(w_or_r);
  // 返回实例句柄
  return ih;
}

/**
 * @brief 获取实例的键值 (Get the key value of an instance)
 *
 * @param[in] entity 实体对象，可以是读取器、写入器或条件变量 (Entity object, can be a reader,
 * writer or condition variable)
 * @param[in] ih 实例句柄 (Instance handle)
 * @param[out] data 存储键值的缓冲区 (Buffer to store the key value)
 * @return dds_return_t 操作结果 (Operation result)
 */
dds_return_t dds_instance_get_key(dds_entity_t entity, dds_instance_handle_t ih, void *data) {
  // 定义返回值变量
  dds_return_t ret;
  // 定义主题指针
  const dds_topic *topic;
  // 定义实例映射结构体指针
  struct ddsi_tkmap_instance *tk;
  // 定义实体指针
  dds_entity *e;

  // 判断data是否为空，如果为空则返回错误参数
  if (data == NULL) return DDS_RETCODE_BAD_PARAMETER;

  // 锁定实体对象并获取实体指针
  if ((ret = dds_entity_lock(entity, DDS_KIND_DONTCARE, &e)) < 0) return ret;
  // 根据实体类型获取主题
  switch (dds_entity_kind(e)) {
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

  // 查找线程状态
  struct ddsi_thread_state *const thrst = ddsi_lookup_thread_state();
  // 唤醒线程状态
  ddsi_thread_state_awake(thrst, &e->m_domain->gv);
  // 根据实例句柄查找实例映射
  if ((tk = ddsi_tkmap_find_by_id(e->m_domain->gv.m_tkmap, ih)) == NULL)
    ret = DDS_RETCODE_BAD_PARAMETER;
  else {
    // 使用主题的序列化类型将样本置零
    ddsi_sertype_zero_sample(topic->m_stype, data);
    // 将未类型化的数据转换为样本
    ddsi_serdata_untyped_to_sample(topic->m_stype, tk->m_sample, data, NULL, NULL);
    // 减少实例映射的引用计数
    ddsi_tkmap_instance_unref(e->m_domain->gv.m_tkmap, tk);
    // 设置返回值为成功
    ret = DDS_RETCODE_OK;
  }
  // 线程状态进入休眠
  ddsi_thread_state_asleep(thrst);
  // 解锁实体对象
  dds_entity_unlock(e);
  // 返回操作结果
  return ret;
}
