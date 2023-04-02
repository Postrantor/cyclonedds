The `Subscriber.c` file mainly contains the statements to wait for a HelloWorld message and reads it when it receives it.

> [!Important]
> The `read` semantics keep the data sample in the data reader cache. To prevent resource exhaustion, It is important to use `take`, where appropriate.
> **`read` 语义将数据样本保存在数据读取器缓存中**。 为了**防止资源耗尽**，在适当的地方使用 `take` 很重要。

The subscriber application implements the steps defined in `key_steps`{.interpreted-text role="ref"}.

The following is a copy of the **subscriber.c** file that is available from the repository.

::: {.literalinclude language="c" linenos=""}
../../../../examples/helloworld/subscriber.c
:::

To create a subscriber:

1.  To recieve data using the DDS API and the `HelloWorldData_Msg` type, include the appropriate header files:

    - The `dds.h` file to give access to the DDS APIs
    - The `HelloWorldData.h` is specific to the data type **defined in the IDL**

    ```c {.C linenos="" lineno-start="1"}
    #include "ddsc/dds.h"
    #include "HelloWorldData.h"
    ```

2.  **At least three DDS entities are needed** to build a minimalistic application:

    - Domain participant
    - Topic
    - Reader

    implicitly creates a DDS Subscriber. If required, this behavior can be overridden.

    ```c {.C linenos="" lineno-start="12"}
    dds_entity_t participant;
    dds_entity_t topic;
    dds_entity_t reader;
    ```

3.  To handle the data, create and declare some buffers:

    ```c {.C linenos="" lineno-start="15"}
    HelloWorldData_Msg *msg;
    void *samples[MAX_SAMPLES];
    dds_sample_info_t info[MAX_SAMPLES];
    ```

    The `read()` operation can return more than one data sample (where several publishing applications are started simultaneously to write different message instances), an array of samples is therefore needed.

    > `read()` 操作可以**返回多个数据样本**（其中同时启动多个发布应用程序以写入不同的消息实例），因此需要一组样本。

    In , data and metadata are propagated together. To handle the metadata, the `dds_sample_info` array must be declared.

    > 在 中，**数据和元数据一起传播。 要处理元数据，必须声明 `dds_sample_info` 数组**。

4.  The DDS participant is always attached to a specific DDS domain. In the HelloWorld example, it is part of the `Default_Domain`, which is specified in the XML configuration file. To override the default behavior, create or edit a configuration file (for example, `cyclonedds.xml`). For further information, refer to the `config-docs`{.interpreted-text role="ref"} and the `configuration_reference`{.interpreted-text role="ref"}.

> **DDS 参与者始终附加到特定的 DDS 域**。 在 HelloWorld 示例中，它是 XML 配置文件中指定的“Default_Domain”的一部分。 要**覆盖默认行为，请创建或编辑配置文件**（例如，`cyclonedds.xml`）。 如需更多信息，请参阅`config-docs`{.interpreted-text role="ref"} 和`configuration_reference`{.interpreted-text role="ref"}。

    ```c {.C linenos="" lineno-start="24"}
    participant = dds_create_participant (DDS_DOMAIN_DEFAULT, NULL, NULL);
    ```

5.  **Create the topic** with a given name. Topics with the same data type description and with different names are considered other topics. This means that readers or writers created for a given topic do not interfere with readers or writers created with another topic even if they have the same data type. Topics with the same name but incompatible datatype are considered an error and should be avoided.

> 使用给定名称创建主题。 具有相同数据类型描述和不同名称的主题被视为其他主题。 这意味着为给定主题创建的读者或作者不会干扰使用另一个主题创建的读者或作者，即使它们具有相同的数据类型。 名称相同但数据类型不兼容的主题被视为错误，应避免使用。

    ```c {.C linenos="" lineno-start="29"}
    topic = dds_create_topic (
        participant, &HelloWorldData_Msg_desc, "HelloWorldData_Msg", NULL, NULL);
    if (topic < 0)
        DDS_FATAL("dds_create_topic: %s\n", dds_strretcode(-topic));
    ```

