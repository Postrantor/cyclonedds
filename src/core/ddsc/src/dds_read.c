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
#include "dds/ddsi/ddsi_domaingv.h"
#include "dds/ddsi/ddsi_entity.h"
#include "dds/ddsi/ddsi_entity_index.h"
#include "dds/ddsi/ddsi_sertype.h"
#include "dds/ddsi/ddsi_thread.h"
#include "dds/ddsi/ddsi_tkmap.h"
#include "dds__entity.h"
#include "dds__reader.h"
#include <assert.h>
#include <string.h>

#include "dds/ddsc/dds_loan_api.h"

/**
 * @brief dds_read_impl: 核心读取/拿取函数。通常情况下，maxs 是 buf 和 si 的大小，
 *        用于写入样本/状态，当设置为零时，表示特殊情况，即从缓存中的样本数量设置大小，
 *        并且缓存已被锁定。这用于支持 C++ API 读取长度无限制，解释为 "缓存中的所有相关样本"。
 *
 * @param take 是否拿取数据
 * @param reader_or_condition 读取器或条件实体
 * @param buf 缓冲区指针数组
 * @param bufsz 缓冲区大小
 * @param maxs 最大样本数
 * @param si 样本信息数组
 * @param mask 状态掩码
 * @param hand 实例句柄
 * @param lock 是否锁定
 * @param only_reader 是否仅为读取器
 * @return dds_return_t 返回操作结果
 */
static dds_return_t dds_read_impl(bool take, dds_entity_t reader_or_condition, void **buf, size_t bufsz, uint32_t maxs, dds_sample_info_t *si, uint32_t mask, dds_instance_handle_t hand, bool lock, bool only_reader)
{
  dds_return_t ret = DDS_RETCODE_OK;
  struct dds_entity *entity;
  struct dds_reader *rd;
  struct dds_readcond *cond;
  unsigned nodata_cleanups = 0;
#define NC_CLEAR_LOAN_OUT 1u
#define NC_FREE_BUF 2u
#define NC_RESET_BUF 4u

  // 检查参数是否有效
  if (buf == NULL || si == NULL || maxs == 0 || bufsz == 0 || bufsz < maxs || maxs > INT32_MAX)
    return DDS_RETCODE_BAD_PARAMETER;

  // 尝试获取实体
  if ((ret = dds_entity_pin(reader_or_condition, &entity)) < 0)
  {
    goto fail;
  }
  // 判断实体类型
  else if (dds_entity_kind(entity) == DDS_KIND_READER)
  {
    rd = (dds_reader *)entity;
    cond = NULL;
  }
  else if (only_reader)
  {
    ret = DDS_RETCODE_ILLEGAL_OPERATION;
    goto fail_pinned;
  }
  else if (dds_entity_kind(entity) != DDS_KIND_COND_READ && dds_entity_kind(entity) != DDS_KIND_COND_QUERY)
  {
    ret = DDS_RETCODE_ILLEGAL_OPERATION;
    goto fail_pinned;
  }
  else
  {
    rd = (dds_reader *)entity->m_parent;
    cond = (dds_readcond *)entity;
  }

  struct ddsi_thread_state *const thrst = ddsi_lookup_thread_state();
  ddsi_thread_state_awake(thrst, &entity->m_domain->gv);

  // 如果没有提供样本，则分配样本（假设全部或没有提供）
  if (buf[0] == NULL)
  {
    // 分配、使用或重新分配读取器上的缓存贷款
    ddsrt_mutex_lock(&rd->m_entity.m_mutex);
    if (rd->m_loan_out)
    {
      ddsi_sertype_realloc_samples(buf, rd->m_topic->m_stype, NULL, 0, maxs);
      nodata_cleanups = NC_FREE_BUF | NC_RESET_BUF;
    }
    else
    {
      if (rd->m_loan)
      {
        if (rd->m_loan_size >= maxs)
        {
          // 确保 buf 正确初始化
          ddsi_sertype_realloc_samples(buf, rd->m_topic->m_stype, rd->m_loan, rd->m_loan_size, rd->m_loan_size);
        }
        else
        {
          ddsi_sertype_realloc_samples(buf, rd->m_topic->m_stype, rd->m_loan, rd->m_loan_size, maxs);
          rd->m_loan_size = maxs;
        }
      }
      else
      {
        ddsi_sertype_realloc_samples(buf, rd->m_topic->m_stype, NULL, 0, maxs);
        rd->m_loan_size = maxs;
      }
      rd->m_loan = buf[0];
      rd->m_loan_out = true;
      nodata_cleanups = NC_RESET_BUF | NC_CLEAR_LOAN_OUT;
    }
    ddsrt_mutex_unlock(&rd->m_entity.m_mutex);
  }

  // 读取/拿取重置数据可用状态
  const uint32_t sm_old = dds_entity_status_reset_ov(&rd->m_entity, DDS_DATA_AVAILABLE_STATUS);
  // 在成功读取/拿取后重置订阅者的 DATA_ON_READERS 状态（如果已实现）
  if (sm_old & (DDS_DATA_ON_READERS_STATUS << SAM_ENABLED_SHIFT))
    dds_entity_status_reset(rd->m_entity.m_parent, DDS_DATA_ON_READERS_STATUS);

  // 根据 take 参数选择执行读取或拿取操作
  if (take)
    ret = dds_rhc_take(rd->m_rhc, lock, buf, si, maxs, mask, hand, cond);
  else
    ret = dds_rhc_read(rd->m_rhc, lock, buf, si, maxs, mask, hand, cond);

  // 如果没有读取到数据，将状态恢复为调用前的状态
  if (ret <= 0 && nodata_cleanups)
  {
    ddsrt_mutex_lock(&rd->m_entity.m_mutex);
    if (nodata_cleanups & NC_CLEAR_LOAN_OUT)
      rd->m_loan_out = false;
    if (nodata_cleanups & NC_FREE_BUF)
      ddsi_sertype_free_samples(rd->m_topic->m_stype, buf, maxs, DDS_FREE_ALL);
    if (nodata_cleanups & NC_RESET_BUF)
      buf[0] = NULL;
    ddsrt_mutex_unlock(&rd->m_entity.m_mutex);
  }
  dds_entity_unpin(entity);
  ddsi_thread_state_asleep(thrst);
  return ret;

#undef NC_CLEAR_LOAN_OUT
#undef NC_FREE_BUF
#undef NC_RESET_BUF

fail_pinned:
  dds_entity_unpin(entity);
fail:
  return ret;
}
/**
 * @brief 读取或获取CDR序列化数据的实现函数。
 *
 * @param[in] take 是否获取数据（true为获取，false为仅读取）
 * @param[in] reader_or_condition 读取器或条件实体句柄
 * @param[out] buf 存储CDR序列化数据的缓冲区指针数组
 * @param[in] maxs 缓冲区最大长度
 * @param[out] si 存储样本信息的数组
 * @param[in] mask 状态掩码
 * @param[in] hand 实例句柄
 * @param[in] lock 是否锁定
 * @return 返回操作结果，成功返回DDS_RETCODE_OK，否则返回相应错误代码
 */
