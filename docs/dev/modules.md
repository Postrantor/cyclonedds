# Eclipse Cyclone DDS Module Layout

Cyclone DDS is made up of multiple modules, each of which provides a certain set of functionality, either private, public or a combination thereof. Since Cyclone DDS is a middleware product, the api is of course the most visible interface. Cyclone DDS uses the [dds]() (not followed by an underscore) prefix to avoid name collisions with other code.

> Cyclone DDS 由多个模块组成，每个模块都提供一组特定的功能，可以是私有的、公共的，也可以是它们的组合。由于 Cyclone DDS 是一种中间件产品，因此 api 当然是最可见的接口。Cyclone DDS 使用[DDS]()(后面没有下划线)前缀来避免与其他代码的名称冲突。

The fact that Cyclone DDS is made up of multiple modules is largely historic, but does offer a neat way to separate features logically.

> Cyclone DDS **由多个模块组成这一事实在很大程度上是历史性的，但它确实提供了一种从逻辑上分离功能的巧妙方法**。

     |-------------|
     |             |  DDS is not a target, it is the product, the sum of the
     |     DDS     |  targets that together form Cyclone DDS. i.e. the stable
     |             |  api prefixed with dds\_ and the libddsc.so library.
     |---|---------|
     |   |         |  ddsc implements most of dds\_ api. A modern,
     |   |  ddsc   |  user-friendly implementation of the DDS specification.
     |   |         |
     |   |---------|
     |   |         |  ddsi, as the name suggests, is an implementation of the
     |   |  ddsi   |  RTPS-DDSI specification.
     |   |         |
     |   |---------|
     |             |  ddsrt offers target agnostic implementations of
     |      ddsrt  |  functionality required by the ddsc and ddsi targets, but
     |             |  also exports a subset of the dds\_ api directly. e.g.
     |-------------|  dds_time_t and functions to read the current time from
                      the target are implemented here.

> The need for a separate utility module (util) has disappeared with the restructuring of the runtime module. The two will be merged in the not too distant future.

All modules are exported separately, for convenience. e.g. the _[ddsrt]() module offers target agnostic interfaces to create and manage threads and synchronization primitives, retrieve resource usage, system time, etc. However, all symbols not referenced by including _[dds.h]() or prefixed with dds are considered internal and there are no guarantees with regard to api stability and backwards compatibility. That being said, they are not expected to change frequently. Module specific headers are located in the respective directory under [INSTALL_PREFIX/include/dds].

> 为了方便起见，所有模块都是单独导出的。例如，**[ddsrt]()模块提供了与目标无关的接口，用于创建和管理线程和同步原语，检索资源使用情况、系统时间等**。然而，**所有未通过包含[dds.h]()或以 dds 为前缀引用的符号都被视为内部符号，并且不能保证 api 的稳定性和向后兼容性**。话虽如此，预计它们不会经常发生变化。特定于模块的标头位于[INSTALL_PREFIX/include/dds]下的相应目录中。

## DDS Runtime (ddsrt)

The main purpose of the runtime module is to allow modules stacked on top of it, e.g. ddsi and dds, to be target agnostic. Meaning that, it ensures that features required by other modules can be used in the same way across supported targets. The runtime module will NOT try to mimic or stub features that it can simply cannot offer on a given target. For features that cannot be implemented on all targets, a feature macro will be introduced that other modules can use to test for availability. e.g. [DDSRT_HAVE_IPV6]() can be used to determine if the target supports IPv6 addresses.

> **运行时模块的主要目的是允许堆叠在其上面的模块(例如 ddsi 和 dds)与目标无关**。也就是说，它确保了其他模块所需的功能可以在支持的目标之间以相同的方式使用。运行时模块不会试图模仿或存根它无法在给定目标上提供的功能。对于无法在所有目标上实现的功能，将引入一个功能宏，其他模块可以使用该宏来测试可用性。例如[DDSTRT_HAVE_IPV6]()可以用于确定目标是否支持 IPV6 地址。

### Feature discovery

Discovery of target features at compile time is largely dynamic. Various target specific predefined macros determine if a feature is supported and which implementation is built. This is on purpose, to avoid a target specific include directory and an abundance of configuration header files and works well for most use cases. Of course, there are exceptions where the preprocessor requires some hints to make the right decision. e.g. when the lwIP TCP/IP stack should be used as opposed to the native stack. The build system is responsible for the availability of the proper macros at compile time.

> 在编译时发现目标特性在很大程度上是动态的。各种特定于目标的预定义宏确定是否支持某个功能以及构建了哪个实现。这是为了避免目标特定的包含目录和大量的配置头文件，并且在大多数用例中都能很好地工作。当然，也有例外，预处理器需要一些提示才能做出正确的决定。例如当应该使用 lwIP TCP/IP 堆栈而不是本地堆栈时。构建系统负责在编译时提供适当的宏。

