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
#include <stddef.h>
#include <string.h>

#include "dds/ddsi/ddsi_serdata.h"
#include "dds/ddsrt/avl.h"
#include "dds/ddsrt/cdtors.h"
#include "dds/ddsrt/fibheap.h"
#include "dds/ddsrt/heap.h"
#include "dds/ddsrt/hopscotch.h"
#include "dds/ddsrt/misc.h"
#include "dds/ddsrt/sync.h"
#include "dds/features.h"
#ifdef DDS_HAS_LIFESPAN
#include "dds/ddsi/ddsi_lifespan.h"
#endif
#ifdef DDS_HAS_DEADLINE_MISSED
#include "dds/ddsi/ddsi_deadline.h"
#endif
#include "dds/ddsi/ddsi_domaingv.h"
#include "dds/ddsi/ddsi_entity.h"
#include "dds/ddsi/ddsi_freelist.h"
#include "dds/ddsi/ddsi_tkmap.h"
#include "dds/ddsi/ddsi_unused.h"
#include "dds__entity.h"
#include "dds__whc.h"
#include "dds__writer.h"

#define USE_EHH 0

// 定义 dds_whc_default_node 结构体
struct dds_whc_default_node {
  struct ddsi_whc_node common;            // 公共节点结构体
  struct dds_whc_default_node *next_seq;  // 本区间内的下一个节点
  struct dds_whc_default_node *prev_seq;  // 本区间内的上一个节点
  struct whc_idxnode *idxnode;            // 索引中的节点，如果不在索引中则为 NULL
  uint32_t idxnode_pos;                   // idxnode.hist 中的索引位置
  uint64_t total_bytes;                   // 包括此节点在内的累计字节数
  size_t size;
  unsigned unacked : 1;                   // 当值为 1 时，在 whc::unacked_bytes 中计数
  unsigned borrowed : 1;                  // 任何时候最多只有一个可以借用它
  ddsrt_mtime_t last_rexmit_ts;           // 最后一次重传时间戳
  uint32_t rexmit_count;                  // 重传计数
#ifdef DDS_HAS_LIFESPAN
  struct ddsi_lifespan_fhnode lifespan;   // 生命周期的 fibheap 节点
#endif
  struct ddsi_serdata *serdata;           // 序列化数据指针
};

// 静态断言，检查 dds_whc_default_node 结构体中 common 成员的偏移量是否为 0
DDSRT_STATIC_ASSERT(offsetof(struct dds_whc_default_node, common) == 0);

// 定义 whc_intvnode 结构体
struct whc_intvnode {
  ddsrt_avl_node_t avlnode;            // AVL 树节点
  ddsi_seqno_t min;                    // 最小序列号
  ddsi_seqno_t maxp1;                  // 最大序列号加 1
  struct dds_whc_default_node *first;  // 连续序列号 [min,maxp1) 的链表中的第一个节点
  struct dds_whc_default_node *last;   // 当 first != NULL 时有效
};

// 定义 whc_idxnode 结构体
struct whc_idxnode {
  uint64_t iid;                         // 实例标识符
  ddsi_seqno_t prune_seq;               // 裁剪序列号
  struct ddsi_tkmap_instance *tk;       // tkmap 实例指针
  uint32_t headidx;                     // 头索引
#ifdef DDS_HAS_DEADLINE_MISSED
  struct deadline_elem deadline;        // 错过截止日期的列表元素
#endif
  struct dds_whc_default_node *hist[];  // 历史记录数组
};
// 如果使用 EHH（Elastic Hash Heap）
#if USE_EHH
// 定义 whc_seq_entry 结构体
struct whc_seq_entry {
  ddsi_seqno_t seq;                   // 序列号
  struct dds_whc_default_node *whcn;  // 指向 dds_whc_default_node 的指针
};
#endif

// 定义 whc_writer_info 结构体
struct whc_writer_info {
  dds_writer *writer;  // 写入器指针，可以为 NULL，例如在内置写入器的 whc 中
  unsigned is_transient_local : 1;  // 是否为瞬态本地
  unsigned has_deadline : 1;        // 是否有截止日期
  uint32_t hdepth;                  // 历史深度，0 表示无限制
  uint32_t tldepth;  // 瞬态本地深度，0 表示禁用/无限制（如果 KEEP_ALL <=> is_transient_local +
                     // tldepth=0，则无需维护索引）
  uint32_t idxdepth;  // 索引深度，等于 max(hdepth, tldepth)
};

// 定义 whc_impl 结构体
struct whc_impl {
  struct ddsi_whc common;         // 公共 whc 结构体
  ddsrt_mutex_t lock;             // 互斥锁
  uint32_t seq_size;              // 序列大小
  size_t unacked_bytes;           // 未确认的字节数
  size_t sample_overhead;         // 样本开销
  uint32_t fragment_size;         // 分片大小
  uint64_t total_bytes;           // 推入的总字节数
  unsigned xchecks : 1;           // 检查标志
  struct ddsi_domaingv *gv;       // 域全局变量指针
  struct ddsi_tkmap *tkmap;       // tkmap 指针
  struct whc_writer_info wrinfo;  // 写入器信息结构体
  ddsi_seqno_t max_drop_seq;  // 序列号 <= max_drop_seq 的样本在 whc 中表示为瞬态本地
  struct whc_intvnode *open_intv;  // 下一个样本通常会进入的区间（通常）
  struct dds_whc_default_node
      *maxseq_node;  // 如果为空，则为空；如果不在 open_intv 中，open_intv 为空
#if USE_EHH
  struct ddsrt_ehh *seq_hash;  // Elastic Hash Heap 序列哈希表
#else
  struct ddsrt_hh *seq_hash;  // 哈希表序列
#endif
  struct ddsrt_hh *idx_hash;          // 索引哈希表
  ddsrt_avl_tree_t seq;               // AVL 树序列
#ifdef DDS_HAS_LIFESPAN
  struct ddsi_lifespan_adm lifespan;  // 生命周期管理
#endif
#ifdef DDS_HAS_DEADLINE_MISSED
  struct ddsi_deadline_adm deadline;  // 错过截止日期管理
#endif
};
// 定义结构体ddsi_whc_sample_iter_impl
struct ddsi_whc_sample_iter_impl {
  struct ddsi_whc_sample_iter_base c;  // 基本迭代器类型
  bool first;                          // 是否为第一个元素的标志
};

// 静态断言，检查我们定义的whc_sample_iter是否适合调用者分配的类型大小
DDSRT_STATIC_ASSERT(sizeof(struct ddsi_whc_sample_iter_impl) <=
                    sizeof(struct ddsi_whc_sample_iter));

/*
 * Hash + interval tree adminitration of samples-by-sequence number
 * - by definition contains all samples in WHC (unchanged from older versions)
 * 序列号样本的哈希+区间树管理
 * - 根据定义，包含WHC中的所有样本（与旧版本相同）
 *
 * Circular array of samples per instance, inited to all 0
 * - length is max (durability_service.history_depth, history.depth), KEEP_ALL => as-if 0
 * - no instance index if above length 0
 * - each sample (i.e., whc_node): backpointer into index
 * - maintain index of latest sample, end of history then trivially follows from index arithmetic
 * 每个实例的循环数组样本，初始化为全0
 * - 长度是max(durability_service.history_depth, history.depth)，KEEP_ALL => 类似于0
 * - 如果长度大于0，则没有实例索引
 * - 每个样本（即whc_node）：指向索引的后指针
 * - 维护最新样本的索引，历史结束后从索引算术中轻松得出
 *
 * Overwriting in insert drops them from index, depending on "aggressiveness" from by-seq
 * - special case for no readers (i.e. no ACKs) and history > transient-local history
 * - cleaning up after ACKs has additional pruning stage for same case
 * 插入时覆盖会根据by-seq的“侵略性”将它们从索引中删除
 * - 没有读者（即没有ACK）且历史记录>瞬态本地历史记录的特殊情况
 * - 在ACK之后清理具有相同情况的额外修剪阶段
 */
// 为 whc_impl 结构体中的序列号查找对应的 dds_whc_default_node
static struct dds_whc_default_node *whc_findseq(const struct whc_impl *whc, ddsi_seqno_t seq);

// 将 whcn 插入到 whc 的哈希表中
static void insert_whcn_in_hash(struct whc_impl *whc, struct dds_whc_default_node *whcn);

// 从 whc 中删除一个 whcn 节点
static void whc_delete_one(struct whc_impl *whc, struct dds_whc_default_node *whcn);

// 比较两个序列号的大小
static int compare_seq(const void *va, const void *vb);

// 释放延迟释放列表中的所有节点
static void free_deferred_free_list(struct dds_whc_default_node *deferred_free_list);

// 获取 whc 的状态并存储在 st 结构体中
static void get_state_locked(const struct whc_impl *whc, struct ddsi_whc_state *st);

// 删除 whc 中已确认的消息，返回删除的消息数量
static uint32_t whc_default_remove_acked_messages_full(struct whc_impl *whc,
                                                       ddsi_seqno_t max_drop_seq,
                                                       struct ddsi_whc_node **deferred_free_list);

// 删除 whc 中已确认的消息，并更新 whc 状态
static uint32_t whc_default_remove_acked_messages(struct ddsi_whc *whc,
                                                  ddsi_seqno_t max_drop_seq,
                                                  struct ddsi_whc_state *whcst,
                                                  struct ddsi_whc_node **deferred_free_list);

// 释放 whc 中的延迟释放列表
static void whc_default_free_deferred_free_list(struct ddsi_whc *whc,
                                                struct ddsi_whc_node *deferred_free_list);

// 获取 whc 的状态并存储在 st 结构体中
static void whc_default_get_state(const struct ddsi_whc *whc, struct ddsi_whc_state *st);

// 将序列号、过期时间、序列化数据和 tkmap 实例插入到 whc 中
static int whc_default_insert(struct ddsi_whc *whc,
                              ddsi_seqno_t max_drop_seq,
                              ddsi_seqno_t seq,
                              ddsrt_mtime_t exp,
                              struct ddsi_serdata *serdata,
                              struct ddsi_tkmap_instance *tk);

// 获取 whc 中下一个序列号
static ddsi_seqno_t whc_default_next_seq(const struct ddsi_whc *whc, ddsi_seqno_t seq);

// 借用 whc 中指定序列号的样本
static bool whc_default_borrow_sample(const struct ddsi_whc *whc,
                                      ddsi_seqno_t seq,
                                      struct ddsi_whc_borrowed_sample *sample);

// 根据 serdata_key 借用 whc 中的样本
static bool whc_default_borrow_sample_key(const struct ddsi_whc *whc,
                                          const struct ddsi_serdata *serdata_key,
                                          struct ddsi_whc_borrowed_sample *sample);

// 归还借用的样本，并根据 update_retransmit_info 参数决定是否更新重传信息
static void whc_default_return_sample(struct ddsi_whc *whc,
                                      struct ddsi_whc_borrowed_sample *sample,
                                      bool update_retransmit_info);

// 初始化 whc 的样本迭代器
static void whc_default_sample_iter_init(const struct ddsi_whc *whc,
                                         struct ddsi_whc_sample_iter *opaque_it);

// 借用迭代器中的下一个样本
static bool whc_default_sample_iter_borrow_next(struct ddsi_whc_sample_iter *opaque_it,
                                                struct ddsi_whc_borrowed_sample *sample);

// 释放 whc 结构体
static void whc_default_free(struct ddsi_whc *whc);

// 定义 whc_seq_treedef，用于比较序列号大小
static const ddsrt_avl_treedef_t whc_seq_treedef = DDSRT_AVL_TREEDEF_INITIALIZER(
    offsetof(struct whc_intvnode, avlnode), offsetof(struct whc_intvnode, min), compare_seq, 0);

