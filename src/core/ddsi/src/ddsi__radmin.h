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
#ifndef DDSI__RADMIN_H
#define DDSI__RADMIN_H

#include <stddef.h>

#include "dds/ddsi/ddsi_locator.h"
#include "dds/ddsi/ddsi_protocol.h"
#include "dds/ddsi/ddsi_radmin.h"
#include "dds/ddsrt/align.h"
#include "dds/ddsrt/atomics.h"
#include "dds/ddsrt/static_assert.h"
#include "dds/ddsrt/threads.h"
#include "dds/ddsrt/time.h"

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @struct ddsi_rbufpool
 * @brief 缓冲区池(Buffer pool)结构体 (Buffer pool structure)
 */
struct ddsi_rbufpool;

/**
 * @struct ddsi_rbuf
 * @brief 接收缓冲区(Receive buffer)结构体 (Receive buffer structure)
 */
struct ddsi_rbuf;

/**
 * @struct ddsi_rmsg
 * @brief 接收消息(Receive message)结构体 (Receive message structure)
 */
struct ddsi_rmsg;

/**
 * @struct ddsi_rdata
 * @brief 接收数据(Receive data)结构体 (Receive data structure)
 */
struct ddsi_rdata;

/**
 * @struct ddsi_rsample
 * @brief 接收样本(Receive sample)结构体 (Receive sample structure)
 */
struct ddsi_rsample;

/**
 * @struct ddsi_rsample_chain
 * @brief 接收样本链(Receive sample chain)结构体 (Receive sample chain structure)
 */
struct ddsi_rsample_chain;

/**
 * @struct ddsi_rsample_info
 * @brief 接收样本信息(Receive sample information)结构体 (Receive sample information structure)
 */
struct ddsi_rsample_info;

/**
 * @struct ddsi_defrag
 * @brief 数据分片重组(Data defragmentation)结构体 (Data defragmentation structure)
 */
struct ddsi_defrag;

/**
 * @struct ddsi_reorder
 * @brief 数据重排序(Data reordering)结构体 (Data reordering structure)
 */
struct ddsi_reorder;

/**
 * @struct ddsi_dqueue
 * @brief 双向队列(Double-ended queue)结构体 (Double-ended queue structure)
 */
struct ddsi_dqueue;

/**
 * @struct ddsi_guid
 * @brief 全局唯一标识符(Globally Unique Identifier)结构体 (Globally Unique Identifier structure)
 */
struct ddsi_guid;

/**
 * @struct ddsi_tran_conn
 * @brief 传输连接(Transport connection)结构体 (Transport connection structure)
 */
struct ddsi_tran_conn;

/**
 * @struct ddsi_proxy_writer
 * @brief 代理写入器(Proxy writer)结构体 (Proxy writer structure)
 */
struct ddsi_proxy_writer;

/**
 * @struct ddsrt_log_cfg
 * @brief 日志配置(Log configuration)结构体 (Log configuration structure)
 */
struct ddsrt_log_cfg;

/**
 * @struct ddsi_fragment_number_set_header
 * @brief 分片序号集合头(Fragment number set header)结构体 (Fragment number set header structure)
 */
struct ddsi_fragment_number_set_header;

/**
 * @struct ddsi_sequence_number_set_header
 * @brief 序列号集合头(Sequence number set header)结构体 (Sequence number set header structure)
 */
struct ddsi_sequence_number_set_header;

/**
 * @brief Allocated inside a chunk of memory by a custom allocator and requires >= 8-byte alignment.
 */
#define DDSI_ALIGNOF_RMSG (dds_alignof(struct ddsi_rmsg) > 8 ? dds_alignof(struct ddsi_rmsg) : 8)

/**
 * @brief 定义处理函数类型，用于处理接收到的数据样本。
 * @param sampleinfo 数据样本信息
 * @param fragchain 数据片段链表
 * @param rdguid 读者GUID
 * @param qarg 队列参数
 * @return 成功时返回0，失败时返回非0值。
 *
 * @brief Define the handler function type for processing received data samples.
 * @param sampleinfo Data sample information
 * @param fragchain Fragment chain of the data
 * @param rdguid Reader GUID
 * @param qarg Queue argument
 * @return Returns 0 on success, non-zero value on failure.
 */
