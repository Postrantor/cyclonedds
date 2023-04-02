/*
 * Copyright(c) 2023 ZettaScale Technology and others
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v. 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0, or the Eclipse Distribution License
 * v. 1.0 which is available at
 * http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * SPDX-License-Identifier: EPL-2.0 OR BSD-3-Clause
 */

#ifndef DDS_DYNAMIC_TYPE_H
#define DDS_DYNAMIC_TYPE_H

#if defined(__cplusplus)
extern "C" {
#endif

struct ddsi_typeinfo;

/**
 * @defgroup dynamic_type (Dynamic Type API)
 * @ingroup dds
 * The Dynamic Type API to construct and manipulate data types.
 */

/**
 * @brief 动态类型 (Dynamic Type)
 * @ingroup dynamic_type
 *
 * 表示动态创建的类型。此结构体具有一个指向类型系统中类型的不透明指针。
 * 在构建类型（设置属性和添加成员）期间，内部类型处于“CONSTRUCTION”状态。
 * 一旦类型被注册，状态将更新为“RESOLVED”，并且类型不能再进行修改。
 *
 * 此结构体的'ret'成员保存在此类型上执行的操作的返回代码。
 * 如果此值不是DDS_RETCODE_OK，则无法将类型用于进一步处理（例如添加成员、注册类型等）。
 *
 */
typedef struct dds_dynamic_type {
  void* x;  // 不透明指针，指向类型系统中的类型 (Opaque pointer to the type in the type system)
  dds_return_t ret;  // 操作的返回代码 (Return code of operations performed on this type)
} dds_dynamic_type_t;

/**
 * @ingroup dynamic_type
 *
 * 无效成员ID：在添加成员时使用，表示成员应获取ID（m+1），其中m是当前成员集中最高成员ID。
 * 有效的成员ID将最高4位设置为0（因为在EMHEADER中使用），因此即使在使用哈希ID时，哈希成员ID也永远不会设置为无效成员ID。
 */
#define DDS_DYNAMIC_MEMBER_ID_INVALID 0xf000000u  // 无效成员ID (Invalid member ID)
#define DDS_DYNAMIC_MEMBER_ID_AUTO \
  DDS_DYNAMIC_MEMBER_ID_INVALID                   // 自动分配成员ID (Auto-assign member ID)

/**
 * @ingroup dynamic_type
 *
 * 在添加成员时，索引0用于表示成员应插入为第一个成员。大于当前最大索引的值可以用来表示成员应在其他成员之后添加。
 */
#define DDS_DYNAMIC_MEMBER_INDEX_START 0u        // 成员开始索引 (Member start index)
#define DDS_DYNAMIC_MEMBER_INDEX_END UINT32_MAX  // 成员结束索引 (Member end index)

/**
 * @brief 动态类型种类 (Dynamic Type Kind)
 * @ingroup dynamic_type
 *
 * 用于创建动态类型的类型种类值的枚举。
 */
typedef enum dds_dynamic_type_kind {
  DDS_DYNAMIC_NONE,         // 无类型 (None type)
  DDS_DYNAMIC_BOOLEAN,      // 布尔类型 (Boolean type)
  DDS_DYNAMIC_BYTE,         // 字节类型 (Byte type)
  DDS_DYNAMIC_INT16,        // 16位整型 (16-bit integer type)
  DDS_DYNAMIC_INT32,        // 32位整型 (32-bit integer type)
  DDS_DYNAMIC_INT64,        // 64位整型 (64-bit integer type)
  DDS_DYNAMIC_UINT16,       // 16位无符号整型 (16-bit unsigned integer type)
  DDS_DYNAMIC_UINT32,       // 32位无符号整型 (32-bit unsigned integer type)
  DDS_DYNAMIC_UINT64,       // 64位无符号整型 (64-bit unsigned integer type)
  DDS_DYNAMIC_FLOAT32,      // 32位浮点型 (32-bit floating-point type)
  DDS_DYNAMIC_FLOAT64,      // 64位浮点型 (64-bit floating-point type)
  DDS_DYNAMIC_FLOAT128,     // 128位浮点型 (128-bit floating-point type)
  DDS_DYNAMIC_INT8,         // 8位整型 (8-bit integer type)
  DDS_DYNAMIC_UINT8,        // 8位无符号整型 (8-bit unsigned integer type)
  DDS_DYNAMIC_CHAR8,        // 8位字符类型 (8-bit character type)
  DDS_DYNAMIC_CHAR16,       // 16位字符类型 (16-bit character type)
  DDS_DYNAMIC_STRING8,      // 8位字符串类型 (8-bit string type)
  DDS_DYNAMIC_STRING16,     // 16位字符串类型 (16-bit string type)
  DDS_DYNAMIC_ENUMERATION,  // 枚举类型 (Enumeration type)
  DDS_DYNAMIC_BITMASK,      // 位掩码类型 (Bitmask type)
  DDS_DYNAMIC_ALIAS,        // 别名类型 (Alias type)
  DDS_DYNAMIC_ARRAY,        // 数组类型 (Array type)
  DDS_DYNAMIC_SEQUENCE,     // 序列类型 (Sequence type)
  DDS_DYNAMIC_MAP,          // 映射类型 (Map type)
  DDS_DYNAMIC_STRUCTURE,    // 结构体类型 (Structure type)
  DDS_DYNAMIC_UNION,        // 联合体类型 (Union type)
  DDS_DYNAMIC_BITSET        // 位集类型 (Bitset type)
} dds_dynamic_type_kind_t;

/**
 * @ingroup dynamic_type
 *
 * 非基元和基元类型的动态类型规范的初始化器的简短表示。
 */
#define DDS_DYNAMIC_TYPE_SPEC(t)                \
  ((dds_dynamic_type_spec_t){                   \
      .kind = DDS_DYNAMIC_TYPE_KIND_DEFINITION, \
      .type = {.type = (t)}})  // 动态类型规范定义 (Dynamic type spec definition)
#define DDS_DYNAMIC_TYPE_SPEC_PRIM(p)          \
  ((dds_dynamic_type_spec_t){                  \
      .kind = DDS_DYNAMIC_TYPE_KIND_PRIMITIVE, \
      .type = {.primitive =                    \
                   (p)}})  // 基元动态类型规范定义 (Primitive dynamic type spec definition)

/**
 * @ingroup dynamic_type
 *
 * 结构成员描述符的简短表示，具有不同的常用属性集。
 */
#define DDS_DYNAMIC_MEMBER_(member_type_spec, member_name, member_id, member_index) \
  ((dds_dynamic_member_descriptor_t){                                               \
      .name = (member_name),                                                        \
      .id = (member_id),                                                            \
      .type = (member_type_spec),                                                   \
      .index = (member_index)})  // 结构成员描述符定义 (Struct member descriptor definition)
#define DDS_DYNAMIC_MEMBER_ID(member_type, member_name, member_id)                      \
  DDS_DYNAMIC_MEMBER_(DDS_DYNAMIC_TYPE_SPEC((member_type)), (member_name), (member_id), \
                      DDS_DYNAMIC_MEMBER_INDEX_END)  // 成员ID定义 (Member ID definition)
#define DDS_DYNAMIC_MEMBER_ID_PRIM(member_prim_type, member_name, member_id)      \
  DDS_DYNAMIC_MEMBER_(                                                            \
      DDS_DYNAMIC_TYPE_SPEC_PRIM((member_prim_type)), (member_name), (member_id), \
      DDS_DYNAMIC_MEMBER_INDEX_END)  // 基元成员ID定义 (Primitive member ID definition)
#define DDS_DYNAMIC_MEMBER(member_type, member_name)  \
  DDS_DYNAMIC_MEMBER_ID((member_type), (member_name), \
                        DDS_DYNAMIC_MEMBER_ID_INVALID)  // 成员定义 (Member definition)
#define DDS_DYNAMIC_MEMBER_PRIM(member_prim_type, member_name) \
  DDS_DYNAMIC_MEMBER_ID_PRIM(                                  \
      (member_prim_type), (member_name),                       \
      DDS_DYNAMIC_MEMBER_ID_INVALID)  // 基元成员定义 (Primitive member definition)

/**
 * @ingroup dynamic_type
 *
 * Short notation for union member descriptor with different sets of commonly used properties
 */
// 定义联合成员描述符的简短表示法，具有不同的常用属性集
#define DDS_DYNAMIC_UNION_MEMBER_(member_type_spec, member_name, member_id, member_index, \
                                  member_num_labels, member_labels, member_is_default)    \
  ((dds_dynamic_member_descriptor_t){.name = (member_name),                               \
                                     .id = (member_id),                                   \
                                     .type = (member_type_spec),                          \
                                     .index = (member_index),                             \
                                     .num_labels = (member_num_labels),                   \
                                     .labels = (member_labels),                           \
                                     .default_label = (member_is_default)})
