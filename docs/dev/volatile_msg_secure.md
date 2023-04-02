# ParticipantVolatileMessageSecure Handling

## Short Introduction

It is expected to have some knowledge of DDSI builtin (security) endpoints.

> 预计它对 DDSI 内置（安全）端点有一定的了解。

```cpp
#define DDSI_ENTITYID_P2P_BUILTIN_PARTICIPANT_VOLATILE_SECURE_WRITER 0xff0202c3
#define DDSI_ENTITYID_P2P_BUILTIN_PARTICIPANT_VOLATILE_SECURE_READER 0xff0202c4
```

These builtin endpoints have caused about the biggest code change in ddsi, regarding security.

> 这些内置端点导致了 ddsi 中关于安全性的最大代码更改。

Chapters 7.4.4.3 and 7.4.4.4 in the DDS Security specification indicates the main issue why these builtin endpoints are different from all the others and somewhat more complex.

> DDS 安全规范中的第 7.4.4.3 和 7.4.4.4 章指出了为什么这些内置端点与所有其他端点不同并且更复杂的主要问题。

> 7.4.4.3 Contents of the ParticipantVolatileMessageSecure
> The ParticipantVolatileMessageSecure is intended as a holder of secure information that
> is sent point-to-point from a DomainParticipant to another.
>
> [...]
>
> 7.4.4.4 Destination of the ParticipantVolatileMessageSecure
>
> If the destination_participant_guid member is not set to GUID_UNKNOWN, the message written is
> intended only for the BuiltinParticipantVolatileMessageSecureReader belonging to the
> DomainParticipant with a matching Participant Key.
>
> This is equivalent to saying that the BuiltinParticipantVolatileMessageSecureReader has an implied
> content filter with the logical expression:
>
> “destination_participant_guid == GUID_UNKNOWN
> || destination_participant_guid==BuiltinParticipantVolatileMessageSecureReader.participant.guid”
>
> Implementations of the specification can use this content filter or some other mechanism as long as the
> resulting behavior is equivalent to having this filter.
>
> [...]

The "point-to-point" and "content filter" remarks makes everything more elaborate.

> “点对点”和“内容过滤器”的备注使一切都变得更加精细。

## Complexity

It would be nice to be able to use the `dds_set_topic_filter()` functionality for these endpoints. However, that only works on the reader history cache (rhc), which is only available for ddsc entities and not for ddsi builtin entities. And it's the builtin entities that are being used.

> 如果能够为这些端点使用“dds_set_topic_filter（）”功能，那就太好了。然而，这只适用于读取器历史缓存（rhc），它只适用于 ddsc 实体，而不适用于 ddsi 内置实体。而正在使用的是内置实体。

