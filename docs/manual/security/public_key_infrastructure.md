# Public-key infrastructure {#public-key_infrastructure}

The comprehensive system required to provide public-key encryption and digital signature services is known as a Public-Key Infrastructure (PKI). The purpose of a PKI is to manage keys and certificates. By managing keys and certificates through a PKI, an organization establishes and maintains a trustworthy networking environment.

> 提供公钥加密和数字签名服务所需的综合系统被称为公钥基础设施。PKI 的目的是管理密钥和证书。通过 PKI 管理密钥和证书，组织建立并维护了一个值得信赖的网络环境。

**Public key cryptography**

Each user has a key pair, generated during the initial certificate deployment process. It consists of:

> 每个用户都有一个密钥对，在初始证书部署过程中生成。它包括：

- A public key, which is shared.
- A private key, which is not shared.

> - 一个共享的公钥。
> - 一个不共享的私钥。

Data is encrypted with the user\'s public key and decrypted with their private key.

> 数据用用户的公钥加密，用用户的私钥解密。

Digital signatures are also generated using public key cryptography (used for non-repudiation, authentication and data integrity).

> 数字签名也使用公钥加密（用于不可否认性、身份验证和数据完整性）生成。

::: {#identity_certificate}
**Identity certificate**
:::

This is an electronic document that proves the ownership of a public key. The certificate includes:

> 这是一份证明公钥所有权的电子文件。证书包括：

- Information about the key.

> -有关密钥的信息。

- Information about the identity of its owner (called the subject).

> -关于其所有者（称为主体）身份的信息。

- The digital signature of an entity that has verified the certificate\'s contents (called the issuer).

> -已验证证书内容的实体（称为颁发者）的数字签名。

If the signature is valid, and the software examining the certificate trusts the issuer, then it can use that key to communicate securely with the certificate\'s subject.

> 如果签名有效，并且检查证书的软件信任颁发者，那么它可以使用该密钥与证书的使用者安全通信。

::: {#certificate_authority}
**Certificate Authority (CA)**
:::

This issues user-certificates and acts as the chief agent of trust. When issuing a certificate to a user, the CA signs the certificate with its private key in order to validate it. During electronic transactions the CA also confirms that certificates are still valid. Certificates may be revoked for various reasons. For example, a user may leave the organization or they may forget their secret passphrase, the certificate may expire or become corrupt. This process is usually through the use of a Certificate Revocation List (CRL) which is a list of the certificates that have been revoked. Only the certificates that have been revoked appear on this list.

> 这将颁发用户证书，并充当信任的主要代理。向用户颁发证书时，CA 会使用其私钥对证书进行签名以验证证书。在电子交易过程中，CA 还会确认证书仍然有效。证书可能因各种原因被吊销。例如，用户可能会离开组织，或者他们可能忘记了自己的密钥短语，证书可能会过期或损坏。此过程通常通过使用证书吊销列表（CRL）来完成，该列表是已吊销的证书的列表。只有已吊销的证书才会显示在此列表中。

**Subject of identity certificate**

> **身份证明主体**

This is the identity to be secured. It contains information such as common name (CN), organization (OU), state (ST) and country (C).

> 这是需要保护的身份。它包含通用名称（CN）、组织单位（OU）、州（ST）和国家（C）等信息。

**Subject name**

> **使用者名称**

This is also knows as the distinguished name. It is the string representation of the certificate subject. For example:

> 这也被称为专有名称。它是证书使用者的字符串表示形式。例如：

> emailAddress=alice@zettascale.ist,CN=Alice,OU=IST,O=ADLink,ST=OV,C=NL

## Example PKI usage in DDS security

Alice and Bob are the DDS participants who have their private and public keys. Identitity Certificate Authority (ID CA) has its own self-signed certificate ([IdentityCA]{.title-ref} in the diagram). **ID CA** gets Alice\'s subject information and public key and generates an [IdentityCertificate]{.title-ref} for her. Alice\'s certificate includes her public key and certificate of **ID CA**; so that her certificate can be verified if it is really issued by ID CA.

> Alice 和 Bob 是 DDS 的参与者，他们拥有自己的私钥和公钥。身份证书颁发机构（ID CA）有自己的自签名证书（图中的[IdentityCA]｛.title-ref｝）**ID CA**获取 Alice\的主题信息和公钥，并为她生成[IdentityCertificate]｛.title-ref｝。Alice 的证书包括她的公钥和**ID CA**的证书；这样，如果她的证书真的是由 ID CA 颁发的，就可以对其进行验证。

![image](../_static/pictures/pki_overview.png){width="1000px"}

Access Control is configured with governance and permissions documents:

> 访问控制是用治理和权限文档配置的：

- A governance document defines the security behavior of domains and topics.

> -治理文档定义了域和主题的安全行为。

- A permissions document contains the permissions of the domain participant (topics, readers and writers), and binds them to an identity certificate by subject name (distinguished name).

> -权限文档包含域参与者（主题、读者和作者）的权限，并通过使用者名称（可分辨名称）将其绑定到身份证书。

Governance documents and Permissions documents are signed by **Permission CA**. Signed documents also contains Permissions CA certificate so that they can be verified that they are really issued by Permissions CA.

> 治理文档和权限文档由**权限 CA**签署。已签名的文档还包含权限 CA 证书，以便验证它们是否确实是由权限 CA 颁发的。

Authenticated participants perform a handshake with each other and generate a shared key by Diffie-Hellman key exchange. This shared key is used for encrypting/decrypting data with AES.

> 经过身份验证的参与者相互握手，并通过 Diffie-Hellman 密钥交换生成共享密钥。此共享密钥用于使用 AES 加密/解密数据。

During the handshake:

> 握手时：

- Alice checks Bob\'s certificate and Bob\'s Permissions document to see whether they are really issued by the ID CA certificate and Permissions CA Certificate that **she** has.

> -Alice 检查 Bob 的证书和 Bob 的权限文档，看看它们是否真的是由**她**拥有的 ID CA 证书和权限 CA 证书颁发的。

- Bob checks Alice\'s certificate and Alice\'s Permissions document to see whether they are really issued by the ID CA certificate and Permissions CA that **he** has.

> -Bob 检查 Alice 的证书和 Alice 的权限文档，看看它们是否真的是由**他**拥有的 ID CA 证书和权限 CA 颁发的。

Permissions documents can contain permissions for several identities. To establish a binding between an identity and its permissions, the subject name of an identity certificate can appear multiple times in a permissions document.

> 权限文档可以包含多个身份的权限。要在身份及其权限之间建立绑定，身份证书的使用者名称可以在权限文档中多次出现。

There are several ways to set up the certificates and signed configuration files to be used with Security, see `Example_security_configuration`{.interpreted-text role="ref"}.

> 有几种方法可以设置要与 Security 一起使用的证书和已签名的配置文件，请参阅`Example_Security_configuration`｛.depreted text role=“ref”｝。
