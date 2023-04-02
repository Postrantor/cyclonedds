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

/**
 * @brief 比较两个ddsi_sertype结构体是否相等
 *
 * @param acmn 第一个ddsi_sertype结构体指针
 * @param bcmn 第二个ddsi_sertype结构体指针
 * @return 如果两个结构体相等，返回true；否则返回false
 */
static bool sertype_default_equal(const struct ddsi_sertype *acmn,
                                  const struct ddsi_sertype *bcmn) {
  // 将acmn和bcmn转换为dds_sertype_default类型的指针
  const struct dds_sertype_default *a = (struct dds_sertype_default *)acmn;
  const struct dds_sertype_default *b = (struct dds_sertype_default *)bcmn;

  // 比较编码格式是否相同
  if (a->encoding_format != b->encoding_format) return false;

  // 比较类型大小是否相同
  if (a->type.size != b->type.size) return false;

  // 比较类型对齐方式是否相同
  if (a->type.align != b->type.align) return false;

  // 比较类型标志集是否相同
  if (a->type.flagset != b->type.flagset) return false;

  // 比较键值数量是否相同
  if (a->type.keys.nkeys != b->type.keys.nkeys) return false;

  // 如果键值数量大于0，比较键值数组是否相同
  if ((a->type.keys.nkeys > 0) && memcmp(a->type.keys.keys, b->type.keys.keys,
                                         a->type.keys.nkeys * sizeof(*a->type.keys.keys)) != 0)
    return false;

  // 比较操作数量是否相同
  if (a->type.ops.nops != b->type.ops.nops) return false;

  // 如果操作数量大于0，比较操作数组是否相同
  if ((a->type.ops.nops > 0) &&
      memcmp(a->type.ops.ops, b->type.ops.ops, a->type.ops.nops * sizeof(*a->type.ops.ops)) != 0)
    return false;

  // 断言两个结构体的opt_size_xcdr1属性相等
  assert(a->type.opt_size_xcdr1 == b->type.opt_size_xcdr1);

  // 断言两个结构体的opt_size_xcdr2属性相等
  assert(a->type.opt_size_xcdr2 == b->type.opt_size_xcdr2);

  // 如果所有条件都满足，返回true
  return true;
}

#ifdef DDS_HAS_TYPE_DISCOVERY

/**
 * @brief 为给定的类型生成默认的类型标识符
 * @param[in] tpcmn 类型公共数据结构指针
 * @param[in] kind 类型标识符的种类（最小或完整）
 * @return 返回类型标识符指针，如果失败则返回NULL
 */
static ddsi_typeid_t *sertype_default_typeid(const struct ddsi_sertype *tpcmn,
                                             ddsi_typeid_kind_t kind) {
  assert(tpcmn);                              // 断言：tpcmn不为空
  assert(kind == DDSI_TYPEID_KIND_MINIMAL ||
         kind == DDSI_TYPEID_KIND_COMPLETE);  // 断言：kind是最小或完整类型
  const struct dds_sertype_default *tp = (struct dds_sertype_default *)tpcmn;  // 类型转换
  ddsi_typeinfo_t *type_info =
      ddsi_typeinfo_deser(tp->typeinfo_ser.data, tp->typeinfo_ser.sz);  // 反序列化类型信息
  if (type_info == NULL)                                                // 如果类型信息为空
    return NULL;
  ddsi_typeid_t *type_id = ddsi_typeinfo_typeid(type_info, kind);       // 获取类型标识符
  ddsi_typeinfo_fini(type_info);                                        // 清理类型信息
  ddsrt_free(type_info);                                                // 释放类型信息内存
  return type_id;                                                       // 返回类型标识符
}

/**
 * @brief 为给定的类型生成默认的类型映射
 * @param[in] tpcmn 类型公共数据结构指针
 * @return 返回类型映射指针
 */
static ddsi_typemap_t *sertype_default_typemap(const struct ddsi_sertype *tpcmn) {
  assert(tpcmn);  // 断言：tpcmn不为空
  const struct dds_sertype_default *tp = (struct dds_sertype_default *)tpcmn;  // 类型转换
  return ddsi_typemap_deser(tp->typemap_ser.data, tp->typemap_ser.sz);  // 反序列化类型映射并返回
}

/**
 * @brief 为给定的类型生成默认的类型信息
 * @param[in] tpcmn 类型公共数据结构指针
 * @return 返回类型信息指针，如果失败则返回NULL
 */
