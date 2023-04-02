#include "dds/ddsc/dds_loan_api.h"

#include "dds__entity.h"
#include "dds__loan.h"
#include "dds__reader.h"
#include "dds__types.h"
#include "dds__writer.h"

#include "dds/ddsi/ddsi_sertype.h"

#ifdef DDS_HAS_SHM
#include "dds/ddsi/ddsi_shm_transport.h"
#include "iceoryx_binding_c/chunk.h"
#endif
/**
 * @brief 检查共享内存是否可用于给定的实体
 *
 * 此函数根据实体类型（读取器或写入器）检查共享内存是否已正确启用。
 * 如果共享内存可用，则返回 true，否则返回 false。
 *
 * @param[in] entity 要检查的 dds_entity_t 实体
 * @return 如果共享内存可用，则返回 true，否则返回 false
 */
bool dds_is_shared_memory_available(const dds_entity_t entity)
{
  bool ret = false; // 初始化返回值为 false

#ifdef DDS_HAS_SHM
  dds_entity *e;

  // 尝试获取实体指针，如果失败则返回 false
  if (DDS_RETCODE_OK != dds_entity_pin(entity, &e))
  {
    return ret;
  }

  // 根据实体类型检查共享内存是否可用
  switch (dds_entity_kind(e))
  {
  case DDS_KIND_READER: // 读取器类型
  {
    struct dds_reader const *const rd = (struct dds_reader *)e;
    // 只有在共享内存正确启用（即 iox subscriber 已初始化）时才返回 true
    ret = (rd->m_iox_sub != NULL);
    break;
  }
  case DDS_KIND_WRITER: // 写入器类型
  {
    struct dds_writer const *const wr = (struct dds_writer *)e;
    // 只有在共享内存正确启用（即 iox publisher 已初始化）时才返回 true
    ret = (wr->m_iox_pub != NULL);
    break;
  }
  default:
    break;
  }

  // 解除实体指针的锁定
  dds_entity_unpin(e);
#endif

  (void)entity; // 防止未使用变量警告
  return ret;   // 返回共享内存是否可用的结果
}
/**
 * @brief 检查给定实体是否支持贷款模式
 *
 * 此函数根据实体类型（读取器或写入器）检查贷款模式是否可用。
 * 如果贷款模式可用，则返回 true，否则返回 false。
 *
 * @param[in] entity 要检查的 dds_entity_t 实体
 * @return 如果贷款模式可用，则返回 true，否则返回 false
 */
bool dds_is_loan_available(const dds_entity_t entity)
{
  bool ret = false; // 初始化返回值为 false

#ifdef DDS_HAS_SHM
  dds_entity *e;

  // 尝试获取实体指针，如果失败则返回 false
  if (DDS_RETCODE_OK != dds_entity_pin(entity, &e))
  {
    return ret;
  }

  // 根据实体类型检查贷款模式是否可用
  switch (dds_entity_kind(e))
  {
  case DDS_KIND_READER: // 读取器类型
  {
    struct dds_reader const *const rd = (struct dds_reader *)e;
    // 只有在共享内存正确启用（即 iox subscriber 已初始化）且类型固定时才返回 true
    ret = (rd->m_iox_sub != NULL) && (rd->m_topic->m_stype->fixed_size);
    break;
  }
  case DDS_KIND_WRITER: // 写入器类型
  {
    struct dds_writer const *const wr = (struct dds_writer *)e;
    // 只有在共享内存正确启用（即 iox publisher 已初始化）且类型固定时才返回 true
    ret = (wr->m_iox_pub != NULL) && (wr->m_topic->m_stype->fixed_size);
    break;
  }
  default:
    break;
  }

  // 解除实体指针的锁定
  dds_entity_unpin(e);
#endif

  (void)entity; // 防止未使用变量警告
  return ret;   // 返回贷款模式是否可用的结果
}
/** @file
 *  此文件包含与共享内存相关的DDS Writer功能。
 */

#ifdef DDS_HAS_SHM

/**
 * 释放 IceOryx chunk。
 *
 * @param[in] wr       指向dds_writer结构的指针。
 * @param[in] sample   要释放的IceOryx chunk。
 */
