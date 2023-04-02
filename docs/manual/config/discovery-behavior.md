<a href="<https://cunit.sourceforge.net/>" target="\_blank">CUnit</a>
<a href="<https://conan.io/>" target="\_blank">Conan</a>
<a href="<https://www.eclipse.org/legal/ECA.php/>">Eclipse Contributor Agreement</a>
<a href="<https://accounts.eclipse.org/user/eca/>">Eclipse Contributor Agreement</a>
<a href="<https://www.omg.org/spec/DDSI-RTPS>" target="\_blank">DDSI specification</a>
<a href="<https://www.omg.org/spec/DDS/>" target="\_blank">OMG DDS specification</a>
<a href="<https://www.omg.org/spec/DDSI-RTPS/2.1>" target="\_blank">DDSI-RTPS 2.1</a>
<a href="<https://www.omg.org/spec/DDSI-RTPS/2.2>" target="\_blank">DDSI-RTPS 2.2</a>
<a href="<https://www.omg.org/spec/DDSI-RTPS/2.5>" target="\_blank">DDSI-RTPS 2.5</a>
<a href="<https://git-scm.com/>" target="\_blank">Git</a>
<a href="<https://cmake.org/>" target="\_blank">CMake</a>
<a href="<https://www.openssl.org/>" target="\_blank">OpenSSL</a>
<a href="<https://projects.eclipse.org/proposals/eclipse-iceoryx/>" target="\_blank">Eclipse iceoryx</a>
<a href="<https://cunit.sourceforge.net/>" target="\_blank">CUnit</a>
<a href="<https://www.sphinx-doc.org/en/master/>" target="\_blank">Sphinx</a>
<a href="<https://chocolatey.org/>" target="\_blank">chocolatey package manager</a>
<a href="<https://scoop.sh/>" target="\_blank">Scoop</a>
<a href"<https://www.omg.org/spec/IDL/4.2>" target="\_blank">OMG IDL</a>

# Discovery behaviour

## Proxy participants and endpoints

In the , is known as a _stateful_ implementation. Writers only send data to discovered Readers, and Readers only accept data from discovered Writers. There is one exception: the `Writer` may choose to multicast the data so that any Reader is able to receive it. If a Reader has already discovered the Writer but not vice-versa, it can accept the data even though the connection is not fully established.

> 在中，被称为 _状态_ 实施。**writer 只向发现的 reader 发送数据，而 reader 只接受来自发现的 writer 的数据**。**有一个例外：“writer”可以选择多播数据，以便任何 reader 都能接收它**。如果 reader 已经发现了 writer，即使连接未完全建立，它也可以接受数据。

Such asymmetrical discovery can cause data to be delivered when it is not expected, which can also cause indefinite blocking. To avoid this, internally creates a proxy for each remote participant and Reader or Writer. In the discovery process, Writers are matched with proxy Readers, and Readers are matched with proxy Writers, based on the topic name, type name, and the QoS settings.

> **这样的不对称发现可能会导致数据在没有预期的情况下传递，这也可能导致不确定的阻塞**。为了避免这种情况，**内部为每个远程参与者，reader 或 writer 创建一个代理**。在发现过程中，writer 与代理 reader 匹配，reader 与代理 writer 相匹配，该 writer 基于主题名称，类型名称和 QoS 设置。

Proxies have the same natural hierarchy as 'normal' DDSI entities. Each proxy endpoint is owned by a proxy participant. When a proxy participant is deleted, all of its proxy endpoints are also deleted. Participants assert their liveliness periodically, which is known as _automatic_ liveliness in the DCPS specification, and the only mode supported by . When nothing has been heard from a participant for the lease duration (published by that participant in its SPDP message), the lease expires, triggering a clean-up.

> 代理具有与“正常” DDSI 实体相同的自然层次结构。每个代理端点均由代理参与者拥有。当删除代理参与者时，其所有代理端点也将被删除。参与者定期主张他们的活泼性，在 DCPS 规范中被称为 _自动_ 活泼性，并且是支持的唯一模式。**当租赁期限（该参与者在 SPDP 消息中发表）中，参与者没有听到任何听到的消息，租赁就会到期，触发清理。**

Deleting endpoints triggers 'disposes' and 'un-registers' in the SEDP protocol. Deleting a participant also creates special messages that allow the peers to immediately reclaim resources instead of waiting for the lease to expire.

