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
#include <stdlib.h>
#include <string.h>

#include "dds/cdr/dds_cdrstream.h"
#include "dds/ddsi/ddsi_domaingv.h"
#include "dds/ddsi/ddsi_freelist.h"
#include "dds/ddsi/ddsi_radmin.h" /* sampleinfo */
#include "dds/ddsi/ddsi_serdata.h"
#include "dds/ddsi/ddsi_tkmap.h"
#include "dds/ddsrt/heap.h"
#include "dds/ddsrt/log.h"
#include "dds/ddsrt/md5.h"
#include "dds/ddsrt/mh3.h"
#include "dds__serdata_default.h"

#ifdef DDS_HAS_SHM
#include "dds/ddsi/ddsi_shm_transport.h"
#include "dds/ddsi/ddsi_xmsg.h"
#include "iceoryx_binding_c/chunk.h"
#endif

// 定义最大池大小为8192，用于限制freelist中的条目数量
#define MAX_POOL_SIZE 8192
// 定义池中最大尺寸为256，用于限制可分配内存块的最大尺寸
#define MAX_SIZE_FOR_POOL 256
// 定义默认新尺寸为128，用于初始化新分配的内存块
#define DEFAULT_NEW_SIZE 128
// 定义块大小为128，用于确定内存块的尺寸
#define CHUNK_SIZE 128

#ifndef NDEBUG
// 检查给定的x是否是2的幂次方
static int ispowerof2_size(size_t x) { return x > 0 && !(x & (x - 1)); }
#endif

// 创建一个新的dds_serdatapool并返回指向它的指针
struct dds_serdatapool *dds_serdatapool_new(void) {
  struct dds_serdatapool *pool;
  // 为dds_serdatapool结构体分配内存
  pool = ddsrt_malloc(sizeof(*pool));
  // 初始化pool的freelist，设置最大池大小和偏移量
  ddsi_freelist_init(&pool->freelist, MAX_POOL_SIZE, offsetof(struct dds_serdata_default, next));
  // 返回创建的dds_serdatapool指针
  return pool;
}

// 释放serdata元素的包装函数
static void serdata_free_wrap(void *elem) {
#ifndef NDEBUG
  struct dds_serdata_default *d = elem;
  // 断言d的引用计数为0
  assert(ddsrt_atomic_ld32(&d->c.refc) == 0);
#endif
  // 释放elem指向的内存
  dds_free(elem);
}

// 释放给定的dds_serdatapool
void dds_serdatapool_free(struct dds_serdatapool *pool) {
  // 清理pool的freelist并使用serdata_free_wrap函数释放元素
  ddsi_freelist_fini(&pool->freelist, serdata_free_wrap);
  // 释放pool指向的内存
  ddsrt_free(pool);
}
// 将给定的大小x向上对齐到最接近的a的倍数
static size_t alignup_size(size_t x, size_t a) {
  // 计算对齐所需的掩码
  size_t m = a - 1;
  // 确保a是2的幂
  assert(ispowerof2_size(a));
  // 对齐x并返回结果
  return (x + m) & ~m;
}

// 向dds_serdata_default结构体追加n个字节，并返回指向新添加数据的指针
static void *serdata_default_append(struct dds_serdata_default **d, size_t n) {
  char *p;
  // 检查是否需要扩展缓冲区
  if ((*d)->pos + n > (*d)->size) {
    // 计算新的缓冲区大小
    size_t size1 = alignup_size((*d)->pos + n, CHUNK_SIZE);
    // 重新分配内存并更新指针
    *d = ddsrt_realloc(*d, offsetof(struct dds_serdata_default, data) + size1);
    // 更新缓冲区大小
    (*d)->size = (uint32_t)size1;
  }
  // 确保足够的空间可用
  assert((*d)->pos + n <= (*d)->size);
  // 获取新数据的位置
  p = (*d)->data + (*d)->pos;
  // 更新当前位置
  (*d)->pos += (uint32_t)n;
  // 返回新数据的指针
  return p;
}

// 将给定的数据追加到dds_serdata_default结构体中
static void serdata_default_append_blob(struct dds_serdata_default **d,
                                        size_t sz,
                                        const void *data) {
  // 获取新数据的位置
  char *p = serdata_default_append(d, sz);
  // 复制数据到新位置
  memcpy(p, data, sz);
}

// 返回dds_serdata_default结构体中的键缓冲区
static const unsigned char *serdata_default_keybuf(const struct dds_serdata_default *d) {
  // 确保键缓冲区类型已设置
  assert(d->key.buftype != KEYBUFTYPE_UNSET);
  // 根据键缓冲区类型返回相应的指针
  return (d->key.buftype == KEYBUFTYPE_STATIC) ? d->key.u.stbuf : d->key.u.dynbuf;
}

// 使用给定的基本哈希值修复dds_serdata_default结构体，并返回指向ddsi_serdata结构体的指针
static struct ddsi_serdata *fix_serdata_default(struct dds_serdata_default *d, uint32_t basehash) {
  // 确保键大小大于0，因为我们对无键情况使用不同的函数实现
  assert(d->key.keysize > 0);
  // 计算哈希值并存储在结构体中
  d->c.hash = ddsrt_mh3(serdata_default_keybuf(d), d->key.keysize,
                        basehash);  // FIXME: 或者使用完整的缓冲区，而不考虑实际大小？
  // 返回指向ddsi_serdata结构体的指针
  return &d->c;
}
// 定义一个静态函数 fix_serdata_default_nokey，用于修复没有 key 的 dds_serdata_default 结构体
static struct ddsi_serdata *fix_serdata_default_nokey(struct dds_serdata_default *d,
                                                      uint32_t basehash) {
  // 设置 d->c.hash 为传入的 basehash 值
  d->c.hash = basehash;
  // 返回 d->c 的地址
  return &d->c;
}

// 定义一个静态函数 serdata_default_get_size，用于获取序列化数据的大小
static uint32_t serdata_default_get_size(const struct ddsi_serdata *dcmn) {
  // 将 dcmn 转换为 dds_serdata_default 类型的指针
  const struct dds_serdata_default *d = (const struct dds_serdata_default *)dcmn;
  // 返回 d->pos 加上 dds_cdr_header 结构体的大小
  return d->pos + (uint32_t)sizeof(struct dds_cdr_header);
}

// 定义一个静态函数 serdata_default_eqkey，用于比较两个序列化数据的 key 是否相等
static bool serdata_default_eqkey(const struct ddsi_serdata *acmn,
                                  const struct ddsi_serdata *bcmn) {
  // 将 acmn 和 bcmn 转换为 dds_serdata_default 类型的指针
  const struct dds_serdata_default *a = (const struct dds_serdata_default *)acmn;
  const struct dds_serdata_default *b = (const struct dds_serdata_default *)bcmn;
  // 断言 a 和 b 的 key.buftype 不是 KEYBUFTYPE_UNSET
  assert(a->key.buftype != KEYBUFTYPE_UNSET && b->key.buftype != KEYBUFTYPE_UNSET);
  // 比较 a 和 b 的 key.keysize 是否相等，以及它们的 keybuf 是否相同
  return a->key.keysize == b->key.keysize &&
         memcmp(serdata_default_keybuf(a), serdata_default_keybuf(b), a->key.keysize) == 0;
}

// 定义一个静态函数 serdata_default_eqkey_nokey，用于在没有 key 的情况下比较两个序列化数据是否相等
static bool serdata_default_eqkey_nokey(const struct ddsi_serdata *acmn,
                                        const struct ddsi_serdata *bcmn) {
  // 忽略 acmn 和 bcmn 的值
  (void)acmn;
  (void)bcmn;
  // 直接返回 true
  return true;
}

