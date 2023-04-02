# HelloWorld IDL {#helloworld_idl}

The HelloWorld data type is described in a language-independent way and stored in the HelloWorldData.idl file:

> HelloWorld 数据类型以独立于语言的方式进行描述，并存储在 HelloWorldData.idl 文件中：

```omg-idl
module HelloWorldData
{
    struct Msg
    {
        @key long userID;
        string message;
    };
};
```

The data definition language used for DDS corresponds to a subset of the OMG Interface Definition Language (IDL). In our simple example, the HelloWorld data model is made of one module `HelloWorldData`. A module can be seen as a namespace where data with interrelated semantics are represented together in the same logical set.

> 用于 DDS 的数据定义语言对应于 OMG 接口定义语言(IDL)的一个子集。在我们的简单示例中，HelloWorld 数据模型由一个模块`HelloWorldData`组成。模块可以被视为一个命名空间，其中具有相互关联语义的数据在同一逻辑集中一起表示。

The `structMsg` is the data type that shapes the data used to build topics. As already mentioned, a topic is an association between a data type and a string name. The topic name is not defined in the IDL file, but the application business logic determines it at runtime.

> `structMsg`是对用于构建主题的数据进行整形的数据类型。如前所述，主题是数据类型和字符串名称之间的关联。IDL 文件中没有定义主题名称，但应用程序业务逻辑在运行时确定它。

In our simplistic case, the data type Msg contains two fields: `userID` and `message` payload. The `userID` is used to uniquely identify each message instance. This is done using the `@key` annotation.

> 在我们简单的例子中，数据类型 Msg 包含两个字段：`userID`和`message`payload。`userID`用于唯一标识每个消息实例。这是使用`@key`注释完成的。

## IDL Compilers

The IDL compiler translates the IDL datatype into a C struct with a name made of the`<ModuleName>_<DataTypeName>` .

> IDL 编译器**将 IDL 数据类型转换为 C 结构**，其名称由`<ModuleName>_<DataTypeName>`组成。

```C
typedef struct HelloWorldData_Msg
{
    int32_t userID;
    char * message;
} HelloWorldData_Msg;
```

The C++ IDL compiler translates module names into namespaces and structure names into classes.

> C++IDL 编译器将模块名称转换为名称空间，将结构名称转换为类。

It also generates code for public accessor functions for all fields mentioned in the IDL struct, separate public constructors, and a destructor:

> 它还为 IDL 结构中提到的所有字段、单独的公共构造函数和析构函数**生成公共访问器函数的代码**：

- A default (empty) constructor that recursively invokes the constructors of all fields.
- A copy-constructor that performs a deep copy from the existing class.
- A move-constructor that moves all arguments to its members.

> - 递归调用所有字段的构造函数的默认(空)构造函数。
> - 从现有类执行深度复制的复制构造函数。
> - 将所有参数移动到其成员的移动构造函数。

The destructor recursively releases all fields. It also generates code for assignment operators that recursively construct all fields based on the parameter class (copy and move versions). The following code snippet is provided without warranty: the internal format may change, but the API delivered to your application code is stable.

> 析构函数递归地释放所有字段。它**还为赋值运算符生成代码**，这些运算符基于参数类**递归地构造**所有字段(复制和移动版本)。以下代码片段不提供保证：内部格式可能会更改，但提供给应用程序代码的 API 是稳定的。

```C++
namespace HelloWorldData
{
    class Msg OSPL_DDS_FINAL
    {
    public:
        int32_t userID_;
        std::string message_;

    public:
        Msg() :
                userID_(0) {}

        explicit Msg(
            int32_t userID,
            const std::string& message) :
                userID_(userID),
                message_(message) {}

        Msg(const Msg &_other) :
                userID_(_other.userID_),
                message_(_other.message_) {}

#ifdef OSPL_DDS_C++11
        Msg(Msg &&_other) :
                userID_(::std::move(_other.userID_)),
                message_(::std::move(_other.message_)) {}
        Msg& operator=(Msg &&_other)
        {
            if (this != &_other) {
                userID_ = ::std::move(_other.userID_);
                message_ = ::std::move(_other.message_);
            }
            return *this;
        }
#endif
        Msg& operator=(const Msg &_other)
        {
            if (this != &_other) {
                userID_ = _other.userID_;
                message_ = _other.message_;
            }
            return *this;
        }

        bool operator==(const Msg& _other) const
        {
            return userID_ == _other.userID_ &&
                message_ == _other.message_;
        }

        bool operator!=(const Msg& _other) const
        {
            return !(*this == _other);
        }

        int32_t userID() const { return this->userID_; }
        int32_t& userID() { return this->userID_; }
        void userID(int32_t _val_) { this->userID_ = _val_; }
        const std::string& message() const { return this->message_; }
        std::string& message() { return this->message_; }
        void message(const std::string& _val_) { this->message_ = _val_; }
#ifdef OSPL_DDS_C++11
        void message(std::string&& _val_) { this->message_ = _val_; }
#endif
    };

}
```

