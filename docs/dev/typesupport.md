# Type support

[DDS](https://www.omg.org/spec/DDS/) is data-centric. To communicate between applications, possibly written in different languages, either locally or remotely, the middleware must marshal the data to a Common Data Representation (CDR) suitable for transmission over a network.

> [DDS](https://www.omg.org/spec/DDS/)以数据为中心。为了在可能以不同语言编写的应用程序之间进行通信，无论是本地还是远程，中间件都必须将数据编组到适合通过网络传输的通用数据表示（CDR - Common Data Representation）。

To marshal the data, the middleware must know how it is constructed. To achieve this, types communicated between applications are defined in the Interface Definition Language (IDL). The IDL type definitions are processed by an IDL compiler to generate language specific type definitions and instructions/routines to marshal in-memory data to/from CDR.

> **要封送数据，中间件必须知道数据是如何构造的。为了实现这一点，应用程序之间通信的类型是在接口定义语言（IDL）中定义的**。IDL 类型定义由 IDL 编译器处理，以生成特定于语言的类型定义和指令/例程，从而将内存中的数据封送至 CDR 或从 CDR 封送。

> Until IDL 3.5, the "Interface Definition Language" (IDL) and "Common Data Representation" (CDR) were part of the _[CORBA](https://www.omg.org/spec/CORBA/)_ specification, but the most recent version of the _Interface Definition Language_, IDL 4.2, can be downloaded from [here](https://www.omg.org/spec/IDL/4.2/Beta1).

## Runtime type generation

Interpreted languages, or languages that are not compiled to machine-language instructions often allow type introspection (e.g. Java) or even reflection (e.g. Python).

> 解释语言或未编译为机器语言指令的语言通常允许类型内省（例如 Java）甚至反射（例如 Python）。

With the introduction of (Extensible and Dynamic Topic Types for DDS) [<https://www.omg.org/spec/DDS-XTypes>] (XTypes) type descriptions must be communicated with the topics so that applications can verify type compatibility and/or type assignability.

> 随着（DDS 的可扩展和动态主题类型）的引入[<https://www.omg.org/spec/DDS-XTypes>]（XTypes）类型描述必须与主题进行通信，以便应用程序能够验证类型兼容性和/或类型可分配性。

The availability of all information required to (re)construct types from the topic description makes it possible to construct language native types at runtime, i.e. without the need for IDL type definitions being available at compile time. For languages that support introspection or reflection, this allows types to be introduced dynamically. Of course, if a language supports introspection or reflection it is also possible to introduce types into the system dynamically.

> 从主题描述中（重新）构造类型所需的所有信息的可用性使得在运行时构造语言本机类型成为可能，即不需要在编译时提供 IDL 类型定义。对于支持内省或反射的语言，这允许动态引入类型。当然，如果一种语言支持内省或反射，那么也可以动态地将类型引入到系统中。

## TypeTree

Type generation at compile time and runtime share a lot of commonalities. As such, if cleverly constructed, it is expected that various parts can simply be shared between the two.

> 编译时和运行时的类型生成有很多共同点。因此，如果巧妙地构建，预计两者之间可以简单地共享各个部分。

As is customary in compiler design, the input must first be converted into a representation suitable for traversal in memory. This part of the process is handled by the _frontend_. The _frontent_ takes a type definition, or set of type definitions, verify they are correct and store them in memory in an intermediate representation that is understood by the _backend_. The _backend_ takes the intermediate format and uses it to generate one or more target representations. e.g. For native C types to be used, the _backend_ would need to generate native C language representations (opcodes for the serialization VM included), the _TypeObject_ representation for XTypes compatibility and the OpenSplice XML format for compatibility with OpenSplice.

> 按照编译器设计中的惯例，输入必须首先转换为适合在内存中遍历的表示。流程的这一部分由*frontend*处理。*frontent*获取一个类型定义或一组类型定义，验证它们是否正确，并将它们以*backend*可以理解的中间表示形式存储在内存中。*backend*采用中间格式，并使用它来生成一个或多个目标表示。例如，对于要使用的本机 C 类型，*backend*需要生成本机 C 语言表示（包括序列化 VM 的操作码）、用于 XTypes 兼容性的*TypeObject*表示以及用于与 OpenSplice 兼容的 OpenSplice XML 格式。

```
    --------                                -----
    |  IDL |---------\                  /-- | C |
    --------         |                  |   -----
                     |                  |   -------
                     |                  |-- | C++ |
                     |                  |   -------
    --------------   |   ------------   |   --------------
    | TypeObject | -->-- | TypeTree | -->-- | TypeObject |
    --------------   |   ------------   |   --------------
                     |                  |   -------
                     |                  |-- | XML |
                     |                  |   -------
    -------          |                  |   ------------------
    | XML |----------/                  \-- | OpenSplice XML |
    -------                                 ------------------
```

The diagram above provides a very minimal overview of the different parts involved with translating language agnostic type definitions into language native type definitions. The diagram clearly shows the importance of the intermediate format, hereafter to be referred to as the _TypeTree_. XML stands for the XML Type Representation, which like the TypeObject, is a way to represent types as specified in the [Extensible and Dynamic Topic Types for DDS](https://www.omg.org/spec/DDS-XTypes/About-DDS-XTypes/) specification.

> 上图提供了将语言不可知类型定义转换为语言本机类型定义所涉及的不同部分的非常简单的概述。该图清楚地显示了中间格式的重要性，以下称为*TypeTree*。XML 代表 XML 类型表示，它与 TypeObject 一样，是一种表示[DDS 的可扩展和动态主题类型]中指定的类型的方法(https://www.omg.org/spec/DDS-XTypes/About-DDS-XTypes/)规范。

### TypeTree design

A backend, in essence, does nothing more than generate a native language representation of the TypeTree. But, the output and the internal flow vary greatly between languages.

> 从本质上讲，后端只不过是生成 TypeTree 的本地语言表示。但是，不同语言的输出和内部流程差别很大。

For instance, C++ requires only a header and a source file. Java, however, requires each (public) class to reside in a separate file.

> 例如，C++只需要一个标头和一个源文件。然而，Java 要求每个（公共）类都驻留在一个单独的文件中。

Previous incarnations of IDL compilers studied often used a visitor pattern. i.e. The backend registers a set of callback functions, usually one or more for each type, and the programmer calls a function that _visits_ each type exactly once and calls the appropriate callback functions in order. However, this method is considered too rigid.

> 所研究的 IDL 编译器的先前版本经常使用访问者模式。即后端注册一组回调函数，通常每种类型都有一个或多个，程序员只调用一次*visis*每种类型的函数，并按顺序调用适当的回调函数。然而，这种方法被认为过于僵化。

1.  There are cases where a part of the tree must be traversed several times with different callback functions. For instance, when generating code for for a struct in an object oriented language, all members must be traversed before the class definition, constructors and destructors can be generated.

> 1. 在某些情况下，必须使用不同的回调函数多次遍历树的一部分。例如，在用面向对象的语言为结构生成代码时，必须遍历所有成员，然后才能生成类定义、构造函数和析构函数。

2.  There could be a need to adjust the order in which the tree is traversed. For instance when a given language does not support nested structs. The order in which the tree is traversed must be reversed to first emit the definitions of the nested structs before the definition of the current struct can be emitted.

> 2. 可能需要调整遍历树的顺序。例如，当给定的语言不支持嵌套结构时。遍历树的顺序必须颠倒，以便首先发出嵌套结构的定义，然后才能发出当前结构的定义。

3. One of the design goals of the intermediate format and accompanying utilities is to facilitate efficient development of backends. Therefore, because there is no single pattern that works well for all supported languages, the TypeTree itself will be considered the API and basic utility functions will be offered to simplify traversing the TypeTree etc.

> 3. 中间格式和配套实用程序的设计目标之一是促进后端的高效开发。 因此，因为没有一种模式适用于所有支持的语言，所以 TypeTree 本身将被视为 API，并且将提供基本实用函数以简化遍历 TypeTree 等。

### TypeTree structure

The TypeTree, as indicated by the name, is a simple tree that closely follows IDL syntax. This is a requirement because the order has importance. A pseudocode example of how types are constructed is provided below.

> 如名称所示，TypeTree 是一个简单的树，严格遵循 IDL 语法。这是一项要求，因为订单很重要。下面提供了一个如何构造类型的伪代码示例。

```c
#define INT (1<<0)
// ...
#define TYPEDEF (1<<10)
#define STRUCT (1<<11)
#define MAP (1<<12)
// ...
#define ARRAY (1<<20)
#define UNBOUND (1<<21)

typedef union type type_t;

typedef void(*dtor_t)(type_t *type, type_t *parent);

typedef struct {
    int flags;
    char *name;
    type_t *next;
    type_t *parent;
    dtor_t dtor;
} typespec_t;

typedef struct {
    typespec_t type; /* ARRAY bit set icw. normal type flags and perhaps UNBOUND */
    size_t size;
} array_t;

typedef struct { /* TYPEDEF bit set, name is name of typedef. */
    typespec_t type;
    typespec_t *target; /* Pointer to target type. */
} typedef_t;

typedef struct {
    typespec_t type; /* FORWARD_DECL and type bits set. */
    type_t *target;
} forward_decl_t;

/* Constructed types introduce a complexity when the TypeTree is destructed
because they can be both embedded and referenced. A constructed type must
only be destructed from the scope in which it is defined, never when merely
referenced. */
// 构造类型在 TypeTree 被破坏时引入了复杂性，因为它们既可以嵌入也可以引用。
// 构造类型只能从定义它的范围中析构，而不能仅在被引用时析构。
typedef struct {
    typespec_t type;
    type_t *members;
} struct_t;

typedef struct {
    typespec_t type;
    type_t *key;
    type_t *value;
} map_t;

typedef struct {
    typespec_t type;
    type_t *elem;
} sequence_t;

/* Allows dereference without a cast. e.g. type->tu_array.size. */
union type {
    typespec_t tu_spec;
    array_t tu_array;
    typedef_t tu_typedef;
    struct_t tu_struct;
    map_t tu_map;
};
```

> [!NOTE]: 在 c 语言中，如何理解 "#define INT (1<<0)"@16:42:17
> 在 C 语言中，通过 `#define` 来定义宏常量，被预处理器替换成实际的值，在编译期间就能够确定，可以用来代替数字字面量或者一些表达式，提高代码的可读性和可维护性。
> `#define INT (1<<0)` 定义了一个宏常量 INT，它的值为 1 左移 0 位，即 1。这个宏常量在代码中可以像变量一样使用，例如：
>
> ```c
> int var = INT;
> printf("%d", var);  // 输出 1
> ```
>
> 这种用法在位运算中非常常见，我们可以通过定义这样的宏来更加方便地进行位操作。例如，如果我们需要对某个整数的第 3 位进行操作，可以定义一个宏：
>
> ```c
> #define SET_BIT_3(x) ((x) | (1<<2)) // 将 x 第 3 位设置为 1
> int num = 0;
> num = SET_BIT_3(num);
> ```
>
> 这样，使用 `SET_BIT_3` 宏就可以实现将某个整数的第 3 位设置为 1 的功能了。

#### Details

Although the TypeTree follows the IDL syntax, there is one notable exceptions. Consider for example, the following fragment of IDL:

> 尽管 TypeTree 遵循 IDL 语法，但有一个值得注意的例外。例如，考虑 IDL 的以下片段：

```c
struct s {
    short a[3], b[5][6];
};
```

The the syntax tree would look like:

```
   struct 's'
   |
   +-- member
       |
       +-- type 'short'
       |
       +-- declarators
           |
           +-- declarator 'a'
           |   |
           |   +-- array size '3'
           |
           +-- declarator 'b'
               |
               +-- array size '5'
               |
               +-- array size '6'
```

But in the TypeTree it is better to use the following tree representation that follows the \'semantic\' representation of a type better:

> 但在 TypeTree 中，最好使用以下树表示法，该表示法遵循类型的“语义”表示法：

```
   struct 's'
   |
   +-- declaration 'a'
   |   |
   |   +-- array size '3'
   |       |
   |       +-- short   <-------+
   |                           |
   +-- declaration 'b'         |
       |                       |
       +-- array size '5'      |
           |                   |
           +-- array size '6'  |
               |               |
               +---------------+
```

Note that the type \'short\' is shared between the two declarations and that a far more complex type could have been used instead. During the freeing of the TypeTree, some mechanism is needed to determine if a tree is shared or not. Although, reference counting is commonly used for this, we decide to use a special flag for this. In the above example this flag will be set on the type [array size \'6\']{.title-ref}. (Because a map has two types, it requires two different flags for this.)

> 请注意，类型 \'short\' 在两个声明之间共享，并且可以使用更复杂的类型来代替。 在释放 TypeTree 期间，需要某种机制来确定树是否共享。 虽然引用计数通常用于此，但我们决定为此使用一个特殊的标志。 在上面的示例中，此标志将设置为类型 [array size \'6\']{.title-ref}。 （因为地图有两种类型，所以它需要两个不同的标志。）

#### Examples with structs

There are several ways to define the \'same\' data structure with (anonymous) structs. Below three examples are given, together with the type trees.

> 有几种方法可以用（匿名）结构来定义“我”数据结构。下面给出了三个例子，以及类型树。

The first example defines a struct [A]{.title-ref}, which is used in struct [B]{.title-ref}.

> 第一个例子定义了一个结构[a]｛.title-ref｝，该结构用于结构[B]｛.title-ref｝。

```c
struct A {
    short a;
};
struct B {
    A b;
};
```

This results in the following TypeTree:

```
    |
    +-- struct 'A'  <---------+
    |   |                     |
    |   +-- declaration 'a'   |
    |       |                 |
    |       +-- short         |
    |                         |
    +-- struct 'B'            |
        |                     |
        +-- declaration 'b'   |
            |                 |
            +-----------------+
```

In the second example, the struct [A]{.title-ref} appears as an embedded struct inside struct [B]{.title-ref}.

> 在第二个示例中，结构[A]｛.title-ref｝显示为结构[B]｛.title-ref｝中的嵌入结构。

```c
struct B {
    struct A {
    short a;
    };
    A b;
};
```

This results in the following TypeTree:

```
    |
    +-- struct 'B'
        |
        +-- struct 'A'  <---------+
        |   |                     |
        |   +-- declaration 'a'   |
        |       |                 |
        |       +-- short         |
        |                         |
        +-- declaration 'b'       |
            |                     |
            +---------------------+
```

In the third example, an anonymous struct is used in the declaration of `b`.

> 在第三个例子中，在“b”的声明中使用了一个匿名结构。

```c
struct B {
    struct {
    short a;
    } b;
};
```

This results in the following TypeTree:

> 这会产生以下类型树：

```
    |
    +-- struct 'B'
        |
        +-- declarator 'b'
            |
            +-- struct
                |
                +-- declarator 'a'
                    |
                    +-- short
```
