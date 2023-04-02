## Write-to-take with memory allocations

Path traversed by a sample, skipping some trivial functions and functions that are simply "more of the same", as well as with a bit of license in the "local subscription bypass" (skips straight from [ddsi_write_sample_gc] to [rhc_store], which is a bit of a lie when you look carefully at the code). This is annotated with memory allocation activity.

> 一个示例遍历的路径，跳过一些琐碎的函数和简单地“大致相同”的函数，以及“本地订阅旁路”中的一点许可证(直接从[ddsi_write_sample_gc]跳到[rhc_store]，仔细看代码时这有点像谎言)。这是用内存分配活动来注释的。

Currently in default configuration, everything from [dds_write] to [sendmsg] happens on the application thread. With current "asynchronous" mode, outgoing packets get queued and are transmitted on a separate thread.

> 目前在默认配置中，从[dds_write]到[sendmsg]的所有操作都发生在应用程序线程上。在当前的“异步”模式下，传出的数据包将排队并在单独的线程上传输。

```
    APPLICATION
      |
      v
    dds_write
      |
      |  serdata: from_sample
      |    - allocates "serdata" and serializes into it key to instance_handle mapping (1) allocates for a unique key
      |    - tkmap_instance (entry in table)
      |    - "untyped serdata" for insertion in table
      |    - possibly resizes hash table (if no readers, undoes the above)
      |
      v
    ddsi_write_sample_gc
      |         \
      |          \ (local subscriptiosn bypass)
      |           \
      |            \
      |             \
      |              \
      |               |
      |  ddsi_whc_insert (for reliable data)
      |    - allocates whc_node (2)
      |    - inserts in seq# hash (which may grow hash table)
      |    - adds to seq# interval tree (which may require an interval tree node)
      |    - may push out old samples, which may cause frees
      |    - if keyed topic: insert in index on instance_handle
      |      - may grow hash table (1)
      |               |
      v               |
    transmit_sample     |
      |               |
      |  allocate and initialise new "xmsg" (3)
      |    - large samples: needs many, for DATAFRAG and HEARTBEATFRAG
      |    - serialised data integrated by-reference
      |    - may also need one for a HEARTBEAT
      |               |
      |  xpack_add called for each "xmsg" to combine into RTPS messages
      |    - fills a (lazily allocated per writer) scatter/gather list
      |    - hands off the packet for synchronous/asynchronous publication when full/flushed
      |               |
    ddsi_xpack_send.. | ......+ (*current* asynchronous mode)
      |               |       |
      |               |       v
      |               |     ddsi_xpack_sendq_thread
      |               |       |
      |               |       |
      v               |       v
    ddsi_xpack_send_real    ddsi_xpack_send_real
      |               |       |
      |    - transmits the packet using sendmsg()
      |    - releases record of samples just sent to track the highest seq# that was actually transmitted, which may release samples if these were ACK'd already (unlikely, but possible) (3)
      |               |       |
      |               |       |
      v               |       v
    sendmsg           |     sendmsg
      .               |
      .               |
      .               |
    [network]         | (local subscriptions bypass)
      .               |
      .               |
      .               |
    ddsi_rmsg_new     |
      |               |
      |  ensure receive buffer space is available (5) defragmenting or dealing with samples received out-of-order
      |    - these buffers are huge in the default config to reduce number of allocations
      v               |
    recvmsg           |
      |               |
      |               |
      v               |
    do_packet/handle_submsg_sequence (5)
      |               |
      |  typically allocates memory, typically contiguous with received datagram by bumping a pointer
      |    - COW receiver state on state changes
      |               |
      |  DATA/DATAFRAG/GAP:
      |    - allocate message defragmenting state
      |    - allocate message reordering state
      |    - (typically GAP doesn't require the above)
      |    - may result in delivering data or discarding fragments, which may free memory
      |               |
      |  ACKNACK:     |
      |    - may drop messages from WHC, freeing (2):
      |      - whc_node, interval tree entry, index entries, possibly serdata
      |      - possible "keyless serdata" and instance_handle index entry
      |  ACKNACK/NACKFRAG:
      |    - possibly queues retransmits, GAPs and HEARTBEATs
      |      - allocates "xmsg"s (like data path) (3)
      |      - allocates queue entries (4)
      |      - freed upon sending
      |               |
      |  HEARTBEAT:   |
      |    - may result in delivering data or discarding fragments, which may free memory
      |               |
      |               |
      |  Note: asynchronous delivery queues samples ready for delivery; the
      |  matching delivery thread then calls deliver_user_data_synchronously
      |  to deliver the data (no allocations needed for enqueuing)
      |               |
      v               |
    deliver_user_data_synchronously
      |               |
      |  serdata: from_ser
      |    - allocates a "serdata" and, depending on the implementation, validates the serialized data and stores it (e.g., the C version), deserialises it immediately (e.g., the C++ version), or leaves if in the receive buffers (incrementing refcounts; not done currently, probably not a wise choice either)
      |               |
      |  frees receive buffer claims after having created the "serdata" (5) typical synchronous delivery path without message loss:
      |    - resets receive buffer allocator pointer to what it was prior to processing datagram, re-using the memory for the next packet
      |    - (but typical is not so interesting in a worst-case analysis ...)
      |               |
      |  key to instance_handle mapping (1)
      |    allocates for a unique key
      |    - tkmap_instance (entry in table)
      |    - "untyped serdata" for insertion in table
      |    - possibly resizes hash table
      |    (if no readers, undoes the above)
      |               |
      |              /
      |             /
      |            /
      |           /
      |          / (local subscriptions bypass)
      v         /
    rhc_store (once per reader)
      |
      |  - allocates new "instance" if instance handle not yet present in its hash table may grow instance hash table
      |  - allocates new sample (typically, though never for KEEP_LAST with depth 1 nor for pushing old samples out of the history
      |  - may free serdatas that were pushed out of the history this won't affect the instance_handle mappings nor the "untyped serdata" because overwriting data in the history doesn't change the set of keys
      |  - may require insertion of a "registration" in a hash table, which in turn may require allocating/growing the hash table (the "registration" itself is not allocated)
      |
      v
    dds_take/dds_read
      |
      |  - serdata: to_sample, deserialises into application representation dds_take:
      |  - frees the "serdata" if the refcount drops to zero
      |  - removes the sample from the reader history (possibly the instance as well) which may involve
      |  - freeing memory allocated for the instance handle mapping and the "untyped serdata" (1)
      |
      v
    APPLICATION
```

