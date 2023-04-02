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
#include <limits.h>
#include <string.h>

#if HAVE_VALGRIND && !defined(NDEBUG)
#include <memcheck.h>
#define USE_VALGRIND 1
#else
#define USE_VALGRIND 0
#endif

#include "dds/ddsc/dds_rhc.h"
#include "dds/ddsi/ddsi_domaingv.h"
#include "dds/ddsi/ddsi_entity.h"
#include "dds/ddsi/ddsi_radmin.h" /* sampleinfo */
#include "dds/ddsi/ddsi_rhc.h"
#include "dds/ddsi/ddsi_serdata.h"
#include "dds/ddsi/ddsi_tkmap.h"
#include "dds/ddsi/ddsi_unused.h"
#include "dds/ddsi/ddsi_xqos.h"
#include "dds/ddsrt/avl.h"
#include "dds/ddsrt/circlist.h"
#include "dds/ddsrt/heap.h"
#include "dds/ddsrt/hopscotch.h"
#include "dds/ddsrt/sync.h"
#include "dds/ddsrt/time.h"
#include "dds__entity.h"
#include "dds__reader.h"
#include "dds__rhc_default.h"
#ifdef DDS_HAS_LIFESPAN
#include "dds/ddsi/ddsi_lifespan.h"
#endif
#ifdef DDS_HAS_DEADLINE_MISSED
#include "dds/ddsi/ddsi_deadline.h"
#endif

/* 实例管理
   ===================

   实例由 "write" 和 "dispose" 隐式创建，由 "unregister" 取消注册。
   有效样本仅通过写操作添加（可能与 dispose 和/或 unregister 结合），
   无效样本仅通过 dispose 和 unregister 操作添加，且仅当没有样本或最新可用样本已读时。
   （在某些情况下，如果有人只取多个有效样本的最新一个，这可能有点奇怪。）

   每个实例最多有一个无效样本，其样本信息直接从实例返回给读取器，
   并且其存在和 sample_state 由两个位表示。任何传入样本（或“传入无效样本”）
   都会导致现有无效样本被丢弃。因此，无效样本仅用于在没有样本的情况下发出实例状态更改信号。

   （注意：这可以很容易地更改为始终在 dispose/unregister 上生成无效样本。）

   实例和 RHC 整体跟踪有效样本数量、已读有效样本数量以及相同的无效样本数量，
   其中 RHC 中的 "inv_exists" 和 "inv_isread" 布尔值同时充当标志和计数器。

   当样本数量（有效和无效样本组合）和注册数量都变为 0 时，将删除实例。
   注册数量在 "wrcount" 中跟踪，最近写入者的唯一标识符通常在 "wr_iid" 中。
   通常，因为通过 "wr_iid" 注销会清除它。原则上，实际的注册集是存储在 "registrations" 中的
<instance,writer> 元组集， 但从中排除了那些具有 "wrcount" = 1 和 "wr_iid" != 0
的实例。典型情况是一个实例有一个活动写入者， 这意味着典型情况下没有 <instance,writer> 元组在
"registrations" 中。

   不幸的是，这种模型使得从 1 个写入者到 2 个写入者的转换以及从 2 个写入者回到 1 个写入者变得复杂。
   对于不必在单个（或单个主导）写入者的情况下执行任何操作所带来的显著性能提升，这似乎是合理的代价。

   （注意："registrations" 可能会移动到全局注册表
   的 <reader,instance,writer> 元组中，使用无锁哈希表，但这不影响模型。）

   实例和写入者的唯一标识符大致均匀地从正无符号 64 位整数集中抽取。
   这意味着它们是优秀的哈希键，实例哈希表和写入者注册哈希表都直接使用这些键。

   QOS 支持
   ===========

   历史记录实现为（循环）链表，但这里实现的无效样本模型允许将其轻松更改为样本数组，
   对于浅层历史记录，这可能是有利的。目前，实例具有一个嵌入式单个样本，特别是为了优化 KEEP_LAST
深度=1 的情况。

   BY_SOURCE 排序与 OpenSplice 的实现不同，不执行历史记录的回填。反对此观点的论据可以在 JIRA
中找到， 但简而言之：（1）不回填要简单得多（因此更快），（2）回填可能需要重写已读取的样本的状态，
   （3）它与“最终一致性”一样，“唯一的区别在于，这里实现的模型认为数据空间基本上是“保留最后一个”，
   并始终向前移动（源时间戳增加），而每个读者的历史记录是读者对该数据空间进行采样的记录，
   而使用回填，模型是最终一致性适用于完整历史记录。

   （事实上，这里实现的模型也是 RTI 和其他可能的实现所使用的模型 - 在这方面，OpenSplice 是个例外。）

   独占所有权是通过丢弃除“wr_iid”之外的所有写入器的所有数据来实现的，
   除非“wr_iid”为0或者到达样本的强度高于实例的当前强度（在“strength”中）。
   写入器 ID 仅通过取消注册重置，在这种情况下，所有权再次被抢占是自然的。
   QoS 更改（在此 DDSI
实现中不支持，但仍然）将通过在独占所有权写入器降低其强度时也重置“wr_iid”来完成。

   生命周期基于接收时间戳，如果将此 QoS 设置为其他值而不是无限，则使用单调时间进行样本过期。

   读取条件
   ===============

   当前，读取条件*始终*附加到读取器上，创建读取条件并将其附加到等待集上有点浪费资源。
   当然，这可以更改，但很怀疑许多读取条件会在实际使用之前创建。

   读取条件的“触发器”计算匹配其条件的实例数，并在实例状态和/或样本状态更改时同步更新。
   实例/样本状态简化为表示实例和视图状态的位掩码三元组，无论实例是否具有未读取的样本，
   以及是否具有已读取的样本。（包括无效样本。）将这两个三元组，更改前和更改后传递给“update_conditions_locked”，
   然后运行附加的读取条件数组并更新触发器。它返回触发器是否从 0 更改为 1，
   因为这表示必须向附加的等待集发出信号。
+*/

/* FIXME: tkmap 应该保留设置为无效的时间戳的数据
   无效的时间戳在逻辑上与有效的时间戳是无序的，这意味着即使在使用 tkmap 数据为
   取消注册消息生成无效样本时，也可以遵循 BY_SOURCE 顺序。 */

#define MAX_ATTACHED_QUERYCONDS (CHAR_BIT * sizeof(dds_querycond_mask_t))

#define INCLUDE_TRACE 1
#if INCLUDE_TRACE
#define TRACE(...) DDS_CLOG(DDS_LC_RHC, &rhc->gv->logconfig, __VA_ARGS__)
#else
#define TRACE(...) ((void)0)
#endif

/******************************
 ******   LIVE WRITERS   ******
 ******************************/

// 定义一个名为lwreg的结构体，包含两个uint64_t类型的成员变量：iid和wr_iid
struct lwreg {
  uint64_t iid;
  uint64_t wr_iid;
};

// 定义一个名为lwregs的结构体，包含一个指向ddsrt_ehh类型的指针regs
struct lwregs {
  struct ddsrt_ehh *regs;
};

// 定义一个名为lwreg_hash的静态函数，输入参数为void指针vl，返回值类型为uint32_t
static uint32_t lwreg_hash(const void *vl) {
  // 将输入参数vl转换为lwreg结构体指针l
  const struct lwreg *l = vl;
  // 返回l中iid与wr_iid异或后的结果
  return (uint32_t)(l->iid ^ l->wr_iid);
}

// 定义一个名为lwreg_equals的静态函数，输入参数为两个void指针va和vb，返回值类型为int
static int lwreg_equals(const void *va, const void *vb) {
  // 将输入参数va和vb分别转换为lwreg结构体指针a和b
  const struct lwreg *a = va;
  const struct lwreg *b = vb;
  // 如果a和b的iid相等且wr_iid相等，则返回1，否则返回0
  return a->iid == b->iid && a->wr_iid == b->wr_iid;
}

// 定义一个名为lwregs_init的静态函数，输入参数为lwregs结构体指针rt，无返回值
static void lwregs_init(struct lwregs *rt) {
  // 将rt中的regs指针设置为NULL
  rt->regs = NULL;
}

// 定义一个名为lwregs_fini的静态函数，输入参数为lwregs结构体指针rt，无返回值
static void lwregs_fini(struct lwregs *rt) {
  // 如果rt中的regs指针不为NULL，则释放该内存空间
  if (rt->regs) ddsrt_ehh_free(rt->regs);
}

// 定义一个名为lwregs_contains的静态函数，输入参数为lwregs结构体指针rt、uint64_t类型的iid和wr_iid，返回值类型为int
static int lwregs_contains(struct lwregs *rt, uint64_t iid, uint64_t wr_iid) {
  // 创建一个名为dummy的lwreg结构体实例，并初始化其iid和wr_iid成员变量
  struct lwreg dummy = {.iid = iid, .wr_iid = wr_iid};
  // 如果rt中的regs指针不为NULL且在regs中查找到dummy，则返回1，否则返回0
  return rt->regs != NULL && ddsrt_ehh_lookup(rt->regs, &dummy) != NULL;
}

// 定义一个名为lwregs_add的静态函数，输入参数为lwregs结构体指针rt、uint64_t类型的iid和wr_iid，返回值类型为int
static int lwregs_add(struct lwregs *rt, uint64_t iid, uint64_t wr_iid) {
  // 创建一个名为dummy的lwreg结构体实例，并初始化其iid和wr_iid成员变量
  struct lwreg dummy = {.iid = iid, .wr_iid = wr_iid};
  // 如果rt中的regs指针为NULL，则创建一个新的ddsrt_ehh实例并赋值给rt->regs
  if (rt->regs == NULL) rt->regs = ddsrt_ehh_new(sizeof(struct lwreg), 1, lwreg_hash, lwreg_equals);
  // 将dummy添加到rt->regs中，并返回操作结果
  return ddsrt_ehh_add(rt->regs, &dummy);
}

// 定义一个名为lwregs_delete的静态函数，输入参数为lwregs结构体指针rt、uint64_t类型的iid和wr_iid，返回值类型为int
static int lwregs_delete(struct lwregs *rt, uint64_t iid, uint64_t wr_iid) {
  // 创建一个名为dummy的lwreg结构体实例，并初始化其iid和wr_iid成员变量
  struct lwreg dummy = {.iid = iid, .wr_iid = wr_iid};
  // 如果rt中的regs指针不为NULL且成功从regs中移除dummy，则返回1，否则返回0
  return rt->regs != NULL && ddsrt_ehh_remove(rt->regs, &dummy);
}

#if 0
void lwregs_dump (struct lwregs *rt)
{
  struct ddsrt_ehh_iter it;
  for (struct lwreg *r = ddsrt_ehh_iter_first(rt->regs, &it); r; r = ddsrt_ehh_iter_next(&it))
    printf("iid=%"PRIu64" wr_iid=%"PRIu64"\n", r->iid, r->wr_iid);
}
#endif

/*************************
 ******     RHC     ******
 *************************/
// 定义rhc_sample结构体
struct rhc_sample {
  struct ddsi_serdata *sample;  // 序列化数据（仅包含键值或实际数据）
  struct rhc_sample *next;  // 时间顺序中的下一个样本，或者最旧的样本（如果是最近的）
  uint64_t wr_iid;             // 此样本的写入者的唯一ID（可能更适合在serdata中）
  dds_querycond_mask_t conds;  // 匹配的查询条件
  bool isread;                 // 样本状态为READ或NOT_READ
  uint32_t disposed_gen;       // 插入时实例计数器的快照
  uint32_t no_writers_gen;     // __/
#ifdef DDS_HAS_LIFESPAN
  struct ddsi_lifespan_fhnode lifespan;  // 生命周期的fibheap节点
  struct rhc_instance *inst;             // 指向rhc实例的引用
#endif
};

// 定义rhc_instance结构体
struct rhc_instance {
  uint64_t iid;  // 唯一实例ID，表的键，也作为实例句柄
  uint64_t wr_iid;  // 最新样本的写入者的唯一ID或0；如果wrcount = 0，则是导致的wr_iid
  struct rhc_sample *latest;  // 收到的最新样本；从旧到新的循环列表；如果没有样本则为空
  uint32_t nvsamples;  // 实例中的“有效”样本数量
  uint32_t nvread;  // 实例中已读取的“有效”样本数量（0 <= nvread <= nvsamples）
  dds_querycond_mask_t conds;  // 匹配的查询条件
  uint32_t wrcount;            // 活跃写入者的数量
  unsigned isnew : 1;          // 视图状态为NEW或NOT_NEW
  unsigned a_sample_free : 1;  // a_sample是否可用
  unsigned
      isdisposed : 1;  // 处理状态为DISPOSED或NOT_DISPOSED（如果未处理，wrcount决定ALIVE/NOT_ALIVE_NO_WRITERS）
  unsigned autodispose : 1;  // wrcount > 0 => 至少有一个注册的写入者在某次更新时设置了自动处理
  unsigned wr_iid_islive : 1;  // wr_iid是否属于活跃的写入者
  unsigned inv_exists : 1;  // 自上次样本以来是否发生了状态更改（即，必须返回无效样本）
  unsigned inv_isread : 1;  // 该状态更改之前是否已读取
  unsigned deadline_reg : 1;  // 是否已注册截止日期（== isdisposed，除非store()推迟更新）
  uint32_t disposed_gen;    // 世代计数器 - 人类最糟糕的发明
  uint32_t no_writers_gen;  // __/
  int32_t strength;         // “当前”所有权强度
  ddsi_guid_t
      wr_guid;  // 最后一个写入者的guid（如果wr_iid != 0，则wr_guid是相应的guid，否则未定义）
  ddsrt_wctime_t tstamp;                     // 上次更新的源时间戳
  struct ddsrt_circlist_elem nonempty_list;  // 非空实例的任意排序链接
#ifdef DDS_HAS_DEADLINE_MISSED
  struct deadline_elem deadline;             // 截止日期错过管理中的元素
#endif
  struct ddsi_tkmap_instance *tk;            // TK的反向引用，用于取消引用
  struct rhc_sample a_sample;                // 为1个样本预分配的存储空间
};

// 定义rhc_store_result枚举类型
typedef enum rhc_store_result {
  RHC_STORED,    // 存储成功
  RHC_FILTERED,  // 被过滤
  RHC_REJECTED   // 被拒绝
} rhc_store_result_t;
// 定义 dds_rhc_default 结构体
struct dds_rhc_default {
  // 公共的 dds_rhc 结构体
  struct dds_rhc common;
  // 实例哈希表
  struct ddsrt_hh *instances;
  // 非空实例的循环列表，指向最近添加的一个，如果没有则为 NULL
  struct ddsrt_circlist nonempty_instances;
  // 注册表，应该是全局的（使用无锁查找）
  struct lwregs registrations;

  // 来自资源限制 QoS 的实例/样本最大值

  // 最大实例数，FIXME: 用 uint32_t 和 MAX_UINT32 表示无限可能更好
  int32_t max_instances;
  // 最大样本数，FIXME: 用 uint32_t 和 MAX_UINT32 表示无限可能更好
  int32_t max_samples;
  // 每个实例的最大样本数，FIXME: 用 uint32_t 和 MAX_UINT32 表示无限可能更好
  int32_t max_samples_per_instance;
  // 来自 time_based_filter QoSPolicy 的最小间隔时间
  dds_duration_t minimum_separation;

  // 包括空实例在内的实例数量
  uint32_t n_instances;
  // 非空实例数量
  uint32_t n_nonempty_instances;
  // 已处理的非空实例数量
  uint32_t n_not_alive_disposed;
  // 无作者的非活动实例数量
  uint32_t n_not_alive_no_writers;
  // 新的非空实例数量
  uint32_t n_new;
  // 所有实例中的 "有效" 样本数量
  uint32_t n_vsamples;
  // 所有实例中已读 "有效" 样本数量
  uint32_t n_vread;
  // 所有实例中的无效样本数量
  uint32_t n_invsamples;
  // 所有实例中已读无效样本数量
  uint32_t n_invread;

  // 如果按源排序则为 true，按接收排序则为 false
  bool by_source_ordering;
  // 如果独占所有权则为 true，共享所有权则为 false
  bool exclusive_ownership;
  // 如果可靠性为 RELIABLE 则为 true
  bool reliable;
  // 是否在检查时执行昂贵的检查
  bool xchecks;

  // 读取器，可能为 NULL（由 rhc_torture 使用）
  dds_reader *reader;
  // 指向 tkmap 的反向指针
  struct ddsi_tkmap *tkmap;
  // 全局变量，目前仅用于日志配置
  struct ddsi_domaingv *gv;
  // 类型描述
  const struct ddsi_sertype *type;
  // 历史深度，对于 KEEP_LAST_1 为 1，对于 KEEP_ALL 为 2**32-1
  uint32_t history_depth;

  // 互斥锁
  ddsrt_mutex_t lock;
  // 关联的读取条件列表
  dds_readcond *conds;
  // 关联的读取条件数量
  uint32_t nconds;
  // 关联的查询条件数量
  uint32_t nqconds;
  // 检查样本状态的关联查询条件的掩码
  dds_querycond_mask_t qconds_samplest;
  // 评估查询条件的临时存储，如果没有 qconds 则为 NULL
  void *qcond_eval_samplebuf;
#ifdef DDS_HAS_LIFESPAN
  // 生命周期管理
  struct ddsi_lifespan_adm lifespan;
#endif
#ifdef DDS_HAS_DEADLINE_MISSED
  // 截止日期未达成管理
  struct ddsi_deadline_adm deadline;
#endif
};
// 定义 trigger_info_cmn 结构体
struct trigger_info_cmn {
  uint32_t qminst;    // 最小实例数量
  bool has_read;      // 是否有已读取的样本
  bool has_not_read;  // 是否有未读取的样本
};

// 定义 trigger_info_pre 结构体
struct trigger_info_pre {
  struct trigger_info_cmn c;  // 包含一个 trigger_info_cmn 结构体
};

// 定义 trigger_info_qcond 结构体
struct trigger_info_qcond {
  // 下面的布尔值表示在添加/删除有效/无效样本时，是否需要更新条件状态
  bool dec_invsample_read;
  bool dec_sample_read;
  bool inc_invsample_read;
  bool inc_sample_read;

  // 条件状态掩码，用于表示哪些查询条件需要更新
  dds_querycond_mask_t dec_conds_invsample;
  dds_querycond_mask_t dec_conds_sample;
  dds_querycond_mask_t inc_conds_invsample;
  dds_querycond_mask_t inc_conds_sample;
};

// 定义 trigger_info_post 结构体
struct trigger_info_post {
  struct trigger_info_cmn c;  // 包含一个 trigger_info_cmn 结构体
};

// 声明 dds_rhc_default_ops 结构体常量
static const struct dds_rhc_ops dds_rhc_default_ops;

// 定义 qmask_of_sample 函数，根据给定的 rhc_sample 结构体返回相应的查询掩码
static uint32_t qmask_of_sample(const struct rhc_sample *s) {
  return s->isread ? DDS_READ_SAMPLE_STATE : DDS_NOT_READ_SAMPLE_STATE;
}

// 定义 qmask_of_invsample 函数，根据给定的 rhc_instance 结构体返回相应的查询掩码
static uint32_t qmask_of_invsample(const struct rhc_instance *i) {
  return i->inv_isread ? DDS_READ_SAMPLE_STATE : DDS_NOT_READ_SAMPLE_STATE;
}

// 定义 inst_nsamples 函数，计算给定的 rhc_instance 结构体中有效样本和无效样本的总数
static uint32_t inst_nsamples(const struct rhc_instance *i) { return i->nvsamples + i->inv_exists; }

// 定义 inst_nread 函数，计算给定的 rhc_instance 结构体中已读取的有效样本和无效样本的总数
static uint32_t inst_nread(const struct rhc_instance *i) {
  return i->nvread + (uint32_t)(i->inv_exists & i->inv_isread);
}
// 判断实例是否为空，即样本数量为0
static bool inst_is_empty(const struct rhc_instance *i) {
  // 返回实例中样本数量是否等于0
  return inst_nsamples(i) == 0;
}

// 判断实例是否已读，即读取的样本数量大于0
static bool inst_has_read(const struct rhc_instance *i) {
  // 返回实例中已读样本数量是否大于0
  return inst_nread(i) > 0;
}

// 判断实例是否有未读样本，即读取的样本数量小于总样本数量
static bool inst_has_unread(const struct rhc_instance *i) {
  // 返回实例中已读样本数量是否小于总样本数量
  return inst_nread(i) < inst_nsamples(i);
}

// 将未类型化的数据转换为干净的无效样本
static bool untyped_to_clean_invsample(const struct ddsi_sertype *type,
                                       const struct ddsi_serdata *d,
                                       void *sample,
                                       void **bufptr,
                                       void *buflim) {
  /* ddsi_serdata_untyped_to_sample 只处理键值，而不关注属性；
     但这会给用户带来困扰：无效样本的属性可能是垃圾，但最终仍需要释放。
     显式地将其置零可以解决这个问题。 */
  // 释放样本的内容
  ddsi_sertype_free_sample(type, sample, DDS_FREE_CONTENTS);
  // 将样本置零
  ddsi_sertype_zero_sample(type, sample);
  // 将未类型化的数据转换为样本
  return ddsi_serdata_untyped_to_sample(type, d, sample, bufptr, buflim);
}
// 定义静态函数原型
static uint32_t qmask_of_inst(const struct rhc_instance *inst);
static void free_sample(struct dds_rhc_default *rhc,
                        struct rhc_instance *inst,
                        struct rhc_sample *s);
static void get_trigger_info_cmn(struct trigger_info_cmn *info, struct rhc_instance *inst);
static void get_trigger_info_pre(struct trigger_info_pre *info, struct rhc_instance *inst);
static void init_trigger_info_qcond(struct trigger_info_qcond *qc);
static void drop_instance_noupdate_no_writers(struct dds_rhc_default *__restrict rhc,
                                              struct rhc_instance *__restrict *__restrict instptr);
static bool update_conditions_locked(struct dds_rhc_default *rhc,
                                     bool called_from_insert,
                                     const struct trigger_info_pre *pre,
                                     const struct trigger_info_post *post,
                                     const struct trigger_info_qcond *trig_qc,
                                     const struct rhc_instance *inst);
static void account_for_nonempty_to_empty_transition(
    struct dds_rhc_default *__restrict rhc,
    struct rhc_instance *__restrict *__restrict instptr,
    const char *__restrict traceprefix);
#ifndef NDEBUG
static int rhc_check_counts_locked(struct dds_rhc_default *rhc,
                                   bool check_conds,
                                   bool check_qcmask);
#endif

// 实例iid哈希函数
static uint32_t instance_iid_hash(const void *va) {
  const struct rhc_instance *a = va;
  return (uint32_t)a->iid;
}

// 实例iid相等性判断函数
static int instance_iid_eq(const void *va, const void *vb) {
  const struct rhc_instance *a = va;
  const struct rhc_instance *b = vb;
  return (a->iid == b->iid);
}

