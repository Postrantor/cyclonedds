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
#ifndef DDSI_ADDRSET_H
#define DDSI_ADDRSET_H

#include "dds/ddsi/ddsi_locator.h"
#include "dds/ddsrt/atomics.h"
#include "dds/ddsrt/avl.h"
#include "dds/ddsrt/sync.h"

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @file ddsi_addrset.h
 * @brief Cyclone DDS 地址集合相关的函数和结构 (Cyclone DDS address set related functions and
 * structures)
 */

struct ddsi_addrset;

/**
 * @brief 用于遍历地址集合的回调函数类型定义 (Callback function type definition for traversing
 * address sets)
 *
 * @param loc 当前遍历到的定位器 (The locator currently being traversed)
 * @param arg 用户提供的参数，用于传递给回调函数 (User-provided argument to be passed to the
 * callback function)
 */
typedef void (*ddsi_addrset_forall_fun_t)(const ddsi_xlocator_t *loc, void *arg);

/**
 * @brief 检查地址集合是否为空 (Check if the address set is empty)
 *
 * @param as 要检查的地址集合指针 (Pointer to the address set to check)
 * @return 如果地址集合为空，则返回非零值；否则返回0 (Returns non-zero value if the address set is
 * empty, otherwise returns 0)
 *
 * @component locators
 */
int ddsi_addrset_empty(const struct ddsi_addrset *as);

/**
 * @brief 对地址集合中的每个元素执行给定的回调函数 (Execute the given callback function for each
 * element in the address set)
 *
 * @param as 要遍历的地址集合指针 (Pointer to the address set to traverse)
 * @param f 要对每个元素执行的回调函数 (Callback function to be executed for each element)
 * @param arg 要传递给回调函数的用户提供的参数 (User-provided argument to be passed to the callback
 * function)
 *
 * @component locators
 */
void ddsi_addrset_forall(struct ddsi_addrset *as, ddsi_addrset_forall_fun_t f, void *arg);

#if defined(__cplusplus)
}
#endif
#endif /* DDSI_ADDRSET_H */