static dds_return_t dds_readcdr_impl(bool take, dds_entity_t reader_or_condition, struct ddsi_serdata **buf, uint32_t maxs, dds_sample_info_t *si, uint32_t mask, dds_instance_handle_t hand, bool lock)
{
  // 初始化返回值为成功
  dds_return_t ret = DDS_RETCODE_OK;
  // 定义读取器和实体指针
  struct dds_reader *rd;
  struct dds_entity *entity;

  // 检查输入参数是否有效
  if (buf == NULL || si == NULL || maxs == 0 || maxs > INT32_MAX)
    return DDS_RETCODE_BAD_PARAMETER;

  // 尝试获取实体并检查结果
  if ((ret = dds_entity_pin(reader_or_condition, &entity)) < 0)
  {
    return ret;
  }
  // 判断实体类型是否为读取器
  else if (dds_entity_kind(entity) == DDS_KIND_READER)
  {
    rd = (dds_reader *)entity;
  }
  // 判断实体类型是否为非法操作
  else if (dds_entity_kind(entity) != DDS_KIND_COND_READ && dds_entity_kind(entity) != DDS_KIND_COND_QUERY)
  {
    dds_entity_unpin(entity);
    return DDS_RETCODE_ILLEGAL_OPERATION;
  }
  // 其他情况，获取父实体作为读取器
  else
  {
    rd = (dds_reader *)entity->m_parent;
  }

  // 查找线程状态并唤醒
  struct ddsi_thread_state *const thrst = ddsi_lookup_thread_state();
  ddsi_thread_state_awake(thrst, &entity->m_domain->gv);

  // 重置数据可用状态
  const uint32_t sm_old = dds_entity_status_reset_ov(&rd->m_entity, DDS_DATA_AVAILABLE_STATUS);
  // 如果成功读取/获取，则重置数据在读取器上的状态
  if (sm_old & (DDS_DATA_ON_READERS_STATUS << SAM_ENABLED_SHIFT))
    dds_entity_status_reset(rd->m_entity.m_parent, DDS_DATA_ON_READERS_STATUS);

  // 根据take参数选择执行读取或获取操作
  if (take)
    ret = dds_rhc_takecdr(rd->m_rhc, lock, buf, si, maxs, mask & DDS_ANY_SAMPLE_STATE, mask & DDS_ANY_VIEW_STATE, mask & DDS_ANY_INSTANCE_STATE, hand);
  else
    ret = dds_rhc_readcdr(rd->m_rhc, lock, buf, si, maxs, mask & DDS_ANY_SAMPLE_STATE, mask & DDS_ANY_VIEW_STATE, mask & DDS_ANY_INSTANCE_STATE, hand);

  // 解除实体锁定并使线程进入休眠状态
  dds_entity_unpin(entity);
  ddsi_thread_state_asleep(thrst);
  // 返回操作结果
  return ret;
}
/**
 * @brief 读取数据
 * @param rd_or_cnd 读取实体或条件实体
 * @param buf 存储读取数据的缓冲区指针
 * @param si 存储样本信息的结构体指针
 * @param bufsz 缓冲区大小
 * @param maxs 最大读取数量
 * @return dds_return_t 返回操作结果
 */
