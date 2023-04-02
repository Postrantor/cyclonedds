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
#ifndef DDS__INIT_H
#define DDS__INIT_H

#include "dds__types.h"

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @component cyclone_root
 *
 * 初始化库并构造由 DDS_CYCLONEDDS_HANDLE 标识的全局伪实体，该实体具有一个引用，
 * 必须（最终）通过在该句柄上调用 dds_delete 来释放。
 *
 * @return dds_return_t 成功时返回 0，否则返回非零错误状态
 */
// 函数：初始化库并构造全局伪实体
dds_return_t dds_init(void);

#if defined(__cplusplus)
}
#endif
#endif /* DDS__INIT_H */