6.  Create a data reader and attach to it:

    ```c {.C linenos="" lineno-start="35"}
    qos = dds_create_qos ();
    dds_qset_reliability (qos, DDS_RELIABILITY_RELIABLE, DDS_SECS (10));
    reader = dds_create_reader (participant, topic, qos, NULL);
    if (reader < 0)
        DDS_FATAL("dds_create_reader: %s\n", dds_strretcode(-reader));
    dds_delete_qos(qos);
    ```

    The read operation expects an array of pointers to a valid memory location. This means the samples array needs initialization by pointing the void pointer within the buffer array to a valid sample memory location. In the example, there is an array of one element; (`#define MAX_SAMPLES 1`.)

    > 读取操作需要一个指向有效内存位置的指针数组。 这意味着样本数组**需要通过将缓冲区数组中的 void 指针指向有效的样本内存位置来进行初始化**。 在这个例子中，有一个只有一个元素的数组； （`#define MAX_SAMPLES 1`。）

7.  Allocate memory for one `HelloWorldData_Msg`:

    ```c {.C linenos="" lineno-start="47"}
    samples[0] = HelloWorldData_Msg__alloc ();
    ```

8.  Attempt to read data by going into a polling loop that regularly scrutinizes and examines the arrival of a message:

    > 尝试通过进入轮询循环来读取数据，轮询循环定期检查和检查消息的到达：

    ```c {.C linenos="" lineno-start="54"}
    rc = dds_read (reader, samples, infos, MAX_SAMPLES, MAX_SAMPLES);
    if (rc < 0)
        DDS_FATAL("dds_read: %s\n", dds_strretcode(-rc));
    ```

    The `dds_read` operation returns the number of samples equal to the parameter `MAX_SAMPLE`. If data has arrived in the reader\'s cache, that number will exceed 0.

    > **`dds_read` 操作返回等于参数 `MAX_SAMPLE` 的样本数**。 如果数据已到达读取器的缓存，则该数字将超过 0。

9.  The Sample_info (`info`) structure shows whether the data is either:

    - **Valid data** means that it contains the payload provided by the publishing application.
    - **Invalid data** means we are reading the DDS state of the data Instance.

    - 有效数据意味着它包含发布应用程序提供的有效负载。
    - 无效数据意味着我们正在读取数据实例的 DDS 状态。

    The state of a data instance can be, _DISPOSED_ by the writer, or it is _NOT_ALIVE_ anymore, which could happen if the publisher application terminates while the subscriber is still active. In this case, the sample is not considered valid, and its sample `info[].Valid_data` the field is `False`:

    > 数据实例的状态可以是，_DISPOSED_ 被编写者，或者它不再是 _NOT_ALIVE_，如果发布者应用程序终止而订阅者仍然处于活动状态，则可能会发生这种情况。 在这种情况下，样本不被认为是有效的，其样本 `info[].Valid_data` 字段为 `False`：

    ```c {.C linenos="" lineno-start="59"}
    if ((ret > 0) && (info[0].valid_data))
    ```

10. If data is read, cast the void pointer to the actual message data type and display the contents:

    ```c {.C linenos="" lineno-start="62"}
    msg = (HelloWorldData_Msg*) samples[0];
    printf ("=== [Subscriber] Received : ");
    printf ("Message (%d, %s)\n", msg->userID, msg->message);
    break;
    ```

11. When data is received and the polling loop is stopped, release the allocated memory and delete the domain participant:

    ```c {.C linenos="" lineno-start="76"}
    HelloWorldData_Msg_free (samples[0], DDS_FREE_ALL);
    ```

12. **All the entities that are created under the participant**, such as the data reader and topic, **are recursively deleted**.

    ```c {.C linenos="" lineno-start="79"}
    rc = dds_delete (participant);
    if (rc != DDS_RETCODE_OK)
        DDS_FATAL("dds_delete: %s\n", dds_strretcode(-rc));
    ```