// 定义一个静态函数 serdata_default_free，用于释放序列化数据占用的内存
static void serdata_default_free(struct ddsi_serdata *dcmn) {
  // 将 dcmn 转换为 dds_serdata_default 类型的指针
  struct dds_serdata_default *d = (struct dds_serdata_default *)dcmn;
  // 断言 d->c.refc 的值为 0
  assert(ddsrt_atomic_ld32(&d->c.refc) == 0);

  // 如果 d->key.buftype 是 KEYBUFTYPE_DYNALLOC，则释放 d->key.u.dynbuf 占用的内存
  if (d->key.buftype == KEYBUFTYPE_DYNALLOC) ddsrt_free(d->key.u.dynbuf);

#ifdef DDS_HAS_SHM
  // 如果定义了 DDS_HAS_SHM，则释放与共享内存相关的资源
  free_iox_chunk(d->c.iox_subscriber, &d->c.iox_chunk);
#endif

  // 如果 d->size 大于 MAX_SIZE_FOR_POOL 或者无法将 d 放回 freelist，则释放 d 占用的内存
  if (d->size > MAX_SIZE_FOR_POOL || !ddsi_freelist_push(&d->serpool->freelist, d)) dds_free(d);
}

// 定义一个静态函数 serdata_default_init，用于初始化 dds_serdata_default 结构体
static void serdata_default_init(struct dds_serdata_default *d,
                                 const struct dds_sertype_default *tp,
                                 enum ddsi_serdata_kind kind,
                                 uint32_t xcdr_version) {
  // 初始化 d->c
  ddsi_serdata_init(&d->c, &tp->c, kind);
  // 设置 d->pos 为 0
  d->pos = 0;
#ifndef NDEBUG
  // 如果没有定义 NDEBUG，则设置 d->fixed 为 false
  d->fixed = false;
#endif
  // 根据 xcdr_version 设置 d->hdr.identifier 的值
  if (xcdr_version != DDSI_RTPS_CDR_ENC_VERSION_UNDEF)
    d->hdr.identifier = ddsi_sertype_get_native_enc_identifier(xcdr_version, tp->encoding_format);
  else
    d->hdr.identifier = 0;
  // 设置 d->hdr.options 为 0
  d->hdr.options = 0;
  // 设置 d->key.buftype 为 KEYBUFTYPE_UNSET
  d->key.buftype = KEYBUFTYPE_UNSET;
  // 设置 d->key.keysize 为 0
  d->key.keysize = 0;
}

// 定义一个静态函数 serdata_default_allocnew，用于分配新的 dds_serdata_default 结构体
static struct dds_serdata_default *serdata_default_allocnew(struct dds_serdatapool *serpool,
                                                            uint32_t init_size) {
  // 分配内存并初始化 dds_serdata_default 结构体
  struct dds_serdata_default *d =
      ddsrt_malloc(offsetof(struct dds_serdata_default, data) + init_size);
  // 设置 d->size 为 init_size
  d->size = init_size;
  // 设置 d->serpool 为传入的 serpool
  d->serpool = serpool;
  // 返回分配的 dds_serdata_default 结构体指针
  return d;
}
// 为 serdata_default_new_size 函数创建一个新的 dds_serdata_default 结构体实例
static struct dds_serdata_default *serdata_default_new_size(const struct dds_sertype_default *tp,
                                                            enum ddsi_serdata_kind kind,
                                                            uint32_t size,
                                                            uint32_t xcdr_version) {
  struct dds_serdata_default *d;
  // 如果 size 小于等于 MAX_SIZE_FOR_POOL 并且从 tp->serpool->freelist 中弹出一个元素成功，则将
  // d->c.refc 原子设置为 1
  if (size <= MAX_SIZE_FOR_POOL && (d = ddsi_freelist_pop(&tp->serpool->freelist)) != NULL)
    ddsrt_atomic_st32(&d->c.refc, 1);
  // 否则，如果分配新的 serdata_default_allocnew 失败，则返回 NULL
  else if ((d = serdata_default_allocnew(tp->serpool, size)) == NULL)
    return NULL;
  // 初始化 dds_serdata_default 结构体实例
  serdata_default_init(d, tp, kind, xcdr_version);
  return d;
}

// 为 serdata_default_new 函数创建一个新的 dds_serdata_default 结构体实例，使用默认大小
static struct dds_serdata_default *serdata_default_new(const struct dds_sertype_default *tp,
                                                       enum ddsi_serdata_kind kind,
                                                       uint32_t xcdr_version) {
  return serdata_default_new_size(tp, kind, DEFAULT_NEW_SIZE, xcdr_version);
}

// 检查 cdr_identifier 是否是有效的 XCDR 标识符
static inline bool is_valid_xcdr_id(unsigned short cdr_identifier) {
  // PL_CDR_(L|B)E version 1 仅支持发现数据，使用 ddsi_serdata_plist
  return (cdr_identifier == DDSI_RTPS_CDR_LE || cdr_identifier == DDSI_RTPS_CDR_BE ||
          cdr_identifier == DDSI_RTPS_CDR2_LE || cdr_identifier == DDSI_RTPS_CDR2_BE ||
          cdr_identifier == DDSI_RTPS_D_CDR2_LE || cdr_identifier == DDSI_RTPS_D_CDR2_BE ||
          cdr_identifier == DDSI_RTPS_PL_CDR2_LE || cdr_identifier == DDSI_RTPS_PL_CDR2_BE);
}

// 定义 gen_serdata_key_input_kind 枚举类型
enum gen_serdata_key_input_kind { GSKIK_SAMPLE, GSKIK_CDRSAMPLE, GSKIK_CDRKEY };

// 检查主题是否具有固定键
static inline bool is_topic_fixed_key(uint32_t flagset, uint32_t xcdrv) {
  if (xcdrv == DDSI_RTPS_CDR_ENC_VERSION_1)
    return flagset & DDS_TOPIC_FIXED_KEY;
  else if (xcdrv == DDSI_RTPS_CDR_ENC_VERSION_2)
    return flagset & DDS_TOPIC_FIXED_KEY_XCDR2;
  assert(0);
  return false;
}

