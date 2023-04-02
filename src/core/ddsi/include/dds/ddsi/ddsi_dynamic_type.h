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
#ifndef DDSI_DYNAMIC_TYPE_H
#define DDSI_DYNAMIC_TYPE_H

#include "dds/ddsi/ddsi_domaingv.h"
#include "dds/ddsi/ddsi_typewrap.h"
#include "dds/ddsrt/avl.h"
#include "dds/export.h"
#include "dds/features.h"

#if defined(__cplusplus)
extern "C" {
#endif

/** @struct ddsi_dynamic_type_struct_member_param
 *  @brief 结构体成员参数 (Structure member parameter)
 *
 *  此结构体用于描述动态类型结构体成员的参数。 (This structure is used to describe the parameters of
 * dynamic type structure members.)
 */
struct ddsi_dynamic_type_struct_member_param {
  uint32_t id;       ///< 成员的唯一标识符 (Unique identifier for the member)
  const char* name;  ///< 成员的名称 (Name of the member)
  uint32_t index;  ///< 成员在结构体中的索引位置 (Index position of the member in the structure)
  bool is_key;  ///< 标记该成员是否为键 (Flag indicating if the member is a key)
};

/** @struct ddsi_dynamic_type_union_member_param
 *  @brief 联合体成员参数 (Union member parameter)
 *
 *  此结构体用于描述动态类型联合体成员的参数。 (This structure is used to describe the parameters of
 * dynamic type union members.)
 */
struct ddsi_dynamic_type_union_member_param {
  uint32_t id;       ///< 成员的唯一标识符 (Unique identifier for the member)
  const char* name;  ///< 成员的名称 (Name of the member)
  uint32_t index;  ///< 成员在联合体中的索引位置 (Index position of the member in the union)
  bool is_default;  ///< 标记该成员是否为默认值 (Flag indicating if the member is a default value)
  uint32_t n_labels;  ///< 与此成员关联的标签数量 (Number of labels associated with this member)
  int32_t* labels;  ///< 指向与此成员关联的标签数组的指针 (Pointer to the array of labels associated
                    ///< with this member)
};

/** @struct ddsi_dynamic_type_enum_literal_param
 *  @brief 枚举字面量参数 (Enumeration literal parameter)
 *
 *  此结构体用于描述动态类型枚举字面量的参数。 (This structure is used to describe the parameters of
 * dynamic type enumeration literals.)
 */
struct ddsi_dynamic_type_enum_literal_param {
  const char* name;  ///< 字面量的名称 (Name of the literal)
  bool is_auto_value;  ///< 标记该字面量值是否为自动生成 (Flag indicating if the literal value is
                       ///< auto-generated)
  int32_t value;  ///< 字面量的值 (Value of the literal)
  bool
      is_default;  ///< 标记该字面量是否为默认值 (Flag indicating if the literal is a default value)
};

/** @struct ddsi_dynamic_type_bitmask_field_param
 *  @brief 位掩码字段参数 (Bitmask field parameter)
 *
 *  此结构体用于描述动态类型位掩码字段的参数。 (This structure is used to describe the parameters of
 * dynamic type bitmask fields.)
 */
struct ddsi_dynamic_type_bitmask_field_param {
  const char* name;  ///< 字段的名称 (Name of the field)
  bool is_auto_position;  ///< 标记该字段位置是否为自动生成 (Flag indicating if the field position
                          ///< is auto-generated)
  uint16_t position;  ///< 字段在位掩码中的位置 (Position of the field in the bitmask)
};

/** @component dynamic_type_support */
/**
 * @brief 创建结构体类型 (Create a struct type)
 *
 * @param[in] gv        全局变量指针 (Pointer to global variables)
 * @param[out] type     结构体类型指针的指针 (Pointer to the pointer of the struct type)
 * @param[in] type_name 类型名称 (Type name)
 * @param[in] base_type 基本类型指针的指针 (Pointer to the pointer of the base type)
 * @return 成功返回 DDS_RETCODE_OK，否则返回错误代码 (Returns DDS_RETCODE_OK on success, error code
 * otherwise)
 */
dds_return_t ddsi_dynamic_type_create_struct(struct ddsi_domaingv* gv,
                                             struct ddsi_type** type,
                                             const char* type_name,
                                             struct ddsi_type** base_type);

/** @component dynamic_type_support */
/**
 * @brief 创建联合体类型 (Create a union type)
 *
 * @param[in] gv                 全局变量指针 (Pointer to global variables)
 * @param[out] type              联合体类型指针的指针 (Pointer to the pointer of the union type)
 * @param[in] type_name          类型名称 (Type name)
 * @param[in] discriminant_type  判别式类型指针的指针 (Pointer to the pointer of the discriminant
 * type)
 * @return 成功返回 DDS_RETCODE_OK，否则返回错误代码 (Returns DDS_RETCODE_OK on success, error code
 * otherwise)
 */
dds_return_t ddsi_dynamic_type_create_union(struct ddsi_domaingv* gv,
                                            struct ddsi_type** type,
                                            const char* type_name,
                                            struct ddsi_type** discriminant_type);

/** @component dynamic_type_support */
/**
 * @brief 创建序列类型 (Create a sequence type)
 *
 * @param[in] gv           全局变量指针 (Pointer to global variables)
 * @param[out] type        序列类型指针的指针 (Pointer to the pointer of the sequence type)
 * @param[in] type_name    类型名称 (Type name)
 * @param[in] element_type 元素类型指针的指针 (Pointer to the pointer of the element type)
 * @param[in] bound        序列边界 (Sequence boundary)
 * @return 成功返回 DDS_RETCODE_OK，否则返回错误代码 (Returns DDS_RETCODE_OK on success, error code
 * otherwise)
 */
dds_return_t ddsi_dynamic_type_create_sequence(struct ddsi_domaingv* gv,
                                               struct ddsi_type** type,
                                               const char* type_name,
                                               struct ddsi_type** element_type,
                                               uint32_t bound);

/** @component dynamic_type_support */
/**
 * @brief 创建数组类型 (Create an array type)
 *
 * @param[in] gv           全局变量指针 (Pointer to global variables)
 * @param[out] type        数组类型指针的指针 (Pointer to the pointer of the array type)
 * @param[in] type_name    类型名称 (Type name)
 * @param[in] element_type 元素类型指针的指针 (Pointer to the pointer of the element type)
 * @param[in] num_bounds   边界数量 (Number of bounds)
 * @param[in] bounds       边界数组 (Array of bounds)
 * @return 成功返回 DDS_RETCODE_OK，否则返回错误代码 (Returns DDS_RETCODE_OK on success, error code
 * otherwise)
 */
dds_return_t ddsi_dynamic_type_create_array(struct ddsi_domaingv* gv,
                                            struct ddsi_type** type,
                                            const char* type_name,
                                            struct ddsi_type** element_type,
                                            uint32_t num_bounds,
                                            const uint32_t* bounds);

/** @component dynamic_type_support */
/**
 * @brief 创建枚举类型 (Create an enum type)
 *
 * @param[in] gv        全局变量指针 (Pointer to global variables)
 * @param[out] type     枚举类型指针的指针 (Pointer to the pointer of the enum type)
 * @param[in] type_name 类型名称 (Type name)
 * @return 成功返回 DDS_RETCODE_OK，否则返回错误代码 (Returns DDS_RETCODE_OK on success, error code
 * otherwise)
 */
dds_return_t ddsi_dynamic_type_create_enum(struct ddsi_domaingv* gv,
                                           struct ddsi_type** type,
                                           const char* type_name);

/** @component dynamic_type_support */
/**
 * @brief 创建位掩码类型 (Create a bitmask type)
 *
 * @param[in] gv        全局变量指针 (Pointer to global variables)
 * @param[out] type     位掩码类型指针的指针 (Pointer to the pointer of the bitmask type)
 * @param[in] type_name 类型名称 (Type name)
 * @return 成功返回 DDS_RETCODE_OK，否则返回错误代码 (Returns DDS_RETCODE_OK on success, error code
 * otherwise)
 */
dds_return_t ddsi_dynamic_type_create_bitmask(struct ddsi_domaingv* gv,
                                              struct ddsi_type** type,
                                              const char* type_name);

/** @component dynamic_type_support */
/**
 * @brief 创建别名类型 (Create an alias type)
 *
 * @param[in] gv           全局变量指针 (Pointer to global variables)
 * @param[out] type        别名类型指针的指针 (Pointer to the pointer of the alias type)
 * @param[in] type_name    类型名称 (Type name)
 * @param[in] aliased_type 被别名化的类型指针的指针 (Pointer to the pointer of the aliased type)
 * @return 成功返回 DDS_RETCODE_OK，否则返回错误代码 (Returns DDS_RETCODE_OK on success, error code
 * otherwise)
 */
dds_return_t ddsi_dynamic_type_create_alias(struct ddsi_domaingv* gv,
                                            struct ddsi_type** type,
                                            const char* type_name,
                                            struct ddsi_type** aliased_type);

/** @component dynamic_type_support */
/**
 * @brief 创建字符串类型 (Create a string8 type)
 *
 * @param[in] gv     全局变量指针 (Pointer to global variables)
 * @param[out] type  字符串类型指针的指针 (Pointer to the pointer of the string8 type)
 * @param[in] bound  字符串边界 (String boundary)
 * @return 成功返回 DDS_RETCODE_OK，否则返回错误代码 (Returns DDS_RETCODE_OK on success, error code
 * otherwise)
 */
dds_return_t ddsi_dynamic_type_create_string8(struct ddsi_domaingv* gv,
                                              struct ddsi_type** type,
                                              uint32_t bound);

/** @component dynamic_type_support */
/**
 * @brief 创建基本类型 (Create a primitive type)
 *
 * @param[in] gv    全局变量指针 (Pointer to global variables)
 * @param[out] type 基本类型指针的指针 (Pointer to the pointer of the primitive type)
 * @param[in] kind  动态类型种类 (Dynamic type kind)
 * @return 成功返回 DDS_RETCODE_OK，否则返回错误代码 (Returns DDS_RETCODE_OK on success, error code
 * otherwise)
 */
dds_return_t ddsi_dynamic_type_create_primitive(struct ddsi_domaingv* gv,
                                                struct ddsi_type** type,
                                                dds_dynamic_type_kind_t kind);

/**
 * @brief 设置动态类型的扩展性 (Set the extensibility of a dynamic type)
 * @param[in] type 动态类型指针 (Pointer to the dynamic type)
 * @param[in] extensibility 扩展性枚举值 (Extensibility enumeration value)
 * @return 成功返回 DDS_RETCODE_OK，失败返回相应错误代码 (Returns DDS_RETCODE_OK on success,
 * appropriate error code on failure)
 */
dds_return_t ddsi_dynamic_type_set_extensibility(struct ddsi_type* type,
                                                 enum dds_dynamic_type_extensibility extensibility);

/**
 * @brief 设置动态类型的位边界 (Set the bit boundary of a dynamic type)
 * @param[in] type 动态类型指针 (Pointer to the dynamic type)
 * @param[in] bit_bound 位边界值 (Bit boundary value)
 * @return 成功返回 DDS_RETCODE_OK，失败返回相应错误代码 (Returns DDS_RETCODE_OK on success,
 * appropriate error code on failure)
 */
dds_return_t ddsi_dynamic_type_set_bitbound(struct ddsi_type* type, uint16_t bit_bound);

/**
 * @brief 设置动态类型是否嵌套 (Set whether a dynamic type is nested)
 * @param[in] type 动态类型指针 (Pointer to the dynamic type)
 * @param[in] is_nested 是否嵌套布尔值 (Boolean value indicating whether the type is nested)
 * @return 成功返回 DDS_RETCODE_OK，失败返回相应错误代码 (Returns DDS_RETCODE_OK on success,
 * appropriate error code on failure)
 */
dds_return_t ddsi_dynamic_type_set_nested(struct ddsi_type* type, bool is_nested);

/**
 * @brief 设置动态类型的 autoid 值 (Set the autoid value of a dynamic type)
 * @param[in] type 动态类型指针 (Pointer to the dynamic type)
 * @param[in] value autoid 枚举值 (Autoid enumeration value)
 * @return 成功返回 DDS_RETCODE_OK，失败返回相应错误代码 (Returns DDS_RETCODE_OK on success,
 * appropriate error code on failure)
 */
dds_return_t ddsi_dynamic_type_set_autoid(struct ddsi_type* type,
                                          enum dds_dynamic_type_autoid value);

/**
 * @brief 为结构体动态类型添加成员 (Add a member to a struct dynamic type)
 * @param[in] type 结构体动态类型指针 (Pointer to the struct dynamic type)
 * @param[in,out] member_type 成员类型指针 (Pointer to the member type)
 * @param[in] params 成员参数 (Member parameters)
 * @return 成功返回 DDS_RETCODE_OK，失败返回相应错误代码 (Returns DDS_RETCODE_OK on success,
 * appropriate error code on failure)
 */
dds_return_t ddsi_dynamic_type_add_struct_member(
    struct ddsi_type* type,
    struct ddsi_type** member_type,
    struct ddsi_dynamic_type_struct_member_param params);

/**
 * @brief 为联合体动态类型添加成员 (Add a member to a union dynamic type)
 * @param[in] type 联合体动态类型指针 (Pointer to the union dynamic type)
 * @param[in,out] member_type 成员类型指针 (Pointer to the member type)
 * @param[in] params 成员参数 (Member parameters)
 * @return 成功返回 DDS_RETCODE_OK，失败返回相应错误代码 (Returns DDS_RETCODE_OK on success,
 * appropriate error code on failure)
 */
dds_return_t ddsi_dynamic_type_add_union_member(struct ddsi_type* type,
                                                struct ddsi_type** member_type,
                                                struct ddsi_dynamic_type_union_member_param params);

/**
 * @brief 为枚举动态类型添加字面量 (Add a literal to an enum dynamic type)
 * @param[in] type 枚举动态类型指针 (Pointer to the enum dynamic type)
 * @param[in] params 字面量参数 (Literal parameters)
 * @return 成功返回 DDS_RETCODE_OK，失败返回相应错误代码 (Returns DDS_RETCODE_OK on success,
 * appropriate error code on failure)
 */
dds_return_t ddsi_dynamic_type_add_enum_literal(struct ddsi_type* type,
                                                struct ddsi_dynamic_type_enum_literal_param params);

/**
 * @brief 为位掩码动态类型添加字段 (Add a field to a bitmask dynamic type)
 * @param[in] type 位掩码动态类型指针 (Pointer to the bitmask dynamic type)
 * @param[in] params 字段参数 (Field parameters)
 * @return 成功返回 DDS_RETCODE_OK，失败返回相应错误代码 (Returns DDS_RETCODE_OK on success,
 * appropriate error code on failure)
 */
dds_return_t ddsi_dynamic_type_add_bitmask_field(
    struct ddsi_type* type, struct ddsi_dynamic_type_bitmask_field_param params);

/**
 * @brief 设置动态类型成员的 key 属性 (Set the key attribute of a dynamic type member)
 * @param[in] type 动态类型指针 (Pointer to the dynamic type)
 * @param[in] member_id 成员 ID (Member ID)
 * @param[in] is_key 是否为 key 布尔值 (Boolean value indicating whether the member is a key)
 * @return 成功返回 DDS_RETCODE_OK，失败返回相应错误代码 (Returns DDS_RETCODE_OK on success,
 * appropriate error code on failure)
 */
dds_return_t ddsi_dynamic_type_member_set_key(struct ddsi_type* type,
                                              uint32_t member_id,
                                              bool is_key);

/**
 * @brief 设置动态类型成员的可选属性 (Set the optional attribute of a dynamic type member)
 * @param[in] type 动态类型指针 (Pointer to the dynamic type)
 * @param[in] member_id 成员 ID (Member ID)
 * @param[in] is_optional 是否为可选布尔值 (Boolean value indicating whether the member is optional)
 * @return 成功返回 DDS_RETCODE_OK，失败返回相应错误代码 (Returns DDS_RETCODE_OK on success,
 * appropriate error code on failure)
 */
dds_return_t ddsi_dynamic_type_member_set_optional(struct ddsi_type* type,
                                                   uint32_t member_id,
                                                   bool is_optional);

/**
 * @component dynamic_type_support
 * @brief 设置结构体成员是否为外部类型
 * @param type 结构体类型指针
 * @param member_id 成员编号
 * @param is_external 是否为外部类型
 * @return dds_return_t 操作结果
 */
// Set whether the struct member is an external type
dds_return_t ddsi_dynamic_struct_member_set_external(struct ddsi_type* type,
                                                     uint32_t member_id,
                                                     bool is_external);

/**
 * @component dynamic_type_support
 * @brief 设置联合体成员是否为外部类型
 * @param type 联合体类型指针
 * @param member_id 成员编号
 * @param is_external 是否为外部类型
 * @return dds_return_t 操作结果
 */
// Set whether the union member is an external type
dds_return_t ddsi_dynamic_union_member_set_external(struct ddsi_type* type,
                                                    uint32_t member_id,
                                                    bool is_external);

/**
 * @component dynamic_type_support
 * @brief 设置类型成员是否必须理解
 * @param type 类型指针
 * @param member_id 成员编号
 * @param is_must_understand 是否必须理解
 * @return dds_return_t 操作结果
 */
// Set whether the type member must be understood
dds_return_t ddsi_dynamic_type_member_set_must_understand(struct ddsi_type* type,
                                                          uint32_t member_id,
                                                          bool is_must_understand);

/**
 * @component dynamic_type_support
 * @brief 设置类型成员的哈希ID
 * @param type 类型指针
 * @param member_id 成员编号
 * @param hash_member_name 成员名称哈希值
 * @return dds_return_t 操作结果
 */
// Set the hash ID of the type member
dds_return_t ddsi_dynamic_type_member_set_hashid(struct ddsi_type* type,
                                                 uint32_t member_id,
                                                 const char* hash_member_name);

/**
 * @component dynamic_type_support
 * @brief 注册动态类型
 * @param type 类型指针的地址
 * @param type_info 类型信息指针的地址
 * @return dds_return_t 操作结果
 */
// Register the dynamic type
dds_return_t ddsi_dynamic_type_register(struct ddsi_type** type, ddsi_typeinfo_t** type_info);

/**
 * @component dynamic_type_support
 * @brief 引用动态类型
 * @param type 类型指针
 * @return struct ddsi_type* 引用后的类型指针
 */
// Reference the dynamic type
struct ddsi_type* ddsi_dynamic_type_ref(struct ddsi_type* type);

/**
 * @component dynamic_type_support
 * @brief 取消引用动态类型
 * @param type 类型指针
 */
// Unreference the dynamic type
void ddsi_dynamic_type_unref(struct ddsi_type* type);

/**
 * @component dynamic_type_support
 * @brief 复制动态类型
 * @param src 源类型指针
 * @return struct ddsi_type* 复制后的类型指针
 */
// Duplicate the dynamic type
struct ddsi_type* ddsi_dynamic_type_dup(const struct ddsi_type* src);

/**
 * @component dynamic_type_support
 * @brief 判断动态类型是否正在构建中
 * @param type 类型指针
 * @return bool 是否正在构建中
 */
// Check if the dynamic type is under construction
bool ddsi_dynamic_type_is_constructing(const struct ddsi_type* type);

#if defined(__cplusplus)
}
#endif

#endif /* DDSI_DYNAMIC_TYPE_H */
