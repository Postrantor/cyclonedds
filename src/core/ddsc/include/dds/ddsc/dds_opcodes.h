/*
 * Copyright(c) 2021 to 2022 ZettaScale Technology and others
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v. 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0, or the Eclipse Distribution License
 * v. 1.0 which is available at
 * http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * SPDX-License-Identifier: EPL-2.0 OR BSD-3-Clause
 */

#ifndef DDS_OPCODES_H
#define DDS_OPCODES_H

#include "dds/ddsrt/align.h"
#include "dds/ddsrt/static_assert.h"

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @defgroup serialization (序列化)
 * @ingroup dds
 *
 * 由 idlc 生成的类型的（反）序列化操作码。在一个单独的头文件中隔离，以便与 idlc 共享，
 * 而无需引入整个 Eclipse Cyclone DDS C 语言绑定。
 *
 * 一个操作码是一个 uint32，根据它是哪个代码，它可以包含几个 uint32 参数。
 */

/**
 * @anchor DDS_OP_MASK
 * @ingroup serialization
 * @brief OP 的掩码
 */
#define DDS_OP_MASK 0xff000000
// 定义 OP 的掩码，用于提取操作码的主要部分

/**
 * @anchor DDS_OP_TYPE_FLAGS_MASK
 * @ingroup serialization
 * @brief OP 类型标志的掩码
 */
#define DDS_OP_TYPE_FLAGS_MASK 0x00800000
// 定义 OP 类型标志的掩码，用于提取操作码的类型标志部分

/**
 * @anchor DDS_OP_TYPE_MASK
 * @ingroup serialization
 * @brief OP 类型的掩码
 */
#define DDS_OP_TYPE_MASK 0x007f0000
// 定义 OP 类型的掩码，用于提取操作码的类型部分

/**
 * @anchor DDS_OP_SUBTYPE_MASK
 * @ingroup serialization
 * @brief Mask for the OP subtype
 * @details 用于 OP 子类型的掩码 (Mask for the OP subtype)
 */
#define DDS_OP_SUBTYPE_MASK 0x0000ff00

/**
 * @anchor DDS_OP_JMP_MASK
 * @ingroup serialization
 * @brief Mask for the OP jump
 * @details 用于 OP 跳转的掩码 (Mask for the OP jump)
 */
#define DDS_OP_JMP_MASK 0x0000ffff

/**
 * @anchor DDS_OP_FLAGS_MASK
 * @ingroup serialization
 * @brief Mask for the OP flags
 * @details 用于 OP 标志的掩码 (Mask for the OP flags)
 */
#define DDS_OP_FLAGS_MASK 0x000000ff

/**
 * @anchor DDS_JEQ_TYPE_FLAGS_MASK
 * @ingroup serialization
 * @brief Mask for the JEQ type flags
 * @details 用于 JEQ 类型标志的掩码 (Mask for the JEQ type flags)
 */
#define DDS_JEQ_TYPE_FLAGS_MASK 0x00800000

/**
 * @anchor DDS_JEQ_TYPE_MASK
 * @ingroup serialization
 * @brief Mask for the JEQ type
 * @details 用于 JEQ 类型的掩码 (Mask for the JEQ type)
 */
#define DDS_JEQ_TYPE_MASK 0x007f0000

/**
 * @anchor DDS_PLM_FLAGS_MASK
 * @ingroup serialization
 * @brief Mask for the PLM flags
 * @details 用于 PLM 标志的掩码 (Mask for the PLM flags)
 */
#define DDS_PLM_FLAGS_MASK 0x00ff0000

/**
 * @anchor DDS_KOF_OFFSET_MASK
 * @ingroup serialization
 * @brief Mask for the KOF offset
 * @details 用于 KOF 偏移量的掩码 (Mask for the KOF offset)
 */
#define DDS_KOF_OFFSET_MASK 0x0000ffff

/**
 * @anchor DDS_OP
 * @ingroup serialization
 * @brief Extract the DDS OP from a uint32 as a \ref dds_stream_opcode
 * @details 从 uint32 中提取 DDS OP 作为 \ref dds_stream_opcode (Extract the DDS OP from a uint32 as
 * a \ref dds_stream_opcode)
 */
#define DDS_OP(o) ((enum dds_stream_opcode)((o)&DDS_OP_MASK))

/**
 * @anchor DDS_OP_TYPE
 * @ingroup serialization
 * @brief 从 uint32 中提取 DDS OP_TYPE 作为 \ref dds_stream_typecode
 * Extract the DDS OP_TYPE from a uint32 as a \ref dds_stream_typecode
 */
#define DDS_OP_TYPE(o) ((enum dds_stream_typecode)(((o)&DDS_OP_TYPE_MASK) >> 16))

/**
 * @anchor DDS_OP_TYPE_FLAGS
 * @ingroup serialization
 * @brief 从 uint32 中提取 DDS OP_TYPE_FLAGS
 * Extract the DDS OP_TYPE_FLAGS from a uint32
 * DOC_TODO: 可能的值？
 * DOC_TODO: possible values?
 */
