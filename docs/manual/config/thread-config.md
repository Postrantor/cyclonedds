# Thread configuration

creates several threads, each with a number of properties that can be controlled individually. These properties are:

> 创建几个线程，每个线程都有一些可以单独控制的属性。这些属性包括：

- Stack size
- Scheduling class
- Scheduling priority

> - 堆栈大小
> - 调度类
> - 调度优先级

Each thread is uniquely named. To set the properties for that thread, use the unique thread name with the `Threads/Thread[@name] <//CycloneDDS/Domain/Threads/Thread[@name]>` option.

> 每个线程都有唯一的名称。要设置该线程的属性，请将唯一的线程名称与“Threads/thread[@name]”选项一起使用。

Any subset of threads can be given special properties. Any paramaters that are not explicitly specified use the default values.

> 任何线程子集都可以被赋予特殊的属性。任何未明确指定的参数都使用默认值。

The following threads exist:

.. list-table::
:align: left
:widths: 20 80

    * - ``gc``
      - Garbage collector, which sleeps until garbage collection is requested for an entity. When requested, it starts monitoring the state of |var-project|, and when safe to do so, pushes the entity through the necessary state transitions. The process ends with the freeing of the memory.
      - 垃圾收集器，它会休眠直到实体请求垃圾收集。 当收到请求时，它会开始监视 |var-project| 的状态，并在安全的情况下推动实体进行必要的状态转换。 该过程以释放内存结束。
    * - ``recv``
      -  - Accepts incoming network packets from all sockets/ports.
         - Performs all protocol processing.
         - Queues (nearly) all protocol messages sent in response for handling by the timed-event thread.
         - Queues for delivery (in special cases, delivers it directly to the data readers).
    * - ``dq.builtins``
      - Processes all discovery data coming in from the network.
    * - ``lease``
      - Performs internal liveliness monitoring of |var-project|.
    * - ``tev``
      - Timed-event handling. Used for:
         - Periodic transmission of participant discovery and liveliness messages.
         - Transmission of control messages for reliable writers and readers (except those that have their own timed-event thread)
         - Retransmitting of reliable data on request (except those that have their own timed-event thread)
         - Handling of start-up mode to normal mode transition.
         - 参与者发现和活跃消息的定期传输。
         - 为可靠的作者和读者传输控制消息（除了那些有自己的定时事件线程的）
         - 根据要求重传可靠数据（除了那些有自己的定时事件线程的）
         - 处理启动模式到正常模式的转换。

For each defined channel:

.. list-table::
:align: left
:widths: 20 80

    * - ``dq.channel-name``
      - Deserialisation and asynchronous delivery of all user data.
    * - ``tev.channel-name``
      - Channel-specific "timed-event" handling transmission of control messages for reliable writers and Readers and re-transmission of data on request. Channel-specific threads exist only if the configuration includes an element for it, or if an auxiliary bandwidth limit is set for the channel.
      - 特定于通道的“定时事件”为可靠的编写者和读者处理控制消息的传输，并根据请求重新传输数据。 特定于通道的线程仅在配置包含它的元素时存在，或者如果为通道设置了辅助带宽限制。

When no channels are explicitly defined, there is one channel named _user_.

> 当没有明确定义通道时，会有一个名为*user*的通道。
