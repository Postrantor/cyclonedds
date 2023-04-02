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

#include "dds/ddsi/ddsi_domaingv.h"
#include "dds/ddsi/ddsi_entity.h"
#include "dds/ddsi/ddsi_entity_index.h"
#include "dds/ddsi/ddsi_proxy_participant.h"
#include "dds/ddsi/ddsi_serdata.h"
#include "dds/ddsi/ddsi_tkmap.h"
#include "dds/ddsi/ddsi_unused.h"
#include "dds/ddsrt/heap.h"
#include "dds__builtin.h"
#include "dds__serdata_builtintopic.h"
#include "dds__whc_builtintopic.h"
// 定义 bwhc 结构体
struct bwhc {
  struct ddsi_whc common;                                  // 通用的 ddsi_whc 结构体
  enum ddsi_sertype_builtintopic_entity_kind entity_kind;  // 实体类型
  const struct ddsi_entity_index *entidx;                  // 实体索引指针
};

// 定义 bwhc_iter_state 枚举
enum bwhc_iter_state {
  BIS_INIT_LOCAL,  // 初始化本地状态
  BIS_LOCAL,       // 本地状态
  BIS_INIT_PROXY,  // 初始化代理状态
  BIS_PROXY        // 代理状态
};

// 定义 bwhc_iter 结构体
struct bwhc_iter {
  struct ddsi_whc_sample_iter_base c;          // 基础迭代器
  enum bwhc_iter_state st;                     // 迭代器状态
  bool have_sample;                            // 是否有样本
  struct ddsi_entity_enum it;                  // 实体枚举
#ifdef DDS_HAS_TOPIC_DISCOVERY
  struct ddsi_proxy_participant *cur_proxypp;  // 当前代理参与者
  ddsi_entityid_t proxytp_eid;                 // 代理主题实体 ID
#endif
};

/* 检查我们定义的 whc_sample_iter 是否适合调用者分配的类型 */
DDSRT_STATIC_ASSERT(sizeof(struct bwhc_iter) <= sizeof(struct ddsi_whc_sample_iter));

/**
 * @brief 释放给定的 ddsi_whc 结构体。
 *
 * @param whc_generic 要释放的 ddsi_whc 结构体。
 */
static void bwhc_free(struct ddsi_whc *whc_generic) { ddsrt_free(whc_generic); }

/**
 * @brief 初始化给定的 ddsi_whc 结构体的样本迭代器。
 *
 * @param whc_generic 要初始化样本迭代器的 ddsi_whc 结构体。
 * @param opaque_it 样本迭代器指针。
 */
static void bwhc_sample_iter_init(const struct ddsi_whc *whc_generic,
                                  struct ddsi_whc_sample_iter *opaque_it) {
  struct bwhc_iter *it = (struct bwhc_iter *)opaque_it;
  it->c.whc = (struct ddsi_whc *)whc_generic;
  it->st = BIS_INIT_LOCAL;
  it->have_sample = false;
}

/**
 * @brief 检查给定的实体是否可见。
 *
 * @param e 要检查的实体。
 * @return 如果实体可见，则返回 true，否则返回 false。
 */
static bool is_visible(const struct ddsi_entity_common *e) {
  const ddsi_vendorid_t vendorid = ddsi_get_entity_vendorid(e);
  return ddsi_builtintopic_is_visible(e->gv->builtin_topic_interface, &e->guid, vendorid);
}

/**
 * @brief 从代理主题迭代器中借用下一个代理主题样本
 *
 * @param[in] it 指向 bwhc_iter 结构体的指针，用于存储当前迭代状态
 * @param[out] sample 指向 ddsi_whc_borrowed_sample 结构体的指针，用于存储借用的样本数据
 * @return 如果找到下一个代理主题样本，则返回 true，否则返回 false
 */
