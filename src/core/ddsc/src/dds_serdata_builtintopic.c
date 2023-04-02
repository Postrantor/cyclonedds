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
#include <ctype.h>
#include <stddef.h>
#include <string.h>

#include "dds/ddsrt/heap.h"
#include "dds/ddsrt/hopscotch.h"
#include "dds/ddsrt/md5.h"
#include "dds/ddsrt/string.h"

#include "dds/ddsi/ddsi_domaingv.h"
#include "dds/ddsi/ddsi_entity_index.h"
#include "dds/ddsi/ddsi_plist.h"
#include "dds/ddsi/ddsi_tkmap.h"
#include "dds/ddsi/ddsi_xqos.h"

#include "dds/ddsi/ddsi_addrset.h"
#include "dds/ddsi/ddsi_entity.h"
#include "dds/ddsi/ddsi_freelist.h"
#include "dds/ddsi/ddsi_participant.h"
#include "dds/ddsi/ddsi_proxy_participant.h"

#include "dds__serdata_builtintopic.h"

// 定义一个常量数组，用于哈希计算
static const uint64_t unihashconsts[] = {
    UINT64_C(16292676669999574021),
    UINT64_C(10242350189706880077),
    UINT64_C(12844332200329132887),
    UINT64_C(16728792139623414127)};

// 计算GUID的哈希值
static uint32_t hash_guid(const ddsi_guid_t *g)
{
  // 使用unihashconsts数组中的常量进行哈希计算
  return (uint32_t)(((((uint32_t)g->prefix.u[0] + unihashconsts[0]) *
                      ((uint32_t)g->prefix.u[1] + unihashconsts[1])) +
                     (((uint32_t)g->prefix.u[2] + unihashconsts[2]) *
                      ((uint32_t)g->entityid.u + unihashconsts[3]))) >>
                    32);
}

// 修复内置序列化数据
static struct ddsi_serdata *fix_serdata_builtin(struct ddsi_serdata_builtintopic *d, enum ddsi_sertype_builtintopic_entity_kind kind, uint32_t basehash)
{
#ifndef DDS_HAS_TOPIC_DISCOVERY
  // 如果没有主题发现功能，则实体类型不能为DSBT_TOPIC
  assert(kind != DSBT_TOPIC);
#endif
  // 根据实体类型计算哈希值
  if (kind == DSBT_TOPIC)
    d->c.hash = (*(uint32_t *)d->key.raw) ^ basehash;
  else
    d->c.hash = hash_guid(&d->key.guid) ^ basehash;
  return &d->c;
}

// 比较两个序列化数据的键是否相等
static bool serdata_builtin_eqkey(const struct ddsi_serdata *acmn, const struct ddsi_serdata *bcmn)
{
  struct ddsi_serdata_builtintopic *a = (struct ddsi_serdata_builtintopic *)acmn;
  struct ddsi_serdata_builtintopic *b = (struct ddsi_serdata_builtintopic *)bcmn;
  // 确保键的原始数据和GUID的大小相同
  DDSRT_STATIC_ASSERT(sizeof(a->key.raw) == sizeof(a->key.guid));
#ifndef DDS_HAS_TOPIC_DISCOVERY
  // 如果没有主题发现功能，则实体类型不能为DSBT_TOPIC
  assert(((struct ddsi_sertype_builtintopic *)acmn->type)->entity_kind != DSBT_TOPIC);
#endif
  // 比较两个键的值是否相等
  return memcmp(&a->key, &b->key, sizeof(a->key)) == 0;
}
// 定义静态函数 serdata_builtin_free，用于释放 ddsi_serdata 结构体
static void serdata_builtin_free(struct ddsi_serdata *dcmn)
{
  // 将通用的 ddsi_serdata 结构体转换为 ddsi_serdata_builtintopic 结构体
  struct ddsi_serdata_builtintopic *d = (struct ddsi_serdata_builtintopic *)dcmn;
  // 如果数据类型为 SDK_DATA，则释放 xqos 结构体
  if (d->c.kind == SDK_DATA)
    ddsi_xqos_fini(&d->xqos);
  // 释放 d 结构体内存
  ddsrt_free(d);
}

