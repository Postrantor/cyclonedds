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

#include "dds/cdr/dds_cdrstream.h"
#include "dds/ddsi/ddsi_domaingv.h"
#include "dds/ddsi/ddsi_freelist.h"
#include "dds/ddsi/ddsi_sertype.h"
#include "dds/ddsi/ddsi_typelib.h"
#include "dds/ddsi/ddsi_xqos.h"
#include "dds/ddsi/ddsi_xt_typeinfo.h"
#include "dds/ddsrt/heap.h"
#include "dds/ddsrt/md5.h"
#include "dds/ddsrt/mh3.h"
#include "dds/ddsrt/string.h"
#include "dds/features.h"
#include "dds__serdata_default.h"

#ifdef DDS_HAS_SHM
#include "dds/ddsi/ddsi_xmsg.h"
#endif

// 判断两个默认序列化类型是否相等
static bool sertype_default_equal(const struct ddsi_sertype *acmn, const struct ddsi_sertype *bcmn)
{
  // 类型转换为默认序列化类型
  const struct dds_sertype_default *a = (struct dds_sertype_default *)acmn;
  const struct dds_sertype_default *b = (struct dds_sertype_default *)bcmn;
  // 比较编码格式是否相同
  if (a->encoding_format != b->encoding_format)
    return false;
  // 比较类型大小是否相同
  if (a->type.size != b->type.size)
    return false;
  // 比较类型对齐方式是否相同
  if (a->type.align != b->type.align)
    return false;
  // 比较类型标志集是否相同
  if (a->type.flagset != b->type.flagset)
    return false;
  // 比较键值数量是否相同
  if (a->type.keys.nkeys != b->type.keys.nkeys)
    return false;
  // 如果键值数量大于0，比较键值数组是否相同
  if (
      (a->type.keys.nkeys > 0) &&
      memcmp(a->type.keys.keys, b->type.keys.keys, a->type.keys.nkeys * sizeof(*a->type.keys.keys)) != 0)
    return false;
  // 比较操作数量是否相同
  if (a->type.ops.nops != b->type.ops.nops)
    return false;
  // 如果操作数量大于0，比较操作数组是否相同
  if (
      (a->type.ops.nops > 0) &&
      memcmp(a->type.ops.ops, b->type.ops.ops, a->type.ops.nops * sizeof(*a->type.ops.ops)) != 0)
    return false;
  // 断言检查可选的序列化大小是否相同
  assert(a->type.opt_size_xcdr1 == b->type.opt_size_xcdr1);
  assert(a->type.opt_size_xcdr2 == b->type.opt_size_xcdr2);
  // 如果所有条件都满足，返回 true
  return true;
}

#ifdef DDS_HAS_TYPE_DISCOVERY

// 获取默认序列化类型的类型ID
static ddsi_typeid_t *sertype_default_typeid(const struct ddsi_sertype *tpcmn, ddsi_typeid_kind_t kind)
{
  // 断言检查输入参数
  assert(tpcmn);
  assert(kind == DDSI_TYPEID_KIND_MINIMAL || kind == DDSI_TYPEID_KIND_COMPLETE);
  // 类型转换为默认序列化类型
  const struct dds_sertype_default *tp = (struct dds_sertype_default *)tpcmn;
  // 反序列化类型信息
  ddsi_typeinfo_t *type_info = ddsi_typeinfo_deser(tp->typeinfo_ser.data, tp->typeinfo_ser.sz);
  if (type_info == NULL)
    return NULL;
  // 获取类型ID
  ddsi_typeid_t *type_id = ddsi_typeinfo_typeid(type_info, kind);
  // 释放类型信息内存
  ddsi_typeinfo_fini(type_info);
  ddsrt_free(type_info);
  // 返回类型ID
  return type_id;
}

// 获取默认序列化类型的类型映射
static ddsi_typemap_t *sertype_default_typemap(const struct ddsi_sertype *tpcmn)
{
  // 断言检查输入参数
  assert(tpcmn);
  // 类型转换为默认序列化类型
  const struct dds_sertype_default *tp = (struct dds_sertype_default *)tpcmn;
  // 反序列化类型映射
  return ddsi_typemap_deser(tp->typemap_ser.data, tp->typemap_ser.sz);
}