// 定义一个名为ddsi_whc_ops的结构体常量whc_ops，用于存储各种操作函数的指针
static const struct ddsi_whc_ops whc_ops = {
    .insert = whc_default_insert,                                // 插入操作的默认实现
    .remove_acked_messages = whc_default_remove_acked_messages,  // 移除已确认消息的默认实现
    .free_deferred_free_list = whc_default_free_deferred_free_list,  // 释放延迟释放列表的默认实现
    .get_state = whc_default_get_state,                              // 获取状态的默认实现
    .next_seq = whc_default_next_seq,                    // 获取下一个序列号的默认实现
    .borrow_sample = whc_default_borrow_sample,          // 借用样本的默认实现
    .borrow_sample_key = whc_default_borrow_sample_key,  // 借用样本键的默认实现
    .return_sample = whc_default_return_sample,          // 归还样本的默认实现
    .sample_iter_init = whc_default_sample_iter_init,    // 初始化样本迭代器的默认实现
    .sample_iter_borrow_next =
        whc_default_sample_iter_borrow_next,  // 借用下一个样本迭代器的默认实现
    .free = whc_default_free                  // 释放操作的默认实现
};

// 定义一个宏TRACE，用于记录WHC相关的日志信息
#define TRACE(...) DDS_CLOG(DDS_LC_WHC, &whc->gv->logconfig, __VA_ARGS__)

/*
 * 定义一个全局变量whc_count，用于记录实例化的WHC数量，
 * 以及一个名为whc_node_freelist的全局自由列表，用于存储WHC节点。
 * whc_node_freelist在第一次使用时进行初始化，并在最后一个WHC释放时自动清理。
 * 通过ddsrt_get_singleton_mutex()函数保护这些全局变量。
 *
 * 在64位机器上，whc_node结构体的大小约为100字节，因此这里的自由列表大小约为1MB。
 * 8192个条目似乎是满足最小样本、最大消息大小和短往返时间所需的大致数量。
 */
#define MAX_FREELIST_SIZE 8192
static uint32_t whc_count;                      // 实例化的WHC数量
static struct ddsi_freelist whc_node_freelist;  // WHC节点的全局自由列表

#if USE_EHH
// 计算 whc_seq_entry 结构的哈希值
static uint32_t whc_seq_entry_hash(const void *vn) {
  // 将传入的指针转换为 whc_seq_entry 结构指针
  const struct whc_seq_entry *n = vn;
  /* 我们对低 32 位进行哈希，假设在 40 亿个样本之间不会有显著的相关性 */
  const uint64_t c = UINT64_C(16292676669999574021);
  // 获取序列号的低 32 位
  const uint32_t x = (uint32_t)n->seq;
  // 计算哈希值并返回
  return (uint32_t)((x * c) >> 32);
}

// 比较两个 whc_seq_entry 结构是否相等
static int whc_seq_entry_eq(const void *va, const void *vb) {
  // 将传入的指针转换为 whc_seq_entry 结构指针
  const struct whc_seq_entry *a = va;
  const struct whc_seq_entry *b = vb;
  // 比较序列号是否相等
  return a->seq == b->seq;
}
#else
// 计算 dds_whc_default_node 结构的哈希值
static uint32_t whc_node_hash(const void *vn) {
  // 将传入的指针转换为 dds_whc_default_node 结构指针
  const struct dds_whc_default_node *n = vn;
  /* 我们对低 32 位进行哈希，假设在 40 亿个样本之间不会有显著的相关性 */
  const uint64_t c = UINT64_C(16292676669999574021);
  // 获取序列号的低 32 位
  const uint32_t x = (uint32_t)n->common.seq;
  // 计算哈希值并返回
  return (uint32_t)((x * c) >> 32);
}

// 比较两个 dds_whc_default_node 结构是否相等
static int whc_node_eq(const void *va, const void *vb) {
  // 将传入的指针转换为 dds_whc_default_node 结构指针
  const struct dds_whc_default_node *a = va;
  const struct dds_whc_default_node *b = vb;
  // 比较序列号是否相等
  return a->common.seq == b->common.seq;
}
#endif

// 计算 whc_idxnode 结构的哈希值
static uint32_t whc_idxnode_hash_key(const void *vn) {
  // 将传入的指针转换为 whc_idxnode 结构指针
  const struct whc_idxnode *n = vn;
  // 返回实例标识符作为哈希值
  return (uint32_t)n->iid;
}

// 比较两个 whc_idxnode 结构是否相等
static int whc_idxnode_eq_key(const void *va, const void *vb) {
  // 将传入的指针转换为 whc_idxnode 结构指针
  const struct whc_idxnode *a = va;
  const struct whc_idxnode *b = vb;
  // 比较实例标识符是否相等
  return (a->iid == b->iid);
}
// 比较两个序列号的大小
// 参数:
//   va: 指向第一个序列号的指针
//   vb: 指向第二个序列号的指针
// 返回值:
//   如果 *a == *b, 返回 0
//   如果 *a < *b, 返回 -1
//   否则返回 1
static int compare_seq(const void *va, const void *vb) {
  const ddsi_seqno_t *a = va;
  const ddsi_seqno_t *b = vb;
  return (*a == *b) ? 0 : (*a < *b) ? -1 : 1;
}

// 在 whc 中找到最大的序列号节点
// 参数:
//   whc: 指向 whc_impl 结构体的指针
// 返回值:
//   如果找到了最大的序列号节点，返回该节点的指针
//   否则返回 NULL
static struct dds_whc_default_node *whc_findmax_procedurally(const struct whc_impl *whc) {
  if (whc->seq_size == 0)
    return NULL;
  else if (whc->open_intv->first) {
    // last 只在 first 不为 NULL 时有效
    return whc->open_intv->last;
  } else {
    struct whc_intvnode *intv = ddsrt_avl_find_pred(&whc_seq_treedef, &whc->seq, whc->open_intv);
    assert(intv && intv->first);
    return intv->last;
  }
}

// 检查 whc 的一致性
// 参数:
//   whc: 指向 whc_impl 结构体的指针
static void check_whc(const struct whc_impl *whc) {
  // 可以检查更多内容，但是很快就会变得非常昂贵：
  // 所有节点（除了 open_intv）都是非空的、不重叠的且不连续的；
  // 区间的 min 和 maxp1 是正确的；
  // 每个区间都是连续的；
  // 所有样本都在 seq 和 seqhash 中；
  // tlidx 是 seq 的子集；
  // 序列号有序列表是正确的；等等。
  // 确保 whc->open_intv 不为 NULL
  assert(whc->open_intv != NULL);
  // 确保 whc->open_intv 是 whc->seq 中的最大元素
  assert(whc->open_intv == ddsrt_avl_find_max(&whc_seq_treedef, &whc->seq));
  // 确保 whc->open_intv 的后继元素不存在
  assert(ddsrt_avl_find_succ(&whc_seq_treedef, &whc->seq, whc->open_intv) == NULL);

  // 如果 whc->maxseq_node 存在
  if (whc->maxseq_node) {
    // 确保 whc->maxseq_node 的 next_seq 为 NULL
    assert(whc->maxseq_node->next_seq == NULL);
  }

  // 如果 whc->open_intv->first 存在
  if (whc->open_intv->first) {
    // 确保 whc->open_intv->last 存在
    assert(whc->open_intv->last);
    // 确保 whc->maxseq_node 等于 whc->open_intv->last
    assert(whc->maxseq_node == whc->open_intv->last);
    // 确保 whc->open_intv->min 小于 whc->open_intv->maxp1
    assert(whc->open_intv->min < whc->open_intv->maxp1);
    // 确保 whc->maxseq_node->common.seq + 1 等于 whc->open_intv->maxp1
    assert(whc->maxseq_node->common.seq + 1 == whc->open_intv->maxp1);
  } else {
    // 确保 whc->open_intv->min 等于 whc->open_intv->maxp1
    assert(whc->open_intv->min == whc->open_intv->maxp1);
  }

  // 确保 whc->maxseq_node 等于 whc_findmax_procedurally(whc) 的返回值
  assert(whc->maxseq_node == whc_findmax_procedurally(whc));

// 如果 NDEBUG 没有定义，执行以下代码块
#if !defined(NDEBUG)
  if (whc->xchecks) {
    struct whc_intvnode *firstintv;
    struct dds_whc_default_node *cur;
    ddsi_seqno_t prevseq = 0;

    // 查找 whc->seq 中的最小元素
    firstintv = ddsrt_avl_find_min(&whc_seq_treedef, &whc->seq);
    // 确保 firstintv 存在
    assert(firstintv);

    // 遍历 whc->seq 中的所有节点
    cur = firstintv->first;
    while (cur) {
      // 确保当前节点的序列号大于前一个节点的序列号
      assert(cur->common.seq > prevseq);
      prevseq = cur->common.seq;
      // 确保 whc_findseq(whc, cur->common.seq) 返回当前节点
      assert(whc_findseq(whc, cur->common.seq) == cur);
      // 移动到下一个节点
      cur = cur->next_seq;
    }
  }
#endif
}
// 插入 whcn 到哈希表中
// 参数:
//   whc: 指向 whc_impl 结构体的指针
//   whcn: 指向 dds_whc_default_node 结构体的指针
static void insert_whcn_in_hash(struct whc_impl *whc, struct dds_whc_default_node *whcn) {
  // 前提条件：whcn 不在哈希表中
#if USE_EHH
  // 使用 ehh 数据结构
  struct whc_seq_entry e = {.seq = whcn->common.seq, .whcn = whcn};
  // 尝试将 e 添加到哈希表中，如果失败则触发断言
  if (!ddsrt_ehh_add(whc->seq_hash, &e)) assert(0);
#else
  // 使用 hh 数据结构
  ddsrt_hh_add_absent(whc->seq_hash, whcn);
#endif
}

// 从哈希表中移除 whcn
// 参数:
//   whc: 指向 whc_impl 结构体的指针
//   whcn: 指向 dds_whc_default_node 结构体的指针
static void remove_whcn_from_hash(struct whc_impl *whc, struct dds_whc_default_node *whcn) {
  // 前提条件：whcn 在哈希表中
#if USE_EHH
  // 使用 ehh 数据结构
  struct whc_seq_entry e = {.seq = whcn->common.seq};
  // 尝试从哈希表中移除 e，如果失败则触发断言
  if (!ddsrt_ehh_remove(whc->seq_hash, &e)) assert(0);
#else
  // 使用 hh 数据结构
  ddsrt_hh_remove_present(whc->seq_hash, whcn);
#endif
}

// 在哈希表中查找具有给定序列号的节点
// 参数:
//   whc: 指向 whc_impl 结构体的指针
//   seq: 要查找的序列号
// 返回值:
//   如果找到具有给定序列号的节点，则返回指向该节点的指针；否则返回 NULL
static struct dds_whc_default_node *whc_findseq(const struct whc_impl *whc, ddsi_seqno_t seq) {
#if USE_EHH
  // 使用 ehh 数据结构
  struct whc_seq_entry e = {.seq = seq}, *r;
  // 查找具有给定序列号的节点
  if ((r = ddsrt_ehh_lookup(whc->seq_hash, &e)) != NULL)
    return r->whcn;  // 找到节点，返回指针
  else
    return NULL;     // 未找到节点，返回 NULL
#else
  // 使用 hh 数据结构
  struct dds_whc_default_node template;
  template.common.seq = seq;
  // 查找具有给定序列号的节点并返回指针（如果找到）
  return ddsrt_hh_lookup(whc->seq_hash, &template);
#endif
}
/**
 * @brief 在哈希表中查找具有给定键的节点
 *
 * @param whc 指向 whc_impl 结构体的指针
 * @param serdata_key 指向 ddsi_serdata 结构体的指针，表示要查找的键
 * @return 如果找到具有给定键的节点，则返回指向该节点的指针；否则返回 NULL
 */
