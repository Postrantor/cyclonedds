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
#ifndef DDSI_PROXY_ENDPOINT_H
#define DDSI_PROXY_ENDPOINT_H

#include "dds/ddsi/ddsi_domaingv.h"
#include "dds/ddsi/ddsi_entity.h"
#include "dds/ddsi/ddsi_guid.h"
#include "dds/ddsi/ddsi_lease.h"
#include "dds/export.h"
#include "dds/features.h"

#if defined(__cplusplus)
extern "C" {
#endif

struct ddsi_proxy_participant;
struct ddsi_proxy_reader;
struct ddsi_writer;
struct dds_qos;
struct ddsi_addrset;
struct ddsi_serdata;

/**
 * @struct ddsi_proxy_endpoint_common
 * @brief 代理端点通用结构体 (Proxy endpoint common structure)
 */
struct ddsi_proxy_endpoint_common {
  struct ddsi_proxy_participant*
      proxypp; /**< 计数后向引用到代理参与者 (Counted backref to proxy participant) */
  struct ddsi_proxy_endpoint_common*
      next_ep; /**< 此代理参与者的下一个端点 (Next endpoint belonging to this proxy participant) */
  struct ddsi_proxy_endpoint_common*
      prev_ep; /**< 以任意顺序排列的上一个端点 (Previous endpoint in arbitrary ordering) */
  struct dds_qos*
      xqos; /**< 代理端点 QoS 存储在此处；FIXME: 本地端点也应将其移动到通用结构中 (Proxy endpoint
               QoS lives here; FIXME: local ones should have it moved to common as well) */
  struct ddsi_addrset*
      as; /**< 用于与此端点通信的地址集 (Address set to use for communicating with this endpoint) */
  ddsi_guid_t group_guid; /**< 如果不可用，则为 0:0:0:0 (0:0:0:0 if not available) */
  ddsi_vendorid_t vendor; /**< 从 proxypp->vendor 缓存 (Cached from proxypp->vendor) */
  ddsi_seqno_t seq; /**< 最近 SEDP 消息的序列号 (Sequence number of most recent SEDP message) */
#ifdef DDS_HAS_TYPE_DISCOVERY
  struct ddsi_type_pair* type_pair; /**< 类型对 (Type pair) */
#endif
#ifdef DDS_HAS_SECURITY
  ddsi_security_info_t security_info; /**< 安全信息 (Security info) */
#endif
};

/**
 * @struct ddsi_generic_proxy_endpoint
 * @brief 通用代理端点结构体 (Generic proxy endpoint structure)
 */
struct ddsi_generic_proxy_endpoint {
  struct ddsi_entity_common e;         /**< 实体通用结构体 (Entity common structure) */
  struct ddsi_proxy_endpoint_common c; /**< 代理端点通用结构体 (Proxy endpoint common structure) */
};

/**
 * @struct ddsi_proxy_writer
 * @brief 代理写入器结构，用于管理远程数据写入器的本地表示。
 *        Proxy writer structure, used for managing the local representation of a remote data
 * writer.
 */
struct ddsi_proxy_writer {
  struct ddsi_entity_common e;         /**< 公共实体结构。Common entity structure. */
  struct ddsi_proxy_endpoint_common c; /**< 代理端点公共结构。Proxy endpoint common structure. */

  /**
   * @brief 匹配的本地读取器列表。
   *        List of matching LOCAL readers, see pwr_rd_match.
   */
  ddsrt_avl_tree_t readers;

  int32_t n_reliable_readers; /**< 可靠读取器的数量。Number of reliable readers. */

  /**
   * @brief 需要特殊处理的读取器数量（接受历史数据，等待历史数据集完成）。
   *        Number of readers that require special handling (accepting historical data, waiting for
   * historical data set to become complete).
   */
  int32_t n_readers_out_of_sync;

  ddsi_seqno_t last_seq; /**< 写入器发布的最高已知序列号，而非最后传递的序列号。Highest known
                            sequence number published by the writer, not the last delivered. */

  /**
   * @brief last_seq 的最后已知分片，如果 last_seq 不是部分分片，则为 UINT32_MAX。
   *        Last known fragment for last_seq, or UINT32_MAX if last_seq is not partial.
   */
  uint32_t last_fragnum;

  ddsi_count_t nackfragcount; /**< 最后一个 nackfrag 序列号。Last nackfrag sequence number. */

  /**
   * @brief 将要传递的下一个序列号的低 32 位；用于生成 acks；在所有支持的平台上都是 32 位原子读取。
   *        Lower 32-bits for the next sequence number that will be delivered; for generating acks;
   * 32-bit so atomic reads on all supported platforms.
   */
  ddsrt_atomic_uint32_t next_deliv_seq_lowword;

  unsigned deliver_synchronously : 1; /**< 当值为 1
                                         时，非历史数据的传递直接从接收线程发生；否则通过传递队列
                                         "dqueue"。If 1, delivery happens straight from the receive
                                         thread for non-historical data; else through the delivery
                                         queue "dqueue". */
  unsigned
      have_seen_heartbeat : 1; /**< 当值为 1 时，我们已经从这个代理写入器接收到至少一个心跳。If 1,
                                  we have received at least one heartbeat from this proxy writer. */
  unsigned local_matching_inprogress : 1; /**< 当值为 1
                                             时，我们仍在忙于匹配本地读取器；这样我们就不会一开始就将传入的数据传递给部分而不是所有读取器。If
                                             1, we are still busy matching local readers; this is so
                                             we don't deliver incoming data to some but not all
                                             readers initially. */
  unsigned
      alive : 1; /**< 当值为 1 时，代理写入器处于活动状态（此代理写入器的租约尚未到期）；只有在持有
                    pwr->e.lock 和 pwr->c.proxypp->e.lock 时才能修改该字段。If 1, the proxy writer
                    is alive (lease for this proxy writer is not expired); field may be modified
                    only when holding both pwr->e.lock and pwr->c.proxypp->e.lock. */
  unsigned filtered : 1; /**< 当值为 1 时，内置代理写入器使用内容过滤器，这会影响心跳和间隙。If 1,
                            builtin proxy writer uses content filter, which affects heartbeats and
                            gaps. */
  unsigned redundant_networking : 1; /**< 当值为 1 时，请求在所有广告接口上接收数据。If 1, requests
                                        receiving data on all advertised interfaces. */
#ifdef DDS_HAS_SSM
  unsigned supports_ssm : 1; /**< 当值为 1 时，此代理写入器支持 SSM。If 1, this proxy writer
                                supports SSM. */
#endif
#ifdef DDS_HAS_SHM
  unsigned is_iceoryx : 1; /**< 当值为 1 时，此代理写入器是 Iceoryx 类型。If 1, this proxy writer is
                              of type Iceoryx. */
#endif
  uint32_t alive_vclock; /**< 计算活动/非活动状态之间转换的虚拟时钟。Virtual clock counting
                            transitions between alive/not-alive states. */

  /**
   * @brief 此代理写入器的分片重组器；FIXME：对于历史数据，可能不应该这样。
   *        Defragmenter for this proxy writer; FIXME: perhaps shouldn't be for historical data.
   */
  struct ddsi_defrag* defrag;

  /**
   * @brief 此代理写入器的消息重新排序，需要特殊处理的读取器可以有自己的重新排序，参见
   * pwr_rd_match。 Message reordering for this proxy writer, out-of-sync readers can have their
   * own, see pwr_rd_match.
   */
  struct ddsi_reorder* reorder;

  /**
   * @brief 异步传递的传递队列（历史数据始终异步传递）。
   *        Delivery queue for asynchronous delivery (historical data is always delivered
   * asynchronously).
   */
  struct ddsi_dqueue* dqueue;

  struct ddsi_xeventq*
      evq; /**< 用于 ACK 生成的定时事件队列。Timed event queue to be used for ACK generation. */

  /**
   * @brief 本地读取器数组，用于快速路径；如果没有快速路径，则回退到扫描 local_readers。
   *        Array of LOCAL readers for fast-pathing; if not fast-pathed, fall back to scanning
   * local_readers.
   */
  struct ddsi_local_reader_ary rdary;

  struct ddsi_lease*
      lease; /**< 与此代理写入器关联的租约。Lease associated with this proxy writer. */
};

/**
 * @brief 过滤函数类型定义 (Filter function type definition)
 *
 * @param wr 指向 ddsi_writer 结构的指针 (Pointer to a ddsi_writer structure)
 * @param prd 指向 ddsi_proxy_reader 结构的指针 (Pointer to a ddsi_proxy_reader structure)
 * @param serdata 指向 ddsi_serdata 结构的指针 (Pointer to a ddsi_serdata structure)
 * @return int 返回过滤结果，通常为 0 或 1 (Returns the filter result, usually 0 or 1)
 */
typedef int (*ddsi_filter_fn_t)(struct ddsi_writer* wr,
                                struct ddsi_proxy_reader* prd,
                                struct ddsi_serdata* serdata);

/**
 * @brief ddsi_proxy_reader 结构定义 (ddsi_proxy_reader structure definition)
 */
struct ddsi_proxy_reader {
  struct ddsi_entity_common e;         /**< 通用实体结构 (Common entity structure) */
  struct ddsi_proxy_endpoint_common c; /**< 通用代理端点结构 (Common proxy endpoint structure) */
  unsigned deleting : 1;               /**< 当删除时设置为 1 (Set to 1 when being deleted) */
  unsigned is_fict_trans_reader : 1; /**< 当确定为虚拟瞬态数据读取器时为真 (True when it is certain
                                        that it is a fictitious transient data reader) */
  unsigned requests_keyhash : 1; /**< 如果此读取器希望接收 keyhash，则为 1 (1 if this reader would
                                    like to receive keyhashes) */
  unsigned redundant_networking : 1; /**< 如果请求在所有广告接口上接收数据，则为 1 (1 if requests
                                        receiving data on all advertised interfaces) */
#ifdef DDS_HAS_SSM
  unsigned favours_ssm : 1; /**< 如果为 1，则此代理读取器在可用时倾向于使用 SSM (If 1, this proxy
                               reader favours SSM when available) */
#endif
#ifdef DDS_HAS_SHM
  unsigned is_iceoryx : 1; /**< 是否使用 iceoryx 共享内存 (Whether to use iceoryx shared memory) */
#endif
  ddsrt_avl_tree_t writers; /**< 匹配的本地写入器 (Matching LOCAL writers) */
  uint32_t receive_buffer_size; /**< 从 proxypp 继承的假定接收缓冲区大小 (Assumed receive buffer
                                   size inherited from proxypp) */
  ddsi_filter_fn_t filter; /**< 过滤函数指针 (Filter function pointer) */
};

#if defined(__cplusplus)
}
#endif

#endif /* DDSI_PROXY_ENDPOINT_H */
