# Networking interfaces

can use multiple network interfaces simultaneously (the default is a single network interface). The set of enabled interfaces determines the addresses that the host advertises in the discovery information (see `Discovery of participants and endpoints`).

> **可以同时使用多个网络接口**(默认为单个网络接口)。启用的接口集确定主机在发现信息中通告的地址(请参阅“发现参与者和端点”)。

## Default behaviour

To determine the default network interface, the eligible interfaces are ranked by quality, and the interface with the highest quality is selected. If there are multiple interfaces of the highest quality, it selects the first enumerated one. Eligible interfaces are those that are connected and have the correct type of address family (IPv4 or IPv6). Priority is determined (in decreasing priority) as follows:

> 为了确定默认网络接口，**按质量对符合条件的接口进行排名，并选择质量最高的接口**。如果有多个最高质量的接口，它会选择第一个枚举的接口。符合条件的接口是那些已连接并具有正确类型的地址族(IPv4 或 IPv6)的接口。优先级确定如下(优先级递减)：

- Interfaces with a non-link-local address are preferred over those with a link-local one.
- Multicast-capable (see `General/Interfaces/NetworkInterface[@multicast] <//CycloneDDS/Domain/General/Interfaces/NetworkInterface[@multicast]>`).
- Non-multicast capable and not point-to-point.
- Point-to-point.
- Loopback.

> - 与具有链接本地地址的接口相比，**具有非链接本地地址接口是优选的**。
> - 支持多播(请参阅`General/Interfaces/NetworkInterface[@multcast]</CycloneDDS/Domain/General/Interfaces/NetwindowInterface[@moutcast]>`)。
> - 不支持多播且不支持点对点。
> - 点对点。
> - 回环。

If this selection procedure does not automatically return the desired interface, to override the selection, set: `General/Interfaces <//CycloneDDS/Domain/General/Interfaces>` adding any of the following:

> 如果此选择过程没有自动返回所需的接口，要覆盖选择，请设置：“General/Interfaces</CycloneDDS/Domain/General/Interfaces>'添加以下任何内容：

- Name of the interface (`<NetworkInterface name='interface_name' />`).
- IP address of the host on the desired interface (`<NetworkInterface address='128.129.0.42' />`).
- Network portion of the IP address for the host on the desired interface (`<NetworkInterface address='128.11.0.0' />`).

> - 接口的名称(`<NetworkInterface Name='interface_Name'/>`)。
> - 所需接口上主机的 IP 地址(`<NetworkInterface address='128.129.0.42'/>`)。
> - 所需接口上主机 IP 地址的网络部分(`<NetworkInterface address='128.11.0'/>`)。

> [NOTE]:
> An exact match on the address is always preferred, and is the only option that allows selecting the desired interface when multiple addresses are tied to a single interface.
> 地址上的精确匹配始终是首选，并且当多个地址绑定到单个接口时，这是唯一允许选择所需接口的选项。

The default address family is IPv4. To change the address family to IPv6, set: `General/Transport <//CycloneDDS/Domain/General/Transport>` to `udp6` or `tcp6`.

> 默认的地址系列是 IPv4。要将地址系列更改为 IPv6，请将“General/Transport</CycloneDDS/Domain/General/Transport>'”设置为“udp6”或“tcp6”。

> [NOTE]:
> does not mix IPv4 and IPv6 addressing. Therefore, all DDSI participants in the network must use the same addressing mode. When interoperating, this behaviour is the same. That is, it looks at either IPv4 or IPv6 addresses in the advertised address information in the SPDP and SEDP discovery protocols.
> 不混合 IPv4 和 IPv6 寻址。因此，**网络中的所有 DDSI 参与者必须使用相同的寻址模式**。当进行互操作时，这种行为是相同的。也就是说，它在 SPDP 和 SEDP 发现协议中的广告地址信息中查看 IPv4 或 IPv6 地址。

IPv6 link-local addresses are considered undesirable because they must be published and received via the discovery mechanism (see `discovery_behaviour`). There is no way to determine to which interface a received link-local address is related.

> IPv6 链路本地地址被认为是不可取的，因为它们必须通过发现机制发布和接收(请参阅“discovery_behavior”)。无法确定接收到的链路本地地址与哪个接口相关。

If IPv6 is requested and the selected interface has a non-link-local address, operates in a _global addressing_ mode and will only consider discovered non-link-local addresses. In this mode, you can select any set of interfaces for listening to multicasts.

> 如果请求 IPv6，并且所选接口具有非链路本地地址，则在*global addressing*模式下操作，并且只考虑发现的非链接本地地址。在这种模式下，您可以选择任何一组接口来监听多播。

