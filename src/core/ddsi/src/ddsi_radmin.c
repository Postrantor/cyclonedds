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
#include <ctype.h>
#include <limits.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>

#if HAVE_VALGRIND && !defined(NDEBUG)
#include <memcheck.h>
#define USE_VALGRIND 1
#else
#define USE_VALGRIND 0
#endif

#include "dds/ddsi/ddsi_domaingv.h" /* for mattr, cattr */
#include "dds/ddsi/ddsi_log.h"
#include "dds/ddsi/ddsi_plist.h"
#include "dds/ddsi/ddsi_unused.h"
#include "dds/ddsrt/avl.h"
#include "dds/ddsrt/heap.h"
#include "dds/ddsrt/string.h"
#include "dds/ddsrt/sync.h"
#include "dds/ddsrt/threads.h"
#include "ddsi__bitset.h"
#include "ddsi__log.h"
#include "ddsi__misc.h"
#include "ddsi__protocol.h"
#include "ddsi__radmin.h"
#include "ddsi__thread.h"

/* OVERVIEW ------------------------------------------------------------

   The receive path of DDSI has any number of receive threads that
   accept data from sockets and (synchronously) push it up the
   protocol stack, potentially offloading processing to other threads
   at some point.  In particular, delivery of data can safely be
   offloaded.

   Each receive thread MUST process each message synchronously to the
   point where all additional indexing and other administrative data
   derived from the message has been stored in memory.  This storage
   is _always_ adjacent to the message that caused it.  Also, once it
   finishes processing a message, the reference count of that message
   may not be incremented anymore.

   In practice that means the receive thread can do everything by
   itself (handling acks and heartbeats, handling discovery,
   delivering data to the kernel), or it can offload everything but
   defragmentation and reordering.

   The data structures and functions in this file are all concerned
   with the storage of messages in buffers, organising their parts
   into ordered chains of fragments of (DDS) samples, reordering them
   into chains of consecutive samples, and queueing these chains for
   further processing.

   Storage is organised in the following hierarchy; rdata is included
   because it is is very intimately involved with the reference
   counting.  For the indexing structures for defragmenting and
   reordering messages, see RDATA, DEFRAG and REORDER below.

   ddsi_rbufpool

                One or more rbufs. Currently, an rbufpool is owned by
                a single receive thread, and only this thread may
                allocate memory from the rbufs contained in the pool
                and increment reference counts to the messages in it,
                while all threads may decrement these reference counts
                / release memory from it.

                (It is probably better to share the pool amongst all
                threads and make the rbuf the thing owned by this
                thread; and in fact the buffer pool isn't really
                necessary 'cos they handle multiple messages and
                therefore the malloc/free overhead is negligible.  It
                does provide a convenient location for storing some
                constant data.)

   ddsi_rbuf

                Largish buffer for receiving several UDP packets and
                for storing partially decoded and indexing information
                directly following the packet.

   ddsi_rmsg

                One message in an rbuf; the layout for one message is
                rmsg, raw udp packet, decoder stuff mixed with rdata,
                defragmentation and message reordering state.  One
                rbuf can contain many messages.

   ddsi_rdata

                Represents one Data/DataFrag submessage.  These
                contain some administrative data & point to the
                corresponding part of the message, and are referenced
                by the defragmentation and reordering (defrag, reorder)
                tables and the delivery queues.

   Each rmsg contains a reference count tracking all references to all
   rdatas contained in that message.  All data for one message in the
   rbuf (raw data, decoder info, &c.) is dependent on the refcount of
   the rmsg: once that reference count goes to zero _all_ dependent
   stuff becomes invalid immediately.

   As noted, the receive thread that owns the rbuf is the only one
   allowed to add data to it, which implies that this thread must do
   all defragmenting and reordering synchronously.  Delivery can be
   offloaded to another thread, and it remains to be seen which thread
   is best used for deserializing the data.

   The main advantage of restricting the adding of data to the buffer
   to the buffer's owning thread is that it allows us to simply append
   decoding information to the message as it becomes available while
   processing the message, without risking interference from another
   thread.  This includes decoded parameter lists/inline QoS settings,
   defragmenting information, &c.

   Once the synchronous processing of a message (a UDP packet) is
   completed, every adminstrative thing related to that message is
   contained in a single block of memory, and can be released very
   easily, regardless of whether the rbuf is a circular buffer, has a
   minimalistic heap inside it, or is simply discarded when the end is
   reached.

   Each rdata (submessage) that has been delivered (or need never be
   delivered) is not referenced anywhere and will therefore not
   contribute to rmsg::refcount, so once all rdatas of an rmsg have
   been delivered, rmsg::refcount will drop to 0.  If all submessages
   are processed by the receive thread, or delivery is delegated to
   other threads that happen to finish doing so before the receive
   thread is done processing the message, the message can be discarded
   trivially by not even updating the memory allocation info in the
   rbuf.

   Just creating an rdata is not sufficient reason for the reference
   count in the corresponding rmsg to be incremented: that happens
   once the defragmenter decides to not throw it away (either because
   it stores it or because it returns it for forwarding to reordering
   or delivery).  (Which is possible because both defragmentation and
   reordering are synchronous.)

   While synchronously processing the message, the reference count is
   biased by 2**31 just so we can detect some illegal activities.
   Furthermore, while still synchronous, each rdata contributes the
   number of actual references to the message plus 2**20 to the
   refcount.  This second bias allows delaying accounting for the
   actual references until after processing all reorder admins, saving
   us from having to update them potentially many times.

   The space needed for processing a message is limited: a UDP packet
   is never larger than 64kB (and it seems very unwise to actually use
   such large packets!), and there is only a finite amount of data
   that gets added to it while interpreting the message.  Although the
   exact amount is not yet known, it seems very unlikely that the
   decoding data for one packet would exceed 64kB size, though one had
   better be careful just in case.  So a maximum RMSG size of 128kB
   and an RBUF size of 1MB should be quite reasonable.

   Sequence of operations:

     receive_thread ()
     {
       ...
       rbpool = ddsi_rbufpool_new (1MB, 128kB)
       ...

       while ...
         rmsg = ddsi_rmsg_new (rbpool)
         actualsize = recvfrom (rmsg.payload, 64kB)
         ddsi_rmsg_setsize (rmsg, actualsize)
         process (rmsg)
         ddsi_rmsg_commit (rmsg)

       ... ensure no references to any buffer in rbpool exist ...
       ddsi_rbufpool_free (rbpool)
       ...
     }

   If there are no outstanding references to the message, commit()
   simply discards it and new() returns the same address next time
   round.

   Processing of a single message in process() is roughly as follows:

     for rdata in each Data/DataFrag submessage in rmsg
       sampleinfo.seq = XX;
       sampleinfo.fragsize = XX;
       sampleinfo.size = XX;
       sampleinfo.(others) = XX if first fragment, else not important
       sample = ddsi_defrag_rsample (pwr->defrag, rdata, &sampleinfo)
       if sample
         fragchain = ddsi_rsample_fragchain (sample)
         refcount_adjust = 0;

         if send-to-proxy-writer-reorder
           if ddsi_reorder_rsample (&sc, pwr->reorder, sample, &refcount_adjust)
              == DELIVER
             deliver-to-group (pwr, sc)
         else
           for (m in out-of-sync-reader-matches)
             sample' = ddsi_reorder_rsample_dup (rmsg, sample)
             if ddsi_reorder_rsample (&sc, m->reorder, sample, &refcount_adjust)
                == DELIVER
               deliver-to-reader (m->reader, sc)

         ddsi_fragchain_adjust_refcount (fragchain, refcount_adjust)
       fi
     rof

   Where deliver-to-x() must of course decrement refcounts after
   delivery when done, using ddsi_fragchain_unref().  See also REORDER
   for the subtleties of the refcount game.

   Note that there is an alternative to all this trickery with
   fragment chains and deserializing off these fragments chains:
   allocating sufficient memory upon reception of the first fragment,
   and then just memcpy'ing the bytes in, with a simple bitmask to
   keep track of which fragments have been received and which have not
   yet been.

   _The_ argument against that is a very unreliable network with huge
   messages: the way we do it here never needs more than a constant
   factor over what is actually received, whereas the simple
   alternative would blow up nearly instantaneously.  Maybe not if you
   drop samples halfway through defragmenting aggressively, but then
   you can't get anything through anymore if there are multiple
   writers.

   Gaps and Heartbeats prune the defragmenting index and are (when
   needed) stored as intervals of specially marked rdatas in the
   reordering indices.

   The procedure for a Gap is:

     for a Gap [a,b] in rmsg
       defrag_notegap (a, b+1)
       refcount_adjust = 0
       gap = ddsi_rdata_newgap (rmsg);
       if ddsi_reorder_gap (&sc, reorder, gap, a, b+1, &refcount_adjust)
         deliver-to-group (pwr, sc)
       for (m in out-of-sync-reader-matches)
         if ddsi_reorder_gap (&sc, m->reorder, gap, a, b+1, &refcount_adjust)
           deliver-to-reader (m->reader, sc)
       ddsi_fragchain_adjust_refcount (gap, refcount_adjust)

   Note that a Gap always gets processed both by the primary and by
   the secondary reorder admins.  This is because it covers a range.

   A heartbeat is similar, except that a heartbeat [a,b] results in a
   gap [1,a-1]. */
/* 概述 ------------------------------------------------------------

   DDSI接收路径有任意数量的接收线程，它们从套接字接收数据并（同步地）将其推送到
   协议栈，可能在某个时候将处理卸载到其他线程。特别是，可以安全地卸载数据传递。

   每个接收线程必须同步处理每条消息，直到所有从消息中派生的附加索引和其他管理数据
   已存储在内存中。这个存储 _始终_ 与导致它的消息相邻。此外，一旦它完成处理消息，
   该消息的引用计数可能不会再增加。

   实际上，这意味着接收线程可以自己完成所有操作（处理ack和心跳，处理发现，
   将数据传递给内核），或者它可以卸载除去碎片化和重新排序之外的所有操作。

   本文件中的数据结构和函数都涉及到将消息存储在缓冲区中，将它们的部分组织成
   （DDS）样本的有序片段链，将它们重新排序成连续样本的链，并将这些链排队以进行
   进一步处理。

   存储按以下层次结构组织；rdata包含在其中，因为它与引用计数密切相关。有关
   对消息进行碎片化和重新排序的索引结构，请参见下面的RDATA、DEFRAG和REORDER。

   ddsi_rbufpool

                一个或多个rbuf。目前，一个rbufpool由一个接收线程拥有，
                只有这个线程可以从池中包含的rbuf分配内存，并增加对其中消息的
                引用计数，而所有线程都可以减少这些引用计数/从中释放内存。

                （在所有线程之间共享池并使rbuf成为此线程拥有的东西可能更好；
                实际上，缓冲池并不是真正必要的，因为它们处理多个消息，因此
                malloc/free开销可以忽略不计。它确实为存储一些常量数据提供了
                方便的位置。）

   ddsi_rbuf

                用于接收多个UDP数据包的较大缓冲区以及直接跟随数据包的部分解码
                和索引信息的存储。

   ddsi_rmsg

                rbuf中的一条消息；一条消息的布局是rmsg，原始udp数据包，
                与rdata混合的解码器内容，碎片化和消息重新排序状态。一个
                rbuf可以包含许多消息。

   ddsi_rdata

                表示一个Data/DataFrag子消息。这些包含一些管理数据并指向
                消息的相应部分，由碎片化和重新排序（defrag，reorder）表和
                传递队列引用。

   每个rmsg都包含一个引用计数，跟踪该消息中包含的所有rdatas的所有引用。rbuf中一个消息的所有数据（原始数据、解码器信息等）都依赖于rmsg的引用计数：一旦引用计数变为零，所有依赖的内容立即失效。

   如前所述，拥有rbuf的接收线程是唯一允许向其添加数据的线程，这意味着此线程必须同步地执行所有碎片整理和重新排序。传递可以卸载到另一个线程，尚待确定哪个线程最适合用于反序列化数据。

   将缓冲区数据添加限制在缓冲区拥有线程的主要优势是，它允许我们在处理消息时简单地将解码信息附加到消息上，而不会冒着受到其他线程干扰的风险。这包括解码参数列表/内联QoS设置、碎片整理信息等。

   一旦完成对消息（UDP数据包）的同步处理，与该消息相关的每个管理事务都包含在一个内存块中，并且可以非常轻松地释放，无论rbuf是循环缓冲区，还是在其中具有极简堆，或者在达到末尾时直接丢弃。

   每个已交付（或永远不需要交付）的rdata都没有在任何地方引用，因此不会对rmsg::refcount产生贡献，所以一旦所有rmsg的rdatas都被交付，rmsg::refcount将降至0。如果接收线程处理了所有子消息，或者传递委托给其他线程，而这些线程在接收线程完成处理消息之前就完成了传递，那么可以通过不更新rbuf中的内存分配信息来轻松丢弃消息。

   仅创建rdata并不足以使相应rmsg中的引用计数递增：只有当碎片整理器决定不丢弃它时（要么存储它，要么返回它以便转发到重新排序或传递），才会发生这种情况。（这是可能的，因为碎片整理和重新排序都是同步的。）

   在同步处理消息时，引用计数偏移2**31，以便我们可以检测到一些非法活动。此外，在同步过程中，每个rdata将实际引用到消息的数量加上2**20的值贡献给refcount。这第二个偏差允许在处理所有重新排序管理员后再延迟计算实际引用，从而避免了潜在多次更新它们的问题。

   处理消息所需的空间是有限的：UDP数据包永远不会大于64kB（实际使用如此大的数据包似乎非常不明智！），在解释消息时只有有限数量的数据被添加到其中。尽管确切的数量尚未知晓，但一个数据包的解码数据大小超过64kB似乎非常不可能，尽管如此，最好还是小心为妙。因此，最大RMSG大小为128kB和RBUF大小为1MB应该是相当合理的。

   操作顺序：

     receive_thread ()
     {
       ...
       rbpool = ddsi_rbufpool_new (1MB, 128kB)
       ...

       while ...
         rmsg = ddsi_rmsg_new (rbpool)
         actualsize = recvfrom (rmsg.payload, 64kB)
         ddsi_rmsg_setsize (rmsg, actualsize)
         process (rmsg)
         ddsi_rmsg_commit (rmsg)

       ... 确保不存在对 rbpool 中任何缓冲区的引用 ...
       ddsi_rbufpool_free (rbpool)
       ...
     }

   如果没有对消息的未完成引用，commit()
   只需丢弃它，new() 下次返回相同的地址。

   在 process() 中处理单个消息大致如下：

     for rdata in each Data/DataFrag submessage in rmsg
       sampleinfo.seq = XX;
       sampleinfo.fragsize = XX;
       sampleinfo.size = XX;
       sampleinfo.(others) = XX if first fragment, else not important
       sample = ddsi_defrag_rsample (pwr->defrag, rdata, &sampleinfo)
       if sample
         fragchain = ddsi_rsample_fragchain (sample)
         refcount_adjust = 0;

         if send-to-proxy-writer-reorder
           if ddsi_reorder_rsample (&sc, pwr->reorder, sample, &refcount_adjust)
              == DELIVER
             deliver-to-group (pwr, sc)
         else
           for (m in out-of-sync-reader-matches)
             sample' = ddsi_reorder_rsample_dup (rmsg, sample)
             if ddsi_reorder_rsample (&sc, m->reorder, sample, &refcount_adjust)
                == DELIVER
               deliver-to-reader (m->reader, sc)

         ddsi_fragchain_adjust_refcount (fragchain, refcount_adjust)
       fi
     rof

   其中，deliver-to-x() 在完成交付后必须使用 ddsi_fragchain_unref() 减少引用计数。
   另请参阅 REORDER 了解引用计数游戏的微妙之处。

   请注意，对于所有这些与片段链和反序列化片段链相关的技巧，还有一种替代方法：
   在接收到第一个片段时分配足够的内存，然后只需将字节 memcpy 进去，
   使用简单的位掩码来跟踪哪些片段已经接收到，哪些尚未接收到。

   针对这种方法的_论据_是非常不可靠的网络和巨大的消息：我们在这里所做的方式永远不需要超过实际接收到的常数因子，
   而简单的替代方案几乎会立即爆炸。也许如果你在解碎片化过程中途丢弃样本，情况就不会这样了，
   但是如果有多个写入器，你就无法再通过任何东西了。

   Gaps 和 Heartbeats 修剪解碎片化索引，并在需要时作为特殊标记的 rdatas 的间隔存储在重新排序索引中。

   Gap 的过程如下：

     for a Gap [a,b] in rmsg
       defrag_notegap (a, b+1)
       refcount_adjust = 0
       gap = ddsi_rdata_newgap (rmsg);
       if ddsi_reorder_gap (&sc, reorder, gap, a, b+1, &refcount_adjust)
         deliver-to-group (pwr, sc)
       for (m in out-of-sync-reader-matches)
         if ddsi_reorder_gap (&sc, m->reorder, gap, a, b+1, &refcount_adjust)
           deliver-to-reader (m->reader, sc)
       ddsi_fragchain_adjust_refcount (gap, refcount_adjust)

   请注意，Gap 总是由主要和次要重新排序管理员处理。这是因为它涵盖了一个范围。

   心跳类似，只是心跳[a,b]导致间隙[1,a-1]。*/

/* RBUFPOOL ------------------------------------------------------------ */

/**
 * @struct ddsi_rbufpool
 * @brief 一个接收缓冲区池结构，用于管理接收缓冲区资源。
 *        A receive buffer pool structure for managing receive buffer resources.
 *
 * 接收缓冲区池由接收线程拥有，且该线程是唯一从池中分配 rmsgs
 * 的线程。任何线程都可以在缓冲区变为空时将其释放回池中。 An rbuf pool is owned by a receive thread,
 * and that thread is the only allocating rmsgs from the rbufs in the pool. Any thread may be
 * releasing buffers to the pool as they become empty.
 *
 * 目前，我们只维护一个当前的 rbuf，当从中分配新的 rbuf 失败时，它会被替换。如果与当前 rbuf
 * 不同，则完全释放已释放的任何 rbuf。 Currently, we only maintain a current rbuf, which gets
 * replaced when allocating a new one from it fails. Any rbufs that are released are freed
 * completely if different from the current one.
 *
 * 可以轻松地实现无锁操作，但需要比较和交换，而我们没有这个功能。不过，这种情况几乎不会发生。
 * Could trivially be done lockless, except that it requires compare-and-swap, and we don't have
 * that. But it hardly ever happens anyway.
 */
struct ddsi_rbufpool {
  ddsrt_mutex_t
      lock; /**< 锁，用于保护对此结构的访问。Lock for protecting access to this structure. */
  struct ddsi_rbuf *current; /**< 当前正在使用的接收缓冲区。Current receive buffer in use. */
  uint32_t rbuf_size;        /**< 接收缓冲区的大小。Size of the receive buffer. */
  uint32_t max_rmsg_size; /**< 最大接收消息大小。Maximum receive message size. */
  const struct ddsrt_log_cfg *logcfg; /**< 日志配置。Log configuration. */
  bool trace;                         /**< 是否启用跟踪。Whether tracing is enabled. */

#ifndef NDEBUG
  /**
   * 拥有此池的线程，以便我们可以检查没有其他线程调用仅所有者可以使用的函数。
   * Thread that owns this pool, so we can check that no other thread is calling functions only the
   * owner may use.
   */
  ddsrt_thread_t owner_tid;
#endif
};

/**
 * @brief 为新的ddsi_rbuf分配内存
 * @param rbp ddsi_rbufpool指针
 * @return 分配的ddsi_rbuf结构指针
 *
 * @brief Allocate memory for a new ddsi_rbuf
 * @param rbp Pointer to ddsi_rbufpool
 * @return Pointer to the allocated ddsi_rbuf structure
 */
static struct ddsi_rbuf *ddsi_rbuf_alloc_new(struct ddsi_rbufpool *rbp);

/**
 * @brief 释放ddsi_rbuf的内存
 * @param rbuf 要释放的ddsi_rbuf指针
 *
 * @brief Release memory of a ddsi_rbuf
 * @param rbuf Pointer to the ddsi_rbuf to be released
 */
static void ddsi_rbuf_release(struct ddsi_rbuf *rbuf);

// 定义TRACE宏，用于在启用跟踪时输出日志
// Define TRACE macros for logging when tracing is enabled
#define TRACE_CFG(obj, logcfg, ...) \
  ((obj)->trace ? (void)DDS_CLOG(DDS_LC_RADMIN, (logcfg), __VA_ARGS__) : (void)0)
#define TRACE(obj, ...) TRACE_CFG((obj), (obj)->logcfg, __VA_ARGS__)
#define RBPTRACE(...) TRACE_CFG(rbp, rbp->logcfg, __VA_ARGS__)
#define RBUFTRACE(...) TRACE_CFG(rbuf, rbuf->rbufpool->logcfg, __VA_ARGS__)
#define RMSGTRACE(...) TRACE_CFG(rmsg, rmsg->chunk.rbuf->rbufpool->logcfg, __VA_ARGS__)
#define RDATATRACE(rdata, ...) \
  TRACE_CFG((rdata)->rmsg, (rdata)->rmsg->chunk.rbuf->rbufpool->logcfg, __VA_ARGS__)

/**
 * @brief 对齐rmsg的大小
 * @param x 要对齐的值
 * @return 对齐后的值
 *
 * @brief Align the size of rmsg
 * @param x The value to be aligned
 * @return The aligned value
 */
static uint32_t align_rmsg(uint32_t x) {
  x += (uint32_t)DDSI_ALIGNOF_RMSG - 1;
  x -= x % (uint32_t)DDSI_ALIGNOF_RMSG;
  return x;
}

#ifndef NDEBUG
// 如果没有定义 NDEBUG，则定义 ASSERT_RBUFPOOL_OWNER(rbp) 宏，用于检查当前线程是否为 rbp 的所有者
// If NDEBUG is not defined, define the ASSERT_RBUFPOOL_OWNER(rbp) macro to check if the current
// thread is the owner of rbp
#define ASSERT_RBUFPOOL_OWNER(rbp) \
  (assert(ddsrt_thread_equal(ddsrt_thread_self(), (rbp)->owner_tid)))
#else
// 如果定义了 NDEBUG，则定义 ASSERT_RBUFPOOL_OWNER(rbp) 为空操作
// If NDEBUG is defined, define ASSERT_RBUFPOOL_OWNER(rbp) as a no-op
#define ASSERT_RBUFPOOL_OWNER(rbp) ((void)(0))
#endif

// 定义一个静态函数 max_uint32，返回两个无符号 32 位整数中的较大值
// Define a static function max_uint32 that returns the larger of two unsigned 32-bit integers
static uint32_t max_uint32(uint32_t a, uint32_t b) { return a >= b ? a : b; }

// 定义一个静态函数 max_rmsg_size_w_hdr，计算最大消息大小（包括头部）
// Define a static function max_rmsg_size_w_hdr that calculates the maximum message size (including
// header)
static uint32_t max_rmsg_size_w_hdr(uint32_t max_rmsg_size) {
  /* rbuf_alloc allocates max_rmsg_size, which is actually max
     _payload_ size (this is so 64kB max_rmsg_size always suffices for
     a UDP packet, regardless of internal structure).  We use it for
     ddsi_rmsg and ddsi_rmsg_chunk, but the difference in size is
     negligible really.  So in the interest of simplicity, we always
     allocate for the worst case, and may waste a few bytes here or
     there. */
  // 计算最大消息大小（包括头部），考虑到 ddsi_rmsg 和 ddsi_rmsg_chunk
  // 的大小差异可以忽略不计，我们总是为最坏情况分配空间，可能会浪费一些字节 Calculate the maximum
  // message size (including header), considering that the size difference between ddsi_rmsg and
  // ddsi_rmsg_chunk can be ignored, we always allocate space for the worst case, which may waste
  // some bytes
  return max_uint32((uint32_t)(offsetof(struct ddsi_rmsg, chunk) + sizeof(struct ddsi_rmsg_chunk)),
                    (uint32_t)sizeof(struct ddsi_rmsg_chunk)) +
         max_rmsg_size;
}

/**
 * @brief 创建一个新的ddsi_rbufpool结构体实例
 * Create a new instance of ddsi_rbufpool structure
 *
 * @param[in] logcfg 日志配置指针 Pointer to the logging configuration
 * @param[in] rbuf_size 接收缓冲区大小 Receive buffer size
 * @param[in] max_rmsg_size 最大接收消息大小 Maximum receive message size
 * @return 成功时返回ddsi_rbufpool指针，失败时返回NULL Pointer to ddsi_rbufpool on success, NULL on
 * failure
 */
struct ddsi_rbufpool *ddsi_rbufpool_new(const struct ddsrt_log_cfg *logcfg,
                                        uint32_t rbuf_size,
                                        uint32_t max_rmsg_size) {
  struct ddsi_rbufpool *rbp;

  // 断言：确保最大接收消息大小大于0
  // Assert: Ensure maximum receive message size is greater than 0
  assert(max_rmsg_size > 0);

  /* 提高rbuf_size到考虑max_rmsg_size的最小可能值，没有理由在用户尝试配置时打扰他们，
     当rbuf_size太小时，崩溃是可怕的 */
  /* Raise rbuf_size to minimum possible considering max_rmsg_size, there is
     no reason to bother the user with the small difference between the two
     when he tries to configure things, and the crash is horrible when
     rbuf_size is too small */
  if (rbuf_size < max_rmsg_size_w_hdr(max_rmsg_size))
    rbuf_size = max_rmsg_size_w_hdr(max_rmsg_size);

  // 分配内存给ddsi_rbufpool结构体
  // Allocate memory for the ddsi_rbufpool structure
  if ((rbp = ddsrt_malloc(sizeof(*rbp))) == NULL) goto fail_rbp;
#ifndef NDEBUG
  rbp->owner_tid = ddsrt_thread_self();
#endif

  // 初始化互斥锁
  // Initialize the mutex lock
  ddsrt_mutex_init(&rbp->lock);

  // 设置ddsi_rbufpool结构体的属性
  // Set the attributes of the ddsi_rbufpool structure
  rbp->rbuf_size = rbuf_size;
  rbp->max_rmsg_size = max_rmsg_size;
  rbp->logcfg = logcfg;
  rbp->trace = (logcfg->c.mask & DDS_LC_RADMIN) != 0;

#if USE_VALGRIND
  VALGRIND_CREATE_MEMPOOL(rbp, 0, 0);
#endif

  // 为ddsi_rbufpool分配新的接收缓冲区
  // Allocate a new receive buffer for the ddsi_rbufpool
  if ((rbp->current = ddsi_rbuf_alloc_new(rbp)) == NULL) goto fail_rbuf;
  return rbp;

fail_rbuf:
#if USE_VALGRIND
  VALGRIND_DESTROY_MEMPOOL(rbp);
#endif
  // 销毁互斥锁并释放内存
  // Destroy the mutex lock and free memory
  ddsrt_mutex_destroy(&rbp->lock);
  ddsrt_free(rbp);
fail_rbp:
  return NULL;
}

