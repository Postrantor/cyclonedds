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
#ifndef DDS__LOAN_H
#define DDS__LOAN_H

#if defined(__cplusplus)
extern "C" {
#endif

#ifdef DDS_HAS_SHM

/**
 * @brief 注册发布者的内存贷款（Register a publisher's memory loan）
 *
 * @param[in] wr 写入器指针（Pointer to the writer）
 * @param[in] pub_loan 发布者内存贷款指针（Pointer to the publisher's memory loan）
 */
void dds_register_pub_loan(dds_writer* wr, void* pub_loan);
// 将发布者的内存贷款添加到写入器的贷款列表中（Add the publisher's memory loan to the writer's loan
// list） 这将允许写入器在发送数据时使用预先分配的内存（This will allow the writer to use
// pre-allocated memory when sending data）

/**
 * @brief 取消注册发布者的内存贷款（Deregister a publisher's memory loan）
 *
 * @param[in] wr 写入器指针（Pointer to the writer）
 * @param[in] pub_loan 发布者内存贷款指针（Pointer to the publisher's memory loan）
 * @return 如果成功取消注册，则返回 true，否则返回 false（Returns true if the deregistration is
 * successful, otherwise returns false）
 */
bool dds_deregister_pub_loan(dds_writer* wr, const void* pub_loan);
// 从写入器的贷款列表中删除发布者的内存贷款（Remove the publisher's memory loan from the writer's
// loan list） 这将阻止写入器在发送数据时使用该内存贷款（This will prevent the writer from using the
// memory loan when sending data）

#endif /* DDS_HAS_SHM */

#if defined(__cplusplus)
}
#endif

#endif /* DDS__LOAN_H */
