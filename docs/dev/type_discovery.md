# Type Discovery

Type discovery in Cyclone is based on the DDS-XTypes type discovery, as described in section 7.6.3 of the DDS-XTypes specification.

> Cyclone 中的类型发现基于 DDS XTypes 类型发现，如 DDS XType 斯规范第 7.6.3 节所述。

Note: as Cyclone currently (end 2020) does not yet support the XTypes type system, the implementation of type discovery is based on the existing type system in Cyclone and is not interoperable with other vendors.

## Type Identifiers

As part of the type discovery implementation a _type identifier_ for a DDSI sertype is introduced. A type identifier field is added to the (proxy) endpoints and topics in DDSI, which stores the identifier of the type used by the endpoint. The type identifier is retrieved by using the `typeid_hash` function that is part of the `ddsi_sertype_ops` interface. Currently this hash function is only implemented for `ddsi_sertype_default`, as the other implementations in Cyclone (_sertype_builtin_topic_, _sertype_plist_ and _sertype_pserop_) are not used for application types and these types are not exchanged using the type discovery protocol.

> 作为类型发现实现的一部分，引入了 DDSI sertype 的*type 标识符。类型标识符字段被添加到 DDSI 中的(代理)端点和主题，它存储端点使用的类型的标识符。类型标识符是通过使用“ddsi_sertype_ops”接口的一部分“typeid_hash”函数来检索的。目前，此哈希函数仅针对“ddsi-sertype_default”实现，因为 Cyclone 中的其他实现(\_sertype_builtin_topic*、*sertype_plist*和*sertype_preseop*)不用于应用程序类型，并且这些类型不使用类型发现协议进行交换。

The `ddsi_sertype_default` implementation of the sertype interface uses a MD5 hash of the (little endian) serialized sertype as the type identifier. This simplified approach (compared to TypeIdentifier and TypeObject definition in the XTypes specification) does not allow to include full type information in the hash (as is the case for the XTypes type system for certain types), and it also does not allow assignability checking for two type identifiers (see below).

> sertype 接口的“ddsi_sertype_default”实现使用(little-endian)序列化的 sertype 的 MD5 哈希作为类型标识符。这种简化的方法(与 XTypes 规范中的 TypeIdentifier 和 TypeObject 定义相比)不允许在散列中包括完整的类型信息(对于某些类型，XTypes 类型系统就是这样)，并且它也不允许对两个类型标识符进行可分配性检查(见下文)。

## Type resolving

Discovery information (SEDP messages) for topics, publications, and subscriptions contains the type identifier and not the full type information. This allows remote nodes to identify the type used by a proxy reader/writer/topic, without the overhead of sending a full type descriptor over the wire. With the type identifier a node can do a lookup of the type in its local type lookup meta-data administration, which is implemented as a domain scoped hash table (`ddsi_domaingv.tl_admin`).

> 主题、发布和订阅的发现信息(SEDP 消息)包含类型标识符，而不是完整的类型信息。这允许远程节点识别代理读取器/写入器/主题使用的类型，而无需通过有线发送完整类型描述符的开销。使用类型标识符，节点可以在其本地类型查找元数据管理中查找该类型，该元数据管理被实现为域范围的哈希表(`ddsi_domaingv.tl_admin`)。

A type lookup request can be triggered by the application, using the API function `dds_domain_resolve_type` or by the endpoint matching code (explained below). The API function for resolving a type takes an optional time-out, that allows the function call to wait for the requested type to become available. This is implemented by a condition variable `ddsi_domaingv.tl_resolved_cond`, which is triggered by the type lookup reply handler when type information is received.

> 类型查找请求可以由应用程序使用 API 函数“dds_domain_resolve_type”或端点匹配代码触发(如下所述)。用于解析类型的 API 函数需要一个可选的超时，这允许函数调用等待请求的类型变为可用。这是由条件变量“ddsi_domaingv.tl_resolved_cond”实现的，当接收到类型信息时，该变量由类型查找回复处理程序触发。

The type discovery implementation adds a set of built-in endpoints to a participant that can be used to lookup type information: the type lookup request reader/writer and the type lookup response reader/writer (see section 7.6.3.3 of the DDS XTypes specification). A type lookup request message contains a list of type identifiers to be resolved. A node that receives the request (and has a type lookup response writer) writes a reply with the serialized type information from its own type administration. Serializing and deserializing a sertype is also part of the sertype ops interface, using the serialize and deserialize functions. For `ddsi_sertype_default` (the only sertype implementation that currently supports this) the generic plist serializer is used using a predefined sequence of ops for serializing the sertype.

> 类型发现实现为参与者添加了一组内置端点，这些端点可用于查找类型信息：类型查找请求读写器和类型查找响应读写器(请参阅 DDS-XTypes 规范的第 7.6.3.3 节)。类型查找请求消息包含要解析的类型标识符的列表。接收请求的节点(具有类型查找响应编写器)从其自己的类型管理中写入带有序列化类型信息的回复。串行化和反序列化 sertype 也是 sertype-ops 接口的一部分，使用串行化和反串行化函数。对于“ddsi_sertype_default”(当前唯一支持此操作的 sertype 实现)，通用 plist 序列化程序使用预定义的操作序列来序列化 sertype。

Note: In the current type discovery implementation a type lookup request is sent to all nodes and any node that reads this request writes a type lookup reply message (in which the set of reply types can be empty if none of the request type ids are known in that node). This may be optimized in a future release, sending the request only to the node that has sent that specific type identifier in one of its SEDP messages.

> 注意：**在当前的类型发现实现中，类型查找请求被发送到所有节点**，任何读取该请求的节点都会写入类型查找回复消息(如果请求类型 ID 未知，则回复类型集可以为空 在该节点中)。 这可能会在未来的版本中进行优化，仅将请求发送到已在其 SEDP 消息之一中发送该特定类型标识符的节点。

## QoS Matching

The type discovery implementation introduces a number of additional checks in QoS matching in DDSI (function `ddsi_qos_match_mask_p`). After the check for matching _topic name_ and _type name_ for reader and writer, the matching function checks if the type information is resolved for the type identifiers of both endpoints. In case any of the types (typically from the proxy endpoint) is not resolved, matching is delayed and a type lookup request for that type identifier is sent.

> 类型发现实现在 DDSI 中的 QoS 匹配中引入了许多附加检查(函数`DDSI_QoS_match_mask_p`)。在检查读取器和写入器的匹配*topic name*和*type name*之后，匹配函数检查是否为两个端点的类型标识符解析了类型信息。如果没有解析任何类型(通常来自代理端点)，则会延迟匹配，并发送针对该类型标识符的类型查找请求。

An incoming type lookup reply triggers the (domain scoped) `tl_resolved_cond` condition variable (so that the type lookup API function can check if the requested type is resolved) and triggers the endpoint matching for all proxy endpoints that are using one of the resolved types. This list of proxy endpoints is retrieved from the type lookup meta-data administration. For each of these proxy endpoints the function `ddsi_update_proxy_endpoint_matching` is called, which tries to connect the proxy endpoint to local endpoints.

> 传入的类型查找回复触发(域范围的)“tl_resolved_cond”条件变量(以便类型查找 API 函数可以检查请求的类型是否已解析)，并触发使用解析类型之一的所有代理端点的端点匹配。此代理终结点列表是从类型查找元数据管理中检索的。对于这些代理端点中的每一个，都会调用函数“ddsi_update_proxy_endpoint_matching”，该函数试图将代理端点连接到本地端点。
