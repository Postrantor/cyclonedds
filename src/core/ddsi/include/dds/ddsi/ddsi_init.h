/*
 * Copyright(c) 2006 to 2022 ZettaScale Technology and others
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v. 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0, or the Eclipse Distribution License
 * v. 1.0 which is available at
 * http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * SPDX-License-Identifier: EPL-2.0 OR BSD-3-Clause
 */
#ifndef DDSI_INIT_H
#define DDSI_INIT_H

#include <stdbool.h>

#include "dds/export.h"

#if defined(__cplusplus)
extern "C" {
#endif

struct ddsi_cfgst;
struct ddsi_domaingv;

/**
 * @component ddsi_init
 * @brief 准备配置信息 (Prepare configuration information)
 *
 * @param gv    指向 ddsi_domaingv 结构体的指针 (Pointer to the ddsi_domaingv structure)
 * @param cfgst 指向 ddsi_cfgst 结构体的指针 (Pointer to the ddsi_cfgst structure)
 * @return 成功返回 0，失败返回非 0 值 (Returns 0 on success, non-zero value on failure)
 */
int ddsi_config_prep(struct ddsi_domaingv* gv, struct ddsi_cfgst* cfgst);

/**
 * @component ddsi_init
 * @brief 初始化 ddsi_domaingv 结构体 (Initialize the ddsi_domaingv structure)
 *
 * @param gv 指向 ddsi_domaingv 结构体的指针 (Pointer to the ddsi_domaingv structure)
 * @return 成功返回 0，失败返回非 0 值 (Returns 0 on success, non-zero value on failure)
 */
int ddsi_init(struct ddsi_domaingv* gv);

/**
 * @component ddsi_init
 * @brief 启动 ddsi_domaingv 结构体 (Start the ddsi_domaingv structure)
 *
 * @param gv 指向 ddsi_domaingv 结构体的指针 (Pointer to the ddsi_domaingv structure)
 * @return 成功返回 0，失败返回非 0 值 (Returns 0 on success, non-zero value on failure)
 */
int ddsi_start(struct ddsi_domaingv* gv);

/**
 * @component ddsi_init
 * @brief 停止 ddsi_domaingv 结构体 (Stop the ddsi_domaingv structure)
 *
 * @param gv 指向 ddsi_domaingv 结构体的指针 (Pointer to the ddsi_domaingv structure)
 */
void ddsi_stop(struct ddsi_domaingv* gv);

/**
 * @component ddsi_init
 * @brief 清理 ddsi_domaingv 结构体 (Clean up the ddsi_domaingv structure)
 *
 * @param gv 指向 ddsi_domaingv 结构体的指针 (Pointer to the ddsi_domaingv structure)
 */
void ddsi_fini(struct ddsi_domaingv* gv);

/**
 * @component ddsi_generic_entity
 * @brief 设置实体的听力和发声状态 (Set the hearing and speaking status of an entity)
 *
 * @param gv          指向 ddsi_domaingv 结构体的指针 (Pointer to the ddsi_domaingv structure)
 * @param deaf        是否使实体变为聋子 (Whether to make the entity deaf)
 * @param mute        是否使实体变为哑巴 (Whether to make the entity mute)
 * @param reset_after 重置状态的时间，单位为纳秒 (Time to reset the status, in nanoseconds)
 */
void ddsi_set_deafmute(struct ddsi_domaingv* gv, bool deaf, bool mute, int64_t reset_after);

#if defined(__cplusplus)
}
#endif

#endif /* DDSI_INIT_H */
