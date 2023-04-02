# Logging and tracing in Cyclone DDS

A lot of effort has gone into providing as much useful information as possible when a log message is written and a fair number of mechanisms were put in place to be able to do so.

> 在编写日志消息时，我们已经付出了很多努力来提供尽可能多的有用信息，并且已经建立了相当多的机制来做到这一点。

The difficulty with logging in Cyclone DDS is the fact that it is both used by user applications and internal services alike. Both use-cases have different needs with regard to logging. e.g. a service may just want to write all information to a log file, whereas a user application may want to display the message in a graphical user interface or reformat it and write it to stdout.

> 登录 Cyclone DDS 的困难在于，用户应用程序和内部服务都在使用它。两个用例在日志记录方面有不同的需求。例如，服务可能只想将所有信息写入日志文件，而用户应用程序可能想在图形用户界面中显示消息，或者重新格式化消息并将其写入 stdout。

This document does not concern itself with error handling, return codes, etc. Only logging and tracing are covered.

> 本文档不涉及错误处理、返回代码等。只涉及日志记录和跟踪。

## Design

The logging and tracing api offered by Cyclone DDS is a merger of the reporting functionality that existed in the various modules that Cyclone is made out of. Why the API needed to change is documented in the section [Issues with previous implementation].

> Cyclone DDS 提供的日志记录和跟踪 api 是 Cyclone 所包含的各种模块中存在的报告功能的合并。API 需要更改的原因记录在[先前实现的问题]一节中。

The new and improved logging mechanism in Cyclone DDS allows the user to simply register a callback function, as is customary for libraries, that matches the signature [dds_log_write_fn_t]{.title-ref}. The callback can be registered by passing it to [dds_set_log_sink]{.title-ref} or [dds_set_trace_sink]{.title-ref}. The functions also allow for registering a [void *]{.title-ref} that will be passed along on every invocation of the the callback. To unregister a log or trace sink, call the respective function and pass NULL for the callback. This will cause Cyclone DDS to reinstate the default handler. aka stdout, stderr, or the file specified in the configuration file.

> Cyclone DDS 中新的和改进的日志记录机制允许用户简单地注册回调函数，与签名[dds_log_write_fn_t]｛.title-ref｝相匹配。可以通过将回调传递给[dds_set_log_ssink]｛.title-ref｝或[dds_se_trace_link]｛.title-rev｝来注册回调。这些函数还允许注册[void*]｛\title-ref}，该函数将在每次调用回调时传递。要注销日志或跟踪接收器，请调用相应的函数并为回调传递 NULL。这将导致 Cyclone DDS 恢复默认处理程序。也就是 stdout、stderr 或配置文件中指定的文件。

```C
typedef void(*dds_log_write_fn_t)(dds_log_data_t *, void *);

struct dds_log_data {
  uint32_t priority;
  const char *file;
  uint32_t line;
  const char *function;
  const char *message;
};

typedef struct dds_log_data dds_log_data_t;

dds_return_t dds_set_log_sink(dds_log_write_fn_t *, const void *);
dds_return_t dds_set_trace_sink(dds_log_write_fn_t *, const void *);
```

Cyclone DDS offers both a logging and a tracing callback to allow the user to easily send traces to a different device than the log messages. The trace function receives both the log messages and trace messages, while the log function will only receive the log messages.

> Cyclone DDS 提供了日志记录和跟踪回调，允许用户轻松地将跟踪发送到与日志消息不同的设备。跟踪函数同时接收日志消息和跟踪消息，而日志函数将只接收日志消息。

The user is responsible for managing resources himself. Cyclone DDS will not attempt to free [userdata]() under any circumstance. The user should revert to the default handler, or register a different sink before invalidating the userdata pointer.

> **用户负责自己管理资源。Cyclone DDS 在任何情况下都不会试图释放[userdata]（）**。用户应恢复到默认处理程序，或在使用户数据指针无效之前注册其他接收器。

The _[dds_set_log_sink]() and _[dds_set_trace_sink]() functions are synchronous. It is guaranteed that on return no thread in the Cyclone DDS stack will reference the sink or the [userdata]().

> _[dds_set_log_sink]（）和_[dds.set_tracke_link]（）函数是同步的。可以保证在返回时，Cyclone DDS 堆栈中的任何线程都不会引用 sink 或[userdata]（）。

