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
#include <assert.h>
#include <string.h>

#include "dds/cdr/dds_cdrstream.h"
#include "dds/ddsc/dds_loan_api.h"
#include "dds/ddsi/ddsi_deliver_locally.h"
#include "dds/ddsi/ddsi_domaingv.h"
#include "dds/ddsi/ddsi_endpoint.h"
#include "dds/ddsi/ddsi_endpoint_match.h"
#include "dds/ddsi/ddsi_entity.h"
#include "dds/ddsi/ddsi_entity_index.h"
#include "dds/ddsi/ddsi_radmin.h" /* sampleinfo */
#include "dds/ddsi/ddsi_rhc.h"
#include "dds/ddsi/ddsi_serdata.h"
#include "dds/ddsi/ddsi_thread.h"
#include "dds/ddsi/ddsi_tkmap.h"
#include "dds/ddsi/ddsi_transmit.h"
#include "dds/ddsi/ddsi_xmsg.h"
#include "dds__loan.h"
#include "dds__write.h"
#include "dds__writer.h"

#ifdef DDS_HAS_SHM
#include "dds/ddsi/ddsi_addrset.h"
#include "dds/ddsi/ddsi_shm_transport.h"
#endif

// 定义ddsi_serdata_plain结构体
struct ddsi_serdata_plain {
  struct ddsi_serdata p;
};

// 定义ddsi_serdata_iox结构体
struct ddsi_serdata_iox {
  struct ddsi_serdata x;
};

// 定义ddsi_serdata_any结构体
struct ddsi_serdata_any {
  struct ddsi_serdata a;
};

/**
 * @brief 将数据写入DDS实体中
 *
 * @param[in] writer DDS实体的标识符
 * @param[in] data 要写入的数据指针
 * @return dds_return_t 返回操作结果，成功返回DDS_RETCODE_OK，否则返回相应的错误码
 */
dds_return_t dds_write(dds_entity_t writer, const void *data) {
  dds_return_t ret;
  dds_writer *wr;

  // 检查输入数据是否为空
  if (data == NULL) return DDS_RETCODE_BAD_PARAMETER;

  // 锁定写入器并获取其指针
  if ((ret = dds_writer_lock(writer, &wr)) != DDS_RETCODE_OK) return ret;

  // 调用dds_write_impl函数执行写入操作
  ret = dds_write_impl(wr, data, dds_time(), 0);

  // 解锁写入器
  dds_writer_unlock(wr);

  // 返回操作结果
  return ret;
}

/**
 * @brief 将序列化数据写入DDS实体中
 *
 * @param[in] writer DDS实体的标识符
 * @param[in] serdata 要写入的序列化数据指针
 * @return dds_return_t 返回操作结果，成功返回DDS_RETCODE_OK，否则返回相应的错误码
 */
dds_return_t dds_writecdr(dds_entity_t writer, struct ddsi_serdata *serdata) {
  dds_return_t ret;
  dds_writer *wr;

  // 检查输入序列化数据是否为空
  if (serdata == NULL) return DDS_RETCODE_BAD_PARAMETER;

  // 锁定写入器并获取其指针
  if ((ret = dds_writer_lock(writer, &wr)) != DDS_RETCODE_OK) return ret;

  // 检查主题过滤模式是否为NONE
  if (wr->m_topic->m_filter.mode != DDS_TOPIC_FILTER_NONE) {
    dds_writer_unlock(wr);
    return DDS_RETCODE_ERROR;
  }

  // 设置序列化数据的状态信息和时间戳
  serdata->statusinfo = 0;
  serdata->timestamp.v = dds_time();

  // 调用dds_writecdr_impl函数执行写入操作
  ret = dds_writecdr_impl(wr, wr->m_xp, serdata, !wr->whc_batch);

  // 解锁写入器
  dds_writer_unlock(wr);

  // 返回操作结果
  return ret;
}

/**
 * @brief 将序列化数据转发到DDS实体中
 *
 * @param[in] writer DDS实体的标识符
 * @param[in] serdata 要转发的序列化数据指针
 * @return dds_return_t 返回操作结果，成功返回DDS_RETCODE_OK，否则返回相应的错误码
 */
dds_return_t dds_forwardcdr(dds_entity_t writer, struct ddsi_serdata *serdata) {
  dds_return_t ret;
  dds_writer *wr;

  // 检查输入序列化数据是否为空
  if (serdata == NULL) return DDS_RETCODE_BAD_PARAMETER;

  // 锁定写入器并获取其指针
  if ((ret = dds_writer_lock(writer, &wr)) != DDS_RETCODE_OK) return ret;

  // 检查主题过滤模式是否为NONE
  if (wr->m_topic->m_filter.mode != DDS_TOPIC_FILTER_NONE) {
    dds_writer_unlock(wr);
    return DDS_RETCODE_ERROR;
  }

  // 调用dds_writecdr_impl函数执行转发操作
  ret = dds_writecdr_impl(wr, wr->m_xp, serdata, !wr->whc_batch);

  // 解锁写入器
  dds_writer_unlock(wr);

  // 返回操作结果
  return ret;
}
/**
 * @brief 将数据写入DDS实体（数据写入器）。
 *
 * @param writer 数据写入器实体。
 * @param data 要写入的数据指针。
 * @param timestamp 时间戳。
 * @return dds_return_t 返回操作结果。
 */
