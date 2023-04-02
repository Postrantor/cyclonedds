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

#include <assert.h>
#include <string.h>

#include "dds/dds.h"
#include "dds/ddsi/ddsi_dynamic_type.h"
#include "dds/ddsi/ddsi_entity.h"
#include "dds/ddsi/ddsi_typelib.h"
#include "dds__entity.h"

/**
 * @brief 获取实体的全局视图 (Get the global view of an entity)
 *
 * @param[in]  entity 实体标识符 (Entity identifier)
 * @param[out] gv     指向实体全局视图的指针的指针 (Pointer to a pointer to the entity's global
 * view)
 *
 * @return 返回操作结果，成功返回 DDS_RETCODE_OK，否则返回相应错误代码 (Returns the operation
 * result, returns DDS_RETCODE_OK on success, otherwise returns the corresponding error code)
 */
static dds_return_t get_entity_gv(dds_entity_t entity, struct ddsi_domaingv **gv) {
  // 定义返回值变量 (Define return value variable)
  dds_return_t ret;

  // 定义实体指针 (Define entity pointer)
  struct dds_entity *e;

  // 尝试获取实体并检查返回值 (Try to get the entity and check the return value)
  if ((ret = dds_entity_pin(entity, &e)) == DDS_RETCODE_OK) {
    // 判断实体类型是否为 CycloneDDS (Check if the entity type is CycloneDDS)
    if (e->m_kind == DDS_KIND_CYCLONEDDS)
      // 如果是，则设置错误参数 (If it is, set the error parameter)
      ret = DDS_RETCODE_BAD_PARAMETER;
    else
      // 否则，将实体的全局视图赋值给输出参数 (Otherwise, assign the entity's global view to the
      // output parameter)
      *gv = &e->m_domain->gv;

    // 解除实体引用 (Unpin the entity)
    dds_entity_unpin(e);
  }

  // 返回操作结果 (Return the operation result)
  return ret;
}

/**
 * @brief 将dds_dynamic_type_kind_t类型转换为DDS_XTypes_TypeKind类型
 *
 * @param type_kind 一个dds_dynamic_type_kind_t枚举值
 * @return 对应的DDS_XTypes_TypeKind枚举值
 */
static DDS_XTypes_TypeKind typekind_to_xtkind(dds_dynamic_type_kind_t type_kind) {
  // 使用switch语句进行类型转换
  switch (type_kind) {
    case DDS_DYNAMIC_NONE:  // 无类型
      return DDS_XTypes_TK_NONE;
    case DDS_DYNAMIC_BOOLEAN:  // 布尔类型
      return DDS_XTypes_TK_BOOLEAN;
    case DDS_DYNAMIC_BYTE:  // 字节类型
      return DDS_XTypes_TK_BYTE;
    case DDS_DYNAMIC_INT16:  // 16位整型
      return DDS_XTypes_TK_INT16;
    case DDS_DYNAMIC_INT32:  // 32位整型
      return DDS_XTypes_TK_INT32;
    case DDS_DYNAMIC_INT64:  // 64位整型
      return DDS_XTypes_TK_INT64;
    case DDS_DYNAMIC_UINT16:  // 16位无符号整型
      return DDS_XTypes_TK_UINT16;
    case DDS_DYNAMIC_UINT32:  // 32位无符号整型
      return DDS_XTypes_TK_UINT32;
    case DDS_DYNAMIC_UINT64:  // 64位无符号整型
      return DDS_XTypes_TK_UINT64;
    case DDS_DYNAMIC_FLOAT32:  // 32位浮点型
      return DDS_XTypes_TK_FLOAT32;
    case DDS_DYNAMIC_FLOAT64:  // 64位浮点型
      return DDS_XTypes_TK_FLOAT64;
    case DDS_DYNAMIC_FLOAT128:  // 128位浮点型
      return DDS_XTypes_TK_FLOAT128;
    case DDS_DYNAMIC_INT8:  // 8位整型
      return DDS_XTypes_TK_INT8;
    case DDS_DYNAMIC_UINT8:  // 8位无符号整型
      return DDS_XTypes_TK_UINT8;
    case DDS_DYNAMIC_CHAR8:  // 8位字符
      return DDS_XTypes_TK_CHAR8;
    case DDS_DYNAMIC_CHAR16:  // 16位字符
      return DDS_XTypes_TK_CHAR16;
    case DDS_DYNAMIC_STRING8:  // 8位字符串
      return DDS_XTypes_TK_STRING8;
    case DDS_DYNAMIC_STRING16:  // 16位字符串
      return DDS_XTypes_TK_STRING16;
    case DDS_DYNAMIC_ENUMERATION:  // 枚举类型
      return DDS_XTypes_TK_ENUM;
    case DDS_DYNAMIC_BITMASK:  // 位掩码类型
      return DDS_XTypes_TK_BITMASK;
    case DDS_DYNAMIC_ALIAS:  // 别名类型
      return DDS_XTypes_TK_ALIAS;
    case DDS_DYNAMIC_ARRAY:  // 数组类型
      return DDS_XTypes_TK_ARRAY;
    case DDS_DYNAMIC_SEQUENCE:  // 序列类型
      return DDS_XTypes_TK_SEQUENCE;
    case DDS_DYNAMIC_MAP:  // 映射类型
      return DDS_XTypes_TK_MAP;
    case DDS_DYNAMIC_STRUCTURE:  // 结构体类型
      return DDS_XTypes_TK_STRUCTURE;
    case DDS_DYNAMIC_UNION:  // 联合体类型
      return DDS_XTypes_TK_UNION;
    case DDS_DYNAMIC_BITSET:  // 位集类型
      return DDS_XTypes_TK_BITSET;
  }
  // 如果没有匹配的类型，返回无类型
  return DDS_XTypes_TK_NONE;
}

/**
 * @brief 将 DDS_XTypes_TypeKind 类型转换为 dds_dynamic_type_kind_t 类型
 *
 * @param xt_kind 一个 DDS_XTypes_TypeKind 枚举值
 * @return 对应的 dds_dynamic_type_kind_t 枚举值
 */