// 定义带有成员 ID 的联合成员
#define DDS_DYNAMIC_UNION_MEMBER_ID(member_type, member_name, member_id, member_num_labels,     \
                                    member_labels)                                              \
  DDS_DYNAMIC_UNION_MEMBER_(DDS_DYNAMIC_TYPE_SPEC((member_type)), (member_name), (member_id),   \
                            DDS_DYNAMIC_MEMBER_INDEX_END, (member_num_labels), (member_labels), \
                            false)
// 定义带有基本类型成员 ID 的联合成员
#define DDS_DYNAMIC_UNION_MEMBER_ID_PRIM(member_prim_type, member_name, member_id,          \
                                         member_num_labels, member_labels)                  \
  DDS_DYNAMIC_UNION_MEMBER_(DDS_DYNAMIC_TYPE_SPEC_PRIM((member_prim_type)), (member_name),  \
                            (member_id), DDS_DYNAMIC_MEMBER_INDEX_END, (member_num_labels), \
                            (member_labels), false)
// 定义联合成员（无成员 ID）
#define DDS_DYNAMIC_UNION_MEMBER(member_type, member_name, member_num_labels, member_labels) \
  DDS_DYNAMIC_UNION_MEMBER_ID((member_type), (member_name), DDS_DYNAMIC_MEMBER_ID_INVALID,   \
                              (member_num_labels), (member_labels))
// 定义基本类型的联合成员（无成员 ID）
#define DDS_DYNAMIC_UNION_MEMBER_PRIM(member_prim_type, member_name, member_num_labels, \
                                      member_labels)                                    \
  DDS_DYNAMIC_UNION_MEMBER_ID_PRIM((member_prim_type), (member_name),                   \
                                   DDS_DYNAMIC_MEMBER_ID_INVALID, (member_num_labels),  \
                                   (member_labels))

