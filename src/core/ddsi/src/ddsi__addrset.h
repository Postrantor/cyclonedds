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
#ifndef DDSI__ADDRSET_H
#define DDSI__ADDRSET_H

#include "dds/ddsi/ddsi_addrset.h"
#include "dds/ddsi/ddsi_feature_check.h"
#include "dds/ddsi/ddsi_locator.h"
#include "dds/ddsi/ddsi_log.h"
#include "dds/ddsi/ddsi_protocol.h"
#include "dds/ddsrt/avl.h"
#include "dds/ddsrt/sync.h"
#include "ddsi__thread.h"

#if defined(__cplusplus)
extern "C" {
#endif

struct ddsi_domaingv;

// ddsi_addrset 结构体定义
struct ddsi_addrset
{
  ddsrt_mutex_t lock; // 用于保护此结构的互斥锁
  ddsrt_atomic_uint32_t refc; // 原子引用计数器
  ddsrt_avl_ctree_t ucaddrs, mcaddrs; // 存储单播地址和多播地址的 AVL 树
};

// 定义一个函数指针类型，用于遍历地址集中的每个元素
typedef ssize_t (*ddsi_addrset_forone_fun_t)(const ddsi_xlocator_t * loc, void * arg);

/**
 * @component locators
 * 创建并返回一个新的地址集对象
 */
struct ddsi_addrset * ddsi_new_addrset(void);

/**
 * @component locators
 * 增加地址集的引用计数，并返回地址集指针
 * @param as 要增加引用计数的地址集
 */
struct ddsi_addrset * ddsi_ref_addrset(struct ddsi_addrset * as);

/**
 * @component locators
 * 减少地址集的引用计数，如果引用计数为0，则释放地址集
 * @param as 要减少引用计数的地址集
 */
void ddsi_unref_addrset(struct ddsi_addrset * as);

/**
 * @component locators
 * 向地址集中添加一个定位器
 * @param gv 全局变量结构体指针
 * @param as 要添加定位器的地址集
 * @param loc 要添加的定位器
 */
void ddsi_add_locator_to_addrset(
  const struct ddsi_domaingv * gv, struct ddsi_addrset * as, const ddsi_locator_t * loc);

/**
 * @component locators
 * 向地址集中添加一个扩展定位器
 * @param gv 全局变量结构体指针
 * @param as 要添加扩展定位器的地址集
 * @param loc 要添加的扩展定位器
 */
void ddsi_add_xlocator_to_addrset(
  const struct ddsi_domaingv * gv, struct ddsi_addrset * as, const ddsi_xlocator_t * loc);

/**
 * @component locators
 * 从地址集中移除一个扩展定位器
 * @param gv 全局变量结构体指针
 * @param as 要移除扩展定位器的地址集
 * @param loc 要移除的扩展定位器
 */
void ddsi_remove_from_addrset(
  const struct ddsi_domaingv * gv, struct ddsi_addrset * as, const ddsi_xlocator_t * loc);

/**
 * @component locators
 * 清除并释放地址集中所有元素，返回清除的元素数量
 * @param as 要清除的地址集
 */
int ddsi_addrset_purge(struct ddsi_addrset * as);

/**
 * @component locators
 * 比较两个定位器是否相等，相等返回0，不等返回非0值
 * @param a 第一个定位器
 * @param b 第二个定位器
 */
int ddsi_compare_locators(const ddsi_locator_t * a, const ddsi_locator_t * b);

/**
 * @component locators
 * 比较两个扩展定位器是否相等，相等返回0，不等返回非0值
 * @param a 第一个扩展定位器
 * @param b 第二个扩展定位器
 */
int ddsi_compare_xlocators(const ddsi_xlocator_t * a, const ddsi_xlocator_t * b);

/**
 * @component locators
 * 将 asadd 中的单播地址复制到 as 中
 * @param gv 全局变量结构体指针
 * @param as 目标地址集
 * @param asadd 源地址集
 */
void ddsi_copy_addrset_into_addrset_uc(
  const struct ddsi_domaingv * gv, struct ddsi_addrset * as, const struct ddsi_addrset * asadd);

/**
 * @component locators
 * 将 asadd 中的多播地址复制到 as 中
 * @param gv 全局变量结构体指针
 * @param as 目标地址集
 * @param asadd 源地址集
 */
void ddsi_copy_addrset_into_addrset_mc(
  const struct ddsi_domaingv * gv, struct ddsi_addrset * as, const struct ddsi_addrset * asadd);

/**
 * @component locators
 * 将 asadd 中的所有地址复制到 as 中
 * @param gv 全局变量结构体指针
 * @param as 目标地址集
 * @param asadd 源地址集
 */
void ddsi_copy_addrset_into_addrset(
  const struct ddsi_domaingv * gv, struct ddsi_addrset * as, const struct ddsi_addrset * asadd);

/**
 * @component locators
 * 获取地址集中的地址数量
 * @param as 要查询的地址集
 * @return 地址数量
 */
size_t ddsi_addrset_count(const struct ddsi_addrset * as);

/**
 * @component locators
 * 获取地址集中单播地址的数量
 * @param as 要查询的地址集
 * @return 单播地址数量
 */
size_t ddsi_addrset_count_uc(const struct ddsi_addrset * as);

/**
 * @component locators
 * 获取地址集中多播地址的数量
 * @param as 要查询的地址集
 * @return 多播地址数量
 */
size_t ddsi_addrset_count_mc(const struct ddsi_addrset * as);

/**
 * @component locators
 * 判断地址集中是否没有单播地址
 * @param as 要查询的地址集
 * @return 如果没有单播地址，返回非0值；否则返回0
 */
int ddsi_addrset_empty_uc(const struct ddsi_addrset * as);

/**
 * @component locators
 * 判断地址集中是否没有多播地址
 * @param as 要查询的地址集
 * @return 如果没有多播地址，返回非0值；否则返回0
 */
int ddsi_addrset_empty_mc(const struct ddsi_addrset * as);

/**
 * @component locators
 * 从地址集中获取任意一个单播地址
 * @param as 要查询的地址集
 * @param dst 存储找到的单播地址的指针
 * @return 如果找到单播地址，返回非0值；否则返回0
 */
int ddsi_addrset_any_uc(const struct ddsi_addrset * as, ddsi_xlocator_t * dst);

/**
 * @component locators
 * 从地址集中获取任意一个多播地址
 * @param as 要查询的地址集
 * @param dst 存储找到的多播地址的指针
 * @return 如果找到多播地址，返回非0值；否则返回0
 */
int ddsi_addrset_any_mc(const struct ddsi_addrset * as, ddsi_xlocator_t * dst);

/**
 * @component locators
 * 从地址集中获取任意一个单播地址，如果没有单播地址，则获取任意一个多播地址
 * @param as 要查询的地址集
 * @param dst 存储找到的地址的指针
 */
void ddsi_addrset_any_uc_else_mc_nofail(const struct ddsi_addrset * as, ddsi_xlocator_t * dst);

/**
 * @component locators
 * 对地址集中的每个地址执行给定的函数，并保持地址集锁定
 * @param as 要遍历的地址集
 * @param f 要对每个地址执行的函数
 * @param arg 传递给函数的参数
 * @return 函数执行成功的次数
 */
int ddsi_addrset_forone(struct ddsi_addrset * as, ddsi_addrset_forone_fun_t f, void * arg);

/**
 * @component locators
 * 对地址集中的每个地址执行给定的函数，并统计函数执行成功的次数
 * @param as 要遍历的地址集
 * @param f 要对每个地址执行的函数
 * @param arg 传递给函数的参数
 * @return 函数执行成功的次数
 */
size_t ddsi_addrset_forall_count(struct ddsi_addrset * as, ddsi_addrset_forall_fun_t f, void * arg);

/**
 * @component locators
 * 对地址集中的每个单播地址执行给定的函数，如果没有单播地址，则执行多播地址，并统计函数执行成功的次数
 * @param as 要遍历的地址集
 * @param f 要对每个地址执行的函数
 * @param arg 传递给函数的参数
 * @return 函数执行成功的次数
 */
size_t ddsi_addrset_forall_uc_else_mc_count(
  struct ddsi_addrset * as, ddsi_addrset_forall_fun_t f, void * arg);

/**
 * @component locators
 * 对地址集中的每个多播地址执行给定的函数，并统计函数执行成功的次数
 * @param as 要遍历的地址集
 * @param f 要对每个地址执行的函数
 * @param arg 传递给函数的参数
 * @return 函数执行成功的次数
 */
size_t ddsi_addrset_forall_mc_count(
  struct ddsi_addrset * as, ddsi_addrset_forall_fun_t f, void * arg);

/**
 * @component locators
 * 记录地址集的信息
 * @param gv 全局变量结构体指针
 * @param tf 日志标志
 * @param prefix 日志前缀
 * @param as 要记录的地址集
 */
void ddsi_log_addrset(
  struct ddsi_domaingv * gv, uint32_t tf, const char * prefix, const struct ddsi_addrset * as);

/**
 * @component locators
 * 尝试锁定 a 和 b 以进行比较，如果尝试锁定 b 失败，则返回 false
 * @param a 地址集
 * @param b 要与之比较的地址集
 * @return 如果地址集相等，返回非0值；否则返回0
 */
int ddsi_addrset_eq_onesidederr(const struct ddsi_addrset * a, const struct ddsi_addrset * b);

/**
 * @component locators
 * 判断定位器是否未指定
 * @param loc 要检查的定位器
 * @return 如果定位器未指定，返回非0值；否则返回0
 */
int ddsi_is_unspec_locator(const ddsi_locator_t * loc);

/**
 * @component locators
 * 判断扩展定位器是否未指定
 * @param loc 要检查的扩展定位器
 * @return 如果扩展定位器未指定，返回非0值；否则返回0
 */
int ddsi_is_unspec_xlocator(const ddsi_xlocator_t * loc);

/**
 * @component locators
 * 设置定位器为未指定状态
 * @param loc 要设置的定位器
 */
void ddsi_set_unspec_locator(ddsi_locator_t * loc);

/**
 * @component locators
 * 设置扩展定位器为未指定状态
 * @param loc 要设置的扩展定位器
 */
void ddsi_set_unspec_xlocator(ddsi_xlocator_t * loc);

/**
 * @component locators
 * 将给定的地址和端口添加到地址集中
 * @param gv 全局变量结构体指针
 * @param as 要添加地址的地址集
 * @param addrs 要添加的地址字符串，多个地址以逗号分隔
 * @param port_mode 端口模式
 * @param msgtag 消息标签
 * @param req_mc 是否需要多播地址
 * @return 成功添加的地址数量
 */
int ddsi_add_addresses_to_addrset(
  const struct ddsi_domaingv * gv, struct ddsi_addrset * as, const char * addrs, int port_mode,
  const char * msgtag, int req_mc);

#ifdef DDS_HAS_SSM

/**
 * @component locators
 * 判断地址集是否包含源特定多播(SSM)地址
 * @param gv 全局变量结构体指针
 * @param as 要检查的地址集
 * @return 如果包含 SSM 地址，返回非0值；否则返回0
 */
int ddsi_addrset_contains_ssm(const struct ddsi_domaingv * gv, const struct ddsi_addrset * as);

/**
 * @component locators
 * 从地址集中获取任意一个源特定多播(SSM)地址
 * @param gv 全局变量结构体指针
 * @param as 要查询的地址集
 * @param dst 存储找到的 SSM 地址的指针
 * @return 如果找到 SSM 地址，返回非0值；否则返回0
 */
int ddsi_addrset_any_ssm(
  const struct ddsi_domaingv * gv, const struct ddsi_addrset * as, ddsi_xlocator_t * dst);

/**
 * @component locators
 * 从地址集中获取任意一个非源特定多播(SSM)的多播地址
 * @param gv 全局变量结构体指针
 * @param as 要查询的地址集
 * @param dst 存储找到的非 SSM 多播地址的指针
 * @return 如果找到非 SSM 多播地址，返回非0值；否则返回0
 */
int ddsi_addrset_any_non_ssm_mc(
  const struct ddsi_domaingv * gv, const struct ddsi_addrset * as, ddsi_xlocator_t * dst);

/**
 * @component locators
 * 将 asadd 中的非源特定多播(SSM)的多播地址复制到 as 中
 * @param gv 全局变量结构体指针
 * @param as 目标地址集
 * @param asadd 源地址集
 */
void ddsi_copy_addrset_into_addrset_no_ssm_mc(
  const struct ddsi_domaingv * gv, struct ddsi_addrset * as, const struct ddsi_addrset * asadd);

/**
 * @component locators
 * 将 asadd 中的非源特定多播(SSM)地址复制到 as 中
 * @param gv 全局变量结构体指针
 * @param as 目标地址集
 * @param asadd 源地址集
 */
void ddsi_copy_addrset_into_addrset_no_ssm(
  const struct ddsi_domaingv * gv, struct ddsi_addrset * as, const struct ddsi_addrset * asadd);

#endif /* DDS_HAS_SSM */

#if defined(__cplusplus)
}
#endif
#endif /* DDSI__ADDRSET_H */
