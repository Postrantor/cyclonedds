<a href="<https://cunit.sourceforge.net/>" target="_blank">CUnit</a>
<a href="<https://conan.io/>" target="_blank">Conan</a>
<a href="<https://www.eclipse.org/legal/ECA.php/>">Eclipse Contributor Agreement</a>
<a href="<https://accounts.eclipse.org/user/eca/>">Eclipse Contributor Agreement</a>
<a href="<https://www.omg.org/spec/DDSI-RTPS>" target="_blank">DDSI specification</a>
<a href="<https://www.omg.org/spec/DDS/>" target="_blank">OMG DDS specification</a>
<a href="<https://www.omg.org/spec/DDSI-RTPS/2.1>" target="_blank">DDSI-RTPS 2.1</a>
<a href="<https://www.omg.org/spec/DDSI-RTPS/2.2>" target="_blank">DDSI-RTPS 2.2</a>
<a href="<https://www.omg.org/spec/DDSI-RTPS/2.5>" target="_blank">DDSI-RTPS 2.5</a>
<a href="<https://git-scm.com/>" target="_blank">Git</a>
<a href="<https://cmake.org/>" target="_blank">CMake</a>
<a href="<https://cmake.org/cmake/help/latest/guide/using-dependencies/index.html#guide:Using%20Dependencies%20Guide>" target="_blank">Using dependencies guide</a>
<a href="<https://www.openssl.org/>" target="_blank">OpenSSL</a>
<a href="<https://projects.eclipse.org/proposals/eclipse-iceoryx/>" target="_blank">Eclipse iceoryx</a>
<a href="<https://cunit.sourceforge.net/>" target="_blank">CUnit</a>
<a href="<https://www.sphinx-doc.org/en/master/>" target="_blank">Sphinx</a>
<a href="<https://chocolatey.org/>" target="_blank">chocolatey package manager</a>
<a href="<https://scoop.sh/>" target="_blank">Scoop</a>
<a href"<https://www.omg.org/spec/IDL/4.2>" target="\_blank">OMG IDL</a>

# Data path configuration

Heartbeat, Unicast, Multicast, Re-transmission merging

> 心跳，单播，多播，重新传输合并

A reader can request re-transmission of many samples at once. When the writer queues all these samples for re-transmission, it can cause a large backlog of samples. As a result, the samples near the queue's end may be delayed so much that the reader issues another re-transmission request.

> reader可以一次要求重新传输许多样本。当writer排队所有这些样本进行重新传输时，它可能会导致大量的样本。结果，队列末端附近的样品可能会被延迟，以至于reader发出了另一个重新传输请求。

limits the number of samples queued for re-transmission and ignores re-transmission requests that either causes the re-transmission queue to contain too many samples, or take too long to process. Two settings govern the size of these queues. The limits are applied per timed-event thread:

> **限制排队重新传输的样品数量，而忽略了重新传输请求**，该请求会导致重新传输队列包含太多样本，或者花费太长处理。两个设置控制着这些队列的大小。每个定时事件线程应用限制：

- The number of re-transmission messages: `Internal/MaxQueuedRexmitMessages <//CycloneDDS/Domain/Internal/MaxQueuedRexmitMessages>`.

> - 重新传输消息的数量：`insern/maxqueuedrexmitmessages <// cyclonedds/domain/domain/internal/nistern/maxqueuedrexmitmessages>`。

- The size of the queue: `Internal/MaxQueuedRexmitBytes <//CycloneDDS/Domain/Internal/MaxQueuedRexmitBytes>`. This defaults to a setting based on the combination of the allowed transmition bandwidth, and the `Internal/NackDelay <//CycloneDDS/Domain/Internal/NackDelay>` setting, which is an approximation of the time until the next re-transmission request from the Reader.

> - 队列的大小：`内部/maxqueuedrexmitbytes <// cyclonedds/domain/insenter/internal/maxqueuedrexmitbytes>`。此默认为基于允许的传输带宽的组合和``nestran/nackdelay <// cyclonedds/domain/domin/internal/nackdelay>`设置，这是下一个时间的近似值reader。

Fragmentation, Datagram, Packing, Header sizes

> 碎片，数据报，包装，标题大小

## Fragmentation

does this need to be rewritten?

> 这需要重写吗？

Samples in DDS can be arbitrarily large, and do not always fit within a single datagram. DDSI can fragment samples so they can fit in UDP datagrams. IP has facilities to fragment UDP datagrams into network packets. The DDSI specification (see section 8.4.14.1.2) describes how to send fragments (Data must only be fragmented if required). However, provides a fully configurable behaviour.

> **DDS 中的样本可以任意大，并不总是适合单个数据报。 DDSI 可以对样本进行分段，以便它们适合 UDP 数据报。 IP 具有将 UDP 数据报分段为网络数据包的功能。** DDSI 规范（参见第 8.4.14.1.2 节）描述了如何发送片段（数据必须仅在需要时才被片段化）。 但是，提供完全可配置的行为。