static struct dds_whc_default_node *whc_findkey(const struct whc_impl *whc,
                                                const struct ddsi_serdata *serdata_key) {
  // 定义一个联合体，包含一个 whc_idxnode 结构体和一个额外的指针大小的空间
  union {
    struct whc_idxnode idxn;
    char pad[sizeof(struct whc_idxnode) + sizeof(struct dds_whc_default_node *)];
  } template;
  struct whc_idxnode *n;

  // 检查 whc 是否有效
  check_whc(whc);

  // 查找给定键对应的实例标识符（iid）
  template.idxn.iid = ddsi_tkmap_lookup(whc->tkmap, serdata_key);

  // 在索引哈希表中查找具有给定实例标识符的节点
  n = ddsrt_hh_lookup(whc->idx_hash, &template.idxn);

  // 判断是否找到节点
  if (n == NULL)
    return NULL;  // 未找到节点，返回 NULL
  else {
    // 找到节点，确保其历史记录不为空
    assert(n->hist[n->headidx]);
    // 返回找到的节点
    return n->hist[n->headidx];
  }
}

#ifdef DDS_HAS_LIFESPAN
/**
 * @brief 生命周期过期回调函数
 *
 * @param hc 指向 whc_impl 结构体的指针
 * @param tnow 当前时间
 * @return 下一个过期时间
 */
static ddsrt_mtime_t whc_sample_expired_cb(void *hc, ddsrt_mtime_t tnow) {
  struct whc_impl *whc = hc;
  void *sample;
  ddsrt_mtime_t tnext;

  // 对 whc 的锁进行加锁操作
  ddsrt_mutex_lock(&whc->lock);

  // 遍历并删除所有已过期的样本
  while ((tnext = ddsi_lifespan_next_expired_locked(&whc->lifespan, tnow, &sample)).v == 0)
    whc_delete_one(whc, sample);

  // 更新最大序列号节点
  whc->maxseq_node = whc_findmax_procedurally(whc);

  // 对 whc 的锁进行解锁操作
  ddsrt_mutex_unlock(&whc->lock);

  // 返回下一个过期时间
  return tnext;
}
#endif
#ifdef DDS_HAS_DEADLINE_MISSED
/**
 * @brief Deadline missed 回调函数
 *
 * @param hc 指向 whc_impl 结构体的指针
 * @param tnow 当前时间
 * @return 下一个过期时间
 */
static ddsrt_mtime_t whc_deadline_missed_cb(void *hc, ddsrt_mtime_t tnow) {
  struct whc_impl *whc = hc;
  void *vidxnode;
  ddsrt_mtime_t tnext;

  // 对 whc 的锁进行加锁操作
  ddsrt_mutex_lock(&whc->lock);

  // 遍历并处理所有已错过 deadline 的实例
  while ((tnext = ddsi_deadline_next_missed_locked(&whc->deadline, tnow, &vidxnode)).v == 0) {
    struct whc_idxnode *idxnode = vidxnode;
    // 计算已过期的 deadline 数量
    uint32_t deadlines_expired =
        idxnode->deadline.deadlines_missed +
        (uint32_t)((tnow.v - idxnode->deadline.t_last_update.v) / whc->deadline.dur);
    // 重新注册实例的 deadline
    ddsi_deadline_reregister_instance_locked(&whc->deadline, &idxnode->deadline, tnow);

    // 准备回调数据
    ddsi_status_cb_data_t cb_data;
    cb_data.raw_status_id = (int)DDS_OFFERED_DEADLINE_MISSED_STATUS_ID;
    cb_data.extra = deadlines_expired;
    cb_data.handle = idxnode->iid;
    cb_data.add = true;

    // 对 whc 的锁进行解锁操作
    ddsrt_mutex_unlock(&whc->lock);

    // 调用写入器状态回调函数
    dds_writer_status_cb(&whc->wrinfo.writer->m_entity, &cb_data);

    // 对 whc 的锁进行加锁操作
    ddsrt_mutex_lock(&whc->lock);

    // 更新当前时间
    tnow = ddsrt_time_monotonic();
  }

  // 对 whc 的锁进行解锁操作
  ddsrt_mutex_unlock(&whc->lock);

  // 返回下一个过期时间
  return tnext;
}
#endif
/**
 * @brief 创建一个 whc_writer_info 结构体实例
 *
 * @param wr 一个 dds_writer 指针，表示写入器
 * @param qos 一个 dds_qos_t 指针，表示质量服务参数
 * @return 返回一个 whc_writer_info 结构体指针
 */
struct whc_writer_info *dds_whc_make_wrinfo(struct dds_writer *wr, const dds_qos_t *qos) {
  // 分配内存空间给 whc_writer_info 结构体
  struct whc_writer_info *wrinfo = ddsrt_malloc(sizeof(*wrinfo));

  // 确保 qos 中包含 DDSI_QP_HISTORY 参数
  assert(qos->present & DDSI_QP_HISTORY);

  // 确保 qos 中包含 DDSI_QP_DEADLINE 参数
  assert(qos->present & DDSI_QP_DEADLINE);

  // 确保 qos 中包含 DDSI_QP_DURABILITY 参数
  assert(qos->present & DDSI_QP_DURABILITY);

  // 确保 qos 中包含 DDSI_QP_DURABILITY_SERVICE 参数
  assert(qos->present & DDSI_QP_DURABILITY_SERVICE);

  // 设置 whc_writer_info 的 writer 字段
  wrinfo->writer = wr;

  // 判断是否为 TRANSIENT_LOCAL 类型的持久性
  wrinfo->is_transient_local = (qos->durability.kind == DDS_DURABILITY_TRANSIENT_LOCAL);

  // 判断是否有 deadline 参数
  wrinfo->has_deadline = (qos->deadline.deadline != DDS_INFINITY);

  // 根据 history 参数设置 hdepth 字段
  wrinfo->hdepth = (qos->history.kind == DDS_HISTORY_KEEP_ALL) ? 0 : (unsigned)qos->history.depth;

  // 根据 is_transient_local 设置 tldepth 字段
  if (!wrinfo->is_transient_local)
    wrinfo->tldepth = 0;
  else
    wrinfo->tldepth = (qos->durability_service.history.kind == DDS_HISTORY_KEEP_ALL)
                          ? 0
                          : (unsigned)qos->durability_service.history.depth;

  // 设置 idxdepth 字段为 hdepth 和 tldepth 中的较大值
  wrinfo->idxdepth = wrinfo->hdepth > wrinfo->tldepth ? wrinfo->hdepth : wrinfo->tldepth;

  // 返回 whc_writer_info 结构体指针
  return wrinfo;
}

/**
 * @brief 释放 whc_writer_info 结构体实例的内存空间
 *
 * @param wrinfo 一个 whc_writer_info 指针，表示要释放的结构体实例
 */
void dds_whc_free_wrinfo(struct whc_writer_info *wrinfo) {
  // 释放 whc_writer_info 结构体实例的内存空间
  ddsrt_free(wrinfo);
}
/**
 * @brief 创建一个 ddsi_whc 结构体实例
 *
 * @param gv 一个 ddsi_domaingv 指针，表示域全局变量
 * @param wrinfo 一个 whc_writer_info 指针，表示写入器信息
 * @return 返回一个 ddsi_whc 结构体指针
 */
struct ddsi_whc *dds_whc_new(struct ddsi_domaingv *gv, const struct whc_writer_info *wrinfo) {
  // 样本开销估计值
  size_t sample_overhead = 80; /* INFO_TS, DATA (estimate), inline QoS */

  // 定义 whc_impl 和 whc_intvnode 结构体指针
  struct whc_impl *whc;
  struct whc_intvnode *intv;

  // 确保 hdepth 和 tldepth 符合要求
  assert((wrinfo->hdepth == 0 || wrinfo->tldepth <= wrinfo->hdepth) || wrinfo->is_transient_local);

  // 为 whc_impl 结构体分配内存空间
  whc = ddsrt_malloc(sizeof(*whc));

  // 初始化 whc_impl 结构体字段
  whc->common.ops = &whc_ops;
  ddsrt_mutex_init(&whc->lock);
  whc->xchecks = (gv->config.enabled_xchecks & DDSI_XCHECK_WHC) != 0;
  whc->gv = gv;
  whc->tkmap = gv->m_tkmap;
  memcpy(&whc->wrinfo, wrinfo, sizeof(*wrinfo));
  whc->seq_size = 0;
  whc->max_drop_seq = 0;
  whc->unacked_bytes = 0;
  whc->total_bytes = 0;
  whc->sample_overhead = sample_overhead;
  whc->fragment_size = gv->config.fragment_size;

  // 初始化 idx_hash 字段
  whc->idx_hash = ddsrt_hh_new(1, whc_idxnode_hash_key, whc_idxnode_eq_key);

#if USE_EHH
  // 初始化 seq_hash 字段（使用 ehh）
  whc->seq_hash =
      ddsrt_ehh_new(sizeof(struct whc_seq_entry), 32, whc_seq_entry_hash, whc_seq_entry_eq);
#else
  // 初始化 seq_hash 字段（不使用 ehh）
  whc->seq_hash = ddsrt_hh_new(1, whc_node_hash, whc_node_eq);
#endif

#ifdef DDS_HAS_LIFESPAN
  // 初始化 lifespan 字段
  ddsi_lifespan_init(gv, &whc->lifespan, offsetof(struct whc_impl, lifespan),
                     offsetof(struct dds_whc_default_node, lifespan), whc_sample_expired_cb);
#endif

#ifdef DDS_HAS_DEADLINE_MISSED
  // 初始化 deadline 字段
  whc->deadline.dur =
      (wrinfo->writer != NULL) ? wrinfo->writer->m_entity.m_qos->deadline.deadline : DDS_INFINITY;
  ddsi_deadline_init(gv, &whc->deadline, offsetof(struct whc_impl, deadline),
                     offsetof(struct whc_idxnode, deadline), whc_deadline_missed_cb);
#endif

  // 初始化 seq interval tree，并创建一个 "open" 节点
  ddsrt_avl_init(&whc_seq_treedef, &whc->seq);
  intv = ddsrt_malloc(sizeof(*intv));
  intv->min = intv->maxp1 = 1;
  intv->first = intv->last = NULL;
  ddsrt_avl_insert(&whc_seq_treedef, &whc->seq, intv);
  whc->open_intv = intv;
  whc->maxseq_node = NULL;

  // 初始化 whc_node_freelist
  ddsrt_mutex_lock(ddsrt_get_singleton_mutex());
  if (whc_count++ == 0)
    ddsi_freelist_init(&whc_node_freelist, MAX_FREELIST_SIZE,
                       offsetof(struct dds_whc_default_node, next_seq));
  ddsrt_mutex_unlock(ddsrt_get_singleton_mutex());

  // 检查 whc 结构体实例
  check_whc(whc);

  // 返回 ddsi_whc 结构体指针
  return (struct ddsi_whc *)whc;
}
/**
 * @brief 释放 whc_node 内容
 *
 * @param whcn 一个 dds_whc_default_node 指针，表示要释放内容的节点
 */
static void free_whc_node_contents(struct dds_whc_default_node *whcn) {
  // 取消对序列化数据的引用
  ddsi_serdata_unref(whcn->serdata);
}

/**
 * @brief 释放 whc_generic 结构体实例
 *
 * @param whc_generic 一个 ddsi_whc 指针，表示要释放的结构体实例
 */
