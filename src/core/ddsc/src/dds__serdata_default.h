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
#ifndef DDS__SERDATA_DEFAULT_H
#define DDS__SERDATA_DEFAULT_H

#include "dds/cdr/dds_cdrstream.h"
#include "dds/dds.h"
#include "dds/ddsi/ddsi_freelist.h"
#include "dds/ddsi/ddsi_protocol.h"
#include "dds/ddsi/ddsi_serdata.h"
#include "dds/ddsi/ddsi_typelib.h"
#include "dds/ddsrt/avl.h"
#include "dds/ddsrt/endian.h"
#include "dds__types.h"

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @brief 定义关键缓冲区类型和默认键大小掩码
 */

// 定义未设置的键缓冲区类型
#define KEYBUFTYPE_UNSET 0u

// 定义静态键缓冲区类型，使用 u.stbuf
#define KEYBUFTYPE_STATIC 1u

// 定义动态别名键缓冲区类型，指向有效负载中的位置
#define KEYBUFTYPE_DYNALIAS 2u

// 定义动态分配的键缓冲区类型
#define KEYBUFTYPE_DYNALLOC 3u

// 定义默认键大小掩码
#define SERDATA_DEFAULT_KEYSIZE_MASK 0x3FFFFFFFu

/**
 * @struct dds_serdata_default_key
 * @brief 默认的序列化数据键结构体
 */
struct dds_serdata_default_key {
  unsigned buftype : 2;                           ///< 缓冲区类型，占用2位
  unsigned keysize : 30;                          ///< 键大小，占用30位
  union {
    unsigned char stbuf[DDS_FIXED_KEY_MAX_SIZE];  ///< 静态缓冲区，大小为 DDS_FIXED_KEY_MAX_SIZE
    unsigned char* dynbuf;                        ///< 动态缓冲区指针
  } u;  ///< 匿名联合体，包含静态缓冲区和动态缓冲区
};

/**
 * @struct dds_serdatapool
 * @brief 序列化数据池结构体
 */
struct dds_serdatapool {
  struct ddsi_freelist freelist;  ///< 自由列表，用于管理可用的序列化数据对象
};

/*
在这段代码中，我们根据编译时是否定义了 NDEBUG 来决定是否需要保留一些额外的状态信息。如果没有定义
NDEBUG（即调试构建），则定义一个名为 DDS_SERDATA_DEFAULT_DEBUG_FIELDS
的宏，其中包含一个布尔类型的变量 fixed，表示缓冲区是否为固定大小。如果定义了
NDEBUG（即非调试构建），则定义一个空的宏。
*/
// 在非调试构建中，定义一个空的宏，否则定义一个包含额外状态的宏
#ifndef NDEBUG
/**
 * @def DDS_SERDATA_DEFAULT_DEBUG_FIELDS
 * @brief 调试构建时使用的宏，用于保留一些额外的状态信息
 */
#define DDS_SERDATA_DEFAULT_DEBUG_FIELDS bool fixed;  ///< 表示是否为固定大小的缓冲区
#else
/**
 * @def DDS_SERDATA_DEFAULT_DEBUG_FIELDS
 * @brief 非调试构建时使用的空宏
 */
#define DDS_SERDATA_DEFAULT_DEBUG_FIELDS
#endif

// 定义一个宏，用于在 dds_serdata_default 结构体中插入所需的字段
// 注意：原始数据有对齐要求（从 dds_stream 转换为/转换回时，偏移量必须是 8 的倍数）
// 因此我们定义两种类型：一种没有任何额外填充的类型，另一种在适当位置插入了填充
#define DDS_SERDATA_DEFAULT_PREPAD                                               \
  struct ddsi_serdata c;                  /*!< 序列化数据通用结构体 */ \
  uint32_t pos;                           /*!< 当前位置 */                   \
  uint32_t size;                          /*!< 数据大小 */                   \
  DDS_SERDATA_DEFAULT_DEBUG_FIELDS        /*!< 调试字段 */                   \
      struct dds_serdata_default_key key; /*!< 默认键值结构体 */          \
  struct dds_serdatapool* serpool;        /*!< 序列化数据池指针 */       \
  struct dds_serdata_default* next        /*!< 指向池中下一个空闲元素的指针 */

// 仅针对 MSVC 编译器且为 C++ 代码时，禁用零数组警告（MSVC C4200）
// 因为只有在这种情况下才会出现问题
#if defined _MSC_VER && defined __cplusplus
#define DDS_SERDATA_DEFAULT_POSTPAD                             \
  struct dds_cdr_header hdr;   /*!< CDR 头部结构体 */      \
  DDSRT_WARNING_MSVC_OFF(4200) /*!< 禁用 MSVC 警告 4200 */  \
  char data[];                 /*!< 可变长度数据数组 */ \
  DDSRT_WARNING_MSVC_ON(4200)  /*!< 启用 MSVC 警告 4200 */
#else
#define DDS_SERDATA_DEFAULT_POSTPAD                      \
  struct dds_cdr_header hdr; /*!< CDR 头部结构体 */ \
  char data[]                /*!< 可变长度数据数组 */
#endif

/**
 * @struct dds_serdata_default_unpadded
 * @brief 一个不带填充的默认序列化数据结构体。
 *
 * 此结构体包含了两个宏，用于表示在序列化数据前后的预填充和后填充。
 */
