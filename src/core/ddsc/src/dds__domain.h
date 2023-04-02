/*
 * Copyright(c) 2006 to 2019 ZettaScale Technology and others
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v. 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0, or the Eclipse Distribution License
 * v. 1.0 which is available at
 * http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * SPDX-License-Identifier: EPL-2.0 OR BSD-3-Clause
 */
#ifndef DDS__DOMAIN_H
#define DDS__DOMAIN_H

#include "dds__types.h"

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @brief 创建一个内部 DDS 域 (Create an internal DDS domain)
 *
 * @param[out] domain_out 输出的 DDS 域指针 (Output pointer to the created DDS domain)
 * @param[in] id 要创建的域的 ID (The ID of the domain to be created)
 * @param[in] implicit 是否为隐式创建 (Whether it is implicitly created)
 * @param[in] config 配置字符串 (Configuration string)
 * @return 返回创建的 DDS 实体 (Returns the created DDS entity)
 */
dds_entity_t dds_domain_create_internal(dds_domain** domain_out,
                                        dds_domainid_t id,
                                        bool implicit,
                                        const char* config) ddsrt_nonnull((1, 4));

/**
 * @brief 在锁定状态下查找 DDS 域 (Find a DDS domain in locked state)
 *
 * @param[in] id 要查找的域的 ID (The ID of the domain to find)
 * @return 返回找到的 DDS 域指针，如果没有找到则返回 NULL (Returns the pointer to the found DDS
 * domain, or NULL if not found)
 */
dds_domain* dds_domain_find_locked(dds_domainid_t id);

#if defined(__cplusplus)
}
#endif
#endif /* DDS__DOMAIN_H */
