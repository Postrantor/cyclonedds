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

static void dds_stream_write_keyBO_impl(DDS_OSTREAM_T *__restrict os,
                                        const struct dds_cdrstream_allocator *__restrict allocator,
                                        const uint32_t *ops,
                                        const void *src,
                                        uint16_t key_offset_count,
                                        const uint32_t *key_offset_insn);
/**
 * @brief 写入字节序列的实现函数
 *
 * @param[in] os                输出流指针
 * @param[in] allocator         分配器指针
 * @param[in] ops               操作列表指针
 * @param[in] src               源数据指针
 * @param[in] key_offset_count  键偏移计数
 * @param[in] key_offset_insn   键偏移指令指针
 */
static void dds_stream_write_keyBO_impl(DDS_OSTREAM_T *__restrict os,
                                        const struct dds_cdrstream_allocator *__restrict allocator,
                                        const uint32_t *ops,
                                        const void *src,
                                        uint16_t key_offset_count,
                                        const uint32_t *key_offset_insn) {
  // 获取操作指令
  uint32_t insn = *ops;
  // 检查操作指令是否为 DDS_OP_ADR 类型
  assert(DDS_OP(insn) == DDS_OP_ADR);
  // 检查键是否合法
  assert(insn_key_ok_p(insn));
  // 计算地址
  void *addr = (char *)src + ops[1];

  // 如果操作类型为外部类型，则分配空间
  if (op_type_external(insn)) dds_stream_alloc_external(ops, insn, &addr, allocator);

  // 根据操作类型进行处理
  switch (DDS_OP_TYPE(insn)) {
    case DDS_OP_VAL_BLN:
    case DDS_OP_VAL_1BY:
      dds_os_put1BO(os, allocator, *((uint8_t *)addr));
      break;
    case DDS_OP_VAL_2BY:
      dds_os_put2BO(os, allocator, *((uint16_t *)addr));
      break;
    case DDS_OP_VAL_4BY:
      dds_os_put4BO(os, allocator, *((uint32_t *)addr));
      break;
    case DDS_OP_VAL_8BY:
      dds_os_put8BO(os, allocator, *((uint64_t *)addr));
      break;
    case DDS_OP_VAL_ENU:
      (void)dds_stream_write_enum_valueBO(os, allocator, insn, *((uint32_t *)addr), ops[2]);
      break;
    case DDS_OP_VAL_BMK:
      (void)dds_stream_write_bitmask_valueBO(os, allocator, insn, addr, ops[2], ops[3]);
      break;
    case DDS_OP_VAL_STR:
      dds_stream_write_stringBO(os, allocator, *(char **)addr);
      break;
    case DDS_OP_VAL_BST:
      dds_stream_write_stringBO(os, allocator, addr);
      break;
    case DDS_OP_VAL_ARR: {
      const uint32_t num = ops[2];
      switch (DDS_OP_SUBTYPE(insn)) {
        case DDS_OP_VAL_BLN:
        case DDS_OP_VAL_1BY:
        case DDS_OP_VAL_2BY:
        case DDS_OP_VAL_4BY:
        case DDS_OP_VAL_8BY: {
          const uint32_t elem_size = get_primitive_size(DDS_OP_SUBTYPE(insn));
          const align_t align =
              dds_cdr_get_align(((struct dds_ostream *)os)->m_xcdr_version, elem_size);
          dds_cdr_alignto_clear_and_resizeBO(os, allocator, align, num * elem_size);
          void *const dst =
              ((struct dds_ostream *)os)->m_buffer + ((struct dds_ostream *)os)->m_index;
          dds_os_put_bytes((struct dds_ostream *)os, allocator, addr, num * elem_size);
          dds_stream_swap_if_needed_insituBO(dst, elem_size, num);
          break;
        }
        case DDS_OP_VAL_ENU:
        case DDS_OP_VAL_BMK: {
          uint32_t offs = 0, xcdrv = ((struct dds_ostream *)os)->m_xcdr_version;
          if (xcdrv == DDSI_RTPS_CDR_ENC_VERSION_2) {
            /* reserve space for DHEADER */
            dds_os_reserve4BO(os, allocator);
            offs = ((struct dds_ostream *)os)->m_index;
          }
          if (DDS_OP_SUBTYPE(insn) == DDS_OP_VAL_ENU)
            (void)dds_stream_write_enum_arrBO(os, allocator, insn, (const uint32_t *)addr, num,
                                              ops[3]);
          else
            (void)dds_stream_write_bitmask_arrBO(os, allocator, insn, (const uint32_t *)addr, num,
                                                 ops[3], ops[4]);
          /* write DHEADER */
          if (xcdrv == DDSI_RTPS_CDR_ENC_VERSION_2)
            *((uint32_t *)(((struct dds_ostream *)os)->m_buffer + offs - 4)) =
                to_BO4u(((struct dds_ostream *)os)->m_index - offs);
          break;
        }
        default:
          abort();
      }
      break;
    }
    case DDS_OP_VAL_EXT: {
      assert(key_offset_count > 0);
      const uint32_t *jsr_ops = ops + DDS_OP_ADR_JSR(ops[2]) + *key_offset_insn;
      dds_stream_write_keyBO_impl(os, allocator, jsr_ops, addr, --key_offset_count,
                                  ++key_offset_insn);
      break;
    }
    case DDS_OP_VAL_SEQ:
    case DDS_OP_VAL_BSQ:
    case DDS_OP_VAL_UNI:
    case DDS_OP_VAL_STU: {
      // FIXME: implement support for sequences and unions as part of the key
      abort();
      break;
    }
  }
}