dds_return_t dds_read(dds_entity_t rd_or_cnd, void **buf, dds_sample_info_t *si, size_t bufsz, uint32_t maxs)
{
  bool lock = true;                  // 默认使用锁
  if (maxs == DDS_READ_WITHOUT_LOCK) // 如果最大读取数量为无锁模式
  {
    lock = false; // 不使用锁
    /* FIXME: Fix the interface. */
    maxs = (uint32_t)bufsz; // 设置最大读取数量为缓冲区大小
  }
  return dds_read_impl(false, rd_or_cnd, buf, bufsz, maxs, si, NO_STATE_MASK_SET, DDS_HANDLE_NIL, lock, false);
}

/**
 * @brief 读取数据（带锁）
 * @param rd_or_cnd 读取实体或条件实体
 * @param buf 存储读取数据的缓冲区指针
 * @param si 存储样本信息的结构体指针
 * @param maxs 最大读取数量
 * @return dds_return_t 返回操作结果
 */
dds_return_t dds_read_wl(dds_entity_t rd_or_cnd, void **buf, dds_sample_info_t *si, uint32_t maxs)
{
  bool lock = true;                  // 默认使用锁
  if (maxs == DDS_READ_WITHOUT_LOCK) // 如果最大读取数量为无锁模式
  {
    lock = false; // 不使用锁
    /* FIXME: Fix the interface. */
    maxs = 100; // 设置最大读取数量为100
  }
  return dds_read_impl(false, rd_or_cnd, buf, maxs, maxs, si, NO_STATE_MASK_SET, DDS_HANDLE_NIL, lock, false);
}

/**
 * @brief 读取数据（带掩码）
 * @param rd_or_cnd 读取实体或条件实体
 * @param buf 存储读取数据的缓冲区指针
 * @param si 存储样本信息的结构体指针
 * @param bufsz 缓冲区大小
 * @param maxs 最大读取数量
 * @param mask 状态掩码
 * @return dds_return_t 返回操作结果
 */
dds_return_t dds_read_mask(dds_entity_t rd_or_cnd, void **buf, dds_sample_info_t *si, size_t bufsz, uint32_t maxs, uint32_t mask)
{
  bool lock = true;                  // 默认使用锁
  if (maxs == DDS_READ_WITHOUT_LOCK) // 如果最大读取数量为无锁模式
  {
    lock = false; // 不使用锁
    /* FIXME: Fix the interface. */
    maxs = (uint32_t)bufsz; // 设置最大读取数量为缓冲区大小
  }
  return dds_read_impl(false, rd_or_cnd, buf, bufsz, maxs, si, mask, DDS_HANDLE_NIL, lock, false);
}

/**
 * @brief 读取数据（带掩码和锁）
 * @param rd_or_cnd 读取实体或条件实体
 * @param buf 存储读取数据的缓冲区指针
 * @param si 存储样本信息的结构体指针
 * @param maxs 最大读取数量
 * @param mask 状态掩码
 * @return dds_return_t 返回操作结果
 */
dds_return_t dds_read_mask_wl(dds_entity_t rd_or_cnd, void **buf, dds_sample_info_t *si, uint32_t maxs, uint32_t mask)
{
  bool lock = true;                  // 默认使用锁
  if (maxs == DDS_READ_WITHOUT_LOCK) // 如果最大读取数量为无锁模式
  {
    lock = false; // 不使用锁
    /* FIXME: Fix the interface. */
    maxs = 100; // 设置最大读取数量为100
  }
  return dds_read_impl(false, rd_or_cnd, buf, maxs, maxs, si, mask, DDS_HANDLE_NIL, lock, false);
}