// 定义带有成员 ID 的默认联合成员
#define DDS_DYNAMIC_UNION_MEMBER_DEFAULT_ID(member_type, member_name, member_id)              \
  DDS_DYNAMIC_UNION_MEMBER_(DDS_DYNAMIC_TYPE_SPEC((member_type)), (member_name), (member_id), \
                            DDS_DYNAMIC_MEMBER_INDEX_END, 0, NULL, true)
// 定义带有基本类型成员 ID 的默认联合成员
#define DDS_DYNAMIC_UNION_MEMBER_DEFAULT_ID_PRIM(member_prim_type, member_name, member_id) \
  DDS_DYNAMIC_UNION_MEMBER_(DDS_DYNAMIC_TYPE_SPEC_PRIM((member_prim_type)), (member_name), \
                            (member_id), DDS_DYNAMIC_MEMBER_INDEX_END, 0, NULL, true)
// 定义默认联合成员（无成员 ID）
#define DDS_DYNAMIC_UNION_MEMBER_DEFAULT(member_type, member_name) \
  DDS_DYNAMIC_UNION_MEMBER_DEFAULT_ID((member_type), (member_name), DDS_DYNAMIC_MEMBER_ID_INVALID)
// 定义基本类型的默认联合成员（无成员 ID）
#define DDS_DYNAMIC_UNION_MEMBER_DEFAULT_PRIM(member_prim_type, member_name)  \
  DDS_DYNAMIC_UNION_MEMBER_DEFAULT_ID_PRIM((member_prim_type), (member_name), \
                                           DDS_DYNAMIC_MEMBER_ID_INVALID)

/**
 * @ingroup dynamic_type
 *
 * Dynamic Type specification kind
 */
// 动态类型规范种类
typedef enum dds_dynamic_type_spec_kind {
  DDS_DYNAMIC_TYPE_KIND_UNSET,
  DDS_DYNAMIC_TYPE_KIND_DEFINITION,
  DDS_DYNAMIC_TYPE_KIND_PRIMITIVE
} dds_dynamic_type_spec_kind_t;

/**
 * @ingroup dynamic_type
 *
 * Dynamic Type specification: a reference to dynamic type, which can be a primitive type
 * kind (just the type kind enumeration value), or a (primitive or non-primitive) dynamic
 * type reference.
 */
// 动态类型规范：对动态类型的引用，可以是基本类型（仅为类型枚举值）或（基本类型或非基本类型）的动态类型引用。
typedef struct dds_dynamic_type_spec {
  dds_dynamic_type_spec_kind_t kind;
  union {
    dds_dynamic_type_t type;
    dds_dynamic_type_kind_t primitive;
  } type;
} dds_dynamic_type_spec_t;

/**
 * @brief Dynamic Type descriptor
 * @ingroup dynamic_type
 *
 * Structure that holds the properties for creating a Dynamic Type. For each type kind,
 * specific member fields are applicable and/or required.
 */
// 动态类型描述符
// 保存创建动态类型所需属性的结构。对于每种类型，特定的成员字段适用且/或必需。
typedef struct dds_dynamic_type_descriptor {
  dds_dynamic_type_kind_t
      kind; /**< Type kind. Required for all types. 类型种类。所有类型都需要。 */
  const char* name; /**< Type name. Required for struct, union, alias, enum, bitmask, array,
                       sequence. 类型名称。结构、联合、别名、枚举、位掩码、数组、序列需要。 */
  dds_dynamic_type_spec_t
      base_type;    /**< Option base type for a struct, or (required) aliased type
                       in case of an alias type. 结构的可选基类型，或者（必需）别名类型的别名类型。 */
  dds_dynamic_type_spec_t discriminator_type; /**< Discriminator type for a union (required).
                                                 联合的判别器类型（必需）。 */
  uint32_t num_bounds; /**< Number of bounds for array and sequence types. In case of sequence, this
                          can be 0 (unbounded) or 1. 数组和序列类型的边界数。对于序列，可以是
                          0（无界）或 1。 */
  const uint32_t* bounds; /**< Bounds for array (0..num_bounds) and sequence (single value)
                             数组（0..num_bounds）和序列（单个值）的边界 */
  dds_dynamic_type_spec_t element_type; /**< Element type for array and sequence, required.
                                           数组和序列的元素类型，必需。 */
  dds_dynamic_type_spec_t
      key_element_type; /**< Key element type for map type 映射类型的键元素类型 */
} dds_dynamic_type_descriptor_t;

/**
 * @brief 动态类型成员描述符 (Dynamic Type Member descriptor)
 * @ingroup dynamic_type
 *
 * 用于向动态类型添加成员的属性结构。根据成员类型，适用并需要不同的字段。
 * Structure that holds the properties for adding a member to a dynamic type. Depending on
 * the member type, different fields apply and are required.
 */
