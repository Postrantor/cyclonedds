# DDS security {#DDS_security}

is compliant with The Object Management Group (OMG) specification, which defines the Security Model and `SPI`{.interpreted-text role="term"} architecture for compliant DDS implementations. The DDS Security Model is enforced by the invocation of these SPIs by the DDS implementation.

> 符合对象管理组（OMG）规范，该规范定义了用于兼容 DDS 实现的安全模型和“SPI`｛.explored text role=“term”｝架构。DDS 安全模型是通过 DDS 实现对这些 SPI 的调用来强制执行的。

![image](../_static/pictures/dds_security_overview.png){width="1000px"}

The DDS Security Model has the following three plugins:

> DDS 安全模型具有以下三个插件：

- **Authentication service plugin**

> -**身份验证服务插件**

Verifies the identity of the application and/or user that invokes operations on DDS. Includes facilities for mutual authentication between participants and establish a shared secret.

> 验证调用 DDS 操作的应用程序和/或用户的身份。包括参与者之间相互验证和建立共享机密的设施。

- **AccessControl service plugin**

> -**访问控制服务插件**

Enforces policy decisions that determine what DDS-related operations an authenticated user is permitted to do. For example, which domains it can join, which topics it can publish or subscribe to, and so on.

> 强制执行策略决策，以确定允许经过身份验证的用户执行哪些与 DDS 相关的操作。例如，可以加入哪些域，可以发布或订阅哪些主题，等等。

- **Cryptographic service plugin**

> -**加密服务插件**

Implements (or interfaces with libraries that implement) all cryptographic operations. Includes encryption, decryption, hashing, digital signatures, and so on (also includes the means to derive keys from a shared secret).

> 实现（或与实现所有加密操作的库接口）。包括加密、解密、哈希、数字签名等（还包括从共享秘密中获取密钥的方法）。

provides built-in implementations of these plugins. Authentication uses `PKI`{.interpreted-text role="term"} with a pre-configured shared `Certificate Authority <certificate_authority>`{.interpreted-text role="ref"}. `RSA`{.interpreted-text role="term"} is for authentication and Diffie-Hellman is for key exchange. The AccessControl service plugin uses a Permissions document that is signed by a shared certificate authority. Cryptography uses `AES-GCM`{.interpreted-text role="term"} for encryption and `AES-GMAC`{.interpreted-text role="term"} for message authentication.

> 提供了这些插件的内置实现。身份验证使用`PKI`｛.depreted text role=“term”｝和预先配置的共享`Certificate Authority<Certificate_Authority>`｛.repreted text role=“ref”｝`RSA`{.depreted text role=“term”}用于身份验证，Diffie-Hellman 用于密钥交换。AccessControl 服务插件使用由共享证书颁发机构签名的 Permissions 文档。密码学使用`AES-GCM`｛.expected text role=“term”｝进行加密，使用`AES-GMAC`｛.Expected text role=”term“｝进行消息身份验证。

Security plugins are dynamically loaded. The locations are defined in configuration or participant QoS settings:

> 安全插件是动态加载的。位置在配置或参与者 QoS 设置中定义：

- `//CycloneDDS/Domain/Security/AccessControl/Library`{.interpreted-text role="ref"}
- `//CycloneDDS/Domain/Security/Authentication/Library`{.interpreted-text role="ref"}
- `//CycloneDDS/Domain/Security/Cryptographic/Library`{.interpreted-text role="ref"}

The security plugins can also be found in:

> 安全插件也可以在以下位置找到：