> [!Note]
> When translated into a different programming language, the data has another representation specific to the target language. This highlights the advantage of using a neutral language such as IDL to describe the data model. It can be translated into different languages that can be shared between different applications written in other programming languages.

## Generated files with the IDL compiler

The IDL compiler is a C program that processes .idl files.

> IDL 编译器是一个处理.IDL 文件的 C 程序。

```bash
idlc HelloWorldData.idl
```

This results in new `HelloWorldData.c` and `HelloWorldData.h` files that need to be compiled, and their associated object file must be linked with the **Hello World!** publisher and subscriber application business logic. When using the provided CMake project, this step is done automatically.

> 这将导致需要编译新的`HelloWorldData.c `和`HelloWorldData.h `文件，并且它们关联的对象文件必须与**Hello World 链接！**发布者和订阅者应用程序业务逻辑。当使用所提供的 CMake 项目时，此步骤将自动完成。

As described earlier, the IDL compiler generates one source and one header file. The header file (`HelloWorldData.h`) contains the shared messages\' data type. While the source file has no direct use from the application developer\'s perspective.

> 如前所述，IDL 编译器生成一个源文件和一个头文件。头文件(`HelloWorldData.h `)包含共享消息的数据类型。而从应用程序开发人员的角度来看，源文件没有直接的用途。

**`HelloWorldData.h` needs to be included in the application code** as it contains the actual message type and contains helper macros to allocate and free memory space for the `HelloWorldData_Msg` type.

```C
typedef struct HelloWorldData_Msg
{
    int32_t userID;
    char * message;
} HelloWorldData_Msg;

HelloWorldData_Msg_alloc()
HelloWorldData_Msg_free(d,o)
```

The header file also contains an extra variable that describes the data type to the DDS middleware. This variable needs to be used by the application when creating the topic.

> **头文件还包含一个额外的变量，用于描述 DDS 中间件的数据类型。**应用程序在创建主题时需要使用此变量。

```C
HelloWorldData_Msg_desc
```

The IDL compiler is a bison-based parser written in pure C and should be fast and portable. It loads dynamic libraries to support different output languages, but this is seldom relevant to you as a user. You can use `CMake` recipes as described above or invoke directly:

> IDL 编译器是一个用纯 C 编写的基于 bison 的解析器，应该是快速和可移植的。它加载动态库以支持不同的输出语言，但这与用户很少相关。您可以使用如上所述的`CMake`配方，也可以直接调用：

```bash
idlc -l C++ HelloWorldData.idl
```

This results in the following new files that need to be compiled and their associated object file linked with the Hello _World!_ publisher and subscriber application business logic:

> 这将导致以下需要编译的新文件及其与 Hello*World！*链接的关联对象文件发布者和订阅者应用程序业务逻辑：

- `HelloWorldData.hpp`
- `HelloWorldData.cpp`

When using CMake to build the application, this step is hidden and is done automatically. For building with CMake, refer to [building the HelloWorld example.](#build-the-dds-C++-hello-world-example)

> 当使用 CMake 构建应用程序时，此步骤将被隐藏并自动完成。要使用 CMake 进行构建，请参阅[构建 HelloWorld 示例。](#build-the-dds-C++-hello-world 示例)

`HelloWorldData.hpp` and `HelloWorldData.cpp` files contain the data type of messages that are shared.

## HelloWorld business logic

As well as the HelloWorldData.h/c generated files, the HelloWorld example also contains two application-level source files (subscriber.c and publisher.c), containing the business logic.

> 除了 HelloWorldData.h/c 生成的文件外，HelloWorld 示例还包含两个应用程序级源文件(subscriber.c 和 publisher.c)，其中包含业务逻辑。

As well as from the `HelloWorldData` data type files that the _DDS C++ Hello World_ example used to send messages, the _DDS C++ Hello World!_ example also contains two application-level source files (`subscriber.cpp` and `publisher.cpp`), containing the business logic.

> 除了来自 _DDS C++Hello World_ 示例用于发送消息的`HelloWorldData`数据类型文件外， _DDS C++Hello World！_ 示例还包含两个应用程序级源文件(`subscriber.cpp`和`publisher.cpp`)，其中包含业务逻辑。
