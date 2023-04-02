---
title: HelloWorld publisher C source code
---

The `Publisher.c` contains the source that writes a **Hello World!** Message.

The DDS publisher application code is almost symmetric to the subscriber, except that you must create a data writer instead of a data reader. To ensure data is written only when at least one matching reader is discovered, a synchronization statement is added to the main thread. Synchronizing the main thread until a reader is discovered ensures we can start the publisher or subscriber program in any order.

> DDS 发布者应用程序代码几乎与订阅者对称，**只是您必须创建数据写入器而不是数据读取器**。 为了**确保只有在至少发现一个匹配的读取器时才写入数据，在主线程中添加了一条同步语句**。 同步主线程直到发现读者确保我们可以以任何顺序启动发布者或订阅者程序。

The following is a copy of the **publisher.c** file that is available from the repository.

::: {.literalinclude linenos=""}
../../../../examples/helloworld/publisher.c
:::

To create a publisher:

1.  Send data using the DDS API and the `HelloWorldData_Msg` type, include the appropriate header files:

    ```c {.C linenos="" lineno-start="1"}
    #include "ddsc/dds.h"
    #include "HelloWorldData.h"
    ```

2.  Create a writer. You **must have a participant and a topic** (must have the same topic name as specified in `subscriber.c`):

    ```c {.C linenos="" lineno-start="8"}
    dds_entity_t participant;
    dds_entity_t topic;
    dds_entity_t writer;
    ```

3.  Create a Participant.

    ```c {.C linenos="" lineno-start="18"}
    participant = dds_create_participant (DDS_DOMAIN_DEFAULT, NULL, NULL);
    ```

4.  Create a Topic.

    ```c {.C linenos="" lineno-start="23"}
    topic = dds_create_topic (
    participant, &HelloWorldData_Msg_desc, "HelloWorldData_Msg", NULL, NULL);
    ```

5.  Create a Writer.

    ```c {.C linenos="" lineno-start="28"}
    writer = dds_create_writer (participant, topic, NULL, NULL);
    ```

6.  When readers and writers are sharing the same data type and topic name, it connects them without the application\'s involvement. To write data only when a DataReader appears, a rendezvous pattern is required. A rendezvous pattern can be implemented by either:

    > 当读者和作者共享相同的数据类型和主题名称时，**它会在没有应用程序参与的情况下将它们连接起来**。 要仅在 DataReader 出现时写入数据，需要集合点模式。 集合点模式可以通过以下任一方式实现：

    - Regularly polling the publication matching status (the preferred option in this example).
    - Waiting for the publication/subscription matched events, where the publisher waits and blocks the writing thread until the appropriate publication-matched event is raised.

    - **定期轮询**发布匹配状态（本例中的首选选项）。
    - 等待发布/订阅匹配事件，发布者等待并阻塞写入线程，直到引发适当的发布匹配事件。

    The following line of code instructs to listen on the `DDS_PUBLICATION_MATCHED_STATUS`:

    ```c {.C linenos="" lineno-start="36"}
    dds_set_status_mask(writer, DDS_PUBLICATION_MATCHED_STATUS);
    ```

7.  At regular intervals, the status change and a matching publication is received. In between, the writing thread sleeps for a time period equal `DDS_MSECS` (in milliseconds).

    > 每隔一定时间，就会收到状态更改和匹配的发布。 在两者之间，写入线程休眠一段等于“DDS_MSECS”（以毫秒为单位）的时间。

    ```c {.C linenos="" lineno-start="40"}
    while(!(status & DDS_PUBLICATION_MATCHED_STATUS))
    {
        rc = dds_get_status_changes (writer, &status);
        if (rc != DDS_RETCODE_OK)
            DDS_FATAL("dds_get_status_changes: %s\n", dds_strretcode(-rc));

        /* Polling sleep. */
        dds_sleepfor (DDS_MSECS (20));
    }
    ```

    After this loop, a matching reader has been discovered.

8.  To write the data instance, create and initialize the data:

    ```c {.C linenos="" lineno-start="12"}
    HelloWorldData_Msg msg;
    ```

    ```c {.C linenos="" lineno-start="51"}
    msg.userID = 1;
    msg.message = "Hello World";
    ```

9.  Send the data instance of the keyed message:

    ```c {.C linenos="" lineno-start="58"}
    rc = dds_write (writer, &msg);
    ```

10. When terminating the program, free the DDS allocated resources by deleting the root entity of all the others (the domain participant):

    > 终止程序时，通过删除所有其他（域参与者）的根实体来释放 DDS 分配的资源：

    ```c {.C linenos="" lineno-start="63"}
    rc = dds_delete (participant);
    ```

    All the underlying entities, such as topic, writer, and so on, are deleted.
