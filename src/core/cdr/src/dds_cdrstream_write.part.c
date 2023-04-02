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

/**
 * @brief 将数据写入输出流的实现函数（大端字节序）
 *
 * @param os 输出流指针，限定符为__restrict
 * @param allocator 分配器指针，限定符为__restrict
 * @param data 数据指针，限定符为__restrict
 * @param ops 操作指针，限定符为__restrict
 * @param is_mutable_member 是否为可变成员
 * @return 返回操作指针
 */
static const uint32_t *dds_stream_write_implBO(
    DDS_OSTREAM_T *__restrict os,
    const struct dds_cdrstream_allocator *__restrict allocator,
    const char *__restrict data,
    const uint32_t *__restrict ops,
    bool is_mutable_member);

/**
 * @brief 写入布尔值到输出流（大端字节序）
 *
 * @param os 输出流指针，限定符为__restrict
 * @param allocator 分配器指针，限定符为__restrict
 * @param val 要写入的布尔值
 * @return 如果成功写入，则返回true，否则返回false
 */
static inline bool dds_stream_write_bool_valueBO(
    DDS_OSTREAM_T *__restrict os,
    const struct dds_cdrstream_allocator *__restrict allocator,
    const uint8_t val) {
  // 检查布尔值是否有效
  if (val > 1) return false;
  // 将布尔值写入输出流
  dds_os_put1BO(os, allocator, val);
  return true;
}

/**
 * @brief 写入枚举值到输出流（大端字节序）
 *
 * @param os 输出流指针，限定符为__restrict
 * @param allocator 分配器指针，限定符为__restrict
 * @param insn 操作数
 * @param val 要写入的枚举值
 * @param max 枚举值的最大值
 * @return 如果成功写入，则返回true，否则返回false
 */
static bool dds_stream_write_enum_valueBO(
    DDS_OSTREAM_T *__restrict os,
    const struct dds_cdrstream_allocator *__restrict allocator,
    uint32_t insn,
    uint32_t val,
    uint32_t max) {
  // 检查枚举值是否有效
  if (val > max) return false;
  // 根据操作数类型大小写入枚举值
  switch (DDS_OP_TYPE_SZ(insn)) {
    case 1:
      dds_os_put1BO(os, allocator, (uint8_t)val);
      break;
    case 2:
      dds_os_put2BO(os, allocator, (uint16_t)val);
      break;
    case 4:
      dds_os_put4BO(os, allocator, val);
      break;
    default:
      // 遇到未知操作类型，中止程序
      abort();
  }
  return true;
}

/**
 * @brief 将位掩码值写入数据流
 *
 * @param[in] os 指向DDS_OSTREAM_T类型的指针，用于输出数据流
 * @param[in] allocator 指向dds_cdrstream_allocator结构体的指针，用于分配内存
 * @param[in] insn 32位无符号整数，表示指令
 * @param[in] addr 指向void类型的指针，表示数据地址
 * @param[in] bits_h 32位无符号整数，表示高位掩码
 * @param[in] bits_l 32位无符号整数，表示低位掩码
 * @return 如果成功写入位掩码值，则返回true，否则返回false
 */
static bool dds_stream_write_bitmask_valueBO(
    DDS_OSTREAM_T *__restrict os,
    const struct dds_cdrstream_allocator *__restrict allocator,
    uint32_t insn,
    const void *__restrict addr,
    uint32_t bits_h,
    uint32_t bits_l) {
  // 根据指令的大小选择相应的处理方式
  switch (DDS_OP_TYPE_SZ(insn)) {
    case 1: {
      // 将地址转换为8位无符号整数指针
      const uint8_t *ptr = (const uint8_t *)addr;
      // 检查位掩码值是否有效
      if (!bitmask_value_valid(*ptr, bits_h, bits_l)) return false;
      // 将位掩码值写入数据流
      dds_os_put1BO(os, allocator, *ptr);
      break;
    }
    case 2: {
      // 将地址转换为16位无符号整数指针
      const uint16_t *ptr = (const uint16_t *)addr;
      // 检查位掩码值是否有效
      if (!bitmask_value_valid(*ptr, bits_h, bits_l)) return false;
      // 将位掩码值写入数据流
      dds_os_put2BO(os, allocator, *ptr);
      break;
    }
    case 4: {
      // 将地址转换为32位无符号整数指针
      const uint32_t *ptr = (const uint32_t *)addr;
      // 检查位掩码值是否有效
      if (!bitmask_value_valid(*ptr, bits_h, bits_l)) return false;
      // 将位掩码值写入数据流
      dds_os_put4BO(os, allocator, *ptr);
      break;
    }
    case 8: {
      // 将地址转换为64位无符号整数指针
      const uint64_t *ptr = (const uint64_t *)addr;
      // 检查位掩码值是否有效
      if (!bitmask_value_valid(*ptr, bits_h, bits_l)) return false;
      // 将位掩码值写入数据流
      dds_os_put8BO(os, allocator, *ptr);
      break;
    }
    default:
      // 如果指令大小不在1、2、4、8之间，则终止程序
      abort();
  }
  // 成功写入位掩码值，返回true
  return true;
}