void whc_default_free(struct ddsi_whc *whc_generic) {
  // 不考虑维护数据结构，直接释放资源
  struct whc_impl *const whc = (struct whc_impl *)whc_generic;
  check_whc(whc);

#ifdef DDS_HAS_LIFESPAN
  // 处理过期样本并释放 lifespan 资源
  whc_sample_expired_cb(whc, DDSRT_MTIME_NEVER);
  ddsi_lifespan_fini(&whc->lifespan);
#endif

#ifdef DDS_HAS_DEADLINE_MISSED
  // 停止并清除 deadline 资源
  ddsi_deadline_stop(&whc->deadline);
  ddsrt_mutex_lock(&whc->lock);
  ddsi_deadline_clear(&whc->deadline);
  ddsrt_mutex_unlock(&whc->lock);
  ddsi_deadline_fini(&whc->deadline);
#endif

  // 遍历并释放 idx_hash 中的节点
  struct ddsrt_hh_iter it;
  struct whc_idxnode *idxn;
  for (idxn = ddsrt_hh_iter_first(whc->idx_hash, &it); idxn != NULL;
       idxn = ddsrt_hh_iter_next(&it)) {
    ddsi_tkmap_instance_unref(whc->tkmap, idxn->tk);
    ddsrt_free(idxn);
  }
  // 释放 idx_hash
  ddsrt_hh_free(whc->idx_hash);

  // 释放 maxseq_node 链表中的节点
  {
    struct dds_whc_default_node *whcn = whc->maxseq_node;
    while (whcn) {
      struct dds_whc_default_node *tmp = whcn;
      /* The compiler doesn't realize that whcn->prev_seq is always initialized. */
      DDSRT_WARNING_MSVC_OFF(6001);
      whcn = whcn->prev_seq;
      DDSRT_WARNING_MSVC_ON(6001);
      free_whc_node_contents(tmp);
      ddsrt_free(tmp);
    }
  }
}

/**
 * @brief 释放 whc->seq 中的 AVL 树节点，并逐行添加详细的中文注释。
 * @param[in] whc_seq_treedef AVL 树定义。
 * @param[in] whc->seq 要释放的 AVL 树。
 * @param[in] ddsrt_free 用于释放内存的函数指针。
 */
ddsrt_avl_free(&whc_seq_treedef, &whc->seq, ddsrt_free);

// 获取全局单例互斥锁并加锁
ddsrt_mutex_lock(ddsrt_get_singleton_mutex());

// 如果 whc_count 减少到 0，则释放 whc_node_freelist
if (--whc_count == 0) ddsi_freelist_fini(&whc_node_freelist, ddsrt_free);

// 解锁全局单例互斥锁
ddsrt_mutex_unlock(ddsrt_get_singleton_mutex());

// 根据 USE_EHH 宏定义选择使用哪种哈希表释放函数
#if USE_EHH
// 使用 ehh 哈希表释放函数
ddsrt_ehh_free(whc->seq_hash);
#else
// 使用 hh 哈希表释放函数
ddsrt_hh_free(whc->seq_hash);
#endif

// 销毁 whc 的互斥锁
ddsrt_mutex_destroy(&whc->lock);

// 释放 whc 结构体占用的内存
ddsrt_free(whc);
}

/*
AVL树（Adelson-Velsky和Landis发明）是一种自平衡二叉搜索树。在AVL树中，任何节点的两个子树的高度最多相差1。这个特性使得AVL树在插入、删除和查找操作时能够保持较低的树高，从而提高了操作的效率。

以下是AVL树的一些关键特性：

1. **平衡因子**：每个节点的平衡因子是其左子树的高度减去右子树的高度。平衡因子的取值范围为{-1, 0,
1}。
2.
**旋转**：当插入或删除节点导致AVL树失去平衡时，需要通过旋转操作来恢复平衡。有四种基本旋转：左旋、右旋、左右旋和右左旋。
   - 左旋（LL）：当一个节点的右子树比左子树高度大2时，将该节点向左旋转。
   - 右旋（RR）：当一个节点的左子树比右子树高度大2时，将该节点向右旋转。
   -
左右旋（LR）：当一个节点的左子树的右子树高度大于左子树的左子树时，先对该节点的左子树进行左旋，然后再对该节点进行右旋。
   -
右左旋（RL）：当一个节点的右子树的左子树高度大于右子树的右子树时，先对该节点的右子树进行右旋，然后再对该节点进行左旋。
3.
**插入**：在AVL树中插入一个新节点时，首先按照二叉搜索树的规则找到合适的位置。然后，更新祖先节点的高度并检查平衡因子。如果有节点失去平衡，执行相应的旋转操作来恢复平衡。
4.
**删除**：从AVL树中删除一个节点时，首先按照二叉搜索树的规则删除节点。然后，更新祖先节点的高度并检查平衡因子。如果有节点失去平衡，执行相应的旋转操作来恢复平衡。
5. **查找**：由于AVL树是一种二叉搜索树，查找操作与普通二叉搜索树相同。

AVL树的主要优点是它能够在插入、删除和查找操作中保持较低的树高，从而提高了操作效率。在最坏情况下，AVL树的时间复杂度为O(log
N)，其中N是树中节点的数量。这使得AVL树在需要频繁执行这些操作的场景中非常有用，例如数据库和文件系统。
*/

/**
 * @brief 获取 whc 的状态信息（已加锁）。
 * @param[in] whc 指向 whc_impl 结构体的指针。
 * @param[out] st 用于存储状态信息的 ddsi_whc_state 结构体指针。
 */
static void get_state_locked(const struct whc_impl *whc, struct ddsi_whc_state *st) {
  // 如果 whc->seq_size 为 0，表示序列为空
  if (whc->seq_size == 0) {
    // 设置最小序列号、最大序列号和未确认字节数为 0
    st->min_seq = st->max_seq = 0;
    st->unacked_bytes = 0;
  } else {
    const struct whc_intvnode *intv;
    // 查找 whc->seq 中的最小节点
    intv = ddsrt_avl_find_min(&whc_seq_treedef, &whc->seq);
    /* 序列非空，打开节点可以是任何值，但根据定义，
       findmax 和 whc 都被声明为非空，所以最小间隔
       不可能为空 */
    assert(intv->maxp1 > intv->min);
    // 设置最小序列号和最大序列号
    st->min_seq = intv->min;
    st->max_seq = whc->maxseq_node->common.seq;
    // 设置未确认字节数
    st->unacked_bytes = whc->unacked_bytes;
  }
}

/**
 * @brief 获取 whc 的默认状态信息。
 * @param[in] whc_generic 指向 ddsi_whc 结构体的指针。
 * @param[out] st 用于存储状态信息的 ddsi_whc_state 结构体指针。
 */
static void whc_default_get_state(const struct ddsi_whc *whc_generic, struct ddsi_whc_state *st) {
  // 将通用 whc 结构体转换为 whc_impl 结构体
  const struct whc_impl *const whc = (const struct whc_impl *)whc_generic;

  // 对 whc 的互斥锁加锁
  ddsrt_mutex_lock((ddsrt_mutex_t *)&whc->lock);

  // 检查 whc 是否有效
  check_whc(whc);

  // 获取已加锁的 whc 状态信息
  get_state_locked(whc, st);

  // 解锁 whc 的互斥锁
  ddsrt_mutex_unlock((ddsrt_mutex_t *)&whc->lock);
}

/**
 * @brief 在给定的序列号中查找下一个序列间隔节点
 *
 * @param[out] p_intv 指向找到的序列间隔节点的指针
 * @param[in] whc 指向whc_impl结构体的指针
 * @param[in] seq 要查找的序列号
 * @return 返回找到的dds_whc_default_node，如果没有找到则返回NULL
 */
static struct dds_whc_default_node *find_nextseq_intv(struct whc_intvnode **p_intv,
                                                      const struct whc_impl *whc,
                                                      ddsi_seqno_t seq) {
  // 定义节点和间隔变量
  struct dds_whc_default_node *n;
  struct whc_intvnode *intv;

  // 查找给定序列号的节点，如果找不到则返回NULL
  if ((n = whc_findseq(whc, seq)) == NULL) {
    /* 不知道序列号 => 查找具有 min > seq 的间隔（间隔是连续的，
       所以如果我们不知道序列号，就不能存在一个间隔 [X,Y) 使得 X < SEQ < Y） */
#ifndef NDEBUG
    {
      struct whc_intvnode *predintv = ddsrt_avl_lookup_pred_eq(&whc_seq_treedef, &whc->seq, &seq);
      assert(predintv == NULL || predintv->maxp1 <= seq);
    }
#endif
    // 查找大于等于给定序列号的最小间隔节点
    if ((intv = ddsrt_avl_lookup_succ_eq(&whc_seq_treedef, &whc->seq, &seq)) == NULL) {
      assert(ddsrt_avl_lookup_pred_eq(&whc_seq_treedef, &whc->seq, &seq) == whc->open_intv);
      return NULL;
    }
    // 如果间隔不为空，则返回第一个节点
    else if (intv->min < intv->maxp1) { /* 只有在非空间隔的情况下 */
      assert(intv->min > seq);
      *p_intv = intv;
      return intv->first;
    }
    // 如果间隔为空，只有 open_intv 可能为空
    else {
      assert(intv == whc->open_intv);
      return NULL;
    }
  }
  // 如果找到了序列号对应的节点，但没有下一个序列节点，则返回NULL
  else if (n->next_seq == NULL) {
    assert(n == whc->maxseq_node);
    return NULL;
  }
  // 如果有下一个序列节点，查找并返回它
  else {
    assert(whc->maxseq_node != NULL);
    assert(n->common.seq < whc->maxseq_node->common.seq);
    n = n->next_seq;
    *p_intv = ddsrt_avl_lookup_pred_eq(&whc_seq_treedef, &whc->seq, &n->common.seq);
    return n;
  }
}
/**
 * @brief 获取下一个序列号
 *
 * @param whc_generic 通用的写历史缓存指针
 * @param seq 当前序列号
 * @return ddsi_seqno_t 下一个序列号
 */
static ddsi_seqno_t whc_default_next_seq(const struct ddsi_whc *whc_generic, ddsi_seqno_t seq) {
  // 将通用的写历史缓存指针转换为具体实现类型的指针
  const struct whc_impl *const whc = (const struct whc_impl *)whc_generic;
  struct dds_whc_default_node *n;
  struct whc_intvnode *intv;
  ddsi_seqno_t nseq;

  // 对写历史缓存上锁
  ddsrt_mutex_lock((ddsrt_mutex_t *)&whc->lock);

  // 检查写历史缓存
  check_whc(whc);

  // 查找下一个序列号所在的区间
  if ((n = find_nextseq_intv(&intv, whc, seq)) == NULL)
    nseq = DDSI_MAX_SEQ_NUMBER;  // 如果没有找到，则返回最大序列号
  else
    nseq = n->common.seq;        // 否则返回找到的序列号

  // 对写历史缓存解锁
  ddsrt_mutex_unlock((ddsrt_mutex_t *)&whc->lock);

  return nseq;
}

/**
 * @brief 从索引中删除一个样本
 *
 * @param whcn 要删除的默认节点指针
 */
static void delete_one_sample_from_idx(struct dds_whc_default_node *whcn) {
  struct whc_idxnode *const idxn = whcn->idxnode;

  // 断言检查
  assert(idxn != NULL);
  assert(idxn->hist[idxn->headidx] != NULL);
  assert(idxn->hist[whcn->idxnode_pos] == whcn);

  // 将索引节点的历史记录置空，并将默认节点的索引节点指针置空
  idxn->hist[whcn->idxnode_pos] = NULL;
  whcn->idxnode = NULL;
}

/**
 * @brief 从索引中释放一个实例
 *
 * @param whc 写历史缓存实现指针
 * @param max_drop_seq 最大丢弃序列号
 * @param idxn 索引节点指针
 */
