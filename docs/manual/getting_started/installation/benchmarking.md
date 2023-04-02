---
title: Benchmarking Tools
---

The `ddsperf` tool primarily measures data _throughput_ and _latency_ of the DDS applications within a network, or within the same board. The `ddsperf` tool also helps to do sanity checks that ensure your configuration is correctly set up and running.

> “ddsperf”工具主要**测量网络内或同一板内 DDS 应用程序的数据吞吐量和延迟**。“ddsperf”工具还有助于进行健全性检查，以确保您的配置正确设置和运行。

In addition to `ddsperf`, there are dedicated `examples`{.interpreted-text role="ref"} that show how to measure the DDS system throughput and the latency with their associated codebase. You can start from the provided code and customize it to fit your requirements. Both the `ddsperf` tool and the examples perform the benchmarking using sequences of octets with different parameterized sizes.

> 除了“ddsperf”之外，还有专门的“示例”｛.explored text role=“ref”｝，展示了如何测量 DDS 系统吞吐量及其相关代码库的延迟。您可以从提供的代码开始，并对其进行自定义以满足您的需求。“ddsperf”工具和示例都使用具有不同参数化大小的八位字节序列来执行基准测试。

> [!Important]
> The `throughput` and `latency` examples are not suitable as general DDS benchmarking tools as they sacrifice performance for simplicity. You cannot compare test results from these tools with results from other tools.

# Test your installation

To test if your installation and configuration are working correctly, either:

> 要测试您的安装和配置是否正常工作，请执行以下操作之一：

- Use the `dsperf_tool`{.interpreted-text role="ref"}
  : The `ddsperf` tool sends a continuous stream of data at a variable frequency. This is useful for sanity checks and to bypass other sporadic network issues.

  > ：“ddsperf”工具以**可变频率发送连续的数据流**。这对于健全性检查和绕过其他偶发的网络问题非常有用。

- Use the `helloworld_test`{.interpreted-text role="ref"} example.
  : The **Hello World!** example sends a single message.

## The `ddsperf` tool {#dsperf_tool}

The `ddsperf` tool is pre-installed within `<installation-dir>/bin`.

> [!Note]
> The Python tooling uses `ddsperf` to provide the cyclonedds `performance` subcommand and acts as a front-end for `ddsperf`.

The following test ensures that the the loopback option is enabled.

> 以下测试可**确保环回选项处于启用状态**。

To complete the sanity checks of your DDS-based system:

1.  Open two terminals.
2.  In the first terminal, run the following command:
    ```bash
    ddsperf sanity
    ```
    The `sanity` option sends one data sample each second (1Hz).
3.  In the second terminal, start `ddsperf` in **Pong** mode to echo the data to the first instance of the `ddsperf` (started with the _Sanity_ option).

> 3.在第二个终端中，以**Pong**模式启动“ddsperf”，将数据回显到“ddspeff”的第一个实例（从*Sanity*选项开始）。

    ```bash
    ddsperf pong
    ```

![image](/_static/gettingstarted-figures/4.2-1.png){.align-center}

If the data is not exchanged on the network between the two ddsperf instances, it is probable that has not selected the the appropriate network card on both machines, or a firewall in-between is preventing communication.

> 如果两个 ddsperf 实例之间的网络上没有交换数据，则可能是两台机器上都没有选择适当的网卡，或者两者之间的防火墙阻止了通信。

automatically selects the most available network interface. This behavior can be overridden by changing the configuration file. For further information, refer to the section `test your installation

> 自动选择最可用的网络接口。可以通过更改配置文件来覆盖此行为。有关更多信息，请参阅“测试您的安装”一节
> <test_your_installation>`{.interpreted-text role="ref"}.

## HelloWorld {#helloworld_test}

To test your installation, includes a simple **HelloWorld!** application (see also the `helloworld_bm`{.interpreted-text role="ref"} example). **HelloWorld!** consists of two executables:

> 要测试您的安装，请包括一个简单的**HelloWorld！**应用程序（另请参见`helloworld_bm`｛.explored text role=“ref”｝示例）**Hello 世界！**由两个可执行文件组成：

> - `HelloworldPublisher`
> - `HelloworldSubscriber`

The **HelloWorld!** executables are located in:

> **地狱世界！**可执行文件位于：

- `<cyclonedds-directory>\build\bin\Debug` on Windows

> -Windows 上的`<循环目录>\build\bin\Debug`

- `<cyclonedds-directory>/build/bin` on Linux/macOS.

> -Linux/macOS 上的`<cycloneds-directory>/build/bin`。

> [!Note]
> Requires CMake with `-DBUILD_EXAMPLES=ON`.

> [!Note]
> There are some common issues with multiple network interface cards on machine configurations. The default behavior automatically detects the first available network interface card on your machine for exchanging the `hello world` message. To ensure that your publisher and subscriber applications are on the same network, you must select the correct interface card. To override the default behavior, create or edit a deployment file (for example, `cyclonedds.xml`) and update the property `//CycloneDDS/Domain/General/Interfaces/NetworkInterface[@address]`{.interpreted-text role="ref"} to point to it through the `CYCLONEDDS_URI` OS environment variable. For further information, refer to `config-docs`{.interpreted-text role="ref"} and the `configuration_reference`{.interpreted-text role="ref"}.

# Measuring latency

To measure latency between two different applications, you run two instances of the `ddsperf` tool. The first instance has the role of a _sender_ that sends a given amount of data (a sequence of octets) at a given rate. The second instance has the role of _receiver_ that sends back the same amount of data to the sender in a Ping-Pong scenario. The sending action is triggered by the **Ping** option. The receiving behavior is triggered by the **Pong** action. The sender measures the roundtrip time and computes the latency as half of the roundtrip time.

> 要测量两个不同应用程序之间的延迟，您需要运行“ddsperf”工具的两个实例。第一个实例的角色是*sender*，它以给定的速率发送给定数量的数据（八位字节序列）。第二个实例的角色是*receiver*，它在 Ping Pong 场景中向发送方发送相同数量的数据。发送操作由**Ping**选项触发。接收行为由**乒乓球**动作触发。发送方测量往返时间，并将延迟计算为往返时间的一半。

The Ping-Pong scenario avoids clock desynchronization issues that can occur between two machines that do not accurately share the same perception of the time in the network.

> 乒乓场景避免了在网络中对时间感知不准确的两台机器之间可能发生的时钟去同步问题。

![image](/_static/gettingstarted-figures/4.3-1.png){.align-center}

To differentiate the two operational modes, the `ddsperf` tool can operate either in a **Ping mode** or in a **Pong mode**.

> 为了区分这两种操作模式，“ddsperf”工具可以在**Ping 模式**或**Pong 模式**下操作。

To run the Ping-Pong scenario (with default values):

> 要运行乒乓球场景（使用默认值）：

1.  Open two terminals (for example, on Linux-like OSs).

> 1.打开两个终端（例如，在类似 Linux 的操作系统上）。

2.  In the first terminal, run the following command:

> 2.在第一个终端中，运行以下命令：

    ```bash
    ddsperf ping
    ```

3.  In the second terminal run the following command:

> 3.在第二个终端中运行以下命令：

    ```bash
    ddsperf pong
    ```

To customize the test scenario, you can change the following options:

> 要自定义测试场景，您可以更改以下选项：

- In **Ping mode** you can specify:

> -在**Ping 模式**中，您可以指定：

- The **Rate** and frequency at which data is written. This is specified through the [R[Hz]] option. The default rate is "as fast as possible". In **ping** mode, it always sends a new ping as soon as it gets a pong.

> -写入数据的**速率**和频率。这是通过[R[Hz]]选项指定的。默认速率是“尽可能快”。在**ping**模式下，它总是在收到乒乓球后立即发送一个新的 ping。

- The **Size** of the data that is exchanged. This is specified through the [Size S] option. Using the built-in default topic, 12 bytes (an integer key, an integer sequence number, and an empty sequence of bytes) are sent every time. The size is "as small as possible" by default, depending on the default topic size.

> -交换的数据的**大小**。这是通过[Size S]选项指定的。使用内置的默认主题，每次发送 12 个字节（一个整数键、一个整数序列号和一个空的字节序列）。默认情况下，大小是“尽可能小”，具体取决于默认的主题大小。