/*
是的，这里的 `rbuf` 是指环形缓冲区（ring
buffer）。环形缓冲区是一种数据结构，它将固定大小的缓冲区视为循环的，即当到达缓冲区末尾时，下一个元素会被放置在缓冲区的开头。这种数据结构特别适用于生产者-消费者场景，其中生产者和消费者以不同的速度处理数据。

环形缓冲区的优点是它可以避免内存分配和释放的开销，因为缓冲区的大小是固定的。此外，环形缓冲区可以在不使用锁的情况下实现多线程安全，只要生产者和消费者访问不同的缓冲区部分。

在这段代码中，`rbuf` 用于存储接收到的消息。`ddsi_rbufpool_new` 函数创建一个新的 `ddsi_rbufpool`
结构体实例，该实例包含一个环形缓冲区，其大小由 `rbuf_size` 参数指定。`max_rmsg_size`
参数表示可以接收的最大消息大小。环形缓冲区用于在 Cyclone DDS 项目中高效地处理接收到的消息。
*/

/**
 * @brief 设置 rbufpool 的所有者线程 ID
 * @param[in] rbp 一个指向 ddsi_rbufpool 结构体的指针
 * @param[in] tid 要设置为所有者的线程 ID
 *
 * Set the owner thread ID of the rbufpool.
 *
 * @param[in] rbp A pointer to a ddsi_rbufpool structure
 * @param[in] tid The thread ID to be set as the owner
 */
void ddsi_rbufpool_setowner(UNUSED_ARG_NDEBUG(struct ddsi_rbufpool *rbp),
                            UNUSED_ARG_NDEBUG(ddsrt_thread_t tid)) {
#ifndef NDEBUG
  // 设置 rbp 的所有者线程 ID 为 tid
  // Set the owner thread ID of rbp to tid
  rbp->owner_tid = tid;
#endif
}

/**
 * @brief 释放 rbufpool 结构体
 * @param[in] rbp 一个指向 ddsi_rbufpool 结构体的指针
 *
 * Free the rbufpool structure.
 *
 * @param[in] rbp A pointer to a ddsi_rbufpool structure
 */
void ddsi_rbufpool_free(struct ddsi_rbufpool *rbp) {
#if 0
  /* 任何人都可以释放它：我希望能够停止接收线程，然后停止所有其他异步处理，然后清除缓冲区。
     这是验证引用计数是否为 0 的唯一方法，因为它们应该是这样的。*/
  /* Anyone may free it: I want to be able to stop the receive
     threads, then stop all other asynchronous processing, then clear
     out the buffers.  That's is the only way to verify that the
     reference counts are all 0, as they should be. */
  ASSERT_RBUFPOOL_OWNER (rbp);
#endif
  // 释放 rbp 当前的缓冲区
  // Release the current buffer of rbp
  ddsi_rbuf_release(rbp->current);
#if USE_VALGRIND
  // 销毁 Valgrind 内存池
  // Destroy the Valgrind memory pool
  VALGRIND_DESTROY_MEMPOOL(rbp);
#endif
  // 销毁 rbp 的互斥锁
  // Destroy the mutex lock of rbp
  ddsrt_mutex_destroy(&rbp->lock);
  // 释放 rbp 结构体
  // Free the rbp structure
  ddsrt_free(rbp);
}

/* RBUF ---------------------------------------------------------------- */

/** ddsi_rbuf 结构体定义
 *  @brief 接收缓冲区结构，用于存储接收到的数据
 */
struct ddsi_rbuf {
  ddsrt_atomic_uint32_t
      n_live_rmsg_chunks; /**< 当前活跃的消息块数量 (Number of currently active message chunks) */
  uint32_t size;          /**< 缓冲区大小 (Buffer size) */
  uint32_t max_rmsg_size; /**< 最大消息大小 (Maximum message size) */
  struct ddsi_rbufpool
      *rbufpool; /**< 指向所属的缓冲池指针 (Pointer to the associated buffer pool) */
  bool trace;    /**< 跟踪标志 (Trace flag) */

  /* 分配顺序内存，随机释放顺序，不立即重用可用内存。
   * 我认为这最终需要改变，但这是最简单的方法。更改将限制在 rmsg_new 和 rmsg_free 中。
   * (Allocating sequentially, releasing in random order, not bothering
   * to reuse memory as soon as it becomes available again. I think
   * this will have to change eventually, but this is the easiest
   * approach. Changes would be confined to rmsg_new and rmsg_free.)
   */
  unsigned char *freeptr; /**< 指向下一个空闲位置的指针 (Pointer to the next free position) */

  /* 确保 raw[] 的合理对齐 (to ensure reasonable alignment of raw[]) */
  union {
    int64_t l;
    double d;
    void *p;
  } u;

  /* 原始数据数组，实际长度为 ddsi_rbuf::size 字节
   * (raw data array, ddsi_rbuf::size bytes long in reality)
   */
  unsigned char raw[];
};

/**
 * @brief 分配并初始化一个新的 ddsi_rbuf 结构体
 * @param[in] rbp 指向 ddsi_rbufpool 结构体的指针
 * @return 成功时返回指向新分配的 ddsi_rbuf 结构体的指针，失败时返回 NULL
 */
static struct ddsi_rbuf *ddsi_rbuf_alloc_new(struct ddsi_rbufpool *rbp) {
  struct ddsi_rbuf *rb;
  ASSERT_RBUFPOOL_OWNER(rbp);

  // 为 ddsi_rbuf 结构体及其缓冲区分配内存
  // (Allocate memory for the ddsi_rbuf structure and its buffer)
  if ((rb = ddsrt_malloc(sizeof(struct ddsi_rbuf) + rbp->rbuf_size)) == NULL) return NULL;
#if USE_VALGRIND
  VALGRIND_MAKE_MEM_NOACCESS(rb->raw, rbp->rbuf_size);
#endif

  // 初始化 ddsi_rbuf 结构体的成员变量
  // (Initialize the members of the ddsi_rbuf structure)
  rb->rbufpool = rbp;
  ddsrt_atomic_st32(&rb->n_live_rmsg_chunks, 1);
  rb->size = rbp->rbuf_size;
  rb->max_rmsg_size = rbp->max_rmsg_size;
  rb->freeptr = rb->raw;
  rb->trace = rbp->trace;
  RBPTRACE("rbuf_alloc_new(%p) = %p\n", (void *)rbp, (void *)rb);
  return rb;
}

/**
 * @brief 创建一个新的ddsi_rbuf对象
 * @param rbp 指向ddsi_rbufpool结构体的指针
 * @return 返回创建的ddsi_rbuf对象指针，如果分配失败则返回NULL
 *
 * Create a new ddsi_rbuf object
 * @param rbp Pointer to the ddsi_rbufpool structure
 * @return Returns a pointer to the created ddsi_rbuf object, or NULL if allocation fails
 */
static struct ddsi_rbuf *ddsi_rbuf_new(struct ddsi_rbufpool *rbp) {
  // 定义一个ddsi_rbuf类型的指针变量
  // Define a pointer variable of type ddsi_rbuf
  struct ddsi_rbuf *rb;

  // 断言rbp->current不为空
  // Assert that rbp->current is not NULL
  assert(rbp->current);

  // 断言当前线程拥有rbp
  // Assert that the current thread owns rbp
  ASSERT_RBUFPOOL_OWNER(rbp);

  // 尝试为新的ddsi_rbuf对象分配内存
  // Attempt to allocate memory for the new ddsi_rbuf object
  if ((rb = ddsi_rbuf_alloc_new(rbp)) != NULL) {
    // 锁定rbp的互斥锁
    // Lock the mutex of rbp
    ddsrt_mutex_lock(&rbp->lock);

    // 释放rbp->current所指向的资源
    // Release the resources pointed to by rbp->current
    ddsi_rbuf_release(rbp->current);

    // 更新rbp->current为新创建的ddsi_rbuf对象
    // Update rbp->current to the newly created ddsi_rbuf object
    rbp->current = rb;

    // 解锁rbp的互斥锁
    // Unlock the mutex of rbp
    ddsrt_mutex_unlock(&rbp->lock);
  }

  // 返回新创建的ddsi_rbuf对象指针
  // Return the pointer to the newly created ddsi_rbuf object
  return rb;
}

/**
 * @brief 释放ddsi_rbuf对象
 * @param rbuf 指向ddsi_rbuf结构体的指针
 *
 * Release a ddsi_rbuf object
 * @param rbuf Pointer to the ddsi_rbuf structure
 */
static void ddsi_rbuf_release(struct ddsi_rbuf *rbuf) {
  // 获取rbuf所属的ddsi_rbufpool对象
  // Get the ddsi_rbufpool object that rbuf belongs to
  struct ddsi_rbufpool *rbp = rbuf->rbufpool;

  // 打印调试信息
  // Print debug information
  RBPTRACE("rbuf_release(%p) pool %p current %p\n", (void *)rbuf, (void *)rbp,
           (void *)rbp->current);

  // 如果rbuf的n_live_rmsg_chunks原子递减后等于1，则释放rbuf
  // If the atomic decrement of n_live_rmsg_chunks in rbuf equals 1, release rbuf
  if (ddsrt_atomic_dec32_ov(&rbuf->n_live_rmsg_chunks) == 1) {
    // 打印调试信息
    // Print debug information
    RBPTRACE("rbuf_release(%p) free\n", (void *)rbuf);

    // 释放rbuf内存
    // Release the memory of rbuf
    ddsrt_free(rbuf);
  }
}

/* RMSG ---------------------------------------------------------------- */

/* 一个rmsg最多有64kB / 32B = 2**11个rdatas，因为一个rmsg的大小限制为64kB，
   而一个Data子消息的大小至少为32B。使用1位表示已提交/未提交（仅用于调试目的），
   可以容纳多达2**20个不同步的读取器与一个代理写入器匹配。我相信在一个节点上
   针对一个主题/分区拥有100万个读取器的可能性非常小... */
/* There are at most 64kB / 32B = 2**11 rdatas in one rmsg, because an
   rmsg is limited to 64kB and a Data submessage is at least 32B bytes
   in size. With 1 bit taken for committed/uncommitted (needed for
   debugging purposes only), there's room for up to 2**20 out-of-sync
   readers matched to one proxy writer. It is considered unlikely that
   anyone will ever attempt to have 1 million readers on one node to
   one topic/partition ... */
#define RMSG_REFCOUNT_UNCOMMITTED_BIAS (1u << 31)
#define RMSG_REFCOUNT_RDATA_BIAS (1u << 20)

#ifndef NDEBUG
// 断言rmsg未提交
// Assert rmsg uncommitted
#define ASSERT_RMSG_UNCOMMITTED(rmsg) \
  (assert(ddsrt_atomic_ld32(&(rmsg)->refcount) >= RMSG_REFCOUNT_UNCOMMITTED_BIAS))
#else
#define ASSERT_RMSG_UNCOMMITTED(rmsg) ((void)0)
#endif

static void *ddsi_rbuf_alloc(struct ddsi_rbufpool *rbp) {
  // 注意：只有一个线程在池上调用ddsi_rmsg_new
  // Note: only one thread calls ddsi_rmsg_new on a pool
  uint32_t asize = max_rmsg_size_w_hdr(rbp->max_rmsg_size);
  struct ddsi_rbuf *rb;
  RBPTRACE("rmsg_rbuf_alloc(%p, %" PRIu32 ")\n", (void *)rbp, asize);
  ASSERT_RBUFPOOL_OWNER(rbp);
  rb = rbp->current;
  assert(rb != NULL);
  assert(rb->freeptr >= rb->raw);
  assert(rb->freeptr <= rb->raw + rb->size);

  if ((uint32_t)(rb->raw + rb->size - rb->freeptr) < asize) {
    // 新的rmsg空间不足
    // Not enough space left for new rmsg
    if ((rb = ddsi_rbuf_new(rbp)) == NULL) return NULL;

    // 新的rbuf应该有足够的空间
    // A new rbuf should have plenty of space
    assert((uint32_t)(rb->raw + rb->size - rb->freeptr) >= asize);
  }

  RBPTRACE("rmsg_rbuf_alloc(%p, %" PRIu32 ") = %p\n", (void *)rbp, asize, (void *)rb->freeptr);
#if USE_VALGRIND
  VALGRIND_MEMPOOL_ALLOC(rbp, rb->freeptr, asize);
#endif
  return rb->freeptr;
}

static void init_rmsg_chunk(struct ddsi_rmsg_chunk *chunk, struct ddsi_rbuf *rbuf) {
  chunk->rbuf = rbuf;
  chunk->next = NULL;
  chunk->u.size = 0;
  // 增加活动的rmsg块计数
  // Increment the count of live rmsg chunks
  ddsrt_atomic_inc32(&rbuf->n_live_rmsg_chunks);
}

/**
 * @brief 为给定的缓冲池分配一个新的 rmsg 结构体
 * @param[in] rbp 指向 ddsi_rbufpool 结构体的指针
 * @return 成功时返回指向新分配的 ddsi_rmsg 结构体的指针，失败时返回 NULL
 *
 * @brief Allocate a new rmsg structure for the given buffer pool
 * @param[in] rbp Pointer to the ddsi_rbufpool structure
 * @return On success, returns a pointer to the newly allocated ddsi_rmsg structure; on failure,
 * returns NULL
 */
struct ddsi_rmsg *ddsi_rmsg_new(struct ddsi_rbufpool *rbp) {
  // 注意：只有一个线程在一个池上调用 ddsi_rmsg_new
  // Note: only one thread calls ddsi_rmsg_new on a pool
  struct ddsi_rmsg *rmsg;
  RBPTRACE("rmsg_new(%p)\n", (void *)rbp);

  // 从缓冲池中分配一个 rmsg 结构体
  // Allocate an rmsg structure from the buffer pool
  rmsg = ddsi_rbuf_alloc(rbp);
  if (rmsg == NULL) return NULL;

  // 对这个 rmsg 的引用，由 rmsg_commit() 撤销
  // Reference to this rmsg, undone by rmsg_commit()
  ddsrt_atomic_st32(&rmsg->refcount, RMSG_REFCOUNT_UNCOMMITTED_BIAS);
  // 初始化 chunk
  // Initialize chunk
  init_rmsg_chunk(&rmsg->chunk, rbp->current);
  rmsg->trace = rbp->trace;
  rmsg->lastchunk = &rmsg->chunk;
  // 在 commit() 中递增 freeptr，以便丢弃消息非常简单
  // Incrementing freeptr happens in commit(), so that discarding the message is really simple
  RBPTRACE("rmsg_new(%p) = %p\n", (void *)rbp, (void *)rmsg);
  return rmsg;
}

/**
 * @brief 设置 rmsg 结构体的大小
 * @param[in,out] rmsg 指向 ddsi_rmsg 结构体的指针
 * @param[in] size 要设置的大小（字节）
 *
 * @brief Set the size of an rmsg structure
 * @param[in,out] rmsg Pointer to the ddsi_rmsg structure
 * @param[in] size The size to set (in bytes)
 */
void ddsi_rmsg_setsize(struct ddsi_rmsg *rmsg, uint32_t size) {
  uint32_t size8P = align_rmsg(size);
  RMSGTRACE("rmsg_setsize(%p, %" PRIu32 " => %" PRIu32 ")\n", (void *)rmsg, size, size8P);
  ASSERT_RBUFPOOL_OWNER(rmsg->chunk.rbuf->rbufpool);
  ASSERT_RMSG_UNCOMMITTED(rmsg);
  assert(ddsrt_atomic_ld32(&rmsg->refcount) == RMSG_REFCOUNT_UNCOMMITTED_BIAS);
  assert(rmsg->chunk.u.size == 0);
  assert(size8P <= rmsg->chunk.rbuf->max_rmsg_size);
  assert(rmsg->lastchunk == &rmsg->chunk);
  rmsg->chunk.u.size = size8P;
#if USE_VALGRIND
  VALGRIND_MEMPOOL_CHANGE(rmsg->chunk.rbuf->rbufpool, rmsg, rmsg,
                          offsetof(struct ddsi_rmsg, chunk.u.payload) + rmsg->chunk.size);
#endif
}

/**
 * @brief 释放 ddsi_rmsg 结构体内存 (Free the memory of a ddsi_rmsg structure)
 *
 * @param[in] rmsg 要释放的 ddsi_rmsg 结构体指针 (Pointer to the ddsi_rmsg structure to be freed)
 */
void ddsi_rmsg_free(struct ddsi_rmsg *rmsg) {
  /* 注意：任何线程都可以调用 rmsg_free。 (Note: any thread may call rmsg_free.) */

  /* FIXME: 注意我们可以通过在此 rmsg 后面有空闲空间的情况下移动 rbuf->freeptr 来优化。
     除非这需要同步 new() 和 free()，而我们目前没有这样做。理想情况下，您将使用 compare-and-swap
     来实现这一点。 */
  struct ddsi_rmsg_chunk *c;
  RMSGTRACE("rmsg_free(%p)\n", (void *)rmsg);

  // 确保 rmsg 的引用计数为 0 (Assert that the reference count of rmsg is 0)
  assert(ddsrt_atomic_ld32(&rmsg->refcount) == 0);
  c = &rmsg->chunk;

  // 遍历并释放所有关联的 ddsi_rmsg_chunk 结构体 (Iterate and free all associated ddsi_rmsg_chunk
  // structures)
  while (c) {
    struct ddsi_rbuf *rbuf = c->rbuf;
    struct ddsi_rmsg_chunk *c1 = c->next;
#if USE_VALGRIND
    if (c == &rmsg->chunk) {
      VALGRIND_MEMPOOL_FREE(rbuf->rbufpool, rmsg);
    } else {
      VALGRIND_MEMPOOL_FREE(rbuf->rbufpool, c);
    }
#endif
    // 确保 rbuf 中的 n_live_rmsg_chunks 大于 0 (Assert that the n_live_rmsg_chunks in rbuf is
    // greater than 0)
    assert(ddsrt_atomic_ld32(&rbuf->n_live_rmsg_chunks) > 0);
    ddsi_rbuf_release(rbuf);
    c = c1;
  }
}

/**
 * @brief 提交 ddsi_rmsg_chunk 结构体，更新相关的 rbuf 的 freeptr (Commit a ddsi_rmsg_chunk
 * structure and update the related rbuf's freeptr)
 *
 * @param[in] chunk 要提交的 ddsi_rmsg_chunk 结构体指针 (Pointer to the ddsi_rmsg_chunk structure to
 * be committed)
 */
static void commit_rmsg_chunk(struct ddsi_rmsg_chunk *chunk) {
  struct ddsi_rbuf *rbuf = chunk->rbuf;
  RBUFTRACE("commit_rmsg_chunk(%p)\n", (void *)chunk);

  // 更新 rbuf 的 freeptr 以反映已提交的 ddsi_rmsg_chunk (Update the rbuf's freeptr to reflect the
  // committed ddsi_rmsg_chunk)
  rbuf->freeptr = (unsigned char *)(chunk + 1) + chunk->u.size;
}

/**
 * @brief 提交接收到的消息 (Commit a received message)
 *
 * @param[in] rmsg 指向要提交的消息的指针 (Pointer to the message to be committed)
 */
void ddsi_rmsg_commit(struct ddsi_rmsg *rmsg) {
  /* 注意：只有一个线程调用 rmsg_commit —— 创建它的线程。
   * (Note: only one thread calls rmsg_commit -- the one that created it in the first place.)
   *
   * 如果没有未处理的引用，我们可以简单地重用内存。例如，当消息无效、不包含异步处理的内容，
   * 或者调度恰好是在我们提交之前完成任何异步活动时发生。
   * (If there are no outstanding references, we can simply reuse the memory. This happens,
   * e.g., when the message is invalid, doesn't contain anything processed asynchronously,
   * or the scheduling happens to be such that any asynchronous activities have completed
   * before we got to commit.)
   */
  struct ddsi_rmsg_chunk *chunk = rmsg->lastchunk;
  RMSGTRACE("rmsg_commit(%p) refcount 0x%" PRIx32 " last-chunk-size %" PRIu32 "\n", (void *)rmsg,
            rmsg->refcount.v, chunk->u.size);
  ASSERT_RBUFPOOL_OWNER(chunk->rbuf->rbufpool);
  ASSERT_RMSG_UNCOMMITTED(rmsg);
  assert(chunk->u.size <= chunk->rbuf->max_rmsg_size);
  assert((chunk->u.size % DDSI_ALIGNOF_RMSG) == 0);
  assert(ddsrt_atomic_ld32(&rmsg->refcount) >= RMSG_REFCOUNT_UNCOMMITTED_BIAS);
  assert(ddsrt_atomic_ld32(&rmsg->chunk.rbuf->n_live_rmsg_chunks) > 0);
  assert(ddsrt_atomic_ld32(&chunk->rbuf->n_live_rmsg_chunks) > 0);
  assert(chunk->rbuf->rbufpool->current == chunk->rbuf);
  if (ddsrt_atomic_sub32_nv(&rmsg->refcount, RMSG_REFCOUNT_UNCOMMITTED_BIAS) == 0)
    ddsi_rmsg_free(rmsg);
  else {
    /* 其他引用存在，因此可能存储在 defrag、reorder 和/或 delivery queue 中
     * (Other references exist, so either stored in defrag, reorder and/or delivery queue)
     */
    RMSGTRACE("rmsg_commit(%p) => keep\n", (void *)rmsg);
    commit_rmsg_chunk(chunk);
  }
}

/**
 * @brief 增加接收到的消息的偏置 (Add bias to a received message)
 *
 * @param[in] rmsg 指向要增加偏置的消息的指针 (Pointer to the message to add bias)
 */
static void ddsi_rmsg_addbias(struct ddsi_rmsg *rmsg) {
  /* 注意：只有拥有接收池的接收线程可以增加引用计数，而且只有在仍未提交时才能这样做。
   * (Note: only the receive thread that owns the receive pool may increase the reference count,
   * and only while it is still uncommitted.)
   *
   * 然而，其他线程（例如，传递线程）可能已经被触发，因此增量必须原子地完成。
   * (However, other threads (e.g., delivery threads) may have been triggered already,
   * so the increment must be done atomically.)
   */
  RMSGTRACE("rmsg_addbias(%p)\n", (void *)rmsg);
  ASSERT_RBUFPOOL_OWNER(rmsg->chunk.rbuf->rbufpool);
  ASSERT_RMSG_UNCOMMITTED(rmsg);
  ddsrt_atomic_add32(&rmsg->refcount, RMSG_REFCOUNT_RDATA_BIAS);
}

/**
 * @brief 减少 rmsg 的引用计数，并根据 adjust 调整。
 *        Reduce the reference count of rmsg and adjust it according to adjust.
 *
 * @param[in] rmsg   指向 ddsi_rmsg 结构的指针。Pointer to a ddsi_rmsg structure.
 * @param[in] adjust 要调整的值。The value to adjust.
 */
static void ddsi_rmsg_rmbias_and_adjust(struct ddsi_rmsg *rmsg, int adjust) {
  // 这可能发生在任何一个仍在管道中传输的样本所引用的 rmsg
  // 上，但只能由接收线程处理。不能要求它未提交。 This can happen to any rmsg referenced by a sample
  // still progressing through the pipeline, but only by a receive thread. Can't require it to be
  // uncommitted.
  uint32_t sub;
  RMSGTRACE("rmsg_rmbias_and_adjust(%p, %d)\n", (void *)rmsg, adjust);

  // 确保 adjust 大于等于0。Ensure that adjust is greater than or equal to 0.
  assert(adjust >= 0);
  // 确保 adjust 小于 RMSG_REFCOUNT_RDATA_BIAS。Ensure that adjust is less than
  // RMSG_REFCOUNT_RDATA_BIAS.
  assert((uint32_t)adjust < RMSG_REFCOUNT_RDATA_BIAS);
  sub = RMSG_REFCOUNT_RDATA_BIAS - (uint32_t)adjust;
  // 确保 rmsg 的 refcount 大于等于 sub。Ensure that the refcount of rmsg is greater than or equal
  // to sub.
  assert(ddsrt_atomic_ld32(&rmsg->refcount) >= sub);
  // 如果 rmsg 的 refcount 减去 sub 后等于0，则释放 rmsg。If the refcount of rmsg minus sub is equal
  // to 0, release rmsg.
  if (ddsrt_atomic_sub32_nv(&rmsg->refcount, sub) == 0) ddsi_rmsg_free(rmsg);
}

/**
 * @brief 减少 rmsg 的引用计数。
 *        Decrease the reference count of rmsg.
 *
 * @param[in] rmsg 指向 ddsi_rmsg 结构的指针。Pointer to a ddsi_rmsg structure.
 */
static void ddsi_rmsg_unref(struct ddsi_rmsg *rmsg) {
  RMSGTRACE("rmsg_unref(%p)\n", (void *)rmsg);
  // 确保 rmsg 的 refcount 大于0。Ensure that the refcount of rmsg is greater than 0.
  assert(ddsrt_atomic_ld32(&rmsg->refcount) > 0);
  // 如果 rmsg 的 refcount 减1后等于1，则释放 rmsg。If the refcount of rmsg minus 1 is equal to 1,
  // release rmsg.
  if (ddsrt_atomic_dec32_ov(&rmsg->refcount) == 1) ddsi_rmsg_free(rmsg);
}

/**
 * @brief 为 rmsg 分配内存。
 *        Allocate memory for rmsg.
 *
 * @param[in] rmsg 指向 ddsi_rmsg 结构的指针。Pointer to a ddsi_rmsg structure.
 * @param[in] size 要分配的内存大小。The size of the memory to allocate.
 * @return 返回分配的内存指针。Return the allocated memory pointer.
 */
void *ddsi_rmsg_alloc(struct ddsi_rmsg *rmsg, uint32_t size) {
  struct ddsi_rmsg_chunk *chunk = rmsg->lastchunk;
  struct ddsi_rbuf *rbuf = chunk->rbuf;
  uint32_t size8P = align_rmsg(size);
  void *ptr;
  RMSGTRACE("rmsg_alloc(%p, %" PRIu32 " => %" PRIu32 ")\n", (void *)rmsg, size, size8P);
  // 确保 rbufpool 的所有者。Ensure the owner of rbufpool.
  ASSERT_RBUFPOOL_OWNER(rbuf->rbufpool);
  // 确保 rmsg 未提交。Ensure that rmsg is uncommitted.
  ASSERT_RMSG_UNCOMMITTED(rmsg);
  // 确保 chunk 的大小是 DDSI_ALIGNOF_RMSG 的倍数。Ensure that the size of chunk is a multiple of
  // DDSI_ALIGNOF_RMSG.
  assert((chunk->u.size % DDSI_ALIGNOF_RMSG) == 0);
  // 确保 size8P 小于等于 rbuf 的 max_rmsg_size。Ensure that size8P is less than or equal to rbuf's
  // max_rmsg_size.
  assert(size8P <= rbuf->max_rmsg_size);

  // 如果 chunk 的大小加上 size8P 大于 rbuf 的 max_rmsg_size，则需要新的 chunk。
  // If the size of chunk plus size8P is greater than rbuf's max_rmsg_size, a new chunk is needed.
  if (chunk->u.size + size8P > rbuf->max_rmsg_size) {
    struct ddsi_rbufpool *rbp = rbuf->rbufpool;
    struct ddsi_rmsg_chunk *newchunk;
    RMSGTRACE("rmsg_alloc(%p, %" PRIu32 ") limit hit - new chunk\n", (void *)rmsg, size);
    commit_rmsg_chunk(chunk);
    newchunk = ddsi_rbuf_alloc(rbp);
    if (newchunk == NULL) {
      DDS_CWARNING(
          rbp->logcfg,
          "ddsi_rmsg_alloc: can't allocate more memory (%" PRIu32 " bytes) ... giving up\n", size);
      return NULL;
    }
    init_rmsg_chunk(newchunk, rbp->current);
    rmsg->lastchunk = chunk->next = newchunk;
    chunk = newchunk;
  }

  // 计算分配的内存指针。Calculate the allocated memory pointer.
  ptr = (unsigned char *)(chunk + 1) + chunk->u.size;
  chunk->u.size += size8P;
  RMSGTRACE("rmsg_alloc(%p, %" PRIu32 ") = %p\n", (void *)rmsg, size, ptr);
#if USE_VALGRIND
  if (chunk == &rmsg->chunk) {
    VALGRIND_MEMPOOL_CHANGE(rbuf->rbufpool, rmsg, rmsg,
                            offsetof(struct ddsi_rmsg, chunk.u.payload) + chunk->size);
  } else {
    VALGRIND_MEMPOOL_CHANGE(rbuf->rbufpool, chunk, chunk,
                            offsetof(struct ddsi_rmsg_chunk, u.payload) + chunk->size);
  }
#endif
  return ptr;
}