static dds_dynamic_type_kind_t xtkind_to_typekind(DDS_XTypes_TypeKind xt_kind) {
  // 使用 switch 语句处理不同的 xt_kind 值
  switch (xt_kind) {
    case DDS_XTypes_TK_BOOLEAN:  // 处理布尔类型
      return DDS_DYNAMIC_BOOLEAN;
    case DDS_XTypes_TK_BYTE:  // 处理字节类型
      return DDS_DYNAMIC_BYTE;
    case DDS_XTypes_TK_INT16:  // 处理 16 位整数类型
      return DDS_DYNAMIC_INT16;
    case DDS_XTypes_TK_INT32:  // 处理 32 位整数类型
      return DDS_DYNAMIC_INT32;
    case DDS_XTypes_TK_INT64:  // 处理 64 位整数类型
      return DDS_DYNAMIC_INT64;
    case DDS_XTypes_TK_UINT16:  // 处理无符号 16 位整数类型
      return DDS_DYNAMIC_UINT16;
    case DDS_XTypes_TK_UINT32:  // 处理无符号 32 位整数类型
      return DDS_DYNAMIC_UINT32;
    case DDS_XTypes_TK_UINT64:  // 处理无符号 64 位整数类型
      return DDS_DYNAMIC_UINT64;
    case DDS_XTypes_TK_FLOAT32:  // 处理单精度浮点数类型
      return DDS_DYNAMIC_FLOAT32;
    case DDS_XTypes_TK_FLOAT64:  // 处理双精度浮点数类型
      return DDS_DYNAMIC_FLOAT64;
    case DDS_XTypes_TK_FLOAT128:  // 处理 128 位浮点数类型
      return DDS_DYNAMIC_FLOAT128;
    case DDS_XTypes_TK_INT8:  // 处理 8 位整数类型
      return DDS_DYNAMIC_INT8;
    case DDS_XTypes_TK_UINT8:  // 处理无符号 8 位整数类型
      return DDS_DYNAMIC_UINT8;
    case DDS_XTypes_TK_CHAR8:  // 处理 8 位字符类型
      return DDS_DYNAMIC_CHAR8;
    case DDS_XTypes_TK_CHAR16:  // 处理 16 位字符类型
      return DDS_DYNAMIC_CHAR16;
    case DDS_XTypes_TK_STRING8:  // 处理 8 位字符串类型
      return DDS_DYNAMIC_STRING8;
    case DDS_XTypes_TK_STRING16:  // 处理 16 位字符串类型
      return DDS_DYNAMIC_STRING16;
    case DDS_XTypes_TK_ENUM:  // 处理枚举类型
      return DDS_DYNAMIC_ENUMERATION;
    case DDS_XTypes_TK_BITMASK:  // 处理位掩码类型
      return DDS_DYNAMIC_BITMASK;
    case DDS_XTypes_TK_ALIAS:  // 处理别名类型
      return DDS_DYNAMIC_ALIAS;
    case DDS_XTypes_TK_ARRAY:  // 处理数组类型
      return DDS_DYNAMIC_ARRAY;
    case DDS_XTypes_TK_SEQUENCE:  // 处理序列类型
      return DDS_DYNAMIC_SEQUENCE;
    case DDS_XTypes_TK_MAP:  // 处理映射类型
      return DDS_DYNAMIC_MAP;
    case DDS_XTypes_TK_STRUCTURE:  // 处理结构体类型
      return DDS_DYNAMIC_STRUCTURE;
    case DDS_XTypes_TK_UNION:  // 处理联合体类型
      return DDS_DYNAMIC_UNION;
    case DDS_XTypes_TK_BITSET:  // 处理位集类型
      return DDS_DYNAMIC_BITSET;
  }
  // 如果没有匹配到任何类型，返回 DDS_DYNAMIC_NONE
  return DDS_DYNAMIC_NONE;
}

/**
 * @brief 从类型规范创建动态类型 (Create a dynamic type from a type specification)
 *
 * @param[in] gv        域全局变量指针 (Pointer to the domain global variables)
 * @param[in] type_spec 类型规范 (Type specification)
 *
 * @return 动态类型结构体 (Dynamic type structure)
 */
static dds_dynamic_type_t dyntype_from_typespec(struct ddsi_domaingv *gv,
                                                dds_dynamic_type_spec_t type_spec) {
  // 根据类型规范的种类进行处理 (Process according to the kind of type specification)
  switch (type_spec.kind) {
    case DDS_DYNAMIC_TYPE_KIND_UNSET:
      // 返回一个状态为DDS_RETCODE_OK的动态类型结构体 (Return a dynamic type structure with a status
      // of DDS_RETCODE_OK)
      return (dds_dynamic_type_t){.ret = DDS_RETCODE_OK};
    case DDS_DYNAMIC_TYPE_KIND_PRIMITIVE: {
      dds_dynamic_type_t type;
      // 创建基本类型并返回 (Create a primitive type and return)
      type.ret = ddsi_dynamic_type_create_primitive(gv, (struct ddsi_type **)&type.x,
                                                    typekind_to_xtkind(type_spec.type.primitive));
      return type;
    }
    case DDS_DYNAMIC_TYPE_KIND_DEFINITION:
      // 返回类型定义 (Return the type definition)
      return type_spec.type.type;
  }

  // 返回一个状态为DDS_RETCODE_BAD_PARAMETER的动态类型结构体 (Return a dynamic type structure with a
  // status of DDS_RETCODE_BAD_PARAMETER)
  return (dds_dynamic_type_t){.ret = DDS_RETCODE_BAD_PARAMETER};
}

/**
 * @brief 检查类型规范是否有效 (Check if the type specification is valid)
 *
 * @param[in] type_spec   类型规范 (Type specification)
 * @param[in] allow_unset 是否允许未设置的类型 (Whether to allow unset types)
 *
 * @return 类型规范是否有效 (Whether the type specification is valid)
 */