/**
 * @brief 将字符串写入DDS流中
 *
 * @param[in] os 指向DDS输出流的指针
 * @param[in] allocator 指向dds_cdrstream_allocator结构体的指针
 * @param[in] val 要写入的字符串
 */
static void dds_stream_write_stringBO(DDS_OSTREAM_T *__restrict os,
                                      const struct dds_cdrstream_allocator *__restrict allocator,
                                      const char *__restrict val) {
  // 计算字符串长度，如果val为NULL，则长度为1
  uint32_t size = val ? (uint32_t)strlen(val) + 1 : 1;

  // 将字符串长度写入DDS流中
  dds_os_put4BO(os, allocator, size);

  // 如果val不为NULL，则将字符串内容写入DDS流中
  if (val)
    dds_os_put_bytes((struct dds_ostream *)os, allocator, val, size);
  else
    dds_os_put1BO(os, allocator, 0);  // 否则，只写入一个字节的0
}

/**
 * @brief 将布尔数组写入DDS流中
 *
 * @param[in] os 指向DDS输出流的指针
 * @param[in] allocator 指向dds_cdrstream_allocator结构体的指针
 * @param[in] addr 布尔数组的地址
 * @param[in] num 数组元素个数
 * @return 成功返回true，失败返回false
 */
static bool dds_stream_write_bool_arrBO(DDS_OSTREAM_T *__restrict os,
                                        const struct dds_cdrstream_allocator *__restrict allocator,
                                        const uint8_t *__restrict addr,
                                        uint32_t num) {
  // 遍历布尔数组
  for (uint32_t i = 0; i < num; i++) {
    // 将布尔值写入DDS流中，如果失败则返回false
    if (!dds_stream_write_bool_valueBO(os, allocator, addr[i])) return false;
  }

  // 成功返回true
  return true;
}

/**
 * @brief 将枚举数组写入DDS流中
 *
 * @param[in] os 指向DDS输出流的指针
 * @param[in] allocator 指向dds_cdrstream_allocator结构体的指针
 * @param[in] insn 枚举类型的大小信息
 * @param[in] addr 枚举数组的地址
 * @param[in] num 数组元素个数
 * @param[in] max 枚举类型的最大值
 * @return 成功返回true，失败返回false
 */
static bool dds_stream_write_enum_arrBO(DDS_OSTREAM_T *__restrict os,
                                        const struct dds_cdrstream_allocator *__restrict allocator,
                                        uint32_t insn,
                                        const uint32_t *__restrict addr,
                                        uint32_t num,
                                        uint32_t max) {
  // 根据枚举类型的大小进行不同的处理
  switch (DDS_OP_TYPE_SZ(insn)) {
    case 1:
      for (uint32_t i = 0; i < num; i++) {
        // 如果枚举值超过最大值，则返回false
        if (addr[i] > max) return false;

        // 将枚举值写入DDS流中
        dds_os_put1BO(os, allocator, (uint8_t)addr[i]);
      }
      break;
    case 2:
      for (uint32_t i = 0; i < num; i++) {
        // 如果枚举值超过最大值，则返回false
        if (addr[i] > max) return false;

        // 将枚举值写入DDS流中
        dds_os_put2BO(os, allocator, (uint16_t)addr[i]);
      }
      break;
    case 4:
      for (uint32_t i = 0; i < num; i++) {
        // 如果枚举值超过最大值，则返回false
        if (addr[i] > max) return false;

        // 将枚举值写入DDS流中
        dds_os_put4BO(os, allocator, addr[i]);
      }
      break;
    default:
      abort();  // 不支持的类型大小，终止程序
  }

  // 成功返回true
  return true;
}

/**
 * @brief 将位掩码数组写入数据流
 *
 * @param[in] os 指向DDS_OSTREAM_T类型的指针，用于输出数据流
 * @param[in] allocator 指向dds_cdrstream_allocator结构体的指针，用于内存分配
 * @param[in] insn 32位无符号整数，表示操作指令
 * @param[in] addr 指向void类型的指针，表示位掩码数组的地址
 * @param[in] num 32位无符号整数，表示数组元素的数量
 * @param[in] bits_h 32位无符号整数，表示高位掩码
 * @param[in] bits_l 32位无符号整数，表示低位掩码
 * @return bool 返回true表示成功写入，返回false表示失败
 */
