discovery maintains the set of readers that match the proxy writer and an array of reader pointers that the data from this proxy writer must be delivered to. This should be versioned, so partition changes are (can be) precise. Currently there is no versioning.

implementation: possibly by using the sequence number of the writer data as a version number: that allows for a very simple, parallel, garbage collector for these sets

> 发现维护与代理写入器匹配的一组读取器和一组读取器指针，来自该代理写入器的数据必须传递到这些读取器指针。 这应该是版本化的，因此分区更改是（可以）精确的。 目前没有版本控制。
>
> 实现：可能通过使用编写器数据的序列号作为版本号：这允许为这些集合提供一个非常简单的并行垃圾收集器

---

DCPS reader entity owns the DDSI reader and stores a pointer and the GUID

---

"reader" is a DDSI reader, which is a proxy for a local DCPS reader; "proxy writer" is a proxy for a remote DDSI writer

> “reader”是 DDSI 阅读器，是本地 DCPS 阅读器的代理； “proxy writer”是远程 DDSI writer 的代理

---

guid hash table maps all (local & proxy) endpoint and participant guids to the corresponding object. Pointers to such objects must be derived from the guid hash, and may not be retained across thread liveliness state updates.

> guid 哈希表将所有（本地和代理）端点和参与者 guid 映射到相应的对象。 指向此类对象的指针必须从 guid 哈希派生，并且可能不会在线程活跃状态更新中保留。

---

proxy-writer – reader matches switch between not-in-sync and in-sync; the general idea being that the in-sync ones are only relevant to heartbeat processing and need not be touched by Data(Frag) processing. ("Too new" data always gets stored in the primary reorder admin, whereas the not-in-sync ones are generally interested in old data and track old data in the secondary reorder admins.)

> proxy-writer——读者匹配在不同步和同步之间切换； 一般的想法是同步的只与心跳处理相关，不需要被数据（片段）处理触及。 （“太新”的数据总是存储在主要重新排序管理员中，而不同步的数据通常对旧数据感兴趣并在次要重新排序管理员中跟踪旧数据。）

---

Major differences from current implementation:

- proxy writer stores all its reader matches currently in a single tree, not discriminating between sync/not-in-sync
- all recv processing currently in a single thread
- no QoS changes yet, no versioning of QoS's

---

the receive thread requests the O/S kernel to dump the data in large rbufs, each containing any number of messages; the decoding appends some information, both for Data and DataFrag sub-messages (the rdata elements), and for the embedded QoS lists.

Each rdata contains all that is necessary to track it in defragmenting and reordering admins, and to link it into the delivery queue. This ensures no (heap) memory allocations are necessary to track an arbitrary number of messages/fragments in the normal case.

The deserializer operates from the rdata elements, it gets a little bit nasty when a primitive is split over mulitple fragments, but so be it. NIY: currently malloc + memcpy

> 接收线程请求 O/S 内核将数据转储到大型 rbuf 中，每个包含任意数量的消息； 解码会附加一些信息，包括 Data 和 DataFrag 子消息（rdata 元素）以及嵌入式 QoS 列表。
>
> 每个 rdata 包含在碎片整理和重新排序管理员中跟踪它并将其链接到交付队列中所需的所有内容。 这确保在正常情况下不需要（堆）内存分配来跟踪任意数量的消息/片段。
>
> 反序列化器从 rdata 元素运行，当一个基元被分割成多个片段时它会变得有点讨厌，但就这样吧。 NIY：目前 malloc + memcpy

---

delivery queues are filled by recv thread (by shuffling rdata references around); heartbeats and acks are a special case: they have no associated rdata and are processed as quickly as possible

> 交付队列由 recv 线程填充（通过混洗 rdata 引用）； heartbeats 和 acks 是一种特殊情况：它们没有关联的 rdata 并且会尽快处理

---

potentially many delivery threads: if a writer always delivers via the same queue, all expected ordering properties are retained

> 可能有很多交付线程：如果作者总是通过同一个队列交付，则保留所有预期的排序属性

given that the kernel can only do groupWrite, and not deliver data to an individual reader, might as well design to that behaviour