static bool typespec_valid(dds_dynamic_type_spec_t type_spec, bool allow_unset) {
  // 根据类型规范的种类进行处理 (Process according to the kind of type specification)
  switch (type_spec.kind) {
    case DDS_DYNAMIC_TYPE_KIND_UNSET:
      // 返回是否允许未设置的类型 (Return whether to allow unset types)
      return allow_unset;
    case DDS_DYNAMIC_TYPE_KIND_PRIMITIVE:
      // 检查基本类型是否在有效范围内 (Check if the primitive type is within the valid range)
      return type_spec.type.primitive >= DDS_DYNAMIC_BOOLEAN &&
             type_spec.type.primitive <= DDS_DYNAMIC_CHAR16;
    case DDS_DYNAMIC_TYPE_KIND_DEFINITION:
      // 检查类型定义是否有效 (Check if the type definition is valid)
      return type_spec.type.type.ret == DDS_RETCODE_OK && type_spec.type.type.x != NULL;
  }
  // 如果没有匹配的类型规范种类，返回false (If there is no matching type specification kind, return
  // false)
  return false;
}

/**
 * @brief 检查给定的类型规范是否为有效的联合体判别器。
 *
 * @param type_spec 类型规范。
 * @return 如果类型规范是有效的联合体判别器，则返回 true，否则返回 false。
 */
static bool union_disc_valid(dds_dynamic_type_spec_t type_spec) {
  // 根据类型规范的 kind 字段进行分类处理
  switch (type_spec.kind) {
    case DDS_DYNAMIC_TYPE_KIND_UNSET:
      // 如果类型未设置，则返回 false
      return false;
    case DDS_DYNAMIC_TYPE_KIND_PRIMITIVE:
      // 如果类型是基本类型，检查是否为允许的基本类型之一
      return type_spec.type.primitive == DDS_DYNAMIC_BOOLEAN ||
             type_spec.type.primitive == DDS_DYNAMIC_BYTE ||
             type_spec.type.primitive == DDS_DYNAMIC_INT8 ||
             type_spec.type.primitive == DDS_DYNAMIC_INT16 ||
             type_spec.type.primitive == DDS_DYNAMIC_INT32 ||
             type_spec.type.primitive == DDS_DYNAMIC_INT64 ||
             type_spec.type.primitive == DDS_DYNAMIC_UINT8 ||
             type_spec.type.primitive == DDS_DYNAMIC_UINT16 ||
             type_spec.type.primitive == DDS_DYNAMIC_UINT32 ||
             type_spec.type.primitive == DDS_DYNAMIC_UINT64 ||
             type_spec.type.primitive == DDS_DYNAMIC_CHAR8 ||
             type_spec.type.primitive == DDS_DYNAMIC_CHAR16;
    case DDS_DYNAMIC_TYPE_KIND_DEFINITION: {
      // 如果类型是定义类型，检查其 ret 是否为 DDS_RETCODE_OK，以及 x 是否非空
      if (type_spec.type.type.ret != DDS_RETCODE_OK || type_spec.type.type.x == NULL) return false;
      // 获取类型的 XTypes 类型种类
      DDS_XTypes_TypeKind xtkind = ddsi_type_get_kind((struct ddsi_type *)type_spec.type.type.x);
      // 检查类型种类是否为枚举或别名
      return xtkind == DDS_XTypes_TK_ENUM || xtkind == DDS_XTypes_TK_ALIAS;
    }
  }
  // 其他情况返回 false
  return false;
}

/**
 * @brief 检查给定的名称是否为有效的类型名称。
 *
 * @param name 类型名称。
 * @return 如果名称有效，则返回 true，否则返回 false。
 */
static bool typename_valid(const char *name) {
  // 获取名称长度
  size_t len = strlen(name);
  // 检查名称长度是否在允许的范围内
  return len > 0 && len < (sizeof(DDS_XTypes_QualifiedTypeName) - 1);
}

/**
 * @brief 检查给定的名称是否为有效的成员名称。
 *
 * @param name 成员名称。
 * @return 如果名称有效，则返回 true，否则返回 false。
 */
static bool membername_valid(const char *name) {
  // 获取名称长度
  size_t len = strlen(name);
  // 检查名称长度是否在允许的范围内
  return len > 0 && len < (sizeof(DDS_XTypes_MemberName) - 1);
}

/**
 * @brief 创建动态类型 (Create a dynamic type)
 *
 * @param entity 实体 (Entity)
 * @param descriptor 动态类型描述符 (Dynamic type descriptor)
 * @return dds_dynamic_type_t 动态类型 (Dynamic type)
 */