#define DDS_OP_TYPE_FLAGS(o) ((o)&DDS_OP_TYPE_FLAGS_MASK)

/**
 * @anchor DDS_OP_SUBTYPE
 * @ingroup serialization
 * @brief 从 uint32 中提取 DDS OP_SUBTYPE 作为 \ref dds_stream_typecode
 * Extract the DDS OP_SUBTYPE from a uint32 as a \ref dds_stream_typecode
 * 用于集合，OP_TYPE 是数组/序列
 * Used for collections, the OP_TYPE is array/sequence
 */
#define DDS_OP_SUBTYPE(o) ((enum dds_stream_typecode)(((o)&DDS_OP_SUBTYPE_MASK) >> 8))

/**
 * @anchor DDS_OP_FLAGS
 * @ingroup serialization
 * @brief 从 uint32 中提取 DDS OP_FLAGS
 * Extract the DDS OP_FLAGS from a uint32
 * DOC_TODO: 可能的值？
 * DOC_TODO: possible values?
 */
#define DDS_OP_FLAGS(o) ((o)&DDS_OP_FLAGS_MASK)

/**
 * @anchor DDS_OP_ADR_JSR
 * @ingroup serialization
 * @brief 从 uint32 中提取 ADR JSR
 * Extract the ADR JSR from a uint32
 * DOC_TODO: 含义？
 * DOC_TODO: meaning?
 */
#define DDS_OP_ADR_JSR(o) ((int16_t)((o)&DDS_OP_JMP_MASK))

/**
 * @anchor DDS_OP_ADR_PLM
 * @ingroup serialization
 * @brief 从 uint32 中提取 ADR PLM (Extract the ADR PLM from a uint32)
 * DOC_TODO: meaning?
 */
// 从给定的 uint32 (o) 中提取 ADR PLM 值，并将其转换为 int16_t 类型
// Extract the ADR PLM value from the given uint32 (o) and cast it to int16_t type
#define DDS_OP_ADR_PLM(o) ((int16_t)((o)&DDS_OP_JMP_MASK))

/**
 * @anchor DDS_OP_LENGTH
 * @ingroup serialization
 * @brief 从 uint32 中提取 LENGTH (Extract the LENGTH from a uint32)
 * DOC_TODO: meaning?
 */
// 从给定的 uint32 (o) 中提取 LENGTH 值，并将其转换为 uint16_t 类型
// Extract the LENGTH value from the given uint32 (o) and cast it to uint16_t type
#define DDS_OP_LENGTH(o) ((uint16_t)((o)&DDS_OP_JMP_MASK))

/**
 * @anchor DDS_OP_JUMP
 * @ingroup serialization
 * @brief 从 uint32 中提取 JUMP (Extract the JUMP from a uint32)
 * DOC_TODO: meaning?
 */
// 从给定的 uint32 (o) 中提取 JUMP 值，并将其转换为 int16_t 类型
// Extract the JUMP value from the given uint32 (o) and cast it to int16_t type
#define DDS_OP_JUMP(o) ((int16_t)((o)&DDS_OP_JMP_MASK))

/**
 * @anchor DDS_OP_ADR_JMP
 * @ingroup serialization
 * @brief 从 uint32 中提取 ADR_JMP (Extract the ADR_JMP from a uint32)
 * DOC_TODO: meaning?
 */
// 从给定的 uint32 (o) 中提取 ADR_JMP 值，并将其右移 16 位
// Extract the ADR_JMP value from the given uint32 (o) and shift it right by 16 bits
#define DDS_OP_ADR_JMP(o) ((o) >> 16)

/**
 * @anchor DDS_JEQ_TYPE
 * @ingroup serialization
 * @brief 从 uint32 中提取 JEQ_TYPE，作为 \ref dds_stream_typecode (Extract the JEQ_TYPE from a
 * uint32 as a \ref dds_stream_typecode) DOC_TODO: meaning?
 */
// 从给定的 uint32 (o) 中提取 JEQ_TYPE 值，并将其右移 16 位后转换为 dds_stream_typecode 枚举类型
// Extract the JEQ_TYPE value from the given uint32 (o), shift it right by 16 bits, and cast it to
// the dds_stream_typecode enumeration type
#define DDS_JEQ_TYPE(o) ((enum dds_stream_typecode)(((o)&DDS_JEQ_TYPE_MASK) >> 16))

/**
 * @anchor DDS_JEQ_TYPE_FLAGS
 * @ingroup serialization
 * @brief 从 uint32 中提取 JEQ_TYPE_FLAGS (Extract the JEQ_TYPE_FLAGS from a uint32)
 * DOC_TODO: meaning?
 */