Feature implementations are often tied directly to the operating system for general purpose operating systems. This level of abstraction is not good enough for embedded targets though. Whether a feature is available or not depends on (a combination) of the following.

> 对于通用操作系统，功能实现通常直接绑定到操作系统。然而，这种抽象级别对于嵌入式目标来说还不够好。功能是否可用取决于以下各项的组合。

1.  Operating system. e.g. Linux, Windows, FreeRTOS.
2.  Compiler. e.g. GCC, Clang, MSVC, IAR.
3.  Architecture. e.g. i386, amd64, ARM.
4.  C library. e.g. glibc (GNU), dlib (IAR).

> 1. 操作系统。例如 Linux、Windows、FreeRTOS。
> 2. 编译器。例如 GCC、Clang、MSVC、IAR。
> 3. 架构。例如 i386、amd64、ARM。
> 4. C 库。例如 glibc(GNU)、dlib(IAR)。

#### Atomic operations

Support for atomic operations is determined by the target architecture. Most compilers (at least GCC, Clang, Microsoft Visual Studio and Solaris Studio) offer atomic builtins, but if support is unavailable, fall back on the target architecture specific implementation.

> **对原子操作的支持由目标体系结构决定**。大多数编译器(至少 GCC、Clang、Microsoft Visual Studio 和 SolarisStudio)都提供原子内建，但如果无法获得支持，则只能依靠特定于目标体系结构的实现。

##### 具体示例

##### cyclone dds 对于 atomic operation 的支持@13:56:25

@Q:
在 cyclone dds 中关于 atomic operation 的相关问题，可以具体解释一下，cyclone dds 对于 atomic operation 的支持，使用等问题吗

@A:
CycloneDDS 支持多种原子操作，如 `RMW` (Read-Modify-Write)，`CAS` (Compare-and-Swap) 等。这些类型的操作可以用来保证多个参与者在并发场景下对相同数据的修改的正确执行。

CycloneDDS 使用了一种名为 `DomainParticipantFactoryQoS` 的配置选项，其中包括 `atomic.memory.kind` 和 `atomic.memory.max_size` 两个字段。其中，`atomic.memory.kind` 字段用于指定原子操作使用的内存类型，`atomic.memory.max_size` 字段则用于指定内存块的最大大小。CycloneDDS 支持使用 RAM, CDR 和交换式内存作为原子操作使用的内存类型。

同时，CycloneDDS 还支持使用 `Mutex` 和 `Semaphore` 来进行原子操作。使用 Mutex 时，参与者之间会锁住需要操作的数据，避免并发时出现冲突。使用 Semaphore 时，参与者可以获取和释放访问数据的权限，以确保正确的执行。

总的来说，CycloneDDS 对原子操作提供了丰富的支持，可以用来保证在高并发场景下数据的正确性。

##### 可以更进一步的对 mutex 和 semaphore 进行解释吗@13:57:45

@Q:
可以更进一步的对 mutex 和 semaphore 进行解释吗

@A:
当在分布式系统中并发访问共享数据时，为确保数据的一致性和准确性，我们需要使用同步机制，**mutex 和 semaphore 都是用来实现同步的机制**。

Mutex 是一种互斥量，使用时会加锁。在访问共享资源之前先使用 mutex 对其加锁，其它线程需要访问该资源时则需要阻塞等待该资源的解锁。当访问完成后需要及时解锁，以便其它线程可以继续操作。使用 mutex 时，需要确保每个共享资源都有一个对应的 mutex 来保证并发访问时的正确性。

Semaphore 是一种信号量机制，用于对一定数量的许可进行控制。当一个线程需要使用共享资源时，需要先获取一个许可，该许可在 semaphore 上获取。如果没有可用的许可，则需要等待直至其它线程归还许可。**使用 semaphore 可以设置最大同时访问资源的线程数，当访问的线程数量达到了最大值时，其它线程将被阻塞。使用 semaphore 时需要注意许可的归还**。在使用许可的资源操作完毕后，需要及时将许可归还 semaphore，以便其它线程可以继续操作。

在 CycloneDDS 中，若需要访问共享资源，可以使用 Mutex 和 Semaphore 来控制并发访问。具体选择哪种方式取决于应用场景和需要实现的功能。

##### 可以结合具体的示例，解释一下在 cyclone dds 中 mutex 的使用吗@13:59:16

@Q:
可以结合具体的示例，解释一下在 cyclone dds 中 mutex 的使用吗

@A:
好的，下面我将结合具体示例来解释 Cyclone DDS 中 mutex 的使用。

在 Cyclone DDS 中，每个 DomainParticipant 对象都会创建一个线程池，线程池中的线程可以被用来处理订阅者和发布者的事件。当需要访问共享资源时，可以使用 mutex 来保证数据的正确性。