static ddsi_typeinfo_t *sertype_default_typeinfo(const struct ddsi_sertype *tpcmn) {
  assert(tpcmn);  // 断言：tpcmn不为空
  const struct dds_sertype_default *tp = (struct dds_sertype_default *)tpcmn;  // 类型转换
  return ddsi_typeinfo_deser(tp->typeinfo_ser.data, tp->typeinfo_ser.sz);  // 反序列化类型信息并返回
}

#endif /* DDS_HAS_TYPE_DISCOVERY */

/**
 * @brief 计算序列化类型的哈希值
 *
 * @param[in] tpcmn 序列化类型的通用结构体指针
 * @return 返回计算得到的哈希值
 */
static uint32_t sertype_default_hash(const struct ddsi_sertype *tpcmn) {
  // 断言：确保传入的指针不为空
  assert(tpcmn);

  // 类型转换：将通用结构体指针转换为默认序列化类型指针
  const struct dds_sertype_default *tp = (struct dds_sertype_default *)tpcmn;

  // 定义一个16字节的缓冲区，用于存储计算结果
  unsigned char buf[16];

  // 初始化MD5状态结构体
  ddsrt_md5_state_t md5st;
  ddsrt_md5_init(&md5st);

  // 添加类型名到MD5计算中
  ddsrt_md5_append(&md5st, (ddsrt_md5_byte_t *)tp->c.type_name, (uint32_t)strlen(tp->c.type_name));

  // 添加编码格式到MD5计算中
  ddsrt_md5_append(&md5st, (ddsrt_md5_byte_t *)&tp->encoding_format, sizeof(tp->encoding_format));

  // 添加类型大小到MD5计算中
  ddsrt_md5_append(&md5st, (ddsrt_md5_byte_t *)&tp->type.size, sizeof(tp->type.size));

  // 添加类型对齐到MD5计算中
  ddsrt_md5_append(&md5st, (ddsrt_md5_byte_t *)&tp->type.align, sizeof(tp->type.align));

  // 添加类型标志集到MD5计算中
  ddsrt_md5_append(&md5st, (ddsrt_md5_byte_t *)&tp->type.flagset, sizeof(tp->type.flagset));

  // 添加类型键值到MD5计算中
  ddsrt_md5_append(&md5st, (ddsrt_md5_byte_t *)tp->type.keys.keys,
                   (uint32_t)(tp->type.keys.nkeys * sizeof(*tp->type.keys.keys)));

  // 添加类型操作到MD5计算中
  ddsrt_md5_append(&md5st, (ddsrt_md5_byte_t *)tp->type.ops.ops,
                   (uint32_t)(tp->type.ops.nops * sizeof(*tp->type.ops.ops)));

  // 完成MD5计算并将结果存储在缓冲区中
  ddsrt_md5_finish(&md5st, (ddsrt_md5_byte_t *)buf);

  // 返回计算得到的哈希值
  return *(uint32_t *)buf;
}

/**
 * @brief 释放默认序列化类型所占用的内存
 *
 * @param[in] tpcmn 序列化类型的通用结构体指针
 */
static void sertype_default_free(struct ddsi_sertype *tpcmn) {
  // 类型转换：将通用结构体指针转换为默认序列化类型指针
  struct dds_sertype_default *tp = (struct dds_sertype_default *)tpcmn;

  // 释放类型键值所占用的内存
  ddsrt_free(tp->type.keys.keys);

  // 释放类型操作所占用的内存
  ddsrt_free(tp->type.ops.ops);

  // 如果类型信息序列化数据不为空，则释放其内存
  if (tp->typeinfo_ser.data != NULL) ddsrt_free(tp->typeinfo_ser.data);

  // 如果类型映射序列化数据不为空，则释放其内存
  if (tp->typemap_ser.data != NULL) ddsrt_free(tp->typemap_ser.data);

  // 结束序列化类型并释放其内存
  ddsi_sertype_fini(&tp->c);
  ddsrt_free(tp);
}

/**
 * @brief 将指定数量的样本置零
 *
 * @param[in] sertype_common 序列化类型的通用结构体指针
 * @param[out] sample 样本指针
 * @param[in] count 要置零的样本数量
 */
static void sertype_default_zero_samples(const struct ddsi_sertype *sertype_common,
                                         void *sample,
                                         size_t count) {
  // 类型转换：将通用结构体指针转换为默认序列化类型指针
  const struct dds_sertype_default *tp = (const struct dds_sertype_default *)sertype_common;

  // 将指定数量的样本置零
  memset(sample, 0, tp->type.size * count);
}