typedef struct dds_dynamic_member_descriptor {
  const char* name;             /**< 成员名称，必需 (Name of the member, required) */
  uint32_t id;                  /**< 成员标识符，适用于结构和联合成员。
                                   可以使用 DDS_DYNAMIC_MEMBER_ID_AUTO 表示应使用下一个可用 ID（当前最大值 + 1）。
                                   (Identifier of the member, applicable for struct and union members.
                                   DDS_DYNAMIC_MEMBER_ID_AUTO can be used to indicate the next available id (current
                                   max + 1) should be used.) */
  dds_dynamic_type_spec_t type; /**< 成员类型，对于结构和联合成员是必需的。
                                    (Member type, required for struct and union members.) */
  char* default_value;          /**< 成员的默认值 (Default value for the member) */
  uint32_t index;               /**< 成员索引，适用于结构和联合成员。
                                   可以使用 DDS_DYNAMIC_MEMBER_INDEX_START 和 DDS_DYNAMIC_MEMBER_INDEX_END
                                   将成员添加为父类型中的第一个或最后一个成员。                    (Member index,
                                   applicable for struct               and union members.               DDS_DYNAMIC_MEMBER_INDEX_START and
                                   DDS_DYNAMIC_MEMBER_INDEX_END               can be used to                    add a member as
                                   first or last member in the parent type.) */
  uint32_t
      num_labels; /**< 标签数量，对于非默认标签的联合成员是必需的。
                     (Number of labels, required for union members in case not default_label) */
  int32_t* labels; /**< 联合成员的标签，对于非默认标签的联合成员需要 1..n 个标签。
                      (Labels for a union member, 1..n required for union members in case not
                      default_label) */
  bool default_label; /**< 是否为默认联合成员 (Is default union member) */
} dds_dynamic_member_descriptor_t;

/**
 * @ingroup dynamic_type
 *
 * 动态类型可扩展性 (Dynamic Type extensibility)
 */
enum dds_dynamic_type_extensibility {
  DDS_DYNAMIC_TYPE_EXT_FINAL,
  DDS_DYNAMIC_TYPE_EXT_APPENDABLE,
  DDS_DYNAMIC_TYPE_EXT_MUTABLE
};

/**
 * @ingroup dynamic_type
 *
 * 动态类型自动成员 ID 类型 (Dynamic Type automatic member ID kind)
 */
enum dds_dynamic_type_autoid {
  DDS_DYNAMIC_TYPE_AUTOID_SEQUENTIAL, /**< 成员 ID 按顺序分配 (The member ID are assigned
                                         sequential) */
  DDS_DYNAMIC_TYPE_AUTOID_HASH /**< 成员 ID 是成员名称的哈希值 (The member ID is the hash of the
                                  member's name) */
};

/**
 * @brief 枚举值类型 (Enum value kind)
 *
 * 参见 @ref dds_dynamic_enum_literal_value
 */
enum dds_dynamic_type_enum_value_kind {
  DDS_DYNAMIC_ENUM_LITERAL_VALUE_NEXT_AVAIL,
  DDS_DYNAMIC_ENUM_LITERAL_VALUE_EXPLICIT
};

/**
 * @ingroup dynamic_type
 *
 * 动态枚举类型文字值类型和值。可以设置为 NEXT_AVAIL，表示应为此成员使用当前最大值 +
 * 1，或者可以提供显式值。 (Dynamic Enumeration type literal value kind and value. Can be set to
 * NEXT_AVAIL to indicate that the current max value + 1 should be used for this member, or an
 * explicit value can be provided.)
 */
typedef struct dds_dynamic_enum_literal_value {
  enum dds_dynamic_type_enum_value_kind value_kind;
  int32_t value;
} dds_dynamic_enum_literal_value_t;

/**
 * @ingroup dynamic_type
 *
 * 初始化动态枚举值结构的简短表示法。
 * (Short notation for initializing a Dynamic Enum value struct.)
 */
#define DDS_DYNAMIC_ENUM_LITERAL_VALUE_AUTO \
  ((dds_dynamic_enum_literal_value_t){DDS_DYNAMIC_ENUM_LITERAL_VALUE_NEXT_AVAIL, 0})
#define DDS_DYNAMIC_ENUM_LITERAL_VALUE(v) \
  ((dds_dynamic_enum_literal_value_t){DDS_DYNAMIC_ENUM_LITERAL_VALUE_EXPLICIT, (v)})

/**
 * @ingroup dynamic_type
 *
 * 用于指示位掩码字段应获取下一个可用位置（当前最大值 + 1）。
 * (Used to indicate that the bitmask field should get the next available position (current maximum
 * + 1))
 */
#define DDS_DYNAMIC_BITMASK_POSITION_AUTO (UINT16_MAX)

/**
 * @brief 创建新的动态类型 (Create a new Dynamic Type)
 * @ingroup dynamic_type
 * @component dynamic_type_api
 *
 * 使用类型描述符中设置的属性创建新的动态类型。如果这些属性包括基本类型、元素类型或鉴别器类型，则这些类型的所有权将转移到新创建的类型。
 * (Creates a new Dynamic Type, using the properties that are set in the type descriptor.
 * In case these properties include a base-type, element-type or discriminator type, the
 * ownership of these types is transferred to the newly created type.)
 *
 * @param[in] entity 一个 DDS 实体（任何实体，除了由 DDS_CYCLONEDDS_HANDLE
 * 标识的伪根实体）。此实体用于获取实体域的类型库，以便将类型添加到其中。 (A DDS entity (any entity,
 * except the pseudo root entity identified by DDS_CYCLONEDDS_HANDLE). This entity is used to get
 * the type library of the entity's domain, to add the type to.)
 * @param[in] descriptor 动态类型描述符。 (The Dynamic Type descriptor.)
 *
 * @return dds_dynamic_type_t 为创建的类型创建的动态类型引用。 (A Dynamic Type reference for the
 * created type.)
 *
 * @retval DDS_RETCODE_OK
 *            类型已成功创建。 (The type is created successfully.)
 * @retval DDS_RETCODE_BAD_PARAMETER
 *            提供的一个或多个参数无效。 (One or more of the provided parameters are invalid.)
 * @retval DDS_RETCODE_UNSUPPORTED
 *            不支持提供的类型种类。 (The provided type kind is not supported.)
 * @retval DDS_RETCODE_OUT_OF_RESOURCES
 *            没有足够的资源来创建类型。 (Not enough resources to create the type.)
 */