// 定义静态函数 serdata_builtin_new，根据实体类型创建相应的 ddsi_serdata_builtintopic 结构体
static struct ddsi_serdata_builtintopic *serdata_builtin_new(const struct ddsi_sertype_builtintopic *tp, enum ddsi_serdata_kind serdata_kind)
{
  size_t size = 0;
  // 根据实体类型选择相应的结构体大小
  switch (tp->entity_kind)
  {
  case DSBT_PARTICIPANT:
    size = sizeof(struct ddsi_serdata_builtintopic_participant);
    break;
  case DSBT_TOPIC:
#ifdef DDS_HAS_TOPIC_DISCOVERY
    size = sizeof(struct ddsi_serdata_builtintopic_topic);
#else
    assert(0);
#endif
    break;
  case DSBT_READER:
  case DSBT_WRITER:
    size = sizeof(struct ddsi_serdata_builtintopic_endpoint);
    break;
  }
  // 分配内存并初始化 ddsi_serdata_builtintopic 结构体
  struct ddsi_serdata_builtintopic *d = ddsrt_malloc(size);
  ddsi_serdata_init(&d->c, &tp->c, serdata_kind);
  return d;
}

// 定义结构体 format_address_arg，用于格式化地址参数
struct format_address_arg
{
  char *buf;       // 缓冲区指针
  size_t buf_pos;  // 缓冲区当前位置
  size_t buf_size; // 缓冲区大小
  bool first;      // 是否为第一个地址
};

// 定义一个静态函数 format_address，参数为指向 ddsi_xlocator_t 类型的指针 n 和指向 void 类型的指针 varg
static void format_address(const ddsi_xlocator_t *n, void *varg)
{
  // 将 varg 转换为指向 format_address_arg 结构体的指针 arg
  struct format_address_arg *arg = varg;
  // 定义一个字符数组 buf，大小为 DDSI_LOCSTRLEN
  char buf[DDSI_LOCSTRLEN];

  // 如果 arg 的 buf 成员为空，则直接返回
  if (!arg->buf)
    return;

  // 将 n 转换为字符串并存储在 buf 中
  ddsi_xlocator_to_string(buf, sizeof(buf), n);
  // 计算 buf 的长度（包括逗号分隔符）
  const size_t nsize = strlen(buf) + (arg->first ? 0 : 1);

  // 如果 nsize 大于剩余可用空间，则重新分配内存
  if (nsize > arg->buf_size - arg->buf_pos - 1)
  {
    // 增加缓冲区大小
    arg->buf_size += 4 * nsize;
    // 使用 ddsrt_realloc 函数重新分配内存，并将结果赋值给 new_buffer
    char *new_buffer = ddsrt_realloc(arg->buf, arg->buf_size);

    // 如果新分配的内存为空，则直接返回
    if (!new_buffer)
      return;
    // 更新 arg 的 buf 指针为新分配的内存
    arg->buf = new_buffer;
  }

  // 将 buf 中的内容追加到 arg 的 buf 中，并更新 buf_pos
  arg->buf_pos += (size_t)snprintf(arg->buf + arg->buf_pos, arg->buf_size - arg->buf_pos, "%s%s", arg->first ? "" : ",", buf);
  // 如果是第一个元素，则将 first 设置为 false
  if (arg->first)
    arg->first = false;
}
// 定义一个静态函数ddsi_format_addrset，参数为一个指向ddsi_addrset结构体的指针，返回值为一个字符指针
static char *ddsi_format_addrset(struct ddsi_addrset *as)
{
  // 定义一个名为pa_arg的format_address_arg结构体变量
  struct format_address_arg pa_arg;

  // 为pa_arg.buf分配内存空间，大小为DDSI_LOCSTRLEN * 3 + 4字节
  pa_arg.buf = (char *)ddsrt_malloc(DDSI_LOCSTRLEN * 3 + 4);

  // 初始化pa_arg.buf_pos为0
  pa_arg.buf_pos = 0;

  // 设置pa_arg.buf_size为DDSI_LOCSTRLEN * 3 + 4
  pa_arg.buf_size = DDSI_LOCSTRLEN * 3 + 4;

  // 将pa_arg.first设置为true
  pa_arg.first = true;

  // 调用ddsi_addrset_forall函数，遍历as中的所有地址，并对每个地址调用format_address函数，传入&pa_arg作为参数
  ddsi_addrset_forall(as, format_address, &pa_arg);

  // 返回pa_arg.buf
  return pa_arg.buf;
}
// 定义一个静态函数 add_pp_addresses_to_xqos，接受两个参数：dds_qos_t 类型的指针 q 和 ddsi_proxy_participant 结构体类型的常量指针 proxypp
static void add_pp_addresses_to_xqos(dds_qos_t *q, const struct ddsi_proxy_participant *proxypp)
{
  // 调用 ddsi_format_addrset 函数，将 proxypp 的 as_meta 成员格式化为字符串，并将结果赋值给 char 类型的指针 addresses
  char *addresses = ddsi_format_addrset(proxypp->as_meta);

  // 判断 addresses 是否不为空
  if (addresses)
  {
    // 如果 addresses 不为空，则调用 ddsi_xqos_add_property_if_unset 函数，将 addresses 添加到 q 的属性中
    ddsi_xqos_add_property_if_unset(q, true, DDS_BUILTIN_TOPIC_PARTICIPANT_PROPERTY_NETWORKADDRESSES, addresses);

    // 使用 ddsrt_free 函数释放 addresses 所占用的内存空间
    ddsrt_free(addresses);
  }
}