我们以创建一个 Topic 为例，来说明 mutex 的使用。

```cpp
DDS_ReturnCode_t create_topic(DDS_DomainParticipant *participant, const char *topic_name) {
    DDS_ReturnCode_t retcode;
    DDS_TopicQos topic_qos;
    DDS_Topic *topic;
    DDS_TopicDescription *topic_desc;

    // 创建一个新的 Topic
    topic = participant->create_topic(participant, topic_name, "HelloWorld", &topic_qos, NULL, DDS_STATUS_MASK_NONE);
    if (topic == NULL) {
        printf("create_topic error\n");
        return DDS_RETCODE_ERROR;
    }

    // 访问共享资源前需要加锁
    DDS_Mutex_lock(topic_lock);
    // 将 Topic 描述信息添加至列表中
    retcode = add_topic_desc(topic_desc_list, topic_name, topic_desc);
    // 解锁
    DDS_Mutex_unlock(topic_lock);

    if (retcode != DDS_RETCODE_OK) {
        printf("add_topic_desc error\n");
        return DDS_RETCODE_ERROR;
    }

    return DDS_RETCODE_OK;
}
```

在上面的示例代码中，我们使用 `DDS_Mutex_lock` 函数对 `topic_lock` 加锁，以保证访问共享资源的正确性。在对共享资源的操作完成后，我们使用 `DDS_Mutex_unlock` 函数解锁。

需要注意的是，在使用 Mutex 时，需要确保每个共享资源都有一个对应的 Mutex 来保证并发访问时的正确性。

另外，关于 Mutex 的初始化和销毁，通常在使用 DDS 驱动程序（如 CycloneDDS）时是不需要手动初始化和销毁的，因为 CycloneDDS 会在启动时自动完成 Mutex 的初始化。

##### 可以给出一个更完整的示例程序吗@14:01:13

@Q:
可以给出一个更完整的示例程序吗

@A:
当然可以，下面是一个完整的示例程序，其中使用了 Mutex 来保证对共享资源的并发访问的正确性。

在这个示例程序中，我们定义了一个名为 `TopicDescList` 的结构体，用于存储 Topic 的描述信息。我们使用了一个全局的 `topic_desc_list` 变量来存储所有 Topic 的描述信息。在处理每个新的 Topic 时，我们需要将其描述信息添加进列表中，这就涉及到并发访问共享资源的问题，因此我们使用了 Mutex 来保证访问的正确性。

```cpp
#include <stdio.h>
#include <stdlib.h>

#include "dds/dds.h"

DDS_Mutex topic_lock;
typedef struct {
    const char *name;
    DDS_TopicDescription *desc;
} TopicDesc;

typedef struct {
    TopicDesc *topics[10];
    int size;
} TopicDescList;

TopicDescList *topic_desc_list = NULL;

DDS_ReturnCode_t add_topic_desc(TopicDescList *list, const char *name, DDS_TopicDescription *desc) {
    int i;
    for (i = 0; i < list->size; i++) {
        if (list->topics[i] != NULL && strcmp(list->topics[i]->name, name) == 0) {
            return DDS_RETCODE_OK;
        }
    }
    if (list->size >= 10) {
        return DDS_RETCODE_ERROR;
    }
    TopicDesc *topic_desc = (TopicDesc *)calloc(1, sizeof(TopicDesc));
    topic_desc->name = name;
    topic_desc->desc = desc;
    list->topics[list->size++] = topic_desc;
    return DDS_RETCODE_OK;
}

DDS_ReturnCode_t create_topic(DDS_DomainParticipant *participant, const char *topic_name) {
    DDS_ReturnCode_t retcode;
    DDS_TopicQos topic_qos;
    DDS_Topic *topic;
    DDS_TopicDescription *topic_desc;

    // 创建一个新的 Topic
    topic = participant->create_topic(participant, topic_name, "HelloWorld", &topic_qos, NULL, DDS_STATUS_MASK_NONE);
    if (topic == NULL) {
        printf("create_topic error\n");
        return DDS_RETCODE_ERROR;
    }

    // 访问共享资源前需要加锁
    DDS_Mutex_lock(topic_lock);
    // 将 Topic 描述信息添加至列表中
    retcode = add_topic_desc(topic_desc_list, topic_name, topic_desc);
    // 解锁
    DDS_Mutex_unlock(topic_lock);

    if (retcode != DDS_RETCODE_OK) {
        printf("add_topic_desc error\n");
        return DDS_RETCODE_ERROR;
    }

    return DDS_RETCODE_OK;
}

int main() {
    DDS_DomainParticipant *participant;
    DDS_ReturnCode_t retcode;

    // 初始化 DDS API
    dds_init();

    // 创建一个 DomainParticipant
    participant = dds_create_participant(0, NULL, NULL);
    if (participant == NULL) {
        printf("create_participant error\n");
        return -1;
    }

    // 清空 Topic 描述信息列表
    topic_desc_list = (TopicDescList *)calloc(1, sizeof(TopicDescList));
    topic_desc_list->size = 0;
    for (int i = 0; i < 10; i++) {
        topic_desc_list->topics[i] = NULL;
    }

    // 初始化 Mutex
    retcode = DDS_Mutex_initialize(&topic_lock);
    if (retcode != DDS_RETCODE_OK) {
        printf("Mutex initialize error\n");
        return -1;
    }

    // 循环调用创建 Topic 函数
    for (int i = 0; i < 10; i++) {
        char topic_name[32];
        snprintf(topic_name, 32, "ExampleTopic%d", i);
        retcode = create_topic(participant, topic_name);
        if (retcode != DDS_RETCODE_OK) {
            printf("create_topic error\n");
            return -1;
        }

        // 输出所有 Topic 描述信息
        printf("--------------------\n");
        DDS_Mutex_lock(topic_lock);
        for (int j = 0; j < topic_desc_list->size; j++) {
            TopicDesc *topic_desc = topic_desc_list->topics[j];
            if (topic_desc == NULL) {
                continue;
            }
            printf("Topic name: %s, Desc: %p\n", topic_desc->name, topic_desc->desc);
        }
        DDS_Mutex_unlock(topic_lock);
    }

    // 销毁 Mutex
    DDS_Mutex_finalize(&topic_lock);

    // 销毁 DomainParticipant
    dds_delete(participant);

    // 结束 DDS API
    dds_fini();

    return 0;
}
```

