Reliable Communication, Writer History Cache, Heartbeat, Best-effort communication

> 可靠的通信、书写器历史缓存、心跳、尽力而为的通信

# Reliable communication

_Best-effort_ communication is a wrapper around UDP/IP. That is, the packet(s) containing a sample are sent to the addresses where the readers reside. No state is maintained on the writer. If a packet is lost, the reader ignores the samples contained in the lost packet and continue with the next one.

> **_Best-effort_ 通信是围绕 UDP/IP 的包装**。也就是说，包含样本的数据包被发送到读取器所在的地址。写入程序未保持任何状态。如果数据包丢失，读取器将忽略丢失数据包中包含的样本，并继续下一个数据包。

When using _reliable_ communication, the writer maintains a copy of the sample in the Writer History Cache (WHC) of the DDSI writer. If a reader detects that there are lost packets, the reader can request a re-transmission of the sample. To ensure that all readers learn of new samples in the WHC, the DDSI writer periodically sends _heartbeats_ to its readers. If all matched readers have acknowledged all samples in the WHC, the DDSI writer can suppress these periodic heartbeats.

> 当使用 _reliable_ 通信时，writer 在 DDSI writer 的 writer 历史缓存（WHC）中维护样本的副本。如果读取器检测到有丢失的数据包，则读取器可以请求重新传输样本。**为了确保所有读者了解 WHC 中的新样本，DDSI writer 定期向其读者发送 _heartbeats_。如果所有匹配的读取器都确认了 WHC 中的所有样本，则 DDSI writer 可以抑制这些周期性心跳。**

If a reader receives a heartbeat and detects it did not receive all samples, it requests a re-transmission by sending an _AckNack_ message to the writer.

> 如果读取器接收到心跳并检测到它没有接收到所有样本，它会通过向写入器发送 _AckNack_ 消息来请求重新传输。

> [NOTE]:
> The heartbeat timing is adjustable. If the latency is longer than the heartbeat interval, this can result in multiple re-transmit requests for a single sample.
> 心跳时间是可调节的。**如果延迟比心跳间隔长，这可能会导致对单个样本的多次重传请求**。

In addition to requesting the re-transmission of some samples, a reader also uses the AckNack messages to inform the writer of samples received and not received. Whenever the writer indicates that it requires a response to a heartbeat, the readers sends an AckNack message (even when no samples are missing).

> 除了请求重新传输一些样本外，读取器还使用 AckNack 消息来通知写入器已接收和未接收的样本。**每当 writer 指示需要对心跳进行响应时，读取器都会发送 AckNack 消息（即使没有样本丢失）。**

Combining these behaviours allows the writer to remove old samples from its WHC when it fills up the cache, enabling readers to receive all data reliably.

> **结合这些行为，writer 可以在填充缓存时从其 WHC 中删除旧样本，从而使读取器能够可靠地接收所有数据。**

The default behaviour is to never to consider readers unresponsive. The DDSI specification does not define how to handle a situation where readers do not respond to a heartbeat, or fail to receive samples from a writer after a re-transmission request. A solution to this situation is to periodically check the participant containing the reader.

> 默认行为是从不认为读者没有反应。DDSI 规范没有定义如何处理读卡器不响应心跳或在重新传输请求后无法从写入器接收样本的情况。解决这种情况的方法是定期检查包含阅读器的参与者。

> [NOTE]:
> The DDSI specification allows for the suppressing of heartbeats, merging AckNacks, and re-transmissions, and so on. These techniques are required to allow for a performant DDSI implementation while avoiding the need for sending redundant messages.
> DDSI 规范允许抑制心跳、合并 AckNacks 和重新传输等。这些技术是为了实现高性能的 DDSI，同时避免发送冗余消息的需要。