// 将实例添加到非空列表中
static void add_inst_to_nonempty_list(struct dds_rhc_default *rhc, struct rhc_instance *inst) {
  ddsrt_circlist_append(&rhc->nonempty_instances, &inst->nonempty_list);
  rhc->n_nonempty_instances++;
}

// 从非空列表中移除实例
static void remove_inst_from_nonempty_list(struct dds_rhc_default *rhc, struct rhc_instance *inst) {
  assert(inst_is_empty(inst));
  ddsrt_circlist_remove(&rhc->nonempty_instances, &inst->nonempty_list);
  assert(rhc->n_nonempty_instances > 0);
  rhc->n_nonempty_instances--;
}
// 定义一个名为oldest_nonempty_instance的静态函数，参数为指向dds_rhc_default结构体的指针rhc，
// 返回值类型为指向rhc_instance结构体的指针。
static struct rhc_instance *oldest_nonempty_instance(const struct dds_rhc_default *rhc) {
  // 使用DDSRT_FROM_CIRCLIST宏从循环列表中获取最旧的非空实例，并返回该实例的指针。
  return DDSRT_FROM_CIRCLIST(struct rhc_instance, nonempty_list,
                             ddsrt_circlist_oldest(&rhc->nonempty_instances));
}

// 定义一个名为latest_nonempty_instance的静态函数，参数为指向dds_rhc_default结构体的指针rhc，
// 返回值类型为指向rhc_instance结构体的指针。
static struct rhc_instance *latest_nonempty_instance(const struct dds_rhc_default *rhc) {
  // 使用DDSRT_FROM_CIRCLIST宏从循环列表中获取最新的非空实例，并返回该实例的指针。
  return DDSRT_FROM_CIRCLIST(struct rhc_instance, nonempty_list,
                             ddsrt_circlist_latest(&rhc->nonempty_instances));
}

// 定义一个名为next_nonempty_instance的静态函数，参数为指向rhc_instance结构体的指针inst，
// 返回值类型为指向rhc_instance结构体的指针。
static struct rhc_instance *next_nonempty_instance(const struct rhc_instance *inst) {
  // 使用DDSRT_FROM_CIRCLIST宏从循环列表中获取下一个非空实例，并返回该实例的指针。
  return DDSRT_FROM_CIRCLIST(struct rhc_instance, nonempty_list, inst->nonempty_list.next);
}
#ifdef DDS_HAS_LIFESPAN                      // 如果定义了DDS_HAS_LIFESPAN宏
static void drop_expired_samples(struct dds_rhc_default *rhc, struct rhc_sample *sample) {
  struct rhc_instance *inst = sample->inst;  // 获取样本对应的实例
  struct trigger_info_pre pre;
  struct trigger_info_post post;
  struct trigger_info_qcond trig_qc;

  assert(!inst_is_empty(inst));  // 断言实例不为空

  // 打印跟踪信息
  TRACE("rhc_default %p drop_exp(iid %" PRIx64 " wriid %" PRIx64 " exp %" PRId64 " %s", rhc,
        inst->iid, sample->wr_iid, sample->lifespan.t_expire.v,
        sample->isread ? "read" : "notread");

  get_trigger_info_pre(&pre, inst);   // 获取触发器前置信息
  init_trigger_info_qcond(&trig_qc);  // 初始化触发器条件信息

  /* 查找前一个样本：如果历史深度为1，则该样本本身就是前一个样本（即inst->latest）。
   * 如果历史深度较大，最可能过期的样本是最旧的样本，在这种情况下，inst->latest是前一个
   * 样本，inst->latest->next指向样本（循环列表）。我们可以假设“样本”在列表中，
   * 因此这里不需要检查以避免无限循环。 */
  struct rhc_sample *psample = inst->latest;
  while (psample->next != sample) psample = psample->next;

  rhc->n_vsamples--;                 // 减少有效样本数量
  if (sample->isread)                // 如果样本已读
  {
    inst->nvread--;                  // 减少实例中的已读样本数量
    rhc->n_vread--;                  // 减少已读样本总数
    trig_qc.dec_sample_read = true;  // 设置触发器条件中的减少样本已读标志
  }
  if (--inst->nvsamples > 0)         // 如果实例中还有其他有效样本
  {
    if (inst->latest == sample) inst->latest = psample;
    psample->next = sample->next;
  } else  // 如果实例中没有其他有效样本
  {
    inst->latest = NULL;
  }
  trig_qc.dec_conds_sample = sample->conds;  // 设置触发器条件中的减少样本条件
  free_sample(rhc, inst, sample);            // 释放样本内存
  get_trigger_info_cmn(&post.c, inst);       // 获取触发器公共信息
  update_conditions_locked(rhc, false, &pre, &post, &trig_qc, inst);  // 更新锁定条件
  if (inst_is_empty(inst))                                            // 如果实例为空
    account_for_nonempty_to_empty_transition(rhc, &inst, "; ");       // 处理非空到空的转换
  TRACE(")\n");                                                       // 打印跟踪信息
}

ddsrt_mtime_t dds_rhc_default_sample_expired_cb(void *hc, ddsrt_mtime_t tnow) {
  struct dds_rhc_default *rhc = hc;  // 获取缓存实例
  struct rhc_sample *sample;
  ddsrt_mtime_t tnext;
  ddsrt_mutex_lock(&rhc->lock);         // 锁定互斥量
  while ((tnext = ddsi_lifespan_next_expired_locked(&rhc->lifespan, tnow, (void **)&sample)).v == 0)
    drop_expired_samples(rhc, sample);  // 删除过期样本
  ddsrt_mutex_unlock(&rhc->lock);       // 解锁互斥量
  return tnext;                         // 返回下一个过期时间
}
#endif                                  /* DDS_HAS_LIFESPAN */
#ifdef DDS_HAS_DEADLINE_MISSED          // 如果定义了DDS_HAS_DEADLINE_MISSED宏
ddsrt_mtime_t dds_rhc_default_deadline_missed_cb(void *hc, ddsrt_mtime_t tnow) {
  struct dds_rhc_default *rhc = hc;  // 将传入的void指针转换为dds_rhc_default结构体指针
  void *vinst;
  ddsrt_mtime_t tnext;
  ddsrt_mutex_lock(&rhc->lock);  // 对rhc结构体的互斥锁进行加锁

  // 当tnext等于0时，循环执行以下操作
  while ((tnext = ddsi_deadline_next_missed_locked(&rhc->deadline, tnow, &vinst)).v == 0) {
    struct rhc_instance *inst = vinst;  // 将vinst转换为rhc_instance结构体指针
    // 计算过期的deadlines数量
    uint32_t deadlines_expired =
        inst->deadline.deadlines_missed +
        (uint32_t)((tnow.v - inst->deadline.t_last_update.v) / rhc->deadline.dur);
    // 重新注册实例的deadline
    ddsi_deadline_reregister_instance_locked(&rhc->deadline, &inst->deadline, tnow);

    inst->wr_iid_islive = 0;        // 设置实例的wr_iid_islive为0

    ddsi_status_cb_data_t cb_data;  // 定义一个ddsi_status_cb_data_t类型的变量cb_data
    cb_data.raw_status_id = (int)
        DDS_REQUESTED_DEADLINE_MISSED_STATUS_ID;  // 设置cb_data的raw_status_id为DDS_REQUESTED_DEADLINE_MISSED_STATUS_ID
    cb_data.extra = deadlines_expired;  // 设置cb_data的extra为过期的deadlines数量
    cb_data.handle = inst->iid;         // 设置cb_data的handle为实例的iid
    cb_data.add = true;                 // 设置cb_data的add为true
    ddsrt_mutex_unlock(&rhc->lock);     // 对rhc结构体的互斥锁进行解锁
    dds_reader_status_cb(&rhc->reader->m_entity,
                         &cb_data);     // 调用dds_reader_status_cb函数处理状态回调
    ddsrt_mutex_lock(&rhc->lock);       // 再次对rhc结构体的互斥锁进行加锁

    tnow = ddsrt_time_monotonic();      // 更新tnow为当前时间
  }
  ddsrt_mutex_unlock(&rhc->lock);       // 对rhc结构体的互斥锁进行解锁
  return tnext;                         // 返回tnext值
}
#endif                                  /* DDS_HAS_DEADLINE_MISSED */
// 定义一个新的dds_rhc_default结构体，并根据给定的参数初始化它
struct dds_rhc *dds_rhc_default_new_xchecks(dds_reader *reader,
                                            struct ddsi_domaingv *gv,
                                            const struct ddsi_sertype *type,
                                            bool xchecks) {
  // 分配内存空间给rhc结构体
  struct dds_rhc_default *rhc = ddsrt_malloc(sizeof(*rhc));
  // 将rhc结构体的内存空间初始化为0
  memset(rhc, 0, sizeof(*rhc));
  // 设置rhc的操作函数指针
  rhc->common.common.ops = &dds_rhc_default_ops;

  // 初始化注册表
  lwregs_init(&rhc->registrations);
  // 初始化互斥锁
  ddsrt_mutex_init(&rhc->lock);
  // 创建一个新的哈希表用于存储实例
  rhc->instances = ddsrt_hh_new(1, instance_iid_hash, instance_iid_eq);
  // 初始化非空实例列表
  ddsrt_circlist_init(&rhc->nonempty_instances);
  // 设置类型
  rhc->type = type;
  // 设置读取器
  rhc->reader = reader;
  // 设置tkmap
  rhc->tkmap = gv->m_tkmap;
  // 设置域全局变量
  rhc->gv = gv;
  // 设置xchecks标志
  rhc->xchecks = xchecks;

#ifdef DDS_HAS_LIFESPAN
  // 初始化生命周期管理
  ddsi_lifespan_init(gv, &rhc->lifespan, offsetof(struct dds_rhc_default, lifespan),
                     offsetof(struct rhc_sample, lifespan), dds_rhc_default_sample_expired_cb);
#endif

#ifdef DDS_HAS_DEADLINE_MISSED
  // 设置deadline属性
  rhc->deadline.dur = (reader != NULL) ? reader->m_entity.m_qos->deadline.deadline : DDS_INFINITY;
  // 初始化deadline管理
  ddsi_deadline_init(gv, &rhc->deadline, offsetof(struct dds_rhc_default, deadline),
                     offsetof(struct rhc_instance, deadline), dds_rhc_default_deadline_missed_cb);
#endif

  // 返回rhc结构体的公共部分
  return &rhc->common;
}

// 创建一个新的dds_rhc_default结构体实例
struct dds_rhc *dds_rhc_default_new(dds_reader *reader, const struct ddsi_sertype *type) {
  // 调用dds_rhc_default_new_xchecks函数创建并初始化一个新的dds_rhc_default实例
  return dds_rhc_default_new_xchecks(
      reader, &reader->m_entity.m_domain->gv, type,
      (reader->m_entity.m_domain->gv.config.enabled_xchecks & DDSI_XCHECK_RHC) != 0);
}

// 关联dds_rhc_default结构体实例与读取器、类型和tkmap
static dds_return_t dds_rhc_default_associate(struct dds_rhc *rhc,
                                              dds_reader *reader,
                                              const struct ddsi_sertype *type,
                                              struct ddsi_tkmap *tkmap) {
  // 由于懒惰原因，忽略这些参数
  (void)rhc;
  (void)reader;
  (void)type;
  (void)tkmap;
  // 返回成功状态码
  return DDS_RETCODE_OK;
}
// 定义一个静态函数 dds_rhc_default_set_qos，用于设置缺省的可靠性历史缓存 (Reliability History
// Cache) 的 QoS 参数
static void dds_rhc_default_set_qos(struct ddsi_rhc *rhc_common, const dds_qos_t *qos) {
  // 将通用的 RHC 结构体转换为默认 RHC 结构体
  struct dds_rhc_default *const rhc = (struct dds_rhc_default *)rhc_common;

  // 设置与读取相关的 QoS 参数
  rhc->max_samples = qos->resource_limits.max_samples;
  rhc->max_instances = qos->resource_limits.max_instances;
  rhc->max_samples_per_instance = qos->resource_limits.max_samples_per_instance;
  rhc->minimum_separation = qos->time_based_filter.minimum_separation;
  rhc->by_source_ordering =
      (qos->destination_order.kind == DDS_DESTINATIONORDER_BY_SOURCE_TIMESTAMP);
  rhc->exclusive_ownership = (qos->ownership.kind == DDS_OWNERSHIP_EXCLUSIVE);
  rhc->reliable = (qos->reliability.kind == DDS_RELIABILITY_RELIABLE);

  // 断言：历史记录类型不是 DDS_HISTORY_KEEP_LAST 或者历史深度大于 0
  assert(qos->history.kind != DDS_HISTORY_KEEP_LAST || qos->history.depth > 0);

  // 设置历史深度
  rhc->history_depth =
      (qos->history.kind == DDS_HISTORY_KEEP_LAST) ? (uint32_t)qos->history.depth : ~0u;

  // FIXME: 目前还不支持更新 deadline duration
  // rhc->deadline.dur = qos->deadline.deadline;
}

// 定义一个静态函数 eval_predicate_sample，用于评估给定样本是否满足谓词条件
static bool eval_predicate_sample(const struct dds_rhc_default *rhc,
                                  const struct ddsi_serdata *sample,
                                  bool (*pred)(const void *sample)) {
  // 将序列化数据转换为样本
  ddsi_serdata_to_sample(sample, rhc->qcond_eval_samplebuf, NULL, NULL);

  // 判断样本是否满足谓词条件
  bool ret = pred(rhc->qcond_eval_samplebuf);
  return ret;
}

// 定义一个静态函数 eval_predicate_invsample，用于评估给定实例的无效样本是否满足谓词条件
static bool eval_predicate_invsample(const struct dds_rhc_default *rhc,
                                     const struct rhc_instance *inst,
                                     bool (*pred)(const void *sample)) {
  // 将未类型化的无效样本转换为干净的无效样本
  untyped_to_clean_invsample(rhc->type, inst->tk->m_sample, rhc->qcond_eval_samplebuf, NULL, NULL);

  // 判断无效样本是否满足谓词条件
  bool ret = pred(rhc->qcond_eval_samplebuf);
  return ret;
}

// 定义一个静态函数 alloc_sample，用于为给定的实例分配一个新的样本
static struct rhc_sample *alloc_sample(struct rhc_instance *inst) {
  // 如果实例有可用的空闲样本
  if (inst->a_sample_free) {
    // 将空闲样本标记为已使用
    inst->a_sample_free = 0;

#if USE_VALGRIND
    // 如果使用 Valgrind，将内存设置为未定义状态
    VALGRIND_MAKE_MEM_UNDEFINED(&inst->a_sample, sizeof(inst->a_sample));
#endif

    // 返回空闲样本的地址
    return &inst->a_sample;
  } else {
    // 否则，动态分配一个新的样本结构体
    struct rhc_sample *s;
    s = ddsrt_malloc(sizeof(*s));
    return s;
  }
}
// 释放样本内存
static void free_sample(struct dds_rhc_default *rhc,
                        struct rhc_instance *inst,
                        struct rhc_sample *s) {
#ifndef DDS_HAS_LIFESPAN
  // 如果不支持生命周期，忽略rhc参数
  DDSRT_UNUSED_ARG(rhc);
#endif
  // 减少样本的引用计数
  ddsi_serdata_unref(s->sample);
#ifdef DDS_HAS_LIFESPAN
  // 如果支持生命周期，从生命周期管理器中注销样本
  ddsi_lifespan_unregister_sample_locked(&rhc->lifespan, &s->lifespan);
#endif
  // 如果s指向inst的a_sample
  if (s == &inst->a_sample) {
    // 断言a_sample没有被释放
    assert(!inst->a_sample_free);
#if USE_VALGRIND
    // 使用Valgrind时，将a_sample标记为不可访问
    VALGRIND_MAKE_MEM_NOACCESS(&inst->a_sample, sizeof(inst->a_sample));
#endif
    // 标记a_sample已经释放
    inst->a_sample_free = 1;
  } else {
    // 释放s指向的内存
    ddsrt_free(s);
  }
}

// 清除实例中的无效样本
static void inst_clear_invsample(struct dds_rhc_default *rhc,
                                 struct rhc_instance *inst,
                                 struct trigger_info_qcond *trig_qc) {
  // 断言实例中存在无效样本
  assert(inst->inv_exists);
  // 断言触发条件中的无效样本计数为0
  assert(trig_qc->dec_conds_invsample == 0);
  // 将实例中的无效样本标记设置为0
  inst->inv_exists = 0;
  // 更新触发条件中的无效样本计数
  trig_qc->dec_conds_invsample = inst->conds;
  // 如果无效样本已读
  if (inst->inv_isread) {
    // 设置触发条件中的无效样本已读标记
    trig_qc->dec_invsample_read = true;
    // 减少rhc中的无效已读样本计数
    rhc->n_invread--;
  }
  // 减少rhc中的无效样本计数
  rhc->n_invsamples--;
}

// 如果存在，清除实例中的无效样本
static void inst_clear_invsample_if_exists(struct dds_rhc_default *rhc,
                                           struct rhc_instance *inst,
                                           struct trigger_info_qcond *trig_qc) {
  // 如果实例中存在无效样本
  if (inst->inv_exists)
    // 清除实例中的无效样本
    inst_clear_invsample(rhc, inst, trig_qc);
}
// 设置无效样本
static void inst_set_invsample(struct dds_rhc_default *rhc,
                               struct rhc_instance *inst,
                               struct trigger_info_qcond *trig_qc,
                               bool *__restrict nda) {
  // 如果实例存在无效样本且未读
  if (inst->inv_exists && !inst->inv_isread) {
    /* FIXME: should this indeed trigger a "notify data available" event?*/
    // 设置通知数据可用事件为真
    *nda = true;
  } else {
    /* Obviously optimisable, but that is perhaps not worth the bother */
    // 如果存在，清除无效样本
    inst_clear_invsample_if_exists(rhc, inst, trig_qc);
    // 断言触发器条件中的无效样本计数为0
    assert(trig_qc->inc_conds_invsample == 0);
    // 将实例条件设置为触发器条件中的无效样本计数
    trig_qc->inc_conds_invsample = inst->conds;
    // 设置实例存在无效样本
    inst->inv_exists = 1;
    // 设置实例无效样本未读
    inst->inv_isread = 0;
    // 增加无效样本计数
    rhc->n_invsamples++;
    // 设置通知数据可用事件为真
    *nda = true;
  }
}

// 释放空实例
static void free_empty_instance(struct rhc_instance *inst, struct dds_rhc_default *rhc) {
  // 断言实例为空
  assert(inst_is_empty(inst));
  // 取消实例在tkmap中的引用
  ddsi_tkmap_instance_unref(rhc->tkmap, inst->tk);
#ifdef DDS_HAS_DEADLINE_MISSED
  // 如果实例有截止日期注册，则取消注册
  if (inst->deadline_reg) ddsi_deadline_unregister_instance_locked(&rhc->deadline, &inst->deadline);
#endif
  // 释放实例内存
  ddsrt_free(inst);
}

// 释放实例并从RHC中删除
static void free_instance_rhc_free(struct rhc_instance *inst, struct dds_rhc_default *rhc) {
  // 获取最新的样本
  struct rhc_sample *s = inst->latest;
  // 判断实例是否为空
  const bool was_empty = inst_is_empty(inst);
  // 定义一个虚拟触发器条件结构体
  struct trigger_info_qcond dummy_trig_qc;

  // 如果存在样本
  if (s) {
    // 遍历所有样本并释放
    do {
      struct rhc_sample *const s1 = s->next;
      free_sample(rhc, inst, s);
      s = s1;
    } while (s != inst->latest);
    // 更新有效样本计数和已读有效样本计数
    rhc->n_vsamples -= inst->nvsamples;
    rhc->n_vread -= inst->nvread;
    inst->nvsamples = 0;
    inst->nvread = 0;
  }
#ifndef NDEBUG
  // 初始化虚拟触发器条件结构体
  memset(&dummy_trig_qc, 0, sizeof(dummy_trig_qc));
#endif
  // 如果存在，清除无效样本
  inst_clear_invsample_if_exists(rhc, inst, &dummy_trig_qc);
  // 如果实例非空，从非空列表中移除
  if (!was_empty) remove_inst_from_nonempty_list(rhc, inst);
  // 如果实例是新的，更新新实例计数
  if (inst->isnew) rhc->n_new--;
  // 释放空实例
  free_empty_instance(inst, rhc);
}
// 定义一个静态函数 dds_rhc_default_lock_samples，返回值类型为 uint32_t，参数为指向 dds_rhc
// 结构体的指针
static uint32_t dds_rhc_default_lock_samples(struct dds_rhc *rhc_common) {
  // 将传入的结构体指针强制转换为指向 dds_rhc_default 结构体的指针
  struct dds_rhc_default *const rhc = (struct dds_rhc_default *)rhc_common;
  // 定义一个 uint32_t 类型的变量 no
  uint32_t no;
  // 对 rhc 结构体中的 lock 进行加锁操作
  ddsrt_mutex_lock(&rhc->lock);
  // 计算有效样本数和无效样本数之和，并赋值给 no
  no = rhc->n_vsamples + rhc->n_invsamples;
  // 如果 no 等于 0，则对 rhc 结构体中的 lock 进行解锁操作
  if (no == 0) {
    ddsrt_mutex_unlock(&rhc->lock);
  }
  // 返回 no 的值
  return no;
}

// 定义一个静态函数 free_instance_rhc_free_wrap，无返回值，参数为两个 void 指针 vnode 和 varg
static void free_instance_rhc_free_wrap(void *vnode, void *varg) {
  // 调用 free_instance_rhc_free 函数，传入 vnode 和 varg 作为参数
  free_instance_rhc_free(vnode, varg);
}

// 定义一个静态函数 dds_rhc_default_free，无返回值，参数为指向 ddsi_rhc 结构体的指针
static void dds_rhc_default_free(struct ddsi_rhc *rhc_common) {
  // 将传入的结构体指针强制转换为指向 dds_rhc_default 结构体的指针
  struct dds_rhc_default *rhc = (struct dds_rhc_default *)rhc_common;
#ifdef DDS_HAS_LIFESPAN
  // 如果定义了 DDS_HAS_LIFESPAN 宏，则调用 dds_rhc_default_sample_expired_cb 函数，并传入 rhc 和
  // DDSRT_MTIME_NEVER 作为参数
  dds_rhc_default_sample_expired_cb(rhc, DDSRT_MTIME_NEVER);
  // 调用 ddsi_lifespan_fini 函数，传入 &rhc->lifespan 作为参数
  ddsi_lifespan_fini(&rhc->lifespan);
#endif
#ifdef DDS_HAS_DEADLINE_MISSED
  // 如果定义了 DDS_HAS_DEADLINE_MISSED 宏，则调用 ddsi_deadline_stop 函数，并传入 &rhc->deadline
  // 作为参数
  ddsi_deadline_stop(&rhc->deadline);
#endif
  // 调用 ddsrt_hh_enum 函数，传入 rhc->instances、free_instance_rhc_free_wrap 和 rhc 作为参数
  ddsrt_hh_enum(rhc->instances, free_instance_rhc_free_wrap, rhc);
  // 断言 ddsrt_circlist_isempty 函数的返回值为真，传入 &rhc->nonempty_instances 作为参数
  assert(ddsrt_circlist_isempty(&rhc->nonempty_instances));
#ifdef DDS_HAS_DEADLINE_MISSED
  // 如果定义了 DDS_HAS_DEADLINE_MISSED 宏，则调用 ddsi_deadline_fini 函数，并传入 &rhc->deadline
  // 作为参数
  ddsi_deadline_fini(&rhc->deadline);
#endif
  // 调用 ddsrt_hh_free 函数，传入 rhc->instances 作为参数
  ddsrt_hh_free(rhc->instances);
  // 调用 lwregs_fini 函数，传入 &rhc->registrations 作为参数
  lwregs_fini(&rhc->registrations);
  // 如果 rhc->qcond_eval_samplebuf 不为 NULL，则调用 ddsi_sertype_free_sample 函数，并传入
  // rhc->type、rhc->qcond_eval_samplebuf 和 DDS_FREE_ALL 作为参数
  if (rhc->qcond_eval_samplebuf != NULL)
    ddsi_sertype_free_sample(rhc->type, rhc->qcond_eval_samplebuf, DDS_FREE_ALL);
  // 销毁 rhc 结构体中的 lock
  ddsrt_mutex_destroy(&rhc->lock);
  // 释放 rhc 结构体所占用的内存空间
  ddsrt_free(rhc);
}

