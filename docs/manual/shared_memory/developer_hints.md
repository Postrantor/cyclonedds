# Developer hints {#dev_hints}

The initial implementation is from . To integrate the latest C-API and support zero copy data transfer, contributions were made by . Further contributions and feedback from the community are very welcome, see `contributing_to_dds`{.interpreted-text role="ref"}.

> 最初的实施来自。为了集成最新的 C-API 并支持零拷贝数据传输，做出了贡献。非常欢迎社区提供进一步的贡献和反馈，请参阅`contributing_to_dds`{.depreted text role=“ref”}。

The following is a list of useful tips:

> 以下是有用提示列表：

- Most of the shared memory modification is under the define :c`DDS_HAS_SHM`{.interpreted-text role="macro"}. [DDS_HAS_SHM]{.title-ref} is a flag set through `cmake` when compiling, which enables shared memory support.

> -大多数共享内存修改都是在定义下进行的：c`DDS_HAS_SHM`{.depreted text role=“macro”}。[DDS_HAS_SHM]｛.title-ref｝是编译时通过“cmake”设置的标志，它支持共享内存。

- To learn about the internal happenings of the service, there is a useful tool from iceoryx called :

> - 要了解服务的内部情况，iceoryx 提供了一个有用的工具，名为：

```bash
~/iceoryx/build/iox-introspection-client --all
```

- can be configured to show logging information from shared memory.

> - 可以被配置为显示来自共享存储器的日志记录信息。

- Setting Tracing::Category to _shm_ shows the log related to shared memory, while SharedMemory::LogLevel decides which log level iceoryx shows:

> -将 Tracing:：Category 设置为*shm*显示与共享内存相关的日志，而 SharedMemory:：LogLevel 决定 iceoryx 显示的日志级别：

```xml
<?xml version="1.0" encoding="UTF-8" ?>
<CycloneDDS xmlns="https://cdds.io/config"
            xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
            xsi:schemaLocation="https://cdds.io/config https://raw.githubusercontent.com/eclipse-cyclonedds/cyclonedds/iceoryx/etc/cyclonedds.xsd">
    <Domain id="any">
        <Tracing>
            <Category>shm</Category>
            <OutputFile>stdout</OutputFile>
        </Tracing>
        <SharedMemory>
            <Enable>true</Enable>
            <LogLevel>info</LogLevel>
        </SharedMemory>
    </Domain>
</CycloneDDS>
```