/**
 * @brief 将样本数据写入输出流，支持键值优化。
 *
 * @param[in] os 指向DDS_OSTREAM_T类型的指针，用于存储输出流。
 * @param[in] allocator 指向dds_cdrstream_allocator结构体的指针，用于分配内存。
 * @param[in] sample 指向字符类型的指针，表示要写入的样本数据。
 * @param[in] desc 指向dds_cdrstream_desc结构体的指针，描述样本数据的结构。
 */
void dds_stream_write_keyBO(DDS_OSTREAM_T *__restrict os,
                            const struct dds_cdrstream_allocator *__restrict allocator,
                            const char *__restrict sample,
                            const struct dds_cdrstream_desc *__restrict desc) {
  // 遍历所有键值
  for (uint32_t i = 0; i < desc->keys.nkeys; i++) {
    // 获取当前键值操作的指针
    const uint32_t *insnp = desc->ops.ops + desc->keys.keys[i].ops_offs;

    // 根据操作码执行相应操作
    switch (DDS_OP(*insnp)) {
      case DDS_OP_KOF: {
        // 获取操作长度
        uint16_t n_offs = DDS_OP_LENGTH(*insnp);

        // 确保操作长度大于0
        assert(n_offs > 0);

        // 调用实现函数处理键值优化
        dds_stream_write_keyBO_impl(os, allocator, desc->ops.ops + insnp[1], sample, --n_offs,
                                    insnp + 2);
        break;
      }
      case DDS_OP_ADR: {
        // 调用实现函数处理地址操作
        dds_stream_write_keyBO_impl(os, allocator, insnp, sample, 0, NULL);
        break;
      }
      default:
        // 遇到未知操作码，终止程序
        abort();
        break;
    }
  }
}

/**
 * @brief 从数据流中提取键值并返回操作指针。
 *
 * @param[in] insn 指令
 * @param[in] is 输入数据流
 * @param[in,out] os 输出数据流
 * @param[in] allocator 内存分配器
 * @param[in] ops_offs_idx 操作偏移索引
 * @param[in,out] ops_offs 操作偏移数组
 * @param[in] op0 操作指针0
 * @param[in] op0_type 操作指针0的类型
 * @param[in] ops 操作指针
 * @param[in] mutable_member 可变成员标志
 * @param[in] mutable_member_or_parent 可变成员或父级标志
 * @param[in] n_keys 键数量
 * @param[in,out] keys_remaining 剩余键数量
 * @param[in] keys 键描述符数组
 * @param[out] key_offs 键偏移信息数组
 * @return 返回操作指针
 */
