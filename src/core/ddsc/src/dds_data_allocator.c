/*
 * Copyright(c) 2021 ZettaScale Technology and others
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v. 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0, or the Eclipse Distribution License
 * v. 1.0 which is available at
 * http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * SPDX-License-Identifier: EPL-2.0 OR BSD-3-Clause
 */
#include "dds/dds.h"
#include "dds/ddsi/ddsi_sertype.h"
#include "dds/ddsrt/heap.h"
#include "dds__data_allocator.h"
#include "dds__entity.h"

#ifdef DDS_HAS_SHM
#include "dds/ddsi/ddsi_shm_transport.h"
#endif

#include "dds/ddsc/dds_loan_api.h"

/**
 * @brief 初始化堆上的数据分配器
 *
 * @param data_allocator 指向数据分配器的指针
 * @return dds_return_t 返回操作结果
 */
dds_return_t dds_data_allocator_init_heap(dds_data_allocator_t *data_allocator)
{
  // 使用特殊实体句柄在堆上分配
  return dds_data_allocator_init(DDS_DATA_ALLOCATOR_ALLOC_ON_HEAP, data_allocator);
}

/**
 * @brief 初始化数据分配器
 *
 * @param entity 实体对象
 * @param data_allocator 指向数据分配器的指针
 * @return dds_return_t 返回操作结果
 */
dds_return_t dds_data_allocator_init(dds_entity_t entity, dds_data_allocator_t *data_allocator)
{
  dds_entity *e;
  dds_return_t ret;

  // 如果数据分配器为空，返回错误参数
  if (data_allocator == NULL)
    return DDS_RETCODE_BAD_PARAMETER;

  // 特殊情况，分配器将此实体视为堆上的分配
  if (entity == DDS_DATA_ALLOCATOR_ALLOC_ON_HEAP)
  {
    ret = DDS_RETCODE_OK;
  }
  else
  {
    // 尝试获取实体对象，如果失败则返回错误码
    if ((ret = dds_entity_pin(entity, &e)) != DDS_RETCODE_OK)
      return ret;

    // 根据实体类型进行不同的初始化操作
    switch (dds_entity_kind(e))
    {
    case DDS_KIND_READER:
      ret = dds__reader_data_allocator_init((struct dds_reader *)e, data_allocator);
      break;
    case DDS_KIND_WRITER:
      ret = dds__writer_data_allocator_init((struct dds_writer *)e, data_allocator);
      break;
    default:
      ret = DDS_RETCODE_ILLEGAL_OPERATION;
      break;
    }
    // 释放实体对象
    dds_entity_unpin(e);
  }

  // 如果操作成功，将实体设置为数据分配器的实体
  if (ret == DDS_RETCODE_OK)
    data_allocator->entity = entity;

  return ret;
}
/**
 * @brief 结束数据分配器
 *
 * @param data_allocator 指向数据分配器的指针
 * @return dds_return_t 返回操作结果
 */
dds_return_t dds_data_allocator_fini(dds_data_allocator_t *data_allocator)
{
  dds_entity *e;
  dds_return_t ret;

  // 如果数据分配器为空，返回错误参数
  if (data_allocator == NULL)
    return DDS_RETCODE_BAD_PARAMETER;

  // 特殊情况，分配器将此实体视为堆上的分配
  if (data_allocator->entity == DDS_DATA_ALLOCATOR_ALLOC_ON_HEAP)
  {
    ret = DDS_RETCODE_OK;
  }
  else
  {
    // 尝试获取实体对象，如果失败则返回错误码
    if ((ret = dds_entity_pin(data_allocator->entity, &e)) != DDS_RETCODE_OK)
      return ret;

    // 根据实体类型进行不同的结束操作
    switch (dds_entity_kind(e))
    {
    case DDS_KIND_READER:
      ret = dds__reader_data_allocator_fini((struct dds_reader *)e, data_allocator);
      break;
    case DDS_KIND_WRITER:
      ret = dds__writer_data_allocator_fini((struct dds_writer *)e, data_allocator);
      break;
    default:
      ret = DDS_RETCODE_ILLEGAL_OPERATION;
      break;
    }
    // 释放实体对象
    dds_entity_unpin(e);
  }

  // 如果操作成功，将数据分配器的实体设置为0
  if (ret == DDS_RETCODE_OK)
    data_allocator->entity = 0;

  return ret;
}

/**
 * @brief 分配内存
 *
 * @param data_allocator 指向数据分配器的指针
 * @param size 要分配的内存大小
 * @return void* 返回分配的内存指针，如果失败则返回NULL
 */