static bool bwhc_sample_iter_borrow_next_proxy_topic(struct bwhc_iter *const it,
                                                     struct ddsi_whc_borrowed_sample *sample) {
#ifdef DDS_HAS_TOPIC_DISCOVERY
  struct ddsi_proxy_topic *proxytp = NULL;

  // 如果不是第一个 proxypp：获取锁并从此 proxypp 获取下一个主题
  if (it->cur_proxypp != NULL) {
    ddsrt_mutex_lock(&it->cur_proxypp->e.lock);
    do {
      proxytp = ddsrt_avl_lookup_succ(&ddsi_proxypp_proxytp_treedef, &it->cur_proxypp->topics,
                                      &it->proxytp_eid);
      if (proxytp != NULL) it->proxytp_eid = proxytp->entityid;
    } while (proxytp != NULL && proxytp->deleted);
  }
  while (proxytp == NULL) {
    // 当前 proxypp 没有下一个可用主题：如果不是第一个 proxypp，则返回锁
    if (it->cur_proxypp != NULL) ddsrt_mutex_unlock(&it->cur_proxypp->e.lock);

    // 枚举下一个 proxypp（如果有）并获取锁
    if ((it->cur_proxypp = (struct ddsi_proxy_participant *)ddsi_entidx_enum_next(&it->it)) == NULL)
      return false;
    ddsrt_mutex_lock(&it->cur_proxypp->e.lock);

    // 获取此 proxypp 的第一个（未删除）主题
    ddsi_entityid_t eid = {.u = 0};
    proxytp = ddsrt_avl_lookup_succ(&ddsi_proxypp_proxytp_treedef, &it->cur_proxypp->topics, &eid);
    while (proxytp != NULL && proxytp->deleted)
      proxytp = ddsrt_avl_lookup_succ(&ddsi_proxypp_proxytp_treedef, &it->cur_proxypp->topics,
                                      &proxytp->entityid);
    if (proxytp != NULL) it->proxytp_eid = proxytp->entityid;
  }
  // 找到下一个主题，制作样本并释放 proxypp 锁
  sample->serdata = dds__builtin_make_sample_proxy_topic(proxytp, proxytp->tupdate, true);
  it->have_sample = true;
  ddsrt_mutex_unlock(&it->cur_proxypp->e.lock);
#else
  (void)it;
  (void)sample;
#endif
  return true;
}
/**
 * @brief 初始化代理主题迭代器
 *
 * @param[in,out] it 指向 bwhc_iter 结构体的指针，用于存储当前迭代状态
 */
static void init_proxy_topic_iteration(struct bwhc_iter *const it) {
#ifdef DDS_HAS_TOPIC_DISCOVERY
  struct bwhc *const whc = (struct bwhc *)it->c.whc;
  // 代理主题不存储在实体索引中，因为它们不是真正的实体。
  // 对于代理主题，遍历所有代理参与者并迭代每个代理参与者的所有代理主题
  ddsi_entidx_enum_init(&it->it, whc->entidx, DDSI_EK_PROXY_PARTICIPANT);
  it->cur_proxypp = NULL;
#else
  (void)it;
#endif
}

/**
 * @brief 根据实体创建样本
 *
 * @param[in] entity 指向 ddsi_entity_common 结构体的指针，表示要创建样本的实体
 * @return 返回一个指向 ddsi_serdata 结构体的指针，表示创建的样本
 */
static struct ddsi_serdata *make_sample(struct ddsi_entity_common *entity) {
  // 如果实体类型为主题
  if (entity->kind == DDSI_EK_TOPIC) {
#ifdef DDS_HAS_TOPIC_DISCOVERY
    // 使用实体和更新时间创建主题样本
    return dds__builtin_make_sample_topic(entity, entity->tupdate, true);
#else
    // 如果不支持主题发现，则断言失败并返回空指针
    assert(0);
    return NULL;
#endif
  } else {
    // 对于其他类型的实体，使用实体和更新时间创建端点样本
    return dds__builtin_make_sample_endpoint(entity, entity->tupdate, true);
  }
}

/**
 * @brief 从迭代器中借用下一个样本
 *
 * @param[in] opaque_it 指向 ddsi_whc_sample_iter 结构体的指针，用于存储迭代器状态
 * @param[out] sample 指向 ddsi_whc_borrowed_sample 结构体的指针，用于存储借用的样本数据
 * @return bool 如果成功获取下一个样本，则返回 true；否则返回 false
 */