// 定义一个静态函数 from_entity_pp，用于从 ddsi_participant 结构体中提取信息并填充到 ddsi_serdata_builtintopic_participant 结构体中
static void from_entity_pp(struct ddsi_serdata_builtintopic_participant *d, const struct ddsi_participant *pp)
{
  // 将 pp->plist->qos 中的质量服务设置复制到 d->common.xqos 中
  ddsi_xqos_copy(&d->common.xqos, &pp->plist->qos);

  // 如果 d->common.xqos 中没有设置 DDS_BUILTIN_TOPIC_PARTICIPANT_PROPERTY_NETWORKADDRESSES 属性，则添加该属性并设置其值为 "localprocess"
  ddsi_xqos_add_property_if_unset(&d->common.xqos, true, DDS_BUILTIN_TOPIC_PARTICIPANT_PROPERTY_NETWORKADDRESSES, "localprocess");

  // 将 pp->e.iid 的值赋给 d->pphandle
  d->pphandle = pp->e.iid;
}
// 从代理参与者（proxypp）中提取信息并填充到 ddsi_serdata_builtintopic_participant 结构体中
static void from_entity_proxypp(struct ddsi_serdata_builtintopic_participant *d, const struct ddsi_proxy_participant *proxypp)
{
  // 复制代理参与者的 QoS 设置到 d->common.xqos 中
  ddsi_xqos_copy(&d->common.xqos, &proxypp->plist->qos);

  // 将代理参与者的地址添加到 d->common.xqos 中
  add_pp_addresses_to_xqos(&d->common.xqos, proxypp);

  // 设置 d->pphandle 为代理参与者的实例标识符
  d->pphandle = proxypp->e.iid;
}

// 从 QoS 设置中提取信息并填充到 ddsi_serdata_builtintopic 结构体中
static void from_qos(struct ddsi_serdata_builtintopic *d, const dds_qos_t *xqos)
{
  // 复制 QoS 设置到 d->xqos 中
  ddsi_xqos_copy(&d->xqos, xqos);

  // 确保 d->xqos.present 包含 DDSI_QP_TOPIC_NAME 标志
  assert(d->xqos.present & DDSI_QP_TOPIC_NAME);

  // 确保 d->xqos.present 包含 DDSI_QP_TYPE_NAME 标志
  assert(d->xqos.present & DDSI_QP_TYPE_NAME);
}
// 从实体读取器（Reader）中提取信息并存储到 ddsi_serdata_builtintopic_endpoint 结构体中
static void from_entity_rd(struct ddsi_serdata_builtintopic_endpoint *d, const struct ddsi_reader *rd)
{
  // 将读取器所属的参与者实例标识符赋值给 d->pphandle
  d->pphandle = rd->c.pp->e.iid;
  // 提取读取器的 QoS 设置并存储到 d->common 中
  from_qos(&d->common, rd->xqos);
}

// 从实体写入器（Writer）中提取信息并存储到 ddsi_serdata_builtintopic_endpoint 结构体中
static void from_entity_wr(struct ddsi_serdata_builtintopic_endpoint *d, const struct ddsi_writer *wr)
{
  // 将写入器所属的参与者实例标识符赋值给 d->pphandle
  d->pphandle = wr->c.pp->e.iid;
  // 提取写入器的 QoS 设置并存储到 d->common 中
  from_qos(&d->common, wr->xqos);
}

// 从代理端点公共结构体中提取信息并存储到 ddsi_serdata_builtintopic_endpoint 结构体中
static void from_proxy_endpoint_common(struct ddsi_serdata_builtintopic_endpoint *d, const struct ddsi_proxy_endpoint_common *pec)
{
  // 将代理端点所属的参与者实例标识符赋值给 d->pphandle
  d->pphandle = pec->proxypp->e.iid;
  // 提取代理端点的 QoS 设置并存储到 d->common 中
  from_qos(&d->common, pec->xqos);
}

// 从实体代理读取器（Proxy Reader）中提取信息并存储到 ddsi_serdata_builtintopic_endpoint 结构体中
static void from_entity_proxy_rd(struct ddsi_serdata_builtintopic_endpoint *d, const struct ddsi_proxy_reader *proxyrd)
{
  // 调用 from_proxy_endpoint_common 函数处理公共部分
  from_proxy_endpoint_common(d, &proxyrd->c);
}