/**
 * @brief 重新分配样本内存并初始化指针数组。
 *
 * @param[out] ptrs          指向样本指针数组的指针
 * @param[in]  sertype_common 序列化类型通用结构体指针
 * @param[in]  old           原始内存块指针
 * @param[in]  oldcount      原始内存块中的样本数量
 * @param[in]  count         新内存块中的样本数量
 */
static void sertype_default_realloc_samples(void **ptrs,
                                            const struct ddsi_sertype *sertype_common,
                                            void *old,
                                            size_t oldcount,
                                            size_t count) {
  // 将通用序列化类型结构体转换为默认序列化类型结构体
  const struct dds_sertype_default *tp = (const struct dds_sertype_default *)sertype_common;

  // 获取类型大小
  const size_t size = tp->type.size;

  // 根据新旧样本数量决定是否需要重新分配内存
  char *new = (oldcount == count) ? old : dds_realloc(old, size * count);

  // 如果新内存块存在且新样本数量大于旧样本数量，将新增加的部分置零
  if (new &&count > oldcount) memset(new + size *oldcount, 0, size * (count - oldcount));

  // 遍历新内存块，设置样本指针数组
  for (size_t i = 0; i < count; i++) {
    void *ptr = (char *)new + i *size;
    ptrs[i] = ptr;
  }
}

/**
 * @brief 判断类型是否可能包含指针。
 *
 * @param[in] tp 默认序列化类型结构体指针
 * @return 如果类型可能包含指针，返回 true；否则返回 false。
 */
static bool type_may_contain_ptr(const struct dds_sertype_default *tp) {
  /* 如果 XCDR1 和 XCDR2 的优化大小都为 0（未优化），
     类型中可能存在指针，free_samples 需要对每个样本执行完整的 dds_stream_free_sample。 */
  /* TODO: 通过检查序列化操作来改进此检查，以便在类型确实包含指针时才返回 true */
  return tp->type.opt_size_xcdr1 == 0 || tp->type.opt_size_xcdr2 == 0;
}

/**
 * @brief 释放指定数量的样本内存。
 *
 * @param[in] sertype_common      指向ddsi_sertype结构体的指针。
 * @param[in] ptrs                指向要释放的样本内存的指针数组。
 * @param[in] count               要释放的样本数量。
 * @param[in] op                  指定释放操作类型（dds_free_op_t）。
 */
static void sertype_default_free_samples(const struct ddsi_sertype *sertype_common,
                                         void **ptrs,
                                         size_t count,
                                         dds_free_op_t op) {
  // 如果要释放的样本数量大于0
  if (count > 0) {
    // 将sertype_common转换为dds_sertype_default类型的指针
    const struct dds_sertype_default *tp = (const struct dds_sertype_default *)sertype_common;
    // 获取类型描述信息
    const struct dds_cdrstream_desc *type = &tp->type;
    // 获取类型大小
    const size_t size = type->size;

    // 如果编译时未定义NDEBUG，则进行断言检查
#ifndef NDEBUG
    for (size_t i = 0, off = 0; i < count; i++, off += size)
      assert((char *)ptrs[i] == (char *)ptrs[0] + off);
#endif

    // 如果类型可能包含指针
    if (type_may_contain_ptr(tp)) {
      char *ptr = ptrs[0];
      // 遍历所有样本，释放内存
      for (size_t i = 0; i < count; i++) {
        dds_stream_free_sample(ptr, &dds_cdrstream_default_allocator, type->ops.ops);
        ptr += size;
      }
    }

    // 如果操作类型包含DDS_FREE_ALL_BIT，则释放所有内存
    if (op & DDS_FREE_ALL_BIT) {
      dds_free(ptrs[0]);
    }
  }
}

/**
 * @brief 根据基本的ddsi_sertype结构体派生新的ddsi_sertype结构体。
 *
 * @param[in] base_sertype        指向基本ddsi_sertype结构体的指针。
 * @param[in] data_representation 数据表示形式（dds_data_representation_id_t）。
 * @param[in] tce_qos 类型一致性强制QoS策略（dds_type_consistency_enforcement_qospolicy_t）。
 * @return 返回派生的ddsi_sertype结构体指针。
 */
