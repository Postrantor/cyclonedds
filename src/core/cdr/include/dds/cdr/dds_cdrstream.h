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
extern "C" {
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
struct dds_cdr_header {
  unsigned short identifier; /**< 标识符 */
  unsigned short options;    /**< 选项 */
};

/**
 * @brief DDS CDR 类型扩展性枚举
 */
enum dds_cdr_type_extensibility {
  DDS_CDR_TYPE_EXT_FINAL = 0,      /**< 最终类型 */
  DDS_CDR_TYPE_EXT_APPENDABLE = 1, /**< 可追加类型 */
  DDS_CDR_TYPE_EXT_MUTABLE = 2     /**< 可变类型 */
};

/**
 * @brief DDS 输入流结构体
 */
typedef struct dds_istream {
  const unsigned char* m_buffer; /**< 缓冲区指针 */
  uint32_t m_size;               /**< 缓冲区大小 */
  uint32_t m_index;              /**< 从缓冲区开始的读/写偏移量 */
  uint32_t m_xcdr_version;       /**< 数据的 XCDR 版本 */
} dds_istream_t;

/**
 * @brief DDS 输出流结构体
 */
typedef struct dds_ostream {
  unsigned char* m_buffer; /**< 缓冲区指针 */
  uint32_t m_size;         /**< 缓冲区大小 */
  uint32_t m_index;        /**< 从缓冲区开始的读/写偏移量 */
  uint32_t m_xcdr_version; /**< 用于序列化数据的 XCDR 版本 */
} dds_ostream_t;

/**
 * @brief DDS 输出流大端字节序结构体
 */
typedef struct dds_ostreamBE {
  dds_ostream_t x; /**< 大端字节序的输出流 */
} dds_ostreamBE_t;

/**
 * @brief DDS 输出流小端字节序结构体
 */
typedef struct dds_ostreamLE {
  dds_ostream_t x; /**< 小端字节序的输出流 */
} dds_ostreamLE_t;

/**
 * @brief DDS CDR 流分配器结构体
 */
typedef struct dds_cdrstream_allocator {
  void* (*malloc)(size_t size);                 /**< 分配内存函数指针 */
  void* (*realloc)(void* ptr, size_t new_size); /**< 重新分配内存函数指针 */
  void (*free)(void* pt);                       /**< 释放内存函数指针 */
  /* 在未来版本中，可能需要一个 void 指针作为自定义分配器实现的参数。 */
} dds_cdrstream_allocator_t;

/**
 * @brief DDS CDR 流描述键结构体
 */
typedef struct dds_cdrstream_desc_key {
  uint32_t ops_offs; /**< 键操作的偏移量 */
  uint32_t idx;      /**< 键索引（用于键顺序） */
} dds_cdrstream_desc_key_t;
// 定义一个结构体类型 dds_cdrstream_desc_key_seq
typedef struct dds_cdrstream_desc_key_seq {
  uint32_t nkeys;                  // 键的数量
  dds_cdrstream_desc_key_t* keys;  // 指向键数组的指针
} dds_cdrstream_desc_key_seq_t;

// 定义一个结构体类型 dds_cdrstream_desc_op_seq
typedef struct dds_cdrstream_desc_op_seq {
  uint32_t nops;  // ops 中的字数（大于等于预处理输出中存储的操作数）
  uint32_t* ops;  // 编组元数据
} dds_cdrstream_desc_op_seq_t;

// 定义一个结构体类型 dds_cdrstream_desc
struct dds_cdrstream_desc {
  uint32_t size;                      // 类型的大小
  uint32_t align;                     // 顶级类型的对齐
  uint32_t flagset;                   // 标志集
  dds_cdrstream_desc_key_seq_t keys;  // 键序列
  dds_cdrstream_desc_op_seq_t ops;    // 操作序列
  size_t opt_size_xcdr1;              // xcdr1 的可选大小
  size_t opt_size_xcdr2;              // xcdr2 的可选大小
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
uint32_t dds_cdr_alignto4_clear_and_resize(
    dds_ostream_t* __restrict os,
    const struct dds_cdrstream_allocator* __restrict allocator,
    uint32_t xcdr_version);

/**
 * @brief 初始化输入流
 * @param is 输入流指针
 * @param size 大小
 * @param input 输入数据指针
 * @param xcdr_version xcdr 版本
 */
void dds_istream_init(dds_istream_t* __restrict is,
                      uint32_t size,
                      const void* __restrict input,
                      uint32_t xcdr_version);

/**
 * @brief 销毁输入流
 * @param is 输入流指针
 */
void dds_istream_fini(dds_istream_t* __restrict is);

/**
 * @brief 初始化输出流
 * @param os 输出流指针
 * @param allocator 内存分配器指针
 * @param size 大小
 * @param xcdr_version xcdr 版本
 */
void dds_ostream_init(dds_ostream_t* __restrict os,
                      const struct dds_cdrstream_allocator* __restrict allocator,
                      uint32_t size,
                      uint32_t xcdr_version);

/**
 * @brief 销毁输出流
 * @param os 输出流指针
 * @param allocator 内存分配器指针
 */
void dds_ostream_fini(dds_ostream_t* __restrict os,
                      const struct dds_cdrstream_allocator* __restrict allocator);

/**
 * @brief 初始化小端字节序的输出流
 * @param os 小端字节序输出流指针
 * @param allocator 内存分配器指针
 * @param size 大小
 * @param xcdr_version xcdr 版本
 */
void dds_ostreamLE_init(dds_ostreamLE_t* __restrict os,
                        const struct dds_cdrstream_allocator* __restrict allocator,
                        uint32_t size,
                        uint32_t xcdr_version);

/**
 * @brief 销毁小端字节序的输出流
 * @param os 小端字节序输出流指针
 * @param allocator 内存分配器指针
 */
void dds_ostreamLE_fini(dds_ostreamLE_t* __restrict os,
                        const struct dds_cdrstream_allocator* __restrict allocator);

/**
 * @brief 初始化大端字节序的输出流
 * @param os 大端字节序输出流指针
 * @param allocator 内存分配器指针
 * @param size 大小
 * @param xcdr_version xcdr 版本
 */
void dds_ostreamBE_init(dds_ostreamBE_t* __restrict os,
                        const struct dds_cdrstream_allocator* __restrict allocator,
                        uint32_t size,
                        uint32_t xcdr_version);

/**
 * @brief 销毁大端字节序的输出流
 * @param os 大端字节序输出流指针
 * @param allocator 内存分配器指针
 */
void dds_ostreamBE_fini(dds_ostreamBE_t* __restrict os,
                        const struct dds_cdrstream_allocator* __restrict allocator);

/**
 * @brief 从缓冲区创建输出流
 * @param buffer 缓冲区指针
 * @param size 大小
 * @param write_encoding_version 写入编码版本
 * @return 返回输出流实例
 */
dds_ostream_t dds_ostream_from_buffer(void* buffer, size_t size, uint16_t write_encoding_version);

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
DDS_EXPORT bool dds_stream_normalize(void* __restrict data,
                                     uint32_t size,
                                     bool bswap,
                                     uint32_t xcdr_version,
                                     const struct dds_cdrstream_desc* __restrict type,
                                     bool just_key,
                                     uint32_t* __restrict actual_size)
    ddsrt_attribute_warn_unused_result ddsrt_nonnull_all;

/** @component cdr_serializer */
/**
 * @brief 将数据规范化为CDR表示形式。
 * @param[in] data 输入的数据指针。
 * @param[in,out] off 数据偏移量指针，用于跟踪当前处理的数据位置。
 * @param[in] size 数据大小。
 * @param[in] bswap 是否交换字节顺序。
 * @param[in] xcdr_version CDR版本（XCDR1或XCDR2）。
 * @param[in] ops 操作序列，用于描述如何将数据序列化为CDR表示形式。
 * @return 规范化后的数据指针。
 *
 * @brief Normalize data to CDR representation.
 * @param[in] data Input data pointer.
 * @param[in,out] off Pointer to data offset, used to track the current position of data being
 * processed.
 * @param[in] size Size of the data.
 * @param[in] bswap Whether to swap byte order.
 * @param[in] xcdr_version CDR version (XCDR1 or XCDR2).
 * @param[in] ops Sequence of operations describing how to serialize the data into CDR
 * representation.
 * @return Pointer to normalized data.
 */
DDS_EXPORT const uint32_t* dds_stream_normalize_data(char* __restrict data,
                                                     uint32_t* __restrict off,
                                                     uint32_t size,
                                                     bool bswap,
                                                     uint32_t xcdr_version,
                                                     const uint32_t* __restrict ops)
    ddsrt_attribute_warn_unused_result ddsrt_nonnull_all;

/** @component cdr_serializer */
/**
 * @brief 将数据写入输出流。
 * @param[in] os 输出流指针。
 * @param[in] allocator 内存分配器指针。
 * @param[in] data 输入的数据指针。
 * @param[in] ops 操作序列，用于描述如何将数据序列化为CDR表示形式。
 * @return 规范化后的数据指针。
 *
 * @brief Write data to the output stream.
 * @param[in] os Output stream pointer.
 * @param[in] allocator Pointer to memory allocator.
 * @param[in] data Input data pointer.
 * @param[in] ops Sequence of operations describing how to serialize the data into CDR
 * representation.
 * @return Pointer to normalized data.
 */
DDS_EXPORT const uint32_t* dds_stream_write(
    dds_ostream_t* __restrict os,
    const struct dds_cdrstream_allocator* __restrict allocator,
    const char* __restrict data,
    const uint32_t* __restrict ops);

/** @component cdr_serializer */
/**
 * @brief 以小端字节顺序将数据写入输出流。
 * @param[in] os 输出流指针。
 * @param[in] allocator 内存分配器指针。
 * @param[in] data 输入的数据指针。
 * @param[in] ops 操作序列，用于描述如何将数据序列化为CDR表示形式。
 * @return 规范化后的数据指针。
 *
 * @brief Write data to the output stream in little-endian byte order.
 * @param[in] os Output stream pointer.
 * @param[in] allocator Pointer to memory allocator.
 * @param[in] data Input data pointer.
 * @param[in] ops Sequence of operations describing how to serialize the data into CDR
 * representation.
 * @return Pointer to normalized data.
 */
DDS_EXPORT const uint32_t* dds_stream_writeLE(
    dds_ostreamLE_t* __restrict os,
    const struct dds_cdrstream_allocator* __restrict allocator,
    const char* __restrict data,
    const uint32_t* __restrict ops);

/** @component cdr_serializer */
/**
 * @brief 以大端字节顺序将数据写入输出流。
 * @param[in] os 输出流指针。
 * @param[in] allocator 内存分配器指针。
 * @param[in] data 输入的数据指针。
 * @param[in] ops 操作序列，用于描述如何将数据序列化为CDR表示形式。
 * @return 规范化后的数据指针。
 *
 * @brief Write data to the output stream in big-endian byte order.
 * @param[in] os Output stream pointer.
 * @param[in] allocator Pointer to memory allocator.
 * @param[in] data Input data pointer.
 * @param[in] ops Sequence of operations describing how to serialize the data into CDR
 * representation.
 * @return Pointer to normalized data.
 */
DDS_EXPORT const uint32_t* dds_stream_writeBE(
    dds_ostreamBE_t* __restrict os,
    const struct dds_cdrstream_allocator* __restrict allocator,
    const char* __restrict data,
    const uint32_t* __restrict ops);

/** @component cdr_serializer */
/**
 * @brief 根据指定的字节顺序将数据写入输出流。
 * @param[in] os 输出流指针。
 * @param[in] allocator 内存分配器指针。
 * @param[in] data 输入的数据指针。
 * @param[in] ops 操作序列，用于描述如何将数据序列化为CDR表示形式。
 * @param[in] bo 字节顺序选择器（DDSRT_BOSEL_NATIVE、DDSRT_BOSEL_LE或DDSRT_BOSEL_BE）。
 * @return 规范化后的数据指针。
 *
 * @brief Write data to the output stream with specified byte order.
 * @param[in] os Output stream pointer.
 * @param[in] allocator Pointer to memory allocator.
 * @param[in] data Input data pointer.
 * @param[in] ops Sequence of operations describing how to serialize the data into CDR
 * representation.
 * @param[in] bo Byte order selector (DDSRT_BOSEL_NATIVE, DDSRT_BOSEL_LE or DDSRT_BOSEL_BE).
 * @return Pointer to normalized data.
 */
DDS_EXPORT const uint32_t* dds_stream_write_with_byte_order(
    dds_ostream_t* __restrict os,
    const struct dds_cdrstream_allocator* __restrict allocator,
    const char* __restrict data,
    const uint32_t* __restrict ops,
    enum ddsrt_byte_order_selector bo);

/** @component cdr_serializer */
/**
 * @brief 将样本数据写入输出流。
 * @param[in] os 输出流指针。
 * @param[in] allocator 内存分配器指针。
 * @param[in] data 输入的样本数据指针。
 * @param[in] type 样本数据类型描述符。
 * @return 是否成功写入样本数据。
 *
 * @brief Write sample data to the output stream.
 * @param[in] os Output stream pointer.
 * @param[in] allocator Pointer to memory allocator.
 * @param[in] data Input sample data pointer.
 * @param[in] type Sample data type descriptor.
 * @return Whether the sample data was successfully written.
 */
DDS_EXPORT bool dds_stream_write_sample(dds_ostream_t* __restrict os,
                                        const struct dds_cdrstream_allocator* __restrict allocator,
                                        const void* __restrict data,
                                        const struct dds_cdrstream_desc* __restrict type);

/**
 * @brief 写入一个小端序列化的样本 (Write a little-endian serialized sample)
 * @param os 指向dds_ostreamLE_t结构体的指针 (Pointer to the dds_ostreamLE_t structure)
 * @param allocator 指向dds_cdrstream_allocator结构体的指针 (Pointer to the dds_cdrstream_allocator
 * structure)
 * @param data 指向待序列化数据的指针 (Pointer to the data to be serialized)
 * @param type 指向dds_cdrstream_desc结构体的指针，描述数据类型 (Pointer to the dds_cdrstream_desc
 * structure, describing the data type)
 * @return 序列化成功返回true，否则返回false (Returns true if serialization is successful, false
 * otherwise)
 */
DDS_EXPORT bool dds_stream_write_sampleLE(
    dds_ostreamLE_t* __restrict os,
    const struct dds_cdrstream_allocator* __restrict allocator,
    const void* __restrict data,
    const struct dds_cdrstream_desc* __restrict type);

/**
 * @brief 写入一个大端序列化的样本 (Write a big-endian serialized sample)
 * @param os 指向dds_ostreamBE_t结构体的指针 (Pointer to the dds_ostreamBE_t structure)
 * @param allocator 指向dds_cdrstream_allocator结构体的指针 (Pointer to the dds_cdrstream_allocator
 * structure)
 * @param data 指向待序列化数据的指针 (Pointer to the data to be serialized)
 * @param type 指向dds_cdrstream_desc结构体的指针，描述数据类型 (Pointer to the dds_cdrstream_desc
 * structure, describing the data type)
 * @return 序列化成功返回true，否则返回false (Returns true if serialization is successful, false
 * otherwise)
 */
DDS_EXPORT bool dds_stream_write_sampleBE(
    dds_ostreamBE_t* __restrict os,
    const struct dds_cdrstream_allocator* __restrict allocator,
    const void* __restrict data,
    const struct dds_cdrstream_desc* __restrict type);

/**
 * @brief 读取一个序列化的样本 (Read a serialized sample)
 * @param is 指向dds_istream_t结构体的指针 (Pointer to the dds_istream_t structure)
 * @param data 指向存储反序列化数据的内存空间的指针 (Pointer to the memory space where the
 * deserialized data will be stored)
 * @param allocator 指向dds_cdrstream_allocator结构体的指针 (Pointer to the dds_cdrstream_allocator
 * structure)
 * @param type 指向dds_cdrstream_desc结构体的指针，描述数据类型 (Pointer to the dds_cdrstream_desc
 * structure, describing the data type)
 */
DDS_EXPORT void dds_stream_read_sample(dds_istream_t* __restrict is,
                                       void* __restrict data,
                                       const struct dds_cdrstream_allocator* __restrict allocator,
                                       const struct dds_cdrstream_desc* __restrict type);

/**
 * @brief 释放序列化样本的内存空间 (Free the memory space of the serialized sample)
 * @param data 指向待释放内存空间的指针 (Pointer to the memory space to be freed)
 * @param allocator 指向dds_cdrstream_allocator结构体的指针 (Pointer to the dds_cdrstream_allocator
 * structure)
 * @param ops 指向操作列表的指针 (Pointer to the list of operations)
 */
DDS_EXPORT void dds_stream_free_sample(void* __restrict data,
                                       const struct dds_cdrstream_allocator* __restrict allocator,
                                       const uint32_t* __restrict ops);

/**
 * @brief 计算操作列表中的操作数量 (Count the number of operations in the operation list)
 * @param ops 指向操作列表的指针 (Pointer to the list of operations)
 * @param nkeys 键描述符数组中的键数量 (Number of keys in the key descriptor array)
 * @param keys 指向dds_key_descriptor_t结构体的指针，描述键 (Pointer to the dds_key_descriptor_t
 * structure, describing the keys)
 * @return 操作数量 (Number of operations)
 */
DDS_EXPORT uint32_t dds_stream_countops(const uint32_t* __restrict ops,
                                        uint32_t nkeys,
                                        const dds_key_descriptor_t* __restrict keys);

/**
 * @brief 检查是否可以优化序列化操作
 * @param[in] desc 序列化描述符
 * @param[in] xcdr_version XCDR版本
 * @return 可优化的大小
 *
 * @brief Check if serialization operation can be optimized
 * @param[in] desc Serialization descriptor
 * @param[in] xcdr_version XCDR version
 * @return Size that can be optimized
 */
size_t dds_stream_check_optimize(const struct dds_cdrstream_desc* __restrict desc,
                                 uint32_t xcdr_version);

/**
 * @brief 将键值写入输出流
 * @param[out] os 输出流
 * @param[in] allocator 分配器
 * @param[in] sample 样本数据
 * @param[in] type 类型描述符
 *
 * @brief Write key value to output stream
 * @param[out] os Output stream
 * @param[in] allocator Allocator
 * @param[in] sample Sample data
 * @param[in] type Type descriptor
 */
void dds_stream_write_key(dds_ostream_t* __restrict os,
                          const struct dds_cdrstream_allocator* __restrict allocator,
                          const char* __restrict sample,
                          const struct dds_cdrstream_desc* __restrict type);

/**
 * @brief 将键值以大端字节序写入输出流
 * @param[out] os 大端字节序输出流
 * @param[in] allocator 分配器
 * @param[in] sample 样本数据
 * @param[in] type 类型描述符
 *
 * @brief Write key value to output stream in big-endian byte order
 * @param[out] os Big-endian byte order output stream
 * @param[in] allocator Allocator
 * @param[in] sample Sample data
 * @param[in] type Type descriptor
 */
void dds_stream_write_keyBE(dds_ostreamBE_t* __restrict os,
                            const struct dds_cdrstream_allocator* __restrict allocator,
                            const char* __restrict sample,
                            const struct dds_cdrstream_desc* __restrict type);

/**
 * @brief 从数据中提取键值
 * @param[in] is 输入流
 * @param[out] os 输出流
 * @param[in] allocator 分配器
 * @param[in] type 类型描述符
 * @return 是否成功提取
 *
 * @brief Extract key value from data
 * @param[in] is Input stream
 * @param[out] os Output stream
 * @param[in] allocator Allocator
 * @param[in] type Type descriptor
 * @return Whether extraction was successful
 */
DDS_EXPORT bool dds_stream_extract_key_from_data(
    dds_istream_t* __restrict is,
    dds_ostream_t* __restrict os,
    const struct dds_cdrstream_allocator* __restrict allocator,
    const struct dds_cdrstream_desc* __restrict type);

/**
 * @brief 从键值中提取键值
 * @param[in] is 输入流
 * @param[out] os 输出流
 * @param[in] allocator 分配器
 * @param[in] type 类型描述符
 *
 * @brief Extract key value from key value
 * @param[in] is Input stream
 * @param[out] os Output stream
 * @param[in] allocator Allocator
 * @param[in] type Type descriptor
 */
DDS_EXPORT void dds_stream_extract_key_from_key(
    dds_istream_t* __restrict is,
    dds_ostream_t* __restrict os,
    const struct dds_cdrstream_allocator* __restrict allocator,
    const struct dds_cdrstream_desc* __restrict type);

/**
 * @brief 以大端字节序从数据中提取键值
 * @param[in] is 输入流
 * @param[out] os 大端字节序输出流
 * @param[in] allocator 分配器
 * @param[in] type 类型描述符
 * @return 是否成功提取
 *
 * @brief Extract key value from data in big-endian byte order
 * @param[in] is Input stream
 * @param[out] os Big-endian byte order output stream
 * @param[in] allocator Allocator
 * @param[in] type Type descriptor
 * @return Whether extraction was successful
 */
DDS_EXPORT bool dds_stream_extract_keyBE_from_data(
    dds_istream_t* __restrict is,
    dds_ostreamBE_t* __restrict os,
    const struct dds_cdrstream_allocator* __restrict allocator,
    const struct dds_cdrstream_desc* __restrict type);

/**
 * @brief 以大端字节序从键值中提取键值
 * @param[in] is 输入流
 * @param[out] os 大端字节序输出流
 * @param[in] allocator 分配器
 * @param[in] type 类型描述符
 *
 * @brief Extract key value from key value in big-endian byte order
 * @param[in] is Input stream
 * @param[out] os Big-endian byte order output stream
 * @param[in] allocator Allocator
 * @param[in] type Type descriptor
 */
DDS_EXPORT void dds_stream_extract_keyBE_from_key(
    dds_istream_t* __restrict is,
    dds_ostreamBE_t* __restrict os,
    const struct dds_cdrstream_allocator* __restrict allocator,
    const struct dds_cdrstream_desc* __restrict type);

/**
 * @brief 从输入流中读取数据
 * @param[in] is 输入流
 * @param[out] data 输出数据
 * @param[in] allocator 分配器
 * @param[in] ops 操作序列
 * @return 下一个操作序列
 *
 * @brief Read data from input stream
 * @param[in] is Input stream
 * @param[out] data Output data
 * @param[in] allocator Allocator
 * @param[in] ops Operation sequence
 * @return Next operation sequence
 */
DDS_EXPORT const uint32_t* dds_stream_read(
    dds_istream_t* __restrict is,
    char* __restrict data,
    const struct dds_cdrstream_allocator* __restrict allocator,
    const uint32_t* __restrict ops);

/**
 * @brief 从输入流中读取键值
 * @param[in] is 输入流
 * @param[out] sample 存储键值的缓冲区
 * @param[in] allocator 分配器
 * @param[in] type 类型描述符
 *
 * @brief Read key value from input stream
 * @param[in] is Input stream
 * @param[out] sample Buffer to store the key value
 * @param[in] allocator Allocator
 * @param[in] type Type descriptor
 */
DDS_EXPORT void dds_stream_read_key(dds_istream_t* __restrict is,
                                    char* __restrict sample,
                                    const struct dds_cdrstream_allocator* __restrict allocator,
                                    const struct dds_cdrstream_desc* __restrict type);

/**
 * @brief 打印键值到字符串
 * @param[in] is 输入流
 * @param[in] type 类型描述符
 * @param[out] buf 字符串缓冲区
 * @param[in] size 缓冲区大小
 * @return 写入字符串的字符数
 *
 * @brief Print key value to string
 * @param[in] is Input stream
 * @param[in] type Type descriptor
 * @param[out] buf String buffer
 * @param[in] size Buffer size
 * @return Number of characters written to the string
 */
DDS_EXPORT size_t dds_stream_print_key(dds_istream_t* __restrict is,
                                       const struct dds_cdrstream_desc* __restrict type,
                                       char* __restrict buf,
                                       size_t size);

/**
 * @brief 打印样本到字符串
 * @param[in] is 输入流
 * @param[in] type 类型描述符
 * @param[out] buf 字符串缓冲区
 * @param[in] size 缓冲区大小
 * @return 写入字符串的字符数
 *
 * @brief Print sample to string
 * @param[in] is Input stream
 * @param[in] type Type descriptor
 * @param[out] buf String buffer
 * @param[in] size Buffer size
 * @return Number of characters written to the string
 */
DDS_EXPORT size_t dds_stream_print_sample(dds_istream_t* __restrict is,
                                          const struct dds_cdrstream_desc* __restrict type,
                                          char* __restrict buf,
                                          size_t size);

/**
 * @brief 获取最小的 XCDR 版本
 * @param[in] ops 操作序列
 * @return 最小的 XCDR 版本
 *
 * @brief Get minimum XCDR version
 * @param[in] ops Operation sequence
 * @return Minimum XCDR version
 */
uint16_t dds_stream_minimum_xcdr_version(const uint32_t* __restrict ops);

/**
 * @brief 获取类型嵌套深度
 * @param[in] ops 操作序列
 * @return 类型嵌套深度
 *
 * @brief Get type nesting depth
 * @param[in] ops Operation sequence
 * @return Type nesting depth
 */
uint32_t dds_stream_type_nesting_depth(const uint32_t* __restrict ops);

/**
 * @brief 获取类型可扩展性
 * @param[in] ops 操作序列
 * @param[out] ext 类型可扩展性枚举
 * @return 是否成功获取类型可扩展性
 *
 * @brief Get type extensibility
 * @param[in] ops Operation sequence
 * @param[out] ext Type extensibility enum
 * @return Whether the type extensibility was successfully obtained
 */
bool dds_stream_extensibility(const uint32_t* __restrict ops, enum dds_cdr_type_extensibility* ext);

/**
 * @brief 清理类型描述符
 * @param[in,out] desc 类型描述符
 * @param[in] allocator 分配器
 *
 * @brief Clean up type descriptor
 * @param[in,out] desc Type descriptor
 * @param[in] allocator Allocator
 */
DDS_EXPORT void dds_cdrstream_desc_fini(struct dds_cdrstream_desc* desc,
                                        const struct dds_cdrstream_allocator* __restrict allocator);

#if defined(__cplusplus)
}
#endif
#endif