static bool dds_stream_write_bitmask_arrBO(
    DDS_OSTREAM_T *__restrict os,
    const struct dds_cdrstream_allocator *__restrict allocator,
    uint32_t insn,
    const void *__restrict addr,
    uint32_t num,
    uint32_t bits_h,
    uint32_t bits_l) {
  // 根据操作指令的大小选择相应的处理方式
  switch (DDS_OP_TYPE_SZ(insn)) {
    case 1: {
      // 将地址转换为8位无符号整数指针
      const uint8_t *ptr = (const uint8_t *)addr;
      // 遍历数组元素
      for (uint32_t i = 0; i < num; i++) {
        // 检查当前值是否有效
        if (!bitmask_value_valid(ptr[i], bits_h, bits_l)) return false;
        // 将有效值写入数据流
        dds_os_put1BO(os, allocator, ptr[i]);
      }
      break;
    }
    case 2: {
      // 将地址转换为16位无符号整数指针
      const uint16_t *ptr = (const uint16_t *)addr;
      // 遍历数组元素
      for (uint32_t i = 0; i < num; i++) {
        // 检查当前值是否有效
        if (!bitmask_value_valid(ptr[i], bits_h, bits_l)) return false;
        // 将有效值写入数据流
        dds_os_put2BO(os, allocator, ptr[i]);
      }
      break;
    }
    case 4: {
      // 将地址转换为32位无符号整数指针
      const uint32_t *ptr = (const uint32_t *)addr;
      // 遍历数组元素
      for (uint32_t i = 0; i < num; i++) {
        // 检查当前值是否有效
        if (!bitmask_value_valid(ptr[i], bits_h, bits_l)) return false;
        // 将有效值写入数据流
        dds_os_put4BO(os, allocator, ptr[i]);
      }
      break;
    }
    case 8: {
      // 将地址转换为64位无符号整数指针
      const uint64_t *ptr = (const uint64_t *)addr;
      // 遍历数组元素
      for (uint32_t i = 0; i < num; i++) {
        // 检查当前值是否有效
        if (!bitmask_value_valid(ptr[i], bits_h, bits_l)) return false;
        // 将有效值写入数据流
        dds_os_put8BO(os, allocator, ptr[i]);
      }
      break;
    }
    default:
      // 如果操作指令的大小不在1、2、4、8之间，则终止程序
      abort();
  }
  // 成功写入数据流，返回true
  return true;
}

/**
 * @brief 将序列数据写入DDS流，支持字节顺序转换。
 *
 * @param[in] os 指向DDS输出流的指针。
 * @param[in] allocator 指向dds_cdrstream_allocator结构体的指针，用于内存分配。
 * @param[in] addr 序列数据的起始地址。
 * @param[in] ops 操作码数组的指针。
 * @param[in] insn 当前操作码。
 * @return 返回处理后的操作码数组指针。
 */
static const uint32_t *dds_stream_write_seqBO(
    DDS_OSTREAM_T *__restrict os,
    const struct dds_cdrstream_allocator *__restrict allocator,
    const char *__restrict addr,
    const uint32_t *__restrict ops,
    uint32_t insn) {
  // 获取序列数据的指针
  const dds_sequence_t *const seq = (const dds_sequence_t *)addr;
  uint32_t offs = 0, xcdrv = ((struct dds_ostream *)os)->m_xcdr_version;

  // 获取子类型
  const enum dds_stream_typecode subtype = DDS_OP_SUBTYPE(insn);
  uint32_t bound_op = seq_is_bounded(DDS_OP_TYPE(insn)) ? 1 : 0;
  uint32_t bound = bound_op ? ops[2] : 0;

  // 判断是否需要DHEADER
  if (is_dheader_needed(subtype, xcdrv)) {
    // 为DHEADER预留空间
    dds_os_reserve4BO(os, allocator);
    offs = ((struct dds_ostream *)os)->m_index;
  }

  // 获取序列长度
  const uint32_t num = seq->_length;
  // 检查序列长度是否超过边界
  if (bound && num > bound) {
    dds_ostreamBO_fini(os, allocator);
    return NULL;
  }

  // 将序列长度写入DDS流
  dds_os_put4BO(os, allocator, num);

  // 处理空序列的情况
  if (num == 0) {
    ops = skip_sequence_insns(insn, ops);
  } else {
    // 根据子类型处理序列数据
    switch (subtype) {
      case DDS_OP_VAL_BLN:
        if (!dds_stream_write_bool_arrBO(os, allocator, (const uint8_t *)seq->_buffer, num))
          return NULL;
        ops += 2 + bound_op;
        break;
      case DDS_OP_VAL_1BY:
      case DDS_OP_VAL_2BY:
      case DDS_OP_VAL_4BY:
      case DDS_OP_VAL_8BY: {
        const uint32_t elem_size = get_primitive_size(subtype);
        const align_t align = dds_cdr_get_align(xcdrv, elem_size);
        void *dst;
        // 将字节数据写入DDS流并进行字节顺序转换
        dds_os_put_bytes_aligned((struct dds_ostream *)os, allocator, seq->_buffer, num, elem_size,
                                 align, &dst);
        dds_stream_to_BO_insitu(dst, elem_size, num);
        ops += 2 + bound_op;
        break;
      }
      case DDS_OP_VAL_ENU:
        if (!dds_stream_write_enum_arrBO(os, allocator, insn, (const uint32_t *)seq->_buffer, num,
                                         ops[2 + bound_op]))
          return NULL;
        ops += 3 + bound_op;
        break;
      case DDS_OP_VAL_BMK: {
        if (!dds_stream_write_bitmask_arrBO(os, allocator, insn, seq->_buffer, num,
                                            ops[2 + bound_op], ops[3 + bound_op]))
          return NULL;
        ops += 4 + bound_op;
        break;
      }
      case DDS_OP_VAL_STR: {
        const char **ptr = (const char **)seq->_buffer;
        for (uint32_t i = 0; i < num; i++) dds_stream_write_stringBO(os, allocator, ptr[i]);
        ops += 2 + bound_op;
        break;
      }
      case DDS_OP_VAL_BST: {
        const char *ptr = (const char *)seq->_buffer;
        const uint32_t elem_size = ops[2 + bound_op];
        for (uint32_t i = 0; i < num; i++)
          dds_stream_write_stringBO(os, allocator, ptr + i * elem_size);
        ops += 3 + bound_op;
        break;
      }
      case DDS_OP_VAL_SEQ:
      case DDS_OP_VAL_BSQ:
      case DDS_OP_VAL_ARR:
      case DDS_OP_VAL_UNI:
      case DDS_OP_VAL_STU: {
        const uint32_t elem_size = ops[2 + bound_op];
        const uint32_t jmp = DDS_OP_ADR_JMP(ops[3 + bound_op]);
        uint32_t const *const jsr_ops = ops + DDS_OP_ADR_JSR(ops[3 + bound_op]);
        const char *ptr = (const char *)seq->_buffer;
        for (uint32_t i = 0; i < num; i++)
          if (!dds_stream_write_implBO(os, allocator, ptr + i * elem_size, jsr_ops, false))
            return NULL;
        ops += (jmp ? jmp : (4 + bound_op)); /* FIXME: why would jmp be 0? */
        break;
      }
      case DDS_OP_VAL_EXT:
        abort(); /* op type EXT as sequence subtype not supported */
        return NULL;
    }
  }

  // 写入DHEADER
  if (is_dheader_needed(subtype, xcdrv))
    *((uint32_t *)(((struct dds_ostream *)os)->m_buffer + offs - 4)) =
        to_BO4u(((struct dds_ostream *)os)->m_index - offs);

  return ops;
}
/**
 * @brief 将数组数据写入DDS流，支持不同类型的数组元素。
 *
 * @param[in] os 指向DDS输出流的指针。
 * @param[in] allocator 指向dds_cdrstream_allocator结构体的指针，用于内存分配。
 * @param[in] addr 指向要写入的数组数据的指针。
 * @param[in] ops 指向操作序列的指针，用于描述如何处理数据。
 * @param[in] insn 包含有关数组元素类型的信息。
 * @return 返回指向下一个操作的指针。
 */