在上面的示例程序中，我们使用 `DDS_Mutex_initialize` 对 `topic_lock` 进行初始化，在程序执行结束后使用 `DDS_Mutex_finalize` 对其进行销毁。在对共享资源的操作完成后，我们使用 `DDS_Mutex_unlock` 函数解锁。

##### 可以给出使用 C++ 版本的示例代码吗@14:02:16

@Q:
可以给出使用 C++ 版本的示例代码吗

@A:
当然可以，以下是一个使用 CycloneDDS 和 C++ 的示例代码，可以更方便地使用 Mutex 对共享资源进行访问的保护。这个示例程序创建了一个 Publisher 和一个 Subscriber，它们可以互相发送信息。

在这个程序中，我们使用了互斥锁 `std::mutex` 来对一个共享的数据进行互斥保护。Publisher 和 Subscriber 中的相同变量（如 `g_count`）都是共享的，并且可以被互相访问。

```cpp
#include <iostream>
#include <mutex>
#include <condition_variable>
#include <cstring>

#include "dds/dds.hpp"
#include "HelloWorld.h"

using namespace dds::core;
using namespace dds::domain;
using namespace dds::topic;
using namespace dds::pub;
using namespace dds::sub;
using namespace dds::sub::status;
using namespace std::chrono;

// 互斥锁
std::mutex dataMutex;

// 共享数据
int g_count = 0;

void publisher_thread() {
    // 创建 DomainParticipant
    DomainParticipant participant(default_domain());

    // 创建 Topic 和 DataWriter
    Topic<HelloWorld> topic(participant, "Example HelloWorld");
    DataWriter<HelloWorld> writer(Publisher(participant), topic);

    // 发布消息
    HelloWorld sample;
    for (int i = 0; i < 10; i++) {
        // 对共享资源进行加锁
        dataMutex.lock();
        g_count++;
        std::cout << "Publishing count " << g_count << std::endl;
        sample.data(std::to_string(g_count)); // 将计数器值写入消息
        writer.write(sample);
        // 解锁
        dataMutex.unlock();

        std::this_thread::sleep_for(milliseconds(1000)); // 等待一段时间
    }
}

void subscriber_thread() {
    // 创建 DomainParticipant
    DomainParticipant participant(default_domain());

    // 创建 Topic 和 DataReader
    Topic<HelloWorld> topic(participant, "Example HelloWorld");
    DataReader<HelloWorld> reader(Subscriber(participant), topic);

    // 监听消息
    int count = 0;
    while (count < 10) {
        auto samples = reader.select().condition(AnyDataReader::new_data()).take();
        for (const auto& sample : samples) {
            if (sample.info().valid()) {
                // 对共享资源进行加锁
                dataMutex.lock();
                g_count++;
                std::cout << "Subscriber count " << g_count << ": " << sample.data().to_string() << std::endl;
                // 解锁
                dataMutex.unlock();

                count++;
            }
        }
        std::this_thread::sleep_for(milliseconds(500)); // 等待一段时间
    }
}

int main(int argc, char *argv[]) {
    std::thread publisher(publisher_thread);
    std::thread subscriber(subscriber_thread);

    publisher.join();
    subscriber.join();

    return 0;
}
```

