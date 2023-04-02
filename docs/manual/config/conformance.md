Conformance modes, RTI

> [NOTE]: 在 cyclone dds 中 RTI 是什么意思
> 在 Cyclone DDS 中，RTI 是 Real-Time Innovations 的缩写，是一家提供实时数据分发和集成的软件和服务的公司。在 DDS（Data Distribution Service）中，RTI 提供了一些基本组件和工具，帮助开发人员构建可靠、可伸缩、实时的分布式系统。Cyclone DDS 作为一个实现 DDS 标 准的开源实现，使用了 RTI 的一些技术和工具，使其更具可靠性和性能。

# Conformance modes

> [NOTE]: 在 cyclone dds 中 lax 是什么意思
> 在 Cyclone DDS 中，LAX 是一种命名约定，表示将 DDS 遵循 LAX 命名约定的实体（如主题、数据 类型等）与其他非 LAX 实体（如 XML 文件）相区分。具体来说，LAX 意味着实体名称采用下划线分隔，例如"my_topic_name"，而不是使用 camelCase 格式。此外，LAX 还规定了 DDS 实体的命 名范围和作用域。

operates in one of three modes:

- _lax_ (default). The accepts some invalid messages and transmits them, which is important for interoperability.
- _pedantic_ (Depreciated)
- _strict_ (Depreciated)

> - _lax_（默认）。接受一些无效的消息并传输它们，这对于互操作性很重要。

To configure the mode, set `Compatibility/StandardsConformance <//CycloneDDS/Domain/Compatibility/StandardsConformance>`

> [NOTE]:
> If there are two processes with different compliancy modes, the one in the stricter mode will complain about messages sent by the one in the less strict mode.
> 如果有两个具有不同合格模式的过程，那么在更严格的模式下，一个在更严格的模式下的消息将投诉。

## RTI compatibility issues

> [NOTE]: 在 cyclone dds 中 lax 是什么意思@10:00:30
> 在 Cyclone DDS 中，Lax 是一个实验性的、可以用于测试和性能优化的开发者调试工具。它提供了一些有用的功能，如流控统计、API 追踪和 DDS 数据包捕获等，旨在帮助开发人员更好的理解和优化 DDS 应用程序的性能和行为。Lax 是 Cyclone DDS 在开发和测试阶段中的一项重要工具，但并不能作为生产环境中的一个正式特性来使用。需要注意的是，Lax 只能用于在开发和测试环境中收集数据，禁止在实际生产中使用。
> 当使用 Cyclone DDS 开发分布式应用程序时，我们需要确保该应用程序能够可靠地交换数据，并具有高吞吐量和低延迟。此时可以使用 Lax 工具来监测和优化此类程序的性能。
> 举个例子，我们正在开发一个基于 DDS 数据发布/订阅模式的分布式系统，其中有多个节点分别运行着不同的处理器芯片。我们需要确保这些节点的通信是可靠的，并且不会影响整个系统的性能。
> 为了达到这个目标，我们可以使用 Lax 工具来监测执行过程中的数据流量和延迟，并分析它们的表现和性能瓶颈。例如，我们可以使用 Lax 来捕获 DDS 数据包以了解通信的细节，分析发布者和订阅者之间的数据流量是否均衡，检测应用程序中的性能问题等等。
> 通过使用 Lax 工具，我们可以及时发现问题并采取相应的措施来优化应用程序的性能。此外，通过调整 Lax 的配置项，我们还可以监控其他性能指标，如延迟、丢失率、吞吐量等等，以满足不同环境和需求下的数据通信需求。
> 总之，在 Cyclone DDS 的开发过程中，Lax 工具是非常有用的一个调试和性能优化工具，可以帮助我们更好地理解和优化 DDS 应用程序的性能和行为。

In _lax_ mode, most topic types should not have significant issues when working across a network.

> 在 _LAX_ 模式下，大多数主题类型在跨网络工作时不应存在重大问题。

Disposing of data may cause problems, as RTI DDS leaves out the serialised key value and expects the Reader to rely on an embedded hash of the key value. In the strict modes, requires a valid key value to be supplied; in the relaxed mode, it is willing to accept key hash, provided it is of a form that contains the key values in an unmangled form.

> 处理数据可能会导致问题，因为 RTI DDS 遗漏了序列化键值并期望 Reader 依赖键值的嵌入式哈希；
> 在严格模式下，需要提供有效的键值；
> 在宽松模式下，它愿意接受键哈希，前提是它的形式包含未损坏形式的键值。

If an RTI DDS DataWriter disposes of an instance with a key of which the serialised representation may be larger than 16 bytes, this problem is likely to occur. In practice, the most likely cause is using a key as a string, either unbounded, or with a maximum length larger than 11 bytes. See the DDSI specification for details.

> 如果 RTI DDS DataWriter 处置了一个实例，其中串行表示形式可能大于 16 个字节，则可能会发生此问题。实际上，最有可能的原因是使用键作为弦，即无限制或最大长度大于 11 个字节。有关详细信息，请参见 DDSI 规范。

In _strict_ mode, there is interoperation with RTI DDS, but at the cost of very high CPU and network load. This is caused by Heartbeats and AckNacks going back-and-forth between a reliable RTI DDS DataWriter and a reliable data Reader. When informs the RTI Writer that it has received all data (using a valid AckNack message), the RTI Writer immediately publishes a message listing the range of available sequence numbers and requesting an acknowledgment, which becomes an endless loop.

> 在 _strict_ 模式下，**与 RTI DDS 存在互操作，但以非常高的 CPU 和网络负载为代价**。这是由可靠的 RTI DDS DataWriter 和可靠的数据读取器来回传动的心跳和 Acknack 引起的。当通知 RTI 作者已收到所有数据（使用有效的 ACKKNACK 消息）时，RTI Writer 立即发布了一条消息，列出了可用序列编号的范围并请求确认，这成为无尽的循环。

In addition, there is a difference in interpretation of the meaning of the "autodispose_unregistered_instances" QoS on the Writer. aligns with OpenSplice.

> 此外，对作者在“自动估计 unregistered instances” QoS 的含义的解释上存在差异。与 OpenSplice 对齐。
