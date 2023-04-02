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
#ifndef DDSI_WHC_H
#define DDSI_WHC_H

#include <stddef.h>

#include "dds/ddsrt/time.h"

#if defined(__cplusplus)
extern "C" {
#endif

struct ddsi_serdata;
struct ddsi_plist;
struct ddsi_tkmap_instance;
struct ddsi_whc;

/**
 * @brief 基本类型的 whc 节点 (Base type for whc node)
 */
struct ddsi_whc_node {
  ddsi_seqno_t seq;  ///< 序列号 (Sequence number)
};

/**
 * @brief 借用的样本结构 (Borrowed sample structure)
 */
struct ddsi_whc_borrowed_sample {
  ddsi_seqno_t seq;              ///< 序列号 (Sequence number)
  struct ddsi_serdata *serdata;  ///< 序列化数据指针 (Pointer to serialized data)
  bool unacked;                  ///< 是否未确认 (Whether it is unacknowledged)
  ddsrt_mtime_t last_rexmit_ts;  ///< 最后重传时间戳 (Last retransmit timestamp)
  unsigned rexmit_count;         ///< 重传计数 (Retransmit count)
};

/**
 * @brief WHC 状态结构 (WHC state structure)
 */
struct ddsi_whc_state {
  ddsi_seqno_t min_seq;  ///< 最小序列号，如果 WHC 为空，则为0，否则大于0 (Minimum sequence number,
                         ///< 0 if WHC empty, else > 0)
  ddsi_seqno_t max_seq;  ///< 最大序列号，如果 WHC 为空，则为0，否则大于等于 min_seq (Maximum
                         ///< sequence number, 0 if WHC empty, else >= min_seq)
  size_t unacked_bytes;  ///< 未确认字节数 (Unacknowledged bytes)
};
#define DDSI_WHCST_ISEMPTY(whcst) \
  ((whcst)->max_seq == 0)  ///< 判断 WHC 是否为空的宏 (Macro to check if WHC is empty)

/**
 * @brief 调整大小和对齐，根据需要分配堆栈上的迭代器 (Adjust SIZE and alignment stuff as needed)
 */
#define DDSI_WHC_SAMPLE_ITER_SIZE \
  (8 * sizeof(void *))  ///< WHC 样本迭代器大小 (WHC sample iterator size)
struct ddsi_whc_sample_iter_base {
  struct ddsi_whc *whc;  ///< WHC 指针 (Pointer to WHC)
};
struct ddsi_whc_sample_iter {
  struct ddsi_whc_sample_iter_base c;  ///< 基本迭代器结构 (Base iterator structure)
  union {
    char opaque[DDSI_WHC_SAMPLE_ITER_SIZE];  ///< 不透明数据 (Opaque data)
    /* cover alignment requirements: */
    uint64_t x;  ///< 用于对齐要求的 uint64_t 类型 (uint64_t type for alignment requirement)
    double y;    ///< 用于对齐要求的 double 类型 (double type for alignment requirement)
    void *p;     ///< 用于对齐要求的指针类型 (pointer type for alignment requirement)
  } opaque;
};

/** @file
 *  @brief Cyclone DDS 相关代码段 (Cyclone DDS related code snippet)
 */

/**
 * @typedef ddsi_seqno_t (*ddsi_whc_next_seq_t)(const struct ddsi_whc *whc, ddsi_seqno_t seq);
 * @brief 获取下一个序列号的函数指针类型 (Function pointer type for getting the next sequence
 * number)
 *
 * @param whc 写历史缓存指针 (Pointer to the write history cache)
 * @param seq 当前序列号 (Current sequence number)
 * @return 下一个序列号 (Next sequence number)
 */
typedef ddsi_seqno_t (*ddsi_whc_next_seq_t)(const struct ddsi_whc *whc, ddsi_seqno_t seq);

/**
 * @typedef void (*ddsi_whc_get_state_t)(const struct ddsi_whc *whc, struct ddsi_whc_state *st);
 * @brief 获取写历史缓存状态的函数指针类型 (Function pointer type for getting the write history
 * cache state)
 *
 * @param whc 写历史缓存指针 (Pointer to the write history cache)
 * @param st 存储状态信息的结构体指针 (Pointer to the structure to store the state information)
 */
typedef void (*ddsi_whc_get_state_t)(const struct ddsi_whc *whc, struct ddsi_whc_state *st);

/**
 * @typedef bool (*ddsi_whc_borrow_sample_t)(const struct ddsi_whc *whc,
 *                                           ddsi_seqno_t seq,
 *                                           struct ddsi_whc_borrowed_sample *sample);
 * @brief 借用指定序列号的样本的函数指针类型 (Function pointer type for borrowing a sample with the
 * specified sequence number)
 *
 * @param whc 写历史缓存指针 (Pointer to the write history cache)
 * @param seq 指定的序列号 (Specified sequence number)
 * @param sample 存储借用样本信息的结构体指针 (Pointer to the structure to store the borrowed sample
 * information)
 * @return 是否成功借用样本 (Whether the sample was successfully borrowed)
 */
typedef bool (*ddsi_whc_borrow_sample_t)(const struct ddsi_whc *whc,
                                         ddsi_seqno_t seq,
                                         struct ddsi_whc_borrowed_sample *sample);

/**
 * @typedef bool (*ddsi_whc_borrow_sample_key_t)(const struct ddsi_whc *whc,
 *                                               const struct ddsi_serdata *serdata_key,
 *                                               struct ddsi_whc_borrowed_sample *sample);
 * @brief 借用具有指定键的样本的函数指针类型 (Function pointer type for borrowing a sample with the
 * specified key)
 *
 * @param whc 写历史缓存指针 (Pointer to the write history cache)
 * @param serdata_key 指定的序列化数据键 (Specified serialized data key)
 * @param sample 存储借用样本信息的结构体指针 (Pointer to the structure to store the borrowed sample
 * information)
 * @return 是否成功借用样本 (Whether the sample was successfully borrowed)
 */
typedef bool (*ddsi_whc_borrow_sample_key_t)(const struct ddsi_whc *whc,
                                             const struct ddsi_serdata *serdata_key,
                                             struct ddsi_whc_borrowed_sample *sample);

/**
 * @brief ddsi_whc_return_sample_t 类型定义，用于返回样本的函数指针。
 *        (A typedef for a function pointer that returns a sample.)
 *
 * @param whc 指向 ddsi_whc 结构体的指针。(Pointer to a ddsi_whc structure.)
 * @param sample 指向 ddsi_whc_borrowed_sample 结构体的指针。(Pointer to a ddsi_whc_borrowed_sample
 * structure.)
 * @param update_retransmit_info 是否更新重传信息的布尔值。(Boolean value indicating whether to
 * update retransmission information.)
 */
typedef void (*ddsi_whc_return_sample_t)(struct ddsi_whc *whc,
                                         struct ddsi_whc_borrowed_sample *sample,
                                         bool update_retransmit_info);

/**
 * @brief ddsi_whc_sample_iter_init_t 类型定义，用于初始化样本迭代器的函数指针。
 *        (A typedef for a function pointer that initializes a sample iterator.)
 *
 * @param whc 指向 ddsi_whc 结构体的指针。(Pointer to a ddsi_whc structure.)
 * @param it 指向 ddsi_whc_sample_iter 结构体的指针。(Pointer to a ddsi_whc_sample_iter structure.)
 */
typedef void (*ddsi_whc_sample_iter_init_t)(const struct ddsi_whc *whc,
                                            struct ddsi_whc_sample_iter *it);

/**
 * @brief ddsi_whc_sample_iter_borrow_next_t 类型定义，用于借用下一个样本的函数指针。
 *        (A typedef for a function pointer that borrows the next sample.)
 *
 * @param it 指向 ddsi_whc_sample_iter 结构体的指针。(Pointer to a ddsi_whc_sample_iter structure.)
 * @param sample 指向 ddsi_whc_borrowed_sample 结构体的指针。(Pointer to a ddsi_whc_borrowed_sample
 * structure.)
 * @return 布尔值，表示是否成功借用了下一个样本。(Boolean value indicating whether the next sample
 * was successfully borrowed.)
 */
typedef bool (*ddsi_whc_sample_iter_borrow_next_t)(struct ddsi_whc_sample_iter *it,
                                                   struct ddsi_whc_borrowed_sample *sample);

/**
 * @brief whc_free_t 类型定义，用于释放 ddsi_whc 结构体的函数指针。
 *        (A typedef for a function pointer that frees a ddsi_whc structure.)
 *
 * @param whc 指向 ddsi_whc 结构体的指针。(Pointer to a ddsi_whc structure.)
 */
typedef void (*whc_free_t)(struct ddsi_whc *whc);

/**
 * @brief ddsi_whc_insert_t 类型定义，用于插入数据的函数指针。
 *        (A typedef for a function pointer that inserts data.)
 *
 * @param whc 指向 ddsi_whc 结构体的指针。(Pointer to a ddsi_whc structure.)
 * @param max_drop_seq 最大丢弃序列号。(The maximum sequence number to drop.)
 * @param seq 序列号。(The sequence number.)
 * @param exp 过期时间。(Expiration time.)
 * @param serdata 指向 ddsi_serdata 结构体的指针。(Pointer to a ddsi_serdata structure.)
 * @param tk 指向 ddsi_tkmap_instance 结构体的指针。(Pointer to a ddsi_tkmap_instance structure.)
 * @return 整数，表示插入操作的结果。(Integer value indicating the result of the insertion
 * operation.)
 */
typedef int (*ddsi_whc_insert_t)(struct ddsi_whc *whc,
                                 ddsi_seqno_t max_drop_seq,
                                 ddsi_seqno_t seq,
                                 ddsrt_mtime_t exp,
                                 struct ddsi_serdata *serdata,
                                 struct ddsi_tkmap_instance *tk);

/**
 * @brief ddsi_whc_remove_acked_messages_t 类型定义，用于移除已确认消息的函数指针。
 *        (A typedef for a function pointer that removes acknowledged messages.)
 *
 * @param whc 指向 ddsi_whc 结构体的指针。(Pointer to a ddsi_whc structure.)
 * @param max_drop_seq 最大丢弃序列号。(The maximum sequence number to drop.)
 * @param whcst 指向 ddsi_whc_state 结构体的指针。(Pointer to a ddsi_whc_state structure.)
 * @param deferred_free_list 指向 ddsi_whc_node 结构体的指针，表示延迟释放列表。(Pointer to a
 * ddsi_whc_node structure representing the deferred free list.)
 * @return 32位无符号整数，表示已移除消息的数量。(A 32-bit unsigned integer indicating the number of
 * messages removed.)
 */
typedef uint32_t (*ddsi_whc_remove_acked_messages_t)(struct ddsi_whc *whc,
                                                     ddsi_seqno_t max_drop_seq,
                                                     struct ddsi_whc_state *whcst,
                                                     struct ddsi_whc_node **deferred_free_list);

/**
 * @brief ddsi_whc_free_deferred_free_list_t 类型定义，用于释放延迟释放列表的函数指针。
 *        (A typedef for a function pointer that frees the deferred free list.)
 *
 * @param whc 指向 ddsi_whc 结构体的指针。(Pointer to a ddsi_whc structure.)
 * @param deferred_free_list 指向 ddsi_whc_node 结构体的指针，表示延迟释放列表。(Pointer to a
 * ddsi_whc_node structure representing the deferred free list.)
 */
typedef void (*ddsi_whc_free_deferred_free_list_t)(struct ddsi_whc *whc,
                                                   struct ddsi_whc_node *deferred_free_list);

/**
 * @brief ddsi_whc_ops 结构体，包含 cyclone dds 中的一系列操作函数。
 *        (ddsi_whc_ops structure, containing a series of operation functions in cyclone dds.)
 *
 * @param insert 插入操作函数。 (Insert operation function.)
 * @param remove_acked_messages 移除已确认消息的操作函数。 (Operation function to remove
 * acknowledged messages.)
 * @param free_deferred_free_list 释放延迟释放列表的操作函数。 (Operation function to free the
 * deferred free list.)
 * @param get_state 获取状态的操作函数。 (Operation function to get the state.)
 * @param next_seq 获取下一个序列号的操作函数。 (Operation function to get the next sequence
 * number.)
 * @param borrow_sample 借用样本的操作函数。 (Operation function to borrow a sample.)
 * @param borrow_sample_key 借用样本键的操作函数。 (Operation function to borrow a sample key.)
 * @param return_sample 归还样本的操作函数。 (Operation function to return a sample.)
 * @param sample_iter_init 初始化样本迭代器的操作函数。 (Operation function to initialize a sample
 * iterator.)
 * @param sample_iter_borrow_next 借用下一个样本迭代器的操作函数。 (Operation function to borrow the
 * next sample iterator.)
 * @param free 释放操作函数。 (Free operation function.)
 */
struct ddsi_whc_ops {
  ddsi_whc_insert_t insert;
  ddsi_whc_remove_acked_messages_t remove_acked_messages;
  ddsi_whc_free_deferred_free_list_t free_deferred_free_list;
  ddsi_whc_get_state_t get_state;
  ddsi_whc_next_seq_t next_seq;
  ddsi_whc_borrow_sample_t borrow_sample;
  ddsi_whc_borrow_sample_key_t borrow_sample_key;
  ddsi_whc_return_sample_t return_sample;
  ddsi_whc_sample_iter_init_t sample_iter_init;
  ddsi_whc_sample_iter_borrow_next_t sample_iter_borrow_next;
  whc_free_t free;
};

/**
 * @brief ddsi_whc 结构体，包含一个指向 ddsi_whc_ops 的指针。
 *        (ddsi_whc structure, containing a pointer to ddsi_whc_ops.)
 *
 * @param ops 指向 ddsi_whc_ops 结构体的指针。 (Pointer to the ddsi_whc_ops structure.)
 */
struct ddsi_whc {
  const struct ddsi_whc_ops *ops;
};

#if defined(__cplusplus)
}
#endif

#endif /* DDSI_WHC_H */