- The **Listening** mode, which can either be `waitset` based or `Listener` Callbacks modes. In the waitset mode, the `ddsperf` application creates a dedicated thread to wait for the data to return back from the receiving instance of `ddsperf` tool (that is, the instance started with the Pong mode). In the Listener Callback mode, the thread is created by the middleware. The Listener mode is the default.

> -**Listening**模式，可以是基于“waitset”的回调模式，也可以是“Listener”回调模式。在 waitset 模式下，“ddsperf”应用程序创建一个专用线程，以等待数据从“ddspeff”工具的接收实例（即以 Pong 模式启动的实例）返回。在 Listener 回调模式中，线程由中间件创建。侦听器模式是默认模式。

- In **Pong mode**,you can only specify one option:

> -在**乒乓球模式**中，您只能指定一个选项：

- The **Listening** mode (with two possible values, `waitset` or `Listener`)

> -**侦听**模式（有两个可能的值，“waitset”或“Listener”）

For example, to measure local latency between to processes exchanging 2KB at the frequency of 50Hz:

> 例如，要测量以 50Hz 频率交换 2KB 的进程之间的本地延迟：

1.  Open two terminals.

> 1.打开两个端子。

2.  In the first terminal, run the following command:

> 2.在第一个终端中，运行以下命令：

    ```bash
    ddsperf ping 50Hz 2048 waitset
    ```

3.  In the second terminal run the following command:

> 3.在第二个终端中运行以下命令：

    ```bash
    ddsperf pong waitset
    ```

The output of the `ddsperf` tool is as shown below:

> “ddsperf”工具的输出如下所示：

- The output for the **Ping** application:

> -**\*Ping**应用程序的输出：

![image](/_static/gettingstarted-figures/4.3-2.png)

> ![image]（/\_static/gettingstarted fights/4.3-2.png）

- The **size of the data** involved in the test (For example, 12 bytes)

> -测试中涉及的数据**的**大小（例如，12 字节）

- The **minimum latency** (For example, 78.89 us)

> -**最小延迟**（例如，78.89 us）

- The **maximum latency** (For example, 544,85 us)

> -**最大延迟**（例如，544,85 us）

- The **mean latency** (For example, 118.434 us)

> -**平均延迟**（例如，118.434 us）

- As well as the latency at 50%, 90%, or 99% of the time.

> -以及 50%、90%或 99%时间的延迟。

- The output for the **Pong** application:

> -**Pong**应用程序的输出：

![image](/_static/gettingstarted-figures/4.3-3.png)

> ![image]（/\_static/gettingstarted fights/4.3-3.png）

- **RSS** is the Resident Set Size; it indicates the amount of memory : used by the process (For example, 3.5MB used by the process ID 2680).

> -**RSS**是常驻集大小；它表示进程使用的内存量（例如，进程 ID 2680 使用的 3.5MB）。

- **VCSW** is the number of voluntary switches, it indicates the times when the process waits for input or an event (For example, 2097 times).

> -**VCSW**是自愿切换的次数，表示进程等待输入或事件的时间（例如 2097 次）。

- **IVCSW** is the number of involuntary switches, it indicates the times when the process is pre-empted or blocked by a mutex (For example, 6 times).

> -**IVCSW**是非自愿切换的次数，表示进程被互斥体抢占或阻塞的次数（例如，6 次）。

- The percentage of time spent executing user code and the percentage of time spent executing the kernel code in a specific thread (For example, spent almost 0% of the time executing the user code and 5% executing kernel code in thread "ping").

> -在特定线程中执行用户代码所花费的时间百分比和执行内核代码所花费时间的百分比（例如，在线程“ping”中，几乎有 0%的时间用于执行用户代码，5%的时间用于运行内核代码）。

# Measuring throughput

To measure throughput between two different applications, run at least two instances of the `ddsperf` tool. One terminal acts as a Publisher that sends a set amount of data (a sequence of octets) at a set rate. The other instances act as Subscriber applications.

> 要测量两个不同应用程序之间的吞吐量，请至少运行两个“ddsperf”工具实例。一个终端充当发布服务器，以设定的速率发送设定数量的数据（八位字节序列）。其他实例充当订阅服务器应用程序。

> [!Note]

When your scenario involves **only one subscriber, the UDP unicast mode is used**. If **several subscriber instances are running, the multicast is automatically used**.