DDS_EXPORT dds_dynamic_type_t dds_dynamic_type_create(dds_entity_t entity,
                                                      dds_dynamic_type_descriptor_t descriptor);

/**
 * @brief 设置动态类型的可扩展性 (Set the extensibility of a Dynamic Type)
 * @ingroup dynamic_type
 * @component dynamic_type_api
 *
 * @param[in,out] type
 * 要为其设置可扩展性的动态类型。这可以是结构、联合、位掩码或枚举类型。此类型必须处于构造状态，并且没有添加成员。
 *                     (Dynamic Type to set the extensibility for. This can be a structure, union,
 *                     bitmask or enum type. This type must be in the CONSTRUCTING state and have no
 * members added.)
 * @param[in] extensibility 要设置的可扩展性（@ref enum dds_dynamic_type_extensibility）。
 *                          (The extensibility to set (@ref enum dds_dynamic_type_extensibility).)
 *
 * @return dds_return_t 返回代码。如果出现错误，提供的类型中的返回代码字段也将设置为此值。
 *                     (Return code. In case of an error, the return code field in the provided type
 *                     is also set to this value.)
 *
 * @retval DDS_RETCODE_OK
 *            可扩展性已成功设置。 (The extensibility is set successfully.)
 * @retval DDS_RETCODE_BAD_PARAMETER
 *            提供的一个或多个参数无效。 (One or more of the provided parameters are invalid.)
 * @retval DDS_RETCODE_PRECONDITION_NOT_MET
 *            提供的类型
 */
DDS_EXPORT dds_return_t dds_dynamic_type_set_extensibility(
    dds_dynamic_type_t* type, enum dds_dynamic_type_extensibility extensibility);

/**
 * @brief 设置动态类型的位边界 (Set the bit-bound of a Dynamic Type)
 * @ingroup dynamic_type
 * @component dynamic_type_api
 *
 * @param[in,out] type 要为其设置位边界的动态类型。这可以是位掩码或枚举类型。(Dynamic Type to set
 * the bit-bound for. This can be a bitmask or enum type.)
 * @param[in] bit_bound
 * 要设置的位边界值，对于枚举类型，范围为1..32（包括），对于位掩码类型，范围为1..64（包括）。(The
 * bit-bound value to set, in the (including) range 1..32 for enum and 1..64 for bitmask.)
 *
 * @return dds_return_t 返回代码。如果出现错误，则在提供的类型中的返回代码字段也设置为此值。(Return
 * code. In case of an error, the return code field in the provided type is also set to this value.)
 *
 * @retval DDS_RETCODE_OK
 *            成功设置位边界。(The bit-bound is set successfully.)
 * @retval DDS_RETCODE_BAD_PARAMETER
 *            提供的一个或多个参数无效。(One or more of the provided parameters are invalid.)
 * @retval DDS_RETCODE_PRECONDITION_NOT_MET
 *            提供的类型不处于构造状态。(The provided type is not in the CONSTRUCTING state.)
 */
DDS_EXPORT dds_return_t dds_dynamic_type_set_bit_bound(dds_dynamic_type_t* type,
                                                       uint16_t bit_bound);

/**
 * @brief 设置动态类型的嵌套标志 (Set the nested flag of a Dynamic Type)
 * @ingroup dynamic_type
 * @component dynamic_type_api
 *
 * @param[in,out] type 要为其设置嵌套标志的动态类型。这可以是结构或联合类型。(Dynamic Type to set
 * the nested flag for. This can be a structure or union type.)
 * @param[in] is_nested 是否设置嵌套标志。(Whether the nested flag is set.)
 *
 * @return dds_return_t 返回代码。如果出现错误，则在提供的类型中的返回代码字段也设置为此值。(Return
 * code. In case of an error, the return code field in the provided type is also set to this value.)
 *
 * @retval DDS_RETCODE_OK
 *            成功设置标志。(The flag is set successfully.)
 * @retval DDS_RETCODE_BAD_PARAMETER
 *            提供的一个或多个参数无效。(One or more of the provided parameters are invalid.)
 * @retval DDS_RETCODE_PRECONDITION_NOT_MET
 *            提供的类型不处于构造状态。(The provided type is not in the CONSTRUCTING state.)
 */
DDS_EXPORT dds_return_t dds_dynamic_type_set_nested(dds_dynamic_type_t* type, bool is_nested);

/**
 * @brief 设置动态类型的自动ID种类 (Set the auto-id kind of a Dynamic Type)
 * @ingroup dynamic_type
 * @component dynamic_type_api
 *
 * @param[in,out] type 要为其设置自动ID种类的动态类型。这可以是结构或联合类型。(Dynamic Type to set
 * the auto-id kind for. This can be a structure or union type.)
 * @param[in] value 自动ID种类，参见@ref dds_dynamic_type_autoid。(The auto-id kind, see @ref
 * dds_dynamic_type_autoid.)
 *
 * @return dds_return_t 返回代码。如果出现错误，则在提供的类型中的返回代码字段也设置为此值。(Return
 * code. In case of an error, the return code field in the provided type is also set to this value.)
 *
 * @retval DDS_RETCODE_OK
 *            成功设置值。(The value is set successfully.)
 * @retval DDS_RETCODE_BAD_PARAMETER
 *            提供的一个或多个参数无效。(One or more of the provided parameters are invalid.)
 * @retval DDS_RETCODE_PRECONDITION_NOT_MET
 *            提供的类型不处于构造状态。(The provided type is not in the CONSTRUCTING state.)
 */
