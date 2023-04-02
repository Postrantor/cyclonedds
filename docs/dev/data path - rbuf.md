A receive thread has a pool of receive buffers, rbuf s, each large enough to contain several network packets and derived administrative data.

Before requesting a packet from the kernel, a receive thread allocates an rmsg from the buffer, which is the administrative entity representing a raw packet along with additional derived data.

During processing the packet, an rdata is created for each Data/DataFrag submessage. These rdata entries contain a reference to the rmsg and a byte range within the packet at which the serialized payload is stored.

For each sample (i.e., sequence number), a sampleinfo containing various metadata on the sample is allocated. The sampleinfo includes a reference to the receiver state as described in the DDSI specification, which contains, a.o., time stamps and source addresses.

For fragmented data, rdata are chained together into a fragchain , using interval tree pointed to by a sample.defrag . Completed samples that have been received out of order are linked into chains of consecutive samples, with the sample chains organized into an interval tree using sample.reorder . Once a sample has been delivered, the reference count of the rmsg s pointed to by its fragchain are decremented, eventually freeing the original network packets and recovering space in the rbuf s.

> 一个接收线程有一个接收缓冲区池，rbufs，每个都足够大以包含几个网络数据包和派生的管理数据。
>
> 在从内核请求数据包之前，接收线程从缓冲区分配一个 rmsg，缓冲区是代表原始数据包和其他派生数据的管理实体。
>
> 在处理数据包期间，为每个 Data/DataFrag 子消息创建一个 rdata。 这些 rdata 条目包含对 rmsg 的引用和数据包中存储序列化有效负载的字节范围。
>
> 对于每个样本（即序列号），分配一个包含样本各种元数据的样本信息。 sampleinfo 包括对 DDSI 规范中描述的接收器状态的引用，其中包含 a.o.、时间戳和源地址。
>
> 对于碎片化数据，rdata 使用 sample.defrag 指向的间隔树链接在一起成为一个 fragchain 。 乱序接收的已完成样本链接到连续样本链中，样本链使用 sample.reorder 组织成间隔树。 一旦样本被传送，其片段链指向的 rmsg 的引用计数就会减少，最终释放原始网络数据包并恢复 rbuf 中的空间。