void *dds_data_allocator_alloc(dds_data_allocator_t *data_allocator, size_t size)
{
#if DDS_HAS_SHM
  // 如果数据分配器为空，返回NULL
  if (data_allocator == NULL)
    return NULL;

  // 如果实体为堆上分配，则使用普通内存分配
  if (data_allocator->entity == DDS_DATA_ALLOCATOR_ALLOC_ON_HEAP)
    return ddsrt_malloc(size);

  dds_iox_allocator_t *d = (dds_iox_allocator_t *)data_allocator->opaque.bytes;
  switch (d->kind)
  {
  case DDS_IOX_ALLOCATOR_KIND_FINI:
    return NULL;
  case DDS_IOX_ALLOCATOR_KIND_NONE:
    return ddsrt_malloc(size);
  case DDS_IOX_ALLOCATOR_KIND_SUBSCRIBER:
    return NULL;
  case DDS_IOX_ALLOCATOR_KIND_PUBLISHER:
    // 如果请求的内存大小超过UINT32_MAX，返回NULL
    if (size > UINT32_MAX)
      return NULL;
    else
    {
      // 加锁以保护共享资源
      ddsrt_mutex_lock(&d->mutex);
      // 注意：这将在分配之外创建一个iceoryx头。
      // 对于小分配，这可能是不合适的...
      // 头部包含分配的大小和其他信息，例如内存是否未初始化或包含数据。
      void *chunk = shm_create_chunk(d->ref.pub, (uint32_t)size);
      // 解锁
      ddsrt_mutex_unlock(&d->mutex);
      return chunk;
    }
  default:
    return NULL;
  }
#else
  // 如果不支持共享内存，直接使用普通内存分配
  (void)data_allocator;
  return ddsrt_malloc(size);
#endif
}
/**
 * @brief 释放分配的数据内存
 *
 * @param[in] data_allocator 数据分配器指针
 * @param[in] ptr 需要释放的内存指针
 * @return dds_return_t 返回操作结果，成功返回DDS_RETCODE_OK
 */
dds_return_t dds_data_allocator_free(dds_data_allocator_t *data_allocator, void *ptr)
{
  // 定义返回值，默认为DDS_RETCODE_OK
  dds_return_t ret = DDS_RETCODE_OK;

#if DDS_HAS_SHM
  // 检查data_allocator是否为空，如果为空则返回错误参数
  if (data_allocator == NULL)
    return DDS_RETCODE_BAD_PARAMETER;

  // 判断实体类型是否为堆上分配
  if (data_allocator->entity == DDS_DATA_ALLOCATOR_ALLOC_ON_HEAP)
  {
    // 释放ptr指向的内存
    ddsrt_free(ptr);
  }
  else
  {
    // 将不透明结构转换为dds_iox_allocator_t类型
    dds_iox_allocator_t *d = (dds_iox_allocator_t *)data_allocator->opaque.bytes;

    // 根据分配器类型进行处理
    switch (d->kind)
    {
    case DDS_IOX_ALLOCATOR_KIND_FINI:
      // 如果分配器已经结束，则返回前提条件未满足错误
      ret = DDS_RETCODE_PRECONDITION_NOT_MET;
      break;
    case DDS_IOX_ALLOCATOR_KIND_NONE:
      // 如果没有分配器类型，则直接释放内存
      ddsrt_free(ptr);
      break;
    case DDS_IOX_ALLOCATOR_KIND_SUBSCRIBER:
      // 如果分配器类型为订阅者
      if (ptr != NULL)
      {
        // 加锁互斥量
        ddsrt_mutex_lock(&d->mutex);
        // 锁定共享内存订阅者
        shm_lock_iox_sub(d->ref.sub);
        // 释放共享内存块
        iox_sub_release_chunk(d->ref.sub, ptr);
        // 解锁共享内存订阅者
        shm_unlock_iox_sub(d->ref.sub);
        // 解锁互斥量
        ddsrt_mutex_unlock(&d->mutex);
      }
      break;
    case DDS_IOX_ALLOCATOR_KIND_PUBLISHER:
      // 如果分配器类型为发布者
      if (ptr != NULL)
      {
        // 加锁互斥量
        ddsrt_mutex_lock(&d->mutex);
        // 释放共享内存块
        iox_pub_release_chunk(d->ref.pub, ptr);
        // 解锁互斥量
        ddsrt_mutex_unlock(&d->mutex);
      }
      break;
    default:
      // 其他情况，返回错误参数
      ret = DDS_RETCODE_BAD_PARAMETER;
    }
  }
#else
  // 如果没有启用共享内存，则直接释放内存
  (void)data_allocator;
  ddsrt_free(ptr);
#endif

  // 返回操作结果
  return ret;
}