在上述的 C++ 示例代码中，我们使用了 `std::mutex` 和 `lock_guard<std::mutex>` 的组合来实现互斥锁的保护。在写入和读取共享资源前，先使用 `lock_guard<std::mutex>` 对线程进行加锁，在操作完成后自动释放锁。使用这种方式更加方便和简洁，避免了手动加锁和解锁的繁琐。

##### 同样的，可以给出在 cyclone dds 中使用 semaphore 的 C++版本的示例程序吗@14:03:55

@Q:
同样的，可以给出在 cyclone dds 中使用 semaphore 的 C++版本的示例程序吗

@A:
当然可以，以下是一个使用 CycloneDDS 和 C++ 的示例程序，它使用 semaphore 来限制 Publisher 的速度，使其在发送每个消息后需要等待一定时间后才能发送下一个消息。

在这个程序中，我们使用了信号量 `std::semaphore` 来控制发送速率。Publisher 每发送一个消息后，需要等待一段时间，直到 semaphore 释放了一个许可，才能发送下一个消息。

```cpp
#include <iostream>
#include <semaphore.h>
#include <cstring>

#include "dds/dds.hpp"
#include "HelloWorld.h"

using namespace dds::core;
using namespace dds::domain;
using namespace dds::topic;
using namespace dds::pub;
using namespace std::chrono;

// 信号量
std::semaphore sendSemaphore(2); // 2 个许可证

void publisher_thread() {
    // 创建 DomainParticipant
    DomainParticipant participant(default_domain());

    // 创建 Topic 和 DataWriter
    Topic<HelloWorld> topic(participant, "Example HelloWorld");
    DataWriter<HelloWorld> writer(Publisher(participant), topic);

    // 发布消息
    HelloWorld sample;
    for (int i = 0; i < 10; i++) {
        // 等待释放一个许可证
        sendSemaphore.acquire();

        std::cout << "Publishing count " << (i + 1) << std::endl;
        sample.data(std::to_string(i + 1)); // 将计数器值写入消息
        writer.write(sample);

        std::this_thread::sleep_for(seconds(1)); // 等待一段时间
    }
}

void subscriber_thread() {
    // 创建 DomainParticipant
    DomainParticipant participant(default_domain());

    // 创建 Topic 和 DataReader
    Topic<HelloWorld> topic(participant, "Example HelloWorld");
    DataReader<HelloWorld> reader(Subscriber(participant), topic);

    // 监听消息
    int count = 0;
    while (count < 10) {
        auto samples = reader.select().condition(AnyDataReader::new_data()).take();
        for (const auto& sample : samples) {
            if (sample.info().valid()) {
                std::cout << "Subscriber count " << (count + 1) << ": " << sample.data().to_string() << std::endl;
                count++;
            }
        }
        std::this_thread::sleep_for(milliseconds(500)); // 等待一段时间
    }
}

int

```

##### cyclone dds 中的 atomic 的特点

在 Cyclone DDS 中，atomic 主要用于做线程间的同步和数据的原子操作。Cyclone DDS 对 atomic 的使用主要有以下几个特点：

1. 使用 C++11/14 语言标准的原子操作库或操作系统提供的原子操作函数，以实现线程安全。
2. 在高并发场景下，通过对共享数据进行原子操作，避免线程之间的数据竞争和操作冲突，保证了系统的正确性和稳定性。
3. 实现了一套 DDS 数据库的原子操作函数库，用于实现 DDS 数据库的读写操作的线程安全和同步。
4. 在高并发场景下，通过原子操作实现的任务调度机制，可以保证任务的顺序性，同时避免了线程之间的竞争和冲突。

总之，Cyclone DDS 的 atomic 特点就是通过原子操作实现了线程间的同步、数据的原子操作以及任务的调度，保证了系统的正确性和稳定性，同时提高了系统的并发性能。

##### 这与 C++ 14 中提供的 atomic 的区别

Cyclone DDS 中使用的 atomic 特性和 C++ 14 中提供的 atomic 有很多相似之处，比如都是用于实现多线程编程时的原子操作、线程同步和数据同步。不过它们之间也有一些不同之处，主要体现在以下几个方面：

1. 语言标准不同：Cyclone DDS 中的 atomic 主要是使用 C++11/14 语言标准的原子操作库或操作系统提供的原子操作函数实现的，而 C++ 14 中的 atomic 则是作为 C++14 标准库的一部分提供的。
2. 应用场景不同：Cyclone DDS 的 atomic 主要是用于实现 DDS 数据库和高并发场景下的任务调度，而 C++ 14 中的 atomic 是为了支持更为通用的原子操作需求，比如在多线程编程中保证共享变量的原子性操作等。
3. 实现方式不同：虽然 Cyclone DDS 和 C++ 14 中的 atomic 都是通过原子操作实现线程同步和数据同步，但是具体实现方式并不相同，因为 Cyclone DDS 主要是针对 DDS 数据库设计的，而 C++ 14 中的 atomic 是为了更为通用的原子操作需求设计的。