// 获取默认序列化类型的类型信息
static ddsi_typeinfo_t *sertype_default_typeinfo(const struct ddsi_sertype *tpcmn)
{
  // 断言检查输入参数
  assert(tpcmn);
  // 类型转换为默认序列化类型
  const struct dds_sertype_default *tp = (struct dds_sertype_default *)tpcmn;
  // 反序列化类型信息
  return ddsi_typeinfo_deser(tp->typeinfo_ser.data, tp->typeinfo_ser.sz);
}

#endif /* DDS_HAS_TYPE_DISCOVERY */
// 定义一个静态函数sertype_default_hash，计算给定结构体的哈希值
static uint32_t sertype_default_hash(const struct ddsi_sertype *tpcmn)
{
  // 断言传入的结构体指针不为空
  assert(tpcmn);
  // 将通用结构体指针转换为特定类型的结构体指针
  const struct dds_sertype_default *tp = (struct dds_sertype_default *)tpcmn;
  // 定义一个16字节的缓冲区
  unsigned char buf[16];
  // 定义一个MD5状态变量
  ddsrt_md5_state_t md5st;
  // 初始化MD5状态
  ddsrt_md5_init(&md5st);
  // 向MD5状态中添加类型名
  ddsrt_md5_append(&md5st, (ddsrt_md5_byte_t *)tp->c.type_name, (uint32_t)strlen(tp->c.type_name));
  // 向MD5状态中添加编码格式
  ddsrt_md5_append(&md5st, (ddsrt_md5_byte_t *)&tp->encoding_format, sizeof(tp->encoding_format));
  // 向MD5状态中添加类型大小
  ddsrt_md5_append(&md5st, (ddsrt_md5_byte_t *)&tp->type.size, sizeof(tp->type.size));
  // 向MD5状态中添加类型对齐
  ddsrt_md5_append(&md5st, (ddsrt_md5_byte_t *)&tp->type.align, sizeof(tp->type.align));
  // 向MD5状态中添加类型标志集
  ddsrt_md5_append(&md5st, (ddsrt_md5_byte_t *)&tp->type.flagset, sizeof(tp->type.flagset));
  // 向MD5状态中添加类型键值
  ddsrt_md5_append(&md5st, (ddsrt_md5_byte_t *)tp->type.keys.keys, (uint32_t)(tp->type.keys.nkeys * sizeof(*tp->type.keys.keys)));
  // 向MD5状态中添加类型操作
  ddsrt_md5_append(&md5st, (ddsrt_md5_byte_t *)tp->type.ops.ops, (uint32_t)(tp->type.ops.nops * sizeof(*tp->type.ops.ops)));
  // 完成MD5计算并将结果存储在缓冲区中
  ddsrt_md5_finish(&md5st, (ddsrt_md5_byte_t *)buf);
  // 返回缓冲区中的哈希值
  return *(uint32_t *)buf;
}

// 定义一个静态函数sertype_default_free，释放给定结构体占用的内存
static void sertype_default_free(struct ddsi_sertype *tpcmn)
{
  // 将通用结构体指针转换为特定类型的结构体指针
  struct dds_sertype_default *tp = (struct dds_sertype_default *)tpcmn;
  // 释放类型键值所占用的内存
  ddsrt_free(tp->type.keys.keys);
  // 释放类型操作所占用的内存
  ddsrt_free(tp->type.ops.ops);
  // 如果类型信息序列化数据不为空，则释放其内存
  if (tp->typeinfo_ser.data != NULL)
    ddsrt_free(tp->typeinfo_ser.data);
  // 如果类型映射序列化数据不为空，则释放其内存
  if (tp->typemap_ser.data != NULL)
    ddsrt_free(tp->typemap_ser.data);
  // 调用ddsi_sertype_fini函数完成结构体的清理工作
  ddsi_sertype_fini(&tp->c);
  // 释放结构体所占用的内存
  ddsrt_free(tp);
}

// 定义一个静态函数sertype_default_zero_samples，将给定样本的值设置为0
static void sertype_default_zero_samples(const struct ddsi_sertype *sertype_common, void *sample, size_t count)
{
  // 将通用结构体指针转换为特定类型的结构体指针
  const struct dds_sertype_default *tp = (const struct dds_sertype_default *)sertype_common;
  // 使用memset函数将样本的值设置为0
  memset(sample, 0, tp->type.size * count);
}