dds_return_t dds_write_ts(dds_entity_t writer, const void *data, dds_time_t timestamp) {
  dds_return_t ret;
  dds_writer *wr;

  // 检查数据和时间戳是否有效
  if (data == NULL || timestamp < 0) return DDS_RETCODE_BAD_PARAMETER;

  // 锁定数据写入器
  if ((ret = dds_writer_lock(writer, &wr)) != DDS_RETCODE_OK) return ret;

  // 写入数据
  ret = dds_write_impl(wr, data, timestamp, 0);

  // 解锁数据写入器
  dds_writer_unlock(wr);

  return ret;
}

// 定义本地源信息结构体
struct local_sourceinfo {
  const struct ddsi_sertype *src_type;
  struct ddsi_serdata *src_payload;
  struct ddsi_tkmap_instance *src_tk;
  ddsrt_mtime_t timeout;
};

/**
 * @brief 创建本地样本。
 *
 * @param tk 实例映射指针。
 * @param gv 域全局变量。
 * @param type 类型。
 * @param vsourceinfo 本地源信息。
 * @return struct ddsi_serdata* 序列化数据。
 */
static struct ddsi_serdata *local_make_sample(struct ddsi_tkmap_instance **tk,
                                              struct ddsi_domaingv *gv,
                                              struct ddsi_sertype const *const type,
                                              void *vsourceinfo) {
  struct local_sourceinfo *si = vsourceinfo;
  struct ddsi_serdata *d = ddsi_serdata_ref_as_type(type, si->src_payload);

  // 如果序列化数据为空，记录警告并返回空
  if (d == NULL) {
    DDS_CWARNING(&gv->logconfig, "local: deserialization %s failed in type conversion\n",
                 type->type_name);
    return NULL;
  }

  // 检查类型是否相同
  if (type != si->src_type)
    *tk = ddsi_tkmap_lookup_instance_ref(gv->m_tkmap, d);
  else {
    // 如果类型相同，则避免查找
    ddsi_tkmap_instance_ref(si->src_tk);
    *tk = si->src_tk;
  }

  return d;
}

/**
 * @brief 快速路径上的本地传输失败处理。
 *
 * @param source_entity 源实体。
 * @param source_entity_locked 源实体锁定状态。
 * @param fastpath_rdary 快速路径读取器数组。
 * @param vsourceinfo 本地源信息。
 * @return dds_return_t 返回操作结果。
 */
static dds_return_t local_on_delivery_failure_fastpath(struct ddsi_entity_common *source_entity,
                                                       bool source_entity_locked,
                                                       struct ddsi_local_reader_ary *fastpath_rdary,
                                                       void *vsourceinfo) {
  (void)fastpath_rdary;
  (void)source_entity_locked;

  // 断言源实体为写入器
  assert(source_entity->kind == DDSI_EK_WRITER);

  struct ddsi_writer *wr = (struct ddsi_writer *)source_entity;
  struct local_sourceinfo *si = vsourceinfo;

  // 获取当前时间
  ddsrt_mtime_t tnow = ddsrt_time_monotonic();

  // 设置超时时间
  if (si->timeout.v == 0)
    si->timeout = ddsrt_mtime_add_duration(tnow, wr->xqos->reliability.max_blocking_time);

  // 检查是否超时
  if (tnow.v >= si->timeout.v)
    return DDS_RETCODE_TIMEOUT;
  else {
    // 等待一段时间后重试
    dds_sleepfor(DDS_HEADBANG_TIMEOUT);
    return DDS_RETCODE_OK;
  }
}
/**
 * @brief 本地传递数据的函数
 *
 * @param[in] wr       写入器指针
 * @param[in] payload  序列化数据指针
 * @param[in] tk       tkmap实例指针
 * @return dds_return_t 返回DDS操作结果
 */
static dds_return_t deliver_locally(struct ddsi_writer *wr,
                                    struct ddsi_serdata *payload,
                                    struct ddsi_tkmap_instance *tk) {
  // 定义本地传递操作结构体常量
  static const struct ddsi_deliver_locally_ops deliver_locally_ops = {
      .makesample = local_make_sample,
      .first_reader = ddsi_writer_first_in_sync_reader,
      .next_reader = ddsi_writer_next_in_sync_reader,
      .on_failure_fastpath = local_on_delivery_failure_fastpath};

  // 初始化源信息结构体
  struct local_sourceinfo sourceinfo = {
      .src_type = wr->type,
      .src_payload = payload,
      .src_tk = tk,
      .timeout = {0},
  };

  dds_return_t rc;
  struct ddsi_writer_info wrinfo;

  // 创建写入器信息
  ddsi_make_writer_info(&wrinfo, &wr->e, wr->xqos, payload->statusinfo);

  // 尝试将数据传递给所有同步的读取器
  rc = ddsi_deliver_locally_allinsync(wr->e.gv, &wr->e, false, &wr->rdary, &wrinfo,
                                      &deliver_locally_ops, &sourceinfo);

  // 如果返回超时，则记录错误日志
  if (rc == DDS_RETCODE_TIMEOUT)
    DDS_CERROR(
        &wr->e.gv->logconfig,
        "The writer could not deliver data on time, probably due to a local reader resources being "
        "full\n");

  return rc;
}