DDS_EXPORT dds_return_t dds_dynamic_type_set_autoid(dds_dynamic_type_t* type,
                                                    enum dds_dynamic_type_autoid value);

/**
 * @brief 向动态类型添加成员 (Add a member to a Dynamic Type)
 * @ingroup dynamic_type
 * @component dynamic_type_api
 *
 * 此函数用于向动态类型添加成员。父类型可以是结构、联合、枚举或位掩码类型。添加成员的父类型接管成员类型的所有权，并在删除时取消引用成员类型。
 * （@see dds_dynamic_type_ref以便重用类型）(This function is used to add a member to a Dynamic
 * Type. The parent type can be a structure, union, enumeration or bitmask type. The parent type the
 * member is added to takes over the ownership of the member type and dereferences the member type
 * when it is deleted. (@see dds_dynamic_type_ref for re-using a type))
 *
 * @param[in,out] type 要向其添加成员的动态类型。(The Dynamic type to add the member to.)
 * @param[in] member_descriptor 添加的成员的属性的成员描述符，@see
 * dds_dynamic_member_descriptor。(The member descriptor that has the properties of the member to
 * add, @see dds_dynamic_member_descriptor.)
 *
 * @return dds_return_t 返回代码。如果出现错误，则在提供的类型中的返回代码字段也设置为此值。(Return
 * code. In case of an error, the return code field in the provided type is also set to this value.)
 *
 * @retval DDS_RETCODE_OK
 *            成功添加成员。(The member is added successfully.)
 * @retval DDS_RETCODE_BAD_PARAMETER
 *            提供的一个或多个参数无效。(One or more of the provided parameters are invalid.)
 * @retval DDS_RETCODE_PRECONDITION_NOT_MET
 *            提供的类型（非成员类型）不处于构造状态。(The provided type (non the member type) is
 * not in the CONSTRUCTING state.)
 */
DDS_EXPORT dds_return_t dds_dynamic_type_add_member(
    dds_dynamic_type_t* type, dds_dynamic_member_descriptor_t member_descriptor);

/**
 * @brief 向动态枚举类型添加文字 (Add a literal to a Dynamic Enum Type)
 * @ingroup dynamic_type
 * @component dynamic_type_api
 *
 * 此函数用于向动态枚举类型添加文字。(This function is used to add a literal to a Dynamic Enum
 * Type.)
 *
 * @param[in,out] type 要向其添加成员的动态枚举类型。(The Dynamic enum type to add the member to.)
 * @param[in] name 要添加的文字的名称。(The name of the literal to add.)
 * @param[in] value 文字的值（@see dds_dynamic_enum_literal_value）。(The value for the literal
 * (@see dds_dynamic_enum_literal_value).)
 * @param[in] is_default 指示枚举是否默认文字。(Indicates if the literal if default for the enum.)
 *
 * @return dds_return_t 返回代码。如果出现错误，则在提供的类型中的返回代码字段也设置为此值。(Return
 * code. In case of an error, the return code field in the provided type is also set to this value.)
 *
 * @retval DDS_RETCODE_OK
 *            成功添加成员。(The member is added successfully)
 * @retval DDS_RETCODE_BAD_PARAMETER
 *            提供的一个或多个参数无效。(One or more of the provided parameters are invalid.)
 * @retval DDS_RETCODE_PRECONDITION_NOT_MET
 *            提供的类型不处于构造状态。(The provided type is not in the CONSTRUCTING state.)
 */
DDS_EXPORT dds_return_t dds_dynamic_type_add_enum_literal(dds_dynamic_type_t* type,
                                                          const char* name,
                                                          dds_dynamic_enum_literal_value_t value,
                                                          bool is_default);

/**
 * @brief 向动态位掩码类型添加字段 (Add a field to a Dynamic bitmask Type)
 * @ingroup dynamic_type
 * @component dynamic_type_api
 *
 * 此函数用于向动态位掩码类型添加字段。(This function is used to add a field to a Dynamic bitmask
 * Type.)
 *
 * @param[in,out] type 动态位掩码类型，用于添加字段。 (The Dynamic bitmask type to add the field
 * to.)
 * @param[in] name 要添加的字段的名称。 (The name of the field to add.)
 * @param[in] position 字段的位置 (@see DDS_DYNAMIC_BITMASK_POSITION_AUTO)。 (The position for the
 * field (@see DDS_DYNAMIC_BITMASK_POSITION_AUTO).)
 *
 * @return dds_return_t 返回代码。如果出现错误，提供的类型中的返回代码字段也将设置为此值。 (Return
 * code. In case of an error, the return code field in the provided type is also set to this value.)
 *
 * @retval DDS_RETCODE_OK
 *            成员添加成功。 (The member is added successfully.)
 * @retval DDS_RETCODE_BAD_PARAMETER
 *            提供的一个或多个参数无效。 (One or more of the provided parameters are invalid.)
 * @retval DDS_RETCODE_PRECONDITION_NOT_MET
 *            提供的类型不处于构造状态。 (The provided type is not in the CONSTRUCTING state.)
 */
DDS_EXPORT dds_return_t dds_dynamic_type_add_bitmask_field(dds_dynamic_type_t* type,
                                                           const char* name,
                                                           uint16_t position);

