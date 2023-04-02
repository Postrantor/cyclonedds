The DCPS specification provides _transient-local_, _transient_, and _persistent_ data. The DDSI specification only provides _transient-local_, which is the only form of durable data available when inter-operating across vendors.

In DDSI, transient-local data is implemented using the `Writer History Cache` (WHC) that is normally used for reliable communication. For transient-local data, samples are retained even when all Readers have acknowledged them. The default history setting of `KEEP_LAST` with `history_depth = 1`, means that late-joining Readers can still obtain the latest sample for each existing instance.

When the DCPS Writer is deleted (or is unavailable), the DDSI Writer and its history are also lost. For this reason, transient data is typically preferred over transient-local data.

Note

has a facility for retrieving transient data from a suitably configured OpenSplice node, but does not include a native service for managing transient data.

> DCPS 规范提供*transient-local*、_transient_ 和*persistent* 数据。 DDSI 规范仅提供*transient-local*，这是跨供应商互操作时唯一可用的持久数据形式。
>
> 在 DDSI 中，临时本地数据是使用通常用于可靠通信的“编写器历史缓存”(WHC) 实现的。 **对于瞬时本地数据，即使所有读者都已确认样本，样本也会保留**。 `KEEP_LAST` 的默认历史记录设置为 `history_depth = 1`，这意味着后期加入的 Reader 仍然可以获得每个现有实例的最新样本。
>
> 当 DCPS 写入器被删除（或不可用）时，DDSI 写入器及其历史记录也会丢失。 出于这个原因，瞬态数据通常优于瞬态本地数据。
>
> 具有从适当配置的 OpenSplice 节点检索瞬态数据的工具，但不包括用于管理瞬态数据的本机服务。