/* RDATA --------------------------------------- */

/**
 * @brief 创建一个新的ddsi_rdata结构体实例 (Create a new ddsi_rdata structure instance)
 *
 * @param[in] rmsg 指向ddsi_rmsg结构体的指针 (Pointer to the ddsi_rmsg structure)
 * @param[in] start 数据开始位置 (Data start position)
 * @param[in] endp1 数据结束位置+1 (Data end position + 1)
 * @param[in] submsg_offset 子消息偏移量 (Submessage offset)
 * @param[in] payload_offset 载荷偏移量 (Payload offset)
 * @param[in] keyhash_offset 键哈希偏移量 (Key hash offset)
 * @return 返回创建的ddsi_rdata结构体指针，如果分配失败则返回NULL (Returns a pointer to the created
 * ddsi_rdata structure, or NULL if allocation fails)
 */
struct ddsi_rdata *ddsi_rdata_new(struct ddsi_rmsg *rmsg,
                                  uint32_t start,
                                  uint32_t endp1,
                                  uint32_t submsg_offset,
                                  uint32_t payload_offset,
                                  uint32_t keyhash_offset) {
  struct ddsi_rdata *d;
  // 尝试为ddsi_rdata结构体分配内存，如果失败则返回NULL (Attempt to allocate memory for the
  // ddsi_rdata structure; return NULL if it fails)
  if ((d = ddsi_rmsg_alloc(rmsg, sizeof(*d))) == NULL) return NULL;
  // 初始化ddsi_rdata结构体的成员变量 (Initialize the member variables of the ddsi_rdata structure)
  d->rmsg = rmsg;
  d->nextfrag = NULL;
  d->min = start;
  d->maxp1 = endp1;
  d->submsg_zoff = (uint16_t)DDSI_OFF_TO_ZOFF(submsg_offset);
  d->payload_zoff = (uint16_t)DDSI_OFF_TO_ZOFF(payload_offset);
  d->keyhash_zoff = (uint16_t)DDSI_OFF_TO_ZOFF(keyhash_offset);
#ifndef NDEBUG
  ddsrt_atomic_st32(&d->refcount_bias_added, 0);
#endif
  // 记录调试信息 (Record debug information)
  RMSGTRACE("rdata_new(%p, bytes [%" PRIu32 ",%" PRIu32 "), submsg @ %u, payload @ %u) = %p\n",
            (void *)rmsg, start, endp1, DDSI_RDATA_SUBMSG_OFF(d), DDSI_RDATA_PAYLOAD_OFF(d),
            (void *)d);
  // 返回创建的ddsi_rdata结构体指针 (Return the created ddsi_rdata structure pointer)
  return d;
}

/**
 * @brief 为ddsi_rdata结构体添加偏置 (Add bias to the ddsi_rdata structure)
 *
 * @param[in] rdata 指向ddsi_rdata结构体的指针 (Pointer to the ddsi_rdata structure)
 */
static void ddsi_rdata_addbias(struct ddsi_rdata *rdata) {
  struct ddsi_rmsg *rmsg = rdata->rmsg;
  // 记录调试信息 (Record debug information)
  RMSGTRACE("rdata_addbias(%p)\n", (void *)rdata);
#ifndef NDEBUG
  ASSERT_RBUFPOOL_OWNER(rmsg->chunk.rbuf->rbufpool);
  if (ddsrt_atomic_inc32_nv(&rdata->refcount_bias_added) != 1) abort();
#endif
  // 为ddsi_rmsg结构体添加偏置 (Add bias to the ddsi_rmsg structure)
  ddsi_rmsg_addbias(rmsg);
}

/**
 * @brief 移除ddsi_rdata结构体的偏置并调整引用计数 (Remove bias from the ddsi_rdata structure and
 * adjust reference count)
 *
 * @param[in] rdata 指向ddsi_rdata结构体的指针 (Pointer to the ddsi_rdata structure)
 * @param[in] adjust 要调整的引用计数值 (Reference count value to adjust)
 */
static void ddsi_rdata_rmbias_and_adjust(struct ddsi_rdata *rdata, int adjust) {
  struct ddsi_rmsg *rmsg = rdata->rmsg;
  // 记录调试信息 (Record debug information)
  RMSGTRACE("rdata_rmbias_and_adjust(%p, %d)\n", (void *)rdata, adjust);
#ifndef NDEBUG
  if (ddsrt_atomic_dec32_ov(&rdata->refcount_bias_added) != 1) abort();
#endif
  // 移除ddsi_rmsg结构体的偏置并调整引用计数 (Remove bias from the ddsi_rmsg structure and adjust
  // reference count)
  ddsi_rmsg_rmbias_and_adjust(rmsg, adjust);
}

/**
 * @brief 取消对ddsi_rdata结构体的引用 (Unreference the ddsi_rdata structure)
 *
 * @param[in] rdata 指向ddsi_rdata结构体的指针 (Pointer to the ddsi_rdata structure)
 */
static void ddsi_rdata_unref(struct ddsi_rdata *rdata) {
  struct ddsi_rmsg *rmsg = rdata->rmsg;
  // 记录调试信息 (Record debug information)
  RMSGTRACE("rdata_rdata_unref(%p)\n", (void *)rdata);
  // 取消对ddsi_rmsg结构体的引用 (Unreference the ddsi_rmsg structure)
  ddsi_rmsg_unref(rmsg);
}

/* DEFRAG --------------------------------------------------------------

   Defragmentation happens separately from reordering, the reason
   being that defragmentation really is best done only once, and
   besides it simplifies reordering because it only ever has to deal
   with whole messages.

   The defragmeter accepts both rdatas that are fragments of samples
   and rdatas that are complete samples.  The unfragmented ones are
   returned immediately for further processing, in the format also
   used for fragmented samples.  Any rdata stored in the defrag index
   as well as unfragmented ones returned immediately are accounted for
   in rmsg::refcount.

   Defragmenting one sample is done using an interval tree where the
   minima and maxima are given by byte indexes of the received
   framgents.  Consecutive frags get chained in one interval, to keep
   the tree small even in the worst case.

   These intervals are represented using defrag_iv, and the fragment
   chain for an interval is built using the nextfrag links in the
   rdata.

   The defragmenter can defragment multiple samples in parallel (even
   though a writer normally produces a single fragment chain only,
   things may be different when packets get lost and/or
   (transient-local) data is resent).

   Each sample is represented using an rsample.  Each contains the
   root of an interval tree of fragments with a cached pointer to the
   last known interval (because we expect the data to arrive in-order
   and like to avoid searching).  The rsamples are stored in a tree
   indexed on sequence number, which itself caches the last sample it
   is currently defragmenting, again to avoid searching.

   The memory for an rsample is later re-used by the reordering
   mechanism.  Hence the union.  For that use, see REORDER.

   Partial and complete overlap of fragments is acceptable, but may
   result in a fragment chain containing fragments that do not add any
   bytes of information.  Those should be skipped by the deserializer.
   If the sender decides to suddenly change the fragmentation for a
   message, we happily keep processing them, even though there is no
   good reason for the sender to do so and the likelihood of such
   messy fragment chains increases significantly.

   Once done defragmenting, the tree consists of a root node only,
   which points to a list of fragments, in-order (but for the caveat
   above).

   Memory used for the storage of interval nodes while defragmenting
   is afterward re-used for chaining samples.  An unfragmented message
   will have a new sample chain allocated for this purpose, a
   fragmented message will have at least one interval allocated to it
   and thus have sufficient space for the chain node.

   FIXME: These AVL trees are overkill.  Either switch to parent-less
   red-black trees (they have better performance anyway and only need
   a single bit of state) or to splay trees (must have a parent
   because they can degenerate to linear structures, unless the number
   of intervals in the tree is limited, which probably is a good idea
   anyway). */
/* DEFRAG --------------------------------------------------------------

   碎片整理与重新排序分开进行，原因在于碎片整理最好只做一次，而且它简化了重新排序，因为只需要处理整个消息。

   defragmeter接受既是样本碎片又是完整样本的rdatas。未分片的数据会立即返回以供进一步处理，采用与分片样本相同的格式。存储在defrag索引中的任何rdata以及立即返回的未分片数据都会计入rmsg::refcount。

   对一个样本进行碎片整理是使用一个区间树，其中最小值和最大值由接收到的framgents的字节索引给出。连续的frags在一个区间内链接，以保持树在最坏情况下仍然很小。

   这些区间由defrag_iv表示，区间的碎片链是通过rdata中的nextfrag链接构建的。

   碎片整理器可以同时对多个样本进行碎片整理（尽管写入程序通常只生成一个碎片链，但当数据包丢失和/或（瞬态-本地）数据被重发时，情况可能会有所不同）。

   每个样本都用rsample表示。每个样本都包含一个带有缓存指针的区间树的根，该指针指向已知的最后一个区间（因为我们希望数据按顺序到达并避免搜索）。这些rsamples存储在一个以序列号为索引的树中，该树本身缓存了它当前正在进行碎片整理的最后一个样本，以避免搜索。

   rsample的内存稍后会被重新排序机制重用。因此union。有关该用途，请参见REORDER。

   碎片的部分和完全重叠是可以接受的，但可能导致碎片链包含不增加任何字节信息的碎片。反序列化器应跳过这些碎片。如果发送方突然决定更改消息的分片方式，我们会继续处理它们，尽管发送方没有理由这样做，而且这种混乱的碎片链的可能性大大增加。

   完成碎片整理后，树只包含一个根节点，该节点指向一个按顺序排列的碎片列表（但上述注意事项除外）。

   在碎片整理过程中用于存储区间节点的内存之后将被重用以链接样本。未分片的消息将为此目的分配一个新的样本链，而分片的消息将至少分配一个区间，并为链节点提供足够的空间。

   FIXME：这些AVL树过于复杂。要么切换到无父红黑树（它们的性能更好，只需要一个状态位），要么切换到伸展树（必须有父节点，因为它们可能退化为线性结构，除非树中区间的数量受到限制，这可能是个好主意）。*/

/* ddsi_defrag_iv 结构体定义 */
struct ddsi_defrag_iv {
  ddsrt_avl_node_t avlnode; /* 用于 ddsi_rsample.defrag::fragtree 的 AVL 节点 */
                            /* AVL node for ddsi_rsample.defrag::fragtree */
  uint32_t min, maxp1;      /* 分片的最小值和最大值加一 */
                            /* Minimum and maximum value plus one of the fragment */
  struct ddsi_rdata *first; /* 指向第一个分片的指针 */
                            /* Pointer to the first fragment */
  struct ddsi_rdata *last;  /* 指向最后一个分片的指针 */
                            /* Pointer to the last fragment */
};

/* ddsi_rsample 结构体定义 */
struct ddsi_rsample {
  union {
    struct ddsi_rsample_defrag {
      ddsrt_avl_node_t avlnode;             /* 用于 ddsi_defrag::sampletree 的 AVL 节点 */
                                            /* AVL node for ddsi_defrag::sampletree */
      ddsrt_avl_tree_t fragtree;            /* 分片树 */
                                            /* Fragment tree */
      struct ddsi_defrag_iv *lastfrag;      /* 指向最后一个分片的指针 */
                                            /* Pointer to the last fragment */
      struct ddsi_rsample_info *sampleinfo; /* 样本信息 */
                                            /* Sample information */
      ddsi_seqno_t seq;                     /* 序列号 */
                                            /* Sequence number */
    } defrag;
    struct ddsi_rsample_reorder {
      ddsrt_avl_node_t avlnode; /* 用于 ddsi_reorder::sampleivtree 的 AVL 节点，如果是链的头部 */
                                /* AVL node for ddsi_reorder::sampleivtree, if head of a chain */
      struct ddsi_rsample_chain sc; /* 这个区间的样本，覆盖... */
                                    /* This interval's samples, covering ... */
      ddsi_seqno_t min, maxp1;      /* 序列号范围：[min,maxp1)，但可能存在空洞 */
      /* Sequence numbers range: [min,maxp1), but possibly with holes in it */
      uint32_t n_samples; /* 链的实际长度 */
                          /* The actual length of the chain */
    } reorder;
  } u;
};

/* ddsi_defrag 结构体定义 */
struct ddsi_defrag {
  ddsrt_avl_tree_t sampletree;          /* 样本树 */
                                        /* Sample tree */
  struct ddsi_rsample *max_sample;      /* 最大样本（= max(sampletree)） */
                                        /* Maximum sample (= max(sampletree)) */
  uint32_t n_samples;                   /* 样本数量 */
                                        /* Number of samples */
  uint32_t max_samples;                 /* 最大样本数量 */
                                        /* Maximum number of samples */
  enum ddsi_defrag_drop_mode drop_mode; /* 丢弃模式 */
                                        /* Drop mode */
  uint64_t discarded_bytes;             /* 丢弃的字节数 */
                                        /* Discarded bytes */
  const struct ddsrt_log_cfg *logcfg;   /* 日志配置 */
                                        /* Log configuration */
  bool trace;                           /* 跟踪标志 */
                                        /* Trace flag */
};

/* 比较两个 uint32_t 类型值的函数 */
/* Function to compare two uint32_t values */
static int compare_uint32(const void *va, const void *vb);

/* 比较两个序列号的函数 */
/* Function to compare two sequence numbers */
static int compare_seqno(const void *va, const void *vb);

/* defrag_sampletree_treedef 的定义 */
/* Definition of defrag_sampletree_treedef */
static const ddsrt_avl_treedef_t defrag_sampletree_treedef =
    DDSRT_AVL_TREEDEF_INITIALIZER(offsetof(struct ddsi_rsample, u.defrag.avlnode),
                                  offsetof(struct ddsi_rsample, u.defrag.seq),
                                  compare_seqno,
                                  0);
/* rsample_defrag_fragtree_treedef 的定义 */
/* Definition of rsample_defrag_fragtree_treedef */
static const ddsrt_avl_treedef_t rsample_defrag_fragtree_treedef =
    DDSRT_AVL_TREEDEF_INITIALIZER(offsetof(struct ddsi_defrag_iv, avlnode),
                                  offsetof(struct ddsi_defrag_iv, min),
                                  compare_uint32,
                                  0);

/**
 * @brief 比较两个无符号32位整数的大小 (Compare two uint32_t values)
 *
 * @param[in] va 第一个无符号32位整数的指针 (Pointer to the first uint32_t value)
 * @param[in] vb 第二个无符号32位整数的指针 (Pointer to the second uint32_t value)
 * @return 返回0表示相等，返回-1表示a小于b，返回1表示a大于b (Return 0 if equal, -1 if a < b, and 1
 * if a > b)
 */
static int compare_uint32(const void *va, const void *vb) {
  uint32_t a =
      *((const uint32_t *)va);  // 将void指针转换为uint32_t指针并解引用获取值 (Convert void pointer
                                // to uint32_t pointer and dereference to get the value)
  uint32_t b =
      *((const uint32_t *)vb);  // 将void指针转换为uint32_t指针并解引用获取值 (Convert void pointer
                                // to uint32_t pointer and dereference to get the value)
  return (a == b) ? 0
         : (a < b)
             ? -1
             : 1;  // 比较a和b的大小并返回结果 (Compare the values of a and b and return the result)
}

/**
 * @brief 比较两个序列号的大小 (Compare two sequence numbers)
 *
 * @param[in] va 第一个序列号的指针 (Pointer to the first sequence number)
 * @param[in] vb 第二个序列号的指针 (Pointer to the second sequence number)
 * @return 返回0表示相等，返回-1表示a小于b，返回1表示a大于b (Return 0 if equal, -1 if a < b, and 1
 * if a > b)
 */
static int compare_seqno(const void *va, const void *vb) {
  ddsi_seqno_t a = *((const ddsi_seqno_t *)
                         va);  // 将void指针转换为ddsi_seqno_t指针并解引用获取值 (Convert void
                               // pointer to ddsi_seqno_t pointer and dereference to get the value)
  ddsi_seqno_t b = *((const ddsi_seqno_t *)
                         vb);  // 将void指针转换为ddsi_seqno_t指针并解引用获取值 (Convert void
                               // pointer to ddsi_seqno_t pointer and dereference to get the value)
  return (a == b) ? 0
         : (a < b)
             ? -1
             : 1;  // 比较a和b的大小并返回结果 (Compare the values of a and b and return the result)
}

/**
 * @brief 创建一个新的ddsi_defrag结构体实例 (Create a new ddsi_defrag instance)
 *
 * @param[in] logcfg 日志配置 (Log configuration)
 * @param[in] drop_mode 数据丢弃模式 (Data drop mode)
 * @param[in] max_samples 最大样本数 (Maximum number of samples)
 * @return 返回创建的ddsi_defrag实例指针，如果分配失败则返回NULL (Return pointer to the created
 * ddsi_defrag instance, or NULL if allocation fails)
 */
struct ddsi_defrag *ddsi_defrag_new(const struct ddsrt_log_cfg *logcfg,
                                    enum ddsi_defrag_drop_mode drop_mode,
                                    uint32_t max_samples) {
  struct ddsi_defrag *d;
  assert(max_samples >=
         1);  // 断言最大样本数至少为1 (Assert that the maximum number of samples is at least 1)
  if ((d = ddsrt_malloc(sizeof(*d))) == NULL)
    return NULL;  // 分配内存并检查是否成功 (Allocate memory and check if successful)
  ddsrt_avl_init(&defrag_sampletree_treedef,
                 &d->sampletree);  // 初始化AVL树 (Initialize the AVL tree)
  d->drop_mode = drop_mode;        // 设置数据丢弃模式 (Set the data drop mode)
  d->max_samples = max_samples;    // 设置最大样本数 (Set the maximum number of samples)
  d->n_samples = 0;                // 初始化样本计数器 (Initialize the sample counter)
  d->max_sample = NULL;    // 初始化最大样本指针 (Initialize the maximum sample pointer)
  d->discarded_bytes = 0;  // 初始化丢弃字节计数器 (Initialize the discarded bytes counter)
  d->logcfg = logcfg;      // 设置日志配置 (Set the log configuration)
  d->trace = (logcfg->c.mask & DDS_LC_RADMIN) != 0;  // 设置跟踪标志 (Set the trace flag)
  return d;  // 返回创建的ddsi_defrag实例 (Return the created ddsi_defrag instance)
}

/**
 * @brief 获取ddsi_defrag实例的统计信息 (Get statistics for a ddsi_defrag instance)
 *
 * @param[in] defrag ddsi_defrag实例 (ddsi_defrag instance)
 * @param[out] discarded_bytes 丢弃的字节数 (Number of discarded bytes)
 */
void ddsi_defrag_stats(struct ddsi_defrag *defrag, uint64_t *discarded_bytes) {
  *discarded_bytes = defrag->discarded_bytes;  // 获取丢弃字节计数器的值 (Get the value of the
                                               // discarded bytes counter)
}

/**
 * @brief 调整片段链的引用计数 (Adjust the reference count of a fragment chain)
 *
 * @param[in] frag 片段链的第一个片段 (First fragment in the fragment chain)
 * @param[in] adjust 要调整的引用计数值 (Reference count adjustment value)
 */
void ddsi_fragchain_adjust_refcount(struct ddsi_rdata *frag, int adjust) {
  RDATATRACE(frag, "fragchain_adjust_refcount(%p, %d)\n", (void *)frag,
             adjust);  // 记录跟踪信息 (Record trace information)
  while (frag) {       // 遍历片段链 (Iterate through the fragment chain)
    struct ddsi_rdata *const frag1 = frag->nextfrag;  // 获取下一个片段 (Get the next fragment)
    ddsi_rdata_rmbias_and_adjust(
        frag,
        adjust);  // 调整当前片段的引用计数 (Adjust the reference count of the current fragment)
    frag = frag1;  // 移动到下一个片段 (Move to the next fragment)
  }
}

/**
 * @brief 移除片段链的偏置 (Remove bias from a fragment chain)
 *
 * @param[in] frag 片段链的第一个片段 (First fragment in the fragment chain)
 */
static void ddsi_fragchain_rmbias(struct ddsi_rdata *frag) {
  ddsi_fragchain_adjust_refcount(
      frag, 0);  // 调整片段链的引用计数，不改变实际值 (Adjust the reference count of the fragment
                 // chain without changing the actual value)
}

/**
 * @brief 释放指定的 defrag 和 rsample 中的资源 (Release resources in the specified defrag and
 * rsample)
 *
 * @param[in] defrag   指向 ddsi_defrag 结构的指针 (Pointer to a ddsi_defrag structure)
 * @param[in] rsample  指向 ddsi_rsample 结构的指针 (Pointer to a ddsi_rsample structure)
 */
static void defrag_rsample_drop(struct ddsi_defrag *defrag, struct ddsi_rsample *rsample) {
  // 在第一个 fragchain_free 之后不能引用 rsample，因为我们不知道哪个 rdata/rmsg 为 rsample
  // 提供存储， 因此无法增加引用计数。(Can't reference rsample after the first fragchain_free,
  // because we don't know which rdata/rmsg provides the storage for the rsample and therefore can't
  // increment the reference count.)
  //
  // 所以我们需要遍历片段，同时保证内存访问中的严格“前进”，这个特定的 inorder treewalk
  // 确实提供了。(So we need to walk the fragments while guaranteeing strict "forward progress" in
  // the memory accesses, which this particular inorder treewalk does provide.)

  ddsrt_avl_iter_t iter;
  struct ddsi_defrag_iv *iv;

  // 记录 defrag 和 rsample 的操作 (Trace the operation of defrag and rsample)
  TRACE(defrag, "  defrag_rsample_drop (%p, %p)\n", (void *)defrag, (void *)rsample);

  // 从 defrag 的 sampletree 中删除 rsample (Delete rsample from defrag's sampletree)
  ddsrt_avl_delete(&defrag_sampletree_treedef, &defrag->sampletree, rsample);

  // 确保 defrag 的 n_samples 大于 0 (Ensure defrag's n_samples is greater than 0)
  assert(defrag->n_samples > 0);

  // 减少 defrag 的 n_samples 计数 (Decrease the count of defrag's n_samples)
  defrag->n_samples--;

  // 遍历 rsample 的片段树 (Iterate through the fragment tree of rsample)
  for (iv = ddsrt_avl_iter_first(&rsample_defrag_fragtree_treedef, &rsample->u.defrag.fragtree,
                                 &iter);
       iv; iv = ddsrt_avl_iter_next(&iter)) {
    if (iv->first) {
      // 如果第一个片段丢失，将插入一个带有空链的哨兵 "iv" (If the first fragment is missing, a
      // sentinel "iv" is inserted with an empty chain)
      ddsi_fragchain_rmbias(iv->first);
    }
  }
}

/**
 * @brief 释放指定的 defrag 中的资源 (Release resources in the specified defrag)
 *
 * @param[in] defrag   指向 ddsi_defrag 结构的指针 (Pointer to a ddsi_defrag structure)
 */
void ddsi_defrag_free(struct ddsi_defrag *defrag) {
  struct ddsi_rsample *s;

  // 查找 defrag 的 sampletree 中的最小元素 (Find the minimum element in defrag's sampletree)
  s = ddsrt_avl_find_min(&defrag_sampletree_treedef, &defrag->sampletree);

  // 遍历 defrag 的 sampletree (Iterate through the sampletree of defrag)
  while (s) {
    TRACE(defrag, "defrag_free(%p, sample %p seq %" PRIu64 ")\n", (void *)defrag, (void *)s,
          s->u.defrag.seq);

    // 释放 defrag 和 s 中的资源 (Release resources in defrag and s)
    defrag_rsample_drop(defrag, s);

    // 再次查找 defrag 的 sampletree 中的最小元素 (Find the minimum element in defrag's sampletree
    // again)
    s = ddsrt_avl_find_min(&defrag_sampletree_treedef, &defrag->sampletree);
  }

  // 确保 defrag 的 n_samples 为 0 (Ensure defrag's n_samples is 0)
  assert(defrag->n_samples == 0);

  // 释放 defrag 结构的内存 (Free the memory of the defrag structure)
  ddsrt_free(defrag);
}

/**
 * @brief 尝试合并当前节点与后继节点的片段 (Try to merge the current node with its successor
 * fragment)
 *
 * @param[in] defrag 指向ddsi_defrag结构体的指针 (Pointer to the ddsi_defrag structure)
 * @param[in] sample 指向ddsi_rsample_defrag结构体的指针 (Pointer to the ddsi_rsample_defrag
 * structure)
 * @param[in] node 当前要处理的节点 (The current node to be processed)
 * @return int 返回0表示未合并，返回1表示已合并 (Return 0 if not merged, return 1 if merged)
 */
static int defrag_try_merge_with_succ(const struct ddsi_defrag *defrag,
                                      struct ddsi_rsample_defrag *sample,
                                      struct ddsi_defrag_iv *node) {
  struct ddsi_defrag_iv *succ;

  // 打印调试信息 (Print debug information)
  TRACE(defrag, "  defrag_try_merge_with_succ(%p [%" PRIu32 "..%" PRIu32 ")):\n", (void *)node,
        node->min, node->maxp1);
  if (node == sample->lastfrag) {
    // 如果当前节点是最后一个片段，则没有后继节点 (If the current node is the last fragment, there
    // is no successor node)
    TRACE(defrag, "  node is lastfrag\n");
    return 0;
  }

  // 查找后继节点 (Find the successor node)
  succ = ddsrt_avl_find_succ(&rsample_defrag_fragtree_treedef, &sample->fragtree, node);
  assert(succ != NULL);
  TRACE(defrag, "  succ is %p [%" PRIu32 "..%" PRIu32 ")\n", (void *)succ, succ->min, succ->maxp1);
  if (succ->min > node->maxp1) {
    // 如果后继节点与当前节点之间存在间隙，则不合并 (If there is a gap between the successor node
    // and the current node, do not merge)
    TRACE(defrag, "  gap between node and succ\n");
    return 0;
  } else {
    uint32_t succ_maxp1 = succ->maxp1;

    // 从片段树中删除后继节点，因为它将被合并 (Remove the successor node from the fragment tree
    // because it will be merged)
    ddsrt_avl_delete(&rsample_defrag_fragtree_treedef, &sample->fragtree, succ);
    if (sample->lastfrag == succ) {
      TRACE(defrag, "  succ is lastfrag\n");
      sample->lastfrag = node;
    }

    // 如果后继节点的链包含比刚收到的片段更多的数据，将其追加到当前节点 (If the chain of the
    // successor node contains more data than the fragment just received, append it to the current
    // node)
    if (node->maxp1 < succ_maxp1)
      TRACE(defrag, "  succ adds data to node\n");
    else
      TRACE(defrag, "  succ is contained in node\n");

    node->last->nextfrag = succ->first;
    node->last = succ->last;
    node->maxp1 = succ_maxp1;

    // 如果新片段包含超出后继节点的数据，甚至可能允许与后继节点的后继节点合并 (If the new fragment
    // contains data beyond the successor node, it may even allow merging with the successor of the
    // successor node)
    return node->maxp1 > succ_maxp1;
  }
}