// 从给定的 uint32 (o) 中提取 JEQ_TYPE_FLAGS 值
// Extract the JEQ_TYPE_FLAGS value from the given uint32 (o)
#define DDS_JEQ_TYPE_FLAGS(o) ((o)&DDS_JEQ_TYPE_FLAGS_MASK)

/**
 * @anchor DDS_PLM_FLAGS
 * @ingroup serialization
 * @brief 从 uint32 中提取 PLM_FLAGS，作为 \ref dds_stream_typecode (Extract the PLM_FLAGS from a
 * uint32 as a \ref dds_stream_typecode) DOC_TODO: meaning?
 */
// 从给定的 uint32 (o) 中提取 PLM_FLAGS 值，并将其右移 16 位后转换为 dds_stream_typecode 枚举类型
// Extract the PLM_FLAGS value from the given uint32 (o), shift it right by 16 bits, and cast it to
// the dds_stream_typecode enumeration type
#define DDS_PLM_FLAGS(o) ((enum dds_stream_typecode)(((o)&DDS_PLM_FLAGS_MASK) >> 16))

/**
 * @ingroup serialization
 * @brief Topic encoding instruction types
 */
enum dds_stream_opcode {
  /**
   * @brief 返回子程序，退出顶层 (Return from subroutine, exits top-level)
   * @details 操作码：[RTS,   0,   0, 0] (Opcode: [RTS,   0,   0, 0])
   */
  DDS_OP_RTS = 0x00 << 24,

  /** 数据字段 (data field)
      [ADR, nBY,   0, f] [offset]
      [ADR, BLN,   0, f] [offset]
      [ADR, ENU,   0, f] [offset] [max]
      [ADR, BMK,   0, f] [offset] [bits-high] [bits-low]
      [ADR, STR,   0, f] [offset]
      [ADR, BST,   0, f] [offset] [max-size]

      [ADR, SEQ, nBY, f] [offset]
      [ADR, SEQ, BLN, f] [offset]
      [ADR, SEQ, ENU, f] [offset] [max]
      [ADR, SEQ, BMK, f] [offset] [bits-high] [bits-low]
      [ADR, SEQ, STR, f] [offset]
      [ADR, SEQ, BST, f] [offset] [max-size]
      [ADR, SEQ,   s, f] [offset] [elem-size] [next-insn, elem-insn]
        其中 s = {SEQ,ARR,UNI,STU,BSQ} (where s = {SEQ,ARR,UNI,STU,BSQ})
      [ADR, SEQ, EXT, f] *** 不支持 (not supported)

      [ADR, BSQ, nBY, f] [offset] [sbound]
      [ADR, BSQ, BLN, f] [offset] [sbound]
      [ADR, BSQ, ENU, f] [offset] [sbound] [max]
      [ADR, BSQ, BMK, f] [offset] [sbound] [bits-high] [bits-low]
      [ADR, BSQ, STR, f] [offset] [sbound]
      [ADR, BSQ, BST, f] [offset] [sbound] [max-size]
      [ADR, BSQ,   s, f] [offset] [sbound] [elem-size] [next-insn, elem-insn]
        其中 s = {SEQ,ARR,UNI,STU,BSQ} (where s = {SEQ,ARR,UNI,STU,BSQ})
      [ADR, BSQ, EXT, f] *** 不支持 (not supported)

      [ADR, ARR, nBY, f] [offset] [alen]
      [ADR, ARR, BLN, f] [offset] [alen]
      [ADR, ARR, ENU, f] [offset] [alen] [max]
      [ADR, ARR, BMK, f] [offset] [alen] [bits-high] [bits-low]
      [ADR, ARR, STR, f] [offset] [alen]
      [ADR, ARR, BST, f] [offset] [alen] [0] [max-size]
      [ADR, ARR,   s, f] [offset] [alen] [next-insn, elem-insn] [elem-size]
          其中 s = {SEQ,ARR,UNI,STU,BSQ} (where s = {SEQ,ARR,UNI,STU,BSQ})
      [ADR, ARR, EXT, f] *** 不支持 (not supported)

      [ADR, UNI,   d, z] [offset] [alen] [next-insn, cases]
      [ADR, UNI, ENU, z] [offset] [alen] [next-insn, cases] [max]
      [ADR, UNI, EXT, f] *** 不支持 (not supported)
        其中
          d = {1BY,2BY,4BY,BLN} 的判别类型 (discriminant type of {1BY,2BY,4BY,BLN})
          z = 默认存在/不存在 (default present/not present (DDS_OP_FLAG_DEF))
          offset = 判别量偏移 (discriminant offset)
          max = 最大枚举值 (max enum value)
        接着是 alen 个 case 标签：以 JEQ 格式 (followed by alen case labels: in JEQ format)

      [ADR, e | EXT,   0, f] [offset] [next-insn, elem-insn] [elem-size 当 "external" 标志 e 设置时,
  或者标志 f 具有 DDS_OP_FLAG_OPT] (elem-size iff "external" flag e is set,
  or flag f has DDS_OP_FLAG_OPT)
      [ADR, STU,   0, f] *** 不支持 (not supported) 其中 s            = 子类型 (subtype)
      e            = 外部：作为外部数据（指针）存储 (external: stored as external data (pointer)
  (DDS_OP_FLAG_EXT)) f            = 标志： (flags:)
                      - 键/非键 (key/not key (DDS_OP_FLAG_KEY))
                      - 基本类型成员，与 EXT 类型一起使用 (base type member, used with EXT type
  (DDS_OP_FLAG_BASE))
                      - 可选的 (optional (DDS_OP_FLAG_OPT))
                      - 必须理解 (must-understand (DDS_OP_FLAG_MU))
                      - 存储大小，仅用于 ENU 和 BMK (storage size, only for ENU and BMK (n <<
  DDS_OP_FLAG_SZ_SHIFT)) [offset]     = 内存中元素起始处的字段偏移 (field offset from start of
  element in memory) [elem-size]  = 内存中的元素大小（仅在设置了 'external' 标志时包含
  elem-size）(element size in memory (elem-size is only included in case 'external' flag is set))
      [max-size]   = 字符串边界 + 1 (string bound + 1)
      [max]        = 最大枚举值 (max enum value)
      [bits-..]    = 位掩码中的已识别位，分为高低 32 位 (identified bits in the bitmask, split into
  high and low 32 bits) [alen]       = 数组长度，情况数 (array length, number of cases) [sbound] =
  有界序列最大元素数 (bounded sequence maximum number of elements) [next-insn]  = （无符号 16
  位）下一个字段的指令偏移，从 insn 开始 ((unsigned 16 bits) offset to instruction for next field,
  from start of insn) [elem-insn]  = （无符号 16 位）第一个元素的第一条指令的偏移，从 insn 开始
  ((unsigned 16 bits) offset to first instruction for element, from start of insn) [cases]      =
  （无符号 16 位）第一个案例标签的偏移，从 insn 开始 ((unsigned 16 bits) offset to first case label,
  from start of insn)
   */
  DDS_OP_ADR = 0x01 << 24,