static const uint32_t *dds_stream_write_arrBO(
    DDS_OSTREAM_T *__restrict os,
    const struct dds_cdrstream_allocator *__restrict allocator,
    const char *__restrict addr,
    const uint32_t *__restrict ops,
    uint32_t insn) {
  // 获取数组元素的子类型
  const enum dds_stream_typecode subtype = DDS_OP_SUBTYPE(insn);
  uint32_t offs = 0, xcdrv = ((struct dds_ostream *)os)->m_xcdr_version;

  // 判断是否需要DHEADER
  if (is_dheader_needed(subtype, xcdrv)) {
    // 为DHEADER预留空间
    dds_os_reserve4BO(os, allocator);
    offs = ((struct dds_ostream *)os)->m_index;
  }

  // 获取数组元素数量
  const uint32_t num = ops[2];

  // 根据子类型处理数组元素
  switch (subtype) {
    case DDS_OP_VAL_BLN:
      if (!dds_stream_write_bool_arrBO(os, allocator, (const uint8_t *)addr, num)) return NULL;
      ops += 3;
      break;
    case DDS_OP_VAL_1BY:
    case DDS_OP_VAL_2BY:
    case DDS_OP_VAL_4BY:
    case DDS_OP_VAL_8BY: {
      const uint32_t elem_size = get_primitive_size(subtype);
      const align_t align = dds_cdr_get_align(xcdrv, elem_size);
      void *dst;

      // 将字节序列写入输出流，大多数情况下不需要交换字节序
      dds_os_put_bytes_aligned((struct dds_ostream *)os, allocator, addr, num, elem_size, align,
                               &dst);
      dds_stream_to_BO_insitu(dst, elem_size, num);
      ops += 3;
      break;
    }
    case DDS_OP_VAL_ENU:
      if (!dds_stream_write_enum_arrBO(os, allocator, insn, (const uint32_t *)addr, num, ops[3]))
        return NULL;
      ops += 4;
      break;
    case DDS_OP_VAL_BMK:
      if (!dds_stream_write_bitmask_arrBO(os, allocator, insn, addr, num, ops[3], ops[4]))
        return NULL;
      ops += 5;
      break;
    case DDS_OP_VAL_STR: {
      const char **ptr = (const char **)addr;
      for (uint32_t i = 0; i < num; i++) dds_stream_write_stringBO(os, allocator, ptr[i]);
      ops += 3;
      break;
    }
    case DDS_OP_VAL_BST: {
      const char *ptr = (const char *)addr;
      const uint32_t elem_size = ops[4];
      for (uint32_t i = 0; i < num; i++)
        dds_stream_write_stringBO(os, allocator, ptr + i * elem_size);
      ops += 5;
      break;
    }
    case DDS_OP_VAL_SEQ:
    case DDS_OP_VAL_BSQ:
    case DDS_OP_VAL_ARR:
    case DDS_OP_VAL_UNI:
    case DDS_OP_VAL_STU: {
      const uint32_t *jsr_ops = ops + DDS_OP_ADR_JSR(ops[3]);
      const uint32_t jmp = DDS_OP_ADR_JMP(ops[3]);
      const uint32_t elem_size = ops[4];
      for (uint32_t i = 0; i < num; i++)
        if (!dds_stream_write_implBO(os, allocator, addr + i * elem_size, jsr_ops, false))
          return NULL;
      ops += (jmp ? jmp : 5);
      break;
    }
    case DDS_OP_VAL_EXT:
      abort();  // 不支持数组子类型为EXT的操作
      break;
  }

  // 写入DHEADER
  if (is_dheader_needed(subtype, xcdrv))
    *((uint32_t *)(((struct dds_ostream *)os)->m_buffer + offs - 4)) =
        to_BO4u(((struct dds_ostream *)os)->m_index - offs);

  return ops;
}