The `dds_set_topic_filter()` basically simulates that the sample was inserted into the rhc (but didn't insert it), which causes the rest of ddsi (regarding heartbeat, acknacks, gaps, etc) to work as normal while the sample just isn't provided to the reader.

> “dds_set_topic_filter（）”基本上模拟了样本被插入到 rhc 中（但没有插入），这导致 ddsi 的其余部分（关于心跳、确认、间隙等）正常工作，而样本只是没有提供给读取器。

Unfortunately, the builtin volatile endpoints can not use that same simple sequence (just handle the sample but ignore it right at the end). Problem is, the sample is encoded. It can only decode samples that are intended for that reader. This would mean that it is best for the reader to only receive 'owned' samples that it can actually decode.

> 不幸的是，内置的 volatile 端点不能使用相同的简单序列（只处理样本，但在最后忽略它）。问题是，样本已编码。它只能解码为该阅读器准备的样本。这意味着阅读器最好只接收它可以实际解码的“自有”样本。

This has all kinds of affects regarding the heartbeat, acknacks, gaps, etc. Basically, every writer/reader combination should have information regarding gaps and sequence numbers between them, while normally such information about proxies are combined.

> 这会对心跳、确认、间隙等产生各种影响。基本上，每个写入器/读取器组合都应该有关于它们之间的间隙和序列号的信息，而通常会组合这些关于代理的信息。

## Implementation Overview

This only depicts an overview. Some details will have been omitted.

> 这只是一个概述。一些细节将被省略。

### Writing

The function `write_crypto_exchange_message()` takes care of generating the right sample information and pass it on to `ddsi_write_sample_p2p_wrlock_held()`.

> 函数“write_crypto_exchange_message（）”负责生成正确的样本信息，并将其传递给“ddsi_write_sample_p2p_wrlock_held（）”。

A proxy reader can now have a filter callback function (`proxy_reader::filter`). This indicates (on the writer side) if a sample will be accepted by the actual reader or not. This could be made more generic for proper 'writer side' content filter implementation. However, now it'll only be used by ParticipantVolatileMessageSecure and the filter is hardcoded to `ddsi_volatile_secure_data_filter()`.

> 代理读取器现在可以具有筛选器回调函数（`proxy_rereader:：filter`）。这表明（在编写方）实际读者是否会接受样本。对于适当的“编写器端”内容筛选器实现，可以使其更通用。然而，现在它将只由 ParticipantVolatileMessageSecure 使用，并且过滤器被硬编码为`ddsi_vivale_secure_data_filter（）`。

So, if `ddsi_write_sample_p2p_wrlock_held()` is called with a proxy reader with a filter, it will get 'send/acked sequences' information between the writer and proxy reader. This is used to determine if gap information has to be send alongside the sample.

> 因此，如果使用带有筛选器的代理读取器调用“ddsi_write_sample_p2p_wrlock_held（）”，它将在写入器和代理读取器之间获得“发送/确认序列”信息。这用于确定间隙信息是否必须与样本一起发送。

Then, `ddsi_write_sample_p2p_wrlock_held()` will enqueue the sample.

> 然后，`ddsi_write_sample_p2p_wrlock_held（）`将对样本进行排队。

Just before the submessage is added to the rtps message and send, it is encoded (todo).

> 就在子消息被添加到 rtps 消息并发送之前，它被编码（todo）。

### Reading

First things first, the submessage is decoded when the rtps message is received (todo).

> 首先，当接收到 rtps 消息时，子消息被解码（todo）。

It is received on a builtin reader, so the builtin queue is used and `ddsi_builtins_dqueue_handler()` is called. That will forward the sample to the token exchange functionality, ignoring every sample that isn't related to the related participant (todo).

> 它是在内置读卡器上接收的，因此使用内置队列，并调用“ddsi_buitins_dqueue_handler（）”。这将把样本转发到令牌交换功能，忽略与相关参与者（todo）无关的每个样本。

### Gaps on reader side

The reader remembers the last_seq it knows from every connected proxy writer (`pwr_rd_match::last_seq`).

> 读者会记住它从每个连接的代理写入程序中知道的 last_seq（`pwr_rd_match:：last_seq`）。

This is updated when handling heartbeats, gaps and regular messages and used to check if there are gaps.

> 这在处理心跳、间隙和常规消息时会更新，并用于检查是否存在间隙。

Normally, the `last_seq` of a specific writer is used here. But when the reader knows that the writer uses a 'writer side content filter' (`proxy_writer::uses_filter`), it'll use the the `last_seq` that is related to the actual reader/writer match.

> 通常，此处使用特定编写器的“last_seq”。但是，当读者知道编写器使用了“编写器端内容过滤器”（“proxy_writer:：uses_filter”）时，它将使用与实际的读取器/写入器匹配相关的“last_seq”。

It is used to generate the AckNack (which contains gap information) response to the writer.

> 它用于生成对写入程序的 AckNack（包含间隙信息）响应。

### Gaps on writer side

The writer remembers which sample sequence it send the last to a specific reader through `wr_prd_match::lst_seq`.

> 编写器会记住最后一个通过`wr_prd_match:：lst_seq`发送给特定读取器的样本序列。

This is used to determine if a reader has received all relevant samples (through handling of acknack).

> 这用于确定读取器是否已接收到所有相关样本（通过处理 acknack）。

It is also used to determine the gap information that is added to samples to a specific reader when necessary.

> 它还用于确定在必要时添加到特定读取器的样本中的间隙信息。

### Heartbeats

A writer is triggered to send heartbeats once in a while. Normally, that is broadcasted. But, for the volatile secure writer, it has to be send to each reader specifically. The heartbeat submessage that is send to each reader individually is encoded with a reader specific key. This key is generated from the shared secret which was determined during the authentication phase.

> 写入程序会被触发，每隔一段时间发送一次心跳。通常情况下，这是广播的。但是，对于易失性安全写入程序，必须将其专门发送给每个读取器。单独发送给每个读卡器的心跳子消息使用读卡器特定密钥进行编码。该密钥是根据在身份验证阶段确定的共享秘密生成的。

When a writer should send heartbeats, `handle_xevk_heartbeat()` is called. For the volatile secure writer, the control is immediately submitted to `send_heartbeat_to_all_readers()`. This will add heartbeat submessages to an rtps message for every reader it deems necessary.

> 当编写器应该发送检测信号时，会调用“handle_xevk_cheartbeat（）”。对于 volatile 安全写入程序，控件会立即提交到“send_heartbeat_to_all_readers（）”。这将为它认为必要的每个阅读器添加心跳子消息到 rtps 消息中。

### Reorder

Normally received samples are placed in the reorder administration of the proxy_writer. However in this case the writer applies a content filter which is specific for each destinated reader. In that case the common reorder administration in the proxy_writer can not be used and the reader specific reorder administration must be used to handle the gap's which will be reader specific.

> 通常接收的样本被放置在 proxy_writer 的重新排序管理中。然而，在这种情况下，作者会应用特定于每个目标读者的内容过滤器。在这种情况下，不能使用 proxy_writer 中的通用重新排序管理，必须使用特定于读取器的重新排序管理来处理间隙，这将是特定于读取器。

### Trying to put the security participant volatile endpoint implementation into context.

The following elements are added to the data structures:

> 以下元素被添加到数据结构中：

- struct ddsi_wr_prd_match::lst_seq : Highest seq send to this reader used when filter is applied
- struct ddsi_pwr_rd_match::last_seq : Reader specific last received sequence number from the writer.
- struct ddsi_proxy_writer::uses_filter : Indicates that a content-filter is active
- struct ddsi_proxy_reader::filter : The filter to apply for this specific reader

> - struct ddsi_wr_prd_match:：lst_seq：应用筛选器时使用的发送到此读取器的最高 seq
> - struct ddsi_pwr_rd_match:：last_seq：读卡器特定的从编写器接收的最后一个序列号。
> - struct ddsi_proxy_writer:：uses_filter：表示内容筛选器处于活动状态
> - struct ddsi_proxy_reader:：filter：要应用于此特定读卡器的筛选器

Functions added:

- writer_hbcontrol_p2p : This function creates a heartbeat destined for a specific reader. The volatile secure writer will use an submessage encoding which uses a distinct key for each reader. Therefor a reader specific heartbeat is needed.
- ddsi_defrag_prune : When a volatile secure reader is deleted then the defragmentation administration could still contain messages destined for this reader. This function removes these messages from the defragmentation administration.
- ddsi_volatile_secure_data_filter : The filter applied to the secure volatile messages which filters on the destination participant guid.
- ddsi_write_sample_p2p_wrlock_held : This function writes a message to a particular reader.

> - writer_hbcontrol_p2p：此函数创建一个指定给特定读取器的心跳。易失性安全写入程序将使用子消息编码，该编码为每个读取器使用不同的密钥。因此，需要一个特定于读取器的心跳。
> - ddsi_defrag_prune：当一个易失性安全读卡器被删除时，碎片整理管理仍然可能包含指向该读卡器的消息。此功能可从碎片整理管理中删除这些消息。
> - ddsi_voile_secure_data_filter：应用于安全易失性消息的筛选器，该筛选器根据目标参与者 guid 进行筛选。
> - ddsi_write_sample_p2p_wrlock_held：此函数将消息写入特定的读取器。

The use of the content-filter for the volatile secure writer implies that for each destination reader which message from the writer history cache is valid and had to be sent.

> 对易失性安全写入程序使用内容过滤器意味着，对于每个目标读取器，写入程序历史缓存中的哪条消息是有效的，必须发送。

For messages that do not match this filter a GAP message should be sent to the reader. Each time a message is sent to a specific reader a possible gap message is added.

> 对于与此筛选器不匹配的消息，应向读取器发送 GAP 消息。每次向特定阅读器发送消息时，都会添加一条可能的间隙消息。

For the volatile secure writer the sequence number of the last message send to a particular reader is maintained in `wr_prd_match::lst_seq'''. It is used to determine if a

> . /对于易失性安全写入程序，发送到特定读卡器的最后一条消息的序列号保持在“wr_prd_match:：lst_seq”中。它用于确定

HEARTBEAT has to send to this particular reader or that the reader has acknowledged all messages. At the reader side the sequence number of the last received message is

> 心跳必须发送给这个特定的阅读器，或者阅读器已经确认了所有消息。在读卡器端，最后接收到的消息的序列号为

maintained in `pwr_rd_match::last_seq'''. It is used to determine the contents of the ACKNACK message as response to a received HEARTBEAT.

> 在`pwr_rd_match:：last_seq''中维护。它用于确定作为对接收到的心跳的响应的 ACKNACK 消息的内容。

When an ACKNACK (handle_AckNack) is received it is determined which samples should be resent related to the applied filter and for which sequence numbers a GAP message should be sent.

> 当接收到 ACKNACK（handle_ACKNACK）时，确定应当重新发送与所应用的滤波器相关的哪些样本以及应当针对哪些序列号发送 GAP 消息。