// 生成 serdata 的 key
static bool gen_serdata_key(const struct dds_sertype_default *type,
                            struct dds_serdata_default_key *kh,
                            enum gen_serdata_key_input_kind input_kind,
                            void *input) {
  const struct dds_cdrstream_desc *desc = &type->type;
  struct dds_istream *is = NULL;
  kh->buftype = KEYBUFTYPE_UNSET;
  // 如果没有键，则将 buftype 设置为 KEYBUFTYPE_STATIC，并将 keysize 设置为 0
  if (desc->keys.nkeys == 0) {
    kh->buftype = KEYBUFTYPE_STATIC;
    kh->keysize = 0;
  }
  // 如果 input_kind 为 GSKIK_CDRKEY
  else if (input_kind == GSKIK_CDRKEY) {
    is = input;
    if (is->m_xcdr_version == DDSI_RTPS_CDR_ENC_VERSION_2) {
      kh->buftype = KEYBUFTYPE_DYNALIAS;
      assert(is->m_size < (1u << 30));
      kh->keysize = is->m_size & SERDATA_DEFAULT_KEYSIZE_MASK;
      kh->u.dynbuf = (unsigned char *)is->m_buffer;
    }
  }

  // 如果 buftype 仍然是 KEYBUFTYPE_UNSET
  if (kh->buftype == KEYBUFTYPE_UNSET) {
    // 强制将 serdata 对象中的 key 序列化为 XCDR2 格式
    // 初始化dds_ostream_t对象
    dds_ostream_t os;
    // 使用默认分配器和DDSI_RTPS_CDR_ENC_VERSION_2版本初始化输出流
    dds_ostream_init(&os, &dds_cdrstream_default_allocator, 0, DDSI_RTPS_CDR_ENC_VERSION_2);
    // 判断主题是否为固定键值类型
    if (is_topic_fixed_key(desc->flagset, DDSI_RTPS_CDR_ENC_VERSION_2)) {
      // FIXME: 还有更多情况我们不必分配内存
      // 设置输出流的缓冲区和大小
      os.m_buffer = kh->u.stbuf;
      os.m_size = DDS_FIXED_KEY_MAX_SIZE;
    }
    // 根据输入类型进行处理
    switch (input_kind) {
      case GSKIK_SAMPLE:
        // 将键值写入输出流
        dds_stream_write_key(&os, &dds_cdrstream_default_allocator, input, &type->type);
        break;
      case GSKIK_CDRSAMPLE:
        // 从数据中提取键值并写入输出流，如果失败则返回false
        if (!dds_stream_extract_key_from_data(input, &os, &dds_cdrstream_default_allocator,
                                              &type->type))
          return false;
        break;
      case GSKIK_CDRKEY:
        // 断言输入流存在且版本为DDSI_RTPS_CDR_ENC_VERSION_1
        assert(is);
        assert(is->m_xcdr_version == DDSI_RTPS_CDR_ENC_VERSION_1);
        // 从输入流中提取键值并写入输出流
        dds_stream_extract_key_from_key(is, &os, &dds_cdrstream_default_allocator, &type->type);
        break;
    }
    // 断言输出流索引小于(1u << 30)
    assert(os.m_index < (1u << 30));
    // 设置键值大小
    kh->keysize = os.m_index & SERDATA_DEFAULT_KEYSIZE_MASK;
    // 判断主题是否为固定键值类型
    if (is_topic_fixed_key(desc->flagset, DDSI_RTPS_CDR_ENC_VERSION_2))
      // 设置缓冲区类型为静态
      kh->buftype = KEYBUFTYPE_STATIC;
    else {
      // 设置缓冲区类型为动态分配
      kh->buftype = KEYBUFTYPE_DYNALLOC;
      // 重新分配输出流缓冲区，以减少浪费的字节
      // FIXME: 可能应该愿意浪费一点
      kh->u.dynbuf = ddsrt_realloc(os.m_buffer, os.m_index);
    }
  }
  return true;
}

// 定义一个从序列化数据向量创建serdata_default的函数
static struct dds_serdata_default *serdata_default_from_ser_iov_common(
    const struct ddsi_sertype *tpcmn,
    enum ddsi_serdata_kind kind,
    ddsrt_msg_iovlen_t niov,
    const ddsrt_iovec_t *iov,
    size_t size) {
  // 将通用的dds_sertype类型转换为dds_sertype_default类型
  const struct dds_sertype_default *tp = (const struct dds_sertype_default *)tpcmn;

  // FIXME:
  // 检查这是否是正确的最大值：偏移量相对于CDR头，但也有一些地方将serdata视为流，并使用相对于serdata起始位置的偏移量（m_index）
  if (size > UINT32_MAX - offsetof(struct dds_serdata_default, hdr)) return NULL;
  // 断言niov至少为1
  assert(niov >= 1);
  // 如果第一个iovec的长度小于4（CDR头），则返回NULL
  if (iov[0].iov_len < 4) /* CDR header */
    return NULL;
  // 创建一个新的serdata_default结构体实例
  struct dds_serdata_default *d =
      serdata_default_new_size(tp, kind, (uint32_t)size, DDSI_RTPS_CDR_ENC_VERSION_UNDEF);
  if (d == NULL) return NULL;

  // 将iov[0]的内容复制到d->hdr中
  memcpy(&d->hdr, iov[0].iov_base, sizeof(d->hdr));
  // 检查d->hdr.identifier是否有效
  if (!is_valid_xcdr_id(d->hdr.identifier)) goto err;
  // 将iov[0]中剩余的数据追加到serdata_default实例中
  serdata_default_append_blob(&d, iov[0].iov_len - 4, (const char *)iov[0].iov_base + 4);
  // 遍历iov数组，将每个iovec中的数据追加到serdata_default实例中
  for (ddsrt_msg_iovlen_t i = 1; i < niov; i++)
    serdata_default_append_blob(&d, iov[i].iov_len, iov[i].iov_base);

  // 检查是否需要字节交换
  const bool needs_bswap = !DDSI_RTPS_CDR_ENC_IS_NATIVE(d->hdr.identifier);
  // 将d->hdr.identifier转换为本地表示形式
  d->hdr.identifier = DDSI_RTPS_CDR_ENC_TO_NATIVE(d->hdr.identifier);
  // 获取填充值和xcdr版本以及编码格式
  const uint32_t pad = ddsrt_fromBE2u(d->hdr.options) & 2;
  const uint32_t xcdr_version = ddsi_sertype_enc_id_xcdr_version(d->hdr.identifier);
  const uint32_t encoding_format = ddsi_sertype_enc_id_enc_format(d->hdr.identifier);
  // 如果编码格式与tp->encoding_format不匹配，则跳转到错误处理
  if (encoding_format != tp->encoding_format) goto err;

  // 定义实际大小变量
  uint32_t actual_size;
  // 对数据进行规范化处理，并检查结果
  if (d->pos < pad || !dds_stream_normalize(d->data, d->pos - pad, needs_bswap, xcdr_version,
                                            &tp->type, kind == SDK_KEY, &actual_size))
    goto err;

  // 初始化输入流
  dds_istream_t is;
  dds_istream_init(&is, actual_size, d->data, xcdr_version);
  // 从CDR中生成serdata_key，并检查结果
  if (!gen_serdata_key_from_cdr(&is, &d->key, tp, kind == SDK_KEY)) goto err;
  // 返回创建的serdata_default实例
  return d;

// 错误处理：释放serdata_default实例并返回NULL
err:
  ddsi_serdata_unref(&d->c);
  return NULL;
}

// 定义一个从序列化数据创建 serdata_default 的函数
static struct dds_serdata_default *serdata_default_from_ser_iov_common(
    const struct ddsi_sertype *tpcmn,
    enum ddsi_serdata_kind kind,
    ddsrt_msg_iovlen_t niov,
    const ddsrt_iovec_t *iov,
    size_t size) {
  // 将通用的 sertype 转换为默认的 sertype
  const struct dds_sertype_default *tp = (const struct dds_sertype_default *)tpcmn;

  // FIXME: 检查这是否是正确的最大值：偏移量相对于 CDR 头，但也有一些地方将 serdata 当作流使用，
  // 并且这些地方使用相对于 serdata 开始的偏移量（m_index）
  if (size > UINT32_MAX - offsetof(struct dds_serdata_default, hdr)) return NULL;
  assert(niov >= 1);
  if (iov[0].iov_len < 4)  // CDR 头
    return NULL;

  // 创建一个新的 serdata_default 结构
  struct dds_serdata_default *d =
      serdata_default_new_size(tp, kind, (uint32_t)size, DDSI_RTPS_CDR_ENC_VERSION_UNDEF);
  if (d == NULL) return NULL;

  // 将 iov[0] 中的数据复制到 d->hdr
  memcpy(&d->hdr, iov[0].iov_base, sizeof(d->hdr));
  if (!is_valid_xcdr_id(d->hdr.identifier)) goto err;

  // 将 iov[0] 中剩余的数据追加到 d
  serdata_default_append_blob(&d, iov[0].iov_len - 4, (const char *)iov[0].iov_base + 4);
  for (ddsrt_msg_iovlen_t i = 1; i < niov; i++)
    serdata_default_append_blob(&d, iov[i].iov_len, iov[i].iov_base);

  // 检查是否需要字节交换
  const bool needs_bswap = !DDSI_RTPS_CDR_ENC_IS_NATIVE(d->hdr.identifier);
  d->hdr.identifier = DDSI_RTPS_CDR_ENC_TO_NATIVE(d->hdr.identifier);
  const uint32_t pad = ddsrt_fromBE2u(d->hdr.options) & 2;
  const uint32_t xcdr_version = ddsi_sertype_enc_id_xcdr_version(d->hdr.identifier);
  const uint32_t encoding_format = ddsi_sertype_enc_id_enc_format(d->hdr.identifier);
  if (encoding_format != tp->encoding_format) goto err;

  // 计算实际大小
  uint32_t actual_size;
  if (d->pos < pad || !dds_stream_normalize(d->data, d->pos - pad, needs_bswap, xcdr_version,
                                            &tp->type, kind == SDK_KEY, &actual_size))
    goto err;

  // 初始化输入流并从 CDR 中生成 serdata_key
  dds_istream_t is;
  dds_istream_init(&is, actual_size, d->data, xcdr_version);
  if (!gen_serdata_key_from_cdr(&is, &d->key, tp, kind == SDK_KEY)) goto err;
  return d;

err:
  // 错误处理：释放已分配的资源
  ddsi_serdata_unref(&d->c);
  return NULL;
}