> 删除端点触发了 SEDP 协议中的“处置”和 “Un-Registers”。删除参与者还会创建特殊消息，**使同行立即收回资源**，而不是等待租赁到期。

## Sharing of discovery information

handles any number of participants in an integrated manner, the discovery protocol as described in `proxy_participants_endpoints` can be wasteful. It is not necessary for each participant in a process to run the full discovery protocol for itself.

> 以集成的方式处理任何数量的参与者，“proxy_participant_endpoints”中所述的发现协议可能会浪费。每个参与者都不需要在一个过程中运行自身的完整发现协议。

Instead of implementing the protocol as suggested by the DDSI specification, shares all discovery activities amongst the participants, allowing you to add participants to a process with minimal impact on the system.

> 与其按照 DDSI 规范建议实施协议，不如在参与者之间共享所有发现活动，使您可以将参与者添加到对系统影响最小的过程中。

It is also possible to have a single DDSI participant in a process regardless of the number of DCPS participants created by the application code, which then becomes the virtual owner of all the endpoints created in that one process. There is no discovery penalty for having many participants, but any participant-based liveliness monitoring can be affected.

> 无论应用程序代码创建的 DCPS 参与者的数量如何，也**可以在一个进程中拥有一个 DDSI 参与者，然后它成为在该进程中创建的所有端点的虚拟所有者**。 有很多参与者没有发现惩罚，但任何基于参与者的活跃度监控都会受到影响。

Because other implementations of the DDSI specification may be written on the assumption that all participants perform their own discovery, it is possible to simulate that with . It will not perform the discovery for each participant independently, but it generates the network traffic _as if_ it does. These are controlled by the `Internal/BuiltinEndpointSet <//CycloneDDS/Domain/Internal/BuiltinEndpointSet>` option.

> 由于 DDSI 规范的其他实现可能是在所有参与者执行自己的发现的假设上写的，因此可以使用该发现。它不会独立执行每个参与者的发现，但是它会生成网络流量*，就好像 *一样。这些由“内部/内置点” <// cyclonedds/domain/internal/internal/internInendPointSet>`选项控制。

By sharing the discovery information across all participants in a single node, each new participant or endpoint is immediately aware of the existing peers and can directly communicate with these peers. If these peers take significant time to discover the new participant or endpoint, it can generate some redundant network traffic.

> 通过**在单个节点中的所有参与者之间共享发现信息**，每个新参与者或端点都会立即知道现有的对等点，并可以直接与这些对等点通信。 如果这些对等点花费大量时间来发现新的参与者或端点，它可能会产生一些冗余的网络流量。

## Lingering writers

When an application deletes a reliable DCPS Writer, there is no guarantee that all its Readers have already acknowledged the correct receipt of all samples. lets the Writer (and the owning participant if necessary) linger in the system for some time, controlled by the `Internal/WriterLingerDuration <//CycloneDDS/Domain/Internal/WriterLingerDuration>` option. The Writer is deleted when all Readers have acknowledged all samples, or the linger duration has elapsed, whichever comes first.

> 当申请删除可靠的 DCPS Writer 时，无法保证其所有 Reader 都已确认正确收到所有样本。 让 Writer（和拥有参与者，如有必要）**在系统中逗留一段时间**，由“Internal/WriterLingerDuration <//CycloneDDS/Domain/Internal/WriterLingerDuration>”选项控制。 当所有 Reader 都确认了所有样本，或者延迟持续时间已经过去时，Writer 将被删除，以先到者为准。

> [NOTE]:
> The Writer linger duration setting is not applied when is requested to terminate.
> 当要求终止时，writer 持续时间设置不会应用。

## Writer history QoS and throttling

The relies on the Writer History Cache (WHC), in which a sequence number uniquely identifies each sample. The WHC integrates two different indices on the samples published by a Writer:

> 依赖 writer 历史记录缓存（WHC），其中序列编号唯一地标识了每个样本。WHC 在 writer 发表的样本上集成了两个不同的索引：

- The **sequence number** index is used for re-transmitting lost samples, and is therefore needed for all reliable Writers (see `reliable_coms`).
- The **key value** index is used for retaining the current state of each instance in the WHC.

- **sequence number** 索引用于重新传输丢失的样本，因此所有可靠的 Writer 都需要（参见 `reliable_coms`）。
- **键值**索引用于保留 WHC 中每个实例的当前状态。