To minimize the amount of information that is outputted when tracing is enabled, the user can enable/disable tracing per category. Actually the priority member that is passed to the handler consists of the priority, e.g. error, info, etc and (if it's a trace message) the category.

> 为了最大限度地减少启用跟踪时输出的信息量，用户可以按类别启用/禁用跟踪。实际上，传递给处理程序的优先级成员包括优先级，例如 error、info 等，以及（如果是跟踪消息）类别。

To be specific. The last four bits of the 32-bit integer contain the priority. The other bits implicitly indicate it's a trace message and are reserved to specify the category to which a message belongs.

> 具体来说。32 位整数的**最后四位包含优先级**。其他位隐式地指示它是一条跟踪消息，并保留用于指定消息所属的类别。

```C
#define DDS_LC_FATAL 1u
#define DDS_LC_ERROR 2u
#define DDS_LC_WARNING 4u
#define DDS_LC_INFO 8u
#define DDS_LC_CONFIG 16u
...
```

DDSI is the Cyclone DDS module that the log and trace mechanism originated from. For that reason not all categories make sense to use in API code and some categories that you would expect may be missing. For now the categories are a work-in-progress and may be changed without prior notice.

> DDSI 是 Cyclone DDS 模块，日志和跟踪机制源自该模块。出于这个原因，并不是所有类别都可以在 API 代码中使用，有些类别可能会丢失。目前，这些类别正在进行中，可能会在没有事先通知的情况下进行更改。

To control which messages are passed to the registered sinks, the user must call [dds_set_log_mask]{.title-ref} or [dds_set_trace_mask]{.title-ref}. Whether or not to call the internal log and trace functions depends on it. The user is strongly discouraged to enable all categories and filter messages in the registered handler, because of the performance impact!

> 要控制将哪些消息传递给已注册的接收器，用户必须调用[dds_set_log_mask]｛.title-ref｝或[dds_se_trace_mask]｛.title-rev｝。是否调用内部日志和跟踪函数取决于此。强烈建议用户启用已注册处理程序中的所有类别并筛选消息，因为这会影响性能！

Tests have shown performance to decrease by roughly 5% if the decision on whether or not to write the message is done outside the function. The reason obviously not being the [if]()-statement, but the creation of the stack frame.

> 测试表明，如果是否编写消息的决定是在函数之外完成的，那么性能将下降大约 5%。原因显然不是[if]（）-语句，而是堆栈框架的创建。

For developer convenience, a couple of macro's are be introduced so the developer does not have to write boilerplate code each time. The implementation will roughly follow what is specified below.

> 为了方便开发人员，引入了几个宏，这样开发人员就不必每次都编写样板代码。实现将大致遵循以下规定。

```C
#define DDS_FATAL(fmt, ...)
#define DDS_ERROR(fmt, ...)
#define DDS_INFO(fmt, ...)
#define DDS_WARNING(fmt, ...)
#define DDS_TRACE(cat, fmt, ...)
```

Log and trace messages are finalized by a newline. If a newline is not present, the buffer is not flushed. This is can be used to extend messages, e.g. to easily append summaries and decisions, and already used throughout the ddsi module. The newline is replaced by a null character when the message is passed to a sink.

> 日志和跟踪消息通过换行完成。如果没有换行符，则不会刷新缓冲区。这可以用于扩展消息，例如，方便地附加摘要和决策，并且已经在整个 ddsi 模块中使用。当消息被传递到接收器时，换行符将被一个 null 字符取代。

### Default handlers and behavior

If the user does not register a sink, messages are printed to the default location. Usually this means a cyclonedds.log file created in the working directory, but the location can be changed in the configuration file. By default only error and warning messages will be printed.

> 如果用户没有注册接收器，消息将打印到默认位置。通常这意味着在工作目录中创建一个 cyclonedds.log 文件，但可以在配置文件中更改位置。默认情况下，只打印错误和警告消息。

As long as no file is open (or e.g. a syslog connection is established), messages are printed to stderr.

> 只要没有打开任何文件（例如，建立了系统日志连接），消息就会打印到 stderr。

For convenience a number of log handlers will ship with Cyclone. Initially the set will consist of a handler that prints to stdout/stderr and one that prints to a file. However, at some point it would be nice to ship handlers that can print to the native log api offered by a target. e.g.

> 为了方便起见，许多原木装卸工将与 Cyclone 一起装运。最初，该集合将由一个打印到 stdout/stderr 的处理程序和一个打印文件的处理程序组成。然而，在某些情况下，最好提供可以打印到目标提供的本地日志 api 的处理程序。例如

- Windows Event Log on Microsoft Windows
- Syslog on Linux
- Unified Logging on macOS
- logMsg or the ED&R (Error Detection and Reporting) subsystem on VxWorks

> - Microsoft Windows 上的 Windows 事件日志
> - Linux 上的 Syslog
> - 统一登录 macOS
> - logMsg 或 VxWorks 上的 ED&R（错误检测和报告）子系统

For now it is unclear what configuration options are available for all the default handlers or how the API to update them will look exactly.

> 目前尚不清楚所有默认处理程序都有哪些配置选项，也不清楚更新它们的 API 会是什么样子。

## Log message guidelines

- Write concise reports.
- Do not leave out information, but also don't turn it into an essay. e.g. a message for a failed write operation could include topic and partition names to indicate the affected system scope.
- Write consistent reports.
- Use the name of the parameter as it appears in the documentation for that language binding to reference a parameter where applicable.
- Use the same formatting style as other messages in the same module.
- e.g. use "could not ..." or "failed to ..." consistently.
- Avoid duplicate reports as much as possible. e.g. if a problem is reported in a lower layer, do not report it again when the error is propagated.
- Discuss with one of the team members if you must deviate from the formatting rules/guidelines and update this page if applicable.
- Report only meaningful events.

> - 写出简明扼要的报告。
> - 不要遗漏信息，但也不要把它变成一篇文章。例如，失败写入操作的消息可以包括主题和分区名称以指示受影响的系统范围。
> - 编写一致的报告。
> - 在适用的情况下，使用该语言绑定文档中显示的参数名称来引用参数。
> - 使用与同一模块中的其他消息相同的格式样式。
> - 例如，始终如一地使用“不能…”或“不能……”。
> - 尽可能避免重复的报告。例如，如果在较低层中报告了问题，则在传播错误时不要再次报告。
> - 与其中一名团队成员讨论是否必须偏离格式规则/准则，并在适用的情况下更新此页面。
> - 只报告有意义的事件。

## Issues with previous implementation

- Multiple files are opened. DDSI, by default, writes to cyclonedds-trace.log, but also contains info, warning and error log messages written by DDSI itself. All other modules write to cyclone-info.log and cyclone-error.log. Options are available, but they're all over the place. Some are adjusted by setting certain environment variables, others are configured by modifying the configuration file. Not exactly what you'd call user friendly. Also, simply writing a bunch of files from a library is not considered good practice.

> -打开了多个文件。默认情况下，DDSI 写入 cycloneds-trace.log，但也包含由 DDSI 本身写入的信息、警告和错误日志消息。所有其他模块都会写入 cyclone-info.log 和 cyclone-error.log。选项是可用的，但它们无处不在。有些是通过设置某些环境变量来调整的，另一些是通过修改配置文件来配置的。并不是你所说的用户友好型。此外，简单地从库中编写一堆文件并不是一种好的做法。

- Cyclone only offers writing to a [FILE]() handle. The filenames mentioned above by default, but a combination of stdout/stderr can also be used. There is no easy way to display them in a GUI or redirect them to a logging API. e.g. syslog on Linux or the Windows Event Log on Microsoft Windows.

> -Cyclone 只提供对[FILE]（）句柄的写入。默认情况下，上面提到的文件名，但也可以使用 stdout/stderr 的组合。没有简单的方法可以在 GUI 中显示它们或将它们重定向到日志 API。例如，Linux 上的 syslog 或 Microsoft Windows 上的 Windows 事件日志。

- The report stack is a cumbersome mechanism to use and (to a certain extend) error prone. Every function (at least every function in the "public" APIs) must start with a report stack instruction and end with a report flush instruction.

> -报告堆栈是一种使用起来很麻烦的机制，而且（在一定程度上）容易出错。每个函数（至少是“公共”API 中的每个函数）都必须以报表堆栈指令开始，以报表刷新指令结束。

- Cyclone assumes files can always be written. For a number of supported targets, e.g. FreeRTOS and VxWorks, this is often not the case. Also, filling the memory with log files is probably undesirable.

> -Cyclone 假设文件总是可以写入的。对于许多受支持的目标，例如 FreeRTOS 和 VxWorks，通常情况并非如此。此外，用日志文件填充内存可能是不可取的。

- Cyclone (except for DDSI) does not support log categories to selectively enable/disable messages that the user is interested in. Causing trace logs to contain (possibly) too much information.

> -Cyclone（DDSI 除外）不支持日志类别来选择性地启用/禁用用户感兴趣的消息。导致跟踪日志包含（可能）太多信息。

- The report mechanism expects an error code, but it's unclear what "type" of code it is. It can be a valid errno value, os_result, utility result, DDS return code, etc. While the problem can be fixed by adding a module field to the log message struct. The message is often generated in the layer above the layer that the error originated from. Apart from that, the report callback is not the place to handle errors gracefully. Return codes and exceptions are the mechanism to do that.

> -报告机制需要一个错误代码，但尚不清楚它是什么“类型”的代码。它可以是有效的 errno 值、os_result、实用程序结果、DDS 返回代码等。但可以通过在日志消息结构中添加模块字段来解决此问题。消息通常是在错误产生的层之上的层中生成的。除此之外，报表回调并不是优雅地处理错误的地方。返回代码和异常就是这样做的机制。

- The logging API is different across modules. DDSI uses the ddsi_log macro family, the abstraction layer uses the OS_REPORT family and DDS uses the dds_report family. Apart from the macros used, the information that would end up in the report callback is also different.

> -不同模块的日志 API 不同。DDSI 使用 DDSI.log 宏族，抽象层使用 OS_REPORT 族，DDS 使用 DDS_REPORT 族。除了使用的宏之外，最终会出现在报表回调中的信息也有所不同。
