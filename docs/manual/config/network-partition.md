# Network partition configuration

Network partitions have alternative multicast addresses for data that is transmitted via readers and writers that are mapped to the topic under that partition.

> 网络分区具有可供选择的多播地址，用于通过映射到该分区下主题的读取器和写入器传输的数据。

In the DDSI discovery protocol, a reader can override the default address at which it is reachable. This feature of the discovery protocol is used for advertising alternative multicast addresses. The DDSI writers in the network multicast to such an alternative multicast address when multicasting samples or control data.

> 在 DDSI 发现协议中，读取器可以覆盖其可访问的默认地址。发现协议的这一特征用于通告替代多播地址。当多播样本或控制数据时，网络中的 DDSI 写入器多播到这样的替代多播地址。

The mapping of a `DCPS` data reader to a network partition is indirect:

> “DCPS”数据读取器到网络分区的映射是间接的：

1.  To obtain the network partition name, the DCPS partitions and topic are matched against a table of _partition mappings_ (partition/topic combinations).
2.  The network partition name is used to find addressing information. This makes it easier to map many different partition/topic combinations to the same multicast address without specifying the actual multicast address many times over.

> 1. 为了获得网络分区名称，DCPS 分区和主题与*partition mappings*（分区/主题组合）表相匹配。
> 2. 网络分区名称用于查找寻址信息。这使得将许多不同的分区/主题组合映射到同一个多播地址变得更容易，而无需多次指定实际的多播地址。

If no match is found, the default multicast address is used.

> 如果未找到匹配项，则使用默认的多播地址。

## Matching rules

The matching of a DCPS partition/topic combination proceeds in the order in which the partition mappings are specified in the configuration. The first matching mapping is the one that is used. The `*` and `?` wildcards are available for the DCPS partition/topic combination in the partition mapping.

> DCPS 分区/主题组合的匹配按照配置中指定分区映射的顺序进行。第一个匹配映射是所使用的映射。“\*”和“？”通配符可用于分区映射中的 DCPS 分区/主题组合。

can be instructed to ignore all DCPS data readers and writers for certain DCPS partition/topic combinations through the use of `Partitioning/IgnoredPartitions <//CycloneDDS/Domain/Partitioning/IgnoredPartitions>` (see `Endpoint discovery <Endpoint discovery>`). The ignored partitions use the same matching rules as normal mappings, and take precedence over the normal mappings.

> 可以指示通过使用`Partitioning/IgnoredPartitions<//CyconeDDS/Domain/Partitioning/InoredPartitions>`忽略某些 DCPS 分区/主题组合的所有 DCPS 数据读取器和写入器（请参阅`Endpoint discovery<Endpoint discover>`）。被忽略的分区使用与正常映射相同的匹配规则，并且优先于正常映射。

## Multiple matching mappings

A single DCPS data reader can be associated with a set of partitions, and each partition/topic combination can potentially map to different network partitions. The first matching network partition is used.

> 单个 DCPS 数据读取器可以与一组分区相关联，并且每个分区/主题组合可以潜在地映射到不同的网络分区。使用第一个匹配的网络分区。

> [NOTE]:
> This does not affect the data that the reader receives. It only affects the addressing on the network.
> 这不会影响读取器接收到的数据。它只影响网络上的寻址。