typedef int (*ddsi_dqueue_handler_t)(const struct ddsi_rsample_info* sampleinfo,
                                     const struct ddsi_rdata* fragchain,
                                     const struct ddsi_guid* rdguid,
                                     void* qarg);

/**
 * @brief 接收器状态结构体，包含了接收器的各种状态信息。
 *
 * @brief Receiver state structure, containing various state information of the receiver.
 */
struct ddsi_receiver_state {
  ddsi_guid_prefix_t src_guid_prefix;   ///< 源GUID前缀 (Source GUID prefix)
  ddsi_guid_prefix_t dst_guid_prefix;   ///< 目标GUID前缀 (Destination GUID prefix)
  struct ddsi_addrset* reply_locators;  ///< 回复定位器集合 (Reply locators set)
  uint32_t forme : 1;  ///< 是否为自己发送的消息 (Whether the message is sent by itself)
  uint32_t rtps_encoded : 1;                 ///< RTPS编码标志 (RTPS encoding flag)
  ddsi_vendorid_t vendor;                    ///< 供应商ID (Vendor ID)
  ddsi_protocol_version_t protocol_version;  ///< 协议版本 (Protocol version)
  struct ddsi_tran_conn* conn;               ///< 请求连接 (Connection for request)
  ddsi_locator_t srcloc;                     ///< 源定位器 (Source locator)
  struct ddsi_domaingv* gv;                  ///< 域全局变量 (Domain global variables)
};

/**
 * @brief 接收到的数据样本信息结构体。
 *
 * @brief Structure of received data sample information.
 */
struct ddsi_rsample_info {
  ddsi_seqno_t seq;                 ///< 序列号 (Sequence number)
  struct ddsi_receiver_state* rst;  ///< 接收器状态 (Receiver state)
  struct ddsi_proxy_writer* pwr;    ///< 代理写入器 (Proxy writer)
  uint32_t size;                    ///< 数据大小 (Data size)
  uint32_t fragsize;                ///< 片段大小 (Fragment size)
  ddsrt_wctime_t timestamp;         ///< 时间戳 (Timestamp)
  ddsrt_wctime_t
      reception_timestamp;  ///< 接收时间戳（OpenSplice扩展，但我们基本上可以免费获得，所以为什么不用呢？）(Reception
                            ///< timestamp - OpenSplice extension, but we get it
                            ///< essentially for free, so why not?)
  unsigned
      statusinfo : 2;  ///< 状态信息的两个已定义位 (Just the two defined bits from the status info)
  unsigned bswap : 1;  ///< 字节交换标志，以便更快地提取格式良好的写入器信息 (Byte swap flag, so we
                       ///< can extract well-formatted writer info quicker)
  unsigned complex_qos : 1;  ///< 包括除keyhash、2-bit状态信息和PT写入器信息之外的QoS (Includes QoS
                             ///< other than keyhash, 2-bit statusinfo, and PT writer info)
};

/** @struct ddsi_rsample_chain_elem
 *  @brief 存储接收到的数据样本链表元素结构 (Structure for storing received data sample chain
 * elements)
 */
struct ddsi_rsample_chain_elem {
  /* FIXME: evidently smaller than a defrag_iv, but maybe better to
     merge it with defrag_iv in a union anyway. */
  // 分片链表，用于存储分片数据 (Fragment chain for storing fragmented data)
  struct ddsi_rdata* fragchain;
  // 指向下一个链表元素的指针 (Pointer to the next chain element)
  struct ddsi_rsample_chain_elem* next;
  /* Gaps have sampleinfo = NULL, but nonetheless a fragchain with 1
     rdata with min=maxp1 (length 0) and valid rmsg pointer.  And (see
     DQUEUE) its lsb gets abused so we can queue "bubbles" in addition
     to data). */
  // 存储样本信息的指针，如果为NULL，则表示存在间隙 (Pointer to store sample information, if NULL,
  // it indicates a gap)
  struct ddsi_rsample_info* sampleinfo;
};

/** @struct ddsi_rsample_chain
 *  @brief 存储接收到的数据样本链表结构 (Structure for storing received data sample chain)
 */