/**
 * @brief 读取CDR数据
 * @param rd_or_cnd 读取实体或条件实体
 * @param buf 存储读取数据的序列化数据指针
 * @param maxs 最大读取数量
 * @param si 存储样本信息的结构体指针
 * @param mask 状态掩码
 * @return dds_return_t 返回操作结果
 */
dds_return_t dds_readcdr(dds_entity_t rd_or_cnd, struct ddsi_serdata **buf, uint32_t maxs, dds_sample_info_t *si, uint32_t mask)
{
  bool lock = true;                  // 默认使用锁
  if (maxs == DDS_READ_WITHOUT_LOCK) // 如果最大读取数量为无锁模式
  {
    lock = false; // 不使用锁
    /* FIXME: Fix the interface. */
    maxs = 100; // 设置最大读取数量为100
  }
  return dds_readcdr_impl(false, rd_or_cnd, buf, maxs, si, mask, DDS_HANDLE_NIL, lock);
}

/**
 * @brief 读取指定实例的数据
 * @param rd_or_cnd 读取实体或条件实体
 * @param buf 存储读取数据的缓冲区指针
 * @param si 存储样本信息的结构体指针
 * @param bufsz 缓冲区大小
 * @param maxs 最大读取数量
 * @param handle 实例句柄
 * @return dds_return_t 返回操作结果
 */
dds_return_t dds_read_instance(dds_entity_t rd_or_cnd, void **buf, dds_sample_info_t *si, size_t bufsz, uint32_t maxs, dds_instance_handle_t handle)
{
  bool lock = true; // 默认使用锁

  if (handle == DDS_HANDLE_NIL)              // 如果实例句柄为空
    return DDS_RETCODE_PRECONDITION_NOT_MET; // 返回前提条件不满足的错误码

  if (maxs == DDS_READ_WITHOUT_LOCK) // 如果最大读取数量为无锁模式
  {
    lock = false; // 不使用锁
    /* FIXME: Fix the interface. */
    maxs = 100; // 设置最大读取数量为100
  }
  return dds_read_impl(false, rd_or_cnd, buf, bufsz, maxs, si, NO_STATE_MASK_SET, handle, lock, false);
}
/**
 * @brief 读取指定实例的数据（带锁）
 *
 * @param rd_or_cnd 读取器或条件实体
 * @param buf 存储读取到的数据的缓冲区
 * @param si 存储样本信息的结构体指针
 * @param maxs 最大读取的样本数
 * @param handle 实例句柄
 * @return dds_return_t 返回操作结果
 */
dds_return_t dds_read_instance_wl(dds_entity_t rd_or_cnd, void **buf, dds_sample_info_t *si, uint32_t maxs, dds_instance_handle_t handle)
{
  bool lock = true; // 默认使用锁

  if (handle == DDS_HANDLE_NIL) // 如果实例句柄为空
    return DDS_RETCODE_PRECONDITION_NOT_MET;

  if (maxs == DDS_READ_WITHOUT_LOCK) // 如果不需要锁
  {
    lock = false; // 不使用锁
    /* FIXME: Fix the interface. */
    maxs = 100; // 设置最大读取样本数为100
  }
  // 调用实现函数进行读取操作
  return dds_read_impl(false, rd_or_cnd, buf, maxs, maxs, si, NO_STATE_MASK_SET, handle, lock, false);
}

/**
 * @brief 读取指定实例的数据（带掩码）
 *
 * @param rd_or_cnd 读取器或条件实体
 * @param buf 存储读取到的数据的缓冲区
 * @param si 存储样本信息的结构体指针
 * @param bufsz 缓冲区大小
 * @param maxs 最大读取的样本数
 * @param handle 实例句柄
 * @param mask 状态掩码
 * @return dds_return_t 返回操作结果
 */
dds_return_t dds_read_instance_mask(dds_entity_t rd_or_cnd, void **buf, dds_sample_info_t *si, size_t bufsz, uint32_t maxs, dds_instance_handle_t handle, uint32_t mask)
{
  bool lock = true; // 默认使用锁

  if (handle == DDS_HANDLE_NIL) // 如果实例句柄为空
    return DDS_RETCODE_PRECONDITION_NOT_MET;

  if (maxs == DDS_READ_WITHOUT_LOCK) // 如果不需要锁
  {
    lock = false; // 不使用锁
    /* FIXME: Fix the interface. */
    maxs = (uint32_t)bufsz; // 设置最大读取样本数为缓冲区大小
  }
  // 调用实现函数进行读取操作
  return dds_read_impl(false, rd_or_cnd, buf, bufsz, maxs, si, mask, handle, lock, false);
}