// 定义一个静态函数sertype_default_realloc_samples，重新分配样本内存并更新指针数组
static void sertype_default_realloc_samples(void **ptrs, const struct ddsi_sertype *sertype_common, void *old, size_t oldcount, size_t count)
{
  // 将通用结构体指针转换为特定类型的结构体指针
  const struct dds_sertype_default *tp = (const struct dds_sertype_default *)sertype_common;
  // 计算类型的大小
  const size_t size = tp->type.size;
  // 根据新旧样本数量重新分配内存
  char *new = (oldcount == count) ? old : dds_realloc(old, size * count);
  // 如果新内存分配成功且新样本数量大于旧样本数量，将新增加的部分设置为0
  if (new &&count > oldcount)
    memset(new + size *oldcount, 0, size * (count - oldcount));
  // 更新指针数组
  for (size_t i = 0; i < count; i++)
  {
    void *ptr = (char *)new + i *size;
    ptrs[i] = ptr;
  }
}
// 定义一个静态函数type_may_contain_ptr，判断给定类型是否可能包含指针
static bool type_may_contain_ptr(const struct dds_sertype_default *tp)
{
  /* 如果XCDR1和XCDR2的优化大小都为0（未优化），
     类型中可能存在指针，free_samples需要对每个样本执行完整的dds_stream_free_sample。 */
  /* TODO: 通过检查序列化操作来改进此检查，以便在类型确实包含指针时返回true */
  return tp->type.opt_size_xcdr1 == 0 || tp->type.opt_size_xcdr2 == 0;
}

// 定义一个静态函数sertype_default_free_samples，释放给定样本占用的内存
static void sertype_default_free_samples(const struct ddsi_sertype *sertype_common, void **ptrs, size_t count, dds_free_op_t op)
{
  if (count > 0)
  {
    // 将通用结构体指针转换为特定类型的结构体指针
    const struct dds_sertype_default *tp = (const struct dds_sertype_default *)sertype_common;
    const struct dds_cdrstream_desc *type = &tp->type;
    const size_t size = type->size;
#ifndef NDEBUG
    for (size_t i = 0, off = 0; i < count; i++, off += size)
      assert((char *)ptrs[i] == (char *)ptrs[0] + off);
#endif
    // 如果类型可能包含指针，则释放样本内存
    if (type_may_contain_ptr(tp))
    {
      char *ptr = ptrs[0];
      for (size_t i = 0; i < count; i++)
      {
        dds_stream_free_sample(ptr, &dds_cdrstream_default_allocator, type->ops.ops);
        ptr += size;
      }
    }
    // 如果操作标志位包含DDS_FREE_ALL_BIT，则释放内存
    if (op & DDS_FREE_ALL_BIT)
    {
      dds_free(ptrs[0]);
    }
  }
}

// 定义一个静态函数sertype_default_derive_sertype，根据基本类型和数据表示形式派生新的类型
static struct ddsi_sertype *sertype_default_derive_sertype(const struct ddsi_sertype *base_sertype, dds_data_representation_id_t data_representation, dds_type_consistency_enforcement_qospolicy_t tce_qos)
{
  const struct dds_sertype_default *base_sertype_default = (const struct dds_sertype_default *)base_sertype;
  struct dds_sertype_default *derived_sertype = NULL;
  const struct ddsi_serdata_ops *required_ops;

  assert(base_sertype);

  // FIXME: 使用类型一致性强制QoS策略中的选项实现(反)序列化器
  (void)tce_qos;

  // 根据数据表示形式选择相应的操作集
  if (data_representation == DDS_DATA_REPRESENTATION_XCDR1)
    required_ops = base_sertype->typekind_no_key ? &dds_serdata_ops_cdr_nokey : &dds_serdata_ops_cdr;
  else if (data_representation == DDS_DATA_REPRESENTATION_XCDR2)
    required_ops = base_sertype->typekind_no_key ? &dds_serdata_ops_xcdr2_nokey : &dds_serdata_ops_xcdr2;
  else
    abort();

