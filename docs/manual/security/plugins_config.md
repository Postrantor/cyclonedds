# Plugin configuration {#Plugins_configuration}

gets the security configuration from XML configuration elements or from the participant QoS policies as stated in the OMG DDS Security specification ().

> 从 XML 配置元素或 OMG DDS 安全规范（）中规定的参与者 QoS 策略中获取安全配置。

This behavior allows applications to use DDS Security without recompiling the binaries. Supplying a new configuration with DDS Security enabled is enough to switch from a non-secure to a secure deployment. The configuration is at domain level, which means that all participants created for that domain receive the same DDS security settings.

> 此行为允许应用程序在不重新编译二进制文件的情况下使用 DDS Security。提供启用了 DDS 安全性的新配置就足以从非安全部署切换到安全部署。配置是在域级别，这意味着为该域创建的所有参与者都会收到相同的 DDS 安全设置。

The configuration options for a domain are in the configuration (`/Domain/Security <//CycloneDDS/Domain/Security>`{.interpreted-text role="ref"}). Every DDS Security plugin has its own configuration sub-section.

> 域的配置选项在配置中（`/domain/Security</CycloneDDS/domain/Security>`{.depreted text role=“ref”}）。每个 DDS 安全插件都有自己的配置小节。

## Authentication properties {#Authentication Properties}

To enable authentication for a node, it must be configured with an `IdentityCertificate <//CycloneDDS/Domain/Security/Authentication/IdentityCertificate>`{.interpreted-text role="ref"}, which authenticates all participants of that particular domain. Associated with the identity certificate is the corresponding `PrivateKey <//CycloneDDS/Domain/Security/Authentication/PrivateKey>`{.interpreted-text role="ref"}.

> 要为节点启用身份验证，必须使用`IdentityCertificate</CycloneDDS/Domain/Security/authentication/IdentityCertificate>`｛.depreced text role=“ref”｝配置该节点，该节点可以对该特定域的所有参与者进行身份验证。与身份证书相关联的是相应的`PrivateKey</CycloneDDS/Domain/Security/Authentication/PrivateKey>`{.depredicted text role=“ref”}。

The private key is either a 2048-bit RSA key, or a 256-bit Elliptic Curve Key with a prime256v1 curve.

> 私钥是 2048 位 RSA 密钥，或者是具有素数 256v1 曲线的 256 位椭圆曲线密钥。

The certificate of identity CA, which is the issuer of the node\'s identity certificate, is configured in `IdentityCA <//CycloneDDS/Domain/Security/Authentication/IdentityCA>`{.interpreted-text role="ref"}.

> 身份证书 CA 是节点身份证书的颁发者，在`IdentityCA</CycloneDDS/Domain/Security/Authentication/IdentityCA>`{.depredicted text role=“ref”}中配置。

The public key of the identity CA (as part of its certificate) is either a 2048-bit RSA key, or a 256-bit Elliptic Curve key for the prime256v1 curve. The identity CA certificate can be a self-signed certificate.

> 身份 CA 的公钥（作为其证书的一部分）是 2048 位 RSA 密钥，或者是素数 256v1 曲线的 256 位椭圆曲线密钥。身份 CA 证书可以是自签名证书。

The identity certificate, private key and the identity CA should be a X509 document in PEM format. It may either be specified directly in the configuration file (as CDATA, prefixed with `data:,`), or the configuration file should contain a reference to a corresponding file (prefixed with `file:`).

> 身份证书、私钥和身份 CA 应该是 PEM 格式的 X509 文档。它可以直接在配置文件中指定（作为 CDATA，前缀为“data:，”），也可以在配置文件包含对相应文件的引用（前缀为“file:”）。

Optionally, the private key can be protected by a `password <//CycloneDDS/Domain/Security/Authentication/Password>`{.interpreted-text role="ref"}.

> 可选地，私钥可以通过`password</CycloneDDS/Domain/Security/Authentication/password>`{.depredicted text role=“ref”}进行保护。

To enable multiple identity CAs throughout the system, you can configure a directory that contains additional identity CA\'s that verify the identity certificates received from remote instances (`TrustedCADirectory <//CycloneDDS/Domain/Security/Authentication/TrustedCADirectory>`{.interpreted-text role="ref"}).

> 要在整个系统中启用多个身份 CA，您可以配置一个目录，该目录包含额外的身份 CA，用于验证从远程实例接收的身份证书（`TrustedCADdirectory</CycloneDDS/Domain/Security/Authentication/TrustedCADdirectory>`{.depredicted text role=“ref”}）。

## Access control properties {#Access Control Properties}

The following are are required for the access control plugin:

> 访问控制插件需要以下内容：

    - A governance document (`//CycloneDDS/Domain/Security/AccessControl/Governance`{.interpreted-text role="ref"}).
    - a permissions document (`//CycloneDDS/Domain/Security/AccessControl/Permissions`{.interpreted-text role="ref"}).
    - The permissions CA certificate (`//CycloneDDS/Domain/Security/AccessControl/PermissionsCA`{.interpreted-text role="ref"}).

These values can be provided as CDATA or by using a path to a file (Similar to the authentication plugin properties).

> 这些值可以作为 CDATA 或使用文件路径提供（类似于身份验证插件财产）。

## Cryptography properties {#Cryptography Properties}

The cryptography plugin has no configuration properties.

> 加密插件没有配置财产。