  /**
   * @brief 跳转到子程序（例如用于递归类型和可追加联合）
   *        Jump to subroutine (e.g. used for recursive types and appendable unions)
   * @code
   * [JSR,   0, e]
   * @endcode
   * 其中 (where):
   *   - e = (有符号16位) 子程序第一条指令的偏移量，从指令开始计算
   *         (signed 16 bits) offset to the first instruction in the subroutine, from the start of
   * insn 指令序列必须以 RTS 结束，执行恢复到 JSR 后的指令 The instruction sequence must end with
   * RTS, and execution resumes at the instruction following JSR
   */
  DDS_OP_JSR = 0x02 << 24,

  /**
   * @brief 跳转-如果相等，用于联合体情况（Jump-if-equal, used for union cases）
   *
   * @param JEQ 指令格式为：[JEQ, nBY, 0] [disc] [offset]
   * @param JEQ 指令格式为：[JEQ, BLN, 0] [disc] [offset]
   * @param JEQ 指令格式为：[JEQ, STR, 0] [disc] [offset]
   * @param JEQ 指令格式为：[JEQ, s,   i] [disc] [offset]
   * @param JEQ4 指令格式为：[JEQ4, e | nBY, 0] [disc] [offset] 0
   * @param JEQ4 指令格式为：[JEQ4, e | STR, 0] [disc] [offset] 0
   * @param JEQ4 指令格式为：[JEQ4, e | ENU, f] [disc] [offset] [max]
   * @param JEQ4 指令格式为：[JEQ4, EXT, 0] *** 不支持，使用 STU/UNI 外部定义类型 (Not supported,
   * use STU/UNI for external defined types)
   * @param JEQ4 指令格式为：[JEQ4, e | s, i] [disc] [offset] [elem-size iff "external" flag e is
   * set, else 0]
   *
   * 其中 (where)：
   *   e = 外部：作为外部数据（指针）存储 (external: stored as external data (pointer)
   * (DDS_OP_FLAG_EXT)) s = 除 {nBY,STR} 之外的 JEQ 子类型和 {nBY,STR,ENU,EXT} 之外的 JEQ4 子类型
   *       （注意 BMK 不能内联，因为它需要在位掩码类型中识别的位的两个附加指令）
   *       (subtype other than {nBY,STR} for JEQ and {nBY,STR,ENU,EXT} for JEQ4)
   *   i = （无符号16位）从指令开始到第一个情况的偏移量
   *       指令序列必须以 RTS 结束，在此时执行继续
   *       在联合体指定的下一个字段的指令
   *       ((unsigned 16 bits) offset to first instruction for case, from start of insn)
   *   f = ENU 指令的大小标志 (size flags for ENU instruction)
   *
   * 注意 JEQ 指令已被弃用，并由 JEQ4 指令替换。IDL 编译器仅为联合体情况生成 JEQ4，
   * JEQ 指令包含在此处以向后兼容（使用先前版本的 IDLC 生成的主题描述符）
   * (Note that the JEQ instruction is deprecated and replaced by the JEQ4 instruction. The
   *  IDL compiler only generates JEQ4 for union cases, the JEQ instruction is included here
   *  for backwards compatibility (topic descriptors generated with a previous version of IDLC))
   */
  DDS_OP_JEQ = 0x03 << 24,