/**
 * @brief 向 defrag_rsample 中添加一个新的片段 (Add a new fragment to the defrag_rsample)
 *
 * @param[in] sample 指向 ddsi_rsample_defrag 结构的指针 (Pointer to the ddsi_rsample_defrag
 * structure)
 * @param[in] rdata  指向 ddsi_rdata 结构的指针 (Pointer to the ddsi_rdata structure)
 * @param[in] path   指向 ddsrt_avl_ipath_t 结构的指针 (Pointer to the ddsrt_avl_ipath_t structure)
 */
static void defrag_rsample_addiv(struct ddsi_rsample_defrag *sample,
                                 struct ddsi_rdata *rdata,
                                 ddsrt_avl_ipath_t *path) {
  struct ddsi_defrag_iv *newiv;
  // 为 newiv 分配内存 (Allocate memory for newiv)
  if ((newiv = ddsi_rmsg_alloc(rdata->rmsg, sizeof(*newiv))) == NULL) return;
  // 将 rdata 的 nextfrag 设置为 NULL (Set rdata's nextfrag to NULL)
  rdata->nextfrag = NULL;
  // 初始化 newiv 的 first 和 last 为 rdata (Initialize newiv's first and last to rdata)
  newiv->first = newiv->last = rdata;
  // 设置 newiv 的 min 和 maxp1 (Set newiv's min and maxp1)
  newiv->min = rdata->min;
  newiv->maxp1 = rdata->maxp1;
  // 增加 rdata 的偏移量 (Increase rdata's bias)
  ddsi_rdata_addbias(rdata);
  // 将 newiv 插入到 sample 的 fragtree 中 (Insert newiv into sample's fragtree)
  ddsrt_avl_insert_ipath(&rsample_defrag_fragtree_treedef, &sample->fragtree, newiv, path);
  // 更新 sample 的 lastfrag (Update sample's lastfrag)
  if (sample->lastfrag == NULL || rdata->min > sample->lastfrag->min) sample->lastfrag = newiv;
}

/**
 * @brief 初始化 rsample 的公共部分 (Initialize the common part of rsample)
 *
 * @param[in] rsample    指向 ddsi_rsample 结构的指针 (Pointer to the ddsi_rsample structure)
 * @param[in] rdata      指向 ddsi_rdata 结构的指针 (Pointer to the ddsi_rdata structure)
 * @param[in] sampleinfo 指向 ddsi_rsample_info 结构的指针 (Pointer to the ddsi_rsample_info
 * structure)
 */
static void rsample_init_common(UNUSED_ARG(struct ddsi_rsample *rsample),
                                UNUSED_ARG(struct ddsi_rdata *rdata),
                                UNUSED_ARG(const struct ddsi_rsample_info *sampleinfo)) {}

/**
 * @brief 创建一个新的 defrag_rsample (Create a new defrag_rsample)
 *
 * @param[in] rdata      指向 ddsi_rdata 结构的指针 (Pointer to the ddsi_rdata structure)
 * @param[in] sampleinfo 指向 ddsi_rsample_info 结构的指针 (Pointer to the ddsi_rsample_info
 * structure)
 * @return               返回创建的 ddsi_rsample 结构的指针，如果创建失败则返回 NULL (Returns a
 * pointer to the created ddsi_rsample structure, or NULL if creation fails)
 */
static struct ddsi_rsample *defrag_rsample_new(struct ddsi_rdata *rdata,
                                               const struct ddsi_rsample_info *sampleinfo) {
  struct ddsi_rsample *rsample;
  struct ddsi_rsample_defrag *dfsample;
  ddsrt_avl_ipath_t ivpath;

  // 为 rsample 分配内存 (Allocate memory for rsample)
  if ((rsample = ddsi_rmsg_alloc(rdata->rmsg, sizeof(*rsample))) == NULL) return NULL;
  // 初始化 rsample 的公共部分 (Initialize the common part of rsample)
  rsample_init_common(rsample, rdata, sampleinfo);
  // 获取 defrag_rsample 的指针 (Get the pointer to defrag_rsample)
  dfsample = &rsample->u.defrag;
  // 初始化 dfsample 的 lastfrag 和 seq (Initialize dfsample's lastfrag and seq)
  dfsample->lastfrag = NULL;
  dfsample->seq = sampleinfo->seq;
  // 为 dfsample 的 sampleinfo 分配内存并初始化 (Allocate memory for dfsample's sampleinfo and
  // initialize it)
  if ((dfsample->sampleinfo = ddsi_rmsg_alloc(rdata->rmsg, sizeof(*dfsample->sampleinfo))) == NULL)
    return NULL;
  *dfsample->sampleinfo = *sampleinfo;

  // 初始化 dfsample 的 fragtree (Initialize dfsample's fragtree)
  ddsrt_avl_init(&rsample_defrag_fragtree_treedef, &dfsample->fragtree);

  // 如果 rdata 不是消息的第一个片段，则添加哨兵 (Add sentinel if rdata is not the first fragment of
  // the message)
  if (rdata->min > 0) {
    struct ddsi_defrag_iv *sentinel;
    // 为 sentinel 分配内存 (Allocate memory for sentinel)
    if ((sentinel = ddsi_rmsg_alloc(rdata->rmsg, sizeof(*sentinel))) == NULL) return NULL;
    // 初始化 sentinel 的 first、last、min 和 maxp1 (Initialize sentinel's first, last, min and
    // maxp1)
    sentinel->first = sentinel->last = NULL;
    sentinel->min = sentinel->maxp1 = 0;
    // 查找插入 sentinel 的路径 (Find the path to insert sentinel)
    ddsrt_avl_lookup_ipath(&rsample_defrag_fragtree_treedef, &dfsample->fragtree, &sentinel->min,
                           &ivpath);
    // 将 sentinel 插入到 dfsample 的 fragtree 中 (Insert sentinel into dfsample's fragtree)
    ddsrt_avl_insert_ipath(&rsample_defrag_fragtree_treedef, &dfsample->fragtree, sentinel,
                           &ivpath);
  }

  // 为第一个接收到的片段添加一个间隔 (Add an interval for the first received fragment)
  ddsrt_avl_lookup_ipath(&rsample_defrag_fragtree_treedef, &dfsample->fragtree, &rdata->min,
                         &ivpath);
  defrag_rsample_addiv(dfsample, rdata, &ivpath);
  return rsample;
}

/**
 * @brief 创建一个新的重排序接收样本 (Create a new reordering received sample)
 *
 * @param[in] rdata 接收到的数据 (Received data)
 * @param[in] sampleinfo 接收样本信息 (Received sample information)
 * @return 返回创建的重排序接收样本，如果分配失败则返回 NULL (Returns the created reordering
 * received sample, or NULL if allocation fails)
 */
static struct ddsi_rsample *reorder_rsample_new(struct ddsi_rdata *rdata,
                                                const struct ddsi_rsample_info *sampleinfo) {
  // 实现 defrag_rsample_new 和 rsample_convert_defrag_to_reorder 的功能
  // (Implements defrag_rsample_new and rsample_convert_defrag_to_reorder functionality)

  // 这个函数足够简单，不需要额外的函数。注意 defrag_rsample_new 和这个函数之间的差异，
  // defrag_rsample_new 完全初始化了 rsample，包括 AVL 节点头，而这个函数没有这么做。
  // (This function is simple enough to warrant having an extra function. Note the discrepancy
  // between defrag_rsample_new which fully initializes the rsample, including the AVL node headers,
  // and this function, which doesn't do so.)

  struct ddsi_rsample *rsample;
  struct ddsi_rsample_reorder *s;
  struct ddsi_rsample_chain_elem *sce;

  // 为 rsample 分配内存，如果分配失败则返回 NULL
  // (Allocate memory for rsample, return NULL if allocation fails)
  if ((rsample = ddsi_rmsg_alloc(rdata->rmsg, sizeof(*rsample))) == NULL) return NULL;
  rsample_init_common(rsample, rdata, sampleinfo);

  // 为 sce 分配内存，如果分配失败则返回 NULL
  // (Allocate memory for sce, return NULL if allocation fails)
  if ((sce = ddsi_rmsg_alloc(rdata->rmsg, sizeof(*sce))) == NULL) return NULL;
  sce->fragchain = rdata;
  sce->next = NULL;
  // 为 sce->sampleinfo 分配内存，如果分配失败则返回 NULL
  // (Allocate memory for sce->sampleinfo, return NULL if allocation fails)
  if ((sce->sampleinfo = ddsi_rmsg_alloc(rdata->rmsg, sizeof(*sce->sampleinfo))) == NULL)
    return NULL;
  *sce->sampleinfo = *sampleinfo;
  rdata->nextfrag = NULL;
  ddsi_rdata_addbias(rdata);

  s = &rsample->u.reorder;
  s->min = sampleinfo->seq;
  s->maxp1 = sampleinfo->seq + 1;
  s->n_samples = 1;
  s->sc.first = s->sc.last = sce;
  return rsample;
}

/**
 * @brief 检查接收样本是否完整 (Check if the received sample is complete)
 *
 * @param[in] sample 待检查的接收样本 (Received sample to be checked)
 * @return 如果样本不完整，则返回 0；否则返回 1 (Returns 0 if the sample is incomplete, otherwise
 * returns 1)
 */
static int is_complete(const struct ddsi_rsample_defrag *sample) {
  // 如果 'sample' 不完整，则返回 NULL，否则返回 'sample'。完整的定义：一个区间覆盖所有字节。
  // 由于 add_fragment() 中的贪婪合并，只有一个区间。如果我们到达这里，至少有一个区间。
  // (Returns: NULL if 'sample' is incomplete, else 'sample'. Complete: one interval covering all
  // bytes. One interval because of the greedy coalescing in add_fragment(). There is at least one
  // interval if we get here.)

  const struct ddsi_defrag_iv *iv =
      ddsrt_avl_root(&rsample_defrag_fragtree_treedef, &sample->fragtree);
  assert(iv != NULL);
  if (iv->min == 0 && iv->maxp1 >= sample->sampleinfo->size) {
    // 接受包含超出样本末尾的数据的片段，仅在稍后的阶段过滤它们（或者不过滤）。
    // 在 defragmeter 之前丢弃它们会导致永远无法完成的样本；在 defragmenter 中丢弃它们是可行的，
    // 方法是丢弃到目前为止收集到的该样本的所有片段。
    // (Accept fragments containing data beyond the end of the sample, only to filter them out (or
    // not, as the case may be) at a later stage. Dropping them before the defragmeter leaves us
    // with samples that will never be completed; dropping them in the defragmenter would be
    // feasible by discarding all fragments of that sample collected so far.)

    assert(ddsrt_avl_is_singleton(&sample->fragtree));
    return 1;
  } else {
    return 0;
  }
}

/**
 * @brief 将 defrag 中存储的 rsample 转换为 reorder admin 中存储的格式 (Converts an rsample as
 * stored in defrag to one as stored in a reorder admin)
 *
 * @param[in,out] sample 指向要转换的 ddsi_rsample 结构的指针 (Pointer to the ddsi_rsample structure
 * to be converted)
 */
static void rsample_convert_defrag_to_reorder(struct ddsi_rsample *sample) {
  // 将 defrag 中的 rsample 转换为 reorder admin 中的格式
  // Converts an rsample as stored in defrag to one as stored in a reorder admin

  // 获取 defrag 中的 fragment tree 的根节点
  // Get the root node of the fragment tree in defrag
  struct ddsi_defrag_iv *iv =
      ddsrt_avl_root_non_empty(&rsample_defrag_fragtree_treedef, &sample->u.defrag.fragtree);

  // 获取第一个分片链
  // Get the first fragment chain
  struct ddsi_rdata *fragchain = iv->first;

  // 获取样本信息
  // Get the sample information
  struct ddsi_rsample_info *sampleinfo = sample->u.defrag.sampleinfo;

  // 定义一个用于存储样本链元素的结构体变量
  // Define a structure variable for storing sample chain elements
  struct ddsi_rsample_chain_elem *sce;

  // 获取序列号
  // Get the sequence number
  ddsi_seqno_t seq = sample->u.defrag.seq;

  // 重用内存片段间隔节点用于样本链
  // Re-use memory fragment interval node for sample chain
  sce = (struct ddsi_rsample_chain_elem *)ddsrt_avl_root_non_empty(&rsample_defrag_fragtree_treedef,
                                                                   &sample->u.defrag.fragtree);

  // 设置分片链、下一个元素和样本信息
  // Set the fragment chain, next element and sample information
  sce->fragchain = fragchain;
  sce->next = NULL;
  sce->sampleinfo = sampleinfo;

  // 设置 reorder 结构中的样本链首尾元素
  // Set the first and last elements of the sample chain in the reorder structure
  sample->u.reorder.sc.first = sample->u.reorder.sc.last = sce;

  // 设置最小序列号和最大序列号加一
  // Set the minimum sequence number and maximum sequence number plus one
  sample->u.reorder.min = seq;
  sample->u.reorder.maxp1 = seq + 1;

  // 设置样本数量
  // Set the number of samples
  sample->u.reorder.n_samples = 1;
}

/**
 * @brief 添加一个片段到 defrag 结构中，并返回处理后的 ddsi_rsample 对象。
 *        Add a fragment to the defrag structure and return the processed ddsi_rsample object.
 *
 * @param[in] defrag 指向 ddsi_defrag 结构的指针。Pointer to the ddsi_defrag structure.
 * @param[in] sample 指向 ddsi_rsample 结构的指针。Pointer to the ddsi_rsample structure.
 * @param[in] rdata  指向 ddsi_rdata 结构的指针。Pointer to the ddsi_rdata structure.
 * @param[in] sampleinfo 指向 ddsi_rsample_info 结构的指针。Pointer to the ddsi_rsample_info
 * structure.
 * @return 返回处理后的 ddsi_rsample 对象。Return the processed ddsi_rsample object.
 */
static struct ddsi_rsample *defrag_add_fragment(struct ddsi_defrag *defrag,
                                                struct ddsi_rsample *sample,
                                                struct ddsi_rdata *rdata,
                                                const struct ddsi_rsample_info *sampleinfo) {
  // 定义一个指向 ddsi_rsample_defrag 结构的指针
  // Define a pointer to the ddsi_rsample_defrag structure
  struct ddsi_rsample_defrag *dfsample = &sample->u.defrag;

  // 定义两个指向 ddsi_defrag_iv 结构的指针
  // Define two pointers to the ddsi_defrag_iv structure
  struct ddsi_defrag_iv *predeq, *succ;

  // 定义两个 uint32_t 类型的变量，分别表示片段的最小和最大字节偏移
  // Define two uint32_t variables representing the minimum and maximum byte offsets of the fragment
  const uint32_t min = rdata->min;
  const uint32_t maxp1 = rdata->maxp1;

  // 检查 min 和 maxp1 的值是否合法，确保片段的大小至少为 1 字节
  // Check if the values of min and maxp1 are valid, ensuring that the size of the fragment is at
  // least 1 byte
  assert(min < maxp1);

  // 确保 dfsample 不为空，并且序列号与 sampleinfo 中的序列号相同
  // Ensure that dfsample is not NULL and that its sequence number is the same as the one in
  // sampleinfo
  assert(dfsample);
  assert(dfsample->seq == sampleinfo->seq);

  // 确保存在一个最后的片段
  // Ensure that there is a last fragment
  assert(dfsample->lastfrag);

  // 较昂贵的测试：检查 lastfrag 和 tree 是否一致
  // Relatively expensive test: check if lastfrag and tree are consistent
  assert(dfsample->lastfrag ==
         ddsrt_avl_find_max(&rsample_defrag_fragtree_treedef, &dfsample->fragtree));

  // 打印跟踪信息，显示最后一个片段的相关信息
  // Print trace information, showing the details of the last fragment
  TRACE(defrag, "  lastfrag %p [%" PRIu32 "..%" PRIu32 ")\n", (void *)dfsample->lastfrag,
        dfsample->lastfrag->min, dfsample->lastfrag->maxp1);

  // 判断新片段是否在已有片段之后
  // Check if the new fragment is after the existing fragments
  if (min >= dfsample->lastfrag->min) {
    // 正常情况：片段追加数据
    // Normal case: fragment appends data
    predeq = dfsample->lastfrag;
    TRACE(defrag, "  fast path: predeq = lastfrag\n");
  } else {
    // 慢路径：通过树搜索查找前面的片段
    // Slow path: find preceding fragment by tree search
    predeq = ddsrt_avl_lookup_pred_eq(&rsample_defrag_fragtree_treedef, &dfsample->fragtree, &min);
    assert(predeq);
    TRACE(defrag, "  slow path: predeq = lookup %" PRIu32 " => %p [%" PRIu32 "..%" PRIu32 ")\n",
          min, (void *)predeq, predeq->min, predeq->maxp1);
  }

  // 在接收到包含消息第一个字节的数据包之前，我们有一个哨兵间隔[0,0)，即总是有predeq
  // We have a sentinel interval of [0,0) until we receive a packet that contains the first byte of
  // the message, that is, there should always be predeq
  assert(predeq != NULL);

  // 判断新片段是否在predeq中
  // Check if the new fragment is contained in predeq
  if (predeq->maxp1 >= maxp1) {
    // 新片段包含在predeq中，丢弃新片段；rdata未导致样本完成
    // New fragment is contained in predeq, discard new fragment; rdata did not cause completion of
    // a sample
    TRACE(defrag, "  new contained in predeq\n");
    defrag->discarded_bytes += maxp1 - min;
    return NULL;
  } else if (min <= predeq->maxp1) {
    // 新片段扩展了predeq，将其添加到链中（必然在末尾）；这可能会关闭到predeq的后继的间隙；predeq可能还没有片段链（它可能是哨兵）
    // New fragment extends predeq, add it to the chain (necessarily at the end); this may close the
    // gap to the successor of predeq; predeq need not have a fragment chain yet (it may be the
    // sentinel)
    TRACE(defrag, "  grow predeq with new\n");
    ddsi_rdata_addbias(rdata);
    rdata->nextfrag = NULL;
    if (predeq->first)
      predeq->last->nextfrag = rdata;
    else {
      // 这是哨兵 => 重写样本信息，以便我们最终总是使用第一个片段提供的样本信息
      // 'Tis the sentinel => rewrite the sample info so we eventually always use the sample info
      // contributed by the first fragment
      predeq->first = rdata;
      *dfsample->sampleinfo = *sampleinfo;
    }
    predeq->last = rdata;
    predeq->maxp1 = maxp1;
    // 现在可能可以与后继合并
    // It may now be possible to merge with the successor
    while (defrag_try_merge_with_succ(defrag, dfsample, predeq))
      ;
    return is_complete(dfsample) ? sample : NULL;
  } else if (predeq != dfsample->lastfrag && /* if predeq is last frag, there is no succ */
             (succ = ddsrt_avl_find_succ(&rsample_defrag_fragtree_treedef, &dfsample->fragtree,
                                         predeq)) != NULL &&
             succ->min <= maxp1) {
    // 扩展后继（在低端；不保证链中的每个片段都有价值）；但与predeq不重叠，因此即使密钥发生变化，树结构也不会改变
    // Extends succ (at the low end; no guarantee each individual fragment in the chain adds value);
    // but doesn't overlap with predeq so the tree structure doesn't change even though the key does
    // change
    TRACE(defrag, "  extending succ %p [%" PRIu32 "..%" PRIu32 ") at head\n", (void *)succ,
          succ->min, succ->maxp1);
    ddsi_rdata_addbias(rdata);
    rdata->nextfrag = succ->first;
    succ->first = rdata;
    succ->min = min;
    // 新片段可能覆盖了所有后继内容，此时我们必须更新后继的最大值并查看是否可以与后继合并
    // The new fragment may cover all of succ & more, in which case we must update the max of succ &
    // see if we can merge it with succ-succ
    if (maxp1 > succ->maxp1) {
      TRACE(defrag, "  extending succ at end as well\n");
      succ->maxp1 = maxp1;
      while (defrag_try_merge_with_succ(defrag, dfsample, succ))
        ;
    }
    assert(!is_complete(dfsample));
    return NULL;
  } else {
    // 不会在predeq末尾或succ头部扩展 => 新间隔；rdata未导致样本完成
    // Doesn't extend either predeq at the end or succ at the head => new interval; rdata did not
    // cause completion of sample
    ddsrt_avl_ipath_t path;
    TRACE(defrag, "  new interval\n");
    if (ddsrt_avl_lookup_ipath(&rsample_defrag_fragtree_treedef, &dfsample->fragtree, &min, &path))
      assert(0);
    defrag_rsample_addiv(dfsample, rdata, &path);
    return NULL;
  }
}

/**
 * @brief 检查数据是否为片段
 * @param[in] rdata 指向 ddsi_rdata 结构的指针
 * @param[in] sampleinfo 指向 ddsi_rsample_info 结构的指针
 * @return 如果是片段，则返回 1，否则返回 0
 *
 * @brief Check if the data is a fragment
 * @param[in] rdata Pointer to the ddsi_rdata structure
 * @param[in] sampleinfo Pointer to the ddsi_rsample_info structure
 * @return Returns 1 if it is a fragment, otherwise returns 0
 */
static int ddsi_rdata_is_fragment(const struct ddsi_rdata *rdata,
                                  const struct ddsi_rsample_info *sampleinfo) {
  /* sanity check: min, maxp1 must be within bounds */
  // 确保 rdata 的 min 和 maxp1 在合理范围内
  // Ensure rdata's min and maxp1 are within reasonable bounds
  assert(rdata->min <= rdata->maxp1);
  assert(rdata->maxp1 <= sampleinfo->size);

  // 判断是否为片段：如果 min 为 0 且 maxp1 等于 sampleinfo 的 size，则不是片段
  // Determine if it is a fragment: if min is 0 and maxp1 equals the size of sampleinfo, it is not a
  // fragment
  return !(rdata->min == 0 && rdata->maxp1 == sampleinfo->size);
}

/**
 * @brief 限制 defrag 中的样本数量
 * @param[in] defrag 指向 ddsi_defrag 结构的指针
 * @param[in] seq 序列号
 * @param[out] max_seq 指向最大序列号的指针
 * @return 如果允许添加新样本，则返回 1，否则返回 0
 *
 * @brief Limit the number of samples in defrag
 * @param[in] defrag Pointer to the ddsi_defrag structure
 * @param[in] seq Sequence number
 * @param[out] max_seq Pointer to the maximum sequence number
 * @return Returns 1 if a new sample is allowed, otherwise returns 0
 */
static int defrag_limit_samples(struct ddsi_defrag *defrag,
                                ddsi_seqno_t seq,
                                ddsi_seqno_t *max_seq) {
  struct ddsi_rsample *sample_to_drop = NULL;

  // 如果当前样本数量小于最大样本数量，允许添加新样本
  // If the current number of samples is less than the maximum number of samples, allow adding new
  // samples
  if (defrag->n_samples < defrag->max_samples) return 1;

  /* max_samples >= 1 => some sample present => max_sample != NULL */
  // 确保 max_sample 不为空
  // Ensure max_sample is not NULL
  assert(defrag->max_sample != NULL);

  TRACE(defrag, "  max samples reached\n");

  switch (defrag->drop_mode) {
    case DDSI_DEFRAG_DROP_LATEST:
      TRACE(defrag, "  drop mode = DROP_LATEST\n");
      if (seq > defrag->max_sample->u.defrag.seq) {
        TRACE(defrag, "  new sample is new latest => discarding it\n");
        return 0;
      }
      sample_to_drop = defrag->max_sample;
      break;
    case DDSI_DEFRAG_DROP_OLDEST:
      TRACE(defrag, "  drop mode = DROP_OLDEST\n");
      sample_to_drop = ddsrt_avl_find_min(&defrag_sampletree_treedef, &defrag->sampletree);
      assert(sample_to_drop);
      if (seq < sample_to_drop->u.defrag.seq) {
        TRACE(defrag, "  new sample is new oldest => discarding it\n");
        return 0;
      }
      break;
  }

  // 确保要丢弃的样本不为空
  // Ensure the sample to be dropped is not NULL
  assert(sample_to_drop != NULL);

  // 从 defrag 中删除要丢弃的样本
  // Remove the sample to be dropped from defrag
  defrag_rsample_drop(defrag, sample_to_drop);

  // 如果要丢弃的样本是最大样本，更新最大样本和最大序列号
  // If the sample to be dropped is the maximum sample, update the maximum sample and maximum
  // sequence number
  if (sample_to_drop == defrag->max_sample) {
    defrag->max_sample = ddsrt_avl_find_max(&defrag_sampletree_treedef, &defrag->sampletree);
    *max_seq = defrag->max_sample ? defrag->max_sample->u.defrag.seq : 0;
    TRACE(defrag, "  updating max_sample: now %p %" PRIu64 "\n", (void *)defrag->max_sample,
          defrag->max_sample ? defrag->max_sample->u.defrag.seq : 0);
  }

  return 1;
}

/**
 * @brief 对输入的 rdata 进行解析，如果需要的话将其记录在 defrag 中，并返回一个表示完整消息的 rdata
 * 链，该消息已准备好进行进一步处理。 Analyzes the input rdata, records it in defrag if needed, and
 * returns an rdata chain representing a complete message ready for further processing.
 *
 * @param[in] defrag 解码器对象。The defragmenter object.
 * @param[in] rdata 输入的 rdata。The input rdata.
 * @param[in] sampleinfo 与 rdata 相关的信息。Information related to the rdata.
 * @return 返回一个表示完整消息的 rdata 链。Returns an rdata chain representing a complete message.
 */
