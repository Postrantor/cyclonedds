# HelloWorld keys steps {#key_steps}

The **Hello World!** example has a minimal 'data layer' with a data model made of one data type `Msg` that represents keyed messages.

> **你好世界！**示例有一个最小的“数据层”，其数据模型由一种表示键控消息的数据类型“Msg”组成。

To exchange data with , applications' business logic needs to:

> 要与交换数据，应用程序的业务逻辑需要：

1.  Declare its participation and involvement in a _DDS domain_. A DDS domain is an administrative boundary that defines, scopes, and gathers all the DDS applications, data, and infrastructure that must interconnect by sharing the same data space. Each DDS domain has a unique identifier. Applications declare their participation within a DDS domain by creating a **Domain Participant entity**.

> 1.宣布其参与\_DDS 领域。**DDS 域是一个管理边界，用于定义、界定和收集必须通过共享同一数据空间进行互连的所有 DDS 应用程序、数据和基础设施**。每个 DDS 域都有一个唯一的标识符。应用程序通过创建**域参与者实体**来声明其在 DDS 域中的参与。

2.  Create a **Data topic** with the data type described in a data model. The data types define the structure of the Topic. The Topic is therefore, an association between the topic's name and a datatype. QoSs can be optionally added to this association. The concept Topic therefore discriminates and categorizes the data in logical classes and streams.

> 2.使用数据模型中描述的数据类型创建一个**数据主题**。数据类型定义了主题的结构。因此，Topic 是主题名称和数据类型之间的关联。可以**选择性地将 QoS 添加到该关联中**。因此，Topic 概念将数据区分和分类为逻辑类和流。

3.  Create the **Data Readers** and **Writers** entities specific to the topic. Applications may want to change the default QoSs. In the `Hello world!` Example, the `ReliabilityQoS` is changed from its default value (`Best-effort`) to `Reliable`.

> 3.创建特定于主题的**数据读取器**和**写入器**实体。应用程序可能想要更改默认 QoS。在“你好世界！”例如，“可靠性 QoS”从其默认值（“尽力而为”）更改为“可靠”。

4.  When the previous DDS computational entities are in place, the application logic can start writing or reading the data.

> 4.当先前的 DDS 计算实体就位时，应用程序逻辑可以开始写入或读取数据。

At the application level, readers and writers do not need to be aware of each other. The reading application, now called Subscriber, polls the data reader periodically until a publishing application, now called The publisher writes the required data into the shared topic, namely `HelloWorldData_Msg`.

> 在应用程序级别，读者和作者不需要相互了解。现在称为 Subscriber 的读取应用程序定期轮询数据读取器，直到发布应用程序（现在称为发布者）将所需数据写入共享主题，即“HelloWorldData_Sg”。

The data type is described using the language located in the `HelloWorldData.idl` file and is the data model of the example.

> 数据类型是使用位于“HelloWorldData.idl”文件中的语言描述的，是示例的数据模型。

This data model is preprocessed and compiled by the IDL Compiler to generate a C representation of the data as described in Chapter 2. These generated source and header files are used by the `HelloworldSubscriber.c` and `HelloworldPublishe.c` programs to share the **Hello World!** Message instance and sample.

> IDL 编译器对该数据模型进行预处理和编译，以生成数据的 C 表示，如第 2 章所述。这些生成的源文件和头文件由`HelloorldSubscriber.c`和`HelloorldPublishe.c`程序用于共享**Hello World！**消息实例和示例。

C++ This data model is preprocessed and compiled by C++ IDL-Compiler to generate a C++ representation of the data as described in Chapter 6. These generated source and header files are used by the `HelloworldSubscriber.cpp` and `HelloworldPublisher.cpp` application programs to share the _Hello World!_ Message instance and sample.

> C++这个数据模型由 C++IDL 编译器进行预处理和编译，以生成数据的 C++表示，如第 6 章所述。这些生成的源文件和头文件由`HelloorldSubscriber.cpp`和`HelloorldPublisher.cpp`应用程序用于共享*Hello World！*消息实例和示例。
