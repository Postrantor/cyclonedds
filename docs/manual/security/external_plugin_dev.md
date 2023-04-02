# External plugin development {#external_plugin_dev}

has three built-in security plugins that comply with the OMG specification:

- Authentication
- AccessControl
- Cryptographic

Security plugins are dynamically loaded. The locations are defined in configuration or participant QoS settings, see `DDS_security`{.interpreted-text role="ref"}.

You can add your own custom plugin in an API by implementing according to the OMG specification. You can implement all of the plugins or just one of them.

## Interface

Implement all plugin-specific functions with exactly same prototype. Plugin-specific function interfaces are in the following header files:

- _dds_security_api_access_control.h_
- _dds_security_api_authentication.h_
- _dds_security_api_cryptography.h_

## `init` and `finalize` functions

A plugin must have an `init` and a `finalize` functions. The `plugin_init` and `plugin_finalize` interfaces are found in the _dds_security_api.h_ header file. The functions must be same as in the configuration file.

- After the plugin is loaded, the `init` function is called.
- Before the plugin is unloaded, the `finalize` function is called.

## Inter-plugin communication

Within the authentication and cryptography plugins, there is a shared object (_DDS_Security_SharedSecretHandle_).

To implement one of the security plugins, and use the built-in for the other one, you must get, or provide the shared object:

- _DDS_Security_SharedSecretHandle_ is the integer representation of the _DDS_Security_SharedSecretHandleImpl_ struct object.
- The cryptography plugin gets the _DDS_Security_SharedSecretHandle_ from the authentication plugin and casts to the _DDS_Security_SharedSecretHandleImpl_ struct.

All required information can be retrieved through the _DDS_Security_SharedSecretHandleImpl_ struct:

```c
typedef struct DDS_Security_SharedSecretHandleImpl {
    DDS_Security_octet* shared_secret;
    DDS_Security_long shared_secret_size;
    DDS_Security_octet challenge1[DDS_SECURITY_AUTHENTICATION_CHALLENGE_SIZE];
    DDS_Security_octet challenge2[DDS_SECURITY_AUTHENTICATION_CHALLENGE_SIZE];

} DDS_Security_SharedSecretHandleImpl;
```
