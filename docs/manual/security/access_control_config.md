# Access control configuration {#Access_Control_Configuration}

Access Control configuration consists of a [governance document](#governance-document) and a [permissions document](#permissions-document). Both documents must be signed by the Permissions CA in S/MIME format. To validate remote permissions, Participants use their own permissions CA.

> 访问控制配置由[治理文档]（#治理文档）和[权限文档]（#permissions 文档）组成。两个文档都必须由权限 CA 以 S/MIME 格式签名。为了验证远程权限，参与者使用自己的权限 CA。

> [!NOTE]

All permissions CA Certificates must be the same for all participants.

> 所有参与者的所有权限 CA 证书必须相同。

The signed document must be formatted as follows:

> 签署的文件必须按以下格式进行格式化：

- S/MIME version 3.2 format (as defined in ).

> -S/MIME 3.2 版格式（如中所定义）。

- SignedData Content Type ().

> -签名数据内容类型（）。

- Signing Using the multipart/signed Format ().

> -使用多部分/签名格式进行签名（）。

This corresponds to the mime-type application/pkcs7-signature.

> 这对应于 mime 类型的 application/pkcs7 签名。

> [!NOTE]

The signer certificate should be be included within the signature.

> 签名者证书应包含在签名中。

## Governance document {#governance_document}

The governance document is an XML file that defines the security behavior of domains and topics. It is specified in OMG (Version 1.1 Section 9.4.1.2.3). For an example of a governance document, see `Create a signed governance document`{.interpreted-text role="ref"}.

> 治理文档是一个 XML 文件，用于定义域和主题的安全行为。它在 OMG（版本 1.1 第 9.4.1.2.3 节）中有规定。有关治理文件的示例，请参阅“创建一个签名的治理文件”｛.depredicted text role=“ref”｝。

> [!NOTE]

To establish communication with a remote node, the options that are specified in a governance document must match with those in the remote node.

> 要与远程节点建立通信，管理文档中指定的选项必须与远程节点中的选项相匹配。

### Protection kinds

The domain governance document provides a means for the application to configure the kinds of cryptographic transformation applied to the complete `RTPS`{.interpreted-text role="term"} message, certain RTPS submessages, and the SerializedPayload RTPS submessage element that appears within the data.

> 域管理文档为应用程序提供了一种方法，用于配置应用于完整`RTPS`｛.explored text role=“term”｝消息、某些 RTPS 子消息和数据中出现的 SerializedPayload RTPS 子信息元素的加密转换类型。

![image](../_static/pictures/rtps_message_structure.png){width="300px"}

The configuration allows specification of five protection levels:

> 该配置允许指定五个保护级别：

- **NONE**

Indicates that no cryptographic transformation is applied.

> 指示未应用加密转换。

- **SIGN**

Indicates that the cryptographic transformation is a `MAC`{.interpreted-text role="term"}, that is, there is no encryption.

> 指示加密转换是`MAC`｛.explored text role=“term”｝，即没有加密。

- **ENCRYPT**

Indicates that the cryptographic transformation is an encryption, followed by a MAC computed on the ciphertext, also known as Encrypt-then-MAC.

> 指示加密转换是一种加密，然后是根据密文计算的 MAC，也称为“先加密后 MAC”。

- **SIGN_WITH_ORIGIN_AUTHENTICATION**

> -**带有原始身份验证的签名**

Indicates that the cryptographic transformation is a set of MACs, that is, no encryption is performed. This cryptographic transformation creates a first common `authenticationcode` (similar to where the Protection Kind is **SIGN**). The cryptographic transformation creates additional authentication codes, each produced with a different secret key. The additional MACs prove to the receiver that the sender originated the message, preventing other receivers from impersonating the sender.

> 指示加密转换是一组 MAC，即不执行加密。此加密转换创建第一个通用的“authenticationcode”（类似于保护类型为**SIGN**的情况）。加密转换会创建额外的身份验证码，每个身份验证码都使用不同的密钥生成。额外的 MAC 向接收方证明发送方发起了消息，从而防止其他接收方冒充发送方。

- **ENCRYPT_WITH_ORIGIN_AUTHENTICATION**.

> -**ENCRYPT_WITH_ORIGIN_AUTHENTICATION**。

Indicates that the cryptographic transformation is an encryption, followed by a MAC computed on the ciphertext, followed by additional authentication codes. Each of the additional authentication codes use a different secret key. The encryption and first (common) authentication code is similar to ones produced when the Protection Kind is **ENCRYPT**. The additional authentication codes are similar to the ones produced when the Protection Kind is **SIGN_WITH_ORIGIN_AUTHENTICATION**.

> 指示加密转换是一种加密，后面是根据密文计算的 MAC，后面是附加的身份验证码。每个附加的身份验证码都使用不同的密钥。加密和第一（通用）身份验证代码与 Protection Kind 为**ENCRYPT**时生成的代码类似。附加的身份验证码与保护类型为**SIGN_WITH_ORIGIN_authentication**时生成的身份验证代码类似。

### Participant attributes

- **Allow Unauthenticated Participants**

> -**允许未经身份验证的参与者**

For communication with non-secure participants. If this option is enabled, a secure participant can only communicate with a non-secure participant via non-protected topics.

> 用于与非安全参与者进行通信。如果启用此选项，则安全参与者只能通过非受保护的主题与非安全参与者进行通信。

- **Enable Join Access Control**

> -**启用加入访问控制**

If this option is enabled, remote participant permissions are checked to see if its subject name is allowed to create a topic.

> 如果启用了此选项，则会检查远程参与者的权限，以查看是否允许其主题名称创建主题。

- **Discovery Protection Kind**

> -**发现保护类型**

This is the protection attribute for discovery communication when it is enabled for topic. For available options, refer to the OMG specification.

> 这是为主题启用发现通信时的保护属性。有关可用选项，请参阅 OMG 规范。

- **Liveliness Protection Kind**

> -**活力保护类**

Protection attribute for liveliness communication when it is enabled for topic. For available options, refer to the OMG specification.

> 为主题启用活动性通信的保护属性。有关可用选项，请参阅 OMG 规范。

- **RTPS Protection Kind**

> -**RTPS 保护类型**

Protection attribute for all messages on the wire. For available options, refer to the OMG specification. If encryption is selected for RTPS, there is no need to encrypt submessages (metadata_protection_kind) and payloads (data_protection_kind) which are defined in topic settings.

> 线路上所有消息的保护属性。有关可用选项，请参阅 OMG 规范。如果为 RTPS 选择了加密，则不需要对主题设置中定义的子消息（元数据*保护*种类）和有效载荷（数据*保护*类型）进行加密。

### Topic attributes

- **Enable Discovery protection**

> -**启用发现保护**

If enabled, discovery is protected according to Discovery Protection Kind attribute of the corresponding participant.

> 如果启用，则根据相应参与者的“发现保护类型”属性对发现进行保护。

- **Enable Liveliness protection**

> -**启用活力保护**

If enabled, liveliness is protected according to Liveliness Protection Kind attribute of the corresponding participant.

> 如果启用，则根据相应参与者的“活力保护种类”属性来保护活力。

- **Enable Read Access Control**

> -**启用读取访问控制**

If enabled, the permissions document is checked if the participant is allowed to create a datareader for the related topic.

> 如果启用，则如果允许参与者为相关主题创建数据读取器，则会检查权限文档。

- **Enable Write Access Control**

> -**启用写访问控制**

If enabled, the permissions document is checked if the participant is allowed to create a datawriter for the related topic.

> 如果启用，则如果允许参与者为相关主题创建数据编写器，则会检查权限文档。

- **Metadata protection Kind**

> -**元数据保护种类**

Protection attribute for submessages.

> 子消息的保护属性。

- **Data protection Kind**

> -**数据保护类型**

Protection attribute for data payload.

> 数据负载的保护属性。

There are different settings for different domain ranges. The domain rules are evaluated in the same order as they appear in the document. A rule only applies to a particular `DomainParticipant` if the domain Section matches the DDS `domain_id` to which the participant belongs. If multiple rules match, the first rule that matches is the only one that applies.

> 不同的域范围有不同的设置。域规则的评估顺序与它们在文档中出现的顺序相同。如果域 Section 与参与者所属的 DDS“domain_id”匹配，则规则仅适用于特定的“DomainParticipant”。如果多个规则匹配，则第一个匹配的规则是唯一适用的规则。

The topic access rules are evaluated in the same order as they appear within the \<topic_access_rules\> Section. If multiple rules match the first rule that matches is the only one that applies.

> 主题访问规则的评估顺序与它们在\<topic_access_rules\>部分中出现的顺序相同。如果多个规则匹配，则第一个匹配的规则是唯一适用的规则。

fnmatch pattern matching can be used for topic expressions including the following patterns:

> fnmatch 模式匹配可用于包括以下模式的主题表达式：

    ---
    Pattern Meaning
    ---
    \* matches everything
    ? matches any single character
    \[seq\] matches any character in seq
    \[!seq\] matches any character not in seq
    ---

## Permissions document {#permissions_document}

The permissions document is an XML file that contains the permissions of the domain participant and binds them to the subject name of the `DomainParticipant`. It is specified in OMG (Version 1.1 Section 9.4.1.3).

> 权限文档是一个 XML 文件，包含域参与者的权限，并将其绑定到“DomainParticipant”的主题名称。它在 OMG（版本 1.1 第 9.4.1.3 节）中有规定。

For an example of a permissions document, refer to: `create_permissions_document`{.interpreted-text role="ref"}.

> 有关权限文档的示例，请参阅：`create_permissions_document`{.depreted text role=“ref”}。

### Validity period

The validity period is checked before creating a participant. The validity period is also checked during handshake with a remote participant.

> 在创建参与者之前会检查有效期。在与远程参与者握手的过程中也会检查有效期。

> [!NOTE]

An expired remote permissions document prevents communications from being established.

> 过期的远程权限文档阻止建立通信。

### Subject name

The subject name must match with Identity Certificate subject. It is checked during a create participant, and during handshake with a remote participant. Use the following openssl command to get subject name from identity certificate:

> 使用者名称必须与身份证书使用者匹配。它在创建参与者期间以及与远程参与者握手期间进行检查。使用以下 openssl 命令从身份证书中获取使用者名称：

> `openssl x509 -noout -subject -nameopt RFC2253 -in <identity_certificate_file.pem>`

### Participant permissions rules

Participant permissions are defined by set of rules. The rules are applied in the same order that appear in the document.

> 参与者权限由一组规则定义。这些规则的应用顺序与文档中出现的顺序相同。

If the criteria for the rule matches the domain_id join and/or publish or subscribe operation that is being attempted, then the allow or deny decision is applied.

> 如果规则的条件与正在尝试的 domain_id 联接和/或发布或订阅操作相匹配，则应用允许或拒绝决定。

If the criteria for a rule does not match the operation being attempted, the evaluation shall proceed to the next rule.

> 如果某个规则的标准与正在尝试的操作不匹配，则应继续评估下一个规则。

If all rules have been examined without a match, then the decision specified by the "default" rule is applied. The default rule, if present, must appear after all allow and deny rules. If the default rule is not present, the implied default decision is DENY.

> 如果检查了所有规则，但没有匹配，则应用“默认”规则指定的决定。默认规则（如果存在）必须出现在所有允许和拒绝规则之后。如果不存在默认规则，则隐含的默认决策为“拒绝”。

The matching criteria for each rule specify the `domain_id`, topics (published and subscribed), the partitions (published and subscribed), and the data-tags associated with the DataWriter and DataReader.

> 每个规则的匹配条件指定“domain_id”、主题（已发布和已订阅）、分区（已发布或已订阅）以及与 DataWriter 和 DataReader 关联的数据标记。

For the grant to match, there must be a match of the topics, partitions, and data-tags criteria. This is interpreted as an _AND_ of each of the criteria. For a specific criterion to match (for example, \<Topics\>) it is enough that one of the topic expressions listed matches (that is, an _OR_ of the expressions with the \<Topics\> section).

> 要使授予匹配，必须匹配主题、分区和数据标记条件。这被解释为每个标准的*AND*。对于要匹配的特定标准（例如，\<Topics\>），列出的主题表达式之一匹配（即，具有\<Topics \>部分的表达式的*OR*）就足够了。

For topic expressions and partition expressions, use [fnmatch pattern matching](#fnmatch pattern matching).

> 对于主题表达式和分区表达式，请使用[fnmatch 模式匹配]（#fnmatch 模式匹配）。
