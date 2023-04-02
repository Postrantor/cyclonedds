::: index
single: Testing your installation single: Installation; Testing single: HelloWorld
:::

# Test your installation {#test\_\_install}

To test if your installation and configuration are working correctly, either:

- Use the `dsperf_tool`{.interpreted-text role="ref"}

  : The `ddsperf` tool sends a continuous stream of data at a variable frequency. This is useful for sanity checks and to bypass other sporadic network issues.

- Use the `helloworld_test`{.interpreted-text role="ref"} example.

  : The **Hello World!** example sends a single message.

::: index
HelloWorld
:::

## HelloWorld {#helloworld_test}

To test your installation, includes a simple **HelloWorld!** application (see also the `helloworld_bm`{.interpreted-text role="ref"} example). **HelloWorld!** consists of two executables:

> - `HelloworldPublisher`
> - `HelloworldSubscriber`

The **HelloWorld!** executables are located in:

- `<cyclonedds-directory>\build\bin\Debug` on Windows
- `<cyclonedds-directory>/build/bin` on Linux/macOS.

::: note
::: title
Note
:::

Requires CMake with `-DBUILD_EXAMPLES=ON`.
:::

::: note
::: title
Note
:::

There are some common issues with multiple network interface cards on machine configurations. The default behavior automatically detects the first available network interface card on your machine for exchanging the `hello world` message. To ensure that your publisher and subscriber applications are on the same network, you must select the correct interface card. To override the default behavior, create or edit a deployment file (for example, `cyclonedds.xml`) and update the property `//CycloneDDS/Domain/General/Interfaces/NetworkInterface[@address]`{.interpreted-text role="ref"} to point to it through the `CYCLONEDDS\_URI` OS environment variable. For further information, refer to `config-docs`{.interpreted-text role="ref"} and the `configuration_reference`{.interpreted-text role="ref"}.

> 机器配置上的多个网络接口卡存在一些常见问题。 默认行为会自动检测您机器上第一个可用的网络接口卡以交换“hello world”消息。 为确保您的发布者和订阅者应用程序在同一网络上，您必须选择正确的接口卡。 要覆盖默认行为，请创建或编辑部署文件（例如，`cyclonedds.xml`）并更新属性`//CycloneDDS/Domain/General/Interfaces/NetworkInterface[@address]`{.interpreted-text role= "ref"} 通过 `CYCLONEDDS\_URI` 操作系统环境变量指向它。 如需更多信息，请参阅`config-docs`{.interpreted-text role="ref"} 和`configuration_reference`{.interpreted-text role="ref"}。

:::
