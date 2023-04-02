# Loan mechanism {#loan_machanism}

When using shared memory exchange, additional performance gains can be made by using the loan mechanism on the writer side. The loan mechanism directly allocates memory from the shared memory pool, and provides this to the user as a message data type. This eliminates a copy step in the publication process.

> 当使用共享内存交换时，可以通过在写入端使用贷款机制来获得额外的性能提升。借出机制直接从共享内存池中分配内存，并将其作为消息数据类型提供给用户。这样就省去了发布过程中的复制步骤。

```c
struct message_type *loaned_sample;
dds_return_t status = dds_loan_sample (writer, (void**)&loaned_sample);
```

If _status_ returns :c`DDS_RETCODE_OK`{.interpreted-text role="macro"}, then _loaned_sample_ contains a pointer to the memory pool object, in all other cases, _loaned_sample_ should not be dereferenced.

> 如果*status*返回：c`DDS_RETCODE_OK`｛.respered text role=“macro”｝，则*loaned_sample*包含指向内存池对象的指针，在所有其他情况下，*loaned-sample*都不应被取消引用。

Necessary information about the data type is supplied by the writer. When requesting loaned samples, the writer used to request the loaned sample must be the same data type as the sample that you are writing in it.

> 有关数据类型的必要信息由编写器提供。在请求借出样本时，用于请求借出样本的写入程序必须与您在其中写入的样本具有相同的数据类型。

When requesting loaned samples, the maximum number of outstanding loans is defined by **MAX_PUB_LOANS** (default set to 8). This is the maximum number of loaned samples that each publisher can have outstanding from the shared memory. If the maximum is reached, to request new loaned samples, some must be returned (handed back to the publisher) through :c`dds_write()`{.interpreted-text role="func"}.

> 请求借出样本时，未偿还贷款的最大数量由**MAX_PUB_loans**定义（默认设置为 8）。这是每个发布者可以从共享内存中获得的最大借出样本数。如果达到最大值，则为了请求新的借出样本，必须通过：c`dds_write（）`{.depredicted text role=“func”}返回一些样本（交还给发布者）。

::: note
::: title
Note
:::

When a loaned sample has been returned to the shared memory pool (by invoking :c`dds_write()`{.interpreted-text role="func"}), dereferencing the pointer is undefined behaviour.

> 当一个借出的样本被返回到共享内存池时（通过调用：c`dds_write（）`{.depreted text role=“func”}），取消引用指针是未定义的行为。
> :::

If is configured to use shared memory, but it is not possible to use the loan mechanism, a :c`dds_write()`{.interpreted-text role="func"} still writes to the shared memory service. This increases the overhead by an additional copy step in publication. That is, when a block for publishing to the shared memory is requested, the data of the published sample is copied into it.

> 如果配置为使用共享内存，但不可能使用借出机制，则 a:c`dds_write（）`{.depreted text role=“func”}仍会写入共享内存服务。这增加了发布中额外的复制步骤的开销。也就是说，当请求发布到共享内存的块时，已发布样本的数据会复制到其中。