struct ddsi_rsample_chain {
  // 链表的第一个元素 (First element of the chain)
  struct ddsi_rsample_chain_elem* first;
  // 链表的最后一个元素 (Last element of the chain)
  struct ddsi_rsample_chain_elem* last;
};

/** @enum ddsi_reorder_mode
 *  @brief 定义数据重排序模式 (Defines data reordering modes)
 */
enum ddsi_reorder_mode {
  DDSI_REORDER_MODE_NORMAL,                    // 正常重排序模式 (Normal reordering mode)
  DDSI_REORDER_MODE_MONOTONICALLY_INCREASING,  // 单调递增重排序模式 (Monotonically increasing
                                               // reordering mode)
  DDSI_REORDER_MODE_ALWAYS_DELIVER  // 总是传递重排序模式 (Always deliver reordering mode)
};

/** @enum ddsi_defrag_drop_mode
 *  @brief 定义分片丢弃模式 (Defines fragment drop modes)
 */
enum ddsi_defrag_drop_mode {
  DDSI_DEFRAG_DROP_OLDEST,  // 丢弃最旧的分片，适用于不可靠传输 (Drop oldest fragment, suitable for
                            // unreliable transport)
  DDSI_DEFRAG_DROP_LATEST  // 丢弃最新的分片，适用于可靠传输 (Drop latest fragment, suitable for
                           // reliable transport)
};

/** @typedef ddsi_reorder_result_t
 *  @brief 重排序结果类型定义 (Reordering result type definition)
 */
typedef int32_t ddsi_reorder_result_t;
/* typedef of reorder result serves as a warning that it is to be
   interpreted as follows: */
/* REORDER_DELIVER > 0 -- number of samples in sample chain */
#define DDSI_REORDER_ACCEPT 0   /* accepted/stored (for gap: also adjusted next_expected) */
#define DDSI_REORDER_TOO_OLD -1 /* discarded because it was too old */
#define DDSI_REORDER_REJECT \
  -2 /* caller may reuse memory ("real" reject for data, "fake" for gap) */

/** @typedef ddsi_dqueue_callback_t
 *  @brief 定义回调函数类型 (Defines callback function type)
 */
typedef void (*ddsi_dqueue_callback_t)(void* arg);

/** @enum ddsi_defrag_nackmap_result
 *  @brief 定义分片NACK映射结果 (Defines fragment NACK map results)
 */
enum ddsi_defrag_nackmap_result {
  DDSI_DEFRAG_NACKMAP_UNKNOWN_SAMPLE,                  // 未知样本 (Unknown sample)
  DDSI_DEFRAG_NACKMAP_ALL_ADVERTISED_FRAGMENTS_KNOWN,  // 已知所有广告分片 (All advertised fragments
                                                       // known)
  DDSI_DEFRAG_NACKMAP_FRAGMENTS_MISSING  // 缺少分片 (Fragments missing)
};

/** @component receive_buffers */
/**
 * @brief 创建一个新的接收缓冲区池 (Create a new receive buffer pool)
 *
 * @param[in] logcfg 日志配置 (Log configuration)
 * @param[in] rbuf_size 接收缓冲区大小 (Receive buffer size)
 * @param[in] max_rmsg_size 最大接收消息大小 (Maximum receive message size)
 * @return 返回一个新的接收缓冲区池指针 (Returns a pointer to a new receive buffer pool)
 */
struct ddsi_rbufpool* ddsi_rbufpool_new(const struct ddsrt_log_cfg* logcfg,
                                        uint32_t rbuf_size,
                                        uint32_t max_rmsg_size);

/** @component receive_buffers */
/**
 * @brief 设置接收缓冲区池的所有者线程 (Set the owner thread of the receive buffer pool)
 *
 * @param[in,out] rbp 接收缓冲区池指针 (Pointer to the receive buffer pool)
 * @param[in] tid 线程ID (Thread ID)
 */
void ddsi_rbufpool_setowner(struct ddsi_rbufpool* rbp, ddsrt_thread_t tid);

/** @component receive_buffers */
/**
 * @brief 释放接收缓冲区池 (Free the receive buffer pool)
 *
 * @param[in] rbp 接收缓冲区池指针 (Pointer to the receive buffer pool)
 */