> 当您的场景只涉及**一个订户时，将使用 UDP 单播模式**。如果**多个订阅者实例正在运行，则会自动使用多播**。

![image](/_static/gettingstarted-figures/4.4-1.png)

Two additional modes are supported:

> 支持两种附加模式：

The **Pub** mode and the **Sub** mode.

> **发布**模式和**子**模式。

In the Sub mode, the subscriber operates in one of the following ways:

> 在子模式中，订阅者以以下方式之一进行操作：

- **Listener** notifications. Receives a notification each time a new set of data is added to the subscriber's cache.

> -**侦听器**通知。每次向订阅服务器的缓存中添加一组新数据时，都会接收一个通知。

- **WaitSet** notifications. Receives a notification whenever the conditions of a WaitSet are met. For further information, refer to `waitset_conditions`{.interpreted-text role="ref"}.

> -**等待设置**通知。只要满足 WaitSet 的条件，就会接收通知。有关更多信息，请参阅`waitset_conditions`｛.depreted text role=“ref”｝。

- **Pooling** mode. The subscriber cyclically fetches the data from its local cache.

> -**汇集**模式。订阅者循环地从其本地缓存中获取数据。

There are two ways to publish each data sample; individually, or by sending them in _Burst_ mode. The following are the parameters for _Burst_ mode:

> 有两种方法可以发布每个数据样本；单独地或通过以*突发*模式发送它们。以下是*Burst*模式的参数：

- The **Rate** and frequency at which data is written (defined by the [R[Hz]] option). The default rate is "as fast as possible". In **pub** mode, instead of trying to reach a set rate, it sends data as fast as possible.

> -**速率**和写入数据的频率（由[R[Hz]]选项定义）。默认速率是“尽可能快”。在**pub**模式下，它不会试图达到设定的速率，而是以尽可能快的速度发送数据。

- The **Size** of the data that is exchanged (defined by the [Size S] option). The default size is "as small as possible" (depending on the size of the topic).

> -交换的数据的**大小**（由[Size S]选项定义）。默认大小为“尽可能小”（取决于主题的大小）。

- The **Burst Size** (only applies to the **pub** mode) is the number of data samples issued together as a batch (defined by the [Burst N] option). The default size for burst is 1. Note: When going "as fast as possible", this option does not make any difference.

> -**突发大小**（仅适用于**pub**模式）是作为一个批次一起发布的数据样本数量（由[Burst N]选项定义）。突发的默认大小为 1。注意：当进行“尽可能快”时，此选项没有任何区别。

- The default triggering mode is _listener_ for the **ping** , **pong** and **sub** mode.

> -对于**ping**、**pong**和**sub**模式，默认触发模式为*listener*。

To run a simple throughput test (with default values):

> 要运行一个简单的吞吐量测试（使用默认值）：

1.  Open two terminals.

> 1.打开两个端子。

2.  In the first terminal, run the following command:

> 2.在第一个终端中，运行以下命令：

    ```bash
    ddsperf pub size 1k
    ```

3.  In the second terminal run the following command:

> 3.在第二个终端中运行以下命令：

    ```bash
    ddsperf -Qrss:1 sub
    ```

This test measures the throughput of data samples with 1Kbytes written as fast as possible.

> 此测试测量以尽可能快的速度写入 1K 字节的数据样本的吞吐量。

The `-Qrss:1` option in **sub** mode sets the maximum allowed an increase in RSS as 1MB. When running the test, if the memory occupied by the process increases by less than 1MB, the test can successfully run. Otherwise, an error message is printed out at the end of the test.

> **子**模式中的“-Qrss:1”选项将允许 RSS 增加的最大值设置为 1MB。运行测试时，如果进程占用的内存增加不到 1MB，则测试可以成功运行。否则，将在测试结束时打印出一条错误消息。

As the `pub` in this example only has a size of 1k, the sub does not print out an RSS error message at the end of the test.

> 由于本例中的“pub”只有 1k 的大小，因此 sub 在测试结束时不会打印出 RSS 错误消息。

The output of the `ddsperf` tool when measuring throughput is as shown below:

> 测量吞吐量时“ddsperf”工具的输出如下所示：