#if DDS_HAS_SHM
/**
 * @brief 通过Iceoryx传递数据的函数
 *
 * @param[in] wr 写入器指针
 * @param[in] d  序列化数据iox结构体指针
 */
static void deliver_data_via_iceoryx(dds_writer *wr, struct ddsi_serdata_iox *d) {
  // 获取chunk头部信息
  iox_chunk_header_t *chunk_header = iox_chunk_header_from_user_payload(d->x.iox_chunk);

  // 获取Iceoryx头部信息
  iceoryx_header_t *ice_hdr = iox_chunk_header_to_user_header(chunk_header);

  // 设置Iceoryx头部信息
  ice_hdr->guid = wr->m_wr->e.guid;
  ice_hdr->tstamp = d->x.timestamp.v;
  ice_hdr->statusinfo = d->x.statusinfo;
  ice_hdr->data_kind = (unsigned char)d->x.kind;

  // 获取并设置keyhash
  ddsi_serdata_get_keyhash(&d->x, &ice_hdr->keyhash, false);

  // 发布chunk，传递所有权
  iox_pub_publish_chunk(wr->m_iox_pub, d->x.iox_chunk);

  // 将iox_chunk设置为NULL，提高检测竞争条件的几率
  d->x.iox_chunk = NULL;
}
#endif
/**
 * @brief 将输入的序列化数据转换为与给定的 ddsi_writer 类型匹配的序列化数据
 *
 * @param[in] ddsi_wr 一个指向 ddsi_writer 结构的指针
 * @param[in] din 一个指向 ddsi_serdata_any 结构的指针，表示输入的序列化数据
 * @return 返回一个指向 ddsi_serdata_any 结构的指针，表示转换后的序列化数据
 */
static struct ddsi_serdata_any *convert_serdata(struct ddsi_writer *ddsi_wr,
                                                struct ddsi_serdata_any *din) {
  struct ddsi_serdata_any *dout;
  if (ddsi_wr->type == din->a.type)  // 如果 ddsi_writer 的类型与输入数据的类型相同
  {
    dout = din;                      // 直接使用输入数据
    // dout refc: must consume 1
    // din refc: must consume 0 (it is an alias of dact)
  } else  // 如果 ddsi_writer 的类型与输入数据的类型不同
  {
    assert(din->a.type->ops->version == ddsi_sertype_v0);
    // 故意允许 d->type 和 ddsi_wr->type 之间的不匹配:
    // 这样我们可以允许将数据从一个域传输到另一个域
    dout = (struct ddsi_serdata_any *)ddsi_serdata_ref_as_type(ddsi_wr->type, &din->a);
    // dout refc: must consume 1
    // din refc: must consume 1 (independent of dact: types are distinct)
  }
  return dout;
}

/**
 * @brief 将数据通过网络传输
 *
 * @param[in] thrst 指向 ddsi_thread_state 结构的指针
 * @param[in] ddsi_wr 指向 ddsi_writer 结构的指针
 * @param[in] d 指向 ddsi_serdata_any 结构的指针，表示要传输的数据
 * @param[in] xp 指向 ddsi_xpack 结构的指针
 * @param[in] flush 是否立即发送数据包
 * @param[in] tk 指向 ddsi_tkmap_instance 结构的指针
 * @return 返回 dds_return_t 类型的结果，表示操作是否成功
 */
static dds_return_t deliver_data_network(struct ddsi_thread_state *const thrst,
                                         struct ddsi_writer *ddsi_wr,
                                         struct ddsi_serdata_any *d,
                                         struct ddsi_xpack *xp,
                                         bool flush,
                                         struct ddsi_tkmap_instance *tk) {
  // ddsi_write_sample_gc always consumes 1 refc from d
  int ret = ddsi_write_sample_gc(thrst, xp, ddsi_wr, &d->a, tk);
  if (ret >= 0)  // 如果写入样本成功
  {
    /* Flush out write unless configured to batch */
    if (flush && xp != NULL)  // 如果需要立即发送数据包
      ddsi_xpack_send(xp, false);
    return DDS_RETCODE_OK;
  } else  // 如果写入样本失败
  {
    return (ret == DDS_RETCODE_TIMEOUT) ? ret : DDS_RETCODE_ERROR;
  }
}