void ddsi_rbufpool_free(struct ddsi_rbufpool* rbp);

/** @component receive_buffers */
/**
 * @brief 创建一个新的接收消息 (Create a new receive message)
 *
 * @param[in] rbufpool 接收缓冲区池指针 (Pointer to the receive buffer pool)
 * @return 返回一个新的接收消息指针 (Returns a pointer to a new receive message)
 */
struct ddsi_rmsg* ddsi_rmsg_new(struct ddsi_rbufpool* rbufpool);

/** @component receive_buffers */
/**
 * @brief 设置接收消息的大小 (Set the size of the receive message)
 *
 * @param[in,out] rmsg 接收消息指针 (Pointer to the receive message)
 * @param[in] size 消息大小 (Message size)
 */
void ddsi_rmsg_setsize(struct ddsi_rmsg* rmsg, uint32_t size);

/** @component receive_buffers */
/**
 * @brief 提交接收消息 (Commit the receive message)
 *
 * @param[in] rmsg 接收消息指针 (Pointer to the receive message)
 */
void ddsi_rmsg_commit(struct ddsi_rmsg* rmsg);

/** @component receive_buffers */
/**
 * @brief 释放接收消息 (Free the receive message)
 *
 * @param[in] rmsg 接收消息指针 (Pointer to the receive message)
 */
void ddsi_rmsg_free(struct ddsi_rmsg* rmsg);

/** @component receive_buffers */
/**
 * @brief 为接收消息分配内存 (Allocate memory for the receive message)
 *
 * @param[in] rmsg 接收消息指针 (Pointer to the receive message)
 * @param[in] size 分配的内存大小 (Size of the allocated memory)
 * @return 返回分配的内存指针 (Returns a pointer to the allocated memory)
 */
void* ddsi_rmsg_alloc(struct ddsi_rmsg* rmsg, uint32_t size);

/** @component receive_buffers */
/**
 * @brief 创建一个新的接收数据 (Create a new receive data)
 *
 * @param[in] rmsg 接收消息指针 (Pointer to the receive message)
 * @param[in] start 开始位置 (Start position)
 * @param[in] endp1 结束位置+1 (End position + 1)
 * @param[in] submsg_offset 子消息偏移量 (Submessage offset)
 * @param[in] payload_offset 有效负载偏移量 (Payload offset)
 * @param[in] keyhash_offset 密钥哈希偏移量 (Key hash offset)
 * @return 返回一个新的接收数据指针 (Returns a pointer to a new receive data)
 */
struct ddsi_rdata* ddsi_rdata_new(struct ddsi_rmsg* rmsg,
                                  uint32_t start,
                                  uint32_t endp1,
                                  uint32_t submsg_offset,
                                  uint32_t payload_offset,
                                  uint32_t keyhash_offset);

/** @component receive_buffers */
/**
 * @brief 创建一个新的ddsi_rdata结构体，表示数据中的一个gap
 * Create a new ddsi_rdata structure representing a gap in the data
 *
 * @param rmsg 指向ddsi_rmsg结构体的指针
 * @return 返回一个指向新创建的ddsi_rdata结构体的指针
 */
struct ddsi_rdata* ddsi_rdata_newgap(struct ddsi_rmsg* rmsg);

/** @component receive_buffers */
/**
 * @brief 调整ddsi_rdata结构体中的引用计数
 * Adjust the reference count in the ddsi_rdata structure
 *
 * @param frag 指向ddsi_rdata结构体的指针
 * @param adjust 要调整的引用计数值
 */
void ddsi_fragchain_adjust_refcount(struct ddsi_rdata* frag, int adjust);

/** @component receive_buffers */
/**
 * @brief 减少ddsi_rdata结构体的引用计数，并在引用计数为0时释放内存
 * Decrease the reference count of the ddsi_rdata structure and free memory when the reference count
 * reaches 0
 *
 * @param frag 指向ddsi_rdata结构体的指针
 */
void ddsi_fragchain_unref(struct ddsi_rdata* frag);

/** @component receive_buffers */
/**
 * @brief 创建一个新的ddsi_defrag结构体
 * Create a new ddsi_defrag structure
 *
 * @param logcfg 日志配置
 * @param drop_mode 数据丢弃模式
 * @param max_samples 最大样本数
 * @return 返回一个指向新创建的ddsi_defrag结构体的指针
 */
