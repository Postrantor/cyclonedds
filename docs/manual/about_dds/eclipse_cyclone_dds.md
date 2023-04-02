# performance data_centric_architecture

is a performant and robust OMG-compliant **Data Distribution Service** (DDS) implementation (see ). is developed completely in the open as an Eclipse IoT project (see ) with a growing list of (if you\'re one of them, please add your . It is a tier-1 middleware for the Robot Operating System .

> 是一种高性能且稳健的符合 OMG 的**数据分发服务**（DDS）实现（请参阅）。是作为一个 Eclipse 物联网项目完全公开开发的（请参阅），有越来越多的（如果你是其中之一，请添加你的）。它是机器人操作系统的一级中间件。

The core of is implemented in C and provides C-APIs to applications. Through its C++ package, the 2003 language binding is also supported.

> 的**核心是用 C 语言实现**的，并为应用程序提供 C-API。**通过其 C++ 包，还支持 2003 语言绑定**。

## About DDS

DDS is the best-kept secret in distributed systems, one that has been around for much longer than most publish-subscribe messaging systems and still outclasses so many of them. DDS is used in a wide variety of systems, including air-traffic control, jet engine testing, railway control, medical systems, naval command-and-control, smart greenhouses and much more. In short, it is well-established in aerospace and defense but no longer limited to that. And yet it is easy to use!

> DDS 是分布式系统中保守得最好的秘密，它的存在时间比大多数发布-订阅消息传递系统长得多，但仍然超过了其中的许多系统。DDS 用于各种系统，包括空中交通控制、喷气发动机测试、铁路控制、医疗系统、海军指挥和控制、智能温室等。简言之，它在航空航天和国防领域已经确立，但不再局限于此。然而它很容易使用！

Types are usually defined in IDL and preprocessed with the IDL compiler included in Cyclone, but our allows you to define data types on the fly:

> **类型通常在 IDL 中定义**，并使用 Cyclone 中包含的 IDL 编译器进行预处理，但我们允许您动态定义数据类型：

```python
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

Today DDS is also popular in robotics and autonomous vehicles because those really depend on high-throuhgput, low-latency control systems without introducing a single point of failure by having a message broker in the middle. Indeed, it is by far the most used and the default middleware choice in ROS 2. It is used to transfer commands, sensor data and even video and point clouds between components.

> 如今，DDS 在机器人和自动车辆中也很受欢迎，因为它们真正依赖于**高通量、低延迟的控制系统**，而**不会通过在中间设置消息代理引入单点故障**。事实上，它是 ROS2 中使用最多的默认中间件选择。它用于在组件之间传输命令、传感器数据，甚至视频和点云。

The OMG DDS specifications cover everything one needs to build systems using publish-subscribe messaging. They define a structural type system that allows automatic endianness conversion and type checking between readers and writers. This type system also supports type evolution. The interoperable networking protocol and standard C++ API make it easy to build systems that integrate multiple DDS implementations. Zero-configuration discovery is also included in the standard and supported by all implementations.

> OMG DDS 规范涵盖了使用发布-订阅消息构建系统所需的一切。它们定义了一个结构类型系统，允许在读写器之间进行自动的端序转换和类型检查。这种类型系统也支持类型演化。**可互操作的网络协议和标准 C++API 使构建集成多个 DDS 实现的系统变得容易。零配置发现也包含在标准中，并受到所有实现的支持**。

DDS actually brings more: publish-subscribe messaging is a nice abstraction over "ordinary" networking, but plain publish-subscribe doesn\'t affect how one _thinks_ about systems. A very powerful architecture that truly changes the perspective on distributed systems is that of the "shared data space", in itself an old idea, and really just a distributed database. Most shared data space designs have failed miserably in real-time control systems because they provided strong consistency guarantees and sacrificed too much performance and flexibility. The _eventually consistent_ shared data space of DDS has been very successful in helping with building systems that need to satisfy many "ilities": dependability, maintainability, extensibility, upgradeability, \... Truth be told, that\'s why it was invented, and publish-subscribe messaging was simply an implementation technique.

> DDS 实际上带来了更多：发布-订阅消息是对“普通”网络的一个很好的抽象，但简单的发布-订阅不会影响人们对系统的思考。真正改变分布式系统观点的一个非常强大的体系结构是“共享数据空间”，这本身就是一个古老的想法，实际上只是一个分布式数据库。大多数共享数据空间设计在实时控制系统中都失败了，因为它们**提供了强大的一致性保证**，并牺牲了太多的性能和灵活性。DDS 的事件一致性共享数据空间在帮助构建需要满足许多“操作”的系统方面非常成功：可靠性、可维护性、可扩展性、可升级性。。。说实话，这就是它被发明的原因，发布订阅消息只是一种实现技术。

Cyclone DDS aims at full coverage of the specs and today already covers most of this. With references to the individual OMG specifications, the following is available:

> Cyclone DDS 旨在全面覆盖规格，目前已经覆盖了大部分规格。参考各个 OMG 规范，可获得以下内容：

- the base specification
- zero configuration discovery (if multicast works)
- publish/subscribe messaging
- configurable storage of data in subscribers
- many QoS settings - liveliness monitoring, deadlines, historical data, \...
- coverage includes the Minimum, Ownership and (partially) Content profiles

> - 基本规范
> - 零配置发现（如果多播有效）
> - 发布/订阅消息
> - 用户中数据的可配置存储
> - 许多 QoS 设置-活动监控、截止日期、历史数据，\。。。
> - 覆盖范围包括最低、所有权和（部分）内容配置文件

- \- providing authentication, access control and encryption
- \- the structural type system (some \[caveats\](docs/dev/xtypes_relnotes.md) here)
- \- the interoperable network protocol

> -\-提供身份验证、访问控制和加密
> -\-结构类型系统（此处为一些\[注意事项\]（docs/dev/xtypes_relnotes.md））
> -\-可互操作的网络协议

The network stack in Cyclone DDS has been around for over a decade in one form or another and has proven itself in many systems, including large, high-availability ones and systems where interoperation with other implementations was needed.

> Cyclone DDS 中的网络堆栈已经以这样或那样的形式存在了十多年，并在许多系统中证明了自己的实力，包括大型、高可用性系统和需要与其他实现进行互操作的系统。
