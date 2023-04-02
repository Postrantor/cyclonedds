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

# TCP support

The is a protocol that provides a connectionless transport with unreliable datagrams. However, there are times where `TCP` is the only practical network transport available (for example, across a WAN). This is the reason can use TCP instead of `UDP` if needed.

> **是一种为无连接传输提供不可靠数据报的协议。然而，有时“TCP”是唯一可用的实际网络传输（例如，通过 WAN）。这就是如果需要，可以使用 TCP 而不是“UDP”的原因。**

The differences in the model of operation between DDSI and TCP are quite large: DDSI is based on the notion of peers, whereas TCP communication is based on the notion of a session that is initiated by a "client" and accepted by a "server"; therefore, TCP requires knowledge of the servers to connect to before the DDSI discovery protocol can exchange that information. The configuration of this is done in the same manner as for unicast-based UDP discovery.

> DDSI 和 TCP 在操作模型上的差异很大：DDSI 基于对等体的概念，**而 TCP 通信基于由“客户端”发起并由“服务器”接受的会话的概念**；因此，**在 DDSI 发现协议能够交换该信息之前，TCP 需要知道要连接到的服务器**。这方面的配置是以与基于单播的 UDP 发现相同的方式完成的。

TCP reliability is defined in terms of these sessions, but DDSI reliability is defined in terms of DDSI discovery and liveliness management. It is therefore possible that a TCP connection is (forcibly) closed while the remote endpoint is still considered alive. Following a reconnect, the samples lost when the TCP connection was closed can be recovered via the standard DDSI reliability. This also means that the Heartbeats and AckNacks still need to be sent over a TCP connection, and consequently that DDSI flow-control occurs on top of TCP flow-control.

> TCP 可靠性是根据这些会话来定义的，但 **DDSI 可靠性是根据 DDSI 发现和活跃度管理来定义的**。因此，**TCP 连接可能（强制）关闭，而远程端点仍被认为是活动的。重新连接后，TCP 连接关闭时丢失的样本可以通过标准 DDSI 可靠性恢复**。这也意味着**心跳和 AckNack 仍然需要通过 TCP 连接发送，因此 DDSI 流控制发生在 TCP 流控制之上**。

Connection establishment potentially takes a long time, and giving up on a transmission to a failed, or no longer reachable host, can also take a long time. These long delays can be visible at the application level.

> 建立连接可能需要很长时间，而放弃向故障或无法再连接的主机的传输也可能需要很短时间。这些长延迟在应用程序级别是可见的。