If the serialised form of a sample is at least the size set in: `General/FragmentSize <//CycloneDDS/Domain/General/FragmentSize>`, it is fragmented using DDSI fragmentation. All fragments are this exact size, except for the last one, which may be smaller.

> 如果样品的序列化形式至少在：`eNerver/fragmentsize <// cyclonedds/domain/enstain/eneral/fragmentsize>`中，则使用 ddsi 碎片片段化。除最后一个片段外，所有片段都是确切的尺寸，除了最后一个片段。

To reduce the number of network packets, the following are all subject to packing into datagrams (based on various attributes such as the destination address) before sending on the network:

> 为了减少网络数据包的数量，以下所有内容都需要在发送网络上发送之前的数据报（基于各种属性，例如目标地址）：

- control messages
- non-fragmented samples
- sample fragments

> - 控制消息
> - 非碎片样品
> - 样品片段

Packing allows datagram payloads of up to `General/MaxMessageSize <//CycloneDDS/Domain/General/MaxMessageSize>`. If the `MaxMessageSize` is too small to contain a datagram payload as a single unit, …

> 包装允许最多可数据载荷<// maxMessagesize <// cyclonedds/domain/enseral/general/maxmessagesize>`。如果“ maxMessagesize”太小，无法包含数据报有效载荷作为一个单元，请…

Need an explanation of "If the `MaxMessageSize` is too small to contain a datagram as a single unit, …"

> 需要说明“如果````MaxMessages）'

> [NOTE]:

UDP/IP header sizes are not taken into account in the maximum message size.

> UDP/IP 标头尺寸未在最大消息大小中考虑到。

To stay within the maximum size that the underlying network supports, the IP layer fragments the datagram into multiple packets.

> 为了保持基础网络支持的最大尺寸，IP 层片段将数据报到多个数据包。

A trade-off is that while DDSI fragments can be re-transmitted individually, the processing overhead of DDSI fragmentation is larger than that of UDP fragmentation.

> 一个折衷的是，尽管可以单独重新传输 DDSI 片段，但 **DDSI 片段化的处理开销大于 UDP 片段的处理**。

Receive processing, Defragmentation, Receive thread, Delivery thread

> 接收处理，碎片部，接收线程，交货线程

## Receive processing

Receiving of data is split into multiple threads:

> **接收数据分为多个线程**：

- A single receive thread, which is responsible for:

> - 一个接收线程，负责：

    - Retrieving all incoming network packets.
    - Running the protocol state machine, which involves scheduling of AckNack and heartbeat messages.
    - Queueing of samples that must be re-transmitted.
    - Defragmenting.
    - Ordering incoming samples.

    - 检索所有传入的网络数据包。
    - 运行协议状态机，包括调度 AckNack 和心跳消息。
    - 必须重新传输的样本排队。
    - 碎片整理。
    - 订购来样。

    Between the receive thread and the delivery threads are queues. To control the maximum queue size, set: `Internal/DeliveryQueueMaxSamples <//CycloneDDS/Domain/Internal/DeliveryQueueMaxSamples>`. Generally, queues do not need to be very large (unless there are very small samples in very large messages). The primary function is to smooth out the processing when batches of samples become available at once, for example following a re-transmission.

    **接收线程和发送线程之间是队列。 要控制最大队列大小**，请设置：`Internal/DeliveryQueueMaxSamples <//CycloneDDS/Domain/Internal/DeliveryQueueMaxSamples>`。 通常，队列不需要很大（除非很大的消息中有很小的样本）。 主要功能是在批量样本立即可用时平滑处理，例如在重新传输之后。

    When any of these receive buffers hit their size limit, and it concerns application data, the receive thread waits for the queue to shrink. However, discovery data never blocks the receive thread.

    当这些接收缓冲区中的任何一个达到其大小限制并且它涉及应用程序数据时，接收线程将等待队列收缩。 但是，**发现数据永远不会阻塞接收线程**。

- A delivery thread dedicated to processing DDSI built-in data:

> - 用于处理 DDSI 内置数据的交付线程：

    - Participant discovery
    - Endpoint discovery
    - Liveliness assertions

- One or more delivery threads dedicated to the handling of application data:

> - 专门用于处理应用程序数据的一个或多个交付线程：

    - deserialisation
    - delivery to the DCPS data reader caches

Fragmented data first enters the defragmentation stage, which is per proxy writer. The number of samples that can be defragmented simultaneously is limited:

> 零散的数据首先进入碎片阶段，这是代理writer。可以同时进行碎片片的样品数量有限：

- Reliable data: `Internal/DefragReliableMaxSamples <//CycloneDDS/Domain/Internal/DefragReliableMaxSamples>`
- Unreliable data: `Internal/DefragUnreliableMaxSamples <//CycloneDDS/Domain/Internal/DefragUnreliableMaxSamples>`.