// 从实体代理写入器（Proxy Writer）中提取信息并存储到 ddsi_serdata_builtintopic_endpoint 结构体中
static void from_entity_proxy_wr(struct ddsi_serdata_builtintopic_endpoint *d, const struct ddsi_proxy_writer *proxywr)
{
  // 调用 from_proxy_endpoint_common 函数处理公共部分
  from_proxy_endpoint_common(d, &proxywr->c);
}
// 从实体端点创建 ddsi_serdata 结构体
struct ddsi_serdata *dds_serdata_builtin_from_endpoint(const struct ddsi_sertype *tpcmn, const ddsi_guid_t *guid, struct ddsi_entity_common *entity, enum ddsi_serdata_kind kind)
{
  // 将通用的 ddsi_sertype 转换为 ddsi_sertype_builtintopic 类型
  const struct ddsi_sertype_builtintopic *tp = (const struct ddsi_sertype_builtintopic *)tpcmn;
  // 确保实体类型不是 DSBT_TOPIC
  assert(tp->entity_kind != DSBT_TOPIC);
  // 创建一个新的 ddsi_serdata_builtintopic 结构体
  struct ddsi_serdata_builtintopic *d = serdata_builtin_new(tp, kind);
  // 设置 d->key.guid 为传入的 guid
  d->key.guid = *guid;
  // 如果实体非空且数据类型为 SDK_DATA，则处理实体信息
  if (entity != NULL && kind == SDK_DATA)
  {
    // 加锁实体的 QoS 设置
    ddsrt_mutex_lock(&entity->qos_lock);
    // 根据实体类型进行相应的处理
    switch (entity->kind)
    {
    case DDSI_EK_PARTICIPANT:
      // 断言实体类型为 DSBT_PARTICIPANT
      assert(tp->entity_kind == DSBT_PARTICIPANT);
      // 处理参与者实体
      from_entity_pp((struct ddsi_serdata_builtintopic_participant *)d, (const struct ddsi_participant *)entity);
      break;
    case DDSI_EK_READER:
      // 断言实体类型为 DSBT_READER
      assert(tp->entity_kind == DSBT_READER);
      // 处理读取器实体
      from_entity_rd((struct ddsi_serdata_builtintopic_endpoint *)d, (const struct ddsi_reader *)entity);
      break;
    case DDSI_EK_WRITER:
      // 断言实体类型为 DSBT_WRITER
      assert(tp->entity_kind == DSBT_WRITER);
      // 处理写入器实体
      from_entity_wr((struct ddsi_serdata_builtintopic_endpoint *)d, (const struct ddsi_writer *)entity);
      break;
    case DDSI_EK_PROXY_PARTICIPANT:
      // 断言实体类型为 DSBT_PARTICIPANT
      assert(tp->entity_kind == DSBT_PARTICIPANT);
      // 处理代理参与者实体
      from_entity_proxypp((struct ddsi_serdata_builtintopic_participant *)d, (const struct ddsi_proxy_participant *)entity);
      break;
    case DDSI_EK_PROXY_READER:
      // 断言实体类型为 DSBT_READER
      assert(tp->entity_kind == DSBT_READER);
      // 处理代理读取器实体
      from_entity_proxy_rd((struct ddsi_serdata_builtintopic_endpoint *)d, (const struct ddsi_proxy_reader *)entity);
      break;
    case DDSI_EK_PROXY_WRITER:
      // 断言实体类型为 DSBT_WRITER
      assert(tp->entity_kind == DSBT_WRITER);
      // 处理代理写入器实体
      from_entity_proxy_wr((struct ddsi_serdata_builtintopic_endpoint *)d, (const struct ddsi_proxy_writer *)entity);
      break;
    case DDSI_EK_TOPIC:
      // 如果实体类型为 DSBT_TOPIC，则终止程序
      abort();
      break;
    }
    // 解锁实体的 QoS 设置
    ddsrt_mutex_unlock(&entity->qos_lock);
  }
  // 返回修复后的 ddsi_serdata_builtintopic 结构体
  return fix_serdata_builtin(d, tp->entity_kind, tp->c.serdata_basehash);
}
// 从样本中创建 ddsi_serdata 结构体
static struct ddsi_serdata *ddsi_serdata_builtin_from_sample(const struct ddsi_sertype *tpcmn, enum ddsi_serdata_kind kind, const void *sample)
{
  // 将通用的 ddsi_sertype 转换为 ddsi_sertype_builtintopic 类型
  const struct ddsi_sertype_builtintopic *tp = (const struct ddsi_sertype_builtintopic *)tpcmn;
  // 定义一个联合体，用于存储 GUID 和 keyhash
  union
  {
    dds_guid_t extguid;
    ddsi_guid_t guid;
    ddsi_keyhash_t keyhash;
  } x;

