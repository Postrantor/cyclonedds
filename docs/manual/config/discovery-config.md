# Discovery addresses

The DDSI discovery protocols:

- Simple Participant Discovery Protocol (SPDP) for the _domain participants_ (usually operates well without any explicit configuration).
- Simple Endpoint Discovery Protocol (SEDP) for their _endpoints_ (never requires configuration, see `endpoint_discovery`).

> - 用于 _域参与者_ 的简单参与者发现协议(SPDP)(通常在没有任何显式配置的情况下运行良好)。
> - 其 _endpoints_ 的简单端点发现协议(SEDP)(从不需要配置，请参阅“Endpoint_Discovery”)。

For each domain participant, the SPDP protocol periodically sends an SPDP sample to a set of addresses (the default only contains the multicast address):

> **对于每个域参与者，SPDP 协议定期向一组地址发送 SPDP 样本**(默认值仅包含多播地址)：

- IPv4 (`239.255.0.1`)
- IPv6 (`ff02::ffff:239.255.0.1`)

To override the address, set the: `Discovery/SPDPMulticastAddress <//CycloneDDS/Domain/Discovery/SPDPMulticastAddress>` (requires a valid multicast address).

> 要覆盖地址，请设置：`Discovery/SDPPMulticastAddress</CycloneDDS/Domain/Discovery/SDPMulticastAddress>`(需要有效的多播地址)。

In addition (or as an alternative) to the multicast-based discovery, any number of unicast addresses can be configured as 'addresses to be contacted', by specifying peers in: `Discovery/Peers <//CycloneDDS/Domain/Discovery/Peers>`. Each time an SPDP message is sent, it is sent to all of these addresses.

> 除了基于多播的发现之外(或作为一种替代)，**任何数量的单播地址都可以配置为“要联系的地址”**，方法是在`discovery/peers<//CycroneDDS/Domain/discovery/peers>`中指定对等方。每次发送 SPDP 消息时，都会将其发送到所有这些地址。

The default behaviour is to include each IP address several times in the set of addresses (for participant indices 0 through `Discovery/MaxAutoParticipantIndex <//CycloneDDS/Domain/Discovery/MaxAutoParticipantIndex>`). Each IP address then has a different UDP port number, each corresponding to a participant index. Configuring several peers in this way causes a large burst of packets to be sent each time an SPDP message is sent out, and each local DDSI participant causes a burst of its own messages. Because most participant indices are not used, this is wasteful behaviour and is only attractive when it is known that there is a single DDSI process on that node.

> 默认行为是将每个 IP 地址多次包含在地址集中(对于参与者索引 0 到`Discovery/MaxAutoParticipantIndex<//CyconeDDS/Domain/Discovery/MaxAutoParticipantIndex>`)。然后，每个 IP 地址都有一个不同的 UDP 端口号，每个端口号对应一个参与者索引。以这种方式配置几个对等点会导致每次发送 SPDP 消息时发送大量分组，并且每个本地 DDSI 参与者会导致其自己的消息的突发。因为大多数参与者索引都没有被使用，所以这是一种浪费行为，只有当知道该节点上有一个 DDSI 进程时才有吸引力。

clarify the above section.

To avoid sending large numbers of packets to each host (that differ only in port number), add the port number to the IP address (formatted as `IP:PORT`). This requires manually calculating the port number.

> 为了避免向每个主机发送大量数据包(仅端口号不同)，请将端口号添加到 IP 地址(格式为“IP:port”)。这需要手动计算端口号。

To ensure that the configured port number corresponds to the port number that the remote DDSI implementation is listening on, also edit the participant index by setting: `Discovery/ParticipantIndex <//CycloneDDS/Domain/Discovery/ParticipantIndex>` (see the description of "PI" in `port_numbers`).

> 为了确保配置的端口号与远程 DDSI 实现正在侦听的端口号相对应，还可以通过设置`Discovery/PaParticipantIndex<//CyconeDDS/Domain/Discovery/PaPartiparticipantIndex>`来编辑参与者索引(请参阅`port_numbers`中对“PI”的描述)。

# Asymmetrical discovery

On receipt of an SPDP packet, the addresses in the packet are added to the set of addresses to which SPDP packets are periodically sent. For example:

> 在接收到 SPDP 分组时，分组中的地址被添加到 SPDP 数据包被周期性地发送到的地址集。例如：

If SPDP multicasting is disabled entirely:

- Host A has the address of host B in its peer list.
- Host B has an empty peer list.

> 如果 SPDP 多播被完全禁用：
>
> - 主机 A 在其对等列表中具有主机 B 的地址。
> - 主机 B 有一个空的对等列表。

B eventually discovers A because of an SPDP message sent by A, at which point it adds A's address to its own set and starts sending its SPDP message to A, therefore allowing A to discover B. This takes longer than normal multicast based discovery, and risks Writers being blocked by unresponsive Readers.

> 由于 A 发送的 SPDP 消息，B 最终发现了 A，此时它将 A 的地址添加到自己的集合中，并开始将其 SPDP 消息发送到 A，因此允许 A 发现 B。**这比正常的基于多播的发现需要更长的时间，并且有可能导致写入程序被无响应的读卡器阻止**。

# Timing of SPDP packets

To configure the interval with which the SPDP packets are transmitted, set `Discovery/SPDPInterval <//CycloneDDS/Domain/Discovery/SPDPInterval>`.

> 要配置发送 SPDP 数据包的间隔，请设置“Discovery/SDPPInterval</CycloneDDS/Domain/Discovery/SPDPInterval>”。

> [!NOTE]
> A longer interval reduces the network load, but also increases the time discovery takes (especially in the face of temporary network disconnections).
> **较长的间隔会减少网络负载，但也会增加发现所需的时间**(尤其是在面临临时网络断开连接时)。

# Endpoint discovery

Although the SEDP protocol never requires any configuration, network partitioning does interact with it.

> 尽管 SEDP 协议从不需要任何配置，但网络分区确实与之交互。

To completely ignore specific DCPS topics and partition combinations, set the `Partitioning/IgnoredPartitions <//CycloneDDS/Domain/Partitioning/IgnoredPartitions>`. This option prevents data for these topic/partition combinations from being forwarded to and from the network.

> 要完全忽略特定的 DCPS 主题和分区组合，请设置`Partitioning/IgnoredPartitions</CycloneDDS/Domain/Partitioning/InoredPartitions>`。此选项可防止将这些主题/分区组合的数据转发到网络或从网络转发。