static struct ddsi_sertype *sertype_default_derive_sertype(
    const struct ddsi_sertype *base_sertype,
    dds_data_representation_id_t data_representation,
    dds_type_consistency_enforcement_qospolicy_t tce_qos) {
  // 将base_sertype转换为dds_sertype_default类型的指针
  const struct dds_sertype_default *base_sertype_default =
      (const struct dds_sertype_default *)base_sertype;
  // 初始化派生的sertype为NULL
  struct dds_sertype_default *derived_sertype = NULL;
  // 定义所需的操作指针
  const struct ddsi_serdata_ops *required_ops;

  // 断言检查base_sertype是否存在
  assert(base_sertype);

  // FIXME: 使用类型一致性强制QoS策略中的选项实现（反）序列化器
  (void)tce_qos;

  // 根据数据表示形式选择所需的操作
  if (data_representation == DDS_DATA_REPRESENTATION_XCDR1)
    required_ops =
        base_sertype->typekind_no_key ? &dds_serdata_ops_cdr_nokey : &dds_serdata_ops_cdr;
  else if (data_representation == DDS_DATA_REPRESENTATION_XCDR2)
    required_ops =
        base_sertype->typekind_no_key ? &dds_serdata_ops_xcdr2_nokey : &dds_serdata_ops_xcdr2;
  else
    abort();

  // 如果基本sertype的serdata_ops与所需的操作相同，则将派生的sertype设置为基本sertype_default
  if (base_sertype->serdata_ops == required_ops)
    derived_sertype = (struct dds_sertype_default *)base_sertype_default;
  else {
    // 复制基本sertype_default到派生的sertype
    derived_sertype = ddsrt_memdup(base_sertype_default, sizeof(*derived_sertype));
    uint32_t refc = ddsrt_atomic_ld32(&derived_sertype->c.flags_refc);
    ddsrt_atomic_st32(&derived_sertype->c.flags_refc, refc & ~DDSI_SERTYPE_REFC_MASK);
    // 设置派生sertype的基本sertype
    derived_sertype->c.base_sertype = ddsi_sertype_ref(base_sertype);
    // 设置派生sertype的serdata_ops
    derived_sertype->c.serdata_ops = required_ops;
    // 设置派生sertype的写入编码版本
    derived_sertype->write_encoding_version = data_representation == DDS_DATA_REPRESENTATION_XCDR1
                                                  ? DDSI_RTPS_CDR_ENC_VERSION_1
                                                  : DDSI_RTPS_CDR_ENC_VERSION_2;
  }

  // 返回派生的ddsi_sertype结构体指针
  return (struct ddsi_sertype *)derived_sertype;
}

/**
 * @brief 从缓冲区创建输出流对象
 *
 * @param buffer 缓冲区指针
 * @param size 缓冲区大小
 * @param write_encoding_version 写入编码版本
 * @return dds_ostream_t 输出流对象
 */
static dds_ostream_t ostream_from_buffer(void *buffer,
                                         size_t size,
                                         uint16_t write_encoding_version) {
  dds_ostream_t os;
  os.m_buffer = buffer;                        // 设置缓冲区
  os.m_size = (uint32_t)size;                  // 设置缓冲区大小
  os.m_index = 0;                              // 初始化索引
  os.m_xcdr_version = write_encoding_version;  // 设置写入编码版本
  return os;                                   // 返回输出流对象
}

/**
 * @brief 获取序列化后的数据大小（占位符实现）
 *
 * @note 实际上，我们需要实现一个高效的方法来获取序列化后的大小。
 *       这个函数类似于序列化，但是计算字节数而不是将数据写入流。
 *       这应该是（几乎...）O(1)的复杂度，可能会有一些问题，
 *       如非平凡类型的序列，这取决于元素的数量。
 *
 * @param type 序列化类型
 * @param sample 样本数据
 * @return size_t 序列化后的数据大小
 */
static size_t sertype_default_get_serialized_size(const struct ddsi_sertype *type,
                                                  const void *sample) {
  // 我们在这里不计算CDR头部
  // TODO 是否需要将CDR头部包含在iceoryx使用的序列化中？
  //      如果字节序不变，似乎没有必要（也许对于XTypes）
  struct ddsi_serdata *serdata = ddsi_serdata_from_sample(type, SDK_DATA, sample);
  size_t serialized_size = ddsi_serdata_size(serdata) - sizeof(struct dds_cdr_header);
  ddsi_serdata_unref(serdata);
  return serialized_size;
}