dds_dynamic_type_t dds_dynamic_type_create(dds_entity_t entity,
                                           dds_dynamic_type_descriptor_t descriptor) {
  // 初始化动态类型结构体 (Initialize the dynamic type structure)
  dds_dynamic_type_t type = {.x = NULL};
  struct ddsi_domaingv *gv;

  // 获取实体的全局变量 (Get the global variable of the entity)
  if ((type.ret = get_entity_gv(entity, &gv)) != DDS_RETCODE_OK) goto err;

  // 根据描述符的类型创建相应的动态类型 (Create the corresponding dynamic type according to the type
  // of the descriptor)
  switch (descriptor.kind) {
    case DDS_DYNAMIC_NONE:
      goto err_bad_param;

    // 基本类型 (Primitive types)
    case DDS_DYNAMIC_BOOLEAN:
    case DDS_DYNAMIC_BYTE:
    case DDS_DYNAMIC_INT16:
    case DDS_DYNAMIC_INT32:
    case DDS_DYNAMIC_INT64:
    case DDS_DYNAMIC_UINT16:
    case DDS_DYNAMIC_UINT32:
    case DDS_DYNAMIC_UINT64:
    case DDS_DYNAMIC_FLOAT32:
    case DDS_DYNAMIC_FLOAT64:
    case DDS_DYNAMIC_FLOAT128:
    case DDS_DYNAMIC_INT8:
    case DDS_DYNAMIC_UINT8:
    case DDS_DYNAMIC_CHAR8:
      type.ret =
          ddsi_dynamic_type_create_primitive(gv, (struct ddsi_type **)&type.x, descriptor.kind);
      break;
    // 字符串类型 (String type)
    case DDS_DYNAMIC_STRING8:
      if (descriptor.num_bounds > 1) goto err_bad_param;
      type.ret = ddsi_dynamic_type_create_string8(gv, (struct ddsi_type **)&type.x,
                                                  descriptor.num_bounds ? descriptor.bounds[0] : 0);
      break;
    // 别名类型 (Alias type)
    case DDS_DYNAMIC_ALIAS: {
      if (!typespec_valid(descriptor.base_type, false) || !typename_valid(descriptor.name))
        goto err_bad_param;
      dds_dynamic_type_t aliased_type = dyntype_from_typespec(gv, descriptor.base_type);
      type.ret = ddsi_dynamic_type_create_alias(gv, (struct ddsi_type **)&type.x, descriptor.name,
                                                (struct ddsi_type **)&aliased_type.x);
      break;
    }
    // 枚举类型 (Enumeration type)
    case DDS_DYNAMIC_ENUMERATION:
      if (!typename_valid(descriptor.name)) goto err_bad_param;
      type.ret = ddsi_dynamic_type_create_enum(gv, (struct ddsi_type **)&type.x, descriptor.name);
      break;
    // 位掩码类型 (Bitmask type)
    case DDS_DYNAMIC_BITMASK:
      if (!typename_valid(descriptor.name)) goto err_bad_param;
      type.ret =
          ddsi_dynamic_type_create_bitmask(gv, (struct ddsi_type **)&type.x, descriptor.name);
      break;
    // 数组类型 (Array type)
    case DDS_DYNAMIC_ARRAY: {
      /**
       * @brief 检查类型规范和类型名是否有效，创建一个动态数组类型。
       *
       * 这段代码首先检查给定的类型规范（descriptor.element_type）和类型名（descriptor.name）是否有效。
       * 然后，根据类型规范创建一个动态元素类型。最后，使用这个元素类型、类型名和边界信息创建一个动态数组类型。
       *
       * @param[in] descriptor 类型描述符，包含元素类型、类型名和边界信息。
       * @param[out] type.ret 创建的动态数组类型。
       * @return 如果参数无效或创建过程中出现错误，跳转到 err_bad_param 标签。
       */
      if (!typespec_valid(descriptor.element_type, false) ||  // 检查类型规范是否有效
          !typename_valid(descriptor.name) ||                 // 检查类型名是否有效
          descriptor.num_bounds == 0 ||                       // 检查边界数量是否为0
          descriptor.bounds == NULL)                          // 检查边界数组是否为空
        goto err_bad_param;

      // 遍历边界数组，检查每个边界值是否为0
      for (uint32_t n = 0; n < descriptor.num_bounds; n++)
        if (descriptor.bounds[n] == 0) goto err_bad_param;

      // 根据类型规范创建动态元素类型
      dds_dynamic_type_t element_type = dyntype_from_typespec(gv, descriptor.element_type);

      // 使用元素类型、类型名和边界信息创建动态数组类型
      type.ret =
          ddsi_dynamic_type_create_array(gv,                            // 全局变量
                                         (struct ddsi_type **)&type.x,  // 动态类型指针
                                         descriptor.name,               // 类型名
                                         (struct ddsi_type **)&element_type.x,  // 元素类型指针
                                         descriptor.num_bounds,                 // 边界数量
                                         descriptor.bounds);                    // 边界数组

      break;
    }
    // 序列类型 (Sequence type)
    case DDS_DYNAMIC_SEQUENCE: {
      /**
       * @brief 检查类型规范和类型名是否有效，创建一个动态序列类型。
       *
       * 这段代码首先检查给定的类型规范（descriptor.element_type）和类型名（descriptor.name）是否有效。
       * 然后，根据类型规范创建一个动态元素类型。最后，使用这个元素类型、类型名和边界信息创建一个动态序列类型。
       *
       * @param[in] descriptor 类型描述符，包含元素类型、类型名和边界信息。
       * @param[out] type.ret 创建的动态序列类型。
       * @return 如果参数无效或创建过程中出现错误，跳转到 err_bad_param 标签。
       */
      if (!typespec_valid(descriptor.element_type, false) ||  // 检查类型规范是否有效
          !typename_valid(descriptor.name) ||                 // 检查类型名是否有效
          descriptor.num_bounds > 1 ||                        // 检查边界数量是否大于1
          (descriptor.num_bounds == 1 &&
           descriptor.bounds == NULL))  // 检查边界数量为1时，边界数组是否为空
        goto err_bad_param;

      // 根据类型规范创建动态元素类型
      dds_dynamic_type_t element_type = dyntype_from_typespec(gv, descriptor.element_type);

      // 使用元素类型、类型名和边界信息创建动态序列类型
      type.ret = ddsi_dynamic_type_create_sequence(
          gv,                                                     // 全局变量
          (struct ddsi_type **)&type.x,                           // 动态类型指针
          descriptor.name,                                        // 类型名
          (struct ddsi_type **)&element_type.x,                   // 元素类型指针
          descriptor.num_bounds > 0 ? descriptor.bounds[0] : 0);  // 边界信息

      break;
    }
    // 结构体类型 (Structure type)
    case DDS_DYNAMIC_STRUCTURE: {
      /**
       * @brief 检查基本类型规范和类型名是否有效，然后创建动态类型的结构体。
       *
       * @param[in] descriptor 结构体描述符，包含基本类型和名称等信息。
       * @param[in] gv 全局变量指针，用于访问动态类型系统。
       * @param[out] type 创建的动态类型结构体。
       *
       * @return 如果成功创建动态类型结构体，则返回 true；否则返回 false。
       */
      if (!typespec_valid(descriptor.base_type, true) ||  // 检查基本类型规范是否有效
          !typename_valid(descriptor.name))               // 检查类型名是否有效
        goto err_bad_param;  // 如果任何一个检查失败，跳转到错误处理部分

      // 根据基本类型规范创建动态类型基本类型
      dds_dynamic_type_t base_type = dyntype_from_typespec(gv, descriptor.base_type);

      // 使用给定的名称、基本类型和全局变量创建动态类型结构体
      type.ret = ddsi_dynamic_type_create_struct(gv, (struct ddsi_type **)&type.x, descriptor.name,
                                                 (struct ddsi_type **)&base_type.x);
      break;
    }
    // 联合体类型 (Union type)
    case DDS_DYNAMIC_UNION: {
      /**
       * @brief 检查类型规范、联合体判别器和类型名是否有效，然后创建动态类型的联合体。
       *
       * @param[in] descriptor 联合体描述符，包含判别器类型和名称等信息。
       * @param[in] gv 全局变量指针，用于访问动态类型系统。
       * @param[out] type 创建的动态类型联合体。
       *
       * @return 如果成功创建动态类型联合体，则返回 true；否则返回 false。
       */
      if (!typespec_valid(descriptor.discriminator_type, false) ||  // 检查类型规范是否有效
          !union_disc_valid(descriptor.discriminator_type) ||  // 检查联合体判别器是否有效
          !typename_valid(descriptor.name))                    // 检查类型名是否有效
        goto err_bad_param;  // 如果任何一个检查失败，跳转到错误处理部分

      // 根据类型规范创建动态类型判别器
      dds_dynamic_type_t discriminator_type =
          dyntype_from_typespec(gv, descriptor.discriminator_type);

      // 使用给定的名称、判别器类型和全局变量创建动态类型联合体
      type.ret = ddsi_dynamic_type_create_union(gv, (struct ddsi_type **)&type.x, descriptor.name,
                                                (struct ddsi_type **)&discriminator_type.x);
      break;
    }

    // 不支持的类型 (Unsupported types)
    case DDS_DYNAMIC_CHAR16:
    case DDS_DYNAMIC_STRING16:
    case DDS_DYNAMIC_MAP:
    case DDS_DYNAMIC_BITSET:
      type.ret = DDS_RETCODE_UNSUPPORTED;
      break;
  }
  return type;

// 错误处理 (Error handling)
err_bad_param:
  type.ret = DDS_RETCODE_BAD_PARAMETER;
err:
  return type;
}