When a new sample overwrites the state of an instance, the key value index allows dropping samples from the sequence number index. For transient-local behaviour (see `DDSI-specific transient-local behaviour`), the key value index also allows retaining the current state of each instance even when all Readers have acknowledged a sample.

> 当新样本覆盖实例状态时，键值索引允许从序列号索引中删除样本。 对于瞬态本地行为（请参阅“DDSI 特定的瞬态本地行为”），键值索引还允许保留每个实例的当前状态，即使所有读取器都已确认样本。

Transient-local data always requires the key values index, and by default is also used for other Writers that have a history setting of `KEEP_LAST`. The advantage of an index on key value is that superseded samples can be dropped aggressively, instead of delivering them to all Readers. The disadvantage is that it is somewhat more resource-intensive.

> Transient-local 数据总是需要键值索引，默认情况下也用于历史设置为“KEEP_LAST”的其他 Writer。 **键值索引的优点是可以主动删除被取代的样本，而不是将它们传递给所有 reader。 缺点是它在某种程度上更耗费资源。**

The WHC distinguishes between:

- History to be retained for existing Readers (controlled by the Writer's history QoS setting).
- History to be retained for late-joining readers for transient-local writers (controlled by the topic's durability-service history QoS setting).

- 为现有 reader 保留的历史记录（由编写者的历史记录 QoS 设置控制）。
- 为临时本地写入者的延迟加入 reader 保留历史（由主题的持久性服务历史 QoS 设置控制）。

It is therefore possible to create a Writer that never overwrites samples for live readers, while maintaining only the most recent samples for late-joining readers. This ensures that the data that is available for late-joining readers is the same for transient-local and for transient data.

> 因此，可以**创建一个永远不会为实时 reader 覆盖样本的 Writer，同时只为后来加入的 reader 保留最新的样本**。 这确保了对于延迟加入的读取器可用的数据对于瞬态本地数据和瞬态数据是相同的。

### Writer throttling

Writer throttling is based on the WHC size. The following settings control writer throttling:

> writer 节流基于 WHC 的大小。以下设置控制 writer 节流：

When the WHC contains at least `high` bytes in unacknowledged samples, it stalls the Writer until the number of bytes in unacknowledged samples drops below the value set in: `Internal/Watermarks/WhcLow <//CycloneDDS/Domain/Internal/Watermarks/WhcLow>`.

> 当 WHC 在未确认的样本中至少包含“高”字节时，它会使 Writer 停止，直到未确认样本中的字节数低于以下设置的值：`Internal/Watermarks/WhcLow <//CycloneDDS/Domain/Internal/Watermarks/ WhcLow>`。

Based on the transmit pressure and receive re-ransmit requests, the value of `high` is dynamically adjusted between: - `Internal/Watermarks/WhcLow <//CycloneDDS/Domain/Internal/Watermarks/WhcLow>` - `Internal/Watermarks/WhcHigh <//CycloneDDS/Domain/Internal/Watermarks/WhcHigh>`

> 基于传输压力和接收重传请求，`high` 的值在以下之间动态调整： - `Internal/Watermarks/WhcLow <//CycloneDDS/Domain/Internal/Watermarks/WhcLow>` - `Internal/Watermarks/ WhcHigh <//CycloneDDS/Domain/Internal/Watermarks/WhcHigh>`

The initial value of `high` is set in: `Internal/Watermarks/WhcHighInit <//CycloneDDS/Domain/Internal/Watermarks/WhcHighInit>`.

The adaptive behavior can be disabled by setting `Internal/Watermarks/WhcAdaptive <//CycloneDDS/Domain/Internal/Watermarks/WhcAdaptive>` to `false`.

> 可以通过将“Internal/Watermarks/WhcAdaptive <//CycloneDDS/Domain/Internal/Watermarks/WhcAdaptive>”设置为“false”来禁用自适应行为。

While the adaptive behaviour generally handles a variety of fast and slow writers and readers quite well, the introduction of a very slow reader with small buffers in an existing network that is transmitting data at high rates can cause a sudden stop while the new reader tries to recover the large amount of data stored in the writer, before things can continue at a much lower rate.

> **虽然自适应行为**通常可以很好地处理各种快速和慢速写入器和读取器，但是在以高速率传输数据的现有网络中引入具有小缓冲区的非常慢的读取器可能会导致突然停止，而新读取器试图 恢复写入器中存储的大量数据，然后事情才能以低得多的速度继续进行。
