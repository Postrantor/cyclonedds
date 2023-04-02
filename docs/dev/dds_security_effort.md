# DDS Security effort

ADLink has decided to donate their Vortex OpenSplice DDS Security implementation to the Cyclone DDS project. However, that will not be a simple code drop.
This document catches all the work that is foreseen to port Vortex OpenSplice DDS Security to Cyclone DDS.

> ADLink 已决定将其 Vortex OpenSplice DDS Security 实现捐赠给 Cyclone DDS 项目。然而，这将不是一个简单的代码删除。
> 本文档涵盖了将 Vortex OpenSplice DDS Security 移植到 Cyclone DDS 的所有工作。

> [!NOTE]: 你了解 ADLink 的具体内容吗
> ADLink 是一家专注于嵌入式计算机、工业自动化和测量领域的公司。其产品包括嵌入式计算机、数据采集卡、工业控制器和智能存储系统等。
> 其中嵌入式计算机被广泛应用于工业控制、交通运输、医疗、智能家居等领域；数据采集卡则用于各种数据采集和测量应用；工业控制器则用于自动化生产线、机械设备等工业控制场合；智能存储系统则提供高效的数据存储和处理解决方案。
> ADLink 的产品以其可靠性、稳定性、灵活性和性能等特点，获得了广泛的应用和认可。
> ADLink 成立于 1995 年，总部位于台湾新北市中和区。目前，ADLink 在全球拥有近 20 个办事处和分支机构，产品销售覆盖全球 50 多个国家和地区。
>
> ADLink 的产品线包括：
>
> 1. 嵌入式计算机：主要包括面向各种嵌入式应用场景的工业计算机、物联网计算机、车载计算机等。
> 2. 数据采集卡：主要包括数模转换器、高速数据采集卡、图像采集卡、运动控制采集卡等。
> 3. 工业控制器：主要包括模块化控制器、工业控制器、运动控制器等。
> 4. 智能存储系统：主要包括固态硬盘、嵌入式固态硬盘、数据存储服务器等。
>
> 除了硬件产品外，ADLink 还提供了全面的软件支持和服务，包括操作系统、驱动程序、开发工具、系统集成等。同时，ADLink 还有自主的研发和制造能力，可以根据客户的需求提供定制化的解决方案。
>
> 总的来说，ADLink 作为一家专注于嵌入式计算机、工业自动化和测量领域的公司，其产品和服务都具有较高的品质和性能。

> [!NOTE]: 可以介绍一些关于 "Vortex OpenSplice DDS Security" 的内容吗@10:44:47
> Vortex OpenSplice DDS Security 是 Vortex OpenSplice DDS 的一个插件，它提供了一系列的安全机制，包括身份验证、访问控制、数据完整性、数据加密等，可以保护 DDS 数据传输的安全性和完整性。
>
> 其中，身份验证是通过在 DDS 的用户数据中增加身份认证数据来实现的，可以通过用户名和密码、数字证书、安全令牌等方式进行身份认证。访问控制是通过定义访问策略来实现的，可以限制特定用户或组对 DDS 数据的访问权限。数据完整性是通过加入摘要或签名等机制来实现的，可以保证 DDS 数据没有被篡改。数据加密是通过加密算法来实现的，可以保证传输的数据不被窃取并且只能被授权的用户解密。
>
> **Vortex OpenSplice DDS Security 的实现可以在不影响 DDS 性能的前提下保护系统的安全性**。
>
> 当使用 DDS 进行实时数据通信时，数据的安全性非常关键。Vortex OpenSplice DDS Security 提供了一种灵活的方法来实现 DDS 数据的安全保护。其主要功能包括：
>
> 1. 身份验证：使用用户名和密码、数字证书、安全令牌等方式来验证用户身份，以保证只有授权用户可以访问 DDS 数据。
> 2. 访问控制：使用访问控制列表 (ACL) 对用户或组的访问权限进行限制，从而确保只有被授权的用户或组可以对 DDS 数据进行读取或写入操作。
> 3. 数据完整性保护：通过在 DDS 数据中添加消息摘要来验证数据的一致性和完整性，以防止数据在传输过程中被篡改或损坏。
> 4. 数据加密：使用 Advanced Encryption Standard (AES) 等算法对传输的数据进行加密，保证数据的机密性。
>
> 在 Vortex OpenSplice DDS 中，DDS 安全性机制被实现为一个可插拔的组件式架构，可以轻松地将 DDS 安全性机制集成到应用程序中。此外，Vortex OpenSplice DDS Security 还支持多种安全性标准，如 Common Criteria、FIPS 等，并支持以模块化的方式进行部署，以满足不同应用场景的需求。

