In DCPS, a domain is uniquely identified by a non-negative integer, the domain ID. In the UDP/IP mapping, this domain ID is mapped to port numbers that are used for communicating with the peer nodes. These port numbers are of significance for the discovery protocol. This mapping of domain IDs to UDP/IP port numbers ensures that accidental cross-domain communication is impossible with the default mapping.

> 在 DCPS 中，**域由一个非负整数（即域 ID）唯一标识**。在 UDP/IP 映射中，此**域 ID 映射到用于与对等节点通信的端口号**。这些端口号对于发现协议具有重要意义。这种域 ID 到 UDP/IP 端口号的映射确保了使用默认映射不可能进行意外的跨域通信。

In DCPS there is a one-to-many mapping of domain ID to port numbers. In DDSI, there is a one-to-one mapping of domain ID to a port number, which is why DDSI does not communicate the DCPS port number in the discovery protocol; it assumes that each domain ID maps to a unique port number.

> **在 DCPS 中，存在域 ID 到端口号的一对多映射。在 DDSI 中，存在域 ID 到端口号的一对一映射**，这就是为什么 DDSI 不在发现协议中通信 DCPS 端口号；它假设每个域 ID 映射到一个唯一的端口号。

> [NOTE]:
> While it is unusual to change the mapping, the specification requires this to be possible, which means that two different DCPS domain IDs can be mapped to a single DDSI domain.
> 虽然更改映射是不寻常的，但规范要求这是可能的，这意味着**两个不同的 DCPS 域 ID 可以映射到一个 DDSI 域**。
