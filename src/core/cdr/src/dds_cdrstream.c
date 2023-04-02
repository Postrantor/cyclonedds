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

#include "dds/cdr/dds_cdrstream.h"

#include <assert.h>
#include <ctype.h>
#include <string.h>

#include "dds/ddsrt/endian.h"
#include "dds/ddsrt/heap.h"
#include "dds/ddsrt/md5.h"
#include "dds/ddsrt/static_assert.h"
#include "dds/ddsrt/string.h"

// 定义TOKENPASTE宏，用于将两个参数连接成一个标识符
#define TOKENPASTE(a, b) a##b
// 定义TOKENPASTE2宏，用于将两个参数连接成一个标识符（通过TOKENPASTE实现）
#define TOKENPASTE2(a, b) TOKENPASTE(a, b)
// 定义TOKENPASTE3宏，用于将三个参数连接成一个标识符（通过TOKENPASTE2实现）
#define TOKENPASTE3(a, b, c) TOKENPASTE2(a, TOKENPASTE2(b, c))

// 定义NAME_BYTE_ORDER宏，用于在名称后添加字节序扩展名
#define NAME_BYTE_ORDER(name) TOKENPASTE2(name, NAME_BYTE_ORDER_EXT)
// 定义NAME2_BYTE_ORDER宏，用于在前缀和后缀之间添加字节序扩展名
#define NAME2_BYTE_ORDER(prefix, postfix) TOKENPASTE3(prefix, NAME_BYTE_ORDER_EXT, postfix)
// 定义DDS_OSTREAM_T宏，用于创建dds_ostream类型的别名，包含字节序扩展名
#define DDS_OSTREAM_T TOKENPASTE3(dds_ostream, NAME_BYTE_ORDER_EXT, _t)

// 定义EMHEADER_FLAG_MASK常量，用于表示标志位掩码
#define EMHEADER_FLAG_MASK 0x80000000u
// 定义EMHEADER_FLAG_MUSTUNDERSTAND常量，表示必须理解的标志位
#define EMHEADER_FLAG_MUSTUNDERSTAND (1u << 31)
// 定义EMHEADER_LENGTH_CODE_MASK常量，用于表示长度代码掩码
#define EMHEADER_LENGTH_CODE_MASK 0x70000000u
// 定义EMHEADER_LENGTH_CODE宏，用于从给定值中提取长度代码
#define EMHEADER_LENGTH_CODE(x) (((x)&EMHEADER_LENGTH_CODE_MASK) >> 28)
// 定义EMHEADER_MEMBERID_MASK常量，用于表示成员ID掩码
#define EMHEADER_MEMBERID_MASK 0x0fffffffu
// 定义EMHEADER_MEMBERID宏，用于从给定值中提取成员ID
#define EMHEADER_MEMBERID(x) ((x)&EMHEADER_MEMBERID_MASK)

/* 长度代码，定义在XTypes规范的7.4.3.4.2节。值4..7表示
 * 在EMHEADER之后的32位整数用于获取成员的长度。
 * 对于长度代码值4，仅为此目的添加此整数。
 * 对于值5..7，成员的长度被重复使用，该长度位于
 * 成员数据的第一个位置。
 */
#define LENGTH_CODE_1B 0
#define LENGTH_CODE_2B 1
#define LENGTH_CODE_4B 2
#define LENGTH_CODE_8B 3
#define LENGTH_CODE_NEXTINT 4
#define LENGTH_CODE_ALSO_NEXTINT 5
#define LENGTH_CODE_ALSO_NEXTINT4 6
#define LENGTH_CODE_ALSO_NEXTINT8 7

// 定义宏函数，将输入参数n原样返回
#define ddsrt_to2u(n) (n)
#define ddsrt_to4u(n) (n)
#define ddsrt_to8u(n) (n)
#define to_BO4u NAME2_BYTE_ORDER(ddsrt_to, 4u)

// 定义字节序相关的宏函数
#define dds_os_put1BO NAME_BYTE_ORDER(dds_os_put1)
#define dds_os_put2BO NAME_BYTE_ORDER(dds_os_put2)
#define dds_os_put4BO NAME_BYTE_ORDER(dds_os_put4)
#define dds_os_put8BO NAME_BYTE_ORDER(dds_os_put8)
#define dds_os_reserve4BO NAME_BYTE_ORDER(dds_os_reserve4)
#define dds_os_reserve8BO NAME_BYTE_ORDER(dds_os_reserve8)
#define dds_ostreamBO_fini NAME2_BYTE_ORDER(dds_ostream, _fini)
#define dds_stream_write_stringBO NAME_BYTE_ORDER(dds_stream_write_string)
#define dds_stream_write_seqBO NAME_BYTE_ORDER(dds_stream_write_seq)
#define dds_stream_write_arrBO NAME_BYTE_ORDER(dds_stream_write_arr)
#define dds_stream_write_bool_valueBO NAME_BYTE_ORDER(dds_stream_write_bool_value)
#define dds_stream_write_bool_arrBO NAME_BYTE_ORDER(dds_stream_write_bool_arr)
#define dds_stream_write_enum_valueBO NAME_BYTE_ORDER(dds_stream_write_enum_value)
#define dds_stream_write_enum_arrBO NAME_BYTE_ORDER(dds_stream_write_enum_arr)
#define dds_stream_write_bitmask_valueBO NAME_BYTE_ORDER(dds_stream_write_bitmask_value)
#define dds_stream_write_bitmask_arrBO NAME_BYTE_ORDER(dds_stream_write_bitmask_arr)
#define dds_stream_write_union_discriminantBO NAME_BYTE_ORDER(dds_stream_write_union_discriminant)
#define dds_stream_write_uniBO NAME_BYTE_ORDER(dds_stream_write_uni)
#define dds_stream_writeBO NAME_BYTE_ORDER(dds_stream_write)
#define dds_stream_write_implBO NAME_BYTE_ORDER(dds_stream_write_impl)
#define dds_stream_write_adrBO NAME_BYTE_ORDER(dds_stream_write_adr)
#define dds_stream_write_plBO NAME_BYTE_ORDER(dds_stream_write_pl)
#define dds_stream_write_pl_memberlistBO NAME_BYTE_ORDER(dds_stream_write_pl_memberlist)
#define dds_stream_write_pl_memberBO NAME_BYTE_ORDER(dds_stream_write_pl_member)
#define dds_stream_write_delimitedBO NAME_BYTE_ORDER(dds_stream_write_delimited)
#define dds_stream_write_keyBO NAME_BYTE_ORDER(dds_stream_write_key)
#define dds_stream_write_keyBO_impl NAME2_BYTE_ORDER(dds_stream_write_key, _impl)
#define dds_cdr_alignto_clear_and_resizeBO NAME_BYTE_ORDER(dds_cdr_alignto_clear_and_resize)
#define dds_stream_swap_if_needed_insituBO NAME_BYTE_ORDER(dds_stream_swap_if_needed_insitu)
#define dds_stream_to_BO_insitu NAME2_BYTE_ORDER(dds_stream_to_, _insitu)
#define dds_stream_extract_keyBO_from_data NAME2_BYTE_ORDER(dds_stream_extract_key, _from_data)
#define dds_stream_extract_keyBO_from_data1 NAME2_BYTE_ORDER(dds_stream_extract_key, _from_data1)
#define dds_stream_extract_keyBO_from_data_adr \
  NAME2_BYTE_ORDER(dds_stream_extract_key, _from_data_adr)
#define dds_stream_extract_keyBO_from_key_prim_op \
  NAME2_BYTE_ORDER(dds_stream_extract_key, _from_key_prim_op)
#define dds_stream_extract_keyBO_from_data_delimited \
  NAME2_BYTE_ORDER(dds_stream_extract_key, _from_data_delimited)
#define dds_stream_extract_keyBO_from_data_pl \
  NAME2_BYTE_ORDER(dds_stream_extract_key, _from_data_pl)
#define dds_stream_extract_keyBO_from_data_pl_member \
  NAME2_BYTE_ORDER(dds_stream_extract_key, _from_data_pl_member)
#define dds_stream_extract_keyBO_from_key NAME2_BYTE_ORDER(dds_stream_extract_key, _from_key)

/**
 * @file dds_cdrstream_keys.part.c
 *
 * 该文件包含用于处理 CDR 数据流的函数和结构。
 */

// 临时存储 CDR 中关键字段位置的类型，以及处理它所需的指令
struct key_off_info {
  uint32_t src_off;       /**< 源偏移量 */
  const uint32_t *op_off; /**< 操作偏移量指针 */
};

// 函数声明

/**
 * @brief 跳过地址操作
 *
 * @param[in] insn 指令
 * @param[in] ops 操作指针
 * @return 更新后的操作指针
 */
static const uint32_t *dds_stream_skip_adr(  //
    uint32_t insn,                           //
    const uint32_t *__restrict ops);

/**
 * @brief 跳过默认数据流
 *
 * @param[in,out] data 数据指针
 * @param[in] allocator 分配器指针
 * @param[in] ops 操作指针
 * @return 更新后的操作指针
 */
static const uint32_t *dds_stream_skip_default(                  //
    char *__restrict data,                                       //
    const struct dds_cdrstream_allocator *__restrict allocator,  //
    const uint32_t *__restrict ops);

/**
 * @brief 从数据中提取键值（第一种方法）
 *
 * @param[in] is 输入流指针
 * @param[out] os 输出流指针
 * @param[in] allocator 分配器指针
 * @param[in] ops_offs_idx 操作偏移索引
 * @param[in,out] ops_offs 操作偏移指针
 * @param[in] op0 操作 0 指针
 * @param[in] op0_type 操作 0 类型指针
 * @param[in] ops 操作指针
 * @param[in] mutable_member 可变成员标志
 * @param[in] mutable_member_or_parent 可变成员或父级标志
 * @param[in] n_keys 键数量
 * @param[in,out] keys_remaining 剩余键指针
 * @param[in] key 键描述指针
 * @param[out] key_offs 键偏移信息指针
 * @return 更新后的操作指针
 */
static const uint32_t *dds_stream_extract_key_from_data1(        //
    dds_istream_t *__restrict is,                                //
    dds_ostream_t *__restrict os,                                //
    const struct dds_cdrstream_allocator *__restrict allocator,  //
    uint32_t ops_offs_idx,                                       //
    uint32_t *__restrict ops_offs,                               //
    const uint32_t *const __restrict op0,                        //
    const uint32_t *const __restrict op0_type,                   //
    const uint32_t *__restrict ops,                              //
    bool mutable_member,                                         //
    bool mutable_member_or_parent,                               //
    uint32_t n_keys,                                             //
    uint32_t *__restrict keys_remaining,                         //
    const dds_cdrstream_desc_key_t *__restrict key,              //
    struct key_off_info *__restrict key_offs);

/**
 * @brief 从数据中提取键值（第一种方法，大端字节序）
 *
 * @param[in] is 输入流指针
 * @param[out] os 输出流指针
 * @param[in] allocator 分配器指针
 * @param[in] ops_offs_idx 操作偏移索引
 * @param[in,out] ops_offs 操作偏移指针
 * @param[in] op0 操作 0 指针
 * @param[in] op0_type 操作 0 类型指针
 * @param[in] ops 操作指针
 * @param[in] mutable_member 可变成员标志
 * @param[in] mutable_member_or_parent 可变成员或父级标志
 * @param[in] n_keys 键数量
 * @param[in,out] keys_remaining 剩余键指针
 * @param[in] key 键描述指针
 * @param[out] key_offs 键偏移信息指针
 * @return 更新后的操作指针
 */
static const uint32_t *dds_stream_extract_keyBE_from_data1(      //
    dds_istream_t *__restrict is,                                //
    dds_ostreamBE_t *__restrict os,                              //
    const struct dds_cdrstream_allocator *__restrict allocator,  //
    uint32_t ops_offs_idx,                                       //
    uint32_t *__restrict ops_offs,                               //
    const uint32_t *const __restrict op0,                        //
    const uint32_t *const __restrict op0_type,                   //
    const uint32_t *__restrict ops,                              //
    bool mutable_member,                                         //
    bool mutable_member_or_parent,                               //
    uint32_t n_keys,                                             //
    uint32_t *__restrict keys_remaining,                         //
    const dds_cdrstream_desc_key_t *__restrict key,              //
    struct key_off_info *__restrict key_offs);

/**
 * @brief 数据流规范化实现
 *
 * @param[in,out] data 数据指针
 * @param[in,out] off 偏移量指针
 * @param[in] size 大小
 * @param[in] bswap 字节交换标志
 * @param[in] xcdr_version XCDR 版本
 * @param[in] ops 操作指针
 * @param[in] is_mutable_member 是否为可变成员
 * @return 更新后的操作指针
 */
static const uint32_t *stream_normalize_data_impl(  //
    char *__restrict data,                          //
    uint32_t *__restrict off,                       //
    uint32_t size,                                  //
    bool bswap,                                     //
    uint32_t xcdr_version,                          //
    const uint32_t *__restrict ops,                 //
    bool is_mutable_member) ddsrt_attribute_warn_unused_result ddsrt_nonnull_all;

/**
 * @brief 读取数据流实现
 *
 * @param[in] is 输入流指针
 * @param[out] data 数据指针
 * @param[in] allocator 分配器指针
 * @param[in] ops 操作指针
 * @param[in] is_mutable_member 是否为可变成员
 * @return 更新后的操作指针
 */
static const uint32_t *dds_stream_read_impl(                     //
    dds_istream_t *__restrict is,                                //
    char *__restrict data,                                       //
    const struct dds_cdrstream_allocator *__restrict allocator,  //
    const uint32_t *__restrict ops,                              //
    bool is_mutable_member);

/**
 * @brief 释放样本地址操作
 *
 * @param[in] insn 指令
 * @param[in,out] data 数据指针
 * @param[in] allocator 分配器指针
 * @param[in] ops 操作指针
 * @return 更新后的操作指针
 */
static const uint32_t *stream_free_sample_adr(                   //
    uint32_t insn,                                               //
    void *__restrict data,                                       //
    const struct dds_cdrstream_allocator *__restrict allocator,  //
    const uint32_t *__restrict ops);

// 对齐宏定义
#ifndef NDEBUG
typedef struct align {
  uint32_t a; /**< 对齐值 */
} align_t;
#define ALIGN(n) ((n).a)
#else
typedef uint32_t align_t; /**< 对齐类型 */
#define ALIGN(n) (n) /**< 对齐宏 */
#endif

/**
 * @brief 获取对齐值的函数
 *
 * 根据给定的CDR版本和大小，计算并返回对齐值。
 *
 * @param xcdr_version CDR版本（例如：DDSI_RTPS_CDR_ENC_VERSION_2）
 * @param size 需要对齐的数据大小
 * @return 返回对齐值
 */
static inline align_t dds_cdr_get_align(uint32_t xcdr_version, uint32_t size) {
  // 在非调试模式下，定义MK_ALIGN宏，用于简化代码
#ifndef NDEBUG
#define MK_ALIGN(n) \
  (struct align) { (n) }
#else
  // 在调试模式下，定义MK_ALIGN宏，直接返回对齐值
#define MK_ALIGN(n) (n)
#endif

  // 如果数据大小大于4字节
  if (size > 4)
    // 根据CDR版本选择对齐值，如果是DDSI_RTPS_CDR_ENC_VERSION_2，则对齐值为4，否则为8
    return xcdr_version == DDSI_RTPS_CDR_ENC_VERSION_2 ? MK_ALIGN(4) : MK_ALIGN(8);

  // 如果数据大小小于等于4字节，直接返回对齐值
  return MK_ALIGN(size);

  // 取消MK_ALIGN宏定义
#undef MK_ALIGN
}

/**
 * @brief 使dds_ostream_t结构体的缓冲区增长到指定大小。
 *
 * @param[in] os 指向dds_ostream_t结构体的指针，该结构体包含缓冲区信息。
 * @param[in] allocator 指向dds_cdrstream_allocator结构体的指针，用于重新分配内存。
 * @param[in] size 需要增加的缓冲区大小。
 */
static void dds_ostream_grow(dds_ostream_t *__restrict os,
                             const struct dds_cdrstream_allocator *__restrict allocator,
                             uint32_t size) {
  // 计算需要的总大小
  uint32_t needed = size + os->m_index;

  // 在4k边界上重新分配内存
  uint32_t new_size = (needed & ~(uint32_t)0xfff) + 0x1000;
  uint8_t *old = os->m_buffer;

  // 更新缓冲区和大小
  os->m_buffer = allocator->realloc(old, new_size);
  os->m_size = new_size;
}

/**
 * @brief 从给定的缓冲区和大小创建一个dds_ostream_t结构体。
 *
 * @param[in] buffer 输入缓冲区的指针。
 * @param[in] size 缓冲区的大小。
 * @param[in] write_encoding_version 写入编码版本。
 * @return 返回初始化后的dds_ostream_t结构体。
 */
dds_ostream_t dds_ostream_from_buffer(void *buffer, size_t size, uint16_t write_encoding_version) {
  dds_ostream_t os;
  os.m_buffer = buffer;
  os.m_size = (uint32_t)size;
  os.m_index = 0;
  os.m_xcdr_version = write_encoding_version;
  return os;
}

/**
 * @brief 调整dds_ostream_t结构体的缓冲区大小。
 *
 * @param[in] os 指向dds_ostream_t结构体的指针，该结构体包含缓冲区信息。
 * @param[in] allocator 指向dds_cdrstream_allocator结构体的指针，用于重新分配内存。
 * @param[in] l 需要调整到的新大小。
 */
static void dds_cdr_resize(dds_ostream_t *__restrict os,
                           const struct dds_cdrstream_allocator *__restrict allocator,
                           uint32_t l) {
  // 如果当前大小不足以容纳新大小，则增长缓冲区
  if (os->m_size < l + os->m_index) dds_ostream_grow(os, allocator, l);
}

/**
 * @brief 初始化dds_istream_t结构体。
 *
 * @param[out] is 指向dds_istream_t结构体的指针，该结构体将被初始化。
 * @param[in] size 输入缓冲区的大小。
 * @param[in] input 输入缓冲区的指针。
 * @param[in] xcdr_version 编码版本。
 */
void dds_istream_init(dds_istream_t *__restrict is,
                      uint32_t size,
                      const void *__restrict input,
                      uint32_t xcdr_version) {
  is->m_buffer = input;
  is->m_size = size;
  is->m_index = 0;
  is->m_xcdr_version = xcdr_version;
}

/**
 * @brief 初始化dds_ostream_t结构体
 *
 * @param os            [out] 指向dds_ostream_t结构体的指针
 * @param allocator     [in]  指向dds_cdrstream_allocator结构体的指针，用于内存分配
 * @param size          [in]  缓冲区大小
 * @param xcdr_version  [in]  xcdr版本
 */
void dds_ostream_init(dds_ostream_t *__restrict os,
                      const struct dds_cdrstream_allocator *__restrict allocator,
                      uint32_t size,
                      uint32_t xcdr_version) {
  os->m_buffer = NULL;                  // 初始化缓冲区指针为空
  os->m_size = 0;                       // 初始化缓冲区大小为0
  os->m_index = 0;                      // 初始化索引值为0
  os->m_xcdr_version = xcdr_version;    // 设置xcdr版本
  dds_cdr_resize(os, allocator, size);  // 调整缓冲区大小
}

/**
 * @brief 初始化dds_ostreamLE_t结构体
 *
 * @param os            [out] 指向dds_ostreamLE_t结构体的指针
 * @param allocator     [in]  指向dds_cdrstream_allocator结构体的指针，用于内存分配
 * @param size          [in]  缓冲区大小
 * @param xcdr_version  [in]  xcdr版本
 */
void dds_ostreamLE_init(dds_ostreamLE_t *__restrict os,
                        const struct dds_cdrstream_allocator *__restrict allocator,
                        uint32_t size,
                        uint32_t xcdr_version) {
  dds_ostream_init(&os->x, allocator, size, xcdr_version);  // 初始化dds_ostream_t结构体
}

/**
 * @brief 初始化dds_ostreamBE_t结构体
 *
 * @param os            [out] 指向dds_ostreamBE_t结构体的指针
 * @param allocator     [in]  指向dds_cdrstream_allocator结构体的指针，用于内存分配
 * @param size          [in]  缓冲区大小
 * @param xcdr_version  [in]  xcdr版本
 */
void dds_ostreamBE_init(dds_ostreamBE_t *__restrict os,
                        const struct dds_cdrstream_allocator *__restrict allocator,
                        uint32_t size,
                        uint32_t xcdr_version) {
  dds_ostream_init(&os->x, allocator, size, xcdr_version);  // 初始化dds_ostream_t结构体
}

/**
 * @brief 释放dds_istream_t结构体资源
 *
 * @param is  [in] 指向dds_istream_t结构体的指针
 */
void dds_istream_fini(dds_istream_t *__restrict is) {
  (void)is;  // 不需要释放资源
}

/**
 * @brief 释放dds_ostream_t结构体资源
 *
 * @param os        [in] 指向dds_ostream_t结构体的指针
 * @param allocator [in] 指向dds_cdrstream_allocator结构体的指针，用于内存释放
 */
void dds_ostream_fini(dds_ostream_t *__restrict os,
                      const struct dds_cdrstream_allocator *__restrict allocator) {
  if (os->m_size)                   // 如果缓冲区大小不为0
    allocator->free(os->m_buffer);  // 释放缓冲区内存
}

/**
 * @brief 释放dds_ostreamLE_t结构体资源
 *
 * @param os        [in] 指向dds_ostreamLE_t结构体的指针
 * @param allocator [in] 指向dds_cdrstream_allocator结构体的指针，用于内存释放
 */
void dds_ostreamLE_fini(dds_ostreamLE_t *__restrict os,
                        const struct dds_cdrstream_allocator *__restrict allocator) {
  dds_ostream_fini(&os->x, allocator);  // 释放dds_ostream_t结构体资源
}

/**
 * @brief 释放dds_ostreamBE_t结构体资源
 *
 * @param os        [in] 指向dds_ostreamBE_t结构体的指针
 * @param allocator [in] 指向dds_cdrstream_allocator结构体的指针，用于内存释放
 */
void dds_ostreamBE_fini(dds_ostreamBE_t *__restrict os,
                        const struct dds_cdrstream_allocator *__restrict allocator) {
  dds_ostream_fini(&os->x, allocator);  // 释放dds_ostream_t结构体资源
}

/**
 * @brief 对齐输入流到指定的对齐值
 *
 * @param is 输入流指针，不可为空
 * @param a 对齐值
 */
static void dds_cdr_alignto(dds_istream_t *__restrict is, align_t a) {
  // 计算并更新输入流索引以实现对齐
  is->m_index = (is->m_index + ALIGN(a) - 1) & ~(ALIGN(a) - 1);
  // 断言输入流索引小于输入流大小
  assert(is->m_index < is->m_size);
}

/**
 * @brief 清除并调整输出流的大小以适应指定的对齐值
 *
 * @param os 输出流指针，不可为空
 * @param allocator 分配器指针，不可为空
 * @param a 对齐值
 * @param extra 额外的大小
 * @return uint32_t 填充字节数
 */
static uint32_t dds_cdr_alignto_clear_and_resize(
    dds_ostream_t *__restrict os,
    const struct dds_cdrstream_allocator *__restrict allocator,
    align_t a,
    uint32_t extra) {
  // 计算输出流索引与对齐值的余数
  const uint32_t m = os->m_index % ALIGN(a);
  if (m == 0) {
    // 如果余数为0，则直接调整输出流大小
    dds_cdr_resize(os, allocator, extra);
    return 0;
  } else {
    // 否则，计算需要填充的字节数
    const uint32_t pad = ALIGN(a) - m;
    // 调整输出流大小并填充指定的字节数
    dds_cdr_resize(os, allocator, pad + extra);
    for (uint32_t i = 0; i < pad; i++) os->m_buffer[os->m_index++] = 0;
    return pad;
  }
}

/**
 * @brief 清除并调整大端输出流的大小以适应指定的对齐值
 *
 * @param os 大端输出流指针，不可为空
 * @param allocator 分配器指针，不可为空
 * @param a 对齐值
 * @param extra 额外的大小
 * @return uint32_t 填充字节数
 */
static uint32_t dds_cdr_alignto_clear_and_resizeBE(
    dds_ostreamBE_t *__restrict os,
    const struct dds_cdrstream_allocator *__restrict allocator,
    align_t a,
    uint32_t extra) {
  return dds_cdr_alignto_clear_and_resize(&os->x, allocator, a, extra);
}

/**
 * @brief 对齐到4字节，清除并调整输出流的大小
 *
 * @param os 输出流指针
 * @param allocator 分配器指针
 * @param xcdr_version xcdr版本
 * @return uint32_t 返回调整后的大小
 */
uint32_t dds_cdr_alignto4_clear_and_resize(
    dds_ostream_t *__restrict os,
    const struct dds_cdrstream_allocator *__restrict allocator,
    uint32_t xcdr_version) {
  // 调用对齐、清除和调整大小的函数
  return dds_cdr_alignto_clear_and_resize(os, allocator, dds_cdr_get_align(xcdr_version, 4), 0);
}

/**
 * @brief 从输入流中获取1字节数据
 *
 * @param is 输入流指针
 * @return uint8_t 返回读取到的1字节数据
 */
static uint8_t dds_is_get1(dds_istream_t *__restrict is) {
  // 检查索引是否在范围内
  assert(is->m_index < is->m_size);
  // 获取1字节数据
  uint8_t v = *(is->m_buffer + is->m_index);
  // 索引递增
  is->m_index++;
  // 返回读取到的数据
  return v;
}

/**
 * @brief 从输入流中获取2字节数据
 *
 * @param is 输入流指针
 * @return uint16_t 返回读取到的2字节数据
 */
static uint16_t dds_is_get2(dds_istream_t *__restrict is) {
  // 对齐输入流
  dds_cdr_alignto(is, dds_cdr_get_align(is->m_xcdr_version, 2));
  // 获取2字节数据
  uint16_t v = *((uint16_t *)(is->m_buffer + is->m_index));
  // 索引递增2
  is->m_index += 2;
  // 返回读取到的数据
  return v;
}

/**
 * @brief 从输入流中获取4字节数据
 *
 * @param is 输入流指针
 * @return uint32_t 返回读取到的4字节数据
 */
static uint32_t dds_is_get4(dds_istream_t *__restrict is) {
  // 对齐输入流
  dds_cdr_alignto(is, dds_cdr_get_align(is->m_xcdr_version, 4));
  // 获取4字节数据
  uint32_t v = *((uint32_t *)(is->m_buffer + is->m_index));
  // 索引递增4
  is->m_index += 4;
  // 返回读取到的数据
  return v;
}

/**
 * @brief 预览输入流中的4字节数据，但不移动索引
 *
 * @param is 输入流指针
 * @return uint32_t 返回预览到的4字节数据
 */
static uint32_t dds_is_peek4(dds_istream_t *__restrict is) {
  // 对齐输入流
  dds_cdr_alignto(is, dds_cdr_get_align(is->m_xcdr_version, 4));
  // 预览4字节数据
  uint32_t v = *((uint32_t *)(is->m_buffer + is->m_index));
  // 返回预览到的数据，不移动索引
  return v;
}

/**
 * @brief 从输入流中获取8字节无符号整数
 * @param is 输入流指针
 * @return 返回8字节无符号整数值
 */
static uint64_t dds_is_get8(dds_istream_t *__restrict is) {
  // 对齐输入流
  dds_cdr_alignto(is, dds_cdr_get_align(is->m_xcdr_version, 8));

  // 根据系统的字节序设置偏移量
  size_t off_low = (DDSRT_ENDIAN == DDSRT_LITTLE_ENDIAN) ? 0 : 4, off_high = 4 - off_low;

  // 获取低32位和高32位的值
  uint32_t v_low = *((uint32_t *)(is->m_buffer + is->m_index + off_low)),
           v_high = *((uint32_t *)(is->m_buffer + is->m_index + off_high));

  // 合并成64位整数
  uint64_t v = (uint64_t)v_high << 32 | v_low;

  // 更新索引
  is->m_index += 8;

  // 返回结果
  return v;
}

/**
 * @brief 从输入流中获取指定数量的元素
 * @param is 输入流指针
 * @param b 目标缓冲区指针
 * @param num 要获取的元素数量
 * @param elem_size 元素大小（字节）
 */
static void dds_is_get_bytes(dds_istream_t *__restrict is,
                             void *__restrict b,
                             uint32_t num,
                             uint32_t elem_size) {
  // 对齐输入流
  dds_cdr_alignto(is, dds_cdr_get_align(is->m_xcdr_version, elem_size));

  // 复制数据到目标缓冲区
  memcpy(b, is->m_buffer + is->m_index, num * elem_size);

  // 更新索引
  is->m_index += num * elem_size;
}

/**
 * @brief 向输出流中写入1字节无符号整数
 * @param os 输出流指针
 * @param allocator 内存分配器指针
 * @param v 要写入的值
 */
static void dds_os_put1(dds_ostream_t *__restrict os,
                        const struct dds_cdrstream_allocator *__restrict allocator,
                        uint8_t v) {
  // 调整输出流大小
  dds_cdr_resize(os, allocator, 1);

  // 写入值
  *((uint8_t *)(os->m_buffer + os->m_index)) = v;

  // 更新索引
  os->m_index += 1;
}

/**
 * @brief 向输出流中写入2字节无符号整数
 * @param os 输出流指针
 * @param allocator 内存分配器指针
 * @param v 要写入的值
 */
static void dds_os_put2(dds_ostream_t *__restrict os,
                        const struct dds_cdrstream_allocator *__restrict allocator,
                        uint16_t v) {
  // 对齐、清除并调整输出流大小
  dds_cdr_alignto_clear_and_resize(os, allocator, dds_cdr_get_align(os->m_xcdr_version, 2), 2);

  // 写入值
  *((uint16_t *)(os->m_buffer + os->m_index)) = v;

  // 更新索引
  os->m_index += 2;
}

/**
 * @brief 向输出流中写入4字节无符号整数
 * @param os 输出流指针
 * @param allocator 内存分配器指针
 * @param v 要写入的值
 */
static void dds_os_put4(dds_ostream_t *__restrict os,
                        const struct dds_cdrstream_allocator *__restrict allocator,
                        uint32_t v) {
  // 对齐、清除并调整输出流大小
  dds_cdr_alignto_clear_and_resize(os, allocator, dds_cdr_get_align(os->m_xcdr_version, 4), 4);

  // 写入值
  *((uint32_t *)(os->m_buffer + os->m_index)) = v;

  // 更新索引
  os->m_index += 4;
}

/**
 * @brief 向输出流中写入8字节无符号整数
 * @param os 输出流指针
 * @param allocator 内存分配器指针
 * @param v 要写入的值
 */
static void dds_os_put8(dds_ostream_t *__restrict os,
                        const struct dds_cdrstream_allocator *__restrict allocator,
                        uint64_t v) {
  // 对齐、清除并调整输出流大小
  dds_cdr_alignto_clear_and_resize(os, allocator, dds_cdr_get_align(os->m_xcdr_version, 8), 8);

  // 根据系统的字节序设置偏移量
  size_t off_low = (DDSRT_ENDIAN == DDSRT_LITTLE_ENDIAN) ? 0 : 4, off_high = 4 - off_low;

  // 写入低32位和高32位的值
  *((uint32_t *)(os->m_buffer + os->m_index + off_low)) = (uint32_t)v;
  *((uint32_t *)(os->m_buffer + os->m_index + off_high)) = (uint32_t)(v >> 32);

  // 更新索引
  os->m_index += 8;
}

/**
 * @brief 在输出流中预留4字节空间
 * @param os 输出流指针
 * @param allocator 内存分配器指针
 * @return 返回预留空间后的索引
 */
static uint32_t dds_os_reserve4(dds_ostream_t *__restrict os,
                                const struct dds_cdrstream_allocator *__restrict allocator) {
  // 对齐、清除并调整输出流大小
  dds_cdr_alignto_clear_and_resize(os, allocator, dds_cdr_get_align(os->m_xcdr_version, 4), 4);

  // 更新索引
  os->m_index += 4;

  // 返回索引
  return os->m_index;
}

/**
 * @brief 在输出流中预留8字节空间
 * @param os 输出流指针
 * @param allocator 内存分配器指针
 * @return 返回预留空间后的索引
 */
static uint32_t dds_os_reserve8(dds_ostream_t *__restrict os,
                                const struct dds_cdrstream_allocator *__restrict allocator) {
  // 对齐、清除并调整输出流大小
  dds_cdr_alignto_clear_and_resize(os, allocator, dds_cdr_get_align(os->m_xcdr_version, 8), 8);

  // 更新索引
  os->m_index += 8;

  // 返回索引
  return os->m_index;
}

/**
 * @brief 向小端字节序的输出流中写入一个8位无符号整数
 * @param os 指向dds_ostreamLE_t结构体的指针
 * @param allocator 指向dds_cdrstream_allocator结构体的指针
 * @param v 要写入的8位无符号整数值
 */
static void dds_os_put1LE(dds_ostreamLE_t *__restrict os,
                          const struct dds_cdrstream_allocator *__restrict allocator,
                          uint8_t v) {
  dds_os_put1(&os->x, allocator, v);  // 调用dds_os_put1函数进行实际的写入操作
}

/**
 * @brief 向小端字节序的输出流中写入一个16位无符号整数
 * @param os 指向dds_ostreamLE_t结构体的指针
 * @param allocator 指向dds_cdrstream_allocator结构体的指针
 * @param v 要写入的16位无符号整数值
 */
static void dds_os_put2LE(dds_ostreamLE_t *__restrict os,
                          const struct dds_cdrstream_allocator *__restrict allocator,
                          uint16_t v) {
  dds_os_put2(&os->x, allocator,
              ddsrt_toLE2u(v));  // 转换为小端字节序并调用dds_os_put2函数进行实际的写入操作
}

/**
 * @brief 向小端字节序的输出流中写入一个32位无符号整数
 * @param os 指向dds_ostreamLE_t结构体的指针
 * @param allocator 指向dds_cdrstream_allocator结构体的指针
 * @param v 要写入的32位无符号整数值
 */
static void dds_os_put4LE(dds_ostreamLE_t *__restrict os,
                          const struct dds_cdrstream_allocator *__restrict allocator,
                          uint32_t v) {
  dds_os_put4(&os->x, allocator,
              ddsrt_toLE4u(v));  // 转换为小端字节序并调用dds_os_put4函数进行实际的写入操作
}

/**
 * @brief 向小端字节序的输出流中写入一个64位无符号整数
 * @param os 指向dds_ostreamLE_t结构体的指针
 * @param allocator 指向dds_cdrstream_allocator结构体的指针
 * @param v 要写入的64位无符号整数值
 */
static void dds_os_put8LE(dds_ostreamLE_t *__restrict os,
                          const struct dds_cdrstream_allocator *__restrict allocator,
                          uint64_t v) {
  dds_os_put8(&os->x, allocator,
              ddsrt_toLE8u(v));  // 转换为小端字节序并调用dds_os_put8函数进行实际的写入操作
}

/**
 * @brief 在小端字节序的输出流中预留4字节空间
 * @param os 指向dds_ostreamLE_t结构体的指针
 * @param allocator 指向dds_cdrstream_allocator结构体的指针
 * @return 返回预留空间的偏移量
 */
static uint32_t dds_os_reserve4LE(dds_ostreamLE_t *__restrict os,
                                  const struct dds_cdrstream_allocator *__restrict allocator) {
  return dds_os_reserve4(&os->x, allocator);  // 调用dds_os_reserve4函数进行实际的预留操作
}

/**
 * @brief 在小端字节序的输出流中预留8字节空间
 * @param os 指向dds_ostreamLE_t结构体的指针
 * @param allocator 指向dds_cdrstream_allocator结构体的指针
 * @return 返回预留空间的偏移量
 */
static uint32_t dds_os_reserve8LE(dds_ostreamLE_t *__restrict os,
                                  const struct dds_cdrstream_allocator *__restrict allocator) {
  return dds_os_reserve8(&os->x, allocator);  // 调用dds_os_reserve8函数进行实际的预留操作
}

/**
 * @brief 向大端字节序的输出流中写入一个8位无符号整数
 * @param os 指向dds_ostreamBE_t结构体的指针
 * @param allocator 指向dds_cdrstream_allocator结构体的指针
 * @param v 要写入的8位无符号整数值
 */
static void dds_os_put1BE(dds_ostreamBE_t *__restrict os,
                          const struct dds_cdrstream_allocator *__restrict allocator,
                          uint8_t v) {
  dds_os_put1(&os->x, allocator, v);  // 调用dds_os_put1函数进行实际的写入操作
}

/**
 * @brief 向大端字节序的输出流中写入一个16位无符号整数
 * @param os 指向dds_ostreamBE_t结构体的指针
 * @param allocator 指向dds_cdrstream_allocator结构体的指针
 * @param v 要写入的16位无符号整数值
 */
static void dds_os_put2BE(dds_ostreamBE_t *__restrict os,
                          const struct dds_cdrstream_allocator *__restrict allocator,
                          uint16_t v) {
  dds_os_put2(&os->x, allocator,
              ddsrt_toBE2u(v));  // 转换为大端字节序并调用dds_os_put2函数进行实际的写入操作
}

/**
 * @brief 向大端字节序的输出流中写入一个32位无符号整数
 * @param os 指向dds_ostreamBE_t结构体的指针
 * @param allocator 指向dds_cdrstream_allocator结构体的指针
 * @param v 要写入的32位无符号整数值
 */
static void dds_os_put4BE(dds_ostreamBE_t *__restrict os,
                          const struct dds_cdrstream_allocator *__restrict allocator,
                          uint32_t v) {
  dds_os_put4(&os->x, allocator,
              ddsrt_toBE4u(v));  // 转换为大端字节序并调用dds_os_put4函数进行实际的写入操作
}

/**
 * @brief 向大端字节序的输出流中写入一个64位无符号整数
 * @param os 指向dds_ostreamBE_t结构体的指针
 * @param allocator 指向dds_cdrstream_allocator结构体的指针
 * @param v 要写入的64位无符号整数值
 */
static void dds_os_put8BE(dds_ostreamBE_t *__restrict os,
                          const struct dds_cdrstream_allocator *__restrict allocator,
                          uint64_t v) {
  dds_os_put8(&os->x, allocator,
              ddsrt_toBE8u(v));  // 转换为大端字节序并调用dds_os_put8函数进行实际的写入操作
}

/**
 * @brief 在大端字节序的输出流中预留4字节空间
 * @param os 指向dds_ostreamBE_t结构体的指针
 * @param allocator 指向dds_cdrstream_allocator结构体的指针
 * @return 返回预留空间的偏移量
 */
static uint32_t dds_os_reserve4BE(dds_ostreamBE_t *__restrict os,
                                  const struct dds_cdrstream_allocator *__restrict allocator) {
  return dds_os_reserve4(&os->x, allocator);  // 调用dds_os_reserve4函数进行实际的预留操作
}

/**
 * @brief 在大端字节序的输出流中预留8字节空间
 * @param os 指向dds_ostreamBE_t结构体的指针
 * @param allocator 指向dds_cdrstream_allocator结构体的指针
 * @return 返回预留空间的偏移量
 */
static uint32_t dds_os_reserve8BE(dds_ostreamBE_t *__restrict os,
                                  const struct dds_cdrstream_allocator *__restrict allocator) {
  return dds_os_reserve8(&os->x, allocator);  // 调用dds_os_reserve8函数进行实际的预留操作
}

/**
 * @brief 交换字节序的函数
 *
 * 根据给定的大小，将缓冲区中的数据进行字节序交换。
 *
 * @param[in] __restrict vbuf 指向要交换字节序的缓冲区的指针
 * @param[in] size 要交换的数据的大小（以字节为单位），可以是1、2、4或8
 * @param[in] num 缓冲区中要交换的数据的数量
 */
static void dds_stream_swap(void *__restrict vbuf, uint32_t size, uint32_t num) {
  // 断言：确保 size 的值只能是1、2、4或8
  assert(size == 1 || size == 2 || size == 4 || size == 8);

  // 根据 size 的值选择相应的处理方式
  switch (size) {
    case 1:
      // 如果 size 为1，则不需要交换字节序，直接退出
      break;
    case 2: {
      // 将 void 类型指针转换为 uint16_t 类型指针
      uint16_t *buf = vbuf;

      // 遍历缓冲区中的每个数据，并交换其字节序
      for (uint32_t i = 0; i < num; i++) buf[i] = ddsrt_bswap2u(buf[i]);
      break;
    }
    case 4: {
      // 将 void 类型指针转换为 uint32_t 类型指针
      uint32_t *buf = vbuf;

      // 遍历缓冲区中的每个数据，并交换其字节序
      for (uint32_t i = 0; i < num; i++) buf[i] = ddsrt_bswap4u(buf[i]);
      break;
    }
    case 8: {
      // 将 void 类型指针转换为 uint64_t 类型指针
      uint64_t *buf = vbuf;

      // 遍历缓冲区中的每个数据，并交换其字节序
      for (uint32_t i = 0; i < num; i++) buf[i] = ddsrt_bswap8u(buf[i]);
      break;
    }
  }
}

/**
 * @brief 将字节放入输出流中
 *
 * @param[in] os 输出流指针
 * @param[in] allocator CDR流分配器指针
 * @param[in] b 要写入的字节数据指针
 * @param[in] l 要写入的字节数量
 */
static void dds_os_put_bytes(dds_ostream_t *__restrict os,
                             const struct dds_cdrstream_allocator *__restrict allocator,
                             const void *__restrict b,
                             uint32_t l) {
  dds_cdr_resize(os, allocator, l);          // 调整输出流大小以适应新的字节数据
  memcpy(os->m_buffer + os->m_index, b, l);  // 将字节数据复制到输出流缓冲区中
  os->m_index += l;                          // 更新输出流索引
}

/**
 * @brief 将对齐的字节放入输出流中
 *
 * @param[in] os 输出流指针
 * @param[in] allocator CDR流分配器指针
 * @param[in] data 要写入的字节数据指针
 * @param[in] num 要写入的元素数量
 * @param[in] elem_sz 每个元素的大小（字节）
 * @param[in] align 对齐方式
 * @param[out] dst 目标地址指针
 */
static void dds_os_put_bytes_aligned(dds_ostream_t *__restrict os,
                                     const struct dds_cdrstream_allocator *__restrict allocator,
                                     const void *__restrict data,
                                     uint32_t num,
                                     uint32_t elem_sz,
                                     align_t align,
                                     void **dst) {
  const uint32_t sz = num * elem_sz;                           // 计算总字节数
  dds_cdr_alignto_clear_and_resize(os, allocator, align, sz);  // 调整输出流大小并进行对齐
  if (dst) *dst = os->m_buffer + os->m_index;                  // 设置目标地址
  memcpy(os->m_buffer + os->m_index, data, sz);  // 将字节数据复制到输出流缓冲区中
  os->m_index += sz;                             // 更新输出流索引
}

/**
 * @brief 判断类型是否为原始类型
 *
 * @param[in] type 类型代码
 * @return 如果是原始类型，返回true；否则返回false
 */
static inline bool is_primitive_type(enum dds_stream_typecode type) {
  return type <= DDS_OP_VAL_8BY || type == DDS_OP_VAL_BLN;
}

#ifndef NDEBUG
/**
 * @brief 判断类型是否为原始类型或枚举类型
 *
 * @param[in] type 类型代码
 * @return 如果是原始类型或枚举类型，返回true；否则返回false
 */
static inline bool is_primitive_or_enum_type(enum dds_stream_typecode type) {
  return is_primitive_type(type) || type == DDS_OP_VAL_ENU;
}
#endif

/**
 * @brief 判断是否需要DHeader
 *
 * @param[in] type 类型代码
 * @param[in] xcdrv CDR编码版本
 * @return 如果需要DHeader，返回true；否则返回false
 */
static inline bool is_dheader_needed(enum dds_stream_typecode type, uint32_t xcdrv) {
  return !is_primitive_type(type) && xcdrv == DDSI_RTPS_CDR_ENC_VERSION_2;
}

/**
 * @brief 获取原始类型的大小
 *
 * @param[in] type 类型代码
 * @return 原始类型的大小（字节）
 */
static uint32_t get_primitive_size(enum dds_stream_typecode type) {
  DDSRT_STATIC_ASSERT(DDS_OP_VAL_1BY == 1 && DDS_OP_VAL_2BY == 2 && DDS_OP_VAL_4BY == 3 &&
                      DDS_OP_VAL_8BY == 4);
  assert(is_primitive_type(type));
  return type == DDS_OP_VAL_BLN ? 1 : (uint32_t)1 << ((uint32_t)type - 1);
}

/**
 * @brief 获取集合元素的大小
 *
 * @param[in] insn 指令
 * @param[in] __restrict ops 受限操作指针
 * @return 返回集合元素的大小（以字节为单位）
 */
static uint32_t get_collection_elem_size(uint32_t insn, const uint32_t *__restrict ops) {
  // 根据指令的子类型进行判断
  switch (DDS_OP_SUBTYPE(insn)) {
    case DDS_OP_VAL_BLN:
    case DDS_OP_VAL_1BY:
    case DDS_OP_VAL_2BY:
    case DDS_OP_VAL_4BY:
    case DDS_OP_VAL_8BY:
      // 获取基本类型的大小
      return get_primitive_size(DDS_OP_SUBTYPE(insn));
    case DDS_OP_VAL_ENU:
      // 枚举类型的大小
      return sizeof(uint32_t);
    case DDS_OP_VAL_BMK:
      // 获取位掩码类型的大小
      return DDS_OP_TYPE_SZ(insn);
    case DDS_OP_VAL_STR:
      // 字符串类型的大小
      return sizeof(char *);
    case DDS_OP_VAL_BST:
    case DDS_OP_VAL_SEQ:
    case DDS_OP_VAL_BSQ:
    case DDS_OP_VAL_ARR:
    case DDS_OP_VAL_UNI:
    case DDS_OP_VAL_STU:
      if (DDS_OP_TYPE(insn) == DDS_OP_VAL_ARR)
        // 数组类型的大小
        return ops[4];
      break;
    case DDS_OP_VAL_EXT:
      // 扩展类型不处理
      break;
  }
  // 其他情况，终止程序
  abort();
}

/**
 * @brief 获取地址类型的大小
 *
 * @param[in] insn 指令
 * @param[in] __restrict ops 受限操作指针
 * @return 返回地址类型的大小（以字节为单位）
 */
static uint32_t get_adr_type_size(uint32_t insn, const uint32_t *__restrict ops) {
  uint32_t sz = 0;
  // 根据指令的类型进行判断
  switch (DDS_OP_TYPE(insn)) {
    case DDS_OP_VAL_BLN:
    case DDS_OP_VAL_1BY:
    case DDS_OP_VAL_2BY:
    case DDS_OP_VAL_4BY:
    case DDS_OP_VAL_8BY:
      // 获取基本类型的大小
      sz = get_primitive_size(DDS_OP_TYPE(insn));
      break;
    case DDS_OP_VAL_ENU:
      // 枚举类型的大小
      sz = sizeof(uint32_t);
      break;
    case DDS_OP_VAL_BMK:
      // 获取位掩码类型的大小
      sz = DDS_OP_TYPE_SZ(insn);
      break;
    case DDS_OP_VAL_STR:
      // 字符串类型的大小
      sz = sizeof(char *);
      break;
    case DDS_OP_VAL_BST:
      // 二进制字符串类型的大小
      sz = ops[2];
      break;
    case DDS_OP_VAL_ARR: {
      // 数组类型的大小
      uint32_t num = ops[2];
      uint32_t elem_sz = get_collection_elem_size(ops[0], ops);
      sz = num * elem_sz;
      break;
    }
    case DDS_OP_VAL_SEQ:
    case DDS_OP_VAL_BSQ:
      // 序列类型的大小
      sz = sizeof(struct dds_sequence);
      break;
    case DDS_OP_VAL_EXT:
      // 扩展类型的大小
      sz = ops[3];
      break;
    case DDS_OP_VAL_UNI:
    case DDS_OP_VAL_STU:
      // 联合体和结构体类型不处理，使用扩展类型
      abort();
      break;
  }
  return sz;
}

/**
 * @brief 获取 jeq4 类型的大小
 *
 * @param valtype 枚举类型 dds_stream_typecode，表示数据流类型
 * @param jeq_op 一个指向 uint32_t 的指针，用于存储操作数
 * @return 返回计算得到的 jeq4 类型的大小
 */
static uint32_t get_jeq4_type_size(const enum dds_stream_typecode valtype,
                                   const uint32_t *__restrict jeq_op) {
  // 定义一个变量 sz，初始化为 0
  uint32_t sz = 0;

  // 根据 valtype 的值进行相应的处理
  switch (valtype) {
    case DDS_OP_VAL_BLN:
    case DDS_OP_VAL_1BY:
    case DDS_OP_VAL_2BY:
    case DDS_OP_VAL_4BY:
    case DDS_OP_VAL_8BY:
      // 获取基本类型的大小
      sz = get_primitive_size(valtype);
      break;
    case DDS_OP_VAL_ENU:
      // 获取枚举类型的大小
      sz = sizeof(uint32_t);
      break;
    case DDS_OP_VAL_STR:
      // 获取字符串类型的大小
      sz = sizeof(char *);
      break;
    case DDS_OP_VAL_BMK:
    case DDS_OP_VAL_BST:
    case DDS_OP_VAL_ARR: {
      // 获取数组类型的大小
      const uint32_t *jsr_ops = jeq_op + DDS_OP_ADR_JSR(jeq_op[0]);
      sz = get_adr_type_size(jsr_ops[0], jsr_ops);
      break;
    }
    case DDS_OP_VAL_SEQ:
    case DDS_OP_VAL_BSQ:
    case DDS_OP_VAL_STU:
    case DDS_OP_VAL_UNI:
      // 获取序列、结构体和联合体类型的大小
      sz = jeq_op[3];
      break;
    case DDS_OP_VAL_EXT:
      // 遇到扩展类型时，终止程序
      abort();
      break;
  }

  // 返回计算得到的大小
  return sz;
}

/**
 * @brief 判断类型是否具有子类型或成员
 *
 * @param type 枚举类型 dds_stream_typecode，表示数据流类型
 * @return 如果类型具有子类型或成员，则返回 true，否则返回 false
 */
static bool type_has_subtype_or_members(enum dds_stream_typecode type) {
  return type == DDS_OP_VAL_SEQ || type == DDS_OP_VAL_BSQ || type == DDS_OP_VAL_ARR ||
         type == DDS_OP_VAL_UNI || type == DDS_OP_VAL_STU;
}

/**
 * @brief 判断序列是否有界
 *
 * @param type 枚举类型 dds_stream_typecode，表示数据流类型
 * @return 如果序列有界，则返回 true，否则返回 false
 */
static bool seq_is_bounded(enum dds_stream_typecode type) {
  // 断言 type 的值为 DDS_OP_VAL_SEQ 或 DDS_OP_VAL_BSQ
  assert(type == DDS_OP_VAL_SEQ || type == DDS_OP_VAL_BSQ);

  // 如果 type 的值为 DDS_OP_VAL_BSQ，则返回 true，表示序列有界
  return type == DDS_OP_VAL_BSQ;
}

/**
 * @brief 检查位掩码值是否有效
 *
 * @param val      需要检查的64位整数值
 * @param bits_h   32位高位掩码
 * @param bits_l   32位低位掩码
 * @return bool    如果有效返回true，否则返回false
 */
static inline bool bitmask_value_valid(uint64_t val, uint32_t bits_h, uint32_t bits_l) {
  // 将val右移32位并与bits_h的补码进行按位与操作，如果结果为0，则表示高位有效
  // 将val强制转换为uint32_t类型，并与bits_l的补码进行按位与操作，如果结果为0，则表示低位有效
  return (val >> 32 & ~bits_h) == 0 && ((uint32_t)val & ~bits_l) == 0;
}

/**
 * @brief 判断指令是否为外部类型
 *
 * @param insn     32位指令
 * @return bool    如果是外部类型返回true，否则返回false
 */
static inline bool op_type_external(const uint32_t insn) {
  uint32_t typeflags = DDS_OP_TYPE_FLAGS(insn);  // 获取指令的类型标志
  return (typeflags & DDS_OP_FLAG_EXT);          // 判断类型标志是否包含DDS_OP_FLAG_EXT
}

/**
 * @brief 判断指令是否为可选类型
 *
 * @param insn     32位指令
 * @return bool    如果是可选类型返回true，否则返回false
 */
static inline bool op_type_optional(const uint32_t insn) {
  uint32_t flags = DDS_OP_FLAGS(insn);  // 获取指令的标志
  return (flags & DDS_OP_FLAG_OPT);     // 判断标志是否包含DDS_OP_FLAG_OPT
}

/**
 * @brief 判断指令是否为基本类型
 *
 * @param insn     32位指令
 * @return bool    如果是基本类型返回true，否则返回false
 */
static inline bool op_type_base(const uint32_t insn) {
  uint32_t opflags = DDS_OP_FLAGS(insn);  // 获取指令的标志
  return (opflags & DDS_OP_FLAG_BASE);    // 判断标志是否包含DDS_OP_FLAG_BASE
}

/**
 * @brief 检查并优化实现
 *
 * @param xcdr_version   XCDR版本
 * @param ops            操作数组
 * @param size           数据大小
 * @param num            数量
 * @param off            偏移量指针
 * @param member_offs    成员偏移量
 * @return bool          如果检查成功返回true，否则返回false
 */
static inline bool check_optimize_impl(uint32_t xcdr_version,
                                       const uint32_t *ops,
                                       uint32_t size,
                                       uint32_t num,
                                       uint32_t *off,
                                       uint32_t member_offs) {
  align_t align = dds_cdr_get_align(xcdr_version, size);  // 获取对齐值

  // 如果off与align的余数不为0，则更新off值
  if (*off % ALIGN(align)) *off += ALIGN(align) - (*off % ALIGN(align));

  // 如果成员偏移量加上操作数组中的第二个元素不等于off，则返回false
  if (member_offs + ops[1] != *off) return false;

  // 更新off值
  *off += num * size;

  return true;
}

/**
 * @brief 检查并优化DDS流
 *
 * @param desc        [in] 一个指向dds_cdrstream_desc结构体的指针，用于描述DDS流
 * @param xcdr_version [in]
 * XCDR版本（例如：DDSI_RTPS_CDR_ENC_VERSION_1或DDSI_RTPS_CDR_ENC_VERSION_2）
 * @param ops         [in] 操作码数组，用于指示如何处理数据
 * @param off         [in] 当前操作的偏移量
 * @param member_offs [in] 成员偏移量
 * @return uint32_t   返回优化后的偏移量
 */
static uint32_t dds_stream_check_optimize1(const struct dds_cdrstream_desc *__restrict desc,
                                           uint32_t xcdr_version,
                                           const uint32_t *ops,
                                           uint32_t off,
                                           uint32_t member_offs) {
  uint32_t insn;
  // 循环遍历操作码数组，直到遇到DDS_OP_RTS操作码
  while ((insn = *ops) != DDS_OP_RTS) {
    // 如果操作码不是DDS_OP_ADR，则返回0
    if (DDS_OP(insn) != DDS_OP_ADR) return 0;

    // 如果操作码类型为外部类型，则返回0
    if (op_type_external(insn)) return 0;

    // 根据操作码类型进行相应处理
    switch (DDS_OP_TYPE(insn)) {
      case DDS_OP_VAL_BLN:
      case DDS_OP_VAL_1BY:
      case DDS_OP_VAL_2BY:
      case DDS_OP_VAL_4BY:
      case DDS_OP_VAL_8BY:
        if (!check_optimize_impl(xcdr_version, ops, get_primitive_size(DDS_OP_TYPE(insn)), 1, &off,
                                 member_offs))
          return 0;
        ops += 2;
        break;
      case DDS_OP_VAL_ENU:
        if (DDS_OP_TYPE_SZ(insn) != 4 ||
            !check_optimize_impl(xcdr_version, ops, sizeof(uint32_t), 1, &off, member_offs))
          return 0;
        ops += 3;
        break;
      case DDS_OP_VAL_BMK:
        if (!check_optimize_impl(xcdr_version, ops, DDS_OP_TYPE_SZ(insn), 1, &off, member_offs))
          return 0;
        ops += 4;
        break;
      case DDS_OP_VAL_ARR:
        switch (DDS_OP_SUBTYPE(insn)) {
          case DDS_OP_VAL_BLN:
          case DDS_OP_VAL_1BY:
          case DDS_OP_VAL_2BY:
          case DDS_OP_VAL_4BY:
          case DDS_OP_VAL_8BY:
            if (!check_optimize_impl(xcdr_version, ops, get_primitive_size(DDS_OP_SUBTYPE(insn)),
                                     ops[2], &off, member_offs))
              return 0;
            ops += 3;
            break;
          case DDS_OP_VAL_ENU:
            if (xcdr_version ==
                DDSI_RTPS_CDR_ENC_VERSION_2) /* xcdr2数组对于非基本类型有一个dheader */
              return 0;
            if (DDS_OP_TYPE_SZ(insn) != 4 ||
                !check_optimize_impl(xcdr_version, ops, sizeof(uint32_t), ops[2], &off,
                                     member_offs))
              return 0;
            ops += 4;
            break;
          case DDS_OP_VAL_BMK:
            if (xcdr_version ==
                DDSI_RTPS_CDR_ENC_VERSION_2) /* xcdr2数组对于非基本类型有一个dheader */
              return 0;
            if (!check_optimize_impl(xcdr_version, ops, DDS_OP_TYPE_SZ(insn), ops[2], &off,
                                     member_offs))
              return 0;
            ops += 5;
            break;
          default:
            return 0;
        }
        break;
      case DDS_OP_VAL_EXT: {
        const uint32_t *jsr_ops = ops + DDS_OP_ADR_JSR(ops[2]);
        const uint32_t jmp = DDS_OP_ADR_JMP(ops[2]);
        if (DDS_OP_ADR_JSR(ops[2]) > 0)
          off = dds_stream_check_optimize1(desc, xcdr_version, jsr_ops, off, member_offs + ops[1]);
        ops += jmp ? jmp : 3;
        break;
      }
      case DDS_OP_VAL_SEQ:
      case DDS_OP_VAL_BSQ:
      case DDS_OP_VAL_STR:
      case DDS_OP_VAL_BST:
      case DDS_OP_VAL_STU:
      case DDS_OP_VAL_UNI:
        return 0;
    }
  }
  return off;
#undef ALLOW_ENUM
}

/**
 * @brief 检查并优化数据流大小
 *
 * @param[in] desc          数据流描述符指针
 * @param[in] xcdr_version  XCDR版本
 * @return size_t           优化后的数据流大小
 */
size_t dds_stream_check_optimize(const struct dds_cdrstream_desc *__restrict desc,
                                 uint32_t xcdr_version) {
  // 计算优化后的数据流大小
  size_t opt_size = dds_stream_check_optimize1(desc, xcdr_version, desc->ops.ops, 0, 0);

  // 如果desc->size包含尾部填充，off < desc可能会发生
  assert(opt_size <= desc->size);

  // 返回优化后的数据流大小
  return opt_size;
}

// 声明静态函数
static void dds_stream_countops1(const uint32_t *__restrict ops,
                                 const uint32_t **ops_end,
                                 uint16_t *min_xcdrv,
                                 uint32_t nestc,
                                 uint32_t *nestm);

/**
 * @brief 计算操作序列的数量
 *
 * @param[in]  ops      操作指针
 * @param[in]  insn     指令
 * @param[out] ops_end  操作结束指针
 * @param[out] min_xcdrv 最小XCDR版本
 * @param[in]  nestc    嵌套计数
 * @param[out] nestm    嵌套最大值
 * @return const uint32_t* 更新后的操作指针
 */
static const uint32_t *dds_stream_countops_seq(const uint32_t *__restrict ops,
                                               uint32_t insn,
                                               const uint32_t **ops_end,
                                               uint16_t *min_xcdrv,
                                               uint32_t nestc,
                                               uint32_t *nestm) {
  // 判断序列是否有边界
  uint32_t bound_op = seq_is_bounded(DDS_OP_TYPE(insn)) ? 1 : 0;

  // 获取子类型
  const enum dds_stream_typecode subtype = DDS_OP_SUBTYPE(insn);

  // 根据子类型处理操作
  switch (subtype) {
    case DDS_OP_VAL_BLN:
    case DDS_OP_VAL_1BY:
    case DDS_OP_VAL_2BY:
    case DDS_OP_VAL_4BY:
    case DDS_OP_VAL_8BY:
    case DDS_OP_VAL_STR:
      ops += 2 + bound_op;
      break;
    case DDS_OP_VAL_BST:
    case DDS_OP_VAL_ENU:
      ops += 3 + bound_op;
      break;
    case DDS_OP_VAL_BMK:
      ops += 4 + bound_op;
      break;
    case DDS_OP_VAL_SEQ:
    case DDS_OP_VAL_BSQ:
    case DDS_OP_VAL_ARR:
    case DDS_OP_VAL_UNI:
    case DDS_OP_VAL_STU: {
      const uint32_t jmp = DDS_OP_ADR_JMP(ops[3 + bound_op]);
      uint32_t const *const jsr_ops = ops + DDS_OP_ADR_JSR(ops[3 + bound_op]);
      if (ops + 4 + bound_op > *ops_end) *ops_end = ops + 4 + bound_op;
      if (DDS_OP_ADR_JSR(ops[3 + bound_op]) > 0)
        dds_stream_countops1(
            jsr_ops, ops_end, min_xcdrv,
            nestc + (subtype == DDS_OP_VAL_UNI || subtype == DDS_OP_VAL_STU ? 1 : 0), nestm);
      ops += (jmp ? jmp : (4 + bound_op)); /* FIXME: why would jmp be 0? */
      break;
    }
    case DDS_OP_VAL_EXT:
      abort();  // not allowed
      break;
  }

  // 更新操作结束指针
  if (ops > *ops_end) *ops_end = ops;

  // 返回更新后的操作指针
  return ops;
}

/**
 * @brief 计算操作数数组的长度，并更新相关参数
 * @param[in] ops 操作数数组指针
 * @param[in] insn 指令
 * @param[out] ops_end 操作数数组结束位置的指针
 * @param[out] min_xcdrv 最小交叉驱动器
 * @param[in] nestc 嵌套计数器
 * @param[out] nestm 嵌套标记
 * @return 更新后的操作数数组指针
 */
static const uint32_t *dds_stream_countops_arr(const uint32_t *__restrict ops,
                                               uint32_t insn,
                                               const uint32_t **ops_end,
                                               uint16_t *min_xcdrv,
                                               uint32_t nestc,
                                               uint32_t *nestm) {
  // 获取子类型
  const enum dds_stream_typecode subtype = DDS_OP_SUBTYPE(insn);
  switch (subtype) {
    case DDS_OP_VAL_BLN:
    case DDS_OP_VAL_1BY:
    case DDS_OP_VAL_2BY:
    case DDS_OP_VAL_4BY:
    case DDS_OP_VAL_8BY:
    case DDS_OP_VAL_STR:
      ops += 3;
      break;
    case DDS_OP_VAL_ENU:
      ops += 4;
      break;
    case DDS_OP_VAL_BST:
    case DDS_OP_VAL_BMK:
      ops += 5;
      break;
    case DDS_OP_VAL_SEQ:
    case DDS_OP_VAL_BSQ:
    case DDS_OP_VAL_ARR:
    case DDS_OP_VAL_UNI:
    case DDS_OP_VAL_STU: {
      const uint32_t jmp = DDS_OP_ADR_JMP(ops[3]);
      const uint32_t *jsr_ops = ops + DDS_OP_ADR_JSR(ops[3]);
      if (ops + 5 > *ops_end) *ops_end = ops + 5;
      if (DDS_OP_ADR_JSR(ops[3]) > 0)
        dds_stream_countops1(
            jsr_ops, ops_end, min_xcdrv,
            nestc + (subtype == DDS_OP_VAL_UNI || subtype == DDS_OP_VAL_STU ? 1 : 0), nestm);
      ops += (jmp ? jmp : 5);
      break;
    }
    case DDS_OP_VAL_EXT:
      abort();  // not allowed
      break;
  }
  if (ops > *ops_end) *ops_end = ops;
  return ops;
}

/**
 * @brief 计算联合操作数数组的长度，并更新相关参数
 * @param[in] ops 操作数数组指针
 * @param[out] ops_end 操作数数组结束位置的指针
 * @param[out] min_xcdrv 最小交叉驱动器
 * @param[in] nestc 嵌套计数器
 * @param[out] nestm 嵌套标记
 * @return 更新后的操作数数组指针
 */
static const uint32_t *dds_stream_countops_uni(const uint32_t *__restrict ops,
                                               const uint32_t **ops_end,
                                               uint16_t *min_xcdrv,
                                               uint32_t nestc,
                                               uint32_t *nestm) {
  // 获取案例数量
  const uint32_t numcases = ops[2];
  const uint32_t *jeq_op = ops + DDS_OP_ADR_JSR(ops[3]);
  for (uint32_t i = 0; i < numcases; i++) {
    const enum dds_stream_typecode valtype = DDS_JEQ_TYPE(jeq_op[0]);
    switch (valtype) {
      case DDS_OP_VAL_BLN:
      case DDS_OP_VAL_1BY:
      case DDS_OP_VAL_2BY:
      case DDS_OP_VAL_4BY:
      case DDS_OP_VAL_8BY:
      case DDS_OP_VAL_STR:
      case DDS_OP_VAL_ENU:
        break;
      case DDS_OP_VAL_BST:
      case DDS_OP_VAL_SEQ:
      case DDS_OP_VAL_BSQ:
      case DDS_OP_VAL_ARR:
      case DDS_OP_VAL_UNI:
      case DDS_OP_VAL_STU:
      case DDS_OP_VAL_BMK:
        if (DDS_OP_ADR_JSR(jeq_op[0]) > 0)
          dds_stream_countops1(
              jeq_op + DDS_OP_ADR_JSR(jeq_op[0]), ops_end, min_xcdrv,
              nestc + (valtype == DDS_OP_VAL_UNI || valtype == DDS_OP_VAL_STU ? 1 : 0), nestm);
        break;
      case DDS_OP_VAL_EXT:
        abort();  // not allowed
        break;
    }
    jeq_op += (DDS_OP(jeq_op[0]) == DDS_OP_JEQ) ? 3 : 4;
  }
  if (jeq_op > *ops_end) *ops_end = jeq_op;
  ops += DDS_OP_ADR_JMP(ops[3]);
  if (ops > *ops_end) *ops_end = ops;
  return ops;
}

/**
 * @brief 计算DDS流操作数，并更新最小交叉连接驱动器值。
 *
 * @param[in] __restrict ops 指向操作数数组的指针。
 * @param[out] ops_end 操作数结束位置的指针。
 * @param[in,out] min_xcdrv 最小交叉连接驱动器值的指针。
 * @param[in] nestc 嵌套计数器值。
 * @param[out] nestm 嵌套层次的指针。
 * @return 返回操作数结束位置的指针。
 */
static const uint32_t *dds_stream_countops_pl(const uint32_t *__restrict ops,
                                              const uint32_t **ops_end,
                                              uint16_t *min_xcdrv,
                                              uint32_t nestc,
                                              uint32_t *nestm) {
  uint32_t insn;                 // 定义指令变量
  assert(ops[0] == DDS_OP_PLC);  // 断言操作数数组的第一个元素是DDS_OP_PLC
  ops++;                         // 跳过PLC操作

  // 当指令不等于DDS_OP_RTS时，循环执行
  while ((insn = *ops) != DDS_OP_RTS) {
    // 根据指令进行相应操作
    switch (DDS_OP(insn)) {
      case DDS_OP_PLM: {
        uint32_t flags = DDS_PLM_FLAGS(insn);                  // 获取指令标志
        const uint32_t *plm_ops = ops + DDS_OP_ADR_PLM(insn);  // 计算PLM操作数地址
        if (flags & DDS_OP_FLAG_BASE)  // 如果标志位包含DDS_OP_FLAG_BASE
          (void)dds_stream_countops_pl(plm_ops, ops_end, min_xcdrv, nestc,
                                       nestm);  // 递归调用本函数
        else
          dds_stream_countops1(plm_ops, ops_end, min_xcdrv, nestc,
                               nestm);  // 调用dds_stream_countops1函数
        ops += 2;                       // 操作数指针向后移动两个位置
        break;
      }
      default:
        abort();  // 只支持(PLM, member-id)列表，其他情况终止程序
        break;
    }
  }

  // 如果操作数指针大于操作数结束位置，则更新操作数结束位置
  if (ops > *ops_end) *ops_end = ops;

  return ops;  // 返回操作数结束位置的指针
}

/**
 * @brief 计算DDS操作的数量
 *
 * @param[in]  ops      指向操作数组的指针
 * @param[out] ops_end  指向操作数组结束位置的指针
 * @param[out] min_xcdrv 最小的xcdr版本
 * @param[in]  nestc    嵌套计数器
 * @param[out] nestm    嵌套最大值
 */
static void dds_stream_countops1(const uint32_t *__restrict ops,
                                 const uint32_t **ops_end,
                                 uint16_t *min_xcdrv,
                                 uint32_t nestc,
                                 uint32_t *nestm) {
  uint32_t insn;  // 当前操作

  // 如果嵌套最大值存在且小于当前嵌套计数器，则更新嵌套最大值
  if (nestm && *nestm < nestc) *nestm = nestc;

  // 遍历操作数组，直到遇到DDS_OP_RTS操作
  while ((insn = *ops) != DDS_OP_RTS) {
    // 根据操作类型进行处理
    switch (DDS_OP(insn)) {
      case DDS_OP_ADR: {
        // 如果操作类型是可选的且min_xcdrv存在，则设置为DDSI_RTPS_CDR_ENC_VERSION_2
        if (op_type_optional(insn) && min_xcdrv) *min_xcdrv = DDSI_RTPS_CDR_ENC_VERSION_2;

        // 根据操作类型进行处理
        switch (DDS_OP_TYPE(insn)) {
          case DDS_OP_VAL_BLN:
          case DDS_OP_VAL_1BY:
          case DDS_OP_VAL_2BY:
          case DDS_OP_VAL_4BY:
          case DDS_OP_VAL_8BY:
          case DDS_OP_VAL_STR:
            ops += 2;
            break;
          case DDS_OP_VAL_BST:
          case DDS_OP_VAL_ENU:
            ops += 3;
            break;
          case DDS_OP_VAL_BMK:
            ops += 4;
            break;
          case DDS_OP_VAL_SEQ:
          case DDS_OP_VAL_BSQ:
            ops = dds_stream_countops_seq(ops, insn, ops_end, min_xcdrv, nestc, nestm);
            break;
          case DDS_OP_VAL_ARR:
            ops = dds_stream_countops_arr(ops, insn, ops_end, min_xcdrv, nestc, nestm);
            break;
          case DDS_OP_VAL_UNI:
            ops = dds_stream_countops_uni(ops, ops_end, min_xcdrv, nestc, nestm);
            break;
          case DDS_OP_VAL_EXT: {
            const uint32_t *jsr_ops = ops + DDS_OP_ADR_JSR(ops[2]);
            const uint32_t jmp = DDS_OP_ADR_JMP(ops[2]);
            if (DDS_OP_ADR_JSR(ops[2]) > 0)
              dds_stream_countops1(jsr_ops, ops_end, min_xcdrv, nestc + 1, nestm);
            ops += jmp ? jmp : 3;
            break;
          }
          case DDS_OP_VAL_STU:
            abort(); /* op type STU only supported as subtype */
            break;
        }
        break;
      }
      case DDS_OP_JSR: {
        if (DDS_OP_JUMP(insn) > 0)
          dds_stream_countops1(ops + DDS_OP_JUMP(insn), ops_end, min_xcdrv, nestc, nestm);
        ops++;
        break;
      }
      case DDS_OP_RTS:
      case DDS_OP_JEQ:
      case DDS_OP_JEQ4:
      case DDS_OP_KOF:
      case DDS_OP_PLM: {
        abort();
        break;
      }
      case DDS_OP_DLC: {
        if (min_xcdrv) *min_xcdrv = DDSI_RTPS_CDR_ENC_VERSION_2;
        ops++;
        break;
      }
      case DDS_OP_PLC: {
        if (min_xcdrv) *min_xcdrv = DDSI_RTPS_CDR_ENC_VERSION_2;
        ops = dds_stream_countops_pl(ops, ops_end, min_xcdrv, nestc, nestm);
        break;
      }
    }
  }

  ++ops;  // 跳过RTS操作

  // 更新操作数组结束位置
  if (ops > *ops_end) *ops_end = ops;
}

/**
 * @brief 计算 key 的偏移量并更新 ops_end 指针
 *
 * @param[in]  ops        输入的操作码数组
 * @param[in]  key        输入的键描述符
 * @param[out] ops_end    操作码数组结束位置的指针
 */
static void dds_stream_countops_keyoffset(const uint32_t *__restrict ops,
                                          const dds_key_descriptor_t *__restrict key,
                                          const uint32_t **__restrict ops_end) {
  // 断言 key 和 *ops_end 不为空
  assert(key);
  assert(*ops_end);

  // 如果 key 的偏移量大于等于 (ops_end - ops)，则进行以下操作
  if (key->m_offset >= (uint32_t)(*ops_end - ops)) {
    // 断言 ops[key->m_offset] 等于 DDS_OP_KOF
    assert(DDS_OP(ops[key->m_offset]) == DDS_OP_KOF);

    // 更新 ops_end 指针
    *ops_end = ops + key->m_offset + 1 + DDS_OP_LENGTH(ops[key->m_offset]);
  }
}

/**
 * @brief 计算操作码数组的长度
 *
 * @param[in]  ops   输入的操作码数组
 * @param[in]  nkeys 键数量
 * @param[in]  keys  键描述符数组
 * @return 返回操作码数组的长度
 */
uint32_t dds_stream_countops(const uint32_t *__restrict ops,
                             uint32_t nkeys,
                             const dds_key_descriptor_t *__restrict keys) {
  const uint32_t *ops_end = ops;

  // 调用 dds_stream_countops1 函数
  dds_stream_countops1(ops, &ops_end, NULL, 0, NULL);

  // 遍历所有键，调用 dds_stream_countops_keyoffset 函数
  for (uint32_t n = 0; n < nkeys; n++) dds_stream_countops_keyoffset(ops, &keys[n], &ops_end);

  // 返回操作码数组的长度
  return (uint32_t)(ops_end - ops);
}

/**
 * @brief 处理有界字符串，并根据需要分配内存
 *
 * @param[in]  is          输入流
 * @param[in]  str         字符串指针
 * @param[in]  allocator   分配器
 * @param[in]  size        字符串大小
 * @param[in]  alloc       是否分配内存
 * @return 返回处理后的字符串指针
 */
static char *dds_stream_reuse_string_bound(
    dds_istream_t *__restrict is,
    char *__restrict str,
    const struct dds_cdrstream_allocator *__restrict allocator,
    const uint32_t size,
    bool alloc) {
  // 获取字符串长度
  const uint32_t length = dds_is_get4(is);

  // 获取源数据指针
  const void *src = is->m_buffer + is->m_index;

  // 如果不分配内存，则断言 str 不为空
  if (!alloc)
    assert(str != NULL);
  else if (str == NULL)  // 如果分配内存且 str 为空，则分配内存
    str = allocator->malloc(size);

  // 拷贝数据到目标字符串
  memcpy(str, src, length > size ? size : length);

  // 如果长度大于 size，则在末尾添加空字符
  if (length > size) str[size - 1] = '\0';

  // 更新输入流索引
  is->m_index += length;

  // 返回处理后的字符串指针
  return str;
}

/**
 * @brief 处理字符串，并根据需要重新分配内存
 *
 * @param[in]  is         输入流
 * @param[in]  str        字符串指针
 * @param[in]  allocator  分配器
 * @return 返回处理后的字符串指针
 */
static char *dds_stream_reuse_string(dds_istream_t *__restrict is,
                                     char *__restrict str,
                                     const struct dds_cdrstream_allocator *__restrict allocator) {
  // 获取字符串长度
  const uint32_t length = dds_is_get4(is);

  // 获取源数据指针
  const void *src = is->m_buffer + is->m_index;

  // 如果 str 为空或其长度加 1 小于 length，则重新分配内存
  if (str == NULL || strlen(str) + 1 < length) str = allocator->realloc(str, length);

  // 拷贝数据到目标字符串
  memcpy(str, src, length);

  // 更新输入流索引
  is->m_index += length;

  // 返回处理后的字符串指针
  return str;
}

/**
 * @brief 处理空字符串，并根据需要重新分配内存
 *
 * @param[in]  str        字符串指针
 * @param[in]  allocator  分配器
 * @return 返回处理后的字符串指针
 */
static char *dds_stream_reuse_string_empty(
    char *__restrict str, const struct dds_cdrstream_allocator *__restrict allocator) {
  // 如果 str 为空，则重新分配内存
  if (str == NULL) str = allocator->realloc(str, 1);

  // 设置字符串为空字符串
  str[0] = '\0';

  // 返回处理后的字符串指针
  return str;
}

/**
 * @brief 跳过输入流中的一定长度数据
 *
 * @param[in]  is         输入流
 * @param[in]  len        需要跳过的元素数量
 * @param[in]  elem_size  单个元素的大小
 */
static void dds_stream_skip_forward(dds_istream_t *__restrict is,
                                    uint32_t len,
                                    const uint32_t elem_size) {
  // 如果 elem_size 和 len 都不为 0，则更新输入流索引
  if (elem_size && len) is->m_index += len * elem_size;
}

/**
 * @brief 跳过输入流中的字符串
 *
 * @param[in] is 输入流指针
 */
static void dds_stream_skip_string(dds_istream_t *__restrict is) {
  // 获取字符串长度
  const uint32_t length = dds_is_get4(is);
  // 向前跳过指定长度的字符串
  dds_stream_skip_forward(is, length, 1);
}

#ifndef NDEBUG
/**
 * @brief 检查指令是否为有效的键
 *
 * @param[in] insn 指令
 * @return bool 如果指令是有效的键，则返回true，否则返回false
 */
static bool insn_key_ok_p(uint32_t insn) {
  return (DDS_OP(insn) == DDS_OP_ADR && (insn & DDS_OP_FLAG_KEY) &&
          (!type_has_subtype_or_members(DDS_OP_TYPE(
               insn))  // 不允许 seq, uni, arr（除非下面的例外），struct（除非下面的例外）
           || (DDS_OP_TYPE(insn) == DDS_OP_VAL_ARR &&
               (is_primitive_or_enum_type(DDS_OP_SUBTYPE(insn)) ||
                DDS_OP_SUBTYPE(insn) ==
                    DDS_OP_VAL_BMK))  // 允许 prim-array, enum-array 和 bitmask-array 作为 key
           || DDS_OP_TYPE(insn) == DDS_OP_VAL_EXT  // 允许嵌套结构体中的字段作为 key
           ));
}
#endif

/**
 * @brief 读取联合体的判别式
 *
 * @param[in] is 输入流指针
 * @param[in] insn 指令
 * @return uint32_t 返回判别式的值
 */
static uint32_t read_union_discriminant(dds_istream_t *__restrict is, uint32_t insn) {
  // 获取类型
  enum dds_stream_typecode type = DDS_OP_SUBTYPE(insn);
  // 断言类型为原始类型或枚举类型
  assert(is_primitive_or_enum_type(type));
  switch (type) {
    case DDS_OP_VAL_BLN:
    case DDS_OP_VAL_1BY:
      // 读取1字节值
      return dds_is_get1(is);
    case DDS_OP_VAL_2BY:
      // 读取2字节值
      return dds_is_get2(is);
    case DDS_OP_VAL_4BY:
      // 读取4字节值
      return dds_is_get4(is);
    case DDS_OP_VAL_ENU:
      // 根据类型大小读取相应的值
      switch (DDS_OP_TYPE_SZ(insn)) {
        case 1:
          return dds_is_get1(is);
        case 2:
          return dds_is_get2(is);
        case 4:
          return dds_is_get4(is);
        default:
          abort();
      }
      break;
    default:
      return 0;
  }
}

/**
 * @brief 查找并返回与给定discriminator匹配的联合体case的地址。
 *
 * @param[in] union_ops 联合体操作指针，指向DDS_OP_VAL_UNI操作。
 * @param[in] disc 要查找的discriminator值。
 * @return
 * 如果找到匹配的case，则返回该case的地址；否则，如果有默认case，则返回默认case的地址；如果没有找到匹配的case且没有默认case，则返回NULL。
 */
static const uint32_t *find_union_case(const uint32_t *__restrict union_ops, uint32_t disc) {
  // 确保union_ops指向一个联合体操作
  assert(DDS_OP_TYPE(*union_ops) == DDS_OP_VAL_UNI);

  // 检查是否存在默认case
  const bool has_default = *union_ops & DDS_OP_FLAG_DEF;

  // 获取联合体中的case数量
  const uint32_t numcases = union_ops[2];

  // 获取跳转相等操作的地址
  const uint32_t *jeq_op = union_ops + DDS_OP_ADR_JSR(union_ops[3]);

  // 默认case总是最后一个
  assert(numcases > 0);
  uint32_t ci;

#ifndef NDEBUG
  size_t idx = 0;
  // 遍历所有case，计算索引
  for (ci = 0; ci < numcases; ci++) {
    if (DDS_OP(jeq_op[idx]) == DDS_OP_JEQ)
      idx += 3;
    else {
      assert(DDS_OP(jeq_op[idx]) == DDS_OP_JEQ4);
      idx += 4;
    }
  }
#endif

  // 遍历所有case，查找匹配的discriminator
  for (ci = 0; ci < numcases - (has_default ? 1 : 0); ci++) {
    if (jeq_op[1] == disc) return jeq_op;
    jeq_op += (DDS_OP(jeq_op[0]) == DDS_OP_JEQ) ? 3 : 4;
  }

  // 如果找到匹配的case或存在默认case，则返回对应地址，否则返回NULL
  return (ci < numcases) ? jeq_op : NULL;
}

/**
 * @brief 跳过序列指令并返回下一个操作的地址。
 *
 * @param[in] insn 当前序列指令。
 * @param[in] ops 操作指针。
 * @return 返回跳过序列指令后的下一个操作的地址。
 */
static const uint32_t *skip_sequence_insns(uint32_t insn, const uint32_t *__restrict ops) {
  // 判断序列是否有界，并获取相应的操作数
  uint32_t bound_op = seq_is_bounded(DDS_OP_TYPE(insn)) ? 1 : 0;

  // 根据子类型跳过序列指令并返回下一个操作的地址
  switch (DDS_OP_SUBTYPE(insn)) {
    case DDS_OP_VAL_BLN:
    case DDS_OP_VAL_1BY:
    case DDS_OP_VAL_2BY:
    case DDS_OP_VAL_4BY:
    case DDS_OP_VAL_8BY:
    case DDS_OP_VAL_STR:
      return ops + 2 + bound_op;
    case DDS_OP_VAL_BST:
    case DDS_OP_VAL_ENU:
      return ops + 3 + bound_op;
    case DDS_OP_VAL_BMK:
      return ops + 4 + bound_op;
    case DDS_OP_VAL_SEQ:
    case DDS_OP_VAL_BSQ:
    case DDS_OP_VAL_ARR:
    case DDS_OP_VAL_UNI:
    case DDS_OP_VAL_STU: {
      const uint32_t jmp = DDS_OP_ADR_JMP(ops[3 + bound_op]);
      return ops + (jmp ? jmp : 4 + bound_op); /* FIXME: why would jmp be 0? */
    }
    case DDS_OP_VAL_EXT: {
      abort(); /* not allowed */
      break;
    }
  }

  return NULL;
}

/**
 * @brief 跳过数组指令
 *
 * @param[in] insn 指令
 * @param[in] __restrict ops 受限制的操作数指针
 * @return 返回跳过后的操作数指针
 */
static const uint32_t *skip_array_insns(uint32_t insn, const uint32_t *__restrict ops) {
  // 断言指令类型为数组
  assert(DDS_OP_TYPE(insn) == DDS_OP_VAL_ARR);

  // 根据指令子类型进行处理
  switch (DDS_OP_SUBTYPE(insn)) {
    case DDS_OP_VAL_BLN:
    case DDS_OP_VAL_1BY:
    case DDS_OP_VAL_2BY:
    case DDS_OP_VAL_4BY:
    case DDS_OP_VAL_8BY:
    case DDS_OP_VAL_STR:
      // 对于这些子类型，返回操作数指针加3
      return ops + 3;
    case DDS_OP_VAL_ENU:
      // 对于枚举类型，返回操作数指针加4
      return ops + 4;
    case DDS_OP_VAL_BST:
    case DDS_OP_VAL_BMK:
      // 对于这些子类型，返回操作数指针加5
      return ops + 5;
    case DDS_OP_VAL_SEQ:
    case DDS_OP_VAL_BSQ:
    case DDS_OP_VAL_ARR:
    case DDS_OP_VAL_UNI:
    case DDS_OP_VAL_STU: {
      // 对于这些子类型，根据 jmp 的值返回操作数指针加 jmp 或加5
      const uint32_t jmp = DDS_OP_ADR_JMP(ops[3]);
      return ops + (jmp ? jmp : 5);
    }
    case DDS_OP_VAL_EXT: {
      // 对于扩展类型，不支持，直接中止
      abort(); /* not supported */
      break;
    }
  }

  // 返回空指针
  return NULL;
}

/**
 * @brief 跳过数组默认值
 *
 * @param[in] insn 指令
 * @param[in] __restrict data 受限制的数据指针
 * @param[in] __restrict allocator 受限制的分配器指针
 * @param[in] __restrict ops 受限制的操作数指针
 * @return 返回跳过后的操作数指针
 */
static const uint32_t *skip_array_default(
    uint32_t insn,
    char *__restrict data,
    const struct dds_cdrstream_allocator *__restrict allocator,
    const uint32_t *__restrict ops) {
  // 获取指令子类型
  const enum dds_stream_typecode subtype = DDS_OP_SUBTYPE(insn);

  // 获取操作数数量
  const uint32_t num = ops[2];

  // 根据子类型进行处理
  switch (subtype) {
    case DDS_OP_VAL_BLN:
    case DDS_OP_VAL_1BY:
    case DDS_OP_VAL_2BY:
    case DDS_OP_VAL_4BY:
    case DDS_OP_VAL_8BY: {
      // 获取基本类型大小
      const uint32_t elem_size = get_primitive_size(subtype);

      // 将数据区域设置为0
      memset(data, 0, num * elem_size);

      // 返回操作数指针加3
      return ops + 3;
    }
    case DDS_OP_VAL_ENU:
    case DDS_OP_VAL_BMK: {
      // 获取元素大小
      const uint32_t elem_size = DDS_OP_TYPE_SZ(insn);

      // 将数据区域设置为0
      memset(data, 0, num * elem_size);

      // 返回操作数指针加4，如果子类型为DDS_OP_VAL_BMK，则再加1
      return ops + 4 + (subtype == DDS_OP_VAL_BMK ? 1 : 0);
    }
    case DDS_OP_VAL_STR: {
      // 初始化字符串指针
      char **ptr = (char **)data;

      // 遍历所有字符串并设置为空字符串
      for (uint32_t i = 0; i < num; i++)
        ptr[i] = dds_stream_reuse_string_empty(*(char **)ptr[i], allocator);

      // 返回操作数指针加3
      return ops + 3;
    }
    case DDS_OP_VAL_BST: {
      // 初始化字符指针
      char *ptr = (char *)data;

      // 获取元素大小
      const uint32_t elem_size = ops[4];

      // 遍历所有元素并设置为空字符
      for (uint32_t i = 0; i < num; i++) ((char *)(ptr + i * elem_size))[0] = '\0';

      // 返回操作数指针加5
      return ops + 5;
    }
    case DDS_OP_VAL_SEQ:
    case DDS_OP_VAL_BSQ:
    case DDS_OP_VAL_ARR:
    case DDS_OP_VAL_UNI:
    case DDS_OP_VAL_STU: {
      // 获取跳转操作数指针
      const uint32_t *jsr_ops = ops + DDS_OP_ADR_JSR(ops[3]);

      // 获取跳转值
      const uint32_t jmp = DDS_OP_ADR_JMP(ops[3]);

      // 获取元素大小
      const uint32_t elem_size = ops[4];

      // 遍历所有元素并跳过默认值
      for (uint32_t i = 0; i < num; i++)
        (void)dds_stream_skip_default(data + i * elem_size, allocator, jsr_ops);

      // 返回操作数指针加 jmp 或加5
      return ops + (jmp ? jmp : 5);
    }
    case DDS_OP_VAL_EXT: {
      // 对于扩展类型，不支持，直接中止
      abort(); /* not supported */
      break;
    }
  }

  // 返回空指针
  return NULL;
}

/**
 * @brief 跳过并设置默认值的联合体处理函数
 *
 * @param[in] insn        指令
 * @param[out] discaddr   存放鉴别器地址的指针
 * @param[in] baseaddr    基地址
 * @param[in] allocator   分配器
 * @param[in] ops         操作列表
 * @return const uint32_t* 更新后的操作列表指针
 */
static const uint32_t *skip_union_default(
    uint32_t insn,
    char *__restrict discaddr,
    char *__restrict baseaddr,
    const struct dds_cdrstream_allocator *__restrict allocator,
    const uint32_t *__restrict ops) {
  // 根据指令的子类型进行处理
  switch (DDS_OP_SUBTYPE(insn)) {
    case DDS_OP_VAL_BLN:
    case DDS_OP_VAL_1BY:
      // 设置鉴别器为0（布尔或1字节）
      *((uint8_t *)discaddr) = 0;
      break;
    case DDS_OP_VAL_2BY:
      // 设置鉴别器为0（2字节）
      *((uint16_t *)discaddr) = 0;
      break;
    case DDS_OP_VAL_4BY:
    case DDS_OP_VAL_ENU:
      // 设置鉴别器为0（4字节或枚举）
      *((uint32_t *)discaddr) = 0;
      break;
    default:
      break;
  }

  // 查找与0匹配的联合体情况
  uint32_t const *const jeq_op = find_union_case(ops, 0);
  // 更新操作列表指针
  ops += DDS_OP_ADR_JMP(ops[3]);

  // 如果找到匹配的情况
  if (jeq_op) {
    // 获取值类型
    const enum dds_stream_typecode valtype = DDS_JEQ_TYPE(jeq_op[0]);
    // 计算值地址
    void *valaddr = baseaddr + jeq_op[2];

    // 根据值类型进行处理
    switch (valtype) {
      case DDS_OP_VAL_BLN:
      case DDS_OP_VAL_1BY:
        // 设置值为0（布尔或1字节）
        *((uint8_t *)valaddr) = 0;
        break;
      case DDS_OP_VAL_2BY:
        // 设置值为0（2字节）
        *((uint16_t *)valaddr) = 0;
        break;
      case DDS_OP_VAL_4BY:
      case DDS_OP_VAL_ENU:
        // 设置值为0（4字节或枚举）
        *((uint32_t *)valaddr) = 0;
        break;
      case DDS_OP_VAL_8BY:
        // 设置值为0（8字节）
        *((uint64_t *)valaddr) = 0;
        break;
      case DDS_OP_VAL_STR:
        // 设置值为空字符串
        *(char **)valaddr = dds_stream_reuse_string_empty(*((char **)valaddr), allocator);
        break;
      case DDS_OP_VAL_BST:
      case DDS_OP_VAL_SEQ:
      case DDS_OP_VAL_BSQ:
      case DDS_OP_VAL_ARR:
      case DDS_OP_VAL_UNI:
      case DDS_OP_VAL_STU:
      case DDS_OP_VAL_BMK:
        // 跳过并设置默认值
        (void)dds_stream_skip_default(valaddr, allocator, jeq_op + DDS_OP_ADR_JSR(jeq_op[0]));
        break;
      case DDS_OP_VAL_EXT: {
        // 不支持的类型，中止程序
        abort();
        break;
      }
    }
  }

  // 返回更新后的操作列表指针
  return ops;
}

/**
 * @brief 获取序列长度编码
 * @param[in] subtype 序列子类型
 * @return 返回序列长度编码
 *
 * 根据序列子类型，返回相应的序列长度编码。
 */
static uint32_t get_length_code_seq(const enum dds_stream_typecode subtype) {
  switch (subtype) {
    /* 序列长度可以用作字节长度 */
    case DDS_OP_VAL_BLN:
    case DDS_OP_VAL_1BY:
      return LENGTH_CODE_ALSO_NEXTINT;

    /* 带有原始子类型的序列不包括DHEADER，
     只有序列长度，因此我们必须包含NEXTINT */
    case DDS_OP_VAL_2BY:
      return LENGTH_CODE_NEXTINT;

    /* 使用序列长度（项目计数）来计算字节长度 */
    case DDS_OP_VAL_4BY:
      return LENGTH_CODE_ALSO_NEXTINT4;
    case DDS_OP_VAL_8BY:
      return LENGTH_CODE_ALSO_NEXTINT8;

    /* 带有非原始子类型的序列包含DHEADER */
    case DDS_OP_VAL_STR:
    case DDS_OP_VAL_BST:
    case DDS_OP_VAL_ENU:
    case DDS_OP_VAL_BMK:
    case DDS_OP_VAL_SEQ:
    case DDS_OP_VAL_BSQ:
    case DDS_OP_VAL_ARR:
    case DDS_OP_VAL_UNI:
    case DDS_OP_VAL_STU:
      return LENGTH_CODE_ALSO_NEXTINT;

    /* 不支持 */
    case DDS_OP_VAL_EXT:
      abort();
      break;
  }
  abort();
}

/**
 * @brief 获取数组长度编码
 * @param[in] subtype 数组子类型
 * @return 返回数组长度编码
 *
 * 根据数组子类型，返回相应的数组长度编码。
 */
static uint32_t get_length_code_arr(const enum dds_stream_typecode subtype) {
  switch (subtype) {
    /* 带有原始子类型的数组不包括DHEADER，
     因此我们必须包含NEXTINT */
    case DDS_OP_VAL_BLN:
    case DDS_OP_VAL_1BY:
    case DDS_OP_VAL_2BY:
    case DDS_OP_VAL_4BY:
    case DDS_OP_VAL_8BY:
      return LENGTH_CODE_NEXTINT;

    /* 带有非原始子类型的数组包含DHEADER */
    case DDS_OP_VAL_STR:
    case DDS_OP_VAL_BST:
    case DDS_OP_VAL_ENU:
    case DDS_OP_VAL_BMK:
    case DDS_OP_VAL_SEQ:
    case DDS_OP_VAL_BSQ:
    case DDS_OP_VAL_ARR:
    case DDS_OP_VAL_UNI:
    case DDS_OP_VAL_STU:
      return LENGTH_CODE_ALSO_NEXTINT;

    /* 不支持 */
    case DDS_OP_VAL_EXT:
      abort();
      break;
  }
  abort();
}

/**
 * @brief 获取长度代码
 * @param[in] ops 指向操作数的指针
 * @return 长度代码
 *
 * 该函数根据给定的操作数计算长度代码。
 */
static uint32_t get_length_code(const uint32_t *__restrict ops) {
  // 获取当前操作数
  const uint32_t insn = *ops;

  // 断言操作数不等于DDS_OP_RTS
  assert(insn != DDS_OP_RTS);

  // 根据操作数的类型进行处理
  switch (DDS_OP(insn)) {
    case DDS_OP_ADR: {
      // 根据操作数的子类型进行处理
      switch (DDS_OP_TYPE(insn)) {
        case DDS_OP_VAL_BLN:
        case DDS_OP_VAL_1BY:
          return LENGTH_CODE_1B;
        case DDS_OP_VAL_2BY:
          return LENGTH_CODE_2B;
        case DDS_OP_VAL_4BY:
          return LENGTH_CODE_4B;
        case DDS_OP_VAL_8BY:
          return LENGTH_CODE_8B;
        case DDS_OP_VAL_ENU:
        case DDS_OP_VAL_BMK:
          // 根据操作数的大小进行处理
          switch (DDS_OP_TYPE_SZ(insn)) {
            case 1:
              return LENGTH_CODE_1B;
            case 2:
              return LENGTH_CODE_2B;
            case 4:
              return LENGTH_CODE_4B;
            case 8:
              return LENGTH_CODE_8B;
          }
          break;
        case DDS_OP_VAL_STR:
        case DDS_OP_VAL_BST:
          return LENGTH_CODE_ALSO_NEXTINT; /* nextint overlaps with length from serialized string
                                              data */
        case DDS_OP_VAL_SEQ:
        case DDS_OP_VAL_BSQ:
          return get_length_code_seq(DDS_OP_SUBTYPE(insn));
        case DDS_OP_VAL_ARR:
          return get_length_code_arr(DDS_OP_SUBTYPE(insn));
        case DDS_OP_VAL_UNI:
        case DDS_OP_VAL_EXT: {
          return LENGTH_CODE_NEXTINT; /* FIXME: may be optimized for specific cases, e.g. when EXT
                                         type is appendable */
        }
        case DDS_OP_VAL_STU:
          abort();
          break; /* op type STU only supported as subtype */
      }
      break;
    }
    case DDS_OP_JSR:
      return get_length_code(ops + DDS_OP_JUMP(insn));
    case DDS_OP_RTS:
    case DDS_OP_JEQ:
    case DDS_OP_JEQ4:
    case DDS_OP_KOF:
    case DDS_OP_PLM:
      abort();
      break;
    case DDS_OP_DLC:
    case DDS_OP_PLC:
      /* members of (final/appendable/mutable) aggregated types are included using ADR | EXT */
      abort();
      break;
  }
  return 0;
}

/**
 * @brief 检查成员是否存在
 *
 * 该函数用于检查给定数据中的成员是否存在。
 *
 * @param data 数据指针，指向要检查的数据
 * @param ops 操作指针，指向操作序列
 * @return 如果成员存在，则返回 true；否则返回 false
 */
static bool is_member_present(const char *__restrict data, const uint32_t *__restrict ops) {
  uint32_t insn;  // 存储当前操作码

  // 循环遍历操作序列，直到遇到 DDS_OP_RTS
  while ((insn = *ops) != DDS_OP_RTS) {
    // 根据操作码执行相应操作
    switch (DDS_OP(insn)) {
      case DDS_OP_ADR: {
        // 判断操作类型是否为可选类型
        if (op_type_optional(insn)) {
          const void *addr = data + ops[1];  // 计算地址
          addr = *(char **)addr;             // 对类型 STR 进行解引用
          return addr != NULL;               // 如果地址不为空，则表示成员存在
        }
        // 假设非可选成员始终存在
        return true;
      }
      case DDS_OP_JSR:
        // 递归调用 is_member_present 函数
        return is_member_present(data, ops + DDS_OP_JUMP(insn));
      case DDS_OP_RTS:
      case DDS_OP_JEQ:
      case DDS_OP_JEQ4:
      case DDS_OP_KOF:
      case DDS_OP_DLC:
      case DDS_OP_PLC:
      case DDS_OP_PLM:
        abort();  // 遇到以上操作码时，中止程序
        break;
    }
  }
  abort();  // 遇到 DDS_OP_RTS 时，中止程序
}

#if DDSRT_ENDIAN == DDSRT_LITTLE_ENDIAN
/**
 * @brief 将数据从小端字节序转换为大端字节序
 *
 * @param vbuf 数据缓冲区指针
 * @param size 数据大小
 * @param num 数据数量
 */
static inline void dds_stream_to_BE_insitu(void *__restrict vbuf, uint32_t size, uint32_t num) {
  dds_stream_swap(vbuf, size, num);
}
/**
 * @brief 不进行字节序转换（小端字节序）
 *
 * @param vbuf 数据缓冲区指针
 * @param size 数据大小
 * @param num 数据数量
 */
static inline void dds_stream_to_LE_insitu(void *__restrict vbuf, uint32_t size, uint32_t num) {
  (void)vbuf;
  (void)size;
  (void)num;
}
#else  /* if DDSRT_ENDIAN == DDSRT_LITTLE_ENDIAN */
/**
 * @brief 不进行字节序转换（大端字节序）
 *
 * @param vbuf 数据缓冲区指针
 * @param size 数据大小
 * @param num 数据数量
 */
static inline void dds_stream_to_BE_insitu(void *__restrict vbuf, uint32_t size, uint32_t num) {
  (void)vbuf;
  (void)size;
  (void)num;
}
/**
 * @brief 将数据从大端字节序转换为小端字节序
 *
 * @param vbuf 数据缓冲区指针
 * @param size 数据大小
 * @param num 数据数量
 */
static inline void dds_stream_to_LE_insitu(void *__restrict vbuf, uint32_t size, uint32_t num) {
  dds_stream_swap(vbuf, size, num);
}
#endif /* if DDSRT_ENDIAN == DDSRT_LITTLE_ENDIAN */

// 小端字节序
#define NAME_BYTE_ORDER_EXT LE  // 定义宏NAME_BYTE_ORDER_EXT为LE（Little-endian）
#include "dds_cdrstream_write.part.c"  // 包含dds_cdrstream_write.part.c文件，此时NAME_BYTE_ORDER_EXT为LE
#undef NAME_BYTE_ORDER_EXT  // 取消定义宏NAME_BYTE_ORDER_EXT

// 大端字节序
#define NAME_BYTE_ORDER_EXT BE  // 定义宏NAME_BYTE_ORDER_EXT为BE（Big-endian）
#include "dds_cdrstream_write.part.c"  // 再次包含dds_cdrstream_write.part.c文件，此时NAME_BYTE_ORDER_EXT为BE
#undef NAME_BYTE_ORDER_EXT  // 取消定义宏NAME_BYTE_ORDER_EXT

// Map some write-native functions to their little-endian or big-endian equivalent
#if DDSRT_ENDIAN == DDSRT_LITTLE_ENDIAN

/**
 * @brief 将字符串写入dds流中
 * @param[in] os 指向dds_ostream_t结构体的指针，用于输出数据
 * @param[in] allocator 指向dds_cdrstream_allocator结构体的指针，用于内存分配
 * @param[in] val 要写入的字符串
 */
static inline void dds_stream_write_string(
    dds_ostream_t *__restrict os,
    const struct dds_cdrstream_allocator *__restrict allocator,
    const char *__restrict val) {
  dds_stream_write_stringLE((dds_ostreamLE_t *)os, allocator, val);  // 调用LE版本的函数实现
}

/**
 * @brief 将枚举值写入dds流中
 * @param[in] os 指向dds_ostream_t结构体的指针，用于输出数据
 * @param[in] allocator 指向dds_cdrstream_allocator结构体的指针，用于内存分配
 * @param[in] insn 指令
 * @param[in] val 要写入的枚举值
 * @param[in] max 枚举值的最大值
 * @return 写入成功返回true，否则返回false
 */
static inline bool dds_stream_write_enum_value(
    dds_ostream_t *__restrict os,
    const struct dds_cdrstream_allocator *__restrict allocator,
    uint32_t insn,
    uint32_t val,
    uint32_t max) {
  return dds_stream_write_enum_valueLE((dds_ostreamLE_t *)os, allocator, insn, val,
                                       max);  // 调用LE版本的函数实现
}

/**
 * @brief 将枚举数组写入dds流中
 * @param[in] os 指向dds_ostream_t结构体的指针，用于输出数据
 * @param[in] allocator 指向dds_cdrstream_allocator结构体的指针，用于内存分配
 * @param[in] insn 指令
 * @param[in] addr 要写入的枚举数组的地址
 * @param[in] num 枚举数组的元素数量
 * @param[in] max 枚举值的最大值
 * @return 写入成功返回true，否则返回false
 */
static inline bool dds_stream_write_enum_arr(
    dds_ostream_t *__restrict os,
    const struct dds_cdrstream_allocator *__restrict allocator,
    uint32_t insn,
    const uint32_t *__restrict addr,
    uint32_t num,
    uint32_t max) {
  return dds_stream_write_enum_arrLE((dds_ostreamLE_t *)os, allocator, insn, addr, num,
                                     max);  // 调用LE版本的函数实现
}

/**
 * @brief 将位掩码值写入dds流中
 * @param[in] os 指向dds_ostream_t结构体的指针，用于输出数据
 * @param[in] allocator 指向dds_cdrstream_allocator结构体的指针，用于内存分配
 * @param[in] insn 指令
 * @param[in] addr 要写入的位掩码值的地址
 * @param[in] bits_h 高位掩码
 * @param[in] bits_l 低位掩码
 * @return 写入成功返回true，否则返回false
 */
static inline bool dds_stream_write_bitmask_value(
    dds_ostream_t *__restrict os,
    const struct dds_cdrstream_allocator *__restrict allocator,
    uint32_t insn,
    const void *__restrict addr,
    uint32_t bits_h,
    uint32_t bits_l) {
  return dds_stream_write_bitmask_valueLE((dds_ostreamLE_t *)os, allocator, insn, addr, bits_h,
                                          bits_l);  // 调用LE版本的函数实现
}

/**
 * @brief 将位掩码数组写入dds流中
 * @param[in] os 指向dds_ostream_t结构体的指针，用于输出数据
 * @param[in] allocator 指向dds_cdrstream_allocator结构体的指针，用于内存分配
 * @param[in] insn 指令
 * @param[in] addr 要写入的位掩码数组的地址
 * @param[in] num 位掩码数组的元素数量
 * @param[in] bits_h 高位掩码
 * @param[in] bits_l 低位掩码
 * @return 写入成功返回true，否则返回false
 */
static inline bool dds_stream_write_bitmask_arr(
    dds_ostream_t *__restrict os,
    const struct dds_cdrstream_allocator *__restrict allocator,
    uint32_t insn,
    const void *__restrict addr,
    uint32_t num,
    uint32_t bits_h,
    uint32_t bits_l) {
  return dds_stream_write_bitmask_arrLE((dds_ostreamLE_t *)os, allocator, insn, addr, num, bits_h,
                                        bits_l);  // 调用LE版本的函数实现
}

/**
 * @brief 将数据写入dds流中
 * @param[in] os 指向dds_ostream_t结构体的指针，用于输出数据
 * @param[in] allocator 指向dds_cdrstream_allocator结构体的指针，用于内存分配
 * @param[in] data 要写入的数据
 * @param[in] ops 操作指令数组
 * @return 返回操作指令数组的下一个位置
 */
const uint32_t *dds_stream_write(dds_ostream_t *__restrict os,
                                 const struct dds_cdrstream_allocator *__restrict allocator,
                                 const char *__restrict data,
                                 const uint32_t *__restrict ops) {
  return dds_stream_writeLE((dds_ostreamLE_t *)os, allocator, data, ops);  // 调用LE版本的函数实现
}

/**
 * @brief 将样本数据写入dds流中
 * @param[in] os 指向dds_ostream_t结构体的指针，用于输出数据
 * @param[in] allocator 指向dds_cdrstream_allocator结构体的指针，用于内存分配
 * @param[in] data 要写入的样本数据
 * @param[in] desc 描述样本数据结构的dds_cdrstream_desc结构体指针
 * @return 写入成功返回true，否则返回false
 */
bool dds_stream_write_sample(dds_ostream_t *__restrict os,
                             const struct dds_cdrstream_allocator *__restrict allocator,
                             const void *__restrict data,
                             const struct dds_cdrstream_desc *__restrict desc) {
  return dds_stream_write_sampleLE((dds_ostreamLE_t *)os, allocator, data,
                                   desc);  // 调用LE版本的函数实现
}

/**
 * @brief 以小端字节序写入样本数据到输出流中
 *
 * @param os 指向dds_ostreamLE_t结构体的指针，用于存储输出流信息
 * @param allocator 指向dds_cdrstream_allocator结构体的指针，用于分配内存
 * @param data 指向待写入数据的指针
 * @param desc 指向dds_cdrstream_desc结构体的指针，描述数据类型
 * @return bool 写入成功返回true，否则返回false
 */
bool dds_stream_write_sampleLE(dds_ostreamLE_t *__restrict os,
                               const struct dds_cdrstream_allocator *__restrict allocator,
                               const void *__restrict data,
                               const struct dds_cdrstream_desc *__restrict desc) {
  // 根据os的m_xcdr_version字段确定opt_size的值
  size_t opt_size = os->x.m_xcdr_version == DDSI_RTPS_CDR_ENC_VERSION_1 ? desc->opt_size_xcdr1
                                                                        : desc->opt_size_xcdr2;

  // 如果opt_size不为0且desc->align不为0，检查当前输出流索引是否满足对齐要求
  if (opt_size && desc->align && (((struct dds_ostream *)os)->m_index % desc->align) == 0) {
    // 将数据写入输出流，并返回true表示成功
    dds_os_put_bytes((struct dds_ostream *)os, allocator, data, (uint32_t)opt_size);
    return true;
  } else {
    // 调用dds_stream_writeLE函数写入数据，并根据返回值判断是否成功
    return dds_stream_writeLE(os, allocator, data, desc->ops.ops) != NULL;
  }
}

/**
 * @brief 以大端字节序写入样本数据到输出流中
 *
 * @param os 指向dds_ostreamBE_t结构体的指针，用于存储输出流信息
 * @param allocator 指向dds_cdrstream_allocator结构体的指针，用于分配内存
 * @param data 指向待写入数据的指针
 * @param desc 指向dds_cdrstream_desc结构体的指针，描述数据类型
 * @return bool 写入成功返回true，否则返回false
 */
bool dds_stream_write_sampleBE(dds_ostreamBE_t *__restrict os,
                               const struct dds_cdrstream_allocator *__restrict allocator,
                               const void *__restrict data,
                               const struct dds_cdrstream_desc *__restrict desc) {
  // 调用dds_stream_writeBE函数写入数据，并根据返回值判断是否成功
  return dds_stream_writeBE(os, allocator, data, desc->ops.ops) != NULL;
}

#else /* if DDSRT_ENDIAN == DDSRT_LITTLE_ENDIAN */

/** @file
 *  @brief 包含dds_stream相关的函数定义
 */

/**
 * @brief 将字符串写入dds_ostream_t流中
 *
 * @param[in] os 指向dds_ostream_t结构体的指针
 * @param[in] allocator 指向dds_cdrstream_allocator结构体的指针
 * @param[in] val 要写入的字符串
 */
static inline void dds_stream_write_string(
    dds_ostream_t *__restrict os,
    const struct dds_cdrstream_allocator *__restrict allocator,
    const char *__restrict val) {
  // 调用dds_stream_write_stringBE函数，将字符串写入流中
  dds_stream_write_stringBE((dds_ostreamBE_t *)os, allocator, val, allocator);
}

/**
 * @brief 将枚举值写入dds_ostream_t流中
 *
 * @param[in] os 指向dds_ostream_t结构体的指针
 * @param[in] allocator 指向dds_cdrstream_allocator结构体的指针
 * @param[in] insn 指令
 * @param[in] val 要写入的枚举值
 * @param[in] max 枚举值的最大值
 * @return 写入成功返回true，否则返回false
 */
static inline bool dds_stream_write_enum_value(
    dds_ostream_t *__restrict os,
    const struct dds_cdrstream_allocator *__restrict allocator,
    uint32_t insn,
    uint32_t val,
    uint32_t max) {
  // 调用dds_stream_write_enum_valueBE函数，将枚举值写入流中
  return dds_stream_write_enum_valueBE((dds_ostreamBE_t *)os, allocator, insn, val, max, allocator);
}

/**
 * @brief 将枚举数组写入dds_ostream_t流中
 *
 * @param[in] os 指向dds_ostream_t结构体的指针
 * @param[in] allocator 指向dds_cdrstream_allocator结构体的指针
 * @param[in] insn 指令
 * @param[in] addr 要写入的枚举数组的地址
 * @param[in] num 枚举数组的元素数量
 * @param[in] max 枚举值的最大值
 * @return 写入成功返回true，否则返回false
 */
static inline bool dds_stream_write_enum_arr(
    dds_ostream_t *__restrict os,
    const struct dds_cdrstream_allocator *__restrict allocator,
    uint32_t insn,
    const uint32_t *__restrict addr,
    uint32_t num,
    uint32_t max) {
  // 调用dds_stream_write_enum_arrBE函数，将枚举数组写入流中
  return dds_stream_write_enum_arrBE((dds_ostreamBE_t *)os, allocator, insn, addr, num, max);
}

/**
 * @brief 将位掩码值写入dds_ostream_t流中
 *
 * @param[in] os 指向dds_ostream_t结构体的指针
 * @param[in] allocator 指向dds_cdrstream_allocator结构体的指针
 * @param[in] insn 指令
 * @param[in] addr 要写入的位掩码值的地址
 * @param[in] bits_h 高位掩码
 * @param[in] bits_l 低位掩码
 * @return 写入成功返回true，否则返回false
 */
static inline bool dds_stream_write_bitmask_value(
    dds_ostream_t *__restrict os,
    const struct dds_cdrstream_allocator *__restrict allocator,
    uint32_t insn,
    const void *__restrict addr,
    uint32_t bits_h,
    uint32_t bits_l) {
  // 调用dds_stream_write_bitmask_valueBE函数，将位掩码值写入流中
  return dds_stream_write_bitmask_valueBE((dds_ostreamBE_t *)os, allocator, insn, addr, bits_h,
                                          bits_l);
}

/**
 * @brief 将位掩码数组写入dds_ostream_t流中
 *
 * @param[in] os 指向dds_ostream_t结构体的指针
 * @param[in] allocator 指向dds_cdrstream_allocator结构体的指针
 * @param[in] insn 指令
 * @param[in] addr 要写入的位掩码数组的地址
 * @param[in] num 位掩码数组的元素数量
 * @param[in] bits_h 高位掩码
 * @param[in] bits_l 低位掩码
 * @return 写入成功返回true，否则返回false
 */
static inline bool dds_stream_write_bitmask_arr(
    dds_ostream_t *__restrict os,
    const struct dds_cdrstream_allocator *__restrict allocator,
    uint32_t insn,
    const void *__restrict addr,
    uint32_t num,
    uint32_t bits_h,
    uint32_t bits_l) {
  // 调用dds_stream_write_bitmask_arrBE函数，将位掩码数组写入流中
  return dds_stream_write_bitmask_arrBE((dds_ostreamBE_t *)os, allocator, insn, addr, num, bits_h,
                                        bits_l);
}

/**
 * @brief 将数据写入dds_ostream_t流中
 *
 * @param[in] os 指向dds_ostream_t结构体的指针
 * @param[in] allocator 指向dds_cdrstream_allocator结构体的指针
 * @param[in] data 要写入的数据
 * @param[in] ops 操作指针
 * @return 写入成功返回操作指针，否则返回NULL
 */
const uint32_t *dds_stream_write(dds_ostream_t *__restrict os,
                                 const struct dds_cdrstream_allocator *__restrict allocator,
                                 const char *__restrict data,
                                 const uint32_t *__restrict ops) {
  // 调用dds_stream_writeBE函数，将数据写入流中
  return dds_stream_writeBE((dds_ostreamBE_t *)os, allocator, data, ops);
}

/**
 * @brief 将样本数据写入dds_ostream_t流中
 *
 * @param[in] os 指向dds_ostream_t结构体的指针
 * @param[in] allocator 指向dds_cdrstream_allocator结构体的指针
 * @param[in] data 要写入的样本数据
 * @param[in] desc 指向dds_cdrstream_desc结构体的指针
 * @return 写入成功返回true，否则返回false
 */
bool dds_stream_write_sample(dds_ostream_t *__restrict os,
                             const struct dds_cdrstream_allocator *__restrict allocator,
                             const void *__restrict data,
                             const struct dds_cdrstream_desc *__restrict desc) {
  // 调用dds_stream_write_sampleBE函数，将样本数据写入流中
  return dds_stream_write_sampleBE((dds_ostreamBE_t *)os, allocator, data, desc);
}

/**
 * @brief 将样本数据以小端字节序写入dds_ostreamLE_t流中
 *
 * @param[in] os 指向dds_ostreamLE_t结构体的指针
 * @param[in] allocator 指向dds_cdrstream_allocator结构体的指针
 * @param[in] data 要写入的样本数据
 * @param[in] desc 指向dds_cdrstream_desc结构体的指针
 * @return 写入成功返回true，否则返回false
 */
bool dds_stream_write_sampleLE(dds_ostreamLE_t *__restrict os,
                               const struct dds_cdrstream_allocator *__restrict allocator,
                               const void *__restrict data,
                               const struct dds_cdrstream_desc *__restrict desc) {
  // 调用dds_stream_writeLE函数，将样本数据以小端字节序写入流中
  return dds_stream_writeLE(os, allocator, data, desc->ops.ops) != NULL;
}

/**
 * @brief 以大端字节序（Big-Endian）写入DDS流样本。
 *
 * @param[in] os 指向dds_ostreamBE_t结构的指针，用于输出流操作。
 * @param[in] allocator 指向dds_cdrstream_allocator结构的指针，用于内存分配。
 * @param[in] data 指向需要写入的数据的指针。
 * @param[in] desc 指向dds_cdrstream_desc结构的指针，描述数据的类型和布局。
 * @return 如果成功写入样本，则返回true，否则返回false。
 */
bool dds_stream_write_sampleBE(dds_ostreamBE_t *__restrict os,
                               const struct dds_cdrstream_allocator *__restrict allocator,
                               const void *__restrict data,
                               const struct dds_cdrstream_desc *__restrict desc) {
  // 根据os的m_xcdr_version字段确定opt_size的值
  size_t opt_size = os->x.m_xcdr_version == DDSI_RTPS_CDR_ENC_VERSION_1 ? desc->opt_size_xcdr1
                                                                        : desc->opt_size_xcdr2;

  // 判断opt_size是否非0，desc->align是否非0，以及当前索引是否为desc->align的倍数
  if (opt_size && desc->align && (((struct dds_ostream *)os)->m_index % desc->align) == 0) {
    // 将data指向的数据按照opt_size的大小写入到os指向的输出流中
    dds_os_put_bytes((struct dds_ostream *)os, data, (uint32_t)opt_size);

    // 写入成功，返回true
    return true;
  } else {
    // 调用dds_stream_writeBE函数，如果返回值不为NULL，则表示写入成功，返回true；否则返回false
    return dds_stream_writeBE(os, allocator, data, desc->ops.ops) != NULL;
  }
}

#endif /* if DDSRT_ENDIAN == DDSRT_LITTLE_ENDIAN */

/**
 * @brief 以指定的字节顺序将数据写入DDS流。
 *
 * @param os            [in] 指向dds_ostream_t结构体的指针，用于输出流操作。
 * @param allocator     [in] 指向dds_cdrstream_allocator结构体的指针，用于内存分配操作。
 * @param data          [in] 指向要写入流的数据的指针。
 * @param ops           [in] 指向uint32_t类型的指针，用于操作。
 * @param bo            [in] 枚举类型ddsrt_byte_order_selector，表示字节顺序选择。
 *
 * @return 返回一个指向uint32_t类型的指针，表示写入操作的结果。
 */
const uint32_t *dds_stream_write_with_byte_order(
    dds_ostream_t *__restrict os,
    const struct dds_cdrstream_allocator *__restrict allocator,
    const char *__restrict data,
    const uint32_t *__restrict ops,
    enum ddsrt_byte_order_selector bo) {
  // 如果字节顺序为小端（Little Endian）
  if (bo == DDSRT_BOSEL_LE) return dds_stream_writeLE((dds_ostreamLE_t *)os, allocator, data, ops);
  // 如果字节顺序为大端（Big Endian）
  else if (bo == DDSRT_BOSEL_BE)
    return dds_stream_writeBE((dds_ostreamBE_t *)os, allocator, data, ops);
  // 其他情况
  else
    return dds_stream_write(os, allocator, data, ops);
}

/**
 * @brief 根据需要重新分配序列缓冲区。
 *
 * @param seq           [in] 指向dds_sequence_t结构体的指针，表示序列。
 * @param allocator     [in] 指向dds_cdrstream_allocator结构体的指针，用于内存分配操作。
 * @param num           [in] uint32_t类型，表示元素数量。
 * @param elem_size     [in] uint32_t类型，表示单个元素的大小。
 * @param init          [in] 布尔值，表示是否初始化缓冲区。
 */
static void realloc_sequence_buffer_if_needed(
    dds_sequence_t *__restrict seq,
    const struct dds_cdrstream_allocator *__restrict allocator,
    uint32_t num,
    uint32_t elem_size,
    bool init) {
  // 计算所需的缓冲区大小
  const uint32_t size = num * elem_size;

  // 维护最大序列长度（可能未被调用者设置）
  if (seq->_length > seq->_maximum) seq->_maximum = seq->_length;

  // 如果需要的元素数量大于最大值且允许释放
  if (num > seq->_maximum && seq->_release) {
    // 重新分配缓冲区大小
    seq->_buffer = allocator->realloc(seq->_buffer, size);
    // 如果需要初始化
    if (init) {
      // 计算偏移量并将新分配的内存空间清零
      const uint32_t off = seq->_maximum * elem_size;
      memset(seq->_buffer + off, 0, size - off);
    }
    // 更新最大值
    seq->_maximum = num;
  }
  // 如果需要的元素数量大于0且最大值为0
  else if (num > 0 && seq->_maximum == 0) {
    // 分配缓冲区大小
    seq->_buffer = allocator->malloc(size);
    // 如果需要初始化，将缓冲区清零
    if (init) memset(seq->_buffer, 0, size);
    // 设置允许释放标志
    seq->_release = true;
    // 更新最大值
    seq->_maximum = num;
  }
}

/**
 * @brief 检查数据流中是否存在成员
 *
 * 该函数用于检查数据流中是否存在指定的成员，同时考虑了成员是否为可选类型以及是否为可变类型。
 *
 * @param[in] insn 成员的指令值（32位无符号整数）
 * @param[in] is 数据流指针（dds_istream_t 类型）
 * @param[in] is_mutable_member 布尔值，表示成员是否为可变类型
 * @return 如果成员存在，则返回 true；否则返回 false
 */
static bool stream_is_member_present(uint32_t insn,
                                     dds_istream_t *__restrict is,
                                     bool is_mutable_member) {
  // 判断成员是否为可选类型，如果不是可选类型或者是可变类型或者在数据流中找到该成员，则返回 true
  return !op_type_optional(insn) || is_mutable_member || dds_is_get1(is);
}

/**
 * @brief 读取序列数据并将其存储在指定的地址中。
 *
 * @param[in] is 输入流，用于从其中读取序列数据。
 * @param[out] addr 存储序列数据的地址。
 * @param[in] allocator 分配器，用于分配和释放内存。
 * @param[in] ops 操作数组，包含序列元素类型的操作。
 * @param[in] insn 指令，包含序列元素的类型信息。
 * @return 返回处理后的操作数组。
 */
static const uint32_t *dds_stream_read_seq(
    dds_istream_t *__restrict is,
    char *__restrict addr,
    const struct dds_cdrstream_allocator *__restrict allocator,
    const uint32_t *__restrict ops,
    uint32_t insn) {
  // 将地址转换为 dds_sequence_t 类型的指针
  dds_sequence_t *const seq = (dds_sequence_t *)addr;

  // 获取序列元素的子类型
  const enum dds_stream_typecode subtype = DDS_OP_SUBTYPE(insn);

  // 判断序列是否有边界限制
  uint32_t bound_op = seq_is_bounded(DDS_OP_TYPE(insn)) ? 1 : 0;

  // 如果需要 DHEADER，则跳过它
  if (is_dheader_needed(subtype, is->m_xcdr_version)) {
    /* skip DHEADER */
    dds_is_get4(is);
  }

  // 读取序列元素的数量
  const uint32_t num = dds_is_get4(is);

  // 如果序列元素数量为0，设置序列长度为0并跳过序列指令
  if (num == 0) {
    seq->_length = 0;
    return skip_sequence_insns(insn, ops);
  }

  // 根据子类型处理序列元素
  switch (subtype) {
    case DDS_OP_VAL_BLN:
    case DDS_OP_VAL_1BY:
    case DDS_OP_VAL_2BY:
    case DDS_OP_VAL_4BY:
    case DDS_OP_VAL_8BY: {
      // 获取基本类型的大小
      const uint32_t elem_size = get_primitive_size(subtype);

      // 如果需要，重新分配序列缓冲区
      realloc_sequence_buffer_if_needed(seq, allocator, num, elem_size, false);

      // 设置序列长度
      seq->_length = (num <= seq->_maximum) ? num : seq->_maximum;

      // 从输入流中读取字节并存储到序列缓冲区中
      dds_is_get_bytes(is, seq->_buffer, seq->_length, elem_size);

      // 如果实际读取的长度小于元素数量，则跳过剩余的字节
      if (seq->_length < num) dds_stream_skip_forward(is, num - seq->_length, elem_size);

      return ops + 2 + bound_op;
    }
    case DDS_OP_VAL_ENU: {
      // 获取枚举类型的大小
      const uint32_t elem_size = DDS_OP_TYPE_SZ(insn);

      // 如果需要，重新分配序列缓冲区
      realloc_sequence_buffer_if_needed(seq, allocator, num, 4, false);

      // 设置序列长度
      seq->_length = (num <= seq->_maximum) ? num : seq->_maximum;

      // 根据枚举类型的大小从输入流中读取数据并存储到序列缓冲区中
      switch (elem_size) {
        case 1:
          for (uint32_t i = 0; i < seq->_length; i++)
            ((uint32_t *)seq->_buffer)[i] = dds_is_get1(is);
          break;
        case 2:
          for (uint32_t i = 0; i < seq->_length; i++)
            ((uint32_t *)seq->_buffer)[i] = dds_is_get2(is);
          break;
        case 4:
          dds_is_get_bytes(is, seq->_buffer, seq->_length, elem_size);
          break;
      }

      // 如果实际读取的长度小于元素数量，则跳过剩余的字节
      if (seq->_length < num) dds_stream_skip_forward(is, num - seq->_length, elem_size);

      return ops + 3 + bound_op;
    }
    case DDS_OP_VAL_BMK: {
      // 获取位掩码类型的大小
      const uint32_t elem_size = DDS_OP_TYPE_SZ(insn);

      // 如果需要，重新分配序列缓冲区
      realloc_sequence_buffer_if_needed(seq, allocator, num, elem_size, false);

      // 设置序列长度
      seq->_length = (num <= seq->_maximum) ? num : seq->_maximum;

      // 从输入流中读取字节并存储到序列缓冲区中
      dds_is_get_bytes(is, seq->_buffer, seq->_length, elem_size);

      // 如果实际读取的长度小于元素数量，则跳过剩余的字节
      if (seq->_length < num) dds_stream_skip_forward(is, num - seq->_length, elem_size);

      return ops + 4 + bound_op;
    }
    case DDS_OP_VAL_STR: {
      // 如果需要，重新分配序列缓冲区
      realloc_sequence_buffer_if_needed(seq, allocator, num, sizeof(char *), true);

      // 设置序列长度
      seq->_length = (num <= seq->_maximum) ? num : seq->_maximum;

      // 读取字符串并存储到序列缓冲区中
      char **ptr = (char **)seq->_buffer;
      for (uint32_t i = 0; i < seq->_length; i++)
        ptr[i] = dds_stream_reuse_string(is, ptr[i], allocator);

      // 跳过剩余的字符串
      for (uint32_t i = seq->_length; i < num; i++) dds_stream_skip_string(is);

      return ops + 2 + bound_op;
    }
    case DDS_OP_VAL_BST: {
      // 获取有界字符串类型的大小
      const uint32_t elem_size = ops[2 + bound_op];

      // 如果需要，重新分配序列缓冲区
      realloc_sequence_buffer_if_needed(seq, allocator, num, elem_size, false);

      // 设置序列长度
      seq->_length = (num <= seq->_maximum) ? num : seq->_maximum;

      // 读取有界字符串并存储到序列缓冲区中
      char *ptr = (char *)seq->_buffer;
      for (uint32_t i = 0; i < seq->_length; i++)
        (void)dds_stream_reuse_string_bound(is, ptr + i * elem_size, allocator, elem_size, false);

      // 跳过剩余的字符串
      for (uint32_t i = seq->_length; i < num; i++) dds_stream_skip_string(is);

      return ops + 3 + bound_op;
    }
    case DDS_OP_VAL_SEQ:
    case DDS_OP_VAL_BSQ:
    case DDS_OP_VAL_ARR:
    case DDS_OP_VAL_UNI:
    case DDS_OP_VAL_STU: {
      // 获取复合类型的大小
      const uint32_t elem_size = ops[2 + bound_op];

      // 获取跳转指令和子操作数组
      const uint32_t jmp = DDS_OP_ADR_JMP(ops[3 + bound_op]);
      uint32_t const *const jsr_ops = ops + DDS_OP_ADR_JSR(ops[3 + bound_op]);

      // 如果需要，重新分配序列缓冲区
      realloc_sequence_buffer_if_needed(seq, allocator, num, elem_size, true);

      // 设置序列长度
      seq->_length = (num <= seq->_maximum) ? num : seq->_maximum;

      // 读取复合类型并存储到序列缓冲区中
      char *ptr = (char *)seq->_buffer;
      for (uint32_t i = 0; i < num; i++)
        (void)dds_stream_read_impl(is, ptr + i * elem_size, allocator, jsr_ops, false);

      return ops + (jmp ? jmp : (4 + bound_op)); /* FIXME: why would jmp be 0? */
    }
    case DDS_OP_VAL_EXT: {
      // 扩展类型不支持，直接中止程序
      abort(); /* not supported */
      break;
    }
  }

  return NULL;
}

/**
 * @brief 读取数组数据并返回操作列表的下一个位置。
 *
 * @param[in] is          输入流指针，用于读取数据。
 * @param[out] addr       存储读取到的数据的地址。
 * @param[in] allocator   分配器指针，用于分配内存。
 * @param[in] ops         操作列表指针，用于指示如何处理数据。
 * @param[in] insn        指令，用于确定子类型和其他信息。
 * @return const uint32_t* 返回操作列表的下一个位置。
 */
static const uint32_t *dds_stream_read_arr(
    dds_istream_t *__restrict is,
    char *__restrict addr,
    const struct dds_cdrstream_allocator *__restrict allocator,
    const uint32_t *__restrict ops,
    uint32_t insn) {
  // 获取子类型
  const enum dds_stream_typecode subtype = DDS_OP_SUBTYPE(insn);

  // 如果需要 DHEADER，则跳过
  if (is_dheader_needed(subtype, is->m_xcdr_version)) {
    /* skip DHEADER */
    dds_is_get4(is);
  }

  // 获取元素数量
  const uint32_t num = ops[2];

  // 根据子类型进行处理
  switch (subtype) {
    case DDS_OP_VAL_BLN:
    case DDS_OP_VAL_1BY:
    case DDS_OP_VAL_2BY:
    case DDS_OP_VAL_4BY:
    case DDS_OP_VAL_8BY: {
      // 获取基本类型的大小
      const uint32_t elem_size = get_primitive_size(subtype);

      // 读取字节数据
      dds_is_get_bytes(is, addr, num, elem_size);

      // 返回操作列表的下一个位置
      return ops + 3;
    }
    case DDS_OP_VAL_ENU: {
      // 根据枚举类型的大小进行处理
      switch (DDS_OP_TYPE_SZ(insn)) {
        case 1:
          for (uint32_t i = 0; i < num; i++) ((uint32_t *)addr)[i] = dds_is_get1(is);
          break;
        case 2:
          for (uint32_t i = 0; i < num; i++) ((uint32_t *)addr)[i] = dds_is_get2(is);
          break;
        case 4:
          dds_is_get_bytes(is, addr, num, 4);
          break;
        default:
          abort();
      }

      // 返回操作列表的下一个位置
      return ops + 4;
    }
    case DDS_OP_VAL_BMK: {
      // 获取元素大小
      const uint32_t elem_size = DDS_OP_TYPE_SZ(insn);

      // 读取字节数据
      dds_is_get_bytes(is, addr, num, elem_size);

      // 返回操作列表的下一个位置
      return ops + 5;
    }
    case DDS_OP_VAL_STR: {
      // 初始化字符串指针数组
      char **ptr = (char **)addr;

      // 读取并重用字符串
      for (uint32_t i = 0; i < num; i++) ptr[i] = dds_stream_reuse_string(is, ptr[i], allocator);

      // 返回操作列表的下一个位置
      return ops + 3;
    }
    case DDS_OP_VAL_BST: {
      // 初始化字符指针
      char *ptr = (char *)addr;

      // 获取元素大小
      const uint32_t elem_size = ops[4];

      // 读取并重用有界字符串
      for (uint32_t i = 0; i < num; i++)
        (void)dds_stream_reuse_string_bound(is, ptr + i * elem_size, allocator, elem_size, false);

      // 返回操作列表的下一个位置
      return ops + 5;
    }
    case DDS_OP_VAL_SEQ:
    case DDS_OP_VAL_BSQ:
    case DDS_OP_VAL_ARR:
    case DDS_OP_VAL_UNI:
    case DDS_OP_VAL_STU: {
      // 获取跳转操作列表
      const uint32_t *jsr_ops = ops + DDS_OP_ADR_JSR(ops[3]);

      // 获取跳转地址
      const uint32_t jmp = DDS_OP_ADR_JMP(ops[3]);

      // 获取元素大小
      const uint32_t elem_size = ops[4];

      // 逐个读取元素数据
      for (uint32_t i = 0; i < num; i++)
        (void)dds_stream_read_impl(is, addr + i * elem_size, allocator, jsr_ops, false);

      // 返回操作列表的下一个位置
      return ops + (jmp ? jmp : 5);
    }
    case DDS_OP_VAL_EXT: {
      abort(); /* not supported */
      break;
    }
  }

  // 如果没有匹配的子类型，返回 NULL
  return NULL;
}

/**
 * @brief 读取并解析DDS流中的联合体数据
 *
 * @param[in] is          输入流指针
 * @param[out] discaddr   联合体判别符地址
 * @param[out] baseaddr   联合体基地址
 * @param[in] allocator   内存分配器指针
 * @param[in] ops         操作码数组指针
 * @param[in] insn        当前操作码
 * @return const uint32_t* 更新后的操作码数组指针
 */
static const uint32_t *dds_stream_read_uni(
    dds_istream_t *__restrict is,
    char *__restrict discaddr,
    char *__restrict baseaddr,
    const struct dds_cdrstream_allocator *__restrict allocator,
    const uint32_t *__restrict ops,
    uint32_t insn) {
  // 读取联合体判别符
  const uint32_t disc = read_union_discriminant(is, insn);

  // 根据判别符类型设置discaddr的值
  switch (DDS_OP_SUBTYPE(insn)) {
    case DDS_OP_VAL_BLN:
    case DDS_OP_VAL_1BY:
      *((uint8_t *)discaddr) = (uint8_t)disc;
      break;
    case DDS_OP_VAL_2BY:
      *((uint16_t *)discaddr) = (uint16_t)disc;
      break;
    case DDS_OP_VAL_4BY:
    case DDS_OP_VAL_ENU:
      *((uint32_t *)discaddr) = disc;
      break;
    default:
      break;
  }

  // 查找与判别符匹配的联合体成员
  uint32_t const *const jeq_op = find_union_case(ops, disc);

  // 更新操作码数组指针
  ops += DDS_OP_ADR_JMP(ops[3]);

  // 如果找到匹配的联合体成员
  if (jeq_op) {
    // 获取值类型和值地址
    const enum dds_stream_typecode valtype = DDS_JEQ_TYPE(jeq_op[0]);
    void *valaddr = baseaddr + jeq_op[2];

    // 如果是外部类型，分配内存并初始化
    if (op_type_external(jeq_op[0])) {
      assert(DDS_OP(jeq_op[0]) == DDS_OP_JEQ4);
      uint32_t sz = get_jeq4_type_size(valtype, jeq_op);
      if (*((char **)valaddr) == NULL) {
        *((char **)valaddr) = allocator->malloc(sz);
        memset(*((char **)valaddr), 0, sz);
      }
      valaddr = *((char **)valaddr);
    }

    // 根据值类型读取并设置值
    switch (valtype) {
      case DDS_OP_VAL_BLN:
      case DDS_OP_VAL_1BY:
        *((uint8_t *)valaddr) = dds_is_get1(is);
        break;
      case DDS_OP_VAL_2BY:
        *((uint16_t *)valaddr) = dds_is_get2(is);
        break;
      case DDS_OP_VAL_4BY:
        *((uint32_t *)valaddr) = dds_is_get4(is);
        break;
      case DDS_OP_VAL_8BY:
        *((uint64_t *)valaddr) = dds_is_get8(is);
        break;
      case DDS_OP_VAL_ENU:
        switch (DDS_OP_TYPE_SZ(jeq_op[0])) {
          case 1:
            *((uint32_t *)valaddr) = dds_is_get1(is);
            break;
          case 2:
            *((uint32_t *)valaddr) = dds_is_get2(is);
            break;
          case 4:
            *((uint32_t *)valaddr) = dds_is_get4(is);
            break;
          default:
            abort();
        }
        break;
      case DDS_OP_VAL_STR:
        *(char **)valaddr = dds_stream_reuse_string(is, *((char **)valaddr), allocator);
        break;
      case DDS_OP_VAL_BST:
      case DDS_OP_VAL_SEQ:
      case DDS_OP_VAL_BSQ:
      case DDS_OP_VAL_ARR:
      case DDS_OP_VAL_BMK:
        (void)dds_stream_read_impl(is, valaddr, allocator, jeq_op + DDS_OP_ADR_JSR(jeq_op[0]),
                                   false);
        break;
      case DDS_OP_VAL_UNI:
      case DDS_OP_VAL_STU: {
        const uint32_t *jsr_ops = jeq_op + DDS_OP_ADR_JSR(jeq_op[0]);
        (void)dds_stream_read_impl(is, valaddr, allocator, jsr_ops, false);
        break;
      }
      case DDS_OP_VAL_EXT: {
        abort(); /* not supported */
        break;
      }
    }
  }

  // 返回更新后的操作码数组指针
  return ops;
}

/**
 * @brief 为 @external 成员分配内存并初始化。
 * @param[in] ops 指向操作列表的指针。
 * @param[in] insn 包含类型信息的指令。
 * @param[in,out] addr 指向要分配内存的地址的指针。
 * @param[in] allocator 指向 dds_cdrstream_allocator 结构体的指针，用于分配内存。
 */
static void dds_stream_alloc_external(const uint32_t *__restrict ops,
                                      uint32_t insn,
                                      void **addr,
                                      const struct dds_cdrstream_allocator *__restrict allocator) {
  // 为 @external 成员分配内存。这段内存必须初始化为 0，
  // 因为类型可能包含需要将索引/大小设置为 0 的序列，
  // 或者需要初始化为空的外部字段。
  uint32_t sz = get_adr_type_size(insn, ops);

  // 如果 *addr 为空，则为其分配内存。
  if (*((char **)*addr) == NULL) {
    // 使用 allocator->malloc 分配内存，并将地址赋值给 *addr。
    *((char **)*addr) = allocator->malloc(sz);
    // 将分配的内存初始化为 0。
    memset(*((char **)*addr), 0, sz);
  }

  // 更新 addr 指向的地址。
  *addr = *((char **)*addr);
}

/**
 * @brief 读取数据流中的地址
 *
 * @param[in] insn 指令
 * @param[in] is 输入数据流
 * @param[in,out] data 数据缓冲区
 * @param[in] allocator 内存分配器
 * @param[in] ops 操作指针
 * @param[in] is_mutable_member 是否为可变成员
 * @return 返回操作指针
 */
static inline const uint32_t *dds_stream_read_adr(
    uint32_t insn,
    dds_istream_t *__restrict is,
    char *__restrict data,
    const struct dds_cdrstream_allocator *__restrict allocator,
    const uint32_t *__restrict ops,
    bool is_mutable_member) {
  // 计算地址
  void *addr = data + ops[1];

  // 如果成员不存在，则释放样本地址并返回
  if (!stream_is_member_present(insn, is, is_mutable_member))
    return stream_free_sample_adr(insn, data, allocator, ops);

  // 如果是外部类型，分配内存
  if (op_type_external(insn)) dds_stream_alloc_external(ops, insn, &addr, allocator);

  // 根据指令类型进行处理
  switch (DDS_OP_TYPE(insn)) {
    case DDS_OP_VAL_BLN:
    case DDS_OP_VAL_1BY:
      *((uint8_t *)addr) = dds_is_get1(is);
      ops += 2;
      break;
    case DDS_OP_VAL_2BY:
      *((uint16_t *)addr) = dds_is_get2(is);
      ops += 2;
      break;
    case DDS_OP_VAL_4BY:
      *((uint32_t *)addr) = dds_is_get4(is);
      ops += 2;
      break;
    case DDS_OP_VAL_8BY:
      *((uint64_t *)addr) = dds_is_get8(is);
      ops += 2;
      break;
    case DDS_OP_VAL_STR:
      *((char **)addr) = dds_stream_reuse_string(is, *((char **)addr), allocator);
      ops += 2;
      break;
    case DDS_OP_VAL_BST:
      (void)dds_stream_reuse_string_bound(is, (char *)addr, allocator, ops[2], false);
      ops += 3;
      break;
    case DDS_OP_VAL_SEQ:
    case DDS_OP_VAL_BSQ:
      ops = dds_stream_read_seq(is, addr, allocator, ops, insn);
      break;
    case DDS_OP_VAL_ARR:
      ops = dds_stream_read_arr(is, addr, allocator, ops, insn);
      break;
    case DDS_OP_VAL_UNI:
      ops = dds_stream_read_uni(is, addr, data, allocator, ops, insn);
      break;
    case DDS_OP_VAL_ENU: {
      switch (DDS_OP_TYPE_SZ(insn)) {
        case 1:
          *((uint32_t *)addr) = dds_is_get1(is);
          break;
        case 2:
          *((uint32_t *)addr) = dds_is_get2(is);
          break;
        case 4:
          *((uint32_t *)addr) = dds_is_get4(is);
          break;
        default:
          abort();
      }
      ops += 3;
      break;
    }
    case DDS_OP_VAL_BMK: {
      switch (DDS_OP_TYPE_SZ(insn)) {
        case 1:
          *((uint8_t *)addr) = dds_is_get1(is);
          break;
        case 2:
          *((uint16_t *)addr) = dds_is_get2(is);
          break;
        case 4:
          *((uint32_t *)addr) = dds_is_get4(is);
          break;
        case 8:
          *((uint64_t *)addr) = dds_is_get8(is);
          break;
        default:
          abort();
      }
      ops += 4;
      break;
    }
    case DDS_OP_VAL_EXT: {
      const uint32_t *jsr_ops = ops + DDS_OP_ADR_JSR(ops[2]);
      const uint32_t jmp = DDS_OP_ADR_JMP(ops[2]);

      // 跳过基本类型的DLC指令，将其视为最终类型
      if (op_type_base(insn) && jsr_ops[0] == DDS_OP_DLC) jsr_ops++;

      (void)dds_stream_read_impl(is, addr, allocator, jsr_ops, false);
      ops += jmp ? jmp : 3;
      break;
    }
    case DDS_OP_VAL_STU:
      abort();
      break;  // op type STU only supported as subtype
  }
  return ops;
}

/**
 * @brief 跳过指定的dds_stream中的地址操作数
 * @param[in] insn 指令
 * @param[in] __restrict ops 受限制的操作数指针
 * @return 返回跳过地址操作数后的指针位置
 */
static const uint32_t *dds_stream_skip_adr(uint32_t insn, const uint32_t *__restrict ops) {
  // 根据指令类型进行判断
  switch (DDS_OP_TYPE(insn)) {
    case DDS_OP_VAL_BLN:  // 布尔值
    case DDS_OP_VAL_1BY:  // 1字节值
    case DDS_OP_VAL_2BY:  // 2字节值
    case DDS_OP_VAL_4BY:  // 4字节值
    case DDS_OP_VAL_8BY:  // 8字节值
    case DDS_OP_VAL_STR:  // 字符串
      return ops + 2;
    case DDS_OP_VAL_BST:  // 比特集
    case DDS_OP_VAL_ENU:  // 枚举
      return ops + 3;
    case DDS_OP_VAL_BMK:  // 位掩码
      return ops + 4;
    case DDS_OP_VAL_SEQ:  // 序列
    case DDS_OP_VAL_BSQ:  // 位序列
      return skip_sequence_insns(insn, ops);
    case DDS_OP_VAL_ARR:  // 数组
      return skip_array_insns(insn, ops);
    case DDS_OP_VAL_UNI:  // 联合体
    {
      const uint32_t jmp = DDS_OP_ADR_JMP(ops[3]);
      return ops + (jmp ? jmp : 4); /* FIXME: jmp不能为0？ */
    }
    case DDS_OP_VAL_EXT:  // 扩展类型
    {
      const uint32_t jmp = DDS_OP_ADR_JMP(ops[2]);
      return ops + (jmp ? jmp : 3);
    }
    case DDS_OP_VAL_STU:  // 结构体
    {
      abort(); /* op type STU仅作为子类型支持 */
      break;
    }
  }
  return NULL;
}

/**
 * @brief 跳过默认地址的DDS流
 *
 * @param[in] insn 指令
 * @param[in,out] data 数据指针
 * @param[in] allocator 内存分配器
 * @param[in] ops 操作列表
 * @return const uint32_t* 更新后的操作列表指针
 *
 * @note 当前仅使用隐式默认值，此代码应使用类型定义中指定的默认值
 */
static const uint32_t *dds_stream_skip_adr_default(
    uint32_t insn,
    char *__restrict data,
    const struct dds_cdrstream_allocator *__restrict allocator,
    const uint32_t *__restrict ops) {
  void *addr = data + ops[1];  // 计算地址

  /* 如果是可选或外部成员，则释放样本中的内存并将指针设置为null。
     对于字符串类型，这是一个例外，它不会获得外部标志 */
  if (op_type_external(insn) || op_type_optional(insn))
    return stream_free_sample_adr(insn, data, allocator, ops);

  switch (DDS_OP_TYPE(insn))  // 根据指令类型进行处理
  {
    case DDS_OP_VAL_BLN:
    case DDS_OP_VAL_1BY:
      *(uint8_t *)addr = 0;
      return ops + 2;
    case DDS_OP_VAL_2BY:
      *(uint16_t *)addr = 0;
      return ops + 2;
    case DDS_OP_VAL_4BY:
      *(uint32_t *)addr = 0;
      return ops + 2;
    case DDS_OP_VAL_8BY:
      *(uint64_t *)addr = 0;
      return ops + 2;

    case DDS_OP_VAL_STR:
      *(char **)addr = dds_stream_reuse_string_empty(*(char **)addr, allocator);
      return ops + 2;
    case DDS_OP_VAL_BST:
      ((char *)addr)[0] = '\0';
      return ops + 3;
    case DDS_OP_VAL_ENU:
      *(uint32_t *)addr = 0;
      return ops + 3;
    case DDS_OP_VAL_BMK:
      switch (DDS_OP_TYPE_SZ(insn)) {
        case 1:
          *(uint8_t *)addr = 0;
          break;
        case 2:
          *(uint16_t *)addr = 0;
          break;
        case 4:
          *(uint32_t *)addr = 0;
          break;
        case 8:
          *(uint64_t *)addr = 0;
          break;
        default:
          abort();
      }
      return ops + 4;
    case DDS_OP_VAL_SEQ:
    case DDS_OP_VAL_BSQ: {
      dds_sequence_t *const seq = (dds_sequence_t *)addr;
      seq->_length = 0;
      return skip_sequence_insns(insn, ops);
    }
    case DDS_OP_VAL_ARR: {
      return skip_array_default(insn, addr, allocator, ops);
    }
    case DDS_OP_VAL_UNI: {
      return skip_union_default(insn, addr, data, allocator, ops);
    }
    case DDS_OP_VAL_EXT: {
      const uint32_t *jsr_ops = ops + DDS_OP_ADR_JSR(ops[2]);
      const uint32_t jmp = DDS_OP_ADR_JMP(ops[2]);
      (void)dds_stream_skip_default(addr, allocator, jsr_ops);
      return ops + (jmp ? jmp : 3);
    }
    case DDS_OP_VAL_STU: {
      abort(); /* op type STU only supported as subtype */
      break;
    }
  }

  return NULL;
}

/**
 * @brief 跳过指定的数据流中的分隔符，默认情况下使用 doxygen 格式。
 *
 * @param[in] data          输入数据流，需要跳过分隔符的数据。
 * @param[in] allocator     分配器，用于管理内存分配。
 * @param[in] ops           操作列表，用于指示如何处理数据流。
 * @return 返回跳过分隔符后的数据流指针。
 */
static const uint32_t *dds_stream_skip_delimited_default(
    char *__restrict data,
    const struct dds_cdrstream_allocator *__restrict allocator,
    const uint32_t *__restrict ops) {
  // 调用 dds_stream_skip_default 函数，并将操作列表递增。
  return dds_stream_skip_default(data, allocator, ++ops);
}

/**
 * @brief 跳过默认的 PL 成员数据流。
 *
 * @param[in] data          输入数据流，需要跳过 PL 成员的数据。
 * @param[in] allocator     分配器，用于管理内存分配。
 * @param[in] ops           操作列表，用于指示如何处理数据流。
 */
static void dds_stream_skip_pl_member_default(
    char *__restrict data,
    const struct dds_cdrstream_allocator *__restrict allocator,
    const uint32_t *__restrict ops) {
  uint32_t insn;  // 定义一个变量来存储指令。

  // 当指令不等于 DDS_OP_RTS 时，继续循环。
  while ((insn = *ops) != DDS_OP_RTS) {
    // 使用 switch 语句根据指令进行相应的操作。
    switch (DDS_OP(insn)) {
      case DDS_OP_ADR: {
        // 跳过默认的数据流，并更新操作列表。
        ops = dds_stream_skip_default(data, allocator, ops);
        break;
      }
      case DDS_OP_JSR:
        // 递归调用 dds_stream_skip_pl_member_default 函数，并根据指令跳转。
        dds_stream_skip_pl_member_default(data, allocator, ops + DDS_OP_JUMP(insn));
        ops++;
        break;
      case DDS_OP_RTS:
      case DDS_OP_JEQ:
      case DDS_OP_JEQ4:
      case DDS_OP_KOF:
      case DDS_OP_DLC:
      case DDS_OP_PLC:
      case DDS_OP_PLM:
        // 如果遇到以上情况，终止程序。
        abort();
        break;
    }
  }
}

/**
 * @brief 跳过默认的PL成员列表
 *
 * 该函数用于跳过默认的PL成员列表，支持doxygen格式。
 *
 * @param[in] data 输入数据
 * @param[in] allocator 分配器结构体指针
 * @param[in] ops 操作码数组指针
 * @return 返回操作码数组指针
 */
static const uint32_t *dds_stream_skip_pl_memberlist_default(
    char *__restrict data,
    const struct dds_cdrstream_allocator *__restrict allocator,
    const uint32_t *__restrict ops) {
  uint32_t insn;  // 定义指令变量

  // 当操作码数组存在且不为DDS_OP_RTS时，循环处理
  while (ops && (insn = *ops) != DDS_OP_RTS) {
    // 根据指令类型进行处理
    switch (DDS_OP(insn)) {
      case DDS_OP_PLM: {
        uint32_t flags = DDS_PLM_FLAGS(insn);                  // 获取标志位
        const uint32_t *plm_ops = ops + DDS_OP_ADR_PLM(insn);  // 计算PLM操作码地址

        // 如果标志位包含DDS_OP_FLAG_BASE
        if (flags & DDS_OP_FLAG_BASE) {
          assert(plm_ops[0] == DDS_OP_PLC);  // 断言第一个操作码为DDS_OP_PLC
          plm_ops++;  // 跳过PLC操作码，进入基本类型的第一个PLM
          (void)dds_stream_skip_pl_memberlist_default(data, allocator,
                                                      plm_ops);  // 递归调用跳过PL成员列表
        } else {
          dds_stream_skip_pl_member_default(data, allocator, plm_ops);  // 跳过PL成员默认值
        }
        ops += 2;  // 更新操作码数组指针
        break;
      }
      default:
        abort();  // 其他操作码类型暂不支持，中止程序
        break;
    }
  }
  return ops;  // 返回操作码数组指针
}

/**
 * @brief 跳过默认的PLC操作
 *
 * @param[in] data 输入数据
 * @param[in] allocator 分配器结构体指针
 * @param[in] ops 操作列表指针
 * @return 返回跳过PLC操作后的操作列表指针
 */
static const uint32_t *dds_stream_skip_pl_default(
    char *__restrict data,
    const struct dds_cdrstream_allocator *__restrict allocator,
    const uint32_t *__restrict ops) {
  /* 跳过 PLC op */
  return dds_stream_skip_pl_memberlist_default(data, allocator, ++ops);
}

/**
 * @brief 默认情况下跳过DDS流中的操作
 *
 * @param[in] data 输入数据
 * @param[in] allocator 分配器结构体指针
 * @param[in] ops 操作列表指针
 * @return 返回跳过操作后的操作列表指针
 */
static const uint32_t *dds_stream_skip_default(
    char *__restrict data,
    const struct dds_cdrstream_allocator *__restrict allocator,
    const uint32_t *__restrict ops) {
  uint32_t insn;  // 定义指令变量

  // 当指令不等于 DDS_OP_RTS 时，继续循环
  while ((insn = *ops) != DDS_OP_RTS) {
    // 根据指令类型进行相应处理
    switch (DDS_OP(insn)) {
      case DDS_OP_ADR: {
        // 跳过默认的ADR操作
        ops = dds_stream_skip_adr_default(insn, data, allocator, ops);
        break;
      }
      case DDS_OP_JSR: {
        // 跳过默认的JSR操作
        (void)dds_stream_skip_default(data, allocator, ops + DDS_OP_JUMP(insn));
        ops++;
        break;
      }
      case DDS_OP_RTS:
      case DDS_OP_JEQ:
      case DDS_OP_JEQ4:
      case DDS_OP_KOF:
      case DDS_OP_PLM:
        // 遇到以上指令类型时，终止程序
        abort();
        break;
      case DDS_OP_DLC:
        // 跳过默认的DLC操作
        ops = dds_stream_skip_delimited_default(data, allocator, ops);
        break;
      case DDS_OP_PLC:
        // 跳过默认的PLC操作
        ops = dds_stream_skip_pl_default(data, allocator, ops);
        break;
    }
  }

  // 返回跳过操作后的操作列表指针
  return ops;
}

/**
 * @brief 读取带有限制的数据流，并根据操作列表进行解析。
 *
 * @param[in] is          输入流指针，用于读取数据。
 * @param[out] data       存储解析后数据的缓冲区。
 * @param[in] allocator   分配器指针，用于内存分配。
 * @param[in] ops         操作列表指针，用于指导解析过程。
 * @return 返回处理后的操作列表指针。
 */
static const uint32_t *dds_stream_read_delimited(
    dds_istream_t *__restrict is,
    char *__restrict data,
    const struct dds_cdrstream_allocator *__restrict allocator,
    const uint32_t *__restrict ops) {
  // 读取限制大小
  uint32_t delimited_sz = dds_is_get4(is), delimited_offs = is->m_index, insn;
  ops++;

  // 循环处理操作列表，直到遇到返回操作
  while ((insn = *ops) != DDS_OP_RTS) {
    // 根据操作码进行相应处理
    switch (DDS_OP(insn)) {
      case DDS_OP_ADR: {
        // 跳过可追加类型中未在序列化数据中的字段
        ops = (is->m_index - delimited_offs < delimited_sz)
                  ? dds_stream_read_adr(insn, is, data, allocator, ops, false)
                  : dds_stream_skip_adr_default(insn, data, allocator, ops);
        break;
      }
      case DDS_OP_JSR: {
        // 调用子程序进行读取操作
        (void)dds_stream_read_impl(is, data, allocator, ops + DDS_OP_JUMP(insn), false);
        ops++;
        break;
      }
      case DDS_OP_RTS:
      case DDS_OP_JEQ:
      case DDS_OP_JEQ4:
      case DDS_OP_KOF:
      case DDS_OP_DLC:
      case DDS_OP_PLC:
      case DDS_OP_PLM: {
        // 遇到不支持的操作码，终止程序
        abort();
        break;
      }
    }
  }

  // 跳过此可追加类型的序列化数据剩余部分
  if (delimited_sz > is->m_index - delimited_offs)
    is->m_index += delimited_sz - (is->m_index - delimited_offs);

  return ops;
}

/**
 * @brief 从数据流中读取指定成员的值。
 *
 * 这个函数会在给定的操作列表中查找与给定成员ID匹配的成员，并从输入流中读取该成员的值。
 * 如果找到了匹配的成员，它将返回true，否则返回false。
 *
 * @param[in] is 输入流，包含要读取的数据。
 * @param[out] data 用于存储读取到的成员值的缓冲区。
 * @param[in] allocator 分配器，用于分配内存空间。
 * @param[in] m_id 要查找和读取的成员的ID。
 * @param[in] ops 操作列表，包含描述数据结构的操作。
 * @return 如果找到并成功读取了指定成员，则返回true，否则返回false。
 */
static bool dds_stream_read_pl_member(dds_istream_t *__restrict is,
                                      char *__restrict data,
                                      const struct dds_cdrstream_allocator *__restrict allocator,
                                      uint32_t m_id,
                                      const uint32_t *__restrict ops) {
  uint32_t insn, ops_csr = 0;
  bool found = false;

  // FIXME: 从上次找到的成员开始，在ops成员列表中继续查找成员，
  // 因为在许多情况下，成员将按顺序出现在数据中。
  while (!found && (insn = ops[ops_csr]) != DDS_OP_RTS) {
    // 确保当前操作是PLM操作。
    assert(DDS_OP(insn) == DDS_OP_PLM);
    uint32_t flags = DDS_PLM_FLAGS(insn);
    const uint32_t *plm_ops = ops + ops_csr + DDS_OP_ADR_PLM(insn);
    if (flags & DDS_OP_FLAG_BASE) {
      // 确保基类型的第一个操作是PLC操作。
      assert(DDS_OP(plm_ops[0]) == DDS_OP_PLC);
      plm_ops++;  // 跳过PLC，转到基类型的第一个PLM。
      found = dds_stream_read_pl_member(is, data, allocator, m_id, plm_ops);
    } else if (ops[ops_csr + 1] == m_id) {
      // 读取匹配成员的值。
      (void)dds_stream_read_impl(is, data, allocator, plm_ops, true);
      found = true;
      break;
    }
    ops_csr += 2;
  }
  return found;
}

/**
 * @brief 从数据流中读取并解析PL（Parameter List）结构。
 *
 * @param[in] is          输入数据流指针，用于读取数据。
 * @param[out] data       存储解析后的数据的缓冲区。
 * @param[in] allocator   分配器指针，用于内存分配。
 * @param[in] ops         操作码数组指针，用于指导解析过程。
 * @return 返回处理后的操作码数组指针。
 */
static const uint32_t *dds_stream_read_pl(
    dds_istream_t *__restrict is,
    char *__restrict data,
    const struct dds_cdrstream_allocator *__restrict allocator,
    const uint32_t *__restrict ops) {
  /* 跳过 PLC op */
  ops++;

  /* 默认初始化所有成员
      FIXME: 优化以便仅初始化未在接收到的数据中的成员 */
  dds_stream_skip_pl_memberlist_default(data, allocator, ops);

  /* 读取 DHEADER */
  uint32_t pl_sz = dds_is_get4(is), pl_offs = is->m_index;
  while (is->m_index - pl_offs < pl_sz) {
    /* 读取 EMHEADER 和 next_int */
    uint32_t em_hdr = dds_is_get4(is);
    uint32_t lc = EMHEADER_LENGTH_CODE(em_hdr), m_id = EMHEADER_MEMBERID(em_hdr), msz;
    switch (lc) {
      case LENGTH_CODE_1B:
      case LENGTH_CODE_2B:
      case LENGTH_CODE_4B:
      case LENGTH_CODE_8B:
        msz = 1u << lc;
        break;
      case LENGTH_CODE_NEXTINT:
        /* 读取 NEXTINT */
        msz = dds_is_get4(is);
        break;
      case LENGTH_CODE_ALSO_NEXTINT:
      case LENGTH_CODE_ALSO_NEXTINT4:
      case LENGTH_CODE_ALSO_NEXTINT8:
        /* 长度是序列化数据的一部分 */
        msz = dds_is_peek4(is);
        if (lc > LENGTH_CODE_ALSO_NEXTINT) msz <<= (lc - 4);
        break;
      default:
        abort();
        break;
    }

    /* 查找成员并反序列化 */
    if (!dds_stream_read_pl_member(is, data, allocator, m_id, ops)) {
      is->m_index += msz;
      if (lc >= LENGTH_CODE_ALSO_NEXTINT)
        is->m_index += 4; /* 成员中嵌入的长度不包括它自己的4个字节 */
    }
  }

  /* 跳过所有 PLM-memberid 对 */
  while (ops[0] != DDS_OP_RTS) ops += 2;

  return ops;
}

/**
 * @brief 读取DDS流的实现函数
 *
 * @param[in] is            输入流指针，用于读取数据
 * @param[out] data         存储读取到的数据的缓冲区
 * @param[in] allocator     分配器指针，用于分配内存
 * @param[in] ops           操作码数组，用于控制读取过程
 * @param[in] is_mutable_member 是否为可变成员，影响处理方式
 * @return const uint32_t*  返回操作码数组的当前位置
 */
static const uint32_t *dds_stream_read_impl(
    dds_istream_t *__restrict is,
    char *__restrict data,
    const struct dds_cdrstream_allocator *__restrict allocator,
    const uint32_t *__restrict ops,
    bool is_mutable_member) {
  uint32_t insn;                       // 指令变量
  while ((insn = *ops) != DDS_OP_RTS)  // 当指令不等于DDS_OP_RTS时循环
  {
    switch (DDS_OP(insn))  // 根据指令进行相应操作
    {
      case DDS_OP_ADR:
        ops = dds_stream_read_adr(insn, is, data, allocator, ops, is_mutable_member);
        break;
      case DDS_OP_JSR:
        (void)dds_stream_read_impl(is, data, allocator, ops + DDS_OP_JUMP(insn), is_mutable_member);
        ops++;
        break;
      case DDS_OP_RTS:
      case DDS_OP_JEQ:
      case DDS_OP_JEQ4:
      case DDS_OP_KOF:
      case DDS_OP_PLM:
        abort();
        break;
      case DDS_OP_DLC:
        assert(is->m_xcdr_version == DDSI_RTPS_CDR_ENC_VERSION_2);
        ops = dds_stream_read_delimited(is, data, allocator, ops);
        break;
      case DDS_OP_PLC:
        assert(is->m_xcdr_version == DDSI_RTPS_CDR_ENC_VERSION_2);
        ops = dds_stream_read_pl(is, data, allocator, ops);
        break;
    }
  }
  return ops;
}

/**
 * @brief 读取DDS流的函数
 *
 * @param[in] is            输入流指针，用于读取数据
 * @param[out] data         存储读取到的数据的缓冲区
 * @param[in] allocator     分配器指针，用于分配内存
 * @param[in] ops           操作码数组，用于控制读取过程
 * @return const uint32_t*  返回操作码数组的当前位置
 */
const uint32_t *dds_stream_read(dds_istream_t *__restrict is,
                                char *__restrict data,
                                const struct dds_cdrstream_allocator *__restrict allocator,
                                const uint32_t *__restrict ops) {
  return dds_stream_read_impl(is, data, allocator, ops, false);
}

/*******************************************************************************************
 **
 **  Validation and conversion to native endian.
 **
 *******************************************************************************************/

/**
 * @file normalize_error.c
 * @brief 本文件包含一组用于处理归一化错误的函数
 */
/**
 * @brief 对错误进行归一化处理
 *
 * 这个函数会对错误进行归一化处理，以便在其他函数中使用。
 */
static inline void normalize_error(void) {}
/**
 * @brief 获取归一化错误的偏移量
 *
 * @return 返回一个 uint32_t 类型的值，表示归一化错误的偏移量
 */
static inline uint32_t normalize_error_offset(void) {
  normalize_error();  // 调用 normalize_error() 函数对错误进行归一化处理
  return UINT32_MAX;  // 返回最大的 uint32_t 值作为归一化错误的偏移量
}
/**
 * @brief 判断是否存在归一化错误
 *
 * @return 返回一个 bool 类型的值，表示是否存在归一化错误
 */
static inline bool normalize_error_bool(void) {
  normalize_error();  // 调用 normalize_error() 函数对错误进行归一化处理
  return false;       // 返回 false，表示不存在归一化错误
}
/**
 * @brief 获取归一化错误操作的指针
 *
 * @return 返回一个指向 uint32_t 类型的常量指针，表示归一化错误操作的地址
 */
static inline const uint32_t *normalize_error_ops(void) {
  normalize_error();  // 调用 normalize_error() 函数对错误进行归一化处理
  return NULL;        // 返回 NULL，表示没有找到归一化错误操作的地址
}

/**
 * @file
 * @brief 用于检查对齐和规范化的函数
 */

/* 限制输入缓冲区的大小，以便我们不需要担心添加
   填充和原始类型溢出我们的偏移量 */
#define CDR_SIZE_MAX ((uint32_t)0xfffffff0)

/**
 * @brief 检查基本类型的对齐
 *
 * @param off 当前偏移量
 * @param size 缓冲区大小
 * @param a_lg2 对齐值的对数
 * @param c_lg2 基本类型大小的对数
 * @return 返回新的偏移量或错误偏移量
 */
static uint32_t check_align_prim(uint32_t off, uint32_t size, uint32_t a_lg2, uint32_t c_lg2) {
  assert(a_lg2 <= 3);
  const uint32_t a = 1u << a_lg2;
  assert(c_lg2 <= 3);
  const uint32_t c = 1u << c_lg2;
  assert(size <= CDR_SIZE_MAX);
  assert(off <= size);
  const uint32_t off1 = (off + a - 1) & ~(a - 1);
  assert(off <= off1 && off1 <= CDR_SIZE_MAX);
  if (size < off1 + c) return normalize_error_offset();
  return off1;
}

/**
 * @brief 检查多个基本类型的对齐
 *
 * @param off 当前偏移量
 * @param size 缓冲区大小
 * @param a_lg2 对齐值的对数
 * @param c_lg2 基本类型大小的对数
 * @param n 基本类型的数量
 * @return 返回新的偏移量或错误偏移量
 */
static uint32_t check_align_prim_many(
    uint32_t off, uint32_t size, uint32_t a_lg2, uint32_t c_lg2, uint32_t n) {
  assert(a_lg2 <= 3);
  const uint32_t a = 1u << a_lg2;
  assert(c_lg2 <= 3);
  assert(size <= CDR_SIZE_MAX);
  assert(off <= size);
  const uint32_t off1 = (off + a - 1) & ~(a - 1);
  assert(off <= off1 && off1 <= CDR_SIZE_MAX);
  if (size < off1 || ((size - off1) >> c_lg2) < n) return normalize_error_offset();
  return off1;
}

/**
 * @brief 规范化 uint8 类型
 *
 * @param off 当前偏移量指针
 * @param size 缓冲区大小
 * @return 如果成功，则返回 true，否则返回 false
 */
static bool normalize_uint8(uint32_t *off,
                            uint32_t size) ddsrt_attribute_warn_unused_result ddsrt_nonnull_all;
static bool normalize_uint8(uint32_t *off, uint32_t size) {
  if (*off == size) return normalize_error_bool();
  (*off)++;
  return true;
}

/**
 * @brief 规范化 uint16 类型
 *
 * @param data 数据缓冲区
 * @param off 当前偏移量指针
 * @param size 缓冲区大小
 * @param bswap 是否需要字节交换
 * @return 如果成功，则返回 true，否则返回 false
 */
static bool normalize_uint16(char *__restrict data,
                             uint32_t *__restrict off,
                             uint32_t size,
                             bool bswap) ddsrt_attribute_warn_unused_result ddsrt_nonnull_all;
static bool normalize_uint16(char *__restrict data,
                             uint32_t *__restrict off,
                             uint32_t size,
                             bool bswap) {
  if ((*off = check_align_prim(*off, size, 1, 1)) == UINT32_MAX) return false;
  if (bswap) *((uint16_t *)(data + *off)) = ddsrt_bswap2u(*((uint16_t *)(data + *off)));
  (*off) += 2;
  return true;
}

/**
 * @brief 对 uint32_t 类型数据进行规范化处理
 * @param[in,out] data 数据指针
 * @param[in,out] off 偏移量指针
 * @param[in] size 数据大小
 * @param[in] bswap 是否需要字节交换
 * @return 规范化处理是否成功
 */
static bool normalize_uint32(char *__restrict data,
                             uint32_t *__restrict off,
                             uint32_t size,
                             bool bswap) {
  // 检查对齐并更新偏移量，如果返回 UINT32_MAX，则表示失败
  if ((*off = check_align_prim(*off, size, 2, 2)) == UINT32_MAX) return false;
  // 如果需要字节交换，则执行字节交换操作
  if (bswap) *((uint32_t *)(data + *off)) = ddsrt_bswap4u(*((uint32_t *)(data + *off)));
  // 更新偏移量
  (*off) += 4;
  return true;
}

/**
 * @brief 对 uint64_t 类型数据进行规范化处理
 * @param[in,out] data 数据指针
 * @param[in,out] off 偏移量指针
 * @param[in] size 数据大小
 * @param[in] bswap 是否需要字节交换
 * @param[in] xcdr_version CDR 编码版本
 * @return 规范化处理是否成功
 */
static bool normalize_uint64(char *__restrict data,
                             uint32_t *__restrict off,
                             uint32_t size,
                             bool bswap,
                             uint32_t xcdr_version) {
  // 检查对齐并更新偏移量，如果返回 UINT32_MAX，则表示失败
  if ((*off = check_align_prim(*off, size, xcdr_version == DDSI_RTPS_CDR_ENC_VERSION_2 ? 2 : 3,
                               3)) == UINT32_MAX)
    return false;
  // 如果需要字节交换，则执行字节交换操作
  if (bswap) {
    uint32_t x = ddsrt_bswap4u(*(uint32_t *)(data + *off));
    *((uint32_t *)(data + *off)) = ddsrt_bswap4u(*((uint32_t *)(data + *off) + 1));
    *((uint32_t *)(data + *off) + 1) = x;
  }
  // 更新偏移量
  (*off) += 8;
  return true;
}

/**
 * @brief 对 bool 类型数据进行规范化处理
 * @param[in,out] data 数据指针
 * @param[in,out] off 偏移量指针
 * @param[in] size 数据大小
 * @return 规范化处理是否成功
 */
static bool normalize_bool(char *__restrict data, uint32_t *__restrict off, uint32_t size) {
  // 如果偏移量等于数据大小，则返回错误
  if (*off == size) return normalize_error_bool();
  // 获取布尔值
  uint8_t b = *((uint8_t *)(data + *off));
  // 如果布尔值大于 1，则返回错误
  if (b > 1) return normalize_error_bool();
  // 更新偏移量
  (*off)++;
  return true;
}

/**
 * @brief 读取并对 bool 类型数据进行规范化处理
 * @param[out] val 读取到的布尔值
 * @param[in,out] data 数据指针
 * @param[in,out] off 偏移量指针
 * @param[in] size 数据大小
 * @return 规范化处理是否成功
 */
static bool read_and_normalize_bool(bool *__restrict val,
                                    char *__restrict data,
                                    uint32_t *__restrict off,
                                    uint32_t size) {
  // 如果偏移量等于数据大小，则返回错误
  if (*off == size) return normalize_error_bool();
  // 获取布尔值
  uint8_t b = *((uint8_t *)(data + *off));
  // 如果布尔值大于 1，则返回错误
  if (b > 1) return normalize_error_bool();
  // 设置读取到的布尔值
  *val = b;
  // 更新偏移量
  (*off)++;
  return true;
}

/**
 * @brief 读取并规范化8位无符号整数
 *
 * @param[out] val  存储读取的值的指针
 * @param[in]  data 数据缓冲区的指针
 * @param[in,out] off 偏移量的指针，用于在数据缓冲区中定位
 * @param[in]  size 缓冲区大小
 * @return 如果成功读取和规范化，则返回true，否则返回false
 */
static inline bool read_and_normalize_uint8(uint8_t *__restrict val,
                                            char *__restrict data,
                                            uint32_t *__restrict off,
                                            uint32_t size) {
  // 检查对齐并更新偏移量，如果失败则返回UINT32_MAX
  if ((*off = check_align_prim(*off, size, 0, 0)) == UINT32_MAX) return false;
  // 从数据缓冲区中读取一个8位无符号整数，并将其存储到val指向的变量中
  *val = *((uint8_t *)(data + *off));
  // 增加偏移量
  (*off)++;
  return true;
}

/**
 * @brief 读取并规范化16位无符号整数
 *
 * @param[out] val   存储读取的值的指针
 * @param[in]  data  数据缓冲区的指针
 * @param[in,out] off  偏移量的指针，用于在数据缓冲区中定位
 * @param[in]  size  缓冲区大小
 * @param[in]  bswap 是否需要字节交换
 * @return 如果成功读取和规范化，则返回true，否则返回false
 */
static inline bool read_and_normalize_uint16(uint16_t *__restrict val,
                                             char *__restrict data,
                                             uint32_t *__restrict off,
                                             uint32_t size,
                                             bool bswap) {
  // 检查对齐并更新偏移量，如果失败则返回UINT32_MAX
  if ((*off = check_align_prim(*off, size, 1, 1)) == UINT32_MAX) return false;
  // 如果需要字节交换，则执行字节交换操作
  if (bswap) *((uint16_t *)(data + *off)) = ddsrt_bswap2u(*((uint16_t *)(data + *off)));
  // 从数据缓冲区中读取一个16位无符号整数，并将其存储到val指向的变量中
  *val = *((uint16_t *)(data + *off));
  // 增加偏移量
  (*off) += 2;
  return true;
}

/**
 * @brief 读取并规范化32位无符号整数
 *
 * @param[out] val   存储读取的值的指针
 * @param[in]  data  数据缓冲区的指针
 * @param[in,out] off  偏移量的指针，用于在数据缓冲区中定位
 * @param[in]  size  缓冲区大小
 * @param[in]  bswap 是否需要字节交换
 * @return 如果成功读取和规范化，则返回true，否则返回false
 */
static inline bool read_and_normalize_uint32(uint32_t *__restrict val,
                                             char *__restrict data,
                                             uint32_t *__restrict off,
                                             uint32_t size,
                                             bool bswap) {
  // 检查对齐并更新偏移量，如果失败则返回UINT32_MAX
  if ((*off = check_align_prim(*off, size, 2, 2)) == UINT32_MAX) return false;
  // 如果需要字节交换，则执行字节交换操作
  if (bswap) *((uint32_t *)(data + *off)) = ddsrt_bswap4u(*((uint32_t *)(data + *off)));
  // 从数据缓冲区中读取一个32位无符号整数，并将其存储到val指向的变量中
  *val = *((uint32_t *)(data + *off));
  // 增加偏移量
  (*off) += 4;
  return true;
}

/**
 * @brief 读取并规范化64位无符号整数
 *
 * @param[out] val   存储读取的值的指针
 * @param[in]  data  数据缓冲区的指针
 * @param[in,out] off  偏移量的指针，用于在数据缓冲区中定位
 * @param[in]  size  缓冲区大小
 * @param[in]  bswap 是否需要字节交换
 * @param[in]  xcdr_version CDR编码版本
 * @return 如果成功读取和规范化，则返回true，否则返回false
 */
static inline bool read_and_normalize_uint64(uint64_t *__restrict val,
                                             char *__restrict data,
                                             uint32_t *__restrict off,
                                             uint32_t size,
                                             bool bswap,
                                             uint32_t xcdr_version) {
  // 检查对齐并更新偏移量，如果失败则返回UINT32_MAX
  if ((*off = check_align_prim(*off, size, xcdr_version == DDSI_RTPS_CDR_ENC_VERSION_2 ? 2 : 3,
                               3)) == UINT32_MAX)
    return false;
  // 如果需要字节交换，则执行字节交换操作
  if (bswap) {
    uint32_t x = ddsrt_bswap4u(*(uint32_t *)(data + *off));
    *((uint32_t *)(data + *off)) = ddsrt_bswap4u(*((uint32_t *)(data + *off) + 1));
    *((uint32_t *)(data + *off) + 1) = x;
  }
  // 从数据缓冲区中读取一个64位无符号整数，并将其存储到val指向的变量中
  *val = *((uint64_t *)(data + *off));
  // 增加偏移量
  (*off) += 8;
  return true;
}

/**
 * @brief 检查并规范化 uint32_t 类型的值
 *
 * @param[out] val  规范化后的 uint32_t 值的指针
 * @param[in] data  数据缓冲区的指针
 * @param[in,out] off  当前偏移量的指针
 * @param[in] size  数据缓冲区的大小
 * @param[in] bswap  是否需要字节交换
 * @return bool  如果成功返回 true，否则返回 false
 */
static bool peek_and_normalize_uint32(uint32_t *__restrict val,
                                      char *__restrict data,
                                      uint32_t *__restrict off,
                                      uint32_t size,
                                      bool bswap) {
  // 检查对齐并更新偏移量
  if ((*off = check_align_prim(*off, size, 2, 2)) == UINT32_MAX) return false;
  // 根据是否需要字节交换进行规范化
  if (bswap)
    *val = ddsrt_bswap4u(*((uint32_t *)(data + *off)));
  else
    *val = *((uint32_t *)(data + *off));
  return true;
}

/**
 * @brief 读取并规范化枚举类型的值
 *
 * @param[out] val  规范化后的枚举值的指针
 * @param[in] data  数据缓冲区的指针
 * @param[in,out] off  当前偏移量的指针
 * @param[in] size  数据缓冲区的大小
 * @param[in] bswap  是否需要字节交换
 * @param[in] insn  指令值
 * @param[in] max  枚举类型的最大值
 * @return bool  如果成功返回 true，否则返回 false
 */
static bool read_normalize_enum(uint32_t *__restrict val,
                                char *__restrict data,
                                uint32_t *__restrict off,
                                uint32_t size,
                                bool bswap,
                                uint32_t insn,
                                uint32_t max) {
  // 根据指令值的大小选择相应的处理方式
  switch (DDS_OP_TYPE_SZ(insn)) {
    case 1: {
      uint8_t val8;
      if (!read_and_normalize_uint8(&val8, data, off, size)) return false;
      *val = val8;
      break;
    }
    case 2: {
      uint16_t val16;
      if (!read_and_normalize_uint16(&val16, data, off, size, bswap)) return false;
      *val = val16;
      break;
    }
    case 4:
      if (!read_and_normalize_uint32(val, data, off, size, bswap)) return false;
      break;
    default:
      return normalize_error_bool();
  }
  // 检查规范化后的值是否超过枚举类型的最大值
  if (*val > max) return normalize_error_bool();
  return true;
}

/**
 * @brief 规范化枚举类型的值
 *
 * @param[in] data  数据缓冲区的指针
 * @param[in,out] off  当前偏移量的指针
 * @param[in] size  数据缓冲区的大小
 * @param[in] bswap  是否需要字节交换
 * @param[in] insn  指令值
 * @param[in] max  枚举类型的最大值
 * @return bool  如果成功返回 true，否则返回 false
 */
static bool normalize_enum(char *__restrict data,
                           uint32_t *__restrict off,
                           uint32_t size,
                           bool bswap,
                           uint32_t insn,
                           uint32_t max) {
  uint32_t val;
  return read_normalize_enum(&val, data, off, size, bswap, insn, max);
}

/**
 * @brief 读取并规范化位掩码值
 *
 * @param[out] val         存储规范化后的位掩码值
 * @param[in]  data        输入数据缓冲区
 * @param[in,out] off      当前偏移量，函数执行后更新
 * @param[in]  size        数据缓冲区大小
 * @param[in]  bswap       是否需要字节交换
 * @param[in]  xcdr_version XCDR版本
 * @param[in]  insn        指令
 * @param[in]  bits_h      高位掩码
 * @param[in]  bits_l      低位掩码
 * @return bool            成功返回true，失败返回false
 */
static bool read_normalize_bitmask(uint64_t *__restrict val,
                                   char *__restrict data,
                                   uint32_t *__restrict off,
                                   uint32_t size,
                                   bool bswap,
                                   uint32_t xcdr_version,
                                   uint32_t insn,
                                   uint32_t bits_h,
                                   uint32_t bits_l) {
  // 根据指令中的类型大小进行处理
  switch (DDS_OP_TYPE_SZ(insn)) {
    case 1: {
      uint8_t val8;
      // 读取并规范化8位无符号整数
      if (!read_and_normalize_uint8(&val8, data, off, size)) return false;
      *val = val8;
      break;
    }
    case 2: {
      uint16_t val16;
      // 读取并规范化16位无符号整数
      if (!read_and_normalize_uint16(&val16, data, off, size, bswap)) return false;
      *val = val16;
      break;
    }
    case 4: {
      uint32_t val32;
      // 读取并规范化32位无符号整数
      if (!read_and_normalize_uint32(&val32, data, off, size, bswap)) return false;
      *val = val32;
      break;
    }
    case 8:
      // 读取并规范化64位无符号整数
      if (!read_and_normalize_uint64(val, data, off, size, bswap, xcdr_version)) return false;
      break;
    default:
      abort();
  }
  // 检查位掩码值是否有效
  if (!bitmask_value_valid(*val, bits_h, bits_l)) return normalize_error_bool();
  return true;
}

/**
 * @brief 规范化位掩码
 *
 * @param[in]  data        输入数据缓冲区
 * @param[in,out] off      当前偏移量，函数执行后更新
 * @param[in]  size        数据缓冲区大小
 * @param[in]  bswap       是否需要字节交换
 * @param[in]  xcdr_version XCDR版本
 * @param[in]  insn        指令
 * @param[in]  bits_h      高位掩码
 * @param[in]  bits_l      低位掩码
 * @return bool            成功返回true，失败返回false
 */
static bool normalize_bitmask(char *__restrict data,
                              uint32_t *__restrict off,
                              uint32_t size,
                              bool bswap,
                              uint32_t xcdr_version,
                              uint32_t insn,
                              uint32_t bits_h,
                              uint32_t bits_l) {
  uint64_t val;
  return read_normalize_bitmask(&val, data, off, size, bswap, xcdr_version, insn, bits_h, bits_l);
}

/**
 * @brief 规范化字符串
 *
 * @param[in]  data        输入数据缓冲区
 * @param[in,out] off      当前偏移量，函数执行后更新
 * @param[in]  size        数据缓冲区大小
 * @param[in]  bswap       是否需要字节交换
 * @param[in]  maxsz       最大字符串长度
 * @return bool            成功返回true，失败返回false
 */
static bool normalize_string(
    char *__restrict data, uint32_t *__restrict off, uint32_t size, bool bswap, size_t maxsz) {
  uint32_t sz;
  // 读取并规范化32位无符号整数
  if (!read_and_normalize_uint32(&sz, data, off, size, bswap)) return false;
  // 检查字符串长度是否有效
  if (sz == 0 || size - *off < sz || maxsz < sz) return normalize_error_bool();
  // 检查字符串是否以空字符结尾
  if (data[*off + sz - 1] != 0) return normalize_error_bool();
  *off += sz;
  return true;
}

/**
 * @brief 规范化原始数组
 *
 * @param[in] data          输入数据指针
 * @param[in,out] off       偏移量指针，用于更新偏移量
 * @param[in] size          数据大小
 * @param[in] bswap         是否需要字节交换
 * @param[in] num           数组元素数量
 * @param[in] type          数据流类型码
 * @param[in] xcdr_version  CDR版本
 * @return bool             如果规范化成功，则返回true，否则返回false
 */
static bool normalize_primarray(char *__restrict data,
                                uint32_t *__restrict off,
                                uint32_t size,
                                bool bswap,
                                uint32_t num,
                                enum dds_stream_typecode type,
                                uint32_t xcdr_version)
    ddsrt_attribute_warn_unused_result ddsrt_nonnull_all;
static bool normalize_primarray(char *__restrict data,
                                uint32_t *__restrict off,
                                uint32_t size,
                                bool bswap,
                                uint32_t num,
                                enum dds_stream_typecode type,
                                uint32_t xcdr_version) {
  // 根据数据流类型码进行处理
  switch (type) {
    case DDS_OP_VAL_1BY:
      // 检查对齐并更新偏移量
      if ((*off = check_align_prim_many(*off, size, 0, 0, num)) == UINT32_MAX) return false;
      // 更新偏移量
      *off += num;
      return true;
    case DDS_OP_VAL_2BY:
      // 检查对齐并更新偏移量
      if ((*off = check_align_prim_many(*off, size, 1, 1, num)) == UINT32_MAX) return false;
      // 如果需要字节交换
      if (bswap) {
        uint16_t *xs = (uint16_t *)(data + *off);
        for (uint32_t i = 0; i < num; i++) xs[i] = ddsrt_bswap2u(xs[i]);
      }
      // 更新偏移量
      *off += 2 * num;
      return true;
    case DDS_OP_VAL_4BY:
      // 检查对齐并更新偏移量
      if ((*off = check_align_prim_many(*off, size, 2, 2, num)) == UINT32_MAX) return false;
      // 如果需要字节交换
      if (bswap) {
        uint32_t *xs = (uint32_t *)(data + *off);
        for (uint32_t i = 0; i < num; i++) xs[i] = ddsrt_bswap4u(xs[i]);
      }
      // 更新偏移量
      *off += 4 * num;
      return true;
    case DDS_OP_VAL_8BY:
      // 检查对齐并更新偏移量
      if ((*off = check_align_prim_many(
               *off, size, xcdr_version == DDSI_RTPS_CDR_ENC_VERSION_2 ? 2 : 3, 3, num)) ==
          UINT32_MAX)
        return false;
      // 如果需要字节交换
      if (bswap) {
        uint64_t *xs = (uint64_t *)(data + *off);
        for (uint32_t i = 0; i < num; i++) {
          uint32_t x = ddsrt_bswap4u(*(uint32_t *)&xs[i]);
          *(uint32_t *)&xs[i] = ddsrt_bswap4u(*(((uint32_t *)&xs[i]) + 1));
          *(((uint32_t *)&xs[i]) + 1) = x;
        }
      }
      // 更新偏移量
      *off += 8 * num;
      return true;
    default:
      // 未知类型，中止程序
      abort();
      break;
  }
  return false;
}

/**
 * @brief 对枚举数组进行规范化处理
 *
 * @param[in,out] data      数据指针，用于存储规范化后的枚举数组
 * @param[in,out] off       偏移量指针，用于记录当前处理位置
 * @param[in]     size      数据大小
 * @param[in]     bswap     是否需要字节交换
 * @param[in]     enum_sz   枚举元素的大小（以字节为单位）
 * @param[in]     num       枚举元素的数量
 * @param[in]     max       枚举元素的最大值
 * @return bool             规范化成功返回 true，否则返回 false
 */
static bool normalize_enumarray(char *__restrict data,
                                uint32_t *__restrict off,
                                uint32_t size,
                                bool bswap,
                                uint32_t enum_sz,
                                uint32_t num,
                                uint32_t max) {
  // 根据枚举元素大小选择相应的处理方式
  switch (enum_sz) {
    case 1: {
      // 检查对齐并更新偏移量
      if ((*off = check_align_prim_many(*off, size, 0, 0, num)) == UINT32_MAX) return false;
      // 获取数据指针
      uint8_t *const xs = (uint8_t *)(data + *off);
      // 遍历枚举元素，检查是否超过最大值
      for (uint32_t i = 0; i < num; i++)
        if (xs[i] > max) return normalize_error_bool();
      // 更新偏移量
      *off += num;
      break;
    }
    case 2: {
      // 检查对齐并更新偏移量
      if ((*off = check_align_prim_many(*off, size, 1, 1, num)) == UINT32_MAX) return false;
      // 获取数据指针
      uint16_t *const xs = (uint16_t *)(data + *off);
      // 遍历枚举元素，检查是否超过最大值，如需字节交换则进行处理
      for (uint32_t i = 0; i < num; i++)
        if ((uint16_t)(bswap ? (xs[i] = ddsrt_bswap2u(xs[i])) : xs[i]) > max)
          return normalize_error_bool();
      // 更新偏移量
      *off += 2 * num;
      break;
    }
    case 4: {
      // 检查对齐并更新偏移量
      if ((*off = check_align_prim_many(*off, size, 2, 2, num)) == UINT32_MAX) return false;
      // 获取数据指针
      uint32_t *const xs = (uint32_t *)(data + *off);
      // 遍历枚举元素，检查是否超过最大值，如需字节交换则进行处理
      for (uint32_t i = 0; i < num; i++)
        if ((uint32_t)(bswap ? (xs[i] = ddsrt_bswap4u(xs[i])) : xs[i]) > max)
          return normalize_error_bool();
      // 更新偏移量
      *off += 4 * num;
      break;
    }
    default:
      // 不支持的枚举元素大小，返回错误
      return normalize_error_bool();
  }
  // 规范化成功
  return true;
}

/**
 * @brief 规范化位掩码数组
 *
 * @param[in] data          输入数据指针
 * @param[in,out] off       数据偏移量指针
 * @param[in] size          数据大小
 * @param[in] bswap         是否需要字节交换
 * @param[in] xcdr_version  CDR编码版本
 * @param[in] insn          指令
 * @param[in] num           数组元素数量
 * @param[in] bits_h        高位掩码
 * @param[in] bits_l        低位掩码
 * @return bool             如果规范化成功，则返回true，否则返回false
 */
static bool normalize_bitmaskarray(char *__restrict data,
                                   uint32_t *__restrict off,
                                   uint32_t size,
                                   bool bswap,
                                   uint32_t xcdr_version,
                                   uint32_t insn,
                                   uint32_t num,
                                   uint32_t bits_h,
                                   uint32_t bits_l) {
  // 根据指令获取操作数类型的大小
  switch (DDS_OP_TYPE_SZ(insn)) {
    case 1: {
      // 检查对齐并更新偏移量
      if ((*off = check_align_prim_many(*off, size, 0, 0, num)) == UINT32_MAX) return false;
      // 获取数据指针
      uint8_t *const xs = (uint8_t *)(data + *off);
      // 遍历数组元素，检查位掩码值是否有效
      for (uint32_t i = 0; i < num; i++)
        if (!bitmask_value_valid(xs[i], bits_h, bits_l)) return normalize_error_bool();
      // 更新偏移量
      *off += num;
      break;
    }
    case 2: {
      // 检查对齐并更新偏移量
      if ((*off = check_align_prim_many(*off, size, 1, 1, num)) == UINT32_MAX) return false;
      // 获取数据指针
      uint16_t *const xs = (uint16_t *)(data + *off);
      // 遍历数组元素，检查位掩码值是否有效，如果需要字节交换，则进行字节交换
      for (uint32_t i = 0; i < num; i++)
        if (!bitmask_value_valid(bswap ? (xs[i] = ddsrt_bswap2u(xs[i])) : xs[i], bits_h, bits_l))
          return normalize_error_bool();
      // 更新偏移量
      *off += 2 * num;
      break;
    }
    case 4: {
      // 检查对齐并更新偏移量
      if ((*off = check_align_prim_many(*off, size, 2, 2, num)) == UINT32_MAX) return false;
      // 获取数据指针
      uint32_t *const xs = (uint32_t *)(data + *off);
      // 遍历数组元素，检查位掩码值是否有效，如果需要字节交换，则进行字节交换
      for (uint32_t i = 0; i < num; i++)
        if (!bitmask_value_valid(bswap ? (xs[i] = ddsrt_bswap4u(xs[i])) : xs[i], bits_h, bits_l))
          return normalize_error_bool();
      // 更新偏移量
      *off += 4 * num;
      break;
    }
    case 8: {
      // 检查对齐并更新偏移量，根据CDR编码版本选择对齐方式
      if ((*off = check_align_prim_many(
               *off, size, xcdr_version == DDSI_RTPS_CDR_ENC_VERSION_2 ? 2 : 3, 3, num)) ==
          UINT32_MAX)
        return false;
      // 获取数据指针
      uint64_t *const xs = (uint64_t *)(data + *off);
      // 遍历数组元素，检查位掩码值是否有效，如果需要字节交换，则进行字节交换
      for (uint32_t i = 0; i < num; i++) {
        if (bswap) {
          uint32_t x = ddsrt_bswap4u(*(uint32_t *)&xs[i]);
          *(uint32_t *)&xs[i] = ddsrt_bswap4u(*(((uint32_t *)&xs[i]) + 1));
          *(((uint32_t *)&xs[i]) + 1) = x;
        }
        if (!bitmask_value_valid(xs[i], bits_h, bits_l)) return normalize_error_bool();
      }
      // 更新偏移量
      *off += 8 * num;
      break;
    }
  }
  return true;
}

/**
 * @brief 读取并规范化集合数据头
 *
 * @param[out] has_dheader      指向布尔值的指针，表示是否有数据头
 * @param[out] size1            指向 uint32_t 的指针，用于存储规范化后的大小
 * @param[in]  data             字符指针，指向要处理的数据
 * @param[in,out] off           指向 uint32_t 的指针，表示当前偏移量
 * @param[in]  size             数据的总大小
 * @param[in]  bswap            布尔值，表示是否需要字节交换
 * @param[in]  subtype          dds_stream_typecode 枚举类型，表示数据流子类型
 * @param[in]  xcdr_version     XCDR 版本号
 * @return bool                 如果操作成功，则返回 true，否则返回 false
 */
static bool read_and_normalize_collection_dheader(bool *__restrict has_dheader,
                                                  uint32_t *__restrict size1,
                                                  char *__restrict data,
                                                  uint32_t *__restrict off,
                                                  uint32_t size,
                                                  bool bswap,
                                                  const enum dds_stream_typecode subtype,
                                                  uint32_t xcdr_version)
    ddsrt_attribute_warn_unused_result ddsrt_nonnull_all;
static bool read_and_normalize_collection_dheader(bool *__restrict has_dheader,
                                                  uint32_t *__restrict size1,
                                                  char *__restrict data,
                                                  uint32_t *__restrict off,
                                                  uint32_t size,
                                                  bool bswap,
                                                  const enum dds_stream_typecode subtype,
                                                  uint32_t xcdr_version) {
  // 判断是否需要数据头
  if (is_dheader_needed(subtype, xcdr_version)) {
    // 读取并规范化 uint32_t 类型的大小
    if (!read_and_normalize_uint32(size1, data, off, size, bswap)) return false;
    // 检查规范化后的大小是否超出数据总大小
    if (*size1 > size - *off) return normalize_error_bool();
    // 设置 has_dheader 为 true
    *has_dheader = true;
    // 更新 size1 的值
    *size1 += *off;
    return true;
  } else {
    // 设置 has_dheader 为 false
    *has_dheader = false;
    // 设置 size1 等于数据总大小
    *size1 = size;
    return true;
  }
}

/**
 * @brief 对序列进行规范化处理
 *
 * @param[in] data          输入数据指针
 * @param[in,out] off       数据偏移量指针，用于读取和更新当前位置
 * @param[in] size          数据大小
 * @param[in] bswap         是否需要字节交换
 * @param[in] xcdr_version  XCDR版本
 * @param[in] ops           操作指针
 * @param[in] insn          指令
 * @return const uint32_t*  规范化后的操作指针，如果失败则返回NULL
 */
static const uint32_t *normalize_seq(char *__restrict data,
                                     uint32_t *__restrict off,
                                     uint32_t size,
                                     bool bswap,
                                     uint32_t xcdr_version,
                                     const uint32_t *__restrict ops,
                                     uint32_t insn) {
  // 获取子类型
  const enum dds_stream_typecode subtype = DDS_OP_SUBTYPE(insn);
  // 判断序列是否有边界
  uint32_t bound_op = seq_is_bounded(DDS_OP_TYPE(insn)) ? 1 : 0;
  // 获取边界值
  uint32_t bound = bound_op ? ops[2] : 0;
  bool has_dheader;
  uint32_t size1;
  // 读取并规范化集合头部
  if (!read_and_normalize_collection_dheader(&has_dheader, &size1, data, off, size, bswap, subtype,
                                             xcdr_version))
    return NULL;
  uint32_t num;
  // 读取并规范化uint32_t值
  if (!read_and_normalize_uint32(&num, data, off, size1, bswap)) return NULL;
  // 如果序列中元素个数为0
  if (num == 0) {
    if (has_dheader && *off != size1) return normalize_error_ops();
    return skip_sequence_insns(insn, ops);
  }
  // 检查边界是否超出限制
  if (bound && num > bound) return normalize_error_ops();
  // 根据子类型进行处理
  switch (subtype) {
    case DDS_OP_VAL_BLN:
      if (!normalize_enumarray(data, off, size1, bswap, 1, num, 1)) return NULL;
      ops += 2 + bound_op;
      break;
    case DDS_OP_VAL_1BY:
    case DDS_OP_VAL_2BY:
    case DDS_OP_VAL_4BY:
    case DDS_OP_VAL_8BY:
      if (!normalize_primarray(data, off, size1, bswap, num, subtype, xcdr_version)) return NULL;
      ops += 2 + bound_op;
      break;
    case DDS_OP_VAL_ENU:
      if (!normalize_enumarray(data, off, size1, bswap, DDS_OP_TYPE_SZ(insn), num,
                               ops[2 + bound_op]))
        return NULL;
      ops += 3 + bound_op;
      break;
    case DDS_OP_VAL_BMK:
      if (!normalize_bitmaskarray(data, off, size1, bswap, xcdr_version, insn, num,
                                  ops[2 + bound_op], ops[3 + bound_op]))
        return NULL;
      ops += 4 + bound_op;
      break;
    case DDS_OP_VAL_STR:
    case DDS_OP_VAL_BST: {
      const size_t maxsz = (subtype == DDS_OP_VAL_STR) ? SIZE_MAX : ops[2 + bound_op];
      for (uint32_t i = 0; i < num; i++)
        if (!normalize_string(data, off, size1, bswap, maxsz)) return NULL;
      ops += (subtype == DDS_OP_VAL_STR ? 2 : 3) + bound_op;
      break;
    }
    case DDS_OP_VAL_SEQ:
    case DDS_OP_VAL_BSQ:
    case DDS_OP_VAL_ARR:
    case DDS_OP_VAL_UNI:
    case DDS_OP_VAL_STU: {
      const uint32_t jmp = DDS_OP_ADR_JMP(ops[3 + bound_op]);
      uint32_t const *const jsr_ops = ops + DDS_OP_ADR_JSR(ops[3 + bound_op]);
      for (uint32_t i = 0; i < num; i++)
        if (stream_normalize_data_impl(data, off, size1, bswap, xcdr_version, jsr_ops, false) ==
            NULL)
          return NULL;
      ops += jmp ? jmp : (4 + bound_op); /* FIXME: why would jmp be 0? */
      break;
    }
    case DDS_OP_VAL_EXT:
      ops = NULL;
      abort(); /* not supported */
      break;
  }
  // 检查头部和偏移量是否匹配
  if (has_dheader && *off != size1) return normalize_error_ops();
  return ops;
}

/**
 * @brief 对数组进行规范化处理
 *
 * @param[in] data          输入数据
 * @param[in,out] off       数据偏移量
 * @param[in] size          数据大小
 * @param[in] bswap         是否需要字节交换
 * @param[in] xcdr_version  XCDR版本
 * @param[in] ops           操作指针
 * @param[in] insn          指令
 * @return const uint32_t*  规范化后的操作指针，如果失败则返回NULL
 */
static const uint32_t *normalize_arr(char *__restrict data,
                                     uint32_t *__restrict off,
                                     uint32_t size,
                                     bool bswap,
                                     uint32_t xcdr_version,
                                     const uint32_t *__restrict ops,
                                     uint32_t insn)
    ddsrt_attribute_warn_unused_result ddsrt_nonnull_all;
static const uint32_t *normalize_arr(char *__restrict data,
                                     uint32_t *__restrict off,
                                     uint32_t size,
                                     bool bswap,
                                     uint32_t xcdr_version,
                                     const uint32_t *__restrict ops,
                                     uint32_t insn) {
  // 获取子类型
  const enum dds_stream_typecode subtype = DDS_OP_SUBTYPE(insn);
  bool has_dheader;
  uint32_t size1;

  // 读取并规范化集合头部
  if (!read_and_normalize_collection_dheader(&has_dheader, &size1, data, off, size, bswap, subtype,
                                             xcdr_version))
    return NULL;

  // 获取操作数
  const uint32_t num = ops[2];

  // 根据子类型进行处理
  switch (subtype) {
    case DDS_OP_VAL_BLN:
      if (!normalize_enumarray(data, off, size1, bswap, 1, num, 1)) return NULL;
      ops += 3;
      break;
    case DDS_OP_VAL_1BY:
    case DDS_OP_VAL_2BY:
    case DDS_OP_VAL_4BY:
    case DDS_OP_VAL_8BY:
      if (!normalize_primarray(data, off, size1, bswap, num, subtype, xcdr_version)) return NULL;
      ops += 3;
      break;
    case DDS_OP_VAL_ENU:
      if (!normalize_enumarray(data, off, size1, bswap, DDS_OP_TYPE_SZ(insn), num, ops[3]))
        return NULL;
      ops += 4;
      break;
    case DDS_OP_VAL_BMK:
      if (!normalize_bitmaskarray(data, off, size1, bswap, xcdr_version, insn, num, ops[3], ops[4]))
        return NULL;
      ops += 5;
      break;
    case DDS_OP_VAL_STR:
    case DDS_OP_VAL_BST: {
      const size_t maxsz = (subtype == DDS_OP_VAL_STR) ? SIZE_MAX : ops[4];
      for (uint32_t i = 0; i < num; i++)
        if (!normalize_string(data, off, size1, bswap, maxsz)) return NULL;
      ops += (subtype == DDS_OP_VAL_STR) ? 3 : 5;
      break;
    }
    case DDS_OP_VAL_SEQ:
    case DDS_OP_VAL_BSQ:
    case DDS_OP_VAL_ARR:
    case DDS_OP_VAL_UNI:
    case DDS_OP_VAL_STU: {
      const uint32_t jmp = DDS_OP_ADR_JMP(ops[3]);
      uint32_t const *const jsr_ops = ops + DDS_OP_ADR_JSR(ops[3]);
      for (uint32_t i = 0; i < num; i++)
        if (stream_normalize_data_impl(data, off, size1, bswap, xcdr_version, jsr_ops, false) ==
            NULL)
          return NULL;
      ops += jmp ? jmp : 5;
      break;
    }
    case DDS_OP_VAL_EXT:
      ops = NULL;
      abort(); /* not supported */
      break;
  }

  // 检查头部和偏移量
  if (has_dheader && *off != size1) return normalize_error_ops();

  return ops;
}

/**
 * @brief 规范化统一的离散值
 *
 * @param[out] val      用于存储规范化后的值的指针
 * @param[in]  data     包含原始数据的字符数组
 * @param[in,out] off   当前处理的偏移量，函数执行后会更新此值
 * @param[in]  size     数据大小
 * @param[in]  bswap    是否需要字节交换
 * @param[in]  insn     指令
 * @param[in]  ops      操作数数组
 * @return bool         如果成功规范化，则返回 true，否则返回 false
 */
static bool normalize_uni_disc(uint32_t *__restrict val,
                               char *__restrict data,
                               uint32_t *__restrict off,
                               uint32_t size,
                               bool bswap,
                               uint32_t insn,
                               const uint32_t *__restrict ops) {
  // 根据指令的子类型进行相应的处理
  switch (DDS_OP_SUBTYPE(insn)) {
    case DDS_OP_VAL_BLN: {
      bool bval;
      // 读取并规范化布尔值
      if (!read_and_normalize_bool(&bval, data, off, size)) return false;
      *val = bval;
      return true;
    }
    case DDS_OP_VAL_1BY:
      // 检查并对齐原始数据
      if ((*off = check_align_prim(*off, size, 0, 0)) == UINT32_MAX) return false;
      // 读取一个字节的值
      *val = *((uint8_t *)(data + *off));
      (*off) += 1;
      return true;
    case DDS_OP_VAL_2BY:
      // 检查并对齐原始数据
      if ((*off = check_align_prim(*off, size, 1, 1)) == UINT32_MAX) return false;
      // 如果需要字节交换，则进行字节交换操作
      if (bswap) *((uint16_t *)(data + *off)) = ddsrt_bswap2u(*((uint16_t *)(data + *off)));
      // 读取两个字节的值
      *val = *((uint16_t *)(data + *off));
      (*off) += 2;
      return true;
    case DDS_OP_VAL_4BY:
      // 检查并对齐原始数据
      if ((*off = check_align_prim(*off, size, 2, 2)) == UINT32_MAX) return false;
      // 如果需要字节交换，则进行字节交换操作
      if (bswap) *((uint32_t *)(data + *off)) = ddsrt_bswap4u(*((uint32_t *)(data + *off)));
      // 读取四个字节的值
      *val = *((uint32_t *)(data + *off));
      (*off) += 4;
      return true;
    case DDS_OP_VAL_ENU:
      // 读取并规范化枚举值
      return read_normalize_enum(val, data, off, size, bswap, insn, ops[4]);
    default:
      abort();
  }
  return false;
}

/**
 * @brief 规范化联合类型数据
 *
 * @param[in] data          输入数据的指针
 * @param[in,out] off       当前处理的偏移量
 * @param[in] size          数据大小
 * @param[in] bswap         是否需要字节交换
 * @param[in] xcdr_version  XCDR版本
 * @param[in] ops           操作码数组
 * @param[in] insn          指令
 * @return const uint32_t*  返回规范化后的操作码数组，如果失败则返回NULL
 */
static const uint32_t *normalize_uni(char *__restrict data,
                                     uint32_t *__restrict off,
                                     uint32_t size,
                                     bool bswap,
                                     uint32_t xcdr_version,
                                     const uint32_t *__restrict ops,
                                     uint32_t insn)
    ddsrt_attribute_warn_unused_result ddsrt_nonnull_all;
static const uint32_t *normalize_uni(char *__restrict data,
                                     uint32_t *__restrict off,
                                     uint32_t size,
                                     bool bswap,
                                     uint32_t xcdr_version,
                                     const uint32_t *__restrict ops,
                                     uint32_t insn) {
  uint32_t disc;  // 存储discriminator值

  // 规范化discriminator值
  if (!normalize_uni_disc(&disc, data, off, size, bswap, insn, ops)) return NULL;

  // 查找与discriminator匹配的联合体case
  uint32_t const *const jeq_op = find_union_case(ops, disc);
  ops += DDS_OP_ADR_JMP(ops[3]);

  if (jeq_op) {
    // 获取值类型
    const enum dds_stream_typecode valtype = DDS_JEQ_TYPE(jeq_op[0]);

    // 根据值类型进行规范化处理
    switch (valtype) {
      case DDS_OP_VAL_BLN:
        if (!normalize_bool(data, off, size)) return NULL;
        break;
      case DDS_OP_VAL_1BY:
        if (!normalize_uint8(off, size)) return NULL;
        break;
      case DDS_OP_VAL_2BY:
        if (!normalize_uint16(data, off, size, bswap)) return NULL;
        break;
      case DDS_OP_VAL_4BY:
        if (!normalize_uint32(data, off, size, bswap)) return NULL;
        break;
      case DDS_OP_VAL_8BY:
        if (!normalize_uint64(data, off, size, bswap, xcdr_version)) return NULL;
        break;
      case DDS_OP_VAL_STR:
        if (!normalize_string(data, off, size, bswap, SIZE_MAX)) return NULL;
        break;
      case DDS_OP_VAL_ENU:
        if (!normalize_enum(data, off, size, bswap, jeq_op[0], jeq_op[3])) return NULL;
        break;
      case DDS_OP_VAL_BST:
      case DDS_OP_VAL_SEQ:
      case DDS_OP_VAL_BSQ:
      case DDS_OP_VAL_ARR:
      case DDS_OP_VAL_UNI:
      case DDS_OP_VAL_STU:
      case DDS_OP_VAL_BMK:
        if (stream_normalize_data_impl(data, off, size, bswap, xcdr_version,
                                       jeq_op + DDS_OP_ADR_JSR(jeq_op[0]), false) == NULL)
          return NULL;
        break;
      case DDS_OP_VAL_EXT:
        abort(); /* not supported */
        break;
    }
  }

  return ops;
}

/**
 * @brief 对输入数据进行规范化处理，以便在序列化和反序列化过程中正确处理。
 *
 * @param[in] insn 指令值
 * @param[in,out] data 输入数据的指针
 * @param[in,out] off 数据偏移量的指针
 * @param[in] size 数据大小
 * @param[in] bswap 是否需要字节交换
 * @param[in] xcdr_version XCDR版本
 * @param[in] ops 操作列表的指针
 * @param[in] is_mutable_member 是否为可变成员
 * @return 返回操作列表的指针，如果规范化失败，则返回NULL
 */
static const uint32_t *stream_normalize_adr(uint32_t insn,
                                            char *__restrict data,
                                            uint32_t *__restrict off,
                                            uint32_t size,
                                            bool bswap,
                                            uint32_t xcdr_version,
                                            const uint32_t *__restrict ops,
                                            bool is_mutable_member) {
  // 判断是否为可选类型
  if (op_type_optional(insn)) {
    bool present = true;
    // 如果不是可变成员，读取并规范化布尔值
    if (!is_mutable_member) {
      if (!read_and_normalize_bool(&present, data, off, size)) return NULL;
    }
    // 如果不需要处理，跳过地址
    if (!present) return dds_stream_skip_adr(insn, ops);
  }

  // 根据指令类型进行相应的规范化处理
  switch (DDS_OP_TYPE(insn)) {
    case DDS_OP_VAL_BLN:
      if (!normalize_bool(data, off, size)) return NULL;
      ops += 2;
      break;
    case DDS_OP_VAL_1BY:
      if (!normalize_uint8(off, size)) return NULL;
      ops += 2;
      break;
    case DDS_OP_VAL_2BY:
      if (!normalize_uint16(data, off, size, bswap)) return NULL;
      ops += 2;
      break;
    case DDS_OP_VAL_4BY:
      if (!normalize_uint32(data, off, size, bswap)) return NULL;
      ops += 2;
      break;
    case DDS_OP_VAL_8BY:
      if (!normalize_uint64(data, off, size, bswap, xcdr_version)) return NULL;
      ops += 2;
      break;
    case DDS_OP_VAL_STR:
      if (!normalize_string(data, off, size, bswap, SIZE_MAX)) return NULL;
      ops += 2;
      break;
    case DDS_OP_VAL_BST:
      if (!normalize_string(data, off, size, bswap, ops[2])) return NULL;
      ops += 3;
      break;
    case DDS_OP_VAL_SEQ:
    case DDS_OP_VAL_BSQ:
      ops = normalize_seq(data, off, size, bswap, xcdr_version, ops, insn);
      if (!ops) return NULL;
      break;
    case DDS_OP_VAL_ARR:
      ops = normalize_arr(data, off, size, bswap, xcdr_version, ops, insn);
      if (!ops) return NULL;
      break;
    case DDS_OP_VAL_UNI:
      ops = normalize_uni(data, off, size, bswap, xcdr_version, ops, insn);
      if (!ops) return NULL;
      break;
    case DDS_OP_VAL_ENU:
      if (!normalize_enum(data, off, size, bswap, insn, ops[2])) return NULL;
      ops += 3;
      break;
    case DDS_OP_VAL_BMK:
      if (!normalize_bitmask(data, off, size, bswap, xcdr_version, insn, ops[2], ops[3]))
        return NULL;
      ops += 4;
      break;
    case DDS_OP_VAL_EXT: {
      const uint32_t *jsr_ops = ops + DDS_OP_ADR_JSR(ops[2]);
      const uint32_t jmp = DDS_OP_ADR_JMP(ops[2]);

      // 跳过基本类型的DLC指令，基本类型成员不需要DHEADER
      if (op_type_base(insn) && jsr_ops[0] == DDS_OP_DLC) jsr_ops++;

      if (stream_normalize_data_impl(data, off, size, bswap, xcdr_version, jsr_ops, false) == NULL)
        return NULL;
      ops += jmp ? jmp : 3;
      break;
    }
    case DDS_OP_VAL_STU:
      abort();  // op type STU只支持作为子类型
      break;
  }

  return ops;
}

/**
 * @brief 对输入数据流进行规范化处理，以便于解析和操作。
 *
 * @param[in] data          输入数据流
 * @param[in,out] off       当前处理的偏移量
 * @param[in] size          数据流的总大小
 * @param[in] bswap         是否需要字节交换
 * @param[in] xcdr_version  XCDR版本
 * @param[in] ops           操作指令集
 * @return const uint32_t*  返回处理后的操作指令集，如果出错则返回NULL
 */
static const uint32_t *stream_normalize_delimited(char *__restrict data,
                                                  uint32_t *__restrict off,
                                                  uint32_t size,
                                                  bool bswap,
                                                  uint32_t xcdr_version,
                                                  const uint32_t *__restrict ops)
    ddsrt_attribute_warn_unused_result ddsrt_nonnull_all;
static const uint32_t *stream_normalize_delimited(char *__restrict data,
                                                  uint32_t *__restrict off,
                                                  uint32_t size,
                                                  bool bswap,
                                                  uint32_t xcdr_version,
                                                  const uint32_t *__restrict ops) {
  // 读取并规范化uint32类型的数据
  uint32_t delimited_sz;
  if (!read_and_normalize_uint32(&delimited_sz, data, off, size, bswap)) return NULL;

  // 检查声明的大小是否合理
  if (delimited_sz > size - *off) return normalize_error_ops();

  // 计算有效数据区域的结束位置
  uint32_t size1 = *off + delimited_sz;
  assert(size1 <= size);

  // 跳过DLC操作
  ops++;

  // 处理操作指令集
  uint32_t insn;
  while ((insn = *ops) != DDS_OP_RTS && *off < size1) {
    switch (DDS_OP(insn)) {
      case DDS_OP_ADR:
        if ((ops = stream_normalize_adr(insn, data, off, size1, bswap, xcdr_version, ops, false)) ==
            NULL)
          return NULL;
        break;
      case DDS_OP_JSR:
        if (stream_normalize_data_impl(data, off, size1, bswap, xcdr_version,
                                       ops + DDS_OP_JUMP(insn), false) == NULL)
          return NULL;
        ops++;
        break;
      case DDS_OP_RTS:
      case DDS_OP_JEQ:
      case DDS_OP_JEQ4:
      case DDS_OP_KOF:
      case DDS_OP_DLC:
      case DDS_OP_PLC:
      case DDS_OP_PLM:
        abort();
        break;
    }
  }

  // 处理序列化数据中不存在的字段
  if (insn != DDS_OP_RTS) {
#if 0  // FIXME: need to deal with type coercion flags
    if (!type_widening_allowed)
      return NULL;
#endif
    while ((insn = *ops) != DDS_OP_RTS) ops = dds_stream_skip_adr(insn, ops);
  }

  // 检查处理后的偏移量是否正确
  assert(*off <= size1);
  *off = size1;
  return ops;
}
/**
 * @file
 * @brief 用于处理数据流规范化的函数
 */

/**
 * @enum normalize_pl_member_result
 * @brief 规范化结果枚举类型
 */
enum normalize_pl_member_result {
  NPMR_NOT_FOUND,  ///< 没有找到数据
  NPMR_FOUND,      ///< 找到了数据
  NPMR_ERROR       ///< 找到了数据，但规范化失败
};

/**
 * @brief 对数据流中的成员进行规范化处理
 *
 * @param[in,out] data         数据流指针
 * @param[in]     m_id         成员ID
 * @param[in,out] off          偏移量指针
 * @param[in]     size         数据大小
 * @param[in]     bswap        是否需要字节交换
 * @param[in]     xcdr_version XCDR版本
 * @param[in]     ops          操作指针
 * @return 返回规范化结果
 */
static enum normalize_pl_member_result dds_stream_normalize_pl_member(
    char *__restrict data,
    uint32_t m_id,
    uint32_t *__restrict off,
    uint32_t size,
    bool bswap,
    uint32_t xcdr_version,
    const uint32_t *__restrict ops) ddsrt_attribute_warn_unused_result ddsrt_nonnull_all;
static enum normalize_pl_member_result dds_stream_normalize_pl_member(
    char *__restrict data,
    uint32_t m_id,
    uint32_t *__restrict off,
    uint32_t size,
    bool bswap,
    uint32_t xcdr_version,
    const uint32_t *__restrict ops) {
  uint32_t insn, ops_csr = 0;                               // 定义指令和操作计数器
  enum normalize_pl_member_result result = NPMR_NOT_FOUND;  // 初始化规范化结果为未找到
  while (result == NPMR_NOT_FOUND &&
         (insn = ops[ops_csr]) != DDS_OP_RTS)  // 当结果未找到且指令不等于DDS_OP_RTS时，继续循环
  {
    assert(DDS_OP(insn) == DDS_OP_PLM);                              // 断言指令等于DDS_OP_PLM
    uint32_t flags = DDS_PLM_FLAGS(insn);                            // 获取标志位
    const uint32_t *plm_ops = ops + ops_csr + DDS_OP_ADR_PLM(insn);  // 计算PLM操作指针
    if (flags & DDS_OP_FLAG_BASE)                                    // 如果有基本标志位
    {
      assert(DDS_OP(plm_ops[0]) == DDS_OP_PLC);  // 断言第一个PLM操作等于DDS_OP_PLC
      plm_ops++;                                 // 跳过PLC，进入基本类型的第一个PLM
      result = dds_stream_normalize_pl_member(data, m_id, off, size, bswap, xcdr_version,
                                              plm_ops);  // 递归调用规范化函数
    } else if (ops[ops_csr + 1] == m_id)  // 如果操作计数器加1后等于成员ID
    {
      if (stream_normalize_data_impl(data, off, size, bswap, xcdr_version, plm_ops,
                                     true))  // 调用实现规范化数据的函数
        result = NPMR_FOUND;                 // 结果设为找到
      else
        result = NPMR_ERROR;  // 结果设为错误
      break;
    }
    ops_csr += 2;  // 操作计数器加2
  }
  return result;  // 返回规范化结果
}

/**
 * @brief 对输入的数据流进行规范化处理，以便在序列化和反序列化时正确解析。
 *
 * @param[in] data          输入数据流
 * @param[in,out] off       数据流中当前处理位置的偏移量
 * @param[in] size          数据流的大小
 * @param[in] bswap         是否需要字节交换
 * @param[in] xcdr_version  CDR版本
 * @param[in] ops           操作指针
 * @return const uint32_t*  规范化后的操作指针
 */
static const uint32_t *stream_normalize_pl(char *__restrict data,
                                           uint32_t *__restrict off,
                                           uint32_t size,
                                           bool bswap,
                                           uint32_t xcdr_version,
                                           const uint32_t *__restrict ops)
    ddsrt_attribute_warn_unused_result ddsrt_nonnull_all;
static const uint32_t *stream_normalize_pl(char *__restrict data,
                                           uint32_t *__restrict off,
                                           uint32_t size,
                                           bool bswap,
                                           uint32_t xcdr_version,
                                           const uint32_t *__restrict ops) {
  /* 跳过 PLC op */
  ops++;

  /* 规范化 DHEADER */
  uint32_t pl_sz;
  if (!read_and_normalize_uint32(&pl_sz, data, off, size, bswap)) return NULL;
  // 如果输入中剩余的字节数少于 pl_sz，则拒绝
  if (pl_sz > size - *off) return normalize_error_ops();
  const uint32_t size1 = *off + pl_sz;

  while (*off < size1) {
    /* 规范化 EMHEADER */
    uint32_t em_hdr;
    if (!read_and_normalize_uint32(&em_hdr, data, off, size1, bswap)) return NULL;
    uint32_t lc = EMHEADER_LENGTH_CODE(em_hdr), m_id = EMHEADER_MEMBERID(em_hdr), msz;
    bool must_understand = em_hdr & EMHEADER_FLAG_MUSTUNDERSTAND;
    switch (lc) {
      case LENGTH_CODE_1B:
      case LENGTH_CODE_2B:
      case LENGTH_CODE_4B:
      case LENGTH_CODE_8B:
        msz = 1u << lc;
        break;
      case LENGTH_CODE_NEXTINT:
        /* NEXTINT */
        if (!read_and_normalize_uint32(&msz, data, off, size1, bswap)) return NULL;
        break;
      case LENGTH_CODE_ALSO_NEXTINT:
      case LENGTH_CODE_ALSO_NEXTINT4:
      case LENGTH_CODE_ALSO_NEXTINT8:
        /* 长度是序列化数据的一部分 */
        if (!peek_and_normalize_uint32(&msz, data, off, size1, bswap)) return NULL;
        if (lc > LENGTH_CODE_ALSO_NEXTINT) {
          uint32_t shift = lc - 4;
          if (msz > UINT32_MAX >> shift) return normalize_error_ops();
          msz <<= shift;
        }
        /* 成员中嵌入的长度不包括它自己的4个字节，我们需要能够添加这4个字节；
         技术上可能这是有效的CDR，但如果是这样，我们不支持它 */
        if (msz > UINT32_MAX - 4)
          return normalize_error_ops();
        else
          msz += 4;
        break;
      default:
        abort();
        break;
    }
    // 如果参数列表的声明大小中剩余的字节数少于 msz，则拒绝
    if (msz > size1 - *off) return normalize_error_ops();
    // 不允许成员值超过其声明的大小
    const uint32_t size2 = *off + msz;
    switch (dds_stream_normalize_pl_member(data, m_id, off, size2, bswap, xcdr_version, ops)) {
      case NPMR_NOT_FOUND:
        /* FIXME: 调用者应该能够区分因未知成员而丢弃的样本和因数据无效而丢弃的样本。
         这需要在cdrstream接口中进行更改，但也需要在serdata接口中将返回值传递给ddsi_receive。 */
        if (must_understand) return normalize_error_ops();
        *off = size2;
        break;
      case NPMR_FOUND:
        if (*off != size2) return normalize_error_ops();
        break;
      case NPMR_ERROR:
        return NULL;
    }
  }

  /* 跳过所有 PLM-memberid 对 */
  while (ops[0] != DDS_OP_RTS) ops += 2;

  return ops;
}

/**
 * @brief 对数据流进行规范化处理的实现函数
 *
 * @param[in] data          输入数据指针，需要规范化处理的数据
 * @param[in,out] off       数据偏移量指针，用于记录当前处理位置
 * @param[in] size          数据大小
 * @param[in] bswap         是否需要字节交换
 * @param[in] xcdr_version  XCDR版本
 * @param[in] ops           操作指令集指针
 * @param[in] is_mutable_member 是否为可变成员
 * @return 返回处理后的操作指令集指针，如果出错则返回NULL
 */
static const uint32_t *stream_normalize_data_impl(char *__restrict data,
                                                  uint32_t *__restrict off,
                                                  uint32_t size,
                                                  bool bswap,
                                                  uint32_t xcdr_version,
                                                  const uint32_t *__restrict ops,
                                                  bool is_mutable_member)
    ddsrt_attribute_warn_unused_result ddsrt_nonnull((1, 2, 6));

static const uint32_t *stream_normalize_data_impl(char *__restrict data,
                                                  uint32_t *__restrict off,
                                                  uint32_t size,
                                                  bool bswap,
                                                  uint32_t xcdr_version,
                                                  const uint32_t *__restrict ops,
                                                  bool is_mutable_member) {
  uint32_t insn;
  // 循环处理操作指令集，直到遇到DDS_OP_RTS指令
  while ((insn = *ops) != DDS_OP_RTS) {
    // 根据指令类型进行相应处理
    switch (DDS_OP(insn)) {
      case DDS_OP_ADR: {
        // 处理DDS_OP_ADR指令
        if ((ops = stream_normalize_adr(insn, data, off, size, bswap, xcdr_version, ops,
                                        is_mutable_member)) == NULL)
          return NULL;
        break;
      }
      case DDS_OP_JSR: {
        // 递归处理DDS_OP_JSR指令
        if (stream_normalize_data_impl(data, off, size, bswap, xcdr_version,
                                       ops + DDS_OP_JUMP(insn), is_mutable_member) == NULL)
          return NULL;
        ops++;
        break;
      }
      case DDS_OP_RTS:
      case DDS_OP_JEQ:
      case DDS_OP_JEQ4:
      case DDS_OP_KOF:
      case DDS_OP_PLM: {
        // 遇到以上指令类型时，终止程序
        abort();
        break;
      }
      case DDS_OP_DLC: {
        // 处理DDS_OP_DLC指令
        if (xcdr_version != DDSI_RTPS_CDR_ENC_VERSION_2) return normalize_error_ops();
        if ((ops = stream_normalize_delimited(data, off, size, bswap, xcdr_version, ops)) == NULL)
          return NULL;
        break;
      }
      case DDS_OP_PLC: {
        // 处理DDS_OP_PLC指令
        if (xcdr_version != DDSI_RTPS_CDR_ENC_VERSION_2) return normalize_error_ops();
        if ((ops = stream_normalize_pl(data, off, size, bswap, xcdr_version, ops)) == NULL)
          return NULL;
        break;
      }
    }
  }
  // 返回处理后的操作指令集指针
  return ops;
}

/**
 * @brief 对数据流进行规范化处理
 *
 * @param[in] data          输入的数据指针
 * @param[in,out] off       数据偏移量指针
 * @param[in] size          数据大小
 * @param[in] bswap         是否需要字节交换
 * @param[in] xcdr_version  XCDR版本
 * @param[in] ops           操作指针
 * @return 返回规范化后的数据流
 */
const uint32_t *dds_stream_normalize_data(char *__restrict data,
                                          uint32_t *__restrict off,
                                          uint32_t size,
                                          bool bswap,
                                          uint32_t xcdr_version,
                                          const uint32_t *__restrict ops) {
  // 调用实现函数进行数据流规范化处理
  return stream_normalize_data_impl(data, off, size, bswap, xcdr_version, ops, false);
}

// 声明静态函数stream_normalize_key_impl
static bool stream_normalize_key_impl(void *__restrict data,
                                      uint32_t size,
                                      uint32_t *offs,
                                      bool bswap,
                                      uint32_t xcdr_version,
                                      const uint32_t *__restrict ops,
                                      uint16_t key_offset_count,
                                      const uint32_t *key_offset_insn)
    ddsrt_attribute_warn_unused_result ddsrt_nonnull((1, 3, 6));

/**
 * @brief 实现对数据流中的键进行规范化处理
 *
 * @param[in] data              输入的数据指针
 * @param[in] size              数据大小
 * @param[in,out] offs          数据偏移量指针
 * @param[in] bswap             是否需要字节交换
 * @param[in] xcdr_version      XCDR版本
 * @param[in] ops               操作指针
 * @param[in] key_offset_count  键偏移量计数
 * @param[in] key_offset_insn   键偏移量指令
 * @return 返回是否成功规范化键
 */
static bool stream_normalize_key_impl(void *__restrict data,
                                      uint32_t size,
                                      uint32_t *offs,
                                      bool bswap,
                                      uint32_t xcdr_version,
                                      const uint32_t *__restrict ops,
                                      uint16_t key_offset_count,
                                      const uint32_t *key_offset_insn) {
  // 获取操作指令
  uint32_t insn = ops[0];
  // 检查指令是否合法
  assert(insn_key_ok_p(insn));

  // 根据指令类型进行相应的处理
  switch (DDS_OP_TYPE(insn)) {
    case DDS_OP_VAL_BLN:
      if (!normalize_bool(data, offs, size)) return false;
      break;
    case DDS_OP_VAL_1BY:
      if (!normalize_uint8(offs, size)) return false;
      break;
    case DDS_OP_VAL_2BY:
      if (!normalize_uint16(data, offs, size, bswap)) return false;
      break;
    case DDS_OP_VAL_4BY:
      if (!normalize_uint32(data, offs, size, bswap)) return false;
      break;
    case DDS_OP_VAL_ENU:
      if (!normalize_enum(data, offs, size, bswap, insn, ops[2])) return false;
      break;
    case DDS_OP_VAL_BMK:
      if (!normalize_bitmask(data, offs, size, bswap, xcdr_version, insn, ops[2], ops[3]))
        return false;
      break;
    case DDS_OP_VAL_8BY:
      if (!normalize_uint64(data, offs, size, bswap, xcdr_version)) return false;
      break;
    case DDS_OP_VAL_STR:
      if (!normalize_string(data, offs, size, bswap, SIZE_MAX)) return false;
      break;
    case DDS_OP_VAL_BST:
      if (!normalize_string(data, offs, size, bswap, ops[2])) return false;
      break;
    case DDS_OP_VAL_ARR:
      if (!normalize_arr(data, offs, size, bswap, xcdr_version, ops, insn)) return false;
      break;
    case DDS_OP_VAL_EXT: {
      assert(key_offset_count > 0);
      const uint32_t *jsr_ops = ops + DDS_OP_ADR_JSR(ops[2]) + *key_offset_insn;
      if (!stream_normalize_key_impl(data, size, offs, bswap, xcdr_version, jsr_ops,
                                     --key_offset_count, ++key_offset_insn))
        return false;
      break;
    }
    case DDS_OP_VAL_SEQ:
    case DDS_OP_VAL_BSQ:
    case DDS_OP_VAL_UNI:
    case DDS_OP_VAL_STU:
      abort();
      break;
  }

  // 返回成功
  return true;
}

/**
 * @brief 对给定的数据流进行规范化处理，以便于在序列化和反序列化过程中正确处理键值。
 *
 * @param[in] data          需要规范化处理的数据指针。
 * @param[in] size          数据的大小（字节）。
 * @param[in] bswap         是否需要交换字节序。
 * @param[in] xcdr_version  XCDR 版本。
 * @param[in] desc          描述数据结构的 dds_cdrstream_desc 结构体指针。
 * @param[out] actual_size  实际处理的数据大小（字节）。
 * @return bool             如果成功规范化，则返回 true，否则返回 false。
 */
static bool stream_normalize_key(void *__restrict data,
                                 uint32_t size,
                                 bool bswap,
                                 uint32_t xcdr_version,
                                 const struct dds_cdrstream_desc *__restrict desc,
                                 uint32_t *actual_size)
    ddsrt_attribute_warn_unused_result ddsrt_nonnull_all;
static bool stream_normalize_key(void *__restrict data,
                                 uint32_t size,
                                 bool bswap,
                                 uint32_t xcdr_version,
                                 const struct dds_cdrstream_desc *__restrict desc,
                                 uint32_t *actual_size) {
  uint32_t offs = 0;                               // 初始化偏移量为 0
  for (uint32_t i = 0; i < desc->keys.nkeys; i++)  // 遍历所有键
  {
    const uint32_t *op = desc->ops.ops + desc->keys.keys[i].ops_offs;  // 获取操作指针
    switch (DDS_OP(*op))  // 根据操作类型进行处理
    {
      case DDS_OP_KOF: {
        uint16_t n_offs = DDS_OP_LENGTH(*op);  // 获取操作长度
        if (!stream_normalize_key_impl(data, size, &offs, bswap, xcdr_version,
                                       desc->ops.ops + op[1], --n_offs, op + 2))
          return false;  // 如果规范化失败，返回 false
        break;
      }
      case DDS_OP_ADR: {
        if (!stream_normalize_key_impl(data, size, &offs, bswap, xcdr_version, op, 0, NULL))
          return false;  // 如果规范化失败，返回 false
        break;
      }
      default:
        abort();  // 遇到未知操作类型，中止程序
        break;
    }
  }
  *actual_size = offs;  // 设置实际处理的数据大小
  return true;          // 规范化成功，返回 true
}

/**
 * @brief 对给定的数据流进行规范化处理。
 *
 * @param[in] data          需要规范化处理的数据指针。
 * @param[in] size          数据的大小（字节）。
 * @param[in] bswap         是否需要交换字节序。
 * @param[in] xcdr_version  XCDR 版本。
 * @param[in] desc          描述数据结构的 dds_cdrstream_desc 结构体指针。
 * @param[in] just_key      是否仅对键值进行规范化处理。
 * @param[out] actual_size  实际处理的数据大小（字节）。
 * @return bool             如果成功规范化，则返回 true，否则返回 false。
 */
bool dds_stream_normalize(void *__restrict data,
                          uint32_t size,
                          bool bswap,
                          uint32_t xcdr_version,
                          const struct dds_cdrstream_desc *__restrict desc,
                          bool just_key,
                          uint32_t *__restrict actual_size) {
  uint32_t off = 0;  // 初始化偏移量为 0
  if (size > CDR_SIZE_MAX)
    return normalize_error_bool();  // 如果数据大小超过最大限制，返回错误
  else if (just_key)
    return stream_normalize_key(
        data, size, bswap, xcdr_version, desc,
        actual_size);  // 如果仅对键值进行规范化处理，调用 stream_normalize_key 函数
  else if (!stream_normalize_data_impl(data, &off, size, bswap, xcdr_version, desc->ops.ops, false))
    return false;  // 如果规范化失败，返回 false
  else {
    *actual_size = off;  // 设置实际处理的数据大小
    return true;         // 规范化成功，返回 true
  }
}

/*******************************************************************************************
 **
 **  Freeing samples
 **
 *******************************************************************************************/

/**
 * @brief 释放DDS序列中的样本空间
 *
 * @param[in] addr 序列地址
 * @param[in] allocator 内存分配器
 * @param[in] ops 操作指针
 * @param[in] insn 指令
 * @return 返回操作指针
 */
static const uint32_t *dds_stream_free_sample_seq(
    char *__restrict addr,
    const struct dds_cdrstream_allocator *__restrict allocator,
    const uint32_t *__restrict ops,
    uint32_t insn) {
  // 将地址转换为dds_sequence_t类型的指针
  dds_sequence_t *const seq = (dds_sequence_t *)addr;

  // 获取序列的最大值和长度中较大的一个
  uint32_t num = (seq->_maximum > seq->_length) ? seq->_maximum : seq->_length;

  // 获取子类型
  const enum dds_stream_typecode subtype = DDS_OP_SUBTYPE(insn);

  // 判断序列是否有界
  uint32_t bound_op = seq_is_bounded(DDS_OP_TYPE(insn)) ? 1 : 0;

  // 如果需要释放内存并且子类型大于字符串类型
  if ((seq->_release && num) || subtype > DDS_OP_VAL_STR) {
    switch (subtype) {
      case DDS_OP_VAL_BLN:
      case DDS_OP_VAL_1BY:
      case DDS_OP_VAL_2BY:
      case DDS_OP_VAL_4BY:
      case DDS_OP_VAL_8BY:
        ops += 2 + bound_op;
        break;
      case DDS_OP_VAL_BST:
      case DDS_OP_VAL_ENU:
        ops += 3 + bound_op;
        break;
      case DDS_OP_VAL_BMK:
        ops += 4 + bound_op;
        break;
      case DDS_OP_VAL_STR: {
        // 释放字符串缓冲区
        char **ptr = (char **)seq->_buffer;
        while (num--) allocator->free(*ptr++);
        ops += 2 + bound_op;
        break;
      }
      case DDS_OP_VAL_SEQ:
      case DDS_OP_VAL_BSQ:
      case DDS_OP_VAL_ARR:
      case DDS_OP_VAL_UNI:
      case DDS_OP_VAL_STU: {
        // 获取元素大小和跳转指针
        const uint32_t elem_size = ops[2 + bound_op];
        const uint32_t jmp = DDS_OP_ADR_JMP(ops[3 + bound_op]);
        const uint32_t *jsr_ops = ops + DDS_OP_ADR_JSR(ops[3 + bound_op]);

        // 递归释放样本空间
        char *ptr = (char *)seq->_buffer;
        while (num--) {
          dds_stream_free_sample(ptr, allocator, jsr_ops);
          ptr += elem_size;
        }
        ops += jmp ? jmp : (4 + bound_op);
        break;
      }
      case DDS_OP_VAL_EXT: {
        abort(); /* not supported */
        break;
      }
    }
  } else
    ops = skip_sequence_insns(insn, ops);

  // 如果需要释放内存，释放缓冲区并重置序列属性
  if (seq->_release) {
    allocator->free(seq->_buffer);
    seq->_maximum = 0;
    seq->_length = 0;
    seq->_buffer = NULL;
  }

  return ops;
}

/**
 * @brief 释放DDS流中的样本数组
 *
 * @param[in] addr          指向需要释放的样本数组的指针
 * @param[in] allocator     指向dds_cdrstream_allocator结构体的指针，用于内存分配和释放
 * @param[in] ops           指向操作码数组的指针
 * @param[in] insn          当前操作的指令
 * @return 返回处理后的操作码数组指针
 */
static const uint32_t *dds_stream_free_sample_arr(
    char *__restrict addr,
    const struct dds_cdrstream_allocator *__restrict allocator,
    const uint32_t *__restrict ops,
    uint32_t insn) {
  // 跳过前两个操作码
  ops += 2;

  // 获取子类型
  const enum dds_stream_typecode subtype = DDS_OP_SUBTYPE(insn);

  // 获取元素数量
  uint32_t num = *ops++;

  // 根据子类型进行处理
  switch (subtype) {
    case DDS_OP_VAL_BLN:
    case DDS_OP_VAL_1BY:
    case DDS_OP_VAL_2BY:
    case DDS_OP_VAL_4BY:
    case DDS_OP_VAL_8BY:
      // 对于这些基本类型，无需额外处理
      break;
    case DDS_OP_VAL_ENU:
      // 跳过枚举类型的操作码
      ops++;
      break;
    case DDS_OP_VAL_BMK:
    case DDS_OP_VAL_BST:
      // 跳过位掩码和位集类型的操作码
      ops += 2;
      break;
    case DDS_OP_VAL_STR: {
      // 处理字符串类型
      char **ptr = (char **)addr;
      while (num--) allocator->free(*ptr++);
      break;
    }
    case DDS_OP_VAL_SEQ:
    case DDS_OP_VAL_BSQ:
    case DDS_OP_VAL_ARR:
    case DDS_OP_VAL_UNI:
    case DDS_OP_VAL_STU: {
      // 处理序列、数组、联合体和结构体类型
      const uint32_t *jsr_ops = ops + DDS_OP_ADR_JSR(*ops) - 3;
      const uint32_t jmp = DDS_OP_ADR_JMP(*ops);
      const uint32_t elem_size = ops[1];
      while (num--) {
        dds_stream_free_sample(addr, allocator, jsr_ops);
        addr += elem_size;
      }
      ops += jmp ? (jmp - 3) : 2;
      break;
    }
    case DDS_OP_VAL_EXT: {
      // 不支持的扩展类型，直接中止程序
      abort();
      break;
    }
  }

  // 返回处理后的操作码数组指针
  return ops;
}

/**
 * @brief 释放DDS流中的样本并返回操作指针
 *
 * @param[in] discaddr 指向鉴别器地址的指针
 * @param[in] baseaddr 指向基地址的指针
 * @param[in] allocator 指向dds_cdrstream_allocator结构体的指针
 * @param[in] ops 指向操作列表的指针
 * @param[in] insn 32位无符号整数，表示指令
 * @return 返回操作指针
 */
static const uint32_t *dds_stream_free_sample_uni(
    char *__restrict discaddr,
    char *__restrict baseaddr,
    const struct dds_cdrstream_allocator *__restrict allocator,
    const uint32_t *__restrict ops,
    uint32_t insn) {
  uint32_t disc = 0;
  // 根据指令的子类型获取鉴别器值
  switch (DDS_OP_SUBTYPE(insn)) {
    case DDS_OP_VAL_BLN:
    case DDS_OP_VAL_1BY:
      disc = *((uint8_t *)discaddr);
      break;
    case DDS_OP_VAL_2BY:
      disc = *((uint16_t *)discaddr);
      break;
    case DDS_OP_VAL_4BY:
    case DDS_OP_VAL_ENU:
      disc = *((uint32_t *)discaddr);
      break;
    default:
      abort();
      break;
  }
  // 查找联合体情况
  uint32_t const *const jeq_op = find_union_case(ops, disc);
  ops += DDS_OP_ADR_JMP(ops[3]);
  if (jeq_op) {
    const enum dds_stream_typecode subtype = DDS_JEQ_TYPE(jeq_op[0]);
    void *valaddr = baseaddr + jeq_op[2];

    // 如果是外部成员，除字符串外，取消引用地址
    if (op_type_external(jeq_op[0])) {
      assert(DDS_OP(jeq_op[0]) == DDS_OP_JEQ4);
      valaddr = *((char **)valaddr);
      if (!valaddr) goto no_ext_member;
    }

    // 根据子类型处理释放操作
    switch (subtype) {
      case DDS_OP_VAL_BLN:
      case DDS_OP_VAL_1BY:
      case DDS_OP_VAL_2BY:
      case DDS_OP_VAL_4BY:
      case DDS_OP_VAL_8BY:
      case DDS_OP_VAL_BST:
      case DDS_OP_VAL_ENU:
        break;
      case DDS_OP_VAL_STR:
        allocator->free(*((char **)valaddr));
        *((char **)valaddr) = NULL;
        break;
      case DDS_OP_VAL_UNI:
      case DDS_OP_VAL_STU:
      case DDS_OP_VAL_SEQ:
      case DDS_OP_VAL_BSQ:
      case DDS_OP_VAL_ARR:
      case DDS_OP_VAL_BMK:
        dds_stream_free_sample(valaddr, allocator, jeq_op + DDS_OP_ADR_JSR(jeq_op[0]));
        break;
      case DDS_OP_VAL_EXT:
        abort(); /* not supported */
        break;
    }

    // 释放外部字段的缓冲区
    if (op_type_external(jeq_op[0])) {
      allocator->free(valaddr);
      valaddr = NULL;
    }
  }
no_ext_member:
  return ops;
}

/**
 * @brief 释放DDS流中的样本内存
 * @param[in] addr 样本地址
 * @param[in] allocator 内存分配器
 * @param[in] ops 操作指针
 * @return 返回操作指针
 *
 * 该函数用于释放DDS流中的样本内存。
 */
static const uint32_t *dds_stream_free_sample_pl(
    char *__restrict addr,
    const struct dds_cdrstream_allocator *__restrict allocator,
    const uint32_t *__restrict ops) {
  uint32_t insn;
  assert(ops[0] == DDS_OP_PLC);
  ops++; /* 跳过 PLC 操作 */
  while ((insn = *ops) != DDS_OP_RTS) {
    switch (DDS_OP(insn)) {
      case DDS_OP_PLM: {
        const uint32_t *plm_ops = ops + DDS_OP_ADR_PLM(insn);
        uint32_t flags = DDS_PLM_FLAGS(insn);
        if (flags & DDS_OP_FLAG_BASE)
          (void)dds_stream_free_sample_pl(addr, allocator, plm_ops);
        else
          dds_stream_free_sample(addr, allocator, plm_ops);
        ops += 2;
        break;
      }
      default:
        abort(); /* 此时不支持其他操作 */
        break;
    }
  }
  return ops;
}

/**
 * @brief 释放非外部地址的样本内存
 * @param[in] insn 指令
 * @param[in] addr 地址
 * @param[in] data 数据
 * @param[in] allocator 内存分配器
 * @param[in] ops 操作指针
 * @return 返回操作指针
 *
 * 该函数用于释放非外部地址的样本内存。
 */
static const uint32_t *stream_free_sample_adr_nonexternal(
    uint32_t insn,
    void *__restrict addr,
    void *__restrict data,
    const struct dds_cdrstream_allocator *__restrict allocator,
    const uint32_t *__restrict ops) {
  assert(DDS_OP(insn) == DDS_OP_ADR);

  switch (DDS_OP_TYPE(insn)) {
    case DDS_OP_VAL_BLN:
    case DDS_OP_VAL_1BY:
    case DDS_OP_VAL_2BY:
    case DDS_OP_VAL_4BY:
    case DDS_OP_VAL_8BY:
      ops += 2;
      break;
    case DDS_OP_VAL_STR: {
      allocator->free(*((char **)addr));
      *(char **)addr = NULL;
      ops += 2;
      break;
    }
    case DDS_OP_VAL_BST:
    case DDS_OP_VAL_ENU:
      ops += 3;
      break;
    case DDS_OP_VAL_BMK:
      ops += 4;
      break;
    case DDS_OP_VAL_SEQ:
    case DDS_OP_VAL_BSQ:
      ops = dds_stream_free_sample_seq(addr, allocator, ops, insn);
      break;
    case DDS_OP_VAL_ARR:
      ops = dds_stream_free_sample_arr(addr, allocator, ops, insn);
      break;
    case DDS_OP_VAL_UNI:
      ops = dds_stream_free_sample_uni(addr, data, allocator, ops, insn);
      break;
    case DDS_OP_VAL_EXT: {
      const uint32_t *jsr_ops = ops + DDS_OP_ADR_JSR(ops[2]);
      const uint32_t jmp = DDS_OP_ADR_JMP(ops[2]);
      dds_stream_free_sample(addr, allocator, jsr_ops);
      ops += jmp ? jmp : 3;
      break;
    }
    case DDS_OP_VAL_STU:
      abort(); /* STU 类型仅作为子类型支持 */
      break;
  }

  return ops;
}

/**
 * @brief 释放非外部类型的样本地址
 *
 * @param[in] insn 指令
 * @param[in] data 数据指针
 * @param[in] allocator 分配器指针
 * @param[in] ops 操作指针
 * @return 返回操作指针
 */
static const uint32_t *stream_free_sample_adr(
    uint32_t insn,
    void *__restrict data,
    const struct dds_cdrstream_allocator *__restrict allocator,
    const uint32_t *__restrict ops) {
  // 断言指令为 DDS_OP_ADR 类型
  assert(DDS_OP(insn) == DDS_OP_ADR);

  // 如果不是外部类型
  if (!op_type_external(insn)) {
    // 计算地址
    void *addr = (char *)data + ops[1];
    // 调用非外部类型的释放函数
    ops = stream_free_sample_adr_nonexternal(insn, addr, data, allocator, ops);
  } else {
    // 计算外部地址
    void **ext_addr = (void **)((char *)data + ops[1]);
    void *addr = *ext_addr;

    // 如果地址为空
    if (addr == NULL) {
      // 跳过地址
      ops = dds_stream_skip_adr(insn, ops);
    } else {
      // 调用非外部类型的释放函数
      ops = stream_free_sample_adr_nonexternal(insn, addr, data, allocator, ops);
      // 释放外部地址
      allocator->free(*ext_addr);
      // 将外部地址置空
      *ext_addr = NULL;
    }
  }

  // 返回操作指针
  return ops;
}

/**
 * @brief 释放样本数据
 *
 * @param[in] data 数据指针
 * @param[in] allocator 分配器指针
 * @param[in] ops 操作指针
 */
void dds_stream_free_sample(void *__restrict data,
                            const struct dds_cdrstream_allocator *__restrict allocator,
                            const uint32_t *__restrict ops) {
  uint32_t insn;

  // 循环处理操作指令，直到遇到 DDS_OP_RTS
  while ((insn = *ops) != DDS_OP_RTS) {
    // 根据指令类型进行处理
    switch (DDS_OP(insn)) {
      case DDS_OP_ADR:
        // 处理地址类型指令
        ops = stream_free_sample_adr(insn, data, allocator, ops);
        break;
      case DDS_OP_JSR:
        // 跳转到子程序
        dds_stream_free_sample(data, allocator, ops + DDS_OP_JUMP(insn));
        ops++;
        break;
      case DDS_OP_RTS:
      case DDS_OP_JEQ:
      case DDS_OP_JEQ4:
      case DDS_OP_KOF:
      case DDS_OP_PLM:
        // 遇到不支持的指令，终止程序
        abort();
        break;
      case DDS_OP_DLC:
        // 跳过指令
        ops++;
        break;
      case DDS_OP_PLC:
        // 处理 PLC 类型指令
        ops = dds_stream_free_sample_pl(data, allocator, ops);
        break;
    }
  }
}

/*******************************************************************************************
 **
 **  提取密钥/密钥哈希（唯一的区别是密钥哈希必须是大端字节序，
 **  填充必须清除，可能需要通过MD5运行该值。
 **
 *******************************************************************************************/

/**
 * @brief 从键值中提取主要操作的数据流。
 *
 * @param[in] is 输入流指针，用于读取数据。
 * @param[out] os 输出流指针，用于写入数据。
 * @param[in] allocator 分配器指针，用于内存分配。
 * @param[in] ops 操作列表指针，包含操作码。
 * @param[in] key_offset_count 键偏移计数。
 * @param[in] key_offset_insn 键偏移指令指针。
 */
static void dds_stream_extract_key_from_key_prim_op(
    dds_istream_t *__restrict is,
    dds_ostream_t *__restrict os,
    const struct dds_cdrstream_allocator *__restrict allocator,
    const uint32_t *__restrict ops,
    uint16_t key_offset_count,
    const uint32_t *key_offset_insn) {
  // 获取操作码
  const uint32_t insn = *ops;
  // 断言操作码是否为键值和地址操作
  assert((insn & DDS_OP_FLAG_KEY) && ((DDS_OP(insn)) == DDS_OP_ADR));

  // 根据操作类型进行处理
  switch (DDS_OP_TYPE(insn)) {
    case DDS_OP_VAL_BLN:
    case DDS_OP_VAL_1BY:
      // 处理布尔值和1字节数据
      dds_os_put1(os, allocator, dds_is_get1(is));
      break;
    case DDS_OP_VAL_2BY:
      // 处理2字节数据
      dds_os_put2(os, allocator, dds_is_get2(is));
      break;
    case DDS_OP_VAL_4BY:
      // 处理4字节数据
      dds_os_put4(os, allocator, dds_is_get4(is));
      break;
    case DDS_OP_VAL_8BY:
      // 处理8字节数据
      dds_os_put8(os, allocator, dds_is_get8(is));
      break;
    case DDS_OP_VAL_ENU:
    case DDS_OP_VAL_BMK:
      // 处理枚举值和位掩码
      switch (DDS_OP_TYPE_SZ(insn)) {
        case 1:
          dds_os_put1(os, allocator, dds_is_get1(is));
          break;
        case 2:
          dds_os_put2(os, allocator, dds_is_get2(is));
          break;
        case 4:
          dds_os_put4(os, allocator, dds_is_get4(is));
          break;
        case 8:
          assert(DDS_OP_TYPE(insn) == DDS_OP_VAL_BMK);
          dds_os_put8(os, allocator, dds_is_get8(is));
          break;
        default:
          abort();
      }
      break;
    case DDS_OP_VAL_STR:
    case DDS_OP_VAL_BST: {
      // 处理字符串和字节串
      uint32_t sz = dds_is_get4(is);
      dds_os_put4(os, allocator, sz);
      dds_os_put_bytes(os, allocator, is->m_buffer + is->m_index, sz);
      is->m_index += sz;
      break;
    }
    case DDS_OP_VAL_ARR: {
      // 处理数组
      const enum dds_stream_typecode subtype = DDS_OP_SUBTYPE(insn);
      uint32_t elem_size, offs = 0, xcdrv = ((struct dds_ostream *)os)->m_xcdr_version;
      if (is_dheader_needed(subtype, xcdrv)) {
        // 非基本元素类型时，为输出流预留DHEADER空间，并跳过输入中的DHEADER
        dds_os_reserve4(os, allocator);
        offs = ((struct dds_ostream *)os)->m_index;
        (void)dds_is_get4(is);
      }
      if (is_primitive_type(subtype))
        elem_size = get_primitive_size(subtype);
      else if (subtype == DDS_OP_VAL_ENU || subtype == DDS_OP_VAL_BMK)
        elem_size = DDS_OP_TYPE_SZ(insn);
      else
        abort();
      const align_t align = dds_cdr_get_align(os->m_xcdr_version, elem_size);
      const uint32_t num = ops[2];
      dds_cdr_alignto(is, align);
      dds_cdr_alignto_clear_and_resize(os, allocator, align, num * elem_size);
      void *const dst = os->m_buffer + os->m_index;
      dds_is_get_bytes(is, dst, num, elem_size);
      os->m_index += num * elem_size;
      // 设置DHEADER
      if (is_dheader_needed(subtype, xcdrv))
        *((uint32_t *)(((struct dds_ostream *)os)->m_buffer + offs - 4)) =
            ((struct dds_ostream *)os)->m_index - offs;
      break;
    }
    case DDS_OP_VAL_EXT: {
      // 处理扩展操作
      assert(key_offset_count > 0);
      const uint32_t *jsr_ops = ops + DDS_OP_ADR_JSR(ops[2]) + *key_offset_insn;
      dds_stream_extract_key_from_key_prim_op(is, os, allocator, jsr_ops, --key_offset_count,
                                              ++key_offset_insn);
      break;
    }
    case DDS_OP_VAL_SEQ:
    case DDS_OP_VAL_BSQ:
    case DDS_OP_VAL_UNI:
    case DDS_OP_VAL_STU: {
      // 不支持的操作类型
      abort();
      break;
    }
  }
}

/**
 * @brief 在小端字节序环境下，将源数据按指定大小进行字节交换并复制到目标缓冲区。
 *
 * @param[out] vdst 目标缓冲区的指针。
 * @param[in]  vsrc 源数据的指针。
 * @param[in]  size 数据块的大小（1、2、4 或 8 字节）。
 * @param[in]  num  要复制的数据块数量。
 */
#if DDSRT_ENDIAN == DDSRT_LITTLE_ENDIAN
static void dds_stream_swap_copy(void *__restrict vdst,
                                 const void *__restrict vsrc,
                                 uint32_t size,
                                 uint32_t num) {
  // 确保 size 参数有效
  assert(size == 1 || size == 2 || size == 4 || size == 8);

  // 根据 size 参数选择不同的处理方式
  switch (size) {
    case 1:
      // 当 size 为 1 时，直接复制数据
      memcpy(vdst, vsrc, num);
      break;
    case 2: {
      // 当 size 为 2 时，对每个 16 位数据进行字节交换并复制
      const uint16_t *src = vsrc;
      uint16_t *dst = vdst;
      for (uint32_t i = 0; i < num; i++) dst[i] = ddsrt_bswap2u(src[i]);
      break;
    }
    case 4: {
      // 当 size 为 4 时，对每个 32 位数据进行字节交换并复制
      const uint32_t *src = vsrc;
      uint32_t *dst = vdst;
      for (uint32_t i = 0; i < num; i++) dst[i] = ddsrt_bswap4u(src[i]);
      break;
    }
    case 8: {
      // 当 size 为 8 时，对每个 64 位数据进行字节交换并复制
      const uint64_t *src = vsrc;
      uint64_t *dst = vdst;
      for (uint32_t i = 0; i < num; i++) {
        *(uint32_t *)&dst[i] = ddsrt_bswap4u(*(((uint32_t *)&src[i]) + 1));
        *(((uint32_t *)&dst[i]) + 1) = ddsrt_bswap4u(*(uint32_t *)&src[i]);
      }
      break;
    }
  }
}
#endif

/**
 * @brief 从键原始操作中提取dds流的键值（大端字节序）
 *
 * @param[in] is 输入流指针
 * @param[out] os 输出流指针（大端字节序）
 * @param[in] allocator 内存分配器指针
 * @param[in] ops 操作列表指针
 * @param[in] key_offset_count 键偏移计数
 * @param[in] key_offset_insn 键偏移指令指针
 */
static void dds_stream_extract_keyBE_from_key_prim_op(
    dds_istream_t *__restrict is,
    dds_ostreamBE_t *__restrict os,
    const struct dds_cdrstream_allocator *__restrict allocator,
    const uint32_t *__restrict ops,
    uint16_t key_offset_count,
    const uint32_t *key_offset_insn) {
  // 获取当前操作指令
  const uint32_t insn = *ops;
  // 断言：检查操作是否为键操作且地址操作
  assert((insn & DDS_OP_FLAG_KEY) && ((DDS_OP(insn)) == DDS_OP_ADR));

  // 根据操作类型进行处理
  switch (DDS_OP_TYPE(insn)) {
    case DDS_OP_VAL_BLN:
    case DDS_OP_VAL_1BY:
      // 处理布尔值和1字节数据
      dds_os_put1BE(os, allocator, dds_is_get1(is));
      break;
    case DDS_OP_VAL_2BY:
      // 处理2字节数据
      dds_os_put2BE(os, allocator, dds_is_get2(is));
      break;
    case DDS_OP_VAL_4BY:
      // 处理4字节数据
      dds_os_put4BE(os, allocator, dds_is_get4(is));
      break;
    case DDS_OP_VAL_8BY:
      // 处理8字节数据
      dds_os_put8BE(os, allocator, dds_is_get8(is));
      break;
    case DDS_OP_VAL_ENU:
    case DDS_OP_VAL_BMK:
      // 处理枚举和位掩码值
      switch (DDS_OP_TYPE_SZ(insn)) {
        case 1:
          dds_os_put1BE(os, allocator, dds_is_get1(is));
          break;
        case 2:
          dds_os_put2BE(os, allocator, dds_is_get2(is));
          break;
        case 4:
          dds_os_put4BE(os, allocator, dds_is_get4(is));
          break;
        case 8:
          assert(DDS_OP_TYPE(insn) == DDS_OP_VAL_BMK);
          dds_os_put8BE(os, allocator, dds_is_get8(is));
          break;
        default:
          abort();
      }
      break;
    case DDS_OP_VAL_STR:
    case DDS_OP_VAL_BST: {
      // 处理字符串和字节串
      uint32_t sz = dds_is_get4(is);
      dds_os_put4BE(os, allocator, sz);
      dds_os_put_bytes(&os->x, allocator, is->m_buffer + is->m_index, sz);
      is->m_index += sz;
      break;
    }
    case DDS_OP_VAL_ARR: {
      // 处理数组类型
      const enum dds_stream_typecode subtype = DDS_OP_SUBTYPE(insn);
      uint32_t elem_size, offs = 0, xcdrv = ((struct dds_ostream *)os)->m_xcdr_version;
      if (is_dheader_needed(subtype, xcdrv)) {
        // 非基本元素类型时，为输出流预留DHEADER空间，并跳过输入中的DHEADER
        dds_os_reserve4BE(os, allocator);
        offs = ((struct dds_ostream *)os)->m_index;
        (void)dds_is_get4(is);
      }
      if (is_primitive_type(subtype))
        elem_size = get_primitive_size(subtype);
      else if (subtype == DDS_OP_VAL_ENU || subtype == DDS_OP_VAL_BMK)
        elem_size = DDS_OP_TYPE_SZ(insn);
      else
        abort();
      const align_t align = dds_cdr_get_align(os->x.m_xcdr_version, elem_size);
      const uint32_t num = ops[2];
      dds_cdr_alignto(is, align);
      dds_cdr_alignto_clear_and_resizeBE(os, allocator, align, num * elem_size);
      void const *const src = is->m_buffer + is->m_index;
      void *const dst = os->x.m_buffer + os->x.m_index;
#if DDSRT_ENDIAN == DDSRT_LITTLE_ENDIAN
      dds_stream_swap_copy(dst, src, elem_size, num);
#else
      memcpy(dst, src, num * elem_size);
#endif
      os->x.m_index += num * elem_size;
      is->m_index += num * elem_size;

      // 设置DHEADER
      if (is_dheader_needed(subtype, xcdrv))
        *((uint32_t *)(((struct dds_ostream *)os)->m_buffer + offs - 4)) =
            ddsrt_toBE4u(((struct dds_ostream *)os)->m_index - offs);
      break;
    }
    case DDS_OP_VAL_EXT: {
      // 处理扩展类型
      assert(key_offset_count > 0);
      const uint32_t *jsr_ops = ops + DDS_OP_ADR_JSR(ops[2]) + *key_offset_insn;
      dds_stream_extract_keyBE_from_key_prim_op(is, os, allocator, jsr_ops, --key_offset_count,
                                                ++key_offset_insn);
      break;
    }
    case DDS_OP_VAL_SEQ:
    case DDS_OP_VAL_BSQ:
    case DDS_OP_VAL_UNI:
    case DDS_OP_VAL_STU: {
      // 不支持的操作类型，中止程序
      abort();
      break;
    }
  }
}

/**
 * @brief 从数据流中提取键值并跳过子类型
 *
 * @param[in] is       输入流指针，用于读取数据
 * @param[in] num      要处理的元素数量
 * @param[in] insn     指令值，用于确定元素大小
 * @param[in] subtype  子类型值，用于区分不同的操作
 * @param[in] subops   子操作数组，用于存储操作信息
 */
static void dds_stream_extract_key_from_data_skip_subtype(dds_istream_t *__restrict is,
                                                          uint32_t num,
                                                          uint32_t insn,
                                                          uint32_t subtype,
                                                          const uint32_t *__restrict subops) {
  // 根据子类型进行相应的操作
  switch (subtype) {
    case DDS_OP_VAL_BLN:
    case DDS_OP_VAL_1BY:
    case DDS_OP_VAL_2BY:
    case DDS_OP_VAL_4BY:
    case DDS_OP_VAL_8BY: {
      // 获取基本类型的大小
      const uint32_t elem_size = get_primitive_size(subtype);
      // 对齐输入流
      dds_cdr_alignto(is, dds_cdr_get_align(is->m_xcdr_version, elem_size));
      // 更新索引值
      is->m_index += num * elem_size;
      break;
    }
    case DDS_OP_VAL_ENU:
    case DDS_OP_VAL_BMK: {
      /* 获取枚举类型的大小：位掩码在联合体(JEQ4)中是一个特殊情况，
       因为位掩码定义在序列化指令的联合体指令之后，所以我们需要从subops[0]中获取位掩码的大小。
       枚举类型是在联合体指令中内联定义的，所以我们将使用insn来获取枚举类型的大小。 */
      const uint32_t elem_size = DDS_OP_TYPE_SZ(
          DDS_OP(insn) == DDS_OP_JEQ4 && subtype == DDS_OP_VAL_BMK ? subops[0] : insn);
      // 对齐输入流
      dds_cdr_alignto(is, dds_cdr_get_align(is->m_xcdr_version, elem_size));
      // 更新索引值
      is->m_index += num * elem_size;
      break;
    }
    case DDS_OP_VAL_STR:
    case DDS_OP_VAL_BST: {
      // 遍历处理字符串或者字节串
      for (uint32_t i = 0; i < num; i++) {
        // 获取长度值
        const uint32_t len = dds_is_get4(is);
        // 更新索引值
        is->m_index += len;
      }
      break;
    }
    case DDS_OP_VAL_SEQ:
    case DDS_OP_VAL_BSQ:
    case DDS_OP_VAL_ARR:
    case DDS_OP_VAL_UNI:
    case DDS_OP_VAL_STU: {
      // 初始化剩余元素数量
      uint32_t remain = UINT32_MAX;
      // 遍历处理序列、数组、联合体或结构体
      for (uint32_t i = 0; i < num; i++)
        dds_stream_extract_key_from_data1(is, NULL, NULL, 0, NULL, NULL, NULL, subops, false, false,
                                          remain, &remain, NULL, NULL);
      break;
    }
    case DDS_OP_VAL_EXT: {
      // 不支持的操作类型，直接中止程序
      abort();
      break;
    }
  }
}

/**
 * @brief 从数据中提取键并跳过数组
 *
 * @param[in] is       输入流指针
 * @param[in] ops      操作指针
 * @return 返回跳过数组指令后的操作指针
 */
static const uint32_t *dds_stream_extract_key_from_data_skip_array(dds_istream_t *__restrict is,
                                                                   const uint32_t *__restrict ops) {
  // 获取指令
  const uint32_t insn = *ops;
  // 断言指令类型为DDS_OP_VAL_ARR
  assert(DDS_OP_TYPE(insn) == DDS_OP_VAL_ARR);
  // 获取子类型
  const enum dds_stream_typecode subtype = DDS_OP_SUBTYPE(insn);
  // 获取操作数
  const uint32_t num = ops[2];

  // 如果需要DHEADER，使用其值跳过数组
  if (is_dheader_needed(subtype, is->m_xcdr_version)) {
    const uint32_t sz = dds_is_get4(is);
    is->m_index += sz;
  } else if (type_has_subtype_or_members(subtype))
    dds_stream_extract_key_from_data_skip_subtype(is, num, insn, subtype,
                                                  ops + DDS_OP_ADR_JSR(ops[3]));
  else
    dds_stream_extract_key_from_data_skip_subtype(is, num, insn, subtype, NULL);
  return skip_array_insns(insn, ops);
}

/**
 * @brief 从数据中提取键并跳过序列
 *
 * @param[in] is       输入流指针
 * @param[in] ops      操作指针
 * @return 返回跳过序列指令后的操作指针
 */
static const uint32_t *dds_stream_extract_key_from_data_skip_sequence(
    dds_istream_t *__restrict is, const uint32_t *__restrict ops) {
  // 获取指令
  const uint32_t insn = *ops;
  // 判断序列是否有边界
  uint32_t bound_op = seq_is_bounded(DDS_OP_TYPE(insn)) ? 1 : 0;
  // 获取子类型
  const enum dds_stream_typecode subtype = DDS_OP_SUBTYPE(insn);

  // 如果需要DHEADER，使用其值跳过序列
  if (is_dheader_needed(subtype, is->m_xcdr_version)) {
    const uint32_t sz = dds_is_get4(is);
    is->m_index += sz;
  } else {
    const uint32_t num = dds_is_get4(is);
    if (num > 0) {
      if (type_has_subtype_or_members(subtype))
        dds_stream_extract_key_from_data_skip_subtype(is, num, insn, subtype,
                                                      ops + DDS_OP_ADR_JSR(ops[3 + bound_op]));
      else
        dds_stream_extract_key_from_data_skip_subtype(is, num, insn, subtype, NULL);
    }
  }
  return skip_sequence_insns(insn, ops);
}

/**
 * @brief 从数据中提取键并跳过联合体
 *
 * @param[in] is       输入流指针
 * @param[in] ops      操作码数组指针
 * @return 返回操作码数组指针
 */
static const uint32_t *dds_stream_extract_key_from_data_skip_union(dds_istream_t *__restrict is,
                                                                   const uint32_t *__restrict ops) {
  // 获取操作码
  const uint32_t insn = *ops;
  // 断言操作码类型为联合体
  assert(DDS_OP_TYPE(insn) == DDS_OP_VAL_UNI);
  // 读取联合体的辨别值
  const uint32_t disc = read_union_discriminant(is, insn);
  // 查找联合体的情况
  uint32_t const *const jeq_op = find_union_case(ops, disc);
  // 如果找到了情况
  if (jeq_op)
    dds_stream_extract_key_from_data_skip_subtype(is, 1, jeq_op[0], DDS_JEQ_TYPE(jeq_op[0]),
                                                  jeq_op + DDS_OP_ADR_JSR(jeq_op[0]));
  // 返回操作码数组指针
  return ops + DDS_OP_ADR_JMP(ops[3]);
}

/**
 * @brief 从数据中提取键并跳过地址
 *
 * @param[in] is       输入流指针
 * @param[in] ops      操作码数组指针
 * @param[in] type     类型
 * @return 返回操作码数组指针
 */
static const uint32_t *dds_stream_extract_key_from_data_skip_adr(dds_istream_t *__restrict is,
                                                                 const uint32_t *__restrict ops,
                                                                 uint32_t type) {
  // 根据类型进行处理
  switch (type) {
    case DDS_OP_VAL_BLN:
    case DDS_OP_VAL_1BY:
    case DDS_OP_VAL_2BY:
    case DDS_OP_VAL_4BY:
    case DDS_OP_VAL_8BY:
    case DDS_OP_VAL_STR:
    case DDS_OP_VAL_BST:
    case DDS_OP_VAL_ENU:
    case DDS_OP_VAL_BMK:
      dds_stream_extract_key_from_data_skip_subtype(is, 1, ops[0], type, NULL);
      if (type == DDS_OP_VAL_BST || type == DDS_OP_VAL_ARR || type == DDS_OP_VAL_ENU)
        ops += 3;
      else if (type == DDS_OP_VAL_BMK)
        ops += 4;
      else
        ops += 2;
      break;
    case DDS_OP_VAL_SEQ:
    case DDS_OP_VAL_BSQ:
      ops = dds_stream_extract_key_from_data_skip_sequence(is, ops);
      break;
    case DDS_OP_VAL_ARR:
      ops = dds_stream_extract_key_from_data_skip_array(is, ops);
      break;
    case DDS_OP_VAL_UNI:
      ops = dds_stream_extract_key_from_data_skip_union(is, ops);
      break;
    case DDS_OP_VAL_STU:
      abort(); /* op type STU only supported as subtype */
      break;
  }
  // 返回操作码数组指针
  return ops;
}

/*******************************************************************************************
 **
 **  Read/write of samples and keys -- i.e., DDSI payloads.
 **
 *******************************************************************************************/

/**
 * @brief 从数据流中读取样本。
 *
 * @param[in] is 指向dds_istream_t结构体的指针，用于读取数据流。
 * @param[out] data 指向接收数据的缓冲区的指针。
 * @param[in] allocator 指向dds_cdrstream_allocator结构体的指针，用于分配内存。
 * @param[in] desc 指向dds_cdrstream_desc结构体的指针，描述数据流的格式。
 */
void dds_stream_read_sample(dds_istream_t *__restrict is,
                            void *__restrict data,
                            const struct dds_cdrstream_allocator *__restrict allocator,
                            const struct dds_cdrstream_desc *__restrict desc) {
  // 根据is->m_xcdr_version选择opt_size值
  size_t opt_size = is->m_xcdr_version == DDSI_RTPS_CDR_ENC_VERSION_1 ? desc->opt_size_xcdr1
                                                                      : desc->opt_size_xcdr2;

  if (opt_size) {
    /* 结构体和CDR的布局相同，但结构体的sizeof可能包含在CDR中不存在的填充，
       因此我们必须使用type->opt_size_xcdrx避免潜在的越界读取 */
    dds_is_get_bytes(is, data, (uint32_t)opt_size, 1);
  } else {
    if (desc->flagset & DDS_TOPIC_CONTAINS_UNION) {
      /* 如果某些情况下有序列或字符串，而其他情况下有其他映射到这些地址的内容，
         切换联合情况会导致很大的麻烦。因此，通过释放已分配的内容，然后清除所有内存来假装友好。
         这将使任何预先分配的缓冲区浪费，但它允许在读取之间重用消息，
         以较慢的反序列化为代价，并且不能在包含联合的主题中使用预先分配的序列。 */
      dds_stream_free_sample(data, allocator, desc->ops.ops);
      memset(data, 0, desc->size);
    }
    // 调用dds_stream_read_impl函数进行实际的数据读取
    (void)dds_stream_read_impl(is, data, allocator, desc->ops.ops, false);
  }
}

/**
 * @brief 读取数据流中的键值并实现解析
 *
 * @param[in] is 数据输入流
 * @param[out] sample 存储解析后的数据样本
 * @param[in] allocator 内存分配器
 * @param[in] ops 操作指令数组
 * @param[in] key_offset_count 键偏移计数
 * @param[in] key_offset_insn 键偏移指令
 */
static void dds_stream_read_key_impl(dds_istream_t *__restrict is,
                                     char *__restrict sample,
                                     const struct dds_cdrstream_allocator *__restrict allocator,
                                     const uint32_t *__restrict ops,
                                     uint16_t key_offset_count,
                                     const uint32_t *key_offset_insn) {
  // 目标地址
  void *dst = sample + ops[1];
  // 获取操作指令
  uint32_t insn = ops[0];
  // 检查指令是否有效
  assert(insn_key_ok_p(insn));

  // 如果是外部类型，则分配内存
  if (op_type_external(insn)) dds_stream_alloc_external(ops, insn, &dst, allocator);

  // 根据操作指令类型进行处理
  switch (DDS_OP_TYPE(insn)) {
    case DDS_OP_VAL_BLN:
    case DDS_OP_VAL_1BY:
      *((uint8_t *)dst) = dds_is_get1(is);
      break;
    case DDS_OP_VAL_2BY:
      *((uint16_t *)dst) = dds_is_get2(is);
      break;
    case DDS_OP_VAL_4BY:
      *((uint32_t *)dst) = dds_is_get4(is);
      break;
    case DDS_OP_VAL_8BY:
      *((uint64_t *)dst) = dds_is_get8(is);
      break;
    case DDS_OP_VAL_ENU:
      switch (DDS_OP_TYPE_SZ(insn)) {
        case 1:
          *((uint32_t *)dst) = dds_is_get1(is);
          break;
        case 2:
          *((uint32_t *)dst) = dds_is_get2(is);
          break;
        case 4:
          *((uint32_t *)dst) = dds_is_get4(is);
          break;
        default:
          abort();
      }
      break;
    case DDS_OP_VAL_BMK:
      switch (DDS_OP_TYPE_SZ(insn)) {
        case 1:
          *((uint8_t *)dst) = dds_is_get1(is);
          break;
        case 2:
          *((uint16_t *)dst) = dds_is_get2(is);
          break;
        case 4:
          *((uint32_t *)dst) = dds_is_get4(is);
          break;
        case 8:
          *((uint64_t *)dst) = dds_is_get8(is);
          break;
        default:
          abort();
      }
      break;
    case DDS_OP_VAL_STR:
      *((char **)dst) = dds_stream_reuse_string(is, *((char **)dst), allocator);
      break;
    case DDS_OP_VAL_BST:
      (void)dds_stream_reuse_string_bound(is, dst, allocator, ops[2], false);
      break;
    case DDS_OP_VAL_ARR: {
      const enum dds_stream_typecode subtype = DDS_OP_SUBTYPE(insn);
      uint32_t num = ops[2];
      // 如果元素类型非基本类型，则跳过输入中的DHEADER
      if (is_dheader_needed(subtype, is->m_xcdr_version)) (void)dds_is_get4(is);
      switch (subtype) {
        case DDS_OP_VAL_BLN:
        case DDS_OP_VAL_1BY:
        case DDS_OP_VAL_2BY:
        case DDS_OP_VAL_4BY:
        case DDS_OP_VAL_8BY:
          dds_is_get_bytes(is, dst, num, get_primitive_size(subtype));
          break;
        case DDS_OP_VAL_ENU:
          switch (DDS_OP_TYPE_SZ(insn)) {
            case 1:
              for (uint32_t i = 0; i < num; i++) ((uint32_t *)dst)[i] = dds_is_get1(is);
              break;
            case 2:
              for (uint32_t i = 0; i < num; i++) ((uint32_t *)dst)[i] = dds_is_get2(is);
              break;
            case 4:
              dds_is_get_bytes(is, dst, num, 4);
              break;
          }
          break;
        case DDS_OP_VAL_BMK: {
          const uint32_t elem_size = DDS_OP_TYPE_SZ(insn);
          dds_is_get_bytes(is, dst, num, elem_size);
          break;
        }
        default:
          abort();
      }
      break;
    }
    case DDS_OP_VAL_SEQ:
    case DDS_OP_VAL_BSQ:
    case DDS_OP_VAL_UNI:
    case DDS_OP_VAL_STU:
      abort();
      break;
    case DDS_OP_VAL_EXT: {
      assert(key_offset_count > 0);
      const uint32_t *jsr_ops = ops + DDS_OP_ADR_JSR(ops[2]) + *key_offset_insn;
      dds_stream_read_key_impl(is, dst, allocator, jsr_ops, --key_offset_count, ++key_offset_insn);
      break;
    }
  }
}

/**
 * @brief 从数据流中读取键值
 *
 * @param[in] is 数据输入流
 * @param[out] sample 存储解析后的数据样本
 * @param[in] allocator 内存分配器
 * @param[in] desc 数据流描述符
 */
void dds_stream_read_key(dds_istream_t *__restrict is,
                         char *__restrict sample,
                         const struct dds_cdrstream_allocator *__restrict allocator,
                         const struct dds_cdrstream_desc *__restrict desc) {
  // 遍历所有键值
  for (uint32_t i = 0; i < desc->keys.nkeys; i++) {
    const uint32_t *op = desc->ops.ops + desc->keys.keys[i].ops_offs;
    switch (DDS_OP(*op)) {
      case DDS_OP_KOF: {
        uint16_t n_offs = DDS_OP_LENGTH(*op);
        dds_stream_read_key_impl(is, sample, allocator, desc->ops.ops + op[1], --n_offs, op + 2);
        break;
      }
      case DDS_OP_ADR: {
        dds_stream_read_key_impl(is, sample, allocator, op, 0, NULL);
        break;
      }
      default:
        abort();
        break;
    }
  }
}

/**
 * @brief
 * 用于在dds_stream_write_key中以本机字节序写入键值，因此在这种情况下不需要交换字节序，此函数为无操作。
 *
 * @param[in] vbuf 指向要处理的缓冲区的指针
 * @param[in] size 缓冲区中每个元素的大小（以字节为单位）
 * @param[in] num 缓冲区中元素的数量
 */
static inline void dds_stream_swap_if_needed_insitu(void *__restrict vbuf,
                                                    uint32_t size,
                                                    uint32_t num) {
  (void)vbuf;
  (void)size;
  (void)num;
}

// 本机字节序
#define NAME_BYTE_ORDER_EXT
#include "dds_cdrstream_keys.part.c"
#undef NAME_BYTE_ORDER_EXT

#if DDSRT_ENDIAN == DDSRT_LITTLE_ENDIAN

/**
 * @brief 在大端字节序环境下，如果需要，交换字节序并将结果存储在原始缓冲区中。
 *
 * @param[in] vbuf 指向要处理的缓冲区的指针
 * @param[in] size 缓冲区中每个元素的大小（以字节为单位）
 * @param[in] num 缓冲区中元素的数量
 */
static void dds_stream_swap_if_needed_insituBE(void *__restrict vbuf, uint32_t size, uint32_t num) {
  dds_stream_swap(vbuf, size, num);
}

// 大端字节序实现
#define NAME_BYTE_ORDER_EXT BE
#include "dds_cdrstream_keys.part.c"
#undef NAME_BYTE_ORDER_EXT

#else /* if DDSRT_ENDIAN == DDSRT_LITTLE_ENDIAN */

/**
 * @brief 在大端字节序环境下，将键值写入输出流。
 *
 * @param[in] os 指向dds_ostreamBE_t结构的指针
 * @param[in] sample 指向要写入的样本数据的指针
 * @param[in] allocator 指向dds_cdrstream_allocator结构的指针
 * @param[in] desc 指向dds_cdrstream_desc结构的指针
 */
void dds_stream_write_keyBE(dds_ostreamBE_t *__restrict os,
                            const char *__restrict sample,
                            const struct dds_cdrstream_allocator *__restrict allocator,
                            const struct dds_cdrstream_desc *__restrict desc) {
  dds_stream_write_key(&os->x, allocator, sample, desc);
}

#endif /* if DDSRT_ENDIAN == DDSRT_LITTLE_ENDIAN */

/*******************************************************************************************
 **
 **  Pretty-printing
 **
 *******************************************************************************************/

/**
 * @brief 返回缓冲区是否未耗尽，如果未耗尽则返回true，否则返回false。
 *
 * @param[in,out] buf      缓冲区指针的地址
 * @param[in,out] bufsize  缓冲区大小的地址
 * @param[in]     fmt      格式化字符串
 * @param[in]     ...      可变参数列表
 * @return 如果缓冲区未耗尽，则返回true，否则返回false
 */
static bool prtf(char *__restrict *buf, size_t *__restrict bufsize, const char *fmt, ...)
    ddsrt_attribute_format_printf(3, 4);

static bool prtf(char *__restrict *buf, size_t *__restrict bufsize, const char *fmt, ...) {
  va_list ap;  // 定义可变参数列表

  if (*bufsize == 0)  // 检查缓冲区大小是否为0
    return false;

  va_start(ap, fmt);                           // 初始化可变参数列表
  int n = vsnprintf(*buf, *bufsize, fmt, ap);  // 将格式化字符串写入缓冲区
  va_end(ap);                                  // 清理可变参数列表

  if (n < 0)  // 检查vsnprintf是否出错
  {
    **buf = 0;
    return false;
  } else if ((size_t)n <= *bufsize)  // 检查写入的字符数是否小于等于缓冲区大小
  {
    *buf += (size_t)n;      // 更新缓冲区指针
    *bufsize -= (size_t)n;  // 更新缓冲区大小
    return (*bufsize > 0);  // 返回缓冲区是否未耗尽
  } else                    // 写入的字符数大于缓冲区大小
  {
    *buf += *bufsize;  // 更新缓冲区指针
    *bufsize = 0;      // 设置缓冲区大小为0
    return false;
  }
}

/**
 * @brief 将字符串写入缓冲区，并返回缓冲区是否未耗尽。
 *
 * @param[in,out] buf      缓冲区指针的地址
 * @param[in,out] bufsize  缓冲区大小的地址
 * @param[in]     is       输入流指针
 * @return 如果缓冲区未耗尽，则返回true，否则返回false
 */
static bool prtf_str(char *__restrict *buf,
                     size_t *__restrict bufsize,
                     dds_istream_t *__restrict is) {
  size_t sz = dds_is_get4(is);                                          // 获取字符串长度
  bool ret = prtf(buf, bufsize, "\"%s\"", is->m_buffer + is->m_index);  // 将字符串写入缓冲区
  is->m_index += (uint32_t)sz;                                          // 更新输入流索引
  return ret;
}

/**
 * @brief 计算给定字符串中可打印字符的连续长度。
 *
 * @param[in] s  字符串指针
 * @param[in] n  要检查的字符数
 * @return 可打印字符的连续长度
 */
static size_t isprint_runlen(const unsigned char *s, size_t n) {
  size_t m;
  for (m = 0; m < n && s[m] != '"' && isprint(s[m]); m++)  // 遍历字符串，计算可打印字符的连续长度
    ;
  return m;
}

/**
 * @brief 以位掩码形式打印枚举值
 *
 * @param[in,out] buf        缓冲区指针，用于存储格式化后的字符串
 * @param[in,out] bufsize    缓冲区大小指针
 * @param[in]     is         输入流指针
 * @param[in]     flags      标志位，用于确定枚举值的大小
 * @return 返回布尔值，表示是否成功打印枚举值
 */
static bool prtf_enum_bitmask(char *__restrict *buf,
                              size_t *__restrict bufsize,
                              dds_istream_t *__restrict is,
                              uint32_t flags) {
  // 根据标志位确定枚举值的大小
  switch (DDS_OP_FLAGS_SZ(flags)) {
    case 1: {
      // 读取一个字节的枚举值
      const uint8_t val = dds_is_get1(is);
      // 将枚举值格式化为字符串并存储到缓冲区中
      return prtf(buf, bufsize, "%" PRIu8, val);
    }
    case 2: {
      // 读取两个字节的枚举值
      const uint16_t val = dds_is_get2(is);
      // 将枚举值格式化为字符串并存储到缓冲区中
      return prtf(buf, bufsize, "%" PRIu16, val);
    }
    case 4: {
      // 读取四个字节的枚举值
      const uint32_t val = dds_is_get4(is);
      // 将枚举值格式化为字符串并存储到缓冲区中
      return prtf(buf, bufsize, "%" PRIu32, val);
    }
    case 8: {
      // 读取八个字节的枚举值
      const uint64_t val = dds_is_get8(is);
      // 将枚举值格式化为字符串并存储到缓冲区中
      return prtf(buf, bufsize, "%" PRIu64, val);
    }
    default:
      // 如果标志位不是预期的值，则终止程序
      abort();
  }
  // 返回 false，表示未能成功打印枚举值
  return false;
}

/**
 * @brief 以简单的格式打印数据
 * @param[out] buf 输出缓冲区指针
 * @param[in,out] bufsize 缓冲区大小指针
 * @param[in] is 输入流指针
 * @param[in] type 数据类型枚举值
 * @param[in] flags 格式化标志位
 * @return 成功返回true，失败返回false
 */
static bool prtf_simple(char *__restrict *buf,
                        size_t *__restrict bufsize,
                        dds_istream_t *__restrict is,
                        enum dds_stream_typecode type,
                        uint32_t flags) {
  // 根据数据类型进行处理
  switch (type) {
    case DDS_OP_VAL_BLN:  // 布尔类型
    {
      const bool x = dds_is_get1(is);  // 从输入流中读取一个字节作为布尔值
      return prtf(buf, bufsize, "%s", x ? "true" : "false");  // 格式化输出布尔值
    }
    case DDS_OP_VAL_1BY:  // 1字节整数类型
    {
      const union {
        int8_t s;                    // 有符号整数
        uint8_t u;                   // 无符号整数
      } x = {.u = dds_is_get1(is)};  // 从输入流中读取一个字节作为整数值

      if (flags & DDS_OP_FLAG_SGN)  // 判断是否为有符号整数
        return prtf(buf, bufsize, "%" PRId8, x.s);
      else
        return prtf(buf, bufsize, "%" PRIu8, x.u);
    }
    case DDS_OP_VAL_2BY:  // 2字节整数类型
    {
      const union {
        int16_t s;                   // 有符号整数
        uint16_t u;                  // 无符号整数
      } x = {.u = dds_is_get2(is)};  // 从输入流中读取两个字节作为整数值

      if (flags & DDS_OP_FLAG_SGN)  // 判断是否为有符号整数
        return prtf(buf, bufsize, "%" PRId16, x.s);
      else
        return prtf(buf, bufsize, "%" PRIu16, x.u);
    }
    case DDS_OP_VAL_4BY:  // 4字节整数或浮点类型
    {
      const union {
        int32_t s;                   // 有符号整数
        uint32_t u;                  // 无符号整数
        float f;                     // 单精度浮点数
      } x = {.u = dds_is_get4(is)};  // 从输入流中读取四个字节作为整数或浮点值

      if (flags & DDS_OP_FLAG_FP)  // 判断是否为浮点数
        return prtf(buf, bufsize, "%g", x.f);
      else if (flags & DDS_OP_FLAG_SGN)  // 判断是否为有符号整数
        return prtf(buf, bufsize, "%" PRId32, x.s);
      else
        return prtf(buf, bufsize, "%" PRIu32, x.u);
    }
    case DDS_OP_VAL_8BY:  // 8字节整数或双精度浮点类型
    {
      const union {
        int64_t s;                   // 有符号整数
        uint64_t u;                  // 无符号整数
        double f;                    // 双精度浮点数
      } x = {.u = dds_is_get8(is)};  // 从输入流中读取八个字节作为整数或双精度浮点值

      if (flags & DDS_OP_FLAG_FP)  // 判断是否为双精度浮点数
        return prtf(buf, bufsize, "%g", x.f);
      else if (flags & DDS_OP_FLAG_SGN)  // 判断是否为有符号整数
        return prtf(buf, bufsize, "%" PRId64, x.s);
      else
        return prtf(buf, bufsize, "%" PRIu64, x.u);
    }
    case DDS_OP_VAL_ENU:                                  // 枚举类型
    case DDS_OP_VAL_BMK:                                  // 位掩码类型
      return prtf_enum_bitmask(buf, bufsize, is, flags);  // 调用专门的枚举和位掩码处理函数
    case DDS_OP_VAL_STR:                                  // 字符串类型
    case DDS_OP_VAL_BST:                                  // 定长字符串类型
      return prtf_str(buf, bufsize, is);                  // 调用专门的字符串处理函数
    case DDS_OP_VAL_ARR:                                  // 数组类型
    case DDS_OP_VAL_SEQ:                                  // 序列类型
    case DDS_OP_VAL_BSQ:                                  // 定长序列类型
    case DDS_OP_VAL_UNI:                                  // 联合类型
    case DDS_OP_VAL_STU:                                  // 结构体类型
    case DDS_OP_VAL_EXT:                                  // 扩展类型
      abort();                                            // 不支持的类型，直接终止程序
  }
  return false;
}

/**
 * @brief 以简单数组的形式打印数据
 *
 * @param buf        输出缓冲区指针
 * @param bufsize    输出缓冲区大小指针
 * @param is         输入流指针
 * @param num        数组元素数量
 * @param type       数据类型枚举值
 * @param flags      标志位
 * @return bool      返回是否成功继续执行
 */
static bool prtf_simple_array(char *__restrict *buf,
                              size_t *__restrict bufsize,
                              dds_istream_t *__restrict is,
                              uint32_t num,
                              enum dds_stream_typecode type,
                              uint32_t flags) {
  // 初始化并输出左大括号
  bool cont = prtf(buf, bufsize, "{");

  // 根据数据类型进行处理
  switch (type) {
    case DDS_OP_VAL_1BY: {
      size_t i = 0, j;
      // 遍历数组元素
      while (cont && i < num) {
        // 计算可打印字符长度
        size_t m = isprint_runlen((unsigned char *)(is->m_buffer + is->m_index), num - i);
        if (m >= 4) {
          // 输出逗号和双引号
          cont = prtf(buf, bufsize, "%s\"", i != 0 ? "," : "");
          // 输出字符
          for (j = 0; cont && j < m; j++)
            cont = prtf(buf, bufsize, "%c", is->m_buffer[is->m_index + j]);
          // 输出双引号
          cont = prtf(buf, bufsize, "\"");
          // 更新索引和计数器
          is->m_index += (uint32_t)m;
          i += m;
        } else {
          // 输出逗号
          if (i != 0) (void)prtf(buf, bufsize, ",");
          // 打印简单类型数据
          cont = prtf_simple(buf, bufsize, is, type, flags);
          // 计数器自增
          i++;
        }
      }
      break;
    }
    case DDS_OP_VAL_ENU:
    case DDS_OP_VAL_BMK:
      for (size_t i = 0; cont && i < num; i++) {
        // 输出逗号
        if (i != 0) (void)prtf(buf, bufsize, ",");
        // 打印枚举或位掩码类型数据
        cont = prtf_enum_bitmask(buf, bufsize, is, flags);
      }
      break;
    case DDS_OP_VAL_BLN:
    case DDS_OP_VAL_2BY:
    case DDS_OP_VAL_4BY:
    case DDS_OP_VAL_8BY:
    case DDS_OP_VAL_STR:
    case DDS_OP_VAL_BST:
      for (size_t i = 0; cont && i < num; i++) {
        // 输出逗号
        if (i != 0) (void)prtf(buf, bufsize, ",");
        // 打印简单类型数据
        cont = prtf_simple(buf, bufsize, is, type, flags);
      }
      break;
    default:
      // 遇到未知类型，终止程序
      abort();
      break;
  }
  // 输出右大括号并返回结果
  return prtf(buf, bufsize, "}");
}

/**
 * @brief 打印序列数据的函数
 *
 * @param[in,out] buf          缓冲区指针，用于存储格式化后的字符串
 * @param[in,out] bufsize      缓冲区大小指针
 * @param[in]     is           输入流指针
 * @param[in]     ops          操作码数组指针
 * @param[in]     insn         当前操作码
 * @return 返回处理后的操作码数组指针
 */
static const uint32_t *prtf_seq(char *__restrict *buf,
                                size_t *bufsize,
                                dds_istream_t *__restrict is,
                                const uint32_t *__restrict ops,
                                uint32_t insn) {
  // 获取子类型
  const enum dds_stream_typecode subtype = DDS_OP_SUBTYPE(insn);
  // 判断序列是否有边界
  uint32_t bound_op = seq_is_bounded(DDS_OP_TYPE(insn)) ? 1 : 0;
  // 如果需要数据头，则获取4字节数据头
  if (is_dheader_needed(subtype, is->m_xcdr_version)) (void)dds_is_get4(is);

  // 获取序列元素数量
  const uint32_t num = dds_is_get4(is);
  // 如果元素数量为0，直接打印空括号，并跳过序列指令
  if (num == 0) {
    (void)prtf(buf, bufsize, "{}");
    return skip_sequence_insns(insn, ops);
  }
  // 根据子类型进行处理
  switch (subtype) {
    case DDS_OP_VAL_BLN:
    case DDS_OP_VAL_1BY:
    case DDS_OP_VAL_2BY:
    case DDS_OP_VAL_4BY:
    case DDS_OP_VAL_8BY:
      (void)prtf_simple_array(buf, bufsize, is, num, subtype, DDS_OP_FLAGS(insn));
      return ops + 2 + bound_op;
    case DDS_OP_VAL_STR:
    case DDS_OP_VAL_BST:
    case DDS_OP_VAL_ENU:
    case DDS_OP_VAL_BMK: {
      (void)prtf_simple_array(buf, bufsize, is, num, subtype, DDS_OP_FLAGS(insn));
      const uint32_t *ret_ops = ops + 2 + bound_op;
      if (subtype == DDS_OP_VAL_BMK)
        ret_ops += 2;
      else if (subtype == DDS_OP_VAL_BST || subtype == DDS_OP_VAL_ENU)
        ret_ops++;
      return ret_ops;
    }
    case DDS_OP_VAL_SEQ:
    case DDS_OP_VAL_BSQ:
    case DDS_OP_VAL_ARR:
    case DDS_OP_VAL_UNI:
    case DDS_OP_VAL_STU: {
      const uint32_t jmp = DDS_OP_ADR_JMP(ops[3 + bound_op]);
      uint32_t const *const jsr_ops = ops + DDS_OP_ADR_JSR(ops[3 + bound_op]);
      bool cont = prtf(buf, bufsize, "{");
      for (uint32_t i = 0; cont && i < num; i++) {
        if (i > 0) (void)prtf(buf, bufsize, ",");
        cont = dds_stream_print_sample1(buf, bufsize, is, jsr_ops, subtype == DDS_OP_VAL_STU,
                                        false) != NULL;
      }
      (void)prtf(buf, bufsize, "}");
      return ops + (jmp ? jmp : (4 + bound_op)); /* FIXME: why would jmp be 0? */
    }
    case DDS_OP_VAL_EXT: {
      abort(); /* not supported */
      break;
    }
  }
  return NULL;
}

/**
 * @brief 打印数组并返回操作指针
 *
 * @param[in] buf        缓冲区指针的指针，用于存储打印结果
 * @param[in] bufsize    缓冲区大小的指针
 * @param[in] is         dds_istream_t类型的指针，用于读取数据流
 * @param[in] ops        操作指针，用于指示处理过程中的操作
 * @param[in] insn       32位无符号整数，表示指令
 * @return const uint32_t* 返回操作指针
 */
static const uint32_t *prtf_arr(char *__restrict *buf,
                                size_t *bufsize,
                                dds_istream_t *__restrict is,
                                const uint32_t *__restrict ops,
                                uint32_t insn) {
  // 获取子类型
  const enum dds_stream_typecode subtype = DDS_OP_SUBTYPE(insn);

  // 如果需要dheader，则获取4字节数据
  if (is_dheader_needed(subtype, is->m_xcdr_version)) (void)dds_is_get4(is);

  // 获取操作数
  const uint32_t num = ops[2];

  // 根据子类型进行处理
  switch (subtype) {
    case DDS_OP_VAL_BLN:
    case DDS_OP_VAL_1BY:
    case DDS_OP_VAL_2BY:
    case DDS_OP_VAL_4BY:
    case DDS_OP_VAL_8BY:
    case DDS_OP_VAL_STR:
    case DDS_OP_VAL_BST:
    case DDS_OP_VAL_ENU:
    case DDS_OP_VAL_BMK: {
      // 处理简单数组
      (void)prtf_simple_array(buf, bufsize, is, num, subtype, DDS_OP_FLAGS(insn));

      // 计算返回操作指针
      const uint32_t *ret_ops = ops + 3;
      if (subtype == DDS_OP_VAL_BST || subtype == DDS_OP_VAL_BMK)
        ret_ops += 2;
      else if (subtype == DDS_OP_VAL_ENU)
        ret_ops++;

      return ret_ops;
    }
    case DDS_OP_VAL_SEQ:
    case DDS_OP_VAL_BSQ:
    case DDS_OP_VAL_ARR:
    case DDS_OP_VAL_UNI:
    case DDS_OP_VAL_STU: {
      // 获取jsr操作指针
      const uint32_t *jsr_ops = ops + DDS_OP_ADR_JSR(ops[3]);

      // 获取跳转地址
      const uint32_t jmp = DDS_OP_ADR_JMP(ops[3]);

      // 初始化循环控制变量
      bool cont = prtf(buf, bufsize, "{");

      // 循环处理数组元素
      for (uint32_t i = 0; cont && i < num; i++) {
        // 如果不是第一个元素，添加逗号分隔符
        if (i > 0) (void)prtf(buf, bufsize, ",");

        // 打印样本并更新循环控制变量
        cont = dds_stream_print_sample1(buf, bufsize, is, jsr_ops, subtype == DDS_OP_VAL_STU,
                                        false) != NULL;
      }

      // 添加结束括号
      (void)prtf(buf, bufsize, "}");

      // 返回操作指针
      return ops + (jmp ? jmp : 5);
    }
    case DDS_OP_VAL_EXT: {
      // 不支持的操作，终止程序
      abort();
      break;
    }
  }

  // 返回空指针
  return NULL;
}

/**
 * @brief 读取并打印DDS序列化数据中的联合体（union）类型数据
 *
 * @param[in,out] buf        缓冲区指针，用于存储格式化后的字符串
 * @param[in,out] bufsize    缓冲区大小
 * @param[in]     is         输入流，包含DDS序列化数据
 * @param[in]     ops        操作码数组，描述了如何解析输入流中的数据
 * @param[in]     insn       当前操作码
 * @return const uint32_t*   返回下一个操作码的地址
 */
static const uint32_t *prtf_uni(char *__restrict *buf,
                                size_t *bufsize,
                                dds_istream_t *__restrict is,
                                const uint32_t *__restrict ops,
                                uint32_t insn) {
  // 读取联合体的判别式值
  const uint32_t disc = read_union_discriminant(is, insn);

  // 查找与判别式值匹配的联合体分支
  uint32_t const *const jeq_op = find_union_case(ops, disc);

  // 将判别式值写入缓冲区
  (void)prtf(buf, bufsize, "%" PRIu32 ":", disc);

  // 跳转到下一个操作码
  ops += DDS_OP_ADR_JMP(ops[3]);

  // 如果找到了匹配的联合体分支
  if (jeq_op) {
    // 获取该分支的值类型
    const enum dds_stream_typecode valtype = DDS_JEQ_TYPE(jeq_op[0]);

    // 根据值类型进行处理
    switch (valtype) {
      case DDS_OP_VAL_BLN:
      case DDS_OP_VAL_1BY:
      case DDS_OP_VAL_2BY:
      case DDS_OP_VAL_4BY:
      case DDS_OP_VAL_8BY:
      case DDS_OP_VAL_ENU:
      case DDS_OP_VAL_STR:
      case DDS_OP_VAL_BST:
        // 简单类型，直接打印
        (void)prtf_simple(buf, bufsize, is, valtype, DDS_OP_FLAGS(jeq_op[0]));
        break;
      case DDS_OP_VAL_SEQ:
      case DDS_OP_VAL_BSQ:
      case DDS_OP_VAL_ARR:
      case DDS_OP_VAL_UNI:
      case DDS_OP_VAL_STU:
      case DDS_OP_VAL_BMK:
        // 复杂类型，递归调用打印函数
        (void)dds_stream_print_sample1(buf, bufsize, is, jeq_op + DDS_OP_ADR_JSR(jeq_op[0]),
                                       valtype == DDS_OP_VAL_STU, false);
        break;
      case DDS_OP_VAL_EXT:
        // 不支持的类型，终止程序
        abort(); /* not supported, use UNI instead */
        break;
    }
  }

  // 返回下一个操作码的地址
  return ops;
}

/**
 * @brief 打印数据流中的地址信息
 *
 * @param[in,out] buf          缓冲区指针，用于存储打印结果
 * @param[in,out] bufsize      缓冲区大小
 * @param[in]     insn         指令
 * @param[in]     is           数据流输入
 * @param[in]     ops          操作列表
 * @param[in]     is_mutable_member 是否为可变成员
 * @return 返回操作列表的下一个位置
 */
static const uint32_t *dds_stream_print_adr(char *__restrict *buf,
                                            size_t *__restrict bufsize,
                                            uint32_t insn,
                                            dds_istream_t *__restrict is,
                                            const uint32_t *__restrict ops,
                                            bool is_mutable_member) {
  // 检查成员是否存在
  if (!stream_is_member_present(insn, is, is_mutable_member)) {
    (void)prtf(buf, bufsize, "NULL");
    return dds_stream_skip_adr(insn, ops);
  }
  // 根据指令类型进行处理
  switch (DDS_OP_TYPE(insn)) {
    case DDS_OP_VAL_BLN:
    case DDS_OP_VAL_1BY:
    case DDS_OP_VAL_2BY:
    case DDS_OP_VAL_4BY:
    case DDS_OP_VAL_8BY:
    case DDS_OP_VAL_STR:
      if (!prtf_simple(buf, bufsize, is, DDS_OP_TYPE(insn), DDS_OP_FLAGS(insn))) return NULL;
      ops += 2;
      break;
    case DDS_OP_VAL_BST:
    case DDS_OP_VAL_ENU:
    case DDS_OP_VAL_BMK:
      if (!prtf_simple(buf, bufsize, is, DDS_OP_TYPE(insn), DDS_OP_FLAGS(insn))) return NULL;
      ops += 3 + (DDS_OP_TYPE(insn) == DDS_OP_VAL_BMK ? 1 : 0);
      break;
    case DDS_OP_VAL_SEQ:
    case DDS_OP_VAL_BSQ:
      ops = prtf_seq(buf, bufsize, is, ops, insn);
      break;
    case DDS_OP_VAL_ARR:
      ops = prtf_arr(buf, bufsize, is, ops, insn);
      break;
    case DDS_OP_VAL_UNI:
      ops = prtf_uni(buf, bufsize, is, ops, insn);
      break;
    case DDS_OP_VAL_EXT: {
      const uint32_t *jsr_ops = ops + DDS_OP_ADR_JSR(ops[2]);
      const uint32_t jmp = DDS_OP_ADR_JMP(ops[2]);
      // 跳过基本类型的DLC指令，因为基本类型的数据中没有DHEADER
      if (op_type_base(insn) && jsr_ops[0] == DDS_OP_DLC) jsr_ops++;
      if (dds_stream_print_sample1(buf, bufsize, is, jsr_ops, true, false) == NULL) return NULL;
      ops += jmp ? jmp : 3;
      break;
    }
    case DDS_OP_VAL_STU:
      abort();  // op type STU 只支持作为子类型
      break;
  }
  return ops;
}

/**
 * @brief 以支持 doxygen 的形式为函数添加参数列表的说明，并逐行添加详细的中文注释。
 *
 * @param[out] buf 输出缓冲区指针
 * @param[in,out] bufsize 缓冲区大小指针
 * @param[in] is 输入流指针
 * @param[in] ops 操作码数组指针
 * @return 返回操作码数组指针，如果失败则返回 NULL
 */
static const uint32_t *prtf_delimited(char *__restrict *buf,
                                      size_t *bufsize,
                                      dds_istream_t *__restrict is,
                                      const uint32_t *__restrict ops) {
  // 获取有界数据大小
  uint32_t delimited_sz = dds_is_get4(is), delimited_offs = is->m_index, insn;
  bool needs_comma = false;

  // 打印有界数据大小
  if (!prtf(buf, bufsize, "dlh:%" PRIu32, delimited_sz)) return NULL;

  // 跳过第一个操作码
  ops++;

  // 遍历操作码，直到遇到 DDS_OP_RTS
  while ((insn = *ops) != DDS_OP_RTS) {
    // 如果需要逗号，打印逗号
    if (needs_comma) (void)prtf(buf, bufsize, ",");
    needs_comma = true;

    // 根据操作码执行相应操作
    switch (DDS_OP(insn)) {
      case DDS_OP_ADR:
        // 跳过可追加类型中未序列化的字段
        if ((ops = (is->m_index - delimited_offs < delimited_sz)
                       ? dds_stream_print_adr(buf, bufsize, insn, is, ops, false)
                       : dds_stream_skip_adr(insn, ops)) == NULL)
          return NULL;
        break;
      case DDS_OP_JSR:
        // 打印样本
        if (dds_stream_print_sample1(buf, bufsize, is, ops + DDS_OP_JUMP(insn), false, false) ==
            NULL)
          return NULL;
        ops++;
        break;
      case DDS_OP_RTS:
      case DDS_OP_JEQ:
      case DDS_OP_JEQ4:
      case DDS_OP_KOF:
      case DDS_OP_DLC:
      case DDS_OP_PLC:
      case DDS_OP_PLM: {
        // 遇到不支持的操作码，终止程序
        abort();
        break;
      }
    }
  }

  // 跳过此可追加类型的序列化数据剩余部分
  if (delimited_sz > is->m_index - delimited_offs)
    is->m_index += delimited_sz - (is->m_index - delimited_offs);

  return ops;
}

/**
 * @brief 以支持 doxygen 的形式为函数添加参数列表的说明，并逐行添加详细的中文注释
 *
 * @param[in,out] buf      缓冲区指针，用于存储输出结果
 * @param[in,out] bufsize  缓冲区大小指针
 * @param[in]     is       输入流指针
 * @param[in]     m_id     成员 ID
 * @param[in]     ops      操作数组指针
 * @return bool            如果找到匹配的成员 ID，则返回 true，否则返回 false
 */
static bool prtf_plm(char *__restrict *buf,
                     size_t *bufsize,
                     dds_istream_t *__restrict is,
                     uint32_t m_id,
                     const uint32_t *__restrict ops) {
  // 定义局部变量 insn 和 ops_csr，初始化 found 为 false
  uint32_t insn, ops_csr = 0;
  bool found = false;

  // 当没有找到匹配的成员 ID 且操作不是 DDS_OP_RTS 时，循环执行
  while (!found && (insn = ops[ops_csr]) != DDS_OP_RTS) {
    // 断言操作码为 DDS_OP_PLM
    assert(DDS_OP(insn) == DDS_OP_PLM);

    // 获取标志位
    uint32_t flags = DDS_PLM_FLAGS(insn);

    // 计算 plm_ops 的地址
    const uint32_t *plm_ops = ops + ops_csr + DDS_OP_ADR_PLM(insn);

    // 如果标志位包含 DDS_OP_FLAG_BASE
    if (flags & DDS_OP_FLAG_BASE) {
      // 断言第一个操作码为 DDS_OP_PLC
      assert(DDS_OP(plm_ops[0]) == DDS_OP_PLC);

      // 跳过 PLC，指向基类型的第一个 PLM
      plm_ops++;

      // 递归调用 prtf_plm 函数
      found = prtf_plm(buf, bufsize, is, m_id, plm_ops);
    }
    // 如果下一个操作码等于成员 ID
    else if (ops[ops_csr + 1] == m_id) {
      // 打印样本
      (void)dds_stream_print_sample1(buf, bufsize, is, plm_ops, true, true);

      // 设置 found 为 true 并跳出循环
      found = true;
      break;
    }

    // 更新 ops_csr
    ops_csr += 2;
  }

  // 返回 found
  return found;
}

/**
 * @brief 解析并打印PL（Parameter List）数据
 *
 * @param[in,out] buf      缓冲区指针，用于存储格式化的输出字符串
 * @param[in,out] bufsize  缓冲区大小
 * @param[in]     is       输入流对象，包含待解析的序列化数据
 * @param[in]     ops      操作码数组，描述了如何解析输入流中的数据
 * @return 返回操作码数组的下一个位置
 */
static const uint32_t *prtf_pl(char *__restrict *buf,
                               size_t *bufsize,
                               dds_istream_t *__restrict is,
                               const uint32_t *__restrict ops) {
  /* 跳过PLC操作码 */
  ops++;

  // 获取PL的大小和偏移量
  uint32_t pl_sz = dds_is_get4(is), pl_offs = is->m_index;
  if (!prtf(buf, bufsize, "pl:%" PRIu32, pl_sz)) return NULL;

  // 遍历输入流中的所有成员
  while (is->m_index - pl_offs < pl_sz) {
    /* 读取emheader和next_int */
    uint32_t em_hdr = dds_is_get4(is);
    uint32_t lc = EMHEADER_LENGTH_CODE(em_hdr), m_id = EMHEADER_MEMBERID(em_hdr), msz;
    if (!prtf(buf, bufsize, ",lc:%" PRIu32 ",m:%" PRIu32 ",", lc, m_id)) return NULL;
    switch (lc) {
      case LENGTH_CODE_1B:
      case LENGTH_CODE_2B:
      case LENGTH_CODE_4B:
      case LENGTH_CODE_8B:
        msz = 1u << lc;
        break;
      case LENGTH_CODE_NEXTINT:
        msz = dds_is_get4(is); /* next-int */
        break;
      case LENGTH_CODE_ALSO_NEXTINT:
      case LENGTH_CODE_ALSO_NEXTINT4:
      case LENGTH_CODE_ALSO_NEXTINT8:
        msz = dds_is_peek4(is); /* 长度是序列化数据的一部分 */
        if (lc > LENGTH_CODE_ALSO_NEXTINT) msz <<= (lc - 4);
        break;
      default:
        abort();
        break;
    }

    /* 查找成员并反序列化 */
    if (!prtf_plm(buf, bufsize, is, m_id, ops)) {
      is->m_index += msz;
      if (lc >= LENGTH_CODE_ALSO_NEXTINT)
        is->m_index += 4; /* 成员中嵌入的长度不包括它自己的4个字节 */
    }
  }

  /* 跳过所有PLM-memberid对 */
  while (ops[0] != DDS_OP_RTS) ops += 2;

  return ops;
}

/**
 * @brief 以支持 doxygen 的形式为函数添加参数列表的说明，并逐行添加详细的中文注释
 *
 * @param[in,out] buf          缓冲区指针，用于存储格式化后的输出字符串
 * @param[in,out] bufsize      缓冲区大小指针
 * @param[in]     is           输入流指针
 * @param[in]     ops          操作码数组指针
 * @param[in]     add_braces   是否在输出字符串前后添加大括号
 * @param[in]     is_mutable_member 是否可变成员
 * @return const uint32_t*     返回操作码数组指针
 */
static const uint32_t *dds_stream_print_sample1(char *__restrict *buf,
                                                size_t *__restrict bufsize,
                                                dds_istream_t *__restrict is,
                                                const uint32_t *__restrict ops,
                                                bool add_braces,
                                                bool is_mutable_member) {
  uint32_t insn;             // 定义操作码变量
  bool cont = true;          // 定义循环控制变量
  bool needs_comma = false;  // 定义是否需要逗号分隔符的变量

  if (add_braces)                   // 如果需要添加大括号
    (void)prtf(buf, bufsize, "{");  // 在输出字符串前添加大括号

  while (
      ops && cont &&
      (insn = *ops) !=
          DDS_OP_RTS)  // 当操作码数组存在且循环控制变量为真且操作码不等于 DDS_OP_RTS 时，执行循环
  {
    if (needs_comma)                  // 如果需要逗号分隔符
      (void)prtf(buf, bufsize, ",");  // 在输出字符串中添加逗号分隔符

    needs_comma = true;  // 设置需要逗号分隔符的变量为真

    switch (DDS_OP(insn))  // 根据操作码进行相应处理
    {
      case DDS_OP_ADR:
        ops =
            dds_stream_print_adr(buf, bufsize, insn, is, ops, is_mutable_member);  // 处理 ADR 操作
        break;
      case DDS_OP_JSR:
        cont = dds_stream_print_sample1(buf, bufsize, is, ops + DDS_OP_JUMP(insn), true,
                                        is_mutable_member) !=
               NULL;  // 递归调用 dds_stream_print_sample1 函数处理 JSR 操作
        ops++;        // 操作码数组指针自增
        break;
      case DDS_OP_RTS:
      case DDS_OP_JEQ:
      case DDS_OP_JEQ4:
      case DDS_OP_KOF:
      case DDS_OP_PLM:
        abort();  // 遇到以上操作码时，终止程序
        break;
      case DDS_OP_DLC:
        assert(is->m_xcdr_version ==
               DDSI_RTPS_CDR_ENC_VERSION_2);  // 断言输入流的版本为 DDSI_RTPS_CDR_ENC_VERSION_2
        ops = prtf_delimited(buf, bufsize, is, ops);  // 处理 DLC 操作
        break;
      case DDS_OP_PLC:
        assert(is->m_xcdr_version ==
               DDSI_RTPS_CDR_ENC_VERSION_2);  // 断言输入流的版本为 DDSI_RTPS_CDR_ENC_VERSION_2
        ops = prtf_pl(buf, bufsize, is, ops);  // 处理 PLC 操作
        break;
    }
  }

  if (add_braces)                   // 如果需要添加大括号
    (void)prtf(buf, bufsize, "}");  // 在输出字符串后添加大括号

  return ops;  // 返回操作码数组指针
}

/**
 * @brief 打印DDS流中的样本
 *
 * @param[in] is         输入流指针
 * @param[in] desc       DDS流描述符指针
 * @param[out] buf       输出缓冲区指针
 * @param[in] bufsize    缓冲区大小
 * @return size_t        返回缓冲区大小
 */
size_t dds_stream_print_sample(dds_istream_t *__restrict is,
                               const struct dds_cdrstream_desc *__restrict desc,
                               char *__restrict buf,
                               size_t bufsize) {
  // 调用dds_stream_print_sample1函数处理输入流，并将结果写入缓冲区
  (void)dds_stream_print_sample1(&buf, &bufsize, is, desc->ops.ops, true, false);
  return bufsize;
}

/**
 * @brief 实现DDS流键值打印
 *
 * @param[in] is                 输入流指针
 * @param[in] ops                操作码数组指针
 * @param[in] key_offset_count   键偏移计数
 * @param[in] key_offset_insn    键偏移指令指针
 * @param[out] buf               输出缓冲区指针
 * @param[out] bufsize           缓冲区大小指针
 * @param[out] cont              控制标志指针
 */
static void dds_stream_print_key_impl(dds_istream_t *__restrict is,
                                      const uint32_t *ops,
                                      uint16_t key_offset_count,
                                      const uint32_t *key_offset_insn,
                                      char *__restrict *buf,
                                      size_t *__restrict bufsize,
                                      bool *cont) {
  // 获取操作码
  uint32_t insn = *ops;
  // 检查操作码是否有效
  assert(insn_key_ok_p(insn));
  // 检查控制标志指针是否有效
  assert(cont);

  // 根据操作码类型进行处理
  switch (DDS_OP_TYPE(insn)) {
    case DDS_OP_VAL_BLN:
    case DDS_OP_VAL_1BY:
    case DDS_OP_VAL_2BY:
    case DDS_OP_VAL_4BY:
    case DDS_OP_VAL_8BY:
    case DDS_OP_VAL_ENU:
    case DDS_OP_VAL_STR:
    case DDS_OP_VAL_BST:
    case DDS_OP_VAL_BMK:
      // 处理简单类型的值
      *cont = prtf_simple(buf, bufsize, is, DDS_OP_TYPE(insn), DDS_OP_FLAGS(insn));
      break;
    case DDS_OP_VAL_ARR:
      // 处理数组类型的值
      *cont = prtf_arr(buf, bufsize, is, ops, insn);
      break;
    case DDS_OP_VAL_SEQ:
    case DDS_OP_VAL_BSQ:
    case DDS_OP_VAL_UNI:
    case DDS_OP_VAL_STU:
      // 不支持的类型，终止程序
      abort();
      break;
    case DDS_OP_VAL_EXT:
      // 处理扩展类型的值
      assert(key_offset_count > 0);
      const uint32_t *jsr_ops = ops + DDS_OP_ADR_JSR(ops[2]) + *key_offset_insn;
      dds_stream_print_key_impl(is, jsr_ops, --key_offset_count, ++key_offset_insn, buf, bufsize,
                                cont);
      break;
  }
}

/**
 * @brief 打印数据流中的键值
 *
 * @param[in] is        输入数据流
 * @param[in] desc      数据流描述符
 * @param[out] buf      用于存储键值的缓冲区
 * @param[in] bufsize   缓冲区大小
 * @return size_t       返回已使用的缓冲区大小
 */
size_t dds_stream_print_key(dds_istream_t *__restrict is,
                            const struct dds_cdrstream_desc *__restrict desc,
                            char *__restrict buf,
                            size_t bufsize) {
  // 初始化控制变量
  bool cont = prtf(&buf, &bufsize, ":k:{");
  bool needs_comma = false;

  // 遍历键值列表
  for (uint32_t i = 0; cont && i < desc->keys.nkeys; i++) {
    // 如果需要添加逗号分隔符
    if (needs_comma) (void)prtf(&buf, &bufsize, ",");

    // 设置下一个元素需要逗号分隔符
    needs_comma = true;

    // 获取操作指针
    const uint32_t *op = desc->ops.ops + desc->keys.keys[i].ops_offs;

    // 根据操作类型进行处理
    switch (DDS_OP(*op)) {
      case DDS_OP_KOF: {
        // 获取操作数长度
        uint16_t n_offs = DDS_OP_LENGTH(*op);

        // 调用打印键值实现函数
        dds_stream_print_key_impl(is, desc->ops.ops + op[1], --n_offs, op + 2, &buf, &bufsize,
                                  &cont);
        break;
      }
      case DDS_OP_ADR: {
        // 调用打印键值实现函数
        dds_stream_print_key_impl(is, op, 0, NULL, &buf, &bufsize, &cont);
        break;
      }
      default:
        // 遇到未知操作类型，终止程序
        abort();
        break;
    }
  }

  // 添加结束符
  (void)prtf(&buf, &bufsize, "}");

  // 返回已使用的缓冲区大小
  return bufsize;
}

/**
 * @brief
 * 获取用于此主题的类型的（最小）可扩展性，并返回所需的XCDR版本以便为此主题描述符（反）序列化类型。
 *
 * @param[in] __restrict ops 指向操作数组的指针。
 * @return 返回所需的XCDR版本。
 */
uint16_t dds_stream_minimum_xcdr_version(const uint32_t *__restrict ops) {
  // 初始化最小XCDR版本为1
  uint16_t min_xcdrv = DDSI_RTPS_CDR_ENC_VERSION_1;
  const uint32_t *ops_end = ops;

  // 计算操作数并更新最小XCDR版本
  dds_stream_countops1(ops, &ops_end, &min_xcdrv, 0, NULL);

  // 返回最小XCDR版本
  return min_xcdrv;
}

/**
 * @brief 通过检查序列化操作，获取主题的顶层类型的可扩展性。
 *
 * @param[in] __restrict ops 指向操作数组的指针。
 * @param[out] ext 存储dds_cdr_type_extensibility枚举值的指针。
 * @return 如果找到可扩展性，则返回true，否则返回false。
 */
bool dds_stream_extensibility(const uint32_t *__restrict ops,
                              enum dds_cdr_type_extensibility *ext) {
  uint32_t insn;

  // 断言ext非空
  assert(ext);

  // 遍历操作数组
  while ((insn = *ops) != DDS_OP_RTS) {
    switch (DDS_OP(insn)) {
      case DDS_OP_ADR:
        *ext = DDS_CDR_TYPE_EXT_FINAL;
        return true;
      case DDS_OP_JSR:
        if (DDS_OP_JUMP(insn) > 0) return dds_stream_extensibility(ops + DDS_OP_JUMP(insn), ext);
        break;
      case DDS_OP_DLC:
        *ext = DDS_CDR_TYPE_EXT_APPENDABLE;
        return true;
      case DDS_OP_PLC:
        *ext = DDS_CDR_TYPE_EXT_MUTABLE;
        return true;
      case DDS_OP_RTS:
      case DDS_OP_JEQ:
      case DDS_OP_JEQ4:
      case DDS_OP_KOF:
      case DDS_OP_PLM:
        abort();
        break;
    }
  }
  return false;
}

/**
 * @brief 获取类型嵌套深度。
 *
 * @param[in] __restrict ops 指向操作数组的指针。
 * @return 返回类型嵌套深度。
 */
uint32_t dds_stream_type_nesting_depth(const uint32_t *__restrict ops) {
  uint32_t nesting_depth = 0;
  const uint32_t *ops_end = ops;

  // 计算操作数并更新嵌套深度
  dds_stream_countops1(ops, &ops_end, NULL, 0, &nesting_depth);

  // 返回嵌套深度
  return nesting_depth;
}

/**
 * @brief 释放dds_cdrstream_desc结构的资源。
 *
 * @param[in,out] desc 指向dds_cdrstream_desc结构的指针。
 * @param[in] __restrict allocator 指向dds_cdrstream_allocator结构的指针。
 */
void dds_cdrstream_desc_fini(struct dds_cdrstream_desc *desc,
                             const struct dds_cdrstream_allocator *__restrict allocator) {
  // 如果有键值，则释放键值数组
  if (desc->keys.nkeys > 0 && desc->keys.keys != NULL) allocator->free(desc->keys.keys);

  // 释放操作数组
  allocator->free(desc->ops.ops);
}