  /** XCDR2 delimited CDR (inserts DHEADER before type)
      [DLC, 0, 0]
    */
  // XCDR2 分隔的 CDR（在类型前插入 DHEADER）
  // [DLC, 0, 0]
  DDS_OP_DLC = 0x04 << 24,

  /** XCDR2 parameter list CDR (inserts DHEADER before type and EMHEADER before each member)
       [PLC, 0, 0]
            followed by a list of JEQ instructions
    */
  // XCDR2 参数列表 CDR（在类型前插入 DHEADER，每个成员前插入 EMHEADER）
  // [PLC, 0, 0]
  // 后面跟一系列 JEQ 指令
  DDS_OP_PLC = 0x05 << 24,

  /**
       [PLM,   f, elem-insn] [member id]
         for members of aggregated mutable types (pl-cdr):
         where
           f           = flags:
                         - jump to base type (DDS_OP_FLAG_BASE)
           [elem-insn] = (unsigned 16 bits) offset to instruction for element, from start of insn
                          when FLAG_BASE is set, this is the offset of the PLM list of the base type
           [member id] = id for this member (0 in case FLAG_BASE is set)
    */
  // [PLM,   f, elem-insn] [成员 id]
  // 对于聚合可变类型的成员（pl-cdr）：
  // 其中
  //   f           = 标志：
  //                 - 跳转到基本类型（DDS_OP_FLAG_BASE）
  //   [elem-insn] = （无符号 16 位）元素指令的偏移量，从 insn 开始
  //                 当 FLAG_BASE 设置时，这是基本类型的 PLM 列表的偏移量
  //   [成员 id]    = 此成员的 id（如果设置了 FLAG_BASE，则为 0）
  DDS_OP_PLM = 0x06 << 24,

  /** Key offset list
       [KOF, 0, n] [offset-1] ... [offset-n]
         where
          n      = number of key offsets in following ops
          offset = Offset of the key field relative to the previous offset, repeated n times when
     key is in a nested struct. In case of inheritance of mutable structs, a single offset of the
     key member relative to the first op of the top-level type (index 0).
    */
  // 密钥偏移列表
  // [KOF, 0, n] [offset-1] ... [offset-n]
  // 其中
  //   n      = 后续操作中密钥偏移量的数量
  //   offset = 密钥字段相对于前一个偏移量的偏移量，在密钥位于嵌套结构时重复 n
  //   次。在可变结构继承的情况下，密钥成员相对于顶级类型的第一个操作（索引 0）的单个偏移量。
  DDS_OP_KOF = 0x07 << 24,

  /** see comment for JEQ/JEQ4 above */
  // 参见上面 JEQ/JEQ4 的注释
  DDS_OP_JEQ4 = 0x08 << 24
};

/**
 * @ingroup serialization
 * @brief 序列化虚拟机识别的数据类型。
 * @brief datatypes as recognized by serialization VM.
 */
enum dds_stream_typecode {
  DDS_OP_VAL_1BY = 0x01, /**< 一个字节的简单类型 (char, octet) */
                         /**< one byte simple type (char, octet) */
  DDS_OP_VAL_2BY = 0x02, /**< 两个字节的简单类型 ((unsigned) short) */
                         /**< two byte simple type ((unsigned) short) */
  DDS_OP_VAL_4BY = 0x03, /**< 四个字节的简单类型 ((unsigned) long, float) */
                         /**< four byte simple type ((unsigned) long, float) */
  DDS_OP_VAL_8BY = 0x04, /**< 八个字节的简单类型 ((unsigned) long long, double) */
                         /**< eight byte simple type ((unsigned) long long, double) */
  DDS_OP_VAL_STR = 0x05, /**< 字符串 */
                         /**< string */
  DDS_OP_VAL_BST = 0x06, /**< 有界字符串 */
                         /**< bounded string */
  DDS_OP_VAL_SEQ = 0x07, /**< 序列 */
                         /**< sequence */
  DDS_OP_VAL_ARR = 0x08, /**< 数组 */
                         /**< array */
  DDS_OP_VAL_UNI = 0x09, /**< 联合体 */
                         /**< union */
  DDS_OP_VAL_STU = 0x0a, /**< 结构体 */
                         /**< struct */
  DDS_OP_VAL_BSQ = 0x0b, /**< 有界序列 */
                         /**< bounded sequence */
  DDS_OP_VAL_ENU = 0x0c, /**< 枚举值 (long) */
                         /**< enumerated value (long) */
  DDS_OP_VAL_EXT = 0x0d, /**< 带有外部定义的字段 */
                         /**< field with external definition */
  DDS_OP_VAL_BLN = 0x0e, /**< 布尔值 */
                         /**< boolean */
  DDS_OP_VAL_BMK = 0x0f  /**< 位掩码 */
                         /**< bitmask */
};

