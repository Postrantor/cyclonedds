![GitHub release](https://img.shields.io/github/v/release/eclipse-cyclonedds/cyclonedds?include_prereleases)
[![Build Status](https://dev.azure.com/eclipse-cyclonedds/cyclonedds/_apis/build/status/Pull%20requests?branchName=master)](https://dev.azure.com/eclipse-cyclonedds/cyclonedds/_build/latest?definitionId=4&branchName=master)
[![Coverity Status](https://scan.coverity.com/projects/19078/badge.svg)](https://scan.coverity.com/projects/eclipse-cyclonedds-cyclonedds)
[![Coverage](https://img.shields.io/azure-devops/coverage/eclipse-cyclonedds/cyclonedds/4/master)](https://dev.azure.com/eclipse-cyclonedds/cyclonedds/_build/latest?definitionId=4&branchName=master)
[![License](https://img.shields.io/badge/License-EPL%202.0-blue)](https://choosealicense.com/licenses/epl-2.0/)
[![License](https://img.shields.io/badge/License-EDL%201.0-blue)](https://choosealicense.com/licenses/edl-1.0/)
[![Website](https://img.shields.io/badge/web-cyclonedds.io-blue)](https://cyclonedds.io)
[![Community](https://img.shields.io/badge/discord-join%20community-5865f2)](https://discord.gg/BkRYQPpZVV)

# Eclipse Cyclone DDS

Eclipse Cyclone DDS is a very performant and robust open-source implementation of the [OMG DDS specification](https://www.omg.org/spec/DDS/1.4/About-DDS/).

> Eclipse Cyclone DDS 是[OMG DDS 规范]的一个非常高性能和健壮的开源实现(https://www.omg.org/spec/DDS/1.4/About-DDS/).

Cyclone DDS is developed completely in the open as an Eclipse IoT project (see [eclipse-cyclone-dds](https://projects.eclipse.org/projects/iot.cyclonedds)) with a growing list of [adopters](https://iot.eclipse.org/adopters/?#iot.cyclonedds) (if you're one of them, please add your [logo](https://github.com/EclipseFdn/iot.eclipse.org/issues/new?template=adopter_request.md)).

> Cyclone DDS 是作为一个 Eclipse 物联网项目完全公开开发的(请参阅[Eclipse Cyclone DDS](https://projects.eclipse.org/projects/iot.cyclonedds))越来越多的[采用者](https://iot.eclipse.org/adopters/?#iot.cyclonedds)(如果您是其中之一，请添加您的[徽标](https://github.com/EclipseFdn/iot.eclipse.org/issues/new?template=adopter_request.md)).

It is a tier-1 middleware for the Robot Operating System [ROS 2](https://docs.ros.org/en/rolling/).

> 它是机器人操作系统[ROS 2]的一级中间件(https://docs.ros.org/en/rolling/).

- [What is DDS?](#what-is-dds)
- [Getting Started](#getting-started)
- [Performance](#performance)
- [Configuration](#run-time-configuration)

# What is DDS?

DDS is the best-kept secret in distributed systems, one that has been around for much longer than most publish-subscribe messaging systems and still outclasses so many of them. DDS is used in a wide variety of systems, including air-traffic control, jet engine testing, railway control, medical systems, naval command-and-control, smart greenhouses and much more. In short, it is well-established in aerospace and defense but no longer limited to that. And yet it is easy to use!

> DDS 是分布式系统中保守得最好的秘密，它的存在时间比大多数发布-订阅消息传递系统长得多，但仍然超过了其中的许多系统。
> DDS 用于各种系统，包括空中交通控制、喷气发动机测试、铁路控制、医疗系统、海军指挥和控制、智能温室等。
> 简言之，它在航空航天和国防领域已经确立，但不再局限于此。
> 然而它很容易使用！

Types are usually defined in IDL and preprocessed with the IDL compiler included in Cyclone, but our [Python binding](https://github.com/eclipse-cyclonedds/cyclonedds-python) allows you to define data types on the fly:

> 类型通常在 IDL 中定义，并使用 Cyclone 中包含的 IDL 编译器进行预处理，但我们的[Python binding](https://github.com/eclipse-cyclonedds/cyclonedds-python)允许您动态定义数据类型：

```Python
from dataclasses import dataclass
from cyclonedds.domain import DomainParticipant
from cyclonedds.core import Qos, Policy
from cyclonedds.pub import DataWriter
from cyclonedds.sub import DataReader
from cyclonedds.topic import Topic
from cyclonedds.idl import IdlStruct
from cyclonedds.idl.annotations import key
from time import sleep
import numpy as np
try:
    from names import get_full_name
    name = get_full_name()
except:
    import os
    name = f"{os.getpid()}"

# C, C++ require using IDL, Python doesn't
@dataclass
class Chatter(IdlStruct, typename="Chatter"):
    name: str
    key("name")
    message: str
    count: int

rng = np.random.default_rng()
dp = DomainParticipant()
tp = Topic(dp, "Hello", Chatter, qos=Qos(Policy.Reliability.Reliable(0)))
dw = DataWriter(dp, tp)
dr = DataReader(dp, tp)
count = 0
while True:
    sample = Chatter(name=name, message="Hello, World!", count=count)
    count = count + 1
    print("Writing ", sample)
    dw.write(sample)
    for sample in dr.take(10):
        print("Read ", sample)
    sleep(rng.exponential())
```

Today DDS is also popular in robotics and autonomous vehicles because those really depend on high-throuhgput, low-latency control systems without introducing a single point of failure by having a message broker in the middle.

> 如今，DDS 在机器人和自动车辆中也很受欢迎，因为它们真正依赖于高通量、低延迟的控制系统，而不会通过在中间设置消息代理引入单点故障。

Indeed, it is by far the most used and the default middleware choice in ROS 2.

> 事实上，它是 ROS2 中使用最多的默认中间件选择。

It is used to transfer commands, sensor data and even video and point clouds between components.

> 它用于在组件之间传输命令、传感器数据，甚至视频和点云。

The OMG DDS specifications cover everything one needs to build systems using publish-subscribe messaging.

> OMG DDS 规范涵盖了使用发布-订阅消息构建系统所需的一切。

They define a structural type system that allows automatic endianness conversion and type checking between readers and writers.

> 它们定义了一个结构类型系统，允许在读写器之间进行自动的端序转换和类型检查。

This type system also supports type evolution.

> 这种类型系统也支持类型演化。

The interoperable networking protocol and standard C++ API make it easy to build systems that integrate multiple DDS implementations.

> 可互操作的网络协议和标准 C++API 使构建集成多个 DDS 实现的系统变得容易。

Zero-configuration discovery is also included in the standard and supported by all implementations.

> 零配置发现也包含在标准中，并受到所有实现的支持。

DDS actually brings more: publish-subscribe messaging is a nice abstraction over "ordinary" networking, but plain publish-subscribe doesn't affect how one _thinks_ about systems.

> DDS 实际上带来了更多：发布-订阅消息是对“普通”网络的一个很好的抽象，但普通的发布-订阅不会影响人们对系统的思考。

A very powerful architecture that truly changes the perspective on distributed systems is that of the "shared data space", in itself an old idea, and really just a distributed database.

> 真正改变分布式系统观点的一个非常强大的体系结构是“共享数据空间”，这本身就是一个古老的想法，实际上只是一个分布式数据库。

Most shared data space designs have failed miserably in real-time control systems because they provided strong consistency guarantees and sacrificed too much performance and flexibility.

> 大多数共享数据空间设计在实时控制系统中都失败了，因为它们提供了强大的一致性保证，并牺牲了太多的性能和灵活性。

The _eventually consistent_ shared data space of DDS has been very successful in helping with building systems that need to satisfy many "ilities": dependability, maintainability, extensibility, upgradeability, ...

> DDS 的事件一致性共享数据空间在帮助构建需要满足许多“操作”的系统方面非常成功：可靠性、可维护性、可扩展性、可升级性。。。

Truth be told, that's why it was invented, and publish-subscribe messaging was simply an implementation technique.

> 说实话，这就是它被发明的原因，发布订阅消息只是一种实现技术。

Cyclone DDS aims at full coverage of the specs and today already covers most of this.

> Cyclone DDS 旨在全面覆盖规格，目前已经覆盖了大部分规格。

With references to the individual OMG specifications, the following is available:

> 参考各个 OMG 规范，可获得以下内容：

- [DCPS](https://www.omg.org/spec/DDS/1.4/PDF) the base specification
  - zero configuration discovery (if multicast works)
  - publish/subscribe messaging
  - configurable storage of data in subscribers
  - many QoS settings - liveliness monitoring, deadlines, historical data, ...
  - coverage includes the Minimum, Ownership and (partially) Content profiles
- [DDS Security](https://www.omg.org/spec/DDS-SECURITY/1.1/PDF) - providing authentication, access control and encryption
- [DDS C++ API](https://www.omg.org/spec/DDS-PSM-Cxx/1.0/PDF)
- [DDS XTypes](https://www.omg.org/spec/DDS-XTypes/1.3/PDF) - the structural type system (some [caveats](docs/dev/xtypes_relnotes.md) here)
- [DDSI-RTPS](https://www.omg.org/spec/DDSI-RTPS/2.5/PDF) - the interoperable network protocol

The network stack in Cyclone DDS has been around for over a decade in one form or another and has proven itself in many systems, including large, high-availability ones and systems where interoperation with other implementations was needed.

> Cyclone DDS 中的网络堆栈已经以这样或那样的形式存在了十多年，并在许多系统中证明了自己的实力，包括大型、高可用性系统和需要与其他实现进行互操作的系统。

> [!NOTE] 如何理解 cyclone dds 中 "DDS XTypes"@17:49:11
> 在 Cyclone DDS 中，DDS XTypes 是一种类型体系结构，用于描述 DDS 数据类型。它不仅包含标准的 DDS 数据类型，如字符串、整数和布尔值，还可以扩展以支持用户自定义数据类型。DDS XTypes 还可以实现可插拔数据序列化以提高效率。
>
> DDS XTypes 可以使用 IDL（接口描述语言）定义，以精确和独立于平台的方式描述数据类型及其结构。它允许在 DDS 域内定义数据类型，并确保不同的 DDS 实现可以相互操作和交换数据。DDS XTypes 类型定义可以存储在 XML 文件中，并使用 DDSI（DDS XTypes 成像）将其转换为运行时结构，以支持动态类型创建。
>
> DDS XTypes 还支持类型版本管理，允许在不破坏数据互操作性的情况下更新数据类型。例如，当发布者更新数据类型时，它们可以在数据中包括版本号，以便订阅者知道如何解析该数据。这样，即使发布者和订阅者使用的是不同版本的数据类型，它们仍然可以相互通信。

This repository provides the core of Cyclone DDS including its C API, the [OMG C++](https://github.com/eclipse-cyclonedds/cyclonedds-cxx) and the [Python](https://github.com/eclipse-cyclonedds/cyclonedds-python) language bindings are in sibling repositories.

> 该存储库提供了 Cyclone DDS 的核心，包括其 C API，[OMG C++](https://github.com/eclipse-cyclonedds/cyclonedds-cxx)和 Python(https://github.com/eclipse-cyclonedds/cyclonedds-python)语言绑定在同级存储库中。

Consult the [roadmap](ROADMAP.md) for a high-level overview of upcoming features.

> 有关即将推出的功能的高级概述，请参阅[路线图](roadmap.md)。

# Getting Started

## Building Eclipse Cyclone DDS

In order to build Cyclone DDS you need a Linux, Mac or Windows 10 machine (or, with some caveats, a \*BSD, QNX, OpenIndiana or a Solaris 2.6 one) with the following installed on your host:

> 为了构建 Cyclone DDS，您需要一台 Linux、Mac 或 Windows 10 机器(或者，需要注意的是，一台\*BSD、QNX、OpenIndiana 或 Solaris 2.6 机器)，主机上安装了以下内容：

- C compiler (most commonly GCC on Linux, Visual Studio on Windows, Xcode on macOS);
- Optionally GIT version control system;
- [CMake](https://cmake.org/download/), version 3.16 or later;
- Optionally [OpenSSL](https://www.openssl.org/), preferably version 1.1;
- Optionally [Eclipse Iceoryx](https://iceoryx.io) version 2.0 for shared memory and zero-copy support;
- Optionally [Bison](https://www.gnu.org/software/bison/) parser generator. A cached source is checked into the repository.

If you want to play around with the parser you will need to install the bison parser generator. On Ubuntu `apt install bison` should do the trick for getting it installed.

> 如果您想使用解析器，则需要安装 bison 解析器生成器。在 Ubuntu 上，“apt-install bison”应该完成安装的技巧。

On Windows, installing chocolatey and `choco install winflexbison3` should get you a long way. On macOS, `brew install bison` is easiest.

> 在 Windows 上，安装 chocolatey 和“choco install winflexbison3”应该会让你有很长的路要走。在 macOS 上，“brew install bison”是最简单的。

To obtain Eclipse Cyclone DDS, do

> 要获得 Eclipse Cyclone DDS，请执行

```bash
$ git clone https://github.com/eclipse-cyclonedds/cyclonedds.git
$ cd cyclonedds
$ mkdir build
```

Depending on whether you want to develop applications using Cyclone DDS or contribute to it you can follow different procedures:

> 根据您是想使用 Cyclone DDS 开发应用程序还是为其做出贡献，您可以遵循不同的过程：

### Build configuration

There are some configuration options specified using CMake defines in addition to the standard options like `CMAKE_BUILD_TYPE`:

> 除了“CMake_BUILD_TYPE”等标准选项外，还使用 CMake 定义指定了一些配置选项：

- `-DBUILD_EXAMPLES=ON`: to build the included examples
- `-DBUILD_TESTING=ON`: to build the test suite (this requires [CUnit](http://cunit.sourceforge.net/), see [Contributing to Eclipse Cyclone DDS](#contributing-to-eclipse-cyclone-dds) below for more information)
- `-DBUILD_IDLC=NO`: to disable building the IDL compiler (affects building examples, tests and `ddsperf`)
- `-DBUILD_DDSPERF=NO`: to disable building the [`ddsperf`](https://github.com/eclipse-cyclonedds/cyclonedds/tree/master/src/tools/ddsperf) tool for performance measurement
- `-DENABLE_SSL=NO`: to not look for OpenSSL, remove TLS/TCP support and avoid building the plugins that implement authentication and encryption (default is `AUTO` to enable them if OpenSSL is found)
- `-DENABLE_SHM=NO`: to not look for Iceoryx and disabled shared memory support (default is `AUTO` to enable it if Iceoryx is found)
- `-DENABLE_SECURITY=NO`: to not build the security interfaces and hooks in the core code, nor the plugins (one can enable security without OpenSSL present, you'll just have to find plugins elsewhere in that case)
- `-DENABLE_LIFESPAN=NO`: to exclude support for finite lifespans QoS
- `-DENABLE_DEADLINE_MISSED=NO`: to exclude support for finite deadline QoS settings
- `-DENABLE_TYPE_DISCOVERY=NO`: to exclude support for type discovery and checking type compatibility (effectively most of XTypes), requires also disabling topic discovery using `-DENABLE_TOPIC_DISCOVERY=NO`
- `-DENABLE_TOPIC_DISCOVERY=NO`: to exclude support for topic discovery
- `-DENABLE_SOURCE_SPECIFIC_MULTICAST=NO`: to disable support for source-specific multicast (disabling this and `-DENABLE_IPV6=NO` may be needed for QNX builds)
- `-DENABLE_IPV6=NO`: to disable ipv6 support (disabling this and `-DENABLE_SOURCE_SPECIFIC_MULTICAST=NO` may be needed for QNX builds)
- `-DBUILD_IDLC_XTESTS=NO`: Include a set of tests for the IDL compiler that use the C back-end to compile an idl file at (test) runtime, and use the C compiler to build a test application for the generated types, that is executed to do the actual testing (not supported on Windows)

### For application developers

To build and install the required libraries needed to develop your own applications using Cyclone DDS requires a few simple steps. There are some small differences between Linux and macOS on the one hand, and Windows on the other.

For Linux or macOS:

```bash
$ cd build
$ cmake -DCMAKE_INSTALL_PREFIX=<install-location> ..
$ cmake --build .
```

and for Windows:

```bash
$ cd build
$ cmake -G "<generator-name>" -DCMAKE_INSTALL_PREFIX=<install-location> ..
$ cmake --build .
```

where you should replace `<install-location>` by the directory under which you would like to install Cyclone DDS and `<generator-name>` by one of the ways CMake [generators](https://cmake.org/cmake/help/latest/manual/cmake-generators.7.html) offer for generating build files.

> 其中应将“＜ install location ＞”替换为要安装 Cyclone DDS 的目录，并将“＜ generator name ＞”替换为由 CMake[generators]方法之一(https://cmake.org/cmake/help/latest/manual/cmake-generators.7.html)提供生成生成文件的功能。

For example, "Visual Studio 15 2017 Win64" would target a 64-bit build using Visual Studio 2017. To install it after a successful build, do:

```
$ cmake --build . --target install
```

which will copy everything to:

- `<install-location>/lib`
- `<install-location>/bin`
- `<install-location>/include/ddsc`
- `<install-location>/share/CycloneDDS`

Depending on the installation location you may need administrator privileges. At this point you are ready to use Eclipse Cyclone DDS in your own projects.

Note that the default build type is a release build with debug information included (RelWithDebInfo), which is generally the most convenient type of build to use from applications because of a good mix between performance and still being able to debug things. If you'd rather have a Debug or pure Release build, set `CMAKE_BUILD_TYPE` accordingly.

> 请注意，默认构建类型是包含调试信息 (RelWithDebInfo) 的发布构建，这通常是应用程序使用的最方便的构建类型，因为它兼顾了性能和仍然能够进行调试。 如果您更喜欢 Debug 或纯 Release 构建，请相应地设置 `CMAKE_BUILD_TYPE`。

### Contributing to Eclipse Cyclone DDS

We very much welcome all contributions to the project, whether that is questions, examples, bug fixes, enhancements or improvements to the documentation, or anything else really.

When considering contributing code, it might be good to know that build configurations for Azure pipelines are present in the repository and that there is a test suite using CTest and CUnit that can be built locally if desired.

> 在考虑贡献代码时，最好知道 Azure 管道的构建配置存在于存储库中，并且有一个使用 CTest 和 CUnit 的测试套件，如果需要，可以在本地构建。

To build it, set the cmake variable `BUILD_TESTING` to on when configuring, e.g.:

> 要构建它，请在配置时将 cmake 变量“build_TESTING”设置为开，例如：

```bash
$ cd build
$ cmake -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON ..
$ cmake --build .
$ ctest
```

Such a build requires the presence of [CUnit](http://cunit.sourceforge.net/).

> 这样的构建需要[CUnit]的存在(http://cunit.sourceforge.net/).

You can install this yourself, or you can choose to instead rely on the [Conan](https://conan.io) packaging system that the CI build infrastructure also uses.

> 你可以自己安装，也可以选择依赖[柯南](https://conan.io)CI 构建基础设施也使用的打包系统。

In that case, install Conan and do:

> 在这种情况下，安装柯南并执行以下操作：

```bash
$ conan install .. --build missing
```

in the build directory prior to running cmake.

For Windows, depending on the generator, you might also need to add switches to select the architecture and build type, e.g., `conan install -s arch=x86_64 -s build_type=Debug ..`

> 对于 Windows，根据生成器的不同，您可能还需要添加开关来选择体系结构和构建类型，例如`conan install-s arch=x86_64-s build_type=Debug`

This will automatically download and/or build CUnit (and, at the moment, OpenSSL).

> 这将自动下载和/或构建 CUnit(目前还有 OpenSSL)。

## Documentation

The [documentation](https://cyclonedds.io/docs) is still rather limited and some parts of it are still only available in the form of text files in the `docs` directory.

> 【文件】(https://cyclonedds.io/docs)仍然相当有限，并且它的某些部分仍然只能以“docs”目录中的文本文件的形式提供。

This README is usually out-of-date and the state of the documentation is slowly improving, so it definitely worth hopping over to have a look.

> 这个自述文件通常已经过时，文档的状态也在慢慢改善，所以绝对值得跳过去看看。

## Building and Running the Roundtrip Example

We will show you how to build and run an example program that measures latency.

> 我们将向您展示如何构建和运行一个测量延迟的示例程序。

The examples are built automatically when you build Cyclone DDS, so you don't need to follow these steps to be able to run the program, it is merely to illustrate the process.

> 这些示例是在构建 Cyclone DDS 时自动构建的，因此您不需要按照这些步骤来运行程序，这只是为了说明过程。

```bash
$ mkdir roundtrip
$ cd roundtrip
$ cmake <install-location>/share/CycloneDDS/examples/roundtrip
$ cmake --build .
```

On one terminal start the application that will be responding to pings:

> 在一个终端上启动将响应 ping 的应用程序：

```bash
$ ./RoundtripPong
```

On another terminal, start the application that will be sending the pings:

> 在另一个终端上，启动将发送 ping 的应用程序：

```bash
$ ./RoundtripPing 0 0 0
# payloadSize: 0 | numSamples: 0 | timeOut: 0
# Waiting for startup jitter to stabilise
# Warm up complete.
# Latency measurements (in us)
#             Latency [us]                                   Write-access time [us]       Read-access time [us]
# Seconds     Count   median      min      99%      max      Count   median      min      Count   median      min
    1     28065       17       16       23       87      28065        8        6      28065        1        0
    2     28115       17       16       23       46      28115        8        6      28115        1        0
    3     28381       17       16       22       46      28381        8        6      28381        1        0
    4     27928       17       16       24      127      27928        8        6      27928        1        0
    5     28427       17       16       20       47      28427        8        6      28427        1        0
    6     27685       17       16       26       51      27685        8        6      27685        1        0
    7     28391       17       16       23       47      28391        8        6      28391        1        0
    8     27938       17       16       24       63      27938        8        6      27938        1        0
    9     28242       17       16       24      132      28242        8        6      28242        1        0
    10     28075       17       16       23       46      28075        8        6      28075        1        0
```

The numbers above were measured on Mac running a 4.2 GHz Intel Core i7 on December 12th 2018.

> 以上数字是在 2018 年 12 月 12 日运行 4.2 GHz 英特尔酷睿 i7 的 Mac 电脑上测得的。

From these numbers you can see how the roundtrip is very stable and the minimal latency is now down to 17 micro-seconds (used to be 25 micro-seconds) on this HW.

> 从这些数字中，您可以看到往返是如何非常稳定的，并且该硬件上的最小延迟现在降至 17 微秒(过去是 25 微秒)。

# Performance

Reliable message throughput is over 1MS/s for very small samples and is roughly 90% of GbE with 100 byte samples, and latency is about 30us when measured using [ddsperf](src/tools/ddsperf) between two Intel(R) Xeon(R) CPU E3-1270 V2 @ 3.50GHz (that's 2012 hardware ...) running Ubuntu 16.04, with the executables built on Ubuntu 18.04 using gcc 7.4.0 for a default (i.e., "RelWithDebInfo") build.

<img src="https://raw.githubusercontent.com/eclipse-cyclonedds/cyclonedds/assets/performance/20190730/throughput-async-listener-rate.png" alt="Throughput" height="400"><img src="https://raw.githubusercontent.com/eclipse-cyclonedds/cyclonedds/assets/performance/20190730/latency-sync-listener.png" alt="Throughput" height="400">

This is with the subscriber in listener mode, using asynchronous delivery for the throughput test. The configuration is a marginally tweaked out-of-the-box configuration: an increased maximum message size and fragment size, and an increased high-water mark for the reliability window on the writer side.

> 这是**在订阅者处于侦听器模式的情况下，使用异步传递来提高吞吐量**
> 测验该配置是一种稍微调整过的开箱即用配置：增加了最大消息大小和片段大小，并增加了写入端可靠性窗口的高水位线。

For details, see the [scripts](examples/perfscript) directory, the [environment details](https://raw.githubusercontent.com/eclipse-cyclonedds/cyclonedds/assets/performance/20190730/config.txt) and the [throughput](https://raw.githubusercontent.com/eclipse-cyclonedds/cyclonedds/assets/performance/20190730/sub.log) and [latency](https://raw.githubusercontent.com/eclipse-cyclonedds/cyclonedds/assets/performance/20190730/ping.log) data underlying the graphs. These also include CPU usage ([throughput](https://raw.githubusercontent.com/eclipse-cyclonedds/cyclonedds/assets/performance/20190730/throughput-async-listener-cpu.png) and [latency](https://raw.githubusercontent.com/eclipse-cyclonedds/cyclonedds/assets/performance/20190730/latency-sync-listener-bwcpu.png)) and [memory usage](https://raw.githubusercontent.com/eclipse-cyclonedds/cyclonedds/assets/performance/20190730/throughput-async-listener-memory.png).

> 有关详细信息，请参阅[scripts](examples/perfscript)目录、[environment-details](https://raw.githubusercontent.com/eclipse-cyclonedds/cyclonedds/assets/performance/20190730/config.txt)以及[吞吐量](https://raw.githubusercontent.com/eclipse-cyclonedds/cyclonedds/assets/performance/20190730/sub.log)和[延迟](https://raw.githubusercontent.com/eclipse-cyclonedds/cyclonedds/assets/performance/20190730/ping.log)图表背后的数据。其中还包括 CPU 使用率([吞吐量](https://raw.githubusercontent.com/eclipse-cyclonedds/cyclonedds/assets/performance/20190730/throughput-async-listener-cpu.png)和[延迟](https://raw.githubusercontent.com/eclipse-cyclonedds/cyclonedds/assets/performance/20190730/latency-sync-listener-bwcpu.png))和[内存使用情况](https://raw.githubusercontent.com/eclipse-cyclonedds/cyclonedds/assets/performance/20190730/throughput-async-listener-memory.png).

# Run-time configuration

The out-of-the-box configuration should usually be fine, but there are a great many options that can be tweaked by creating an XML file with the desired settings and defining the `CYCLONEDDS_URI` to point to it.

> **开箱即用的配置通常应该很好**，但有很多选项可以通过创建具有所需设置的 XML 文件并定义`CYCLONEDDS_URI`来进行调整。

E.g. (on Linux):

```xml
$ cat cyclonedds.xml
<?xml version="1.0" encoding="UTF-8" ?>
<CycloneDDS xmlns="https://cdds.io/config" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:schemaLocation="https://cdds.io/config https://raw.githubusercontent.com/eclipse-cyclonedds/cyclonedds/master/etc/cyclonedds.xsd">
    <Domain Id="any">
        <General>
            <Interfaces>
                <NetworkInterface autodetermine="true" priority="default" multicast="default" />
            </Interfaces>
            <AllowMulticast>default</AllowMulticast>
            <MaxMessageSize>65500B</MaxMessageSize>
        </General>
        <Discovery>
            <EnableTopicDiscoveryEndpoints>true</EnableTopicDiscoveryEndpoints>
        </Discovery>
        <Internal>
            <Watermarks>
                <WhcHigh>500kB</WhcHigh>
            </Watermarks>
        </Internal>
        <Tracing>
            <Verbosity>config</Verbosity>
            <OutputFile>cdds.log.${CYCLONEDDS_PID}</OutputFile>
        </Tracing>
    </Domain>
</CycloneDDS>
$ export CYCLONEDDS_URI=file://$PWD/cyclonedds.xml
```

(on Windows, one would have to use `set CYCLONEDDS_URI=file://...` instead.)

This example shows a few things:

- `Interfaces` can be used to override the interfaces selected by default.
  Members are
  - `NetworkInterface[@autodetermine]` tells Cyclone DDS to autoselect the interface it deems best.
  - `NetworkInterface[@name]` specifies the name of an interface to select (not shown above, alternative for autodetermine).
  - `NetworkInterface[@address]` specifies the ipv4/ipv6 address of an interface to select (not shown above, alternative for autodetermine).
  - `NetworkInterface[@multicast]` specifies whether multicast should be used on this interface.
    The default value 'default' means Cyclone DDS will check the OS reported flags of the interface and enable multicast if it is supported.
    Use 'true' to ignore what the OS reports and enable it anyway and 'false' to always disable multicast on this interface.
  - `NetworkInterface[@priority]` specifies the priority of an interface.
    The default value (`default`) means priority `0` for normal interfaces and `2` for loopback interfaces.
- `AllowMulticast` configures the circumstances under which multicast will be used.
  If the selected interface doesn't support it, it obviously won't be used (`false`); but if it does support it, the type of the network adapter determines the default value.
  For a wired network, it will use multicast for initial discovery as well as for data when there are multiple peers that the data needs to go to (`true`).
  On a WiFi network it will use it only for initial discovery (`spdp`), because multicast on WiFi is very unreliable.
- `EnableTopicDiscoveryEndpoints` turns on topic discovery (assuming it is enabled at compile time), it is disabled by default because it isn't used in many system and comes with a significant amount of overhead in discovery traffic.
- `Verbosity` allows control over the tracing, "config" dumps the configuration to the trace output (which defaults to "cyclonedds.log", but here the process id is appended).
  Which interface is used, what multicast settings are used, etc., is all in the trace.
  Setting the verbosity to "finest" gives way more output on the inner workings, and there are various other levels as well.
- `MaxMessageSize` controls the maximum size of the RTPS messages (basically the size of the UDP payload).
  Large values such as these typically improve performance over the (current) default values on a loopback interface.
- `WhcHigh` determines when the sender will wait for acknowledgements from the readers because it has buffered too much unacknowledged data.
  There is some auto-tuning, the (current) default value is a bit small to get really high throughput.

- `Interfaces` 可用于覆盖默认选择的接口。
  成员是
  - `NetworkInterface[@autodetermine]` 告诉 Cyclone DDS 自动选择它认为最好的接口。
  - `NetworkInterface[@name]` 指定要选择的接口的名称（上面未显示，自动确定的替代方法）。
  - `NetworkInterface[@address]` 指定要选择的接口的 ipv4/ipv6 地址（上面未显示，自动确定的替代方法）。
  - `NetworkInterface[@multicast]` 指定是否应该在此接口上使用多播。
    默认值“default”意味着 Cyclone DDS 将检查操作系统报告的接口标志并在支持时启用多播。
    使用“true”忽略操作系统报告的内容并启用它，使用“false”始终禁用此接口上的多播。
  - `NetworkInterface[@priority]` 指定接口的优先级。
    默认值 (`default`) 表示普通接口的优先级为 0，环回接口的优先级为 2。
- `AllowMulticast` 配置使用多播的环境。
  如果选择的接口不支持，显然不会被使用（`false`）； 但如果它确实支持它，则网络适配器的类型决定了默认值。
  对于有线网络，它将使用多播进行初始发现以及当数据需要转到多个对等点时用于数据（`true`）。
  在 WiFi 网络上，它将仅用于初始发现 (`spdp`)，因为 WiFi 上的多播非常不可靠。
- `EnableTopicDiscoveryEndpoints` 打开主题发现（假设它在编译时启用），默认情况下它是禁用的，因为它没有在许多系统中使用并且在发现流量中有大量的开销。
- `Verbosity` 允许控制跟踪，“config”将配置转储到跟踪输出（默认为“cyclonedds.log”，但此处附加了进程 ID）。
  使用了哪个接口，使用了什么组播设置等等，都在 trace 中。
  将详细程度设置为“最好”可以让更多的内部工作输出，并且还有各种其他级别。
- `MaxMessageSize` 控制 RTPS 消息的最大大小（基本上是 UDP 负载的大小）。
  诸如此类的大值通常会提高环回接口上（当前）默认值的性能。
- `WhcHigh` 确定发送方何时等待来自读者的确认，因为它缓冲了太多未确认的数据。
  有一些自动调整，（当前）默认值有点小以获得真正的高吞吐量。

Background information on configuring Cyclone DDS can be found [here](docs/manual/config.rst) and a list of settings is [available](docs/manual/options.md).

# Trademarks

- "Eclipse Cyclone DDS", "Cyclone DDS", "Eclipse Iceoryx" and "Iceoryx" are trademarks of the Eclipse Foundation.
- "DDS" is a trademark of the Object Management Group, Inc.
- "ROS" is a trademark of Open Source Robotics Foundation, Inc.