struct ddsi_rsample *ddsi_defrag_rsample(struct ddsi_defrag *defrag,
                                         struct ddsi_rdata *rdata,
                                         const struct ddsi_rsample_info *sampleinfo) {
  // 当函数返回时，'rdata' 可能是以下情况之一：
  // (a) 存储在 defrag 中且 rmsg 的引用计数有偏差；
  // (b) 引用计数有偏差并立即返回样本，因为它实际上不是一个片段；
  // (c) 没有影响引用计数且未存储，因为它没有添加任何信息。
  //
  // On return, 'rdata' is either:
  // (a) stored in defrag and the rmsg refcount is biased;
  // (b) refcount is biased and sample returned immediately because it wasn't actually a fragment;
  // (c) no effect on refcount & and not stored because it did not add any information.

  // 入口条件：
  // - rdata 没有引用计数，链式字段不需要初始化。
  // - 如果是第一个片段，则完全初始化 sampleinfo，否则仅初始化 seq、fragsize 和
  // size；将从接收缓冲区分配的内存复制到上面。
  //
  // On entry:
  // - rdata not refcounted, chaining fields need not be initialized.
  // - sampleinfo fully initialised if first frag, else just seq, fragsize and size; will be copied
  // onto memory allocated from the receive buffer

  // 返回：此函数返回的链中引用的所有 rdatas 都已通过在其 rmsgs 的引用计数中添加 BIAS 来记录。
  // Return: all rdatas referenced in the chain returned by this function have been accounted for in
  // the refcount of their rmsgs by adding BIAS to the refcount.
  struct ddsi_rsample *sample, *result;
  ddsi_seqno_t max_seq;
  ddsrt_avl_ipath_t path;

  // 确保 defrag 中的 n_samples 不大于 max_samples
  // Ensure that n_samples in defrag is not greater than max_samples
  assert(defrag->n_samples <= defrag->max_samples);

  // 如果 rdata 不是一个分片，则总是返回完整的数据，将 rdata 转换为有效的链表并返回
  // If rdata is not a fragment, always return complete data, convert rdata to a valid chain and
  // return
  if (!ddsi_rdata_is_fragment(rdata, sampleinfo)) return reorder_rsample_new(rdata, sampleinfo);

  // 确保 defrag 的 max_sample 与树结构一致
  // Ensure that the max_sample of defrag is consistent with the tree structure
  assert(defrag->max_sample == ddsrt_avl_find_max(&defrag_sampletree_treedef, &defrag->sampletree));
  max_seq = defrag->max_sample ? defrag->max_sample->u.defrag.seq : 0;

  // 打印 defrag 和 rdata 的相关信息
  // Print relevant information about defrag and rdata
  TRACE(defrag,
        "defrag_rsample(%p, %p [%" PRIu32 "..%" PRIu32 ") msg %p, %p seq %" PRIu64 " size %" PRIu32
        ") max_seq %p %" PRIu64 ":\n",
        (void *)defrag, (void *)rdata, rdata->min, rdata->maxp1, (void *)rdata->rmsg,
        (void *)sampleinfo, sampleinfo->seq, sampleinfo->size, (void *)defrag->max_sample, max_seq);

  struct reorder_rsample *result;
  struct defrag_sample *sample;
  ddsrt_avl_ipath_t path;

  // 快速路径：rdata 是当前正在进行重组的具有最高序列号的消息的一部分，或者超出该范围
  // Fast path: rdata is part of a message with the highest sequence number currently being
  // reassembled, or beyond that
  if (sampleinfo->seq == max_seq) {
    TRACE(defrag, "  add fragment to max_sample\n");
    result = defrag_add_fragment(defrag, defrag->max_sample, rdata, sampleinfo);
  } else if (!defrag_limit_samples(defrag, sampleinfo->seq, &max_seq)) {
    TRACE(defrag, "  discarding sample\n");
    result = NULL;
  } else if (sampleinfo->seq > max_seq) {
    // 新的具有大于最大值的键的节点总是旧的最大节点的右子节点
    // A new node with a key greater than the maximum is always the right child of the old maximum
    // node
    TRACE(defrag, "  new max sample\n");
    ddsrt_avl_lookup_ipath(&defrag_sampletree_treedef, &defrag->sampletree, &sampleinfo->seq,
                           &path);
    if ((sample = defrag_rsample_new(rdata, sampleinfo)) == NULL) return NULL;
    ddsrt_avl_insert_ipath(&defrag_sampletree_treedef, &defrag->sampletree, sample, &path);
    defrag->max_sample = sample;
    defrag->n_samples++;
    result = NULL;
  } else if ((sample = ddsrt_avl_lookup_ipath(&defrag_sampletree_treedef, &defrag->sampletree,
                                              &sampleinfo->seq, &path)) == NULL) {
    // 新的序列号，但小于最大值
    // A new sequence number, but smaller than the maximum
    TRACE(defrag, "  new sample less than max\n");
    assert(sampleinfo->seq < max_seq);
    if ((sample = defrag_rsample_new(rdata, sampleinfo)) == NULL) return NULL;
    ddsrt_avl_insert_ipath(&defrag_sampletree_treedef, &defrag->sampletree, sample, &path);
    defrag->n_samples++;
    result = NULL;
  } else {
    // 将（或者可能不添加）添加到已知消息中
    // Adds (or, as the case may be, doesn't add) to a known message
    TRACE(defrag, "  add fragment to %p\n", (void *)sample);
    result = defrag_add_fragment(defrag, sample, rdata, sampleinfo);
  }

  if (result != NULL) {
    /* 当完成时，从 defrag
     * 样本树中删除，并转换为重新排序格式。如果它是树中序列最大的样本，则需要更新 max_sample。 */
    /* Once completed, remove from defrag sample tree and convert to reorder format. If it is the
     * sample with the maximum sequence in the tree, an update of max_sample is required. */
    TRACE(defrag, "  complete\n");
    ddsrt_avl_delete(&defrag_sampletree_treedef, &defrag->sampletree, result);
    assert(defrag->n_samples > 0);
    defrag->n_samples--;
    if (result == defrag->max_sample) {
      defrag->max_sample = ddsrt_avl_find_max(&defrag_sampletree_treedef, &defrag->sampletree);
      /* 更新 max_sample: 现在 %p %" PRIu64 "\n" */
      /* Updating max_sample: now %p %" PRIu64 "\n" */
      TRACE(defrag, "  updating max_sample: now %p %" PRIu64 "\n", (void *)defrag->max_sample,
            defrag->max_sample ? defrag->max_sample->u.defrag.seq : 0);
    }
    rsample_convert_defrag_to_reorder(result);
  }

  /* 确保 defrag->max_sample 与 ddsrt_avl_find_max(&defrag_sampletree_treedef, &defrag->sampletree)
   * 相等。 */
  /* Assert that defrag->max_sample is equal to ddsrt_avl_find_max(&defrag_sampletree_treedef,
   * &defrag->sampletree). */
  assert(defrag->max_sample == ddsrt_avl_find_max(&defrag_sampletree_treedef, &defrag->sampletree));
  return result;
}

/**
 * @brief 从分片重组缓冲区中删除指定序列号范围内的所有片段 (Remove all fragments in the specified
 * sequence number range from the defragmentation buffer)
 *
 * @param[in] defrag 分片重组结构体指针 (Pointer to the ddsi_defrag structure)
 * @param[in] min 要删除的片段序列号范围的最小值 (Minimum value of the sequence number range of the
 * fragments to be removed)
 * @param[in] maxp1 要删除的片段序列号范围的最大值加1 (Maximum value of the sequence number range of
 * the fragments to be removed, plus 1)
 */
void ddsi_defrag_notegap(struct ddsi_defrag *defrag, ddsi_seqno_t min, ddsi_seqno_t maxp1) {
  // 所有[min,maxp1)范围内的序列号都不可用，因此必须丢弃该范围内的任何片段。用于Hearbeats（通过设置min=1）和Gaps。
  // All sequence numbers in [min,maxp1) are unavailable so any fragments in that range must be
  // discarded. Used both for Hearbeats (by setting min=1) and for Gaps.
  struct ddsi_rsample *s = ddsrt_avl_lookup_succ_eq(
      &defrag_sampletree_treedef, &defrag->sampletree,
      &min);  // 查找大于等于min的第一个片段 (Find the first fragment greater than or equal to min)

  // 当片段存在且片段序列号小于maxp1时，继续循环 (Continue looping while the fragment exists and its
  // sequence number is less than maxp1)
  while (s && s->u.defrag.seq < maxp1) {
    struct ddsi_rsample *s1 = ddsrt_avl_find_succ(
        &defrag_sampletree_treedef, &defrag->sampletree,
        s);  // 查找当前片段的后继片段 (Find the successor of the current fragment)
    defrag_rsample_drop(defrag, s);  // 从分片重组缓冲区中删除当前片段 (Remove the current fragment
                                     // from the defragmentation buffer)
    s = s1;  // 将后继片段设置为当前片段 (Set the successor fragment as the current fragment)
  }

  // 更新最大样本值 (Update the maximum sample value)
  defrag->max_sample = ddsrt_avl_find_max(&defrag_sampletree_treedef, &defrag->sampletree);
}

/**
 * @brief 生成一个NACK位图，表示缺失的片段。
 * Generate a NACK bitmap representing missing fragments.
 *
 * @param[in] defrag 指向ddsi_defrag结构的指针，用于处理数据碎片。
 *                  Pointer to the ddsi_defrag structure for handling data fragments.
 * @param[in] seq 序列号。
 *               Sequence number.
 * @param[in] maxfragnum 最大片段编号。
 *                      Maximum fragment number.
 * @param[out] map 指向ddsi_fragment_number_set_header结构的指针，用于存储位图信息。
 *                 Pointer to the ddsi_fragment_number_set_header structure for storing bitmap
 * information.
 * @param[out] mapbits 位图数组。
 *                    Bitmap array.
 * @param[in] maxsz 位图数组的最大大小。
 *                 Maximum size of the bitmap array.
 * @return ddsi_defrag_nackmap_result 枚举值，表示NACK位图的结果。
 *         Enumeration value of ddsi_defrag_nackmap_result, indicating the result of the NACK
 * bitmap.
 */
enum ddsi_defrag_nackmap_result ddsi_defrag_nackmap(struct ddsi_defrag *defrag,
                                                    ddsi_seqno_t seq,
                                                    uint32_t maxfragnum,
                                                    struct ddsi_fragment_number_set_header *map,
                                                    uint32_t *mapbits,
                                                    uint32_t maxsz) {
  struct ddsi_rsample *s;
  struct ddsi_defrag_iv *iv;
  uint32_t i, fragsz, nfrags;
  assert(maxsz <= 256);
  s = ddsrt_avl_lookup(&defrag_sampletree_treedef, &defrag->sampletree, &seq);
  if (s == NULL) {
    if (maxfragnum == UINT32_MAX) {
      /* 如果调用者和分片处理器都不知道样本的信息，则返回未知样本 */
      /* If neither the caller nor the defragmenter knows anything about the sample, say so */
      return DDSI_DEFRAG_NACKMAP_UNKNOWN_SAMPLE;
    } else {
      /* 如果调用者表示应该有[0..maxfragnum]的片段，但我们没有记录，
         我们仍然可以生成一个正确的nackmap */
      /* If caller says fragments [0..maxfragnum] should be there, but
         we do not have a record of it, we can still generate a proper
         nackmap */
      if (maxfragnum + 1 > maxsz)
        map->numbits = maxsz;
      else
        map->numbits = maxfragnum + 1;
      map->bitmap_base = 0;
      ddsi_bitset_one(map->numbits, mapbits);
      return DDSI_DEFRAG_NACKMAP_FRAGMENTS_MISSING;
    }
  }

  /* 将maxfragnum限制为实际样本大小，这样调用者在不知道maxfragnum的情况下也能获得准确的信息。
     MAXFRAGNUM是基于0的，所以最多是nfrags-1。 */
  /* Limit maxfragnum to actual sample size, so that the caller can
     get accurate info without knowing maxfragnum.  MAXFRAGNUM is
     0-based, so at most nfrags-1. */
  fragsz = s->u.defrag.sampleinfo->fragsize;
  nfrags = (s->u.defrag.sampleinfo->size + fragsz - 1) / fragsz;
  if (maxfragnum >= nfrags) maxfragnum = nfrags - 1;

  /* 确定位图开始和大小 */
  /* Determine bitmap start & size */
  {
    struct ddsi_defrag_iv *liv = s->u.defrag.lastfrag;
    ddsi_fragment_number_t map_end;
    iv = ddsrt_avl_find_min(&rsample_defrag_fragtree_treedef, &s->u.defrag.fragtree);
    assert(iv != NULL);
    map->bitmap_base = iv->maxp1 / fragsz;
    if (liv->maxp1 < (maxfragnum + 1) * fragsz && liv->maxp1 < s->u.defrag.sampleinfo->size)
      map_end = maxfragnum;
    else if (liv->min > 0)
      map_end = (liv->min - 1) / fragsz;
    else
      map_end = 0;
    if (map_end < map->bitmap_base) return DDSI_DEFRAG_NACKMAP_ALL_ADVERTISED_FRAGMENTS_KNOWN;
    map->numbits = map_end - map->bitmap_base + 1;
    iv = ddsrt_avl_find_succ(&rsample_defrag_fragtree_treedef, &s->u.defrag.fragtree, iv);
  }

  /* 清除位图，然后为可用片段中的间隙设置位 */
  /* Clear bitmap, then set bits for gaps in available fragments */
  if (map->numbits > maxsz) map->numbits = maxsz;
  ddsi_bitset_zero(map->numbits, mapbits);
  i = map->bitmap_base;
  while (iv && i < map->bitmap_base + map->numbits) {
    uint32_t bound = iv->min / fragsz;
    if ((iv->min % fragsz) != 0) {
      ++bound;
    }
    for (; i < map->bitmap_base + map->numbits && i < bound; i++) {
      unsigned x = (unsigned)(i - map->bitmap_base);
      ddsi_bitset_set(map->numbits, mapbits, x);
    }
    i = iv->maxp1 / fragsz;
    iv = ddsrt_avl_find_succ(&rsample_defrag_fragtree_treedef, &s->u.defrag.fragtree, iv);
  }
  /* 为最高间隔之后的缺失片段设置位 */
  /* and set bits for missing fragments beyond the highest interval */
  for (; i < map->bitmap_base + map->numbits; i++) {
    unsigned x = (unsigned)(i - map->bitmap_base);
    ddsi_bitset_set(map->numbits, mapbits, x);
  }
  return DDSI_DEFRAG_NACKMAP_FRAGMENTS_MISSING;
}

/**
 * @brief 对 defrag 结构进行修剪，删除与指定目标前缀相关的片段。此函数在 Volatile Secure
 * 读取器被删除时使用。 Prunes the defrag structure by removing fragments associated with a
 * specified destination prefix. This function is used when a Volatile Secure reader is deleted.
 *
 * @param[in] defrag 指向 ddsi_defrag 结构的指针。A pointer to the ddsi_defrag structure.
 * @param[in] dst 指向目标 GUID 前缀的指针。A pointer to the destination GUID prefix.
 * @param[in] min 序列号下限。The lower bound of sequence numbers.
 */
void ddsi_defrag_prune(struct ddsi_defrag *defrag, ddsi_guid_prefix_t *dst, ddsi_seqno_t min) {
  // 查找具有大于等于 min 的序列号的第一个样本。Find the first sample with a sequence number greater
  // than or equal to min.
  struct ddsi_rsample *s =
      ddsrt_avl_lookup_succ_eq(&defrag_sampletree_treedef, &defrag->sampletree, &min);
  // 遍历样本树。Iterate through the sample tree.
  while (s) {
    // 查找 s 的后继样本。Find the successor of s.
    struct ddsi_rsample *s1 =
        ddsrt_avl_find_succ(&defrag_sampletree_treedef, &defrag->sampletree, s);
    // 如果样本的目标 GUID 前缀与给定的目标前缀相等，则删除该样本。If the sample's destination GUID
    // prefix is equal to the given destination prefix, drop the sample.
    if (ddsi_guid_prefix_eq(&s->u.defrag.sampleinfo->rst->dst_guid_prefix, dst)) {
      defrag_rsample_drop(defrag, s);
    }
    // 更新 s 为后继样本。Update s to be the successor sample.
    s = s1;
  }
  // 更新 defrag 的最大样本值。Update the max_sample value of defrag.
  defrag->max_sample = ddsrt_avl_find_max(&defrag_sampletree_treedef, &defrag->sampletree);
}

/* REORDER -------------------------------------------------------------

   The reorder index tracks out-of-order messages as non-overlapping,
   non-consecutive intervals of sequence numbers, with each interval
   pointing to a chain of rsamples (rsample_chain{,_elem}).  The
   maximum number of samples stored by the radmin is max_samples
   (setting it to 2**32-1 effectively makes it unlimited, by you're
   then you're probably into TB territority as you need at least an
   rmsg, rdata, sampleinfo, rsample, and a rsample_chain_elem, which
   adds up to quite a few bytes).

   The policy is to prefer the lowest sequence numbers, as those need
   to be delivered before the higher ones can be, and also because one
   radmin tracks only a single sequence.  Historical data uses a
   per-reader radmin.

   Each reliable proxy writer has a reorder admin for reordering
   messages, the "primary" reorder admin.  For the primary one, it is
   possible to store indexing data in memory originally allocated
   memory for defragmenting, as the defragmenter is done with it and
   this admin is the only one indexing the sample.

   Each out-of-sync proxy-writer--reader match also has an reorder
   instance, a "secondary" reorder admin, but those can't re-use
   memory like the proxy-writer's can, because there can be any number
   of them.  Before inserting in one of these, the sample must first
   be replicated using reorder_rsample_dup(), which fortunately is an
   extremely cheap operation.

   A sample either goes to the primary one (which may store it, reject
   it, or return it and subsequent samples immediately) [CASE I], or
   it goes to any number of secondary ones [CASE II].

   The reorder_rsample function may require updates to the reference
   counts of the rmsgs referenced by the rdatas in the sample it was
   called with (and _only_ to those of that particular sample, as
   others underwent all this processing before).  The
   "refcount_adjust" in/out parameter is updated to reflect the
   required change.

   A complicating factor is that after storing a sample in a reorder
   admin it potentially becomes part of a chain of samples, and may be
   located anywhere within that chain.  When that happens, the rsample
   parameter provided to reorder_rsample becomes useless for adjusting
   the reference counts as required.

   The initial reference count as it comes out of defragmentation is
   always BIAS-per-rdata, which means all rmgs referenced by the
   sample have refcount = BIAS if there is only ever a single sample
   in each rmsg.  (If multiple data submessages have been packed into
   a single message, they'll all contribute to the refcount.)

   The reference count adjustment is incremented by reorder_rsample
   whenever it stores or forwards the sample, and left unchanged when
   it rejects it (old samples & duplicates).  The initial reference
   needs to be accounted for as well, and so:

   - In [CASE I]: accept (or forward): +1 for accepting it, -BIAS for
     the initial reference, for a net change of 1-BIAS.  Reject: 0 for
     rejecting it, still -BIAS for the initial reference, for a net
     change of -BIAS.

   - In [CASE 2], each reorder admin gets its own copy of the sample,
     and therefore the sample that came out of defragmentation is
     unchanged, and may thus be used, regardless of the adjustment
     required.

     Accept by M out N: +M for accepting, 0 for the N-M rejects, -BIAS
     for the initial reference.  For a net change of M-BIAS.

   So in both cases, the adjustment needed is the number of reorder
   admins that accepted it, less BIAS for the initial reference.  We
   can't use the original sample because of [CASE I], so we adjust
   based on the fragment chain instead of the sample.  Example code is
   in the overview comment at the top of this file. */
/* REORDER -----------------------------------------------------------

   重排序索引通过非重叠、非连续的序列号区间跟踪乱序消息，每个区间指向一条rsamples链（rsample_chain{,_elem}）。radmin存储的最大样本数为max_samples（将其设置为2**32-1实际上使其无限制，但此时您可能进入TB领域，因为至少需要一个rmsg、rdata、sampleinfo、rsample和rsample_chain_elem，这些加起来相当多字节）。

   策略是优先选择最低的序列号，因为这些序列号需要在更高的序列号之前传递，而且因为一个radmin只跟踪一个序列。历史数据使用每个读取器的radmin。

   每个可靠的代理写入器都有一个用于重新排序消息的重排序管理器，即“主”重排序管理器。对于主要的管理器，可以将索引数据存储在最初分配的内存中，用于内存碎片整理，因为碎片整理器已经完成了工作，而且这个管理器是唯一对样本进行索引的。

   每个不同步的代理写入器-读取器匹配也有一个重排序实例，即“次要”的重排序管理器，但是这些不能像代理写入器那样重复使用内存，因为它们可以有任意数量。在插入其中一个之前，首先需要使用reorder_rsample_dup()复制样本，幸运的是这是一种非常便宜的操作。

   样本要么进入主要管理器（可能存储它、拒绝它或立即返回它和后续样本）[CASE
   I]，要么进入任意数量的次要管理器[CASE II]。

   reorder_rsample函数可能需要更新由样本中的rdatas引用的rmsgs的引用计数（仅限于特定样本的引用计数，因为其他样本在此之前已经经过了所有处理）。"refcount_adjust"输入/输出参数会根据所需更改进行更新。

   一个复杂的因素是，在将样本存储到重排序管理器中后，它可能成为样本链的一部分，并且可能位于该链的任何位置。当发生这种情况时，提供给reorder_rsample的rsample参数对于根据需要调整引用计数变得无用。

   初始引用计数在碎片整理结束时始终为BIAS-per-rdata，这意味着如果每个rmsg中只有一个样本，那么样本引用的所有rmgs的引用计数=BIAS。（如果将多个数据子消息打包到单个消息中，它们都将贡献给引用计数。）

   当reorder_rsample存储或转发样本时，引用计数调整会递增，当拒绝它时（旧样本和重复项）保持不变。还需要考虑初始引用，因此：

   - 在[CASE
   I]中：接受（或转发）：+1表示接受，-BIAS表示初始引用，净变化为1-BIAS。拒绝：0表示拒绝，仍然是-
   BIAS表示初始引用，净变化为-BIAS。

   - 在[CASE
   2]中，每个重排序管理器都有自己的样本副本，因此从碎片整理中得到的样本没有改变，因此可以使用，无论所需的调整如何。

     N中的M接受：+M表示接受，0表示N-M拒绝，-BIAS表示初始引用。净变化为M-BIAS。

   因此，在这两种情况下，所需的调整是接受它的重排序管理器的数量，减去初始引用的BIAS。我们不能使用原始样本，因为[CASE
   I]，所以我们根据片段链而不是样本进行调整。示例代码在此文件顶部的概述注释中。*/

/*
概括总结：

重排序索引通过非重叠、非连续的序列号区间跟踪乱序消息，每个区间指向一条rsamples链。radmin存储的最大样本数为max_samples。

策略是优先选择最低的序列号。历史数据使用每个读取器的radmin。

每个可靠的代理写入器都有一个用于重新排序消息的重排序管理器，即“主”重排序管理器。每个不同步的代理写入器-读取器匹配也有一个重排序实例，即“次要”的重排序管理器。

样本要么进入主要管理器（可能存储它、拒绝它或立即返回它和后续样本），要么进入任意数量的次要管理器。

reorder_rsample函数可能需要更新由样本中的rdatas引用的rmsgs的引用计数。

在将样本存储到重排序管理器中后，它可能成为样本链的一部分，并且可能位于该链的任何位置。

初始引用计数在碎片整理结束时始终为BIAS-per-rdata。当reorder_rsample存储或转发样本时，引用计数调整会递增，当拒绝它时保持不变。

所需的调整是接受它的重排序管理器的数量，减去初始引用的BIAS。我们不能使用原始样本，因此我们根据片段链而不是样本进行调整。
*/

/* ddsi_reorder 结构体定义 */
struct ddsi_reorder {
  ddsrt_avl_tree_t sampleivtree;       // 样本间隔树（Sample interval tree）
  struct ddsi_rsample *max_sampleiv;   // 最大样本间隔值（Max sample interval value）
  ddsi_seqno_t next_seq;               // 下一个序列号（Next sequence number）
  enum ddsi_reorder_mode mode;         // 重排序模式（Reorder mode）
  uint32_t max_samples;                // 最大样本数（Maximum samples）
  uint32_t n_samples;                  // 当前样本数（Current number of samples）
  uint64_t discarded_bytes;            // 丢弃的字节数（Discarded bytes）
  const struct ddsrt_log_cfg *logcfg;  // 日志配置（Log configuration）
  bool late_ack_mode;                  // 延迟确认模式（Late acknowledgement mode）
  bool trace;                          // 跟踪标志（Trace flag）
};

/* 重排序样本间隔树定义 */
static const ddsrt_avl_treedef_t reorder_sampleivtree_treedef =
    DDSRT_AVL_TREEDEF_INITIALIZER(offsetof(struct ddsi_rsample, u.reorder.avlnode),
                                  offsetof(struct ddsi_rsample, u.reorder.min),
                                  compare_seqno,
                                  0);

/**
 * @brief 创建新的 ddsi_reorder 实例
 * @param[in] logcfg 日志配置
 * @param[in] mode 重排序模式
 * @param[in] max_samples 最大样本数
 * @param[in] late_ack_mode 延迟确认模式
 * @return ddsi_reorder 实例指针，如果分配失败则返回 NULL
 */
struct ddsi_reorder *ddsi_reorder_new(const struct ddsrt_log_cfg *logcfg,
                                      enum ddsi_reorder_mode mode,
                                      uint32_t max_samples,
                                      bool late_ack_mode) {
  struct ddsi_reorder *r;
  if ((r = ddsrt_malloc(sizeof(*r))) == NULL) return NULL;  // 分配内存（Allocate memory）
  ddsrt_avl_init(&reorder_sampleivtree_treedef,
                 &r->sampleivtree);  // 初始化样本间隔树（Initialize sample interval tree）
  r->max_sampleiv = NULL;  // 设置最大样本间隔值为空（Set max sample interval value to NULL）
  r->next_seq = 1;         // 设置下一个序列号为 1（Set next sequence number to 1）
  r->mode = mode;          // 设置重排序模式（Set reorder mode）
  r->max_samples = max_samples;  // 设置最大样本数（Set maximum samples）
  r->n_samples = 0;        // 设置当前样本数为 0（Set current number of samples to 0）
  r->discarded_bytes = 0;  // 设置丢弃的字节数为 0（Set discarded bytes to 0）
  r->late_ack_mode = late_ack_mode;  // 设置延迟确认模式（Set late acknowledgement mode）
  r->logcfg = logcfg;                // 设置日志配置（Set log configuration）
  r->trace = (logcfg->c.mask & DDS_LC_RADMIN) != 0;  // 设置跟踪标志（Set trace flag）
  return r;
}

/**
 * @brief 获取 ddsi_reorder 的统计信息
 * @param[in] reorder ddsi_reorder 实例
 * @param[out] discarded_bytes 丢弃的字节数
 */
void ddsi_reorder_stats(struct ddsi_reorder *reorder, uint64_t *discarded_bytes) {
  *discarded_bytes = reorder->discarded_bytes;  // 返回丢弃的字节数（Return discarded bytes）
}

/**
 * @brief 取消对分片链表的引用
 * @param[in] frag 分片链表头
 */
void ddsi_fragchain_unref(struct ddsi_rdata *frag) {
  struct ddsi_rdata *frag1;
  while (frag) {
    frag1 = frag->nextfrag;  // 获取下一个分片（Get next fragment）
    ddsi_rdata_unref(frag);  // 取消对当前分片的引用（Unreference current fragment）
    frag = frag1;            // 移动到下一个分片（Move to next fragment）
  }
}

/**
 * @brief 释放 ddsi_reorder 结构体中的资源
 * @param r 指向要释放的 ddsi_reorder 结构体的指针
 *
 * Free resources in the ddsi_reorder structure.
 * @param r Pointer to the ddsi_reorder structure to be freed.
 */