  /* 如果 kind 不是 SDK_KEY，则返回 NULL。因为不应该尝试将用户提供的数据转换为内置主题样本，
     但是在某些情况下（例如 dds_lookup_instance），可能需要转换 key */
  if (kind != SDK_KEY)
    return NULL;

  /* 使用 memset 初始化 x（即使这是完全多余的），这样我们可以在 switch 中省略 default case
     （确保至少有一些编译器会在添加更多类型时发出警告），而不会收到任何编译器的警告 */
  memset(&x, 0, sizeof(x));
  // 根据实体类型进行相应的处理
  switch (tp->entity_kind)
  {
  case DSBT_PARTICIPANT:
  {
    // 将样本转换为 dds_builtintopic_participant_t 类型
    const dds_builtintopic_participant_t *s = sample;
    // 将 s->key 赋值给 x.extguid
    x.extguid = s->key;
    break;
  }
  case DSBT_READER:
  case DSBT_WRITER:
  {
    // 将样本转换为 dds_builtintopic_endpoint_t 类型
    const dds_builtintopic_endpoint_t *s = sample;
    // 将 s->key 赋值给 x.extguid
    x.extguid = s->key;
    break;
  }
  case DSBT_TOPIC:
    /* 不应该用于主题。对于主题，将使用一组特定的 ddsi_serdata_ops，
       其中包含此函数的专门变体。 */
    assert(0);
    break;
  }
  // 获取 ddsi_domaingv 结构体指针
  struct ddsi_domaingv *const gv = ddsrt_atomic_ldvoidp(&tp->c.gv);
  // 将 GUID 转换为网络字节序
  x.guid = ddsi_ntoh_guid(x.guid);
  // 在实体索引中查找实体
  struct ddsi_entity_common *entity = ddsi_entidx_lookup_guid_untyped(gv->entity_index, &x.guid);
  // 从实体端点创建 ddsi_serdata 结构体并返回
  return dds_serdata_builtin_from_endpoint(tpcmn, &x.guid, entity, kind);
}
/* 将通用序列化数据转换为未类型化序列化数据 */
static struct ddsi_serdata *serdata_builtin_to_untyped(const struct ddsi_serdata *serdata_common)
{
  /* 当前所有内置类型都是未类型化的 */
  return ddsi_serdata_ref(serdata_common);
}

/* 转换 GUID 以生成 key */
static void convkey(dds_guid_t *key, const ddsi_guid_t *guid)
{
  ddsi_guid_t tmp;
  /* 将 GUID 转换为网络字节序 */
  tmp = ddsi_hton_guid(*guid);
  /* 将转换后的 GUID 复制到 key 中 */
  memcpy(key, &tmp, sizeof(*key));
}

/* 复制字符串并重用旧内存空间 */
static char *dds_string_dup_reuse(char *old, const char *src)
{
  size_t size = strlen(src) + 1;
  /* 使用新大小重新分配内存 */
  char *new = dds_realloc(old, size);
  /* 将源字符串复制到新内存位置 */
  return memcpy(new, src, size);
}

/* 从 xqos 创建 dds_qos 并重用旧内存空间 */
static dds_qos_t *dds_qos_from_xqos_reuse(dds_qos_t *old, const dds_qos_t *src)
{
  if (old == NULL)
    /* 如果旧内存为空，则分配新内存 */
    old = ddsrt_malloc(sizeof(*old));
  else
  {
    /* 否则，清除旧内存中的 xqos 数据 */
    ddsi_xqos_fini(old);
  }
  /* 初始化空的 xqos */
  ddsi_xqos_init_empty(old);
  /* 将 src 中缺失的 qos 参数合并到 old 中 */
  ddsi_xqos_mergein_missing(old, src, ~(DDSI_QP_TOPIC_NAME | DDSI_QP_TYPE_NAME));
  return old;
}