/**
 * @brief 读取指定实例的数据（带掩码，带锁）
 *
 * @param rd_or_cnd 读取器或条件实体
 * @param buf 存储读取到的数据的缓冲区
 * @param si 存储样本信息的结构体指针
 * @param maxs 最大读取的样本数
 * @param handle 实例句柄
 * @param mask 状态掩码
 * @return dds_return_t 返回操作结果
 */
dds_return_t dds_read_instance_mask_wl(dds_entity_t rd_or_cnd, void **buf, dds_sample_info_t *si, uint32_t maxs, dds_instance_handle_t handle, uint32_t mask)
{
  bool lock = true; // 默认使用锁

  if (handle == DDS_HANDLE_NIL) // 如果实例句柄为空
    return DDS_RETCODE_PRECONDITION_NOT_MET;

  if (maxs == DDS_READ_WITHOUT_LOCK) // 如果不需要锁
  {
    lock = false; // 不使用锁
    /* FIXME: Fix the interface. */
    maxs = 100; // 设置最大读取样本数为100
  }
  // 调用实现函数进行读取操作
  return dds_read_impl(false, rd_or_cnd, buf, maxs, maxs, si, mask, handle, lock, false);
}

/**
 * @brief 读取指定实例的CDR数据
 *
 * @param rd_or_cnd 读取器或条件实体
 * @param buf 存储读取到的CDR数据的缓冲区
 * @param maxs 最大读取的样本数
 * @param si 存储样本信息的结构体指针
 * @param handle 实例句柄
 * @param mask 状态掩码
 * @return dds_return_t 返回操作结果
 */
dds_return_t dds_readcdr_instance(dds_entity_t rd_or_cnd, struct ddsi_serdata **buf, uint32_t maxs, dds_sample_info_t *si, dds_instance_handle_t handle, uint32_t mask)
{
  bool lock = true; // 默认使用锁

  if (handle == DDS_HANDLE_NIL) // 如果实例句柄为空
    return DDS_RETCODE_PRECONDITION_NOT_MET;

  if (maxs == DDS_READ_WITHOUT_LOCK) // 如果不需要锁
  {
    lock = false; // 不使用锁
    /* FIXME: Fix the interface. */
    maxs = 100; // 设置最大读取样本数为100
  }
  // 调用实现函数进行读取操作
  return dds_readcdr_impl(false, rd_or_cnd, buf, maxs, si, mask, handle, lock);
}

/**
 * @brief 读取下一个未读样本
 *
 * @param reader 读取器实体
 * @param buf 存储读取到的数据的缓冲区
 * @param si 存储样本信息的结构体指针
 * @return dds_return_t 返回操作结果
 */
dds_return_t dds_read_next(dds_entity_t reader, void **buf, dds_sample_info_t *si)
{
  uint32_t mask = DDS_NOT_READ_SAMPLE_STATE | DDS_ANY_VIEW_STATE | DDS_ANY_INSTANCE_STATE; // 设置状态掩码
  // 调用实现函数进行读取操作
  return dds_read_impl(false, reader, buf, 1u, 1u, si, mask, DDS_HANDLE_NIL, true, true);
}

/**
 * @brief 读取下一个未读样本（带锁）
 *
 * @param reader 读取器实体
 * @param buf 存储读取到的数据的缓冲区
 * @param si 存储样本信息的结构体指针
 * @return dds_return_t 返回操作结果
 */
dds_return_t dds_read_next_wl(
    dds_entity_t reader,
    void **buf,
    dds_sample_info_t *si)
{
  uint32_t mask = DDS_NOT_READ_SAMPLE_STATE | DDS_ANY_VIEW_STATE | DDS_ANY_INSTANCE_STATE; // 设置状态掩码
  // 调用实现函数进行读取操作
  return dds_read_impl(false, reader, buf, 1u, 1u, si, mask, DDS_HANDLE_NIL, true, true);
}
/**
 * @brief 从实体中获取数据
 *
 * @param rd_or_cnd 读取或条件实体
 * @param buf 存储读取到的数据的缓冲区
 * @param si 存储样本信息的数组
 * @param bufsz 缓冲区大小
 * @param maxs 最大读取样本数
 * @return dds_return_t 操作结果
 */
dds_return_t dds_take(dds_entity_t rd_or_cnd, void **buf, dds_sample_info_t *si, size_t bufsz, uint32_t maxs)
{
  bool lock = true;                  // 是否需要锁定
  if (maxs == DDS_READ_WITHOUT_LOCK) // 如果不需要锁定
  {
    lock = false;
    /* FIXME: Fix the interface. */
    maxs = (uint32_t)bufsz; // 设置最大读取样本数为缓冲区大小
  }
  // 调用实现函数
  return dds_read_impl(true, rd_or_cnd, buf, bufsz, maxs, si, NO_STATE_MASK_SET, DDS_HANDLE_NIL, lock, false);
}

