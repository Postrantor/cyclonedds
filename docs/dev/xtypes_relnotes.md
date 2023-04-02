# Cyclone XTypes support

## Release 0.9

### Type System

- The following data types are not supported: map, bitset, wide-strings, char16, float128

> -不支持以下数据类型：map、bitset、wide string、char16、float128

- For the C language binding, additionally the following types are not supported as part of a type’s key: union, sequence

> -对于 C 语言绑定，另外，以下类型不支持作为类型键的一部分：并集、序列

- Union types:

> -活接头类型：

- Using bitmask type as discriminator is not supported

> -不支持使用位掩码类型作为鉴别器

- Inheritance (7.2.2.4.5) is not supported

> -不支持继承（7.2.2.4.5）

- Extensibility `mutable` for unions is not supported

> -不支持联合的扩展性“可变”

- The Dynamic Language Binding (7.5.2) is not supported (7.6.6, DynamicData and DynamicType API). Note: the Python API supports dynamic types without requiring a separate API.

> -不支持动态语言绑定（7.5.2）（7.6.6，DynamicData 和 DynamicType API）。注意：Python API 支持动态类型，而不需要单独的 API。

- The built-in TypeLookup service (7.6.3.3) has no support for requesting type dependencies (service operation `getTypeDependencies`, section 7.6.3.3.4.1) and replying to a request of this type.

> -内置 TypeLookup 服务（7.6.3.3）不支持请求类型依赖项（服务操作“getTypeDependences”，第 7.6.3.3.4.1 节）和回复此类型的请求。

- Because of this, handling `PublicationBuiltinTopicData` or `SubscriptionBuiltinTopicData` with an incomplete set of dependent types (i.e. number of entries in `dependent_typeids` is less than `dependent_typeid_count`) may result in a failure to match a reader with a writer.

> -因此，使用一组不完整的依赖类型处理“PublicationBuiltinTopicData”或“SubscriptionBuiltinTopicData”（即“dependent_typeids”中的条目数小于“dependent_typeid_count”）可能会导致读取器与写入器无法匹配。

- In case a union has a default case, the C (de)serializer requires that the default case comes last because of a limitation of the IDL compiler.

> -如果一个并集有一个默认情况，由于 IDL 编译器的限制，C（反）序列化程序要求默认情况排在最后。

- Using the `try_construct` annotation (7.2.2.7) with a parameter other than `DISCARD` (the default) is not supported.

> -不支持将“try_construct”注释（7.2.2.7）与“DISCARD”（默认值）以外的参数一起使用。

- The C deserializer does not support explicit defaults for members of an aggregated type (`default` annotation)

> -C 反序列化程序不支持聚合类型的成员的显式默认值（`default`注释）

- External (7.3.1.2.1.4) collections element types not supported (e.g. `sequence<@external b>`)

> -不支持外部（7.3.1.2.1.4）集合元素类型（例如`sequence<@External b>`）

- Using `default_literal` (7.3.1.2.1.10) to set the default for enumerated types is not supported

> -不支持使用“default_lilateral”（7.3.1.2.1.10）设置枚举类型的默认值

- Default extensibility is `final` rather than `appendable` to maintain backwards compatibility with DDS implementations that do not support XTypes (including Cyclone DDS versions prior to 0.9.0). The IDL compiler has command-line option to select a different default.

> -默认的可扩展性是“final”而不是“appendable”，以保持与不支持 XTypes 的 DDS 实现的向后兼容性（包括 0.9.0 之前的 Cyclone DDS 版本）。IDL 编译器有命令行选项来选择不同的默认值。

### Type Representation

- Type Object type representation

> -类型对象类型表示

- Recursive types are not supported (Strongly Connected Components, 7.3.4.9)

> -不支持递归类型（强连接组件，7.3.4.9）

- User-defined annotations (7.3.1.2.4) and `verbatim` annotations (7.3.2.5.1.1) are not included in complete type objects

> -用户定义的注释（7.3.1.2.4）和“逐字”注释（7.3.2.5.1.1）不包括在完整的类型对象中

- IDL type representation

> -IDL 类型表示

- Pragma declarations other than `keylist` are not supported

> -不支持“keylist”以外的 Pragma 声明

- Alternative Annotation Syntax (7.3.1.2.3) is not supported

> -不支持替代注释语法（7.3.1.2.3）

- `verbatim` annotation (7.3.2.5.1.1) is not supported

> -不支持“逐字逐句”注释（7.3.2.5.1.1）

- `ignore_literal_names` annotation (7.3.1.2.1.11) is not supported

> -不支持`ignore_literal_names`注释（7.3.1.2.1.11）

- `non_serialized` annotation (7.3.1.2.1.14) is not supported

> -不支持`non_serialized`注释（7.3.1.2.1.14）

- XML (7.3.2) and XSD (7.3.3) type representation not supported

> -不支持 XML（7.3.2）和 XSD（7.3.3）类型表示

### Data Representation

- Default data representation is XCDR1 for `@final` types without optional members to maintain backwards compatibility with DDS implementations that do not support XTypes (including Cyclone DDS versions prior to 0.9.0).

> -对于没有可选成员的“@final”类型，默认数据表示为 XCDR1，以保持与不支持 XTypes 的 DDS 实现的向后兼容性（包括 0.9.0 之前的 Cyclone DDS 版本）。

All other types require XCDR2: following 7.6.3.1.1 there is no need to support XCDR1 for interoperating with DDS implementations (ignoring those that only support XTypes 1.0 or 1.1, but not 1.2 or later).

> 所有其他类型都需要 XCDR2：根据 7.6.3.1.1，不需要支持 XCDR1 与 DDS 实现进行互操作（忽略那些只支持 XTypes 1.0 或 1.1，但不支持 1.2 或更高版本的实现）。

The C serializer does not support PL-CDR version 1 nor optional members in PLAIN-CDR version 1.

> C 序列化程序不支持 PL-CDR 版本 1，也不支持 PLAIN-CDR 版本 1 中的可选成员。

- XML data representation (7.4.4) is not supported

> -不支持 XML 数据表示（7.4.4）