/**
 * @brief 为动态类型成员设置键标志 (Set the key flag for a Dynamic Type member)
 * @ingroup dynamic_type
 * @component dynamic_type_api
 *
 * @param[in,out] type 包含要为其设置键标志的成员的动态类型（必须是结构类型）。 (Dynamic Type that
 * contains the member to set the key flag for (must be a structure type).)
 * @param[in] member_id 要设置标志的成员的 ID。 (The ID of the member to set the flag for.)
 * @param[in] is_key 指示是否应设置或清除键标志。 (Indicates whether the key flag should be set or
 * cleared.)
 *
 * @return dds_return_t 返回代码。如果出现错误，提供的类型中的返回代码字段也将设置为此值。 (Return
 * code. In case of an error, the return code field in the provided type is also set to this value.)
 *
 * @retval DDS_RETCODE_OK
 *            标志更新成功。 (The flag is updated successfully.)
 * @retval DDS_RETCODE_BAD_PARAMETER
 *            提供的一个或多个参数无效。 (One or more of the provided parameters are invalid.)
 * @retval DDS_RETCODE_PRECONDITION_NOT_MET
 *            提供的类型不处于构造状态。 (The provided type is not in the CONSTRUCTING state.)
 */
DDS_EXPORT dds_return_t dds_dynamic_member_set_key(dds_dynamic_type_t* type,
                                                   uint32_t member_id,
                                                   bool is_key);

/**
 * @brief 为动态类型成员设置可选标志 (Set the optional flag for a Dynamic Type member)
 * @ingroup dynamic_type
 * @component dynamic_type_api
 *
 * @param[in,out] type 包含要为其设置可选标志的成员的动态类型（必须是结构类型）。 (Dynamic Type that
 * contains the member to set the optional flag for (must be a structure type).)
 * @param[in] member_id 要设置标志的成员的 ID。 (The ID of the member to set the flag for.)
 * @param[in] is_optional 指示是否应设置或清除可选标志。 (Indicates whether the optional flag should
 * be set or cleared.)
 *
 * @return dds_return_t 返回代码。如果出现错误，提供的类型中的返回代码字段也将设置为此值。 (Return
 * code. In case of an error, the return code field in the provided type is also set to this value.)
 *
 * @retval DDS_RETCODE_OK
 *            标志更新成功。 (The flag is updated successfully.)
 * @retval DDS_RETCODE_BAD_PARAMETER
 *            提供的一个或多个参数无效。 (One or more of the provided parameters are invalid.)
 * @retval DDS_RETCODE_PRECONDITION_NOT_MET
 *            提供的类型不处于构造状态。 (The provided type is not in the CONSTRUCTING state.)
 */
DDS_EXPORT dds_return_t dds_dynamic_member_set_optional(dds_dynamic_type_t* type,
                                                        uint32_t member_id,
                                                        bool is_optional);

/**
 * @brief 为动态类型成员设置外部标志 (Set the external flag for a Dynamic Type member)
 * @ingroup dynamic_type
 * @component dynamic_type_api
 *
 * @param[in,out] type 包含要为其设置外部标志的成员的动态类型（必须是结构或联合类型）。 (Dynamic
 * Type that contains the member to set the external flag for (must be a structure or union type).)
 * @param[in] member_id 要设置标志的成员的 ID。 (The ID of the member to set the flag for.)
 * @param[in] is_external 指示是否应设置或清除外部标志。 (Indicates whether the external flag should
 * be set or cleared.)
 *
 * @return dds_return_t 返回代码。如果出现错误，提供的类型中的返回代码字段也将设置为此值。 (Return
 * code. In case of an error, the return code field in the provided type is also set to this value.)
 *
 * @retval DDS_RETCODE_OK
 *            标志更新成功。 (The flag is updated successfully.)
 * @retval DDS_RETCODE_BAD_PARAMETER
 *            提供的一个或多个参数无效。 (One or more of the provided parameters are invalid.)
 * @retval DDS_RETCODE_PRECONDITION_NOT_MET
 *            提供的类型不处于构造状态。 (The provided type is not in the CONSTRUCTING state.)
 */
DDS_EXPORT dds_return_t dds_dynamic_member_set_external(dds_dynamic_type_t* type,
                                                        uint32_t member_id,
                                                        bool is_external);

/**
 * @brief 设置动态类型成员的哈希ID标志和哈希字段名称 (Set the hash ID flag and hash field name for a
 * Dynamic Type member)
 * @ingroup dynamic_type
 * @component dynamic_type_api
 *
 * @param[in,out] type 包含要为其设置标志和哈希名称的成员的动态类型（必须是结构或联合类型）(Dynamic
 * Type that contains the member to set the flag and hash-name for (must be a structure or union
 * type))
 * @param[in] member_id 要设置标志和哈希名称的成员的ID (The ID of the member to set the flag and
 * hash-name for)
 * @param[in] hash_member_name 用于计算成员ID的哈希名称 (The hash-name that should be used for
 * calculating the member ID)
 *
 * @return dds_return_t 返回代码。如果出现错误，还会将提供的类型中的返回代码字段设置为此值 (Return
 * code. In case of an error, the return code field in the provided type is also set to this value)
 *
 * @retval DDS_RETCODE_OK
 *            标志更新成功 (The flag is updated successfully)
 * @retval DDS_RETCODE_BAD_PARAMETER
 *            提供的一个或多个参数无效 (One or more of the provided parameters are invalid)
 * @retval DDS_RETCODE_PRECONDITION_NOT_MET
 *            提供的类型不处于CONSTRUCTING状态 (The provided type is not in the CONSTRUCTING state)
 */