void ddsi_reorder_free(struct ddsi_reorder *r) {
  struct ddsi_rsample *iv;
  struct ddsi_rsample_chain_elem *sce;

  // FIXME: 可以使用 treewalk 替代 findmin/delete
  // FXIME: instead of findmin/delete, a treewalk can be used.
  iv = ddsrt_avl_find_min(&reorder_sampleivtree_treedef, &r->sampleivtree);
  while (iv) {
    ddsrt_avl_delete(&reorder_sampleivtree_treedef, &r->sampleivtree, iv);
    sce = iv->u.reorder.sc.first;
    while (sce) {
      struct ddsi_rsample_chain_elem *sce1 = sce->next;
      ddsi_fragchain_unref(sce->fragchain);
      sce = sce1;
    }
    iv = ddsrt_avl_find_min(&reorder_sampleivtree_treedef, &r->sampleivtree);
  }
  ddsrt_free(r);
}

/**
 * @brief 将 rsample 添加到 reorder 的 sampleivtree 中
 * @param reorder 指向 ddsi_reorder 结构体的指针
 * @param rsample 指向要添加的 ddsi_rsample 结构体的指针
 *
 * Add rsample to the sampleivtree of reorder.
 * @param reorder Pointer to the ddsi_reorder structure.
 * @param rsample Pointer to the ddsi_rsample structure to be added.
 */
static void reorder_add_rsampleiv(struct ddsi_reorder *reorder, struct ddsi_rsample *rsample) {
  ddsrt_avl_ipath_t path;
  if (ddsrt_avl_lookup_ipath(&reorder_sampleivtree_treedef, &reorder->sampleivtree,
                             &rsample->u.reorder.min, &path) != NULL)
    assert(0);
  ddsrt_avl_insert_ipath(&reorder_sampleivtree_treedef, &reorder->sampleivtree, rsample, &path);
}

#ifndef NDEBUG
/**
 * @brief 检查 rsample 是否为单例
 * @param s 指向 ddsi_rsample_reorder 结构体的指针
 * @return 如果是单例，返回 1；否则返回 0
 *
 * Check if rsample is a singleton.
 * @param s Pointer to the ddsi_rsample_reorder structure.
 * @return Returns 1 if it is a singleton, otherwise returns 0.
 */
static int rsample_is_singleton(const struct ddsi_rsample_reorder *s) {
  assert(s->min < s->maxp1);
  if (s->n_samples != 1) return 0;
  assert(s->min + 1 == s->maxp1);
  assert(s->min + s->n_samples <= s->maxp1);
  assert(s->sc.first != NULL);
  assert(s->sc.first == s->sc.last);
  assert(s->sc.first->next == NULL);
  return 1;
}
#endif

/**
 * @brief 将两个 rsample 间隔追加到一起
 * @param a 指向第一个 ddsi_rsample 结构体的指针
 * @param b 指向第二个 ddsi_rsample 结构体的指针
 *
 * Append two rsample intervals together.
 * @param a Pointer to the first ddsi_rsample structure.
 * @param b Pointer to the second ddsi_rsample structure.
 */
static void append_rsample_interval(struct ddsi_rsample *a, struct ddsi_rsample *b) {
  a->u.reorder.sc.last->next = b->u.reorder.sc.first;
  a->u.reorder.sc.last = b->u.reorder.sc.last;
  a->u.reorder.maxp1 = b->u.reorder.maxp1;
  a->u.reorder.n_samples += b->u.reorder.n_samples;
}

/**
 * @brief 尝试将待丢弃的数据样本合并到目标数据样本中，并从重排序列表中删除待丢弃的数据样本。
 *        Try to append the sample to be discarded to the target sample and remove it from the
 * reorder list.
 *
 * @param[in] reorder 重排序结构体指针。Pointer to the reorder structure.
 * @param[in,out] appendto 目标数据样本，待合并的数据样本将被添加到此数据样本中。Target data sample,
 * the sample to be discarded will be appended to this sample.
 * @param[in] todiscard 待丢弃的数据样本。Data sample to be discarded.
 *
 * @return 如果成功合并和删除，则返回1；如果失败，则返回0。Returns 1 if successfully appended and
 * removed; 0 if failed.
 */
static int reorder_try_append_and_discard(struct ddsi_reorder *reorder,
                                          struct ddsi_rsample *appendto,
                                          struct ddsi_rsample *todiscard) {
  // 检查待丢弃的数据样本是否为空
  // Check if the sample to be discarded is NULL
  if (todiscard == NULL) {
    TRACE(reorder, "  try_append_and_discard: fail: todiscard = NULL\n");
    return 0;
  } else if (appendto->u.reorder.maxp1 < todiscard->u.reorder.min) {
    // 如果目标数据样本的最大序列号加1小于待丢弃数据样本的最小序列号，说明存在间隙，无法合并
    // If the maximum sequence number plus 1 of the target sample is less than the minimum sequence
    // number of the sample to be discarded, there is a gap and they cannot be merged
    TRACE(reorder,
          "  try_append_and_discard: fail: appendto = [%" PRIu64 ",%" PRIu64
          ") @ %p, "
          "todiscard = [%" PRIu64 ",%" PRIu64 ") @ %p - gap\n",
          appendto->u.reorder.min, appendto->u.reorder.maxp1, (void *)appendto,
          todiscard->u.reorder.min, todiscard->u.reorder.maxp1, (void *)todiscard);
    return 0;
  } else {
    // 成功合并数据样本
    // Successfully merge data samples
    TRACE(reorder,
          "  try_append_and_discard: success: appendto = [%" PRIu64 ",%" PRIu64
          ") @ %p, "
          "todiscard = [%" PRIu64 ",%" PRIu64 ") @ %p\n",
          appendto->u.reorder.min, appendto->u.reorder.maxp1, (void *)appendto,
          todiscard->u.reorder.min, todiscard->u.reorder.maxp1, (void *)todiscard);
    assert(todiscard->u.reorder.min == appendto->u.reorder.maxp1);
    // 从重排序列表中删除待丢弃的数据样本
    // Remove the sample to be discarded from the reorder list
    ddsrt_avl_delete(&reorder_sampleivtree_treedef, &reorder->sampleivtree, todiscard);
    // 将待丢弃的数据样本合并到目标数据样本中
    // Merge the sample to be discarded into the target sample
    append_rsample_interval(appendto, todiscard);
    TRACE(reorder, "  try_append_and_discard: max_sampleiv needs update? %s\n",
          (todiscard == reorder->max_sampleiv) ? "yes" : "no");
    // 通知调用者是否需要更新reorder->max，如果appendto实际上不在树中，则更新会失败。
    // Notify the caller whether reorder->max needs to be updated. The update will fail if appendto
    // is not actually in the tree.
    return todiscard == reorder->max_sampleiv;
  }
}

/**
 * @brief 复制第一个rsampleiv，但不更新任何引用计数。
 *        这是留给调用者的，因为如果副本最终没有被使用，就不需要更新它们。
 *        rmsg 是要分配的消息，必须是当前正在处理的消息（只能从未提交的rmsg中分配内存），
 *        并且必须由 rsampleiv 中的 rdata 引用。
 *
 * @param rmsg 当前正在处理的消息
 * @param rsampleiv 要复制的rsampleiv
 * @return struct ddsi_rsample* 复制后的新rsampleiv
 */
struct ddsi_rsample *ddsi_reorder_rsample_dup_first(struct ddsi_rmsg *rmsg,
                                                    struct ddsi_rsample *rsampleiv) {
  // 定义新的rsampleiv结构体指针
  struct ddsi_rsample *rsampleiv_new;
  // 定义新的链表元素指针
  struct ddsi_rsample_chain_elem *sce;

#ifndef NDEBUG
  {
    struct ddsi_rdata *d = rsampleiv->u.reorder.sc.first->fragchain;
    while (d && d->rmsg != rmsg) d = d->nextfrag;
    assert(d != NULL);
  }
#endif

  // 为新的rsampleiv_new分配内存
  if ((rsampleiv_new = ddsi_rmsg_alloc(rmsg, sizeof(*rsampleiv_new))) == NULL) return NULL;
  // 为新的链表元素sce分配内存
  if ((sce = ddsi_rmsg_alloc(rmsg, sizeof(*sce))) == NULL) return NULL;

  // 初始化新的链表元素
  sce->fragchain = rsampleiv->u.reorder.sc.first->fragchain;
  sce->next = NULL;
  sce->sampleinfo = rsampleiv->u.reorder.sc.first->sampleinfo;

  // 初始化新的rsampleiv_new结构体
  rsampleiv_new->u.reorder.min = rsampleiv->u.reorder.min;
  rsampleiv_new->u.reorder.maxp1 = rsampleiv_new->u.reorder.min + 1;
  rsampleiv_new->u.reorder.n_samples = 1;
  rsampleiv_new->u.reorder.sc.first = rsampleiv_new->u.reorder.sc.last = sce;

  // 返回新的rsampleiv_new结构体指针
  return rsampleiv_new;
}

/**
 * @brief 获取rsample的片段链。
 *
 * @param rsample 要获取片段链的rsample
 * @return struct ddsi_rdata* 片段链
 */
struct ddsi_rdata *ddsi_rsample_fragchain(struct ddsi_rsample *rsample) {
  assert(rsample_is_singleton(&rsample->u.reorder));
  return rsample->u.reorder.sc.first->fragchain;
}

/**
 * @brief 将重排序模式转换为字符表示。
 *
 * @param reorder 重排序结构体
 * @return char 重排序模式的字符表示
 */
static char reorder_mode_as_char(const struct ddsi_reorder *reorder) {
  switch (reorder->mode) {
    case DDSI_REORDER_MODE_NORMAL:
      return 'R';
    case DDSI_REORDER_MODE_MONOTONICALLY_INCREASING:
      return 'U';
    case DDSI_REORDER_MODE_ALWAYS_DELIVER:
      return 'A';
  }
  assert(0);
  return '?';
}

/**
 * @brief 删除最后一个样本 (Delete the last sample)
 *
 * @param[in] reorder 指向 ddsi_reorder 结构的指针 (Pointer to a ddsi_reorder structure)
 */
static void delete_last_sample(struct ddsi_reorder *reorder) {
  // 定义一个指向 ddsi_rsample_reorder 结构的指针，用于存储最大样本间隔 (Define a pointer to a
  // ddsi_rsample_reorder structure to store the maximum sample interval)
  struct ddsi_rsample_reorder *last = &reorder->max_sampleiv->u.reorder;
  // 定义一个指向 ddsi_rdata 结构的指针，用于存储片段链 (Define a pointer to a ddsi_rdata structure
  // to store the fragment chain)
  struct ddsi_rdata *fragchain;

  /* 这只是删除它，并不调整计数。不应该在只有一个样本的 radmin 上调用它。
     (This just removes it, it doesn't adjust the count. It is not
     supposed to be called on an radmin with only one sample.) */
  assert(reorder->n_samples > 0);
  assert(reorder->max_sampleiv != NULL);

  if (last->sc.first == last->sc.last) {
    /* 最后一个样本在它自己的间隔中 - 删除它，并重新计算 max_sampleiv。
       (Last sample is in an interval of its own - delete it, and
       recalc max_sampleiv.) */
    TRACE(reorder, "  delete_last_sample: in singleton interval\n");
    if (last->sc.first->sampleinfo) reorder->discarded_bytes += last->sc.first->sampleinfo->size;
    fragchain = last->sc.first->fragchain;
    ddsrt_avl_delete(&reorder_sampleivtree_treedef, &reorder->sampleivtree, reorder->max_sampleiv);
    reorder->max_sampleiv =
        ddsrt_avl_find_max(&reorder_sampleivtree_treedef, &reorder->sampleivtree);
    /* 如果 sampleivtree 为空，除了我们选择不允许它之外，没有任何损害。
       (No harm done if the sampleivtree is empty, except that we
       chose not to allow it) */
    assert(reorder->max_sampleiv != NULL);
  } else {
    /* 最后一个样本将从最后一个间隔中删除。这需要扫描样本链，因为它是一个单向链表
       （所以你可能不希望 max_samples
       设置得非常大！）。不能是单例列表，所以可以削减一个循环条件的评估。 (Last sample is to be
       removed from the final interval.  Which requires scanning the sample chain because it is a
       singly-linked list (so you might not want max_samples set very
       large!).  Can't be a singleton list, so might as well chop off
       one evaluation of the loop condition.) */
    struct ddsi_rsample_chain_elem *e, *pe;
    TRACE(reorder, "  delete_last_sample: scanning last interval [%" PRIu64 "..%" PRIu64 ")\n",
          last->min, last->maxp1);
    assert(last->n_samples >= 1);
    assert(last->min + last->n_samples <= last->maxp1);
    e = last->sc.first;
    do {
      pe = e;
      e = e->next;
    } while (e != last->sc.last);
    if (e->sampleinfo) reorder->discarded_bytes += e->sampleinfo->size;
    fragchain = e->fragchain;
    pe->next = NULL;
    assert(pe->sampleinfo == NULL || pe->sampleinfo->seq + 1 < last->maxp1);
    last->sc.last = pe;
    last->maxp1--;
    last->n_samples--;
  }

  // 取消对片段链的引用 (Unreference the fragment chain)
  ddsi_fragchain_unref(fragchain);
}

/**
 * @brief
 * 添加一个rsample（表示为间隔）到重排序管理，并返回因插入而准备好交付的连续样本链。因此，如果返回一个样本链，则由rsampleiv引用的样本是链中的第一个。
 * Adds an rsample (represented as an interval) to the reorder admin and returns the chain of
 * consecutive samples ready for delivery because of the insertion. Consequently, if it returns a
 * sample chain, the sample referenced by rsampleiv is the first in the chain.
 *
 * @param sc [in] 指向dds_rsample_chain结构体的指针 Pointer to dds_rsample_chain structure
 * @param reorder [in] 指向dds_reorder结构体的指针 Pointer to dds_reorder structure
 * @param rsampleiv [in] 指向dds_rsample结构体的指针 Pointer to dds_rsample structure
 * @param refcount_adjust [out] 调整引用计数的指针 Pointer to adjust reference count
 * @param delivery_queue_full_p [in] 交付队列是否已满的标志 Flag indicating whether the delivery
 * queue is full or not
 * @return 返回ddsi_reorder_result_t类型的结果 Result of type ddsi_reorder_result_t
 */
ddsi_reorder_result_t ddsi_reorder_rsample(struct ddsi_rsample_chain *sc,
                                           struct ddsi_reorder *reorder,
                                           struct ddsi_rsample *rsampleiv,
                                           int *refcount_adjust,
                                           int delivery_queue_full_p) {
  /* 添加一个rsample（表示为间隔）到重排序管理
    Adds an rsample (represented as an interval) to the reorder admin

    并返回因插入而准备好交付的连续样本链
    and returns the chain of consecutive samples ready for delivery because of the insertion

    因此，如果返回一个样本链，则由rsampleiv引用的样本是链中的第一个
    Consequently, if it returns a sample chain, the sample referenced by rsampleiv is the first in
    the chain

    refcount_adjust is incremented if the sample is not discarded. */
  struct ddsi_rsample_reorder *s = &rsampleiv->u.reorder;

  // 打印重排序样本信息
  // Print the information of the reordering sample
  TRACE(reorder, "reorder_sample(%p %c, %" PRIu64 " @ %p) expecting %" PRIu64 ":\n",
        (void *)reorder, reorder_mode_as_char(reorder), rsampleiv->u.reorder.min, (void *)rsampleiv,
        reorder->next_seq);

  /**
   * @note 输入的 rsample 必须是单例
   * @note The incoming rsample must be a singleton
   */
  assert(rsample_is_singleton(s));

/**
 * @note 重排序不能包含序列号 <= next 的样本；当且仅当重排序非空时，max 必须设置。
 * @note Reorder must not contain samples with sequence numbers <= next
 *       seq; max must be set iff the reorder is non-empty.
 */
#ifndef NDEBUG
  {
    // 查找最小的样本
    // Find the minimum sample
    struct ddsi_rsample *min =
        ddsrt_avl_find_min(&reorder_sampleivtree_treedef, &reorder->sampleivtree);
    if (min) {
      // 打印最小样本信息
      // Print the information of the minimum sample
      TRACE(reorder, "  min = %" PRIu64 " @ %p\n", min->u.reorder.min, (void *)min);
    }
    // 检查最小样本是否满足条件
    // Check if the minimum sample meets the condition
    assert(min == NULL || reorder->next_seq < min->u.reorder.min);
    // 检查 max_sampleiv 是否设置正确
    // Check if max_sampleiv is set correctly
    assert((reorder->max_sampleiv == NULL && min == NULL) ||
           (reorder->max_sampleiv != NULL && min != NULL));
  }
#endif
  /* 确保以下条件成立：
     1. 如果 sampleivtree 为空，则 max_sampleiv 必须为 NULL；
     2. 如果 max_sampleiv 不为 NULL，则它必须是 sampleivtree 中的最大元素。
     Ensure the following conditions hold:
     1. If sampleivtree is empty, then max_sampleiv must be NULL;
     2. If max_sampleiv is not NULL, it must be the maximum element in sampleivtree. */
  assert((!!ddsrt_avl_is_empty(&reorder->sampleivtree)) == (reorder->max_sampleiv == NULL));
  assert(reorder->max_sampleiv == NULL ||
         reorder->max_sampleiv ==
             ddsrt_avl_find_max(&reorder_sampleivtree_treedef, &reorder->sampleivtree));

  /* 确保 n_samples 不超过 max_samples
     Ensure n_samples does not exceed max_samples */
  assert(reorder->n_samples <= reorder->max_samples);

  /* 如果 max_sampleiv 存在，打印相关信息
     If max_sampleiv exists, print related information */
  if (reorder->max_sampleiv)
    TRACE(reorder, "  max = [%" PRIu64 ",%" PRIu64 ") @ %p\n", reorder->max_sampleiv->u.reorder.min,
          reorder->max_sampleiv->u.reorder.maxp1, (void *)reorder->max_sampleiv);

  /* 检查是否可以至少交付一个样本
     Check if at least one sample can be delivered */
  if (s->min == reorder->next_seq ||
      (s->min > reorder->next_seq && reorder->mode == DDSI_REORDER_MODE_MONOTONICALLY_INCREASING) ||
      reorder->mode == DDSI_REORDER_MODE_ALWAYS_DELIVER) {
    /* 可以交付至少一个样本，但这会将样本附加到交付队列。
       如果 delivery_queue_full_p 设置，则交付队列已达到其最大长度，因此将其附加到不是一个好主意。
       因此，我们只需拒绝样本。
       (我们必须这样做，我们不能在重新排序管理中有可交付的样本，否则事情会很快出错。) Can deliver at
       least one sample, but that appends samples to the delivery queue. If delivery_queue_full_p is
       set, the delivery queue has hit its maximum length, so appending to it isn't such a great
       idea. Therefore, we simply reject the sample. (We have to, we can't have a deliverable sample
       in the reorder admin, or things go wrong very quickly.) */
    if (delivery_queue_full_p) {
      TRACE(reorder, "  discarding deliverable sample: delivery queue is full\n");
      reorder->discarded_bytes += s->sc.first->sampleinfo->size;
      return DDSI_REORDER_REJECT;
    }

    /* 's' 是要交付的下一个样本；也许我们可以将树中的第一个间隔附加到它上面。
       如果索引为空，这是正常情况，我们可以避免所有处理。
       不可靠的 out-of-order 要么在这里，要么在 discard 中。)
       's' is next sample to be delivered; maybe we can append the
       first interval in the tree to it. We can avoid all processing
       if the index is empty, which is the normal case. Unreliable
       out-of-order either ends up here or in discard.) */
    if (reorder->max_sampleiv != NULL) {
      struct ddsi_rsample *min =
          ddsrt_avl_find_min(&reorder_sampleivtree_treedef, &reorder->sampleivtree);
      TRACE(reorder, "  try append_and_discard\n");
      if (reorder_try_append_and_discard(reorder, rsampleiv, min)) reorder->max_sampleiv = NULL;
    }
    reorder->next_seq = s->maxp1;
    *sc = rsampleiv->u.reorder.sc;
    (*refcount_adjust)++;
    TRACE(reorder, "  return [%" PRIu64 ",%" PRIu64 ")\n", s->min, s->maxp1);

    /* 调整 reorder->n_samples，新样本尚未计算
       Adjust reorder->n_samples, new sample is not counted yet */
    assert(s->maxp1 - s->min >= 1);
    assert(s->maxp1 - s->min <= (int)INT32_MAX);
    assert(s->min + s->n_samples <= s->maxp1);
    assert(reorder->n_samples >= s->n_samples - 1);
    reorder->n_samples -= s->n_samples - 1;
    return (ddsi_reorder_result_t)s->n_samples;
  } else if (s->min < reorder->next_seq) {
    /* 我们已经超越了这个：丢弃它；无需调整 n_samples
       we've moved beyond this one: discard it; no need to adjust
       n_samples */
    TRACE(reorder, "  discard: too old\n");
    reorder->discarded_bytes += s->sc.first->sampleinfo->size;
    return DDSI_REORDER_TOO_OLD; /* don't want refcount increment */
  } else if (ddsrt_avl_is_empty(&reorder->sampleivtree)) {
    // 如果 reorder 的 sampleivtree 为空
    // If the sampleivtree of reorder is empty
    assert(reorder->n_samples == 0);
    TRACE(reorder, "  adding to empty store\n");
    if (reorder->max_samples == 0) {
      // 如果 max_samples 为 0
      // If max_samples is 0
      TRACE(reorder, "  NOT - max_samples hit\n");
      reorder->discarded_bytes += s->sc.first->sampleinfo->size;
      return DDSI_REORDER_REJECT;
    } else {
      // 否则，将 rsampleiv 添加到 reorder 中
      // Otherwise, add rsampleiv to reorder
      reorder_add_rsampleiv(reorder, rsampleiv);
      reorder->max_sampleiv = rsampleiv;
      reorder->n_samples++;
    }
  } else if (((void)assert(reorder->max_sampleiv != NULL)),
             (s->min == reorder->max_sampleiv->u.reorder.maxp1)) {
    // 如果 sampleivtree 不为空且 s->min 等于 reorder->max_sampleiv->u.reorder.maxp1
    // If sampleivtree is not empty and s->min equals reorder->max_sampleiv->u.reorder.maxp1
    if (delivery_queue_full_p) {
      // 如果 delivery_queue_full_p 为真
      // If delivery_queue_full_p is true
      TRACE(
          reorder,
          "  discarding sample: only accepting delayed samples due to backlog in delivery queue\n");
      reorder->discarded_bytes += s->sc.first->sampleinfo->size;
      return DDSI_REORDER_REJECT;
    }

    // 增加最后一个区间
    // Grow the last interval
    TRACE(reorder, "  growing last interval\n");
    if (reorder->n_samples < reorder->max_samples) {
      append_rsample_interval(reorder->max_sampleiv, rsampleiv);
      reorder->n_samples++;
    } else {
      TRACE(reorder, "  discarding sample: max_samples reached and sample at end\n");
      reorder->discarded_bytes += s->sc.first->sampleinfo->size;
      return DDSI_REORDER_REJECT;
    }
  } else if (s->min > reorder->max_sampleiv->u.reorder.maxp1) {
    // 如果 s->min 大于 reorder->max_sampleiv->u.reorder.maxp1
    // If s->min is greater than reorder->max_sampleiv->u.reorder.maxp1
    if (delivery_queue_full_p) {
      // 如果 delivery_queue_full_p 为真
      // If delivery_queue_full_p is true
      TRACE(
          reorder,
          "  discarding sample: only accepting delayed samples due to backlog in delivery queue\n");
      reorder->discarded_bytes += s->sc.first->sampleinfo->size;
      return DDSI_REORDER_REJECT;
    }
    if (reorder->n_samples < reorder->max_samples) {
      // 如果 n_samples 小于 max_samples
      // If n_samples is less than max_samples
      TRACE(reorder, "  new interval at end\n");
      reorder_add_rsampleiv(reorder, rsampleiv);
      reorder->max_sampleiv = rsampleiv;
      reorder->n_samples++;
    } else {
      TRACE(reorder, "  discarding sample: max_samples reached and sample at end\n");
      reorder->discarded_bytes += s->sc.first->sampleinfo->size;
      return DDSI_REORDER_REJECT;
    }
  } else {
    // 查找区间 predeq=[m,n)，使得 m <= s->min，以及 immsucc=[m',n')，使得 m' = s->maxp1：
    // (Find interval predeq=[m,n) such that m <= s->min, and immsucc=[m',n') such that m' =
    // s->maxp1:)
    //
    // - 如果 m <= s->min < n，我们将其丢弃（重复）
    //   (If m <= s->min < n, we discard it (duplicate))
    // - 如果 n=s->min，我们可以将 s 添加到 predeq
    //   (If n=s->min, we can append s to predeq)
    // - 如果 immsucc 存在，我们可以将 s 添加到 immsucc 的开头
    //   (If immsucc exists, we can prepend s to immsucc)
    // - 可能还需要连接 predeq、s 和 immsucc
    //   (And possibly join predeq, s, and immsucc)
    struct ddsi_rsample *predeq, *immsucc;
    TRACE(reorder, "  hard case ...\n");

    if (reorder->late_ack_mode && delivery_queue_full_p) {
      TRACE(reorder, "  discarding sample: delivery queue full\n");
      reorder->discarded_bytes += s->sc.first->sampleinfo->size;
      return DDSI_REORDER_REJECT;
    }

    // 查找 predeq
    // (Find predeq)
    predeq =
        ddsrt_avl_lookup_pred_eq(&reorder_sampleivtree_treedef, &reorder->sampleivtree, &s->min);
    if (predeq)
      TRACE(reorder, "  predeq = [%" PRIu64 ",%" PRIu64 ") @ %p\n", predeq->u.reorder.min,
            predeq->u.reorder.maxp1, (void *)predeq);
    else
      TRACE(reorder, "  predeq = null\n");
    if (predeq && s->min >= predeq->u.reorder.min && s->min < predeq->u.reorder.maxp1) {
      // 包含在 predeq 中
      // (Contained in predeq)
      TRACE(reorder, "  discard: contained in predeq\n");
      reorder->discarded_bytes += s->sc.first->sampleinfo->size;
      return DDSI_REORDER_REJECT;
    }

    // 查找 immsucc
    // (Find immsucc)
    immsucc = ddsrt_avl_lookup(&reorder_sampleivtree_treedef, &reorder->sampleivtree, &s->maxp1);
    if (immsucc)
      TRACE(reorder, "  immsucc = [%" PRIu64 ",%" PRIu64 ") @ %p\n", immsucc->u.reorder.min,
            immsucc->u.reorder.maxp1, (void *)immsucc);
    else
      TRACE(reorder, "  immsucc = null\n");
    if (predeq && s->min == predeq->u.reorder.maxp1) {
      // 在末尾增加 predeq，可能还需要添加 immsucc
      // (Grow predeq at end, and maybe append immsucc as well)
      TRACE(reorder, "  growing predeq at end ...\n");
      append_rsample_interval(predeq, rsampleiv);
      if (reorder_try_append_and_discard(reorder, predeq, immsucc)) reorder->max_sampleiv = predeq;
    } else if (immsucc) {
      // 没有前驱，从头部增加 immsucc，这会改变树中节点的键，但不会改变树的结构。
      // (No predecessor, grow immsucc at head, which _does_ alter the key of the node in the tree,
      // but _doesn't_ change the tree's structure.)
      TRACE(reorder, "  growing immsucc at head\n");
      s->sc.last->next = immsucc->u.reorder.sc.first;
      immsucc->u.reorder.sc.first = s->sc.first;
      immsucc->u.reorder.min = s->min;
      immsucc->u.reorder.n_samples += s->n_samples;

      // delete_last_sample 可能最终决定删除 immsucc 中包含的最后一个样本，而不检查 immsucc
      // 是否依赖于该样本。 这反过来会导致 sampleivtree 指向已释放的内存（可能是 free()
      // 释放的，也可能是可重用的，因此可能会导致间接地破坏区间树）。
      //
      // 我们知道 rsampleiv 将保持活动状态，它不依赖于最后一个样本（因为我们在 immsucc
      // 的头部进行扩展），并且我们不再需要它。 因此，我们可以将 rsampleiv 交换为
      // immsucc，避免上述情况。
      //
      // (delete_last_sample may eventually decide to delete the last sample contained in immsucc
      // without checking whether immsucc were allocated dependent on that sample. That in turn
      // would cause sampleivtree to point to freed memory (either freed as in free(), or freed as
      // in available for reuse, and hence the result may be a silent corruption of the interval
      // tree).
      //
      // We do know that rsampleiv will remain live, that it is not dependent on the last sample
      // (because we're growing immsucc at the head), and that we don't otherwise need it anymore.
      // Therefore, we can swap rsampleiv in for immsucc and avoid the case above.)
      rsampleiv->u.reorder = immsucc->u.reorder;
      ddsrt_avl_swap_node(&reorder_sampleivtree_treedef, &reorder->sampleivtree, immsucc,
                          rsampleiv);
      if (immsucc == reorder->max_sampleiv) reorder->max_sampleiv = rsampleiv;
    } else {
      // 既不扩展 predeq 也不扩展 immsucc
      // (Neither extends predeq nor immsucc)
      TRACE(reorder, "  new interval\n");
      reorder_add_rsampleiv(reorder, rsampleiv);
    }

    /* 不允许 radmin 超过 max_samples；现在我们已经插入了它（并且可能已经使 radmin
       超过了其最大大小）， 我们不再冒着在删除最后一个样本时删除新样本所属的间隔的风险。*/
    /* Do not let radmin grow beyond max_samples; now that we've
       inserted it (and possibly have grown the radmin beyond its max
       size), we no longer risk deleting the interval that the new
       sample belongs to when deleting the last sample. */
    if (reorder->n_samples < reorder->max_samples)
      reorder->n_samples++;
    else {
      delete_last_sample(reorder);
    }

    // 增加样本引用计数调整值
    // Increase the sample reference count adjustment value
    (*refcount_adjust)++;

    // 返回重新排序操作的结果
    // Return the result of the reorder operation
    return DDSI_REORDER_ACCEPT;
  }
}

