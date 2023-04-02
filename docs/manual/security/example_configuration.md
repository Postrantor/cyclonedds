# Example DDS security configuration {#Example_security_configuration}

This section shows an example configuration for DDS Security.

The steps for configuring DDS Security are:

1.  `create_certificate_authority`{.interpreted-text role="ref"}
2.  `create_identity_certificate`{.interpreted-text role="ref"}
3.  `create_governance_document`{.interpreted-text role="ref"}
4.  `create_permissions_document`{.interpreted-text role="ref"}
5.  Either:

> - `set_security_properties`{.interpreted-text role="ref"}
> - `apply_security_settings`{.interpreted-text role="ref"}

::: index
Certificate authority; Creating
:::

## Create a permissions Certificate Authority (CA) {#create_certificate_authority}

To generate the CA for identity management (authentication):

1.  Create the private key for the CA:

    ```bash
    openssl genrsa -out example_id_ca_priv_key.pem 2048
    ```

2.  Create the certificate for the identity CA (which is a self-signed certificate):

    ```bash
    openssl req -x509 -key example_id_ca_priv_key.pem -out example_id_ca_cert.pem -days 3650 -subj "/C=NL/ST=OV/L=Locality Name/OU=Example OU/O=Example ID CA Organization/CN=Example ID CA/emailAddress=authority@cycloneddssecurity.zettascale.com"
    ```

3.  Create the private key of the permissions CA (used for signing the AccessControl configuration files):

    ```bash
    openssl genrsa -out example_perm_ca_priv_key.pem 2048
    ```

4.  Create the self-signed certificate for the permissions CA:

    ```bash
    openssl req -x509 -key example_perm_ca_priv_key.pem -out example_perm_ca_cert.pem -days 3650 -subj "/C=NL/ST=OV/L=Locality Name/OU=Example OU/O=Example CA Organization/CN=Example Permissions CA/emailAddress=authority@cycloneddssecurity.zettascale.com"
    ```

::: index
Identity certificate; Creating
:::

## Create an identity certificate {#create_identity_certificate}

Create an identity certificate (signed by the CA), and the private key corresponding to an identity named _Alice_.

::: note
::: title
Note
:::

These steps need to be repeated for each identity in the system.
:::

To create a private key and an identity certificate for an identity named _Alice_:

1.  Create the **private key** for _Alice\'s_ identity:

    ```bash
    openssl genrsa -out example_alice_priv_key.pem 2048
    ```

2.  To request that the identity CA generates a certificate, create a Certificate Signing Request (`CSR`{.interpreted-text role="term"}):

    ```bash
    openssl req -new -key example_alice_priv_key.pem -out example_alice.csr -subj "/C=NL/ST=OV/L=Locality Name/OU=Organizational Unit Name/O=Example Organization/CN=Alice Example/emailAddress=alice@cycloneddssecurity.zettascale.com"
    ```

3.  Create _Alice\'s_ **identity certificate**:

    ```bash
    openssl x509 -req -CA example_id_ca_cert.pem -CAkey example_id_ca_priv_key.pem -CAcreateserial -days 3650 -in example_alice.csr -out example_alice_cert.pem
    ```

4.  In the DDS Security authentication configuration:

    - Use _Alice\'s_ private key (example_alice_priv_key.pem) file for the `PrivateKey <//CycloneDDS/Domain/Security/Authentication/PrivateKey>`{.interpreted-text role="ref"} setting.
    - Use _Alice\'s_ identity certificate (example_alice_cert.pem) file for the `IdentityCertificate <//CycloneDDS/Domain/Security/Authentication/IdentityCertificate>`{.interpreted-text role="ref"} setting.
    - Use the certificate of the CA used for signing this identity (example_id_ca_cert.pem), for the `IdentityCA <//CycloneDDS/Domain/Security/Authentication/IdentityCA>`{.interpreted-text role="ref"} setting.

::: index
Governance document; Creating
:::

## Create a signed governance document {#create_governance_document}

The following shows an example of a governance document that uses _signing for submessage_ and an encrypted payload:

::: {.literalinclude linenos="" language="xml"}
\_static/example_governance.xml
:::

The governance document must be signed by the `permissions CA <create_certificate_authority>`{.interpreted-text role="ref"}.

To sign the governance document:

```bash
openssl smime -sign -in example_governance.xml -text -out example_governance.p7s -signer example_perm_ca_cert.pem -inkey example_perm_ca_priv_key.pem
```

::: index
Permissions document; Creating
:::

## Create a signed permissions document {#create_permissions_document}

The permissions document is an XML document that contains the permissions of the participant and binds them to the subject name in the identity certificate (distinguished name) for the participant as defined in the DDS .

An example of a permissions document:

::: {.literalinclude linenos="" language="xml"}
../\_static/example_permissions.xml
:::

This document also needs to be signed by the `permissions CA <create_certificate_authority>`{.interpreted-text role="ref"}:

```bash
openssl smime -sign -in example_permissions.xml -text -out example_permissions.p7s -signer example_perm_ca_cert.pem -inkey example_perm_ca_priv_key.pem
```

## Set the security properties in participant QoS {#set_security_properties}

The following code fragment shows how to set the security properties to a QoS object, and use this QoS when creating a participant:

::: {.literalinclude linenos="" language="c"}
../\_static/security_by_qos.c
:::

## Apply security settings {#apply_security_settings}

As an alternative for using the QoS, security settings can also be applied using the configuration XML. If both QoS and the configuration XML contain security settings, the values from the QoS is used and the security settings in the configuration XML are ignored.

> 作为使用 QoS 的替代方法，还可以使用配置 XML 应用安全设置。 如果 QoS 和配置 XML 都包含安全设置，则使用 QoS 中的值并忽略配置 XML 中的安全设置。

The following XML fragment shows how to set security settings through configuration:

::: {.literalinclude linenos="" language="xml"}
../\_static/security_by_config.xml
:::

To use this configuration file for an application, set the `CYCLONEDDS_URI` environment variable to this config file:

```bash
export CYCLONEDDS_URI=/path/to/secure_config.xml
```

::: note
::: title
Note
:::

This example configuration uses the attribute `id=any` for the `domain` element, any participant that is created (which implicitly creates a domain) in an application using this configuration gets these security settings.
:::