- The output for the **Pub** application:

> -**发布**应用程序的输出：

    ![image](/_static/gettingstarted-figures/4.4-2.png)

- **RSS** is the Resident Set Size; it indicates the amount of memory used by the process (For example, 6.3MB used by the process ID "4026");

> -**RSS**是常驻集大小；它表示进程使用的内存量（例如，进程 ID“4026”使用的 6.3MB）；

- **VCSW** is the number of voluntary switches, it indicates the times when the process waits for input or an event (For example, 1054 times);

> -**VCSW**是自愿切换的次数，表示进程等待输入或事件的次数（例如 1054 次）；

- **IVCSW** is the number of involuntary switches, it indicates the times when the process is pre-empted or blocked by a mutex (For example, 24 times);

> -**IVCSW**是非自愿切换的次数，表示进程被互斥体抢占或阻塞的次数（例如 24 次）；

- The percentage of time spent executing user code and the percentage of time spent executing kernel code in a specific thread (For example, spent 34% of the time executing the user code and 11% executing kernel code in thread "pub").

> -在特定线程中执行用户代码所花费的时间百分比和执行内核代码所花费时间的百分比（例如，在线程“pub”中，34%的时间用于执行用户代码，11%的时间用于运行内核代码）。

- The output for the **Sub** application:

> -**\*Sub**应用程序的输出：

    ![image](/_static/gettingstarted-figures/4.4-3.png)

- The **size of the data** involved in this test (For example, 1024 bytes, which is the "size 1k" defined in the pub command).

> -此测试中涉及的数据**的**大小（例如，1024 字节，这是 pub 命令中定义的“大小 1k”）。

- The **total packets received** (For example, 614598).

> -接收到的**个数据包总数**（例如，614598 个）。

- The **total packets lost** (For example, 0).

> -**丢失的数据包总数**（例如，0）。

- The **packets received in a 1 second reporting period** (For example, 212648).

> -在 1 秒报告周期内接收到的**数据包**（例如，212648）。

- The **packets lost in a 1 second report period** (For example, 0).

> -在 1 秒的报告周期内丢失的**数据包**（例如，0）。

- The **number of samples processed by the Sub application** in 1s (For example, 21260 KS/s, with the unit KS/s is 1000 samples per second).

> -子应用程序**在 1 秒内处理的样本数量**（例如，21260 KS/s，单位为 KS/s，即每秒 1000 个样本）。

# Measuring Throughput and Latency in a mixed scenario

In some scenarios, it can be useful to measure the throughput and latency at the same time.

> 在某些情况下，同时测量吞吐量和延迟可能很有用。

The `ddsperf` tool allows you to mix these two scenarios. The Ping mode can be combined with the Pub mode to address such cases.

> “ddsperf”工具允许您混合使用这两种场景。Ping 模式可以与 Pub 模式相结合来解决这种情况。

The [Ping x%] switch combined with the Pub mode allows you to send a fraction of samples x% as if they were used in the Ping mode.

> [Ping x%]开关与 Pub 模式相结合，允许您发送一小部分样本 x%，就好像它们在 Ping 模式中使用一样。

The different modes of the `ddsperf` tool are summarized in the figure below.

> 下图总结了“ddsperf”工具的不同模式。

![image](/_static/gettingstarted-figures/4.5-1.png){.align-center}

To get more information for the `ddsperf` tool, use the [help] option:

> 要获取有关“ddsperf”工具的更多信息，请使用[help]选项：

    ```bash
    ddsperf help
    ```

# Additional options

As well as selecting the `mode`, you can also select the `options` to specify how to send and receive the data (such as modifying the reliable QoS from Reliable to Best-Effort with the `-u` option), or how to evaluate or view the data in the `ddsperf`tool.

> 除了选择“模式”外，您还可以选择“选项”来指定如何发送和接收数据（例如使用“-u”选项将可靠 QoS 从可靠修改为尽力而为），或者如何在“ddsperf”工具中评估或查看数据。

The `options` you can select are listed in the `ddsperf` `help` menu, as shown below.

> 您可以选择的“选项”列在“ddsperf”“帮助”菜单中，如下所示。

![image](/_static/gettingstarted-figures/4.6-1.png){.align-center}
