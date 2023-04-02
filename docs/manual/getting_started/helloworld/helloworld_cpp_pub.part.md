---
title: HelloWorld publisher C++ source code
---

The `Publisher.cpp` contains the source that writes a _Hello World_ message.

The DDS publisher application code is almost symmetric to the subscriber, except that you must create a publisher and `DataWriter`. To ensure data is written only when at least one matching reader is discovered, a synchronization statement is added to the main thread. Synchronizing the main thread until a reader is discovered ensures we can start the publisher or subscriber program in any order.

The following is a copy of the **publisher.cpp** file that is available from the repository.

To create a publisher:

1.  Send data using the ISOCPP DDS API and the `HelloWorldData_Msg` type, include the appropriate header files:

    ```cpp {.C++ linenos="" lineno-start="18"}
    #include "dds/dds.hpp"
    ```

    ```cpp {.C++ linenos="" lineno-start="21"}
    #include "HelloWorldData.hpp"
    ```

2.  An exception handling mechanism `try/catch` block is used.

    ```cpp {.C++ linenos="" lineno-start="26"}
    try {
    ```

3.  **Create a writer**. You must have **a participant, a topic, and a publisher** (must have the same topic name as specified in `subscriber.cpp`):

    ```cpp {.C++ linenos=""}
    dds::domain::DomainParticipant participant(domain::default_id());
    dds::topic::Topic<HelloWorldData::Msg> topic(participant, "HelloWorldData_Msg");
    dds::pub::Publisher publisher(participant);
    ```

4.  Create the writer for a specific topic `“ddsC++_helloworld_example”` in the default DDS domain.

    ```cpp {.C++ linenos="" lineno-start="40"}
    dds::pub::DataWriter<HelloWorldData::Msg> writer(publisher, topic);
    ```

5.  When readers and writers are sharing the same data type and topic name, it connects them without the application\'s involvement. To write data only when a DataReader appears, a rendezvous pattern is required. A rendezvous pattern can be implemented by either:

    > 当读者和作者共享相同的数据类型和主题名称时，它会在没有应用程序参与的情况下将它们连接起来。 要仅在 DataReader 出现时写入数据，需要集合点模式。 集合点模式可以通过以下任一方式实现：

    - Regularly polling the publication matching status (the preferred option in this example).
    - Waiting for the publication/subscription matched events, where the publisher waits and blocks the writing thread until the appropriate publication-matched event is raised.

    The following line of code instructs to listen on the `writer.publication_matched_status()`:

    ```cpp {.C++ linenos="" lineno-start="40"}
    dds::pub::DataWriter<HelloWorldData::Msg> writer(publisher, topic);
    ```

6.  At regular intervals, the status change and a matching publication is received. In between, the writing thread sleeps for for 20 milliseconds:

    ```cpp {.C++ linenos="" lineno-start="49"}
    while (writer.publication_matched_status().current_count() == 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    ```

    After this loop, a matching reader has been discovered.

7.  To write the data instance, create and initialize the data:

    ```cpp {.C++ linenos="" lineno-start="54"}
    HelloWorldData::Msg msg(1, "Hello World");
    ```

8.  Send the data instance of the keyed message.

    ```cpp {.C++ linenos="" lineno-start="58"}
    writer.write(msg);
    ```

9.  After writing the data to the writer, the _DDS C++ Hello World_ example checks if a matching subscriber(s) is still available. If matching subscribers exist, **the example waits for 50ms and starts publishing the data again**. If no matching subscriber is found, then the publisher program is ended:

    ```cpp {.C++ linenos="" lineno-start="78"}
    return EXIT_SUCCESS;
    ```

    Through scoping, all the entities such as topic, writer, etc. are deleted automatically.

> [!Note]
> To modify the `DataWriter` Default Reliability Qos to Reliable:
```cpp {.C++ linenos="" lineno-start="60"}
/* With a normal configuration (see dds::pub::qos::DataWriterQos
```

```C++
dds::pub::qos::DataReaderQos dwqos = topic.qos() << dds::core::policy::Reliability::Reliable();
dds::sub::DataWriter<HelloWorldData::Msg> dr(publisher, topic, dwqos);
```