struct ddsi_defrag* ddsi_defrag_new(const struct ddsrt_log_cfg* logcfg,
                                    enum ddsi_defrag_drop_mode drop_mode,
                                    uint32_t max_samples);

/** @component receive_buffers */
/**
 * @brief 释放ddsi_defrag结构体的内存
 * Free the memory of the ddsi_defrag structure
 *
 * @param defrag 指向ddsi_defrag结构体的指针
 */
void ddsi_defrag_free(struct ddsi_defrag* defrag);

/** @component receive_buffers */
/**
 * @brief 对ddsi_defrag结构体进行重组操作，返回一个ddsi_rsample结构体
 * Perform reassembly on the ddsi_defrag structure and return a ddsi_rsample structure
 *
 * @param defrag 指向ddsi_defrag结构体的指针
 * @param rdata 指向ddsi_rdata结构体的指针
 * @param sampleinfo 指向ddsi_rsample_info结构体的指针
 * @return 返回一个指向新创建的ddsi_rsample结构体的指针
 */
struct ddsi_rsample* ddsi_defrag_rsample(struct ddsi_defrag* defrag,
                                         struct ddsi_rdata* rdata,
                                         const struct ddsi_rsample_info* sampleinfo);

/** @component receive_buffers */
/**
 * @brief 在ddsi_defrag结构体中记录一个gap
 * Record a gap in the ddsi_defrag structure
 *
 * @param defrag 指向ddsi_defrag结构体的指针
 * @param min gap的最小序列号
 * @param maxp1 gap的最大序列号加1
 */
void ddsi_defrag_notegap(struct ddsi_defrag* defrag, ddsi_seqno_t min, ddsi_seqno_t maxp1);

/** @component receive_buffers */
/**
 * @brief 在ddsi_defrag结构体中生成一个nackmap
 * Generate a nackmap in the ddsi_defrag structure
 *
 * @param defrag 指向ddsi_defrag结构体的指针
 * @param seq 序列号
 * @param maxfragnum 最大分片数
 * @param map 指向ddsi_fragment_number_set_header结构体的指针
 * @param mapbits 指向存储map位的数组的指针
 * @param maxsz 最大大小
 * @return 返回一个表示nackmap结果的枚举值
 */
enum ddsi_defrag_nackmap_result ddsi_defrag_nackmap(struct ddsi_defrag* defrag,
                                                    ddsi_seqno_t seq,
                                                    uint32_t maxfragnum,
                                                    struct ddsi_fragment_number_set_header* map,
                                                    uint32_t* mapbits,
                                                    uint32_t maxsz);

/** @component receive_buffers */
/**
 * @brief 修剪ddsi_defrag结构体，删除过时的数据
 * Prune the ddsi_defrag structure, removing outdated data
 *
 * @param defrag 指向ddsi_defrag结构体的指针
 * @param dst 目标GUID前缀
 * @param min 最小序列号
 */
void ddsi_defrag_prune(struct ddsi_defrag* defrag, ddsi_guid_prefix_t* dst, ddsi_seqno_t min);

/** @component receive_buffers */
/**
 * @brief 创建一个新的ddsi_reorder结构体
 * Create a new ddsi_reorder structure
 *
 * @param logcfg 日志配置
 * @param mode 重排序模式
 * @param max_samples 最大样本数
 * @param late_ack_mode 是否启用延迟确认模式
 * @return 返回一个指向新创建的ddsi_reorder结构体的指针
 */
struct ddsi_reorder* ddsi_reorder_new(const struct ddsrt_log_cfg* logcfg,
                                      enum ddsi_reorder_mode mode,
                                      uint32_t max_samples,
                                      bool late_ack_mode);

/** @component receive_buffers */
/**
 * @brief 释放ddsi_reorder结构体的内存
 * Free the memory of the ddsi_reorder structure
 *
 * @param r 指向ddsi_reorder结构体的指针
 */
void ddsi_reorder_free(struct ddsi_reorder* r);

