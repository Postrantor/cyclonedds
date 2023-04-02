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
#include "dds/ddsi/ddsi_protocol.h"
#include "ddsi__whc.h"

/**
 * @brief 获取下一个序列号 (Get the next sequence number)
 *
 * @param[in] whc  ddsi_whc 结构的指针 (Pointer to a ddsi_whc structure)
 * @param[in] seq  当前序列号 (Current sequence number)
 * @return 下一个序列号 (The next sequence number)
 */
extern inline ddsi_seqno_t ddsi_whc_next_seq(const struct ddsi_whc* whc, ddsi_seqno_t seq);

/**
 * @brief 获取 ddsi_whc 的状态 (Get the state of ddsi_whc)
 *
 * @param[in]  whc ddsi_whc 结构的指针 (Pointer to a ddsi_whc structure)
 * @param[out] st  ddsi_whc_state 结构的指针，用于存储状态信息 (Pointer to a ddsi_whc_state structure to store the state information)
 */
extern inline void ddsi_whc_get_state(const struct ddsi_whc* whc, struct ddsi_whc_state* st);

/**
 * @brief 借用指定序列号的样本 (Borrow a sample with the specified sequence number)
 *
 * @param[in]  whc    ddsi_whc 结构的指针 (Pointer to a ddsi_whc structure)
 * @param[in]  seq    要借用的样本的序列号 (Sequence number of the sample to borrow)
 * @param[out] sample 存储借用样本信息的 ddsi_whc_borrowed_sample 结构的指针 (Pointer to a ddsi_whc_borrowed_sample structure to store the borrowed sample information)
 * @return 是否成功借用样本 (Whether the sample was successfully borrowed)
 */
extern inline bool ddsi_whc_borrow_sample(const struct ddsi_whc* whc,
                                          ddsi_seqno_t seq,
                                          struct ddsi_whc_borrowed_sample* sample);

/**
 * @brief 借用具有指定键的样本 (Borrow a sample with the specified key)
 *
 * @param[in]  whc         ddsi_whc 结构的指针 (Pointer to a ddsi_whc structure)
 * @param[in]  serdata_key 存储键信息的 ddsi_serdata 结构的指针 (Pointer to a ddsi_serdata structure storing the key information)
 * @param[out] sample      存储借用样本信息的 ddsi_whc_borrowed_sample 结构的指针 (Pointer to a ddsi_whc_borrowed_sample structure to store the borrowed sample information)
 * @return 是否成功借用样本 (Whether the sample was successfully borrowed)
 */
extern inline bool ddsi_whc_borrow_sample_key(const struct ddsi_whc* whc,
                                              const struct ddsi_serdata* serdata_key,
                                              struct ddsi_whc_borrowed_sample* sample);

/**
 * @brief 归还借用的样本 (Return the borrowed sample)
 *
 * @param[in] whc                  ddsi_whc 结构的指针 (Pointer to a ddsi_whc structure)
 * @param[in] sample               存储借用样本信息的 ddsi_whc_borrowed_sample 结构的指针 (Pointer to a ddsi_whc_borrowed_sample structure storing the borrowed sample information)
 * @param[in] update_retransmit_info 是否更新重传信息 (Whether to update retransmit information)
 */
extern inline void ddsi_whc_return_sample(struct ddsi_whc* whc,
                                          struct ddsi_whc_borrowed_sample* sample,
                                          bool update_retransmit_info);

/**
 * @brief 初始化样本迭代器 (Initialize the sample iterator)
 *
 * @param[in]  whc ddsi_whc 结构的指针 (Pointer to a ddsi_whc structure)
 * @param[out] it  ddsi_whc_sample_iter 结构的指针，用于存储迭代器信息 (Pointer to a ddsi_whc_sample_iter structure to store the iterator information)
 */
extern inline void ddsi_whc_sample_iter_init(const struct ddsi_whc* whc,
                                             struct ddsi_whc_sample_iter* it);

/**
 * @brief 借用下一个样本并使迭代器前进 (Borrow the next sample and advance the iterator)
 *
 * @param[in]  it     ddsi_whc_sample_iter 结构的指针 (Pointer to a ddsi_whc_sample_iter structure)
 * @param[out] sample 存储借用样本信息的 ddsi_whc_borrowed_sample 结构的指针 (Pointer to a ddsi_whc_borrowed_sample structure to store the borrowed sample information)
 * @return 是否成功借用下一个样本 (Whether the next sample was successfully borrowed)
 */
extern inline bool ddsi_whc_sample_iter_borrow_next(struct ddsi_whc_sample_iter* it,
                                                    struct ddsi_whc_borrowed_sample* sample);

/**
 * @brief 释放 ddsi_whc 结构 (Free the ddsi_whc structure)
 *
 * @param[in] whc ddsi_whc 结构的指针 (Pointer to a ddsi_whc structure)
 */
extern inline void ddsi_whc_free(struct ddsi_whc* whc);

/**
 * @brief 插入样本到 ddsi_whc 结构 (Insert a sample into the ddsi_whc structure)
 *
 * @param[in] whc            ddsi_whc 结构的指针 (Pointer to a ddsi_whc structure)
 * @param[in] max_drop_seq   最大丢弃序列号 (Maximum drop sequence number)
 * @param[in] seq            样本的序列号 (Sequence number of the sample)
 * @param[in] exp            样本的过期时间 (Expiration time of the sample)
 * @param[in] serdata        存储样本数据的 ddsi_serdata 结构的指针 (Pointer to a ddsi_serdata structure storing the sample data)
 * @param[in] tk             ddsi_tkmap_instance 结构的指针 (Pointer to a ddsi_tkmap_instance structure)
 * @return 插入操作的结果，0 表示成功，其他值表示失败 (Result of the insert operation, 0 means success, other values mean failure)
 */
extern int ddsi_whc_insert(struct ddsi_whc* whc,
                           ddsi_seqno_t max_drop_seq,
                           ddsi_seqno_t seq,
                           ddsrt_mtime_t exp,
                           struct ddsi_serdata* serdata,
                           struct ddsi_tkmap_instance* tk);

/**
 * @brief 移除已确认的消息 (Remove acknowledged messages)
 *
 * @param[in]  whc                ddsi_whc 结构的指针 (Pointer to a ddsi_whc structure)
 * @param[in]  max_drop_seq       最大丢弃序列号 (Maximum drop sequence number)
 * @param[out] whcst              存储 ddsi_whc 状态信息的 ddsi_whc_state 结构的指针 (Pointer to a ddsi_whc_state structure storing the ddsi_whc state information)
 * @param[out] deferred_free_list 存储延迟释放节点信息的 ddsi_whc_node 结构的指针 (Pointer to a ddsi_whc_node structure storing the deferred free node information)
 * @return 被移除的消息数量 (The number of messages removed)
 */
extern unsigned ddsi_whc_remove_acked_messages(struct ddsi_whc* whc,
                                               ddsi_seqno_t max_drop_seq,
                                               struct ddsi_whc_state* whcst,
                                               struct ddsi_whc_node** deferred_free_list);

/**
 * @brief 释放延迟释放列表 (Free the deferred free list)
 *
 * @param[in] whc               ddsi_whc 结构的指针 (Pointer to a ddsi_whc structure)
 * @param[in] deferred_free_list 存储延迟释放节点信息的 ddsi_whc_node 结构的指针 (Pointer to a ddsi_whc_node structure storing the deferred free node information)
 */
extern void ddsi_whc_free_deferred_free_list(struct ddsi_whc* whc,
                                             struct ddsi_whc_node* deferred_free_list);