/**
 * @brief 从实体中获取数据（无锁版本）
 *
 * @param rd_or_cnd 读取或条件实体
 * @param buf 存储读取到的数据的缓冲区
 * @param si 存储样本信息的数组
 * @param maxs 最大读取样本数
 * @return dds_return_t 操作结果
 */
dds_return_t dds_take_wl(dds_entity_t rd_or_cnd, void **buf, dds_sample_info_t *si, uint32_t maxs)
{
  bool lock = true;                  // 是否需要锁定
  if (maxs == DDS_READ_WITHOUT_LOCK) // 如果不需要锁定
  {
    lock = false;
    /* FIXME: Fix the interface. */
    maxs = 100; // 设置最大读取样本数为100
  }
  // 调用实现函数
  return dds_read_impl(true, rd_or_cnd, buf, maxs, maxs, si, NO_STATE_MASK_SET, DDS_HANDLE_NIL, lock, false);
}

/**
 * @brief 从实体中获取数据（带掩码）
 *
 * @param rd_or_cnd 读取或条件实体
 * @param buf 存储读取到的数据的缓冲区
 * @param si 存储样本信息的数组
 * @param bufsz 缓冲区大小
 * @param maxs 最大读取样本数
 * @param mask 状态掩码
 * @return dds_return_t 操作结果
 */
dds_return_t dds_take_mask(dds_entity_t rd_or_cnd, void **buf, dds_sample_info_t *si, size_t bufsz, uint32_t maxs, uint32_t mask)
{
  bool lock = true;                  // 是否需要锁定
  if (maxs == DDS_READ_WITHOUT_LOCK) // 如果不需要锁定
  {
    lock = false;
    /* FIXME: Fix the interface. */
    maxs = (uint32_t)bufsz; // 设置最大读取样本数为缓冲区大小
  }
  // 调用实现函数
  return dds_read_impl(true, rd_or_cnd, buf, bufsz, maxs, si, mask, DDS_HANDLE_NIL, lock, false);
}

/**
 * @brief 从实体中获取数据（带掩码，无锁版本）
 *
 * @param rd_or_cnd 读取或条件实体
 * @param buf 存储读取到的数据的缓冲区
 * @param si 存储样本信息的数组
 * @param maxs 最大读取样本数
 * @param mask 状态掩码
 * @return dds_return_t 操作结果
 */
dds_return_t dds_take_mask_wl(dds_entity_t rd_or_cnd, void **buf, dds_sample_info_t *si, uint32_t maxs, uint32_t mask)
{
  bool lock = true;                  // 是否需要锁定
  if (maxs == DDS_READ_WITHOUT_LOCK) // 如果不需要锁定
  {
    lock = false;
    /* FIXME: Fix the interface. */
    maxs = 100; // 设置最大读取样本数为100
  }
  // 调用实现函数
  return dds_read_impl(true, rd_or_cnd, buf, maxs, maxs, si, mask, DDS_HANDLE_NIL, lock, false);
}

/**
 * @brief 从实体中获取CDR数据
 *
 * @param rd_or_cnd 读取或条件实体
 * @param buf 存储读取到的CDR数据的缓冲区
 * @param maxs 最大读取样本数
 * @param si 存储样本信息的数组
 * @param mask 状态掩码
 * @return dds_return_t 操作结果
 */
dds_return_t dds_takecdr(dds_entity_t rd_or_cnd, struct ddsi_serdata **buf, uint32_t maxs, dds_sample_info_t *si, uint32_t mask)
{
  bool lock = true;                  // 是否需要锁定
  if (maxs == DDS_READ_WITHOUT_LOCK) // 如果不需要锁定
  {
    lock = false;
    /* FIXME: Fix the interface. */
    maxs = 100; // 设置最大读取样本数为100
  }
  // 调用实现函数
  return dds_readcdr_impl(true, rd_or_cnd, buf, maxs, si, mask, DDS_HANDLE_NIL, lock);
}

/**
 * @brief 从实体中获取特定实例的数据
 *
 * @param rd_or_cnd 读取或条件实体
 * @param buf 存储读取到的数据的缓冲区
 * @param si 存储样本信息的数组
 * @param bufsz 缓冲区大小
 * @param maxs 最大读取样本数
 * @param handle 实例句柄
 * @return dds_return_t 操作结果
 */
