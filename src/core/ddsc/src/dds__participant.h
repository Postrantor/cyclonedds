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
#ifndef DDS__PARTICIPANT_H
#define DDS__PARTICIPANT_H

#include "dds__entity.h"

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @brief 定义实体锁定和解锁的宏 (Define entity lock and unlock macro)
 *
 * @param[in] dds_participant DDS参与者实例 (DDS participant instance)
 * @param[in] DDS_KIND_PARTICIPANT 参与者类型 (Participant type)
 * @param[in] participant 参与者变量名 (Participant variable name)
 */
DEFINE_ENTITY_LOCK_UNLOCK(dds_participant, DDS_KIND_PARTICIPANT, participant)

// 外部声明 AVL 树定义 (External declaration of the AVL tree definition)
/**
 * @brief 参与者关键主题树定义 (Participant key topics tree definition)
 */
extern const ddsrt_avl_treedef_t participant_ktopics_treedef;

#if defined(__cplusplus)
}
#endif

#endif /* DDS__PARTICIPANT_H */