/**
 * @brief 以字节序的方式写入联合体的判别器值
 *
 * @param os            [in] 输出流指针
 * @param allocator     [in] CDR流分配器指针
 * @param ops           [in] 操作码数组指针
 * @param insn          [in] 当前操作码
 * @param addr          [in] 联合体数据地址
 * @param disc          [out] 判别器值指针
 * @return bool         成功返回true，失败返回false
 */
static bool dds_stream_write_union_discriminantBO(
    DDS_OSTREAM_T *__restrict os,
    const struct dds_cdrstream_allocator *__restrict allocator,
    const uint32_t *__restrict ops,
    uint32_t insn,
    const void *__restrict addr,
    uint32_t *disc) {
  // 断言：判别器值指针不为空
  assert(disc);

  // 获取操作码的子类型
  enum dds_stream_typecode type = DDS_OP_SUBTYPE(insn);

  // 断言：子类型必须是以下之一
  assert(type == DDS_OP_VAL_BLN || type == DDS_OP_VAL_1BY || type == DDS_OP_VAL_2BY ||
         type == DDS_OP_VAL_4BY || type == DDS_OP_VAL_ENU);

  // 根据子类型进行相应处理
  switch (type) {
    case DDS_OP_VAL_BLN:
      // 读取布尔值作为判别器值
      *disc = *((const uint8_t *)addr);
      // 写入布尔值到输出流
      if (!dds_stream_write_bool_valueBO(os, allocator, (uint8_t)*disc)) return false;
      break;
    case DDS_OP_VAL_1BY:
      // 读取1字节值作为判别器值
      *disc = *((const uint8_t *)addr);
      // 写入1字节值到输出流
      dds_os_put1BO(os, allocator, (uint8_t)*disc);
      break;
    case DDS_OP_VAL_2BY:
      // 读取2字节值作为判别器值
      *disc = *((const uint16_t *)addr);
      // 写入2字节值到输出流
      dds_os_put2BO(os, allocator, (uint16_t)*disc);
      break;
    case DDS_OP_VAL_4BY:
      // 读取4字节值作为判别器值
      *disc = *((const uint32_t *)addr);
      // 写入4字节值到输出流
      dds_os_put4BO(os, allocator, *disc);
      break;
    case DDS_OP_VAL_ENU:
      // 读取枚举值作为判别器值
      *disc = *((const uint32_t *)addr);
      // 写入枚举值到输出流
      if (!dds_stream_write_enum_valueBO(os, allocator, insn, *disc, ops[4])) return false;
      break;
    default:
      // 其他情况，终止程序
      abort();
  }

  // 返回成功
  return true;
}

/**
 * @brief 写入一个 uniBO 类型的数据到 DDS 输出流中。
 *
 * @param[in] os          指向 DDS_OSTREAM_T 结构体的指针，用于写入数据。
 * @param[in] allocator   指向 dds_cdrstream_allocator 结构体的指针，用于分配内存。
 * @param[in] discaddr    指向 discriminant 地址的指针。
 * @param[in] baseaddr    指向基地址的指针。
 * @param[in] ops         指向操作列表的指针。
 * @param[in] insn        32位无符号整数，表示指令。
 * @return                返回指向 uint32_t 的指针，表示操作成功；如果失败，则返回 NULL。
 */