/**
 * @brief 合并与给定范围相邻的间隔 (Coalesce intervals touching a given range)
 *
 * @param[in] reorder 重排序结构指针 (Pointer to the reorder structure)
 * @param[in] min 序列号最小值 (Minimum sequence number)
 * @param[in] maxp1 序列号最大值加1 (Maximum sequence number plus 1)
 * @param[out] valuable 指示是否有价值的指针 (Pointer indicating whether it is valuable)
 * @return 返回合并后的 ddsi_rsample 结构指针 (Returns a pointer to the merged ddsi_rsample
 * structure)
 */
static struct ddsi_rsample *coalesce_intervals_touching_range(struct ddsi_reorder *reorder,
                                                              ddsi_seqno_t min,
                                                              ddsi_seqno_t maxp1,
                                                              int *valuable) {
  struct ddsi_rsample *s, *t;
  *valuable = 0;

  // 查找第一个满足 n >= min && m <= maxp1 的区间 [m,n) (Find first (lowest m) interval [m,n) s.t. n
  // >= min && m <= maxp1)
  s = ddsrt_avl_lookup_pred_eq(&reorder_sampleivtree_treedef, &reorder->sampleivtree, &min);
  if (s && s->u.reorder.maxp1 >= min) {
    // m <= min && n >= min (注意：s 的前驱 [m',n') 必然有 n' < m) (m <= min && n >= min (note: pred
    // of s [m',n') necessarily has n' < m))

#ifndef NDEBUG
    struct ddsi_rsample *q =
        ddsrt_avl_find_pred(&reorder_sampleivtree_treedef, &reorder->sampleivtree, s);
    assert(q == NULL || q->u.reorder.maxp1 < min);
#endif
  } else {
    // 没有找到，但第一个（如果 s = NULL）或下一个（如果 s != NULL）仍可能有 m <= maxp1 (m > min
    // 是隐含的)。如果没有这样的区间，则返回 NULL。 (No good, but the first (if s = NULL) or the
    // next one (if s != NULL) may still have m <= maxp1 (m > min is implied now).  If not, no such
    // interval.)
    s = ddsrt_avl_find_succ(&reorder_sampleivtree_treedef, &reorder->sampleivtree, s);
    if (!(s && s->u.reorder.min <= maxp1)) return NULL;
  }

  // 将后继 [m',n') 添加到 s 中，使得 m' <= maxp1 (Append successors [m',n') s.t. m' <= maxp1 to s)
  assert(s->u.reorder.min + s->u.reorder.n_samples <= s->u.reorder.maxp1);
  while ((t = ddsrt_avl_find_succ(&reorder_sampleivtree_treedef, &reorder->sampleivtree, s)) !=
             NULL &&
         t->u.reorder.min <= maxp1) {
    ddsrt_avl_delete(&reorder_sampleivtree_treedef, &reorder->sampleivtree, t);
    assert(t->u.reorder.min + t->u.reorder.n_samples <= t->u.reorder.maxp1);
    append_rsample_interval(s, t);
    *valuable = 1;
  }

  // 如果需要，将范围扩展到 [min,maxp1) (If needed, grow range to [min,maxp1))
  if (min < s->u.reorder.min) {
    *valuable = 1;
    s->u.reorder.min = min;
  }
  if (maxp1 > s->u.reorder.maxp1) {
    *valuable = 1;
    s->u.reorder.maxp1 = maxp1;
  }
  return s;
}

/**
 * @brief 创建一个新的 ddsi_rdata 结构，表示数据中的间隙 (Create a new ddsi_rdata structure
 * representing a gap in the data)
 *
 * @param[in] rmsg 消息指针 (Pointer to the message)
 * @return 返回新创建的 ddsi_rdata 结构指针 (Returns a pointer to the newly created ddsi_rdata
 * structure)
 */
struct ddsi_rdata *ddsi_rdata_newgap(struct ddsi_rmsg *rmsg) {
  struct ddsi_rdata *d;

  // 创建一个新的 ddsi_rdata 结构 (Create a new ddsi_rdata structure)
  if ((d = ddsi_rdata_new(rmsg, 0, 0, 0, 0, 0)) == NULL) return NULL;

  // 为新创建的 ddsi_rdata 结构添加偏置 (Add bias to the newly created ddsi_rdata structure)
  ddsi_rdata_addbias(d);

  return d;
}

/**
 * @brief 将缺失的序列号范围插入重排序结构中 (Insert a range of missing sequence numbers into the
 * reorder structure)
 *
 * @param[in] reorder 重排序结构指针 (Pointer to the reorder structure)
 * @param[in] rdata 接收到的数据指针 (Pointer to the received data)
 * @param[in] min 缺失序列号范围的最小值 (Minimum value in the range of missing sequence numbers)
 * @param[in] maxp1 缺失序列号范围的最大值加1 (Maximum value in the range of missing sequence
 * numbers plus 1)
 * @return 成功时返回1，失败时返回0 (Returns 1 on success, 0 on failure)
 */
static int reorder_insert_gap(struct ddsi_reorder *reorder,
                              struct ddsi_rdata *rdata,
                              ddsi_seqno_t min,
                              ddsi_seqno_t maxp1) {
  // 定义一个接收样本链表元素指针 (Define a pointer to a received sample chain element)
  struct ddsi_rsample_chain_elem *sce;
  // 定义一个接收样本指针 (Define a pointer to a received sample)
  struct ddsi_rsample *s;
  // 定义一个AVL树查找路径变量 (Define an AVL tree lookup path variable)
  ddsrt_avl_ipath_t path;

  // 检查序列号是否已经在重排序结构中 (Check if the sequence number is already in the reorder
  // structure)
  if (ddsrt_avl_lookup_ipath(&reorder_sampleivtree_treedef, &reorder->sampleivtree, &min, &path) !=
      NULL)
    assert(0);

  // 为接收样本链表元素分配内存 (Allocate memory for the received sample chain element)
  if ((sce = ddsi_rmsg_alloc(rdata->rmsg, sizeof(*sce))) == NULL) return 0;
  sce->fragchain = rdata;  // 设置接收数据指针 (Set the pointer to the received data)
  sce->next = NULL;  // 初始化下一个链表元素指针 (Initialize the pointer to the next chain element)
  sce->sampleinfo = NULL;  // 初始化样本信息指针 (Initialize the sample info pointer)

  // 为接收样本分配内存 (Allocate memory for the received sample)
  if ((s = ddsi_rmsg_alloc(rdata->rmsg, sizeof(*s))) == NULL) return 0;
  s->u.reorder.sc.first = s->u.reorder.sc.last =
      sce;  // 设置接收样本链表的首尾元素 (Set the first and last elements of the received sample
            // chain)
  s->u.reorder.min = min;  // 设置缺失序列号范围的最小值 (Set the minimum value in the range of
                           // missing sequence numbers)
  s->u.reorder.maxp1 = maxp1;  // 设置缺失序列号范围的最大值加1 (Set the maximum value in the range
                               // of missing sequence numbers plus 1)
  s->u.reorder.n_samples = 1;  // 设置接收样本数量 (Set the number of received samples)

  // 将接收样本插入重排序结构的AVL树中 (Insert the received sample into the AVL tree of the reorder
  // structure)
  ddsrt_avl_insert_ipath(&reorder_sampleivtree_treedef, &reorder->sampleivtree, s, &path);

  return 1;
}

/**
 * @brief 处理数据重排序中的间隙（gap），用于处理缺失的序列号范围。
 *        Handle gaps in data reordering, used for dealing with missing sequence number ranges.
 *
 * @param[in] sc 指向 ddsi_rsample_chain 结构的指针，用于存储被接受的样本。
 *              Pointer to a ddsi_rsample_chain structure, used to store accepted samples.
 * @param[in] reorder 指向 ddsi_reorder 结构的指针，用于管理重排序操作。
 *                   Pointer to a ddsi_reorder structure, used for managing reordering operations.
 * @param[in] rdata 指向 ddsi_rdata 结构的指针，用于存储接收到的数据。
 *                 Pointer to a ddsi_rdata structure, used for storing received data.
 * @param[in] min 序列号范围的最小值。
 *               The minimum value of the sequence number range.
 * @param[in] maxp1 序列号范围的最大值加一。
 *                 The maximum value of the sequence number range plus one.
 * @param[out] refcount_adjust 用于调整引用计数的指针。
 *                             Pointer for adjusting reference count.
 *
 * @return 返回 ddsi_reorder_result_t 类型的结果，表示处理间隙的状态。
 *         Returns a ddsi_reorder_result_t type result, indicating the status of handling the gap.
 */
ddsi_reorder_result_t ddsi_reorder_gap(struct ddsi_rsample_chain *sc,
                                       struct ddsi_reorder *reorder,
                                       struct ddsi_rdata *rdata,
                                       ddsi_seqno_t min,
                                       ddsi_seqno_t maxp1,
                                       int *refcount_adjust) {
  /* 所有在 [min,maxp1) 范围内的序列号都不可用，因此该范围内的任何片段都必须被丢弃。
   * 用于处理 Hearbeats（通过设置 min=1）和 Gaps。
   *
   * All sequence numbers in [min,maxp1) are unavailable so any
   * fragments in that range must be discarded.  Used both for
   * Hearbeats (by setting min=1) and for Gaps.
   *
   * 情况 I：maxp1 <= next_seq。没有任何影响。
   * Case I: maxp1 <= next_seq.  No effect whatsoever.
   *
   * 否则：
   * Otherwise:
   *
   * 情况 II：min <= next_seq。所有序列号小于 maxp1 的样本以及其后连续的样本都将返回，
   *         并将 next_seq 更新为 max(maxp1, 最高返回序列号+1)。
   * Case II: min <= next_seq.  All samples we have with sequence
   *         numbers less than maxp1 plus those following it consecutively
   *         are returned, and next_seq is updated to max(maxp1, highest
   *         returned sequence number+1)
   *
   * 否则：
   * Else:
   *
   * 情况 III：导致与[min,maxp1)重叠或连续的间隔合并，可能将间隔扩展到下限的 min 或上限的 maxp1，
   *          或者如果没有这样的间隔，则创建一个没有任何样本的[min,maxp1)间隔。
   * Case III: Causes coalescing of intervals overlapping with
   *         [min,maxp1) or consecutive to it, possibly extending
   *         intervals to min on the lower bound or maxp1 on the upper
   *         one, or if there are no such intervals, the creation of a
   *         [min,maxp1) interval without any samples.
   *
   * 注意：如果间隙导致数据被传递，则不能存储任何内容（即修改 rdata、refcount_adjust）：
   *      如果所有可靠代理写入器的读取器都不可靠，则依赖于它的 out-of-order 传递的替代路径。
   * NOTE: must not store anything (i.e. modify rdata,
   * refcount_adjust) if gap causes data to be delivered: altnerative
   * path for out-of-order delivery if all readers of a reliable
   * proxy-writer are unrelibale depends on it. */
  struct ddsi_rsample *coalesced;
  int valuable;

  // 记录重排序操作的跟踪信息
  // Trace information of the reordering operation
  TRACE(reorder, "reorder_gap(%p %c, [%" PRIu64 ",%" PRIu64 ") data %p) expecting %" PRIu64 ":\n",
        (void *)reorder, reorder_mode_as_char(reorder), min, maxp1, (void *)rdata,
        reorder->next_seq);

  // 情况 I：maxp1 <= next_seq
  // Case I: maxp1 <= next_seq
  if (maxp1 <= reorder->next_seq) {
    TRACE(reorder, "  too old\n");
    return DDSI_REORDER_TOO_OLD;
  }
  // 如果重排序模式不是正常模式，则不处理间隙
  // If the reordering mode is not normal, don't handle the gap
  if (reorder->mode != DDSI_REORDER_MODE_NORMAL) {
    TRACE(reorder, "  special mode => don't care\n");
    return DDSI_REORDER_REJECT;
  }

  /* 合并所有与 [min,maxp1) 重叠或连续的间隔 [m,n)，其中 n >= min 或 m <= maxp1 */
  /* Coalesce all intervals [m,n) with n >= min or m <= maxp1 */
  if ((coalesced = coalesce_intervals_touching_range(reorder, min, maxp1, &valuable)) == NULL) {
    ddsi_reorder_result_t res;
    TRACE(reorder, "  coalesced = null\n");
    if (min <= reorder->next_seq) {
      TRACE(reorder, "  next expected: %" PRIu64 "\n", maxp1);
      reorder->next_seq = maxp1;
      res = DDSI_REORDER_ACCEPT;
    } else if (reorder->n_samples == reorder->max_samples &&
               (reorder->max_sampleiv == NULL || min > reorder->max_sampleiv->u.reorder.maxp1)) {
      /* n_samples = max_samples => (max_sampleiv = NULL <=> max_samples = 0) */
      TRACE(reorder, "  discarding gap: max_samples reached and gap at end\n");
      res = DDSI_REORDER_REJECT;
    } else if (!reorder_insert_gap(reorder, rdata, min, maxp1)) {
      TRACE(reorder, "  store gap failed: no memory\n");
      res = DDSI_REORDER_REJECT;
    } else {
      TRACE(reorder, "  storing gap\n");
      res = DDSI_REORDER_ACCEPT;
      /* do not let radmin grow beyond max_samples; there is a small
         possibility that we insert it & delete it immediately
         afterward. */
      if (reorder->n_samples < reorder->max_samples)
        reorder->n_samples++;
      else
        delete_last_sample(reorder);
      (*refcount_adjust)++;
    }
    reorder->max_sampleiv =
        ddsrt_avl_find_max(&reorder_sampleivtree_treedef, &reorder->sampleivtree);
    return res;
  } else if (coalesced->u.reorder.min <= reorder->next_seq) {
    // 打印合并后的数据包信息
    // Print the information of the merged data packet
    TRACE(reorder, "  coalesced = [%" PRIu64 ",%" PRIu64 ") @ %p containing %" PRId32 " samples\n",
          coalesced->u.reorder.min, coalesced->u.reorder.maxp1, (void *)coalesced,
          coalesced->u.reorder.n_samples);
    // 从重排序树中删除合并后的数据包节点
    // Remove the merged data packet node from the reordering tree
    ddsrt_avl_delete(&reorder_sampleivtree_treedef, &reorder->sampleivtree, coalesced);
    if (coalesced->u.reorder.min <= reorder->next_seq) assert(min <= reorder->next_seq);
    // 更新下一个期望的序列号
    // Update the next expected sequence number
    reorder->next_seq = coalesced->u.reorder.maxp1;
    // 查找重排序树中的最大节点
    // Find the maximum node in the reordering tree
    reorder->max_sampleiv =
        ddsrt_avl_find_max(&reorder_sampleivtree_treedef, &reorder->sampleivtree);
    // 打印下一个期望的序列号
    // Print the next expected sequence number
    TRACE(reorder, "  next expected: %" PRIu64 "\n", reorder->next_seq);
    // 更新序列号更改
    // Update the sequence number change
    *sc = coalesced->u.reorder.sc;

    // 调整 n_samples
    // Adjust n_samples
    assert(coalesced->u.reorder.min + coalesced->u.reorder.n_samples <= coalesced->u.reorder.maxp1);
    assert(reorder->n_samples >= coalesced->u.reorder.n_samples);
    reorder->n_samples -= coalesced->u.reorder.n_samples;
    return (ddsi_reorder_result_t)coalesced->u.reorder.n_samples;
  } else {
    // 打印合并后的数据包信息
    // Print the information of the merged data packet
    TRACE(reorder, "  coalesced = [%" PRIu64 ",%" PRIu64 ") @ %p - that is all\n",
          coalesced->u.reorder.min, coalesced->u.reorder.maxp1, (void *)coalesced);
    // 查找重排序树中的最大节点
    // Find the maximum node in the reordering tree
    reorder->max_sampleiv =
        ddsrt_avl_find_max(&reorder_sampleivtree_treedef, &reorder->sampleivtree);
    // 返回处理结果，根据 valuable 变量判断是接受还是拒绝
    // Return the processing result, determine whether to accept or reject based on the valuable
    // variable
    return valuable ? DDSI_REORDER_ACCEPT : DDSI_REORDER_REJECT;
  }
}

/**
 * @brief 丢弃指定序列号之前的数据样本 (Drop data samples up to a specified sequence number)
 *
 * @param[in] reorder 指向 ddsi_reorder 结构体的指针 (Pointer to the ddsi_reorder structure)
 * @param[in] maxp1 要丢弃的最大序列号加 1 (The maximum sequence number to drop plus 1)
 */
void ddsi_reorder_drop_upto(struct ddsi_reorder *reorder, ddsi_seqno_t maxp1) {
  // ddsi_reorder_gap returns the chain of available samples starting with the first
  // sequence number in the gap interval and ending at the highest sequence number
  // >= maxp1 for which all sequence numbers starting from maxp1 are present.
  // Requiring that no samples are present beyond maxp1 means we're not dropping
  // too much.  That's good enough for the current purpose.
  // ddsi_reorder_gap 返回一个链表，该链表包含从间隔区间的第一个序列号开始，
  // 到所有序列号都存在且 >= maxp1 的最高序列号结束的可用样本。
  // 要求 maxp1 之后没有样本意味着我们不会丢弃太多。对于当前目的来说，这已经足够好了。
  assert(reorder->max_sampleiv == NULL || reorder->max_sampleiv->u.reorder.maxp1 <= maxp1);
  // gap won't be stored, so can safely be stack-allocated for the purpose of calling
  // ddsi_reorder_gap
  // gap 不会被存储，因此可以安全地在栈上分配，以便调用 ddsi_reorder_gap
  struct ddsi_rdata gap = {.rmsg = NULL,
                           .nextfrag = NULL,
                           .min = 0,
                           .maxp1 = 0,
                           .submsg_zoff = 0,
                           .payload_zoff = 0
#ifndef NDEBUG
                           ,
                           .refcount_bias_added = DDSRT_ATOMIC_UINT32_INIT(0)
#endif
  };
  struct ddsi_rsample_chain sc;
  int refc_adjust = 0;
  if (ddsi_reorder_gap(&sc, reorder, &gap, 1, maxp1, &refc_adjust) > 0) {
    while (sc.first) {
      struct ddsi_rsample_chain_elem *e = sc.first;
      sc.first = e->next;
      ddsi_fragchain_unref(e->fragchain);
    }
  }
  assert(refc_adjust == 0 && !ddsrt_atomic_ld32(&gap.refcount_bias_added));
  assert(ddsi_reorder_next_seq(reorder) >= maxp1);
}

/**
 * @brief 判断是否需要指定序列号的样本 (Determine whether a sample with a specified sequence number
 * is needed)
 *
 * @param[in] reorder 指向 ddsi_reorder 结构体的指针 (Pointer to the ddsi_reorder structure)
 * @param[in] seq 要检查的序列号 (The sequence number to check)
 * @return 如果需要该序列号的样本，则返回 1，否则返回 0 (Returns 1 if the sample with the sequence
 * number is needed, otherwise returns 0)
 */
int ddsi_reorder_wantsample(const struct ddsi_reorder *reorder, ddsi_seqno_t seq) {
  struct ddsi_rsample *s;
  if (seq < reorder->next_seq) /* trivially not interesting */
    return 0;
  // 如果我们知道序列号 seq，则查找包含 seq 的区间。如果 seq
  // 在此区间之外（如果有的话），则我们感兴趣。
  s = ddsrt_avl_lookup_pred_eq(&reorder_sampleivtree_treedef, &reorder->sampleivtree, &seq);
  return (s == NULL || s->u.reorder.maxp1 <= seq);
}

/**
 * @brief 生成NACK（未确认）位图，用于请求丢失的数据样本。
 * Generate a NACK (not acknowledged) bitmap for requesting missing data samples.
 *
 * @param[in] reorder 排序结构体指针。Pointer to the reorder structure.
 * @param[in] base 基础序列号。Base sequence number.
 * @param[in] maxseq 已知存在的最大序列号。Maximum known sequence number.
 * @param[out] map 序列号集合头部。Sequence number set header.
 * @param[out] mapbits 位图数组。Bitmap array.
 * @param[in] maxsz 最大位图长度。Maximum bitmap length.
 * @param[in] notail 是否不包含尾部。Whether to exclude tail.
 * @return 返回位图中有效位的数量。Returns the number of valid bits in the bitmap.
 */
unsigned ddsi_reorder_nackmap(const struct ddsi_reorder *reorder,
                              ddsi_seqno_t base,
                              ddsi_seqno_t maxseq,
                              struct ddsi_sequence_number_set_header *map,
                              uint32_t *mapbits,
                              uint32_t maxsz,
                              int notail) {
  // 确保maxsz在允许的范围内。Ensure maxsz is within the allowed range.
  assert(maxsz <= 256);

  // 如果maxsz大于reorder->max_samples，则将其设置为reorder->max_samples。
  // Set maxsz to reorder->max_samples if it's greater than reorder->max_samples.
  if (maxsz > reorder->max_samples) maxsz = reorder->max_samples;

#if 0
  // 这是以前的实现方式，其中reorder buffer与delivery一起使用。
  // This is what it used to be, where the reorder buffer is with delivery.
  base = reorder->next_seq;
#else
  if (base > reorder->next_seq) {
    DDS_CERROR(reorder->logcfg,
               "ddsi_reorder_nackmap: incorrect base sequence number supplied (%" PRIu64
               " > %" PRIu64 ")\n",
               base, reorder->next_seq);
    base = reorder->next_seq;
  }
#endif
  if (maxseq + 1 < base) {
    DDS_CERROR(reorder->logcfg,
               "ddsi_reorder_nackmap: incorrect max sequence number supplied (maxseq %" PRIu64
               " base %" PRIu64 ")\n",
               maxseq, base);
    maxseq = base - 1;
  }

  map->bitmap_base = ddsi_to_seqno(base);
  if (maxseq + 1 - base > maxsz)
    map->numbits = maxsz;
  else
    map->numbits = (uint32_t)(maxseq + 1 - base);
  ddsi_bitset_zero(map->numbits, mapbits);

  struct ddsi_rsample *iv =
      ddsrt_avl_find_min(&reorder_sampleivtree_treedef, &reorder->sampleivtree);
  assert(iv == NULL || iv->u.reorder.min > base);
  ddsi_seqno_t i = base;
  while (iv && i < base + map->numbits) {
    for (; i < base + map->numbits && i < iv->u.reorder.min; i++) {
      uint32_t x = (uint32_t)(i - base);
      ddsi_bitset_set(map->numbits, mapbits, x);
    }
    i = iv->u.reorder.maxp1;
    iv = ddsrt_avl_find_succ(&reorder_sampleivtree_treedef, &reorder->sampleivtree, iv);
  }
  if (notail && i < base + map->numbits)
    map->numbits = (uint32_t)(i - base);
  else {
    for (; i < base + map->numbits; i++) {
      uint32_t x = (uint32_t)(i - base);
      ddsi_bitset_set(map->numbits, mapbits, x);
    }
  }
  return map->numbits;
}

/**
 * @brief 获取下一个序列号。
 * Get the next sequence number.
 *
 * @param[in] reorder 排序结构体指针。Pointer to the reorder structure.
 * @return 返回下一个序列号。Returns the next sequence number.
 */
ddsi_seqno_t ddsi_reorder_next_seq(const struct ddsi_reorder *reorder) { return reorder->next_seq; }

/**
 * @brief 设置下一个序列号。
 * Set the next sequence number.
 *
 * @param[out] reorder 排序结构体指针。Pointer to the reorder structure.
 * @param[in] seq 要设置的序列号。Sequence number to set.
 */
void ddsi_reorder_set_next_seq(struct ddsi_reorder *reorder, ddsi_seqno_t seq) {
  reorder->next_seq = seq;
}

/* DQUEUE -------------------------------------------------------------- */

/**
 * @struct ddsi_dqueue
 * @brief 数据队列结构体 (Data queue structure)
 *
 * 用于存储和处理数据的队列结构体。 (A queue structure for storing and processing data.)
 */
struct ddsi_dqueue {
  ddsrt_mutex_t lock; /**< 互斥锁，用于保护队列 (Mutex lock for protecting the queue) */
  ddsrt_cond_t cond; /**< 条件变量，用于通知线程 (Condition variable for notifying threads) */
  ddsi_dqueue_handler_t handler;     /**< 队列处理函数 (Queue handler function) */
  void *handler_arg;                 /**< 处理函数的参数 (Handler function argument) */

