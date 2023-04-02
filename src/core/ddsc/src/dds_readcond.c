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
#include "dds/ddsc/dds_rhc.h"
#include "dds/ddsi/ddsi_entity.h"
#include "dds/ddsi/ddsi_entity_index.h"
#include "dds/ddsi/ddsi_iid.h"
#include "dds/ddsi/ddsi_thread.h"
#include "dds__entity.h"
#include "dds__readcond.h"
#include "dds__reader.h"
#include <assert.h>

/**
 * @brief 关闭读取条件
 *
 * @param[in] e 读取条件实体指针
 */
static void dds_readcond_close(dds_entity *e) ddsrt_nonnull_all;

/**
 * @brief 实际关闭读取条件的函数
 *
 * @param[in] e 读取条件实体指针
 */
static void dds_readcond_close(dds_entity *e)
{
  // 从 RHC 中分离读取条件，以便在释放之前进行处理
  struct dds_reader *const rd = (struct dds_reader *)e->m_parent;
  assert(dds_entity_kind(&rd->m_entity) == DDS_KIND_READER);
  dds_rhc_remove_readcondition(rd->m_rhc, (dds_readcond *)e);
}

// 定义读取条件实体派生器
const struct dds_entity_deriver dds_entity_deriver_readcondition = {
    .interrupt = dds_entity_deriver_dummy_interrupt,
    .close = dds_readcond_close,
    .delete = dds_entity_deriver_dummy_delete,
    .set_qos = dds_entity_deriver_dummy_set_qos,
    .validate_status = dds_entity_deriver_dummy_validate_status,
    .create_statistics = dds_entity_deriver_dummy_create_statistics,
    .refresh_statistics = dds_entity_deriver_dummy_refresh_statistics};

/**
 * @brief 创建读取条件实现
 *
 * @param[in] rd 读取器指针
 * @param[in] kind 实体类型
 * @param[in] mask 状态掩码
 * @param[in] filter 查询条件过滤函数
 * @return 返回创建的读取条件指针
 */
dds_readcond *dds_create_readcond_impl(dds_reader *rd, dds_entity_kind_t kind, uint32_t mask, dds_querycondition_filter_fn filter)
{
  // 分配读取条件内存
  dds_readcond *cond = dds_alloc(sizeof(*cond));
  assert((kind == DDS_KIND_COND_READ && filter == 0) || (kind == DDS_KIND_COND_QUERY && filter != 0));

  // 初始化实体
  (void)dds_entity_init(&cond->m_entity, &rd->m_entity, kind, false, true, NULL, NULL, 0);
  cond->m_entity.m_iid = ddsi_iid_gen();

  // 注册子实体
  dds_entity_register_child(&rd->m_entity, &cond->m_entity);

  // 设置状态掩码
  cond->m_sample_states = mask & DDS_ANY_SAMPLE_STATE;
  cond->m_view_states = mask & DDS_ANY_VIEW_STATE;
  cond->m_instance_states = mask & DDS_ANY_INSTANCE_STATE;

  // 如果是查询条件，设置过滤函数和查询掩码
  if (kind == DDS_KIND_COND_QUERY)
  {
    cond->m_query.m_filter = filter;
    cond->m_query.m_qcmask = 0;
  }

  // 将读取条件添加到 RHC
  if (!dds_rhc_add_readcondition(rd->m_rhc, cond))
  {
    /* FIXME: current entity management code can't deal with an error late in the creation of the
       entity because it doesn't allow deleting it again ... */
    abort();
  }

  return cond;
}
/**
 * @brief 创建一个读取条件实体
 *
 * @param reader 读取器实体
 * @param mask 状态掩码
 * @return 成功时返回创建的读取条件实体，失败时返回错误代码
 */
dds_entity_t dds_create_readcondition(dds_entity_t reader, uint32_t mask)
{
  // 定义读取器指针
  dds_reader *rd;
  // 定义返回值变量
  dds_return_t rc;
  // 尝试锁定读取器
  if ((rc = dds_reader_lock(reader, &rd)) != DDS_RETCODE_OK)
    return rc;
  else
  {
    // 定义实体句柄
    dds_entity_t hdl;
    // 创建读取条件实现
    dds_readcond *cond = dds_create_readcond_impl(rd, DDS_KIND_COND_READ, mask, 0);
    // 断言条件不为空
    assert(cond);
    // 获取实体句柄
    hdl = cond->m_entity.m_hdllink.hdl;
    // 完成实体初始化
    dds_entity_init_complete(&cond->m_entity);
    // 解锁读取器
    dds_reader_unlock(rd);
    // 返回实体句柄
    return hdl;
  }
}

/**
 * @brief 获取与条件关联的数据读取器
 *
 * @param condition 条件实体
 * @return 成功时返回数据读取器实体，失败时返回错误代码
 */
dds_entity_t dds_get_datareader(dds_entity_t condition)
{
  // 定义实体指针
  struct dds_entity *e;
  // 定义返回值变量
  dds_return_t rc;
  // 尝试锁定实体
  if ((rc = dds_entity_pin(condition, &e)) != DDS_RETCODE_OK)
    return rc;
  else
  {
    // 定义读取器句柄
    dds_entity_t rdh;
    // 根据实体类型进行处理
    switch (dds_entity_kind(e))
    {
    case DDS_KIND_COND_READ:
    case DDS_KIND_COND_QUERY:
      // 断言父实体类型为读取器
      assert(dds_entity_kind(e->m_parent) == DDS_KIND_READER);
      // 获取读取器句柄
      rdh = e->m_parent->m_hdllink.hdl;
      break;
    default:
      // 设置错误代码
      rdh = DDS_RETCODE_ILLEGAL_OPERATION;
      break;
    }
    // 解锁实体
    dds_entity_unpin(e);
    // 返回读取器句柄
    return rdh;
  }
}

/**
 * @brief 获取条件的掩码
 *
 * @param condition 条件实体
 * @param mask 指向存储掩码的指针
 * @return 成功时返回DDS_RETCODE_OK，失败时返回错误代码
 */
dds_return_t dds_get_mask(dds_entity_t condition, uint32_t *mask)
{
  // 定义实体指针
  dds_entity *entity;
  // 定义返回值变量
  dds_return_t rc;

  // 检查掩码指针是否为空
  if (mask == NULL)
    return DDS_RETCODE_BAD_PARAMETER;

  // 尝试锁定实体
  if ((rc = dds_entity_lock(condition, DDS_KIND_DONTCARE, &entity)) != DDS_RETCODE_OK)
    return rc;
  else if (dds_entity_kind(entity) != DDS_KIND_COND_READ && dds_entity_kind(entity) != DDS_KIND_COND_QUERY)
  {
    // 解锁实体
    dds_entity_unlock(entity);
    // 返回错误代码
    return DDS_RETCODE_ILLEGAL_OPERATION;
  }
  else
  {
    // 转换为读取条件指针
    dds_readcond *cond = (dds_readcond *)entity;
    // 获取掩码值
    *mask = (cond->m_sample_states | cond->m_view_states | cond->m_instance_states);
    // 解锁实体
    dds_entity_unlock(entity);
    // 返回成功代码
    return DDS_RETCODE_OK;
  }
}