/**
 * @brief 检查动态类型参数是否有效。
 *
 * @param[in] type 动态类型指针。
 * @param[in] allow_non_constructing 是否允许非构造类型。
 * @return 返回 dds_return_t 类型的结果，表示检查是否成功。
 */
static dds_return_t check_type_param(const dds_dynamic_type_t *type, bool allow_non_constructing) {
  // 如果 type 为 NULL，则返回错误参数
  if (type == NULL) return DDS_RETCODE_BAD_PARAMETER;
  // 如果 type 的 ret 不是成功状态，则返回 type 的 ret 值
  if (type->ret != DDS_RETCODE_OK) return type->ret;
  // 如果不允许非构造类型且类型不是构造类型，则返回前提条件未满足错误
  if (!allow_non_constructing && !ddsi_dynamic_type_is_constructing((struct ddsi_type *)type->x))
    return DDS_RETCODE_PRECONDITION_NOT_MET;
  // 否则返回成功状态
  return DDS_RETCODE_OK;
}

/**
 * @brief 向动态类型中添加枚举字面量。
 *
 * @param[in,out] type 动态类型指针。
 * @param[in] name 枚举字面量名称。
 * @param[in] value 枚举字面量值。
 * @param[in] is_default 是否为默认值。
 * @return 返回 dds_return_t 类型的结果，表示添加操作是否成功。
 */
dds_return_t dds_dynamic_type_add_enum_literal(dds_dynamic_type_t *type,
                                               const char *name,
                                               dds_dynamic_enum_literal_value_t value,
                                               bool is_default) {
  dds_return_t ret;
  // 检查动态类型参数是否有效
  if ((ret = check_type_param(type, false)) != DDS_RETCODE_OK) return ret;
  // 如果成员名称无效，则设置 type 的 ret 为错误参数
  else if (!membername_valid(name))
    type->ret = DDS_RETCODE_BAD_PARAMETER;
  else {
    // 否则，向动态类型中添加枚举字面量
    type->ret = ddsi_dynamic_type_add_enum_literal(
        (struct ddsi_type *)type->x,
        (struct ddsi_dynamic_type_enum_literal_param){
            .name = name,
            .is_auto_value = value.value_kind == DDS_DYNAMIC_ENUM_LITERAL_VALUE_NEXT_AVAIL,
            .value = value.value,
            .is_default = is_default});
  }
  // 返回 type 的 ret 值
  return type->ret;
}

/**
 * @brief 为动态类型添加位掩码字段
 *
 * @param type 动态类型指针
 * @param name 位掩码字段的名称
 * @param position 位掩码字段的位置
 * @return dds_return_t 返回操作结果
 */
dds_return_t dds_dynamic_type_add_bitmask_field(dds_dynamic_type_t *type,
                                                const char *name,
                                                uint16_t position) {
  dds_return_t ret;
  // 检查类型参数是否有效
  if ((ret = check_type_param(type, false)) != DDS_RETCODE_OK) return ret;
  // 检查成员名称是否有效
  else if (!membername_valid(name))
    type->ret = DDS_RETCODE_BAD_PARAMETER;
  else {
    // 添加位掩码字段到动态类型中
    type->ret = ddsi_dynamic_type_add_bitmask_field(
        (struct ddsi_type *)type->x,
        (struct ddsi_dynamic_type_bitmask_field_param){
            .name = name,
            .is_auto_position = (position == DDS_DYNAMIC_BITMASK_POSITION_AUTO),
            .position = (position == DDS_DYNAMIC_BITMASK_POSITION_AUTO) ? 0 : position});
  }
  return type->ret;
}

