# FreeRTOS

[FreeRTOS][1] is real-time operating system kernel for embedded devices. Think of it as a thread library rather than a general purpose operating system like Linux or Microsoft Windows. Out-of-the-box, FreeRTOS provides support for tasks (threads), mutexes, semaphores and software times. Third-party modules are available to add features. e.g. [lwIP][2] can be used to add networking.

> [FreeRTOS][1]是嵌入式设备的实时操作系统内核。把它**看作是一个线程库**，而不是像 Linux 或 Microsoft Windows 这样的通用操作系统。开箱即用的 FreeRTOS 提供了对任务(线程)、互斥、信号量和软件时间的支持。第三方模块可用于添加功能。例如[lwIP][2]可以用于添加网络。

> FreeRTOS+lwIP is currently supported by Eclipse Cyclone DDS. Support for other network stacks, e.g. [FreeRTOS+TCP][3], may be added in the future.

> Eclipse Cyclone DDS does not make use of [FreeRTOS+POSIX][4] because it was not available at the time. Future versions of Eclipse Cyclone DDS may or may not require FreeRTOS+POSIX for compatibility when it becomes stable.

[1]: https://www.freertos.org/
[2]: https://savannah.nongnu.org/projects/lwip/
[3]: https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/index.html
[4]: https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_POSIX/index.html

## Target

FreeRTOS provides an operating system kernel. Batteries are not included. i.e. no C library or device drivers. Third-party distributions, known as board support packages (BSP), for various (hardware) platforms are available though.

> FreeRTOS 提供了一个操作系统内核。不包括电池。即没有 C 库或设备驱动程序。第三方发行版，即板支持包(BSP)，可用于各种(硬件)平台。

Board support packages, apart from FreeRTOS, contain:

- C library. Often ships with the compiler toolchain, e.g. [IAR Embedded Workbench][5] includes the DLIB runtime, but open source libraries can also be used. e.g. The [Xilinx Software Development Kit][6] includes newlib.
- Device drivers. Generally available from the hardware vendor, e.g. NXP or Xilinx. Device drivers for extra components, like a real-time clock, must also be included in the board support package.

> - C 库。通常随编译器工具链一起提供，例如[IAR Embedded Workbench][5]包括 DLIB 运行时，但也可以使用开源库。例如，[Silinx 软件开发工具包][6]包括 newlib。
> - 设备驱动程序。通常可从硬件供应商处获得，例如 NXP 或 Xilinx。用于额外组件(如实时时钟)的设备驱动程序也必须包含在板支持包中。

[5]: https://www.iar.com/iar-embedded-workbench/
[6]: https://www.xilinx.com/products/design-tools/embedded-software/sdk.html

A board support package is linked with the application by the toolchain to generate a binary that can be flashed to the target.

> 板支持包通过工具链与应用程序链接，以生成可以闪存到目标的二进制文件。

### Requirements

Eclipse Cyclone DDS requires certain compile-time options to be enabled in FreeRTOS ([FreeRTOSConfig.h]{.title-ref}) and lwIP ([lwipopts.h]{.title-ref}) for correct operation. The compiler will croak when a required compile-time option is not enabled.

> Eclipse Cyclone DDS 要求在 FreeRTOS([FreeRTOSConfig.h]｛.title-ref｝)和 lwIP([lwipopts.h]｛.title-rev｝)中启用某些编译时选项以进行正确操作。当未启用所需的编译时选项时，编译器将发出嘎嘎声。

Apart from the aforementioned compile-time options, the target and toolchain must provide the following.

> 除了上述编译时选项外，目标和工具链必须提供以下内容。

- Support for thread-local storage (TLS) from the compiler and linker.
- Berkeley socket API compatible socket interface.

> - 编译器和链接器支持线程本地存储(TLS)。
> - Berkeley 套接字 API 兼容的套接字接口。

- Real-time clock (RTC). A high-precision real-time clock is preferred, but the monotonic clock can be combined with an offset obtained from e.g. the network if the target lacks an actual real-time clock. A proper [clock_gettime]{.title-ref} implementation is required to retrieve the wall clock time.

