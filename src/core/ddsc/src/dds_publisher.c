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
#include "dds/ddsi/ddsi_entity.h"
#include "dds/ddsi/ddsi_iid.h"
#include "dds/ddsrt/misc.h"
#include "dds/version.h"
#include "dds__listener.h"
#include "dds__participant.h"
#include "dds__publisher.h"
#include "dds__qos.h"
#include "dds__writer.h"
#include <assert.h>
#include <string.h>

DECL_ENTITY_LOCK_UNLOCK(dds_publisher)

#define DDS_PUBLISHER_STATUS_MASK (0u)

/**
 * @brief 设置发布者的QoS参数
 *
 * @param e      实体指针
 * @param qos    QoS参数指针
 * @param enabled 是否启用
 * @return dds_return_t 返回DDS_RETCODE_OK
 */
static dds_return_t dds_publisher_qos_set(dds_entity *e, const dds_qos_t *qos, bool enabled)
{
  /* 注意: e->m_qos仍然是旧的，以允许在此处失败 */
  (void)e;
  (void)qos;
  (void)enabled;
  return DDS_RETCODE_OK;
}

/**
 * @brief 验证发布者状态掩码
 *
 * @param mask 状态掩码
 * @return dds_return_t 返回DDS_RETCODE_BAD_PARAMETER或DDS_RETCODE_OK
 */
static dds_return_t dds_publisher_status_validate(uint32_t mask)
{
  return (mask & ~DDS_PUBLISHER_STATUS_MASK) ? DDS_RETCODE_BAD_PARAMETER : DDS_RETCODE_OK;
}

/* dds_entity_deriver_publisher结构体定义 */
const struct dds_entity_deriver dds_entity_deriver_publisher = {
    .interrupt = dds_entity_deriver_dummy_interrupt,
    .close = dds_entity_deriver_dummy_close,
    .delete = dds_entity_deriver_dummy_delete,
    .set_qos = dds_publisher_qos_set,
    .validate_status = dds_publisher_status_validate,
    .create_statistics = dds_entity_deriver_dummy_create_statistics,
    .refresh_statistics = dds_entity_deriver_dummy_refresh_statistics};

/**
 * @brief 创建一个发布者实体（内部函数）
 *
 * @param par       参与者指针
 * @param implicit  是否为隐式创建
 * @param qos       质量服务设置
 * @param listener  监听器
 * @return dds_entity_t 返回创建的发布者实体
 */
dds_entity_t dds__create_publisher_l(dds_participant *par, bool implicit, const dds_qos_t *qos, const dds_listener_t *listener)
{
  dds_publisher *pub; // 发布者指针
  dds_entity_t hdl;   // 发布者实体句柄
  dds_qos_t *new_qos; // 新的质量服务设置
  dds_return_t ret;   // 返回值

  // 创建新的质量服务设置
  new_qos = dds_create_qos();

  // 如果提供了qos参数，则合并缺失的qos设置
  if (qos)
    ddsi_xqos_mergein_missing(new_qos, qos, DDS_PUBLISHER_QOS_MASK);

  // 合并默认的发布者/订阅者qos设置
  ddsi_xqos_mergein_missing(new_qos, &ddsi_default_qos_publisher_subscriber, ~(uint64_t)0);

  // 应用实体命名规则
  dds_apply_entity_naming(new_qos, par->m_entity.m_qos, &par->m_entity.m_domain->gv);

  // 检查新的qos设置是否有效
  if ((ret = ddsi_xqos_valid(&par->m_entity.m_domain->gv.logconfig, new_qos)) != DDS_RETCODE_OK)
  {
    // 如果无效，则删除新的qos设置并返回错误
    dds_delete_qos(new_qos);
    return ret;
  }

  // 分配发布者内存空间
  pub = dds_alloc(sizeof(*pub));

  // 初始化发布者实体
  hdl = dds_entity_init(&pub->m_entity, &par->m_entity, DDS_KIND_PUBLISHER, implicit, true, new_qos, listener, DDS_PUBLISHER_STATUS_MASK);

  // 生成实例标识符
  pub->m_entity.m_iid = ddsi_iid_gen();

  // 注册子实体
  dds_entity_register_child(&par->m_entity, &pub->m_entity);

  // 完成实体初始化
  dds_entity_init_complete(&pub->m_entity);

  // 返回发布者实体句柄
  return hdl;
}