static const uint32_t *dds_stream_extract_keyBO_from_data_adr(   //
    uint32_t insn,                                               //
    dds_istream_t *__restrict is,                                //
    DDS_OSTREAM_T *__restrict os,                                //
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
    const dds_cdrstream_desc_key_t *__restrict keys,             //
    struct key_off_info *__restrict key_offs) {
  // 检查指令是否为DDS_OP_ADR
  assert(DDS_OP(insn) == DDS_OP_ADR);
  // 获取类型码
  const enum dds_stream_typecode type = DDS_OP_TYPE(insn);
  // 判断是否为键值
  const bool is_key = (insn & DDS_OP_FLAG_KEY) && (os != NULL);

  // 如果成员不存在，则跳过
  if (!stream_is_member_present(insn, is, mutable_member)) {
    assert(!is_key);
    return dds_stream_skip_adr(insn, ops);
  }

  // 处理扩展类型
  if (type == DDS_OP_VAL_EXT) {
    const uint32_t *jsr_ops = ops + DDS_OP_ADR_JSR(ops[2]);
    const uint32_t jmp = DDS_OP_ADR_JMP(ops[2]);

    // 设置操作偏移
    if (ops_offs) {
      assert(ops_offs_idx < DDS_CDRSTREAM_MAX_NESTING_DEPTH);
      ptrdiff_t offs = ops - op0_type;
      assert(offs >= INT32_MIN && offs <= INT32_MAX);
      ops_offs[ops_offs_idx] = (uint32_t)(offs);
    }

    // 跳过基本类型的DLC指令
    if (op_type_base(insn) && jsr_ops[0] == DDS_OP_DLC) jsr_ops++;

    // 如果ADR|EXT设置了key标志，则传递实际的ostream，否则跳过EXT类型并传递NULL作为ostream
    (void)dds_stream_extract_keyBO_from_data1(
        is, is_key ? os : NULL, allocator, ops_offs_idx + 1, ops_offs, op0, jsr_ops, jsr_ops, false,
        mutable_member_or_parent, n_keys, keys_remaining, keys, key_offs);
    ops += jmp ? jmp : 3;
  } else {
    // 处理键值
    if (is_key) {
      assert(*keys_remaining <= n_keys);
      uint32_t idx = n_keys - *keys_remaining;  // 键在CDR中的位置（索引）
      if (((struct dds_ostream *)os)->m_xcdr_version == DDSI_RTPS_CDR_ENC_VERSION_1) {
        // CDR编码版本1中的键按照定义顺序排序，因此我们可以使用键描述符键列表中的键索引字段
        key_offs[idx].src_off = is->m_index;
        key_offs[idx].op_off = ops;
        assert(*keys_remaining > 0);
        (*keys_remaining)--;
      } else {
        assert(ops_offs);
        assert(ops_offs_idx < DDS_CDRSTREAM_MAX_NESTING_DEPTH);
        ptrdiff_t offs = ops - op0_type;
        assert(offs >= INT32_MIN && offs <= INT32_MAX);
        ops_offs[ops_offs_idx] = (uint32_t)(offs);
        bool found = false;
        uint32_t n;

        // XCDR2中的键按照成员ID（升序）排序，因此我们需要从键描述符中找到键在键列表中的位置（该列表按成员ID排序）
        for (n = 0; !found && n < n_keys; n++) {
          // 如果键路径中没有可变类型，键成员将按照包含在键描述符“index”列中的顺序出现在CDR中
          if (!mutable_member_or_parent && keys[n].idx == idx) {
            found = true;
            break;
          } else if (mutable_member_or_parent) {
            // 对于键路径中具有可变成员的键，我们将查看所有实际键成员祖先的键成员偏移
            const uint32_t *kof_op = op0 + keys[n].ops_offs;
            assert(DDS_OP(*kof_op) == DDS_OP_KOF);
            uint16_t n_offs = DDS_OP_LENGTH(*kof_op);
            if (n_offs == ops_offs_idx + 1 &&
                !memcmp(&kof_op[1], ops_offs, n_offs * sizeof(kof_op[1]))) {
              found = true;
              break;
            }
          }
        }
        if (found) {
          key_offs[n].src_off = is->m_index;
          key_offs[n].op_off = ops;
          assert(*keys_remaining > 0);
          (*keys_remaining)--;
        }
      }
    }
    // 跳过地址并提取键值
    ops = dds_stream_extract_key_from_data_skip_adr(is, ops, type);
  }
  return ops;
}