/**
 * @brief 为动态类型添加成员
 *
 * @param type 动态类型指针
 * @param member_descriptor 成员描述符
 * @return dds_return_t 返回操作结果
 */
dds_return_t dds_dynamic_type_add_member(dds_dynamic_type_t *type,
                                         dds_dynamic_member_descriptor_t member_descriptor) {
  dds_return_t ret;
  // 检查类型参数是否有效
  if ((ret = check_type_param(type, false)) != DDS_RETCODE_OK) return ret;

  // 检查成员名称是否有效
  if (!membername_valid(member_descriptor.name)) {
    type->ret = DDS_RETCODE_BAD_PARAMETER;
    goto err;
  }

  // 根据给定的类型和成员描述符，为动态类型添加相应的成员。
  switch (xtkind_to_typekind(ddsi_type_get_kind((struct ddsi_type *)type->x))) {
    // 添加枚举字面量
    case DDS_DYNAMIC_ENUMERATION:
      type->ret = dds_dynamic_type_add_enum_literal(type, member_descriptor.name,
                                                    DDS_DYNAMIC_ENUM_LITERAL_VALUE_AUTO,
                                                    member_descriptor.default_label);
      break;
    // 添加位掩码字段
    case DDS_DYNAMIC_BITMASK:
      type->ret = dds_dynamic_type_add_bitmask_field(type, member_descriptor.name,
                                                     DDS_DYNAMIC_BITMASK_POSITION_AUTO);
      break;
    // 添加联合成员
    case DDS_DYNAMIC_UNION: {
      if (!typespec_valid(member_descriptor.type, false) ||
          (!member_descriptor.default_label &&
           (member_descriptor.num_labels == 0 || member_descriptor.labels == NULL))) {
        type->ret = DDS_RETCODE_BAD_PARAMETER;
        goto err;
      }
      dds_dynamic_type_t member_type = dyntype_from_typespec(
          ddsi_type_get_gv((struct ddsi_type *)type->x), member_descriptor.type);
      type->ret = ddsi_dynamic_type_add_union_member(
          (struct ddsi_type *)type->x, (struct ddsi_type **)&member_type.x,
          (struct ddsi_dynamic_type_union_member_param){
              .id = member_descriptor.id,
              .name = member_descriptor.name,
              .index = member_descriptor.index,
              .is_default = member_descriptor.default_label,
              .labels = member_descriptor.labels,
              .n_labels = member_descriptor.num_labels});
      if (type->ret != DDS_RETCODE_OK) dds_dynamic_type_unref(&member_type);
      break;
    }
    // 添加结构成员
    case DDS_DYNAMIC_STRUCTURE: {
      if (!typespec_valid(member_descriptor.type, false)) {
        type->ret = DDS_RETCODE_BAD_PARAMETER;
        goto err;
      }
      dds_dynamic_type_t member_type = dyntype_from_typespec(
          ddsi_type_get_gv((struct ddsi_type *)type->x), member_descriptor.type);
      type->ret = ddsi_dynamic_type_add_struct_member(
          (struct ddsi_type *)type->x, (struct ddsi_type **)&member_type.x,
          (struct ddsi_dynamic_type_struct_member_param){.id = member_descriptor.id,
                                                         .name = member_descriptor.name,
                                                         .index = member_descriptor.index,
                                                         .is_key = false});
      if (type->ret != DDS_RETCODE_OK) dds_dynamic_type_unref(&member_type);
      break;
    }
    // 其他情况，返回错误参数
    default:
      type->ret = DDS_RETCODE_BAD_PARAMETER;
      break;
  }

err:
  return type->ret;
}

/**
 * @brief 设置动态类型的可扩展性
 *
 * @param[in,out] type 动态类型指针
 * @param[in] extensibility 可扩展性枚举值
 * @return dds_return_t 返回操作结果，成功返回DDS_RETCODE_OK，否则返回错误代码
 */
dds_return_t dds_dynamic_type_set_extensibility(dds_dynamic_type_t *type,
                                                enum dds_dynamic_type_extensibility extensibility) {
  // 定义返回值变量
  dds_return_t ret;

  // 检查类型参数是否有效
  if ((ret = check_type_param(type, false)) != DDS_RETCODE_OK) return ret;

  // 检查可扩展性参数是否有效
  if (extensibility > DDS_DYNAMIC_TYPE_EXT_MUTABLE) return DDS_RETCODE_BAD_PARAMETER;

  // 根据类型种类设置可扩展性
  switch (xtkind_to_typekind(ddsi_type_get_kind((struct ddsi_type *)type->x))) {
    case DDS_DYNAMIC_STRUCTURE:
    case DDS_DYNAMIC_UNION:
    case DDS_DYNAMIC_ENUMERATION:
    case DDS_DYNAMIC_BITMASK:
      type->ret = ddsi_dynamic_type_set_extensibility((struct ddsi_type *)type->x, extensibility);
      break;
    default:
      type->ret = DDS_RETCODE_BAD_PARAMETER;
      break;
  }

  // 返回操作结果
  return type->ret;
}

/**
 * @brief 设置动态类型的嵌套属性
 *
 * @param[in,out] type 动态类型指针
 * @param[in] is_nested 嵌套属性布尔值
 * @return dds_return_t 返回操作结果，成功返回DDS_RETCODE_OK，否则返回错误代码
 */
dds_return_t dds_dynamic_type_set_nested(dds_dynamic_type_t *type, bool is_nested) {
  // 定义返回值变量
  dds_return_t ret;

  // 检查类型参数是否有效
  if ((ret = check_type_param(type, false)) != DDS_RETCODE_OK) return ret;

  // 根据类型种类设置嵌套属性
  switch (xtkind_to_typekind(ddsi_type_get_kind((struct ddsi_type *)type->x))) {
    case DDS_DYNAMIC_STRUCTURE:
    case DDS_DYNAMIC_UNION:
      type->ret = ddsi_dynamic_type_set_nested((struct ddsi_type *)type->x, is_nested);
      break;
    default:
      type->ret = DDS_RETCODE_BAD_PARAMETER;
      break;
  }

  // 返回操作结果
  return type->ret;
}