// 定义一个从序列化数据创建 ddsi_serdata 的函数
static struct ddsi_serdata *serdata_default_from_ser(const struct ddsi_sertype *tpcmn,
                                                     enum ddsi_serdata_kind kind,
                                                     const struct ddsi_rdata *fragchain,
                                                     size_t size) {
  struct dds_serdata_default *d;

  // 使用通用函数从序列化数据创建 serdata_default
  if ((d = serdata_default_from_ser_common(tpcmn, kind, fragchain, size)) == NULL) return NULL;

  // 修复 serdata_default 并返回结果
  return fix_serdata_default(d, tpcmn->serdata_basehash);
}

// 定义一个从序列化数据的 iov 创建 ddsi_serdata 的函数
static struct ddsi_serdata *serdata_default_from_ser_iov(const struct ddsi_sertype *tpcmn,
                                                         enum ddsi_serdata_kind kind,
                                                         ddsrt_msg_iovlen_t niov,
                                                         const ddsrt_iovec_t *iov,
                                                         size_t size) {
  struct dds_serdata_default *d;

  // 使用通用函数从序列化数据的 iov 创建 serdata_default
  if ((d = serdata_default_from_ser_iov_common(tpcmn, kind, niov, iov, size)) == NULL) return NULL;

  // 修复 serdata_default 并返回结果
  return fix_serdata_default(d, tpcmn->serdata_basehash);
}

// 定义一个从序列化数据创建没有 key 的 ddsi_serdata 的函数
static struct ddsi_serdata *serdata_default_from_ser_nokey(const struct ddsi_sertype *tpcmn,
                                                           enum ddsi_serdata_kind kind,
                                                           const struct ddsi_rdata *fragchain,
                                                           size_t size) {
  struct dds_serdata_default *d;

  // 使用通用函数从序列化数据创建 serdata_default
  if ((d = serdata_default_from_ser_common(tpcmn, kind, fragchain, size)) == NULL) return NULL;

  // 修复没有 key 的 serdata_default 并返回结果
  return fix_serdata_default_nokey(d, tpcmn->serdata_basehash);
}

// 定义一个从序列化数据的 iov 创建没有 key 的 ddsi_serdata 的函数
static struct ddsi_serdata *serdata_default_from_ser_iov_nokey(const struct ddsi_sertype *tpcmn,
                                                               enum ddsi_serdata_kind kind,
                                                               ddsrt_msg_iovlen_t niov,
                                                               const ddsrt_iovec_t *iov,
                                                               size_t size) {
  struct dds_serdata_default *d;

  // 使用通用函数从序列化数据的 iov 创建 serdata_default
  if ((d = serdata_default_from_ser_iov_common(tpcmn, kind, niov, iov, size)) == NULL) return NULL;

  // 修复没有 key 的 serdata_default 并返回结果
  return fix_serdata_default_nokey(d, tpcmn->serdata_basehash);
}

// 定义一个从 CDR 格式的 keyhash 创建 ddsi_serdata 的函数
static struct ddsi_serdata *serdata_default_from_keyhash_cdr(const struct ddsi_sertype *tpcmn,
                                                             const ddsi_keyhash_t *keyhash) {
  const struct dds_sertype_default *tp = (const struct dds_sertype_default *)tpcmn;
  if (!is_topic_fixed_key(tp->type.flagset, DDSI_RTPS_CDR_ENC_VERSION_2)) {
    // keyhash 是键值的 MD5，因此无法转换为键值
    return NULL;
  } else {
    // 构造 iovec 数组并调用 serdata_default_from_ser_iov 函数创建 ddsi_serdata
    const ddsrt_iovec_t iovec[2] = {
        {.iov_base = (unsigned char[]){0, 0, 0, 0},
         .iov_len = 4},  // big-endian, unspecified padding
        {.iov_base = (void *)keyhash->value, .iov_len = (ddsrt_iov_len_t)sizeof(*keyhash)}};
    return serdata_default_from_ser_iov(tpcmn, SDK_KEY, 2, iovec, 4 + sizeof(*keyhash));
  }
}
// 根据给定的类型和键哈希值创建一个默认的序列化数据对象，但不包含键
static struct ddsi_serdata *serdata_default_from_keyhash_cdr_nokey(const struct ddsi_sertype *tpcmn,
                                                                   const ddsi_keyhash_t *keyhash) {
  // 将通用类型转换为默认类型
  const struct dds_sertype_default *tp = (const struct dds_sertype_default *)tpcmn;
  // 对于无键主题，CDR编码版本是无关紧要的
  struct dds_serdata_default *d = serdata_default_new(tp, SDK_KEY, DDSI_RTPS_CDR_ENC_VERSION_UNDEF);
  if (d == NULL) return NULL;
  (void)keyhash;
  return fix_serdata_default_nokey(d, tp->c.serdata_basehash);
}

#ifdef DDS_HAS_SHM
// 从接收到的iox缓冲区创建一个序列化数据对象
static struct ddsi_serdata *serdata_default_from_received_iox_buffer(
    const struct ddsi_sertype *tpcmn, enum ddsi_serdata_kind kind, void *sub, void *iox_buffer) {
  const iceoryx_header_t *ice_hdr = iceoryx_header_from_chunk(iox_buffer);

  const struct dds_sertype_default *tp = (const struct dds_sertype_default *)tpcmn;

  struct dds_serdata_default *d =
      serdata_default_new_size(tp, kind, ice_hdr->data_size, tp->write_encoding_version);

  // 注意：这里我们不进行反序列化或memcpy操作，只是获取chunk的所有权
  d->c.iox_chunk = iox_buffer;
  d->c.iox_subscriber = sub;
  d->key.buftype = KEYBUFTYPE_STATIC;
  d->key.keysize = DDS_FIXED_KEY_MAX_SIZE;
  memcpy(d->key.u.stbuf, ice_hdr->keyhash.value, DDS_FIXED_KEY_MAX_SIZE);

  fix_serdata_default(d, tpcmn->serdata_basehash);

  return (struct ddsi_serdata *)d;
}

// 仅在需要iceoryx的情况下创建一个序列化数据对象（即不需要网络）。
// 这将跳过昂贵的序列化操作，只获取iceoryx缓冲区的所有权。
// 计算keyhash目前仍然是必需的。
static struct ddsi_serdata *dds_serdata_default_from_loaned_sample(const struct ddsi_sertype *type,
                                                                   enum ddsi_serdata_kind kind,
                                                                   const char *sample) {
  const struct dds_sertype_default *t = (const struct dds_sertype_default *)type;
  struct dds_serdata_default *d = serdata_default_new(t, kind, t->write_encoding_version);

  if (d == NULL) return NULL;

  // 目前即使在共享内存情况下也需要这个（因为它可能在读取器端使用）。
  // 这可能仍然会导致与样本大小成线性关系的计算成本（？）。
  // TODO：我们是否可以通过在读取器端进行特定处理来避免这种情况，而不需要keyhash？
  if (!gen_serdata_key_from_sample(t, &d->key, sample)) return NULL;

  struct ddsi_serdata *serdata = &d->c;
  serdata->iox_chunk = (void *)sample;
  return serdata;
}