// 定义一个静态函数 init_trigger_info_cmn_nonmatch，无返回值，参数为指向 trigger_info_cmn
// 结构体的指针
static void init_trigger_info_cmn_nonmatch(struct trigger_info_cmn *info) {
  // 将 info 结构体中的 qminst 成员设置为最大无符号整数
  info->qminst = ~0u;
  // 将 info 结构体中的 has_read 成员设置为 false
  info->has_read = false;
  // 将 info 结构体中的 has_not_read 成员设置为 false
  info->has_not_read = false;
}

// 定义一个静态函数 get_trigger_info_cmn，无返回值，参数为指向 trigger_info_cmn 结构体的指针和指向
// rhc_instance 结构体的指针
static void get_trigger_info_cmn(struct trigger_info_cmn *info, struct rhc_instance *inst) {
  // 将 inst 的 qmask 赋值给 info 结构体中的 qminst 成员
  info->qminst = qmask_of_inst(inst);
  // 将 inst 是否已读的状态赋值给 info 结构体中的 has_read 成员
  info->has_read = inst_has_read(inst);
  // 将 inst 是否未读的状态赋值给 info 结构体中的 has_not_read 成员
  info->has_not_read = inst_has_unread(inst);
}
// 定义 get_trigger_info_pre 函数，用于获取触发器信息
static void get_trigger_info_pre(struct trigger_info_pre *info, struct rhc_instance *inst) {
  // 调用 get_trigger_info_cmn 函数获取通用触发器信息
  get_trigger_info_cmn(&info->c, inst);
}

// 定义 init_trigger_info_qcond 函数，用于初始化触发器条件信息
static void init_trigger_info_qcond(struct trigger_info_qcond *qc) {
  // 初始化各个触发器条件信息的字段
  qc->dec_invsample_read = false;
  qc->dec_sample_read = false;
  qc->inc_invsample_read = false;
  qc->inc_sample_read = false;
  qc->dec_conds_invsample = 0;
  qc->dec_conds_sample = 0;
  qc->inc_conds_invsample = 0;
  qc->inc_conds_sample = 0;
}

// 定义 trigger_info_differs 函数，用于判断触发器信息是否有差异
static bool trigger_info_differs(const struct dds_rhc_default *rhc,
                                 const struct trigger_info_pre *pre,
                                 const struct trigger_info_post *post,
                                 const struct trigger_info_qcond *trig_qc) {
  // 判断 pre 和 post 的触发器信息是否有差异
  if (pre->c.qminst != post->c.qminst || pre->c.has_read != post->c.has_read ||
      pre->c.has_not_read != post->c.has_not_read)
    return true;
  else if (rhc->nqconds == 0)
    return false;
  else
    // 判断触发器条件信息是否有差异
    return (trig_qc->dec_conds_invsample != trig_qc->inc_conds_invsample ||
            trig_qc->dec_conds_sample != trig_qc->inc_conds_sample ||
            trig_qc->dec_invsample_read != trig_qc->inc_invsample_read ||
            trig_qc->dec_sample_read != trig_qc->inc_sample_read);
}

