/*
 * Copyright(c) 2006 to 2022 ZettaScale Technology and others
 * Copyright(c) 2021 Apex.AI, Inc
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v. 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0, or the Eclipse Distribution License
 * v. 1.0 which is available at
 * http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * SPDX-License-Identifier: EPL-2.0 OR BSD-3-Clause
 */

#ifndef _DDS_BASIC_TYPES_H_
#define _DDS_BASIC_TYPES_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief 实体句柄 (Handle to an entity)
 * @ingroup entity
 *
 * 有效的实体句柄总是具有正整数值。 (A valid entity handle will always have a positive integer
 * value.) 如果值为负，则为 DDS_RETCODE_* 错误代码之一。 (Should the value be negative, it is one of
 * the DDS_RETCODE_* error codes.)
 */
typedef int32_t dds_entity_t;

/**
 * @anchor DDS_MIN_PSEUDO_HANDLE
 * @ingroup internal
 * @brief 伪句柄起点 (Pseudo Handle origin)
 *
 * CycloneDDS 中的一些句柄是“假”的，最重要的是内置主题句柄。 (Some handles in CycloneDDS are 'fake',
 * most importantly the builtin topic handles.) 这些句柄源自此常量。 (These handles are derived from
 * this constant.)
 */
#define DDS_MIN_PSEUDO_HANDLE ((dds_entity_t)0x7fff0000)

/**
 * @anchor DDS_CYCLONEDDS_HANDLE
 * @ingroup internal
 * @brief 表示与 CycloneDDS 库本身对应的实体的特殊句柄 (Special handle representing the entity
 * corresponding to the CycloneDDS library itself)
 */
#define DDS_CYCLONEDDS_HANDLE ((dds_entity_t)(DDS_MIN_PSEUDO_HANDLE + 256))

#endif /*_DDS_PUBLIC_TYPES_H_*/