// 从iox创建一个序列化数据对象
static struct ddsi_serdata *serdata_default_from_iox(const struct ddsi_sertype *tpcmn,
                                                     enum ddsi_serdata_kind kind,
                                                     void *sub,
                                                     void *buffer) {
  if (sub == NULL)
    return dds_serdata_default_from_loaned_sample(tpcmn, kind, buffer);
  else
    return serdata_default_from_received_iox_buffer(tpcmn, kind, sub, buffer);
}
#endif
// 从 dds_serdata_default 结构体中创建一个输入流对象
static void istream_from_serdata_default(dds_istream_t *__restrict s,
                                         const struct dds_serdata_default *__restrict d) {
  // 设置输入流的缓冲区指针为 dds_serdata_default 结构体的起始地址
  s->m_buffer = (const unsigned char *)d;
  // 设置输入流的索引为 dds_serdata_default 结构体中 data 成员的偏移量
  s->m_index = (uint32_t)offsetof(struct dds_serdata_default, data);
  // 设置输入流的大小为 dds_serdata_default 结构体中 size 成员的值加上索引值
  s->m_size = d->size + s->m_index;
#if DDSRT_ENDIAN == DDSRT_LITTLE_ENDIAN
  // 如果系统字节序为小端，检查 d->hdr.identifier 是否为小端编码
  assert(DDSI_RTPS_CDR_ENC_LE(d->hdr.identifier));
#elif DDSRT_ENDIAN == DDSRT_BIG_ENDIAN
  // 如果系统字节序为大端，检查 d->hdr.identifier 是否不为小端编码
  assert(!DDSI_RTPS_CDR_ENC_LE(d->hdr.identifier));
#endif
  // 设置输入流的 xcdr 版本
  s->m_xcdr_version = ddsi_sertype_enc_id_xcdr_version(d->hdr.identifier);
}

// 从 dds_serdata_default 结构体中创建一个输出流对象
static void ostream_from_serdata_default(dds_ostream_t *__restrict s,
                                         const struct dds_serdata_default *__restrict d) {
  // 设置输出流的缓冲区指针为 dds_serdata_default 结构体的起始地址
  s->m_buffer = (unsigned char *)d;
  // 设置输出流的索引为 dds_serdata_default 结构体中 data 成员的偏移量
  s->m_index = (uint32_t)offsetof(struct dds_serdata_default, data);
  // 设置输出流的大小为 dds_serdata_default 结构体中 size 成员的值加上索引值
  s->m_size = d->size + s->m_index;
#if DDSRT_ENDIAN == DDSRT_LITTLE_ENDIAN
  // 如果系统字节序为小端，检查 d->hdr.identifier 是否为小端编码
  assert(DDSI_RTPS_CDR_ENC_LE(d->hdr.identifier));
#elif DDSRT_ENDIAN == DDSRT_BIG_ENDIAN
  // 如果系统字节序为大端，检查 d->hdr.identifier 是否不为小端编码
  assert(!DDSI_RTPS_CDR_ENC_LE(d->hdr.identifier));
#endif
  // 设置输出流的 xcdr 版本
  s->m_xcdr_version = ddsi_sertype_enc_id_xcdr_version(d->hdr.identifier);
}

// 将输出流添加到 dds_serdata_default 结构体中
static void ostream_add_to_serdata_default(dds_ostream_t *__restrict s,
                                           struct dds_serdata_default **__restrict d) {
  // DDSI 要求 4 字节对齐
  const uint32_t pad =
      dds_cdr_alignto4_clear_and_resize(s, &dds_cdrstream_default_allocator, s->m_xcdr_version);
  // 断言 pad 的值应小于等于 3
  assert(pad <= 3);

  // 重置数据指针，因为流可能已重新分配
  (*d) = (void *)s->m_buffer;
  // 设置 dds_serdata_default 结构体中 pos 成员的值
  (*d)->pos = (s->m_index - (uint32_t)offsetof(struct dds_serdata_default, data));
  // 设置 dds_serdata_default 结构体中 size 成员的值
  (*d)->size = (s->m_size - (uint32_t)offsetof(struct dds_serdata_default, data));
  // 设置 dds_serdata_default 结构体中 hdr.options 成员的值
  (*d)->hdr.options = ddsrt_toBE2u((uint16_t)pad);
}
// 定义一个静态函数，用于从CDR样本创建serdata_default结构体
static struct dds_serdata_default *serdata_default_from_sample_cdr_common(
    const struct ddsi_sertype *tpcmn,
    enum ddsi_serdata_kind kind,
    uint32_t xcdr_version,
    const void *sample) {
  // 将通用的dds_sertype类型转换为dds_sertype_default类型
  const struct dds_sertype_default *tp = (const struct dds_sertype_default *)tpcmn;

  // 创建一个新的serdata_default结构体实例
  struct dds_serdata_default *d = serdata_default_new(tp, kind, xcdr_version);

  // 如果创建失败，返回NULL
  if (d == NULL) return NULL;

  // 定义一个dds_ostream_t类型的变量os
  dds_ostream_t os;

  // 初始化os，使其指向serdata_default结构体中的数据
  ostream_from_serdata_default(&os, d);

  // 根据serdata的类型进行不同的处理
  switch (kind) {
    case SDK_EMPTY:
      ostream_add_to_serdata_default(&os, &d);
      break;
    case SDK_KEY:
      dds_stream_write_key(&os, &dds_cdrstream_default_allocator, sample, &tp->type);
      ostream_add_to_serdata_default(&os, &d);

      // FIXME: 检测XCDR1和XCDR2表示相等的情况，
      // 这样我们可以从d->data中获取XCDR1密钥的别名
      // FIXME: 使用写入器使用的CDR编码版本，而不是sertype的写入编码
      if (tp->write_encoding_version == DDSI_RTPS_CDR_ENC_VERSION_2) {
        d->key.buftype = KEYBUFTYPE_DYNALIAS;

        // dds_ostream_add_to_serdata_default将大小填充为4的倍数，
        // 并在hdr.options的最低有效位中写入添加的填充字节数，
        // 以符合XTypes规范。
        //
        // 这些填充字节不是密钥的一部分！
        assert(ddsrt_fromBE2u(d->hdr.options) < 4);
        d->key.keysize = (d->pos - ddsrt_fromBE2u(d->hdr.options)) & SERDATA_DEFAULT_KEYSIZE_MASK;
        d->key.u.dynbuf = (unsigned char *)d->data;
      } else {
        // 我们有一个XCDR1密钥，因此必须将其转换为XCDR2以将其存储为serdata中的密钥。
        if (!gen_serdata_key_from_sample(tp, &d->key, sample)) goto error;
      }
      break;
    case SDK_DATA: {
      const bool ok =
          dds_stream_write_sample(&os, &dds_cdrstream_default_allocator, sample, &tp->type);

      // `os` 别名了 `d` 中的内容，但已更改并可能已移动。
      // 即使 write_sample 失败，`d` 也需要更新。
      ostream_add_to_serdata_default(&os, &d);
      if (!ok) goto error;
      if (!gen_serdata_key_from_sample(tp, &d->key, sample)) goto error;
      break;
    }
  }

  // 返回创建的serdata_default结构体实例
  return d;

// 错误处理标签，释放serdata并返回NULL
error:
  ddsi_serdata_unref(&d->c);
  return NULL;
}