// 定义 add_sample 函数，用于向实例中添加样本
static bool add_sample(struct dds_rhc_default *rhc,
                       struct rhc_instance *inst,
                       const struct ddsi_writer_info *wrinfo,
                       const struct ddsi_serdata *sample,
                       ddsi_status_cb_data_t *cb_data,
                       struct trigger_info_qcond *trig_qc,
                       bool *__restrict nda) {
  // 定义一个指向 rhc_sample 结构体的指针 s
  struct rhc_sample *s;

  /* 添加样本时总是清除无效样本（因为无效样本中包含的信息 - 实例状态和生成计数 - 都包含在样本中）。
     虽然这里可以执行此操作，但我们稍后会执行，以避免在分配失败时回滚 */

  /* 在 BY_SOURCE 模式下，我们不进行回填操作 -- 我们可以这么做，但选择不这么做 --
     并且已经过滤掉了 inst->latest 之前的样本，所以我们可以直接插入而无需任何搜索 */
  if (inst->nvsamples == rhc->history_depth) {
    /* 替换最旧的样本；latest 指向最新的一个，列表从旧到新是循环的，所以 latest->next 是最旧的 */
    inst_clear_invsample_if_exists(rhc, inst, trig_qc);
    assert(inst->latest != NULL);
    s = inst->latest->next;
    assert(trig_qc->dec_conds_sample == 0);
    ddsi_serdata_unref(s->sample);
    // 定义一个静态函数 get_trigger_info_pre，用于获取触发器信息
    static void get_trigger_info_pre(struct trigger_info_pre * info, struct rhc_instance * inst) {
      // 调用 get_trigger_info_cmn 函数获取通用触发器信息
      get_trigger_info_cmn(&info->c, inst);
    }

    // 定义一个静态函数 init_trigger_info_qcond，用于初始化触发器条件信息
    static void init_trigger_info_qcond(struct trigger_info_qcond * qc) {
      // 初始化各个触发器条件信息的字段
      qc->dec_invsample_read = false;
      qc->dec_sample_read = false;
      qc->inc_invsample_read = false;
      qc->inc_sample_read = false;
      qc->dec_conds_invsample = 0;
      qc->dec_conds_sample = 0;
      qc->inc_conds_invsample = 0;
      qc->inc_conds_sample = 0;
    }

    // 定义一个静态函数 trigger_info_differs，用于判断触发器信息是否有差异
    static bool trigger_info_differs(
        const struct dds_rhc_default *rhc, const struct trigger_info_pre *pre,
        const struct trigger_info_post *post, const struct trigger_info_qcond *trig_qc) {
      // 判断 pre 和 post 的触发器信息是否有差异
      if (pre->c.qminst != post->c.qminst || pre->c.has_read != post->c.has_read ||
          pre->c.has_not_read != post->c.has_not_read)
        return true;
      else if (rhc->nqconds == 0)
        return false;
      else
        // 判断触发器条件信息是否有差异
        return (trig_qc->dec_conds_invsample != trig_qc->inc_conds_invsample ||
                trig_qc->dec_conds_sample != trig_qc->inc_conds_sample ||
                trig_qc->dec_invsample_read != trig_qc->inc_invsample_read ||
                trig_qc->dec_sample_read != trig_qc->inc_sample_read);
    }

    // 定义一个静态函数 add_sample，用于向实例中添加样本
    static bool add_sample(struct dds_rhc_default * rhc, struct rhc_instance * inst,
                           const struct ddsi_writer_info *wrinfo, const struct ddsi_serdata *sample,
                           ddsi_status_cb_data_t *cb_data, struct trigger_info_qcond *trig_qc,
                           bool *__restrict nda) {
      // 定义一个指向 rhc_sample 结构体的指针 s
      struct rhc_sample *s;

      // 添加样本时总是清除无效样本（因为无效样本中包含的信息 - 实例状态和生成计数 -
      // 都包含在样本中）。 虽然这里可以执行清除操作，但我们稍后再执行，以避免在分配失败时回滚

      // 在 BY_SOURCE 模式下，我们不进行回填操作
      // 因为我们已经过滤掉了 inst->latest 之前的样本，所以我们可以直接插入它，而无需搜索
      if (inst->nvsamples == rhc->history_depth) {
        // 替换最旧的样本；latest 指向最新的样本，列表从旧到新是循环的，所以 latest->next 是最旧的
        inst_clear_invsample_if_exists(rhc, inst, trig_qc);
        assert(inst->latest != NULL);
        s = inst->latest->next;
        assert(trig_qc->dec_conds_sample == 0);
        ddsi_serdata_unref(s->sample);

#ifdef DDS_HAS_LIFESPAN
        // 在有生命周期支持的情况下，取消注册已锁定的样本的生命周期
        ddsi_lifespan_unregister_sample_locked(&rhc->lifespan, &s->lifespan);
#endif

        // 更新触发器条件信息
        trig_qc->dec_sample_read = s->isread;
        trig_qc->dec_conds_sample = s->conds;
        if (s->isread) {
          inst->nvread--;
          rhc->n_vread--;
        }
      } else {
        // 检查资源 max_samples QoS 是否超过限制
        if (rhc->reader && rhc->max_samples != DDS_LENGTH_UNLIMITED &&
            rhc->n_vsamples >= (uint32_t)rhc->max_samples) {
          cb_data->raw_status_id = (int)DDS_SAMPLE_REJECTED_STATUS_ID;
          cb_data->extra = DDS_REJECTED_BY_SAMPLES_LIMIT;
          cb_data->handle = inst->iid;
          cb_data->add = true;
          return false;
        }

        // 检查资源 max_samples_per_instance QoS 是否超过限制
        if (rhc->reader && rhc->max_samples_per_instance != DDS_LENGTH_UNLIMITED &&
            inst->nvsamples >= (uint32_t)rhc->max_samples_per_instance) {
          cb_data->raw_status_id = (int)DDS_SAMPLE_REJECTED_STATUS_ID;
          cb_data->extra = DDS_REJECTED_BY_SAMPLES_PER_INSTANCE_LIMIT;
          cb_data->handle = inst->iid;
          cb_data->add = true;
          return false;
        }

        // 添加新的最新样本
        s = alloc_sample(inst);
        inst_clear_invsample_if_exists(rhc, inst, trig_qc);
        if (inst->latest == NULL) {
          s->next = s;
        } else {
          s->next = inst->latest->next;
          inst->latest->next = s;
        }
        inst->nvsamples++;
        rhc->n_vsamples++;
      }
    }
    // 为 s->sample 赋值，引用 ddsi_serdata 类型的 sample 对象（注意：此处会丢失 const
    // 属性，但引用计数会发生变化）
    s->sample = ddsi_serdata_ref(sample);
    // 为 s->wr_iid 赋值，设置为 wrinfo 结构体中的 iid 成员值
    s->wr_iid = wrinfo->iid;
    // 设置 s->isread 为 false，表示该样本尚未被读取
    s->isread = false;
    // 将 inst 结构体中的 disposed_gen 成员值赋给 s->disposed_gen
    s->disposed_gen = inst->disposed_gen;
    // 将 inst 结构体中的 no_writers_gen 成员值赋给 s->no_writers_gen
    s->no_writers_gen = inst->no_writers_gen;

// 如果定义了 DDS_HAS_LIFESPAN 宏
#ifdef DDS_HAS_LIFESPAN
    // 为 s->inst 赋值，设置为 inst 指针
    s->inst = inst;
    // 为 s->lifespan.t_expire 赋值，设置为 wrinfo 结构体中的 lifespan_exp 成员值
    s->lifespan.t_expire = wrinfo->lifespan_exp;
    // 注册样本的生命周期信息
    ddsi_lifespan_register_sample_locked(&rhc->lifespan, &s->lifespan);
#endif

    // 初始化 s->conds 为 0
    s->conds = 0;
    // 如果 rhc 结构体中的 nqconds 不等于 0
    if (rhc->nqconds != 0) {
      // 遍历 rhc 结构体中的 conds 链表
      for (dds_readcond *rc = rhc->conds; rc != NULL; rc = rc->m_next)
        // 如果 rc 结构体中的 m_query.m_filter 不为 0，且 eval_predicate_sample 函数返回 true
        if (rc->m_query.m_filter != 0 &&
            eval_predicate_sample(rhc, s->sample, rc->m_query.m_filter))
          // 将 s->conds 的值与 rc 结构体中的 m_query.m_qcmask 进行按位或操作
          s->conds |= rc->m_query.m_qcmask;
    }

    // 设置 trig_qc 结构体中的 inc_conds_sample 成员值为 s->conds
    trig_qc->inc_conds_sample = s->conds;
    // 将 inst 结构体中的 latest 成员设置为 s 指针
    inst->latest = s;
    // 将 nda 指针指向的布尔值设置为 true
    *nda = true;
    // 返回 true
    return true;
  }

  // 定义 content_filter_make_sampleinfo 静态函数，参数包括 dds_sample_info 类型的 si
  // 指针、ddsi_serdata 类型的 sample 指针、rhc_instance 类型的 inst 指针、uint64_t 类型的 wr_iid 和
  // uint64_t 类型的 iid
  static void content_filter_make_sampleinfo(
      struct dds_sample_info * si, const struct ddsi_serdata *sample,
      const struct rhc_instance *inst, uint64_t wr_iid, uint64_t iid) {
    // 设置 si 结构体中的 sample_state 成员值为 DDS_SST_NOT_READ
    si->sample_state = DDS_SST_NOT_READ;
    // 设置 si 结构体中的 publication_handle 成员值为 wr_iid
    si->publication_handle = wr_iid;
    // 设置 si 结构体中的 source_timestamp 成员值为 sample 结构体中的 timestamp.v 成员值
    si->source_timestamp = sample->timestamp.v;
    // 初始化 si 结构体中的 sample_rank、generation_rank 和 absolute_generation_rank 成员值为 0
    si->sample_rank = 0;
    si->generation_rank = 0;
    si->absolute_generation_rank = 0;
    // 设置 si 结构体中的 valid_data 成员值为 true
    si->valid_data = true;

    // 如果 inst 指针不为空
    if (inst) {
      // 根据 inst 结构体中的 isnew 成员值设置 si 结构体中的 view_state 成员值
      si->view_state = inst->isnew ? DDS_VST_NEW : DDS_VST_OLD;
      // 根据 inst 结构体中的 isdisposed 和 wrcount 成员值设置 si 结构体中的 instance_state 成员值
      si->instance_state = inst->isdisposed       ? DDS_IST_NOT_ALIVE_DISPOSED
                           : (inst->wrcount == 0) ? DDS_IST_NOT_ALIVE_NO_WRITERS
                                                  : DDS_IST_ALIVE;
      // 设置 si 结构体中的 instance_handle 成员值为 inst 结构体中的 iid 成员值
      si->instance_handle = inst->iid;
      // 设置 si 结构体中的 disposed_generation_count 和 no_writers_generation_count 成员值分别为
      // inst 结构体中的 disposed_gen 和 no_writers_gen 成员值
      si->disposed_generation_count = inst->disposed_gen;
      si->no_writers_generation_count = inst->no_writers_gen;
    } else {
      // 设置 si 结构体中的 view_state、instance_state、instance_handle、disposed_generation_count
      // 和 no_writers_generation_count 成员值为默认值
      si->view_state = DDS_VST_NEW;
      si->instance_state = DDS_IST_ALIVE;
      si->instance_handle = iid;
      si->disposed_generation_count = 0;
      si->no_writers_generation_count = 0;
    }
  }
  // 定义一个静态函数 content_filter_accepts，用于判断内容过滤器是否接受给定的样本
  static bool content_filter_accepts(const dds_reader *reader, const struct ddsi_serdata *sample,
                                     const struct rhc_instance *inst, uint64_t wr_iid,
                                     uint64_t iid) {
    // 初始化返回值为 true
    bool ret = true;

    // 如果 reader 存在
    if (reader) {
      // 获取 reader 对应的主题
      const struct dds_topic *tp = reader->m_topic;

      // 根据主题过滤器模式进行处理
      switch (tp->m_filter.mode) {
        case DDS_TOPIC_FILTER_NONE:
          // 如果没有过滤器，则直接返回 true
          ret = true;
          break;
        case DDS_TOPIC_FILTER_SAMPLEINFO_ARG: {
          // 如果过滤器模式为 SAMPLEINFO_ARG
          struct dds_sample_info si;
          // 创建 sampleinfo 结构体
          content_filter_make_sampleinfo(&si, sample, inst, wr_iid, iid);
          // 调用过滤器函数并更新返回值
          ret = tp->m_filter.f.sampleinfo_arg(&si, tp->m_filter.arg);
          break;
        }
        case DDS_TOPIC_FILTER_SAMPLE:
        case DDS_TOPIC_FILTER_SAMPLE_ARG:
        case DDS_TOPIC_FILTER_SAMPLE_SAMPLEINFO_ARG: {
          // 如果过滤器模式为 SAMPLE、SAMPLE_ARG 或 SAMPLE_SAMPLEINFO_ARG
          char *tmp;
          // 为样本分配内存
          tmp = ddsi_sertype_alloc_sample(tp->m_stype);
          // 将 serdata 转换为样本
          ddsi_serdata_to_sample(sample, tmp, NULL, NULL);

          // 根据过滤器模式进行处理
          switch (tp->m_filter.mode) {
            case DDS_TOPIC_FILTER_NONE:
            case DDS_TOPIC_FILTER_SAMPLEINFO_ARG:
              // 如果模式为 NONE 或 SAMPLEINFO_ARG，则触发断言错误
              assert(0);
            case DDS_TOPIC_FILTER_SAMPLE:
              // 调用过滤器函数并更新返回值
              ret = (tp->m_filter.f.sample)(tmp);
              break;
            case DDS_TOPIC_FILTER_SAMPLE_ARG:
              // 调用过滤器函数并更新返回值
              ret = (tp->m_filter.f.sample_arg)(tmp, tp->m_filter.arg);
              break;
            case DDS_TOPIC_FILTER_SAMPLE_SAMPLEINFO_ARG: {
              struct dds_sample_info si;
              // 创建 sampleinfo 结构体
              content_filter_make_sampleinfo(&si, sample, inst, wr_iid, iid);
              // 调用过滤器函数并更新返回值
              ret = tp->m_filter.f.sample_sampleinfo_arg(tmp, &si, tp->m_filter.arg);
              break;
            }
          }

          // 释放样本内存
          ddsi_sertype_free_sample(tp->m_stype, tmp, DDS_FREE_ALL);
          break;
        }
      }
    }

    // 返回结果
    return ret;
  }
  // 判断实例是否接受指定写入者GUID的样本
  static int inst_accepts_sample_by_writer_guid(const struct rhc_instance *inst,
                                                const struct ddsi_writer_info *wrinfo) {
    // 如果实例的写入者IID是活跃的，并且与给定的写入者信息的IID相等，或者写入者GUID与实例的写入者GUID比较结果小于0，则返回1（接受），否则返回0（不接受）
    return (inst->wr_iid_islive && inst->wr_iid == wrinfo->iid) ||
           memcmp(&wrinfo->guid, &inst->wr_guid, sizeof(inst->wr_guid)) < 0;
  }

  // 判断实例是否接受样本
  static int inst_accepts_sample(const struct dds_rhc_default *rhc, const struct rhc_instance *inst,
                                 const struct ddsi_writer_info *wrinfo,
                                 const struct ddsi_serdata *sample, const bool has_data) {
    // 如果按源顺序排序
    if (rhc->by_source_ordering) {
      // 按源顺序排序，因此需要比较时间戳
      if (sample->timestamp.v == DDS_TIME_INVALID || inst->tstamp.v == DDS_TIME_INVALID ||
          inst->tstamp.v == sample->timestamp.v) {
        // 一个或两个样本没有有效的时间戳，或者两个样本具有相同的时间戳，需要检查写入者GUID
        if (!inst_accepts_sample_by_writer_guid(inst, wrinfo)) return 0;  // 不接受
      } else if (sample->timestamp.v < inst->tstamp.v) {
        // 样本在实例之前，因此肯定拒绝
        return 0;  // 不接受
      }
      // 样本晚于实例，可能需要进一步检查
    }

    // 如果存在最小间隔，并且样本和实例的时间戳都有效
    if (rhc->minimum_separation > 0 && sample->timestamp.v != DDS_TIME_INVALID &&
        inst->tstamp.v != DDS_TIME_INVALID) {
      if (sample->timestamp.v < INT64_MIN + rhc->minimum_separation ||
          sample->timestamp.v - rhc->minimum_separation < inst->tstamp.v) {
        return 0;  // 拒绝
      }
    }

    // 如果设置了独占所有权，并且实例的写入者IID是活跃的，且与给定的写入者信息的IID不相等
    if (rhc->exclusive_ownership && inst->wr_iid_islive && inst->wr_iid != wrinfo->iid) {
      int32_t strength = wrinfo->ownership_strength;
      if (strength > inst->strength) {
        // 可以接受
      } else if (strength < inst->strength) {
        return 0;  // 不接受
      } else if (inst_accepts_sample_by_writer_guid(inst, wrinfo)) {
        // 可以接受
      } else {
        return 0;  // 不接受
      }
    }
    // 如果有数据并且内容过滤器不接受给定的读取器、样本、实例和写入者IID
    if (has_data && !content_filter_accepts(rhc->reader, sample, inst, wrinfo->iid, inst->iid)) {
      return 0;  // 不接受
    }
    return 1;    // 接受
  }
  // 更新实例的通用部分，包括时间戳和所有权强度
  static void update_inst_common(struct rhc_instance * inst,
                                 const struct ddsi_writer_info *__restrict wrinfo,
                                 ddsrt_wctime_t tstamp) {
    inst->tstamp = tstamp;
    inst->strength = wrinfo->ownership_strength;
  }

  // 更新实例，设置写入者iid、guid，并将wr_iid_islive标志设置为true
  static void update_inst_have_wr_iid(struct rhc_instance * inst,
                                      const struct ddsi_writer_info *__restrict wrinfo,
                                      ddsrt_wctime_t tstamp) {
    update_inst_common(inst, wrinfo, tstamp);
    inst->wr_iid = wrinfo->iid;
    inst->wr_guid = wrinfo->guid;
    inst->wr_iid_islive = true;
  }

  // 更新实例，将wr_iid_islive标志设置为false
  static void update_inst_no_wr_iid(struct rhc_instance * inst,
                                    const struct ddsi_writer_info *__restrict wrinfo,
                                    ddsrt_wctime_t tstamp) {
    update_inst_common(inst, wrinfo, tstamp);
    inst->wr_iid_islive = false;
  }

  // 删除没有关联写入者的实例，并更新相关计数器
  // 定义一个静态函数 drop_instance_noupdate_no_writers，用于在没有写入者的情况下删除实例
  static void drop_instance_noupdate_no_writers(
      struct dds_rhc_default *__restrict rhc, struct rhc_instance *__restrict *__restrict instptr) {
    // 定义一个指向实例的指针，并将其初始化为传入的实例指针
    struct rhc_instance *inst = *instptr;
    // 断言该实例为空
    assert(inst_is_empty(inst));

    // 实例计数减一
    rhc->n_instances--;
    // 如果实例是新的，则新实例计数减一
    if (inst->isnew) rhc->n_new--;

    // 从实例集合中移除当前实例
    ddsrt_hh_remove_present(rhc->instances, inst);
    // 释放空实例所占用的内存
    free_empty_instance(inst, rhc);
    // 将传入的实例指针设置为 NULL
    *instptr = NULL;
  }
  // 定义一个静态函数 dds_rhc_register，用于注册实例
  static void dds_rhc_register(struct dds_rhc_default * rhc, struct rhc_instance * inst,
                               uint64_t wr_iid, bool autodispose, bool sample_accepted,
                               bool *__restrict nda) {
    // 获取实例的写入者ID，如果实例是活跃的，则使用实例的写入者ID，否则为0
    const uint64_t inst_wr_iid = inst->wr_iid_islive ? inst->wr_iid : 0;

    // 输出调试信息
    TRACE(" register:");

    /* 判断隐式注册的dispose是否等同于register和dispose的组合。
       如果是这样，当旧实例状态为DISPOSED时，需要递增no_writers_gen和disposed_gen，
       否则只需递增disposed_gen。将其解释为等价。

       dispose是否为样本？我认为不是（尽管write dispose是）。
       纯粹的register是否为样本？也不这么认为。 */
    if (inst_wr_iid == wr_iid) {
      /* 与上次相同的写入者 => 我们知道它已经注册了。
         这是快速路径 -- 我们不必检查其他任何内容。 */
      TRACE("cached");
      assert(inst->wrcount > 0);
    } else if (inst->wrcount == 0) {
      /* 当前没有写入者 */
      assert(!inst->wr_iid_islive);

      /* 当基于拒绝的样本注册写入者并使实例从非活动状态转换为活动状态时，
         我们确实希望生成具有新注册（或重新注册）写入者ID的无效样本，
         但我们不希望inst_accepts_sample受到影响（在排序中它是“太旧”的）。
         wr_iid_islive决定wr_iid是否有意义，因此设置wr_iid同时保持
         wr_iid_islive为false可以获得所需的行为。 */
      inst->wr_iid = wr_iid;
      if (sample_accepted) inst->wr_iid_islive = 1;
      inst->wrcount++;
      inst->no_writers_gen++;
      inst->autodispose = autodispose;
      TRACE("new1");

      if (!inst_is_empty(inst) && !inst->isdisposed) rhc->n_not_alive_no_writers--;
      *nda = true;
    } else if (inst_wr_iid == 0 && inst->wrcount == 1) {
      /* 存在写入者，但wr_iid为空 => 有人取消注册。

         当wrcount为1时，如果wr_iid恰好是剩余的写入者，
         我们删除显式注册并再次依赖inst->wr_iid，但如果wr_iid恰好是新的写入者，
         我们递增写入者计数并显式注册第二个写入者。

         如果我决定使用并发跳跃哈希表实现全局注册表，
         那么这应该仍然具有很好的扩展性，因为lwregs_add首先调用lwregs_contains，
         这是无锁的。 */
      if (lwregs_add(&rhc->registrations, inst->iid, wr_iid)) {
        inst->wrcount++;
        if (autodispose) inst->autodispose = 1;
        TRACE("new2iidnull");
      } else {
        int x = lwregs_delete(&rhc->registrations, inst->iid, wr_iid);
        assert(x);
        (void)x;
        TRACE("restore");
      }
      /* 当register被拒绝的样本调用时，避免更新wr_iid */
      if (sample_accepted) {
        inst->wr_iid = wr_iid;
        inst->wr_iid_islive = 1;
      }
    } else {
      /* 如上所述 -- 如果使用并发跳跃哈希，如果写入者已知，则lwregs_add是无锁的 */
      if (inst->wrcount == 1) {
        /* 第二个写入者 => 正确注册我们知道的那个 */
        TRACE("rescue1");
        int x;
        x = lwregs_add(&rhc->registrations, inst->iid, inst_wr_iid);
        assert(x);
        (void)x;
      }
      if (lwregs_add(&rhc->registrations, inst->iid, wr_iid)) {
        /* 一旦我们达到至少两个写入者，我们必须检查lwregs_add的结果，
           以了解此样本是否注册了以前未知的写入者 */
        TRACE("new3");
        inst->wrcount++;
        if (autodispose) inst->autodispose = 1;
      } else {
        TRACE("known");
      }
      assert(inst->wrcount >= 2);
      /* 最近的写入者获得快速路径 */
      /* 当register被拒绝的样本调用时，避免更新wr_iid */
      if (sample_accepted) {
        inst->wr_iid = wr_iid;
        inst->wr_iid_islive = 1;
      }
    }
  }
  // 定义一个静态函数，用于处理从空实例到非空实例的转换
  static void account_for_empty_to_nonempty_transition(struct dds_rhc_default * rhc,
                                                       struct rhc_instance * inst) {
    // 断言实例中的样本数量为1
    assert(inst_nsamples(inst) == 1);
    // 将实例添加到非空列表中
    add_inst_to_nonempty_list(rhc, inst);
    // 如果实例已被销毁，则增加不活跃且已销毁的计数
    if (inst->isdisposed) rhc->n_not_alive_disposed++;
    // 否则，如果实例的写入计数为0，则增加不活跃且无写入者的计数
    else if (inst->wrcount == 0)
      rhc->n_not_alive_no_writers++;
  }

  // 定义一个静态函数，用于处理从非空实例到空实例的转换
  static void account_for_nonempty_to_empty_transition(
      struct dds_rhc_default *__restrict rhc, struct rhc_instance *__restrict *__restrict instptr,
      const char *__restrict traceprefix) {
    // 获取实例指针
    struct rhc_instance *inst = *instptr;
    // 断言实例为空
    assert(inst_is_empty(inst));
    // 从非空列表中移除实例
    remove_inst_from_nonempty_list(rhc, inst);
    // 如果实例已被销毁，则减少不活跃且已销毁的计数
    if (inst->isdisposed) rhc->n_not_alive_disposed--;
    // 如果实例的写入计数为0
    if (inst->wrcount == 0) {
      // 输出跟踪信息
      TRACE("%siid %" PRIx64 " #0,empty,drop\n", traceprefix, inst->iid);
      // 如果实例未被销毁
      if (!inst->isdisposed) {
        // 减少不活跃且无写入者的计数（为什么不只用两个比特位？）
        rhc->n_not_alive_no_writers--;
      }
      // 删除实例，但不更新无写入者的计数
      drop_instance_noupdate_no_writers(rhc, instptr);
    }
  }

  // 定义一个静态函数，用于在实例中删除注册信息
  static int rhc_unregister_delete_registration(struct dds_rhc_default * rhc,
                                                const struct rhc_instance *inst, uint64_t wr_iid) {
    // 如果实例的写入计数为0，返回1表示最后一个注册信息刚刚消失
    if (inst->wrcount == 0) {
      TRACE("unknown(#0)");
      return 0;
    }
    // 否则，如果实例的写入计数为1且写入者ID是有效的
    else if (inst->wrcount == 1 && inst->wr_iid_islive) {
      // 断言写入者ID不为0
      assert(inst->wr_iid != 0);
      // 如果传入的写入者ID与实例中的写入者ID不匹配
      if (wr_iid != inst->wr_iid) {
        TRACE("unknown(cache)");
        return 0;
      }
      // 否则，返回1表示最后一个注册信息刚刚消失
      else {
        TRACE("last(cache)");
        return 1;
      }
    }
    // 如果无法从注册表中删除写入者ID，则返回0表示未知情况
    else if (!lwregs_delete(&rhc->registrations, inst->iid, wr_iid)) {
      TRACE("unknown(regs)");
      return 0;
    }
    // 否则，删除成功
    else {
      TRACE("delreg");
      // 如果实例的写入计数从2变为1，且我们正在删除的写入者与实例中缓存的写入者不同
      // 那么在此之后将只有一个写入者，它将被缓存，其注册记录必须被删除
      // （这是一个不变式：当wrcount = 1且wr_iid != 0时，wr_iid不在"registrations"中）
      if (inst->wrcount == 2 && inst->wr_iid_islive && inst->wr_iid != wr_iid) {
        TRACE(",delreg(remain)");
        (void)lwregs_delete(&rhc->registrations, inst->iid, inst->wr_iid);
      }
      return 1;
    }
  }

  // 静态函数，用于注销更新实例
  static int rhc_unregister_updateinst(
      struct dds_rhc_default * rhc,                      // 默认的可靠性历史缓存指针
      struct rhc_instance * inst,                        // 实例指针
      const struct ddsi_writer_info *__restrict wrinfo,  // 写入器信息指针
      ddsrt_wctime_t tstamp,                             // 时间戳
      struct trigger_info_qcond *trig_qc,                // 触发条件队列指针
      bool *__restrict nda  // 布尔值指针，表示是否需要处理非活动状态
  ) {
    // 断言：实例的写入计数大于0
    assert(inst->wrcount > 0);

    // 如果写入器信息中的自动处理标志为真，则设置实例的自动处理标志为1
    if (wrinfo->auto_dispose) inst->autodispose = 1;

    // 如果实例的写入计数减1后仍大于0
    if (--inst->wrcount > 0) {
      // 如果实例的写入器IID是活动的，并且写入器信息的IID等于实例的写入器IID
      if (inst->wr_iid_islive && wrinfo->iid == inst->wr_iid) {
        // 下一个注册将在我们再次缓存wr_iid之前执行实际工作
        inst->wr_iid_islive = 0;

        // 重置所有权强度，以允许从其他写入器读取样本
        inst->strength = 0;
        TRACE(",clearcache");
      }
      return 0;
    } else {
      // 如果实例不为空
      if (!inst_is_empty(inst)) {
        // 实例仍然有内容 - 在应用程序获取最后一个样本之前不要删除
        // 如果最新的样本已经被读取，则设置无效样本，以便应用程序可以读取到非活动状态的更改。
        // （如果最新的样本仍未读取，我们不会费心，尽管这意味着应用程序将看不到注销事件的时间戳。它不应该关心。）
        // 如果实例没有被处理过
        if (!inst->isdisposed) {
          // 如果实例的最新样本为空或已读
          if (inst->latest == NULL || inst->latest->isread) {
            // 设置无效样本
            inst_set_invsample(rhc, inst, trig_qc, nda);
            // 更新实例，不包含写入器实例ID
            update_inst_no_wr_iid(inst, wrinfo, tstamp);
          }
          // 如果实例没有自动处理
          if (!inst->autodispose)
            // 增加未活跃且无写入器的实例计数
            rhc->n_not_alive_no_writers++;
          else {
            // 输出调试信息：自动处理
            TRACE(",autodispose");
            // 标记实例为已处理
            inst->isdisposed = 1;
            // 增加未活跃且已处理的实例计数
            rhc->n_not_alive_disposed++;
          }
          // 设置非活跃状态为真
          *nda = true;
        }
        // 将写入器实例ID设置为非活跃
        inst->wr_iid_islive = 0;
        // 返回0表示成功
        return 0;
      } else if (inst->isdisposed) {
        // 没有剩余内容，没有剩余注册，所以删除
        TRACE(",#0,empty,nowriters,disposed");
        return 1;
      } else {
        // 添加无效样本以进行无写入器转换
        TRACE(",#0,empty,nowriters");
        // 断言实例为空（没有数据）
        assert(inst_is_empty(inst));
        // 设置无效样本
        inst_set_invsample(rhc, inst, trig_qc, nda);
        // 更新实例，不包括写入器的实例ID
        update_inst_no_wr_iid(inst, wrinfo, tstamp);
        // 如果实例设置了自动处理
        if (inst->autodispose) {
          // 输出跟踪信息
          TRACE(",autodispose");
          // 将实例标记为已处理
          inst->isdisposed = 1;
        }
        // 处理空到非空的转换
        account_for_empty_to_nonempty_transition(rhc, inst);
        // 将写入器实例ID标记为非活动状态
        inst->wr_iid_islive = 0;
        // 设置非活动状态为真
        *nda = true;
        // 返回0表示成功
        return 0;
      }
    }
  }
  // dds_rhc_unregister 函数用于注销实例的注册信息
  static void dds_rhc_unregister(
      struct dds_rhc_default * rhc,                      // 默认的可靠历史缓存指针
      struct rhc_instance * inst,                        // 实例指针
      const struct ddsi_writer_info *__restrict wrinfo,  // 写入器信息指针
      ddsrt_wctime_t tstamp,                             // 时间戳
      struct trigger_info_post *post,                    // 触发器信息指针
      struct trigger_info_qcond *trig_qc,                // 查询条件触发器信息指针
      bool *__restrict nda  // 布尔值指针，表示是否需要删除实例
  ) {
    // 'post' 总是会被设置
    TRACE(" unregister:");
    if (!rhc_unregister_delete_registration(rhc, inst, wrinfo->iid)) {
      // 其他注册信息仍然存在
      get_trigger_info_cmn(&post->c, inst);
    } else if (rhc_unregister_updateinst(rhc, inst, wrinfo, tstamp, trig_qc, nda)) {
      // 实例已删除
      init_trigger_info_cmn_nonmatch(&post->c);
    } else {
      // 没有写入器剩余，但实例不为空
      get_trigger_info_cmn(&post->c, inst);
    }
    TRACE(" nda=%d\n", *nda);
  }

  // alloc_new_instance 函数用于分配一个新的实例
  static struct rhc_instance *alloc_new_instance(
      struct dds_rhc_default * rhc,           // 默认的可靠历史缓存指针
      const struct ddsi_writer_info *wrinfo,  // 写入器信息指针
      struct ddsi_serdata *serdata,           // 序列化数据指针
      struct ddsi_tkmap_instance *tk          // 实例映射指针
  ) {                                         // 定义一个rhc_instance结构体指针inst
    struct rhc_instance *inst;

    // 增加tk的引用计数
    ddsi_tkmap_instance_ref(tk);
    // 为inst分配内存空间，大小为rhc_instance结构体的大小
    inst = ddsrt_malloc(sizeof(*inst));
    // 将inst所指向的内存空间初始化为0
    memset(inst, 0, sizeof(*inst));
    // 设置inst的iid字段为tk的m_iid字段值
    inst->iid = tk->m_iid;
    // 设置inst的tk字段为tk
    inst->tk = tk;
    // 初始化inst的wrcount字段为1
    inst->wrcount = 1;
    // 根据serdata的statusinfo字段判断是否处于disposed状态，并设置inst的isdisposed字段
    inst->isdisposed = (serdata->statusinfo & DDSI_STATUSINFO_DISPOSE) != 0;
    // 设置inst的autodispose字段为wrinfo的auto_dispose字段值
    inst->autodispose = wrinfo->auto_dispose;
    // 初始化inst的deadline_reg字段为0
    inst->deadline_reg = 0;
    // 初始化inst的isnew字段为1
    inst->isnew = 1;
    // 初始化inst的a_sample_free字段为1
    inst->a_sample_free = 1;
    // 初始化inst的conds字段为0
    inst->conds = 0;
    // 设置inst的wr_iid字段为wrinfo的iid字段值
    inst->wr_iid = wrinfo->iid;
    // 根据inst的wrcount字段判断是否为live状态，并设置inst的wr_iid_islive字段
    inst->wr_iid_islive = (inst->wrcount != 0);
    // 设置inst的wr_guid字段为wrinfo的guid字段值
    inst->wr_guid = wrinfo->guid;
    // 设置inst的tstamp字段为serdata的timestamp字段值
    inst->tstamp = serdata->timestamp;
    // 设置inst的strength字段为wrinfo的ownership_strength字段值
    inst->strength = wrinfo->ownership_strength;

    // 如果rhc的nqconds字段不为0，则遍历rhc的conds链表
    if (rhc->nqconds != 0) {
      for (dds_readcond *c = rhc->conds; c != NULL; c = c->m_next) {
        // 断言：实体类型为DDS_KIND_COND_READ且过滤器为空，或者实体类型为DDS_KIND_COND_QUERY且过滤器不为空
        assert((dds_entity_kind(&c->m_entity) == DDS_KIND_COND_READ && c->m_query.m_filter == 0) ||
               (dds_entity_kind(&c->m_entity) == DDS_KIND_COND_QUERY && c->m_query.m_filter != 0));
        // 如果过滤器存在且eval_predicate_invsample函数返回true，则将inst的conds字段与c的m_query.m_qcmask字段进行按位或操作
        if (c->m_query.m_filter && eval_predicate_invsample(rhc, inst, c->m_query.m_filter))
          inst->conds |= c->m_query.m_qcmask;
      }
    }
    // 返回inst指针
    return inst;
  }

  // 定义一个静态函数，用于创建新的rhc实例
  static rhc_store_result_t rhc_store_new_instance(
      struct rhc_instance * *out_inst,  // 输出参数：指向新创建的rhc实例的指针
      struct dds_rhc_default * rhc,     // 输入参数：指向默认的dds_rhc结构体的指针
      const struct ddsi_writer_info *wrinfo,  // 输入参数：指向ddsi_writer_info结构体的指针
      struct ddsi_serdata *sample,            // 输入参数：指向ddsi_serdata结构体的指针
      struct ddsi_tkmap_instance *tk,  // 输入参数：指向ddsi_tkmap_instance结构体的指针
      const bool has_data,             // 输入参数：布尔值，表示是否有数据
      ddsi_status_cb_data_t *cb_data,  // 输入输出参数：指向ddsi_status_cb_data_t结构体的指针
      struct trigger_info_qcond *trig_qc,  // 输入参数：指向trigger_info_qcond结构体的指针
      bool *__restrict nda)  // 输入输出参数：指向布尔值的指针，表示是否需要触发条件
  {
    // 定义一个指向rhc_instance结构体的指针变量inst
    struct rhc_instance *inst;
    // 定义一个整型变量ret
    int ret;

    // 新建一个读取器实例。可能仍然会过滤掉键值。
    // 在这里进行过滤意味着在接受数据的正常情况下避免了过滤处理，
    // 在数据被过滤掉的情况下接受了一些额外的开销。
    // 当然，对于这些IIDs来说，使用avl树并不是很明智，
    // 如果将AVL树替换为哈希表，那么在这里的过滤代码的开销权衡应该相当不错。

    // 注意：永远不会基于被过滤掉的样本实例化，
    // 尽管可以争论的是，如果它是基于属性（而不是键）被拒绝，
    // 应该实例化一个空实例。
    if (has_data && !content_filter_accepts(rhc->reader, sample, NULL, wrinfo->iid, tk->m_iid)) {
      return RHC_FILTERED;
    }

    // 检查资源max_instances QoS是否超出限制
    if (rhc->reader && rhc->max_instances != DDS_LENGTH_UNLIMITED &&
        rhc->n_instances >= (uint32_t)rhc->max_instances) {
      cb_data->raw_status_id = (int)DDS_SAMPLE_REJECTED_STATUS_ID;
      cb_data->extra = DDS_REJECTED_BY_INSTANCES_LIMIT;
      cb_data->handle = tk->m_iid;
      cb_data->add = true;
      return RHC_REJECTED;
    }

    // 分配并创建新的实例
    inst = alloc_new_instance(rhc, wrinfo, sample, tk);
    // 如果有数据
    if (has_data) {
      // 添加样本
      if (!add_sample(rhc, inst, wrinfo, sample, cb_data, trig_qc, nda)) {
        // 如果添加失败，释放空实例并返回RHC_REJECTED
        free_empty_instance(inst, rhc);
        return RHC_REJECTED;
      }
    } else {
      // 如果没有数据且实例已被处理，则设置无效样本
      if (inst->isdisposed) inst_set_invsample(rhc, inst, trig_qc, nda);
    }
    // 账户从空到非空的转换
    account_for_empty_to_nonempty_transition(rhc, inst);
    // 将实例添加到rhc->instances中
    ret = ddsrt_hh_add(rhc->instances, inst);
    // 断言返回值为真
    assert(ret);
    // 忽略返回值
    (void)ret;
    // 增加实例计数器
    rhc->n_instances++;
    // 增加新实例计数器
    rhc->n_new++;

    // 设置输出实例
    *out_inst = inst;
    // 返回RHC_STORED状态
    return RHC_STORED;
  }

  // 更新实例后的后处理函数
  static void postprocess_instance_update(
      struct dds_rhc_default *__restrict rhc, struct rhc_instance *__restrict *__restrict instptr,
      const struct trigger_info_pre *pre, const struct trigger_info_post *post,
      struct trigger_info_qcond *trig_qc) {
    {
      // 获取实例指针
      struct rhc_instance *inst = *instptr;

#ifdef DDS_HAS_DEADLINE_MISSED
      // 如果实例已被处理
      if (inst->isdisposed) {
        // 如果存在deadline注册
        if (inst->deadline_reg) {
          // 取消deadline注册
          inst->deadline_reg = 0;
          // 解除实例的deadline锁定
          ddsi_deadline_unregister_instance_locked(&rhc->deadline, &inst->deadline);
        }
      } else {
        // 如果存在deadline注册
        if (inst->deadline_reg)
          // 更新实例的deadline锁定
          ddsi_deadline_renew_instance_locked(&rhc->deadline, &inst->deadline);
        else {
          // 注册实例的deadline锁定
          ddsi_deadline_register_instance_locked(&rhc->deadline, &inst->deadline,
                                                 ddsrt_time_monotonic());
          // 设置deadline注册标志
          inst->deadline_reg = 1;
        }
      }
#endif

      // 如果实例为空且写入计数为0
      if (inst_is_empty(inst) && inst->wrcount == 0) {
        // 删除实例，不更新写入器
        drop_instance_noupdate_no_writers(rhc, instptr);
      }
    }

    // 如果触发信息有差异
    if (trigger_info_differs(rhc, pre, post, trig_qc))
      // 更新锁定条件
      update_conditions_locked(rhc, true, pre, post, trig_qc, *instptr);

    // 断言检查计数锁定正确
    assert(rhc_check_counts_locked(rhc, true, true));
  }

  // 更新视图状态和处理状态函数
  static void update_viewstate_and_disposedness(
      struct dds_rhc_default *__restrict rhc, struct rhc_instance *__restrict inst, bool has_data,
      bool not_alive, bool is_dispose, bool *__restrict nda) {
    // 对于NOT_ALIVE实例接收到的样本 => 视图状态为NEW
    if (has_data && not_alive) {
      // 输出跟踪信息
      TRACE(" notalive->alive");
      // 设置实例为新实例
      inst->isnew = 1;
      // 设置非活动状态
      *nda = true;
    }

    /* Desired effect on instance state and disposed_gen:
         op     DISPOSED    NOT_DISPOSED
         W      ND;gen++    ND
         D      D           D
         WD     D;gen++     D
       Simplest way is to toggle istate when it is currently DISPOSED
       and the operation is WD. */
    // 判断是否有数据并且实例已被处理
    if (has_data && inst->isdisposed) {
      // 输出跟踪信息
      TRACE(" disposed->notdisposed");
      // 处理的实例计数加1
      inst->disposed_gen++;
      // 如果不是处置操作，将实例的处理状态设为0
      if (!is_dispose) inst->isdisposed = 0;
      // 设置通知数据可用标志为真
      *nda = true;
    }
    // 如果是处置操作
    if (is_dispose) {
      // 记录实例是否已被处理
      bool wasdisposed = inst->isdisposed;
      // 如果实例未被处理
      if (!inst->isdisposed) {
        // 将实例的处理状态设为1
        inst->isdisposed = 1;
        // 设置通知数据可用标志为真
        *nda = true;
      }
      // 输出跟踪信息
      TRACE(" dispose(%d)", !wasdisposed);
    }

    /*
      dds_rhc_store: DDSI up call into read cache to store new sample. Returns whether sample
      delivered (true unless a reliable sample rejected).
    */

    // 定义默认的dds_rhc存储函数
    static bool dds_rhc_default_store(
        struct ddsi_rhc *__restrict rhc_common, const struct ddsi_writer_info *__restrict wrinfo,
        struct ddsi_serdata *__restrict sample, struct ddsi_tkmap_instance *__restrict tk) {
      // 定义默认的dds_rhc结构体指针
      struct dds_rhc_default *const __restrict rhc = (struct dds_rhc_default *__restrict)rhc_common;
      // 定义写入实例的ID
      const uint64_t wr_iid = wrinfo->iid;
      // 定义状态信息
      const uint32_t statusinfo = sample->statusinfo;
      // 判断是否有数据
      const bool has_data = (sample->kind == SDK_DATA);
      // 判断是否是处置操作
      const int is_dispose = (statusinfo & DDSI_STATUSINFO_DISPOSE) != 0;
      // 定义虚拟实例结构体
      struct rhc_instance dummy_instance;
      // 定义实例指针
      struct rhc_instance *inst;
      // 定义触发器信息预处理结构体
      struct trigger_info_pre pre;
      // 定义触发器信息后处理结构体
      struct trigger_info_post post;
      // 定义触发器信息查询条件结构体
      struct trigger_info_qcond trig_qc;
      // 定义存储结果枚举变量
      rhc_store_result_t stored;
      // 定义读取状态回调数据结构体
      ddsi_status_cb_data_t cb_data;
      // 定义通知数据可用布尔变量
      bool notify_data_available;

      // 输出跟踪信息
      TRACE("rhc_store %" PRIx64 ",%" PRIx64 " si %" PRIx32 " has_data %d:", tk->m_iid, wr_iid,
            statusinfo, has_data);
      // 如果没有数据并且状态信息为0
      if (!has_data && statusinfo == 0) {
        // 写入仅包含键的数据，这可能是一个注册操作，我们将其隐式执行（目前DDSI2不允许通过）
        TRACE(" ignore explicit register\n");
        // 返回真
        return true;
      }
    }
    // 设置 notify_data_available 为 false
    notify_data_available = false;
    // 将 dummy_instance 的 iid 设置为 tk->m_iid
    dummy_instance.iid = tk->m_iid;
    // 将 stored 设置为 RHC_FILTERED
    stored = RHC_FILTERED;
    // 初始化 cb_data 的 raw_status_id 为 -1
    cb_data.raw_status_id = -1;

    // 初始化触发器条件信息
    init_trigger_info_qcond(&trig_qc);

    // 对 rhc 的锁进行加锁操作
    ddsrt_mutex_lock(&rhc->lock);

    // 在 rhc 的实例中查找与 dummy_instance 匹配的实例
    inst = ddsrt_hh_lookup(rhc->instances, &dummy_instance);
    // 如果 inst 为空
    if (inst == NULL) {
      // 这个读取器的新实例。如果没有数据内容 -- 不是（也不是）
      // 写入 -- 忽略它，我认为我们可以忽略未知实例上的 dispose 或 unregister
      if (!has_data && !is_dispose) {
        TRACE(" unreg on unknown instance\n");
        goto error_or_nochange;
      } else {
        TRACE(" new instance\n");
        // 存储新实例
        stored = rhc_store_new_instance(&inst, rhc, wrinfo, sample, tk, has_data, &cb_data,
                                        &trig_qc, &notify_data_available);
        if (stored != RHC_STORED) goto error_or_nochange;

        // 初始化非匹配的通用触发器信息
        init_trigger_info_cmn_nonmatch(&pre.c);
      }
    }
    // 如果实例不接受样本
    else if (!inst_accepts_sample(rhc, inst, wrinfo, sample, has_data)) {
      // 被拒绝的样本（和 dispose）仍应注册写入器；
      // 必须处理注销，否则我们会有内存泄漏。（我们
      // 将引发 SAMPLE_REJECTED，并指示系统应该
      // 自杀。）不根据被拒绝的样本让实例进入 ALIVE 或 NEW 状态 - （没人知道，似乎）
      TRACE(" instance rejects sample\n");
      // 获取触发器信息
      get_trigger_info_pre(&pre, inst);

      // 如果有数据或者是处置操作
      if (has_data || is_dispose) {
        // 注册实例
        dds_rhc_register(rhc, inst, wr_iid, wrinfo->auto_dispose, false, &notify_data_available);

        // 如果需要通知数据可用
        if (notify_data_available) {
          // 如果实例的最新样本为空或已读
          if (inst->latest == NULL || inst->latest->isread) {
            // 判断实例是否为空
            const bool was_empty = inst_is_empty(inst);

            // 设置无效样本
            inst_set_invsample(rhc, inst, &trig_qc, &notify_data_available);

            // 如果实例原本为空，处理空到非空的转换
            if (was_empty) account_for_empty_to_nonempty_transition(rhc, inst);
          }
        }
      }

      // 通知样本丢失
      cb_data.raw_status_id = (int)DDS_SAMPLE_LOST_STATUS_ID;
      cb_data.extra = 0;
      cb_data.handle = 0;
      cb_data.add = true;
    } else {
      // 获取触发器信息
      get_trigger_info_pre(&pre, inst);

      // 输出实例的写入计数
      TRACE(" wc %" PRIu32, inst->wrcount);

      // 如果有数据或者是处置操作
      if (has_data || is_dispose) {
        // 确定实例当前是否为 NOT_ALIVE 状态
        const int not_alive = inst->wrcount == 0 || inst->isdisposed;
        const bool old_isdisposed = inst->isdisposed;
        const bool old_isnew = inst->isnew;
        const bool was_empty = inst_is_empty(inst);

        // 注册实例并更新视图状态和处置状态
        dds_rhc_register(rhc, inst, wr_iid, wrinfo->auto_dispose, true, &notify_data_available);
        update_viewstate_and_disposedness(rhc, inst, has_data, not_alive, is_dispose,
                                          &notify_data_available);

        // 如果输入确实是一个样本，则需要将样本添加到历史记录中
        if (has_data) {
          TRACE(" add_sample");
          if (!add_sample(rhc, inst, wrinfo, sample, &cb_data, &trig_qc, &notify_data_available)) {
            TRACE("(reject)\n");
            stored = RHC_REJECTED;

            // FIXME:
            // 修复错误的拒绝处理，可能会重新放入正确的回滚，直到那时这样的权宜之计就必须这样做：
            inst->isnew = old_isnew;
            if (old_isdisposed) inst->disposed_gen--;
            inst->isdisposed = old_isdisposed;
            goto error_or_nochange;
          }
        }

        // 如果实例变为已处置且没有剩余样本，则添加无效样本
        if ((bool)inst->isdisposed > old_isdisposed &&
            (inst->latest == NULL || inst->latest->isread))
          inst_set_invsample(rhc, inst, &trig_qc, &notify_data_available);

        // 更新实例的写入器ID
        update_inst_have_wr_iid(inst, wrinfo, sample->timestamp);

        // 处理实例状态的变化
        if (inst->latest || (bool)inst->isdisposed > old_isdisposed) {
          if (was_empty)
            account_for_empty_to_nonempty_transition(rhc, inst);
          else
            rhc->n_not_alive_disposed += (uint32_t)(inst->isdisposed - old_isdisposed);
          rhc->n_new += (uint32_t)(inst->isnew - old_isnew);
        } else {
          assert(inst_is_empty(inst) == was_empty);
        }
      }
      // 打印通知数据可用的值
      TRACE(" nda=%d\n", notify_data_available);
      // 断言检查锁定的rhc计数是否正确（不需要更新和不需要触发）
      assert(rhc_check_counts_locked(rhc, false, false));
    }

    // 如果状态信息包含DDSI_STATUSINFO_UNREGISTER
    if (statusinfo & DDSI_STATUSINFO_UNREGISTER) {
      /* 注释：
         这可能是一个纯粹的注销操作，或者实例因时间戳、内容过滤器或其他原因拒绝了样本。
         如果写入者注销实例，我认为我们应该忽略接受过滤器并继续处理。

         对于以下情况：

           write_w_timestamp(x,1) ; unregister_w_timestamp(x,0)

         如果选择了BY_SOURCE排序，其含义有些不清楚：这是否意味着在写入后读取“x”的应用程序，
         并在注销后再次读取它，将看到no_writers_generation字段的更改？ */
      dds_rhc_unregister(rhc, inst, wrinfo, sample->timestamp, &post, &trig_qc,
                         &notify_data_available);
    } else {
      // 获取触发器信息
      get_trigger_info_cmn(&post.c, inst);
    }

    // 处理实例更新后的操作
    postprocess_instance_update(rhc, &inst, &pre, &post, &trig_qc);

  // 错误或无变化的标签
  error_or_nochange:
    // 解锁rhc的互斥锁
    ddsrt_mutex_unlock(&rhc->lock);

    // 如果存在rhc的读取器
    if (rhc->reader) {
      // 如果通知数据可用，则调用数据可用回调函数
      if (notify_data_available) dds_reader_data_available_cb(rhc->reader);
      // 如果原始状态ID大于等于0，则调用读取器状态回调函数
      if (cb_data.raw_status_id >= 0) dds_reader_status_cb(&rhc->reader->m_entity, &cb_data);
    }
    // 返回结果，如果rhc是可靠的且存储被拒绝，则返回false，否则返回true
    return !(rhc->reliable && stored == RHC_REJECTED);
  }
  // 定义一个静态函数 dds_rhc_default_unregister_wr，用于在特定的 writer 死亡时取消注册
  static void dds_rhc_default_unregister_wr(struct ddsi_rhc *__restrict rhc_common,
                                            const struct ddsi_writer_info *__restrict wrinfo) {
    // 只有当具有 ID WR_IID 的 writer 已死亡时才调用此函数。
    //
    // 如果我们要求它永远不会复活，即下次为同一个 writer 使用新的 WR_IID，
    // 那么我们有足够的时间来扫描缓存并清理，并且我们不必一直保持锁定（即使我们现在就这样做）。
    //
    // 在没有生成内置主题的情况下，WR_IID 从未重用过，但是这些内置主题确实需要相同的实例 id
    // 对于那些 GUID 中仍然存在某个读取器的实例。因此，如果希望在不锁定 RHC 的情况下进行注销，
    // 实体需要获得两个 IID：应用程序在内置主题中可见的一个以及在 get_instance_handle 中可见的一个，
    // 以及内部用于跟踪注册和注销的一个。

    // 将 rhc_common 转换为 dds_rhc_default 类型的指针
    struct dds_rhc_default *__restrict const rhc = (struct dds_rhc_default *__restrict)rhc_common;
    bool notify_data_available = false;
    struct rhc_instance *inst;
    struct ddsrt_hh_iter iter;
    const uint64_t wr_iid = wrinfo->iid;

    // 锁定 rhc 的互斥锁
    ddsrt_mutex_lock(&rhc->lock);
    TRACE("rhc_unregister_wr_iid %" PRIx64 ",%d:\n", wr_iid, wrinfo->auto_dispose);
    // 遍历 rhc 的实例
    for (inst = ddsrt_hh_iter_first(rhc->instances, &iter); inst;
         inst = ddsrt_hh_iter_next(&iter)) {
      // 如果实例的 writer iid 是活动的并且与给定的 writer iid 相等，或者实例包含在注册表中
      if ((inst->wr_iid_islive && inst->wr_iid == wr_iid) ||
          lwregs_contains(&rhc->registrations, inst->iid, wr_iid)) {
        // 断言实例的 wrcount 大于 0
        assert(inst->wrcount > 0);
        struct trigger_info_pre pre;
        struct trigger_info_post post;
        struct trigger_info_qcond trig_qc;
        get_trigger_info_pre(&pre, inst);
        init_trigger_info_qcond(&trig_qc);
        TRACE("  %" PRIx64 ":", inst->iid);
        // 取消注册实例
        dds_rhc_unregister(rhc, inst, wrinfo, inst->tstamp, &post, &trig_qc,
                           &notify_data_available);
        // 对实例进行后处理更新
        postprocess_instance_update(rhc, &inst, &pre, &post, &trig_qc);
        TRACE("\n");
      }
    }
    // 解锁 rhc 的互斥锁
    ddsrt_mutex_unlock(&rhc->lock);

    // 如果有可用的数据，调用回调函数通知读取器
    if (rhc->reader && notify_data_available) dds_reader_data_available_cb(rhc->reader);
  }
  // 释放指定写入者实例的所有权
  static void dds_rhc_default_relinquish_ownership(struct ddsi_rhc *__restrict rhc_common,
                                                   const uint64_t wr_iid) {
    // 将通用读取历史缓存转换为默认读取历史缓存类型
    struct dds_rhc_default *__restrict const rhc = (struct dds_rhc_default *__restrict)rhc_common;
    struct rhc_instance *inst;
    struct ddsrt_hh_iter iter;

    // 锁定读取历史缓存
    ddsrt_mutex_lock(&rhc->lock);
    TRACE("rhc_relinquish_ownership(%" PRIx64 ":\n", wr_iid);

    // 遍历所有实例
    for (inst = ddsrt_hh_iter_first(rhc->instances, &iter); inst;
         inst = ddsrt_hh_iter_next(&iter)) {
      // 如果实例的写入者ID与给定的写入者ID匹配，则取消实例的活跃状态
      if (inst->wr_iid_islive && inst->wr_iid == wr_iid) {
        inst->wr_iid_islive = 0;
      }
    }

    TRACE(")\n");

    // 检查锁定的读取历史缓存计数是否正确
    assert(rhc_check_counts_locked(rhc, true, false));

    // 解锁读取历史缓存
    ddsrt_mutex_unlock(&rhc->lock);
  }

  // 状态：
  //   sample:   ANY, READ, NOT_READ
  //   view:     ANY, NEW, NOT_NEW
  //   instance: ANY, ALIVE, NOT_ALIVE, NOT_ALIVE_NO_WRITERS, NOT_ALIVE_DISPOSED

  // 获取实例的查询掩码
  static uint32_t qmask_of_inst(const struct rhc_instance *inst) {
    // 如果实例是新的，则设置为DDS_NEW_VIEW_STATE，否则设置为DDS_NOT_NEW_VIEW_STATE
    uint32_t qm = inst->isnew ? DDS_NEW_VIEW_STATE : DDS_NOT_NEW_VIEW_STATE;

    // 根据实例状态设置相应的查询掩码
    if (inst->isdisposed)
      qm |= DDS_NOT_ALIVE_DISPOSED_INSTANCE_STATE;
    else if (inst->wrcount > 0)
      qm |= DDS_ALIVE_INSTANCE_STATE;
    else
      qm |= DDS_NOT_ALIVE_NO_WRITERS_INSTANCE_STATE;

    return qm;
  }
  // 定义一个静态函数，根据给定的sample_states、view_states和instance_states参数生成qmask
  static uint32_t qmask_from_dcpsquery(uint32_t sample_states, uint32_t view_states,
                                       uint32_t instance_states) {
    // 初始化qminv变量为0
    uint32_t qminv = 0;

    // 根据sample_states的值进行switch判断
    switch ((dds_sample_state_t)sample_states) {
      case DDS_SST_READ:
        // 如果sample_states为DDS_SST_READ，则将qminv与DDS_NOT_READ_SAMPLE_STATE进行按位或操作
        qminv |= DDS_NOT_READ_SAMPLE_STATE;
        break;
      case DDS_SST_NOT_READ:
        // 如果sample_states为DDS_SST_NOT_READ，则将qminv与DDS_READ_SAMPLE_STATE进行按位或操作
        qminv |= DDS_READ_SAMPLE_STATE;
        break;
    }
    // 根据view_states的值进行switch判断
    switch ((dds_view_state_t)view_states) {
      case DDS_VST_NEW:
        // 如果view_states为DDS_VST_NEW，则将qminv与DDS_NOT_NEW_VIEW_STATE进行按位或操作
        qminv |= DDS_NOT_NEW_VIEW_STATE;
        break;
      case DDS_VST_OLD:
        // 如果view_states为DDS_VST_OLD，则将qminv与DDS_NEW_VIEW_STATE进行按位或操作
        qminv |= DDS_NEW_VIEW_STATE;
        break;
    }
    // 根据instance_states的值进行switch判断
    switch (instance_states) {
      case DDS_IST_ALIVE:
        // 如果instance_states为DDS_IST_ALIVE，则将qminv与以下两个值进行按位或操作
        qminv |= DDS_NOT_ALIVE_DISPOSED_INSTANCE_STATE | DDS_NOT_ALIVE_NO_WRITERS_INSTANCE_STATE;
        break;
      case DDS_IST_ALIVE | DDS_IST_NOT_ALIVE_DISPOSED:
        // 如果instance_states为DDS_IST_ALIVE和DDS_IST_NOT_ALIVE_DISPOSED的组合，则将qminv与DDS_NOT_ALIVE_NO_WRITERS_INSTANCE_STATE进行按位或操作
        qminv |= DDS_NOT_ALIVE_NO_WRITERS_INSTANCE_STATE;
        break;
      case DDS_IST_ALIVE | DDS_IST_NOT_ALIVE_NO_WRITERS:
        // 如果instance_states为DDS_IST_ALIVE和DDS_IST_NOT_ALIVE_NO_WRITERS的组合，则将qminv与DDS_NOT_ALIVE_DISPOSED_INSTANCE_STATE进行按位或操作
        qminv |= DDS_NOT_ALIVE_DISPOSED_INSTANCE_STATE;
        break;
      case DDS_IST_NOT_ALIVE_DISPOSED:
        // 如果instance_states为DDS_IST_NOT_ALIVE_DISPOSED，则将qminv与以下两个值进行按位或操作
        qminv |= DDS_ALIVE_INSTANCE_STATE | DDS_NOT_ALIVE_NO_WRITERS_INSTANCE_STATE;
        break;
      case DDS_IST_NOT_ALIVE_DISPOSED | DDS_IST_NOT_ALIVE_NO_WRITERS:
        // 如果instance_states为DDS_IST_NOT_ALIVE_DISPOSED和DDS_IST_NOT_ALIVE_NO_WRITERS的组合，则将qminv与DDS_ALIVE_INSTANCE_STATE进行按位或操作
        qminv |= DDS_ALIVE_INSTANCE_STATE;
        break;
      case DDS_IST_NOT_ALIVE_NO_WRITERS:
        // 如果instance_states为DDS_IST_NOT_ALIVE_NO_WRITERS，则将qminv与以下两个值进行按位或操作
        qminv |= DDS_ALIVE_INSTANCE_STATE | DDS_NOT_ALIVE_DISPOSED_INSTANCE_STATE;
        break;
    }
    // 返回qminv的值
    return qminv;
  }
  // 根据给定的掩码和条件，计算查询掩码
  static uint32_t qmask_from_mask_n_cond(uint32_t mask, dds_readcond * cond) {
    uint32_t qminv;
    // 如果没有设置掩码
    if (mask == NO_STATE_MASK_SET) {
      // 如果有条件
      if (cond) {
        // 没有设置掩码，使用条件中的掩码
        qminv = cond->m_qminv;
      } else {
        // 没有设置掩码且没有条件：读取所有
        qminv =
            qmask_from_dcpsquery(DDS_ANY_SAMPLE_STATE, DDS_ANY_VIEW_STATE, DDS_ANY_INSTANCE_STATE);
      }
    } else {
      // 需要时合并给定的掩码与条件掩码
      qminv = qmask_from_dcpsquery(mask & DDS_ANY_SAMPLE_STATE, mask & DDS_ANY_VIEW_STATE,
                                   mask & DDS_ANY_INSTANCE_STATE);
      if (cond != NULL) {
        qminv &= cond->m_qminv;
      }
    }
    return qminv;
  }

  // 设置样本信息
  static void set_sample_info(dds_sample_info_t * si, const struct rhc_instance *inst,
                              const struct rhc_sample *sample) {
    si->sample_state = sample->isread ? DDS_SST_READ : DDS_SST_NOT_READ;
    si->view_state = inst->isnew ? DDS_VST_NEW : DDS_VST_OLD;
    si->instance_state = inst->isdisposed       ? DDS_IST_NOT_ALIVE_DISPOSED
                         : (inst->wrcount == 0) ? DDS_IST_NOT_ALIVE_NO_WRITERS
                                                : DDS_IST_ALIVE;
    si->instance_handle = inst->iid;
    si->publication_handle = sample->wr_iid;
    si->disposed_generation_count = sample->disposed_gen;
    si->no_writers_generation_count = sample->no_writers_gen;
    si->sample_rank = 0;      // 后续修补：尚不知道返回集中的最后一个样本
    si->generation_rank = 0;  // __/
    si->absolute_generation_rank = (inst->disposed_gen + inst->no_writers_gen) -
                                   (sample->disposed_gen + sample->no_writers_gen);
    si->valid_data = true;
    si->source_timestamp = sample->sample->timestamp.v;
  }

  // 设置无效样本的样本信息
  static void set_sample_info_invsample(dds_sample_info_t * si, const struct rhc_instance *inst) {
    si->sample_state = inst->inv_isread ? DDS_SST_READ : DDS_SST_NOT_READ;
    si->view_state = inst->isnew ? DDS_VST_NEW : DDS_VST_OLD;
    si->instance_state = inst->isdisposed       ? DDS_IST_NOT_ALIVE_DISPOSED
                         : (inst->wrcount == 0) ? DDS_IST_NOT_ALIVE_NO_WRITERS
                                                : DDS_IST_ALIVE;
    si->instance_handle = inst->iid;
    si->publication_handle = inst->wr_iid;
    si->disposed_generation_count = inst->disposed_gen;
    si->no_writers_generation_count = inst->no_writers_gen;
    si->sample_rank = 0;  // 按构造方式始终是集合中的最后一个（但会被修补）
    si->generation_rank = 0;  // __/
    si->absolute_generation_rank = 0;
    si->valid_data = false;
    si->source_timestamp = inst->tstamp.v;
  }
  // 定义一个静态函数patch_generations，用于修正dds_sample_info_t结构体数组中的generation信息
  static void patch_generations(dds_sample_info_t * si, uint32_t last_of_inst) {
    // 如果last_of_inst大于0，表示有需要修正的generation信息
    if (last_of_inst > 0) {
      // 计算参考值ref，等于disposed_generation_count和no_writers_generation_count之和
      const uint32_t ref =
          si[last_of_inst].disposed_generation_count + si[last_of_inst].no_writers_generation_count;

      // 断言sample_rank和generation_rank都为0
      assert(si[last_of_inst].sample_rank == 0);
      assert(si[last_of_inst].generation_rank == 0);

      // 遍历si数组，更新sample_rank和generation_rank
      for (uint32_t i = 0; i < last_of_inst; i++) {
        si[i].sample_rank = last_of_inst - i;
        si[i].generation_rank =
            ref - (si[i].disposed_generation_count + si[i].no_writers_generation_count);
      }
    }
  }

  // 定义一个静态函数read_sample_update_conditions，用于在读取样本时更新条件状态
  static bool read_sample_update_conditions(
      struct dds_rhc_default * rhc, struct trigger_info_pre * pre, struct trigger_info_post * post,
      struct trigger_info_qcond * trig_qc, struct rhc_instance * inst, dds_querycond_mask_t conds,
      bool sample_wasread) {
    // 如果没有依赖于样本状态的查询条件，则返回false
    if (rhc->qconds_samplest == 0) return false;

    // 如果没有匹配此样本的查询条件，则返回false
    if ((conds & rhc->qconds_samplest) == 0) return false;

    // 输出调试信息
    TRACE("read_sample_update_conditions\n");

    // 更新trig_qc结构体中的相关字段
    trig_qc->dec_conds_sample = trig_qc->inc_conds_sample = conds;
    trig_qc->dec_sample_read = sample_wasread;
    trig_qc->inc_sample_read = true;

    // 获取触发器信息并更新条件状态
    get_trigger_info_cmn(&post->c, inst);
    update_conditions_locked(rhc, false, pre, post, trig_qc, inst);

    // 重置trig_qc结构体中的相关字段
    trig_qc->dec_conds_sample = trig_qc->inc_conds_sample = 0;
    pre->c = post->c;

    return false;
  }

  // 定义一个静态函数take_sample_update_conditions，用于在获取样本时更新条件状态
  static bool take_sample_update_conditions(
      struct dds_rhc_default * rhc, struct trigger_info_pre * pre, struct trigger_info_post * post,
      struct trigger_info_qcond * trig_qc, struct rhc_instance * inst, dds_querycond_mask_t conds,
      bool sample_wasread) {
    // 如果没有查询条件或者没有匹配此样本的查询条件，则返回false
    if (rhc->nqconds == 0 || conds == 0) return false;

    // 输出调试信息
    TRACE("take_sample_update_conditions\n");

    // 更新trig_qc结构体中的相关字段
    trig_qc->dec_conds_sample = conds;
    trig_qc->dec_sample_read = sample_wasread;

    // 获取触发器信息并更新条件状态
    get_trigger_info_cmn(&post->c, inst);
    update_conditions_locked(rhc, false, pre, post, trig_qc, inst);

    // 重置trig_qc结构体中的相关字段
    trig_qc->dec_conds_sample = 0;
    pre->c = post->c;

    return false;
  }
  // 定义函数指针类型 read_take_to_sample_t，用于将序列化数据转换为样本
  typedef bool (*read_take_to_sample_t)(
      const struct ddsi_serdata *__restrict d, void *__restrict *__restrict sample,
      void *__restrict *__restrict bufptr, void *__restrict buflim);

  // 定义函数指针类型 read_take_to_invsample_t，用于将序列化数据转换为无类型的样本
  typedef bool (*read_take_to_invsample_t)(
      const struct ddsi_sertype *__restrict type, const struct ddsi_serdata *__restrict d,
      void *__restrict *__restrict sample, void *__restrict *__restrict bufptr,
      void *__restrict buflim);

  // 实现 read_take_to_sample 函数，将序列化数据 d 转换为样本 *sample
  static bool read_take_to_sample(const struct ddsi_serdata *__restrict d,
                                  void *__restrict *__restrict sample,
                                  void *__restrict *__restrict bufptr, void *__restrict buflim) {
    return ddsi_serdata_to_sample(d, *sample, (void **)bufptr, buflim);
  }

  // 实现 read_take_to_invsample 函数，将序列化数据 d 转换为无类型的样本 *sample
  static bool read_take_to_invsample(const struct ddsi_sertype *__restrict type,
                                     const struct ddsi_serdata *__restrict d,
                                     void *__restrict *__restrict sample,
                                     void *__restrict *__restrict bufptr, void *__restrict buflim) {
    return untyped_to_clean_invsample(type, d, *sample, (void **)bufptr, buflim);
  }

  // 实现 read_take_to_sample_ref 函数，将序列化数据 d 的引用赋值给 *sample
  static bool read_take_to_sample_ref(
      const struct ddsi_serdata *__restrict d, void *__restrict *__restrict sample,
      void *__restrict *__restrict bufptr, void *__restrict buflim) {
    (void)bufptr;
    (void)buflim;
    *sample = ddsi_serdata_ref(d);
    return true;
  }

  // 实现 read_take_to_invsample_ref 函数，将序列化数据 d 的引用赋值给无类型的 *sample
  static bool read_take_to_invsample_ref(
      const struct ddsi_sertype *__restrict type, const struct ddsi_serdata *__restrict d,
      void *__restrict *__restrict sample, void *__restrict *__restrict bufptr,
      void *__restrict buflim) {
    (void)type;
    (void)bufptr;
    (void)buflim;
    *sample = ddsi_serdata_ref(d);
    return true;
  }
  // 定义一个静态函数，返回值类型为 int32_t，用于读取实例数据
  static int32_t read_w_qminv_inst(
      struct dds_rhc_default *const __restrict rhc,  // 传入一个指向dds_rhc_default结构体的指针
      struct rhc_instance *const __restrict inst,  // 传入一个指向rhc_instance结构体的指针
      void *__restrict *__restrict values,  // 传入一个二级指针，用于存储读取到的数据
      dds_sample_info_t
          *__restrict info_seq,  // 传入一个指向dds_sample_info_t结构体的指针，用于存储样本信息
      const int32_t max_samples,  // 传入一个常量整数，表示最大可读取的样本数量
      const uint32_t qminv,  // 传入一个常量无符号整数，表示查询条件中的最小可见性
      const dds_querycond_mask_t qcmask,  // 传入一个常量dds_querycond_mask_t类型，表示查询条件掩码
      read_take_to_sample_t to_sample,  // 传入一个函数指针，用于将读取到的数据转换为样本
      read_take_to_invsample_t to_invsample  // 传入一个函数指针，用于将读取到的数据转换为无效样本
  ) {
    // 断言：max_samples 必须大于0
    assert(max_samples > 0);

    // 判断实例是否为空，或者实例的状态与qminv不匹配
    if (inst_is_empty(inst) || (qmask_of_inst(inst) & qminv) != 0) {
      // 如果满足条件，说明没有样本存在或者实例/视图状态不匹配，返回0
      return 0;
    }

    // 定义三个结构体变量：pre, post, trig_qc
    struct trigger_info_pre pre;
    struct trigger_info_post post;
    struct trigger_info_qcond trig_qc;

    // 获取实例中已读取的样本数量
    const uint32_t nread = inst_nread(inst);

    // 初始化计数器 n 为0
    int32_t n = 0;

    // 获取触发信息
    get_trigger_info_pre(&pre, inst);
    init_trigger_info_qcond(&trig_qc);

    // 判断实例中是否有有效样本
    if (inst->latest) {
      // 定义一个指向rhc_sample结构体的指针 sample，并初始化为实例中最新的样本
      struct rhc_sample *sample = inst->latest->next, *const end1 = sample;

      // 遍历实例中的所有样本，直到达到最大样本数或遍历完所有样本
      do {
        // 判断样本状态是否与qminv和qcmask匹配
        if ((qmask_of_sample(sample) & qminv) == 0 && (qcmask == 0 || (sample->conds & qcmask))) {
          // 如果匹配，则设置样本信息
          set_sample_info(info_seq + n, inst, sample);

          // 转换样本数据
          to_sample(sample->sample, values + n, 0, 0);

          // 如果样本未读，则更新条件并将样本标记为已读
          if (!sample->isread) {
            read_sample_update_conditions(rhc, &pre, &post, &trig_qc, inst, sample->conds, false);
            sample->isread = true;
            inst->nvread++;
            rhc->n_vread++;
          }

          // 计数器加1
          ++n;
        }

        // 移动到下一个样本
        sample = sample->next;
      } while (n < max_samples && sample != end1);
    }

    // 判断实例中是否有无效样本，且结果中还有空间存储
    if (inst->inv_exists && n < max_samples && (qmask_of_invsample(inst) & qminv) == 0 &&
        (qcmask == 0 || (inst->conds & qcmask))) {
      // 设置无效样本信息
      set_sample_info_invsample(info_seq + n, inst);

      // 转换无效样本数据
      to_invsample(rhc->type, inst->tk->m_sample, values + n, 0, 0);

      // 如果无效样本未读，则更新条件并将无效样本标记为已读
      if (!inst->inv_isread) {
        read_sample_update_conditions(rhc, &pre, &post, &trig_qc, inst, inst->conds, false);
        inst->inv_isread = 1;
        rhc->n_invread++;
      }

      // 计数器加1
      ++n;
    }

    // 如果有读取到的样本，则设置样本信息中的生成计数，并更新实例状态
    bool inst_became_old = false;
    if (n > 0) {
      patch_generations(info_seq, (uint32_t)n - 1);
      if (inst->isnew) {
        inst_became_old = true;
        inst->isnew = 0;
        rhc->n_new--;
      }
    }

    // 如果实例中已读取的样本数量发生变化，或者实例变为旧实例，则更新触发条件
    if (nread != inst_nread(inst) || inst_became_old) {
      get_trigger_info_cmn(&post.c, inst);
      assert(trig_qc.dec_conds_invsample == 0);
      assert(trig_qc.dec_conds_sample == 0);
      assert(trig_qc.inc_conds_invsample == 0);
      assert(trig_qc.inc_conds_sample == 0);
      update_conditions_locked(rhc, false, &pre, &post, &trig_qc, inst);
    }

    // 返回读取到的样本数量
    return n;
  }
  // 定义一个静态函数，用于从实例中获取满足条件的样本，并将其从数据读取历史缓存中删除
  // 定义一个静态函数 take_w_qminv_inst，用于处理实例的读取和删除操作
  static int32_t take_w_qminv_inst(  //
      struct dds_rhc_default
          *const __restrict rhc,  // 传入一个指向dds_rhc_default结构体的指针，表示资源管理器
      struct rhc_instance *__restrict
          *__restrict instptr,  // 传入一个指向rhc_instance结构体指针的指针，表示实例指针
      void *__restrict
          *__restrict values,  // 传入一个指向void类型指针数组的指针，表示存储数据的缓冲区
      dds_sample_info_t
          *__restrict info_seq,  // 传入一个指向dds_sample_info_t结构体数组的指针，表示样本信息序列
      const int32_t max_samples,  // 传入一个int32_t类型的常量，表示最大样本数
      const uint32_t qminv,       // 传入一个uint32_t类型的常量，表示查询条件掩码
      const dds_querycond_mask_t
          qcmask,  // 传入一个dds_querycond_mask_t类型的常量，表示查询条件掩码
      read_take_to_sample_t
          to_sample,  // 传入一个read_take_to_sample_t类型的函数指针，表示将数据转换为样本的函数
      read_take_to_invsample_t
          to_invsample)  // 传入一个read_take_to_invsample_t类型的函数指针，表示将数据转换为无效样本的函数
  {
    // 获取实例指针
    struct rhc_instance *inst = *instptr;
    // 断言最大样本数大于0
    assert(max_samples > 0);
    // 如果实例为空或实例/视图状态不匹配，则返回0
    if (inst_is_empty(inst) || (qmask_of_inst(inst) & qminv) != 0) {
      /* no samples present, or the instance/view state doesn't match */
      return 0;
    }

    // 定义触发器信息结构体变量
    struct trigger_info_pre pre;
    struct trigger_info_post post;
    struct trigger_info_qcond trig_qc;
    // 初始化样本计数为0
    int32_t n = 0;
    // 获取实例的触发器信息
    get_trigger_info_pre(&pre, inst);
    // 初始化查询条件触发器信息
    init_trigger_info_qcond(&trig_qc);

    // 判断 inst->latest 是否存在
    if (inst->latest) {
      // 定义一个指向最新样本的指针 psample
      struct rhc_sample *psample = inst->latest;
      // 定义一个指向下一个样本的指针 sample
      struct rhc_sample *sample = psample->next;
      // 获取当前实例中有效样本的数量
      uint32_t nvsamples = inst->nvsamples;

      // 遍历所有有效样本
      while (nvsamples--) {
        // 定义一个指向下一个样本的常量指针 sample1
        struct rhc_sample *const sample1 = sample->next;

        // 判断样本掩码是否匹配，或者内容谓词是否匹配
        if ((qmask_of_sample(sample) & qminv) != 0 || (qcmask != 0 && !(sample->conds & qcmask))) {
          // 样本掩码不匹配，或者内容谓词不匹配
          psample = sample;
        } else {
          // 更新条件并获取样本
          take_sample_update_conditions(rhc, &pre, &post, &trig_qc, inst, sample->conds,
                                        sample->isread);
          // 设置样本信息
          set_sample_info(info_seq + n, inst, sample);
          // 将样本转换为对应的数据类型
          to_sample(sample->sample, values + n, 0, 0);
          // 减少可见样本的数量
          rhc->n_vsamples--;

          // 如果样本已读
          if (sample->isread) {
            // 减少可见已读样本的数量
            inst->nvread--;
            rhc->n_vread--;
          }

          // 如果实例中有效样本数量减少到 0
          if (--inst->nvsamples == 0)
            inst->latest = NULL;
          else {
            // 更新最新样本指针
            if (inst->latest == sample) inst->latest = psample;

            // 更新下一个样本指针
            psample->next = sample1;
          }

          // 释放当前样本内存
          free_sample(rhc, inst, sample);

          // 如果达到最大样本数，跳出循环
          if (++n == max_samples) break;
        }

        // 更新当前样本指针
        sample = sample1;
      }
    }
    // 如果当前指令存在反向指令，且采样数小于最大采样数，且反向指令的条件码不包含qminv，且（qcmask为0或者当前指令的条件码包含qcmask）
    if (inst->inv_exists && n < max_samples && (qmask_of_invsample(inst) & qminv) == 0 &&
        (qcmask == 0 || (inst->conds & qcmask) != 0)) {
      // 声明一个触发信息结构体
      struct trigger_info_qcond dummy_trig_qc;
#ifndef NDEBUG
      // 初始化触发信息结构体
      init_trigger_info_qcond(&dummy_trig_qc);
#endif
      // 更新前后条件码
      take_sample_update_conditions(rhc, &pre, &post, &trig_qc, inst, inst->conds,
                                    inst->inv_isread);
      // 设置反向指令的信息
      set_sample_info_invsample(info_seq + n, inst);
      // 将反向指令的采样值赋值给values
      to_invsample(rhc->type, inst->tk->m_sample, values + n, 0, 0);
      // 清除反向指令
      inst_clear_invsample(rhc, inst, &dummy_trig_qc);
      // 采样数加1
      ++n;
    }
    // 判断 n 是否大于 0
    if (n > 0) {
      // 如果 n 大于 0，调用 patch_generations 函数处理 info_seq，参数为 n - 1
      patch_generations(info_seq, (uint32_t)n - 1);

      // 判断 inst 的 isnew 属性是否为 true
      if (inst->isnew) {
        // 如果是新实例，将 isnew 设为 false，并将 rhc 的 n_new 减 1
        inst->isnew = 0;
        rhc->n_new--;
      }

      // 如果 nsamples 等于 0，则不会匹配任何内容，因此无需在此处为
      // drop_instance_noupdate_no_writers 执行任何操作
      get_trigger_info_cmn(&post.c, inst);

      // 检查 trig_qc 的 dec_conds_invsample 是否等于 0
      assert(trig_qc.dec_conds_invsample == 0);

      // 检查 trig_qc 的 dec_conds_sample 是否等于 0
      assert(trig_qc.dec_conds_sample == 0);

      // 检查 trig_qc 的 inc_conds_invsample 是否等于 0
      assert(trig_qc.inc_conds_invsample == 0);

      // 检查 trig_qc 的 inc_conds_sample 是否等于 0
      assert(trig_qc.inc_conds_sample == 0);

      // 调用 update_conditions_locked 函数更新条件
      update_conditions_locked(rhc, false, &pre, &post, &trig_qc, inst);
    }

    // 判断 inst 是否为空
    if (inst_is_empty(inst))
      // 如果 inst 为空，调用 account_for_nonempty_to_empty_transition 函数处理
      account_for_nonempty_to_empty_transition(rhc, instptr, "take: ");

    // 返回 n 的值
    return n;
  }

  // 定义一个静态函数 read_w_qminv，用于读取满足条件的数据
  static int32_t read_w_qminv(  //
      struct dds_rhc_default
          *__restrict rhc,  // 限定符为__restrict的rhc指针，指向dds_rhc_default结构体
      bool lock,            // 布尔类型变量lock，表示是否需要锁定
      void *__restrict *__restrict values,  // 限定符为__restrict的values指针，指向void类型的指针
      dds_sample_info_t
          *__restrict info_seq,  // 限定符为__restrict的info_seq指针，指向dds_sample_info_t结构体
      int32_t max_samples,           // 整型变量max_samples，表示最大样本数
      uint32_t qminv,                // 无符号整型变量qminv，表示查询最小值
      dds_instance_handle_t handle,  // 变量handle，表示实例句柄
      dds_readcond *__restrict cond,  // 限定符为__restrict的cond指针，指向dds_readcond结构体
      read_take_to_sample_t to_sample,  // 函数指针to_sample，用于将读取到的数据转换为样本
      read_take_to_invsample_t
          to_invsample)  // 函数指针to_invsample，用于将读取到的数据转换为非法样本
  {
    int32_t n = 0;       // 定义整型变量n，初始化为0

    assert(max_samples > 0);  // 断言max_samples大于0

    // 如果需要锁定，则对rhc的lock进行加锁
    if (lock) {
      ddsrt_mutex_lock(&rhc->lock);
    }

    // 打印调试信息
    TRACE("read_w_qminv(%p,%p,%p,%" PRId32 ",%" PRIx32 ",%" PRIx64 ",%p) - inst %" PRIu32
          " nonempty %" PRIu32 " disp %" PRIu32 " nowr %" PRIu32 " new %" PRIu32 " samples %" PRIu32
          "+%" PRIu32 " read %" PRIu32 "+%" PRIu32 "\n",
          (void *)rhc, (void *)values, (void *)info_seq, max_samples, qminv, handle, (void *)cond,
          rhc->n_instances, rhc->n_nonempty_instances, rhc->n_not_alive_disposed,
          rhc->n_not_alive_no_writers, rhc->n_new, rhc->n_vsamples, rhc->n_invsamples, rhc->n_vread,
          rhc->n_invread);

    // 定义查询条件掩码qcmask，如果cond存在且有过滤器，则使用cond的qcmask，否则为0
    const dds_querycond_mask_t qcmask =
        (cond && cond->m_query.m_filter) ? cond->m_query.m_qcmask : 0;

    // 如果handle存在
    if (handle) {
      struct rhc_instance template, *inst;  // 定义rhc_instance结构体变量template和指针inst
      template.iid = handle;                // 将handle赋值给template的iid字段
      // 在rhc的instances中查找template，如果找到则赋值给inst
      if ((inst = ddsrt_hh_lookup(rhc->instances, &template)) != NULL)
        // 调用read_w_qminv_inst函数，读取满足条件的数据，并将结果赋值给n
        n = read_w_qminv_inst(rhc, inst, values, info_seq, max_samples, qminv, qcmask, to_sample,
                              to_invsample);
      else
        // 如果没有找到，则将DDS_RETCODE_PRECONDITION_NOT_MET赋值给n
        n = DDS_RETCODE_PRECONDITION_NOT_MET;
    }
    // 如果handle不存在且rhc的nonempty_instances不为空
    // 如果非空实例列表不为空
    else if (!ddsrt_circlist_isempty(&rhc->nonempty_instances)) {
      // 获取最旧的非空实例
      struct rhc_instance *inst = oldest_nonempty_instance(rhc);
      // 定义指针end，指向inst
      struct rhc_instance *const end = inst;
      // 使用do-while循环处理每个非空实例
      do {
        // 调用read_w_qminv_inst函数处理当前实例，并累加返回值到n
        n += read_w_qminv_inst(rhc, inst, values + n, info_seq + n, max_samples - n, qminv, qcmask,
                               to_sample, to_invsample);
        // 获取下一个非空实例
        inst = next_nonempty_instance(inst);
        // 当实例不等于end且n小于max_samples时继续循环
      } while (inst != end && n < max_samples);
    }
    // 输出调试信息，显示返回的样本数量
    TRACE("read: returning %" PRIu32 "\n", n);
    // 断言检查锁定状态下的计数是否正确
    assert(rhc_check_counts_locked(rhc, true, false));

    // FIXME: 条件“lock”加上无条件的“unlock”是不可原谅的糟糕设计
    // 看起来这是在某个时候引入的，以便另一种语言绑定可以使用dds_rhc_default_lock_samples锁定RHC，
    // 以了解样本的数量，然后分配资源并使用lock=true调用read/take。所有这些都需要修复。
    // 解锁rhc的互斥锁
    ddsrt_mutex_unlock(&rhc->lock);
    // 返回处理的样本数量
    return n;
  }
  // 定义一个静态函数 take_w_qminv，用于从缓存中获取数据
  static int32_t take_w_qminv(
      struct dds_rhc_default *__restrict rhc, bool lock, void *__restrict *__restrict values,
      dds_sample_info_t *__restrict info_seq, int32_t max_samples, uint32_t qminv,
      dds_instance_handle_t handle, dds_readcond *__restrict cond, read_take_to_sample_t to_sample,
      read_take_to_invsample_t to_invsample) {
    // 初始化计数器 n 为 0
    int32_t n = 0;
    // 断言 max_samples 大于 0
    assert(max_samples > 0);
    // 如果 lock 为 true，则锁定 rhc 的互斥锁
    if (lock) {
      ddsrt_mutex_lock(&rhc->lock);
    }

    // 打印调试信息
    TRACE("take_w_qminv(%p,%p,%p,%" PRId32 ",%" PRIx32 ",%" PRIx64 ",%p) - inst %" PRIu32
          " nonempty %" PRIu32 " disp %" PRIu32 " nowr %" PRIu32 " new %" PRIu32 " samples %" PRIu32
          "+%" PRIu32 " read %" PRIu32 "+%" PRIu32 "\n",
          (void *)rhc, (void *)values, (void *)info_seq, max_samples, qminv, handle, (void *)cond,
          rhc->n_instances, rhc->n_nonempty_instances, rhc->n_not_alive_disposed,
          rhc->n_not_alive_no_writers, rhc->n_new, rhc->n_vsamples, rhc->n_invsamples, rhc->n_vread,
          rhc->n_invread);

    // 获取查询条件掩码
    const dds_querycond_mask_t qcmask =
        (cond && cond->m_query.m_filter) ? cond->m_query.m_qcmask : 0;
    // 如果 handle 存在
    if (handle) {
      struct rhc_instance template, *inst;
      template.iid = handle;
      // 查找实例
      if ((inst = ddsrt_hh_lookup(rhc->instances, &template)) != NULL)
        n = take_w_qminv_inst(rhc, &inst, values, info_seq, max_samples, qminv, qcmask, to_sample,
                              to_invsample);
      else
        n = DDS_RETCODE_PRECONDITION_NOT_MET;
    }
    // 如果非空实例列表不为空
    else if (!ddsrt_circlist_isempty(&rhc->nonempty_instances)) {
      // 获取最旧的非空实例
      struct rhc_instance *inst = oldest_nonempty_instance(rhc);
      uint32_t n_insts = rhc->n_nonempty_instances;
      // 遍历非空实例，直到达到 max_samples 或遍历完所有实例
      while (n_insts-- > 0 && n < max_samples) {
        struct rhc_instance *const inst1 = next_nonempty_instance(inst);
        n += take_w_qminv_inst(rhc, &inst, values + n, info_seq + n, max_samples - n, qminv, qcmask,
                               to_sample, to_invsample);
        inst = inst1;
      }
    }
    // 打印调试信息
    TRACE("take: returning %" PRIu32 "\n", n);
    // 断言检查计数器
    assert(rhc_check_counts_locked(rhc, true, false));

    // FIXME: 条件 "lock" 加上无条件 "unlock" 是不可原谅的糟糕设计
    // 这似乎是在某个时候引入的，以便另一种语言绑定可以使用 dds_rhc_default_lock_samples 锁定 RHC，
    // 然后找出存在的样本数量，分配资源并调用带有 lock=true 的 read/take。所有这些都需要修复。
    ddsrt_mutex_unlock(&rhc->lock);
    // 返回计数器 n
    return n;
  }
  // 定义一个静态函数 dds_rhc_read_w_qminv，用于读取数据并根据给定的条件进行筛选
  static int32_t dds_rhc_read_w_qminv(
      struct dds_rhc_default * rhc, bool lock, void **values, dds_sample_info_t *info_seq,
      uint32_t max_samples, uint32_t qminv, dds_instance_handle_t handle, dds_readcond *cond) {
    // 断言：max_samples 不大于 INT32_MAX
    assert(max_samples <= INT32_MAX);
    // 调用 read_w_qminv 函数，并传入相应的参数和回调函数
    return read_w_qminv(rhc, lock, values, info_seq, (int32_t)max_samples, qminv, handle, cond,
                        read_take_to_sample, read_take_to_invsample);
  }

  // 定义一个静态函数 dds_rhc_take_w_qminv，用于获取数据并根据给定的条件进行筛选
  static int32_t dds_rhc_take_w_qminv(
      struct dds_rhc_default * rhc, bool lock, void **values, dds_sample_info_t *info_seq,
      uint32_t max_samples, uint32_t qminv, dds_instance_handle_t handle, dds_readcond *cond) {
    // 断言：max_samples 不大于 INT32_MAX
    assert(max_samples <= INT32_MAX);
    // 调用 take_w_qminv 函数，并传入相应的参数和回调函数
    return take_w_qminv(rhc, lock, values, info_seq, (int32_t)max_samples, qminv, handle, cond,
                        read_take_to_sample, read_take_to_invsample);
  }

  // 定义一个静态函数 dds_rhc_readcdr_w_qminv，用于读取序列化数据并根据给定的条件进行筛选
  static int32_t dds_rhc_readcdr_w_qminv(struct dds_rhc_default * rhc, bool lock,
                                         struct ddsi_serdata **values, dds_sample_info_t *info_seq,
                                         uint32_t max_samples, uint32_t qminv,
                                         dds_instance_handle_t handle, dds_readcond *cond) {
    // 静态断言：void * 类型和 struct ddsi_serdata * 类型的大小相等
    DDSRT_STATIC_ASSERT(sizeof(void *) == sizeof(struct ddsi_serdata *));
    // 断言：max_samples 不大于 INT32_MAX
    assert(max_samples <= INT32_MAX);
    // 调用 read_w_qminv 函数，并传入相应的参数和回调函数
    return read_w_qminv(rhc, lock, (void **)values, info_seq, (int32_t)max_samples, qminv, handle,
                        cond, read_take_to_sample_ref, read_take_to_invsample_ref);
  }

  // 定义一个静态函数 dds_rhc_takecdr_w_qminv，用于获取序列化数据并根据给定的条件进行筛选
  static int32_t dds_rhc_takecdr_w_qminv(struct dds_rhc_default * rhc, bool lock,
                                         struct ddsi_serdata **values, dds_sample_info_t *info_seq,
                                         uint32_t max_samples, uint32_t qminv,
                                         dds_instance_handle_t handle, dds_readcond *cond) {
    // 静态断言：void * 类型和 struct ddsi_serdata * 类型的大小相等
    DDSRT_STATIC_ASSERT(sizeof(void *) == sizeof(struct ddsi_serdata *));
    // 断言：max_samples 不大于 INT32_MAX
    assert(max_samples <= INT32_MAX);
    // 调用 take_w_qminv 函数，并传入相应的参数和回调函数
    return take_w_qminv(rhc, lock, (void **)values, info_seq, (int32_t)max_samples, qminv, handle,
                        cond, read_take_to_sample_ref, read_take_to_invsample_ref);
  }

  /*************************
   ******   WAITSET   ******
   *************************/
  // 定义一个静态函数，获取条件触发器
  static uint32_t rhc_get_cond_trigger(struct rhc_instance *const inst,
                                       const dds_readcond *const c) {
    // 断言实例不为空
    assert(!inst_is_empty(inst));
    // 计算掩码与条件的qminv的按位与结果是否为0
    bool m = ((qmask_of_inst(inst) & c->m_qminv) == 0);
    // 根据条件的样本状态进行判断
    switch (c->m_sample_states) {
      case DDS_SST_READ:
        // 如果样本状态为已读，检查实例是否有已读样本
        m = m && inst_has_read(inst);
        break;
      case DDS_SST_NOT_READ:
        // 如果样本状态为未读，检查实例是否有未读样本
        m = m && inst_has_unread(inst);
        break;
      case DDS_SST_READ | DDS_SST_NOT_READ:
      case 0:
        // 注意：只有在实例非空时才会执行此处，因此这是一个无操作
        m = m && !inst_is_empty(inst);
        break;
      default:
        // 如果样本状态无效，则输出错误信息并终止程序
        DDS_FATAL("update_readconditions: sample_states invalid: %" PRIx32 "\n",
                  c->m_sample_states);
    }
    // 返回条件触发器的值（1或0）
    return m ? 1 : 0;
  }

  // 定义一个静态函数，判断条件是否依赖于样本状态
  static bool cond_is_sample_state_dependent(const struct dds_readcond *cond) {
    // 根据条件的样本状态进行判断
    switch (cond->m_sample_states) {
      case DDS_SST_READ:
      case DDS_SST_NOT_READ:
        // 如果样本状态为已读或未读，则返回true
        return true;
      case DDS_SST_READ | DDS_SST_NOT_READ:
      case 0:
        // 如果样本状态为已读和未读，或为0，则返回false
        return false;
      default:
        // 如果样本状态无效，则输出错误信息并终止程序
        DDS_FATAL("update_readconditions: sample_states invalid: %" PRIx32 "\n",
                  cond->m_sample_states);
        return false;
    }
  }
  // 添加读取条件到默认的dds_rhc结构中
  static bool dds_rhc_default_add_readcondition(struct dds_rhc * rhc_common, dds_readcond * cond) {
    // 假设一个读取条件在其生命周期内几乎都会附加到waitset上，
    // 我们将所有读取条件放在一个集合中，而不区分它们是否附加到waitset上。
    struct dds_rhc_default *const rhc = (struct dds_rhc_default *)rhc_common;
    struct ddsrt_hh_iter it;

    // 检查条件实体类型和过滤器设置是否正确
    assert(
        (dds_entity_kind(&cond->m_entity) == DDS_KIND_COND_READ && cond->m_query.m_filter == 0) ||
        (dds_entity_kind(&cond->m_entity) == DDS_KIND_COND_QUERY && cond->m_query.m_filter != 0));
    // 确保触发状态为0
    assert(ddsrt_atomic_ld32(&cond->m_entity.m_status.m_trigger) == 0);
    // 确保查询条件掩码为0
    assert(cond->m_query.m_qcmask == 0);

    // 根据给定的状态参数计算条件的qminv值
    cond->m_qminv =
        qmask_from_dcpsquery(cond->m_sample_states, cond->m_view_states, cond->m_instance_states);

    // 锁定rhc结构以进行修改
    ddsrt_mutex_lock(&rhc->lock);

    // 如果有过滤器，则在条件位掩码中分配一个插槽；如果没有可用插槽，则返回错误
    if (cond->m_query.m_filter != 0) {
      dds_querycond_mask_t avail_qcmask = ~(dds_querycond_mask_t)0;
      for (dds_readcond *rc = rhc->conds; rc != NULL; rc = rc->m_next) {
        assert((rc->m_query.m_filter == 0 && rc->m_query.m_qcmask == 0) ||
               (rc->m_query.m_filter != 0 && rc->m_query.m_qcmask != 0));
        avail_qcmask &= ~rc->m_query.m_qcmask;
      }
      if (avail_qcmask == 0) {
        // 没有可用的索引
        ddsrt_mutex_unlock(&rhc->lock);
        return false;
      }

      // 使用最低有效位设置
      cond->m_query.m_qcmask = avail_qcmask & (~avail_qcmask + 1);
    }

    // 增加条件计数，将新条件添加到链表中
    rhc->nconds++;
    cond->m_next = rhc->conds;
    rhc->conds = cond;

    uint32_t trigger = 0;
    if (cond->m_query.m_filter == 0) {
      // 读取条件不会缓存在实例和样本内部，因此只需要对非空实例进行评估
      // 如果非空实例列表不为空
      if (!ddsrt_circlist_isempty(&rhc->nonempty_instances)) {
        // 获取最新的非空实例
        struct rhc_instance *inst = latest_nonempty_instance(rhc);
        // 定义一个指向实例结束位置的常量指针
        struct rhc_instance const *const end = inst;
        // 遍历非空实例列表
        do {
          // 计算触发条件并累加到 trigger 变量中
          trigger += rhc_get_cond_trigger(inst, cond);
          // 获取下一个非空实例
          inst = next_nonempty_instance(inst);
        } while (inst != end);  // 当遍历到结束位置时，退出循环
      }
    } else {
      // 如果条件依赖于样本状态
      if (cond_is_sample_state_dependent(cond))
        // 更新 qconds_samplest 变量
        rhc->qconds_samplest |= cond->m_query.m_qcmask;
      // 如果 nqconds 为0
      if (rhc->nqconds++ == 0) {
        // 断言 qcond_eval_samplebuf 为空
        assert(rhc->qcond_eval_samplebuf == NULL);
        // 为 qcond_eval_samplebuf 分配内存
        rhc->qcond_eval_samplebuf = ddsi_sertype_alloc_sample(rhc->type);
      }

      // 附加查询条件意味着在所有实例和样本中清除分配的位，除了匹配谓词的那些。
      // 获取查询条件掩码
      const dds_querycond_mask_t qcmask = cond->m_query.m_qcmask;

      // 遍历rhc实例中的所有实例
      for (struct rhc_instance *inst = ddsrt_hh_iter_first(rhc->instances, &it); inst != NULL;
           inst = ddsrt_hh_iter_next(&it)) {
        // 计算实例是否满足查询条件的反向样本
        const bool instmatch = eval_predicate_invsample(rhc, inst, cond->m_query.m_filter);
        uint32_t matches = 0;

        // 更新实例的条件掩码
        inst->conds = (inst->conds & ~qcmask) | (instmatch ? qcmask : 0);

        // 如果实例有最新的样本
        if (inst->latest) {
          struct rhc_sample *sample = inst->latest->next, *const end = sample;
          do {
            // 计算样本是否满足查询条件
            const bool m = eval_predicate_sample(rhc, sample->sample, cond->m_query.m_filter);

            // 更新样本的条件掩码
            sample->conds = (sample->conds & ~qcmask) | (m ? qcmask : 0);

            // 累加匹配的样本数量
            matches += m;

            // 移动到下一个样本
            sample = sample->next;
          } while (sample != end);  // 当遍历完所有样本时结束循环
        }

        // 如果实例不为空且满足触发条件
        if (!inst_is_empty(inst) && rhc_get_cond_trigger(inst, cond))
          // 更新触发计数
          trigger += (inst->inv_exists ? instmatch : 0) + matches;
      }

      // 如果有触发事件
      if (trigger) {
        // 设置实体状态的触发值
        ddsrt_atomic_st32(&cond->m_entity.m_status.m_trigger, trigger);

        // 发送DDS数据可用状态信号
        dds_entity_status_signal(&cond->m_entity, DDS_DATA_AVAILABLE_STATUS);
      }

      // 记录添加读取条件的操作
      TRACE("add_readcondition(%p, %" PRIx32 ", %" PRIx32 ", %" PRIx32 ") => %p qminv %" PRIx32
            " ; rhc %" PRIu32 " conds\n",
            (void *)rhc, cond->m_sample_states, cond->m_view_states, cond->m_instance_states,
            (void *)cond, cond->m_qminv, rhc->nconds);

      // 解锁rhc结构
      ddsrt_mutex_unlock(&rhc->lock);
      return true;
    }
    // 移除读条件的函数定义
    static void dds_rhc_default_remove_readcondition(struct dds_rhc * rhc_common,
                                                     dds_readcond * cond) {
      // 将通用的rhc结构体转换为默认的rhc结构体
      struct dds_rhc_default *const rhc = (struct dds_rhc_default *)rhc_common;
      dds_readcond **ptr;

      // 对rhc的锁进行加锁操作，确保线程安全
      ddsrt_mutex_lock(&rhc->lock);

      // 查找要移除的读条件在链表中的位置
      ptr = &rhc->conds;
      while (*ptr != cond) ptr = &(*ptr)->m_next;

      // 从链表中移除读条件
      *ptr = (*ptr)->m_next;
      // 更新条件计数器
      rhc->nconds--;

      // 如果是查询条件，则需要更新相关状态和释放资源
      if (cond->m_query.m_filter) {
        rhc->nqconds--;
        rhc->qconds_samplest &= ~cond->m_query.m_qcmask;
        cond->m_query.m_qcmask = 0;
        if (rhc->nqconds == 0) {
          assert(rhc->qcond_eval_samplebuf != NULL);
          ddsi_sertype_free_sample(rhc->type, rhc->qcond_eval_samplebuf, DDS_FREE_ALL);
          rhc->qcond_eval_samplebuf = NULL;
        }
      }

      // 解锁rhc的锁
      ddsrt_mutex_unlock(&rhc->lock);
    }
    // 定义一个静态函数 update_conditions_locked，用于更新条件
    static bool update_conditions_locked(
        struct dds_rhc_default * rhc,          // 参数1：指向dds_rhc_default结构体的指针
        bool called_from_insert,               // 参数2：表示是否从insert函数调用
        const struct trigger_info_pre *pre,    // 参数3：指向trigger_info_pre结构体的指针
        const struct trigger_info_post *post,  // 参数4：指向trigger_info_post结构体的指针
        const struct trigger_info_qcond *trig_qc,  // 参数5：指向trigger_info_qcond结构体的指针
        const struct rhc_instance *inst)           // 参数6：指向rhc_instance结构体的指针
    {
      // 前置条件：rhc->lock已持有；返回值为1表示需要触发，否则为0
      bool trigger = false;  // 初始化触发标志为false
      dds_readcond *iter;    // 定义一个dds_readcond类型的指针iter
      bool m_pre, m_post;    // 定义两个布尔变量m_pre和m_post

      // 输出跟踪信息
      TRACE("update_conditions_locked(%p %p) - inst %" PRIu32 " nonempty %" PRIu32 " disp %" PRIu32
            " nowr %" PRIu32 " new %" PRIu32 " samples %" PRIu32 " read %" PRIu32 "\n",
            (void *)rhc, (void *)inst, rhc->n_instances, rhc->n_nonempty_instances,
            rhc->n_not_alive_disposed, rhc->n_not_alive_no_writers, rhc->n_new, rhc->n_vsamples,
            rhc->n_vread);
      TRACE("  pre (%" PRIx32 ",%d,%d) post (%" PRIx32
            ",%d,%d) read -[%d,%d]+[%d,%d] qcmask -[%" PRIx32 ",%" PRIx32 "]+[%" PRIx32 ",%" PRIx32
            "]\n",
            pre->c.qminst, pre->c.has_read, pre->c.has_not_read, post->c.qminst, post->c.has_read,
            post->c.has_not_read, trig_qc->dec_invsample_read, trig_qc->dec_sample_read,
            trig_qc->inc_invsample_read, trig_qc->inc_sample_read, trig_qc->dec_conds_invsample,
            trig_qc->dec_conds_sample, trig_qc->inc_conds_invsample, trig_qc->inc_conds_sample);

      // 断言：rhc->n_nonempty_instances >= rhc->n_not_alive_disposed + rhc->n_not_alive_no_writers
      assert(rhc->n_nonempty_instances >= rhc->n_not_alive_disposed + rhc->n_not_alive_no_writers);
#ifndef DDS_HAS_LIFESPAN
      // 如果禁用了生命周期，样本不能过期，因此空实例不能处于“新”状态
      assert(rhc->n_nonempty_instances >= rhc->n_new);
#endif
      // 断言：rhc->n_vsamples >= rhc->n_vread
      assert(rhc->n_vsamples >= rhc->n_vread);

      iter = rhc->conds;
      while (iter) {
        // 根据 pre 和 iter 的 qminst 值计算 m_pre，如果它们的按位与结果为 0，则 m_pre 为 true
        m_pre = ((pre->c.qminst & iter->m_qminv) == 0);
        // 根据 post 和 iter 的 qminst 值计算 m_post，如果它们的按位与结果为 0，则 m_post 为 true
        m_post = ((post->c.qminst & iter->m_qminv) == 0);

        // 快速路径出口：基于实例和视图状态，实例在过去和将来都不匹配，因此无需评估其他内容
        if (!m_pre && !m_post) {
          // 将迭代器指向下一个元素
          iter = iter->m_next;
          // 跳过当前循环，继续执行下一次循环
          continue;
        }

        // FIXME: 使用位掩码？
        switch (iter->m_sample_states) {
          case DDS_SST_READ:
            // 如果样本状态为 READ，则根据 pre 和 post 的 has_read 值更新 m_pre 和 m_post
            m_pre = m_pre && pre->c.has_read;
            m_post = m_post && post->c.has_read;
            break;
          case DDS_SST_NOT_READ:
            // 如果样本状态为 NOT_READ，则根据 pre 和 post 的 has_not_read 值更新 m_pre 和 m_post
            m_pre = m_pre && pre->c.has_not_read;
            m_post = m_post && post->c.has_not_read;
            break;
          case DDS_SST_READ | DDS_SST_NOT_READ:
          case 0:
            // 如果样本状态为 READ 或 NOT_READ，则根据 pre 和 post 的 has_read 和 has_not_read
            // 值更新 m_pre 和 m_post
            m_pre = m_pre && (pre->c.has_read + pre->c.has_not_read);
            m_post = m_post && (post->c.has_read + post->c.has_not_read);
            break;
          default:
            // 如果样本状态无效，输出错误信息并终止程序
            DDS_FATAL("update_readconditions: sample_states invalid: %" PRIx32 "\n",
                      iter->m_sample_states);
        }
        // 初始化 m_pre 和 m_post 变量，用于判断触发条件是否满足
        m_pre = ((pre->c.qminst & iter->m_qminv) == 0);
        m_post = ((post->c.qminst & iter->m_qminv) == 0);

        // 快速路径出口：基于实例和视图状态，实例不匹配且将来也不会匹配，因此无需评估其他内容
        if (!m_pre && !m_post) {
          iter = iter->m_next;
          continue;
        }

        // FIXME: 使用位掩码？
        switch (iter->m_sample_states) {
          case DDS_SST_READ:
            // 根据读取状态更新 m_pre 和 m_post
            m_pre = m_pre && pre->c.has_read;
            m_post = m_post && post->c.has_read;
            break;
          case DDS_SST_NOT_READ:
            // 根据未读取状态更新 m_pre 和 m_post
            m_pre = m_pre && pre->c.has_not_read;
            m_post = m_post && post->c.has_not_read;
            break;
          case DDS_SST_READ | DDS_SST_NOT_READ:
          case 0:
            // 根据读取和未读取状态更新 m_pre 和 m_post
            m_pre = m_pre && (pre->c.has_read + pre->c.has_not_read);
            m_post = m_post && (post->c.has_read + post->c.has_not_read);
            break;
          default:
            // 如果 sample_states 无效，则输出错误信息并终止程序
            DDS_FATAL("update_readconditions: sample_states invalid: %" PRIx32 "\n",
                      iter->m_sample_states);
        }

        // 输出调试信息
        TRACE("  cond %p %08" PRIx32 ": ", (void *)iter, iter->m_query.m_qcmask);
        if (iter->m_query.m_filter == 0) {
          // 确保实体类型为 DDS_KIND_COND_READ
          assert(dds_entity_kind(&iter->m_entity) == DDS_KIND_COND_READ);
          if (m_pre == m_post)
            TRACE("no change");                          // 没有变化
          else if (m_pre < m_post) {
            TRACE("now matches");                        // 现在匹配
            trigger = (ddsrt_atomic_inc32_ov(&iter->m_entity.m_status.m_trigger) == 0);
            if (trigger) TRACE(" (cond now triggers)");  // 条件现在触发
          } else {
            TRACE("no longer matches");                  // 不再匹配
            if (ddsrt_atomic_dec32_nv(&iter->m_entity.m_status.m_trigger) == 0)
              TRACE(" (cond no longer triggers)");       // 条件不再触发
          }
        } else if (m_pre || m_post) /* no need to look any further if both are false */
        {
          // 确保实体类型为 DDS_KIND_COND_QUERY
          assert(dds_entity_kind(&iter->m_entity) == DDS_KIND_COND_QUERY);
          // 确保查询条件掩码不为0
          assert(iter->m_query.m_qcmask != 0);
          // 定义查询条件掩码变量 qcmask
          const dds_querycond_mask_t qcmask = iter->m_query.m_qcmask;
          // 初始化 mdelta 变量，用于计算触发条件的变化
          int32_t mdelta = 0;

          // 根据样本状态进行处理
          switch (iter->m_sample_states) {
            case DDS_SST_READ:
              // 如果需要减少无效样本的读取次数
              if (trig_qc->dec_invsample_read)
                mdelta -= (trig_qc->dec_conds_invsample & qcmask) != 0;
              // 如果需要减少有效样本的读取次数
              if (trig_qc->dec_sample_read) mdelta -= (trig_qc->dec_conds_sample & qcmask) != 0;
              // 如果需要增加无效样本的读取次数
              if (trig_qc->inc_invsample_read)
                mdelta += (trig_qc->inc_conds_invsample & qcmask) != 0;
              // 如果需要增加有效样本的读取次数
              if (trig_qc->inc_sample_read) mdelta += (trig_qc->inc_conds_sample & qcmask) != 0;
              break;
            case DDS_SST_NOT_READ:
              // 如果不需要减少无效样本的读取次数
              if (!trig_qc->dec_invsample_read)
                mdelta -= (trig_qc->dec_conds_invsample & qcmask) != 0;
              // 如果不需要减少有效样本的读取次数
              if (!trig_qc->dec_sample_read) mdelta -= (trig_qc->dec_conds_sample & qcmask) != 0;
              // 如果不需要增加无效样本的读取次数
              if (!trig_qc->inc_invsample_read)
                mdelta += (trig_qc->inc_conds_invsample & qcmask) != 0;
              // 如果不需要增加有效样本的读取次数
              if (!trig_qc->inc_sample_read) mdelta += (trig_qc->inc_conds_sample & qcmask) != 0;
              break;
            case DDS_SST_READ | DDS_SST_NOT_READ:
            case 0:
              // 减少无效样本和有效样本的触发条件计数
              mdelta -= (trig_qc->dec_conds_invsample & qcmask) != 0;
              mdelta -= (trig_qc->dec_conds_sample & qcmask) != 0;
              // 增加无效样本和有效样本的触发条件计数
              mdelta += (trig_qc->inc_conds_invsample & qcmask) != 0;
              mdelta += (trig_qc->inc_conds_sample & qcmask) != 0;
              break;
            default:
              // 如果样本状态无效，输出错误信息并终止程序
              DDS_FATAL("update_readconditions: sample_states invalid: %" PRIx32 "\n",
                        iter->m_sample_states);
          }
          // 判断 m_pre 和 m_post 是否相等
          if (m_pre == m_post) {
            // 断言 m_pre 为真
            assert(m_pre);
            /* 在 read-condition 级别存在匹配
               - 因此实例中的匹配样本已计入触发计数
               - 因此需要进行增量更新
               总是有空间放置有效和无效的样本，都可以添加和删除
               插入更新时总是添加未读数据，但读取时假装它是要删除的内容并插入已读数据 */
            assert(mdelta >= 0 ||
                   ddsrt_atomic_ld32(&iter->m_entity.m_status.m_trigger) >= (uint32_t)-mdelta);
            if (mdelta == 0)
              TRACE("no change @ %" PRIu32 " (0)",
                    ddsrt_atomic_ld32(&iter->m_entity.m_status.m_trigger));
            else
              TRACE("m=%" PRId32 " @ %" PRIu32 " (0)", mdelta,
                    ddsrt_atomic_ld32(&iter->m_entity.m_status.m_trigger) + (uint32_t)mdelta);
            /* 即使现在匹配且之前匹配过，也不能确定之前的任何样本是否匹配过，因此 m_trigger
             * 可能仍然为 0 */
            const uint32_t ov =
                ddsrt_atomic_add32_ov(&iter->m_entity.m_status.m_trigger, (uint32_t)mdelta);
            if (mdelta > 0 && ov == 0) trigger = true;
            if (trigger)
              TRACE(" (cond now triggers)");
            else if (mdelta < 0 && ov == (uint32_t)-mdelta)
              TRACE(" (cond no longer triggers)");
          } else {
            // 如果之前没有匹配的读条件级别，现在有：扫描所有样本以查找匹配项；
            // 或者之前有匹配，现在没有：因此也扫描所有样本以查找匹配项。唯一的区别是匹配数量应该是增加还是减少。
            int32_t mcurrent = 0;
            if (inst) {
              // 如果存在无效实例，则检查该实例是否与查询条件匹配
              if (inst->inv_exists)
                mcurrent +=
                    (qmask_of_invsample(inst) & iter->m_qminv) == 0 && (inst->conds & qcmask) != 0;
              // 如果存在最新的实例
              if (inst->latest) {
                struct rhc_sample *sample = inst->latest->next, *const end = sample;
                // 遍历所有样本，检查它们是否与查询条件匹配
                do {
                  mcurrent += (qmask_of_sample(sample) & iter->m_qminv) == 0 &&
                              (sample->conds & qcmask) != 0;
                  sample = sample->next;
                } while (sample != end);
              }
            }
            // 如果之前没有匹配，现在有匹配
            if (mdelta == 0 && mcurrent == 0)
              // 没有变化，触发器保持不变
              TRACE("no change @ %" PRIu32 " (2)",
                    ddsrt_atomic_ld32(&iter->m_entity.m_status.m_trigger));
            else if (m_pre < m_post) {
              // 之前没有匹配，所以实例在触发值中根本没有计算。
              // 因此，在插入数据时，只需关心当前有多少匹配。

              // 当读取或获取时，在更改样本状态之前，会增量地进行评估，
              // 所以 mrem 反映了更改之前的状态，需要考虑增量变化。
              const int32_t m = called_from_insert ? mcurrent : mcurrent + mdelta;
              TRACE("mdelta=%" PRId32 " mcurrent=%" PRId32 " => %" PRId32 " => %" PRIu32 " (2a)",
                    mdelta, mcurrent, m,
                    ddsrt_atomic_ld32(&iter->m_entity.m_status.m_trigger) + (uint32_t)m);
              assert(m >= 0 ||
                     ddsrt_atomic_ld32(&iter->m_entity.m_status.m_trigger) >= (uint32_t)-m);
              trigger =
                  (ddsrt_atomic_add32_ov(&iter->m_entity.m_status.m_trigger, (uint32_t)m) == 0 &&
                   m > 0);
              if (trigger)
                // 条件现在触发
                TRACE(" (cond now triggers)");
            } else {
              // 之前匹配，但现在不再匹配，这意味着我们需要减去当前的匹配数量
              // 以及之前刚刚删除的那些，因此需要增量变化
              const int32_t m = mcurrent - mdelta;
              TRACE("mdelta=%" PRId32 " mcurrent=%" PRId32 " => %" PRId32 " => %" PRIu32 " (2b)",
                    mdelta, mcurrent, m,
                    ddsrt_atomic_ld32(&iter->m_entity.m_status.m_trigger) - (uint32_t)m);
              assert(m < 0 || ddsrt_atomic_ld32(&iter->m_entity.m_status.m_trigger) >= (uint32_t)m);
              if (ddsrt_atomic_sub32_nv(&iter->m_entity.m_status.m_trigger, (uint32_t)m) == 0)
                // 条件不再触发
                TRACE(" (cond no longer triggers)");
            }
          }
        }

        // 如果触发条件为真
        if (trigger) {
          // 向实体发送数据可用状态信号
          dds_entity_status_signal(&iter->m_entity, DDS_DATA_AVAILABLE_STATUS);
        }
        // 输出跟踪信息并换行
        TRACE("\n");
        // 将迭代器指向下一个元素
        iter = iter->m_next;
      }
      return trigger;
    }

    /*************************
     ******  READ/TAKE  ******
     *************************/

    // dds_rhc_default_read 函数定义
    static int32_t dds_rhc_default_read(  //
        struct dds_rhc * rhc_common,      // 通用的可靠历史缓存指针
        bool lock,                        // 是否需要锁定
        void **values,                    // 存储读取到的数据的数组
        dds_sample_info_t *info_seq,      // 存储读取到的样本信息的数组
        uint32_t max_samples,             // 最大读取样本数
        uint32_t mask,                    // 状态掩码
        dds_instance_handle_t handle,     // 实例句柄
        dds_readcond *cond)               // 读条件指针
    {
      struct dds_rhc_default *const rhc = (struct dds_rhc_default *)rhc_common;
      uint32_t qminv = qmask_from_mask_n_cond(  //
          mask,                                 //
          cond);
      return dds_rhc_read_w_qminv(              //
          rhc,                                  //
          lock,                                 //
          values,                               //
          info_seq,                             //
          max_samples,                          //
          qminv,                                //
          handle,                               //
          cond);
    }

    // dds_rhc_default_take 函数定义
    static int32_t dds_rhc_default_take(  //
        struct dds_rhc * rhc_common,      // 通用的可靠历史缓存指针
        bool lock,                        // 是否需要锁定
        void **values,                    // 存储读取到的数据的数组
        dds_sample_info_t *info_seq,      // 存储读取到的样本信息的数组
        uint32_t max_samples,             // 最大读取样本数
        uint32_t mask,                    // 状态掩码
        dds_instance_handle_t handle,     // 实例句柄
        dds_readcond *cond)               // 读条件指针
    {
      struct dds_rhc_default *const rhc = (struct dds_rhc_default *)rhc_common;
      uint32_t qminv = qmask_from_mask_n_cond(  //
          mask,                                 //
          cond);
      return dds_rhc_take_w_qminv(              //
          rhc,                                  //
          lock,                                 //
          values,                               //
          info_seq,                             //
          max_samples,                          //
          qminv,                                //
          handle,                               //
          cond);
    }

    // dds_rhc_default_readcdr 函数定义
    static int32_t dds_rhc_default_readcdr(  //
        struct dds_rhc * rhc_common,         // 通用的可靠历史缓存指针
        bool lock,                           // 是否需要锁定
        struct ddsi_serdata **values,        // 存储读取到的序列化数据的数组
        dds_sample_info_t *info_seq,         // 存储读取到的样本信息的数组
        uint32_t max_samples,                // 最大读取样本数
        uint32_t sample_states,              // 样本状态
        uint32_t view_states,                // 视图状态
        uint32_t instance_states,            // 实例状态
        dds_instance_handle_t handle)        // 实例句柄
    {
      struct dds_rhc_default *const rhc = (struct dds_rhc_default *)rhc_common;
      uint32_t qminv = qmask_from_dcpsquery(  //
          sample_states,                      //
          view_states,                        //
          instance_states);
      return dds_rhc_readcdr_w_qminv(         //
          rhc,                                //
          lock,                               //
          values,                             //
          info_seq,                           //
          max_samples,                        //
          qminv,                              //
          handle,                             //
          NULL);
    }

    // dds_rhc_default_takecdr 函数定义
    static int32_t dds_rhc_default_takecdr(  //
        struct dds_rhc * rhc_common,         // 通用的可靠历史缓存指针
        bool lock,                           // 是否需要锁定
        struct ddsi_serdata **values,        // 存储读取到的序列化数据的数组
        dds_sample_info_t *info_seq,         // 存储读取到的样本信息的数组
        uint32_t max_samples,                // 最大读取样本数
        uint32_t sample_states,              // 样本状态
        uint32_t view_states,                // 视图状态
        uint32_t instance_states,            // 实例状态
        dds_instance_handle_t handle)        // 实例句柄
    {
      struct dds_rhc_default *const rhc = (struct dds_rhc_default *)rhc_common;
      uint32_t qminv = qmask_from_dcpsquery(  //
          sample_states,                      //
          view_states,                        //
          instance_states);
      return dds_rhc_takecdr_w_qminv(         //
          rhc,                                //
          lock,                               //
          values,                             //
          info_seq,                           //
          max_samples,                        //
          qminv,                              //
          handle,                             //
          NULL);
    }

    /*************************
     ******    CHECK    ******
     *************************/

#ifndef NDEBUG
#define CHECK_MAX_CONDS 64
    static int rhc_check_counts_locked(struct dds_rhc_default * rhc, bool check_conds,
                                       bool check_qcmask) {
      // 如果rhc->xchecks为0，则直接返回1
      if (!rhc->xchecks) return 1;

      // 计算需要检查的条件数，取rhc->nconds和CHECK_MAX_CONDS中的较小值
      const uint32_t ncheck = rhc->nconds < CHECK_MAX_CONDS ? rhc->nconds : CHECK_MAX_CONDS;
      // 定义并初始化各种计数器变量
      uint32_t n_instances = 0, n_nonempty_instances = 0;
      uint32_t n_not_alive_disposed = 0, n_not_alive_no_writers = 0, n_new = 0;
      uint32_t n_vsamples = 0, n_vread = 0;
      uint32_t n_invsamples = 0, n_invread = 0;
      uint32_t cond_match_count[CHECK_MAX_CONDS];
      dds_querycond_mask_t enabled_qcmask = 0;
      struct rhc_instance *inst;
      struct ddsrt_hh_iter iter;
      dds_readcond *rciter;
      uint32_t i;

      // 初始化cond_match_count数组
      for (i = 0; i < CHECK_MAX_CONDS; i++) cond_match_count[i] = 0;

      // 遍历rhc->conds链表，更新enabled_qcmask
      for (rciter = rhc->conds; rciter; rciter = rciter->m_next) {
        assert((dds_entity_kind(&rciter->m_entity) == DDS_KIND_COND_READ &&
                rciter->m_query.m_filter == 0) ||
               (dds_entity_kind(&rciter->m_entity) == DDS_KIND_COND_QUERY &&
                rciter->m_query.m_filter != 0));
        assert((rciter->m_query.m_filter != 0) == (rciter->m_query.m_qcmask != 0));
        assert(!(enabled_qcmask & rciter->m_query.m_qcmask));
        enabled_qcmask |= rciter->m_query.m_qcmask;
      }

      // 遍历rhc->instances哈希表
      for (inst = ddsrt_hh_iter_first(rhc->instances, &iter); inst;
           inst = ddsrt_hh_iter_next(&iter)) {
        uint32_t n_vsamples_in_instance = 0, n_read_vsamples_in_instance = 0;
        bool a_sample_free = true;

        // 更新实例计数器
        n_instances++;
        if (inst->isnew) n_new++;
        if (inst_is_empty(inst)) continue;

        // 更新非空实例计数器
        n_nonempty_instances++;
        if (inst->isdisposed)
          n_not_alive_disposed++;
        else if (inst->wrcount == 0)
          n_not_alive_no_writers++;

        // 如果存在最新的样本，遍历该实例的样本链表
        if (inst->latest) {
          struct rhc_sample *sample = inst->latest->next, *const end = sample;
          do {
            if (sample == &inst->a_sample) {
              assert(a_sample_free);
              a_sample_free = false;
            }
            // 更新样本计数器
            n_vsamples++;
            n_vsamples_in_instance++;
            if (sample->isread) {
              n_vread++;
              n_read_vsamples_in_instance++;
            }
            sample = sample->next;
          } while (sample != end);
        }

        // 更新无效样本计数器
        if (inst->inv_exists) {
          n_invsamples++;
          n_invread += inst->inv_isread;
        }

        // 检查断言
        assert(n_read_vsamples_in_instance == inst->nvread);
        assert(n_vsamples_in_instance == inst->nvsamples);
        assert(a_sample_free == inst->a_sample_free);

        // 如果需要检查条件
        if (check_conds) {
          // 如果需要检查查询条件掩码且查询条件数量大于0
          if (check_qcmask && rhc->nqconds > 0) {
            // 定义查询条件掩码变量
            dds_querycond_mask_t qcmask;
            // 将无类型的样本转换为干净的无效样本
            untyped_to_clean_invsample(rhc->type, inst->tk->m_sample, rhc->qcond_eval_samplebuf, 0,
                                       0);
            // 初始化查询条件掩码为0
            qcmask = 0;
            // 遍历所有的读取条件
            for (rciter = rhc->conds; rciter; rciter = rciter->m_next)
              // 如果当前读取条件有过滤器并且过滤器对评估样本缓冲区有效，则将当前查询条件掩码与读取条件的查询条件掩码进行或操作
              if (rciter->m_query.m_filter != 0 &&
                  rciter->m_query.m_filter(rhc->qcond_eval_samplebuf))
                qcmask |= rciter->m_query.m_qcmask;
            // 断言实例的条件与启用的查询条件掩码相等
            assert((inst->conds & enabled_qcmask) == qcmask);
            // 如果实例有最新的样本
            if (inst->latest) {
              // 获取实例中最新样本的下一个样本，并定义结束位置
              struct rhc_sample *sample = inst->latest->next, *const end = sample;
              // 遍历实例中的所有样本
              do {
                // 将序列化数据转换为样本
                ddsi_serdata_to_sample(sample->sample, rhc->qcond_eval_samplebuf, NULL, NULL);
                // 重置查询条件掩码为0
                qcmask = 0;
                // 再次遍历所有的读取条件
                for (rciter = rhc->conds; rciter; rciter = rciter->m_next)
                  // 如果当前读取条件有过滤器并且过滤器对评估样本缓冲区有效，则将当前查询条件掩码与读取条件的查询条件掩码进行或操作
                  if (rciter->m_query.m_filter != 0 &&
                      rciter->m_query.m_filter(rhc->qcond_eval_samplebuf))
                    qcmask |= rciter->m_query.m_qcmask;
                // 断言样本的条件与启用的查询条件掩码相等
                assert((sample->conds & enabled_qcmask) == qcmask);
                // 移动到下一个样本
                sample = sample->next;
              } while (sample != end);  // 当遍历到结束位置时，退出循环
            }
          }

          // 遍历条件链表，更新cond_match_count数组
          // 遍历rhc->conds中的条件，直到遍历完所有条件或达到ncheck限制
          for (i = 0, rciter = rhc->conds; rciter && i < ncheck; i++, rciter = rciter->m_next) {
            // 如果当前实例inst不满足rciter条件，则跳过此次循环
            if (!rhc_get_cond_trigger(inst, rciter))
              ;
            // 如果rciter的查询过滤器为空，则将cond_match_count[i]加1
            else if (rciter->m_query.m_filter == 0)
              cond_match_count[i]++;
            // 如果rciter的查询过滤器不为空
            else {
              // 如果实例inst存在无效样本
              if (inst->inv_exists)
                // 检查qmask_of_invsample和rciter->m_qminv是否匹配，以及inst->conds和rciter->m_query.m_qcmask是否匹配
                cond_match_count[i] += (qmask_of_invsample(inst) & rciter->m_qminv) == 0 &&
                                       (inst->conds & rciter->m_query.m_qcmask) != 0;
              // 如果实例inst有最新的样本
              if (inst->latest) {
                // 遍历实例inst的所有样本
                struct rhc_sample *sample = inst->latest->next, *const end = sample;
                do {
                  // 检查qmask_of_sample和rciter->m_qminv是否匹配，以及sample->conds和rciter->m_query.m_qcmask是否匹配
                  cond_match_count[i] += ((qmask_of_sample(sample) & rciter->m_qminv) == 0 &&
                                          (sample->conds & rciter->m_query.m_qcmask) != 0);
                  // 移动到下一个样本
                  sample = sample->next;
                } while (sample != end);  // 当遍历完所有样本时，结束循环
              }
            }
          }
        }
      }
      // 断言rhc的n_instances与计算得到的n_instances相等
      assert(rhc->n_instances == n_instances);
      // 断言rhc的n_nonempty_instances与计算得到的n_nonempty_instances相等
      assert(rhc->n_nonempty_instances == n_nonempty_instances);
      // 断言rhc的n_not_alive_disposed与计算得到的n_not_alive_disposed相等
      assert(rhc->n_not_alive_disposed == n_not_alive_disposed);
      // 断言rhc的n_not_alive_no_writers与计算得到的n_not_alive_no_writers相等
      assert(rhc->n_not_alive_no_writers == n_not_alive_no_writers);
      // 断言rhc的n_new与计算得到的n_new相等
      assert(rhc->n_new == n_new);
      // 断言rhc的n_vsamples与计算得到的n_vsamples相等
      assert(rhc->n_vsamples == n_vsamples);
      // 断言rhc的n_vread与计算得到的n_vread相等
      assert(rhc->n_vread == n_vread);
      // 断言rhc的n_invsamples与计算得到的n_invsamples相等
      assert(rhc->n_invsamples == n_invsamples);
      // 断言rhc的n_invread与计算得到的n_invread相等
      assert(rhc->n_invread == n_invread);

      // 如果需要检查条件
      if (check_conds) {
        // 遍历rhc的conds，确保cond_match_count[i]与rciter的m_entity.m_status.m_trigger相等
        for (i = 0, rciter = rhc->conds; rciter && i < ncheck; i++, rciter = rciter->m_next)
          assert(cond_match_count[i] == ddsrt_atomic_ld32(&rciter->m_entity.m_status.m_trigger));
      }

      // 如果rhc的n_nonempty_instances为0
      if (rhc->n_nonempty_instances == 0) {
        // 断言rhc的nonempty_instances列表为空
        assert(ddsrt_circlist_isempty(&rhc->nonempty_instances));
      } else {
        // 断言rhc的nonempty_instances列表不为空
        assert(!ddsrt_circlist_isempty(&rhc->nonempty_instances));
        // 获取rhc中最新的非空实例
        struct ddsrt_circlist_elem const *prev = rhc->nonempty_instances.latest->prev;
        inst = latest_nonempty_instance(rhc);
        struct rhc_instance const *const end = inst;
        n_nonempty_instances = 0;
        do {
          // 断言当前实例不为空
          assert(!inst_is_empty(inst));
          // 断言前一个元素的next指针指向当前实例的nonempty_list
          assert(prev->next == &inst->nonempty_list);
          // 断言当前实例的nonempty_list的prev指针指向前一个元素
          assert(inst->nonempty_list.prev == prev);
          // 更新prev和inst为下一个非空实例
          prev = &inst->nonempty_list;
          inst = next_nonempty_instance(inst);
          // 增加非空实例计数
          n_nonempty_instances++;
        } while (inst != end);
        // 断言rhc的n_nonempty_instances与计算得到的n_nonempty_instances相等
        assert(rhc->n_nonempty_instances == n_nonempty_instances);
      }
      return 1;
    }
#undef CHECK_MAX_CONDS
#endif

    // 定义一个名为dds_rhc_default_ops的结构体常量，该结构体包含了一系列函数指针
    static const struct dds_rhc_ops dds_rhc_default_ops = {
        // 初始化rhc_ops子结构体，设置各个函数指针
        .rhc_ops =
            {
                .store = dds_rhc_default_store,                  // 存储数据的函数
                .unregister_wr = dds_rhc_default_unregister_wr,  // 注销写入者的函数
                .relinquish_ownership = dds_rhc_default_relinquish_ownership,  // 放弃所有权的函数
                .set_qos = dds_rhc_default_set_qos,                            // 设置QoS的函数
                .free = dds_rhc_default_free  // 释放资源的函数
            },
        .read = dds_rhc_default_read,         // 读取数据的函数
        .take = dds_rhc_default_take,         // 获取并移除数据的函数
        .readcdr = dds_rhc_default_readcdr,   // 以CDR格式读取数据的函数
        .takecdr = dds_rhc_default_takecdr,   // 以CDR格式获取并移除数据的函数
        .add_readcondition = dds_rhc_default_add_readcondition,  // 添加读取条件的函数
        .remove_readcondition = dds_rhc_default_remove_readcondition,  // 移除读取条件的函数
        .lock_samples = dds_rhc_default_lock_samples,                  // 锁定样本的函数
        .associate = dds_rhc_default_associate                         // 关联实体的函数
    };
