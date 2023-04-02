# Asynchronous write

`Asynchronous Write` is a feature where the actual sending of the data packets **occurs on a different thread**, than the thread where the `dds_write()` function is invoked. By default when application writes data using a DDS data writer, the entire write operation involving the steps from serialization to writing the actual data packets on to the socket happen synchronously in the same thread, which is called `Synchronous Write`. Applications can enable `Asynchronous Write` mode where the dds_write() call will only copy the data to the writer history cache and queues the data, but the actual transmission of the data happens asynchronously in a separate thread.

> `Asynchronous Write` 是一种功能，其中**数据包的实际发送发生在与调用 `dds_write()` 函数的线程不同的线程上**。默认情况下，当应用程序使用 DDS 数据写入器写入数据时，**从序列化到将实际数据包写入套接字的整个写入操作在同一个线程中同步发生**，称为 **“同步写入”**。
> 应用程序可以启用 **“异步写入”** 模式，**其中 `dds_write()` 调用只会将数据复制到写入器历史缓存并将数据排队，但数据的实际传输在单独的线程中异步发生**。

## Asynchronous write behavior

By default Cyclone DDS uses "synchronous write", to use "asynchronous write", **the latency budget of a data writer should be greater than zero**.

> 默认情况下 Cyclone DDS 使用“同步写入”，要使用“异步写入”，数据写入器的延迟预算应该大于零。

```c
/* Create a Writer with non-zero latency budget */
dwQos = dds_create_qos ();
dds_qset_latency_budget(dwQos, DDS_MSECS(5));
writer = dds_create_writer (participant, topic, dwQos, NULL);
...
// This writer sends the data asynchronously
```

The below diagram shows the current API call sequence for the write:

<img src="pictures/async_write_cyclone_dds.png" alt="Asynchronous write API sequence">

- `sendq` thread (thread which is responsible for sending the data asynchronously) is started when the first data writer is created with a latency budget greater then zero.
- `sendq` thread is stopped during the shutdown clean-up
- When a sample is written in asynchronous mode, the following happens on the high level:

  - `ddsi_serdata` is constructed from the sample
  - Sample is inserted into writer history cache (WHC)
  - Packs the sample into RTPS message
  - Queues the RTPS messages
  - `sendq` thread transmits the queued RTPS messages asynchronously

  - `ddsi_serdata` 是从样本构建的
  - 示例被插入到编写器历史缓存 (WHC)
  - 将样本打包成 RTPS 消息
  - 排队 RTPS 消息
  - `sendq` 线程异步传输排队的 RTPS 消息

## Improvements

1. Queue individual samples (`ddsi_serdata`), pack and send them instead of queueing and sending the entire RTPS messages. This will be an improvement for use case of sending small messages at high rates.

> 1. 排队单个样本 (`ddsi_serdata`)，打包并发送它们，而不是排队并发送整个 RTPS 消息。这将是对以高速率发送小消息的用例的改进。