// 从样本数据表示创建serdata_default的函数
static struct ddsi_serdata *serdata_default_from_sample_data_representation(
    const struct ddsi_sertype *tpcmn,
    enum ddsi_serdata_kind kind,
    dds_data_representation_id_t data_representation,
    const void *sample,
    bool key) {
  // 断言数据表示为XCDR1或XCDR2
  assert(data_representation == DDS_DATA_REPRESENTATION_XCDR1 ||
         data_representation == DDS_DATA_REPRESENTATION_XCDR2);
  struct dds_serdata_default *d;
  // 根据数据表示设置xcdr版本
  uint32_t xcdr_version = data_representation == DDS_DATA_REPRESENTATION_XCDR1
                              ? DDSI_RTPS_CDR_ENC_VERSION_1
                              : DDSI_RTPS_CDR_ENC_VERSION_2;
  // 调用serdata_default_from_sample_cdr_common函数，如果返回NULL，则返回NULL
  if ((d = serdata_default_from_sample_cdr_common(tpcmn, kind, xcdr_version, sample)) == NULL)
    return NULL;
  // 根据key值调用fix_serdata_default或fix_serdata_default_nokey函数
  return key ? fix_serdata_default(d, tpcmn->serdata_basehash)
             : fix_serdata_default_nokey(d, tpcmn->serdata_basehash);
}

// 从CDR样本创建serdata_default的函数
static struct ddsi_serdata *serdata_default_from_sample_cdr(const struct ddsi_sertype *tpcmn,
                                                            enum ddsi_serdata_kind kind,
                                                            const void *sample) {
  // 调用serdata_default_from_sample_data_representation函数，传入XCDR1数据表示和true作为key参数
  return serdata_default_from_sample_data_representation(tpcmn, kind, DDS_DATA_REPRESENTATION_XCDR1,
                                                         sample, true);
}

// 从XCDR2样本创建serdata_default的函数
static struct ddsi_serdata *serdata_default_from_sample_xcdr2(const struct ddsi_sertype *tpcmn,
                                                              enum ddsi_serdata_kind kind,
                                                              const void *sample) {
  // 调用serdata_default_from_sample_data_representation函数，传入XCDR2数据表示和true作为key参数
  return serdata_default_from_sample_data_representation(tpcmn, kind, DDS_DATA_REPRESENTATION_XCDR2,
                                                         sample, true);
}

// 从无键CDR样本创建serdata_default的函数
static struct ddsi_serdata *serdata_default_from_sample_cdr_nokey(const struct ddsi_sertype *tpcmn,
                                                                  enum ddsi_serdata_kind kind,
                                                                  const void *sample) {
  // 调用serdata_default_from_sample_data_representation函数，传入XCDR1数据表示和false作为key参数
  return serdata_default_from_sample_data_representation(tpcmn, kind, DDS_DATA_REPRESENTATION_XCDR1,
                                                         sample, false);
}

// 从无键XCDR2样本创建serdata_default的函数
static struct ddsi_serdata *serdata_default_from_sample_xcdr2_nokey(
    const struct ddsi_sertype *tpcmn, enum ddsi_serdata_kind kind, const void *sample) {
  // 调用serdata_default_from_sample_data_representation函数，传入XCDR2数据表示和false作为key参数
  return serdata_default_from_sample_data_representation(tpcmn, kind, DDS_DATA_REPRESENTATION_XCDR2,
                                                         sample, false);
}

// 将serdata_common转换为无类型的serdata_default的函数
static struct ddsi_serdata *serdata_default_to_untyped(const struct ddsi_serdata *serdata_common) {
  // 类型转换serdata_common为dds_serdata_default
  const struct dds_serdata_default *d = (const struct dds_serdata_default *)serdata_common;
  // 类型转换d->c.type为dds_sertype_default
  const struct dds_sertype_default *tp = (const struct dds_sertype_default *)d->c.type;

  // 断言d->hdr.identifier为本地CDR编码
  assert(DDSI_RTPS_CDR_ENC_IS_NATIVE(d->hdr.identifier));
  // 创建新的serdata_default对象，设置SDK_KEY和XCDR2版本
  struct dds_serdata_default *d_tl = serdata_default_new(tp, SDK_KEY, DDSI_RTPS_CDR_ENC_VERSION_2);
  if (d_tl == NULL) return NULL;
  // 设置d_tl的type、hash和timestamp属性
  d_tl->c.type = NULL;
  d_tl->c.hash = d->c.hash;
  d_tl->c.timestamp.v = INT64_MIN;
  // 如果d->c.ops为dds_serdata_ops_cdr或dds_serdata_ops_xcdr2，则处理键值相关内容
  if (d->c.ops == &dds_serdata_ops_cdr || d->c.ops == &dds_serdata_ops_xcdr2) {
    serdata_default_append_blob(&d_tl, d->key.keysize, serdata_default_keybuf(d));
    d_tl->key.buftype = KEYBUFTYPE_DYNALIAS;
    d_tl->key.keysize = d->key.keysize;
    d_tl->key.u.dynbuf = (unsigned char *)d_tl->data;
  } else {
    // 断言d->c.ops为dds_serdata_ops_cdr_nokey或dds_serdata_ops_xcdr2_nokey
    assert(d->c.ops == &dds_serdata_ops_cdr_nokey || d->c.ops == &dds_serdata_ops_xcdr2_nokey);
  }
  // 返回类型转换后的d_tl
  return (struct ddsi_serdata *)d_tl;
}
// 使用默认序列化数据填充缓冲区，从 'off' 开始，填充 'size' 字节；0 <= off < off+sz <=
// alignup4(size(d))
static void serdata_default_to_ser(const struct ddsi_serdata *serdata_common,
                                   size_t off,
                                   size_t sz,
                                   void *buf) {
  // 将通用序列化数据结构转换为默认序列化数据结构
  const struct dds_serdata_default *d = (const struct dds_serdata_default *)serdata_common;
  // 检查偏移量是否在有效范围内
  assert(off < d->pos + sizeof(struct dds_cdr_header));
  // 检查大小是否在有效范围内
  assert(sz <= alignup_size(d->pos + sizeof(struct dds_cdr_header), 4) - off);
  // 将序列化数据复制到缓冲区
  memcpy(buf, (char *)&d->hdr + off, sz);
}

static struct ddsi_serdata *serdata_default_to_ser_ref(const struct ddsi_serdata *serdata_common,
                                                       size_t off,
                                                       size_t sz,
                                                       ddsrt_iovec_t *ref) {
  // 将通用序列化数据结构转换为默认序列化数据结构
  const struct dds_serdata_default *d = (const struct dds_serdata_default *)serdata_common;
  // 检查偏移量是否在有效范围内
  assert(off < d->pos + sizeof(struct dds_cdr_header));
  // 检查大小是否在有效范围内
  assert(sz <= alignup_size(d->pos + sizeof(struct dds_cdr_header), 4) - off);
  // 设置引用的基址和长度
  ref->iov_base = (char *)&d->hdr + off;
  ref->iov_len = (ddsrt_iov_len_t)sz;
  // 返回序列化数据的引用
  return ddsi_serdata_ref(serdata_common);
}

static void serdata_default_to_ser_unref(struct ddsi_serdata *serdata_common,
                                         const ddsrt_iovec_t *ref) {
  // 忽略引用参数
  (void)ref;
  // 取消序列化数据的引用
  ddsi_serdata_unref(serdata_common);
}