/** @component receive_buffers */
// 复制第一个接收到的样本 (Duplicate the first received sample)
struct ddsi_rsample* ddsi_reorder_rsample_dup_first(struct ddsi_rmsg* rmsg,
                                                    struct ddsi_rsample* rsampleiv);

/** @component receive_buffers */
// 获取接收到的样本的分片链表 (Get the fragment chain of the received sample)
struct ddsi_rdata* ddsi_rsample_fragchain(struct ddsi_rsample* rsample);

/** @component receive_buffers */
// 对接收到的样本进行重排序 (Reorder the received sample)
ddsi_reorder_result_t ddsi_reorder_rsample(struct ddsi_rsample_chain* sc,
                                           struct ddsi_reorder* reorder,
                                           struct ddsi_rsample* rsampleiv,
                                           int* refcount_adjust,
                                           int delivery_queue_full_p);

/** @component receive_buffers */
// 处理接收到的数据中的间隙 (Handle gaps in the received data)
ddsi_reorder_result_t ddsi_reorder_gap(struct ddsi_rsample_chain* sc,
                                       struct ddsi_reorder* reorder,
                                       struct ddsi_rdata* rdata,
                                       ddsi_seqno_t min,
                                       ddsi_seqno_t maxp1,
                                       int* refcount_adjust);

/** @component receive_buffers */
// 丢弃指定序列号范围内的数据 (Drop data within a specified sequence number range)
void ddsi_reorder_drop_upto(struct ddsi_reorder* reorder,
                            ddsi_seqno_t maxp1);  // drops [1,maxp1); next_seq' = maxp1

/** @component receive_buffers */
// 判断是否需要接收指定序列号的样本 (Determine if a sample with the specified sequence number is
// needed)
int ddsi_reorder_wantsample(const struct ddsi_reorder* reorder, ddsi_seqno_t seq);

/** @component receive_buffers */
// 生成 NACK 地图 (Generate NACK map)
unsigned ddsi_reorder_nackmap(const struct ddsi_reorder* reorder,
                              ddsi_seqno_t base,
                              ddsi_seqno_t maxseq,
                              struct ddsi_sequence_number_set_header* map,
                              uint32_t* mapbits,
                              uint32_t maxsz,
                              int notail);

/** @component receive_buffers */
// 获取下一个期望的序列号 (Get the next expected sequence number)
ddsi_seqno_t ddsi_reorder_next_seq(const struct ddsi_reorder* reorder);

/** @component receive_buffers */
// 设置下一个期望的序列号 (Set the next expected sequence number)
void ddsi_reorder_set_next_seq(struct ddsi_reorder* reorder, ddsi_seqno_t seq);

/** @component receive_buffers */
// 创建新的数据队列 (Create a new data queue)
struct ddsi_dqueue* ddsi_dqueue_new(const char* name,
                                    const struct ddsi_domaingv* gv,
                                    uint32_t max_samples,
                                    ddsi_dqueue_handler_t handler,
                                    void* arg);

/** @component receive_buffers */

/**
 * @brief 启动接收缓冲区队列 (Start the receive buffer queue)
 * @param[in] q 指向 ddsi_dqueue 结构的指针 (Pointer to the ddsi_dqueue structure)
 * @return 成功返回 true，失败返回 false (Returns true on success, false on failure)
 */
bool ddsi_dqueue_start(struct ddsi_dqueue* q);

/** @component receive_buffers */

/**
 * @brief 释放接收缓冲区队列 (Free the receive buffer queue)
 * @param[in] q 指向 ddsi_dqueue 结构的指针 (Pointer to the ddsi_dqueue structure)
 */
void ddsi_dqueue_free(struct ddsi_dqueue* q);

/** @component receive_buffers */

/**
 * @brief 将采样链入队并延迟唤醒 (Enqueue the sample chain and defer wakeup)
 * @param[in] q 指向 ddsi_dqueue 结构的指针 (Pointer to the ddsi_dqueue structure)
 * @param[in] sc 指向 ddsi_rsample_chain 结构的指针 (Pointer to the ddsi_rsample_chain structure)
 * @param[in] rres 接收到的数据包重排序结果 (Reorder result of received packets)
 * @return 成功返回 true，失败返回 false (Returns true on success, false on failure)
 */
