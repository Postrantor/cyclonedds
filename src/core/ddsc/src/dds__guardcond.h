/*
 * Copyright(c) 2006 to 2021 ZettaScale Technology and others
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v. 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0, or the Eclipse Distribution License
 * v. 1.0 which is available at
 * http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * SPDX-License-Identifier: EPL-2.0 OR BSD-3-Clause
 */
#ifndef DDS__GUARDCOND_H
#define DDS__GUARDCOND_H

#include "dds__entity.h"

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @brief 定义实体锁和解锁函数 (Define entity lock and unlock functions)
 *
 * @param dds_guardcond 实体类型 (Entity type)
 * @param DDS_KIND_COND_GUARD 实体种类 (Entity kind)
 * @param guard_condition 实体变量名 (Entity variable name)
 */
DEFINE_ENTITY_LOCK_UNLOCK(dds_guardcond, DDS_KIND_COND_GUARD, guard_condition)
// 使用宏定义 DEFINE_ENTITY_LOCK_UNLOCK 为 dds_guardcond 类型的实体创建锁定和解锁功能
// (Use the macro DEFINE_ENTITY_LOCK_UNLOCK to create lock and unlock functions for entities of type
// dds_guardcond)

// dds_guardcond - 实体类型，表示 DDS 守护条件 (Entity type representing a DDS guard condition)
// DDS_KIND_COND_GUARD - 实体种类，用于识别实体类型 (Entity kind used to identify the entity type)
// guard_condition - 实体变量名，用于在锁定和解锁函数中引用实体 (Entity variable name used to
// reference the entity in lock and unlock functions)

#if defined(__cplusplus)
}
#endif

#endif /* DDS__GUARDCOND_H */