dds_return_t dds_take_instance(dds_entity_t rd_or_cnd, void **buf, dds_sample_info_t *si, size_t bufsz, uint32_t maxs, dds_instance_handle_t handle)
{
  bool lock = true; // 是否需要锁定

  if (handle == DDS_HANDLE_NIL) // 如果实例句柄无效
    return DDS_RETCODE_PRECONDITION_NOT_MET;

  if (maxs == DDS_READ_WITHOUT_LOCK) // 如果不需要锁定
  {
    lock = false;
    /* FIXME: Fix the interface. */
    maxs = 100; // 设置最大读取样本数为100
  }
  // 调用实现函数
  return dds_read_impl(true, rd_or_cnd, buf, bufsz, maxs, si, NO_STATE_MASK_SET, handle, lock, false);
}
/**
 * @brief 从实例中获取数据（带锁）
 *
 * @param rd_or_cnd 读取或条件实体
 * @param buf 存储数据的缓冲区指针
 * @param si 存储样本信息的结构体指针
 * @param maxs 最大样本数
 * @param handle 实例句柄
 * @return dds_return_t 返回操作结果
 */
dds_return_t dds_take_instance_wl(dds_entity_t rd_or_cnd, void **buf, dds_sample_info_t *si, uint32_t maxs, dds_instance_handle_t handle)
{
  bool lock = true;

  // 如果实例句柄为空，返回前提条件未满足错误
  if (handle == DDS_HANDLE_NIL)
    return DDS_RETCODE_PRECONDITION_NOT_MET;

  // 如果最大样本数为无锁读取，设置锁为false并修复接口
  if (maxs == DDS_READ_WITHOUT_LOCK)
  {
    lock = false;
    /* FIXME: Fix the interface. */
    maxs = 100;
  }
  // 调用dds_read_impl函数进行读取操作
  return dds_read_impl(true, rd_or_cnd, buf, maxs, maxs, si, NO_STATE_MASK_SET, handle, lock, false);
}

/**
 * @brief 从实例中获取数据（带掩码）
 *
 * @param rd_or_cnd 读取或条件实体
 * @param buf 存储数据的缓冲区指针
 * @param si 存储样本信息的结构体指针
 * @param bufsz 缓冲区大小
 * @param maxs 最大样本数
 * @param handle 实例句柄
 * @param mask 状态掩码
 * @return dds_return_t 返回操作结果
 */
dds_return_t dds_take_instance_mask(dds_entity_t rd_or_cnd, void **buf, dds_sample_info_t *si, size_t bufsz, uint32_t maxs, dds_instance_handle_t handle, uint32_t mask)
{
  bool lock = true;

  // 如果实例句柄为空，返回前提条件未满足错误
  if (handle == DDS_HANDLE_NIL)
    return DDS_RETCODE_PRECONDITION_NOT_MET;

  // 如果最大样本数为无锁读取，设置锁为false并修复接口
  if (maxs == DDS_READ_WITHOUT_LOCK)
  {
    lock = false;
    /* FIXME: Fix the interface. */
    maxs = (uint32_t)bufsz;
  }
  // 调用dds_read_impl函数进行读取操作
  return dds_read_impl(true, rd_or_cnd, buf, bufsz, maxs, si, mask, handle, lock, false);
}

/**
 * @brief 从实例中获取数据（带掩码和锁）
 *
 * @param rd_or_cnd 读取或条件实体
 * @param buf 存储数据的缓冲区指针
 * @param si 存储样本信息的结构体指针
 * @param maxs 最大样本数
 * @param handle 实例句柄
 * @param mask 状态掩码
 * @return dds_return_t 返回操作结果
 */
dds_return_t dds_take_instance_mask_wl(dds_entity_t rd_or_cnd, void **buf, dds_sample_info_t *si, uint32_t maxs, dds_instance_handle_t handle, uint32_t mask)
{
  bool lock = true;

  // 如果实例句柄为空，返回前提条件未满足错误
  if (handle == DDS_HANDLE_NIL)
    return DDS_RETCODE_PRECONDITION_NOT_MET;

  // 如果最大样本数为无锁读取，设置锁为false并修复接口
  if (maxs == DDS_READ_WITHOUT_LOCK)
  {
    lock = false;
    /* FIXME: Fix the interface. */
    maxs = 100;
  }
  // 调用dds_read_impl函数进行读取操作
  return dds_read_impl(true, rd_or_cnd, buf, maxs, maxs, si, mask, handle, lock, false);
}

/**
 * @brief 从CDR实例中获取数据
 *
 * @param rd_or_cnd 读取或条件实体
 * @param buf 存储序列化数据的缓冲区指针
 * @param maxs 最大样本数
 * @param si 存储样本信息的结构体指针
 * @param handle 实例句柄
 * @param mask 状态掩码
 * @return dds_return_t 返回操作结果
 */