/**
 * @brief 设置动态类型的自动ID策略。
 *
 * @param[in] type 动态类型指针。
 * @param[in] value
 * 自动ID策略值，可以是DDS_DYNAMIC_TYPE_AUTOID_HASH或DDS_DYNAMIC_TYPE_AUTOID_SEQUENTIAL。
 * @return 返回dds_return_t类型的结果，表示操作成功或失败。
 */
dds_return_t dds_dynamic_type_set_autoid(dds_dynamic_type_t *type,
                                         enum dds_dynamic_type_autoid value) {
  // 定义返回值变量
  dds_return_t ret;

  // 检查类型参数是否有效
  if ((ret = check_type_param(type, false)) != DDS_RETCODE_OK) return ret;

  // 判断value是否为有效的自动ID策略
  if (value != DDS_DYNAMIC_TYPE_AUTOID_HASH && value != DDS_DYNAMIC_TYPE_AUTOID_SEQUENTIAL)
    type->ret = DDS_RETCODE_BAD_PARAMETER;
  else {
    // 根据类型种类设置自动ID策略
    switch (xtkind_to_typekind(ddsi_type_get_kind((struct ddsi_type *)type->x))) {
      case DDS_DYNAMIC_STRUCTURE:
      case DDS_DYNAMIC_UNION:
        type->ret = ddsi_dynamic_type_set_autoid((struct ddsi_type *)type->x, value);
        break;
      default:
        type->ret = DDS_RETCODE_BAD_PARAMETER;
        break;
    }
  }
  return type->ret;
}

/**
 * @brief 设置动态类型的位边界。
 *
 * @param[in] type 动态类型指针。
 * @param[in] bit_bound 位边界值，范围为1到32（包含）或1到64（包含）。
 * @return 返回dds_return_t类型的结果，表示操作成功或失败。
 */
dds_return_t dds_dynamic_type_set_bit_bound(dds_dynamic_type_t *type, uint16_t bit_bound) {
  // 定义返回值变量
  dds_return_t ret;

  // 检查类型参数是否有效
  if ((ret = check_type_param(type, false)) != DDS_RETCODE_OK) return ret;

  // 根据类型种类设置位边界
  switch (xtkind_to_typekind(ddsi_type_get_kind((struct ddsi_type *)type->x))) {
    case DDS_DYNAMIC_ENUMERATION:
      type->ret = (bit_bound > 0 && bit_bound <= 32)
                      ? ddsi_dynamic_type_set_bitbound((struct ddsi_type *)type->x, bit_bound)
                      : DDS_RETCODE_BAD_PARAMETER;
      break;
    case DDS_DYNAMIC_BITMASK:
      type->ret = (bit_bound > 0 && bit_bound <= 64)
                      ? ddsi_dynamic_type_set_bitbound((struct ddsi_type *)type->x, bit_bound)
                      : DDS_RETCODE_BAD_PARAMETER;
      break;
    default:
      type->ret = DDS_RETCODE_BAD_PARAMETER;
      break;
  }
  return type->ret;
}

/**
 * @brief 设置结构体属性的函数指针类型。
 *
 * @param type 结构体类型指针。
 * @param member_id 成员ID。
 * @param is_key 是否为键值。
 * @return dds_return_t 返回操作结果。
 */
typedef dds_return_t (*set_struct_prop_fn)(struct ddsi_type *type, uint32_t member_id, bool is_key);

/**
 * @brief 设置成员布尔属性的通用函数。
 *
 * @param type 动态类型指针。
 * @param member_id 成员ID。
 * @param value 布尔值。
 * @param set_fn_struct 结构体设置函数指针。
 * @param set_fn_union 联合体设置函数指针。
 * @return dds_return_t 返回操作结果。
 */
static dds_return_t set_member_bool_prop(dds_dynamic_type_t *type,
                                         uint32_t member_id,
                                         bool value,
                                         set_struct_prop_fn set_fn_struct,
                                         set_struct_prop_fn set_fn_union) {
  dds_return_t ret;
  // 检查类型参数
  if ((ret = check_type_param(type, false)) != DDS_RETCODE_OK) return ret;

  // 根据类型执行相应的设置函数
  switch (xtkind_to_typekind(ddsi_type_get_kind((struct ddsi_type *)type->x))) {
    case DDS_DYNAMIC_STRUCTURE:
      type->ret = set_fn_struct ? set_fn_struct((struct ddsi_type *)type->x, member_id, value)
                                : DDS_RETCODE_BAD_PARAMETER;
      break;
    case DDS_DYNAMIC_UNION:
      type->ret = set_fn_union ? set_fn_union((struct ddsi_type *)type->x, member_id, value)
                               : DDS_RETCODE_BAD_PARAMETER;
      break;
    default:
      type->ret = DDS_RETCODE_BAD_PARAMETER;
      break;
  }
  return type->ret;
}

/**
 * @brief 设置动态类型成员的键值属性。
 *
 * @param type 动态类型指针。
 * @param member_id 成员ID。
 * @param is_key 是否为键值。
 * @return dds_return_t 返回操作结果。
 */
dds_return_t dds_dynamic_member_set_key(dds_dynamic_type_t *type, uint32_t member_id, bool is_key) {
  return (type->ret =
              set_member_bool_prop(type, member_id, is_key, ddsi_dynamic_type_member_set_key, 0));
}

/**
 * @brief 设置动态类型成员的可选属性。
 *
 * @param type 动态类型指针。
 * @param member_id 成员ID。
 * @param is_optional 是否为可选。
 * @return dds_return_t 返回操作结果。
 */
dds_return_t dds_dynamic_member_set_optional(dds_dynamic_type_t *type,
                                             uint32_t member_id,
                                             bool is_optional) {
  return (type->ret = set_member_bool_prop(type, member_id, is_optional,
                                           ddsi_dynamic_type_member_set_optional, 0));
}