/**
 * @brief 将数据传输到任意目标
 *
 * @param[in] thrst 指向 ddsi_thread_state 结构的指针
 * @param[in] ddsi_wr 指向 ddsi_writer 结构的指针
 * @param[in] wr 指向 dds_writer 结构的指针
 * @param[in] d 指向 ddsi_serdata_any 结构的指针，表示要传输的数据
 * @param[in] xp 指向 ddsi_xpack 结构的指针
 * @param[in] flush 是否立即发送数据包
 * @return 返回 dds_return_t 类型的结果，表示操作是否成功
 */
static dds_return_t deliver_data_any(struct ddsi_thread_state *const thrst,
                                     struct ddsi_writer *ddsi_wr,
                                     dds_writer *wr,
                                     struct ddsi_serdata_any *d,
                                     struct ddsi_xpack *xp,
                                     bool flush) {
  struct ddsi_tkmap_instance *const tk =
      ddsi_tkmap_lookup_instance_ref(ddsi_wr->e.gv->m_tkmap, &d->a);
  dds_return_t ret;
  if ((ret = deliver_data_network(thrst, ddsi_wr, d, xp, flush, tk)) != DDS_RETCODE_OK) {
    ddsi_tkmap_instance_unref(ddsi_wr->e.gv->m_tkmap, tk);
    return ret;
  }
#ifdef DDS_HAS_SHM
  if (d->a.iox_chunk != NULL) {
    // delivers to all iceoryx readers, including local ones
    deliver_data_via_iceoryx(wr, (struct ddsi_serdata_iox *)d);
  }
#else
  (void)wr;
#endif
  ret = deliver_locally(ddsi_wr, &d->a, tk);
  ddsi_tkmap_instance_unref(ddsi_wr->e.gv->m_tkmap, tk);
  return ret;
}
/**
 * @brief 写入CDR序列化数据的通用实现函数
 *
 * @param[in] ddsi_wr 一个ddsi_writer结构体指针
 * @param[in] xp 一个ddsi_xpack结构体指针
 * @param[in] din 一个ddsi_serdata_any结构体指针，表示输入的序列化数据
 * @param[in] flush 布尔值，表示是否立即刷新缓冲区
 * @param[in] wr 一个dds_writer结构体指针
 * @return dds_return_t 返回操作结果状态码
 */
static dds_return_t dds_writecdr_impl_common(struct ddsi_writer *ddsi_wr,
                                             struct ddsi_xpack *xp,
                                             struct ddsi_serdata_any *din,
                                             bool flush,
                                             dds_writer *wr) {
  // 在所有路径中消耗din的1个引用计数（奇怪，但...历史原因...）
  // 设refc(din)为r，因此在返回时必须是r-1
  struct ddsi_thread_state *const thrst = ddsi_lookup_thread_state();
  int ret = DDS_RETCODE_OK;
  assert(wr != NULL);

  struct ddsi_serdata_any *const d = convert_serdata(ddsi_wr, din);
  if (d == NULL) {
    ddsi_serdata_unref(&din->a);  // refc(din) = r - 1，满足要求
    return DDS_RETCODE_ERROR;
  }

  // d = din: refc(d) = r, 否则 refc(d) = 1

  ddsi_thread_state_awake(thrst, ddsi_wr->e.gv);
  ddsi_serdata_ref(&d->a);  // d = din: refc(d) = r + 1, 否则 refc(d) = 2

#ifdef DDS_HAS_SHM
  // 如果存在iceoryx chunk，则转移所有权
  // din和d可能是别名
  // 注意：为了提高效率，使用这些赋值而不是if语句（跳转）
  void *iox_chunk = din->a.iox_chunk;
  din->a.iox_chunk = NULL;
  d->a.iox_chunk = iox_chunk;
  assert((wr->m_iox_pub == NULL) == (d->a.iox_chunk == NULL));
#endif

  ret = deliver_data_any(thrst, ddsi_wr, wr, d, xp, flush);

  if (d != din)
    ddsi_serdata_unref(&din->a);  // d != din: refc(din) = r - 1，满足要求，refc(d)保持不变
  ddsi_serdata_unref(&d->a);  // d = din: refc(d) = r - 1, 否则 refc(din) = r-1 和 refc(d) = 0

  ddsi_thread_state_asleep(thrst);
  return ret;
}

/**
 * @brief 评估主题过滤器的函数
 *
 * @param[in] wr 一个dds_writer结构体指针
 * @param[in] data 指向数据的指针
 * @param[in] writekey 布尔值，表示是否写入键
 * @return bool 返回过滤器是否接受数据，如果被拒绝则返回false
 */
