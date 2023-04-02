#include "HelloWorldData.h"
#include "dds/dds.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
  dds_entity_t participant;
  dds_entity_t topic;
  dds_entity_t writer;
  dds_return_t rc;
  HelloWorldData_Msg msg;
  uint32_t status = 0;
  (void)argc;
  (void)argv;

  /* Create a Participant. */
  participant = dds_create_participant(DDS_DOMAIN_DEFAULT, NULL, NULL);
  if (participant < 0)
    DDS_FATAL("dds_create_participant: %s\n", dds_strretcode(-participant));

  /* Create a Topic. */
  topic = dds_create_topic(
      participant, &HelloWorldData_Msg_desc, "HelloWorldData_Msg", NULL, NULL);
  if (topic < 0)
    DDS_FATAL("dds_create_topic: %s\n", dds_strretcode(-topic));

  /* Create a Writer. */
  writer = dds_create_writer(participant, topic, NULL, NULL);
  if (writer < 0)
    DDS_FATAL("dds_create_writer: %s\n", dds_strretcode(-writer));

  printf("=== [Publisher]  Waiting for a reader to be discovered ...\n");
  fflush(stdout);

  rc = dds_set_status_mask(writer, DDS_PUBLICATION_MATCHED_STATUS);
  if (rc != DDS_RETCODE_OK)
    DDS_FATAL("dds_set_status_mask: %s\n", dds_strretcode(-rc));

  while (!(status & DDS_PUBLICATION_MATCHED_STATUS))
  {
    rc = dds_get_status_changes(writer, &status);
    if (rc != DDS_RETCODE_OK)
      DDS_FATAL("dds_get_status_changes: %s\n", dds_strretcode(-rc));

    /* Polling sleep. */
    // At regular intervals we get the status change and a matching publication. In between, the writing thread sleeps for a time period equal `DDS_MSECS` (in milliseconds).
    // 每隔一段时间，我们就会得到状态更改和匹配的发布。在此期间，写入线程睡眠的时间段等于`DDS_MSECS`(以毫秒为单位)。
    dds_sleepfor(DDS_MSECS(20));
  }

  /* `getting_started.md:1043`
    > [!NOTE]: 如何避免 “脏写入”
    > 这段代码是针对 Cyclone DDS 编写的。Cyclone DDS 是一种用于实现分布式数据交换的软件组件，主要应用于需要高可靠性和低延迟的系统中。本段代码所述的功能是在分享相同数据类型和主题名称的读者和写者之间建立连接，而无需应用程序的介入。这也就是 DDS 自动发现机制的基本思想。
    >
    > 为了在仅在出现数据读取器时写入数据，需要使用一种称为“rendezvous”模式的模式。该模式可以通过下面两种方式之一来实现：
    >
    > 等待发布/订阅匹配事件，其中发布者等待并阻塞写线程，直到引发适当的发布匹配事件，或
    > 定期轮询发布匹配状态。这是我们在此示例中实现的首选方法。
    > 下面这行代码告诉 Cyclone DDS 监听 DDS_PUBLICATION_MATCHED_STATUS 事件：
    >
    > 这意味着当数据写者与相应的数据读者相匹配时，即可使用数据写入操作。这样**可以确保数据在可用时才写入，同时避免了不必要的资源浪费**。
    > 脏写入指的是在数据发布者和订阅者之间发生了一些不同步的情况，导致接收方读取到了不正确或过时的数据。例如，如果一个写入操作完成得比预期慢，而此时另一个读取操作正在进行，则读取操作可能会检索到上一个写操作留下的旧数据，而不是最新数据。这种情况下，如果应用程序没有处理这种不同步，就会出现脏写入问题。
    >
    > 在对数据进行发布和订阅时，可以通过使用特定模式（如 Rendezvous 模式）以及相应的状态事件来避免脏写入。在 Cyclone DDS 中，当发现具有相同数据类型和主题名称的读者和写者时，它们将被自动连接，但为了确保在可靠的方式下进行数据传输，需要通过监听 DDS_PUBLICATION_MATCHED_STATUS 状态事件，以便确定何时开始发送数据。这样，写入操作只会发生在该数据读取器出现并已准备好接收数据的情况下，从而避免了脏写入的问题。
  */

  /* Create a message to write. */
  msg.userID = 1;
  msg.message = "Hello World";

  printf("=== [Publisher]  Writing : ");
  printf("Message (%" PRId32 ", %s)\n", msg.userID, msg.message);
  fflush(stdout);

  // After this loop, we are sure that a matching reader has been discovered. Now, we commence the writing of the data instance. First, the data must be created and initialized
  // > 在这个循环之后，我们确信已经发现了一个匹配的读取器。现在，我们开始编写数据实例。首先，必须创建并初始化数据
  rc = dds_write(writer, &msg);
  if (rc != DDS_RETCODE_OK)
    DDS_FATAL("dds_write: %s\n", dds_strretcode(-rc));

  /* Deleting the participant will delete all its children recursively as well. */
  rc = dds_delete(participant);
  if (rc != DDS_RETCODE_OK)
    DDS_FATAL("dds_delete: %s\n", dds_strretcode(-rc));

  return EXIT_SUCCESS;
}