  // 如果基本类型的操作集与所需操作集相同，则使用基本类型作为派生类型
  if (base_sertype->serdata_ops == required_ops)
    derived_sertype = (struct dds_sertype_default *)base_sertype_default;
  else
  {
    // 否则，复制基本类型并修改相应的属性
    derived_sertype = ddsrt_memdup(base_sertype_default, sizeof(*derived_sertype));
    uint32_t refc = ddsrt_atomic_ld32(&derived_sertype->c.flags_refc);
    ddsrt_atomic_st32(&derived_sertype->c.flags_refc, refc & ~DDSI_SERTYPE_REFC_MASK);
    derived_sertype->c.base_sertype = ddsi_sertype_ref(base_sertype);
    derived_sertype->c.serdata_ops = required_ops;
    derived_sertype->write_encoding_version = data_representation == DDS_DATA_REPRESENTATION_XCDR1 ? DDSI_RTPS_CDR_ENC_VERSION_1 : DDSI_RTPS_CDR_ENC_VERSION_2;
  }

  return (struct ddsi_sertype *)derived_sertype;
}
// 移动到 cdr_stream？
// 按照给定的缓冲区、大小和写入编码版本创建一个 dds_ostream_t 结构体实例
static dds_ostream_t ostream_from_buffer(void *buffer, size_t size, uint16_t write_encoding_version)
{
  dds_ostream_t os;                           // 定义一个 dds_ostream_t 结构体变量 os
  os.m_buffer = buffer;                       // 将传入的 buffer 赋值给 os 的 m_buffer 成员
  os.m_size = (uint32_t)size;                 // 将传入的 size 转换为 uint32_t 类型并赋值给 os 的 m_size 成员
  os.m_index = 0;                             // 初始化 os 的 m_index 成员为 0
  os.m_xcdr_version = write_encoding_version; // 将传入的 write_encoding_version 赋值给 os 的 m_xcdr_version 成员
  return os;                                  // 返回初始化完成的 os 结构体实例
}

// 占位符实现
// TODO: 高效实现（我们现在实际上是序列化以获得大小）
//       这类似于序列化，但是计算字节数而不是将数据写入流。
//       这应该是（几乎...）O(1)，可能会有一些问题，
//       如非平凡类型的序列，它将取决于元素的数量。
static size_t sertype_default_get_serialized_size(const struct ddsi_sertype *type, const void *sample)
{
  // 我们在这里不计算 CDR 头部。
  // TODO 我们是否希望将 CDR 头部包含到 iceoryx 使用的序列化中？
  //      如果字节序不变，则似乎没有必要（也许对于 XTypes）
  struct ddsi_serdata *serdata = ddsi_serdata_from_sample(type, SDK_DATA, sample);     // 从样本创建 serdata
  size_t serialized_size = ddsi_serdata_size(serdata) - sizeof(struct dds_cdr_header); // 计算序列化大小，减去 CDR 头部的大小
  ddsi_serdata_unref(serdata);                                                         // 取消对 serdata 的引用
  return serialized_size;                                                              // 返回序列化大小
}

// 将给定类型和样本序列化到目标缓冲区
static bool sertype_default_serialize_into(const struct ddsi_sertype *type, const void *sample, void *dst_buffer, size_t dst_size)
{
  const struct dds_sertype_default *type_default = (const struct dds_sertype_default *)type;          // 类型转换为 dds_sertype_default 结构体指针
  dds_ostream_t os = ostream_from_buffer(dst_buffer, dst_size, type_default->write_encoding_version); // 使用目标缓冲区、大小和编码版本创建一个 dds_ostream_t 实例
  // 将样本写入输出流，并返回操作是否成功
  return dds_stream_write_sample(&os, &dds_cdrstream_default_allocator, sample, &type_default->type);
}
// 定义默认的ddsi_sertype_ops结构体实例
const struct ddsi_sertype_ops dds_sertype_ops_default = {
    .version = ddsi_sertype_v0,                         // 设置版本为ddsi_sertype_v0
    .arg = 0,                                           // 初始化arg为0
    .equal = sertype_default_equal,                     // 设置equal函数为sertype_default_equal
    .hash = sertype_default_hash,                       // 设置hash函数为sertype_default_hash
    .free = sertype_default_free,                       // 设置free函数为sertype_default_free
    .zero_samples = sertype_default_zero_samples,       // 设置zero_samples函数为sertype_default_zero_samples
    .realloc_samples = sertype_default_realloc_samples, // 设置realloc_samples函数为sertype_default_realloc_samples
    .free_samples = sertype_default_free_samples,       // 设置free_samples函数为sertype_default_free_samples
#ifdef DDS_HAS_TYPE_DISCOVERY
    .type_id = sertype_default_typeid,     // 设置type_id函数为sertype_default_typeid
    .type_map = sertype_default_typemap,   // 设置type_map函数为sertype_default_typemap
    .type_info = sertype_default_typeinfo, // 设置type_info函数为sertype_default_typeinfo
#else
    .type_id = 0,   // 初始化type_id为0
    .type_map = 0,  // 初始化type_map为0
    .type_info = 0, // 初始化type_info为0
#endif
    .derive_sertype = sertype_default_derive_sertype,           // 设置derive_sertype函数为sertype_default_derive_sertype
    .get_serialized_size = sertype_default_get_serialized_size, // 设置get_serialized_size函数为sertype_default_get_serialized_size
    .serialize_into = sertype_default_serialize_into            // 设置serialize_into函数为sertype_default_serialize_into
};

