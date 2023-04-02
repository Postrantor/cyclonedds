single: Addresses; Unicast single: Addresses; Multicast single: Partition single: Network partition single: DCPS single: Unicast single: Multicast

> single:地址；单播单曲：地址；多播单：分区单：网络分区单：DCPS 单：单播单：多播

# Network partitions

The `Partitioning <//CycloneDDS/Domain/Partitioning>` element in the configuration allows configuring `NetworkPartition <//CycloneDDS/Domain/Partitioning/NetworkPartitions>` elements and mapping topic/partition names to these "network partitions" using `PartitionMappings <//CycloneDDS/Domain/Partitioning/PartitionMappings>` elements.

> 配置中的`Partitioning</CycloneDDS/Domain/Partitioning>`元素允许配置`NetworkPartition</CycloneDDS/Domain/Partitioning/NetworkPartitions>`元素，并使用`PartitionMappings</CycloneDS/Domain/Paartitioning/PartitionMappings>`元件将主题/分区名称映射到这些“网络分区”。

Network partitions introduce alternative multicast addresses for data and/or restrict the set of unicast addresses (that is, interfaces). In the DDSI discovery protocol, a reader can override the addresses at which it is reachable, which is a feature of the discovery protocol that is used to advertise alternative multicast addresses and/or a subset of the unicast addresses. The writers in the network use the addresses advertised by the reader rather than the default addresses advertised by the reader's participant.

> 网络分区为数据引入替代多播地址和/或限制单播地址集（即接口）。在 DDSI 发现协议中，读取器可以覆盖其可到达的地址，这是发现协议的一个特征，用于通告替代多播地址和/或单播地址的子集。网络中的写入程序使用阅读器公布的地址，而不是阅读器参与者公布的默认地址。

Unicast and multicast addresses in a network partition play different roles:

> 网络分区中的单播和多播地址扮演不同的角色：

- The multicast addresses specify an alternative set of addresses to be used instead of the participant's default. This is particularly useful to limit high-bandwidth flows to the parts of a network where the data is needed (for IP/Ethernet, this assumes switches that are configured to do IGMP snooping).

> - **多播地址指定要使用的替代地址集**，而不是参与者的默认地址集。**这对于将高带宽流限制到需要数据的网络部分特别有用**（对于 IP/以太网，这假设配置为执行 IGMP 窥探的交换机）。

- The unicast addresses not only influence the set of interfaces are used for unicast, but thereby also the set of interfaces that are considered for use by multicast. For example: specifying a unicast address that matches network interface A, ensures all traffic to that reader uses interface A, whether unicast or multicast.

> - 单播地址不仅影响用于单播的接口集合，还影响被认为由多播使用的接口集合。例如：指定一个与网络接口 a 匹配的单播地址，确保该读卡器的所有流量都使用接口 a，无论是单播还是多播。

The typical use of unicast addresses is to force traffic onto certain interfaces, the configuration also allows specifying interface names (using the `interface` attribute).

> 单播地址的典型用途是将流量强制到某些接口上，该配置还允许指定接口名称（使用“接口”属性）。

The mapping of a data reader or writer to a network partition is indirect:

> 数据读取器或写入器到网络分区的映射是间接的：

1.  The partition and topic are matched against a table of _partition mappings_, partition/topic combinations to obtain the name of a network partition
2.  The network partition name is used to find the addressing information.

> 1. 将分区和主题与*partition mappings*、分区/主题组合表进行匹配，以获得网络分区的名称
> 2. 网络分区名称用于查找寻址信息。

This makes it easier to map many different partition/topic combinations to the same multicast address without having to specify the actual multicast address many times over. If no match is found, the default addresses are used.

> 这使得将许多不同的分区/主题组合映射到同一个多播地址变得更容易，而不必多次指定实际的多播地址。如果未找到匹配项，则使用默认地址。

The matching sequence is in the order in which the partition mappings are specified in the configuration. The first matching mapping is the one that is used. The `*` and `?` wildcards are available for the DCPS partition/topic combination in the partition mapping.

> 匹配的顺序是在配置中指定分区映射的顺序。第一个匹配映射是所使用的映射。`*` 和 `?` 通配符可用于分区映射中的 DCPS 分区/主题组合。

A single reader or writer is associated with a set of partitions, and each partition/topic combination can potentially map to a different network partition. In this case, the first matching network partition is used. This does not affect the data the reader receives, it only affects the addressing on the network.

> 单个读写器与一组分区相关联，每个分区/主题组合都可能映射到不同的网络分区。在这种情况下，使用第一个匹配的网络分区。这不会影响读取器接收的数据，只会影响网络上的寻址。