There are a number of points worth noting in the above:

- Cyclone defers freeing memory in some cases, relying on a garbage collector, but this garbage collector is one in the sense of the garbage trucks that drive through the streets collecting the garbage that has been put on the sidewalk, rather than the stop-the-world/"thief in the night" model used in Java, C#, Haskell, &c.

> - Cyclone 在**某些情况下会延迟释放内存，依赖于垃圾收集器**，但这种垃圾收集器**是一种在街道上行驶的垃圾车，收集放在人行道上的垃圾，而不是 Java、C#、Haskell 等中使用的“阻止世界”/“夜间小偷”模型**。

The deferring is so that some data can be used without having to do reference counting for dynamic references caused by using some data for a very short period of time, as well as, to some extent, to not incur the cost of freeing at the point of use. This is currently used for:

> 延迟是为了使一些数据可以被使用，而不必对由于在很短的时间内使用一些数据而导致的动态引用进行引用计数，并且在某种程度上，不需要在使用点产生释放的成本。当前用于：

- mapping entries for key value to instance handle
- all DDSI-level entities (writers, readers, participants, topics and their "proxy" variants for remote ones)
- hash table buckets for the concurrent hash tables used to index the above
- active manual-by-participant lease objects in proxy participants

> - 将键值的条目映射到实例句柄
> - 所有 DDSI 级别的实体(作者、读者、参与者、主题及其远程实体的“代理”变体)
> - 用于索引上述内容的并发哈希表的哈希表桶
> - 代理参与者中的参与者租赁对象的活动手册

Freeing these requires enqueueing them for the garbage collector; that in turn is currently implemented by allocating a "gcreq" queue entry.

> 释放这些需要将它们排队等待垃圾收集器；这反过来又是当前通过分配一个“gcreq”队列条目来实现的。

- If one only uses keyless topics (like ROS 2 in its current version) for each topic there is at most a single "instance" and consequently, at most a single instance handle and mapping entry at any one time. For administrating these instance handles, the implementation reduces the sample to its key value and erases the topic from it (the "untyped serdata" in the above). This way, if different topics from the same underlying type implementation have the same key value, they may get the same instance handle and the same mapping entry.