static bool evalute_topic_filter(const dds_writer *wr, const void *data, bool writekey) {
  // 如果数据被过滤器拒绝，则返回false
  if (wr->m_topic->m_filter.mode == DDS_TOPIC_FILTER_NONE || writekey) return true;

  const struct dds_topic_filter *f = &wr->m_topic->m_filter;
  switch (f->mode) {
    case DDS_TOPIC_FILTER_NONE:
    case DDS_TOPIC_FILTER_SAMPLEINFO_ARG:
      break;
    case DDS_TOPIC_FILTER_SAMPLE:
      if (!f->f.sample(data)) return false;
      break;
    case DDS_TOPIC_FILTER_SAMPLE_ARG:
      if (!f->f.sample_arg(data, f->arg)) return false;
      break;
    case DDS_TOPIC_FILTER_SAMPLE_SAMPLEINFO_ARG: {
      struct dds_sample_info si;
      memset(&si, 0, sizeof(si));
      if (!f->f.sample_sampleinfo_arg(data, &si, f->arg)) return false;
      break;
    }
  }
  return true;
}

/**
 * @brief 设置状态信息和时间戳
 *
 * @param[in] d       指向ddsi_serdata_any结构体的指针
 * @param[in] tstamp  时间戳
 * @param[in] action  写操作动作
 */
static void set_statusinfo_timestamp(struct ddsi_serdata_any *d,
                                     dds_time_t tstamp,
                                     dds_write_action action) {
  // 根据写操作动作设置状态信息
  d->a.statusinfo = (((action & DDS_WR_DISPOSE_BIT) ? DDSI_STATUSINFO_DISPOSE : 0) |
                     ((action & DDS_WR_UNREGISTER_BIT) ? DDSI_STATUSINFO_UNREGISTER : 0));
  // 设置时间戳
  d->a.timestamp.v = tstamp;
}

#ifdef DDS_HAS_SHM
/**
 * @brief 获取所需缓冲区大小
 *
 * @param[in] topic   主题指针
 * @param[in] sample  数据样本指针
 * @return 返回所需缓冲区大小
 */
static size_t get_required_buffer_size(struct dds_topic *topic, const void *sample) {
  // 判断是否为固定大小类型
  bool has_fixed_size_type = topic->m_stype->fixed_size;
  if (has_fixed_size_type) {
    return topic->m_stype->iox_size;
  }

  // 获取序列化后的大小
  return ddsi_sertype_get_serialized_size(topic->m_stype, (void *)sample);
}

/**
 * @brief 填充iox块
 *
 * @param[in] wr          写入器指针
 * @param[in] sample      数据样本指针
 * @param[out] iox_chunk  iox块指针
 * @param[in] sample_size 数据样本大小
 * @return 返回是否成功填充iox块
 */
static bool fill_iox_chunk(dds_writer *wr,
                           const void *sample,
                           void *iox_chunk,
                           size_t sample_size) {
  // 判断是否为固定大小类型
  bool has_fixed_size_type = wr->m_topic->m_stype->fixed_size;
  bool ret = true;
  iceoryx_header_t *iox_header = iceoryx_header_from_chunk(iox_chunk);
  if (has_fixed_size_type) {
    // 复制数据到iox块
    memcpy(iox_chunk, sample, sample_size);
    iox_header->shm_data_state = IOX_CHUNK_CONTAINS_RAW_DATA;
  } else {
    // 序列化数据到iox块
    size_t size = iox_header->data_size;
    ret = ddsi_sertype_serialize_into(wr->m_wr->type, sample, iox_chunk, size);
    if (ret) {
      iox_header->shm_data_state = IOX_CHUNK_CONTAINS_SERIALIZED_DATA;
    } else {
      // 数据处于无效状态
      iox_header->shm_data_state = IOX_CHUNK_UNINITIALIZED;
    }
  }
  return ret;
}

/**
 * @brief 创建并填充一个iox_chunk。
 *
 * @param[in] wr 指向dds_writer的指针。
 * @param[in] data 要写入的数据的指针。
 * @param[out] iox_chunk 用于存储创建的iox_chunk的指针。
 * @return 成功时返回DDS_RETCODE_OK，否则返回相应的错误代码。
 */
static dds_return_t create_and_fill_chunk(dds_writer *wr, const void *data, void **iox_chunk) {
  // 获取所需缓冲区大小
  const size_t required_size = get_required_buffer_size(wr->m_topic, data);
  // 如果所需大小为SIZE_MAX，则返回资源不足错误
  if (required_size == SIZE_MAX) return DDS_RETCODE_OUT_OF_RESOURCES;
  // 尝试创建shm_chunk，如果失败则返回资源不足错误
  if ((*iox_chunk = shm_create_chunk(wr->m_iox_pub, required_size)) == NULL)
    return DDS_RETCODE_OUT_OF_RESOURCES;
  // 填充iox_chunk，如果失败则返回错误参数
  if (!fill_iox_chunk(wr, data, *iox_chunk, required_size))
    return DDS_RETCODE_BAD_PARAMETER;  // 序列化失败
  // 成功返回
  return DDS_RETCODE_OK;
}

/**
 * @brief 获取一个iox_chunk。
 *
 * @param[in] wr 指向dds_writer的指针。
 * @param[in] data 要写入的数据的指针。
 * @param[out] iox_chunk 用于存储获取的iox_chunk的指针。
 * @return 成功时返回DDS_RETCODE_OK，否则返回相应的错误代码。
 */