static bool bwhc_sample_iter_borrow_next(struct ddsi_whc_sample_iter *opaque_it,
                                         struct ddsi_whc_borrowed_sample *sample) {
  // 将 opaque_it 转换为 bwhc_iter 类型的指针
  struct bwhc_iter *const it = (struct bwhc_iter *)opaque_it;
  // 将 it->c.whc 转换为 bwhc 类型的指针
  struct bwhc *const whc = (struct bwhc *)it->c.whc;
  // 初始化实体类型为参与者（pacify gcc）
  enum ddsi_entity_kind kind = DDSI_EK_PARTICIPANT;
  // 初始化实体指针为空
  struct ddsi_entity_common *entity = NULL;

  // 如果已经有样本，释放对应的序列化数据并设置 have_sample 为 false
  if (it->have_sample) {
    ddsi_serdata_unref(sample->serdata);
    it->have_sample = false;
  }

  // 使用 memset 清空 sample 结构体的大部分字段
  memset(sample, 0, sizeof(*sample));

  // 根据迭代器状态进行处理
  switch (it->st) {
    case BIS_INIT_LOCAL:
      // 根据 whc 的实体类型设置 kind 变量
      switch (whc->entity_kind) {
        case DSBT_PARTICIPANT:
          kind = DDSI_EK_PARTICIPANT;
          break;
        case DSBT_TOPIC:
          kind = DDSI_EK_TOPIC;
          break;
        case DSBT_WRITER:
          kind = DDSI_EK_WRITER;
          break;
        case DSBT_READER:
          kind = DDSI_EK_READER;
          break;
      }
      // 检查实体类型是否为参与者或其他有效类型
      assert(whc->entity_kind == DSBT_PARTICIPANT || kind != DDSI_EK_PARTICIPANT);
      // 初始化实体索引枚举器
      ddsi_entidx_enum_init(&it->it, whc->entidx, kind);
      // 设置迭代器状态为本地实体
      it->st = BIS_LOCAL;
      // 继续执行下一段代码（不需要显式的 break）
      /* FALLS THROUGH */
    case BIS_LOCAL:
      // 遍历本地实体，直到找到一个可见的实体
      while ((entity = ddsi_entidx_enum_next(&it->it)) != NULL)
        if (is_visible(entity)) break;
      // 如果找到了可见实体，创建样本并返回 true
      if (entity) {
        sample->serdata = make_sample(entity);
        it->have_sample = true;
        return true;
      }
      // 结束实体索引枚举器
      ddsi_entidx_enum_fini(&it->it);
      // 设置迭代器状态为初始化代理实体
      it->st = BIS_INIT_PROXY;
      // 继续执行下一段代码（不需要显式的 break）
      /* FALLS THROUGH */
    case BIS_INIT_PROXY:
      // 如果 whc 的实体类型是主题，则初始化代理主题迭代器
      if (whc->entity_kind == DSBT_TOPIC)
        init_proxy_topic_iteration(it);
      else {
        // 根据 whc 的实体类型设置 kind 变量
        switch (whc->entity_kind) {
          case DSBT_PARTICIPANT:
            kind = DDSI_EK_PROXY_PARTICIPANT;
            break;
          case DSBT_TOPIC:
            assert(0);
            break;
          case DSBT_WRITER:
            kind = DDSI_EK_PROXY_WRITER;
            break;
          case DSBT_READER:
            kind = DDSI_EK_PROXY_READER;
            break;
        }
        // 检查实体类型是否为参与者或其他有效类型
        assert(kind != DDSI_EK_PARTICIPANT);
        // 初始化实体索引枚举器
        ddsi_entidx_enum_init(&it->it, whc->entidx, kind);
      }

      // 设置迭代器状态为代理实体
      it->st = BIS_PROXY;
      // 继续执行下一段代码（不需要显式的 break）
      /* FALLS THROUGH */
    case BIS_PROXY:
      // 如果 whc 的实体类型是主题，则调用专门的代理主题迭代函数
      if (whc->entity_kind == DSBT_TOPIC)
        return bwhc_sample_iter_borrow_next_proxy_topic(it, sample);
      else {
        // 遍历代理实体，直到找到一个可见的实体
        while ((entity = ddsi_entidx_enum_next(&it->it)) != NULL)
          if (is_visible(entity)) break;
        // 如果没有找到可见实体，结束实体索引枚举器并返回 false
        if (!entity) {
          ddsi_entidx_enum_fini(&it->it);
          return false;
        }
        // 创建样本并设置 have_sample 为 true，然后返回 true
        sample->serdata = dds__builtin_make_sample_endpoint(entity, entity->tupdate, true);
        it->have_sample = true;
        return true;
      }
  }
  // 如果执行到这里，说明代码逻辑有误，触发断言
  assert(0);
  return false;
}
// 函数参数列表说明：
// whc: 指向ddsi_whc结构体的指针
// st: 指向ddsi_whc_state结构体的指针
static void bwhc_get_state(const struct ddsi_whc *whc, struct ddsi_whc_state *st) {
  (void)whc;              // 忽略未使用的参数whc
  st->max_seq = 0;        // 将st的max_seq设置为0
  st->min_seq = 0;        // 将st的min_seq设置为0
  st->unacked_bytes = 0;  // 将st的unacked_bytes设置为0
}

