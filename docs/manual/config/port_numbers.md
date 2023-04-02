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

# Port numbers

The port numbers are configured as follows:

> [NOTE]:
> The first two items are defined by the . The third item is unique to as a way of serving multiple participants by a single DDSI instance.
> 前两项由定义。第三项是唯一的，作为一种通过单个 DDSI 实例为多个参与者提供服务的方式。

- Two "well-known" multicast ports: `B` and `B+1`.
- Two unicast ports at which only this instance is listening: `B+PG*PI+10` and `B+PG*PI+11`
- One unicast port per domain participant it serves, chosen by the kernel from the list of anonymous ports, that is, >= 32768.

> - 两个“众所周知”的多播端口：`B` and `B+1`
> - 只有此实例正在侦听的两个单播端口： `B+PG*PI+10` and `B+PG*PI+11`
> - 它所服务的每个域参与者一个单播端口，由内核从匿名端口列表中选择，即>=32768。

where:

- _B_ is `Discovery/Ports/Base <//CycloneDDS/Domain/Discovery/Ports/Base>` (`7400`) + `Discovery/Ports/DomainGain <//CycloneDDS/Domain/Discovery/Ports/DomainGain>` (`250`) \* `Domain[@Id] <//CycloneDDS/Domain[@Id]>`
- _PG_ is `Discovery/Ports/ParticipantGain <//CycloneDDS/Domain/Discovery/Ports/ParticipantGain>` (`2`)
- _PI_ is `Discovery/ParticipantIndex <//CycloneDDS/Domain/Discovery/ParticipantIndex>`

> - _B_ 为`Discovery/Ports/Base</CycloneDDS/Domain/Discovery/Ports/Base>`(`7400`)+`Discovery/Ports/DomainGain</CycloneDDS/Domain/Discovery/Ports/DomainGain>`(` 250`)\*`Domain[@Id]</CycloneDS/Domain[@Id]>`
> - _PG_ 为`Discovery/Ports/ParticipantGain</CycloneDDS/Domain/Discovery/Ports/Participant增益>`(`2')
> - _PI_ 是`发现/参与者索引</CycloneDDS/Domain/Discovery/参与者索引>`

The default values (taken from the DDSI specification) are in parentheses.

> [NOTE]:
> This shows only a sub-set of the available parameters. The other parameters in the specification have no bearing on . However, these are configurable. For further information, refer to the or specification, section 9.6.1.
> 这只显示了可用参数的子集。规范中的其他参数与此无关。但是，这些是可配置的。有关更多信息，请参阅或规范第 9.6.1 节。

_PI_ relates to having multiple processes in the same domain on a single node. Its configured value is either _auto_, _none_ or a non-negative integer. This setting matters:

> _PI_ 涉及在**单个节点上的同一域中具有多个进程**。其配置值为 _auto_、_none_ 或非负整数。此设置很重要：

- _none_ (default): It ignores the "participant index" altogether and asks the kernel to pick random ports (>= 32768). This eliminates the limit on the number of standalone deployments on a single machine and works well with multicast discovery, while complying with all other parts of the specification for interoperability. However, it is incompatible with unicast discovery.
- _auto_: polls UDP port numbers on start-up, starting with `PI = 0`, incrementing it by one each time until it finds a pair of available port numbers, or it hits the limit. To limit the cost of unicast discovery, the maximum PI is set in: `Discovery/MaxAutoParticipantIndex <//CycloneDDS/Domain/Discovery/MaxAutoParticipantIndex>`.
- _non-negative integer_: It is the value of PI in the above calculations. If multiple processes on a single machine are required, they need unique values for PI, and therefore for standalone deployments, this alternative is of little use.

> - _none_(默认值)：它完全忽略“参与者索引”，并要求**内核选择随机端口(>=32768)**。这消除了对单机上独立部署数量的限制，并且**与多播发现配合良好**，同时符合互操作性规范的所有其他部分。但是，它与单播发现不兼容。
> - _auto_：**在启动时轮询 UDP 端口号，从“PI=0”开始，每次递增一，直到找到一对可用端口号，或者达到极限**。为了限制单播发现的成本，最大 PI 设置为：`discovery/MaxAutoParticipantIndex</CycloneDDS/Domain/discovery/MaxAutoParticipantIndex>`。
> - _non-negative integer_：它是上述计算中 PI 的值。如果需要在一台机器上有多个进程，它们需要 PI 的唯一值，因此对于独立部署，这种替代方案用处不大。

To fully control port numbers, setting (= PI) to a hard-coded value is the only possibility. `Discovery/ParticipantIndex <//CycloneDDS/Domain/Discovery/ParticipantIndex>` By defining PI, the port numbers needed for unicast discovery are fixed as well. This allows listing peers as IP:PORT pairs, which significantly reduces traffic.

> 为了完全控制端口号，将(=PI)设置为硬编码值是唯一的可能性`发现/参与者索引</CycloneDDS/Domain/Discovery/PParticipantIndex>` **通过定义 PI，单播发现所需的端口号也是固定的。这允许将对等端列为 IP:PORT 对，从而显著减少流量**。

The other non-fixed ports that are used are the per-domain participant ports, the third item in the list. These are used only because there exist some DDSI implementations that assume each domain participant advertises a unique port number as part of the discovery protocol, and hence that there is never any need for including an explicit destination participant ID when intending to address a single domain participant by using its unicast locator. never makes this assumption, instead opting to send a few bytes extra to ensure the contents of a message are all that is needed. With other implementations, you will need to check.

> 使用的其他非固定端口是每个域参与者的端口，这是列表中的第三项。使用这些只是因为存在一些 DDSI 实现，这些 DDSI 实现假设每个域参与者通告唯一的端口号作为发现协议的一部分，并且因此当打算通过使用其单播定位器来寻址单个域参与者时，从不需要包括显式目的地参与者 ID。从不做这种假设，而是选择额外发送几个字节，以确保消息的内容是所需的全部内容。对于其他实现，您需要进行检查。

If all DDSI implementations in the network include full addressing information in the messages like does, then the per-domain participant ports serve no purpose at all. The default `false` setting of `Compatibility/ManySocketsMode <//CycloneDDS/Domain/Compatibility/ManySocketsMode>` disables the creation of these ports.

> 如果网络中的所有 DDSI 实现都像这样在消息中包括完整的寻址信息，那么每个域的参与者端口根本没有任何作用。“Compatibility/ManySocketsMode<//CyconeDDS/Domain/Compatibility/ManySocketsMode>”的默认“false”设置禁止创建这些端口。

This setting can have a few other side benefits, as there may be multiple DCPS participants using the same unicast locator. This improves the chances of a single unicast sufficing even when addressing multiple participants.

> 此设置可能还有一些其他好处，因为可能有多个 DCPS 参与者使用相同的单播定位器。这提高了即使在寻址多个参与者时单个单播也足够的机会。