> -如果每个主题只使用无键主题(如当前版本中的 ROS 2)，那么在任何时候最多只能有一个“实例”，因此最多只能有单个实例句柄和映射条目。为了管理这些实例句柄，实现将样本减少到其键值，并从中删除主题(上面的“非类型化的 serdata”)。这样，如果来自同一底层类型实现的不同主题具有相同的键值，那么它们可能会获得相同的实例句柄和相同的映射条目。

- The allocations of [whc_node] (marked [(2)]) for tracking individual samples in the writer history cache potentially happen at very high rates (> 1M/s for throughput tests with small samples) and I have seen the allocator becomes a bottleneck. Caching is the current trick to speed this up; the cache today is bounded by what can reasonably be expected to be needed.

> - [whc_node](<标记为[(2)]>)**用于跟踪写入程序历史缓存中的单个样本的分配可能会以非常高的速率发生(对于小样本的吞吐量测试，速率>1M/s)，我看到分配器成了瓶颈。缓存是当前加快速度的技巧；目前的缓存是由合理预期的需求所限制的**。

The interval tree is allocated/freed without caching, because there is far less churn for those (e.g., for a simple queue, there is only one interval needed).

> 间隔树是在没有缓存的情况下分配/释放的，因为这些树的流失要少得多(例如，对于一个简单的队列，只需要一个间隔)。

The number of samples in the writer history cache is, of course, bounded by history settings, and so these could just as easily be pre-allocated. Even better is to use the fact that the WHC has been abstracted in the code. A simple pre-allocated circular array is sufficient for a implementing a queue with limited depth, and so using a different implementation is altogether more sensible.

> 当然，写入程序历史缓存中的样本数量受历史设置的限制，因此可以很容易地预先分配这些样本。更好的是使用 WHC 已经在代码中抽象的事实。一个简单的预分配循环数组就足以实现深度有限的队列，因此使用不同的实现更明智。

- The "xmsg" (marked [(3)]) that represent RTPS submessages (or more precisely: groups of submessages that must be kept together, like an INFO_TS and the DATA it applies to) are cached for the same reason the WHC entries are cached. Unlike the latter, the lifetime of the "xmsg" is the actual sending of data. The number of "xmsg" that can be packed into a message is bounded, and certainly for the data path these can be preallocated. Currently they are cached.

> - 表示 RTPS 子消息的“xmsg”(标记为[(3)])(或者更准确地说：必须保持在一起的子消息组，如 INFO_TS 及其应用的 DATA)被缓存的原因与缓存 WHC 条目的原因相同。与后者不同，“xmsg”的生存期是数据的实际发送。可以打包到消息中的“xmsg”的数量是有界的，当然对于数据路径来说，这些可以预先分配。目前它们被缓存。

For generating responses to ACKNACKs and HEARTBEATs and queueing them, maintaining a pool with a sensible policy in case the pool is too small should be possible: not sending a message in response because no "xmsg" is available is equivalent to it getting lost on the network, and that is supported. The "sensible" part of the policy is that it may be better to prioritise some types over others, e.g., perhaps it would be better to prioritise acknowledgements over retransmits. That's something worth investigating.

> 为了生成对 ACKNACK 和 HEARTBEAT 的响应并对其进行排队，应该可以使用合理的策略来维护池，以防池太小：因为没有“xmsg”可用而不发送响应消息相当于它在网络上丢失，这是受支持的。该政策的“明智”部分是，优先考虑某些类型可能比其他类型更好，例如，优先考虑确认而不是重传可能会更好。这是值得调查的事情。

- Responses to received RTPS messages are not sent by the same thread, but rather by a separate thread (marked [(4)]). Retransmits (for example) are generated and queued by the thread handling the incoming packet, but (for example) ACKNACKs are generated by rescheduling a pre-existing event that then generates an "xmsg" in that separate thread and transmits it similar to regular data transmission.

> - 对接收到的 RTPS 消息的响应不是由同一个线程发送的，而是由一个单独的线程(标记为[(4)])发送的。重传(例如)由处理传入数据包的线程生成并排队，但是(例如)ACKNACK 是通过重新调度预先存在的事件来生成的，该事件然后在该单独的线程中生成“xmsg”，并类似于常规数据传输来传输它。