/**
 * @ingroup serialization
 * @brief DDS_OP_ADR、DDS_OP_JEQ 的主类型代码
 * @brief primary type code for DDS_OP_ADR, DDS_OP_JEQ
 * 方便预位移值。
 * Convinience pre-bitshifted values.
 */
enum dds_stream_typecode_primary {
  DDS_OP_TYPE_1BY = DDS_OP_VAL_1BY << 16, /**< 一个字节的简单类型 (char, octet) */
                                          /**< one byte simple type (char, octet) */
  DDS_OP_TYPE_2BY = DDS_OP_VAL_2BY << 16, /**< 两个字节的简单类型 ((unsigned) short) */
                                          /**< two byte simple type ((unsigned) short) */
  DDS_OP_TYPE_4BY = DDS_OP_VAL_4BY << 16, /**< 四个字节的简单类型 ((unsigned) long, float) */
                                          /**< four byte simple type ((unsigned) long, float) */
  DDS_OP_TYPE_8BY = DDS_OP_VAL_8BY << 16, /**< 八个字节的简单类型 ((unsigned) long long, double) */
  /**< eight byte simple type ((unsigned) long long, double) */
  DDS_OP_TYPE_STR = DDS_OP_VAL_STR << 16, /**< 字符串 */
                                          /**< string */
  DDS_OP_TYPE_BST = DDS_OP_VAL_BST << 16, /**< 有界字符串 */
                                          /**< bounded string */
  DDS_OP_TYPE_SEQ = DDS_OP_VAL_SEQ << 16, /**< 序列 */
                                          /**< sequence */
  DDS_OP_TYPE_ARR = DDS_OP_VAL_ARR << 16, /**< 数组 */
                                          /**< array */
  DDS_OP_TYPE_UNI = DDS_OP_VAL_UNI << 16, /**< 联合体 */
                                          /**< union */
  DDS_OP_TYPE_STU = DDS_OP_VAL_STU << 16, /**< 结构体 */
                                          /**< struct */
  DDS_OP_TYPE_BSQ = DDS_OP_VAL_BSQ << 16, /**< 有界序列 */
                                          /**< bounded sequence */
  DDS_OP_TYPE_ENU = DDS_OP_VAL_ENU << 16, /**< 枚举值 (long) */
                                          /**< enumerated value (long) */
  DDS_OP_TYPE_EXT = DDS_OP_VAL_EXT << 16, /**< 带有外部定义的字段 */
                                          /**< field with external definition */
  DDS_OP_TYPE_BLN = DDS_OP_VAL_BLN << 16, /**< 布尔值 */
                                          /**< boolean */
  DDS_OP_TYPE_BMK = DDS_OP_VAL_BMK << 16  /**< 位掩码 */
                                          /**< bitmask */
};

/**
 * @anchor DDS_OP_FLAG_EXT
 * @ingroup serialization
 * @brief 此标志表示类型具有外部数据
 * @brief This flag indicates that the type has external data
 * (即映射到指针类型)，这可能是因为 (1) idl 中的 \@external 注释
 * (i.e. a mapped to a pointer type), which can be the case because of (1) the \@external annotation
 * 或者 (2) \@optional 注释（如 XTypes 规范中所述，可选字段也映射到指针类型）。
 * in idl or (2) the \@optional annotation (optional fields are also mapped to pointer types as
 * described in the XTypes spec). 此标志存储在序列化器指令的“类型”部分的最高有效位。
 * This flag is stored in the most-significant bit of the 'type' part
 * of the serializer instruction.
 */
#define DDS_OP_FLAG_EXT (1u << 23)

/**
 * @ingroup serialization
 * @brief 子类型代码（Sub-type code）
 *  - 为 DDS_OP_TYPE_{SEQ,ARR} 编码元素类型（Encodes element type）
 *  - 为 DDS_OP_TYPE_UNI 编码辨别类型（Discriminant type）
 * 方便的预位移值（Convenience pre-bitshifted values）。
 */