static void free_one_instance_from_idx(struct whc_impl *whc,
                                       ddsi_seqno_t max_drop_seq,
                                       struct whc_idxnode *idxn) {
  // 遍历写历史缓存的索引深度
  for (uint32_t i = 0; i < whc->wrinfo.idxdepth; i++) {
    if (idxn->hist[i]) {
      struct dds_whc_default_node *oldn = idxn->hist[i];

      // 将旧节点的索引节点指针置空
      oldn->idxnode = NULL;

      // 如果旧节点的序列号小于等于最大丢弃序列号，则删除该节点
      if (oldn->common.seq <= max_drop_seq) {
        TRACE("  prune tl whcn %p\n", (void *)oldn);
        assert(oldn != whc->maxseq_node);
        whc_delete_one(whc, oldn);
      }
    }
  }

  // 取消对实例的引用并释放索引节点
  ddsi_tkmap_instance_unref(whc->tkmap, idxn->tk);
  ddsrt_free(idxn);
}
/** 删除索引中的一个实例
 *  @param[in] whc 指向whc_impl结构体的指针
 *  @param[in] max_drop_seq 最大丢弃序列号
 *  @param[in] idxn 指向whc_idxnode结构体的指针
 */
static void delete_one_instance_from_idx(struct whc_impl *whc,
                                         ddsi_seqno_t max_drop_seq,
                                         struct whc_idxnode *idxn) {
  // 从哈希表中移除当前实例
  ddsrt_hh_remove_present(whc->idx_hash, idxn);

  // 如果定义了DDS_HAS_DEADLINE_MISSED宏，则取消注册实例的deadline
#ifdef DDS_HAS_DEADLINE_MISSED
  ddsi_deadline_unregister_instance_locked(&whc->deadline, &idxn->deadline);
#endif

  // 释放索引中的一个实例
  free_one_instance_from_idx(whc, max_drop_seq, idxn);
}

/** 判断实例是否在时间线索引中
 *  @param[in] whc 指向whc_impl结构体的指针
 *  @param[in] idxn 指向whc_idxnode结构体的指针
 *  @param[in] pos 位置
 *  @return 如果实例在时间线索引中，返回1，否则返回0
 */
static int whcn_in_tlidx(const struct whc_impl *whc, const struct whc_idxnode *idxn, uint32_t pos) {
  if (idxn == NULL)
    return 0;
  else {
    // 计算距离
    uint32_t d = (idxn->headidx + (pos > idxn->headidx ? whc->wrinfo.idxdepth : 0)) - pos;
    // 断言距离小于索引深度
    assert(d < whc->wrinfo.idxdepth);
    // 如果距离小于时间线深度，返回1，否则返回0
    return d < whc->wrinfo.tldepth;
  }
}

/** 计算whcn的大小
 *  @param[in] whc 指向whc_impl结构体的指针
 *  @param[in] whcn 指向dds_whc_default_node结构体的指针
 *  @return 返回whcn的大小
 */
static size_t whcn_size(const struct whc_impl *whc, const struct dds_whc_default_node *whcn) {
  // 计算序列化数据的大小
  size_t sz = ddsi_serdata_size(whcn->serdata);
  // 计算并返回whcn的总大小
  return sz + ((sz + whc->fragment_size - 1) / whc->fragment_size) * whc->sample_overhead;
}
/**
 * @brief 删除一个 whc_intvnode 中的一个 dds_whc_default_node 节点
 *
 * 从 *p_whcn 中删除节点，可能会删除或拆分 *p_intv。不更新 whc->seq_size。
 * *p_intv 必须是包含 *p_whcn 的区间（且两者都必须存在）。
 *
 * @param[in] whc 指向 whc_impl 结构体的指针
 * @param[in,out] p_intv 指向 whc_intvnode 指针的指针
 * @param[in,out] p_whcn 指向 dds_whc_default_node 指针的指针
 *
 * @return
 * - 0 如果删除失败（唯一可能的原因是内存耗尽），此时 *p_intv 和 *p_whcn 未定义；
 * - 1 如果成功，则 *p_intv 和 *p_whcn 设置正确，按序列号顺序进入下一个样本
 */
static void whc_delete_one_intv(struct whc_impl *whc,
                                struct whc_intvnode **p_intv,
                                struct dds_whc_default_node **p_whcn) {
  // 获取 whc_intvnode 和 dds_whc_default_node 的实际结构体
  struct whc_intvnode *intv = *p_intv;
  struct dds_whc_default_node *whcn = *p_whcn;

  // 确保 whcn 的序列号在 intv 的范围内
  assert(whcn->common.seq >= intv->min && whcn->common.seq < intv->maxp1);

  // 更新 p_whcn 指向下一个序列节点
  *p_whcn = whcn->next_seq;

  // 如果它在 tlidx 中，将其移除。Transient-local 数据不会到这里
  if (whcn->idxnode) delete_one_sample_from_idx(whcn);

  // 如果 whcn 未被确认，则更新 whc 的 unacked_bytes 并将 whcn 的 unacked 设为 0
  if (whcn->unacked) {
    assert(whc->unacked_bytes >= whcn->size);
    whc->unacked_bytes -= whcn->size;
    whcn->unacked = 0;
  }

#ifdef DDS_HAS_LIFESPAN
  ddsi_lifespan_unregister_sample_locked(&whc->lifespan, &whcn->lifespan);
#endif

  // 从 seqhash 中移除 whcn；从基于序列号排序的列表中删除它留给调用者处理
  remove_whcn_from_hash(whc, whcn);

  // 我们可能引入了一个空洞并且必须拆分区间节点，或者我们可能削减了第一个，甚至是最后一个
  if (whcn == intv->first) {
    if (whcn == intv->last && intv != whc->open_intv) {
      struct whc_intvnode *tmp = intv;
      *p_intv = ddsrt_avl_find_succ(&whc_seq_treedef, &whc->seq, intv);
      // 只有样本在区间内且不是开放区间 => 删除区间
      ddsrt_avl_delete(&whc_seq_treedef, &whc->seq, tmp);
      ddsrt_free(tmp);
    } else {
      intv->first = whcn->next_seq;
      intv->min++;
      assert(intv->first != NULL || intv == whc->open_intv);
      assert(intv->min < intv->maxp1 || intv == whc->open_intv);
      assert((intv->first == NULL) == (intv->min == intv->maxp1));
    }
  } else if (whcn == intv->last) {
    // 至少它不是第一个，所以区间仍然非空，我们不必删除区间
    assert(intv->min < whcn->common.seq);
    assert(whcn->prev_seq);
    assert(whcn->prev_seq->common.seq + 1 == whcn->common.seq);
    intv->last = whcn->prev_seq;
    intv->maxp1--;
    *p_intv = ddsrt_avl_find_succ(&whc_seq_treedef, &whc->seq, intv);
  } else {
    // 在中间的某个地方 => 拆分区间（理想情况下，会懒惰地拆分它，但这实际上只是一个瞬态本地问题，
    // 因此我们可以（暂时）贪婪地拆分它）
    struct whc_intvnode *new_intv;
    ddsrt_avl_ipath_t path;

    new_intv = ddsrt_malloc(sizeof(*new_intv));

    // 新区间从下一个节点开始
    assert(whcn->next_seq);
    assert(whcn->common.seq + 1 == whcn->next_seq->common.seq);
    new_intv->first = whcn->next_seq;
    new_intv->last = intv->last;
    new_intv->min = whcn->common.seq + 1;
    new_intv->maxp1 = intv->maxp1;
    intv->last = whcn->prev_seq;
    intv->maxp1 = whcn->common.seq;
    assert(intv->min < intv->maxp1);
    assert(new_intv->min < new_intv->maxp1);

    // 插入新节点并将 intv 设置为新区间，然后继续循环
    if (ddsrt_avl_lookup_ipath(&whc_seq_treedef, &whc->seq, &new_intv->min, &path) != NULL)
      assert(0);
    ddsrt_avl_insert_ipath(&whc_seq_treedef, &whc->seq, new_intv, &path);

    if (intv == whc->open_intv) whc->open_intv = new_intv;
    *p_intv = new_intv;
  }
}
/**
 * @brief 删除一个 whc 节点
 *
 * @param[in] whc 指向 whc_impl 结构体的指针
 * @param[in] whcn 指向 dds_whc_default_node 结构体的指针
 */
static void whc_delete_one(struct whc_impl *whc, struct dds_whc_default_node *whcn) {
  // 定义一个指向 whc_intvnode 结构体的指针
  struct whc_intvnode *intv;
  // 定义一个临时指针，指向 whcn
  struct dds_whc_default_node *whcn_tmp = whcn;
  // 查找 whcn->common.seq 在 whc->seq 中的前驱节点或相等节点
  intv = ddsrt_avl_lookup_pred_eq(&whc_seq_treedef, &whc->seq, &whcn->common.seq);
  // 断言 intv 不为空
  assert(intv != NULL);
  // 删除一个 whc 节点
  whc_delete_one_intv(whc, &intv, &whcn);
  // 更新 whcn_tmp 的前序节点的后继节点
  if (whcn_tmp->prev_seq) whcn_tmp->prev_seq->next_seq = whcn_tmp->next_seq;
  // 更新 whcn_tmp 的后继节点的前序节点
  if (whcn_tmp->next_seq) whcn_tmp->next_seq->prev_seq = whcn_tmp->prev_seq;
  // 将 whcn_tmp 的后继节点置空
  whcn_tmp->next_seq = NULL;
  // 释放延迟释放列表
  free_deferred_free_list(whcn_tmp);
  // 减少 whc 的 seq_size
  whc->seq_size--;
}

/**
 * @brief 释放延迟释放的节点列表
 *
 * @param[in] deferred_free_list 指向 dds_whc_default_node 结构体的指针
 */
static void free_deferred_free_list(struct dds_whc_default_node *deferred_free_list) {
  // 判断延迟释放列表是否为空
  if (deferred_free_list) {
    // 定义两个指针，用于遍历延迟释放列表
    struct dds_whc_default_node *cur, *last;
    // 定义一个计数器
    uint32_t n = 0;
    // 遍历延迟释放列表
    for (cur = deferred_free_list, last = NULL; cur; last = cur, cur = cur->next_seq) {
      // 计数器递增
      n++;
      // 如果当前节点没有被借用，则释放节点内容
      if (!cur->borrowed) free_whc_node_contents(cur);
    }
    // 将延迟释放列表中的节点压入 freelist
    cur = ddsi_freelist_pushmany(&whc_node_freelist, deferred_free_list, last, n);
    // 释放延迟释放列表中的节点
    while (cur) {
      struct dds_whc_default_node *tmp = cur;
      cur = cur->next_seq;
      ddsrt_free(tmp);
    }
  }
}

/**
 * @brief 默认释放延迟释放的节点列表
 *
 * @param[in] whc_generic 指向 ddsi_whc 结构体的指针
 * @param[in] deferred_free_list 指向 ddsi_whc_node 结构体的指针
 */
static void whc_default_free_deferred_free_list(struct ddsi_whc *whc_generic,
                                                struct ddsi_whc_node *deferred_free_list) {
  // 忽略 whc_generic 参数
  (void)whc_generic;
  // 调用 free_deferred_free_list 函数释放延迟释放列表
  free_deferred_free_list((struct dds_whc_default_node *)deferred_free_list);
}
/**
 * @brief 从 WHC 中删除已确认的消息，不使用索引。
 *
 * @param[in] whc 指向 whc_impl 结构体的指针
 * @param[in] max_drop_seq 可以删除的最大序列号
 * @param[out] deferred_free_list 被删除节点的链表，用于后续释放内存
 * @return 返回删除的消息数量
 */