static bool serdata_default_to_sample_cdr(const struct ddsi_serdata *serdata_common,
                                          void *sample,
                                          void **bufptr,
                                          void *buflim) {
  // 将通用序列化数据结构转换为默认序列化数据结构
  const struct dds_serdata_default *d = (const struct dds_serdata_default *)serdata_common;
  // 将通用序列化类型结构转换为默认序列化类型结构
  const struct dds_sertype_default *tp = (const struct dds_sertype_default *)d->c.type;
  // 定义输入流变量
  dds_istream_t is;
#ifdef DDS_HAS_SHM
  if (d->c.iox_chunk) {
    void *iox_chunk = d->c.iox_chunk;
    iceoryx_header_t *hdr = iceoryx_header_from_chunk(iox_chunk);
    if (hdr->shm_data_state == IOX_CHUNK_CONTAINS_SERIALIZED_DATA) {
      // 初始化输入流
      dds_istream_init(&is, hdr->data_size, iox_chunk,
                       ddsi_sertype_enc_id_xcdr_version(d->hdr.identifier));
      // 检查序列化数据的标识符是否为本地格式
      assert(DDSI_RTPS_CDR_ENC_IS_NATIVE(d->hdr.identifier));
      if (d->c.kind == SDK_KEY)
        // 读取键值
        dds_stream_read_key(&is, sample, &dds_cdrstream_default_allocator, &tp->type);
      else
        // 读取样本
        dds_stream_read_sample(&is, sample, &dds_cdrstream_default_allocator, &tp->type);
    } else {
      // 应包含原始未序列化数据
      // 我们可以检查数据状态，但不应该需要
      memcpy(sample, iox_chunk, hdr->data_size);
    }
    return true;
  }
#endif
  if (bufptr)
    abort();
  else {
    (void)buflim;
  } /* FIXME: haven't implemented that bit yet! */
  // 检查序列化数据的标识符是否为本地格式
  assert(DDSI_RTPS_CDR_ENC_IS_NATIVE(d->hdr.identifier));
  // 从默认序列化数据创建输入流
  istream_from_serdata_default(&is, d);
  if (d->c.kind == SDK_KEY)
    // 读取键值
    dds_stream_read_key(&is, sample, &dds_cdrstream_default_allocator, &tp->type);
  else
    // 读取样本
    dds_stream_read_sample(&is, sample, &dds_cdrstream_default_allocator, &tp->type);
  return true; /* FIXME: can't conversion to sample fail? */
}
// 将未类型化的serdata转换为CDR样本
static bool serdata_default_untyped_to_sample_cdr(const struct ddsi_sertype *sertype_common,
                                                  const struct ddsi_serdata *serdata_common,
                                                  void *sample,
                                                  void **bufptr,
                                                  void *buflim) {
  // 类型转换
  const struct dds_serdata_default *d = (const struct dds_serdata_default *)serdata_common;
  const struct dds_sertype_default *tp = (const struct dds_sertype_default *)sertype_common;
  dds_istream_t is;

  // 断言检查
  assert(d->c.type == NULL);
  assert(d->c.kind == SDK_KEY);
  assert(d->c.ops == sertype_common->serdata_ops);
  assert(DDSI_RTPS_CDR_ENC_IS_NATIVE(d->hdr.identifier));

  // 检查bufptr是否为空
  if (bufptr)
    abort();
  else {
    (void)buflim;
  } /* FIXME: haven't implemented that bit yet! */

  // 从serdata_default创建输入流
  istream_from_serdata_default(&is, d);

  // 读取键值
  dds_stream_read_key(&is, sample, &dds_cdrstream_default_allocator, &tp->type);

  return true; /* FIXME: can't conversion to sample fail? */
}

// 将未类型化的serdata转换为CDR样本（无键）
static bool serdata_default_untyped_to_sample_cdr_nokey(const struct ddsi_sertype *sertype_common,
                                                        const struct ddsi_serdata *serdata_common,
                                                        void *sample,
                                                        void **bufptr,
                                                        void *buflim) {
  // 忽略未使用的参数
  (void)sertype_common;
  (void)sample;
  (void)bufptr;
  (void)buflim;
  (void)serdata_common;

  // 断言检查
  assert(serdata_common->type == NULL);
  assert(serdata_common->kind == SDK_KEY);

  return true;
}

// 打印CDR样本的serdata
static size_t serdata_default_print_cdr(const struct ddsi_sertype *sertype_common,
                                        const struct ddsi_serdata *serdata_common,
                                        char *buf,
                                        size_t size) {
  // 类型转换
  const struct dds_serdata_default *d = (const struct dds_serdata_default *)serdata_common;
  const struct dds_sertype_default *tp = (const struct dds_sertype_default *)sertype_common;
  dds_istream_t is;

  // 从serdata_default创建输入流
  istream_from_serdata_default(&is, d);

  // 根据serdata类型打印键值或样本
  if (d->c.kind == SDK_KEY)
    return dds_stream_print_key(&is, &tp->type, buf, size);
  else
    return dds_stream_print_sample(&is, &tp->type, buf, size);
}
// 定义一个静态函数 serdata_default_get_keyhash，用于获取序列化数据的 keyhash
static void serdata_default_get_keyhash(const struct ddsi_serdata *serdata_common,
                                        struct ddsi_keyhash *buf,
                                        bool force_md5) {
  // 将通用序列化数据结构转换为默认序列化数据结构
  const struct dds_serdata_default *d = (const struct dds_serdata_default *)serdata_common;
  // 将类型信息转换为默认类型信息
  const struct dds_sertype_default *tp = (const struct dds_sertype_default *)d->c.type;
  // 断言 buf 不为空
  assert(buf);
  // 断言 d->key.buftype 不是 KEYBUFTYPE_UNSET
  assert(d->key.buftype != KEYBUFTYPE_UNSET);

  // 将原生表示法转换为 keyhash 所需的表示法
  // d->key 也可以是大端字节序，但这会消除小端字节序机器上别名 d->data 的可能性
  // 并强制将密钥的两个副本呈现给 SDK_KEY
  //
  // 由于没有人应该使用 DDSI keyhash，因此支付转换所需的代价似乎是值得的

  // 获取 XCDR 版本
  uint32_t xcdrv = ddsi_sertype_enc_id_xcdr_version(d->hdr.identifier);

  // 初始化 istream，使用 XCDR2 序列化的 key 和 key 的大小（d->key.keysize）
  dds_istream_t is;
  dds_istream_init(&is, d->key.keysize, serdata_default_keybuf(d), DDSI_RTPS_CDR_ENC_VERSION_2);

  // 初始化输出流，使用来自 serdata 的 XCDR 版本，以便在 ostream 中计算 keyhash
  // 使用此 CDR 表示法（XTypes 规范 7.6.8，RTPS 规范 9.6.3.8）
  dds_ostreamBE_t os;
  dds_ostreamBE_init(&os, &dds_cdrstream_default_allocator, 0, xcdrv);
  dds_stream_extract_keyBE_from_key(&is, &os, &dds_cdrstream_default_allocator, &tp->type);
  // 断言 is.m_index 等于 d->key.keysize
  assert(is.m_index == d->key.keysize);

  // 我们知道 XCDR2 编码的 key 大小，但对于 XCDR1，由于 key 字段的 8 字节对齐，可能会有额外的填充
  if (xcdrv == DDSI_RTPS_CDR_ENC_VERSION_2) assert(os.x.m_index == d->key.keysize);

  // 无法在此处使用 is_topic_fixed_key，因为如果存在有界字符串键字段，它可能包含较短的字符串并适应
  // 16 字节
  uint32_t actual_keysz = os.x.m_index;
  if (force_md5 || actual_keysz > DDS_FIXED_KEY_MAX_SIZE) {
    // 使用 MD5 计算 keyhash
    ddsrt_md5_state_t md5st;
    ddsrt_md5_init(&md5st);
    ddsrt_md5_append(&md5st, (ddsrt_md5_byte_t *)os.x.m_buffer, actual_keysz);
    ddsrt_md5_finish(&md5st, (ddsrt_md5_byte_t *)buf->value);
  } else {
    // 将 buf->value 的前 DDS_FIXED_KEY_MAX_SIZE 字节设置为 0
    memset(buf->value, 0, DDS_FIXED_KEY_MAX_SIZE);
    // 将 os.x.m_buffer 的前 actual_keysz 字节复制到 buf->value
    memcpy(buf->value, os.x.m_buffer, actual_keysz);
  }
  // 结束输出流
  dds_ostreamBE_fini(&os, &dds_cdrstream_default_allocator);
}
// 定义一个名为ddsi_serdata_ops_cdr的结构体常量，其类型为ddsi_serdata_ops
const struct ddsi_serdata_ops dds_serdata_ops_cdr = {
    .get_size = serdata_default_get_size,          // 获取序列化数据的大小
    .eqkey = serdata_default_eqkey,                // 比较两个序列化数据的键是否相等
    .free = serdata_default_free,                  // 释放序列化数据占用的内存
    .from_ser = serdata_default_from_ser,          // 从序列化数据创建serdata对象
    .from_ser_iov = serdata_default_from_ser_iov,  // 从序列化数据的IO向量创建serdata对象
    .from_keyhash = serdata_default_from_keyhash_cdr,  // 从键哈希创建serdata对象
    .from_sample = serdata_default_from_sample_cdr,    // 从样本创建serdata对象
    .to_ser = serdata_default_to_ser,                  // 将serdata对象转换为序列化数据
    .to_sample = serdata_default_to_sample_cdr,        // 将serdata对象转换为样本
    .to_ser_ref = serdata_default_to_ser_ref,      // 获取serdata对象的序列化数据引用
    .to_ser_unref = serdata_default_to_ser_unref,  // 取消serdata对象的序列化数据引用
    .to_untyped = serdata_default_to_untyped,  // 将serdata对象转换为无类型serdata对象
    .untyped_to_sample = serdata_default_untyped_to_sample_cdr,  // 将无类型serdata对象转换为样本
    .print = serdata_default_print_cdr,                          // 打印serdata对象的信息
    .get_keyhash = serdata_default_get_keyhash                   // 获取serdata对象的键哈希
#ifdef DDS_HAS_SHM
    ,
    .get_sample_size = ddsi_serdata_iox_size,    // 获取共享内存中的样本大小
    .from_iox_buffer = serdata_default_from_iox  // 从共享内存缓冲区创建serdata对象
#endif
};