dds_return_t dds_takecdr_instance(dds_entity_t rd_or_cnd, struct ddsi_serdata **buf, uint32_t maxs, dds_sample_info_t *si, dds_instance_handle_t handle, uint32_t mask)
{
  bool lock = true;

  // 如果实例句柄为空，返回前提条件未满足错误
  if (handle == DDS_HANDLE_NIL)
    return DDS_RETCODE_PRECONDITION_NOT_MET;

  // 如果最大样本数为无锁读取，设置锁为false并修复接口
  if (maxs == DDS_READ_WITHOUT_LOCK)
  {
    lock = false;
    /* FIXME: Fix the interface. */
    maxs = 100;
  }
  // 调用dds_readcdr_impl函数进行读取操作
  return dds_readcdr_impl(true, rd_or_cnd, buf, maxs, si, mask, handle, lock);
}

/**
 * @brief 获取下一个数据
 *
 * @param reader 读取实体
 * @param buf 存储数据的缓冲区指针
 * @param si 存储样本信息的结构体指针
 * @return dds_return_t 返回操作结果
 */
dds_return_t dds_take_next(dds_entity_t reader, void **buf, dds_sample_info_t *si)
{
  uint32_t mask = DDS_NOT_READ_SAMPLE_STATE | DDS_ANY_VIEW_STATE | DDS_ANY_INSTANCE_STATE;
  // 调用dds_read_impl函数进行读取操作
  return dds_read_impl(true, reader, buf, 1u, 1u, si, mask, DDS_HANDLE_NIL, true, true);
}

/**
 * @brief 获取下一个数据（带锁）
 *
 * @param reader 读取实体
 * @param buf 存储数据的缓冲区指针
 * @param si 存储样本信息的结构体指针
 * @return dds_return_t 返回操作结果
 */
dds_return_t dds_take_next_wl(dds_entity_t reader, void **buf, dds_sample_info_t *si)
{
  uint32_t mask = DDS_NOT_READ_SAMPLE_STATE | DDS_ANY_VIEW_STATE | DDS_ANY_INSTANCE_STATE;
  // 调用dds_read_impl函数进行读取操作
  return dds_read_impl(true, reader, buf, 1u, 1u, si, mask, DDS_HANDLE_NIL, true, true);
}

/**
 * @brief 归还读取器贷款
 *
 * @param rd 读取器指针
 * @param buf 存储数据的缓冲区指针
 * @param bufsz 缓冲区大小
 * @return dds_return_t 返回操作结果
 */
dds_return_t dds_return_reader_loan(dds_reader *rd, void **buf, int32_t bufsz)
{
  // 如果缓冲区大小小于等于0，返回成功
  if (bufsz <= 0)
  {
    /* No data whatsoever, or an invocation following a failed read/take call.  Read/take
       already take care of restoring the state prior to their invocation if they return
       no data.  Return late so invalid handles can be detected. */
    return DDS_RETCODE_OK;
  }
  assert(buf[0] != NULL);

  const struct ddsi_sertype *st = rd->m_topic->m_stype;

  // 加锁
  /* 在这里可能会耗费时间的部分（释放样本）可以在不持有读取器锁的情况下安全地完成，
   但是在插入数据和触发waitsets期间并未使用该特定锁（那是observer_lock），
   因此在简化代码的同时保持更长时间是一个公平的权衡。 */
  ddsrt_mutex_lock(&rd->m_entity.m_mutex);
  if (buf[0] != rd->m_loan)
  {
    /* Not so much a loan as a buffer allocated by the middleware on behalf of the
       application.  So it really is no more than a sophisticated variant of "free". */
    ddsi_sertype_free_samples(st, buf, (size_t)bufsz, DDS_FREE_ALL);
    buf[0] = NULL;
  }
  else if (!rd->m_loan_out)
  {
    /* Trying to return a loan that has been returned already */
    ddsrt_mutex_unlock(&rd->m_entity.m_mutex);
    return DDS_RETCODE_PRECONDITION_NOT_MET;
  }
  else
  {
    /* 仅释放样本中引用的内存，而不是样本本身。
       将它们清零以确保后续操作中不存在可能导致问题的悬空指针。FIXME: 应该有更好的方法 */
    ddsi_sertype_free_samples(st, buf, (size_t)bufsz, DDS_FREE_CONTENTS);
    ddsi_sertype_zero_samples(st, rd->m_loan, rd->m_loan_size);
    rd->m_loan_out = false;
    buf[0] = NULL;
  }
  // 解锁
  ddsrt_mutex_unlock(&rd->m_entity.m_mutex);
  return DDS_RETCODE_OK;
}
