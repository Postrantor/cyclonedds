# Multicasting

You can configure the extent to which `multicast` is used (regular, any-source multicast, as well as source-specific multicast):

> 您可以配置“多播”的使用范围（常规、任何源多播以及特定于源的多播）：

- whether to use multicast for data communications,
- whether to use multicast for participant discovery,
- on which interfaces to listen for multicasts.

> - 是否使用多播进行数据通信，
> - 是否使用多播来进行参与者发现，
> - 在哪些接口上侦听多播。

We recommend that you use multicasting. However, if there are restrictions on the use of multicasting, or if the network reliability is dramatically different for multicast than for unicast, disable multicast for normal communications. To force the use of unicast communications for everything, set: `General/AllowMulticast <//CycloneDDS/Domain/General/AllowMulticast>` to `false`.

> 我们**建议您使用多播**。但是，如果对多播的使用有限制，或者如果多播的网络可靠性与单播的网络可靠性显著不同，则禁用正常通信的多播。要**强制对所有内容使用单播通信**，请将：`General/AllowMulticast</CycloneDDS/Domain/General/AllowMulticast>`设置为`false`。

> [!IMPORTANT]
> We strongly advise you to have multicast-based participant discovery enabled, which avoids having to specify a list of nodes to contact, and reduces the network load considerably. To allow participant discovery via multicast while disabling multicast for everything else, set: `General/AllowMulticast <//CycloneDDS/Domain/General/AllowMulticast>` to `spdp`
> 我们强烈建议您**启用基于多播的参与者发现，这样可以避免指定要联系的节点列表，并大大降低网络负载**。要允许通过多播发现参与者，**同时禁用其他所有内容的多播**，请将：`General/AllowMulticast</CycloneDDS/Domain/General/AllowMulticast>`设置为`spdp`

To disable incoming multicasts, or to control from which interfaces multicasts are to be accepted, set: `General/MulticastRecvNetworkInterfaceAddresses <//CycloneDDS/Domain/General/MulticastRecvNetworkInterfaceAddresses>` setting. The options are:

> 要禁用传入的多播，或控制从哪些接口接受多播，请设置：`General/MulticastRecvNetworkInterfaceAddresses</CycloneDDS/Domain/General/MulticastRecvNetworkInterface Addresses>`设置。选项包括：

> - Listening on no interface
> - Preferred
> - All
> - A specific set of interfaces