static const uint32_t *dds_stream_write_uniBO(
    DDS_OSTREAM_T *__restrict os,
    const struct dds_cdrstream_allocator *__restrict allocator,
    const char *__restrict discaddr,
    const char *__restrict baseaddr,
    const uint32_t *__restrict ops,
    uint32_t insn) {
  uint32_t disc;  // 定义一个 32 位无符号整数变量 disc

  // 写入联合体的 discriminant 值，如果失败则返回 NULL
  if (!dds_stream_write_union_discriminantBO(os, allocator, ops, insn, discaddr, &disc))
    return NULL;

  // 查找与 disc 匹配的联合体 case
  uint32_t const *const jeq_op = find_union_case(ops, disc);

  // 更新 ops 指针
  ops += DDS_OP_ADR_JMP(ops[3]);

  // 如果找到了匹配的联合体 case
  if (jeq_op) {
    const enum dds_stream_typecode valtype = DDS_JEQ_TYPE(jeq_op[0]);  // 获取值类型
    const void *valaddr = baseaddr + jeq_op[2];                        // 计算值地址

    // 联合体成员不能是可选的，只能是外部的。对于字符串类型，下面会解引用指针
    if (op_type_external(jeq_op[0]) && valtype != DDS_OP_VAL_STR) {
      assert(DDS_OP(jeq_op[0]) == DDS_OP_JEQ4);
      valaddr = *(char **)valaddr;
      assert(valaddr);
    }

    // 根据值类型进行相应的处理
    switch (valtype) {
      case DDS_OP_VAL_BLN:
        if (!dds_stream_write_bool_valueBO(os, allocator, *(const uint8_t *)valaddr)) return NULL;
        break;
      case DDS_OP_VAL_1BY:
        dds_os_put1BO(os, allocator, *(const uint8_t *)valaddr);
        break;
      case DDS_OP_VAL_2BY:
        dds_os_put2BO(os, allocator, *(const uint16_t *)valaddr);
        break;
      case DDS_OP_VAL_4BY:
        dds_os_put4BO(os, allocator, *(const uint32_t *)valaddr);
        break;
      case DDS_OP_VAL_8BY:
        dds_os_put8BO(os, allocator, *(const uint64_t *)valaddr);
        break;
      case DDS_OP_VAL_ENU:
        if (!dds_stream_write_enum_valueBO(os, allocator, jeq_op[0], *((const uint32_t *)valaddr),
                                           jeq_op[3]))
          return NULL;
        break;
      case DDS_OP_VAL_STR:
        dds_stream_write_stringBO(os, allocator, *(const char **)valaddr);
        break;
      case DDS_OP_VAL_BST:
        dds_stream_write_stringBO(os, allocator, (const char *)valaddr);
        break;
      case DDS_OP_VAL_SEQ:
      case DDS_OP_VAL_BSQ:
      case DDS_OP_VAL_ARR:
      case DDS_OP_VAL_UNI:
      case DDS_OP_VAL_STU:
      case DDS_OP_VAL_BMK:
        if (!dds_stream_write_implBO(os, allocator, valaddr, jeq_op + DDS_OP_ADR_JSR(jeq_op[0]),
                                     false))
          return NULL;
        break;
      case DDS_OP_VAL_EXT:
        abort(); /* op type EXT as union subtype not supported */
        break;
    }
  }

  // 返回更新后的 ops 指针
  return ops;
}

/**
 * @brief 将数据写入DDS流。
 *
 * @param[in] insn 指令
 * @param[in,out] os DDS输出流
 * @param[in] allocator 内存分配器
 * @param[in] data 数据指针
 * @param[in] ops 操作数组
 * @param[in] is_mutable_member 是否为可变成员
 * @return 返回操作后的操作数组指针
 */
static const uint32_t *dds_stream_write_adrBO(
    uint32_t insn,
    DDS_OSTREAM_T *__restrict os,
    const struct dds_cdrstream_allocator *__restrict allocator,
    const char *__restrict data,
    const uint32_t *__restrict ops,
    bool is_mutable_member) {
  // 计算地址
  const void *addr = data + ops[1];
  // 如果是外部类型、可选类型或字符串类型，更新地址
  if (op_type_external(insn) || op_type_optional(insn) || DDS_OP_TYPE(insn) == DDS_OP_VAL_STR)
    addr = *(char **)addr;
  // 处理可选类型
  if (op_type_optional(insn)) {
    // 如果不是可变成员，将标志位写入输出流
    if (!is_mutable_member) dds_os_put1BO(os, allocator, addr ? 1 : 0);
    // 如果地址为空，跳过当前操作
    if (!addr) return dds_stream_skip_adr(insn, ops);
  }
  // 断言：地址非空或类型为字符串类型
  assert(addr || DDS_OP_TYPE(insn) == DDS_OP_VAL_STR);

  // 根据操作类型进行处理
  switch (DDS_OP_TYPE(insn)) {
    case DDS_OP_VAL_BLN:
      // 写入布尔值
      if (!dds_stream_write_bool_valueBO(os, allocator, *((const uint8_t *)addr))) return NULL;
      ops += 2;
      break;
    case DDS_OP_VAL_1BY:
      // 写入1字节数据
      dds_os_put1BO(os, allocator, *((const uint8_t *)addr));
      ops += 2;
      break;
    case DDS_OP_VAL_2BY:
      // 写入2字节数据
      dds_os_put2BO(os, allocator, *((const uint16_t *)addr));
      ops += 2;
      break;
    case DDS_OP_VAL_4BY:
      // 写入4字节数据
      dds_os_put4BO(os, allocator, *((const uint32_t *)addr));
      ops += 2;
      break;
    case DDS_OP_VAL_8BY:
      // 写入8字节数据
      dds_os_put8BO(os, allocator, *((const uint64_t *)addr));
      ops += 2;
      break;
    case DDS_OP_VAL_ENU:
      // 写入枚举值
      if (!dds_stream_write_enum_valueBO(os, allocator, insn, *((const uint32_t *)addr), ops[2]))
        return NULL;
      ops += 3;
      break;
    case DDS_OP_VAL_BMK:
      // 写入位掩码值
      if (!dds_stream_write_bitmask_valueBO(os, allocator, insn, addr, ops[2], ops[3])) return NULL;
      ops += 4;
      break;
    case DDS_OP_VAL_STR:
      // 写入字符串
      dds_stream_write_stringBO(os, allocator, (const char *)addr);
      ops += 2;
      break;
    case DDS_OP_VAL_BST:
      // 写入定长字符串
      dds_stream_write_stringBO(os, allocator, (const char *)addr);
      ops += 3;
      break;
    case DDS_OP_VAL_SEQ:
    case DDS_OP_VAL_BSQ:
      // 写入序列或定长序列
      ops = dds_stream_write_seqBO(os, allocator, addr, ops, insn);
      break;
    case DDS_OP_VAL_ARR:
      // 写入数组
      ops = dds_stream_write_arrBO(os, allocator, addr, ops, insn);
      break;
    case DDS_OP_VAL_UNI:
      // 写入联合体
      ops = dds_stream_write_uniBO(os, allocator, addr, data, ops, insn);
      break;
    case DDS_OP_VAL_EXT: {
      // 处理扩展类型
      const uint32_t *jsr_ops = ops + DDS_OP_ADR_JSR(ops[2]);
      const uint32_t jmp = DDS_OP_ADR_JMP(ops[2]);

      // 跳过基本类型的DLC指令，以避免序列化基本类型的DHEADER
      if (op_type_base(insn) && jsr_ops[0] == DDS_OP_DLC) jsr_ops++;

      // 不传递is_mutable_member，子类型可能具有其他可扩展性
      if (!dds_stream_write_implBO(os, allocator, addr, jsr_ops, false)) return NULL;
      ops += jmp ? jmp : 3;
      break;
    }
    case DDS_OP_VAL_STU:
      // STU类型仅作为子类型支持
      abort();
      break;
  }
  return ops;
}