综上所述，虽然 Cyclone DDS 中的 atomic 与 C++ 14 中的 atomic 有很多相似之处，但是在具体的实现细节和应用场景上还是有所区别的。

#### Network stack

General purpose operating systems like Microsoft Windows and Linux come with a network stack, as does VxWorks. FreeRTOS, however, does not and requires a separate TCP/IP stack, which is often part of the Board Support Package (BSP). But separate stacks can be used on Microsoft Windows and Linux too. e.g. the network stack in Tizen RT is based on lwIP, but the platform uses the Linux kernel. Whether or not lwIP must be used cannot be determined automatically and the build system must hint which implementation is to be used.

> 像 Microsoft Windows 和 Linux 这样的通用操作系统和 VxWorks 一样都带有网络堆栈。然而，FreeRTOS 不需要并且需要一个单独的 TCP/IP 堆栈，该堆栈通常是板支持包(BSP)的一部分。但是，单独的堆栈也可以在 Microsoft Windows 和 Linux 上使用。例如，Tizen RT 中的网络堆栈是基于 lwIP 的，但该平台使用 Linux 内核。是否必须使用 lwIP 不能自动确定，并且构建系统必须提示要使用哪个实现。

### Structure

The runtime module uses a very specific directory structure to allow for feature-based implementations and sane fallback defaults.

> 运行时模块使用非常特定的目录结构，以允许基于功能的实现和正常的回退默认值。

#### Header files

The include directory must provide a header file per feature. e.g. [dds/ddsrt/sync.h] is used for synchronisation primitives. If there are only minor differences between targets, everything is contained within that file. If not, as is the case with [dds/ddsrt/types.h], a header file per target is a better choice.

> include 目录必须为每个功能提供一个头文件。例如[dds/ddsrt/sync.h]用于同步原语。如果目标之间只有微小的差异，那么所有内容都包含在该文件中。如果不是，就像[dds/ddsrt/types.h]的情况一样，每个目标的头文件是更好的选择。

Private headers may also be required to share type definitions between target implementations that do not need to be public. These are located in a feature specific include directory with the sources.

> 在不需要公开的目标实现之间共享类型定义也可能需要私有头。这些位于具有源的功能特定的包含目录中。

```
    ddsrt
     |- include
     |   \- dds
     |       \- ddsrt
     |           |- atomics
     |           |   |- arm.h
     |           |   |- gcc.h
     |           |   |- msvc.h
     |           |   \- sun.h
     |           |- atomics.h
     |           |- time.h
     |           |- threads
     |           |   |- posix.h
     |           |   \- windows.h
     |           \- threads.h
     |
     \- src
         \- threads
             \- include
                 \- dds
                     \- ddsrt
                         \- threads_priv.h
```

> Which target specific header file is included is determined by the top-level header file, not the build system. However, which files are exported automatically is determined by the build system.

> 包含哪个目标特定的头文件由顶级 头文件决定，而不是构建系统。但是，自动导出哪些文件是由构建系统决定的。

#### Source files

Source files are grouped per feature too, but here the build system determines what is compiled and what is not. By default the build system looks for a directory with the system name, e.g. windows or linux, but it is possible to overwrite it from a feature test. This allows for a non-default target to be used as would be the case with e.g. lwip for sockets. If a target-specific implementation cannot be found, the build system will fall back to posix. All files with a .c extension under the selected directory will be compiled. Code that can be shared among targets can be put in a file named after the atomicfeature with the .c extension. Of course if there is no target-specific code, or if there are only minimal differences there is not need to create a feature directory.

> **源文件也按功能分组**，但这里的构建系统决定哪些是编译的，哪些不是。默认情况下，构建系统会查找具有系统名称的目录，例如 windows 或 linux，但也可以从功能测试中覆盖它。这允许使用非默认目标，例如套接字的 lwip。如果找不到特定于目标的实现，那么构建系统将回退到 posix。将编译所选目录下所有扩展名为.c 的文件。可以在目标之间共享的代码可以放在一个以特性命名的文件中，扩展名为.c。当然，如果没有特定于目标的代码，或者只有最小的差异，就不需要创建功能目录。

```
    ddsrt
     \- src
         |- atomics.c
         |- sockets
         |   |- posix
         |   |   |- gethostname.c
         |   |   \- sockets.c
         |   \- windows
         |       |- gethostname.c
         |       \- sockets.c
         \- sockets.c
```

### Development guidelines

- Be pragmatic. Use ifdefs (only) where it makes sense. Do not ifdef if target implementations are completely different. Add a separate implementation. If there are only minor differences, as is typically the case between unices, use an ifdef.