static uint32_t whc_default_remove_acked_messages_noidx(struct whc_impl *whc,
                                                        ddsi_seqno_t max_drop_seq,
                                                        struct ddsi_whc_node **deferred_free_list) {
  struct whc_intvnode *intv;
  struct dds_whc_default_node *whcn;
  uint32_t ndropped = 0;

  // 如果 WHC 为空，则快速返回
  if (max_drop_seq <= whc->max_drop_seq || whc->maxseq_node == NULL) {
    if (max_drop_seq > whc->max_drop_seq) whc->max_drop_seq = max_drop_seq;
    *deferred_free_list = NULL;
    return 0;
  }

  // 在简单情况下，我们总是删除一切直到 whc->max_drop_seq，并且只有一个间隔
#ifndef NDEBUG
  whcn = find_nextseq_intv(&intv, whc, whc->max_drop_seq);
  assert(whcn == NULL || whcn->prev_seq == NULL);
  assert(ddsrt_avl_is_singleton(&whc->seq));
#endif
  intv = whc->open_intv;

  // 删除所有直到包括 max_drop_seq
  // 的内容，或者在没有这个序列号的情况下，删除最高可用的序列号（必须更小）
  if ((whcn = whc_findseq(whc, max_drop_seq)) == NULL) {
    if (max_drop_seq < intv->min) {
      // 在启动时，whc->max_drop_seq = 0，读取器状态从 wr->seq 获取最大确认序列；
      // 因此，如果有多个匹配的读取器，并且写入器超前于读取器，则对于第一个 ack，whc->max_drop_seq <
      // max_drop_seq = MIN(readers max ack) < intv->min
      if (max_drop_seq > whc->max_drop_seq) whc->max_drop_seq = max_drop_seq;
      *deferred_free_list = NULL;
      return 0;
    } else {
      whcn = whc->maxseq_node;
      assert(whcn->common.seq < max_drop_seq);
    }
  }

  // 将 intv->first 赋值给 *deferred_free_list，用于后续释放内存
  *deferred_free_list = (struct ddsi_whc_node *)intv->first;
  // 计算删除的消息数量
  ndropped = (uint32_t)(whcn->common.seq - intv->min + 1);

  // 更新 intv 的 first 和 min 值
  intv->first = whcn->next_seq;
  intv->min = max_drop_seq + 1;
  // 如果 whcn 的 next_seq 为空，则更新 whc->maxseq_node 和 intv->maxp1
  if (whcn->next_seq == NULL) {
    whc->maxseq_node = NULL;
    intv->maxp1 = intv->min;
  } else {
    // 断言 whcn 的 next_seq 的 common.seq 等于 max_drop_seq + 1
    assert(whcn->next_seq->common.seq == max_drop_seq + 1);
    // 将 whcn 的 next_seq 的 prev_seq 设置为 NULL
    whcn->next_seq->prev_seq = NULL;
  }
  // 将 whcn 的 next_seq 设置为 NULL
  whcn->next_seq = NULL;

  // 获取 *deferred_free_list 对应的 dds_whc_default_node 结构体指针
  struct dds_whc_default_node *dfln = (struct dds_whc_default_node *)*deferred_free_list;
  // 断言 whcn 的 total_bytes 减去 dfln 的 total_bytes 加上 dfln 的 size 不大于 whc 的 unacked_bytes
  assert(whcn->total_bytes - dfln->total_bytes + dfln->size <= whc->unacked_bytes);
  // 更新 whc 的 unacked_bytes
  whc->unacked_bytes -= (size_t)(whcn->total_bytes - dfln->total_bytes + dfln->size);
  // 遍历 dfln 链表，处理每个节点
  for (whcn = (struct dds_whc_default_node *)dfln; whcn; whcn = whcn->next_seq) {
#ifdef DDS_HAS_LIFESPAN
    // 如果定义了 DDS_HAS_LIFESPAN，则取消注册样本的生命周期
    ddsi_lifespan_unregister_sample_locked(&whc->lifespan, &whcn->lifespan);
#endif
    // 从哈希表中移除 whcn 节点
    remove_whcn_from_hash(whc, whcn);
    // 断言 whcn 的 unacked 为真
    assert(whcn->unacked);
  }

  // 断言 ndropped 不大于 whc 的 seq_size
  assert(ndropped <= whc->seq_size);
  // 更新 whc 的 seq_size
  whc->seq_size -= ndropped;
  // 更新 whc 的 max_drop_seq
  whc->max_drop_seq = max_drop_seq;
  // 返回删除的消息数量
  return ndropped;
}
/**
 * @brief 从 whc 中删除已确认的消息，并将要延迟释放的节点添加到 deferred_free_list 中。
 *
 * @param[in] whc 指向 whc_impl 结构体的指针
 * @param[in] max_drop_seq 最大可删除序列号
 * @param[out] deferred_free_list 要延迟释放的节点列表
 * @return 删除的消息数量
 */
static uint32_t whc_default_remove_acked_messages_full(struct whc_impl *whc,
                                                       ddsi_seqno_t max_drop_seq,
                                                       struct ddsi_whc_node **deferred_free_list) {
  // 定义变量
  struct whc_intvnode *intv;
  struct dds_whc_default_node *whcn;
  struct dds_whc_default_node *prev_seq;
  struct dds_whc_default_node deferred_list_head, *last_to_free = &deferred_list_head;
  uint32_t ndropped = 0;

  // 查找下一个序列号间隔
  whcn = find_nextseq_intv(&intv, whc, whc->max_drop_seq);
  if (whc->wrinfo.is_transient_local && whc->wrinfo.tldepth == 0) {
    // 如果是 KEEP_ALL 的 transient local，我们不能删除任何数据，但是必须在 whc 中确认数据
    TRACE("  KEEP_ALL transient-local: ack data\n");
    while (whcn && whcn->common.seq <= max_drop_seq) {
      if (whcn->unacked) {
        assert(whc->unacked_bytes >= whcn->size);
        whc->unacked_bytes -= whcn->size;
        whcn->unacked = 0;
      }
      whcn = whcn->next_seq;
    }
    whc->max_drop_seq = max_drop_seq;
    *deferred_free_list = NULL;
    return 0;
  }

  deferred_list_head.next_seq = NULL;
  prev_seq = whcn ? whcn->prev_seq : NULL;
  while (whcn && whcn->common.seq <= max_drop_seq) {
    TRACE("  whcn %p %" PRIu64, (void *)whcn, whcn->common.seq);
    if (whcn_in_tlidx(whc, whcn->idxnode, whcn->idxnode_pos)) {
      // 快速跳过 tlidx 中的样本
      TRACE(" tl:keep");
      // 如果 whcn 未被确认
      if (whcn->unacked) {
        // 断言 whc 中未确认的字节数大于等于 whcn 的大小
        assert(whc->unacked_bytes >= whcn->size);
        // 减少 whc 中未确认的字节数
        whc->unacked_bytes -= whcn->size;
        // 将 whcn 标记为已确认
        whcn->unacked = 0;
      }

      // 如果 whcn 是当前间隔的最后一个节点
      if (whcn == intv->last)
        // 查找下一个间隔
        intv = ddsrt_avl_find_succ(&whc_seq_treedef, &whc->seq, intv);
      // 如果存在前一个序列节点
      if (prev_seq)
        // 将前一个序列节点的 next_seq 指向 whcn
        prev_seq->next_seq = whcn;
      // 更新 whcn 的 prev_seq 为 prev_seq
      whcn->prev_seq = prev_seq;
      // 将 prev_seq 设置为当前的 whcn
      prev_seq = whcn;
      // 将 whcn 设置为下一个序列节点
      whcn = whcn->next_seq;
    } else {
      TRACE(" delete");
      // 将 last_to_free 的 next_seq 指向 whcn
      last_to_free->next_seq = whcn;
      // 更新 last_to_free 为其 next_seq 节点
      last_to_free = last_to_free->next_seq;
      // 从 whc 中删除一个间隔节点，并更新 intv 和 whcn
      whc_delete_one_intv(whc, &intv, &whcn);
      // 增加已删除的消息数量
      ndropped++;
    }
    TRACE("\n");
  }
  // 如果存在前一个序列节点
  if (prev_seq)
    // 将前一个序列节点的 next_seq 指向 whcn
    prev_seq->next_seq = whcn;
  // 如果存在 whcn 节点
  if (whcn)
    // 更新 whcn 的 prev_seq 为 prev_seq
    whcn->prev_seq = prev_seq;
  // 将 last_to_free 的 next_seq 设置为 NULL，表示列表结束
  last_to_free->next_seq = NULL;
  // 将延迟释放列表设置为 deferred_list_head 的 next_seq 节点
  *deferred_free_list = (struct ddsi_whc_node *)deferred_list_head.next_seq;

  // 如果历史记录比 durability_service.history 更深（但不是 KEEP_ALL），则此实例中可能有旧样本
  if (whc->wrinfo.tldepth > 0 && whc->wrinfo.idxdepth > whc->wrinfo.tldepth) {
    assert(whc->wrinfo.hdepth == whc->wrinfo.idxdepth);
    TRACE("  idxdepth %" PRIu32 " > tldepth %" PRIu32 " > 0 -- must prune\n", whc->wrinfo.idxdepth,
          whc->wrinfo.tldepth);

    // 对刚刚处理过的序列号范围进行第二次遍历：这次我们只会遇到因为 transient-local
    // 持久性设置而保留的样本
    whcn = find_nextseq_intv(&intv, whc, whc->max_drop_seq);
    while (whcn && whcn->common.seq <= max_drop_seq) {
      /**
       * @brief 为 whcn 和 idxn 添加详细的中文注释
       *
       * @param[in] whcn 指向 whc_node 结构体的指针
       * @param[in] idxn 指向 whc_idxnode 结构体的指针
       * @param[in] max_drop_seq 最大可删除序列号
       */
      struct whc_idxnode *const idxn = whcn->idxnode;  // 获取 whcn 的 idxnode 成员
      uint32_t cnt, idx;

      // 打印调试信息，包括 whcn、whcn 序列号、idxn 和 idxn 的剪枝序列号
      TRACE("  whcn %p %" PRIu64 " idxn %p prune_seq %" PRIu64 ":", (void *)whcn, whcn->common.seq,
            (void *)idxn, idxn->prune_seq);

      // 断言：检查 whcn 是否在 tlidx 中
      assert(whcn_in_tlidx(whc, idxn, whcn->idxnode_pos));
      // 断言：检查 idxn 的剪枝序列号是否小于等于 max_drop_seq
      assert(idxn->prune_seq <= max_drop_seq);

      // 如果 idxn 的剪枝序列号等于 max_drop_seq
      if (idxn->prune_seq == max_drop_seq) {
        TRACE(" already pruned\n");    // 打印已经剪枝的信息
        whcn = whcn->next_seq;         // 更新 whcn 为下一个序列节点
        continue;                      // 跳过当前循环，进入下一次循环
      }
      idxn->prune_seq = max_drop_seq;  // 更新 idxn 的剪枝序列号为 max_drop_seq

      idx = idxn->headidx;
      cnt = whc->wrinfo.idxdepth - whc->wrinfo.tldepth;
      while (cnt--) {
        struct dds_whc_default_node *oldn;
        if (++idx == whc->wrinfo.idxdepth) idx = 0;
        if ((oldn = idxn->hist[idx]) != NULL) {
          // 删除它，但这可能不会导致删除索引节点，因为必须仍然有一个更近期的可用
#ifndef NDEBUG
          struct whc_idxnode template;
          template.iid = idxn->iid;
          assert(oldn->common.seq < whcn->common.seq);
#endif
          TRACE(" del %p %" PRIu64, (void *)oldn, oldn->common.seq);
          whc_delete_one(whc, oldn);
#ifndef NDEBUG
          assert(ddsrt_hh_lookup(whc->idx_hash, &template) == idxn);
#endif
        }
      }
      TRACE("\n");
      whcn = whcn->next_seq;
    }
  }

  assert(ndropped <= whc->seq_size);
  whc->seq_size -= ndropped;

  // 懒惰的人这样做：
  whc->maxseq_node = whc_findmax_procedurally(whc);
  whc->max_drop_seq = max_drop_seq;
  return ndropped;
}

