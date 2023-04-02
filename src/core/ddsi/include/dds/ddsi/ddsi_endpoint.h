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
#ifndef DDSI_ENDPOINT_H
#define DDSI_ENDPOINT_H

#include "dds/ddsi/ddsi_entity.h"
#include "dds/ddsi/ddsi_hbcontrol.h"
#include "dds/ddsrt/fibheap.h"
#include "dds/export.h"
#include "dds/features.h"

#if defined(__cplusplus)
extern "C" {
#endif

struct ddsi_participant;
struct ddsi_type_pair;
struct ddsi_writer_info;
struct ddsi_entity_common;
struct ddsi_endpoint_common;
struct ddsi_ldur_fhnode;
struct ddsi_entity_index;
struct dds_qos;

/* Liveliness changed is more complicated than just add/remove. Encode the event
   in ddsi_status_cb_data_t::extra and ignore ddsi_status_cb_data_t::add */
// 活跃度变化比仅添加/删除更复杂。将事件编码在ddsi_status_cb_data_t::extra中并忽略ddsi_status_cb_data_t::add
enum ddsi_liveliness_changed_data_extra {
  DDSI_LIVELINESS_CHANGED_ADD_ALIVE,         // 添加活跃的实体 (Add an alive entity)
  DDSI_LIVELINESS_CHANGED_ADD_NOT_ALIVE,     // 添加非活跃的实体 (Add a not-alive entity)
  DDSI_LIVELINESS_CHANGED_REMOVE_NOT_ALIVE,  // 移除非活跃的实体 (Remove a not-alive entity)
  DDSI_LIVELINESS_CHANGED_REMOVE_ALIVE,      // 移除活跃的实体 (Remove an alive entity)
  DDSI_LIVELINESS_CHANGED_ALIVE_TO_NOT_ALIVE,  // 将活跃的实体转为非活跃的实体 (Change an alive
                                               // entity to not-alive)
  DDSI_LIVELINESS_CHANGED_NOT_ALIVE_TO_ALIVE  // 将非活跃的实体转为活跃的实体 (Change a not-alive
                                              // entity to alive)
};

struct ddsi_endpoint_common {
  struct ddsi_participant*
      pp;  // 指向ddsi_participant结构体的指针 (Pointer to ddsi_participant structure)
  ddsi_guid_t group_guid;  // 组GUID (Group GUID)
#ifdef DDS_HAS_TYPE_DISCOVERY
  struct ddsi_type_pair* type_pair;  // 类型对指针，仅在启用类型发现时使用 (Pointer to type pair,
                                     // used only when type discovery is enabled)
#endif
};

enum ddsi_writer_state {
  WRST_OPERATIONAL, /* 正常情况 (normal situation) */
  WRST_INTERRUPT, /* 将被删除，解除throttle_writer的阻塞但不进一步操作 (will be deleted, unblock
                     throttle_writer but do not do anything further) */
  WRST_LINGERING, /* 请求删除writer但仍有未确认的数据 (writer deletion has been requested but still
                     has unack'd data) */
  WRST_DELETING /* 实际正在删除writer（从哈希表中移除）(writer is actually being deleted (removed
                   from hash table)) */
};

typedef ddsrt_atomic_uint64_t seq_xmit_t;  // 定义seq_xmit_t为原子无符号64位整数类型 (Define
                                           // seq_xmit_t as an atomic unsigned 64-bit integer type)

/**
 * @struct ddsi_writer
 * @brief DDSI Writer 结构体，用于管理数据写入和与读取器的交互。
 *
 * DDSI Writer structure, used for managing data writing and interaction with readers.
 */
struct ddsi_writer {
  struct ddsi_entity_common e;   /**< 公共实体信息。Common entity information. */
  struct ddsi_endpoint_common c; /**< 公共端点信息。Common endpoint information. */

  ddsi_status_cb_t status_cb; /**< 状态回调函数。Status callback function. */
  void* status_cb_entity; /**< 状态回调函数关联的实体。Entity associated with the status callback
                             function. */

  ddsrt_cond_t throttle_cond; /**< 用于在 throttle_writer() 或 wait_for_acks()
                                 中触发阻塞的传输线程。Used to trigger a transmit thread blocked in
                                 throttle_writer() or wait_for_acks(). */

  ddsi_seqno_t
      seq; /**< 最后一个序列号（已传输的序列号为 1 ... seq，当尚未发布时为 0）。Last sequence number
              (transmitted seqs are 1 ... seq, 0 when nothing published yet). */
  seq_xmit_t seq_xmit; /**< 实际传输的最后一个序列号。Last sequence number actually transmitted. */
  ddsi_seqno_t min_local_readers_reject_seq; /**< local_readers->last_deliv_seq 的最小值。Minimum of
                                                local_readers->last_deliv_seq. */

  ddsi_count_t hbcount; /**< 最后一个心跳序列号。Last heartbeat sequence number. */
  ddsi_count_t hbfragcount; /**< 最后一个心跳片段序列号。Last heartbeat fragment sequence number. */

  int throttling; /**< 当某个线程等待 WHC 缩小时，值为非零。Non-zero when some thread is waiting for
                     the WHC to shrink. */
  struct ddsi_hbcontrol
      hbcontrol; /**< 控制心跳计时和 piggybacking。Controls heartbeat timing, piggybacking. */

  struct dds_qos* xqos;         /**< 写入器的 QoS 设置。QoS settings of the writer. */
  enum ddsi_writer_state state; /**< 写入器的状态。State of the writer. */

  unsigned reliable : 1; /**< 如果为 1，则写入器是可靠的（即 heartbeat_xevent != NULL）。If 1,
                            writer is reliable <=> heartbeat_xevent != NULL. */
  unsigned handle_as_transient_local : 1; /**< 控制是否在 WHC 中保留数据。Controls whether data is
                                             retained in WHC. */
  unsigned force_md5_keyhash : 1; /**< 如果为 1，则当 keyhash 需要哈希时，无论大小如何都需要哈希。If
                                     1, when keyhash has to be hashed, no matter the size. */
  unsigned retransmitting : 1; /**< 如果为 1，则此写入器当前正在重新传输。If 1, this writer is
                                  currently retransmitting. */
  unsigned alive : 1; /**< 如果为 1，则写入器处于活动状态（此写入器的租约未过期）；只有在持有
                         wr->e.lock 和 wr->c.pp->e.lock 时才能修改字段。If 1, the writer is alive
                         (lease for this writer is not expired); field may be modified only when
                         holding both wr->e.lock and wr->c.pp->e.lock. */
  unsigned test_ignore_acknack : 1; /**< 如果为 1，则写入器将忽略所有到达的 ACKNACK 消息。If 1, the
                                       writer ignores all arriving ACKNACK messages. */
  unsigned test_suppress_retransmit : 1; /**< 如果为 1，则写入器不响应重新传输请求。If 1, the writer
                                            does not respond to retransmit requests. */
  unsigned test_suppress_heartbeat : 1; /**< 如果为 1，则写入器抑制所有周期性心跳。If 1, the writer
                                           suppresses all periodic heartbeats. */
  unsigned
      test_drop_outgoing_data : 1; /**< 如果为 1，则写入器丢弃传出数据，迫使读取器请求重新传输。If
                                      1, the writer drops outgoing data, forcing the readers to
                                      request a retransmit. */

#ifdef DDS_HAS_SHM
  unsigned has_iceoryx : 1; /**< 是否使用 iceoryx 共享内存。Whether using iceoryx shared memory. */
#endif

#ifdef DDS_HAS_SSM
  unsigned supports_ssm : 1; /**< 是否支持 SSM（源特定多播）。Whether supporting SSM (Source
                                Specific Multicast). */
  struct ddsi_addrset* ssm_as; /**< SSM 地址集。SSM address set. */
#endif

  uint32_t alive_vclock; /**< 虚拟时钟，计算活动/非活动状态之间的转换。Virtual clock counting
                            transitions between alive/not-alive. */
  const struct ddsi_sertype*
      type; /**< 此写入器写入的数据类型。Type of the data written by this writer. */

  struct ddsi_addrset* as; /**< 要发布到的地址集。Set of addresses to publish to. */
  struct ddsi_xevent*
      heartbeat_xevent; /**< 当存在未确认的数据时，定时事件用于“周期性”发布心跳；NULL <=>
                           不可靠。Timed event for "periodically" publishing heartbeats when unack'd
                           data present, NULL <=> unreliable. */

  struct ddsi_ldur_fhnode*
      lease_duration; /**< 保留此写入器的租约期限的 fibheap 节点，在具有无限期限的自动活跃度情况下为
                         NULL。Fibheap node to keep lease duration for this writer, NULL in case of
                         automatic liveliness with infinite duration. */

  struct ddsi_whc* whc; /**< WHC 跟踪历史记录，T-L 持久性服务历史记录 +
                           根据序列号重新传输的样本。WHC tracking history, T-L durability service
                           history + samples by sequence number for retransmit. */

  uint32_t whc_low, whc_high; /**< WHC 的字节水位线（仅计算未确认的数据）。Watermarks for WHC in
                                 bytes (counting only unack'd data). */

  ddsrt_etime_t t_rexmit_start; /**< 重新传输开始时间。Time of retransmit start. */
  ddsrt_etime_t t_rexmit_end; /**< “retransmitting”的最后一个 1->0 转换的时间。Time of last 1->0
                                 transition of "retransmitting". */
  ddsrt_etime_t t_whc_high_upd; /**< 控制吞吐量逐步提高的“whc_high”最后更新时间。Time "whc_high" was
                                   last updated for controlled ramp-up of throughput. */

  uint32_t init_burst_size_limit; /**< 派生自读取器的 receive_buffer_size。Derived from reader's
                                     receive_buffer_size. */
  uint32_t rexmit_burst_size_limit; /**< 派生自读取器的 receive_buffer_size。Derived from reader's
                                       receive_buffer_size. */

  uint32_t num_readers; /**< 匹配 PROXY 读取器的总数。Total number of matching PROXY readers. */
  uint32_t num_reliable_readers; /**< 匹配的可靠 PROXY 读取器数量。Number of matching reliable PROXY
                                    readers. */
  uint32_t
      num_readers_requesting_keyhash; /**< 请求生成 keyhash
                                         的读取器数量，对于受保护的密钥和配置覆盖也要 +1。Number of
                                         readers requesting keyhash, also +1 for protected keys and
                                         config override for generating keyhash. */

  ddsrt_avl_tree_t readers; /**< 所有匹配的 PROXY 读取器，参见 struct ddsi_wr_prd_match。All
                               matching PROXY readers, see struct ddsi_wr_prd_match. */
  ddsrt_avl_tree_t local_readers; /**< 所有匹配的本地读取器，参见 struct ddsi_wr_rd_match。All
                                     matching LOCAL readers, see struct ddsi_wr_rd_match. */

#ifdef DDS_HAS_NETWORK_PARTITIONS
  /* 网络分区配置 (Network partition configuration) */
  const struct ddsi_config_networkpartition_listelem* network_partition;
#endif
  /* 累计收到的无重传请求的ACKNACK数量 (Cumulative number of ACKNACKs received without
   * retransmission request) */
  uint32_t num_acks_received;
  /* 累计收到的需要重传的ACKNACK数量 (Cumulative number of ACKNACKs received with retransmission
   * request) */
  uint32_t num_nacks_received;
  /* 发送被节流的累计次数（whc达到高水位标记）(Cumulative times transmission was throttled due to
   * whc hitting high-level mark) */
  uint32_t throttle_count;
  /* 节流追踪计数器 (Throttle tracing counter) */
  uint32_t throttle_tracing;
  /* 累计重传样本数量（计算事件；1个样本可以计数多次）(Cumulative samples retransmitted, counting
   * events; 1 sample can be counted many times) */
  uint32_t rexmit_count;
  /* 累计丢失但请求重传的样本数量（也计算事件）(Cumulative samples lost but retransmit requested,
   * also counting events) */
  uint32_t rexmit_lost_count;
  /* 累计排队等待重传的字节数 (Cumulative bytes queued for retransmit) */
  uint64_t rexmit_bytes;
  /* 节流状态下的累计时间 (Cumulative time spent in throttled state) */
  uint64_t time_throttled;
  /* 重传状态下的累计时间 (Cumulative time spent in retransmitting state) */
  uint64_t time_retransmit;
  /* 此写入器使用的定时事件队列 (Timed event queue to be used by this writer) */
  struct ddsi_xeventq* evq;
  /* 快速通道的本地读取器数组；如果未使用快速通道，则回退到扫描local_readers (Array of LOCAL readers
   * for fast-pathing; if not fast-pathed, fall back to scanning local_readers) */
  struct ddsi_local_reader_ary rdary;
  /* 活动度管理租约（仅在使用手动活动度时，写入器才能变为非活动状态）(Lease for liveliness
   * administration, writer can only become inactive when using manual liveliness) */
  struct ddsi_lease* lease;
#ifdef DDS_HAS_SECURITY
  /* 安全属性 (Security attributes) */
  struct ddsi_writer_sec_attributes* sec_attr;
#endif
};

/**
 * @brief 本地孤立写入器结构体 (Local orphan writer structure)
 */
struct ddsi_local_orphan_writer {
  struct ddsi_writer wr; /**< 写入器 (Writer) */
};

/**
 * @brief DDSI 读取器结构体 (DDSI Reader structure)
 */
struct ddsi_reader {
  struct ddsi_entity_common e;   /**< 实体通用结构体 (Entity common structure) */
  struct ddsi_endpoint_common c; /**< 端点通用结构体 (Endpoint common structure) */
  ddsi_status_cb_t status_cb;    /**< 状态回调函数 (Status callback function) */
  void* status_cb_entity;        /**< 状态回调实体 (Status callback entity) */
  struct ddsi_rhc*
      rhc; /**< 读者历史记录，跟踪注册和数据 (Reader history, tracks registrations and data) */
  struct dds_qos* xqos;  /**< 质量服务 (Quality of Service) */
  unsigned reliable : 1; /**< 1 当且仅当读者可靠 (1 iff reader is reliable) */
  unsigned handle_as_transient_local : 1; /**< 1 当且仅当读者希望从代理写入器获取历史数据 (1 iff
                                             reader wants historical data from proxy writers) */
  unsigned request_keyhash : 1; /**< 受 sertype 控制 (Really controlled by the sertype) */
#ifdef DDS_HAS_SSM
  unsigned favours_ssm : 1; /**< 当且仅当为 1 时，此读者支持 SSM (iff 1, this reader favours SSM) */
#endif
#ifdef DDS_HAS_SHM
  unsigned has_iceoryx : 1; /**< 是否具有 Iceoryx 支持 (Whether it has Iceoryx support) */
#endif
  ddsi_count_t
      init_acknack_count; /**< 新匹配代理写入器的 "count"（即 ACK 序列号）的初始值 (Initial value
                             for "count" (i.e. ACK seq num) for newly matched proxy writers) */
#ifdef DDS_HAS_NETWORK_PARTITIONS
  struct ddsi_networkpartition_address* uc_as; /**< 单播地址 (Unicast address) */
  struct ddsi_networkpartition_address* mc_as; /**< 组播地址 (Multicast address) */
#endif
  const struct ddsi_sertype*
      type; /**< 此读者读取的数据类型 (Type of the data read by this reader) */
  uint32_t num_writers; /**< 匹配 PROXY 写入器的总数 (Total number of matching PROXY writers) */
  ddsrt_avl_tree_t writers; /**< 所有匹配的 PROXY 写入器，参见结构体 ddsi_rd_pwr_match (All matching
                               PROXY writers, see struct ddsi_rd_pwr_match) */
  ddsrt_avl_tree_t local_writers; /**< 所有匹配的 LOCAL 写入器，参见结构体 ddsi_rd_wr_match (All
                                     matching LOCAL writers, see struct ddsi_rd_wr_match) */
#ifdef DDS_HAS_SECURITY
  struct ddsi_reader_sec_attributes* sec_attr; /**< 安全属性 (Security attributes) */
#endif
};

// 导出 AVL 树定义（Export the AVL tree definitions）
DDS_EXPORT extern const ddsrt_avl_treedef_t ddsi_wr_readers_treedef;
DDS_EXPORT extern const ddsrt_avl_treedef_t ddsi_wr_local_readers_treedef;
DDS_EXPORT extern const ddsrt_avl_treedef_t ddsi_rd_writers_treedef;
DDS_EXPORT extern const ddsrt_avl_treedef_t ddsi_rd_local_writers_treedef;

/**
 * @brief 判断端点是否为内置端点（Determine if an endpoint is a built-in endpoint）
 * @param[in] id 实体ID（Entity ID）
 * @param[in] vendorid 供应商ID（Vendor ID）
 * @return 如果是内置端点，则返回非零值；否则返回0（Returns non-zero if it is a built-in endpoint, 0
 * otherwise）
 */
/** @component ddsi_endpoint */
int ddsi_is_builtin_endpoint(ddsi_entityid_t id, ddsi_vendorid_t vendorid);

// writer

/**
 * @brief 创建一个新的本地孤立写入器（Create a new local orphan writer）
 * @param[in] gv 域全局变量（Domain global variables）
 * @param[in] entityid 实体ID（Entity ID）
 * @param[in] topic_name 主题名称（Topic name）
 * @param[in] type 序列化类型（Serialization type）
 * @param[in] xqos QoS设置（QoS settings）
 * @param[in] whc 写历史缓存（Write history cache）
 * @return 新创建的本地孤立写入器指针（Pointer to the newly created local orphan writer）
 */
/** @component ddsi_endpoint */
struct ddsi_local_orphan_writer* ddsi_new_local_orphan_writer(struct ddsi_domaingv* gv,
                                                              ddsi_entityid_t entityid,
                                                              const char* topic_name,
                                                              struct ddsi_sertype* type,
                                                              const struct dds_qos* xqos,
                                                              struct ddsi_whc* whc);

/**
 * @brief 删除本地孤立写入器（Delete a local orphan writer）
 * @param[in] wr 本地孤立写入器指针（Pointer to the local orphan writer）
 */
/** @component ddsi_endpoint */
void ddsi_delete_local_orphan_writer(struct ddsi_local_orphan_writer* wr);

/**
 * @brief 创建一个新的写入器（Create a new writer）
 * @param[out] wr_out 新创建的写入器指针（Pointer to the newly created writer）
 * @param[in] wrguid 写入器GUID（Writer GUID）
 * @param[in] group_guid 组GUID（Group GUID）
 * @param[in] pp 参与者指针（Pointer to the participant）
 * @param[in] topic_name 主题名称（Topic name）
 * @param[in] type 序列化类型（Serialization type）
 * @param[in] xqos QoS设置（QoS settings）
 * @param[in] whc 写历史缓存（Write history cache）
 * @param[in] status_cb 状态回调函数（Status callback function）
 * @param[in] status_cb_arg 状态回调函数参数（Status callback function argument）
 * @return 成功返回DDS_RETCODE_OK，否则返回错误代码（Returns DDS_RETCODE_OK on success, error code
 * otherwise）
 */
/** @component ddsi_endpoint */
dds_return_t ddsi_new_writer(struct ddsi_writer** wr_out,
                             struct ddsi_guid* wrguid,
                             const struct ddsi_guid* group_guid,
                             struct ddsi_participant* pp,
                             const char* topic_name,
                             const struct ddsi_sertype* type,
                             const struct dds_qos* xqos,
                             struct ddsi_whc* whc,
                             ddsi_status_cb_t status_cb,
                             void* status_cb_arg);

/**
 * @brief 更新写入器的QoS设置（Update the QoS settings of a writer）
 * @param[in] wr 写入器指针（Pointer to the writer）
 * @param[in] xqos 新的QoS设置（New QoS settings）
 */
/** @component ddsi_endpoint */
void ddsi_update_writer_qos(struct ddsi_writer* wr, const struct dds_qos* xqos);

/**
 * @brief 生成写入器信息（Generate writer information）
 * @param[out] wrinfo 写入器信息结构体指针（Pointer to the writer info structure）
 * @param[in] e 实体通用结构体指针（Pointer to the entity common structure）
 * @param[in] xqos QoS设置（QoS settings）
 * @param[in] statusinfo 状态信息（Status information）
 */
/** @component ddsi_endpoint */
void ddsi_make_writer_info(struct ddsi_writer_info* wrinfo,
                           const struct ddsi_entity_common* e,
                           const struct dds_qos* xqos,
                           uint32_t statusinfo);

/**
 * @brief 写入器等待确认（Writer waits for acknowledgements）
 * @param[in] wr 写入器指针（Pointer to the writer）
 * @param[in] rdguid 读取器GUID（Reader GUID）
 * @param[in] abstimeout 绝对超时时间（Absolute timeout time）
 * @return 成功返回DDS_RETCODE_OK，否则返回错误代码（Returns DDS_RETCODE_OK on success, error code
 * otherwise）
 */
/** @component ddsi_endpoint */
dds_return_t ddsi_writer_wait_for_acks(struct ddsi_writer* wr,
                                       const ddsi_guid_t* rdguid,
                                       dds_time_t abstimeout);

/**
 * @brief 解锁被限流的写入器（Unblock a throttled writer）
 * @param[in] gv 域全局变量（Domain global variables）
 * @param[in] guid 写入器GUID（Writer GUID）
 * @return 成功返回DDS_RETCODE_OK，否则返回错误代码（Returns DDS_RETCODE_OK on success, error code
 * otherwise）
 */
/** @component ddsi_endpoint */
dds_return_t ddsi_unblock_throttled_writer(struct ddsi_domaingv* gv, const struct ddsi_guid* guid);

/** @component ddsi_endpoint */
/**
 * @brief 删除一个数据写入器（Delete a data writer）
 *
 * @param[in] gv       指向域全局变量的指针（Pointer to domain global variables）
 * @param[in] guid     要删除的数据写入器的GUID（GUID of the data writer to be deleted）
 * @return             成功时返回DDS_RETCODE_OK，否则返回错误代码（Returns DDS_RETCODE_OK on
 * success, otherwise returns an error code）
 */
dds_return_t ddsi_delete_writer(struct ddsi_domaingv* gv, const struct ddsi_guid* guid);

/** @component ddsi_endpoint */
/**
 * @brief 获取与给定数据写入器同步的第一个数据读取器（Get the first data reader in sync with the
 * given data writer）
 *
 * @param[in]  entity_index   实体索引（Entity index）
 * @param[in]  wrcmn          数据写入器公共实体（Data writer common entity）
 * @param[out] it             AVL迭代器（AVL iterator）
 * @return                    返回与给定数据写入器同步的第一个数据读取器（Returns the first data
 * reader in sync with the given data writer）
 */
struct ddsi_reader* ddsi_writer_first_in_sync_reader(struct ddsi_entity_index* entity_index,
                                                     struct ddsi_entity_common* wrcmn,
                                                     ddsrt_avl_iter_t* it);

/** @component ddsi_endpoint */
/**
 * @brief 获取下一个与给定数据写入器同步的数据读取器（Get the next data reader in sync with the
 * given data writer）
 *
 * @param[in]  entity_index   实体索引（Entity index）
 * @param[out] it             AVL迭代器（AVL iterator）
 * @return                    返回与给定数据写入器同步的下一个数据读取器（Returns the next data
 * reader in sync with the given data writer）
 */
struct ddsi_reader* ddsi_writer_next_in_sync_reader(struct ddsi_entity_index* entity_index,
                                                    ddsrt_avl_iter_t* it);

// reader

/** @component ddsi_endpoint */
/**
 * @brief 创建一个新的数据读取器（Create a new data reader）
 *
 * @param[out] rd_out         新创建的数据读取器的指针（Pointer to the newly created data reader）
 * @param[in]  rdguid         数据读取器的GUID（Data reader's GUID）
 * @param[in]  group_guid     组GUID（Group GUID）
 * @param[in]  pp             参与者实体（Participant entity）
 * @param[in]  topic_name     主题名称（Topic name）
 * @param[in]  type           序列化类型（Serialization type）
 * @param[in]  xqos           QoS设置（QoS settings）
 * @param[in]  rhc            资源历史缓存（Resource history cache）
 * @param[in]  status_cb      状态回调函数（Status callback function）
 * @param[in]  status_cb_arg  状态回调函数参数（Status callback function argument）
 * @return                    成功时返回DDS_RETCODE_OK，否则返回错误代码（Returns DDS_RETCODE_OK on
 * success, otherwise returns an error code）
 */
dds_return_t ddsi_new_reader(struct ddsi_reader** rd_out,
                             struct ddsi_guid* rdguid,
                             const struct ddsi_guid* group_guid,
                             struct ddsi_participant* pp,
                             const char* topic_name,
                             const struct ddsi_sertype* type,
                             const struct dds_qos* xqos,
                             struct ddsi_rhc* rhc,
                             ddsi_status_cb_t status_cb,
                             void* status_cb_arg);

/** @component ddsi_endpoint */
/**
 * @brief 更新数据读取器的QoS设置（Update the QoS settings of a data reader）
 *
 * @param[in] rd    要更新QoS设置的数据读取器（Data reader to update the QoS settings for）
 * @param[in] xqos  新的QoS设置（New QoS settings）
 */
void ddsi_update_reader_qos(struct ddsi_reader* rd, const struct dds_qos* xqos);

/** @component ddsi_endpoint */
/**
 * @brief 删除一个数据读取器（Delete a data reader）
 *
 * @param[in] gv       指向域全局变量的指针（Pointer to domain global variables）
 * @param[in] guid     要删除的数据读取器的GUID（GUID of the data reader to be deleted）
 * @return             成功时返回DDS_RETCODE_OK，否则返回错误代码（Returns DDS_RETCODE_OK on
 * success, otherwise returns an error code）
 */
dds_return_t ddsi_delete_reader(struct ddsi_domaingv* gv, const struct ddsi_guid* guid);

#if defined(__cplusplus)
}
#endif

#endif /* DDSI_ENDPOINT_H */