// 定义一个名为ddsi_serdata_ops_xcdr2的结构体常量，其类型为ddsi_serdata_ops
const struct ddsi_serdata_ops dds_serdata_ops_xcdr2 = {
    .get_size = serdata_default_get_size,          // 获取序列化数据的大小
    .eqkey = serdata_default_eqkey,                // 比较两个序列化数据的键是否相等
    .free = serdata_default_free,                  // 释放序列化数据占用的内存
    .from_ser = serdata_default_from_ser,          // 从序列化数据创建serdata对象
    .from_ser_iov = serdata_default_from_ser_iov,  // 从序列化数据的IO向量创建serdata对象
    .from_keyhash = serdata_default_from_keyhash_cdr,  // 从键哈希创建serdata对象
    .from_sample = serdata_default_from_sample_xcdr2,  // 从样本创建serdata对象
    .to_ser = serdata_default_to_ser,                  // 将serdata对象转换为序列化数据
    .to_sample = serdata_default_to_sample_cdr,        // 将serdata对象转换为样本
    .to_ser_ref = serdata_default_to_ser_ref,      // 获取serdata对象的序列化数据引用
    .to_ser_unref = serdata_default_to_ser_unref,  // 取消serdata对象的序列化数据引用
    .to_untyped = serdata_default_to_untyped,  // 将serdata对象转换为无类型serdata对象
    .untyped_to_sample = serdata_default_untyped_to_sample_cdr,  // 将无类型serdata对象转换为样本
    .print = serdata_default_print_cdr,                          // 打印serdata对象的信息
    .get_keyhash = serdata_default_get_keyhash                   // 获取serdata对象的键哈希
#ifdef DDS_HAS_SHM
    ,
    .get_sample_size = ddsi_serdata_iox_size,    // 获取共享内存中的样本大小
    .from_iox_buffer = serdata_default_from_iox  // 从共享内存缓冲区创建serdata对象
#endif
};

// 定义一个名为ddsi_serdata_ops_cdr_nokey的结构体常量，其类型为ddsi_serdata_ops
const struct ddsi_serdata_ops dds_serdata_ops_cdr_nokey = {
    .get_size = serdata_default_get_size,  // 获取序列化数据的大小
    .eqkey = serdata_default_eqkey_nokey,  // 比较两个序列化数据的键是否相等（不使用键）
    .free = serdata_default_free,                // 释放序列化数据占用的内存
    .from_ser = serdata_default_from_ser_nokey,  // 从序列化数据创建serdata对象（不使用键）
    .from_ser_iov =
        serdata_default_from_ser_iov_nokey,  // 从序列化数据的IO向量创建serdata对象（不使用键）
    .from_keyhash = serdata_default_from_keyhash_cdr_nokey,  // 从键哈希创建serdata对象（不使用键）
    .from_sample = serdata_default_from_sample_cdr_nokey,  // 从样本创建serdata对象（不使用键）
    .to_ser = serdata_default_to_ser,              // 将serdata对象转换为序列化数据
    .to_sample = serdata_default_to_sample_cdr,    // 将serdata对象转换为样本
    .to_ser_ref = serdata_default_to_ser_ref,      // 获取serdata对象的序列化数据引用
    .to_ser_unref = serdata_default_to_ser_unref,  // 取消serdata对象的序列化数据引用
    .to_untyped = serdata_default_to_untyped,  // 将serdata对象转换为无类型serdata对象
    .untyped_to_sample =
        serdata_default_untyped_to_sample_cdr_nokey,  // 将无类型serdata对象转换为样本（不使用键）
    .print = serdata_default_print_cdr,         // 打印serdata对象的信息
    .get_keyhash = serdata_default_get_keyhash  // 获取serdata对象的键哈希
#ifdef DDS_HAS_SHM
    ,
    .get_sample_size = ddsi_serdata_iox_size,    // 获取共享内存中的样本大小
    .from_iox_buffer = serdata_default_from_iox  // 从共享内存缓冲区创建serdata对象
#endif
};

// 定义一个名为ddsi_serdata_ops_xcdr2_nokey的结构体常量，其类型为ddsi_serdata_ops
const struct ddsi_serdata_ops dds_serdata_ops_xcdr2_nokey = {
    .get_size = serdata_default_get_size,  // 获取序列化数据的大小
    .eqkey = serdata_default_eqkey_nokey,  // 比较两个序列化数据的键是否相等（不使用键）
    .free = serdata_default_free,                // 释放序列化数据占用的内存
    .from_ser = serdata_default_from_ser_nokey,  // 从序列化数据创建serdata对象（不使用键）
    .from_ser_iov =
        serdata_default_from_ser_iov_nokey,  // 从序列化数据的IO向量创建serdata对象（不使用键）
    .from_keyhash = serdata_default_from_keyhash_cdr_nokey,  // 从键哈希创建serdata对象（不使用键）
    .from_sample = serdata_default_from_sample_xcdr2_nokey,  // 从样本创建serdata对象（不使用键）
    .to_ser = serdata_default_to_ser,              // 将serdata对象转换为序列化数据
    .to_sample = serdata_default_to_sample_cdr,    // 将serdata对象转换为样本
    .to_ser_ref = serdata_default_to_ser_ref,      // 获取serdata对象的序列化数据引用
    .to_ser_unref = serdata_default_to_ser_unref,  // 取消serdata对象的序列化数据引用
    .to_untyped = serdata_default_to_untyped,  // 将serdata对象转换为无类型serdata对象
    .untyped_to_sample =
        serdata_default_untyped_to_sample_cdr_nokey,  // 将无类型serdata对象转换为样本（不使用键）
    .print = serdata_default_print_cdr,         // 打印serdata对象的信息
    .get_keyhash = serdata_default_get_keyhash  // 获取serdata对象的键哈希
#ifdef DDS_HAS_SHM
    ,
    .get_sample_size = ddsi_serdata_iox_size,    // 获取共享内存中的样本大小
    .from_iox_buffer = serdata_default_from_iox  // 从共享内存缓冲区创建serdata对象
#endif
};