/**
 * @brief 创建一个发布者实体
 *
 * @param participant 参与者实体
 * @param qos         质量服务设置
 * @param listener    监听器
 * @return dds_entity_t 返回创建的发布者实体
 */
dds_entity_t dds_create_publisher(dds_entity_t participant, const dds_qos_t *qos, const dds_listener_t *listener)
{
  dds_participant *par; // 参与者指针
  dds_entity_t hdl;     // 发布者实体句柄
  dds_return_t ret;     // 返回值

  // 锁定参与者并检查返回值
  if ((ret = dds_participant_lock(participant, &par)) != DDS_RETCODE_OK)
    return ret;

  // 创建发布者实体
  hdl = dds__create_publisher_l(par, false, qos, listener);

  // 解锁参与者
  dds_participant_unlock(par);

  // 返回发布者实体句柄
  return hdl;
}

/**
 * @brief 暂停发布者操作（未实现）
 *
 * @param publisher 发布者实体
 * @return dds_return_t 返回操作结果
 */
dds_return_t dds_suspend(dds_entity_t publisher)
{
  return dds_generic_unimplemented_operation(publisher, DDS_KIND_PUBLISHER);
}

/**
 * @brief 恢复发布者操作（未实现）
 *
 * @param publisher 发布者实体
 * @return dds_return_t 返回操作结果
 */
dds_return_t dds_resume(dds_entity_t publisher)
{
  return dds_generic_unimplemented_operation(publisher, DDS_KIND_PUBLISHER);
}

/**
 * @brief 等待发布者或写入者的确认
 *
 * @param publisher_or_writer 发布者或写入者实体
 * @param timeout             超时时间
 * @return dds_return_t 返回操作结果
 */
dds_return_t dds_wait_for_acks(dds_entity_t publisher_or_writer, dds_duration_t timeout)
{
  dds_return_t ret;       // 返回值
  dds_entity *p_or_w_ent; // 发布者或写入者实体指针

  // 检查超时参数是否合法
  if (timeout < 0)
    return DDS_RETCODE_BAD_PARAMETER;

  // 锁定实体并检查返回值
  if ((ret = dds_entity_pin(publisher_or_writer, &p_or_w_ent)) < 0)
    return ret;

  const dds_time_t tnow = dds_time(); // 当前时间
  // 计算绝对超时时间
  const dds_time_t abstimeout = (DDS_INFINITY - timeout <= tnow) ? DDS_NEVER : (tnow + timeout);

  // 根据实体类型进行处理
  switch (dds_entity_kind(p_or_w_ent))
  {
  case DDS_KIND_PUBLISHER:
    /* FIXME: wait_for_acks on all writers of the same publisher */
    dds_entity_unpin(p_or_w_ent);
    return DDS_RETCODE_UNSUPPORTED;

  case DDS_KIND_WRITER:
    ret = dds__ddsi_writer_wait_for_acks((struct dds_writer *)p_or_w_ent, NULL, abstimeout);
    dds_entity_unpin(p_or_w_ent);
    return ret;

  default:
    dds_entity_unpin(p_or_w_ent);
    return DDS_RETCODE_ILLEGAL_OPERATION;
  }
}

/**
 * @brief 开始发布者的一致性操作（未实现）
 *
 * @param publisher 发布者实体
 * @return dds_return_t 返回操作结果
 */
dds_return_t dds_publisher_begin_coherent(dds_entity_t publisher)
{
  return dds_generic_unimplemented_operation(publisher, DDS_KIND_PUBLISHER);
}

/**
 * @brief 结束发布者的一致性操作（未实现）
 *
 * @param publisher 发布者实体
 * @return dds_return_t 返回操作结果
 */
dds_return_t dds_publisher_end_coherent(dds_entity_t publisher)
{
  return dds_generic_unimplemented_operation(publisher, DDS_KIND_PUBLISHER);
}
