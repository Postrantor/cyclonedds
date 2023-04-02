/*
 * Copyright(c) 2006 to 2020 ZettaScale Technology and others
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v. 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0, or the Eclipse Distribution License
 * v. 1.0 which is available at
 * http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * SPDX-License-Identifier: EPL-2.0 OR BSD-3-Clause
 */
#ifndef DDSI_PMD_H
#define DDSI_PMD_H

#include <stdint.h>

#include "dds/ddsi/ddsi_guid.h"

#if defined(__cplusplus)
extern "C" {
#endif

/* 定义 ddsi_domaingv 结构体 */
/* Define the ddsi_domaingv structure */
struct ddsi_domaingv;

/* 定义 ddsi_guid 结构体 */
/* Define the ddsi_guid structure */
struct ddsi_guid;

/**
 * @brief 向具有指定 GUID 的参与者发送 PMD 消息
 * @param gv 指向 ddsi_domaingv 结构体的指针，用于存储域全局变量
 * @param pp_guid 指向 ddsi_guid 结构体的指针，表示要发送 PMD 消息的参与者的 GUID
 * @param pmd_kind PMD 消息的类型（例如，PMD_KIND_AUTOMATIC_LIVELINESS）
 *
 * @component pmd
 * @note 该函数不会返回任何值
 *
 * @brief Send a PMD message to a participant with the specified GUID
 * @param gv Pointer to the ddsi_domaingv structure, used for storing domain global variables
 * @param pp_guid Pointer to the ddsi_guid structure, representing the GUID of the participant to
 * send the PMD message to
 * @param pmd_kind The kind of PMD message (e.g., PMD_KIND_AUTOMATIC_LIVELINESS)
 *
 * @component pmd
 * @note This function does not return any value
 */
/*
在 Cyclone DDS 项目中，"PMD" 是 "Participant Message Data"
*/
void ddsi_write_pmd_message_guid(struct ddsi_domaingv* const gv,
                                 struct ddsi_guid* pp_guid,
                                 unsigned pmd_kind);

#if defined(__cplusplus)
}
#endif
#endif /* DDSI_PMD_H */
