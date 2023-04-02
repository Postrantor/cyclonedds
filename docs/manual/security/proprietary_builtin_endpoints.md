# Proprietary builtin endpoints

The DDS standard contains some builtin endpoints. A few are added by the DDS Security specification (). The behaviour of all these builtin endpoints are specified and handled by the plugins (see `Plugins_configuration`{.interpreted-text role="ref"}) that implement the OMG DDS Security specification. That is, they do not have to be present in the `governance document <governance_document>`{.interpreted-text role="ref"} or `permissions document

> DDS 标准包含一些内置端点。DDS 安全规范（）添加了一些。所有这些内置端点的行为都是由实现 OMG DDS 安全规范的插件指定和处理的（请参阅`plugins_configuration`｛.depreted text role=“ref”｝）。也就是说，它们不必出现在`governance document＜governance_document>`{.depreted text role=“ref”}或`permissions document中
<permissions_document>`{.interpreted-text role="ref"}, and users do not have to consider these endpoints.

A few of these builtin endpoints behave according to the _protection kinds_ within the `governance document <governance_document>`{.interpreted-text role="ref"}. Related submessages are protected according to the value of their protection kind, which protects the meta information that is send during the discovery phase.

> 这些内置端点中的一些根据`governance document<governance_document>`{.depredicted text role=“ref”}中的*protection kinds*行为。相关子消息根据其保护类型的值进行保护，该保护类型保护在发现阶段发送的元信息。
