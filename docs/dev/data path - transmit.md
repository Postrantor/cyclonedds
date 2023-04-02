"writer" is a DDSI writer, which is a proxy for a local DCPS writer; "proxy reader" is a proxy for a remote DDSI reader.

---

The local delivery path is essentially the same for local writers & for proxy writers

---

serdata is CDR encoded data and a separate copy of its key (except for discovery message where it uses the DDSI "parameter-list" encoding).

> serdata 是 CDR 编码数据和其密钥的单独副本（发现消息除外，它使用 DDSI“参数列表”编码）。

---

ack info: highest sequence number ack'd so far; writer can drop non-TL msgs from the WHC with sequence numbers less than the minimum over all matched readers; unreliable readers have it fixed at MAX.

> ack info: 迄今为止确认的最高序列号； 编写器可以从 WHC 中删除序列号小于所有匹配读取器的最小值的非 TL 消息； 不可靠的读者将其固定为 MAX。

---

writer history indexed by sequence number (seqidx; for retransmitting on request) and by key (tlidx)—key used only for retaining/dropping data when new data comes in

> 按序列号（seqidx；用于根据请求重新传输）和键 (tlidx) 索引的编写器历史记录——键仅用于在新数据进入时保留/删除数据

---

an xmsg for transmitting a sample references a serdata along with a base+length pair; large samples are fragmented "on-the-fly", each fragment references a different range of the CDR encoded data.

> 用于传输样本的 xmsg 引用 serdata 以及碱基+长度对； 大样本被“即时”分割，每个片段引用不同范围的 CDR 编码数据。

---

an xmsg either references the destination address set of a writer, or a locator of a proxy-reader.

---

timed events: xmsgs for delayed transmission (delay can be 0), heartbeat, ack generation, automatic liveliness "participant message" generation, SPDP generation, &c.

> 定时事件：用于延迟传输的 xmsgs（延迟可以为 0）、心跳、ack 生成、自动活跃度“参与者消息”生成、SPDP 生成等。

---

xpacks bundle xmsgs into DDSI messages, copying some things and leaving others as references; xmsgs included in an xpack are explicitly tracked.

> xpacks 将 xmsgs 捆绑到 DDSI 消息中，复制一些东西并留下其他作为参考； xpack 中包含的 xmsgs 被显式跟踪。