Samples (defragmented if necessary) received out of sequence are buffered:

> **按顺序收到的样品（如有必要，剥落）被缓冲：**

- Initially per proxy writer. The size is limited to: `Internal/PrimaryReorderMaxSamples <//CycloneDDS/Domain/Internal/PrimaryReorderMaxSamples>`.
- Then per Reader (catching up on historical (transient-local) data). The size is limited to: `Internal/SecondaryReorderMaxSamples <//CycloneDDS/Domain/Internal/SecondaryReorderMaxSamples>`.

Receive latency, Receive thread

> 接收延迟，接收线程

## **Minimising receive latency**

In low-latency environments, a few microseconds can be gained by processing the application data either:

> 在低延迟环境中，可以通过处理应用程序数据来获得一些微秒：

- Directly in the receive thread.
- Synchronously, with respect to the incoming network traffic (instead of queueing it for asynchronous processing by a delivery thread).

> - 直接在接收线程中。
> - 同步，就传入的网络流量而言（而不是通过交付线程排队以异步处理）。

This happens for data transmitted where the _max_latency_ QoS is set at a configurable value, and the _transport_priority_ QoS is set to a configurable value. By default, these values are `inf` and the maximum transport priority, effectively enabling synchronous delivery for all data.

> 这发生在 _max_latency_ QoS 设置为可配置值且 _transport_priority_ QoS 设置为可配置值的情况下传输的数据。 默认情况下，这些值为“inf”和最大传输优先级，有效地启用所有数据的同步传输。

Sample size, CDR, Dropping samples

## Maximum sample size

To control the maximum size of samples that the service can process, set: `Internal/MaxSampleSize <//CycloneDDS/Domain/Internal/MaxSampleSize>`. The size is the size of the Common Data Representation (`CDR`) serialised payload, and the limit is the same for both built-in data and application data.

> 为了控制服务可以处理的样品的最大大小，请设置：`nestran/axaMsampleSize <// cyclonedds/domain/domain/internal/maxsamplesize>`。大小是序列化有效载荷的通用数据表示（`cdr'）的大小，并且内置数据和应用程序数据的限制相同。

> [NOTE]:
> The (CDR) serialised payload is never larger than the in-memory representation of the data.
> （CDR）序列化有效载荷永远不会大于数据的内存表示。

When transmitting, samples larger than `Internal/MaxSampleSize <//CycloneDDS/Domain/Internal/MaxSampleSize>` are dropped with a warning. behaves as if the sample never existed.

> 传输时，样品大于内部/maxsamplesize <// cyclonedds/domain/domain/internal/maxsamplesize>`被警告删除。行为好像样本不存在。

Where the transmitting side completely ignores the sample, the receiving side assumes that the sample has been correctly received and acknowledges reception to the writer, which allows communication to continue.

> 如果传输侧完全忽略了样本，则接收端假设已正确接收样本并确认对writer的接收，从而可以继续进行通信。

When receiving, samples larger than `Internal/MaxSampleSize <//CycloneDDS/Domain/Internal/MaxSampleSize>` are dropped as early as possible. To prevent any resources from being claimed for longer than strictly necessary, samples are dropped immediately following the reception of a sample or fragment of one.

> 接收时，大于内部/maxsamplesize 的样品<// cyclonedds/域/内部/maxsamplesize>`尽早删除。为了防止任何资源的要求超过严格必要的要求，在接**受样本或片段后立即删除样品。

> [NOTE**]:
> When the receiving side drops a sample, readers receive a _sample lost_ notification included with the next delivered sample. This notification can be easily overlooked. Therefore, the only reliable way of determining whether samples have been dropped or not is by checking the logs.
> 当接收方丢弃样本时，reader会收到一个 *丢失的样本* 通知，其中包括下一个交付的样本。该通知很容易被忽略。因此，确定样品是否已删除的唯一可靠方法是检查日志。

While dropping samples (or fragments thereof) as early as possible is beneficial from the point of view of reducing resource usage, it can make it hard to decide whether or not dropping a particular sample has been recorded in the log already. Under normal operational circumstances, only a single message is recorded for each sample dropped, but can occasionally report multiple events for the same sample.

> **从减少资源使用情况的角度来看，尽早删除样品（或其片段）是有益的，但它可能很难确定是否已经在日志中记录了删除特定样本。** 在正常操作情况下，仅记录每个样本的一条消息，但偶尔可以报告同一样本的多个事件。

It is possible (but not recommended) to set `Internal/MaxSampleSize <//CycloneDDS/Domain/Internal/MaxSampleSize>` to a very small size (to the point that the discovery data can no longer be communicated).

> 可能（但不建议）设置`Inestran/gaxsamplesize）<// cyclonedds/domain/dominain/internal/maxsamplesize>`尺寸很小（以至于无法再传达发现数据的点）。
