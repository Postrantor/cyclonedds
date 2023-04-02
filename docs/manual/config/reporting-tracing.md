# Reporting and tracing {#Reporting and tracing}

can produce highly detailed traces of all traffic and internal activities. It enables individual categories of information and a simple verbosity level that enables fixed sets of categories.

> 可以生成所有流量和内部活动的高度详细的跟踪。它支持单独的信息类别和一个简单的详细级别，该级别支持固定的类别集。

All _fatal_ and _error_ messages are written both to the trace and to the `cyclonedds-error.log` file.

> 所有 _fatal_ 和 _error_ 消息都写入跟踪和“cycloneds error.log”文件。

All \"warning\" messages are written to the trace and the `cyclonedds-info.log` file.

> 所有“警告”消息都会写入跟踪和“cycloneds-info.log”文件。

The Tracing element has the following sub elements:

> Tracing 元素包含以下子元素：

- Verbosity
- EnableCategory

Whether these logging levels are set in the verbosity level or by enabling the corresponding categories is immaterial.

> 无论这些日志记录级别是在详细级别中设置的，还是通过启用相应的类别来设置的，都无关紧要。

## Verbosity

Selects a tracing level by enabling a pre-defined set of categories. The following table lists the known tracing levels, and the categories they enable:

> 通过启用预定义的一组类别来选择跟踪级别。下表列出了已知的跟踪级别及其启用的类别：

.. list-table::
:align: left
:widths: 20 80

    * - ``none``
    -
    * - ``severe``
    - ``error``, ``fatal``
    * - ``warning``, ``info``
    - ``severe``, ``warning``
    * - ``config``
    - ``info``, ``config``

        Writes the complete configuration to the trace file and any warnings or errors, which can be an effective method to verify that everything is configured and behaving as expected.
        将完整的配置写入跟踪文件以及任何警告或错误，这可能是一种验证一切是否按预期配置和运行的有效方法。
    * - ``fine``
    - ``config``, ``discovery``

        Adds the complete discovery information in the trace (but nothing related to application data or protocol activities). If a system has a stable topology, this typically results in a moderate size trace.
    * - ``finer``
    - ``fine``, ``traffic``, ``timing``, ``info``
    * - ``finest``
    - ``fine``, ``trace``

        Provides a detailed trace of everything that occurs and is an indispensable source of information when analysing problems. However, it also requires a significant amount of time and results in very large log files.

## EnableCategory

This is a comma-separated list of keywords. Each keyword enables individual categories. The following keywords are recognised:

> 这是一个逗号分隔的关键字列表。每个关键字都支持单独的类别。可识别以下关键字：

---

`fatal` All fatal errors, errors causing immediate termination.
`error` Failures probably impacting correctness but not necessarily causing immediate termination.
`warning` Abnormal situations that will likely not impact correctness.
`config` Full dump of the configuration.
`info` General informational notices.
`discovery` All discovery activity.
`data` Include data content of samples in traces.
`timing` Periodic reporting of CPU loads per thread.
`traffic` Periodic reporting of total outgoing data.
`tcp` Connection and connection cache management for the TCP support.
`throttle` Throttling events where the Writer stalls because its WHC hit the high-water mark.
`topic` Detailed information on topic interpretation (in particular topic keys).
`plist` Dumping of parameter lists encountered in discovery and inline QoS.
`radmin` Receive buffer administration.
`whc` Very detailed tracing of WHC content management.

> `fatal` 所有致命错误，导致立即终止的错误。
> `error` 失败可能会影响正确性，但不一定会立即终止。
> `warning` 可能不会影响正确性的异常情况。
> `config` 配置的完整转储。
> `info` 一般信息通知。
> `discovery` 所有发现活动。
> `data` 包含迹线中样本的数据内容。
> `timing` 定期报告每个线程的 CPU 负载。
> `traffic` 总传出数据的定期报告。
> `tcp` TCP 支持的连接和连接缓存管理。
> `throttle` Writer 因 WHC 达到高水位线而停止的节流事件。
> `topic` 有关主题解释的详细信息（特别是主题键）。
> `plist` 转储发现和内联 QoS 中遇到的参数列表。
> `radmin` 接收缓冲区管理。
> `whc` WHC 内容管理的非常详细的跟踪。

---

The keyword _trace_ enables all categories from _fatal_ to _throttle_.

> 关键字 _trace_ 启用从 _fatal_ 到 _throttle_ 的所有类别。

The _topic_ and _plist_ categories are useful only for particular classes of discovery failures.

> _topic_ 和 _plist_ 类别仅对特定类别的发现失败有用。

The _radmin_ and _whc_ categories only help in analysing the detailed behaviour of those two components and produce significant amounts of output.

> _radmin_ 和 _whc_ 类别只能帮助分析这两个组件的详细行为，并产生大量的输出。

The file location is set in the configuration: `OutputFile <//CycloneDDS/Domain/Tracing/OutputFile>`{.interpreted-text role="ref"}

> 文件位置在配置中设置：`OutputFile</CycloneDDS/Domain/Tracing/OutputFile>`｛.depredicted text role=“ref”｝

To append to the trace instead of replacing the file, set: `AppendToFile <//CycloneDDS/Domain/Tracing/AppendToFile>`{.interpreted-text role="ref"} to `true`

> 要追加到跟踪而不是替换文件，请将：`AppendToFile</CycloneDDS/Domain/Tracing/AppendToFile>`{.depreced text role=“ref”}设置为`true`