bool ddsi_dqueue_enqueue_deferred_wakeup(struct ddsi_dqueue* q,
                                         struct ddsi_rsample_chain* sc,
                                         ddsi_reorder_result_t rres);

/** @component receive_buffers */

/**
 * @brief 将触发器入队 (Enqueue the trigger)
 * @param[in] q 指向 ddsi_dqueue 结构的指针 (Pointer to the ddsi_dqueue structure)
 */
void ddsi_dqueue_enqueue_trigger(struct ddsi_dqueue* q);

/** @component receive_buffers */

/**
 * @brief 将采样链入队 (Enqueue the sample chain)
 * @param[in] q 指向 ddsi_dqueue 结构的指针 (Pointer to the ddsi_dqueue structure)
 * @param[in] sc 指向 ddsi_rsample_chain 结构的指针 (Pointer to the ddsi_rsample_chain structure)
 * @param[in] rres 接收到的数据包重排序结果 (Reorder result of received packets)
 */
void ddsi_dqueue_enqueue(struct ddsi_dqueue* q,
                         struct ddsi_rsample_chain* sc,
                         ddsi_reorder_result_t rres);

/** @component receive_buffers */

/**
 * @brief 将单个采样链入队 (Enqueue a single sample chain)
 * @param[in] q 指向 ddsi_dqueue 结构的指针 (Pointer to the ddsi_dqueue structure)
 * @param[in] rdguid 读者 GUID (Reader GUID)
 * @param[in] sc 指向 ddsi_rsample_chain 结构的指针 (Pointer to the ddsi_rsample_chain structure)
 * @param[in] rres 接收到的数据包重排序结果 (Reorder result of received packets)
 */
void ddsi_dqueue_enqueue1(struct ddsi_dqueue* q,
                          const ddsi_guid_t* rdguid,
                          struct ddsi_rsample_chain* sc,
                          ddsi_reorder_result_t rres);

/** @component receive_buffers */

/**
 * @brief 将回调函数入队 (Enqueue the callback function)
 * @param[in] q 指向 ddsi_dqueue 结构的指针 (Pointer to the ddsi_dqueue structure)
 * @param[in] cb 回调函数 (Callback function)
 * @param[in] arg 回调函数参数 (Argument for the callback function)
 */
void ddsi_dqueue_enqueue_callback(struct ddsi_dqueue* q, ddsi_dqueue_callback_t cb, void* arg);

/** @component receive_buffers */

/**
 * @brief 检查接收缓冲区队列是否已满 (Check if the receive buffer queue is full)
 * @param[in] q 指向 ddsi_dqueue 结构的指针 (Pointer to the ddsi_dqueue structure)
 * @return 队列已满返回非零值，否则返回零 (Returns non-zero if the queue is full, zero otherwise)
 */
int ddsi_dqueue_is_full(struct ddsi_dqueue* q);

/** @component receive_buffers */

/**
 * @brief 如果接收缓冲区队列已满，则等待直到为空 (Wait until empty if the receive buffer queue is
 * full)
 * @param[in] q 指向 ddsi_dqueue 结构的指针 (Pointer to the ddsi_dqueue structure)
 */
void ddsi_dqueue_wait_until_empty_if_full(struct ddsi_dqueue* q);

/** @component receive_buffers */

/**
 * @brief 获取碎片处理统计信息 (Get defragmentation statistics)
 * @param[in] defrag 指向 ddsi_defrag 结构的指针 (Pointer to the ddsi_defrag structure)
 * @param[out] discarded_bytes 丢弃的字节数 (Number of discarded bytes)
 */
void ddsi_defrag_stats(struct ddsi_defrag* defrag, uint64_t* discarded_bytes);

/** @component receive_buffers */

/**
 * @brief 获取重排序统计信息 (Get reordering statistics)
 * @param[in] reorder 指向 ddsi_reorder 结构的指针 (Pointer to the ddsi_reorder structure)
 * @param[out] discarded_bytes 丢弃的字节数 (Number of discarded bytes)
 */
void ddsi_reorder_stats(struct ddsi_reorder* reorder, uint64_t* discarded_bytes);

#if defined(__cplusplus)
}
#endif

#endif /* DDSI__RADMIN_H */
