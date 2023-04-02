# DDS secure discovery

When DDS Security is enabled, discovers remote participants, topics, readers and writers as normal. However, if the system contains a number of slow platforms, or the platform is large, discovery can take longer to due to the more complex handshaking that is required. The effort to perform discovery grows quadratically with the number of nodes.

> 启用 DDS 安全性时，会正常发现远程参与者、主题、读取器和写入程序。然而，如果系统包含许多速度较慢的平台，或者平台很大，由于需要更复杂的握手，发现可能需要更长的时间。执行发现的工作量随着节点数量的增加而呈二次方增长。