/**
 * @brief 设置动态类型成员的外部属性。
 *
 * @param type         动态类型指针。
 * @param member_id    成员ID。
 * @param is_external  是否为外部属性。
 * @return 返回操作结果，成功返回DDS_RETCODE_OK。
 */
dds_return_t dds_dynamic_member_set_external(dds_dynamic_type_t *type,
                                             uint32_t member_id,
                                             bool is_external) {
  // 调用set_member_bool_prop函数设置成员的外部属性，并返回结果
  return (type->ret = set_member_bool_prop(type, member_id, is_external,
                                           ddsi_dynamic_struct_member_set_external,
                                           ddsi_dynamic_union_member_set_external));
}

/**
 * @brief 设置动态类型成员的哈希ID。
 *
 * @param type             动态类型指针。
 * @param member_id        成员ID。
 * @param hash_member_name 哈希成员名称。
 * @return 返回操作结果，成功返回DDS_RETCODE_OK。
 */
dds_return_t dds_dynamic_member_set_hashid(dds_dynamic_type_t *type,
                                           uint32_t member_id,
                                           const char *hash_member_name) {
  dds_return_t ret;
  // 检查类型参数是否正确
  if ((ret = check_type_param(type, false)) != DDS_RETCODE_OK) return ret;

  // 根据类型种类进行相应操作
  switch (xtkind_to_typekind(ddsi_type_get_kind((struct ddsi_type *)type->x))) {
    case DDS_DYNAMIC_STRUCTURE:
    case DDS_DYNAMIC_UNION:
      // 设置动态类型成员的哈希ID
      type->ret = ddsi_dynamic_type_member_set_hashid((struct ddsi_type *)type->x, member_id,
                                                      hash_member_name);
      break;
    default:
      // 类型参数错误
      type->ret = DDS_RETCODE_BAD_PARAMETER;
      break;
  }
  return type->ret;
}

/**
 * @brief 设置动态类型成员的必须理解属性。
 *
 * @param type               动态类型指针。
 * @param member_id          成员ID。
 * @param is_must_understand 是否为必须理解属性。
 * @return 返回操作结果，成功返回DDS_RETCODE_OK。
 */
dds_return_t dds_dynamic_member_set_must_understand(dds_dynamic_type_t *type,
                                                    uint32_t member_id,
                                                    bool is_must_understand) {
  // 调用set_member_bool_prop函数设置成员的必须理解属性，并返回结果
  return (type->ret = set_member_bool_prop(type, member_id, is_must_understand,
                                           ddsi_dynamic_type_member_set_must_understand, 0));
}

/**
 * @brief 注册动态类型。
 *
 * @param type      动态类型指针。
 * @param type_info 类型信息指针。
 * @return 返回操作结果，成功返回DDS_RETCODE_OK。
 */
dds_return_t dds_dynamic_type_register(dds_dynamic_type_t *type, dds_typeinfo_t **type_info) {
  dds_return_t ret;
  // 检查类型参数是否正确
  if ((ret = check_type_param(type, false)) != DDS_RETCODE_OK) return ret;
  // 注册动态类型
  return ddsi_dynamic_type_register((struct ddsi_type **)&type->x, type_info);
}

/**
 * @brief 引用动态类型 (Reference a dynamic type)
 *
 * @param[in] type 动态类型指针 (Pointer to the dynamic type)
 * @return 返回一个新的动态类型结构，其中包含引用的类型 (Returns a new dynamic type structure
 * containing the referenced type)
 */
dds_dynamic_type_t dds_dynamic_type_ref(dds_dynamic_type_t *type) {
  // 初始化一个新的动态类型结构 (Initialize a new dynamic type structure)
  dds_dynamic_type_t ref = {NULL, 0};

  // 检查类型参数是否有效 (Check if the type parameter is valid)
  if ((ref.ret = check_type_param(type, true)) != DDS_RETCODE_OK) return ref;

  // 引用动态类型 (Reference the dynamic type)
  ref.x = ddsi_dynamic_type_ref((struct ddsi_type *)type->x);

  // 返回引用后的动态类型 (Return the referenced dynamic type)
  return ref;
}

/**
 * @brief 取消引用动态类型 (Unreference a dynamic type)
 *
 * @param[in] type 动态类型指针 (Pointer to the dynamic type)
 * @return 返回操作结果代码 (Returns the operation result code)
 */
dds_return_t dds_dynamic_type_unref(dds_dynamic_type_t *type) {
  // 检查类型参数是否为空 (Check if the type parameter is NULL)
  if (type == NULL) return DDS_RETCODE_BAD_PARAMETER;

  // 取消引用动态类型 (Unreference the dynamic type)
  ddsi_dynamic_type_unref((struct ddsi_type *)type->x);

  // 返回操作成功代码 (Return the operation success code)
  return DDS_RETCODE_OK;
}

/**
 * @brief 复制动态类型 (Duplicate a dynamic type)
 *
 * @param[in] src 源动态类型指针 (Pointer to the source dynamic type)
 * @return 返回一个新的动态类型结构，其中包含复制的类型 (Returns a new dynamic type structure
 * containing the duplicated type)
 */
dds_dynamic_type_t dds_dynamic_type_dup(const dds_dynamic_type_t *src) {
  // 初始化一个新的动态类型结构 (Initialize a new dynamic type structure)
  dds_dynamic_type_t dst = {NULL, 0};

  // 检查源类型参数是否有效 (Check if the source type parameter is valid)
  if ((dst.ret = check_type_param(src, true)) == DDS_RETCODE_OK) {
    // 复制动态类型 (Duplicate the dynamic type)
    dst.x = ddsi_dynamic_type_dup((struct ddsi_type *)src->x);

    // 设置返回代码 (Set the return code)
    dst.ret = src->ret;
  }

  // 返回复制后的动态类型 (Return the duplicated dynamic type)
  return dst;
}
