# Runtime configuration

The out-of-the-box configuration should usually be fine, but there are a great many options that can be tweaked by creating an XML file with the desired settings and defining the _CYCLONEDDS_URI_ to point to it. E.g. (on Linux):

> **开箱即用的配置通常应该很好**，但有很多选项可以通过创建具有所需设置的 XML 文件并定义 _CYCLONEDDS_URI_ 来进行调整。例如(在 Linux 上)：

```xml
$ cat cyclonedds.xml
<?xml version="1.0" encoding="UTF-8" ?>
<CycloneDDS xmlns="https://cdds.io/config" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:schemaLocation="https://cdds.io/config https://raw.githubusercontent.com/eclipse-cyclonedds/cyclonedds/master/etc/cyclonedds.xsd">
    <Domain Id="any">
        <General>
            <Interfaces>
                <NetworkInterface autodetermine="true" priority="default" multicast="default" />
            </Interfaces>
            <AllowMulticast>default</AllowMulticast>
            <MaxMessageSize>65500B</MaxMessageSize>
        </General>
        <Discovery>
            <EnableTopicDiscoveryEndpoints>true</EnableTopicDiscoveryEndpoints>
        </Discovery>
        <Internal>
            <Watermarks>
                <WhcHigh>500kB</WhcHigh>
            </Watermarks>
        </Internal>
        <Tracing>
            <Verbosity>config</Verbosity>
            <OutputFile>cdds.log.${CYCLONEDDS_PID}</OutputFile>
        </Tracing>
    </Domain>
</CycloneDDS>
$ export CYCLONEDDS_URI=file://$PWD/cyclonedds.xml
```

(on Windows, one would have to use _set CYCLONEDDS_URI=file://..._ instead.)

This example shows a few things:

- _Interfaces_ can be used to override the interfaces selected by default. Members are

> - _Interfaces_ 可用于覆盖默认选择的接口。成员为

    - _NetworkInterface[@autodetermine]_ tells Cyclone DDS to autoselect the interface it deems best.
    - _NetworkInterface[@name]_ specifies the name of an interface to select (not shown above, alternative for autodetermine).
    - _NetworkInterface[@address]_ specifies the IPv4/IPv6 address of an interface to select (not shown above, alternative for autodetermine).
    - _NetworkInterface[@multicast]_ specifies whether multicast should be used on this interface. The default value 'default' means Cyclone DDS will check the OS reported flags of the interface and enable multicast if it is supported. Use 'true' to ignore what the OS reports and enable it anyway and 'false' to always disable multicast on this interface.
    - _NetworkInterface[@priority]_ specifies the priority of an interface. The default value (_default_) means priority _0_ for normal interfaces and _2_ for loopback interfaces.

    - _NetworkInterface[@autodetermine]_ 告诉 Cyclone DDS 自动选择它认为最好的接口。
    - _NetworkInterface[@name]_ 指定要选择的接口的名称(上面未显示，自动确定的替代方法)。
    - _NetworkInterface[@address]_ 指定要选择的接口的 IPv4/IPv6 地址(上面未显示，自动确定的替代方法)。
    - _NetworkInterface[@multicast]_ 指定是否应在此接口上使用多播。 默认值“default”意味着 Cyclone DDS 将检查操作系统报告的接口标志并在支持时启用多播。 使用“true”忽略操作系统报告的内容并启用它，使用“false”始终禁用此接口上的多播。
    - _NetworkInterface[@priority]_ 指定接口的优先级。 默认值 (_default_) 表示普通接口的优先级为 _0_，环回接口的优先级为 _2_。

- _AllowMulticast_ configures the circumstances under which multicast will be used. If the selected interface doesn't support it, it obviously won't be used (_false_); but if it does support it, the type of the network adapter determines the default value. For a wired network, it will use multicast for initial discovery as well as for data when there are multiple peers that the data needs to go to (_true_). On a WiFi network it will use it only for initial discovery (_spdp_), because multicast on WiFi is very unreliable.

> - _AllowMulticast_ 配置将使用多播的情况。如果所选接口不支持它，那么它显然不会被使用(_false_)；但如果它确实支持它，那么网络适配器的类型将决定默认值。对于有线网络，它将使用多播进行初始发现，以及在数据需要到达多个对等点时使用多播(_true_)进行数据发现。在 WiFi 网络上，它只会将其用于初始发现(_spdp_)，因为 WiFi 上的多播非常不可靠。

- _EnableTopicDiscoveryEndpoints_ turns on topic discovery (assuming it is enabled at compile time), it is disabled by default because it isn't used in many system and comes with a significant amount of overhead in discovery traffic.

> - _EnableTopicDiscoveryEndpoints_ 打开主题发现(假设它在编译时启用)，默认情况下会被禁用，因为它在许多系统中都不使用，并且在发现流量中会带来大量开销。

- _Verbosity_ allows control over the tracing, "config" dumps the configuration to the trace output (which defaults to "cyclonedds.log", but here the process id is appended). Which interface is used, what multicast settings are used, etc., is all in the trace. Setting the verbosity to "finest" gives way more output on the inner workings, and there are various other levels as well.

> - _Verbosity_ 允许控制跟踪，“config”将配置转储到跟踪输出(默认为“cycloneds.log”，但此处附加了进程 id)。使用的接口、使用的多播设置等都在跟踪中。将详细程度设置为“最精细”会让位于内部工作的更多输出，还有各种其他级别。

- _MaxMessageSize_ controls the maximum size of the RTPS messages (basically the size of the UDP payload). Large values such as these typically improve performance over the (current) default values on a loopback interface.

> - _MaxMessageSize_ 控制 RTPS 消息的最大大小(基本上是 UDP 有效载荷的大小)。与环回接口上的(当前)默认值相比，这样的大值通常会提高性能。

- _WhcHigh_ determines when the sender will wait for acknowledgements from the readers because it has buffered too much unacknowledged data. There is some auto-tuning, the (current) default value is a bit small to get really high throughput.

> - _WhcHigh_ 确定发送器何时等待来自读取器的确认，因为它缓冲了太多未确认的数据。有一些自动调整，(当前)默认值有点小，以获得真正的高吞吐量。

Background information on configuring Cyclone DDS can be found [here](docs/manual/config.rst) and a list of settings is [available](docs/manual/options.md).

> 有关配置 Cyclone DDS 的背景信息可以在[此处](docs/manual/config.rst)找到，设置列表也可以在[可用](docs/manual/options.md)找到。