DDS_EXPORT dds_return_t dds_dynamic_member_set_hashid(dds_dynamic_type_t* type,
                                                      uint32_t member_id,
                                                      const char* hash_member_name);

/**
 * @brief 为动态类型成员设置必须理解的标志 (Set the must-understand flag for a Dynamic Type member)
 * @ingroup dynamic_type
 * @component dynamic_type_api
 *
 * @param[in,out] type 包含要为其设置必须理解标志的成员的动态类型（必须是结构类型）(Dynamic Type
 * that contains the member to set the must-understand flag for (must be a structure type))
 * @param[in] member_id 要设置标志的成员的ID (The ID of the member to set the flag for)
 * @param[in] is_must_understand 指示是否应设置或清除必须理解标志 (Indicates whether the
 * must-understand flag should be set or cleared)
 *
 * @return dds_return_t 返回代码。如果出现错误，还会将提供的类型中的返回代码字段设置为此值 (Return
 * code. In case of an error, the return code field in the provided type is also set to this value)
 *
 * @retval DDS_RETCODE_OK
 *            标志更新成功 (The flag is updated successfully)
 * @retval DDS_RETCODE_BAD_PARAMETER
 *            提供的一个或多个参数无效 (One or more of the provided parameters are invalid)
 * @retval DDS_RETCODE_PRECONDITION_NOT_MET
 *            提供的类型不处于CONSTRUCTING状态 (The provided type is not in the CONSTRUCTING state)
 */
DDS_EXPORT dds_return_t dds_dynamic_member_set_must_understand(dds_dynamic_type_t* type,
                                                               uint32_t member_id,
                                                               bool is_must_understand);

/**
 * @brief 注册动态类型 (Registers a Dynamic Type)
 * @ingroup dynamic_type
 * @component dynamic_type_api
 *
 * 此函数注册动态类型，使其不可变并最终确定其定义。已注册的类型将获得“RESOLVED”状态，并存储在类型库中
 * (This function registers a dynamic type, making it immutable and finalizing its definition. A
 * type that is registered, get the state 'RESOLVED' and is stored in the type library)
 *
 * @param[in] type 要注册的动态类型的指针 (A pointer to the dynamic type to be registered)
 * @param[out] type_info 保存有关已注册类型信息的ddsi_typeinfo结构的指针的指针 (A pointer to a
 * pointer to a ddsi_typeinfo structure that holds information about the registered type)
 *
 * @return dds_return_t 返回代码 (Return code)
 *
 * @retval DDS_RETCODE_OK
 *            类型注册成功 (The type was successfully registered)
 * @retval DDS_RETCODE_BAD_PARAMETER
 *            提供的一个或多个参数无效 (One or more of the provided parameters are invalid)
 * @retval DDS_RETCODE_PRECONDITION_NOT_MET
 *            提供的类型不处于CONSTRUCTING状态 (The provided type is not in the CONSTRUCTING state)
 * @retval DDS_RETCODE_OUT_OF_RESOURCES
 *            没有足够的资源来创建类型 (Not enough resources to create the type)
 */
DDS_EXPORT dds_return_t dds_dynamic_type_register(dds_dynamic_type_t* type,
                                                  struct ddsi_typeinfo** type_info);

/**
 * @brief 引用动态类型 (Reference a Dynamic Type)
 * @ingroup dynamic_type
 * @component dynamic_type_api
 *
 * 引用动态类型并增加类型的引用计数。这可以用于在构造类型时重用子类型 (References a Dynamic Type and
 * increases the ref-count of the type. This can e.g. be used to re-use a subtype when constructing
 * a type)
 *
 * @param type 要引用的动态类型 (Dynamic Type to reference)
 *
 * @return dds_dynamic_type_t 增加了引用计数的动态类型 (Dynamic Type with increased ref-count)
 */
DDS_EXPORT dds_dynamic_type_t dds_dynamic_type_ref(dds_dynamic_type_t* type);

/**
 * @brief 取消引用动态类型 (Unref a Dynamic Type)
 *
 * @param type 要取消引用的动态类型 (The Dynamic Type to dereference)
 *
 * @return dds_return_t 返回代码 (Return code)
 *
 * @retval DDS_RETCODE_OK
 *            类型注册成功 (The type was successfully registered)
 * @retval DDS_RETCODE_BAD_PARAMETER
 *            提供的一个或多个参数无效 (One or more of the provided parameters are invalid)
 * @retval DDS_RETCODE_PRECONDITION_NOT_MET
 *            提供的类型不处于CONSTRUCTING状态 (The provided type is not in the CONSTRUCTING state)
 */
DDS_EXPORT dds_return_t dds_dynamic_type_unref(dds_dynamic_type_t* type);

/**
 * @brief 复制动态类型 (Duplicate a Dynamic Type)
 * @ingroup dynamic_type
 * @component dynamic_type_api
 *
 * 复制动态类型。类型的依赖项不会被复制，但它们的引用计数会增加 (Duplicates a Dynamic Type.
 * Dependencies of the type are not duplicated, but their ref-count is increased)
 *
 * @param src 要复制的类型 (The type to duplicate)
 *
 * @return dds_dynamic_type_t 源类型的副本 (A duplicate of the source type)
 */
DDS_EXPORT dds_dynamic_type_t dds_dynamic_type_dup(const dds_dynamic_type_t* src);

#if defined(__cplusplus)
}
#endif

#endif  // DDS_DYNAMIC_TYPE_H