// 初始化默认的dds_sertype结构体实例
dds_return_t dds_sertype_default_init(const struct dds_domain *domain, struct dds_sertype_default *st, const dds_topic_descriptor_t *desc, uint16_t min_xcdrv, dds_data_representation_id_t data_representation)
{
  const struct ddsi_domaingv *gv = &domain->gv; // 获取domain的domaingv
  const struct ddsi_serdata_ops *serdata_ops;
  // 根据data_representation选择对应的serdata_ops
  switch (data_representation)
  {
  case DDS_DATA_REPRESENTATION_XCDR1:
    serdata_ops = desc->m_nkeys ? &dds_serdata_ops_cdr : &dds_serdata_ops_cdr_nokey;
    break;
  case DDS_DATA_REPRESENTATION_XCDR2:
    serdata_ops = desc->m_nkeys ? &dds_serdata_ops_xcdr2 : &dds_serdata_ops_xcdr2_nokey;
    break;
  default:
    abort();
  }

  // 获取最外层对象类型的可扩展性
  enum dds_cdr_type_extensibility type_ext;
  if (!dds_stream_extensibility(desc->m_ops, &type_ext))
    return DDS_RETCODE_BAD_PARAMETER;

  // 初始化ddsi_sertype结构体
  ddsi_sertype_init(&st->c, desc->m_typename, &dds_sertype_ops_default, serdata_ops, (desc->m_nkeys == 0));
#ifdef DDS_HAS_SHM
  st->c.iox_size = desc->m_size; // 设置iox_size为desc的m_size
#endif
  st->c.fixed_size = (st->c.fixed_size || (desc->m_flagset & DDS_TOPIC_FIXED_SIZE)) ? 1u : 0u;                                                                                  // 设置fixed_size
  st->c.allowed_data_representation = desc->m_flagset & DDS_TOPIC_RESTRICT_DATA_REPRESENTATION ? desc->restrict_data_representation : DDS_DATA_REPRESENTATION_RESTRICT_DEFAULT; // 设置allowed_data_representation
  if (min_xcdrv == DDSI_RTPS_CDR_ENC_VERSION_2)
    st->c.allowed_data_representation &= ~DDS_DATA_REPRESENTATION_FLAG_XCDR1;
  st->encoding_format = ddsi_sertype_extensibility_enc_format(type_ext); // 设置encoding_format
  // 设置写入数据时使用的编码版本
  st->write_encoding_version = data_representation == DDS_DATA_REPRESENTATION_XCDR1 ? DDSI_RTPS_CDR_ENC_VERSION_1 : DDSI_RTPS_CDR_ENC_VERSION_2;
  st->serpool = domain->serpool;                                                        // 设置serpool为domain的serpool
  st->type.size = desc->m_size;                                                         // 设置type的size为desc的m_size
  st->type.align = desc->m_align;                                                       // 设置type的align为desc的m_align
  st->type.flagset = desc->m_flagset;                                                   // 设置type的flagset为desc的m_flagset
  st->type.keys.nkeys = desc->m_nkeys;                                                  // 设置type的keys的nkeys为desc的m_nkeys
  st->type.keys.keys = ddsrt_malloc(st->type.keys.nkeys * sizeof(*st->type.keys.keys)); // 分配内存给type的keys的keys
  for (uint32_t i = 0; i < st->type.keys.nkeys; i++)
  {
    st->type.keys.keys[i].ops_offs = desc->m_keys[i].m_offset; // 设置type的keys的keys的ops_offs
    st->type.keys.keys[i].idx = desc->m_keys[i].m_idx;         // 设置type的keys的keys的idx
  }
  st->type.ops.nops = dds_stream_countops(desc->m_ops, desc->m_nkeys, desc->m_keys);           // 设置type的ops的nops
  st->type.ops.ops = ddsrt_memdup(desc->m_ops, st->type.ops.nops * sizeof(*st->type.ops.ops)); // 分配内存并复制desc的m_ops给type的ops的ops

  // 检查嵌套深度是否超过最大值
  if (min_xcdrv == DDSI_RTPS_CDR_ENC_VERSION_2 && dds_stream_type_nesting_depth(desc->m_ops) > DDS_CDRSTREAM_MAX_NESTING_DEPTH)
  {
    ddsi_sertype_unref(&st->c);
    GVTRACE("Serializer ops for type %s has unsupported nesting depth (max %u)\n", desc->m_typename, DDS_CDRSTREAM_MAX_NESTING_DEPTH);
    return DDS_RETCODE_BAD_PARAMETER;
  }

  // 处理DDS_TOPIC_XTYPES_METADATA标志
  if (desc->m_flagset & DDS_TOPIC_XTYPES_METADATA)
  {
    if (desc->type_information.sz == 0 || desc->type_information.data == NULL || desc->type_mapping.sz == 0 || desc->type_mapping.data == NULL)
    {
      ddsi_sertype_unref(&st->c);
      GVTRACE("Flag DDS_TOPIC_XTYPES_METADATA set for type %s but topic descriptor does not contains type information\n", desc->m_typename);
      return DDS_RETCODE_BAD_PARAMETER;
    }
    st->typeinfo_ser.data = ddsrt_memdup(desc->type_information.data, desc->type_information.sz); // 分配内存并复制desc的type_information给st的typeinfo_ser
    st->typeinfo_ser.sz = desc->type_information.sz;                                              // 设置st的typeinfo_ser的sz为desc的type_information的sz
    st->typemap_ser.data = ddsrt_memdup(desc->type_mapping.data, desc->type_mapping.sz);          // 分配内存并复制desc的type_mapping给st的typemap_ser
    st->typemap_ser.sz = desc->type_mapping.sz;                                                   // 设置st的typemap_ser的sz为desc的type_mapping的sz
  }
  else
  {
    st->typeinfo_ser.data = NULL; // 初始化st的typeinfo_ser的data为NULL
    st->typeinfo_ser.sz = 0;      // 初始化st的typeinfo_ser的sz为0
    st->typemap_ser.data = NULL;  // 初始化st的typemap_ser的data为NULL
    st->typemap_ser.sz = 0;       // 初始化st的typemap_ser的sz为0
  }

  // 检查XCDR1和XCDR2编码是否可以优化
  st->type.opt_size_xcdr1 = (st->c.allowed_data_representation & DDS_DATA_REPRESENTATION_FLAG_XCDR1) ? dds_stream_check_optimize(&st->type, DDSI_RTPS_CDR_ENC_VERSION_1) : 0;
  if (st->type.opt_size_xcdr1 > 0)
    GVTRACE("Marshalling XCDR1 for type: %s is %soptimised\n", st->c.type_name, st->type.opt_size_xcdr1 ? "" : "not ");

  st->type.opt_size_xcdr2 = (st->c.allowed_data_representation & DDS_DATA_REPRESENTATION_FLAG_XCDR2) ? dds_stream_check_optimize(&st->type, DDSI_RTPS_CDR_ENC_VERSION_2) : 0;
  if (st->type.opt_size_xcdr2 > 0)
    GVTRACE("Marshalling XCDR2 for type: %s is %soptimised\n", st->c.type_name, st->type.opt_size_xcdr2 ? "" : "not ");

  return DDS_RETCODE_OK;
}
