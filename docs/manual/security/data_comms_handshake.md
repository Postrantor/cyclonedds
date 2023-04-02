# Data communication and handshake process

An authentication handshake between participants starts after participant discovery. If a reader and a writer are created during that period, their match is delayed until after the handshake succeeds.

> 参与者之间的身份验证握手在发现参与者之后开始。如果在这段时间内创建了读取器和写入器，那么它们的匹配将延迟到握手成功之后。

::: warning
::: title
Warning
:::

During the handshake process, volatile data is lost (as if there is no reader).

> 在握手过程中，易失性数据会丢失（就好像没有读取器一样）。

After publication match, the encryption / decryption keys are exchanged between the reader and writer. Best-effort data that is sent during this exchange is lost. Reliable data is resent.

> 在发布匹配之后，在读取器和写入器之间交换加密/解密密钥。在此交换过程中发送的尽力而为数据将丢失。重新发送可靠的数据。