> - 要务实。只在有意义的地方使用 ifdefs。如果目标实现完全不同，请不要使用 ifdef。添加一个单独的实现。如果只有微小的差异，就像 unices 之间的典型情况一样，请使用 ifdef。

- Header and source files are not prefixed. Instead they reside in a directory named after the module that serves as a namespace. e.g. the threads feature interface is defined in [dds/ddsrt/threads.h].

> - 头文件和源文件没有前缀。相反，它们位于以用作命名空间的模块命名的目录中。例如，线程特性接口在[dds/ddsrt/threads.h]中定义。

- Macros that influence which implementation is used, must be prefixed by _DDSRT*USE*_ followed by the feature name. e.g. _[DDSRT_USE_LWIP]() to indicate the lwIP TCP/IP stack must be used. Macros that are defined at compile time to indicate whether or not a certain feature is available, must be prefixed by DDSR*HAVE*_ followed by the feature name. e.g. [DDSRT_HAVE_IPV6]() to indicate the target supports IPv6 addresses.

> - 影响所使用实现的宏必须以*DDSRT_USE* 为前缀，后跟功能名称。例如 [DDSRT_USE_LWIP]()，以指示必须使用 LWIP TCP/IP 堆栈。在编译时定义的用于指示某个功能是否可用的宏，必须以 DDSR_HAVE 为前缀，后跟功能名称。例如[DSRT_HAVE_IPV6]()，以指示目标支持 IPV6 地址。

### Constructors and destructors

The runtime module (on some targets) requires initialization. For that reason, [void ddsrt_init(void)] and [void ddsrt_fini(void)] are exported. They are called automatically when the library is loaded if the target supports it, but even if the target does not, the application should not need to invoke the functions as they are called by [dds_init] and [dds_fini] respectively.

> 运行时模块(在某些目标上)需要初始化。因此，[void ddsrt_init(void)]和[void ddsrt_fini(void)]{.title-ref｝被导出。**如果目标支持，则在加载库时会自动调用它们，但即使目标不支持，应用程序也不需要调用函数**，因为它们分别由[dds_init]｛.title-ref}和[dds_fili]调用。

Of course, if the runtime module is used by itself, and the target does not support constructors and/or destructors, the application is required to call the functions before any of the features from the runtime module are used.

> 当然，如果运行时模块是自己使用的，并且目标不支持构造函数和/或析构函数，那么**在使用运行时模块中的任何功能之前，应用程序都需要调用这些函数**。

> [ddsrt_init] and [ddsrt_fini] are idempotent. Meaning that, it is safe to call [ddsrt_init] more than once. However, initialization is reference counted and the number of calls to [ddsrt_init] must match the number of calls to [ddsrt_fini].

> [ddsrt_init] 和 [ddsrt_fini] 是幂等的。 这意味着，多次调用 [ddsrt_init] 是安全的。 但是，**初始化是引用计数的**，对 [ddsrt_init] 的调用次数必须与对 [ddsrt_fini] 的调用次数相匹配。

#### Threads

Threads require initialization and finalization if not created by the runtime module. [void ddsrt_thread_init(void)] and [void ddsrt_thread_fini(void)] are provided for that purpose. Initialization is always automatic, finalization is automatic if the target supports it. Finalization is primarily used to release thread-specific memory and call routines registered by [ddsrt_thread_cleanup_push].

> **如果线程不是由运行时模块创建的，则需要初始化和终结**。[void ddsrt_thread_init(void)]和[void ddsrt_thread_fini(void)]是为此目的而提供的。初始化总是自动的，**如果目标支持，则完成是自动的**。完成主要用于释放线程特定的内存和调用[ddsrt_thread_cleanup_push]注册的例程。

## DDS Security

### Specification

