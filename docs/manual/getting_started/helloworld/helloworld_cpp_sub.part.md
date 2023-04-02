---
title: HelloWorld subscriber C++ source code
---

The `subscriber.cpp` file mainly contains the statements to wait for a HelloWorld message and reads it when it receives it.

> [!NOTE]
> The `read` semantics keep the data sample in the data reader cache.

The subscriber application implements the steps defined in `key_steps`{.interpreted-text role="ref"}.
The following is a copy of the **subscriber.cpp** file that is available from the repository.

To create a subscriber:

1.  To recieve data using the DDS ISOCPP API and the `HelloWorldData_Msg` type, include the appropriate header files:

    - The `dds.hpp` file give access to the DDS APIs,
    - The `HelloWorldData.hpp` is specific to the data type defined in the IDL.

    ```cpp {.C++ linenos="" lineno-start="18"}
    #include "dds/dds.hpp"
    ```

    ```cpp {.C++ linenos="" lineno-start="21"}
    #include "HelloWorldData.hpp"
    ```

At least **four DDS entities** are needed to build a minimalistic application:

- Domain participant
- Topic
- Subscriber
- Reader

2.  **The DDS participant is always attached to a specific DDS domain**. In the HelloWorld example, it is part of the `Default_Domain`, which is specified in the XML configuration file. To override the default behavior, create or edit a configuration file (for example, `$CYCLONEDDS_URI`). For further information, refer to the `config-docs`{.interpreted-text role="ref"} and the `configuration_reference`{.interpreted-text role="ref"}.

    > DDS 参与者始终附加到特定的 DDS 域。 在 HelloWorld 示例中，它是 XML 配置文件中指定的“Default_Domain”的一部分。 要覆盖默认行为，请创建或编辑配置文件(例如，`$CYCLONEDDS_URI`)。 如需更多信息，请参阅`config-docs`{.interpreted-text role="ref"} 和`configuration_reference`{.interpreted-text role="ref"}。

    ```cpp {.C++ linenos="" lineno-start="31"}
    dds::domain::DomainParticipant participant(domain::default_id());
    ```

3.  Create the topic with a given name (`ddsC++_helloworld_example`) and the predefined data type (`HelloWorldData::Msg`). Topics with the same data type description and with different names are considered other topics. This means that readers or writers created for a given topic do not interfere with readers or writers created with another topic even if they have the same data type.

    > 使用给定名称 (`ddsC++_helloworld_example`) 和预定义数据类型 (`HelloWorldData::Msg`) 创建主题。 具有相同数据类型描述和不同名称的主题被视为其他主题。 这意味着为给定主题创建的读者或作者不会干扰使用另一个主题创建的读者或作者，即使它们具有相同的数据类型。

    ```cpp {.C++ linenos="" lineno-start="34"}
    dds::topic::Topic<HelloWorldData::Msg> topic(participant, "HelloWorldData_Msg");
    ```

4.  Create the subscriber:

    ```cpp {.C++ linenos="" lineno-start="37"}
    dds::sub::Subscriber subscriber(participant);
    ```

5.  Create a data reader and attach to it:

    ```cpp {.C++ linenos="" lineno-start="40"}
    dds::sub::DataReader<HelloWorldData::Msg> reader(subscriber, topic);
    ```

6.  The C++ API simplifies and extends how data can be read or taken. To handle the data some, `LoanedSamples` is declared and created which loan samples from the Service pool. Return of the loan is implicit and managed by scoping:

    > CycloneDDS 的 C++ API 简化并扩展了数据的读取和提取方式。**为了处理数据，声明并创建了 `LoanedSamples` 对象，从服务池中借用样本数据。归还样本数据的操作是隐式进行的，并由作用域管理**。

    ```cpp {.C++ linenos="" lineno-start="53"}
    dds::sub::LoanedSamples<HelloWorldData::Msg> samples;
    dds::sub::LoanedSamples<HelloWorldData::Msg>::const_iterator sample_iter;
    ```