/**
 * @brief 将数据写入dds流，并添加序列化数据大小的dheader。
 *
 * @param os            [in] 一个指向DDS_OSTREAM_T结构体的指针，用于输出流操作。
 * @param allocator     [in] 一个指向dds_cdrstream_allocator结构体的指针，用于内存分配。
 * @param data          [in] 一个指向要写入流的数据的指针。
 * @param ops           [in] 一个指向uint32_t类型的指针，表示操作序列。
 * @return 返回一个指向uint32_t类型的指针，表示下一个操作；如果失败，则返回NULL。
 */
static const uint32_t *dds_stream_write_delimitedBO(
    DDS_OSTREAM_T *__restrict os,
    const struct dds_cdrstream_allocator *__restrict allocator,
    const char *__restrict data,
    const uint32_t *__restrict ops) {
  // 预留4字节空间，用于存储序列化数据大小的dheader
  uint32_t offs = dds_os_reserve4BO(os, allocator);

  // 写入数据到dds流
  if (!(ops = dds_stream_write_implBO(os, allocator, data, ops + 1, false))) return NULL;

  // 添加dheader，即数据的序列化大小
  *((uint32_t *)(os->x.m_buffer + offs - 4)) = to_BO4u(os->x.m_index - offs);
  return ops;
}

/**
 * @brief 将具有特定成员ID的数据写入dds流，并添加emheader。
 *
 * @param mid           [in] 成员ID。
 * @param os            [in] 一个指向DDS_OSTREAM_T结构体的指针，用于输出流操作。
 * @param allocator     [in] 一个指向dds_cdrstream_allocator结构体的指针，用于内存分配。
 * @param data          [in] 一个指向要写入流的数据的指针。
 * @param ops           [in] 一个指向uint32_t类型的指针，表示操作序列。
 * @return 返回一个布尔值，表示操作是否成功。
 */
static bool dds_stream_write_pl_memberBO(uint32_t mid,
                                         DDS_OSTREAM_T *__restrict os,
                                         const struct dds_cdrstream_allocator *__restrict allocator,
                                         const char *__restrict data,
                                         const uint32_t *__restrict ops) {
  // 检查成员ID是否有效
  assert(!(mid & ~EMHEADER_MEMBERID_MASK));

  // 获取长度编码
  uint32_t lc = get_length_code(ops);
  assert(lc <= LENGTH_CODE_ALSO_NEXTINT8);

  // 预留空间以添加emheader
  uint32_t data_offs = (lc != LENGTH_CODE_NEXTINT) ? dds_os_reserve4BO(os, allocator)
                                                   : dds_os_reserve8BO(os, allocator);

  // 写入数据到dds流
  if (!(dds_stream_write_implBO(os, allocator, data, ops, true))) return false;

  // 从第一个成员操作中获取must-understand标志
  uint32_t flags = DDS_OP_FLAGS(ops[0]);
  bool must_understand = flags & (DDS_OP_FLAG_MU | DDS_OP_FLAG_KEY);

  // 添加emheader，包括数据长度编码、标志和可选的序列化数据大小
  uint32_t em_hdr = 0;
  if (must_understand) em_hdr |= EMHEADER_FLAG_MUSTUNDERSTAND;
  em_hdr |= lc << 28;
  em_hdr |= mid & EMHEADER_MEMBERID_MASK;

  // 设置emheader的值
  uint32_t *em_hdr_ptr =
      (uint32_t *)(os->x.m_buffer + data_offs - (lc == LENGTH_CODE_NEXTINT ? 8 : 4));
  em_hdr_ptr[0] = to_BO4u(em_hdr);
  if (lc == LENGTH_CODE_NEXTINT)
    em_hdr_ptr[1] = to_BO4u(os->x.m_index - data_offs); /* 成员大小在emheader的next_int字段中 */
  return true;
}
/**
 * @brief 写入PL（Parameter List）成员列表的函数，支持大尾序（Big-Endian Order）
 *
 * @param[in] os          输出流指针
 * @param[in] allocator   分配器指针
 * @param[in] data        数据指针
 * @param[in] ops         操作指针
 * @return                返回操作指针
 */