/* 将内置主题参与者序列化数据转换为样本 */
static bool to_sample_pp(const struct ddsi_serdata_builtintopic_participant *d, struct dds_builtintopic_participant *sample)
{
  /* 转换 GUID 以生成 key */
  convkey(&sample->key, &d->common.key.guid);
  if (d->common.c.kind == SDK_DATA)
    /* 如果数据类型为 SDK_DATA，则从 xqos 创建 qos */
    sample->qos = dds_qos_from_xqos_reuse(sample->qos, &d->common.xqos);
  return true;
}
/* 将内置主题端点序列化数据转换为样本 */
static bool to_sample_endpoint(const struct ddsi_serdata_builtintopic_endpoint *dep, struct dds_builtintopic_endpoint *sample)
{
  ddsi_guid_t ppguid;
  /* 转换 GUID 以生成 key */
  convkey(&sample->key, &dep->common.key.guid);
  ppguid = dep->common.key.guid;
  ppguid.entityid.u = DDSI_ENTITYID_PARTICIPANT;
  /* 转换参与者 GUID 以生成参与者 key */
  convkey(&sample->participant_key, &ppguid);
  /* 设置参与者实例句柄 */
  sample->participant_instance_handle = dep->pphandle;
  if (dep->common.c.kind == SDK_DATA)
  {
    /* 确保主题名称和类型名称存在 */
    assert(dep->common.xqos.present & DDSI_QP_TOPIC_NAME);
    assert(dep->common.xqos.present & DDSI_QP_TYPE_NAME);
    /* 复制主题名称并重用旧内存空间 */
    sample->topic_name = dds_string_dup_reuse(sample->topic_name, dep->common.xqos.topic_name);
    /* 复制类型名称并重用旧内存空间 */
    sample->type_name = dds_string_dup_reuse(sample->type_name, dep->common.xqos.type_name);
    /* 从 xqos 创建 qos 并重用旧内存空间 */
    sample->qos = dds_qos_from_xqos_reuse(sample->qos, &dep->common.xqos);
  }
  return true;
}

#ifdef DDS_HAS_TOPIC_DISCOVERY
/* 将内置主题序列化数据转换为样本 */
static bool to_sample_topic(const struct ddsi_serdata_builtintopic_topic *dtp, struct dds_builtintopic_topic *sample)
{
  /* 复制 key */
  memcpy(&sample->key, &dtp->common.key.raw, sizeof(sample->key));
  if (dtp->common.c.kind == SDK_DATA)
  {
    /* 确保主题名称和类型名称存在 */
    assert(dtp->common.xqos.present & DDSI_QP_TOPIC_NAME);
    assert(dtp->common.xqos.present & DDSI_QP_TYPE_NAME);
    /* 复制主题名称并重用旧内存空间 */
    sample->topic_name = dds_string_dup_reuse(sample->topic_name, dtp->common.xqos.topic_name);
    /* 复制类型名称并重用旧内存空间 */
    sample->type_name = dds_string_dup_reuse(sample->type_name, dtp->common.xqos.type_name);
    /* 从 xqos 创建 qos 并重用旧内存空间 */
    sample->qos = dds_qos_from_xqos_reuse(sample->qos, &dtp->common.xqos);
  }
  return true;
}
#endif /* DDS_HAS_TOPIC_DISCOVERY */
// 将未类型化的内置序列化数据转换为样本
static bool serdata_builtin_untyped_to_sample(const struct ddsi_sertype *type, const struct ddsi_serdata *serdata_common, void *sample, void **bufptr, void *buflim)
{
  // 将通用序列化数据结构转换为内置主题序列化数据结构
  const struct ddsi_serdata_builtintopic *d = (const struct ddsi_serdata_builtintopic *)serdata_common;
  // 将通用序列化类型结构转换为内置主题序列化类型结构
  const struct ddsi_sertype_builtintopic *tp = (const struct ddsi_sertype_builtintopic *)type;
  // 如果bufptr不为空，则调用abort()函数终止程序
  if (bufptr)
    abort();
  else
  {
    // 否则，忽略buflim参数
    (void)buflim;
  } /* FIXME: haven't implemented that bit yet! */
  /* FIXME: completing builtin topic support along these lines requires subscribers, publishers and topics to also become DDSI entities - which is probably a good thing anyway */
  // 根据实体类型进行相应的处理
  switch (tp->entity_kind)
  {
  case DSBT_PARTICIPANT:
    // 处理参与者实体
    return to_sample_pp((struct ddsi_serdata_builtintopic_participant *)d, sample);
  case DSBT_TOPIC:
#ifdef DDS_HAS_TOPIC_DISCOVERY
    // 处理主题实体（如果启用了主题发现）
    return to_sample_topic((struct ddsi_serdata_builtintopic_topic *)d, sample);
#else
    break;
#endif
  case DSBT_READER:
  case DSBT_WRITER:
    // 处理读取器或写入器实体
    return to_sample_endpoint((struct ddsi_serdata_builtintopic_endpoint *)d, sample);
  }
  // 如果没有匹配的实体类型，触发断言
  assert(0);
  return false;
}

