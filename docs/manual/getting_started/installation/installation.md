# Installation {#installation_bm}

The core of is implemented in C and provides C-APIs to applications. Additional language bindings are:

- C++

  : This API wraps the C core and makes it easy to create portable and interoperable distributed systems in C++. For further information, refer to

- Python

  : Build portable and interoperable distributed systems with the ease and simplicity of modern, idiomatic Python. For further information, refer to

## Supported platforms

The supports three primary platforms:

- Linux
- macOS
- Windows

Code examples are provided for each of these platforms throughout the documentation. For example:

::: tabs
::: group-tab
Linux

Linux-specific information
:::

Other platforms where is supported (with some caveats):

- FreeRTOS
- QNX
- Openindiana OS, which is similar to Solaris

::: note
::: title
Note
:::

has not been extensively tested on these other platforms and therefore there may be unexpected results.
:::

::: index
Software prerequisites, Prerequisites
:::

## Prerequisites {#software_prerequisites}

Install the following software on your machine:

> - A C compiler (For example, GCC or Clang on Linux, Visual Studio on Windows (MSVC), Clang on macOS).
> - version control system.
> - , version 3.10 or later, see `cmake_config`{.interpreted-text role="ref"}.
> - Optionally, , preferably version 1.1 later to use TLS over TCP.

To obtain the dependencies for , follow the platform-specific instructions:

::: tabs
::: group-tab
Linux

To install the dependencies, use a package manager. For example:

```bash
yum install git cmake gcc
apt-get install git cmake gcc
aptitude install git cmake gcc
# or others
```

:::

::: group-tab
macOS

Install XCode from the App Store.
:::

::: group-tab
Windows

Install Visual Studio Code for the C compiler, then install the .

```bash
choco install cmake
choco install git
```

Alternatively, to install the dependencies, use .
:::
:::

### Additional tools {#tools}

While developing for , additional tools and dependencies may be required. The following is a list of the suggested tools:

> - Shared memory
>
>   :
>
> - Unit testing / Development
>
>   :
>
> - Documentation
>
>   :
>
> - Security
>
>   :

## Language-specific installation

::: tabs
::: group-tab
Core DDS (C)
:::

::: group-tab
C++ .. include:: installation.cpp.rst
:::

::: group-tab
Python .. include:: installation.python.rst
:::
:::

## Windows environment variables

To run executables on Windows, the required libraries (`ddsc.dll` and so on) must be available to the executables. Typically, these libraries are installed in system default locations and work out of the box. However, if they are not installed in those locations, you must change the library search path, either:

> 要在 Windows 上运行可执行文件，所需的库（`ddsc.dll` 等）必须可用于可执行文件。 通常，这些库安装在系统默认位置并且开箱即用。 但是，如果它们未安装在这些位置，则必须更改库搜索路径，或者：

- Execute the following command:

```PowerShell
set PATH=<install-location>\bin;%PATH%
```

- Set the path from the \"Environment variables\" Windows menu.

::: note
::: title
Note
:::

An alternative to make the required libraries available to the executables are to copy the necessary libraries for the executables\' directory. This is not recommended.
:::

::: index
single: Testing your installation single: Installation; Testing single: HelloWorld
:::

## Test your installation {#test\_\_install}

To test if your installation and configuration are working correctly, either:

- Use the `dsperf_tool`{.interpreted-text role="ref"}

  : The `ddsperf` tool sends a continuous stream of data at a variable frequency. This is useful for sanity checks and to bypass other sporadic network issues.

- Use the `helloworld_test`{.interpreted-text role="ref"} example.

  : The **Hello World!** example sends a single message.

::: index
HelloWorld
:::

### HelloWorld {#helloworld_test}

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

There are some common issues with multiple network interface cards on machine configurations. The default behavior automatically detects the first available network interface card on your machine for exchanging the `hello world` message. To ensure that your publisher and subscriber applications are on the same network, you must select the correct interface card. To override the default behavior, create or edit a deployment file (for example, `cyclonedds.xml`) and update the property `//CycloneDDS/Domain/General/Interfaces/NetworkInterface[@address]`{.interpreted-text role="ref"} to point to it through the `CYCLONEDDS_URI` OS environment variable. For further information, refer to `config-docs`{.interpreted-text role="ref"} and the `configuration_reference`{.interpreted-text role="ref"}.

> 机器配置上的多个网络接口卡存在一些常见问题。 默认行为会自动检测您机器上第一个可用的网络接口卡以交换“hello world”消息。 为确保您的发布者和订阅者应用程序在同一网络上，您必须选择正确的接口卡。 要覆盖默认行为，请创建或编辑部署文件（例如，`cyclonedds.xml`）并更新属性`//CycloneDDS/Domain/General/Interfaces/NetworkInterface[@address]`{.interpreted-text role= "ref"} 通过 `CYCLONEDDS_URI` 操作系统环境变量指向它。 如需更多信息，请参阅`config-docs`{.interpreted-text role="ref"} 和`configuration_reference`{.interpreted-text role="ref"}。

:::

## Uninstalling

To uninstall , manually remove the install and build directories:

::: tabs
::: group-tab
Linux

```bash
rm -rf cyclonedds
rm -rf <install-location>
```

:::

::: group-tab
macOS

```bash
rm -rf cyclonedds
rm -rf <install-location>
```

:::

::: group-tab
Windows

Navigate to your install location and remove the directory.
:::
:::
