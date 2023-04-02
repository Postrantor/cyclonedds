Each DCPS domain participant in a domain has a corresponding DDSI participant. These DDSI participants drive the discovery of participants, readers, and writers via the discovery protocols (`discovery_behaviour`). By default, each DDSI participant has a unique address on the network in the form of its own UDP/IP socket with a unique port number.

> **域中的每个 DCPS 域参与者都有一个相应的 DDSI 参与者。** 这些 DDSI 参与者通过发现协议（“discovery_behavior”）推动参与者、读者和 writer 的发现。默认情况下，**每个 DDSI 参与者在网络上都有一个唯一的地址，该地址以其自己的 UDP/IP 套接字的形式存在，并具有唯一的端口号**。

Any DataReader or DataWriter created by a DCPS domain participant has a corresponding DDSI reader or writer (referred to as _endpoints_). However, there is no one-to-one mapping. In DDSI:

> DCPS 域参与者创建的任何 DataReader 或 DataWriter 都有相应的 DDSI 读取器或写入器（称为*endpoints*）。然而，没有一对一的映射。在 DDSI 中：

- a _reader_ is the combination of DCPS DataReader and the DCPS subscriber it belongs to,
- a _writer_ is a combination of DCPS DataWriter and DCPS publisher.

> -*reader*是 DCPS 数据读取器及其所属 DCPS 订户的组合， -*writer*是 DCPS 数据写入器和 DCPS 发布器的组合。

There are no standardized built-in topics for describing DCPS subscribers and publishers. However, there are non-standard extensions that enable implementations to offer `<span class="title-ref">additional</span>` built-in topics to represent these entities and include them in the discovery.

> **没有标准化的内置主题来描述 DCPS 订阅者和发布者。** 然而，有一些非标准扩展使实现能够提供`<span class="title-ref">additional</span>`内置主题来表示这些实体并将它们包括在发现中。

In addition to the application-created readers and writers, DDSI participants have several DDSI built-in endpoints for discovery and liveliness checking/asserting.

> 除了应用程序创建的读取器和写入器之外，DDSI 参与者还有几个 DDSI 内置端点，用于发现和活动性检查/断言。

If there are no corresponding endpoints, a DDSI implementation can exclude a participant. For example, if a participant does not have writers, there is no need to include the DDSI built-in endpoints for describing writers, nor the DDSI built-in endpoint for learning of readers of other participants.

> 如果没有对应的端点，DDSI 实现可以排除参与者。例如，如果一个参与者没有 writer，则不需要包括用于描述 writer 的 DDSI 内置端点，也不需要包含用于学习其他参与者的读者的 DDSI 内建端点。