static dds_return_t get_iox_chunk(dds_writer *wr, const void *data, void **iox_chunk) {
  // 注意：在非iceoryx情况下，当前无法确定数据是否已借出
  if (!dds_deregister_pub_loan(wr, data))
    return create_and_fill_chunk(wr, data, iox_chunk);
  else {
    // 用户已经通过dds_loan API提供了一个带有数据的iceoryx_chunk
    // 我们假设如果我们从贷款中获得了数据，它包含原始数据（即未序列化）
    // 这需要用户遵守合同，我们不能用给定的API强制执行这一点
    *iox_chunk = (void *)data;
    iceoryx_header_t *iox_header = iceoryx_header_from_chunk(*iox_chunk);
    iox_header->shm_data_state = IOX_CHUNK_CONTAINS_RAW_DATA;
    return DDS_RETCODE_OK;
  }
}

/**
 * @brief 同步当前快速路径读取器的数量并返回它。
 *
 * 需要锁定互斥锁以同步该值。当我们不持有锁时，此数字可能随时更改，
 * 即从函数返回时变为过时。
 *
 * @param[in] ddsi_wr 指向ddsi_writer的指针。
 * @return 当前快速路径读取器的数量。
 */
static uint32_t get_num_fast_path_readers(struct ddsi_writer *ddsi_wr) {
  // 锁定互斥锁
  ddsrt_mutex_lock(&ddsi_wr->rdary.rdary_lock);
  // 获取读取器数量
  uint32_t n = ddsi_wr->rdary.n_readers;
  // 解锁互斥锁
  ddsrt_mutex_unlock(&ddsi_wr->rdary.rdary_lock);
  // 返回读取器数量
  return n;
}
/**
 * @brief 写入实现函数，支持两种情况：
 * 1) 数据在栈上或动态分配的外部缓冲区中
 * 2) 数据在通过dds_loan_sample获得的iceoryx缓冲区中
 *
 * @param[in] wr dds_writer指针
 * @param[in] ddsi_wr ddsi_writer指针
 * @param[in] writekey 是否写入键值
 * @param[in] data 要写入的数据指针
 * @param[in] tstamp 时间戳
 * @param[in] action 写入操作类型
 * @return dds_return_t 返回执行结果
 */
