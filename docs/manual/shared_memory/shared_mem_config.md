# Shared memory configuration {#shared_mem_config}

There are two levels of configuration for with shared memory support, the shared memory service () level, and the level.

> 具有共享内存支持的配置有两个级别，即共享内存服务（）级别和级别。

The performance of the shared memory service is strongly dependent on how well its configuration matches up with the use-cases it is asked to support. By configuring shared memory correctly, large gains in performance can be made.

> 共享内存服务的性能在很大程度上取决于其配置与要求支持的用例匹配的程度。通过正确配置共享内存，可以大幅提高性能。

The memory of iceoryx is layed out as numbers of fixed-size segments, which are iceoryx\'s memory pool. When a subscriber requests a block of memory from iceoryx, the smallest block that fits the requested size (and is available) is provided to the subscriber from the pool. If no blocks can be found that satisfy these requirements, the publisher requesting the block gives an error and aborts the process.

> iceoryx 的内存以固定大小的段的数量进行布局，这些段是 iceoryx\的内存池。当订阅者从 iceoryx 请求内存块时，会从池中向订阅者提供符合请求大小（并且可用）的最小内存块。如果找不到满足这些要求的块，则请求该块的发布者会给出一个错误并中止进程。

::: note
::: title
Note
:::

For testing, the default memory configuration usually suffices.

> 对于测试来说，默认的内存配置通常就足够了。
> :::

The default configuration has blocks in varying numbers and sizes up to 4MiB. iceoryx falls back to this configuration when it is not supplied with a suitable configuration file, or cannot find one at the default location.

> 默认配置具有不同数量和大小的块，最大可达 4MiB。当 iceoryx 没有提供合适的配置文件，或者在默认位置找不到配置文件时，它会返回到此配置。

To ensure the best performance with the smallest footprint, configure iceoryx so that the memory pool only consists of blocks that are useful to the exchanges to be done. Due to header information being sent along with the sample, the block size required from the pool is 64 bytes larger than the data type being exchanged. iceoryx requires that the blocks are aligned to 4 bytes.

> 为了确保以最小的占用空间获得最佳性能，请配置 iceoryx，使内存池仅由对要进行的交换有用的块组成。由于标头信息与样本一起发送，因此池中所需的块大小比正在交换的数据类型大 64 字节。iceoryx 要求将块对齐到 4 个字节。

The following is an example of an iceoryx configuration file that has a memory pool of 2\^15 blocks, which can store data types of 16384 bytes (+ 64 byte header = 16448 byte block):

> 以下是一个 iceoryx 配置文件的示例，该文件的内存池为 2/15 个块，可以存储 16384 字节的数据类型（+64 字节的标头=16448 字节的块）：

```toml
[general]
version = 1

[[segment]]

[[segment.mempool]]
size = 16448
count = 32768
```

The configuration file is supplied to iceoryx using the `-c` parameter.

> 配置文件是使用“-c”参数提供给 iceoryx 的。

::: note
::: title
Note
:::

This file is used in the `shared_mem_example`{.interpreted-text role="ref"}. Save this file as _iox_config.toml_ in your home directory.

> 此文件用于`shared_mem_example`｛.explored text role=“ref”｝中。将此文件保存为主目录中的*iox_config.toml*。
> :::

also needs to be configured correctly, to allow it to use shared memory exchange.

> 还需要正确配置，以允许其使用共享内存交换。

The location where looks for the config file is set through the environment variable _CYCLONEDDS_URI_:

> 查找配置文件的位置是通过环境变量*CYCLONEDDS_URI*设置的：

```bash
export CYCLONEDDS_URI=file://cyclonedds.xml
```

The following optional configuration parameters in SharedMemory govern how treats shared memory:

> SharedMemory 中的以下可选配置参数控制如何处理共享内存：

- Enable
- When set to _true_ enables to use shared memory for local data exchange
- Defaults to _false_
- LogLevel
- Controls the output of the iceoryx runtime and can be set to, in order of decreasing output:
  - _verbose_
  - _debug_
  - _info_ (default)
  - _warn_
  - _error_
  - _fatal_
  - _off_

The following is an example of a configuration file supporting shared memory exchange:

> 以下是支持共享内存交换的配置文件示例：

```xml
<?xml version="1.0" encoding="UTF-8" ?>
<CycloneDDS xmlns="https://cdds.io/config"
            xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
            xsi:schemaLocation="https://cdds.io/config https://raw.githubusercontent.com/eclipse-cyclonedds/cyclonedds/iceoryx/etc/cyclonedds.xsd">
    <Domain id="any">
        <SharedMemory>
            <Enable>true</Enable>
            <LogLevel>info</LogLevel>
        </SharedMemory>
    </Domain>
</CycloneDDS>
```

::: note
::: title
Note
:::

This file is used in the `shared_mem_example`{.interpreted-text role="ref"}. Save this file as _cyclonedds.xml_ in your home directory.

> 此文件用于`shared_mem_example`｛.explored text role=“ref”｝中。将此文件保存为主目录中的\_cyclonedds.xml。
> :::