Pre-allocating the queue entries (or better yet: turning the queue into a circular array) has similar considerations to pre-allocating the "xmsg".

> 预分配队列条目(或者更好的做法是：将队列变成一个循环数组)与预分配“xmsg”有类似的注意事项。

- Receive buffers are troublesome (marked [(5)]): to support platforms without scatter/gather support in the kernel (are there still any left?) it allocates large chunks of memory so it can accept a full UDP datagram in it with some room to spare. That room to spare is then used to store data generated as a consequence of interpreting the packet: receiver state, sample information, administration for tracking fragments or samples that were received out of order. Data is the longest lived of all this, and so releasing the memory happens when all data in the buffer has been delivered (or discarded).

> - 接收缓冲区很麻烦(标记为[(5)])：为了支持内核中没有分散/聚集支持的平台(还有剩余的吗？)，它会分配大块内存，这样它就可以接受其中的完整 UDP 数据报，并留出一些空间。然后，该空闲空间用于存储由于解释数据包而生成的数据：接收器状态、样本信息、用于跟踪无序接收的片段或样本的管理。数据是所有这些中寿命最长的，因此当缓冲区中的所有数据都已传递(或丢弃)时，就会释放内存。

If fragmented data is received or data is received out-of-order, that means the packets can hang around for a while. The benefit (and primary reason why I made this experiment way back when) is that there is no copying of anything until the received data is turned into "serdata"s by [from_ser]. Even at that stage, it is still possible to not copy the data but instead leave it in the receive buffer, but I have never actually tried that.

> 如果接收到碎片数据或数据接收不正常，这意味着数据包可能会挂一段时间。好处(也是我早在什么时候做这个实验的主要原因)是，在[from_ser]将接收到的数据转换为“serdata”之前，不会复制任何东西。即使在那个阶段，仍然可以不复制数据，而是将其留在接收缓冲区，但我从未真正尝试过。

The allocator used within the buffers is a simple bump allocator with the optimisation that it resets completely if no part of the packet remains live after processing all its submessages, and so for non-fragmented data with negligible packet loss, you're basically never allocating memory. Conversely, when data is fragmented or there is significant packet loss, it becomes a bit of a mess.

> 缓冲区中使用的分配器是一个简单的缓冲分配器，经过优化，如果数据包的任何部分在处理完所有子消息后都没有保持活动状态，它会完全重置，因此对于数据包丢失可忽略不计的非碎片数据，基本上永远不会分配内存。相反，当数据被分割或出现严重的数据包丢失时，它会变得有点混乱。

Eliminating the allocating and freeing of these buffers needs some thought ... quite possibly the best way is to assume scatter/gather support and accept an additional copy if the data can't be delivered to the reader history immediately.

> 消除这些缓冲区的分配和释放需要一些思考。。。很可能最好的方法是假设支持分散/收集，并在数据无法立即传递到读取器历史记录的情况下接受额外的副本。

- The reader history cache is really vastly overcomplicated for many simlpe use cases. Similarly to the writer history cache, it can be replaced by an alternate implementation, and that's probably the most straightforward path to eliminating allocations. Alternatively, one could pre-allocate instances and samples.

> - 对于许多 simlpe 用例来说，读取器历史缓存确实过于复杂。与编写器历史缓存类似，它可以被替代实现所取代，这可能是消除分配的最直接方法。或者，可以预先分配实例和样本。

The "registrations" that track which writer is writing which instances do not incur any allocations unless there are multiple writers writing the same instance. Cyclone follows the DDS spec in treating topics that have no key fields as topics having a single instance, and so multiple publishers for the same topic will cause these to be tracked. That means typical queuing patterns result in as many "registrations" for a topic as there are writers for that topic.

> 跟踪哪个写入程序正在写入哪些实例的“注册”不会产生任何分配，除非有多个写入程序在写入同一实例。Cyclone 遵循 DDS 规范，将没有关键字段的主题视为具有单个实例的主题，因此同一主题的多个发布者将导致跟踪这些主题。这意味着典型的排队模式会导致一个主题的“注册”数量与该主题的作者数量一样多。

It may be a good idea to add a setting to avoid tracking registrations. Note that the DDSI spec already describes a model wherein there are no registrations for keyless topics.

> 添加一个设置以避免跟踪注册可能是个好主意。注意，DDSI 规范已经描述了一种模型，其中没有无钥匙主题的注册。
