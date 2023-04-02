---
title: Python Tutorial
---

Let\'s enter the world of DDS by making our presence known. We will not worry about configuration or what DDS does under the hood but write a single message. To publish anything to DDS we need to define the type of message first. Suppose you are worried about talking to other applications that are not necessarily running Python. In that case, you will use the IDL compiler, but for now, we will manually define our message type directly in Python using the `cyclonedds.idl` tools:

> 让我们通过展示我们的存在来进入 DDS 的世界。 我们不会担心配置或 DDS 在幕后做了什么，而是写一条消息。 要向 DDS 发布任何内容，我们需要首先定义消息的类型。 假设您担心与不一定运行 Python 的其他应用程序通信。 在这种情况下，您将使用 IDL 编译器，但现在，我们将使用 `cyclonedds.idl` 工具直接在 Python 中手动定义我们的消息类型：

```{.python3 linenos=""}
from dataclasses import dataclass
from cyclonedds.idl import IdlStruct

@dataclass
class Message(IdlStruct):
    text: str


name = input("What is your name? ")
message = Message(text=f"{name} has started his first DDS Python application!")
```

With `cyclonedds.idl` write typed classes with the standard library module [dataclasses \<python:dataclasses\>]{.title-ref}. For this simple application, the data being transmitted only contains a piece of text, but this system has the same expressive power as the OMG IDL specification, allowing you to use almost any complex datastructure.

> 使用 `cyclonedds.idl` 使用标准库模块 [dataclasses \<python:dataclasses\>]{.title-ref} 编写类型化类。 对于这个简单的应用程序，传输的数据只包含一段文本，但这个系统具有与 OMG IDL 规范相同的表达能力，允许您使用几乎任何复杂的数据结构。

To send your message over a DDS domain, carry out the following steps:

1.  Join the DDS network using a DomainParticipant
2.  Define which data type and under what name you will publish your message as a Topic
3.  Create the `DataWriter` that publishes that Topic
4.  And finally, publish the message.

```{.python3 linenos=""}
from cyclonedds.topic import Topic
from cyclonedds.pub import DataWriter

participant = DomainParticipant()
topic = Topic(participant, "Announcements", Message)
writer = DataWriter(participant, topic)

writer.write(message)
```

You have now published your first message successfully! However, it is hard to tell if that did anything since we don\'t have anything set up to listen for incoming messages. Let\'s make a second script that takes messages from DDS and prints them to the terminal:

> 您现在已经成功发布了您的第一条消息！ 然而，很难判断这是否起到了作用，因为我们没有设置任何东西来监听传入的消息。 让我们制作第二个脚本，从 DDS 获取消息并将它们打印到终端：

```{.python3 linenos=""}
from dataclasses import dataclass
from cyclonedds.domain import DomainParticipant
from cyclonedds.topic import Topic
from cyclonedds.sub import DataReader
from cyclonedds.util import duration
from cyclonedds.idl import IdlStruct

@dataclass
class Message(IdlStruct):
    text: str

participant = DomainParticipant()
topic = Topic(participant, "Announcements", Message)
reader = DataReader(participant, topic)

# If we don't receive a single announcement for five minutes, we want the script to exit.
for msg in reader.take_iter(timeout=duration(minutes=5)):
    print(msg.text)
```

Now with this script running in a second terminal, you should see the message pop up when you rerun the first script.
