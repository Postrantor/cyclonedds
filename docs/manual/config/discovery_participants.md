DDSI participants discover each other using the _Simple Participant Discovery Protocol_ (SPDP). This protocol periodically sends a message containing the specifics of the participant to a set of known addresses. By default, this address is a standardised multicast address (`239.255.0.1` where the port number is derived from the domain ID) that all DDSI implementations listen to.

> DDSI 参与者使用*简单参与者发现协议* (SPDP) 相互发现。 该协议定期将包含参与者详细信息的消息发送到一组已知地址。 默认情况下，此地址是所有 DDSI 实现侦听的标准多播地址（“239.255.0.1”，其中端口号源自域 ID）。

In the SPDP message, the unicast and multicast addresses are important, as that is where the participant can be reached. Typically, each participant has a unique unicast address, which means all participants on a node have a different UDP/IP port number in their unicast address. The actual address (including port number) in a multicast-capable network is unimportant because all participants learn them through the SPDP messages.

> 在 SPDP 消息中，单播和多播地址很重要，因为这是可以到达参与者的地方。 通常，**每个参与者都有一个唯一的单播地址**，这意味着一个节点上的所有参与者在其单播地址中都有不同的 UDP/IP 端口号。 支持多播的网络中的实际地址（包括端口号）并不重要，因为所有参与者都通过 SPDP 消息了解它们。

The SPDP protocol allows for unicast-based discovery. This requires listing the addresses of machines where participants are located, and ensuring that each participant uses one of a small set of port numbers. Because of this, some port numbers are derived from the domain ID and a _participant index_, which is a small non-negative integer, unique to a participant within a node. adds an indirection and uses one participant index for a domain for each process, regardless of how many DCPS participants are created by the process.

> **SPDP 协议允许基于单播的发现。** 这需要列出参与者所在机器的地址，并确保每个参与者使用一小组端口号中的一个。 因此，一些端口号是从域 ID 和 _参与者索引_ 派生的，这是一个小的非负整数，对于节点内的参与者是唯一的。 添加一个间接并为每个进程的域使用一个参与者索引，而不管该进程创建了多少 DCPS 参与者。

When two participants have discovered each other and both have matched the DDSI built-in endpoints that their peer is advertising in the SPDP message, the _Simple Endpoint Discovery Protocol_ (SEDP) takes over. The SEDP exchanges information about the DCPS Readers and writers with the two participants. For , SEDP also exchanges information about publishers, subscribers and topics in a manner compatible with OpenSplice.

> **当两个参与者发现彼此并且都匹配了他们的对等方在 SPDP 消息中通告的 DDSI 内置端点时，_简单端点发现协议_ (SEDP) 接管。 SEDP 与两个参与者交换有关 DCPS 读者和作者的信息。 对于 ，SEDP 还以与 OpenSplice 兼容的方式交换有关发布者、订阅者和主题的信息。**

The SEDP data is handled as reliable (see `reliable_coms`), transient-local data (see `DDSI-specific transient-local behaviour`). Therefore, the SEDP Writers send Heartbeats. If the SEDP Readers detect they have not yet received all samples and send AckNacks requesting re-transmissions, the Writer responds to these and eventually receives a pure acknowledgement informing it that the reader has now received the complete set.

> SEDP 数据被处理为可靠的（参见“reliable_coms”）、瞬态本地数据（参见“DDSI 特定的瞬态本地行为”）。 因此，SEDP 编写器发送心跳。 如果 SEDP 读取器检测到它们尚未收到所有样本并发送 AckNack 请求重新传输，写入器将响应这些并最终收到一个纯确认，通知它读取器现在已收到完整的集合。

Note

The discovery process creates a burst of traffic each time a participant is added to the system: **all** existing participants respond to the SPDP message, which all start exchanging SEDP data.