static const uint32_t *dds_stream_write_pl_memberlistBO(
    DDS_OSTREAM_T *__restrict os,
    const struct dds_cdrstream_allocator *__restrict allocator,
    const char *__restrict data,
    const uint32_t *__restrict ops) {
  uint32_t insn;
  // 循环处理操作列表，直到遇到DDS_OP_RTS操作
  while (ops && (insn = *ops) != DDS_OP_RTS) {
    // 根据操作码进行相应处理
    switch (DDS_OP(insn)) {
      case DDS_OP_PLM: {
        uint32_t flags = DDS_PLM_FLAGS(insn);
        const uint32_t *plm_ops = ops + DDS_OP_ADR_PLM(insn);
        // 如果有基类型标志，则处理基类型成员
        if (flags & DDS_OP_FLAG_BASE) {
          assert(plm_ops[0] == DDS_OP_PLC);
          plm_ops++; /* 跳过PLC操作，进入基类型的第一个PLM */
          if (!dds_stream_write_pl_memberlistBO(os, allocator, data, plm_ops)) return NULL;
        }
        // 如果成员存在，则写入成员数据
        else if (is_member_present(data, plm_ops)) {
          uint32_t member_id = ops[1];
          if (!dds_stream_write_pl_memberBO(member_id, os, allocator, data, plm_ops)) return NULL;
        }
        ops += 2;
        break;
      }
      default:
        abort(); /* 其他操作暂不支持 */
        break;
    }
  }
  return ops;
}

/**
 * @brief 写入PL（Parameter List）的函数，支持大尾序（Big-Endian Order）
 *
 * @param[in] os          输出流指针
 * @param[in] allocator   分配器指针
 * @param[in] data        数据指针
 * @param[in] ops         操作指针
 * @return                返回操作指针
 */
static const uint32_t *dds_stream_write_plBO(
    DDS_OSTREAM_T *__restrict os,
    const struct dds_cdrstream_allocator *__restrict allocator,
    const char *__restrict data,
    const uint32_t *__restrict ops) {
  // 跳过PLC操作
  ops++;

  // 为dheader分配空间
  dds_os_reserve4BO(os, allocator);
  uint32_t data_offs = os->x.m_index;

  // 写入成员数据，包括基类型的成员
  ops = dds_stream_write_pl_memberlistBO(os, allocator, data, ops);

  // 在dheader中写入序列化大小
  *((uint32_t *)(os->x.m_buffer + data_offs - 4)) = to_BO4u(os->x.m_index - data_offs);
  return ops;
}

/**
 * @brief 以大端字节序实现DDS流写入操作
 *
 * @param[in] os             指向DDS输出流的指针
 * @param[in] allocator      指向dds_cdrstream_allocator结构体的指针
 * @param[in] data           指向要写入数据的指针
 * @param[in] ops            指向操作列表的指针
 * @param[in] is_mutable_member 是否为可变成员
 * @return 返回操作列表的指针，如果执行失败则返回NULL
 */
static const uint32_t *dds_stream_write_implBO(
    DDS_OSTREAM_T *__restrict os,
    const struct dds_cdrstream_allocator *__restrict allocator,
    const char *__restrict data,
    const uint32_t *__restrict ops,
    bool is_mutable_member) {
  uint32_t insn;  // 定义指令变量
  // 当ops不为空且指令不等于DDS_OP_RTS时，循环执行
  while (ops && (insn = *ops) != DDS_OP_RTS) {
    // 根据指令类型进行相应操作
    switch (DDS_OP(insn)) {
      case DDS_OP_ADR:
        ops = dds_stream_write_adrBO(insn, os, allocator, data, ops, is_mutable_member);
        break;
      case DDS_OP_JSR:
        if (!dds_stream_write_implBO(os, allocator, data, ops + DDS_OP_JUMP(insn),
                                     is_mutable_member))
          return NULL;
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
        assert(((struct dds_ostream *)os)->m_xcdr_version == DDSI_RTPS_CDR_ENC_VERSION_2);
        ops = dds_stream_write_delimitedBO(os, allocator, data, ops);
        break;
      case DDS_OP_PLC:
        assert(((struct dds_ostream *)os)->m_xcdr_version == DDSI_RTPS_CDR_ENC_VERSION_2);
        ops = dds_stream_write_plBO(os, allocator, data, ops);
        break;
    }
  }
  return ops;
}

/**
 * @brief 以大端字节序实现DDS流写入操作的包装函数
 *
 * @param[in] os             指向DDS输出流的指针
 * @param[in] allocator      指向dds_cdrstream_allocator结构体的指针
 * @param[in] data           指向要写入数据的指针
 * @param[in] ops            指向操作列表的指针
 * @return 返回操作列表的指针，如果执行失败则返回NULL
 */
const uint32_t *dds_stream_writeBO(DDS_OSTREAM_T *__restrict os,
                                   const struct dds_cdrstream_allocator *__restrict allocator,
                                   const char *__restrict data,
                                   const uint32_t *__restrict ops) {
  return dds_stream_write_implBO(os, allocator, data, ops, false);
}
