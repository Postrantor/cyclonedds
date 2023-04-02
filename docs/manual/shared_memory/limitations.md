# Limitations of shared memory {#shared_mem_limits}

Due to the manner in which data is exchanged over shared memory, there are some limitations to the types of data that may be exchanged, and delivery are required to ensure their correct functioning.

> 由于在共享内存上交换数据的方式，可能交换的数据类型存在一些限制，需要进行传输以确保其正确运行。

## Fixed size data types

The data types to be exchanged need to have a fixed size. This precludes the use of strings and sequences at any level in the data type. This does not prevent the use of arrays, as their size is fixed at compile time. If any of these types of member variables are encountered in the IDL code generating the data types, shared memory exchange is disabled.

> 要交换的数据类型需要具有固定的大小。这就排除了在数据类型的任何级别上使用字符串和序列。这并不妨碍数组的使用，因为数组的大小在编译时是固定的。如果在生成数据类型的 IDL 代码中遇到这些类型的成员变量中的任何一个，则将禁用共享内存交换。

A possible workaround for this limitation is to use fixed size arrays of chars instead of strings (and arrays of other types in stead of sequences), and accept the overhead.

> 对于这种限制，一种可能的解决方法是使用固定大小的字符数组而不是字符串（以及其他类型的数组而不是序列），并接受开销。

## iceoryx data exchange

The manner in which the memory pool keeps track of exchanged data puts a number of limitations on the QoS settings. For Writers, the following QoS settings are prerequisites for shared memory exchange:

> 内存池跟踪交换数据的方式对 QoS 设置提出了许多限制。对于写入程序，以下 QoS 设置是共享内存交换的先决条件：

    +-------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
    | Liveliness | :c`DDS_LIVELINESS_AUTOMATIC`{.interpreted-text role="macro"} |
    +-------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
    | Deadline | :c`DDS_INFINITY`{.interpreted-text role="macro"} |
    | | |
    | | `DDS_INFINITY` is a define in that indicates an amount of time in excess of the maximum duration that can be represented. It is used to tell the interface that this duration is infinite. If timeouts depend on this duration, they will never occur in the program\'s lifetime. Due to this, some code paths can differ, and be more efficient. |
    +-------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
    | Reliability | :c`DDS_RELIABILITY_RELIABLE`{.interpreted-text role="macro"} |
    +-------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
    | Durability | :c`DDS_DURABILITY_VOLATILE`{.interpreted-text role="macro"} |
    +-------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
    | History | :c`DDS_HISTORY_KEEP_LAST`{.interpreted-text role="macro"} |
    | | |
    | | The depth is no larger than the publisher history capacity as set in the configuration file |
    +-------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+

For Readers, the following QoS settings are prerequisites for shared memory exchange:

> 对于读卡器，以下 QoS 设置是共享内存交换的先决条件：

    +-------------+---------------------------------------------------------------------------------------------+
    | Liveliness | :c`DDS_LIVELINESS_AUTOMATIC`{.interpreted-text role="macro"} |
    +-------------+---------------------------------------------------------------------------------------------+
    | Deadline | :c`DDS_INFINITY`{.interpreted-text role="macro"} |
    +-------------+---------------------------------------------------------------------------------------------+
    | Reliability | :c`DDS_RELIABILITY_RELIABLE`{.interpreted-text role="macro"} |
    +-------------+---------------------------------------------------------------------------------------------+
    | Durability | :c`DDS_DURABILITY_VOLATILE`{.interpreted-text role="macro"} |
    +-------------+---------------------------------------------------------------------------------------------+
    | History | :c`DDS_HISTORY_KEEP_LAST`{.interpreted-text role="macro"} |
    | | |
    | | The depth is no larger than the subscriber history request as set in the configuration file |
    +-------------+---------------------------------------------------------------------------------------------+

If any of these prerequisites are not satisfied, shared memory exchange is disabled and data transfer falls back to the network interface.

> 如果不满足这些先决条件中的任何一个，共享内存交换将被禁用，数据传输将回退到网络接口。

A further limitation is that the maximum number of subscriptions per process for the iceoryx service is 127.

> 另一个限制是 iceoryx 服务的每个进程的最大订阅数为 127。

## Operating system limitations

The limit on the operating system. iceoryx has no functioning implementation for the Windows operating system. Refer to for further information.

> 操作系统的限制。iceoryx 没有适用于 Windows 操作系统的功能实现。有关更多信息，请参阅。