// 将内置序列化数据转换为样本
static bool serdata_builtin_to_sample(const struct ddsi_serdata *serdata_common, void *sample, void **bufptr, void *buflim)
{
  return serdata_builtin_untyped_to_sample(serdata_common->type, serdata_common, sample, bufptr, buflim);
}

// 获取内置序列化数据的大小
static uint32_t serdata_builtin_get_size(const struct ddsi_serdata *serdata_common)
{
  // 忽略serdata_common参数
  (void)serdata_common;
  // 返回0（表示未实现）
  return 0;
}
// 将内置序列化数据转换为序列化字节流
static void serdata_builtin_to_ser(const struct ddsi_serdata *serdata_common, size_t off, size_t sz, void *buf)
{
  // 忽略所有参数，因为此功能尚未实现
  (void)serdata_common;
  (void)off;
  (void)sz;
  (void)buf;
}

// 将内置序列化数据转换为序列化字节流的引用
static struct ddsi_serdata *serdata_builtin_to_ser_ref(const struct ddsi_serdata *serdata_common, size_t off, size_t sz, ddsrt_iovec_t *ref)
{
  // 忽略所有参数，因为此功能尚未实现
  (void)serdata_common;
  (void)off;
  (void)sz;
  (void)ref;
  // 返回NULL表示未实现
  return NULL;
}

// 取消对内置序列化数据的序列化字节流引用
static void serdata_builtin_to_ser_unref(struct ddsi_serdata *serdata_common, const ddsrt_iovec_t *ref)
{
  // 忽略所有参数，因为此功能尚未实现
  (void)serdata_common;
  (void)ref;
}

// 打印内置序列化数据类型的信息
static size_t serdata_builtin_type_print(const struct ddsi_sertype *type, const struct ddsi_serdata *serdata_common, char *buf, size_t size)
{
  // 忽略type和serdata_common参数，因为此功能尚未实现
  (void)type;
  (void)serdata_common;
  // 将"(blob)"字符串写入buf，并返回实际写入的字符数（不包括结尾的'\0'）
  return (size_t)snprintf(buf, size, "(blob)");
}
// 定义一个名为ddsi_serdata_ops_builtintopic的结构体常量，用于存储内置主题序列化数据操作函数
const struct ddsi_serdata_ops ddsi_serdata_ops_builtintopic = {
    .get_size = serdata_builtin_get_size,                   // 获取序列化数据大小的函数
    .eqkey = serdata_builtin_eqkey,                         // 比较键是否相等的函数
    .free = serdata_builtin_free,                           // 释放序列化数据的函数
    .from_ser = 0,                                          // 从序列化数据创建serdata的函数（未实现）
    .from_ser_iov = 0,                                      // 从序列化数据IO向量创建serdata的函数（未实现）
    .from_keyhash = 0,                                      // 从键哈希创建serdata的函数（未实现）
    .from_sample = ddsi_serdata_builtin_from_sample,        // 从样本创建serdata的函数
    .to_ser = serdata_builtin_to_ser,                       // 将serdata转换为序列化数据的函数
    .to_sample = serdata_builtin_to_sample,                 // 将serdata转换为样本的函数
    .to_ser_ref = serdata_builtin_to_ser_ref,               // 将serdata转换为序列化数据引用的函数
    .to_ser_unref = serdata_builtin_to_ser_unref,           // 取消serdata序列化数据引用的函数
    .to_untyped = serdata_builtin_to_untyped,               // 将serdata转换为无类型serdata的函数
    .untyped_to_sample = serdata_builtin_untyped_to_sample, // 将无类型serdata转换为样本的函数
    .print = serdata_builtin_type_print,                    // 打印序列化数据类型的函数
    .get_keyhash = 0};                                      // 获取键哈希的函数（未实现）

#ifdef DDS_HAS_TOPIC_DISCOVERY

// 从主题定义创建内置序列化数据的函数
struct ddsi_serdata *dds_serdata_builtin_from_topic_definition(const struct ddsi_sertype *tpcmn, const dds_builtintopic_topic_key_t *key, const struct ddsi_topic_definition *tpd, enum ddsi_serdata_kind kind)
{
  const struct ddsi_sertype_builtintopic *tp = (const struct ddsi_sertype_builtintopic *)tpcmn;
  assert(tp->entity_kind == DSBT_TOPIC);
  struct ddsi_serdata_builtintopic_topic *d = (struct ddsi_serdata_builtintopic_topic *)serdata_builtin_new(tp, kind);
  memcpy(&d->common.key.raw, key, sizeof(d->common.key.raw));
  if (tpd != NULL && kind == SDK_DATA)
    from_qos(&d->common, tpd->xqos);
  return fix_serdata_builtin(&d->common, DSBT_TOPIC, tp->c.serdata_basehash);
}

