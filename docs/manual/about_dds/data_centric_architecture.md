# Data-centric architecture

In a service-centric architecture, applications need to know each other\'s interfaces to share data, share events, and share commands or replies to interact. These interfaces are modeled as sets of operations and functions that are managed in centralized repositories. This type of architecture creates unnecessary dependencies that form a tightly coupled system. The centralized interface repositories are usually seen as a single point of failure.

> 在**以服务为中心**的体系结构中，**应用程序需要了解彼此的接口**，以共享数据、共享事件以及共享命令或回复进行交互。这些接口被建模为在集中式存储库中管理的操作和功能集。这种类型的体系结构**会产生不必要的依赖关系**，从而形成一个紧密耦合的系统。**集中式接口存储库通常被视为单个故障点**。

In a data-centric architecture, your design focuses on the data each application produces and decides to share rather than on the Interfaces\' operations and the internal processing that produced them.

> 在以**数据为中心**的体系结构中，您的设计侧重于每个**应用程序产生并决定共享的数据**，而不是接口的操作和产生这些操作的内部处理。

A data-centric architecture creates a decoupled system that focuses on the data and applications states\' that need to be shared rather than the applications\' details. In a data-centric system, data and their associated quality of services are the only contracts that bound the applications together. With , the system decoupling is bi-dimensional, in both space and time.

> **以数据为中心的体系结构创建了一个解耦的系统**，它关注需要共享的数据和应用程序状态，而不是应用程序的细节。在以数据为中心的系统中，数据及其相关的服务质量是将应用程序绑定在一起的唯一合同。通过，**系统解耦是二维的，在空间和时间上都是如此**。

Space-decoupling derives from the fact that applications do not need to know the identity of the data produced or consumed, nor their logical or a physical location in the network. Under the hood, runs a zero-configuration, interoperable discovery protocol that searches matching data readers and data writers that are interested in the same data topic.

> 空间解耦源于这样一个事实，即应用程序不需要知道产生或消耗的数据的身份，也不需要知道它们在网络中的逻辑或物理位置。在引擎盖下，运行一个**零配置、可互操作的发现协议**，搜索对同一数据主题感兴趣的匹配数据读取器和数据写入器。

Time-decoupling derives from the fact that, fundamentally, the nature of communication is asynchronous. Data producers and consumers, known as `DataWriter`s and `DataReader`s, are not forced to be active and connected simultaneously to share data. In this scenario, the middleware can handle and manage data on behalf of late joining `DataReader` applications and deliver it to them when they join the system.

> **时间去耦**源于这样一个事实，即从根本上说，**通信的本质是异步的。数据生产者和消费者**，即“DataWriter”和“DataReader”，不必被迫处于活动状态并同时连接以共享数据。在这种情况下，中间件可以代表后期加入的“DataReader”应用程序处理和管理数据，并在它们加入系统时将数据交付给它们。

Time and space decoupling gives applications the freedom to be plugged or unplugged from the system at any time, from anywhere, in any order. This keeps the complexity and administration of a data-centric architecture relatively low when adding more and more `DataReader` and `DataWriter` applications.

> **时间和空间解耦**使应用**程序可以在任何时间、任何地点、以任何顺序从系统中插入或拔出**。当添加越来**越多的**“DataReader”和“DataWriter”应用程序时，这使以数据为中心的体系结构的复杂性和管理**相对较低**。