/**
 * @brief 将样本数据序列化到目标缓冲区
 *
 * @param type 序列化类型
 * @param sample 样本数据
 * @param dst_buffer 目标缓冲区
 * @param dst_size 目标缓冲区大小
 * @return bool 序列化是否成功
 */
static bool sertype_default_serialize_into(const struct ddsi_sertype *type,
                                           const void *sample,
                                           void *dst_buffer,
                                           size_t dst_size) {
  const struct dds_sertype_default *type_default = (const struct dds_sertype_default *)type;
  dds_ostream_t os =
      ostream_from_buffer(dst_buffer, dst_size, type_default->write_encoding_version);
  return dds_stream_write_sample(&os, &dds_cdrstream_default_allocator, sample,
                                 &type_default->type);
}

// 默认的序列化操作结构体
const struct ddsi_sertype_ops dds_sertype_ops_default = {
    .version = ddsi_sertype_v0,
    .arg = 0,
    .equal = sertype_default_equal,
    .hash = sertype_default_hash,
    .free = sertype_default_free,
    .zero_samples = sertype_default_zero_samples,
    .realloc_samples = sertype_default_realloc_samples,
    .free_samples = sertype_default_free_samples,
#ifdef DDS_HAS_TYPE_DISCOVERY
    .type_id = sertype_default_typeid,
    .type_map = sertype_default_typemap,
    .type_info = sertype_default_typeinfo,
#else
    .type_id = 0,
    .type_map = 0,
    .type_info = 0,
#endif
    .derive_sertype = sertype_default_derive_sertype,
    .get_serialized_size = sertype_default_get_serialized_size,
    .serialize_into = sertype_default_serialize_into};

/**
 * @brief 初始化默认的序列化类型对象
 *
 * 该函数根据提供的主题描述符和数据表示形式初始化一个默认的序列化类型对象。
 *
 * @param[in] domain                DDS 域指针
 * @param[out] st                   默认序列化类型对象指针
 * @param[in] desc                  主题描述符指针
 * @param[in] min_xcdrv             最小 XCDR 版本
 * @param[in] data_representation   数据表示形式
 * @return 返回 dds_return_t 类型的结果，成功返回 DDS_RETCODE_OK
 */