/**
 * @brief 默认删除已确认消息的函数实现
 *
 * @param[in] whc_generic 通用的 ddsi_whc 结构体指针
 * @param[in] max_drop_seq 最大可删除序列号
 * @param[out] whcst 存储当前 WHC 状态的 ddsi_whc_state 结构体指针
 * @param[out] deferred_free_list 指向延迟释放列表的指针
 * @return 删除的消息数量
 */
static uint32_t whc_default_remove_acked_messages(struct ddsi_whc *whc_generic,
                                                  ddsi_seqno_t max_drop_seq,
                                                  struct ddsi_whc_state *whcst,
                                                  struct ddsi_whc_node **deferred_free_list) {
  struct whc_impl *const whc =
      (struct whc_impl *)whc_generic;  // 将通用的 ddsi_whc 转换为具体的 whc_impl 结构体
  uint32_t cnt;

  ddsrt_mutex_lock(&whc->lock);  // 锁定 WHC
  assert(max_drop_seq < DDSI_MAX_SEQ_NUMBER);
  assert(max_drop_seq >= whc->max_drop_seq);

  // 如果启用了 WHC 日志记录
  if (whc->gv->logconfig.c.mask & DDS_LC_WHC) {
    struct ddsi_whc_state tmp;
    get_state_locked(whc, &tmp);
    TRACE("whc_default_remove_acked_messages(%p max_drop_seq %" PRIu64 ")\n", (void *)whc,
          max_drop_seq);
    TRACE("  whc: [%" PRIu64 ",%" PRIu64 "] max_drop_seq %" PRIu64 " h %" PRIu32 " tl %" PRIu32
          "\n",
          tmp.min_seq, tmp.max_seq, whc->max_drop_seq, whc->wrinfo.hdepth, whc->wrinfo.tldepth);
  }

  check_whc(whc);

  // 如果设置了 deadline，可能会将样本临时添加到 WHC 中，这些样本可能已经处于 acked 状态。
  // _noidx 变体的删除消息函数假设 WHC 中存在未确认的数据。因此，在设置了 deadline 的情况下，
  // 即使索引深度为 0，也会使用 _full 变体。
  if (whc->wrinfo.idxdepth == 0 && !whc->wrinfo.has_deadline && !whc->wrinfo.is_transient_local)
    cnt = whc_default_remove_acked_messages_noidx(whc, max_drop_seq, deferred_free_list);
  else
    cnt = whc_default_remove_acked_messages_full(whc, max_drop_seq, deferred_free_list);
  get_state_locked(whc, whcst);    // 获取当前 WHC 状态
  ddsrt_mutex_unlock(&whc->lock);  // 解锁 WHC
  return cnt;                      // 返回删除的消息数量
}
/**
 * @brief 向 whc 中插入一个序列号为 seq 的节点，并返回新插入的节点指针。
 *
 * @param[in] whc        指向 whc_impl 结构体的指针
 * @param[in] max_drop_seq 最大可丢弃序列号
 * @param[in] seq        要插入的序列号
 * @param[in] exp        生命周期到期时间
 * @param[in] serdata    序列化数据
 * @return 返回新插入的节点指针
 */
static struct dds_whc_default_node *whc_default_insert_seq(struct whc_impl *whc,
                                                           ddsi_seqno_t max_drop_seq,
                                                           ddsi_seqno_t seq,
                                                           ddsrt_mtime_t exp,
                                                           struct ddsi_serdata *serdata) {
  // 新节点指针初始化为 NULL
  struct dds_whc_default_node *newn = NULL;

#ifndef DDS_HAS_LIFESPAN
  // 如果不支持生命周期，忽略 exp 参数
  DDSRT_UNUSED_ARG(exp);
#endif

  // 从 freelist 中获取一个空闲节点，如果没有则分配内存
  if ((newn = ddsi_freelist_pop(&whc_node_freelist)) == NULL) newn = ddsrt_malloc(sizeof(*newn));
  // 设置新节点的序列号
  newn->common.seq = seq;
  // 根据序列号判断是否未确认
  newn->unacked = (seq > max_drop_seq);
  // 初始化借用计数为 0
  newn->borrowed = 0;
  // 初始化索引节点为 NULL
  newn->idxnode = NULL;
  // 初始化索引节点位置为 0
  newn->idxnode_pos = 0;
  // 初始化最后重传时间戳为 0
  newn->last_rexmit_ts.v = 0;
  // 初始化重传计数为 0
  newn->rexmit_count = 0;
  // 引用序列化数据
  newn->serdata = ddsi_serdata_ref(serdata);
  // 初始化下一个序列节点为 NULL
  newn->next_seq = NULL;
  // 设置前一个序列节点为 whc 的最大序列号节点
  newn->prev_seq = whc->maxseq_node;
  // 更新前一个序列节点的 next_seq 指针
  if (newn->prev_seq) newn->prev_seq->next_seq = newn;
  // 更新 whc 的最大序列号节点为新节点
  whc->maxseq_node = newn;

  // 计算新节点的大小
  newn->size = whcn_size(whc, newn);
  // 更新 whc 的总字节数
  whc->total_bytes += newn->size;
  // 设置新节点的总字节数
  newn->total_bytes = whc->total_bytes;
  // 如果新节点未确认，更新 whc 的未确认字节数
  if (newn->unacked) whc->unacked_bytes += newn->size;

#ifdef DDS_HAS_LIFESPAN
  // 设置新节点的生命周期到期时间
  newn->lifespan.t_expire = exp;
#endif

  // 将新节点插入到哈希表中
  insert_whcn_in_hash(whc, newn);

  // 处理 whc 的 open_intv
  if (whc->open_intv->first == NULL) {
    // 如果 open_intv 为空，重置 open_intv
    whc->open_intv->min = seq;
    whc->open_intv->maxp1 = seq + 1;
    whc->open_intv->first = whc->open_intv->last = newn;
  } else if (whc->open_intv->maxp1 == seq) {
    // 没有间隙，将新节点添加到 open_intv
    whc->open_intv->last = newn;
    whc->open_intv->maxp1++;
  } else {
    // 有间隙，需要新的 open_intv
    struct whc_intvnode *intv1;
    ddsrt_avl_ipath_t path;
    intv1 = ddsrt_malloc(sizeof(*intv1));
    intv1->min = seq;
    intv1->maxp1 = seq + 1;
    intv1->first = intv1->last = newn;
    if (ddsrt_avl_lookup_ipath(&whc_seq_treedef, &whc->seq, &seq, &path) != NULL) assert(0);
    ddsrt_avl_insert_ipath(&whc_seq_treedef, &whc->seq, intv1, &path);
    whc->open_intv = intv1;
  }

  // 更新 whc 的序列大小
  whc->seq_size++;
#ifdef DDS_HAS_LIFESPAN
  // 注册新节点的生命周期
  ddsi_lifespan_register_sample_locked(&whc->lifespan, &newn->lifespan);
#endif
  // 返回新插入的节点指针
  return newn;
}

/**
 * @brief 将数据插入到写历史缓存中（Write History Cache）。
 *
 * @param[in] whc_generic 写历史缓存的通用结构指针。
 * @param[in] max_drop_seq 允许丢弃的最大序列号。
 * @param[in] seq 要插入的数据的序列号。
 * @param[in] exp 数据的过期时间。
 * @param[in] serdata 要插入的序列化数据。
 * @param[in] tk 序列化数据对应的主题实例。
 * @return 成功时返回0，失败时返回错误代码。
 */
static int whc_default_insert(struct ddsi_whc *whc_generic,
                              ddsi_seqno_t max_drop_seq,
                              ddsi_seqno_t seq,
                              ddsrt_mtime_t exp,
                              struct ddsi_serdata *serdata,
                              struct ddsi_tkmap_instance *tk) {
  // 将通用结构转换为具体的whc_impl结构
  struct whc_impl *const whc = (struct whc_impl *)whc_generic;
  struct dds_whc_default_node *newn = NULL;
  struct whc_idxnode *idxn;
  union {
    struct whc_idxnode idxn;
    char pad[sizeof(struct whc_idxnode) + sizeof(struct dds_whc_default_node *)];
  } template;

  // 加锁以确保线程安全
  ddsrt_mutex_lock(&whc->lock);
  check_whc(whc);

  // 记录日志
  if (whc->gv->logconfig.c.mask & DDS_LC_WHC) {
    struct ddsi_whc_state whcst;
    get_state_locked(whc, &whcst);
    TRACE("whc_default_insert(%p max_drop_seq %" PRIu64 " seq %" PRIu64 " exp %" PRId64
          " serdata %p:%" PRIx32 ")\n",
          (void *)whc, max_drop_seq, seq, exp.v, (void *)serdata, serdata->hash);
    TRACE("  whc: [%" PRIu64 ",%" PRIu64 "] max_drop_seq %" PRIu64 " h %" PRIu32 " tl %" PRIu32
          "\n",
          whcst.min_seq, whcst.max_seq, whc->max_drop_seq, whc->wrinfo.hdepth, whc->wrinfo.tldepth);
  }

  // 检查序列号是否有效
  assert(max_drop_seq < DDSI_MAX_SEQ_NUMBER);
  assert(max_drop_seq >= whc->max_drop_seq);

  // 序列号必须大于当前存储的序列号
  assert(whc->seq_size == 0 || seq > whc->maxseq_node->common.seq);

  // 在序列管理中插入新节点
  newn = whc_default_insert_seq(whc, max_drop_seq, seq, exp, serdata);

  TRACE("  whcn %p:", (void *)newn);

  // 空数据（如提交消息）不能进入索引，如果我们不维护索引，也可以完成
  if (serdata->kind == SDK_EMPTY) {
    TRACE(" empty or no hist\n");
    ddsrt_mutex_unlock(&whc->lock);
    return 0;
  }

  // 查找索引节点
  template.idxn.iid = tk->m_iid;
  if ((idxn = ddsrt_hh_lookup(whc->idx_hash, &template)) != NULL) {
    TRACE(" idxn %p", (void *)idxn);
    // 如果是注销操作，则从索引中删除实例，否则在历史记录中添加/覆盖
    if (serdata->statusinfo & DDSI_STATUSINFO_UNREGISTER) {
      TRACE(" unreg:delete\n");
      delete_one_instance_from_idx(whc, max_drop_seq, idxn);
      if (newn->common.seq <= max_drop_seq) {
        struct dds_whc_default_node *prev_seq = newn->prev_seq;
        TRACE(" unreg:seq <= max_drop_seq: delete newn\n");
        whc_delete_one(whc, newn);
        whc->maxseq_node = prev_seq;
      }
    } else {
#ifdef DDS_HAS_DEADLINE_MISSED
      ddsi_deadline_renew_instance_locked(&whc->deadline, &idxn->deadline);
#endif
      // 处理非注销操作的情况
      if (whc->wrinfo.idxdepth > 0) {
        struct dds_whc_default_node *oldn;
        if (++idxn->headidx == whc->wrinfo.idxdepth) idxn->headidx = 0;
        if ((oldn = idxn->hist[idxn->headidx]) != NULL) {
          TRACE(" overwrite whcn %p", (void *)oldn);
          oldn->idxnode = NULL;
        }
        idxn->hist[idxn->headidx] = newn;
        newn->idxnode = idxn;
        newn->idxnode_pos = idxn->headidx;

        // 如果需要，删除旧节点
        if (oldn && (whc->wrinfo.hdepth > 0 || oldn->common.seq <= max_drop_seq) &&
            (!whc->wrinfo.is_transient_local || whc->wrinfo.tldepth > 0)) {
          TRACE(" prune whcn %p", (void *)oldn);
          assert(oldn != whc->maxseq_node || whc->wrinfo.has_deadline);
          whc_delete_one(whc, oldn);
          if (oldn == whc->maxseq_node) whc->maxseq_node = whc_findmax_procedurally(whc);
        }

        // 处理特殊情况：当新样本被自动确认（因为没有可靠的读者）时，丢弃超出T-L历史的所有内容，并且保持最后的T-L历史比保持最后的常规历史更浅（正常路径通过修剪处理这种情况）
        if (seq <= max_drop_seq && whc->wrinfo.tldepth > 0 &&
            whc->wrinfo.idxdepth > whc->wrinfo.tldepth) {
          uint32_t pos = idxn->headidx + whc->wrinfo.idxdepth - whc->wrinfo.tldepth;
          if (pos >= whc->wrinfo.idxdepth) pos -= whc->wrinfo.idxdepth;
          if ((oldn = idxn->hist[pos]) != NULL) {
            TRACE(" prune tl whcn %p", (void *)oldn);
            assert(oldn != whc->maxseq_node);
            whc_delete_one(whc, oldn);
          }
        }
        TRACE("\n");
      }
    }
  } else {
    TRACE(" newkey");
    // 忽略注销操作，但插入其他所有内容
    if (!(serdata->statusinfo & DDSI_STATUSINFO_UNREGISTER)) {
      idxn = ddsrt_malloc(sizeof(*idxn) + whc->wrinfo.idxdepth * sizeof(idxn->hist[0]));
      TRACE(" idxn %p", (void *)idxn);
      ddsi_tkmap_instance_ref(tk);
      idxn->iid = tk->m_iid;
      idxn->tk = tk;
      idxn->prune_seq = 0;
      idxn->headidx = 0;
      if (whc->wrinfo.idxdepth > 0) {
        idxn->hist[0] = newn;
        for (uint32_t i = 1; i < whc->wrinfo.idxdepth; i++) idxn->hist[i] = NULL;
        newn->idxnode = idxn;
        newn->idxnode_pos = 0;
      }
      ddsrt_hh_add_absent(whc->idx_hash, idxn);
#ifdef DDS_HAS_DEADLINE_MISSED
      ddsi_deadline_register_instance_locked(&whc->deadline, &idxn->deadline,
                                             ddsrt_time_monotonic());
#endif
    } else {
      TRACE(" unreg:skip");
      if (newn->common.seq <= max_drop_seq) {
        struct dds_whc_default_node *prev_seq = newn->prev_seq;
        TRACE(" unreg:seq <= max_drop_seq: delete newn\n");
        whc_delete_one(whc, newn);
        whc->maxseq_node = prev_seq;
      }
    }
    TRACE("\n");
  }
  ddsrt_mutex_unlock(&whc->lock);
  return 0;
}