  struct ddsi_rsample_chain sc;      /**< 接收到的样本链表 (Received sample chain) */

  struct ddsi_thread_state *thrst;   /**< 线程状态指针 (Thread state pointer) */
  struct ddsi_domaingv *gv;          /**< 域全局变量指针 (Domain global variables pointer) */
  char *name;                        /**< 队列名称 (Queue name) */
  uint32_t max_samples;              /**< 最大样本数 (Maximum number of samples) */
  ddsrt_atomic_uint32_t nof_samples; /**< 当前样本数 (Current number of samples) */
};

/**
 * @enum dqueue_elem_kind
 * @brief 队列元素类型 (Queue element types)
 */
enum dqueue_elem_kind {
  DQEK_DATA,  /**< 数据类型 (Data type) */
  DQEK_GAP,   /**< 间隔类型 (Gap type) */
  DQEK_BUBBLE /**< 泡沫类型 (Bubble type) */
};

/**
 * @enum ddsi_dqueue_bubble_kind
 * @brief 队列泡沫类型 (Queue bubble types)
 */
enum ddsi_dqueue_bubble_kind {
  DDSI_DQBK_STOP, /**< 停止类型，不使用ddsrt_malloc()分配内存 (_not_ ddsrt_malloc()ed!) */
  DDSI_DQBK_CALLBACK, /**< 回调类型 (Callback type) */
  DDSI_DQBK_RDGUID    /**< 读者GUID类型 (Reader GUID type) */
};

/**
 * @struct ddsi_dqueue_bubble
 * @brief 队列泡沫结构体 (Queue bubble structure)
 *
 * 用于处理队列中的特殊事件。 (For handling special events in the queue.)
 */
struct ddsi_dqueue_bubble {
  /* sample_chain_elem must be first: and is used to link it into the
     queue, with the sampleinfo pointing to itself, but mangled */
  struct ddsi_rsample_chain_elem sce; /**< 样本链表元素 (Sample chain element) */

  enum ddsi_dqueue_bubble_kind kind;  /**< 泡沫类型 (Bubble kind) */
  union {
    /* stop */
    struct {
      ddsi_dqueue_callback_t cb; /**< 回调函数 (Callback function) */
      void *arg;                 /**< 回调函数参数 (Callback function argument) */
    } cb;
    struct {
      ddsi_guid_t rdguid; /**< 读者GUID (Reader GUID) */
      uint32_t count;     /**< 计数器 (Counter) */
    } rdguid;
  } u;
};

/**
 * @brief 获取队列元素类型 (Get the queue element kind)
 *
 * @param[in] e 队列元素指针 (Queue element pointer)
 * @return 队列元素类型 (Queue element kind)
 */
static enum dqueue_elem_kind dqueue_elem_kind(const struct ddsi_rsample_chain_elem *e) {
  if (e->sampleinfo == NULL)
    return DQEK_GAP;    /**< 间隔类型 (Gap type) */
  else if ((char *)e->sampleinfo != (char *)e)
    return DQEK_DATA;   /**< 数据类型 (Data type) */
  else
    return DQEK_BUBBLE; /**< 泡沫类型 (Bubble type) */
}

/**
 * @brief 处理 dqueue 中的线程，处理数据、间隔和气泡事件。
 *        Process the thread in dqueue, handling data, gap and bubble events.
 *
 * @param[in] q 指向 ddsi_dqueue 结构体的指针。
 *             Pointer to the ddsi_dqueue structure.
 * @return uint32_t 返回 0。
 *                 Return 0.
 */
static uint32_t dqueue_thread(struct ddsi_dqueue *q) {
  // 查找当前线程状态
  // Lookup the current thread state
  struct ddsi_thread_state *const thrst = ddsi_lookup_thread_state();

#if DDSRT_HAVE_RUSAGE
  // 获取域全局变量
  // Get domain global variable
  struct ddsi_domaingv const *const gv = ddsrt_atomic_ldvoidp(&thrst->gv);
#endif

  // 初始化下一个线程 CPU 时间
  // Initialize next thread CPU time
  ddsrt_mtime_t next_thread_cputime = {0};

  // 控制循环是否继续进行的标志
  // Flag controlling whether the loop continues
  int keepgoing = 1;

  // 初始化读者 GUID 和相关指针及计数器
  // Initialize reader GUID, related pointer and counter
  ddsi_guid_t rdguid, *prdguid = NULL;
  uint32_t rdguid_count = 0;

  // 锁定 dqueue 的互斥锁
  // Lock the mutex of dqueue
  ddsrt_mutex_lock(&q->lock);

  // 当 keepgoing 为真时，继续循环
  // Continue looping while keepgoing is true
  while (keepgoing) {
    // 初始化接收样本链结构体
    // Initialize received sample chain structure
    struct ddsi_rsample_chain sc;

    // 记录线程 CPU 时间
    // Log thread CPU time
    LOG_THREAD_CPUTIME(&gv->logconfig, next_thread_cputime);

    // 如果 q->sc.first 为空，等待条件变量
    // If q->sc.first is NULL, wait for the condition variable
    if (q->sc.first == NULL) ddsrt_cond_wait(&q->cond, &q->lock);

    // 将 q->sc 赋值给 sc，并清空 q->sc
    // Assign q->sc to sc and clear q->sc
    sc = q->sc;
    q->sc.first = q->sc.last = NULL;

    // 解锁 dqueue 的互斥锁
    // Unlock the mutex of dqueue
    ddsrt_mutex_unlock(&q->lock);

    // 唤醒固定域的线程状态
    // Wake up the thread state of fixed domain
    ddsi_thread_state_awake_fixed_domain(thrst);

    // 当 sc.first 不为空时，继续循环
    // Continue looping while sc.first is not NULL
    while (sc.first) {
      // 获取接收样本链元素并更新 sc.first
      // Get received sample chain element and update sc.first
      struct ddsi_rsample_chain_elem *e = sc.first;
      int ret;
      sc.first = e->next;

      // 减少 nof_samples 计数器并检查是否为 1
      // Decrease nof_samples counter and check if it's 1
      if (ddsrt_atomic_dec32_ov(&q->nof_samples) == 1) {
        // 广播条件变量
        // Broadcast the condition variable
        ddsrt_cond_broadcast(&q->cond);
      }

      // 唤醒线程状态并避免嵌套
      // Wake up thread state and avoid nesting
      ddsi_thread_state_awake_to_awake_no_nest(thrst);

      // 根据元素类型进行处理
      // Process according to element type
      switch (dqueue_elem_kind(e)) {
        case DQEK_DATA:
          // 调用处理函数并检查返回值
          // Call handler function and check return value
          ret = q->handler(e->sampleinfo, e->fragchain, prdguid, q->handler_arg);
          (void)ret;        /* eliminate set-but-not-used in NDEBUG case */
          assert(ret == 0); /* so every handler will return 0 */

          // 继续处理下一个元素
          // Continue processing the next element
          /* FALLS THROUGH */

        case DQEK_GAP:
          // 取消引用 fragchain
          // Unreference fragchain
          ddsi_fragchain_unref(e->fragchain);

          // 如果 rdguid_count 大于 0，递减计数器并更新 prdguid
          // If rdguid_count is greater than 0, decrement counter and update prdguid
          if (rdguid_count > 0) {
            if (--rdguid_count == 0) prdguid = NULL;
          }
          break;

        case DQEK_BUBBLE: {
          // 获取气泡事件结构体
          // Get bubble event structure
          struct ddsi_dqueue_bubble *b = (struct ddsi_dqueue_bubble *)e->sampleinfo;

          // 处理不同类型的气泡事件
          // Handle different types of bubble events
          if (b->kind == DDSI_DQBK_STOP) {
            // 设置 keepgoing 为 0，停止循环
            // Set keepgoing to 0, stopping the loop
            keepgoing = 0;
          } else {
            switch (b->kind) {
              case DDSI_DQBK_STOP:
                abort();
              case DDSI_DQBK_CALLBACK:
                // 调用回调函数
                // Call callback function
                b->u.cb.cb(b->u.cb.arg);
                break;
              case DDSI_DQBK_RDGUID:
                // 更新 rdguid 和计数器
                // Update rdguid and counter
                rdguid = b->u.rdguid.rdguid;
                rdguid_count = b->u.rdguid.count;
                prdguid = &rdguid;
                break;
            }
            // 释放气泡事件结构体内存
            // Free memory of bubble event structure
            ddsrt_free(b);
          }
          break;
        }
      }
    }

    // 将线程状态设置为休眠
    // Set thread state to asleep
    ddsi_thread_state_asleep(thrst);

    // 锁定 dqueue 的互斥锁
    // Lock the mutex of dqueue
    ddsrt_mutex_lock(&q->lock);
  }

  // 解锁 dqueue 的互斥锁
  // Unlock the mutex of dqueue
  ddsrt_mutex_unlock(&q->lock);

  // 返回 0
  // Return 0
  return 0;
}

/**
 * @brief 创建一个新的 ddsi_dqueue 结构体实例
 *        Create a new instance of the ddsi_dqueue structure
 *
 * @param[in] name 队列名称 Queue name
 * @param[in] gv 指向 ddsi_domaingv 结构体的指针 Pointer to the ddsi_domaingv structure
 * @param[in] max_samples 队列中最大样本数 Maximum number of samples in the queue
 * @param[in] handler 处理函数 Handler function
 * @param[in] arg 传递给处理函数的参数 Argument passed to the handler function
 * @return 成功时返回指向新创建的 ddsi_dqueue 结构体的指针，失败时返回 NULL
 *         On success, returns a pointer to the newly created ddsi_dqueue structure; on failure,
 * returns NULL
 */
struct ddsi_dqueue *ddsi_dqueue_new(const char *name,
                                    const struct ddsi_domaingv *gv,
                                    uint32_t max_samples,
                                    ddsi_dqueue_handler_t handler,
                                    void *arg) {
  struct ddsi_dqueue *q;

  // 分配内存并检查是否成功 Allocate memory and check for success
  if ((q = ddsrt_malloc(sizeof(*q))) == NULL) goto fail_q;
  // 复制名称并检查是否成功 Copy name and check for success
  if ((q->name = ddsrt_strdup(name)) == NULL) goto fail_name;
  // 设置最大样本数 Set maximum number of samples
  q->max_samples = max_samples;
  // 初始化样本计数器 Initialize sample counter
  ddsrt_atomic_st32(&q->nof_samples, 0);
  // 设置处理函数 Set handler function
  q->handler = handler;
  // 设置处理函数参数 Set handler function argument
  q->handler_arg = arg;
  // 初始化样本链表 Initialize sample chain
  q->sc.first = q->sc.last = NULL;
  // 设置 ddsi_domaingv 结构体 Set the ddsi_domaingv structure
  q->gv = (struct ddsi_domaingv *)gv;
  // 初始化线程指针 Initialize thread pointer
  q->thrst = NULL;

  // 初始化互斥锁和条件变量 Initialize mutex and condition variable
  ddsrt_mutex_init(&q->lock);
  ddsrt_cond_init(&q->cond);

  return q;
fail_name:
  ddsrt_free(q);
fail_q:
  return NULL;
}

/**
 * @brief 启动 ddsi_dqueue 实例的线程
 *        Start the thread for the ddsi_dqueue instance
 *
 * @param[in] q 指向 ddsi_dqueue 结构体的指针 Pointer to the ddsi_dqueue structure
 * @return 成功时返回 true，失败时返回 false
 *         Returns true on success, false on failure
 */
bool ddsi_dqueue_start(struct ddsi_dqueue *q) {
  char *thrname;
  size_t thrnamesz;
  // 计算线程名称长度 Calculate thread name length
  thrnamesz = 3 + strlen(q->name) + 1;
  // 分配内存并检查是否成功 Allocate memory and check for success
  if ((thrname = ddsrt_malloc(thrnamesz)) == NULL) return false;
  // 格式化线程名称 Format thread name
  (void)snprintf(thrname, thrnamesz, "dq.%s", q->name);
  // 创建线程并检查返回值 Create thread and check return value
  dds_return_t ret =
      ddsi_create_thread(&q->thrst, q->gv, thrname, (uint32_t(*)(void *))dqueue_thread, q);
  // 释放线程名称内存 Free thread name memory
  ddsrt_free(thrname);
  return ret == DDS_RETCODE_OK;
}

/**
 * @brief 将样本链表加入到 ddsi_dqueue 实例中（已锁定）
 *        Enqueue a sample chain into the ddsi_dqueue instance (locked)
 *
 * @param[in] q 指向 ddsi_dqueue 结构体的指针 Pointer to the ddsi_dqueue structure
 * @param[in] sc 指向 ddsi_rsample_chain 结构体的指针 Pointer to the ddsi_rsample_chain structure
 * @return 如果需要发出信号，则返回 1，否则返回 0
 *         Returns 1 if a signal must be sent, 0 otherwise
 */
static int ddsi_dqueue_enqueue_locked(struct ddsi_dqueue *q, struct ddsi_rsample_chain *sc) {
  int must_signal;
  // 检查队列是否为空 Check if the queue is empty
  if (q->sc.first == NULL) {
    must_signal = 1;
    // 设置新的样本链表 Set the new sample chain
    q->sc = *sc;
  } else {
    must_signal = 0;
    // 连接新的样本链表 Connect the new sample chain
    q->sc.last->next = sc->first;
    q->sc.last = sc->last;
  }
  return must_signal;
}

/**
 * @brief 将数据样本链入队并根据需要唤醒线程 (Enqueue a data sample chain and wake up the thread if
 * needed)
 *
 * @param[in] q 指向 ddsi_dqueue 结构的指针 (Pointer to the ddsi_dqueue structure)
 * @param[in] sc 指向 ddsi_rsample_chain 结构的指针 (Pointer to the ddsi_rsample_chain structure)
 * @param[in] rres 重排序结果 (Reorder result)
 * @return 唤醒信号 (Wake-up signal)
 */
bool ddsi_dqueue_enqueue_deferred_wakeup(struct ddsi_dqueue *q,
                                         struct ddsi_rsample_chain *sc,
                                         ddsi_reorder_result_t rres) {
  bool signal;
  assert(rres > 0);  // 确保重排序结果大于0 (Ensure reorder result is greater than 0)
  assert(sc->first);  // 确保样本链的第一个元素存在 (Ensure the first element of the sample chain
                      // exists)
  assert(sc->last->next == NULL);  // 确保样本链的最后一个元素的 next 为 NULL (Ensure the last
                                   // element of the sample chain has a next of NULL)
  ddsrt_mutex_lock(&q->lock);  // 对队列加锁 (Lock the queue)
  ddsrt_atomic_add32(
      &q->nof_samples,
      (uint32_t)rres);  // 更新队列中的样本数量 (Update the number of samples in the queue)
  signal = ddsi_dqueue_enqueue_locked(q, sc);  // 将样本链入队 (Enqueue the sample chain)
  ddsrt_mutex_unlock(&q->lock);                // 对队列解锁 (Unlock the queue)
  return signal;                               // 返回唤醒信号 (Return wake-up signal)
}

/**
 * @brief 触发条件变量广播 (Trigger a condition variable broadcast)
 *
 * @param[in] q 指向 ddsi_dqueue 结构的指针 (Pointer to the ddsi_dqueue structure)
 */
void ddsi_dqueue_enqueue_trigger(struct ddsi_dqueue *q) {
  ddsrt_mutex_lock(&q->lock);      // 对队列加锁 (Lock the queue)
  ddsrt_cond_broadcast(&q->cond);  // 广播条件变量 (Broadcast the condition variable)
  ddsrt_mutex_unlock(&q->lock);    // 对队列解锁 (Unlock the queue)
}

/**
 * @brief 将数据样本链入队 (Enqueue a data sample chain)
 *
 * @param[in] q 指向 ddsi_dqueue 结构的指针 (Pointer to the ddsi_dqueue structure)
 * @param[in] sc 指向 ddsi_rsample_chain 结构的指针 (Pointer to the ddsi_rsample_chain structure)
 * @param[in] rres 重排序结果 (Reorder result)
 */
void ddsi_dqueue_enqueue(struct ddsi_dqueue *q,
                         struct ddsi_rsample_chain *sc,
                         ddsi_reorder_result_t rres) {
  assert(rres > 0);  // 确保重排序结果大于0 (Ensure reorder result is greater than 0)
  assert(sc->first);  // 确保样本链的第一个元素存在 (Ensure the first element of the sample chain
                      // exists)
  assert(sc->last->next == NULL);  // 确保样本链的最后一个元素的 next 为 NULL (Ensure the last
                                   // element of the sample chain has a next of NULL)
  ddsrt_mutex_lock(&q->lock);  // 对队列加锁 (Lock the queue)
  ddsrt_atomic_add32(
      &q->nof_samples,
      (uint32_t)rres);  // 更新队列中的样本数量 (Update the number of samples in the queue)
  if (ddsi_dqueue_enqueue_locked(q, sc))
    ddsrt_cond_broadcast(
        &q->cond);  // 将样本链入队，如果需要唤醒线程则广播条件变量 (Enqueue the sample chain and
                    // broadcast the condition variable if needed to wake up the thread)
  ddsrt_mutex_unlock(&q->lock);  // 对队列解锁 (Unlock the queue)
}

/**
 * @brief 将气泡消息入队 (Enqueue a bubble message)
 *
 * @param[in] q 指向 ddsi_dqueue 结构的指针 (Pointer to the ddsi_dqueue structure)
 * @param[in] b 指向 ddsi_dqueue_bubble 结构的指针 (Pointer to the ddsi_dqueue_bubble structure)
 * @return 入队结果 (Enqueue result)
 */
static int ddsi_dqueue_enqueue_bubble_locked(struct ddsi_dqueue *q, struct ddsi_dqueue_bubble *b) {
  struct ddsi_rsample_chain sc;
  b->sce.next = NULL;  // 设置气泡消息的 next 为 NULL (Set the bubble message's next to NULL)
  b->sce.fragchain =
      NULL;  // 设置气泡消息的 fragchain 为 NULL (Set the bubble message's fragchain to NULL)
  b->sce.sampleinfo = (struct ddsi_rsample_info *)
      b;     // 设置气泡消息的 sampleinfo (Set the bubble message's sampleinfo)
  sc.first = sc.last =
      &b->sce;  // 将气泡消息加入样本链 (Add the bubble message to the sample chain)
  return ddsi_dqueue_enqueue_locked(q, &sc);  // 将样本链入队 (Enqueue the sample chain)
}

/**
 * @brief 将气泡消息入队并根据需要唤醒线程 (Enqueue a bubble message and wake up the thread if
 * needed)
 *
 * @param[in] q 指向 ddsi_dqueue 结构的指针 (Pointer to the ddsi_dqueue structure)
 * @param[in] b 指向 ddsi_dqueue_bubble 结构的指针 (Pointer to the ddsi_dqueue_bubble structure)
 */
static void ddsi_dqueue_enqueue_bubble(struct ddsi_dqueue *q, struct ddsi_dqueue_bubble *b) {
  ddsrt_mutex_lock(&q->lock);  // 对队列加锁 (Lock the queue)
  ddsrt_atomic_inc32(
      &q->nof_samples);  // 更新队列中的样本数量 (Update the number of samples in the queue)
  if (ddsi_dqueue_enqueue_bubble_locked(q, b))
    ddsrt_cond_broadcast(
        &q->cond);  // 将气泡消息入队，如果需要唤醒线程则广播条件变量 (Enqueue the bubble message
                    // and broadcast the condition variable if needed to wake up the thread)
  ddsrt_mutex_unlock(&q->lock);  // 对队列解锁 (Unlock the queue)
}

/**
 * @brief 将回调函数加入队列 (Enqueue a callback function)
 * @param[in] q 指向 ddsi_dqueue 结构的指针 (Pointer to the ddsi_dqueue structure)
 * @param[in] cb 回调函数 (Callback function)
 * @param[in] arg 回调函数的参数 (Argument for the callback function)
 */
void ddsi_dqueue_enqueue_callback(struct ddsi_dqueue *q, ddsi_dqueue_callback_t cb, void *arg) {
  struct ddsi_dqueue_bubble *b;
  // 为 ddsi_dqueue_bubble 结构分配内存 (Allocate memory for the ddsi_dqueue_bubble structure)
  b = ddsrt_malloc(sizeof(*b));
  // 设置 bubble 类型为回调函数类型 (Set the bubble kind to callback type)
  b->kind = DDSI_DQBK_CALLBACK;
  // 存储回调函数和参数 (Store the callback function and its argument)
  b->u.cb.cb = cb;
  b->u.cb.arg = arg;
  // 将 bubble 加入队列 (Enqueue the bubble)
  ddsi_dqueue_enqueue_bubble(q, b);
}

/**
 * @brief 将数据样本加入队列 (Enqueue a data sample)
 * @param[in] q 指向 ddsi_dqueue 结构的指针 (Pointer to the ddsi_dqueue structure)
 * @param[in] rdguid 读者 GUID (Reader GUID)
 * @param[in] sc 指向 ddsi_rsample_chain 结构的指针 (Pointer to the ddsi_rsample_chain structure)
 * @param[in] rres 排序结果 (Reorder result)
 */
void ddsi_dqueue_enqueue1(struct ddsi_dqueue *q,
                          const ddsi_guid_t *rdguid,
                          struct ddsi_rsample_chain *sc,
                          ddsi_reorder_result_t rres) {
  struct ddsi_dqueue_bubble *b;

  // 为 ddsi_dqueue_bubble 结构分配内存 (Allocate memory for the ddsi_dqueue_bubble structure)
  b = ddsrt_malloc(sizeof(*b));
  // 设置 bubble 类型为读者 GUID 类型 (Set the bubble kind to reader GUID type)
  b->kind = DDSI_DQBK_RDGUID;
  // 存储读者 GUID 和排序结果 (Store the reader GUID and reorder result)
  b->u.rdguid.rdguid = *rdguid;
  b->u.rdguid.count = (uint32_t)rres;

  // 检查输入参数的有效性 (Check the validity of input arguments)
  assert(rres > 0);
  assert(rdguid != NULL);
  assert(sc->first);
  assert(sc->last->next == NULL);
  // 加锁以保护队列操作 (Lock to protect queue operations)
  ddsrt_mutex_lock(&q->lock);
  // 更新样本数量 (Update the number of samples)
  ddsrt_atomic_add32(&q->nof_samples, 1 + (uint32_t)rres);
  // 将 bubble 加入队列并检查是否需要广播条件变量 (Enqueue the bubble and check if broadcasting the
  // condition variable is needed)
  if (ddsi_dqueue_enqueue_bubble_locked(q, b)) ddsrt_cond_broadcast(&q->cond);
  // 将数据样本加入队列 (Enqueue the data sample)
  (void)ddsi_dqueue_enqueue_locked(q, sc);
  // 解锁 (Unlock)
  ddsrt_mutex_unlock(&q->lock);
}

/**
 * @brief 检查队列是否已满 (Check if the queue is full)
 * @param[in] q 指向 ddsi_dqueue 结构的指针 (Pointer to the ddsi_dqueue structure)
 * @return 如果队列已满，返回非零值；否则，返回零 (Non-zero if the queue is full, zero otherwise)
 */
int ddsi_dqueue_is_full(struct ddsi_dqueue *q) {
  // 读取样本数量 (Read the number of samples)
  const uint32_t count = ddsrt_atomic_ld32(&q->nof_samples);
  // 判断队列是否已满 (Determine if the queue is full)
  return (count >= q->max_samples);
}

/**
 * @brief 如果队列已满，则等待直到队列为空 (Wait until the queue is empty if it is full)
 * @param[in] q 指向 ddsi_dqueue 结构的指针 (Pointer to the ddsi_dqueue structure)
 */
void ddsi_dqueue_wait_until_empty_if_full(struct ddsi_dqueue *q) {
  const uint32_t count = ddsrt_atomic_ld32(&q->nof_samples);
  // 如果队列已满 (If the queue is full)
  if (count >= q->max_samples) {
    // 加锁以保护条件变量操作 (Lock to protect condition variable operations)
    ddsrt_mutex_lock(&q->lock);
    // 广播条件变量 (Broadcast the condition variable)
    ddsrt_cond_broadcast(&q->cond);
    // 等待队列为空 (Wait for the queue to be empty)
    while (ddsrt_atomic_ld32(&q->nof_samples) > 0) ddsrt_cond_wait(&q->cond, &q->lock);
    // 解锁 (Unlock)
    ddsrt_mutex_unlock(&q->lock);
  }
}

/** @brief 释放队列中剩余的元素 (Free remaining elements in the queue)
 *
 * @param[in] q 指向 ddsi_dqueue 结构体的指针 (Pointer to the ddsi_dqueue structure)
 */
static void dqueue_free_remaining_elements(struct ddsi_dqueue *q) {
  // 确保 q->thrst 为空，表示没有线程正在操作该队列 (Ensure q->thrst is NULL, indicating that no
  // thread is operating on the queue)
  assert(q->thrst == NULL);

  // 遍历队列中的所有元素 (Traverse all elements in the queue)
  while (q->sc.first) {
    struct ddsi_rsample_chain_elem *e = q->sc.first;
    q->sc.first = e->next;

    // 根据元素类型执行相应的操作 (Perform corresponding operations based on the element type)
    switch (dqueue_elem_kind(e)) {
      case DQEK_DATA:
      case DQEK_GAP:
        // 释放分片链表 (Release the fragment chain)
        ddsi_fragchain_unref(e->fragchain);
        break;
      case DQEK_BUBBLE: {
        struct ddsi_dqueue_bubble *b = (struct ddsi_dqueue_bubble *)e->sampleinfo;
        // 如果气泡类型不是停止类型，则释放内存 (If the bubble type is not a stop type, release the
        // memory)
        if (b->kind != DDSI_DQBK_STOP) ddsrt_free(b);
        break;
      }
    }
  }
}

/** @brief 释放 ddsi_dqueue 结构体 (Free the ddsi_dqueue structure)
 *
 * @param[in] q 指向 ddsi_dqueue 结构体的指针 (Pointer to the ddsi_dqueue structure)
 */
void ddsi_dqueue_free(struct ddsi_dqueue *q) {
  // 在此点，不应该有任何线程再将元素加入队列 (At this point, there should be no threads enqueueing
  // elements anymore)
  if (q->thrst) {
    struct ddsi_dqueue_bubble b;
    b.kind = DDSI_DQBK_STOP;

    // 将停止气泡加入队列 (Enqueue the stop bubble)
    ddsi_dqueue_enqueue_bubble(q, &b);

    // 等待线程结束 (Wait for the thread to finish)
    ddsi_join_thread(q->thrst);
    assert(q->sc.first == NULL);
  } else {
    // 释放队列中剩余的元素 (Free remaining elements in the queue)
    dqueue_free_remaining_elements(q);
  }

  // 销毁条件变量和互斥锁 (Destroy the condition variable and mutex)
  ddsrt_cond_destroy(&q->cond);
  ddsrt_mutex_destroy(&q->lock);

  // 释放名称和结构体内存 (Release the name and structure memory)
  ddsrt_free(q->name);
  ddsrt_free(q);
}