static void release_iox_chunk(dds_writer *wr, void *sample)
{
  iox_pub_release_chunk(wr->m_iox_pub, sample); // 释放IceOryx chunk。
}

/**
 * 注册一个已借出的IceOryx chunk。
 *
 * @param[in] wr        指向dds_writer结构的指针。
 * @param[in] pub_loan  已借出的IceOryx chunk。
 */
void dds_register_pub_loan(dds_writer *wr, void *pub_loan)
{
  for (uint32_t i = 0; i < MAX_PUB_LOANS; ++i) // 遍历所有可用的借款槽位。
  {
    if (!wr->m_iox_pub_loans[i]) // 如果找到空槽位。
    {
      wr->m_iox_pub_loans[i] = pub_loan; // 将借出的chunk注册到该槽位。
      return;
    }
  }

  /* 借款池应足够大，以存储最大数量的打开
   * IceOryx loans。因此，如果IceOryx授予了贷款，我们应该能够存储
   * 它。
   */
  assert(false);
}

/**
 * 取消注册一个已借出的IceOryx chunk。
 *
 * @param[in]  wr        指向dds_writer结构的指针。
 * @param[in]  pub_loan  已借出的IceOryx chunk。
 * @return     如果成功取消注册，则返回true，否则返回false。
 */
bool dds_deregister_pub_loan(dds_writer *wr, const void *pub_loan)
{
  for (uint32_t i = 0; i < MAX_PUB_LOANS; ++i) // 遍历所有可用的借款槽位。
  {
    if (wr->m_iox_pub_loans[i] == pub_loan) // 如果找到匹配的chunk。
    {
      wr->m_iox_pub_loans[i] = NULL; // 将该槽位设置为空。
      return true;
    }
  }
  return false;
}

/**
 * 从IceOryx publisher借用一个chunk。
 *
 * @param[in]  wr    指向dds_writer结构的指针。
 * @param[in]  size  请求的chunk大小。
 * @return     如果成功借用chunk，则返回指向chunk的指针，否则返回NULL。
 */
static void *dds_writer_loan_chunk(dds_writer *wr, size_t size)
{
  void *chunk = shm_create_chunk(wr->m_iox_pub, size); // 从IceOryx publisher创建一个chunk。

  if (chunk) // 如果成功创建了chunk。
  {
    dds_register_pub_loan(wr, chunk); // 注册已借出的chunk。

    // NB: 我们设置这个，因为用户可以使用这个chunk不仅仅是用write
    // 在那里我们检查它是否在之前被借出。
    // 目前只有固定大小类型才能借用。

    // 不幸的是，因为这个chunk在这个时候实际上还没有填充。
    // 我们应该确保我们不能绕过写API
    // (API重新设计)。
    shm_set_data_state(chunk, IOX_CHUNK_CONTAINS_RAW_DATA); // 设置chunk的数据状态。
    return chunk;
  }
  return NULL;
}

#endif
/**
 * @brief 为写入者实体分配共享内存缓冲区
 *
 * @param[in] writer 写入者实体
 * @param[in] size 请求的缓冲区大小
 * @param[out] buffer 分配的缓冲区指针
 * @return dds_return_t 操作结果
 */
dds_return_t dds_loan_shared_memory_buffer(dds_entity_t writer, size_t size,
                                           void **buffer)
{
#ifndef DDS_HAS_SHM
  (void)writer; // 忽略未使用的参数
  (void)size;
  (void)buffer;
  return DDS_RETCODE_UNSUPPORTED; // 不支持共享内存时返回错误码
#else
  dds_return_t ret;
  dds_writer *wr;

  if (!buffer)
    return DDS_RETCODE_BAD_PARAMETER; // 参数检查

  if ((ret = dds_writer_lock(writer, &wr)) != DDS_RETCODE_OK)
    return ret; // 锁定写入者失败时返回错误码

  if (wr->m_iox_pub)
  {
    *buffer = shm_create_chunk(wr->m_iox_pub, size); // 创建共享内存块
    if (*buffer == NULL)
    {
      ret = DDS_RETCODE_ERROR; // 无法获取缓冲区内存时返回错误码
    }
    shm_set_data_state(*buffer, IOX_CHUNK_UNINITIALIZED); // 设置数据状态
  }
  else
  {
    ret = DDS_RETCODE_UNSUPPORTED; // 不支持共享内存时返回错误码
  }

  dds_writer_unlock(wr); // 解锁写入者
  return ret;
#endif
}

