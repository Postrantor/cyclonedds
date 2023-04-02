/*
 * Copyright(c) 2006 to 2021 ZettaScale Technology and others
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v. 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0, or the Eclipse Distribution License
 * v. 1.0 which is available at
 * http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * SPDX-License-Identifier: EPL-2.0 OR BSD-3-Clause
 */
#ifndef DDSI_RADMIN_H
#define DDSI_RADMIN_H

#include <stddef.h>

#include "dds/ddsi/ddsi_locator.h"
#include "dds/ddsi/ddsi_protocol.h"
#include "dds/ddsrt/align.h"
#include "dds/ddsrt/atomics.h"
#include "dds/ddsrt/static_assert.h"
#include "dds/ddsrt/threads.h"
#include "dds/ddsrt/time.h"

#if defined(__cplusplus)
extern "C" {
#endif

struct ddsi_rbuf;
struct ddsi_rmsg;
struct ddsi_rdata;
struct ddsi_rsample_info;
struct ddsi_tran_conn;

/**
 * @struct ddsi_rmsg_chunk
 * @brief 用于存储接收到的数据包的一部分的结构体 (Structure for storing a part of received data
 * packet)
 */
struct ddsi_rmsg_chunk {
  struct ddsi_rbuf *rbuf;  ///< 指向与此数据包关联的接收缓冲区的指针 (Pointer to the receive buffer
                           ///< associated with this packet)
  struct ddsi_rmsg_chunk
      *next;  ///< 指向下一个数据包块的指针 (Pointer to the next data packet chunk)

  /* Size is 0 after initial allocation, must be set with
     ddsi_rmsg_setsize after receiving a packet from the kernel and
     before processing it.  */
  union {
    uint32_t size;  ///< 数据包块的大小 (Size of the data packet chunk)

    /* to ensure reasonable alignment of payload */
    int64_t l;
    double d;
    void *p;
  } u;

  /* unsigned char payload[] -- disallowed by C99 because of nesting */
};

/**
 * @struct ddsi_rmsg
 * @brief DDSI 接收消息结构 (DDSI received message structure)
 *
 * 用于处理接收到的数据包，包括引用计数、内存块管理等功能。
 * (Used for handling received packets, including reference counting, memory chunk management, etc.)
 */
struct ddsi_rmsg {
  /**
   * @brief 引用计数 (Reference count)
   *
   * 所有指向此消息的 rdatas 的引用都被计数。rdatas 本身没有引用计数。
   * (All references to rdatas of this message are counted. The rdatas themselves do not have a
   * reference count.)
   *
   * 在插入过程中，引用计数偏移为
   * RMSG_REFCOUNT_UNCOMMITED_BIAS，以便在分配内存、增加引用计数等操作时验证其仍未提交。 (The
   * refcount is biased by RMSG_REFCOUNT_UNCOMMITED_BIAS while still being inserted to allow
   * verifying it is still uncommitted when allocating memory, increasing refcounts, &c.)
   *
   * 当 rdata 离开碎片重组直到被重新排序拒绝或已安排交付时，每个 rdata 添加
   * RMS_REFCOUNT_RDATA_BIAS。 这允许在将样本添加到所有 radmins
   * 后延迟减少引用计数，尽管并发地进行交付。 (Each rdata adds RMS_REFCOUNT_RDATA_BIAS when it
   * leaves defragmentation until it has been rejected by reordering or has been scheduled for
   * delivery. This allows delaying the decrementing of refcounts until after a sample has been
   * added to all radmins even though be delivery of it may take place in concurrently.)
   */
  ddsrt_atomic_uint32_t refcount;

  /**
   * @brief 最后一个内存块 (Last memory chunk)
   *
   * 动态添加内存块，甚至整个接收缓冲区。特别对待其他块，这不是严格要求的，但也不是完全不合理的，考虑到第一个块具有引用计数和实际数据包。
   * (Dynamically add chunks of memory, and even entire receive buffers. We treat the other chunks
   * specially, which is not strictly required but also not entirely unreasonable, considering that
   * the first chunk has the refcount & the real packet.)
   */
  struct ddsi_rmsg_chunk *lastchunk;

  /**
   * @brief 是否记录日志 (Whether to log)
   *
   * 根据此标志确定是否记录接收消息的相关日志。
   * (Determine whether to log related received messages based on this flag.)
   */
  bool trace;

  /**
   * @brief 内存块 (Memory chunk)
   *
   * 用于存储接收到的数据包的内存块。
   * (Memory chunk used for storing received packets.)
   */
  struct ddsi_rmsg_chunk chunk;
};

// 确保 ddsi_rmsg 结构体的大小等于 ddsi_rmsg_chunk 结构体的偏移量加上其大小
// Ensure the size of the ddsi_rmsg structure is equal to the offset of the ddsi_rmsg_chunk
// structure plus its size
DDSRT_STATIC_ASSERT(sizeof(struct ddsi_rmsg) ==
                    offsetof(struct ddsi_rmsg, chunk) + sizeof(struct ddsi_rmsg_chunk));

// 定义宏，获取 rmsg 的有效负载地址
// Define a macro to get the payload address of rmsg
#define DDSI_RMSG_PAYLOAD(m) ((unsigned char *)(m + 1))

// 定义宏，获取 rmsg 的有效负载偏移地址
// Define a macro to get the payload offset address of rmsg
#define DDSI_RMSG_PAYLOADOFF(m, o) (DDSI_RMSG_PAYLOAD(m) + (o))

// 定义 ddsi_rdata 结构体
// Define the ddsi_rdata structure
struct ddsi_rdata {
  struct ddsi_rmsg *rmsg;       // 接收到的（并引用计数）的 rmsg
                                // Received (and refcounted) rmsg

  struct ddsi_rdata *nextfrag;  // 分片链
                                // Fragment chain

  uint32_t min, maxp1;          // 分片作为字节偏移量
                                // Fragment as byte offsets

  uint16_t submsg_zoff;         // 子消息与数据包开始的偏移量，或为 0
                                // Offset to submessage from packet start, or 0

  uint16_t payload_zoff;        // 有效负载与数据包开始的偏移量
                                // Offset to payload from packet start

  uint16_t keyhash_zoff;        // 密钥哈希与数据包开始的偏移量，或为 0
                                // Offset to keyhash from packet start, or 0

#ifndef NDEBUG
  ddsrt_atomic_uint32_t refcount_bias_added;  // 调试模式下，用于记录引用计数偏差
                                              // In debug mode, used to record reference count bias
#endif
};

/* All relative offsets in packets that we care about (submessage
   header, payload, writer info) are at multiples of 4 bytes and
   within 64kB, so technically we can make do with 14 bits instead of
   16, in case we run out of space.

   If we _really_ need to squeeze out every last bit, only the submsg
   offset really requires 14 bits, the for the others we could use an
   offset relative to the submessage header so that it is limited by
   the maximum size of the inline QoS ...  Defining the macros now, so
   we have the option to do wild things. */
/* 所有我们关心的数据包中的相对偏移量（子消息头，有效负载，写入器信息）都是 4 字节的倍数且在 64kB
   内， 因此从技术上讲，我们可以使用 14 位而不是 16 位，以防我们用完空间。

   如果我们_真的_需要挤出每一个最后一位，只有子消息偏移量确实需要 14
   位，对于其他偏移量，我们可以使用相对于子消息头的偏移量， 这样它就受到内联 QoS 的最大尺寸的限制...
   现在定义宏，这样我们就有了采取疯狂行动的选择。 */
#ifndef NDEBUG
#define DDSI_ZOFF_TO_OFF(zoff) ((unsigned)(zoff))
#define DDSI_OFF_TO_ZOFF(off) (assert((off) < 65536), ((unsigned short)(off)))
#else
#define DDSI_ZOFF_TO_OFF(zoff) ((unsigned)(zoff))
#define DDSI_OFF_TO_ZOFF(off) ((unsigned short)(off))
#endif
#define DDSI_RDATA_PAYLOAD_OFF(rdata) DDSI_ZOFF_TO_OFF((rdata)->payload_zoff)
#define DDSI_RDATA_SUBMSG_OFF(rdata) DDSI_ZOFF_TO_OFF((rdata)->submsg_zoff)
#define DDSI_RDATA_KEYHASH_OFF(rdata) DDSI_ZOFF_TO_OFF((rdata)->keyhash_zoff)

#if defined(__cplusplus)
}
#endif

#endif /* DDSI_RADMIN_H */