static dds_return_t dds_write_impl_iox(dds_writer *wr,
                                       struct ddsi_writer *ddsi_wr,
                                       bool writekey,
                                       const void *data,
                                       dds_time_t tstamp,
                                       dds_write_action action) {
  // 确保线程已唤醒
  assert(ddsi_thread_is_awake());
  // 确保wr和wr->m_iox_pub不为空
  assert(wr != NULL && wr->m_iox_pub != NULL);

  void *iox_chunk = NULL;
  dds_return_t ret;

  // 注意：目前无法在非iceoryx情况下确定数据是否被借用
  if ((ret = get_iox_chunk(wr, data, &iox_chunk)) != 0) return ret;
  // 确保iox_chunk不为空
  assert(iox_chunk != NULL);

  // 以下条件成立：
  // 1) data仍然指向原始数据
  // 2) iox_chunk不为空
  //    a) 由写调用创建
  //    b) 数据是iceoryx块，且iox_chunk == data
  // 在情况2 a)中，iox_chunk将包含序列化或原始数据
  //
  // 在情况2 a)和b)中，我们必须确保发布或释放块（失败）
  // 在2b)中，我们可以认为不释放块，但用户通过调用write实际上传递了所有权

  // ddsi_wr->as可能在没有锁定的情况下被匹配/不匹配的代理读取器更改
  // 不幸的是，这意味着我们必须在此处检查锁定，然后再次锁定以实际分发数据，因此需要进一步重构。
  ddsrt_mutex_lock(&ddsi_wr->e.lock);
  const bool no_network_readers = ddsi_addrset_empty(ddsi_wr->as);
  ddsrt_mutex_unlock(&ddsi_wr->e.lock);

  // 注意：使用iceoryx的本地读取器不在L := ddsi_wr->rdary中。
  // 此外，忽略本地发布者的所有读取器都不会使用iceoryx。
  // 如果L中有任何本地读取器，我们将永远不仅仅使用iceoryx。
  // 在这种情况下，我们将序列化数据并混合交付，即
  // 根据QoS和类型的要求部分使用iceoryx。L中的读取器
  // 将通过本地传输机制（快速路径或慢速路径）获取数据。

  const uint32_t num_fast_path_readers = get_num_fast_path_readers(ddsi_wr);

  // 如果use_only_iceoryx为true，则在我们检查时没有快速路径读取器。
  // 如果以后出现快速路径读取器，它们可能无法获取数据，但这
  // 是可以接受的，因为我们可以认为它们的连接尚未完全建立
  // 因此，它们不被视为数据传输。
  // 另一种方法是在数据传输完成之前完全阻止新的快速路径连接（通过保持互斥锁）。
  const bool use_only_iceoryx = no_network_readers &&
                                ddsi_wr->xqos->durability.kind == DDS_DURABILITY_VOLATILE &&
                                num_fast_path_readers == 0;

  // 4. 准备serdata
  // 如果没有网络读取器，避免对易失性写入器进行序列化

  /** @brief 为下面这段代码以支持 doxygen的形式为函数添加参数列表的说明，并逐行添加详细的中文注释
   *
   * @param[in] use_only_iceoryx 是否仅使用 iceoryx 进行通信
   * @param[in] ddsi_wr 写入器的类型信息
   * @param[in] writekey 是否需要写入键值
   * @param[in] iox_chunk 用于存储数据的 iceoryx 块
   * @param[in] data 需要发送的数据
   * @param[in] tstamp 时间戳
   * @param[in] action 操作类型
   * @param[in] wr 写入器对象
   * @return 返回操作结果，成功时返回 DDS_RETCODE_OK
   */
  struct ddsi_serdata_iox *d = NULL;

  if (use_only_iceoryx) {
    // 如果仅使用 iceoryx，则不立即序列化数据（可能在没有网络读取器的情况下不需要序列化）
    d = (struct ddsi_serdata_iox *)ddsi_serdata_from_loaned_sample(
        ddsi_wr->type, writekey ? SDK_KEY : SDK_DATA, iox_chunk);
  } else {
    // 由于我们需要通过网络发送数据，因此对其进行序列化
    // 我们还需要将数据序列化到一个 iceoryx 块中

    struct ddsi_serdata *dtmp =
        ddsi_serdata_from_sample(ddsi_wr->type, writekey ? SDK_KEY : SDK_DATA, data);
    if (dtmp != NULL) {
      // 当 serdata d 是为网络路径创建的，但也可以使用 iceoryx 时需要
      // 在这种情况下，d 是通过 ddsi_serdata_from_sample 创建的，我们需要设置 iceoryx 块
      dtmp->iox_chunk = iox_chunk;
      d = (struct ddsi_serdata_iox *)dtmp;
    }
  }

  if (d == NULL) {
    iox_pub_release_chunk(wr->m_iox_pub, iox_chunk);
    return DDS_RETCODE_BAD_PARAMETER;
  }

  // 断言：确保 d->x.iox_chunk 不为空
  assert(d->x.iox_chunk != NULL);

  // 在成功构建后，设置状态信息和时间戳
  set_statusinfo_timestamp((struct ddsi_serdata_any *)d, tstamp, action);

  // 5. 传递数据
  if (use_only_iceoryx) {
    // 仅通过 iceoryx 传递数据
    // TODO: 在这种情况下，我们能避免构造 d 吗？
    // 在这种情况下，没有本地非 iceoryx 读取器
    deliver_data_via_iceoryx(wr, d);
    ddsi_serdata_unref(&d->x);  // refc(d) = 0
  } else {
    // 这可能会在需要时转换输入数据（convert_serdata），然后根据需要使用网络和/或 iceoryx 传递它
    // d refc(d) = 1，调用将减少引用计数 1
    ret = dds_writecdr_impl_common(ddsi_wr, wr->m_xp, (struct ddsi_serdata_any *)d, !wr->whc_batch,
                                   wr);
    if (ret != DDS_RETCODE_OK) iox_pub_release_chunk(wr->m_iox_pub, d->x.iox_chunk);
  }

  return ret;
}
#endif
/**
 * @brief 写入实现函数（纯数据）
 *
 * @param[in] wr          DDS writer 对象
 * @param[in] ddsi_wr     DDSI writer 对象
 * @param[in] writekey    是否仅写入键值
 * @param[in] data        要写入的数据
 * @param[in] tstamp      时间戳
 * @param[in] action      写入操作类型
 * @return dds_return_t   返回操作结果
 */
static dds_return_t dds_write_impl_plain(dds_writer *wr,
                                         struct ddsi_writer *ddsi_wr,
                                         bool writekey,
                                         const void *data,
                                         dds_time_t tstamp,
                                         dds_write_action action) {
  // 确保线程已唤醒
  assert(ddsi_thread_is_awake());

#ifdef DDS_HAS_SHM
  // 确保共享内存发布者为空
  assert(wr->m_iox_pub == NULL);
#endif

  // 定义并初始化序列化数据对象
  struct ddsi_serdata_plain *d = NULL;
  d = (struct ddsi_serdata_plain *)ddsi_serdata_from_sample(ddsi_wr->type,
                                                            writekey ? SDK_KEY : SDK_DATA, data);

  // 检查序列化数据是否为空
  if (d == NULL) return DDS_RETCODE_BAD_PARAMETER;

  // 设置状态信息和时间戳
  set_statusinfo_timestamp((struct ddsi_serdata_any *)d, tstamp, action);

  // 调用通用写入实现函数
  return dds_writecdr_impl_common(ddsi_wr, wr->m_xp, (struct ddsi_serdata_any *)d, !wr->whc_batch,
                                  wr);
}