struct dds_serdata_default_unpadded {
  DDS_SERDATA_DEFAULT_PREPAD;   ///< @brief 序列化数据前的预填充宏。
  DDS_SERDATA_DEFAULT_POSTPAD;  ///< @brief 序列化数据后的后填充宏。
};

/**
 * @file
 * @brief 定义一个用于计算填充大小的宏。
 */
#ifdef __GNUC__
/**
 * @def DDS_SERDATA_DEFAULT_PAD(n)
 * @brief 使用 GNU 编译器时，计算 n 对 8 取模的结果作为填充大小。
 *
 * @param n 输入值，用于计算填充大小。
 */
#define DDS_SERDATA_DEFAULT_PAD(n) ((n) % 8)
#else
/**
 * @def DDS_SERDATA_DEFAULT_PAD(n)
 * @brief 使用非 GNU 编译器时，直接使用 n 作为填充大小。
 *
 * @param n 输入值，用于计算填充大小。
 */
#define DDS_SERDATA_DEFAULT_PAD(n) (n)
#endif

/**
 * @struct dds_serdata_default
 * @brief 默认的序列化数据结构体
 */
struct dds_serdata_default {
  DDS_SERDATA_DEFAULT_PREPAD;  ///< 预填充，用于确保数据对齐
  /**
   * @brief 填充数组
   *
   * 用于确保 data 成员变量的偏移量是 8 的倍数。offsetof 计算 data 成员在
   * dds_serdata_default_unpadded 结构体中的偏移量，然后取余 8，得到需要填充的字节数。
   */
  char pad[DDS_SERDATA_DEFAULT_PAD(8 - (offsetof(struct dds_serdata_default_unpadded, data) % 8))];
  DDS_SERDATA_DEFAULT_POSTPAD;  ///< 后填充，用于确保数据对齐
};

// 静态断言，检查 data 成员变量的偏移量是否为 8 的倍数
DDSRT_STATIC_ASSERT((offsetof(struct dds_serdata_default, data) % 8) == 0);

#undef DDS_SERDATA_DEFAULT_PAD
#undef DDS_SERDATA_DEFAULT_POSTPAD
#undef DDS_SERDATA_DEFAULT_PREPAD
#undef DDS_SERDATA_DEFAULT_FIXED_FIELD

/**
 * @brief 结构体 dds_sertype_default_cdr_data，用于存储序列化数据的大小和数据指针
 */
struct dds_sertype_default_cdr_data {
  uint32_t sz;         /**< 数据大小 */
  unsigned char* data; /**< 指向序列化数据的指针 */
};

/**
 * @brief 结构体 dds_sertype_default，包含了与默认序列化类型相关的信息
 */
struct dds_sertype_default {
  struct ddsi_sertype c; /**< 基本的序列化类型信息 */

  // DDSI_RTPS_CDR_ENC_FORMAT_(PLAIN|DELIMITED|PL) - 顶层类型在此序列化类型中的 CDR 编码格式
  uint16_t encoding_format; /**< CDR 编码格式 */

  // DDSI_RTPS_CDR_ENC_VERSION_(1|2) - 使用此序列化类型写入数据时使用的 CDR 编码版本
  uint16_t write_encoding_version; /**< 写入数据时使用的 CDR 编码版本 */

  struct dds_serdatapool* serpool; /**< 序列化数据池指针 */

  struct dds_cdrstream_desc type;  /**< CDR 流描述符 */

  /**
   * @brief 类型信息序列化数据
   */
  struct dds_sertype_default_cdr_data typeinfo_ser;

  /**
   * @brief 类型映射序列化数据
   */
  struct dds_sertype_default_cdr_data typemap_ser;
};

extern const struct ddsi_sertype_ops dds_sertype_ops_default;

extern const struct ddsi_serdata_ops dds_serdata_ops_cdr;
extern const struct ddsi_serdata_ops dds_serdata_ops_cdr_nokey;

extern const struct ddsi_serdata_ops dds_serdata_ops_xcdr2;
extern const struct ddsi_serdata_ops dds_serdata_ops_xcdr2_nokey;

/**
 * @brief 创建一个新的序列化数据池
 *
 * @component typesupport_c
 * @return 返回指向新创建的dds_serdatapool结构体的指针
 */
struct dds_serdatapool* dds_serdatapool_new(void);

/**
 * @brief 释放序列化数据池
 *
 * @component typesupport_c
 * @param pool 指向需要释放的dds_serdatapool结构体的指针
 */
void dds_serdatapool_free(struct dds_serdatapool* pool);

/**
 * @brief 初始化默认的dds_sertype_default结构体
 *
 * @component typesupport_c
 * @param domain 指向dds_domain结构体的指针，表示DDS域
 * @param st 指向需要初始化的dds_sertype_default结构体的指针
 * @param desc 指向dds_topic_descriptor_t结构体的指针，表示主题描述符
 * @param min_xcdrv 表示最小交叉驱动程序版本的uint16_t值
 * @param data_representation 表示数据表示形式的dds_data_representation_id_t枚举值
 * @return 返回dds_return_t类型的结果，表示操作成功或失败
 */
dds_return_t dds_sertype_default_init(const struct dds_domain* domain,
                                      struct dds_sertype_default* st,
                                      const dds_topic_descriptor_t* desc,
                                      uint16_t min_xcdrv,
                                      dds_data_representation_id_t data_representation);

#if defined(__cplusplus)
}
#endif

#endif /* DDS__SERDATA_DEFAULT_H */