7.  The `read()/take()` operation can return more than one data sample (where several publishing applications are started simultaneously to write different message instances), an an iterator is used:

    > `read()/take()` 操作可以返回多个数据样本(其中同时启动多个发布应用程序以写入不同的消息实例)，使用迭代器：

    ```cpp {.C++ linenos="" lineno-start="66"}
    const HelloWorldData::Msg& msg = sample_iter->data();
    const dds::sub::SampleInfo& info = sample_iter->info();
    ```

8.  In , data and metadata are propagated together. The samples are a set of data samples (that is, user-defined data) and metadata describing the sample state and validity, etc ,,, (`info`). To get the data and metadata from each sample, use iterators:

    > 在 中，数据和元数据一起传播。 样本是一组数据样本(即用户定义的数据)和**描述样本状态和有效性**等的元数据 ,,, (`info`)。 **要从每个样本中获取数据和元数据，请使用迭代器**：

    > [!NOTE]: 请问，在 cyclone dds 中的 "元数据" 具体该怎么理解
    > 在 CycloneDDS 中，"元数据" 通常指的是数据的描述信息，它包含的是与数据本身相关的信息，如数据类型、数据发送者和接收者等。这些信息也被称为 "数据的上下文"。元数据可以让数据的接收者在接收到数据之前对数据作出预处理，比如确定数据如何解析和处理，或者过滤掉不需要的数据。在 CycloneDDS 中，元数据可以使用不同的方式传输，比如直接附加在数据本身的尾部，或者使用历史记录、时间戳等其他信息来生成。元数据对于实现高效的数据传输和处理非常重要，它可以帮助确保数据的质量和可靠性，并最大限度地提高系统的性能。

    ```cpp {.C++ linenos="" lineno-start="87"}
    } catch (const dds::core::Exception& e) {
        std::cerr << "=== [Subscriber] DDS exception: " << e.what() << std::endl;
        return EXIT_FAILURE;
    } catch (const std::exception& e) {
        std::cerr << "=== [Subscriber] C++ exception: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    ```

9.  To locate issues precisely when they occur, it is good practice to surround every key verb of the DDS APIs with `try/catch` block. For example, the following shows how one block is used to facilitate the programming model of the applications and improve source code readability:

    > 为了在问题发生时准确定位问题，最好用“try/catch”块包围 DDS API 的每个关键动词。 例如，下图显示了如何使用一个块来简化应用程序的编程模型并提高源代码的可读性：

    ```cpp {.C++ linenos="" lineno-start="26"}
    try {
    ```

    ```cpp {.C++ linenos="" lineno-start="87"}
    } catch (const dds::core::Exception& e) {
    ```

10. To retrieve data in your application code from the data reader's cache, you can either:

    > 要从数据读取器的缓存中检索应用程序代码中的数据，您可以：

    - Use a pre-allocated buffer to store the data. Create an array/vector-like like container.
    - Loan it from the middleware. These buffers are actually 'owned' by the middleware, precisely by the `DataReader`. The C++ API implicitly allows you to return the loans through scoping.

    - 使用预先分配的缓冲区来存储数据。 创建一个类似数组/向量的容器。
    - **从中间件借出**。 这些缓冲区实际上由中间件“拥有”，准确地说是“DataReader”。 **C++ API 隐式允许您通过范围界定归还贷款**。

    In the example, use the loan samples mode (`LoanedSamples`). `Samples` are an unbounded sequence of samples; the sequence length depends on the amount of data available in the data reader's cache:

    > 在示例中，使用贷款样本模式 (`LoanedSamples`)。 `Samples` 是**无限的样本序列； 序列长度取决于数据读取器缓存中可用的数据量**：

    ```cpp {.C++ linenos="" lineno-start="53"}
    dds::sub::LoanedSamples<HelloWorldData::Msg> samples;
    ```