/**
 * @brief 为借用的样本填充数据
 *
 * @param[out] sample 借用的样本结构体指针
 * @param[in] whcn dds_whc_default_node 结构体指针
 */
static void make_borrowed_sample(struct ddsi_whc_borrowed_sample *sample,
                                 struct dds_whc_default_node *whcn) {
  // 断言：whcn->borrowed 不为真，确保节点没有被借用
  assert(!whcn->borrowed);

  // 设置节点为已借用状态
  whcn->borrowed = 1;

  // 将 whcn 的序列号赋值给 sample 的 seq
  sample->seq = whcn->common.seq;

  // 将 whcn 的 serdata 赋值给 sample 的 serdata
  sample->serdata = whcn->serdata;

  // 将 whcn 的 unacked 赋值给 sample 的 unacked
  sample->unacked = whcn->unacked;

  // 将 whcn 的 rexmit_count 赋值给 sample 的 rexmit_count
  sample->rexmit_count = whcn->rexmit_count;

  // 将 whcn 的 last_rexmit_ts 赋值给 sample 的 last_rexmit_ts
  sample->last_rexmit_ts = whcn->last_rexmit_ts;
}

/**
 * @brief 借用一个样本
 *
 * @param[in] whc_generic ddsi_whc 结构体指针
 * @param[in] seq 序列号
 * @param[out] sample 借用的样本结构体指针
 * @return bool 如果找到并成功借用样本，返回 true，否则返回 false
 */
static bool whc_default_borrow_sample(const struct ddsi_whc *whc_generic,
                                      ddsi_seqno_t seq,
                                      struct ddsi_whc_borrowed_sample *sample) {
  // 将 whc_generic 转换为 whc_impl 类型的指针
  const struct whc_impl *const whc = (const struct whc_impl *)whc_generic;

  // 定义一个 dds_whc_default_node 类型的指针
  struct dds_whc_default_node *whcn;

  // 定义一个布尔变量表示是否找到样本
  bool found;

  // 对 whc 的锁进行加锁操作
  ddsrt_mutex_lock((ddsrt_mutex_t *)&whc->lock);

  // 在 whc 中查找序列号为 seq 的节点
  if ((whcn = whc_findseq(whc, seq)) == NULL)
    // 如果没有找到，设置 found 为 false
    found = false;
  else {
    // 如果找到了，调用 make_borrowed_sample 函数填充 sample 数据
    make_borrowed_sample(sample, whcn);

    // 设置 found 为 true
    found = true;
  }

  // 对 whc 的锁进行解锁操作
  ddsrt_mutex_unlock((ddsrt_mutex_t *)&whc->lock);

  // 返回 found
  return found;
}
/**
 * @brief 从 whc_generic 中借用一个样本的键值。
 *
 * @param[in] whc_generic 指向 ddsi_whc 结构体的指针。
 * @param[in] serdata_key 指向 ddsi_serdata 结构体的指针，表示要查找的键值。
 * @param[out] sample 指向 ddsi_whc_borrowed_sample 结构体的指针，用于存储找到的样本信息。
 * @return 如果找到了对应的样本，则返回 true；否则返回 false。
 */
static bool whc_default_borrow_sample_key(const struct ddsi_whc *whc_generic,
                                          const struct ddsi_serdata *serdata_key,
                                          struct ddsi_whc_borrowed_sample *sample) {
  // 将 whc_generic 转换为 whc_impl 类型的指针
  const struct whc_impl *const whc = (const struct whc_impl *)whc_generic;
  struct dds_whc_default_node *whcn;
  bool found;

  // 对 whc->lock 进行加锁操作
  ddsrt_mutex_lock((ddsrt_mutex_t *)&whc->lock);

  // 在 whc 中查找与 serdata_key 匹配的节点
  if ((whcn = whc_findkey(whc, serdata_key)) == NULL)
    found = false;
  else {
    // 创建一个借用的样本
    make_borrowed_sample(sample, whcn);
    found = true;
  }

  // 对 whc->lock 进行解锁操作
  ddsrt_mutex_unlock((ddsrt_mutex_t *)&whc->lock);

  return found;
}

/**
 * @brief 在锁定状态下归还样本。
 *
 * @param[in] whc 指向 whc_impl 结构体的指针。
 * @param[in] sample 指向 ddsi_whc_borrowed_sample 结构体的指针，表示要归还的样本。
 * @param[in] update_retransmit_info 是否更新重传信息。
 */
static void return_sample_locked(struct whc_impl *whc,
                                 struct ddsi_whc_borrowed_sample *sample,
                                 bool update_retransmit_info) {
  struct dds_whc_default_node *whcn;

  // 在 whc 中查找与 sample->seq 匹配的节点
  if ((whcn = whc_findseq(whc, sample->seq)) == NULL) {
    // 数据不再存在于 WHC 中
    ddsi_serdata_unref(sample->serdata);
  } else {
    assert(whcn->borrowed);
    whcn->borrowed = 0;
    if (update_retransmit_info) {
      whcn->rexmit_count = sample->rexmit_count;
      whcn->last_rexmit_ts = sample->last_rexmit_ts;
    }
  }
}

/**
 * @brief 归还从 whc_generic 借用的样本。
 *
 * @param[in] whc_generic 指向 ddsi_whc 结构体的指针。
 * @param[in] sample 指向 ddsi_whc_borrowed_sample 结构体的指针，表示要归还的样本。
 * @param[in] update_retransmit_info 是否更新重传信息。
 */
static void whc_default_return_sample(struct ddsi_whc *whc_generic,
                                      struct ddsi_whc_borrowed_sample *sample,
                                      bool update_retransmit_info) {
  // 将 whc_generic 转换为 whc_impl 类型的指针
  struct whc_impl *const whc = (struct whc_impl *)whc_generic;

  // 对 whc->lock 进行加锁操作
  ddsrt_mutex_lock(&whc->lock);

  // 在锁定状态下归还样本
  return_sample_locked(whc, sample, update_retransmit_info);

  // 对 whc->lock 进行解锁操作
  ddsrt_mutex_unlock(&whc->lock);
}

/**
 * @brief 初始化 whc_generic 的样本迭代器。
 *
 * @param[in] whc_generic 指向 ddsi_whc 结构体的指针。
 * @param[out] opaque_it 指向 ddsi_whc_sample_iter 结构体的指针，用于存储初始化后的迭代器信息。
 */
static void whc_default_sample_iter_init(const struct ddsi_whc *whc_generic,
                                         struct ddsi_whc_sample_iter *opaque_it) {
  // 将 opaque_it 转换为 ddsi_whc_sample_iter_impl 类型的指针
  struct ddsi_whc_sample_iter_impl *it = (struct ddsi_whc_sample_iter_impl *)opaque_it;

  // 设置迭代器的 whc 属性
  it->c.whc = (struct ddsi_whc *)whc_generic;

  // 设置迭代器的 first 属性为 true
  it->first = true;
}
/**
 * @brief 从写历史缓存中借用下一个样本
 *
 * @param[in] opaque_it 指向ddsi_whc_sample_iter结构的指针，用于迭代写历史缓存中的样本
 * @param[out] sample 指向ddsi_whc_borrowed_sample结构的指针，用于存储借用的样本信息
 * @return bool 如果成功找到并借用下一个样本，则返回true，否则返回false
 */
static bool whc_default_sample_iter_borrow_next(struct ddsi_whc_sample_iter *opaque_it,
                                                struct ddsi_whc_borrowed_sample *sample) {
  // 将opaque_it转换为ddsi_whc_sample_iter_impl类型的指针
  struct ddsi_whc_sample_iter_impl *const it = (struct ddsi_whc_sample_iter_impl *)opaque_it;

  // 将it->c.whc转换为whc_impl类型的指针
  struct whc_impl *const whc = (struct whc_impl *)it->c.whc;

  // 定义一个指向dds_whc_default_node结构的指针
  struct dds_whc_default_node *whcn;

  // 定义一个指向whc_intvnode结构的指针
  struct whc_intvnode *intv;

  // 定义一个序列号变量
  ddsi_seqno_t seq;

  // 定义一个布尔变量表示样本是否有效
  bool valid;

  // 对whc的锁进行加锁操作
  ddsrt_mutex_lock(&whc->lock);

  // 检查写历史缓存的状态
  check_whc(whc);

  // 如果不是第一个样本
  if (!it->first) {
    // 获取当前样本的序列号
    seq = sample->seq;

    // 返回已锁定的样本
    return_sample_locked(whc, sample, false);
  } else {
    // 设置迭代器的first标志为false
    it->first = false;

    // 将序列号设置为0
    seq = 0;
  }

  // 查找下一个序列号对应的间隔节点
  if ((whcn = find_nextseq_intv(&intv, whc, seq)) == NULL)
    // 如果未找到，则将valid设置为false
    valid = false;
  else {
    // 根据找到的节点创建借用的样本
    make_borrowed_sample(sample, whcn);

    // 将valid设置为true
    valid = true;
  }

  // 对whc的锁进行解锁操作
  ddsrt_mutex_unlock(&whc->lock);

  // 返回valid值，表示是否成功借用了下一个样本
  return valid;
}