/**
 * @brief 为写入者实体分配样本
 *
 * @param[in] writer 写入者实体
 * @param[out] sample 分配的样本指针
 * @return dds_return_t 操作结果
 */
dds_return_t dds_loan_sample(dds_entity_t writer, void **sample)
{
#ifndef DDS_HAS_SHM
  (void)writer; // 忽略未使用的参数
  (void)sample;
  return DDS_RETCODE_UNSUPPORTED; // 不支持共享内存时返回错误码
#else
  dds_return_t ret;
  dds_writer *wr;

  if (!sample)
    return DDS_RETCODE_BAD_PARAMETER; // 参数检查

  if ((ret = dds_writer_lock(writer, &wr)) != DDS_RETCODE_OK)
    return ret; // 锁定写入者失败时返回错误码

  // 只有在正确启用共享内存并且类型固定时才允许借用
  if (wr->m_iox_pub && wr->m_topic->m_stype->fixed_size)
  {
    *sample = dds_writer_loan_chunk(wr, wr->m_topic->m_stype->iox_size); // 借用一个块
    if (*sample == NULL)
    {
      ret = DDS_RETCODE_ERROR; // 无法获取样本时返回错误码
    }
  }
  else
  {
    ret = DDS_RETCODE_UNSUPPORTED; // 不支持共享内存时返回错误码
  }

  dds_writer_unlock(wr); // 解锁写入者
  return ret;
#endif
}
/**
 * @brief 为DDS写入器返回贷款
 *
 * @param[in] writer DDS写入器指针
 * @param[in,out] buf 缓冲区指针数组，用于存储贷款数据
 * @param[in] bufsz 缓冲区大小（数组长度）
 * @return dds_return_t 返回操作结果代码
 */
dds_return_t dds_return_writer_loan(dds_writer *writer, void **buf,
                                    int32_t bufsz)
{
#ifndef DDS_HAS_SHM
  (void)writer;                   // 不使用的参数
  (void)buf;                      // 不使用的参数
  (void)bufsz;                    // 不使用的参数
  return DDS_RETCODE_UNSUPPORTED; // 如果没有共享内存支持，则返回不支持的错误代码
#else
  // Iceoryx发布者指针是常量，因此我们可以在锁外部检查
  // 只有在正确启用SHM（即iox发布者已初始化）且类型固定时，才能返回贷款
  if (writer->m_iox_pub == NULL || !writer->m_topic->m_stype->fixed_size)
    return DDS_RETCODE_UNSUPPORTED;

  if (bufsz <= 0)
  {
    // 类似于长期存在的读取器情况，这在某种程度上是有意义的，
    // 因为它允许传递读取/获取操作的结果，无论该操作是否成功
    return DDS_RETCODE_OK;
  }

  ddsrt_mutex_lock(&writer->m_entity.m_mutex); // 锁定互斥量
  dds_return_t ret = DDS_RETCODE_OK;
  for (int32_t i = 0; i < bufsz; i++)
  {
    if (buf[i] == NULL)
    {
      ret = DDS_RETCODE_BAD_PARAMETER; // 如果缓冲区指针为空，则返回错误参数代码
      break;
    }
    else if (!dds_deregister_pub_loan(writer, buf[i]))
    {
      ret = DDS_RETCODE_PRECONDITION_NOT_MET; // 如果无法取消注册贷款，则返回未满足的前提条件代码
      break;
    }
    else
    {
      release_iox_chunk(writer, buf[i]); // 释放iox块
      // 在读取器上返回贷款会将buf[0]置空，但在这里更有意义的是清除所有成功返回的贷款：
      // 然后，在失败时，应用程序可以通过查找第一个非空指针来判断哪些贷款未返回
      buf[i] = NULL;
    }
  }
  ddsrt_mutex_unlock(&writer->m_entity.m_mutex); // 解锁互斥量
  return ret;
#endif
}