/**
 * @brief 从数据中提取键值并处理分隔符。
 *
 * @param[in] is 输入流指针，用于读取数据。
 * @param[out] os 输出流指针，用于写入数据。
 * @param[in] allocator 分配器指针，用于内存管理。
 * @param[in] ops_offs_idx 操作偏移索引。
 * @param[out] ops_offs 操作偏移数组。
 * @param[in] op0 第一个操作数指针。
 * @param[in] op0_type 第一个操作数类型指针。
 * @param[in] ops 操作数组指针。
 * @param[in] mutable_member_or_parent 可变成员或父级标志。
 * @param[in] n_keys 键数量。
 * @param[out] keys_remaining 剩余键数组。
 * @param[in] keys 键描述数组。
 * @param[out] key_offs 键偏移信息数组。
 * @return 返回操作数组指针。
 */
static const uint32_t *dds_stream_extract_keyBO_from_data_delimited(
    dds_istream_t *__restrict is,                                // 输入流指针
    DDS_OSTREAM_T *__restrict os,                                // 输出流指针
    const struct dds_cdrstream_allocator *__restrict allocator,  // 分配器指针
    uint32_t ops_offs_idx,                                       // 操作偏移索引
    uint32_t *__restrict ops_offs,                               // 操作偏移数组
    const uint32_t *const __restrict op0,                        // 第一个操作数指针
    const uint32_t *const __restrict op0_type,                   // 第一个操作数类型指针
    const uint32_t *__restrict ops,                              // 操作数组指针
    bool mutable_member_or_parent,                               // 可变成员或父级标志
    uint32_t n_keys,                                             // 键数量
    uint32_t *__restrict keys_remaining,                         // 剩余键数组
    const dds_cdrstream_desc_key_t *__restrict keys,             // 键描述数组
    struct key_off_info *__restrict key_offs)                    // 键偏移信息数组
{
  uint32_t delimited_sz = dds_is_get4(is), delimited_offs = is->m_index, insn;
  ops++;
  while ((insn = *ops) != DDS_OP_RTS) {
    switch (DDS_OP(insn)) {
      case DDS_OP_ADR:
        // 跳过可追加类型的序列化数据中不存在的字段
        ops = (is->m_index - delimited_offs < delimited_sz)
                  ? dds_stream_extract_keyBO_from_data_adr(
                        insn, is, os, allocator, ops_offs_idx, ops_offs, op0, op0_type, ops, false,
                        mutable_member_or_parent, n_keys, keys_remaining, keys, key_offs)
                  : dds_stream_skip_adr(insn, ops);
        break;
      case DDS_OP_JSR:
        (void)dds_stream_extract_keyBO_from_data1(
            is, os, allocator, ops_offs_idx, ops_offs, op0, op0_type, ops + DDS_OP_JUMP(insn),
            false, mutable_member_or_parent, n_keys, keys_remaining, keys, key_offs);
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
  assert(delimited_sz == is->m_index - delimited_offs);
  return ops;
}

/**
 * @brief 从数据中提取键值的函数
 *
 * @param[in] is 输入流指针
 * @param[out] os 输出流指针
 * @param[in] allocator 内存分配器指针
 * @param[in] m_id 成员ID
 * @param[in] ops_offs_idx 操作偏移索引
 * @param[out] ops_offs 操作偏移数组
 * @param[in] op0 第一个操作指针
 * @param[in] op0_type 第一个操作类型指针
 * @param[in] ops 操作数组
 * @param[in] n_keys 键数量
 * @param[out] keys_remaining 剩余键数量
 * @param[in] keys 键描述符数组
 * @param[out] key_offs 键偏移信息数组
 * @return bool 如果找到了键值则返回true，否则返回false
 */
static bool dds_stream_extract_keyBO_from_data_pl_member(        //
    dds_istream_t *__restrict is,                                // 输入流指针
    DDS_OSTREAM_T *__restrict os,                                // 输出流指针
    const struct dds_cdrstream_allocator *__restrict allocator,  // 内存分配器指针
    uint32_t m_id,                                               // 成员ID
    uint32_t ops_offs_idx,                                       // 操作偏移索引
    uint32_t *__restrict ops_offs,                               // 操作偏移数组
    const uint32_t *const __restrict op0,                        // 第一个操作指针
    const uint32_t *const __restrict op0_type,                   // 第一个操作类型指针
    const uint32_t *__restrict ops,                              // 操作数组
    uint32_t n_keys,                                             // 键数量
    uint32_t *__restrict keys_remaining,                         // 剩余键数量
    const dds_cdrstream_desc_key_t *__restrict keys,             // 键描述符数组
    struct key_off_info *__restrict key_offs)                    // 键偏移信息数组
{
  uint32_t insn, ops_csr = 0;  // 定义指令和操作计数器
  bool found = false;          // 初始化找到标志为false

  // 当剩余键数量大于0且未找到键值且指令不等于DDS_OP_RTS时，执行循环
  while (*keys_remaining > 0 && !found && (insn = ops[ops_csr]) != DDS_OP_RTS) {
    assert(DDS_OP(insn) == DDS_OP_PLM);    // 断言指令的操作码为DDS_OP_PLM
    uint32_t flags = DDS_PLM_FLAGS(insn);  // 获取指令的标志位
    const uint32_t *plm_ops = ops + ops_csr + DDS_OP_ADR_PLM(insn);  // 计算PLM操作的地址
    if (flags & DDS_OP_FLAG_BASE)  // 如果标志位包含DDS_OP_FLAG_BASE
    {
      assert(DDS_OP(plm_ops[0]) == DDS_OP_PLC);  // 断言第一个PLM操作的操作码为DDS_OP_PLC
      plm_ops++;                                 // 跳过PLC，进入基类型的第一个PLM操作
      // 递归调用函数，查找键值
      found = dds_stream_extract_keyBO_from_data_pl_member(is, os, allocator, m_id, ops_offs_idx,
                                                           ops_offs, op0, op0_type, plm_ops, n_keys,
                                                           keys_remaining, keys, key_offs);
    } else if (ops[ops_csr + 1] == m_id)  // 如果操作数组中的下一个元素等于成员ID
    {
      // 调用dds_stream_extract_keyBO_from_data1函数，查找键值
      (void)dds_stream_extract_keyBO_from_data1(is, os, allocator, ops_offs_idx, ops_offs, op0,
                                                op0_type, plm_ops, true, true, n_keys,
                                                keys_remaining, keys, key_offs);
      found = true;  // 设置找到标志为true
      break;         // 跳出循环
    }
    ops_csr += 2;  // 操作计数器加2
  }
  return found;  // 返回找到标志
}

/**
 * @brief 从数据负载中提取键值的二进制序列。
 *
 * @param[in] is 输入流指针，用于读取数据。
 * @param[out] os 输出流指针，用于写入提取的键值。
 * @param[in] allocator 内存分配器指针，用于分配内存。
 * @param[in] ops_offs_idx 操作偏移索引。
 * @param[out] ops_offs 操作偏移数组。
 * @param[in] op0 第一个操作指针。
 * @param[in] op0_type 第一个操作类型指针。
 * @param[in] ops 操作数组。
 * @param[in] n_keys 键值数量。
 * @param[out] keys_remaining 剩余键值数量。
 * @param[in] keys 键值描述数组。
 * @param[out] key_offs 键值偏移信息数组。
 * @return 返回操作数组的下一个位置。
 */
static const uint32_t *dds_stream_extract_keyBO_from_data_pl(    //
    dds_istream_t *__restrict is,                                // 输入流指针
    DDS_OSTREAM_T *__restrict os,                                // 输出流指针
    const struct dds_cdrstream_allocator *__restrict allocator,  // 内存分配器指针
    uint32_t ops_offs_idx,                                       // 操作偏移索引
    uint32_t *__restrict ops_offs,                               // 操作偏移数组
    const uint32_t *const __restrict op0,                        // 第一个操作指针
    const uint32_t *const __restrict op0_type,                   // 第一个操作类型指针
    const uint32_t *__restrict ops,                              // 操作数组
    uint32_t n_keys,                                             // 键值数量
    uint32_t *__restrict keys_remaining,                         // 剩余键值数量
    const dds_cdrstream_desc_key_t *__restrict keys,             // 键值描述数组
    struct key_off_info *__restrict key_offs)                    // 键值偏移信息数组
{
  /* 跳过 PLC 操作 */
  ops++;

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

    /* 如果成员未找到或者没有更多的键值需要找到，跳过输入流中的成员 */
    if (!dds_stream_extract_keyBO_from_data_pl_member(is, os, allocator, m_id, ops_offs_idx,
                                                      ops_offs, op0, op0_type, ops, n_keys,
                                                      keys_remaining, keys, key_offs)) {
      is->m_index += msz;
      if (lc >= LENGTH_CODE_ALSO_NEXTINT)
        is->m_index += 4; /* 成员内嵌的长度不包括它自己的 4 字节 */
    }
  }

  /* 跳过所有 PLM-memberid 对 */
  while (ops[0] != DDS_OP_RTS) ops += 2;

  return ops;
}

/**
 * @brief 从数据中提取密钥信息
 *
 * @param is 输入流指针
 * @param os 输出流指针
 * @param allocator 内存分配器指针
 * @param ops_offs_idx 操作偏移索引
 * @param ops_offs 操作偏移指针
 * @param op0 第一个操作数指针
 * @param op0_type 第一个操作数类型指针
 * @param ops 操作指针
 * @param mutable_member 可变成员标志
 * @param mutable_member_or_parent 可变成员或父级标志
 * @param n_keys 密钥数量
 * @param keys_remaining 剩余密钥数量指针
 * @param keys 密钥描述指针
 * @param key_offs 密钥偏移信息指针
 * @return const uint32_t* 返回操作指针
 */
static const uint32_t *dds_stream_extract_keyBO_from_data1(
    dds_istream_t *__restrict is,                                // 输入流指针
    DDS_OSTREAM_T *__restrict os,                                // 输出流指针
    const struct dds_cdrstream_allocator *__restrict allocator,  // 内存分配器指针
    uint32_t ops_offs_idx,                                       // 操作偏移索引
    uint32_t *__restrict ops_offs,                               // 操作偏移指针
    const uint32_t *const __restrict op0,                        // 第一个操作数指针
    const uint32_t *const __restrict op0_type,                   // 第一个操作数类型指针
    const uint32_t *__restrict ops,
    bool mutable_member,                              // 操作指针和可变成员标志
    bool mutable_member_or_parent,                    // 可变成员或父级标志
    uint32_t n_keys,                                  // 密钥数量
    uint32_t *__restrict keys_remaining,              // 剩余密钥数量指针
    const dds_cdrstream_desc_key_t *__restrict keys,  // 密钥描述指针
    struct key_off_info *__restrict key_offs)         // 密钥偏移信息指针
{
  uint32_t insn;                       // 指令
  while ((insn = *ops) != DDS_OP_RTS)  // 当指令不等于DDS_OP_RTS时循环
  {
    switch (DDS_OP(insn))  // 根据指令类型进行判断
    {
      case DDS_OP_ADR:
        ops = dds_stream_extract_keyBO_from_data_adr(
            insn, is, os, allocator, ops_offs_idx, ops_offs, op0, op0_type, ops, mutable_member,
            mutable_member_or_parent, n_keys, keys_remaining, keys, key_offs);
        break;
      case DDS_OP_JSR:
        (void)dds_stream_extract_keyBO_from_data1(
            is, os, allocator, ops_offs_idx, ops_offs, op0, op0_type, ops + DDS_OP_JUMP(insn),
            mutable_member, mutable_member_or_parent, n_keys, keys_remaining, keys, key_offs);
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
        ops = dds_stream_extract_keyBO_from_data_delimited(
            is, os, allocator, ops_offs_idx, ops_offs, op0, ops, ops, mutable_member_or_parent,
            n_keys, keys_remaining, keys, key_offs);
        break;
      case DDS_OP_PLC:
        ops =
            dds_stream_extract_keyBO_from_data_pl(is, os, allocator, ops_offs_idx, ops_offs, op0,
                                                  ops, ops, n_keys, keys_remaining, keys, key_offs);
        break;
    }
  }
  return ops;
}

/**
 * @brief 从数据中提取键值并进行序列化操作
 *
 * @param[in] is 输入流指针，用于读取数据
 * @param[out] os 输出流指针，用于写入序列化后的数据
 * @param[in] allocator 内存分配器指针，用于动态分配内存
 * @param[in] desc 数据描述符指针，包含元数据信息
 * @return bool 提取和序列化操作是否成功
 */
bool dds_stream_extract_keyBO_from_data(                         //
    dds_istream_t *__restrict is,                                // 输入流指针
    DDS_OSTREAM_T *__restrict os,                                // 输出流指针
    const struct dds_cdrstream_allocator *__restrict allocator,  // 内存分配器指针
    const struct dds_cdrstream_desc *__restrict desc)            // 数据描述符指针
{
  bool ret = true;
  uint32_t keys_remaining = desc->keys.nkeys;  // 剩余的键值数量
  if (keys_remaining == 0) return ret;

#define MAX_ST_KEYS 16
  struct key_off_info st_key_offs[MAX_ST_KEYS];  // 静态键值偏移信息数组
  struct key_off_info *const key_offs =
      (desc->keys.nkeys <= MAX_ST_KEYS)
          ? st_key_offs
          : allocator->malloc(desc->keys.nkeys * sizeof(*key_offs));  // 动态或静态键值偏移信息指针
  uint32_t ops_offs[DDS_CDRSTREAM_MAX_NESTING_DEPTH];                 // 操作偏移数组

  uint32_t *ops = desc->ops.ops, *op0 = ops, *op0_type = ops;  // 操作指针
  (void)dds_stream_extract_keyBO_from_data1(is, os, allocator, 0, ops_offs, op0, op0_type, ops,
                                            false, false, desc->keys.nkeys, &keys_remaining,
                                            desc->keys.keys, key_offs);  // 提取键值并进行序列化操作
  if (keys_remaining > 0) {
    /* FIXME: stream_normalize should check for missing keys by implementing the
       must_understand annotation, so the check keys_remaining > 0 can become an assert. */
    ret = false;
    goto err_missing_key;
  }
  for (uint32_t i = 0; i < desc->keys.nkeys; i++)  // 遍历所有键值
  {
    is->m_index = key_offs[i].src_off;  // 设置输入流索引
    dds_stream_extract_keyBO_from_key_prim_op(is, os, allocator, key_offs[i].op_off, 0,
                                              NULL);  // 提取键值并进行序列化操作
  }

err_missing_key:                       // 错误处理标签
  if (desc->keys.nkeys > MAX_ST_KEYS)  // 如果键值数量大于静态数组大小
    allocator->free(key_offs);         // 释放动态分配的内存
  return ret;                          // 返回操作结果
#undef MAX_ST_KEYS
}

/**
 * @brief
 * 该函数用于创建序列化密钥，以便创建密钥哈希（大端）并将XCDR1密钥CDR转换为XCDR2表示法（本机字节顺序）。
 *        前者不是Cyclone经常使用的，后者仅在接收密钥样本时使用，例如处置。出于这个原因，我们采用了一种（性能方面）次优的方法，
 *        对每个密钥字段都遍历整个CDR。优化是可能的，但会导致代码更复杂。
 *
 * @param is 输入流指针，限定符为__restrict
 * @param os 输出流指针，限定符为__restrict
 * @param allocator 分配器指针，限定符为__restrict
 * @param desc CDR流描述符指针，限定符为__restrict
 */
void dds_stream_extract_keyBO_from_key(dds_istream_t *__restrict is,
                                       DDS_OSTREAM_T *__restrict os,
                                       const struct dds_cdrstream_allocator *__restrict allocator,
                                       const struct dds_cdrstream_desc *__restrict desc) {
  // 遍历所有密钥
  for (uint32_t i = 0; i < desc->keys.nkeys; i++) {
    // 获取操作指针
    uint32_t const *const op = desc->ops.ops + desc->keys.keys[i].ops_offs;
    // 根据操作类型进行处理
    switch (DDS_OP(*op)) {
      case DDS_OP_KOF: {
        // 获取操作长度
        uint16_t n_offs = DDS_OP_LENGTH(*op);
        // 断言操作长度大于0
        assert(n_offs > 0);
        // 处理基本操作
        dds_stream_extract_keyBO_from_key_prim_op(is, os, allocator, desc->ops.ops + op[1],
                                                  --n_offs, op + 2);
        break;
      }
      case DDS_OP_ADR: {
        // 处理基本操作
        dds_stream_extract_keyBO_from_key_prim_op(is, os, allocator, op, 0, NULL);
        break;
      }
      default:
        // 遇到未知操作类型，中止程序
        abort();
        break;
    }
  }
}