enum dds_stream_typecode_subtype {
  DDS_OP_SUBTYPE_1BY =
      DDS_OP_VAL_1BY
      << 8, /**< 一个字节的简单类型（char，octet）(One byte simple type (char, octet)) */
  DDS_OP_SUBTYPE_2BY =
      DDS_OP_VAL_2BY
      << 8, /**< 两个字节的简单类型（（unsigned）short）(Two byte simple type ((unsigned) short)) */
  DDS_OP_SUBTYPE_4BY = DDS_OP_VAL_4BY << 8, /**< 四个字节的简单类型（（unsigned）long，float）(Four
                                               byte simple type ((unsigned) long, float)) */
  DDS_OP_SUBTYPE_8BY = DDS_OP_VAL_8BY
                       << 8, /**< 八个字节的简单类型（（unsigned）long long，double）(Eight byte
                                simple type ((unsigned) long long, double)) */
  DDS_OP_SUBTYPE_STR = DDS_OP_VAL_STR << 8, /**< 字符串（String） */
  DDS_OP_SUBTYPE_BST = DDS_OP_VAL_BST << 8, /**< 有界字符串（Bounded string） */
  DDS_OP_SUBTYPE_SEQ = DDS_OP_VAL_SEQ << 8, /**< 序列（Sequence） */
  DDS_OP_SUBTYPE_ARR = DDS_OP_VAL_ARR << 8, /**< 数组（Array） */
  DDS_OP_SUBTYPE_UNI = DDS_OP_VAL_UNI << 8, /**< 联合体（Union） */
  DDS_OP_SUBTYPE_STU = DDS_OP_VAL_STU << 8, /**< 结构体（Struct） */
  DDS_OP_SUBTYPE_BSQ = DDS_OP_VAL_BSQ << 8, /**< 有界序列（Bounded sequence） */
  DDS_OP_SUBTYPE_ENU = DDS_OP_VAL_ENU << 8, /**< 枚举值（long）(Enumerated value (long)) */
  DDS_OP_SUBTYPE_BLN = DDS_OP_VAL_BLN << 8, /**< 布尔值（Boolean） */
  DDS_OP_SUBTYPE_BMK = DDS_OP_VAL_BMK << 8  /**< 位掩码（Bitmask） */
};

/**
 * @anchor DDS_OP_FLAG_KEY
 * @ingroup serialization
 * @brief 标记字段为键（Mark field as key）
 * 适用于 {1,2,4,8}BY，STR，BST，ARR-of-{1,2,4,8}BY。
 * 注意，在定义嵌套类型中的键时，应在子类型中的字段和封闭的 STU/EXT 字段上设置键标志。
 * Applicable to {1,2,4,8}BY, STR, BST, ARR-of-{1,2,4,8}BY.
 * Note that when defining keys in nested types, the key flag should be set
 * on both the field(s) in the subtype and on the enclosing STU/EXT field.
 */
#define DDS_OP_FLAG_KEY (1u << 0)

/**
 * @anchor DDS_OP_FLAG_DEF
 * @ingroup serialization
 * @brief 对于一个联合体(union)：
 *  -# 判别器(discriminator)可能是一个关键字段(key field)；
 *  -# 可能有一个默认值(default value)；
 *  -# 判别器可以是整数类型(integral type)（或枚举类型(enumerated) - 在这里被视为等价的）。
 * 判别器不能是浮点类型(floating-point type)。因此，DEF 和 FP 不需要同时设置。
 * 标志位只有几个，所以节省一个也不是什么坏主意。
 *
 * union has a default case (for DDS_OP_ADR | DDS_OP_TYPE_UNI)
 */
#define DDS_OP_FLAG_DEF (1u << 1)

/**
 * @anchor DDS_OP_FLAG_FP
 * @ingroup serialization
 * @brief 浮点类型(floating-point),
 * 适用于 {4,8}BY 和数组，它们的序列(sequence)
 */
#define DDS_OP_FLAG_FP (1u << 1)

/**
 * @anchor DDS_OP_FLAG_SGN
 * @ingroup serialization
 * @brief 有符号类型(signed),
 * 适用于 {1,2,4,8}BY 和数组，它们的序列(sequence)
 */
#define DDS_OP_FLAG_SGN (1u << 2)

/**
 * @anchor DDS_OP_FLAG_MU
 * @ingroup serialization
 * @brief 必须理解标志(must-understand flag),
 * 根据 XTypes 规范定义。
 */
#define DDS_OP_FLAG_MU (1u << 3)

/**
 * @anchor DDS_OP_FLAG_BASE
 * @ingroup serialization
 * @brief 跳转到基本类型(jump to base type),
 * 与 PLM 在可变类型(mutable types)中使用，以及在最终(final)和可附加(appendable)类型的 TYPE_EXT
 * 'parent' 成员中使用
 */
#define DDS_OP_FLAG_BASE (1u << 4)

/**
 * @anchor DDS_OP_FLAG_OPT
 * @ingroup serialization
 * @brief 可选标志(optional flag),
 * 用于结构体(struct)成员。对于非字符串类型(non-string types)，可选成员还会获得 FLAG_EXT，如上所述。
 */
#define DDS_OP_FLAG_OPT (1u << 5)

/**
 * @anchor DDS_OP_FLAG_SZ_SHIFT
 * @ingroup serialization
 * @brief 枚举(Enum)和位掩码(bitmask)存储大小（移位(shift)数量）
 */