dds_return_t dds_sertype_default_init(const struct dds_domain *domain,
                                      struct dds_sertype_default *st,
                                      const dds_topic_descriptor_t *desc,
                                      uint16_t min_xcdrv,
                                      dds_data_representation_id_t data_representation) {
  // 获取域全局变量指针
  const struct ddsi_domaingv *gv = &domain->gv;
  // 定义序列化数据操作指针
  const struct ddsi_serdata_ops *serdata_ops;
  // 根据数据表示形式选择序列化数据操作
  switch (data_representation) {
    case DDS_DATA_REPRESENTATION_XCDR1:
      serdata_ops = desc->m_nkeys ? &dds_serdata_ops_cdr : &dds_serdata_ops_cdr_nokey;
      break;
    case DDS_DATA_REPRESENTATION_XCDR2:
      serdata_ops = desc->m_nkeys ? &dds_serdata_ops_xcdr2 : &dds_serdata_ops_xcdr2_nokey;
      break;
    default:
      abort();
  }

  // 获取主题类型最外层对象的可扩展性
  enum dds_cdr_type_extensibility type_ext;
  if (!dds_stream_extensibility(desc->m_ops, &type_ext)) return DDS_RETCODE_BAD_PARAMETER;

  // 初始化序列化类型对象
  ddsi_sertype_init(&st->c, desc->m_typename, &dds_sertype_ops_default, serdata_ops,
                    (desc->m_nkeys == 0));
#ifdef DDS_HAS_SHM
  st->c.iox_size = desc->m_size;
#endif
  // 设置固定大小标志
  st->c.fixed_size = (st->c.fixed_size || (desc->m_flagset & DDS_TOPIC_FIXED_SIZE)) ? 1u : 0u;
  // 设置允许的数据表示形式
  st->c.allowed_data_representation = desc->m_flagset & DDS_TOPIC_RESTRICT_DATA_REPRESENTATION
                                          ? desc->restrict_data_representation
                                          : DDS_DATA_REPRESENTATION_RESTRICT_DEFAULT;
  if (min_xcdrv == DDSI_RTPS_CDR_ENC_VERSION_2)
    st->c.allowed_data_representation &= ~DDS_DATA_REPRESENTATION_FLAG_XCDR1;
  // 设置编码格式
  st->encoding_format = ddsi_sertype_extensibility_enc_format(type_ext);
  // 存储用于写入数据的编码版本，读取数据时使用 CDR 中的封装头编码版本
  st->write_encoding_version = data_representation == DDS_DATA_REPRESENTATION_XCDR1
                                   ? DDSI_RTPS_CDR_ENC_VERSION_1
                                   : DDSI_RTPS_CDR_ENC_VERSION_2;
  // 设置序列化池指针
  st->serpool = domain->serpool;
  // 设置类型大小、对齐和标志集
  st->type.size = desc->m_size;
  st->type.align = desc->m_align;
  st->type.flagset = desc->m_flagset;
  // 设置键值数量和键值数组
  st->type.keys.nkeys = desc->m_nkeys;
  st->type.keys.keys = ddsrt_malloc(st->type.keys.nkeys * sizeof(*st->type.keys.keys));
  for (uint32_t i = 0; i < st->type.keys.nkeys; i++) {
    st->type.keys.keys[i].ops_offs = desc->m_keys[i].m_offset;
    st->type.keys.keys[i].idx = desc->m_keys[i].m_idx;
  }
  // 设置操作数量和操作数组
  st->type.ops.nops = dds_stream_countops(desc->m_ops, desc->m_nkeys, desc->m_keys);
  st->type.ops.ops = ddsrt_memdup(desc->m_ops, st->type.ops.nops * sizeof(*st->type.ops.ops));

  // 检查嵌套深度是否超过最大值
  if (min_xcdrv == DDSI_RTPS_CDR_ENC_VERSION_2 &&
      dds_stream_type_nesting_depth(desc->m_ops) > DDS_CDRSTREAM_MAX_NESTING_DEPTH) {
    ddsi_sertype_unref(&st->c);
    GVTRACE("Serializer ops for type %s has unsupported nesting depth (max %u)\n", desc->m_typename,
            DDS_CDRSTREAM_MAX_NESTING_DEPTH);
    return DDS_RETCODE_BAD_PARAMETER;
  }

  // 处理扩展类型元数据
  if (desc->m_flagset & DDS_TOPIC_XTYPES_METADATA) {
    if (desc->type_information.sz == 0 || desc->type_information.data == NULL ||
        desc->type_mapping.sz == 0 || desc->type_mapping.data == NULL) {
      ddsi_sertype_unref(&st->c);
      GVTRACE(
          "Flag DDS_TOPIC_XTYPES_METADATA set for type %s but topic descriptor does not contains "
          "type information\n",
          desc->m_typename);
      return DDS_RETCODE_BAD_PARAMETER;
    }
    st->typeinfo_ser.data = ddsrt_memdup(desc->type_information.data, desc->type_information.sz);
    st->typeinfo_ser.sz = desc->type_information.sz;
    st->typemap_ser.data = ddsrt_memdup(desc->type_mapping.data, desc->type_mapping.sz);
    st->typemap_ser.sz = desc->type_mapping.sz;
  } else {
    st->typeinfo_ser.data = NULL;
    st->typeinfo_ser.sz = 0;
    st->typemap_ser.data = NULL;
    st->typemap_ser.sz = 0;
  }

  // 检查 XCDR1 和 XCDR2 的优化大小
  st->type.opt_size_xcdr1 = (st->c.allowed_data_representation & DDS_DATA_REPRESENTATION_FLAG_XCDR1)
                                ? dds_stream_check_optimize(&st->type, DDSI_RTPS_CDR_ENC_VERSION_1)
                                : 0;
  if (st->type.opt_size_xcdr1 > 0)
    GVTRACE("Marshalling XCDR1 for type: %s is %soptimised\n", st->c.type_name,
            st->type.opt_size_xcdr1 ? "" : "not ");

  st->type.opt_size_xcdr2 = (st->c.allowed_data_representation & DDS_DATA_REPRESENTATION_FLAG_XCDR2)
                                ? dds_stream_check_optimize(&st->type, DDSI_RTPS_CDR_ENC_VERSION_2)
                                : 0;
  if (st->type.opt_size_xcdr2 > 0)
    GVTRACE("Marshalling XCDR2 for type: %s is %soptimised\n", st->c.type_name,
            st->type.opt_size_xcdr2 ? "" : "not ");

  return DDS_RETCODE_OK;
}