// 函数参数列表说明：
// whc: 指向ddsi_whc结构体的指针
// max_drop_seq: 最大丢弃序列号
// seq: 序列号
// exp: 过期时间
// serdata: 指向ddsi_serdata结构体的指针
// tk: 指向ddsi_tkmap_instance结构体的指针
static int bwhc_insert(struct ddsi_whc *whc,
                       ddsi_seqno_t max_drop_seq,
                       ddsi_seqno_t seq,
                       ddsrt_mtime_t exp,
                       struct ddsi_serdata *serdata,
                       struct ddsi_tkmap_instance *tk) {
  (void)whc;           // 忽略未使用的参数whc
  (void)max_drop_seq;  // 忽略未使用的参数max_drop_seq
  (void)seq;           // 忽略未使用的参数seq
  (void)exp;           // 忽略未使用的参数exp
  (void)serdata;       // 忽略未使用的参数serdata
  (void)tk;            // 忽略未使用的参数tk
  return 0;            // 返回0
}

// 函数参数列表说明：
// whc: 指向ddsi_whc结构体的指针
// max_drop_seq: 最大丢弃序列号
// whcst: 指向ddsi_whc_state结构体的指针
// deferred_free_list: 指向ddsi_whc_node结构体指针的指针
static uint32_t bwhc_remove_acked_messages(struct ddsi_whc *whc,
                                           ddsi_seqno_t max_drop_seq,
                                           struct ddsi_whc_state *whcst,
                                           struct ddsi_whc_node **deferred_free_list) {
  (void)whc;                   // 忽略未使用的参数whc
  (void)max_drop_seq;          // 忽略未使用的参数max_drop_seq
  (void)whcst;                 // 忽略未使用的参数whcst
  *deferred_free_list = NULL;  // 将deferred_free_list设置为NULL
  return 0;                    // 返回0
}

// 函数参数列表说明：
// whc: 指向ddsi_whc结构体的指针
// deferred_free_list: 指向ddsi_whc_node结构体的指针
static void bwhc_free_deferred_free_list(struct ddsi_whc *whc,
                                         struct ddsi_whc_node *deferred_free_list) {
  (void)whc;                 // 忽略未使用的参数whc
  (void)deferred_free_list;  // 忽略未使用的参数deferred_free_list
}

// 定义bwhc_ops结构体，包含各种操作函数指针
static const struct ddsi_whc_ops bwhc_ops = {
    .insert = bwhc_insert,
    .remove_acked_messages = bwhc_remove_acked_messages,
    .free_deferred_free_list = bwhc_free_deferred_free_list,
    .get_state = bwhc_get_state,
    .next_seq = 0,
    .borrow_sample = 0,
    .borrow_sample_key = 0,
    .return_sample = 0,
    .sample_iter_init = bwhc_sample_iter_init,
    .sample_iter_borrow_next = bwhc_sample_iter_borrow_next,
    .free = bwhc_free};

// 函数参数列表说明：
// entity_kind: 枚举类型ddsi_sertype_builtintopic_entity_kind的值
// entidx: 指向ddsi_entity_index结构体的指针
struct ddsi_whc *dds_builtintopic_whc_new(enum ddsi_sertype_builtintopic_entity_kind entity_kind,
                                          const struct ddsi_entity_index *entidx) {
  struct bwhc *whc = ddsrt_malloc(sizeof(*whc));  // 分配内存给whc
  whc->common.ops = &bwhc_ops;                    // 将whc的ops字段设置为bwhc_ops的地址
  whc->entity_kind = entity_kind;  // 将whc的entity_kind字段设置为传入的entity_kind
  whc->entidx = entidx;            // 将whc的entidx字段设置为传入的entidx
  return (struct ddsi_whc *)whc;   // 返回类型转换后的whc指针
}