#define DDS_OP_FLAG_SZ_SHIFT (6)

/**
 * @anchor DDS_OP_FLAG_SZ_MASK
 * @ingroup serialization
 * @brief 枚举(Enum)和位掩码(bitmask)存储大小
 *  -# 00 = 1 字节(byte)，
 *  -# 01 = 2 字节(bytes)，
 *  -# 10 = 4 字节(bytes)，
 *  -# 11 = 8 字节(bytes)（仅限位掩码(bitmask)）
 */
#define DDS_OP_FLAG_SZ_MASK (3u << DDS_OP_FLAG_SZ_SHIFT)

/**
 * @anchor DDS_OP_FLAGS_SZ
 * @ingroup serialization
 * @brief 从标志(flags)中提取枚举(Enum)和位掩码(bitmask)存储大小
 */
#define DDS_OP_FLAGS_SZ(f) (1u << (((f)&DDS_OP_FLAG_SZ_MASK) >> DDS_OP_FLAG_SZ_SHIFT))

/**
 * @anchor DDS_OP_TYPE_SZ
 * @ingroup serialization
 * @brief 从 uint32 中提取枚举(Enum)和位掩码(bitmask)存储大小
 */
#define DDS_OP_TYPE_SZ(o) DDS_OP_FLAGS_SZ(DDS_OP_FLAGS(o))

/**
 * @defgroup topic_flags (Topic flags)
 * @ingroup serialization
 * IDL 编译器可以向 dds_topic_descriptor_t 添加几个标志。
 */

/**
 * @anchor DDS_TOPIC_NO_OPTIMIZE
 * @ingroup topic_flags
 * @brief 不要优化 opcodes 以快速 memcpy's。
 * 如果一个主题仅包含简单类型(plain types)和直接嵌套的结构体(directly nested structs)，
 * 那么简单的复制(copy)+可选字节交换(byteswap)就可以直接从 CDR 生成正确的
 * C 内存布局。如果添加了此标志，表示在解码类型时涉及指针 'magic'，不应采取任何捷径。
 * @deprecated 当为类型创建 sertype_default 时，将在运行时确定可优化性，不使用此标志。
 */
#define DDS_TOPIC_NO_OPTIMIZE (1u << 0)

/**
 * @anchor DDS_TOPIC_FIXED_KEY
 * @ingroup topic_flags
 * @brief XCDRV1 序列化键(serialized key)适用于 16 字节。
 * 如果静态确定密钥始终适用于 16 字节
 * 规范指定样本的密钥是生成的 CDR。
 * 如果它更长，我们必须使用 MD5 对生成的密钥 CDR 进行哈希。
 */
#define DDS_TOPIC_FIXED_KEY (1u << 1)

/**
 * @anchor DDS_TOPIC_CONTAINS_UNION
 * @ingroup topic_flags
 * @brief 在任意深度嵌套下，主题类型至少包含一个联合体(union)。
 */
#define DDS_TOPIC_CONTAINS_UNION (1u << 2)

// (1u << 3) unused, was used for DDS_TOPIC_DISABLE_TYPECHECK

/**
 * @anchor DDS_TOPIC_FIXED_SIZE
 * @ingroup topic_flags
 * @brief 此主题类型的样本在内存中的大小是完全固定的。
 */
#define DDS_TOPIC_FIXED_SIZE (1u << 4)

/**
 * @anchor DDS_TOPIC_FIXED_KEY_XCDR2
 * @ingroup topic_flags
 * @brief XCDRV2 序列化键(serialized key)适用于 16 字节。
 * 如果静态确定密钥始终适用于 16 字节
 * 规范指定样本的密钥是生成的 CDR。
 * 如果它更长，我们必须使用 MD5 对生成的密钥 CDR 进行哈希。
 *
 * 由于 8 字节类型对齐方式的变化，此版本与 XCDRV1 版本分开，因为它可能不同。
 */
#define DDS_TOPIC_FIXED_KEY_XCDR2 (1u << 5)

/**
 * @anchor DDS_TOPIC_XTYPES_METADATA
 * @ingroup topic_flags
 * @brief 如果此主题存在 XTypes 元数据，则设置
 */
#define DDS_TOPIC_XTYPES_METADATA (1u << 6)

/**
 * @anchor DDS_TOPIC_RESTRICT_DATA_REPRESENTATION
 * @ingroup topic_flags
 * @brief 如果主题描述符中存在顶级类型的数据表示限制，则设置
 */
#define DDS_TOPIC_RESTRICT_DATA_REPRESENTATION (1u << 7)

/**
 * @anchor DDS_FIXED_KEY_MAX_SIZE
 * @ingroup topic_flags
 * @brief 固定密钥的最大大小
 */
#define DDS_FIXED_KEY_MAX_SIZE (16)

#if defined(__cplusplus)
}
#endif

#endif /* DDS_OPCODES_H */