> - 实时时钟(RTC)。高精度实时时钟是优选的，但是**如果目标缺乏实际实时时钟，则单调时钟可以与从例如网络获得的偏移相组合**。需要一个正确的[clock_gettime]{.title-ref}实现来检索墙上的时钟时间。

### Thread-local storage

FreeRTOS tasks are not threads and compiler supported thread-local storage (tls) might not work as desired/expected under FreeRTOS on embedded targets. i.e. the address of a given variable defined with [thread]() may be the same for different tasks.

> FreeRTOS 任务不是线程，编译器支持的线程本地存储(tl)在嵌入式目标上的 FreeRTOS 下可能无法按要求/预期工作。即，用[thread]()定义的给定变量的地址对于不同的任务可能是相同的。

The compiler generates code to retrieve a unique address per thread when it encounters a variable defined with **[thread](). What code it generates depends on the compiler and the target it generates the code for. e.g. [iccarm.exe]{.title-ref} that comes with IAR Embedded Workbench requires [**aeabi_read_tp]{.title-ref} to be implemented and [mb-gcc]{.title-ref} that comes with the Xilinx SDK requires [__tls_get_addr]{.title-ref} to be implemented.

> 当编译器遇到用[thread]()定义的变量时，它会生成代码来检索每个线程的唯一地址。它生成的代码取决于编译器和生成代码的目标。例如，IAR Embedded Workbench 附带的[icham.exe]｛.title-ref｝需要实现[aeabi_read_tp]｛.title-ref}，Xilinx SDK 附带的[mb-gcc]｛\title-ref｝则需要实现[__tls_get_addr]｛title-ref}。

The implementation for each of these functions is more-or-less the same. Generally speaking they require the number of bytes to allocate, call [pvTaskGetThreadLocalStoragePointer]{.title-ref} and return the address of the memory block to the caller.

> 这些功能中的每一个的实现或多或少是相同的。一般来说，它们需要分配的字节数，调用[pvTaskGetThreadLocalStoragePointer]｛.title-ref｝并将内存块的地址返回给调用方。

## Simulator

FreeRTOS ports for Windows and POSIX exist to test compatibility. How to cross-compile Eclipse Cyclone DDS for the [FreeRTOS Windows Port][7] or the unofficial [FreeRTOS POSIX Port][8] can be found in the msvc and [posix](/ports/freertos-posix) port directories.

> 用于 Windows 和 POSIX 的 FreeRTOS 端口用于测试兼容性。可以在 msvc 和[POSIX](/ports/FreeRTOS-POSIX)端口目录中找到如何为[FreeRTOS Windows 端口][7]或非官方[FreeRTOS-POSIX 端口][8]交叉编译 Eclipse Cyclone DDS。

[7]: https://www.freertos.org/FreeRTOS-Windows-Simulator-Emulator-for-Visual-Studio-and-Eclipse-MingW.html
[8]: https://github.com/shlinym/FreeRTOS-Sim.git

## Known Limitations

Triggering the socket waitset is not (yet) implemented for FreeRTOS+lwIP. This introduces issues in scenarios where it is required.

> FreeRTOS+lwIP 尚未(尚未)实现触发套接字等待集。这在需要的场景中引入了问题。

- Receive threads require a trigger to shutdown or a thread may block indefinitely if no packet arrives from the network.
- Sockets are created dynamically if ManySocketsMode is used and a participant is created, or TCP is used. A trigger is issued after the sockets are added to the set if I/O multiplexing logic does not automatically wait for data on the newly created sockets as well.

> - **接收线程需要一个触发器来关闭**，或者如果没有来自网络的数据包到达，**线程可能会无限期地阻塞**。
> - 如果使用 ManySocketsMode 并创建参与者，或者使用 TCP，则套接字将动态创建。如果 I/O 多路复用逻辑没有自动等待新创建的套接字上的数据，则在将套接字添加到集合后会发出触发器。