/**
 * @brief 写入实现函数
 *
 * 支持两种情况：
 * 1) 数据在栈上或动态分配的外部缓冲区中
 * 2) 数据在由 dds_loan_sample 获取的 iceoryx 缓冲区中
 *
 * @param[in] wr          DDS writer 对象
 * @param[in] data        要写入的数据
 * @param[in] tstamp      时间戳
 * @param[in] action      写入操作类型
 * @return dds_return_t   返回操作结果
 */
dds_return_t dds_write_impl(dds_writer *wr,
                            const void *data,
                            dds_time_t tstamp,
                            dds_write_action action) {
  // 1. 输入验证
  struct ddsi_thread_state *const thrst = ddsi_lookup_thread_state();
  const bool writekey = action & DDS_WR_KEY_BIT;
  struct ddsi_writer *ddsi_wr = wr->m_wr;
  int ret = DDS_RETCODE_OK;

  // 检查数据是否为空
  if (data == NULL) return DDS_RETCODE_BAD_PARAMETER;

  // 2. 主题过滤
  if (!evalute_topic_filter(wr, data, writekey)) return DDS_RETCODE_OK;

  // 唤醒线程状态
  ddsi_thread_state_awake(thrst, &wr->m_entity.m_domain->gv);

#ifdef DDS_HAS_SHM
  // 根据是否使用共享内存选择不同的写入实现
  if (wr->m_iox_pub)
    ret = dds_write_impl_iox(wr, ddsi_wr, writekey, data, tstamp, action);
  else
    ret = dds_write_impl_plain(wr, ddsi_wr, writekey, data, tstamp, action);
#else
  // 使用纯数据写入实现
  ret = dds_write_impl_plain(wr, ddsi_wr, writekey, data, tstamp, action);
#endif

  // 线程进入休眠状态
  ddsi_thread_state_asleep(thrst);

  return ret;
}
/**
 * @brief 写入CDR序列化数据到dds_writer中
 *
 * @param[in] wr  dds_writer指针，用于写入数据
 * @param[in] xp  ddsi_xpack指针，用于数据包处理
 * @param[in] dinp  ddsi_serdata指针，表示要写入的序列化数据
 * @param[in] flush  布尔值，表示是否立即刷新缓冲区
 * @return 返回dds_return_t类型，表示操作结果
 */
dds_return_t dds_writecdr_impl(dds_writer *wr,
                               struct ddsi_xpack *xp,
                               struct ddsi_serdata *dinp,
                               bool flush) {
  // 调用通用实现函数，并传入参数
  return dds_writecdr_impl_common(wr->m_wr, xp, (struct ddsi_serdata_any *)dinp, flush, wr);
}

/**
 * @brief 刷新dds_writer的缓冲区
 *
 * @param[in] writer  dds_entity_t类型，表示要刷新的实体
 */
void dds_write_flush(dds_entity_t writer) {
  dds_writer *wr;
  // 尝试锁定writer
  if (dds_writer_lock(writer, &wr) == DDS_RETCODE_OK) {
    // 查找线程状态
    struct ddsi_thread_state *const thrst = ddsi_lookup_thread_state();
    // 唤醒线程状态
    ddsi_thread_state_awake(thrst, &wr->m_entity.m_domain->gv);
    // 发送数据包
    ddsi_xpack_send(wr->m_xp, true);
    // 线程进入休眠状态
    ddsi_thread_state_asleep(thrst);
    // 解锁writer
    dds_writer_unlock(wr);
  }
}
/**
 * @brief 将序列化数据写入本地孤立的 writer 实现
 *
 * @param[in] lowr 本地孤立的 writer 结构体指针
 * @param[in] d 序列化数据结构体指针
 * @return dds_return_t 返回操作结果状态码
 */
dds_return_t dds_writecdr_local_orphan_impl(struct ddsi_local_orphan_writer *lowr,
                                            struct ddsi_serdata *d) {
  // 这个函数永远不会在网络上发送，xp 只对网络相关
#ifdef DDS_HAS_SHM
  assert(d->iox_chunk == NULL);
#endif

  // 在所有路径中消耗来自 din 的 1 refc（奇怪，但...历史原因...）
  // 让 refc(din) 为 r，所以返回时必须是 r-1
  struct ddsi_thread_state *const thrst = ddsi_lookup_thread_state();
  int ret = DDS_RETCODE_OK;
  assert(lowr->wr.type == d->type);

  // d = din: refc(d) = r, 否则 refc(d) = 1

  ddsi_thread_state_awake(thrst, lowr->wr.e.gv);
  struct ddsi_tkmap_instance *const tk = ddsi_tkmap_lookup_instance_ref(lowr->wr.e.gv->m_tkmap, d);
  deliver_locally(&lowr->wr, d, tk);
  ddsi_tkmap_instance_unref(lowr->wr.e.gv->m_tkmap, tk);
  ddsi_serdata_unref(d);  // d = din: refc(d) = r - 1
  ddsi_thread_state_asleep(thrst);
  return ret;
}
