# //CycloneDDS

Children: [Domain](#cycloneddsdomain)
CycloneDDS configuration

## //CycloneDDS/Domain

Attributes: [Id](#cycloneddsdomainid)
Children: [Compatibility](#cycloneddsdomaincompatibility), [Discovery](#cycloneddsdomaindiscovery), [General](#cycloneddsdomaingeneral), [Internal](#cycloneddsdomaininternal), [Partitioning](#cycloneddsdomainpartitioning), [SSL](#cycloneddsdomainssl), [Security](#cycloneddsdomainsecurity), [SharedMemory](#cycloneddsdomainsharedmemory), [Sizing](#cycloneddsdomainsizing), [TCP](#cycloneddsdomaintcp), [Threads](#cycloneddsdomainthreads), [Tracing](#cycloneddsdomaintracing)

The General element specifying Domain related settings.

> 指定与域相关设置的 General 元素。

## //CycloneDDS/Domain[@Id]

[Text]: Domain id this configuration applies to, or "any" if it applies to all domain ids.

> 此配置适用于的域 id，如果适用于所有域 id，则为“任意”。

The default value is: `any`

### //CycloneDDS/Domain/Compatibility

Children: [AssumeRtiHasPmdEndpoints](#cycloneddsdomaincompatibilityassumertihaspmdendpoints), [ExplicitlyPublishQosSetToDefault](#cycloneddsdomaincompatibilityexplicitlypublishqossettodefault), [ManySocketsMode](#cycloneddsdomaincompatibilitymanysocketsmode), [StandardsConformance](#cycloneddsdomaincompatibilitystandardsconformance)

The Compatibility element allows you to specify various settings related to compatibility with standards and with other DDSI implementations.

> Compatibility 元素允许您指定与标准和其他 DDSI 实现的兼容性相关的各种设置。

#### //CycloneDDS/Domain/Compatibility/AssumeRtiHasPmdEndpoints

[Boolean]: This option assumes ParticipantMessageData endpoints required by the liveliness protocol are present in RTI participants even when not properly advertised by the participant discovery protocol.

> 该选项假设活跃性协议所需的 ParticipantMessageData 端点存在于 RTI 参与者中，即使参与者发现协议没有正确地通告。

The default value is: `false`

#### //CycloneDDS/Domain/Compatibility/ExplicitlyPublishQosSetToDefault

[Boolean]: This element specifies whether QoS settings set to default values are explicitly published in the discovery protocol. Implementations are to use the default value for QoS settings not published, which allows a significant reduction of the amount of data that needs to be exchanged for the discovery protocol, but this requires all implementations to adhere to the default values specified by the specifications.

> 此元素指定是否在发现协议中显式发布设置为默认值的 QoS 设置。实现将使用未发布的 QoS 设置的默认值，这允许显著减少需要为发现协议交换的数据量，但这要求所有实现都遵守规范指定的默认值。

When interoperability is required with an implementation that does not follow the specifications in this regard, setting this option to true will help.

> 当不遵循这方面规范的实现需要互操作性时，将此选项设置为 true 将有所帮助。

The default value is: `false`

#### //CycloneDDS/Domain/Compatibility/ManySocketsMode

One of: false, true, single, none, many

This option specifies whether a network socket will be created for each domain participant on a host. The specification seems to assume that each participant has a unique address, and setting this option will ensure this to be the case. This is not the default.

> 此选项指定是否为主机上的每个域参与者创建网络套接字。规范似乎假设每个参与者都有一个唯一的地址，设置此选项将确保情况确实如此。这不是默认设置。

Disabling it slightly improves performance and reduces network traffic somewhat. It also causes the set of port numbers needed by Cyclone DDS to become predictable, which may be useful for firewall and NAT configuration.

> 禁用它会略微提高性能，并在一定程度上减少网络流量。它还使 Cyclone DDS 所需的端口号集变得可预测，这可能对防火墙和 NAT 配置有用。

The default value is: `single`

#### //CycloneDDS/Domain/Compatibility/StandardsConformance

One of: lax, strict, pedantic

This element sets the level of standards conformance of this instance of the Cyclone DDS Service. Stricter conformance typically means less interoperability with other implementations. Currently, three modes are defined:

> 此元素设置了 Cyclone DDS 服务实例的标准一致性级别。更严格的一致性通常意味着与其他实现的互操作性更低。目前，定义了三种模式：

- pedantic: very strictly conform to the specification, ultimately for compliance testing, but currently of little value because it adheres even to what will most likely turn out to be editing errors in the DDSI standard. Arguably, as long as no errata have been published, the current text is in effect, and that is what pedantic currently does.

- strict: a relatively less strict view of the standard than does pedantic: it follows the established behaviour where the standard is obviously in error.

- lax: attempt to provide the smoothest possible interoperability, anticipating future revisions of elements in the standard in areas that other implementations do not adhere to, even though there is no good reason not to.

The default value is: `lax`

### //CycloneDDS/Domain/Discovery

Children: [DSGracePeriod](#cycloneddsdomaindiscoverydsgraceperiod), [DefaultMulticastAddress](#cycloneddsdomaindiscoverydefaultmulticastaddress), [EnableTopicDiscoveryEndpoints](#cycloneddsdomaindiscoveryenabletopicdiscoveryendpoints), [ExternalDomainId](#cycloneddsdomaindiscoveryexternaldomainid), [LeaseDuration](#cycloneddsdomaindiscoveryleaseduration), [MaxAutoParticipantIndex](#cycloneddsdomaindiscoverymaxautoparticipantindex), [ParticipantIndex](#cycloneddsdomaindiscoveryparticipantindex), [Peers](#cycloneddsdomaindiscoverypeers), [Ports](#cycloneddsdomaindiscoveryports), [SPDPInterval](#cycloneddsdomaindiscoveryspdpinterval), [SPDPMulticastAddress](#cycloneddsdomaindiscoveryspdpmulticastaddress), [Tag](#cycloneddsdomaindiscoverytag)

The Discovery element allows you to specify various parameters related to the discovery of peers.

> Discovery 元素允许您指定与对等点发现相关的各种参数。

#### //CycloneDDS/Domain/Discovery/DSGracePeriod

[Number-with-unit]: This setting controls for how long endpoints discovered via a Cloud discovery service will survive after the discovery service disappears, allowing reconnection without loss of data when the discovery service restarts (or another instance takes over).

> 此设置控制通过云发现服务发现的端点在发现服务消失后的生存时间，从而允许在发现服务重新启动（或另一个实例接管）时重新连接而不会丢失数据。

Valid values are finite durations with an explicit unit or the keyword 'inf' for infinity. Recognised units: ns, us, ms, s, min, hr, day.

> 有效值是带有显式单位的有限持续时间或表示无穷大的关键字“inf”。公认单位：ns、us、ms、s、min、hr、day。

The default value is: `30 s`

#### //CycloneDDS/Domain/Discovery/DefaultMulticastAddress

[Text]: This element specifies the default multicast address for all traffic other than participant discovery packets. It defaults to Discovery/SPDPMulticastAddress.

> 此元素指定除参与者发现数据包之外的所有流量的默认多播地址。它默认为 Discovery/SDPMulticastAddress。

The default value is: `auto`

#### //CycloneDDS/Domain/Discovery/EnableTopicDiscoveryEndpoints

[Boolean]: This element controls whether the built-in endpoints for topic discovery are created and used to exchange topic discovery information.

> 此元素控制是否创建用于主题发现的内置端点并将其用于交换主题发现信息。

The default value is: `false`

#### //CycloneDDS/Domain/Discovery/ExternalDomainId

[Text]: An override for the domain id is used to discovery and determine the port number mapping. This allows the creating of multiple domains in a single process while making them appear as a single domain on the network. The value "default" disables the override.

> 域 id 的覆盖用于发现和确定端口号映射。这允许在单个进程中创建多个域，同时使它们在网络上显示为单个域。值“default”将禁用覆盖。

The default value is: `default`

#### //CycloneDDS/Domain/Discovery/LeaseDuration

[Number-with-unit]: This setting controls the default participant lease duration.

> 此设置控制默认的参与者租赁期限。

The unit must be specified explicitly. Recognised units: ns, us, ms, s, min, hr, day.

The default value is: `10 s`

#### //CycloneDDS/Domain/Discovery/MaxAutoParticipantIndex

[Integer]: This element specifies the maximum DDSI participant index selected by this instance of the Cyclone DDS service if the Discovery/ParticipantIndex is "auto".

> 如果 Discovery/PacipantIndex 为“auto”，则此元素指定 Cyclone DDS 服务的此实例选择的最大 DDSI 参与者索引。

The default value is: `9`

#### //CycloneDDS/Domain/Discovery/ParticipantIndex

[Text]: This element specifies the DDSI participant index used by this instance of the Cyclone DDS service for discovery purposes. Only one such participant id is used, independent of the number of actual DomainParticipants on the node. It is either:

> 此元素指定 Cyclone DDS 服务的此实例用于发现目的的 DDSI 参与者索引。仅使用一个这样的参与者 id，与节点上实际 DomainParticipants 的数量无关。它是：

- auto: which will attempt to automatically determine an available participant index (see also Discovery/MaxAutoParticipantIndex), or

- a non-negative integer less than 120, or

- none:, which causes it to use arbitrary port numbers for unicast sockets which entirely removes the constraints on the participant index but makes unicast discovery impossible.

The default value is: `none`

#### //CycloneDDS/Domain/Discovery/Peers

Children: [Peer](#cycloneddsdomaindiscoverypeerspeer)

This element statically configures addresses for discovery.

> 此元素静态地配置用于发现的地址。

##### //CycloneDDS/Domain/Discovery/Peers/Peer

Attributes: [Address](#cycloneddsdomaindiscoverypeerspeeraddress)

This element statically configures addresses for discovery.

> 此元素静态地配置用于发现的地址。

##### //CycloneDDS/Domain/Discovery/Peers/Peer[@Address]

[Text]: This element specifies an IP address to which discovery packets must be sent, in addition to the default multicast address (see also General/AllowMulticast). Both hostnames and a numerical IP address are accepted; the hostname or IP address may be suffixed with :PORT to explicitly set the port to which it must be sent. Multiple Peers may be specified.

> 除了默认的多播地址外，此元素还指定必须向其发送发现数据包的 IP 地址（另请参阅常规/AllowMulticast）。主机名和数字 IP 地址都可以接受；主机名或 IP 地址的后缀可以是：PORT，以明确设置必须发送到的端口。可以指定多个对等方。

The default value is: `<empty>`

#### //CycloneDDS/Domain/Discovery/Ports

Children: [Base](#cycloneddsdomaindiscoveryportsbase), [DomainGain](#cycloneddsdomaindiscoveryportsdomaingain), [MulticastDataOffset](#cycloneddsdomaindiscoveryportsmulticastdataoffset), [MulticastMetaOffset](#cycloneddsdomaindiscoveryportsmulticastmetaoffset), [ParticipantGain](#cycloneddsdomaindiscoveryportsparticipantgain), [UnicastDataOffset](#cycloneddsdomaindiscoveryportsunicastdataoffset), [UnicastMetaOffset](#cycloneddsdomaindiscoveryportsunicastmetaoffset)

The Ports element specifies various parameters related to the port numbers used for discovery. These all have default values specified by the DDSI 2.1 specification and rarely need to be changed.

> Ports 元素指定与用于查找的端口号相关的各种参数。这些都有 DDSI 2.1 规范指定的默认值，很少需要更改。

##### //CycloneDDS/Domain/Discovery/Ports/Base

[Integer]: This element specifies the base port number (refer to the DDSI 2.1 specification, section 9.6.1, constant PB).

> 此元素指定基本端口号（请参阅 DDSI 2.1 规范第 9.6.1 节，常数 PB）。

The default value is: `7400`

##### //CycloneDDS/Domain/Discovery/Ports/DomainGain

[Integer]: This element specifies the domain gain, relating domain ids to sets of port numbers (refer to the DDSI 2.1 specification, section 9.6.1, constant DG).

> 该元素指定域增益，将域 id 与端口号集相关联（请参阅 DDSI 2.1 规范第 9.6.1 节，常数 DG）。

The default value is: `250`

##### //CycloneDDS/Domain/Discovery/Ports/MulticastDataOffset

[Integer]: This element specifies the port number for multicast data traffic (refer to the DDSI 2.1 specification, section 9.6.1, constant d2).

> 此元素指定多播数据流量的端口号（请参阅 DDSI 2.1 规范第 9.6.1 节，常数 d2）。

The default value is: `1`

##### //CycloneDDS/Domain/Discovery/Ports/MulticastMetaOffset

[Integer]: This element specifies the port number for multicast meta traffic (refer to the DDSI 2.1 specification, section 9.6.1, constant d0).

> 此元素指定多播元流量的端口号（请参阅 DDSI 2.1 规范第 9.6.1 节，常数 d0）。

The default value is: `0`

##### //CycloneDDS/Domain/Discovery/Ports/ParticipantGain

[Integer]: This element specifies the participant gain, relating p0, participant index to sets of port numbers (refer to the DDSI 2.1 specification, section 9.6.1, constant PG).

> 该元素规定了参与者增益，将 p0、参与者索引与端口号集相关联（参见 DDSI 2.1 规范第 9.6.1 节，常数 PG）。

The default value is: `2`

##### //CycloneDDS/Domain/Discovery/Ports/UnicastDataOffset

[Integer]: This element specifies the port number for unicast data traffic (refer to the DDSI 2.1 specification, section 9.6.1, constant d3).

> 此元素指定单播数据流量的端口号（请参阅 DDSI 2.1 规范第 9.6.1 节，常数 d3）。

The default value is: `11`

##### //CycloneDDS/Domain/Discovery/Ports/UnicastMetaOffset

[Integer]: This element specifies the port number for unicast meta traffic (refer to the DDSI 2.1 specification, section 9.6.1, constant d1).

> 该元素指定单播元流量的端口号（请参阅 DDSI 2.1 规范第 9.6.1 节，常数 d1）。

The default value is: `10`

#### //CycloneDDS/Domain/Discovery/SPDPInterval

[Number-with-unit]: This element specifies the interval between spontaneous transmissions of participant discovery packets.

> 此元素指定参与者发现分组的自发传输之间的间隔。

The unit must be specified explicitly. Recognised units: ns, us, ms, s, min, hr, day.

The default value is: `30 s`

#### //CycloneDDS/Domain/Discovery/SPDPMulticastAddress

[Text]: This element specifies the multicast address used as the destination for the participant discovery packets. In IPv4 mode the default is the (standardised) 239.255.0.1, in IPv6 mode it becomes ff02::ffff:239.255.0.1, which is a non-standardised link-local multicast address.

> 此元素指定用作参与者发现数据包的目的地的多播地址。在 IPv4 模式下，默认值为（标准化）239.255.0.1，在 IPv6 模式下，它变为 ff02:：ffff:239.2555.0.1，这是一个非标准化链路本地多播地址。

The default value is: `239.255.0.1`

#### //CycloneDDS/Domain/Discovery/Tag

[Text]: String extension for domain id that remote participants must match to be discovered.

> 远程参与者必须匹配才能被发现的域 id 的字符串扩展。

The default value is: `<empty>`

### //CycloneDDS/Domain/General

Children: [AllowMulticast](#cycloneddsdomaingeneralallowmulticast), [DontRoute](#cycloneddsdomaingeneraldontroute), [EnableMulticastLoopback](#cycloneddsdomaingeneralenablemulticastloopback), [EntityAutoNaming](#cycloneddsdomaingeneralentityautonaming), [ExternalNetworkAddress](#cycloneddsdomaingeneralexternalnetworkaddress), [ExternalNetworkMask](#cycloneddsdomaingeneralexternalnetworkmask), [FragmentSize](#cycloneddsdomaingeneralfragmentsize), [Interfaces](#cycloneddsdomaingeneralinterfaces), [MaxMessageSize](#cycloneddsdomaingeneralmaxmessagesize), [MaxRexmitMessageSize](#cycloneddsdomaingeneralmaxrexmitmessagesize), [MulticastRecvNetworkInterfaceAddresses](#cycloneddsdomaingeneralmulticastrecvnetworkinterfaceaddresses), [MulticastTimeToLive](#cycloneddsdomaingeneralmulticasttimetolive), [RedundantNetworking](#cycloneddsdomaingeneralredundantnetworking), [Transport](#cycloneddsdomaingeneraltransport), [UseIPv6](#cycloneddsdomaingeneraluseipv)

The General element specifies overall Cyclone DDS service settings.

> General 元素指定了 Cyclone DDS 服务的总体设置。

#### //CycloneDDS/Domain/General/AllowMulticast

One of:

- Keyword: default

> \*关键字：默认

- Comma-separated list of: false, spdp, asm, ssm, true

> \*逗号分隔的列表：false、spdp、asm、ssm、true

This element controls whether Cyclone DDS uses multicasts for data traffic.

> 此元素控制 Cyclone DDS 是否对数据流量使用多播。

It is a comma-separated list of some of the following keywords: "spdp", "asm", "ssm", or either of "false" or "true", or "default".

> 它是以下一些关键字的逗号分隔列表：“spdp”、“asm”、“ssm”，或“false”、“true”或“default”。

- spdp: enables the use of ASM (any-source multicast) for participant discovery, joining the multicast group on the discovery socket, transmitting SPDP messages to this group, but never advertising nor using any multicast address in any discovery message, thus forcing unicast communications for all endpoint discovery and user data.

- asm: enables the use of ASM for all traffic, including receiving SPDP but not transmitting SPDP messages via multicast

- ssm: enables the use of SSM (source-specific multicast) for all non-SPDP traffic (if supported)

When set to "false" all multicasting is disabled. The default, "true" enables the full use of multicasts. Listening for multicasts can be controlled by General/MulticastRecvNetworkInterfaceAddresses.

> 当设置为“false”时，将禁用所有多播。默认情况下，“true”允许充分使用多播。监听多播可以由通用/MulticastRecvNetworkInterfaceAddresses 控制。

"default" maps on spdp if the network is a WiFi network, on true if it is a wired network

> 如果网络是 WiFi 网络，则“默认”映射在 spdp 上，如果是有线网络，则映射在 true 上

The default value is: `default`

#### //CycloneDDS/Domain/General/DontRoute

[Boolean]: This element allows setting the SO_DONTROUTE option for outgoing packets to bypass the local routing tables. This is generally useful only when the routing tables cannot be trusted, which is highly unusual.

> 此元素允许为传出数据包设置 SO_DONTROUTE 选项，以绕过本地路由表。这通常只有在路由表不可信时才有用，这是非常不寻常的。

The default value is: `false`

#### //CycloneDDS/Domain/General/EnableMulticastLoopback

[Boolean]: This element specifies whether Cyclone DDS allows IP multicast packets to be visible to all DDSI participants in the same node, including itself. It must be "true" for intra-node multicast communications. However, if a node runs only a single Cyclone DDS service and does not host any other DDSI-capable programs, it should be set to "false" for improved performance.

> 此元素指定 Cyclone DDS 是否允许 IP 多播数据包对同一节点中的所有 DDSI 参与者可见，包括其自身。对于节点内多播通信，它必须是“true”。但是，如果一个节点只运行一个 Cyclone DDS 服务，并且不托管任何其他具有 DDSI 功能的程序，则应将其设置为“false”以提高性能。

The default value is: `true`

#### //CycloneDDS/Domain/General/EntityAutoNaming

Attributes: [seed](#cycloneddsdomaingeneralentityautonamingseed)

One of: empty, fancy

This element specifies the entity autonaming mode. By default set to 'empty' which means no name will be set (but you can still use dds_qset_entity_name). When set to 'fancy' participants, publishers, subscribers, writers, and readers will get randomly generated names. An autonamed entity will share a 3-letter prefix with their parent entity.

> 此元素指定实体的自动命名模式。默认情况下，设置为“空”，这意味着不会设置任何名称（但您仍然可以使用 dds_qset\entity_name）。当设置为“花式”参与者时，出版商、订阅者、作家和读者将获得随机生成的姓名。自动命名实体将与其父实体共享一个 3 个字母的前缀。

The default value is: `empty`

#### //CycloneDDS/Domain/General/EntityAutoNaming[@seed]

[Text]: Provide an initial seed for the entity naming. Your string will be hashed to provide the random state. When provided, the same sequence of names is generated every run. Creating your entities in the same order will ensure they are the same between runs. If you run multiple nodes, set this via environment variable to ensure every node generates unique names. A random starting seed is chosen when left empty, (the default).

> 为实体命名提供初始种子。您的字符串将被散列以提供随机状态。当提供时，每次运行都会生成相同的名称序列。以相同的顺序创建实体将确保它们在运行之间是相同的。如果运行多个节点，请通过环境变量进行设置，以确保每个节点都生成唯一的名称。当保留为空时，将选择随机起始种子（默认值）。

The default value is: `<empty>`

#### //CycloneDDS/Domain/General/ExternalNetworkAddress

[Text]: This element allows explicitly overruling the network address Cyclone DDS advertises in the discovery protocol, which by default is the address of the preferred network interface (General/NetworkInterfaceAddress), to allow Cyclone DDS to communicate across a Network Address Translation (NAT) device.

> 该元素允许明确否决 Cyclone DDS 在发现协议中通告的网络地址，默认情况下，该地址是首选网络接口的地址（General/NetworkInterfaceAddress），以允许 Cyclone DDS 通过网络地址转换（NAT）设备进行通信。

The default value is: `auto`

#### //CycloneDDS/Domain/General/ExternalNetworkMask

[Text]: This element specifies the network mask of the external network address. This element is relevant only when an external network address (General/ExternalNetworkAddress) is explicitly configured. In this case locators received via the discovery protocol that are within the same external subnet (as defined by this mask) will be translated to an internal address by replacing the network portion of the external address with the corresponding portion of the preferred network interface address. This option is IPv4-only.

> 此元素指定外部网络地址的网络掩码。只有在明确配置了外部网络地址（General/ExternalNetworkAddress）时，此元素才相关。在这种情况下，通过发现协议接收的位于相同外部子网（如该掩码所定义的）内的定位器将通过用优选网络接口地址的相应部分替换外部地址的网络部分而被转换为内部地址。此选项仅适用于 IPv4。

The default value is: `0.0.0.0`

#### //CycloneDDS/Domain/General/FragmentSize

[Number-with-unit]: This element specifies the size of DDSI sample fragments generated by Cyclone DDS. Samples larger than FragmentSize are fragmented into fragments of FragmentSize bytes each, except the last one, which may be smaller. The DDSI spec mandates a minimum fragment size of 1025 bytes, but Cyclone DDS will do whatever size is requested, accepting fragments of which the size is at least the minimum of 1025 and FragmentSize.

> 该元素规定了 DDS 气旋产生的 DDSI 样本碎片的大小。大于 FragmentSize 的样本被分割成每个 FragmentSize 字节的片段，但最后一个可能更小。DDSI 规范要求最小片段大小为 1025 字节，但 Cyclone DDS 将执行任何请求的大小，接受大小至少为 1025 和 FragmentSize 的最小片段。

The unit must be specified explicitly. Recognised units: B (bytes), kB & KiB (2^10 bytes), MB & MiB (2^20 bytes), GB & GiB (2^30 bytes).

The default value is: `1344 B`

#### //CycloneDDS/Domain/General/Interfaces

Children: [NetworkInterface](#cycloneddsdomaingeneralinterfacesnetworkinterface)

This element specifies the network interfaces for use by Cyclone DDS. Multiple interfaces can be specified with an assigned priority. The list in use will be sorted by priority. If interfaces have an equal priority, the specification order will be preserved.

> 此元素指定 Cyclone DDS 使用的网络接口。可以指定多个具有指定优先级的接口。正在使用的列表将按优先级排序。如果接口具有相同的优先级，则将保留规范顺序。

##### //CycloneDDS/Domain/General/Interfaces/NetworkInterface

Attributes: [address](#cycloneddsdomaingeneralinterfacesnetworkinterfaceaddress), [autodetermine](#cycloneddsdomaingeneralinterfacesnetworkinterfaceautodetermine), [multicast](#cycloneddsdomaingeneralinterfacesnetworkinterfacemulticast), [name](#cycloneddsdomaingeneralinterfacesnetworkinterfacename), [prefer_multicast](#cycloneddsdomaingeneralinterfacesnetworkinterfaceprefermulticast), [presence_required](#cycloneddsdomaingeneralinterfacesnetworkinterfacepresencerequired), [priority](#cycloneddsdomaingeneralinterfacesnetworkinterfacepriority)

This element defines a network interface. You can set autodetermine="true" to autoselect the interface CycloneDDS considers the highest quality. If autodetermine="false" (the default), you must specify the name and/or address attribute. If you specify both, they must match the same interface.

> 此元素定义了一个网络接口。您可以设置 autodetermine=“true”来自动选择 CycloneDDS 认为质量最高的接口。如果 autodeterm=“false”（默认值），则必须指定名称和/或地址属性。如果同时指定两者，则它们必须匹配同一接口。

##### //CycloneDDS/Domain/General/Interfaces/NetworkInterface[@address]

[Text]: This attribute specifies the address of the interface. With ipv4 allows matching on the network part if the host part is set to zero.

> 此属性指定接口的地址。如果主机部分设置为零，则使用 ipv4 可以在网络部分进行匹配。

The default value is: `<empty>`

##### //CycloneDDS/Domain/General/Interfaces/NetworkInterface[@autodetermine]

[Text]: If set to "true" an interface is automatically selected. Specifying a name or an address when automatic is set is considered an error.

> 如果设置为“true”，则会自动选择一个界面。设置自动时指定名称或地址被视为错误。

The default value is: `false`

##### //CycloneDDS/Domain/General/Interfaces/NetworkInterface[@multicast]

[Text]: This attribute specifies whether the interface should use multicast. On its default setting, 'default', it will use the value as return by the operating system. If set to 'true', the interface will be assumed to be multicast capable even when the interface flags returned by the operating system state it is not (this provides a workaround for some platforms). If set to 'false', the interface will never be used for multicast.

> 此属性指定接口是否应使用多播。在其默认设置“default”中，它将使用操作系统返回的值。如果设置为“true”，则即使操作系统返回的接口标志表明该接口不支持多播，也会认为该接口支持多播（这为某些平台提供了一种变通方法）。如果设置为“false”，则该接口将永远不会用于多播。
> The default value is: `default`

##### //CycloneDDS/Domain/General/Interfaces/NetworkInterface[@name]

[Text]: This attribute specifies the name of the interface.

> 此属性指定接口的名称。

The default value is: `<empty>`

##### //CycloneDDS/Domain/General/Interfaces/NetworkInterface[@prefer_multicast]

[Boolean]: When false (default), Cyclone DDS uses unicast for data whenever a single unicast suffices. Setting this to true makes it prefer multicasting data, falling back to unicast only when no multicast is available.

> 当为 false（默认值）时，只要一个单播就足够了，Cyclone DDS 就会对数据使用单播。将此设置为 true 会使其更喜欢多播数据，只有在没有多播可用时才返回单播。

The default value is: `false`

##### //CycloneDDS/Domain/General/Interfaces/NetworkInterface[@presence_required]

[Boolean]: By default, all specified network interfaces must be present; if they are missing Cyclone will not start. By explicitly setting this setting for an interface, you can instruct Cyclone to ignore that interface if it is not present.

> 默认情况下，所有指定的网络接口都必须存在；如果他们失踪了，飓风就不会开始了。通过为接口显式设置此设置，您可以指示 Cyclone 在该接口不存在的情况下忽略该接口。

The default value is: `true`

##### //CycloneDDS/Domain/General/Interfaces/NetworkInterface[@priority]

[Text]: This attribute specifies the interface priority (decimal integer or default). The default value for loopback interfaces is 2, for all other interfaces it is 0.

> 此属性指定接口优先级（十进制整数或默认值）。环回接口的默认值是 2，对于所有其他接口，它是 0。

The default value is: `default`

#### //CycloneDDS/Domain/General/MaxMessageSize

[Number-with-unit]: This element specifies the maximum size of the UDP payload that Cyclone DDS will generate. Cyclone DDS will try to maintain this limit within the bounds of the DDSI specification, which means that in some cases (especially for very low values of MaxMessageSize) larger payloads may sporadically be observed (currently up to 1192 B).

> 此元素指定 Cyclone DDS 将生成的 UDP 有效负载的最大大小。Cyclone DDS 将试图将这一限制保持在 DDSI 规范的范围内，这意味着在某些情况下（尤其是对于 MaxMessageSize 的非常低的值），可能会偶尔观察到更大的有效载荷（目前高达 1192B）。

On some networks it may be necessary to set this item to keep the packetsize below the MTU to prevent IP fragmentation.

> 在某些网络上，可能需要设置此项以将数据包大小保持在 MTU 以下，以防止 IP 碎片。

The unit must be specified explicitly. Recognised units: B (bytes), kB & KiB (2^10 bytes), MB & MiB (2^20 bytes), GB & GiB (2^30 bytes).

The default value is: `14720 B`

#### //CycloneDDS/Domain/General/MaxRexmitMessageSize

[Number-with-unit]: This element specifies the maximum size of the UDP payload that Cyclone DDS will generate for a retransmit. Cyclone DDS will try to maintain this limit within the bounds of the DDSI specification, which means that in some cases (especially for very low values) larger payloads may sporadically be observed (currently up to 1192 B).

> 此元素指定 Cyclone DDS 将为重传生成的 UDP 有效载荷的最大大小。旋风 DDS 将试图将这一限制保持在 DDSI 规范的范围内，这意味着在某些情况下（尤其是对于非常低的值），可能会偶尔观察到更大的有效载荷（目前高达 1192B）。

On some networks it may be necessary to set this item to keep the packetsize below the MTU to prevent IP fragmentation.

> 在某些网络上，可能需要设置此项以将数据包大小保持在 MTU 以下，以防止 IP 碎片。

The unit must be specified explicitly. Recognised units: B (bytes), kB & KiB (2^10 bytes), MB & MiB (2^20 bytes), GB & GiB (2^30 bytes).

The default value is: `1456 B`

#### //CycloneDDS/Domain/General/MulticastRecvNetworkInterfaceAddresses

[Text]: This element specifies which network interfaces Cyclone DDS listens to multicasts. The following options are available:

> 此元素指定 Cyclone DDS 侦听多播的网络接口。以下选项可用：

- all: listen for multicasts on all multicast-capable interfaces; or

- any: listen for multicasts on the operating system default interface; or

- preferred: listen for multicasts on the preferred interface (General/Interface/NetworkInterface with the highest priority); or

- none: does not listen for multicasts on any interface; or

- a comma-separated list of network addresses: configures Cyclone DDS to listen for multicasts on all listed addresses.

If Cyclone DDS is in IPv6 mode and the address of the preferred network interface is a link-local address, "all" is treated as a synonym for "preferred" and a comma-separated list is treated as "preferred" if it contains the preferred interface and as "none" if not.

> 如果 Cyclone DDS 处于 IPv6 模式，并且首选网络接口的地址是链路本地地址，则“all”被视为“preferred”的同义词，并且逗号分隔的列表如果包含首选接口则被视为”preferred“，如果不包含，则被视为由”none“。

The default value is: `preferred`

#### //CycloneDDS/Domain/General/MulticastTimeToLive

[Integer]: This element specifies the time-to-live setting for outgoing multicast packets.

> 此元素指定传出多播数据包的生存时间设置。

The default value is: `32`

#### //CycloneDDS/Domain/General/RedundantNetworking

[Boolean]: When enabled, use selected network interfaces in parallel for redundancy.

> 启用时，并行使用选定的网络接口以实现冗余。

The default value is: `false`

#### //CycloneDDS/Domain/General/Transport

One of: default, udp, udp6, tcp, tcp6, raweth

This element allows selecting the transport to be used (udp, udp6, tcp, tcp6, raweth)

> 此元素允许选择要使用的传输（udp、udp6、tcp、tcp6、raweth）

The default value is: `default`

#### //CycloneDDS/Domain/General/UseIPv6

One of: false, true, default

Deprecated (use Transport instead)

> 已弃用（请改用 Transport）

The default value is: `default`

### //CycloneDDS/Domain/Internal

Children: [AccelerateRexmitBlockSize](#cycloneddsdomaininternalacceleraterexmitblocksize), [AckDelay](#cycloneddsdomaininternalackdelay), [AutoReschedNackDelay](#cycloneddsdomaininternalautoreschednackdelay), [BuiltinEndpointSet](#cycloneddsdomaininternalbuiltinendpointset), [BurstSize](#cycloneddsdomaininternalburstsize), [ControlTopic](#cycloneddsdomaininternalcontroltopic), [DefragReliableMaxSamples](#cycloneddsdomaininternaldefragreliablemaxsamples), [DefragUnreliableMaxSamples](#cycloneddsdomaininternaldefragunreliablemaxsamples), [DeliveryQueueMaxSamples](#cycloneddsdomaininternaldeliveryqueuemaxsamples), [EnableExpensiveChecks](#cycloneddsdomaininternalenableexpensivechecks), [GenerateKeyhash](#cycloneddsdomaininternalgeneratekeyhash), [HeartbeatInterval](#cycloneddsdomaininternalheartbeatinterval), [LateAckMode](#cycloneddsdomaininternallateackmode), [LivelinessMonitoring](#cycloneddsdomaininternallivelinessmonitoring), [MaxParticipants](#cycloneddsdomaininternalmaxparticipants), [MaxQueuedRexmitBytes](#cycloneddsdomaininternalmaxqueuedrexmitbytes), [MaxQueuedRexmitMessages](#cycloneddsdomaininternalmaxqueuedrexmitmessages), [MaxSampleSize](#cycloneddsdomaininternalmaxsamplesize), [MeasureHbToAckLatency](#cycloneddsdomaininternalmeasurehbtoacklatency), [MonitorPort](#cycloneddsdomaininternalmonitorport), [MultipleReceiveThreads](#cycloneddsdomaininternalmultiplereceivethreads), [NackDelay](#cycloneddsdomaininternalnackdelay), [PreEmptiveAckDelay](#cycloneddsdomaininternalpreemptiveackdelay), [PrimaryReorderMaxSamples](#cycloneddsdomaininternalprimaryreordermaxsamples), [PrioritizeRetransmit](#cycloneddsdomaininternalprioritizeretransmit), [RediscoveryBlacklistDuration](#cycloneddsdomaininternalrediscoveryblacklistduration), [RetransmitMerging](#cycloneddsdomaininternalretransmitmerging), [RetransmitMergingPeriod](#cycloneddsdomaininternalretransmitmergingperiod), [RetryOnRejectBestEffort](#cycloneddsdomaininternalretryonrejectbesteffort), [SPDPResponseMaxDelay](#cycloneddsdomaininternalspdpresponsemaxdelay), [ScheduleTimeRounding](#cycloneddsdomaininternalscheduletimerounding), [SecondaryReorderMaxSamples](#cycloneddsdomaininternalsecondaryreordermaxsamples), [SocketReceiveBufferSize](#cycloneddsdomaininternalsocketreceivebuffersize), [SocketSendBufferSize](#cycloneddsdomaininternalsocketsendbuffersize), [SquashParticipants](#cycloneddsdomaininternalsquashparticipants), [SynchronousDeliveryLatencyBound](#cycloneddsdomaininternalsynchronousdeliverylatencybound), [SynchronousDeliveryPriorityThreshold](#cycloneddsdomaininternalsynchronousdeliveryprioritythreshold), [Test](#cycloneddsdomaininternaltest), [UnicastResponseToSPDPMessages](#cycloneddsdomaininternalunicastresponsetospdpmessages), [UseMulticastIfMreqn](#cycloneddsdomaininternalusemulticastifmreqn), [Watermarks](#cycloneddsdomaininternalwatermarks), [WriteBatch](#cycloneddsdomaininternalwritebatch), [WriterLingerDuration](#cycloneddsdomaininternalwriterlingerduration)

The Internal elements deal with a variety of settings that are evolving and that are not necessarily fully supported. For the majority of the Internal settings the functionality is supported, but the right to change the way the options control the functionality is reserved. This includes renaming or moving options.

> 内部元素处理各种不断发展的设置，这些设置不一定得到完全支持。对于大多数内部设置，支持该功能，但保留更改选项控制该功能的方式的权利。这包括重命名或移动选项。

#### //CycloneDDS/Domain/Internal/AccelerateRexmitBlockSize

[Integer]: Proxy readers that are assumed to still be retrieving historical data get this many samples retransmitted when they NACK something, even if some of these samples have sequence numbers outside the set covered by the NACK.

> 假设仍在检索历史数据的代理读取器在 NACK 时会重新发送这么多样本，即使其中一些样本的序列号在 NACK 覆盖的集合之外。

The default value is: `0`

#### //CycloneDDS/Domain/Internal/AckDelay

[Number-with-unit]: This setting controls the delay between sending identical acknowledgements.

> 此设置控制发送相同确认之间的延迟。

The unit must be specified explicitly. Recognised units: ns, us, ms, s, min, hr, day.

The default value is: `10 ms`

#### //CycloneDDS/Domain/Internal/AutoReschedNackDelay

[Number-with-unit]: This setting controls the interval with which a reader will continue NACK'ing missing samples in the absence of a response from the writer, as a protection mechanism against writers incorrectly stopping the sending of HEARTBEAT messages.

> 此设置控制在写入程序没有响应的情况下，读取器继续 NACK’ing 丢失样本的时间间隔，作为防止写入程序错误地停止发送心跳消息的保护机制。

Valid values are finite durations with an explicit unit or the keyword 'inf' for infinity. Recognised units: ns, us, ms, s, min, hr, day.

> 有效值是带有显式单位的有限持续时间或表示无穷大的关键字“inf”。公认单位：ns、us、ms、s、min、hr、day。

The default value is: `3 s`

#### //CycloneDDS/Domain/Internal/BuiltinEndpointSet

One of: full, writers, minimal

This element controls which participants will have which built-in endpoints for the discovery and liveliness protocols. Valid values are:

> 该元素控制哪些参与者将具有用于发现和活跃性协议的内置端点。有效值为：

- full: all participants have all endpoints;

- writers: all participants have the writers, but just one has the readers;

- minimal: only one participant has built-in endpoints.

The default is writers, as this is thought to be compliant and reasonably efficient. Minimal may or may not be compliant but is most efficient, and full is inefficient but certain to be compliant.

> 默认为编写器，因为这被认为是符合要求的，并且相当有效。Minimal 可能符合，也可能不符合，但最有效，full 是低效的，但肯定符合。

The default value is: `writers`

#### //CycloneDDS/Domain/Internal/BurstSize

Children: [MaxInitTransmit](#cycloneddsdomaininternalburstsizemaxinittransmit), [MaxRexmit](#cycloneddsdomaininternalburstsizemaxrexmit)

Setting for controlling the size of transmitting bursts.

> 用于控制发送突发的大小的设置。

##### //CycloneDDS/Domain/Internal/BurstSize/MaxInitTransmit

[Number-with-unit]: This element specifies how much more than the (presumed or discovered) receive buffer size may be sent when transmitting a sample for the first time, expressed as a percentage; the remainder will then be handled via retransmits. Usually, the receivers can keep up with the transmitter, at least on average, so generally it is better to hope for the best and recover. Besides, the retransmits will be unicast, and so any multicast advantage will be lost as well.

> 该元素指定在第一次发送样本时，可以发送比（推测或发现的）接收缓冲区大小大多少的数据，以百分比表示；则剩余部分将通过重传进行处理。通常，接收器可以跟上发射器，至少在平均水平上是这样，所以通常最好抱着最好的希望并恢复。此外，重传将是单播的，因此也将失去任何多播优势。

The unit must be specified explicitly. Recognised units: B (bytes), kB & KiB (2^10 bytes), MB & MiB (2^20 bytes), GB & GiB (2^30 bytes).

The default value is: `4294967295`

##### //CycloneDDS/Domain/Internal/BurstSize/MaxRexmit

[Number-with-unit]: This element specifies the amount of data to be retransmitted in response to one NACK.

> 此元素指定响应于一个 NACK 而要重新传输的数据量。

The unit must be specified explicitly. Recognised units: B (bytes), kB & KiB (2^10 bytes), MB & MiB (2^20 bytes), GB & GiB (2^30 bytes).

The default value is: `1 MiB`

#### //CycloneDDS/Domain/Internal/ControlTopic

The ControlTopic element allows configured whether Cyclone DDS provides a special control interface via a predefined topic or not.

> ControlTopic 元素允许配置 Cyclone DDS 是否通过预定义的主题提供特殊的控制接口。

#### //CycloneDDS/Domain/Internal/DefragReliableMaxSamples

[Integer]: This element sets the maximum number of samples that can be defragmented simultaneously for a reliable writer. This has to be large enough to handle retransmissions of historical data in addition to new samples.

> 此元素设置可靠写入程序可以同时进行碎片整理的最大样本数。这必须足够大，以处理除了新样本之外的历史数据的重新传输。

The default value is: `16`

#### //CycloneDDS/Domain/Internal/DefragUnreliableMaxSamples

[Integer]: This element sets the maximum number of samples that can be defragmented simultaneously for best-effort writers.

> 此元素设置可以为尽力而为的写入程序同时进行碎片整理的最大样本数。

The default value is: `4`

#### //CycloneDDS/Domain/Internal/DeliveryQueueMaxSamples

[Integer]: This element controls the maximum size of a delivery queue, expressed in samples. Once a delivery queue is full, incoming samples destined for that queue are dropped until space becomes available again.

> 此元素控制传递队列的最大大小，以示例表示。一旦交付队列已满，则会丢弃指定给该队列的传入样本，直到空间再次可用。

The default value is: `256`

#### //CycloneDDS/Domain/Internal/EnableExpensiveChecks

One of:

- Comma-separated list of: whc, rhc, xevent, all
- Or empty

This element enables expensive checks in builds with assertions enabled and is ignored otherwise. Recognised categories are:

> 此元素在启用断言的构建中启用昂贵的检查，否则将被忽略。公认的类别包括：

- whc: writer history cache checking
- rhc: reader history cache checking
- xevent: xevent checking

In addition, there is the keyword all that enables all checks.

> 此外，还有关键字 all 可以启用所有检查。

The default value is: `<empty>`

#### //CycloneDDS/Domain/Internal/GenerateKeyhash

[Boolean]: When true, include keyhashes in outgoing data for topics with keys.

> 如果为 true，则在带有关键字的主题的传出数据中包含关键字散列。

The default value is: `false`

#### //CycloneDDS/Domain/Internal/HeartbeatInterval

Attributes: [max](#cycloneddsdomaininternalheartbeatintervalmax), [min](#cycloneddsdomaininternalheartbeatintervalmin), [minsched](#cycloneddsdomaininternalheartbeatintervalminsched)

[Number-with-unit]: This element allows configuring the base interval for sending writer heartbeats and the bounds within which it can vary.

> 此元素允许配置发送写入程序检测信号的基本间隔以及它可以变化的范围。

Valid values are finite durations with an explicit unit or the keyword 'inf' for infinity. Recognised units: ns, us, ms, s, min, hr, day.

> 有效值是带有显式单位的有限持续时间或表示无穷大的关键字“inf”。公认单位：ns、us、ms、s、min、hr、day。

The default value is: `100 ms`

#### //CycloneDDS/Domain/Internal/HeartbeatInterval[@max]

[Number-with-unit]: This attribute sets the maximum interval for periodic heartbeats.

> 此属性设置周期性检测信号的最大间隔。

Valid values are finite durations with an explicit unit or the keyword 'inf' for infinity. Recognised units: ns, us, ms, s, min, hr, day.

> 有效值是带有显式单位的有限持续时间或表示无穷大的关键字“inf”。公认单位：ns、us、ms、s、min、hr、day。

The default value is: `8 s`

#### //CycloneDDS/Domain/Internal/HeartbeatInterval[@min]

[Number-with-unit]: This attribute sets the minimum interval that must have passed since the most recent heartbeat from a writer, before another asynchronous (not directly related to writing) will be sent.

> 此属性设置在发送另一个异步（与写入无关）之前，自写入程序的最近一次检测信号以来必须经过的最小间隔。

Valid values are finite durations with an explicit unit or the keyword 'inf' for infinity. Recognised units: ns, us, ms, s, min, hr, day.

> 有效值是带有显式单位的有限持续时间或表示无穷大的关键字“inf”。公认单位：ns、us、ms、s、min、hr、day。

The default value is: `5 ms`

#### //CycloneDDS/Domain/Internal/HeartbeatInterval[@minsched]

[Number-with-unit]: This attribute sets the minimum interval for periodic heartbeats. Other events may still cause heartbeats to go out.

> 此属性设置周期性检测信号的最小间隔。其他事件仍然可能导致心跳停止。

Valid values are finite durations with an explicit unit or the keyword 'inf' for infinity. Recognised units: ns, us, ms, s, min, hr, day.

> 有效值是带有显式单位的有限持续时间或表示无穷大的关键字“inf”。公认单位：ns、us、ms、s、min、hr、day。

The default value is: `20 ms`

#### //CycloneDDS/Domain/Internal/LateAckMode

[Boolean]: Ack a sample only when it has been delivered, instead of when committed to delivering it.

> 只有在样品交付时才确认，而不是在承诺交付时确认。

The default value is: `false`

#### //CycloneDDS/Domain/Internal/LivelinessMonitoring

Attributes: [Interval](#cycloneddsdomaininternallivelinessmonitoringinterval), [StackTraces](#cycloneddsdomaininternallivelinessmonitoringstacktraces)

[Boolean]: This element controls whether or not implementation should internally monitor its own liveliness. If liveliness monitoring is enabled, stack traces can be dumped automatically when some thread appears to have stopped making progress.

> 这个元素控制实现是否应该在内部监视它自己的活动性。如果启用了活动性监视，则当某个线程似乎已停止进程时，可以自动转储堆栈跟踪。

The default value is: `false`

#### //CycloneDDS/Domain/Internal/LivelinessMonitoring[@Interval]

[Number-with-unit]: This element controls the interval to check whether threads have been making progress.

> 此元素控制检查线程是否已取得进展的间隔。

The unit must be specified explicitly. Recognised units: ns, us, ms, s, min, hr, day.

The default value is: `1s`

#### //CycloneDDS/Domain/Internal/LivelinessMonitoring[@StackTraces]

[Boolean]: This element controls whether or not to write stack traces to the DDSI2 trace when a thread fails to make progress (on select platforms only).

> 此元素控制当线程无法取得进展时（仅在选定平台上）是否将堆栈跟踪写入 DDSI2 跟踪。

The default value is: `true`

#### //CycloneDDS/Domain/Internal/MaxParticipants

[Integer]: This elements configures the maximum number of DCPS domain participants this Cyclone DDS instance is willing to service. 0 is unlimited.

> 这些元素配置了此 Cyclone DDS 实例愿意服务的 DCPS 域参与者的最大数量。0 是无限的。

The default value is: `0`

#### //CycloneDDS/Domain/Internal/MaxQueuedRexmitBytes

[Number-with-unit]: This setting limits the maximum number of bytes queued for retransmission. The default value of 0 is unlimited unless an AuxiliaryBandwidthLimit has been set, in which case it becomes NackDelay \* AuxiliaryBandwidthLimit. It must be large enough to contain the largest sample that may need to be retransmitted.

> 此设置限制排队等待重新传输的最大字节数。默认值 0 是不受限制的，除非设置了 AuxiliaryBandwidthLimit，在这种情况下，它将变为 NackDelay\*AuxiliayBandwidthLimited。它必须足够大，以包含可能需要重新传输的最大样本。

The unit must be specified explicitly. Recognised units: B (bytes), kB & KiB (2^10 bytes), MB & MiB (2^20 bytes), GB & GiB (2^30 bytes).

The default value is: `512 kB`

#### //CycloneDDS/Domain/Internal/MaxQueuedRexmitMessages

[Integer]: This setting limits the maximum number of samples queued for retransmission.

> 此设置限制排队等待重新传输的最大样本数。

The default value is: `200`

#### //CycloneDDS/Domain/Internal/MaxSampleSize

[Number-with-unit]: This setting controls the maximum (CDR) serialised size of samples that Cyclone DDS will forward in either direction. Samples larger than this are discarded with a warning.

> 此设置控制 Cyclone DDS 将在任一方向上前进的样本的最大（CDR）串行大小。大于此值的样本将被丢弃，并发出警告。

The unit must be specified explicitly. Recognised units: B (bytes), kB & KiB (2^10 bytes), MB & MiB (2^20 bytes), GB & GiB (2^30 bytes).

The default value is: `2147483647 B`

#### //CycloneDDS/Domain/Internal/MeasureHbToAckLatency

[Boolean]: This element enables heartbeat-to-ack latency among Cyclone DDS services by prepending timestamps to Heartbeat and AckNack messages and calculating round trip times. This is non-standard behaviour. The measured latencies are quite noisy and are currently not used anywhere.

> 此元素通过为 heartbeat 和 AckNack 消息预先设置时间戳并计算往返时间，实现 Cyclone DDS 服务之间的 heartbeat-to-ack 延迟。这是不规范的行为。测量的延迟非常嘈杂，并且目前没有在任何地方使用。

The default value is: `false`

#### //CycloneDDS/Domain/Internal/MonitorPort

[Integer]: This element allows configuring a service that dumps a text description of part the internal state to TCP clients. By default (-1), this is disabled; specifying 0 means a kernel-allocated port is used; a positive number is used as the TCP port number.

> 此元素允许配置一个服务，该服务将部分内部状态的文本描述转储到 TCP 客户端。默认情况下（-1），这是禁用的；指定 0 意味着使用内核分配的端口；使用正数作为 TCP 端口号。

The default value is: `-1`

#### //CycloneDDS/Domain/Internal/MultipleReceiveThreads

Attributes: [maxretries](#cycloneddsdomaininternalmultiplereceivethreadsmaxretries)

One of: false, true, default

This element controls whether all traffic is handled by a single receive thread (false) or whether multiple receive threads may be used to improve latency (true). By default it is disabled on Windows because it appears that one cannot count on being able to send packets to oneself, which is necessary to stop the thread during shutdown. Currently multiple receive threads are only used for connectionless transport (e.g., UDP) and ManySocketsMode not set to single (the default).

> 此元素控制所有流量是由单个接收线程处理（false），还是可以使用多个接收线程来提高延迟（true）。默认情况下，它在 Windows 上被禁用，因为人们似乎无法指望能够向自己发送数据包，这对于在关闭期间停止线程是必要的。目前，多个接收线程仅用于无连接传输（例如 UDP），并且 ManySocketsMode 未设置为单个（默认）。

The default value is: `default`

#### //CycloneDDS/Domain/Internal/MultipleReceiveThreads[@maxretries]

[Integer]: Receive threads dedicated to a single socket can only be triggered for termination by sending a packet. Reception of any packet will do, so termination failure due to packet loss is exceedingly unlikely, but to eliminate all risks, it will retry as many times as specified by this attribute before aborting.

> 专用于单个套接字的接收线程只能通过发送数据包来触发终止。可以接收任何数据包，因此由于数据包丢失而导致终止失败的可能性极低，但为了消除所有风险，它将在中止之前重试此属性指定的次数。

The default value is: `4294967295`

#### //CycloneDDS/Domain/Internal/NackDelay

[Number-with-unit]: This setting controls the delay between receipt of a HEARTBEAT indicating missing samples and a NACK (ignored when the HEARTBEAT requires an answer). However, no NACK is sent if a NACK had been scheduled already for a response earlier than the delay requests: then that NACK will incorporate the latest information.

> 此设置控制接收到指示丢失样本的心跳和 NACK（当心跳需要应答时忽略）之间的延迟。然而，如果 NACK 已经被调度用于比延迟请求更早的响应，则不发送 NACK：那么该 NACK 将包含最新信息。

The unit must be specified explicitly. Recognised units: ns, us, ms, s, min, hr, day.

The default value is: `100 ms`

#### //CycloneDDS/Domain/Internal/PreEmptiveAckDelay

[Number-with-unit]: This setting controls the delay between the discovering a remote writer and sending a pre-emptive AckNack to discover the available range of data.

> 此设置控制发现远程写入程序和发送先发制人的 AckNack 以发现可用数据范围之间的延迟。

The unit must be specified explicitly. Recognised units: ns, us, ms, s, min, hr, day.

The default value is: `10 ms`

#### //CycloneDDS/Domain/Internal/PrimaryReorderMaxSamples

[Integer]: This element sets the maximum size in samples of a primary re-order administration. Each proxy writer has one primary re-order administration to buffer the packet flow in case some packets arrive out of order. Old samples are forwarded to secondary re-order administrations associated with readers needing historical data.

> 此元素设置主要重新订购管理的最大样本大小。每个代理写入程序都有一个主要的重新排序管理，以在某些数据包无序到达时缓冲数据包流。旧样本被转发给与需要历史数据的读者相关联的二级重新订购管理部门。

The default value is: `128`

#### //CycloneDDS/Domain/Internal/PrioritizeRetransmit

[Boolean]: This element controls whether retransmits are prioritized over new data, speeding up recovery.

> 此元素控制重传是否优先于新数据，从而加快恢复速度。

The default value is: `true`

#### //CycloneDDS/Domain/Internal/RediscoveryBlacklistDuration

Attributes: [enforce](#cycloneddsdomaininternalrediscoveryblacklistdurationenforce)

[Number-with-unit]: This element controls for how long a remote participant that was previously deleted will remain on a blacklist to prevent rediscovery, giving the software on a node time to perform any cleanup actions it needs to do. To some extent this delay is required internally by Cyclone DDS, but in the default configuration with the 'enforce' attribute set to false, Cyclone DDS will reallow rediscovery as soon as it has cleared its internal administration. Setting it to too small a value may result in the entry being pruned from the blacklist before Cyclone DDS is ready, it is therefore recommended to set it to at least several seconds.

> 此元素控制之前删除的远程参与者将在黑名单上保留多长时间，以防止重新发现，从而使节点上的软件有时间执行所需的任何清理操作。在某种程度上，Cyclone DDS 内部需要此延迟，但在默认配置中，“强制执行”属性设置为 false，一旦清除了其内部管理，气旋 DDS 就会重新发现。将其设置得太小可能会导致在 Cyclone DDS 准备就绪之前从黑名单中删除条目，因此建议将其设置为至少几秒钟。

Valid values are finite durations with an explicit unit or the keyword 'inf' for infinity. Recognised units: ns, us, ms, s, min, hr, day.

> 有效值是带有显式单位的有限持续时间或表示无穷大的关键字“inf”。公认单位：ns、us、ms、s、min、hr、day。

The default value is: `0s`

#### //CycloneDDS/Domain/Internal/RediscoveryBlacklistDuration[@enforce]

[Boolean]: This attribute controls whether the configured time during which recently deleted participants will not be rediscovered (i.e., "black listed") is enforced and following complete removal of the participant in Cyclone DDS, or whether it can be rediscovered earlier provided all traces of that participant have been removed already.

> 此属性控制在 Cyclone DDS 中完全删除参与者后，是否强制执行最近删除的参与者不会被重新发现的配置时间（即“黑名单”），或者如果该参与者的所有痕迹都已删除，是否可以更早地重新发现。

The default value is: `false`

#### //CycloneDDS/Domain/Internal/RetransmitMerging

One of: never, adaptive, always

This elements controls the addressing and timing of retransmits. Possible values are:

> 这些元件控制重新传输的寻址和定时。可能的值为：

- never: retransmit only to the NACK-ing reader;

- adaptive: attempt to combine retransmits needed for reliability, but send historical (transient-local) data to the requesting reader only;

- always: do not distinguish between different causes, always try to merge.

The default is never. See also Internal/RetransmitMergingPeriod.

> 默认值为 never。另请参阅内部/重新传输合并周期。

The default value is: `never`

#### //CycloneDDS/Domain/Internal/RetransmitMergingPeriod

[Number-with-unit]: This setting determines the time window size in which a NACK of some sample is ignored because a retransmit of that sample has been multicasted too recently. This setting has no effect on unicasted retransmits.

> 此设置确定时间窗口大小，在该时间窗口大小中，由于某个样本的重新传输最近被多播而忽略该样本的 NACK。此设置对单播重传没有影响。

See also Internal/RetransmitMerging.

> 另请参阅内部/重新传输合并。

The unit must be specified explicitly. Recognised units: ns, us, ms, s, min, hr, day.

The default value is: `5 ms`

#### //CycloneDDS/Domain/Internal/RetryOnRejectBestEffort

[Boolean]: Whether or not to locally retry pushing a received best-effort sample into the reader caches when resource limits are reached.

> 当达到资源限制时，是否本地重试将接收到的尽力而为样本推入读取器缓存。

The default value is: `false`

#### //CycloneDDS/Domain/Internal/SPDPResponseMaxDelay

[Number-with-unit]: Maximum pseudo-random delay in milliseconds between discovering aremote participant and responding to it.

> 发现远程参与者和对其做出响应之间的最大伪随机延迟（以毫秒为单位）。

The unit must be specified explicitly. Recognised units: ns, us, ms, s, min, hr, day.

The default value is: `0 ms`

#### //CycloneDDS/Domain/Internal/ScheduleTimeRounding

[Number-with-unit]: This setting allows the timing of scheduled events to be rounded up so that more events can be handled in a single cycle of the event queue. The default is 0 and causes no rounding at all, i.e. are scheduled exactly, whereas a value of 10ms would mean that events are rounded up to the nearest 10 milliseconds.

> 此设置允许对计划事件的时间进行四舍五入，以便在事件队列的单个周期中处理更多事件。默认值为 0，根本不会导致四舍五入，即精确安排，而 10ms 的值意味着事件四舍五舍五入到最接近的 10 毫秒。

The unit must be specified explicitly. Recognised units: ns, us, ms, s, min, hr, day.

The default value is: `0 ms`

#### //CycloneDDS/Domain/Internal/SecondaryReorderMaxSamples

[Integer]: This element sets the maximum size in samples of a secondary re-order administration. The secondary re-order administration is per reader needing historical data.

> 此元素设置二次重新订购管理的最大样本大小。二次重新订购管理是根据需要历史数据的读者进行的。

The default value is: `128`

#### //CycloneDDS/Domain/Internal/SocketReceiveBufferSize

Attributes: [max](#cycloneddsdomaininternalsocketreceivebuffersizemax), [min](#cycloneddsdomaininternalsocketreceivebuffersizemin)

The settings in this element control the size of the socket receive buffers. The operating system provides some size receive buffer upon creation of the socket, this option can be used to increase the size of the buffer beyond that initially provided by the operating system. If the buffer size cannot be increased to the requested minimum size, an error is reported.

> 此元素中的设置控制套接字接收缓冲区的大小。操作系统在创建套接字时提供一定大小的接收缓冲区，此选项可用于将缓冲区的大小增加到操作系统最初提供的大小之外。如果缓冲区大小无法增加到请求的最小大小，则会报告错误。

The default setting requests a buffer size of 1MiB but accepts whatever is available after that.

> 默认设置要求缓冲区大小为 1MiB，但接受之后可用的任何缓冲区。

#### //CycloneDDS/Domain/Internal/SocketReceiveBufferSize[@max]

[Number-with-unit]: This sets the size of the socket receive buffer to request, with the special value of "default" indicating that it should try to satisfy the minimum buffer size. If both are at "default", it will request 1MiB and accept anything. It is ignored if the maximum is set to less than the minimum.

> 这设置了要请求的套接字接收缓冲区的大小，特殊值“default”表示它应该尝试满足最小缓冲区大小。如果两者都处于“默认”，它将请求 1MiB 并接受任何内容。如果将最大值设置为小于最小值，则会忽略此选项。

The unit must be specified explicitly. Recognised units: B (bytes), kB & KiB (2^10 bytes), MB & MiB (2^20 bytes), GB & GiB (2^30 bytes).

The default value is: `default`

#### //CycloneDDS/Domain/Internal/SocketReceiveBufferSize[@min]

[Number-with-unit]: This sets the minimum acceptable socket receive buffer size, with the special value "default" indicating that whatever is available is acceptable.

> 这设置了可接受的最小套接字接收缓冲区大小，特殊值“default”表示任何可用的都是可接受的。

The unit must be specified explicitly. Recognised units: B (bytes), kB & KiB (2^10 bytes), MB & MiB (2^20 bytes), GB & GiB (2^30 bytes).

The default value is: `default`

#### //CycloneDDS/Domain/Internal/SocketSendBufferSize

Attributes: [max](#cycloneddsdomaininternalsocketsendbuffersizemax), [min](#cycloneddsdomaininternalsocketsendbuffersizemin)

The settings in this element control the size of the socket send buffers. The operating system provides some size send buffer upon creation of the socket, this option can be used to increase the size of the buffer beyond that initially provided by the operating system. If the buffer size cannot be increased to the requested minimum size, an error is reported.

> 此元素中的设置控制套接字发送缓冲区的大小。操作系统在创建套接字时提供一定大小的发送缓冲区，此选项可用于将缓冲区的大小增加到操作系统最初提供的大小之外。如果缓冲区大小无法增加到请求的最小大小，则会报告错误。

The default setting requires a buffer of at least 64KiB.

> 默认设置要求缓冲区至少为 64KiB。

#### //CycloneDDS/Domain/Internal/SocketSendBufferSize[@max]

[Number-with-unit]: This sets the size of the socket send buffer to request, with the special value of "default" indicating that it should try to satisfy the minimum buffer size. If both are at "default", it will use whatever is the system default. It is ignored if the maximum is set to less than the minimum.

> 这设置了要请求的套接字发送缓冲区的大小，特殊值“default”表示它应该尝试满足最小缓冲区大小。如果两者都处于“默认值”，它将使用系统默认值。如果将最大值设置为小于最小值，则会忽略此选项。

The unit must be specified explicitly. Recognised units: B (bytes), kB & KiB (2^10 bytes), MB & MiB (2^20 bytes), GB & GiB (2^30 bytes).

The default value is: `default`

#### //CycloneDDS/Domain/Internal/SocketSendBufferSize[@min]

[Number-with-unit]: This sets the minimum acceptable socket send buffer size, with the special value "default" indicating that whatever is available is acceptable.

> 这设置了可接受的最小套接字发送缓冲区大小，特殊值“default”表示任何可用的都是可接受的。

The unit must be specified explicitly. Recognised units: B (bytes), kB & KiB (2^10 bytes), MB & MiB (2^20 bytes), GB & GiB (2^30 bytes).

The default value is: `64 KiB`

#### //CycloneDDS/Domain/Internal/SquashParticipants

[Boolean]: This element controls whether Cyclone DDS advertises all the domain participants it serves in DDSI (when set to false), or rather only one domain participant (the one corresponding to the Cyclone DDS process; when set to true). In the latter case, Cyclone DDS becomes the virtual owner of all readers and writers of all domain participants, dramatically reducing discovery traffic (a similar effect can be obtained by setting Internal/BuiltinEndpointSet to "minimal" but with less loss of information).

> 此元素控制 Cyclone DDS 是否播发其在 DDSI 中服务的所有域参与者（当设置为 false 时），或者仅播发一个域参与者（对应于 Cyclone DDS 进程的域参与者；当设置为 true 时）。在后一种情况下，Cyclone DDS 成为所有域参与者的所有读写器的虚拟所有者，大大减少了发现流量（通过将 Internal/BuiltinEndpointSet 设置为“最小”，可以获得类似的效果，但信息损失较小）。

The default value is: `false`

#### //CycloneDDS/Domain/Internal/SynchronousDeliveryLatencyBound

[Number-with-unit]: This element controls whether samples sent by a writer with QoS settings transport_priority >= SynchronousDeliveryPriorityThreshold and a latency_budget at most this element's value will be delivered synchronously from the "recv" thread, all others will be delivered asynchronously through delivery queues. This reduces latency at the expense of aggregate bandwidth.

> 此元素控制由具有 QoS 设置 transport_priority>=SynchronousDelveryPriorityThreshold 和 latency 的写入程序发送的样本。最多此元素的值将从“recv”线程同步传递，所有其他样本将通过传递队列异步传递。这以牺牲聚合带宽为代价来减少延迟。

Valid values are finite durations with an explicit unit or the keyword 'inf' for infinity. Recognised units: ns, us, ms, s, min, hr, day.

> 有效值是带有显式单位的有限持续时间或表示无穷大的关键字“inf”。公认单位：ns、us、ms、s、min、hr、day。

The default value is: `inf`

#### //CycloneDDS/Domain/Internal/SynchronousDeliveryPriorityThreshold

[Integer]: This element controls whether samples sent by a writer with QoS settings latency_budget <= SynchronousDeliveryLatencyBound and transport_priority greater than or equal to this element's value will be delivered synchronously from the "recv" thread, all others will be delivered asynchronously through delivery queues. This reduces latency at the expense of aggregate bandwidth.

> 此元素控制由 QoS 设置 latency 和 transport 优先级大于或等于此元素值的写入程序发送的样本是否将从“recv”线程同步传递，所有其他样本是否将通过传递队列异步传递。这以牺牲聚合带宽为代价来减少延迟。

The default value is: `0`

#### //CycloneDDS/Domain/Internal/Test

Children: [XmitLossiness](#cycloneddsdomaininternaltestxmitlossiness)

Testing options.

> 测试选项。

##### //CycloneDDS/Domain/Internal/Test/XmitLossiness

[Integer]: This element controls the fraction of outgoing packets to drop, specified as samples per thousand.

> 此元素控制要丢弃的传出数据包的分数，指定为每千个样本。

The default value is: `0`

#### //CycloneDDS/Domain/Internal/UnicastResponseToSPDPMessages

[Boolean]: This element controls whether the response to a newly discovered participant is sent as a unicasted SPDP packet instead of rescheduling the periodic multicasted one. There is no known benefit to setting this to false.

> 该元素控制对新发现的参与者的响应是否作为单播 SPDP 分组发送，而不是重新调度周期性多播的 SPDP 分组。将此设置为 false 没有已知的好处。

The default value is: `true`

#### //CycloneDDS/Domain/Internal/UseMulticastIfMreqn

[Integer]: Do not use.

The default value is: `0`

#### //CycloneDDS/Domain/Internal/Watermarks

Children: [WhcAdaptive](#cycloneddsdomaininternalwatermarkswhcadaptive), [WhcHigh](#cycloneddsdomaininternalwatermarkswhchigh), [WhcHighInit](#cycloneddsdomaininternalwatermarkswhchighinit), [WhcLow](#cycloneddsdomaininternalwatermarkswhclow)

Watermarks for flow-control.

> 用于流量控制的水印。

##### //CycloneDDS/Domain/Internal/Watermarks/WhcAdaptive

[Boolean]: This element controls whether Cyclone DDS will adapt the high-water mark to current traffic conditions based on retransmit requests and transmit pressure.

> 该元件控制 Cyclone DDS 是否会根据重新传输请求和传输压力使高水位线适应当前的交通状况。

The default value is: `true`

##### //CycloneDDS/Domain/Internal/Watermarks/WhcHigh

[Number-with-unit]: This element sets the maximum allowed high-water mark for the Cyclone DDS WHCs, expressed in bytes. A writer is suspended when the WHC reaches this size.

> 此元素设置旋风 DDS WHC 的最大允许高水位标记，以字节表示。当 WHC 达到此大小时，写入程序将被暂停。

The unit must be specified explicitly. Recognised units: B (bytes), kB & KiB (2^10 bytes), MB & MiB (2^20 bytes), GB & GiB (2^30 bytes).

The default value is: `500 kB`

##### //CycloneDDS/Domain/Internal/Watermarks/WhcHighInit

[Number-with-unit]: This element sets the initial level of the high-water mark for the Cyclone DDS WHCs, expressed in bytes.

> 此元素设置旋风 DDS WHCs 的高水位线的初始水位，以字节表示。

The unit must be specified explicitly. Recognised units: B (bytes), kB & KiB (2^10 bytes), MB & MiB (2^20 bytes), GB & GiB (2^30 bytes).

The default value is: `30 kB`

##### //CycloneDDS/Domain/Internal/Watermarks/WhcLow

[Number-with-unit]: This element sets the low-water mark for the Cyclone DDS WHCs, expressed in bytes. A suspended writer resumes transmitting when its Cyclone DDS WHC shrinks to this size.

> 此元素设置 Cyclone DDS WHCs 的低水位标记，以字节表示。当 Cyclone DDS WHC 缩小到这个大小时，挂起的写入程序将恢复传输。

The unit must be specified explicitly. Recognised units: B (bytes), kB & KiB (2^10 bytes), MB & MiB (2^20 bytes), GB & GiB (2^30 bytes).

The default value is: `1 kB`

#### //CycloneDDS/Domain/Internal/WriteBatch

[Boolean]: This element enables the batching of write operations. By default each write operation writes through the write cache and out onto the transport. Enabling write batching causes multiple small write operations to be aggregated within the write cache into a single larger write. This gives greater throughput at the expense of latency. Currently, there is no mechanism for the write cache to automatically flush itself, so that if write batching is enabled, the application may have to use the dds_write_flush function to ensure that all samples are written.

> 此元素启用写入操作的批处理。默认情况下，每个写操作都会通过写缓存进行写入，并输出到传输上。启用写批处理会导致多个较小的写操作在写缓存中聚合为一个较大的写操作。这以牺牲延迟为代价提供了更大的吞吐量。目前，写缓存没有自动刷新自身的机制，因此，如果启用了写批处理，应用程序可能必须使用 dds_write_flush 函数来确保写入所有样本。

The default value is: `false`

#### //CycloneDDS/Domain/Internal/WriterLingerDuration

[Number-with-unit]: This setting controls the maximum duration for which actual deletion of a reliable writer with unacknowledged data in its history will be postponed to provide proper reliable transmission.

> 此设置控制最长持续时间，在该时间内，历史中具有未确认数据的可靠写入程序的实际删除将被推迟，以提供适当的可靠传输。

The unit must be specified explicitly. Recognised units: ns, us, ms, s, min, hr, day.

The default value is: `1 s`

### //CycloneDDS/Domain/Partitioning

Children: [IgnoredPartitions](#cycloneddsdomainpartitioningignoredpartitions), [NetworkPartitions](#cycloneddsdomainpartitioningnetworkpartitions), [PartitionMappings](#cycloneddsdomainpartitioningpartitionmappings)

The Partitioning element specifies Cyclone DDS network partitions and how DCPS partition/topic combinations are mapped onto the network partitions.

> Partitioning 元素指定 Cyclone DDS 网络分区以及 DCPS 分区/主题组合如何映射到网络分区。

#### //CycloneDDS/Domain/Partitioning/IgnoredPartitions

Children: [IgnoredPartition](#cycloneddsdomainpartitioningignoredpartitionsignoredpartition)

The IgnoredPartitions element specifies DCPS partition/topic combinations that are not distributed over the network.

> IgnoredPartitions 元素指定未在网络上分布的 DCPS 分区/主题组合。

##### //CycloneDDS/Domain/Partitioning/IgnoredPartitions/IgnoredPartition

Attributes: [DCPSPartitionTopic](#cycloneddsdomainpartitioningignoredpartitionsignoredpartitiondcpspartitiontopic)

[Text]: This element can prevent certain combinations of DCPS partition and topic from being transmitted over the network. Cyclone DDS will completely ignore readers and writers for which all DCPS partitions as well as their topic is ignored, not even creating DDSI readers and writers to mirror the DCPS ones.

> 该元素可以防止 DCPS 分区和主题的某些组合在网络上传输。Cyclone DDS 将完全忽略所有 DCPS 分区及其主题都被忽略的读写器，甚至不会创建 DDSI 读写器来镜像 DCPS 分区。

The default value is: `<empty>`

##### //CycloneDDS/Domain/Partitioning/IgnoredPartitions/IgnoredPartition[@DCPSPartitionTopic]

[Text]: This attribute specifies a partition and a topic expression, separated by a single '.', which are used to determine if a given partition and topic will be ignored or not. The expressions may use the usual wildcards '\*' and '?'. Cyclone DDS will consider a wildcard DCPS partition to match an expression if a string that satisfies both expressions exists.

> 此属性指定分区和主题表达式，由单个“.”分隔，其用于确定给定分区和主题是否将被忽略。表达式可以使用常用的通配符“\*”和“？”。如果存在满足两个表达式的字符串，Cyclone DDS 将考虑使用通配符 DCPS 分区来匹配表达式。

The default value is: `<empty>`

#### //CycloneDDS/Domain/Partitioning/NetworkPartitions

Children: [NetworkPartition](#cycloneddsdomainpartitioningnetworkpartitionsnetworkpartition)

The NetworkPartitions element specifies the Cyclone DDS network partitions.

> NetworkPartitions 元素指定 Cyclone DDS 网络分区。

##### //CycloneDDS/Domain/Partitioning/NetworkPartitions/NetworkPartition

Attributes: [Address](#cycloneddsdomainpartitioningnetworkpartitionsnetworkpartitionaddress), [Interface](#cycloneddsdomainpartitioningnetworkpartitionsnetworkpartitioninterface), [Name](#cycloneddsdomainpartitioningnetworkpartitionsnetworkpartitionname)

[Text]: This element defines a Cyclone DDS network partition.

> 此元素定义了 Cyclone DDS 网络分区。

The default value is: `<empty>`

##### //CycloneDDS/Domain/Partitioning/NetworkPartitions/NetworkPartition[@Address]

[Text]: This attribute specifies the addresses associated with the network partition as a comma-separated list. The addresses are typically multicast addresses. Non-multicast addresses are allowed, provided the "Interface" attribute is not used: \* An address matching the address or the "external address" (see General/ExternalNetworkAddress; default is the actual address) of a configured interface results in adding the corresponding "external" address to the set of advertised unicast addresses.

> 此属性将与网络分区关联的地址指定为逗号分隔的列表。这些地址通常是多播地址。如果不使用“接口”属性，则允许使用非多播地址：\*与已配置接口的地址或“外部地址”（请参阅常规/ExternalNetworkAddress；默认为实际地址）匹配的地址会将相应的“外部”地址添加到播发的单播地址集。

- An address corresponding to the (external) address of a configured interface, but not the address of the host itself, for example, a match when masking the addresses with the netmask for IPv4, results in adding the external address. For IPv4, this requires the host part to be all-zero.

Readers matching this network partition (cf. Partitioning/PartitionMappings) will advertise all addresses listed to the matching writers via the discovery protocol and will join the specified multicast groups. The writers will select the most suitable address from the addresses advertised by the readers.

> 与此网络分区匹配的读卡器（参见 Partitioning/PartitionMappings）将通过发现协议向匹配的写入程序通告列出的所有地址，并加入指定的多播组。作者将从读者公布的地址中选择最合适的地址。

The unicast addresses advertised by a reader are the only unicast addresses a writer will use to send data to it and are used to select the subset of network interfaces to use for transmitting multicast data with the intent of reaching it.

> 由读取器通告的单播地址是写入器将用于向其发送数据的唯一单播地址，并且用于选择网络接口的子集以用于传输多播数据并意图到达多播数据。

The default value is: `<empty>`

##### //CycloneDDS/Domain/Partitioning/NetworkPartitions/NetworkPartition[@Interface]

[Text]: This attribute takes a comma-separated list of interface name that the reader is willing to receive data on. This is implemented by adding the interface addresses to the set address set configured using the sibling "Address" attribute. See there for more details.

> 此属性采用逗号分隔的接口名称列表，读取器愿意在该列表上接收数据。这是通过将接口地址添加到使用同级“address”属性配置的设置地址集来实现的。请参阅此处了解更多详细信息。

The default value is: `<empty>`

##### //CycloneDDS/Domain/Partitioning/NetworkPartitions/NetworkPartition[@Name]

[Text]: This attribute specifies the name of this Cyclone DDS network partition. Two network partitions cannot have the same name. Partition mappings (cf. Partitioning/PartitionMappings) refer to network partitions using these names.

> 此属性指定此 Cyclone DDS 网络分区的名称。两个网络分区不能具有相同的名称。分区映射（参见 Partitioning/PartitionMappings）指的是使用这些名称的网络分区。

The default value is: `<empty>`

#### //CycloneDDS/Domain/Partitioning/PartitionMappings

Children: [PartitionMapping](#cycloneddsdomainpartitioningpartitionmappingspartitionmapping)

The PartitionMappings element specifies the mapping from DCPS partition/topic combinations to Cyclone DDS network partitions.

> PartitionMappings 元素指定从 DCPS 分区/主题组合到 Cyclone DDS 网络分区的映射。

##### //CycloneDDS/Domain/Partitioning/PartitionMappings/PartitionMapping

Attributes: [DCPSPartitionTopic](#cycloneddsdomainpartitioningpartitionmappingspartitionmappingdcpspartitiontopic), [NetworkPartition](#cycloneddsdomainpartitioningpartitionmappingspartitionmappingnetworkpartition)

[Text]: This element defines a mapping from a DCPS partition/topic combination to a Cyclone DDS network partition. This allows partitioning data flows by using special multicast addresses for part of the data and possibly encrypting the data flow.

> 此元素定义了从 DCPS 分区/主题组合到 Cyclone DDS 网络分区的映射。这允许通过对部分数据使用特殊的多播地址来划分数据流，并可能对数据流进行加密。

The default value is: `<empty>`

##### //CycloneDDS/Domain/Partitioning/PartitionMappings/PartitionMapping[@DCPSPartitionTopic]

[Text]: This attribute specifies a partition and a topic expression, separated by a single '.', which are used to determine if a given partition and topic maps to the Cyclone DDS network partition named by the NetworkPartition attribute in this PartitionMapping element. The expressions may use the usual wildcards '\*' and '?'. Cyclone DDS will consider a wildcard DCPS partition to match an expression if there exists a string that satisfies both expressions.

> 此属性指定分区和主题表达式，由单个“.”分隔，其用于确定给定分区和主题是否映射到由该 PartitionMapping 元素中的 NetworkPartition 属性命名的 Cyclone DDS 网络分区。表达式可以使用常用的通配符“\*”和“？”。如果存在满足两个表达式的字符串，Cyclone DDS 将考虑使用通配符 DCPS 分区来匹配表达式。

The default value is: `<empty>`

##### //CycloneDDS/Domain/Partitioning/PartitionMappings/PartitionMapping[@NetworkPartition]

[Text]: This attribute specifies which Cyclone DDS network partition is to be used for DCPS partition/topic combinations matching the DCPSPartitionTopic attribute within this PartitionMapping element.

> 此属性指定哪个 Cyclone DDS 网络分区将用于与此 PartitionMapping 元素中的 DCPPartitionTopic 属性匹配的 DCPS 分区/主题组合。

The default value is: `<empty>`

### //CycloneDDS/Domain/SSL

Children: [CertificateVerification](#cycloneddsdomainsslcertificateverification), [Ciphers](#cycloneddsdomainsslciphers), [Enable](#cycloneddsdomainsslenable), [EntropyFile](#cycloneddsdomainsslentropyfile), [KeyPassphrase](#cycloneddsdomainsslkeypassphrase), [KeystoreFile](#cycloneddsdomainsslkeystorefile), [MinimumTLSVersion](#cycloneddsdomainsslminimumtlsversion), [SelfSignedCertificates](#cycloneddsdomainsslselfsignedcertificates), [VerifyClient](#cycloneddsdomainsslverifyclient)

The SSL element allows specifying various parameters related to using SSL/TLS for DDSI over TCP.

> SSL 元素允许指定与通过 TCP 对 DDSI 使用 SSL/TLS 相关的各种参数。

#### //CycloneDDS/Domain/SSL/CertificateVerification

[Boolean]: If disabled this allows SSL connections to occur even if an X509 certificate fails verification.

> 如果禁用，则即使 X509 证书验证失败，也可以进行 SSL 连接。

The default value is: `true`

#### //CycloneDDS/Domain/SSL/Ciphers

[Text]: The set of ciphers used by SSL/TLS

> SSL/TLS 使用的密码集

The default value is: `ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH`

#### //CycloneDDS/Domain/SSL/Enable

[Boolean]: This enables SSL/TLS for TCP.

> 这为 TCP 启用了 SSL/TLS。

The default value is: `false`

#### //CycloneDDS/Domain/SSL/EntropyFile

[Text]: The SSL/TLS random entropy file name.

> SSL/TLS 随机熵文件名。

The default value is: `<empty>`

#### //CycloneDDS/Domain/SSL/KeyPassphrase

[Text]: The SSL/TLS key pass phrase for encrypted keys.

> 加密密钥的 SSL/TLS 密钥传递短语。

The default value is: `secret`

#### //CycloneDDS/Domain/SSL/KeystoreFile

[Text]: The SSL/TLS key and certificate store file name. The keystore must be in PEM format.

> SSL/TLS 密钥和证书存储文件名。密钥存储必须采用 PEM 格式。

The default value is: `keystore`

#### //CycloneDDS/Domain/SSL/MinimumTLSVersion

[Text]: The minimum TLS version that may be negotiated, valid values are 1.2 and 1.3.

> 可以协商的最低 TLS 版本，有效值为 1.2 和 1.3。

The default value is: `1.3`

#### //CycloneDDS/Domain/SSL/SelfSignedCertificates

[Boolean]: This enables the use of self signed X509 certificates.

> 这样就可以使用自签名 X509 证书。

The default value is: `false`

#### //CycloneDDS/Domain/SSL/VerifyClient

[Boolean]: This enables an SSL server to check the X509 certificate of a connecting client.

> 这使 SSL 服务器能够检查正在连接的客户端的 X509 证书。

The default value is: `true`

### //CycloneDDS/Domain/Security

Children: [AccessControl](#cycloneddsdomainsecurityaccesscontrol), [Authentication](#cycloneddsdomainsecurityauthentication), [Cryptographic](#cycloneddsdomainsecuritycryptographic)

This element is used to configure Cyclone DDS with the DDS Security specification plugins and settings.

> 此元素用于使用 DDS 安全规范插件和设置配置 Cyclone DDS。

#### //CycloneDDS/Domain/Security/AccessControl

Children: [Governance](#cycloneddsdomainsecurityaccesscontrolgovernance), [Library](#cycloneddsdomainsecurityaccesscontrollibrary), [Permissions](#cycloneddsdomainsecurityaccesscontrolpermissions), [PermissionsCA](#cycloneddsdomainsecurityaccesscontrolpermissionsca)

This element configures the Access Control plugin of the DDS Security specification.

> 此元素配置 DDS 安全规范的访问控制插件。

##### //CycloneDDS/Domain/Security/AccessControl/Governance

[Text]: URI to the shared Governance Document signed by the Permissions CA in S/MIME format

> 权限 CA 以 S/MIME 格式签署的共享治理文档的 URI

URI schemes: file, data<br>

> URI 方案：文件，数据<br>

Examples file URIs:

> 文件 URI 示例：

<Governance>file:governance.smime</Governance>

<Governance>file:/home/myuser/governance.smime</Governance><br>
<Governance><![CDATA[data:,MIME-Version: 1.0

Content-Type: multipart/signed; protocol="application/x-pkcs7-signature"; micalg="sha-256"; boundary="----F9A8A198D6F08E1285A292ADF14DD04F"

> 内容类型：多部分/签名；protocol=“应用程序/x-pkcs7-签名”；micalg=“sha-256”；boundary=“----F9A8A198D6F08E1285A292ADF14DD04F”

This is an S/MIME signed message

> 这是一封 S/MIME 签名邮件

------F9A8A198D6F08E1285A292ADF14DD04F

<?xml version="1.0" encoding="UTF-8"?>

<dds xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"

xsi:noNamespaceSchemaLocation="omg_shared_ca_governance.xsd">

> xsi:noNamespaceSchemaLocation=“omg_shared\ca_governance.xsd”>

<domain_access_rules>

. . .

> . . .

</domain_access_rules>

</dds>

...

> ...

------F9A8A198D6F08E1285A292ADF14DD04F

Content-Type: application/x-pkcs7-signature; name="smime.p7s"

> 内容类型：application/x-pkcs7-签名；name=“假笑.p7s”

Content-Transfer-Encoding: base64

> 内容传输编码：base64

Content-Disposition: attachment; filename="smime.p7s"

> 内容处置：附件；filename=“smime.p7s”

MIIDuAYJKoZIhv ...al5s=

> MIIDuAYJKoZIhv。。。铝 5=

------F9A8A198D6F08E1285A292ADF14DD04F-]]</Governance>

The default value is: `<empty>`

##### //CycloneDDS/Domain/Security/AccessControl/Library

Attributes: [finalizeFunction](#cycloneddsdomainsecurityaccesscontrollibraryfinalizefunction), [initFunction](#cycloneddsdomainsecurityaccesscontrollibraryinitfunction), [path](#cycloneddsdomainsecurityaccesscontrollibrarypath)

[Text]: This element specifies the library to be loaded as the DDS Security Access Control plugin.

> 此元素指定要作为 DDS 安全访问控制插件加载的库。

The default value is: `<empty>`

##### //CycloneDDS/Domain/Security/AccessControl/Library[@finalizeFunction]

[Text]: This element names the finalization function of Access Control plugin. This function is called to let the plugin release its resources.

> 此元素命名访问控制插件的终结函数。调用此函数是为了让插件释放其资源。

The default value is: `finalize\_access\_control`

##### //CycloneDDS/Domain/Security/AccessControl/Library[@initFunction]

[Text]: This element names the initialization function of Access Control plugin. This function is called after loading the plugin library for instantiation purposes. The Init function must return an object that implements the DDS Security Access Control interface.

> 此元素命名访问控制插件的初始化函数。此函数是在加载插件库后调用的，用于实例化。Init 函数必须返回一个实现 DDS 安全访问控制接口的对象。

The default value is: `init\_access\_control`

##### //CycloneDDS/Domain/Security/AccessControl/Library[@path]

[Text]: This element points to the path of Access Control plugin library.

> 此元素指向访问控制插件库的路径。

It can be either absolute path excluding file extension ( /usr/lib/dds_security_ac ) or single file without extension ( dds_security_ac ).

> 它可以是不包括文件扩展名的绝对路径（/usr/lib/dds_security\ac），也可以是没有扩展名的单个文件（dds_security_ac）。

If a single file is supplied, the library is located by the current working directory, or LD_LIBRARY_PATH for Unix systems, and PATH for Windows systems.

> 如果提供了单个文件，则库按当前工作目录定位，对于 Unix 系统，则按 LD_library_PATH 定位，对于 Windows 系统，则按照 PATH 定位。

The default value is: `dds\_security\_ac`

##### //CycloneDDS/Domain/Security/AccessControl/Permissions

[Text]: URI to the DomainParticipant permissions document signed by the Permissions CA in S/MIME format

> 权限 CA 以 S/MIME 格式签署的 DomainParticipant 权限文档的 URI

The permissions document specifies the permissions to be applied to a domain.<br>

> 权限文档指定要应用于域的权限<br>

Example file URIs:

> 示例文件 URI：

<Permissions>file:permissions_document.p7s</Permissions>

<Permissions>file:/path_to/permissions_document.p7s</Permissions>

Example data URI:

> 示例数据 URI:

<Permissions><![CDATA[data:,.........]]</Permissions>

The default value is: `<empty>`

##### //CycloneDDS/Domain/Security/AccessControl/PermissionsCA

[Text]: URI to an X509 certificate for the PermissionsCA in PEM format.

> PEM 格式的 PermissionsCA 的 X509 证书的 URI。

Supported URI schemes: file, data

> 支持的 URI 方案：文件、数据

The file and data schemas shall refer to a X.509 v3 certificate (see X.509 v3 ITU-T Recommendation X.509 (2005) [39]) in PEM format.<br>

> 文件和数据模式应参考 PEM 格式的 X.509 v3 证书（见 X.509 v3 ITU-T 建议 X.509（2005）[39]）<br>

Examples:<br>

> 示例：<br> > <PermissionsCA>file:permissions_ca.pem</PermissionsCA>

<PermissionsCA>file:/home/myuser/permissions_ca.pem</PermissionsCA><br>
<PermissionsCA>data:<strong>,</strong>-----BEGIN CERTIFICATE-----

MIIC3DCCAcQCCQCWE5x+Z ... PhovK0mp2ohhRLYI0ZiyYQ==

> MIIC3DCCAcQCCQCWE5x+Z。。。PhovK0mp2ohhRLYI0ZiyYQ 型==

-----END CERTIFICATE-----</PermissionsCA>

The default value is: `<empty>`

#### //CycloneDDS/Domain/Security/Authentication

Children: [CRL](#cycloneddsdomainsecurityauthenticationcrl), [IdentityCA](#cycloneddsdomainsecurityauthenticationidentityca), [IdentityCertificate](#cycloneddsdomainsecurityauthenticationidentitycertificate), [IncludeOptionalFields](#cycloneddsdomainsecurityauthenticationincludeoptionalfields), [Library](#cycloneddsdomainsecurityauthenticationlibrary), [Password](#cycloneddsdomainsecurityauthenticationpassword), [PrivateKey](#cycloneddsdomainsecurityauthenticationprivatekey), [TrustedCADirectory](#cycloneddsdomainsecurityauthenticationtrustedcadirectory)

This element configures the Authentication plugin of the DDS Security specification.

> 此元素配置 DDS 安全规范的身份验证插件。

##### //CycloneDDS/Domain/Security/Authentication/CRL

[Text]: Optional URI to load an X509 Certificate Revocation List

> 加载 X509 证书吊销列表的可选 URI

Supported URI schemes: file, data

> 支持的 URI 方案：文件、数据

Examples:

> 示例：

<CRL>file:crl.pem</CRL>

<CRL>data:,-----BEGIN X509 CRL-----<br>

MIIEpAIBAAKCAQEA3HIh...AOBaaqSV37XBUJg=<br>

> MIIEpAIBAAKCAQEA3HIh。。。AOBaaqSV37XBUJg=<br>
> -----END X509 CRL-----</CRL>

The default value is: `<empty>`

##### //CycloneDDS/Domain/Security/Authentication/IdentityCA

[Text]: URI to the X509 certificate [39] of the Identity CA that is the signer of Identity Certificate.

> 作为身份证书签名者的身份 CA 的 X509 证书[39]的 URI。

Supported URI schemes: file, data

> 支持的 URI 方案：文件、数据

The file and data schemas shall refer to a X.509 v3 certificate (see X.509 v3 ITU-T Recommendation X.509 (2005) [39]) in PEM format.

> 文件和数据模式应参考 PEM 格式的 X.509 v3 证书（见 X.509 v3 ITU-T 建议 X.509（2005）[39]）。

Examples:

> 示例：

<IdentityCA>file:identity_ca.pem</IdentityCA>

<IdentityCA>data:,-----BEGIN CERTIFICATE-----<br>

MIIC3DCCAcQCCQCWE5x+Z...PhovK0mp2ohhRLYI0ZiyYQ==<br>

> MIIC3DCCAcQCCQCWE5x+Z。。。PhovK0mp2ohhRLYI0ZiyYQ==<br>
> -----END CERTIFICATE-----</IdentityCA>

The default value is: `<empty>`

##### //CycloneDDS/Domain/Security/Authentication/IdentityCertificate

[Text]: An identity certificate will identify all participants in the OSPL instance.<br>The content is URI to an X509 certificate signed by the IdentityCA in PEM format containing the signed public key.

> 身份证书将识别 OSPL 实例中的所有参与者<br>内容是 IdentityCA 以 PEM 格式签署的 X509 证书的 URI，该证书包含已签名的公钥。

Supported URI schemes: file, data

> 支持的 URI 方案：文件、数据

Examples:

> 示例：

<IdentityCertificate>file:participant1_identity_cert.pem</IdentityCertificate>

<IdentityCertificate>data:,-----BEGIN CERTIFICATE-----<br>

MIIDjjCCAnYCCQDCEu9...6rmT87dhTo=<br>

> MIIDjjCCAnYCCQDCEu9…6rmT87dhTo=<br>
> -----END CERTIFICATE-----</IdentityCertificate>

The default value is: `<empty>`

##### //CycloneDDS/Domain/Security/Authentication/IncludeOptionalFields

[Boolean]: The authentication handshake tokens may contain optional fields to be included for finding interoperability problems. If this parameter is set to true the optional fields are included in the handshake token exchange.

> 认证握手令牌可以包含用于发现互操作性问题的可选字段。如果将此参数设置为 true，则握手令牌交换中将包括可选字段。

The default value is: `false`

##### //CycloneDDS/Domain/Security/Authentication/Library

Attributes: [finalizeFunction](#cycloneddsdomainsecurityauthenticationlibraryfinalizefunction), [initFunction](#cycloneddsdomainsecurityauthenticationlibraryinitfunction), [path](#cycloneddsdomainsecurityauthenticationlibrarypath)

[Text]: This element specifies the library to be loaded as the DDS Security Access Control plugin.

> 此元素指定要作为 DDS 安全访问控制插件加载的库。

The default value is: `<empty>`

##### //CycloneDDS/Domain/Security/Authentication/Library[@finalizeFunction]

[Text]: This element names the finalization function of the Authentication plugin. This function is called to let the plugin release its resources.

> 此元素命名身份验证插件的终结函数。调用此函数是为了让插件释放其资源。

The default value is: `finalize\_authentication`

##### //CycloneDDS/Domain/Security/Authentication/Library[@initFunction]

[Text]: This element names the initialization function of the Authentication plugin. This function is called after loading the plugin library for instantiation purposes. The Init function must return an object that implements the DDS Security Authentication interface.

> 此元素命名身份验证插件的初始化函数。此函数是在加载插件库后调用的，用于实例化。Init 函数必须返回一个实现 DDS 安全身份验证接口的对象。

The default value is: `init\_authentication`

##### //CycloneDDS/Domain/Security/Authentication/Library[@path]

[Text]: This element points to the path of the Authentication plugin library.

> 此元素指向身份验证插件库的路径。

It can be either absolute path excluding file extension ( /usr/lib/dds_security_auth ) or single file without extension ( dds_security_auth ).

> 它可以是不包括文件扩展名的绝对路径（/usr/lib/dds_security\auth），也可以是没有扩展名的单个文件（dds_security_auth）。

If a single file is supplied, the library is located by the current working directory, or LD_LIBRARY_PATH for Unix systems, and PATH for Windows systems.

> 如果提供了单个文件，则库按当前工作目录定位，对于 Unix 系统，则按 LD_library_PATH 定位，对于 Windows 系统，则按照 PATH 定位。

The default value is: `dds\_security\_auth`

##### //CycloneDDS/Domain/Security/Authentication/Password

[Text]: A password is used to decrypt the private_key.

> 密码用于解密私钥。

The value of the password property shall be interpreted as the Base64 encoding of the AES-128 key that shall be used to decrypt the private_key using AES128-CBC.

> 密码属性的值应解释为 AES-128 密钥的 Base64 编码，该密钥应用于使用 AES128-CBC 解密私钥。

If the password property is not present, then the value supplied in the private_key property must contain the unencrypted private key.

> 如果密码属性不存在，那么 private_key 属性中提供的值必须包含未加密的私钥。

The default value is: `<empty>`

##### //CycloneDDS/Domain/Security/Authentication/PrivateKey

[Text]: URI to access the private Private Key for all of the participants in the OSPL federation.

> 用于访问 OSPL 联盟中所有参与者的私有私钥的 URI。

Supported URI schemes: file, data

> 支持的 URI 方案：文件、数据

Examples:

> 示例：

<PrivateKey>file:identity_ca_private_key.pem</PrivateKey>

<PrivateKey>data:,-----BEGIN RSA PRIVATE KEY-----<br>

MIIEpAIBAAKCAQEA3HIh...AOBaaqSV37XBUJg==<br>

> MIIEpAIBAAKCAQEA3HIh。。。AOBaaqSV37XBUJg==<br>
> -----END RSA PRIVATE KEY-----</PrivateKey>

The default value is: `<empty>`

##### //CycloneDDS/Domain/Security/Authentication/TrustedCADirectory

[Text]: Trusted CA Directory which contains trusted CA certificates as separated files.

> 受信任的 CA 目录，其中包含作为分离文件的受信任 CA 证书。

The default value is: `<empty>`

#### //CycloneDDS/Domain/Security/Cryptographic

Children: [Library](#cycloneddsdomainsecuritycryptographiclibrary)

This element configures the Cryptographic plugin of the DDS Security specification.

> 此元素配置 DDS 安全规范的加密插件。

##### //CycloneDDS/Domain/Security/Cryptographic/Library

Attributes: [finalizeFunction](#cycloneddsdomainsecuritycryptographiclibraryfinalizefunction), [initFunction](#cycloneddsdomainsecuritycryptographiclibraryinitfunction), [path](#cycloneddsdomainsecuritycryptographiclibrarypath)

[Text]: This element specifies the library to be loaded as the DDS Security Cryptographic plugin.

> 此元素指定要作为 DDS 安全加密插件加载的库。

The default value is: `<empty>`

##### //CycloneDDS/Domain/Security/Cryptographic/Library[@finalizeFunction]

[Text]: This element names the finalization function of the Cryptographic plugin. This function is called to let the plugin release its resources.

> 此元素命名 Cryptographic 插件的终结函数。调用此函数是为了让插件释放其资源。

The default value is: `finalize\_crypto`

##### //CycloneDDS/Domain/Security/Cryptographic/Library[@initFunction]

[Text]: This element names the initialization function of the Cryptographic plugin. This function is called after loading the plugin library for instantiation purposes. The Init function must return an object that implements the DDS Security Cryptographic interface.

> 此元素命名 Cryptographic 插件的初始化函数。此函数是在加载插件库后调用的，用于实例化。Init 函数必须返回一个实现 DDS 安全加密接口的对象。

The default value is: `init\_crypto`

##### //CycloneDDS/Domain/Security/Cryptographic/Library[@path]

[Text]: This element points to the path of the Cryptographic plugin library.

> 此元素指向 Cryptographic 插件库的路径。

It can be either absolute path excluding file extension ( /usr/lib/dds_security_crypto ) or single file without extension ( dds_security_crypto ).

> 它可以是不包括文件扩展名的绝对路径（/usr/lib/dds_security_crypto），也可以是不包含扩展名的单个文件（dds_security_crypto）。

If a single file is supplied, the is library located by the current working directory, or LD_LIBRARY_PATH for Unix systems, and PATH for Windows systems.

> 如果提供了单个文件，则 is 库位于当前工作目录下，或者对于 Unix 系统为 LD_library_PATH，对于 Windows 系统为 PATH。

The default value is: `dds\_security\_crypto`

### //CycloneDDS/Domain/SharedMemory

Children: [Enable](#cycloneddsdomainsharedmemoryenable), [Locator](#cycloneddsdomainsharedmemorylocator), [LogLevel](#cycloneddsdomainsharedmemoryloglevel), [Prefix](#cycloneddsdomainsharedmemoryprefix)

The Shared Memory element allows specifying various parameters related to using shared memory.

> 共享内存元素允许指定与使用共享内存相关的各种参数。

#### //CycloneDDS/Domain/SharedMemory/Enable

[Boolean]: This element allows for enabling shared memory in Cyclone DDS.

> 此元素允许在 Cyclone DDS 中启用共享内存。

The default value is: `false`

#### //CycloneDDS/Domain/SharedMemory/Locator

[Text]: Explicitly set the Iceoryx locator used by Cyclone to check whether a pair of processes is attached to the same Iceoryx shared memory. The default is to use one of the MAC addresses of the machine, which should work well in most cases.

> 显式设置 Cyclone 使用的 Iceoryx 定位器，以检查一对进程是否连接到相同的 Iceooryx 共享内存。默认情况是使用机器的一个 MAC 地址，这在大多数情况下应该可以很好地工作。

The default value is: `<empty>`

#### //CycloneDDS/Domain/SharedMemory/LogLevel

One of: off, fatal, error, warn, info, debug, verbose

This element decides the verbosity level of shared memory message:

> 此元素决定共享内存消息的详细级别：

- off: no log

- fatal: show fatal log

- error: show error log

- warn: show warn log

- info: show info log

- debug: show debug log

- verbose: show verbose log

If you don't want to see any log from shared memory, use off to disable logging.

> 如果您不想从共享内存中看到任何日志，请使用 off 禁用日志记录。

The default value is: `info`

#### //CycloneDDS/Domain/SharedMemory/Prefix

[Text]: Override the Iceoryx service name used by Cyclone.

> 覆盖 Cyclone 使用的 Iceoryx 服务名称。

The default value is: `DDS\_CYCLONE`

### //CycloneDDS/Domain/Sizing

Children: [ReceiveBufferChunkSize](#cycloneddsdomainsizingreceivebufferchunksize), [ReceiveBufferSize](#cycloneddsdomainsizingreceivebuffersize)

The Sizing element allows you to specify various configuration settings dealing with expected system sizes, buffer sizes, &c.

> Sizing 元素允许您指定各种配置设置，以处理预期的系统大小、缓冲区大小等。

#### //CycloneDDS/Domain/Sizing/ReceiveBufferChunkSize

[Number-with-unit]: This element specifies the size of one allocation unit in the receive buffer. It must be greater than the maximum packet size by a modest amount (too large packets are dropped). Each allocation is shrunk immediately after processing a message or freed straightaway.

> 此元素指定接收缓冲区中一个分配单元的大小。它必须比最大数据包大小大一点（丢弃的数据包太大）。每个分配都会在处理完消息后立即收缩或立即释放。

The unit must be specified explicitly. Recognised units: B (bytes), kB & KiB (2^10 bytes), MB & MiB (2^20 bytes), GB & GiB (2^30 bytes).

The default value is: `128 KiB`

#### //CycloneDDS/Domain/Sizing/ReceiveBufferSize

[Number-with-unit]: This element sets the size of a single receive buffer. Many receive buffers may be needed. The minimum workable size is a little larger than Sizing/ReceiveBufferChunkSize, and the value used is taken as the configured value and the actual minimum workable size.

> 此元素设置单个接收缓冲区的大小。可能需要许多接收缓冲器。最小可行大小略大于 Sizing/ReceiveBufferChunkSize，使用的值作为配置值和实际最小可行大小。

The unit must be specified explicitly. Recognised units: B (bytes), kB & KiB (2^10 bytes), MB & MiB (2^20 bytes), GB & GiB (2^30 bytes).

The default value is: `1 MiB`

### //CycloneDDS/Domain/TCP

Children: [AlwaysUsePeeraddrForUnicast](#cycloneddsdomaintcpalwaysusepeeraddrforunicast), [Enable](#cycloneddsdomaintcpenable), [NoDelay](#cycloneddsdomaintcpnodelay), [Port](#cycloneddsdomaintcpport), [ReadTimeout](#cycloneddsdomaintcpreadtimeout), [WriteTimeout](#cycloneddsdomaintcpwritetimeout)

The TCP element allows you to specify various parameters related to running DDSI over TCP.

> TCP 元素允许您指定与通过 TCP 运行 DDSI 相关的各种参数。

#### //CycloneDDS/Domain/TCP/AlwaysUsePeeraddrForUnicast

[Boolean]: Setting this to true means the unicast addresses in SPDP packets will be ignored, and the peer address from the TCP connection will be used instead. This may help work around incorrectly advertised addresses when using TCP.

> 将此设置为 true 意味着将忽略 SPDP 数据包中的单播地址，而使用 TCP 连接中的对等地址。这可能有助于在使用 TCP 时解决广告地址不正确的问题。

The default value is: `false`

#### //CycloneDDS/Domain/TCP/Enable

One of: false, true, default

This element enables the optional TCP transport - deprecated, use General/Transport instead.

> 此元素启用可选的 TCP 传输-不推荐使用，请改用常规/transport。

The default value is: `default`

#### //CycloneDDS/Domain/TCP/NoDelay

[Boolean]: This element enables the TCP_NODELAY socket option, preventing multiple DDSI messages from being sent in the same TCP request. Setting this option typically optimises latency over throughput.

> 此元素启用 TCP_NODELAY 套接字选项，防止在同一 TCP 请求中发送多条 DDSI 消息。设置此选项通常会优化延迟而不是吞吐量。

The default value is: `true`

#### //CycloneDDS/Domain/TCP/Port

[Integer]: This element specifies the TCP port number on which Cyclone DDS accepts connections. If the port is set, it is used in entity locators, published with DDSI discovery, dynamically allocated if zero, and disabled if -1 or not configured. If disabled other DDSI services will not be able to establish connections with the service, the service can only communicate by establishing connections to other services.

> 此元素指定 Cyclone DDS 接受连接的 TCP 端口号。如果设置了端口，则在实体定位器中使用该端口，使用 DDSI 发现发布，如果为零则动态分配，如果为-1 或未配置则禁用该端口。如果被禁用，其他 DDSI 服务将无法与该服务建立连接，则该服务只能通过与其他服务建立连接来进行通信。

The default value is: `-1`

#### //CycloneDDS/Domain/TCP/ReadTimeout

[Number-with-unit]: This element specifies the timeout for blocking TCP read operations. If this timeout expires then the connection is closed.

> 此元素指定阻止 TCP 读取操作的超时时间。如果此超时超时，则连接将关闭。

The unit must be specified explicitly. Recognised units: ns, us, ms, s, min, hr, day.

The default value is: `2 s`

#### //CycloneDDS/Domain/TCP/WriteTimeout

[Number-with-unit]: This element specifies the timeout for blocking TCP write operations. If this timeout expires then the connection is closed.

> 此元素指定阻止 TCP 写入操作的超时时间。如果此超时超时，则连接将关闭。

The unit must be specified explicitly. Recognised units: ns, us, ms, s, min, hr, day.
The default value is: `2 s`

### //CycloneDDS/Domain/Threads

Children: [Thread](#cycloneddsdomainthreadsthread)

This element is used to set thread properties.

> 此元素用于设置线程财产。

#### //CycloneDDS/Domain/Threads/Thread

Attributes: [Name](#cycloneddsdomainthreadsthreadname)
Children: [Scheduling](#cycloneddsdomainthreadsthreadscheduling), [StackSize](#cycloneddsdomainthreadsthreadstacksize)

This element is used to set thread properties.

> 此元素用于设置线程财产。

#### //CycloneDDS/Domain/Threads/Thread[@Name]

[Text]: The Name of the thread for which properties are being set. The following threads exist:

> 正在为其设置财产的线程的名称。存在以下线程：

- gc: garbage collector thread involved in deleting entities;
- recv: receive thread, taking data from the network and running the protocol state machine;
- dq.builtins: delivery thread for DDSI-builtin data, primarily for discovery;
- lease: DDSI liveliness monitoring;
- tev: general timed-event handling, retransmits and discovery;
- fsm: finite state machine thread for handling security handshake;
- xmit.CHAN: transmit thread for channel CHAN;
- dq.CHAN: delivery thread for channel CHAN;
- tev.CHAN: timed-event thread for channel CHAN.

The default value is: `<empty>`

##### //CycloneDDS/Domain/Threads/Thread/Scheduling

Children: [Class](#cycloneddsdomainthreadsthreadschedulingclass), [Priority](#cycloneddsdomainthreadsthreadschedulingpriority)

This element configures the scheduling properties of the thread.

> 此元素配置线程的调度财产。

###### //CycloneDDS/Domain/Threads/Thread/Scheduling/Class

One of: realtime, timeshare, default

This element specifies the thread scheduling class (realtime, timeshare or default). The user may need special privileges from the underlying operating system to be able to assign some of the privileged scheduling classes.

> 此元素指定线程调度类（实时、分时或默认）。用户可能需要来自底层操作系统的特殊权限才能分配一些特权调度类。

The default value is: `default`

###### //CycloneDDS/Domain/Threads/Thread/Scheduling/Priority

[Text]: This element specifies the thread priority (decimal integer or default). Only priorities supported by the underlying operating system can be assigned to this element. The user may need special privileges from the underlying operating system to be able to assign some of the privileged priorities.

> 此元素指定线程优先级（十进制整数或默认值）。只有底层操作系统支持的优先级才能分配给此元素。用户可能需要底层操作系统的特殊权限才能分配一些特权优先级。

The default value is: `default`

##### //CycloneDDS/Domain/Threads/Thread/StackSize

[Number-with-unit]: This element configures the stack size for this thread. The default value default leaves the stack size at the operating system default.

> 此元素配置此线程的堆栈大小。默认值 default 将堆栈大小保留为操作系统默认值。

The unit must be specified explicitly. Recognised units: B (bytes), kB & KiB (2^10 bytes), MB & MiB (2^20 bytes), GB & GiB (2^30 bytes).

The default value is: `default`

### //CycloneDDS/Domain/Tracing

Children: [AppendToFile](#cycloneddsdomaintracingappendtofile), [Category](#cycloneddsdomaintracingcategory), [OutputFile](#cycloneddsdomaintracingoutputfile), [PacketCaptureFile](#cycloneddsdomaintracingpacketcapturefile), [Verbosity](#cycloneddsdomaintracingverbosity)

The Tracing element controls the amount and type of information that is written into the tracing log by the DDSI service. This is useful to track the DDSI service during application development.

> Tracing 元素控制 DDSI 服务写入跟踪日志的信息的数量和类型。这对于在应用程序开发过程中跟踪 DDSI 服务非常有用。

#### //CycloneDDS/Domain/Tracing/AppendToFile

[Boolean]: This option specifies whether the output should be appended to an existing log file. The default is to create a new log file each time, which is generally the best option if a detailed log is generated.

> 此选项指定是否应将输出附加到现有日志文件。默认情况是每次创建一个新的日志文件，如果生成详细的日志，这通常是最佳选项。

The default value is: `false`

#### //CycloneDDS/Domain/Tracing/Category

One of:

- Comma-separated list of: fatal, error, warning, info, config, discovery, data, radmin, timing, traffic, topic, tcp, plist, whc, throttle, rhc, content, shm, trace

> \*逗号分隔的列表包括：致命、错误、警告、信息、配置、发现、数据、radmin、定时、流量、主题、tcp、plist、whc、throttle、rhc、content、shm、trace

- Or empty

This element enables individual logging categories. These are enabled in addition to those enabled by Tracing/Verbosity. Recognised categories are:

> 此元素启用各个日志记录类别。除了由跟踪/详细启用的功能外，还启用了这些功能。公认的类别包括：

- fatal: all fatal errors, errors causing immediate termination
- error: failures probably impacting correctness but not necessarily causing immediate termination
- warning: abnormal situations that will likely not impact correctness
- config: full dump of the configuration
- info: general informational notices
- discovery: all discovery activity
- data: include data content of samples in traces
- radmin: receive buffer administration
- timing: periodic reporting of CPU loads per thread
- traffic: periodic reporting of total outgoing data
- whc: tracing of writer history cache changes
- tcp: tracing of TCP-specific activity
- topic: tracing of topic definitions
- plist: tracing of discovery parameter list interpretation

In addition, there is the keyword trace that enables all but radmin, topic, plist and whc.

> 此外，还有关键字 trace，它可以启用除 radmin、topic、plist 和 whc 之外的所有关键字。

The categorisation of tracing output is incomplete and hence most of the verbosity levels and categories are not of much use in the current release. This is an ongoing process and here we describe the target situation rather than the current situation. Currently, the most useful is trace.

> 跟踪输出的分类是不完整的，因此大多数详细级别和类别在当前版本中没有多大用处。这是一个持续的过程，我们在这里描述的是目标情况，而不是当前情况。目前，最有用的是跟踪。

The default value is: `<empty>`

#### //CycloneDDS/Domain/Tracing/OutputFile

[Text]: This option specifies where the logging is printed to. Note that stdout and stderr are treated as special values, representing "standard out" and "standard error" respectively. No file is created unless logging categories are enabled using the Tracing/Verbosity or Tracing/EnabledCategory settings.

> 此选项指定日志记录的打印位置。请注意，stdout 和 stderr 被视为特殊值，分别表示“标准输出”和“标准错误”。除非使用 Tracing/Verbose 或 Tracing/EnabledCategory 设置启用日志记录类别，否则不会创建任何文件。

The default value is: `cyclonedds.log`

#### //CycloneDDS/Domain/Tracing/PacketCaptureFile

[Text]: This option specifies the file to which received and sent packets will be logged in the "pcap" format suitable for analysis using common networking tools, such as WireShark. IP and UDP headers are fictitious, in particular the destination address of received packets. The TTL may be used to distinguish between sent and received packets: it is 255 for sent packets and 128 for received ones. Currently IPv4 only.

> 此选项指定接收和发送的数据包将以“pcap”格式记录到的文件，该格式适用于使用常见网络工具（如 WireShark）进行分析。IP 和 UDP 报头是虚构的，特别是接收到的数据包的目的地地址。TTL 可以用来区分发送的和接收的数据包：发送的数据包为 255，接收的数据包包括 128。当前仅 IPv4。

The default value is: `<empty>`

#### //CycloneDDS/Domain/Tracing/Verbosity

One of: finest, finer, fine, config, info, warning, severe, none

This element enables standard groups of categories, based on a desired verbosity level. This is in addition to the categories enabled by the Tracing/Category setting. Recognised verbosity levels and the categories they map to are:

> 此元素基于所需的详细程度级别启用标准类别组。这是对“跟踪/类别”设置启用的类别的补充。公认的详细程度及其映射到的类别为：

- none: no Cyclone DDS log
- severe: error and fatal
- warning: severe + warning
- info: warning + info
- config: info + config
- fine: config + discovery
- finer: fine + traffic and timing
- finest: finer + trace

While none prevents any message from being written to a DDSI2 log file.

> 而 none 则阻止任何消息写入 DDSI2 日志文件。

The categorisation of tracing output is incomplete and hence most of the verbosity levels and categories are not of much use in the current release. This is an ongoing process and here we describe the target situation rather than the current situation. Currently, the most useful verbosity levels are config, fine and finest.

> 跟踪输出的分类是不完整的，因此大多数详细级别和类别在当前版本中没有多大用处。这是一个持续的过程，我们在这里描述的是目标情况，而不是当前情况。目前，最有用的详细级别是 config、fine 和 finest。

The default value is: `none`

<!--- generated from ddsi_config.h[d0e749e6d58d560b8172dcb0cb109d6e5f6897ba] -->
<!--- generated from ddsi__cfgunits.h[be1b976c6e9466472b0c331487c05180ec1052d4] -->
<!--- generated from ddsi__cfgelems.h[63812c3c423805d6d3ad61b1706651329935ac40] -->
<!--- generated from ddsi_config.c[3564342f60516165409e1a563f2346bc91895f15] -->
<!--- generated from _confgen.h[f2d235d5551cbf920a8a2962831dddeabd2856ac] -->
<!--- generated from _confgen.c[d74e4fd06e485c5d299dbcc7741cbdb95c5ec706] -->
<!--- generated from generate_rnc.c[a2ec6e48d33ac14a320c8ec3f320028a737920e0] -->
<!--- generated from generate_md.c[37efe4fa9caf56e2647bafc9a7f009f72ff5d2e0] -->
<!--- generated from generate_rst.c[50739f627792ef056e2b4feeb20fda4edfcef079] -->
<!--- generated from generate_xsd.c[45064e8869b3c00573057d7c8f02d20f04b40e16] -->
<!--- generated from generate_defconfig.c[ddf5057ceb5fb512b47f7028da2d75837a349830] -->