> [NOTE]:
> This behaviour is identical to that when using IPv4, as IPv4 does not have the formal notion of address scopes that IPv6 has. If only a link-local address is available, runs in a _link-local addressing_ mode. In this mode, it accepts any address in a discovery packet (assuming that a link-local address is valid on the selected interface). To minimise the risk involved in this assumption, it only allows the selected interface for listening to multicasts.
> 这种行为与使用 IPv4 时的行为相同，因为 IPv4 没有 IPv6 所具有的地址作用域的正式概念。如果只有链接本地地址可用，则以*link-local addressing*模式运行。在这种模式下，它接受发现数据包中的任何地址(假设链路本地地址在所选接口上有效)。为了最大限度地降低这种假设所涉及的风险，它只允许所选接口监听多播。

## Multiple network interfaces

Multiple network interfaces can be used simultaneously by listing multiple `NetworkInterface <//CycloneDDS/Domain/General/Interfaces/NetworkInterface>` elements. The default behaviour still applies, but with extended network interfaces. For example, the SPDP packets advertise multiple addresses and sends these packets out on all interfaces. If link-local addresses are used, the issue with _link-local addressing_ gains importance.

> 通过列出多个`NetworkInterface</CycloneDDS/Domain/General/interfaces/NetworkInterface>`元素，可以同时使用多个网络接口。默认行为仍然适用，但具有扩展的网络接口。例如，SPDP 分组通告多个地址，并在所有接口上发送这些分组。如果使用链接本地地址，则 _link-local addressing_ 的问题变得更加重要。

In a configuration with a single network interface, it is obvious which one to use for sending packets to a peer. When there are multiple network interfaces, it is necessary to establish the set of interfaces through which multicasts can be sent (these are sent on a specific interface). This in turn requires determining via which subset of interfaces a peer is reachable.

> 在具有单个网络接口的配置中，使用哪一个网络接口向对等方发送数据包是显而易见的。当有多个网络接口时，有必要建立一组接口，通过这些接口可以发送多播(这些多播是在特定接口上发送的)。这反过来又需要确定对等方可以通过哪一个子集的接口进行访问。

checks which interfaces match the addresses advertised by a peer in its SPDP or SEDP messages, which assumes that:

> 检查哪些接口与对等方在其 SPDP 或 SEDP 消息中通告的地址相匹配，这假设：

- The peer is attached to at least one of the configured networks.
- That checking the network parts of the addresses results in a subset of the interfaces.

> - 对等方连接到至少一个配置的网络。
> - 检查地址的网络部分会产生接口的子集。

The network interfaces in this subset are the interfaces on which the peer is assumed to be reachable via multicast. This leaves open two classes of addresses:

> 该子集中的网络接口是假定对等方可以通过多播到达的接口。这留下了两类地址：

- **Loopback addresses**: these are ignored unless:
- The configuration has enabled only loopback interfaces.
- No other addresses are advertised in the discovery message.
- A non-loopback address matches that of the machine.

> - **环回地址**：这些地址将被忽略，除非：
> - 该配置仅启用了环回接口。
> - 在发现消息中没有播发其他地址。
> - 非环回地址与机器的地址相匹配。

- **Routable addresses that do not match an interface**: these are ignored if the `General/DontRoute <//CycloneDDS/Domain/General/DontRoute>` option is set, otherwise it is assumed that the network stack knows how to route them, and any of the interfaces may be used.

> - **与接口不匹配的可路由地址**：如果设置了“General/DontRoute</CycloneDDS/Domain/General/DontRoute>”选项，则会忽略这些地址，否则会认为网络堆栈知道如何路由它们，并且可以使用任何接口。

When a message needs to be sent to a set of peers, uses the set of addresses spanning the set of intended recipients with the lowest cost. That is, the number of nodes that:

> 当需要将消息发送到一组对等方时，使用一组地址，该地址以最低的成本跨越一组预期收件人。也就是说，以下节点的数量：

- Receive it without having a use for it.
- Unicast vs multicast.
- Loopback vs real network interface.
- Configured priority.

> - 在没有使用的情况下接收它。
> - 单播与多播。
> - 环回与真实网络接口。
> - 已配置优先级。

uses some heuristics rather than computing the optimal solution. The address selection can be influenced in two ways:

> 使用一些启发式方法，而不是计算最优解。地址选择可能受到两种方式的影响：

- By using the `priority` attribute, which is used as an offset in the cost calculation. The default configuration gives loopback interfaces a slightly higher priority than other network types.
- By setting the `prefer_multicast` attribute, which raises the assumed cost of a unicast message.

> - 通过使用“优先级”属性，该属性在成本计算中用作偏移量。默认配置为环回接口提供的优先级略高于其他网络类型。
> - 通过设置“preferr_multcast”属性，这会增加单播消息的假定成本。

The `General/RedundantNetworking <//CycloneDDS/Domain/General/RedundantNetworking>` setting forces the address selection code to consider all interfaces advertised by a peer.

> `General/RedundantNetworking</CycloneDDS/Domain/General/RedundantNetworking>`设置强制地址选择代码考虑对等方通告的所有接口。