11. Attempt to read data by going into a polling loop that regularly scrutinizes and examines the arrival of a message. Samples are removed from the reader's cache when taken with the `take()`:

    > 尝试**通过进入轮询循环来读取数据**，轮询循环定期检查和检查消息的到达。 当使用 `take()` 获取样本时，样本将从读取器的缓存中删除：

    ```cpp {.C++ linenos="" lineno-start="56"}
    samples = reader.take();
    ```

    If you choose to read the samples with `read()`, data remains in the data reader cache. A length() of samples greater than zero indicates that the data reader cache was not empty:

    > 如果您选择**使用 read() 读取样本，数据将保留在数据读取器缓存中**。 样本的 **length() 大于零表示数据读取器缓存不为空**：

    ```cpp {.C++ linenos="" lineno-start="59"}
    if (samples.length() > 0)
    ```

12. As sequences are **NOT** pre-allocated by the user, buffers are 'loaned' by the `DataReader`:

    ```cpp {.C++ linenos="" lineno-start="61"}
    dds::sub::LoanedSamples<HelloWorldData::Msg>::const_iterator sample_iter;
    for (sample_iter = samples.begin();
        sample_iter < samples.end();
        ++sample_iter)
    ```

13. For each sample, cast and extract its user-defined data (`Msg`) and metadate (`info`):

    ```cpp {.C++ linenos="" lineno-start="66"}
    const HelloWorldData::Msg& msg = sample_iter->data();
    const dds::sub::SampleInfo& info = sample_iter->info();
    ```

    The SampleInfo (`info`) shows whether the data we are taking is _Valid_ or _Invalid_. Valid data means that it contains the payload provided by the publishing application. Invalid data means that we are reading the DDS state of the data Instance. The state of a data instance can be `DISPOSED` by the writer, or it is `NOT_ALIVE` anymore, which could happen when the publisher application terminates while the subscriber is still active. In this case, the sample is not considered valid, and its sample `info.valid()` field is False.

    > SampleInfo (`info`) 显示我们获取的数据是 _Valid_ 还是 _Invalid_。 **有效数据意味着它包含发布应用程序提供的有效负载。 无效数据意味着我们正在读取数据实例的 DDS 状态**。 数据实例的状态可以由编写者“已处置”，或者不再是“不活动”，这可能发生在发布者应用程序终止而订阅者仍处于活动状态时。 在这种情况下，样本被认为是无效的，其**样本 info.valid() 字段为 False**。

    ```cpp {.C++ linenos="" lineno-start="74"}
    if (info.valid())
    ```

14. Display the sample containing valid data:

    ```cpp {.C++ linenos="" lineno-start="75"}
    std::cout << "=== [Subscriber] Message received:" << std::endl;
    std::cout << "    userID  : " << msg.userID() << std::endl;
    std::cout << "    Message : \"" << msg.message() << "\"" << std::endl;
    ```

    As we are using the Poll data reading mode, we repeat the above steps every 20 milliseconds.

    ```cpp {.C++ linenos="" lineno-start="83"}
    else {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    ```

    This example uses the polling mode to read or take data. offers _waitSet_ and _Listener_ mechanism to notify the application that data is available in their cache, which avoids polling the cache at a regular intervals. The discretion of these mechanisms is beyond the scope of this document.

    > **本例使用轮询方式读取或取数据**。 **提供 _waitSet_ 和 _Listener_ 机制来通知应用程序其缓存中有数据可用，从而避免定期轮询缓存**。 这些机制的自由裁量权超出了本文档的范围。

**All the entities that are created under the participant**, such as the data reader, subscriber, and topic **are automatically deleted by middleware** through the scoping mechanism.

> [!NOTE]
> To modify the data reader default reliability Qos to \`reliable\`:
>
> ```cpp {.C++ linenos="" lineno-start="60"}
> /* With a normal configuration (see dds::pub::qos::DataWriterQos
> ```
>
> ```C++
> dds::sub::qos::DataReaderQos drqos = topic.qos() << dds::core::policy::Reliability::Reliable();
> dds::sub::DataReader<HelloWorldData::Msg> dr(subscriber, topic, drqos);
> ```
