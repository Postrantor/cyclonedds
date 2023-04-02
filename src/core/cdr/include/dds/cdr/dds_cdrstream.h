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
#ifndef DDS_CDRSTREAM_H
#define DDS_CDRSTREAM_H

#include "dds/dds.h"
#include "dds/ddsrt/bswap.h"
#include "dds/ddsrt/static_assert.h"

#if defined(__cplusplus)
extern "C"
{
#endif

#define DDS_CDRSTREAM_MAX_NESTING_DEPTH 32 /* maximum level of nesting for key extraction */

/*
 * 用于序列化的编码版本。编码版本 1 表示 DDS XTypes 规范中定义的 XCDR1 格式，
 * 使用 PLAIN_CDR(1)，它与非 XTypes 启用节点使用的 CDR 编码向后兼容。
 */
#define DDSI_RTPS_CDR_ENC_VERSION_UNDEF 0 /**< 未定义的编码版本 */
#define DDSI_RTPS_CDR_ENC_VERSION_1 1     /**< 编码版本 1，表示 XCDR1 格式 */
#define DDSI_RTPS_CDR_ENC_VERSION_2 2     /**< 编码版本 2 */

#define DDSI_RTPS_CDR_ENC_FORMAT_PLAIN 0     /**< PLAIN 格式编码 */
#define DDSI_RTPS_CDR_ENC_FORMAT_DELIMITED 1 /**< DELIMITED 格式编码 */
#define DDSI_RTPS_CDR_ENC_FORMAT_PL 2        /**< PL 格式编码 */

/* X-Types 规范 7.6.3.1.2: 实现此规范的应设置选项字段的第二个字节中的最低有效两位为一个值，
 * 该值编码在序列化负载结束后需要填充的字节数，以达到下一个 4 字节对齐的偏移量。 */
#define DDS_CDR_HDR_PADDING_MASK 0x3 /**< 定义用于计算填充字节数的掩码 */

  /** @file
   *  @brief 示例代码文件，包含了一些结构体和枚举类型的定义
   */

  /**
   * @brief DDS CDR 头部结构体
   */
  struct dds_cdr_header
  {
    unsigned short identifier; /**< 标识符 */
    unsigned short options;    /**< 选项 */
  };

  /**
   * @brief DDS CDR 类型扩展性枚举
   */
  enum dds_cdr_type_extensibility
  {
    DDS_CDR_TYPE_EXT_FINAL = 0,      /**< 最终类型 */
    DDS_CDR_TYPE_EXT_APPENDABLE = 1, /**< 可追加类型 */
    DDS_CDR_TYPE_EXT_MUTABLE = 2     /**< 可变类型 */
  };

  /**
   * @brief DDS 输入流结构体
   */
  typedef struct dds_istream
  {
    const unsigned char *m_buffer; /**< 缓冲区指针 */
    uint32_t m_size;               /**< 缓冲区大小 */
    uint32_t m_index;              /**< 从缓冲区开始的读/写偏移量 */
    uint32_t m_xcdr_version;       /**< 数据的 XCDR 版本 */
  } dds_istream_t;

  /**
   * @brief DDS 输出流结构体
   */
  typedef struct dds_ostream
  {
    unsigned char *m_buffer; /**< 缓冲区指针 */
    uint32_t m_size;         /**< 缓冲区大小 */
    uint32_t m_index;        /**< 从缓冲区开始的读/写偏移量 */
    uint32_t m_xcdr_version; /**< 用于序列化数据的 XCDR 版本 */
  } dds_ostream_t;

  /**
   * @brief DDS 输出流大端字节序结构体
   */
  typedef struct dds_ostreamBE
  {
    dds_ostream_t x; /**< 大端字节序的输出流 */
  } dds_ostreamBE_t;

  /**
   * @brief DDS 输出流小端字节序结构体
   */
  typedef struct dds_ostreamLE
  {
    dds_ostream_t x; /**< 小端字节序的输出流 */
  } dds_ostreamLE_t;

  /**
   * @brief DDS CDR 流分配器结构体
   */
  typedef struct dds_cdrstream_allocator
  {
    void *(*malloc)(size_t size);                 /**< 分配内存函数指针 */
    void *(*realloc)(void *ptr, size_t new_size); /**< 重新分配内存函数指针 */
    void (*free)(void *pt);                       /**< 释放内存函数指针 */
    /* 在未来版本中，可能需要一个 void 指针作为自定义分配器实现的参数。 */
  } dds_cdrstream_allocator_t;

  /**
   * @brief DDS CDR 流描述键结构体
   */
  typedef struct dds_cdrstream_desc_key
  {
    uint32_t ops_offs; /**< 键操作的偏移量 */
    uint32_t idx;      /**< 键索引（用于键顺序） */
  } dds_cdrstream_desc_key_t;
  // 定义一个结构体类型 dds_cdrstream_desc_key_seq
  typedef struct dds_cdrstream_desc_key_seq
  {
    uint32_t nkeys;                 // 键的数量
    dds_cdrstream_desc_key_t *keys; // 指向键数组的指针
  } dds_cdrstream_desc_key_seq_t;

  // 定义一个结构体类型 dds_cdrstream_desc_op_seq
  typedef struct dds_cdrstream_desc_op_seq
  {
    uint32_t nops; // ops 中的字数（大于等于预处理输出中存储的操作数）
    uint32_t *ops; // 编组元数据
  } dds_cdrstream_desc_op_seq_t;

  // 定义一个结构体类型 dds_cdrstream_desc
  struct dds_cdrstream_desc
  {
    uint32_t size;                     // 类型的大小
    uint32_t align;                    // 顶级类型的对齐
    uint32_t flagset;                  // 标志集
    dds_cdrstream_desc_key_seq_t keys; // 键序列
    dds_cdrstream_desc_op_seq_t ops;   // 操作序列
    size_t opt_size_xcdr1;             // xcdr1 的可选大小
    size_t opt_size_xcdr2;             // xcdr2 的可选大小
  };

  // 静态断言，确保 dds_ostreamLE_t 结构体中 x 成员的偏移量为 0
  DDSRT_STATIC_ASSERT(offsetof(dds_ostreamLE_t, x) == 0);
  // 静态断言，确保 dds_ostreamBE_t 结构体中 x 成员的偏移量为 0
  DDSRT_STATIC_ASSERT(offsetof(dds_ostreamBE_t, x) == 0);

  /**
   * @brief 对齐到 4 字节并调整大小
   * @param os 输出流指针
   * @param allocator 内存分配器指针
   * @param xcdr_version xcdr 版本
   * @return 返回新的大小
   */
  uint32_t dds_cdr_alignto4_clear_and_resize(dds_ostream_t *__restrict os, const struct dds_cdrstream_allocator *__restrict allocator, uint32_t xcdr_version);

  /**
   * @brief 初始化输入流
   * @param is 输入流指针
   * @param size 大小
   * @param input 输入数据指针
   * @param xcdr_version xcdr 版本
   */
  void dds_istream_init(dds_istream_t *__restrict is, uint32_t size, const void *__restrict input, uint32_t xcdr_version);

  /**
   * @brief 销毁输入流
   * @param is 输入流指针
   */
  void dds_istream_fini(dds_istream_t *__restrict is);

  /**
   * @brief 初始化输出流
   * @param os 输出流指针
   * @param allocator 内存分配器指针
   * @param size 大小
   * @param xcdr_version xcdr 版本
   */
  void dds_ostream_init(dds_ostream_t *__restrict os, const struct dds_cdrstream_allocator *__restrict allocator, uint32_t size, uint32_t xcdr_version);

  /**
   * @brief 销毁输出流
   * @param os 输出流指针
   * @param allocator 内存分配器指针
   */
  void dds_ostream_fini(dds_ostream_t *__restrict os, const struct dds_cdrstream_allocator *__restrict allocator);

  /**
   * @brief 初始化小端字节序的输出流
   * @param os 小端字节序输出流指针
   * @param allocator 内存分配器指针
   * @param size 大小
   * @param xcdr_version xcdr 版本
   */
  void dds_ostreamLE_init(dds_ostreamLE_t *__restrict os, const struct dds_cdrstream_allocator *__restrict allocator, uint32_t size, uint32_t xcdr_version);

  /**
   * @brief 销毁小端字节序的输出流
   * @param os 小端字节序输出流指针
   * @param allocator 内存分配器指针
   */
  void dds_ostreamLE_fini(dds_ostreamLE_t *__restrict os, const struct dds_cdrstream_allocator *__restrict allocator);

  /**
   * @brief 初始化大端字节序的输出流
   * @param os 大端字节序输出流指针
   * @param allocator 内存分配器指针
   * @param size 大小
   * @param xcdr_version xcdr 版本
   */
  void dds_ostreamBE_init(dds_ostreamBE_t *__restrict os, const struct dds_cdrstream_allocator *__restrict allocator, uint32_t size, uint32_t xcdr_version);

  /**
   * @brief 销毁大端字节序的输出流
   * @param os 大端字节序输出流指针
   * @param allocator 内存分配器指针
   */
  void dds_ostreamBE_fini(dds_ostreamBE_t *__restrict os, const struct dds_cdrstream_allocator *__restrict allocator);

  /**
   * @brief 从缓冲区创建输出流
   * @param buffer 缓冲区指针
   * @param size 大小
   * @param write_encoding_version 写入编码版本
   * @return 返回输出流实例
   */
  dds_ostream_t dds_ostream_from_buffer(void *buffer, size_t size, uint16_t write_encoding_version);

  /**
   * @brief 标准化并验证CDR数据
   * @component cdr_serializer
   *
   * @param data          数据样本
   * @param size          数据的大小
   * @param bswap         需要字节交换
   * @param xcdr_version  CDR数据的XCDR版本
   * @param type          类型描述符
   * @param just_key      表示数据是序列化键还是完整样本
   * @param actual_size   成功返回时设置为数据的实际大小（*actual_size <= size）
   * @returns             当验证和标准化成功时返回True
   */
  DDS_EXPORT bool dds_stream_normalize(void *__restrict data, uint32_t size, bool bswap, uint32_t xcdr_version, const struct dds_cdrstream_desc *__restrict type, bool just_key, uint32_t *__restrict actual_size) ddsrt_attribute_warn_unused_result ddsrt_nonnull_all;

  /** @component cdr_serializer */
  DDS_EXPORT const uint32_t *dds_stream_normalize_data(char *__restrict data, uint32_t *__restrict off, uint32_t size, bool bswap, uint32_t xcdr_version, const uint32_t *__restrict ops) ddsrt_attribute_warn_unused_result ddsrt_nonnull_all;

  /** @component cdr_serializer */
  DDS_EXPORT const uint32_t *dds_stream_write(dds_ostream_t *__restrict os, const struct dds_cdrstream_allocator *__restrict allocator, const char *__restrict data, const uint32_t *__restrict ops);

  /** @component cdr_serializer */
  DDS_EXPORT const uint32_t *dds_stream_writeLE(dds_ostreamLE_t *__restrict os, const struct dds_cdrstream_allocator *__restrict allocator, const char *__restrict data, const uint32_t *__restrict ops);

  /** @component cdr_serializer */
  DDS_EXPORT const uint32_t *dds_stream_writeBE(dds_ostreamBE_t *__restrict os, const struct dds_cdrstream_allocator *__restrict allocator, const char *__restrict data, const uint32_t *__restrict ops);

  /** @component cdr_serializer */
  DDS_EXPORT const uint32_t *dds_stream_write_with_byte_order(dds_ostream_t *__restrict os, const struct dds_cdrstream_allocator *__restrict allocator, const char *__restrict data, const uint32_t *__restrict ops, enum ddsrt_byte_order_selector bo);

  /** @component cdr_serializer */
  DDS_EXPORT bool dds_stream_write_sample(dds_ostream_t *__restrict os, const struct dds_cdrstream_allocator *__restrict allocator, const void *__restrict data, const struct dds_cdrstream_desc *__restrict type);

  /** @component cdr_serializer */
  DDS_EXPORT bool dds_stream_write_sampleLE(dds_ostreamLE_t *__restrict os, const struct dds_cdrstream_allocator *__restrict allocator, const void *__restrict data, const struct dds_cdrstream_desc *__restrict type);

  /** @component cdr_serializer */
  DDS_EXPORT bool dds_stream_write_sampleBE(dds_ostreamBE_t *__restrict os, const struct dds_cdrstream_allocator *__restrict allocator, const void *__restrict data, const struct dds_cdrstream_desc *__restrict type);

  /** @component cdr_serializer */
  DDS_EXPORT void dds_stream_read_sample(dds_istream_t *__restrict is, void *__restrict data, const struct dds_cdrstream_allocator *__restrict allocator, const struct dds_cdrstream_desc *__restrict type);

  /** @component cdr_serializer */
  DDS_EXPORT void dds_stream_free_sample(void *__restrict data, const struct dds_cdrstream_allocator *__restrict allocator, const uint32_t *__restrict ops);

  /** @component cdr_serializer */
  DDS_EXPORT uint32_t dds_stream_countops(const uint32_t *__restrict ops, uint32_t nkeys, const dds_key_descriptor_t *__restrict keys);

  /** @component cdr_serializer */
  size_t dds_stream_check_optimize(const struct dds_cdrstream_desc *__restrict desc, uint32_t xcdr_version);

  /** @component cdr_serializer */
  void dds_stream_write_key(dds_ostream_t *__restrict os, const struct dds_cdrstream_allocator *__restrict allocator, const char *__restrict sample, const struct dds_cdrstream_desc *__restrict type);

  /** @component cdr_serializer */
  void dds_stream_write_keyBE(dds_ostreamBE_t *__restrict os, const struct dds_cdrstream_allocator *__restrict allocator, const char *__restrict sample, const struct dds_cdrstream_desc *__restrict type);

  /** @component cdr_serializer */
  DDS_EXPORT bool dds_stream_extract_key_from_data(dds_istream_t *__restrict is, dds_ostream_t *__restrict os, const struct dds_cdrstream_allocator *__restrict allocator, const struct dds_cdrstream_desc *__restrict type);

  /** @component cdr_serializer */
  DDS_EXPORT void dds_stream_extract_key_from_key(dds_istream_t *__restrict is, dds_ostream_t *__restrict os, const struct dds_cdrstream_allocator *__restrict allocator, const struct dds_cdrstream_desc *__restrict type);

  /** @component cdr_serializer */
  DDS_EXPORT bool dds_stream_extract_keyBE_from_data(dds_istream_t *__restrict is, dds_ostreamBE_t *__restrict os, const struct dds_cdrstream_allocator *__restrict allocator, const struct dds_cdrstream_desc *__restrict type);

  /** @component cdr_serializer */
  DDS_EXPORT void dds_stream_extract_keyBE_from_key(dds_istream_t *__restrict is, dds_ostreamBE_t *__restrict os, const struct dds_cdrstream_allocator *__restrict allocator, const struct dds_cdrstream_desc *__restrict type);

  /** @component cdr_serializer */
  DDS_EXPORT const uint32_t *dds_stream_read(dds_istream_t *__restrict is, char *__restrict data, const struct dds_cdrstream_allocator *__restrict allocator, const uint32_t *__restrict ops);

  /** @component cdr_serializer */
  DDS_EXPORT void dds_stream_read_key(dds_istream_t *__restrict is, char *__restrict sample, const struct dds_cdrstream_allocator *__restrict allocator, const struct dds_cdrstream_desc *__restrict type);

  /** @component cdr_serializer */
  DDS_EXPORT size_t dds_stream_print_key(dds_istream_t *__restrict is, const struct dds_cdrstream_desc *__restrict type, char *__restrict buf, size_t size);

  /** @component cdr_serializer */
  DDS_EXPORT size_t dds_stream_print_sample(dds_istream_t *__restrict is, const struct dds_cdrstream_desc *__restrict type, char *__restrict buf, size_t size);

  /** @component cdr_serializer */
  uint16_t dds_stream_minimum_xcdr_version(const uint32_t *__restrict ops);

  /** @component cdr_serializer */
  uint32_t dds_stream_type_nesting_depth(const uint32_t *__restrict ops);

  /** @component cdr_serializer */
  bool dds_stream_extensibility(const uint32_t *__restrict ops, enum dds_cdr_type_extensibility *ext);

  /** @component cdr_serializer */
  DDS_EXPORT void dds_cdrstream_desc_fini(struct dds_cdrstream_desc *desc, const struct dds_cdrstream_allocator *__restrict allocator);

#if defined(__cplusplus)
}
#endif
#endif
