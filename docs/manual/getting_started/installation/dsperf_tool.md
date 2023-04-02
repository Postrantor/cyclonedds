---
title: The `ddsperf` tool
---

The `ddsperf` tool is pre-installed within `<installation-dir>/bin`.

> [!Note]
> The Python tooling uses `ddsperf` to provide the cyclonedds `performance` subcommand and acts as a front-end for `ddsperf`.

The following test ensures that the the loopback option is enabled.

> 以下测试可确保环回选项处于启用状态。

To complete the sanity checks of your DDS-based system:

> 要完成基于 DDS 的系统的健全性检查：

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