This document can be removed when DDS Security has been implemented.

**Table of contents**

- [DDS Security effort](#dds-security-effort)
  - [Definition of done](#definition-of-done)
  - [Footprint](#footprint)
  - [Multi process testing (done)](#multi-process-testing-done)
  - [Runtime library loading (done)](#runtime-library-loading-done)
  - [Hopscotch utility (done)](#hopscotch-utility-done)
  - [FSM utility (in progress)](#fsm-utility-in-progress)
  - [Port DDS Security plugin API (done)](#port-dds-security-plugin-api-done)
  - [De-Serializing messages in DDSI (done)](#de-serializing-messages-in-ddsi-done)
  - [De-Serializing security message parameters in DDSI (done)](#de-serializing-security-message-parameters-in-ddsi-done)
  - [Port DDS Security builtin plugins (in progress)](#port-dds-security-builtin-plugins-in-progress)
  - [Port DDSI DDS Security (in progress)](#port-ddsi-dds-security-in-progress)
  - [Move configuration (in progress)](#move-configuration-in-progress)
  - [Failure handling](#failure-handling)
  - [Multiple configurations](#multiple-configurations)
  - [Example](#example)
  - [QosProvider](#qosprovider)
  - [Data Tags (optional)](#data-tags-optional)

## Definition of done

When this document tells that a certain aspect is 'done', it means that it has been accepted into the security branch of the cyclonedds repository (<https://github.com/eclipse-cyclonedds/cyclonedds/tree/security>).

> 当这个文档告诉某个方面已经“完成”时，意味着它已经被 cyclonedds 存储库的安全分支所接受(<https://github.com/eclipse-cyclonedds/cyclonedds/tree/security>).

However, it is possible that various parts need some rework before the security branch can be merged into the cyclonedds master branch.

> 然而，在安全分支可以合并到 cycloneds 主分支之前，各个部分可能需要一些返工。

## Footprint

A non-functional requirement is that cyclonedds should be buildable without the DDS Security support in it. That will reduce the footprint (and possibly improve performance) for applications that don't need security.

> 一个非功能性要求是，cycloneds 应该可以在没有 DDS 安全支持的情况下构建。这将减少不需要安全性的应用程序的占地面积（并可能提高性能）。

For that, the ENABLE_SECURITY build option is introduced that translates into the DDSI_INCLUDE_SECURITY compile switch. However, the usage of that switch should not explode. That'll reduce the maintainability.

> 为此，引入了 ENABLE_SECURITY 构建选项，该选项转换为 DDSI_INCLUDE_SECURITY 编译开关。但是，该开关的使用不应该爆炸。这将降低可维护性。

For instance, the usage of the switch can be minimized by using functions that will reduce to an inline function that just returns a hardcode value when security is not included (otherwise they'll do some certain task). The compiler can use these inline functions to do clever stuff regarding footprint and performance.

> 例如，可以通过使用将减少为内联函数的函数来最小化开关的使用，该内联函数在不包括安全性时只返回硬编码值（否则他们将执行某些特定任务）。编译器可以使用这些内联函数在封装和性能方面做一些巧妙的事情。

There can be other solutions to decrease security footprint without impeding on the maintainability of the code by inserting the switch too much.

> 可以有其他解决方案来减少安全占用，而不会因为过多地插入开关而妨碍代码的可维护性。

## Multi process testing (done)

To properly test DDS Security, multi process testing will be necessary. This is not yet available in Cyclone DDS. See the [Multi Process Testing](multi_process_testing.md) document for more information.

> 为了正确测试 DDS 安全性，需要进行多进程测试。这在 Cyclone DDS 中尚不可用。有关详细信息，请参阅[Multi-Process Testing]（Multi_Process_Testing.md）文档。

## Runtime library loading (done)

The ddsi component needs to be able to load DDS Security plugins at runtime. These plugins are provided as libraries. Loading libraries at runtime is currently not possible in Cyclone DDS.

> ddsi 组件需要能够在运行时加载 DDS 安全插件。这些插件是作为库提供的。Cyclone DDS 目前无法在运行时加载库。

## Hopscotch utility (done)

This hash table is used by the Security plugins. Both versions on OpenSplice and Cyclone are equivalent. No additional effort is expected.

> 此哈希表由安全插件使用。OpenSplice 和 Cyclone 上的两个版本都是等效的。预计不会有额外的努力。

## FSM utility (in progress)

The Finite State Machine utility has been introduced in OpenSplice to support the handshake of DDS Security. This has to be ported to Cyclone.

> 在 OpenSplice 中引入了有限状态机实用程序，以支持 DDS 安全性的握手。必须将其运至旋风分离器。

However, it already has some technical dept, which should be removed before adding it to Cyclone. This means that a refactor should happen as part of the porting.

> 然而，它已经有了一些技术部门，在将其添加到 Cyclone 之前应该将其删除。这意味着重构应该作为移植的一部分进行。

The related DBTs should also be ported to Cyclone unit tests.

> 相关的 DBT 也应移植到 Cyclone 单元测试中。

It was decided to just port the FSM at the moment. The refactor will take place when trying to get the security branch into master.

> 目前决定只移植 FSM。重构将在尝试将安全分支放入 master 时进行。

## Port DDS Security plugin API (done)

The DDS Security plugin API are just a few header files. The ddsi component uses that to link against. The implementation of the API is done in the individual plugins. The plugins are [loaded at runtime](#loading) (when configured).

> DDS 安全插件 API 只是一些头文件。ddsi 组件使用它进行链接。API 的实现是在各个插件中完成的。插件是[在运行时加载]（#loading）（配置时）。

This means that ddsi can be DDS Security prepared (after building against this API) without there being actual DDS Security plugins.

> 这意味着 ddsi 可以在没有实际 DDS 安全插件的情况下进行 DDS 安全准备（在根据此 API 构建之后）。

It seems to be just a code drop of a number of header files. Maybe add some CMake module for both ddsi and the plugins to easily link against?

> 这似乎只是一些头文件的代码删除。也许可以为 ddsi 和插件添加一些 CMake 模块，以便轻松链接？

## De-Serializing messages in DDSI (done)

DDSI needs to be able to (de)serialize a few Security messages. In OpenSplice, some functionality of the database is used. This is unavailable in Cyclone.

> DDSI 需要能够（反）序列化一些安全消息。在 OpenSplice 中，使用了数据库的一些功能。这在 Cyclone 中不可用。

What is available is a serializer that uses marshaling operations (see, for for instance, m_ops in the dds_topic_descriptor struct).

> 可用的是一个使用封送处理操作的序列化程序（例如，请参阅 dds_topic_descriptor 结构中的 m_ops）。

The (de)serializing of the Security messages should be supported by supplying the m_ops sequences, message structs (if not yet available) and some convenience functions using both.

> 应通过提供 m_ops 序列、消息结构（如果还不可用）和使用这两者的一些便利函数来支持安全消息的（反）序列化。

## De-Serializing security message parameters in DDSI (done)

DDSI needs to be able to (de)serialize a few message parameters that have been introduced by the DDS Security spec.

> DDSI 需要能够（反）序列化 DDS Security 规范引入的一些消息参数。

## Port DDS Security builtin plugins (in progress)

No major changes between the DDS Security plugins in OpenSplice and Cyclone are expected.

> 预计 OpenSplice 和 Cyclone 中的 DDS 安全插件之间不会有重大变化。

The DDS Security plugins require OpenSSL. Cyclone DDS already uses OpenSSL. However, it expects (or at least it's preferred to have) version 1.1 or newer, while the OpenSplice Security plugins are build against 1.0.2. There are some API changes between the two versions. This will take some porting effort.

> DDS 安全插件需要 OpenSSL。Cyclone DDS 已经使用了 OpenSSL。然而，它期望（或者至少首选）1.1 或更高版本，而 OpenSplice Security 插件是针对 1.0.2 构建的。这两个版本之间有一些 API 更改。这将需要一些移植工作。

The build system should be ported from makefiles to cmake files.

> 构建系统应该从 makefile 移植到 cmake 文件。

There are security_plugin DBTs in OpenSplice. These tests are base on cunit, which is also used in Cyclone. However, it is used slightly different. A small porting effort is expected (i.e. let it work with cmake and runner generation).

> OpenSplice 中有 security_plugin-DBT。这些测试是基于 cunit 的，cunit 也用于 Cyclone。然而，它的使用略有不同。预计会有少量的移植工作（即让它与 cmake 和 runner 生成一起工作）。

This means some additional effort, compared to just a code drop. But it is not expected to be major.

> 这意味着要付出一些额外的努力，而不仅仅是减少代码。但预计不会是重大事件。

- Authentication plugin (done).
- Access Control plugin (in progress).
- Cryptography plugin (done).

There are a few sub-features that can be implemented separately.

> 有一些子功能可以单独实现。

- Check/handle expiry dates (in progress).
- Trusted directory support.
- etc?

## Port DDSI DDS Security (in progress)

There is already quite a bit of difference between the DDSI codebases in OpenSplice and Cyclone. So, the copy/merge of the DDSI Security code from OpenSplice to Cyclone will not be trivial.

> OpenSplice 和 Cyclone 中的 DDSI 代码库之间已经有了相当大的差异。因此，将 DDSI 安全代码从 OpenSplice 复制/合并到 Cyclone 将不是一件小事。

Most parts of the merging will not be trivial, but should be quite straightforward nonetheless. Things that are noticed to be somewhat different between the DDSI code bases that could impact the merging work:

> 合并的大多数部分都不是微不足道的，但应该非常简单。注意到 DDSI 代码库之间有些不同，可能会影响合并工作：

- Entity matching is slightly different.
- The q_entity.c has a lot of differences that can obfuscate the differences related to DDS Security.
- Unacked messages logic has changed a bit. Does that impact gaps?
- (De)serializing, of course (see also [De-Serializing in DDSI](#deserializing)).
- Writer history cache is different, which can impact the builtin volatile Security endpoints.
- Unknown unknowns.

The buildsystem has to be upgraded.

- A few files are added which are easy to add to cmake.
- There's a new dependency on the [DDS Security API](#port-api), which is done.

Then, of course, there are the tests First of all, [Multi Process Testing](#testing) should be available, which now it is. When that's the case, then the OpenSplice tests business logic have to be ported from scripts and applications to that new framework. That porting shouldn't be that hard. However, it will probably take a while.

> 当然，还有测试。首先，[多进程测试]（#Testing）应该是可用的，现在就是这样。当这种情况发生时，OpenSplice 测试业务逻辑必须从脚本和应用程序移植到新的框架。移植不应该那么难。然而，这可能需要一段时间。

The DDSI Port doesn't have to be a big bang. It can be split up into various different pull requests. Examples are

> DDSI 端口不一定是一个大爆炸。它可以分为各种不同的拉取请求。例如

- Extend configuration XML parsing with the security configuration (done).
- Extend ddsi_qos with security related policies. Fill them with values from the configuration when applicable (done).
- Add DDS Security endpoints that are non-volatile (done).
- Add DDS Security endpoint that is volatile. This change has more impact than all the non-volatile endpoints combined (done).
- Handshake (in progress).
- Payload (en)(de)coding (DDSI support: done. Wrapper: todo).
- Submsg (en)(de)coding (DDSI support: done. Wrapper: todo).
- RTPSmsg (en)(de)coding (DDSI support: done. Wrapper: todo).
- Etc

## Move configuration (in progress)

After the port, the DDS Security configuration is still (partly) done through the overall configuration XML file (rest is coming from the permissions and governance files). However, according to the specification, the configuration should happen by means of the Participant QoS.

> 在端口之后，DDS 安全配置仍然（部分）通过整个配置 XML 文件完成（其余来自权限和管理文件）。然而，根据规范，配置应该通过参与者 QoS 来进行。

The ddsc dds_qos_t is mapped on the ddsi xqos. The ddsi xqos already has the necessary policy (after the [port](#port-ddsi)), namely the property_policy. This means that the ddsc qos itself is automatically prepared. However, getting and setting policies are done through getter and setter functions in ddsc. This means we have to add these functions for the security configuration values.

> ddsc dds_qos_t 映射在 ddsi xqos 上。ddsi-xqos 已经有了必要的策略（在[port]（#port-ddsi）之后），即 property_policy。这意味着 ddscqos 本身是自动准备的。然而，获取和设置策略是通过 ddsc 中的 getter 和 setter 函数完成的。这意味着我们必须为安全配置值添加这些函数。

The ddsc policy getter and setter functions use (arrays of) primitive types as arguments. The configuration of Security is given by means of the property policy, which isn't a primitive. To keep in line with the QoS API, we could add something like:

> ddsc 策略 getter 和 setter 函数使用（数组）基元类型作为参数。Security 的配置是通过属性策略给出的，而属性策略不是基元。为了与 QoS API 保持一致，我们可以添加以下内容：

```cpp
typedef struct dds_properties_t; /* opaque type in API, but mapped to
                                    ddsi_property_qospolicy_t internally */
dds_properties_t *dds_properties_create();
void dds_properties_delete(dds_properties_t *);
void dds_properties_merge(dds_properties_t *, dds_properties_t *);
void dds_properties_add_property(dds_properties_t *, char *name, char *value);
void dds_properties_add_binaryproperty(dds_properties_t *, char *name,
                                       uchar *value, int valuelength);
void dds_qset_properties(dds_qos_t*, dds_properties_t *);
void dds_qget_properties(dds_qos_t*, dds_properties_t **);
```

But this is very preliminary and is still up for debate.

After moving the Security configuration to the participant QoS, it's possible to have different configurations within a single application if you have multiple participants. However, ddsi only supports one Security configuration for now. That doesn't change by changing where that configuration comes from. To solve this, it is expected that creation of a participant with a different configuration will force a failure for now. Until [Multiple Configurations](#multiple-configurations) is implemented.

> 在将安全配置移动到参与者 QoS 之后，如果您有多个参与者，则可以在单个应用程序中使用不同的配置。然而，ddsi 目前只支持一种安全配置。这不会因为改变配置的来源而改变。为了解决这个问题，预计创建一个具有不同配置的参与者将暂时导致失败。直到实现[多个配置]（#多个配置）。

After the ddsc API has been extended, we can decide on what to do with the configuration through XML.

> 在 ddsc API 被扩展之后，我们可以决定如何通过 XML 处理配置。

- Keep it. It seems usable: no need to change applications when changing (or adding) Security settings. However, conflicts between XML and QoS configuration could cause problems. Simplest seems to be to only allow QoS security configuration when it's not configured in XML already.
- Remove it. No conflict resolving needed.

> - 保留它。它看起来很有用：在更改（或添加）安全设置时无需更改应用程序。然而，**XML 和 QoS 配置之间的冲突可能会导致问题**。最简单的方法似乎是只允许在尚未在 XML 中配置 QoS 安全性时进行配置。
> - 删除它。不需要解决冲突。

All the Security tests depend on providing (different) configurations through XML. Depending on if we keep or remove the XML configuration option, a lot of tests have to be updated, or a few added (that test security configuration through QoS).

> 所有的安全性测试都依赖于通过 XML 提供（不同的）配置。根据我们是保留还是删除 XML 配置选项，必须更新许多测试，还是添加一些测试（通过 QoS 测试安全配置）。

For the loading of the plugin libraries, properties with specific names have to be added to the property policy to know the location and names of the plugins. As inspiration, fastrtps can be used: <https://github.com/ros2/rmw_fastrtps/blob/master/rmw_fastrtps_shared_cpp/src/rmw_node.cpp#L296>

> 为了加载插件库，必须将具有特定名称的财产添加到属性策略中，以了解插件的位置和名称。作为灵感来源，可以使用 fastrtps：<https://github.com/ros2/rmw_fastrtps/blob/master/rmw_fastrtps_shared_cpp/src/rmw_node.cpp#L296>

## Failure handling

Currently, when an local action is tried that isn't allowed by DDS Security (like creating a participant when it's not permitted), DDSI is shut down. Mainly because in OpenSplice it's quite hard to get a failure state from DDSI to the application.

> 目前，当尝试 **DDS Security 不允许的本地操作时**（比如在不允许的情况下创建参与者），DDSI 会关闭。**主要是因为在 OpenSplice 中，很难从 DDSI 获得应用程序的故障状态**。

In Cyclone, however, ddsc::dds_create_participant() results in a direct call to ddsi::ddsi_new_participant(). This means that if creation of an entity (or participant in this example) fails due to security issues in ddsi, we can fail the actual ddsc API call with a proper error result (there's already the DDS_RETCODE_NOT_ALLOWED_BY_SECURITY in the ddsc API (not used)).

> 然而，在 Cyclone 中，ddsc::dds_create_participant（）会直接调用 ddsi::ddsi_new_participant.（）。这意味着，如果实体（或本例中的参与者）的创建由于 ddsi 中的安全问题而失败，我们可以使实际的 ddsc API 调用失败，并得到正确的错误结果（ddsc API 中已经有 DDS_RETCODE_NOT_ALLOWED_BY_security（未使用））。

Maybe we have to do some additional cleanup when a failure is encountered.

> 当遇到故障时，也许我们必须做一些额外的清理工作。

Some tests probably have to be adjusted for the new behaviour.

> 有些测试可能需要根据新的行为进行调整。

## Multiple configurations

Currently (because it's done through the overall XML configuration), only one DDS Security configuration could be supported. Because of this fact, at various locations, shortcuts could be made in both DDSI and plugins. However, because the configuration is coming from participants now (see [Move Configuration](#Move-configuration), we should be able to support multiple different DDS Security configurations.

> 目前（因为它是通过整个 XML 配置完成的），**只能支持一个 DDS 安全配置**。正因为如此，在不同的位置，可以在 DDSI 和插件中创建快捷方式。然而，由于配置现在来自参与者（请参阅[Move configuration]（#Move configuration），我们**应该能够支持多种不同的 DDS 安全配置**。

Until now, the creation of a second participant with a different configuration would force a failure (again, see [Move Configuration](#Move-configuration)).

> 到目前为止，创建具有不同配置的第二个参与者将导致失败（再次参见[Move configuration]（#Move configuration））。

It is expected that the plugin loading still happens through the configuration XML (see [Move Configuration](#Move-configuration)). This means that DDSI doesn't have to support multiple sets of plugins. Just the one set, provided at initialization. This means that DDSI shouldn't have to be changed to support this.

> 预计插件加载仍然通过配置 XML 进行（请参阅[Move configuration]（#Move configuration））。这意味着 DDSI 不必支持多组插件。只有一个集合，在初始化时提供。这意味着不应该为了支持这一点而更改 DDSI。

So, it's the plugins need to be able to support multiple configurations.

> 因此，插件需要能够支持多种配置。

The Cryptography plugin doesn't seem to care about global DDS Security configurations. It has basically configurations per participant/topic/ endpoints, which already works. So, this plugin doesn't have to be upgraded.

> Cryptography 插件似乎并不关心全局 DDS 安全配置。它基本上有每个参与者/主题/端点的配置，这已经起作用了。所以，这个插件不需要升级。

The Authentication plugin does have global DDS Security configurations. Main function related to that is validate_local_identity(). This function already creates a new local identity every time it is called. So, this plugin doesn't have to be upgraded either.

> 身份验证插件具有全局 DDS 安全配置。与此相关的主要函数是 validate_local_identity（）。此函数在每次调用时都会创建一个新的本地标识。所以，这个插件也不需要升级。

That leaves the Access Control plugin. The main function related to configuration is validate_local_permissions(). This function creates access rights depending on Permissions and Governance files. Currently, there's only one local 'rights' structure that is linked directly to the plugin (see also the ACCESS_CONTROL_USE_ONE_PERMISSION compile switch). This has to change. The local rights structure needs to be coupled to a participant. This also means that we have to search for it instead of having direct access when entering the plugin. The remote rights can be used as example. That is basically a list of rights/ permissions with the remote identity handle as key.

> 这就留下了访问控制插件。与配置相关的主要功能是 validate_local_permissions（）。此功能根据权限和治理文件创建访问权限。目前，只有一个本地“权限”结构直接链接到插件（另请参阅 ACCESS_CONTROL_USE_one_PERMISSION 编译开关）。这种情况必须改变。本地权限结构需要与参与者耦合。这也意味着我们必须搜索它，而不是在输入插件时直接访问它。远程权限可以作为示例。这基本上是一个以远程标识句柄为密钥的权限列表。

Tests have to be added to make sure that a setup with different Security configurations works.

> 必须添加测试，以确保具有不同安全配置的设置能够正常工作。

## Example

A Security example has to be added.

## QosProvider

The Participant QoS now contains Security related information. This means that the QosProvider has to be upgraded to support that.

> 参与者 QoS 现在包含与安全相关的信息。这意味着必须升级 QosProvider 才能支持这一点。

## Data Tags (optional)

The specification is somewhat fuzzy about the data tags. The following is a summary (still elaborate) of how it seems to work:

> 该规范对数据标签有些模糊。
> 以下是它的工作原理摘要（仍在详细说明）：

The permissions document can contain the tag on publish and subscribe level. It's related to the data samples, but don't have to be related to the keys of those samples. The QoS of a writer/reader can also have data tags by means of the DataTagQosPolicy. A writer/reader can only be created when the data_tags in the QoS matches those in the permissions document. This check should happen on both the local and remote level.

> 权限文档可以包含发布和订阅级别的标记。它与数据样本相关，但不必与这些样本的键相关。写入器/读取器的 QoS 也可以通过 DataTagQosPolicy 具有数据标签。只有当 QoS 中的 data_tag 与权限文档中的 data_Tag 匹配时，才能创建编写器/读取器。此检查应在本地和远程级别上进行。

This creation check is the only thing that DDS Security actually does with the data tags. They are only authenticated by DDS Security, but not interpreted further. This is only a minor security addition, because the publisher can still publish data that doesn't match the data tags because DDS doesn't interpret the data nor compares it with the data tags.

> 这个创建检查是 DDSSecurity 对数据标记实际执行的唯一操作。它们仅通过 DDS Security 进行身份验证，但不会进行进一步解释。这只是一个小的安全添加，因为发布者仍然可以发布与数据标记不匹配的数据，因为 DDS 既不解释数据，也不将其与数据标记进行比较。

What it can be used for is a kind of custom access control scheme on the application level. An application that consumes data can see if a publisher is allowed to publish that sample by comparing the data within the sample with the data tag(s) associated with that publisher. As said, this comparison is not done on the DDS level, but has to be done within the application itself.

> 它可以用于应用程序级别上的一种自定义访问控制方案。使用数据的应用程序可以通过将样本中的数据与与发布者关联的数据标记进行比较来查看是否允许发布者发布该样本。如上所述，这种比较不是在 DDS 级别上进行的，而是必须在应用程序本身内进行的。

That leaves the question, how does the application get the tags associated with the related writer? In other words; the application gets a sample. It has to know from which writer it originated and it has to have access to the data tag(s) of that writer. The dds_sample_info_t contains the dds_instance_handle_t publication_handle, which is unique to the writer (locally). That dds_instance_handle_t can be used to get the right DDS_Security_PublicationBuiltinTopicDataSecure sample for the related secure builtin reader [**note1**]. DDS_Security_PublicationBuiltinTopicDataSecure contains QoS information regarding that writer, including the DataTagQosPolicy. The remote DDS_Security_PublicationBuiltinTopicDataSecure contents have been authenticated by DDS Security and the data tags can be trusted. The application can check the sample data against the data_tags within that QoS.

> 这就留下了一个问题，应用程序是如何获得与相关编写器相关联的标签的？换句话说；应用程序得到一个示例。它必须知道它来自哪个编写器，并且必须能够访问该编写器的数据标签。**dds_sample_info_t 包含 dds_instance_handle_t publication_handle**，它对编写器（本地）是唯一的。该 dds_instance_handle_t 可用于为相关的安全内置读取器[**note1**]获取正确的 dds_Security_PublicationBuiltinTopicDataSecure 示例。DDS_Security_PublicationBuiltinTopicDataSecure 包含有关该写入程序的 QoS 信息，包括 DataTagQosPolicy。远程 DDS_Security_PublicationBuiltinTopicDataSecure 内容已通过 DDS Security 的身份验证，并且可以信任数据标记。应用程序可以对照该 QoS 内的 data_tag 来检查样本数据。

Things to do:

- Add DataTagQosPolicy to the ddsc API and the related QoSses.
- Add DDS_Security_PublicationBuiltinTopicDataSecure data type to the ddsc API (better yet, all secure builtin types).
- Add the related builtin reader to the ddsc99 API (better yet, all secure builtin readers).
- Add test regarding the secure builtin readers and data.
- Add data tag comparisons between QoS and permission documents during local and remote entity creation.
- Add data tag remote/local (mis)match tests.

Especially because of the lack of access to builtin secure readers, supporting data tags doesn't seem feasible in the near future. Also, it's optional in the specification.

> 特别是由于缺乏对内置安全读取器的访问，支持数据标签在不久的将来似乎不可行。此外，它在规范中是可选的。

**note1** That DDS_Security_PublicationBuiltinTopicDataSecure reader is not yet available within the ddsc API, nor is the related data type. Don't know how much work it would be to add them to that API.

> **note1**DDS_Security_PublicationBuiltinTopicDataSecure 读取器在 ddsc API 中尚不可用，相关数据类型也不可用。不知道要将它们添加到那个 API 中需要做多少工作。