// 从内置主题样本创建序列化数据的函数
static struct ddsi_serdata *ddsi_serdata_builtin_from_sample_topic(const struct ddsi_sertype *tpcmn, enum ddsi_serdata_kind kind, const void *sample)
{
  // 如果kind不是SDK_KEY，则返回NULL，因为不应该尝试将用户提供的数据转换为内置主题样本
  // 如果kind不是SDK_KEY，则返回NULL，因为不应该尝试将用户提供的数据转换为内置主题样本
  if (kind != SDK_KEY)
    return NULL;

  // 将tpcmn强制转换为ddsi_sertype_builtintopic类型，并赋值给tp
  const struct ddsi_sertype_builtintopic *tp = (const struct ddsi_sertype_builtintopic *)tpcmn;
  // 断言tp的实体类型为DSBT_TOPIC
  assert(tp->entity_kind == DSBT_TOPIC);
  // 从tp中获取domaingv并赋值给gv
  struct ddsi_domaingv *gv = ddsrt_atomic_ldvoidp(&tp->c.gv);
  // 将sample强制转换为dds_builtintopic_topic_t类型，并赋值给s
  const dds_builtintopic_topic_t *s = sample;
  // 定义一个联合体x，包含guid和key两个成员
  union
  {
    ddsi_guid_t guid;
    dds_builtintopic_topic_key_t key;
  } x;
  // 将s的key赋值给x的key
  x.key = s->key;
  // 将x的guid转换为网络字节序并赋值给x的guid
  x.guid = ddsi_ntoh_guid(x.guid);
  // 定义一个名为templ的ddsi_topic_definition结构体变量，并将其所有字段初始化为0
  struct ddsi_topic_definition templ;
  memset(&templ, 0, sizeof(templ));
  // 将x的key复制到templ的key
  memcpy(&templ.key, &x.key, sizeof(templ.key));
  // 对gv的topic_defs_lock加锁
  ddsrt_mutex_lock(&gv->topic_defs_lock);
  // 在gv的topic_defs中查找与templ匹配的ddsi_topic_definition，并赋值给tpd
  struct ddsi_topic_definition *tpd = ddsrt_hh_lookup(gv->topic_defs, &templ);
  // 使用tpcmn、x的key、tpd和kind创建一个ddsi_serdata对象，并赋值给sd
  struct ddsi_serdata *sd = dds_serdata_builtin_from_topic_definition(tpcmn, &x.key, tpd, kind);
  // 对gv的topic_defs_lock解锁
  ddsrt_mutex_unlock(&gv->topic_defs_lock);
  // 返回sd
  return sd;
}

// 定义一个名为ddsi_serdata_ops_builtintopic_topic的结构体常量，用于存储内置主题序列化数据操作函数
const struct ddsi_serdata_ops ddsi_serdata_ops_builtintopic_topic = {
    .get_size = serdata_builtin_get_size,                   // 获取序列化数据大小的函数
    .eqkey = serdata_builtin_eqkey,                         // 比较键是否相等的函数
    .free = serdata_builtin_free,                           // 释放序列化数据的函数
    .from_ser = 0,                                          // 从序列化数据创建serdata的函数（未实现）
    .from_ser_iov = 0,                                      // 从序列化数据IO向量创建serdata的函数（未实现）
    .from_keyhash = 0,                                      // 从键哈希创建serdata的函数（未实现）
    .from_sample = ddsi_serdata_builtin_from_sample_topic,  // 从样本创建serdata的函数
    .to_ser = serdata_builtin_to_ser,                       // 将serdata转换为序列化数据的函数
    .to_sample = serdata_builtin_to_sample,                 // 将serdata转换为样本的函数
    .to_ser_ref = serdata_builtin_to_ser_ref,               // 将serdata转换为序列化数据引用的函数
    .to_ser_unref = serdata_builtin_to_ser_unref,           // 取消serdata序列化数据引用的函数
    .to_untyped = serdata_builtin_to_untyped,               // 将serdata转换为无类型serdata的函数
    .untyped_to_sample = serdata_builtin_untyped_to_sample, // 将无类型serdata转换为样本的函数
    .print = serdata_builtin_type_print,                    // 打印序列化数据类型的函数
    .get_keyhash = 0};                                      // 获取键哈希的函数（未实现）

#endif /* DDS_HAS_TOPIC_DISCOVERY */