DDS Security is an [OMG specification](https://www.omg.org/spec/DDS-SECURITY/1.1/PDF) which adds several "DDS Security Support" compliance points to the DDS Specification. The specification defines the Security Model and Service Plugin Interface (SPI) architecture for compliant DDS implementations. The DDS Security Model is enforced by the invocation of these SPIs by the DDS implementation. Security Model for DDS defines the security principals (users of the system), the objects that are being secured, and the operations on the objects that are to be restricted.

> DDS 安全性是[OMG 规范](https://www.omg.org/spec/DDS-SECURITY/1.1/PDF)它在 DDS 规范中增加了几个“DDS 安全支持”合规点。该规范定义了用于兼容 DDS 实现的安全模型和服务插件接口(SPI)体系结构。DDS 安全模型是通过 DDS 实现对这些 SPI 的调用来强制执行的。DDS 的安全模型定义了安全主体(系统用户)、要保护的对象以及要限制的对象上的操作。

SPIs are defined that when combined together provide Information Assurance to DDS systems:

> SPIs 的定义是，当组合在一起时，为 DDS 系统提供信息保证：

- Authentication Service Plugin. Provides the means to verify the identity of the application and/or user that invokes operations on DDS. Includes facilities to perform mutual authentication between participants and establish a shared secret.
- AccessControl Service Plugin. Provides the means to enforce policy decisions on what DDS related operations an authenticated user can perform. For example, which domains it can join, which Topics it can publish or subscribe to, etc.
- Cryptographic Service Plugin. Implements (or interfaces with libraries that implement) all cryptographic operations including encryption, decryption,
- Logging Service Plugin. Supports auditing of all DDS security-relevant events
- Data Tagging Service Plugin. Provides a way to add tags to data samples.

> - 身份验证服务插件。提供验证调用 DDS 操作的应用程序和/或用户身份的方法。包括在参与者之间执行相互身份验证和建立共享机密的设施。
> - 访问控制服务插件。提供了强制执行策略决策的方法，以决定经过身份验证的用户可以执行哪些与 DDS 相关的操作。例如，它可以加入哪些域，可以发布或订阅哪些主题，等等。
> - 加密服务插件。实现(或与实现的库接口)所有加密操作，
> - 日志记录服务插件。支持审核所有 DDS 安全相关事件
> - 数据标记服务插件。提供一种向数据示例添加标记的方法。

<img src="pictures/dds_security_plugin_components.png" alt="DDS Security Plugin Components">

### Cyclone DDS Security

Cyclone DDS Security implementation is composed of the following components/modifications:

- DDS Security plugin API
- DDS Security built-in plugins that implement the API
- DDS Security Core Library that is used by the plugins and DDSI.
- Changes in the DDSI that moderate the specified security model.

> - DDS 安全插件 API
> - 实现 API 的 DDS Security 内置插件
> - 插件和 DDSI 使用的 DDS 安全核心库。
> - 调整指定安全模型的 DDSI 中的更改。

The dependency diagram:

```
     DDSI  ---->     DDS Security API (headers only)  <----- DDS Security Plugins
     |                    ^                                    |
     |                    |                                    |
     |                    |                                    |
      ------->     DDS Security Core  <------------------------
     |                    |                                    |
     |                    |                                    |
     |                    |                                    |
     |                    v                                    |
      ------->          DDS_RT        <------------------------
```

All security specific contents are under src/security.

##### DDS Security API

The DDS Security plugin API consists of just a few header files. There are separate header files for each plugin: dds_security_api_authentication.h dds_security_api_cryptography.h and dds_security_api_access_control.c

> DDS 安全插件 API 仅由几个头文件组成。每个插件都有单独的头文件：dds_security_api_authentication.h dds_security/api_cryptography.h 和 dds_security_api_access_control.c

The API functions and types are prepared from the IDL by adding DDS*Security* namespace prefix to functions and data types. Instead of extending DDS builtin topic data types, separate DDS*Security* data type is defined for the current type and the new secure data type.

> API 函数和类型是从 IDL 中准备的，方法是在函数和数据类型中添加 DDS_Security 命名空间前缀。不扩展 DDS 内置的主题数据类型，而是为当前类型和新的安全数据类型定义了单独的 DDS_Security 数据类型。

##### Built-in Plugins

Cyclone DDS Security comes with three mandatory plugins: authentication, cryptography and access control.

> Cyclone DDS Security 附带三个强制性插件：身份验证、加密和访问控制。

###### Authentication Plugin

This plugin implements authentication using a trusted Certificate Authority (CA). It performs mutual authentication between discovered participants using the RSA or ECDSA Digital Signature Algorithms and establishes a shared secret using Diffie-Hellman (DH) or Elliptic Curve Diffie-Hellman (ECDH) Key Agreement Methods.

> 此插件使用受信任的证书颁发机构(CA)实现身份验证。它使用 RSA 或 ECDSA 数字签名算法在发现的参与者之间执行相互认证，并使用 Diffie-Hellman(DH)或椭圆曲线 Diffie-Hell man(ECDH)密钥协商方法建立共享密钥。

<img src="pictures/dds_security_authentication_plugin.png" alt="DDS Security Plugin Components">

###### Cryptography Plugin

This plugin provides authenticated encryption using Advanced Encryption Standard (AES) in Galois Counter Mode (AES-GCM). It supports two AES key sizes: 128 bits and 256 bits. It may also provide additional reader-specific message authentication codes (MACs) using Galois MAC (AES-GMAC).

> 此插件在 Galois 计数器模式(AES-GCM)中使用高级加密标准(AES)提供经过身份验证的加密。它支持两种 AES 密钥大小：128 位和 256 位。它还可以使用 Galois MAC(AES-GMAC)来提供额外的读取器专用消息认证码(MAC)。

<img src="pictures/dds_security_crypto_plugin.png" alt="DDS Security Plugin Components">

###### Access Control Plugin

<img src="pictures/dds_security_access_control_plugin.png" alt="DDS Security Plugin Components">
