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
#ifndef DDSI_PROXY_PARTICIPANT_H
#define DDSI_PROXY_PARTICIPANT_H

#include "dds/ddsi/ddsi_domaingv.h"
#include "dds/ddsi/ddsi_topic.h"
#include "dds/ddsrt/atomics.h"
#include "dds/ddsrt/avl.h"
#include "dds/ddsrt/fibheap.h"
#include "dds/export.h"
#include "dds/features.h"

#if defined(__cplusplus)
extern "C" {
#endif

struct ddsi_lease;
struct ddsi_plist;
struct ddsi_addrset;
struct ddsi_proxy_endpoint_common;

/**
 * @struct ddsi_proxy_participant
 * @brief 代理参与者结构体，用于存储远程参与者的信息 (Structure for proxy participant, used to store
 * information about remote participants)
 */
struct ddsi_proxy_participant {
  struct ddsi_entity_common e; /**< 实体通用结构体 (Common entity structure) */
  uint32_t
      refc; /**< 代理端点数量（包括用户和内置端点；不包括组，因为它们没有自己的生命周期）(Number of
               proxy endpoints (both user & built-in; not groups, they don't have a life of their
               own)) */
  ddsi_vendorid_t vendor; /**< 发现过程中的厂商代码 (Vendor code from discovery) */
  unsigned bes;           /**< 内置端点集合 (Built-in endpoint set) */
  ddsi_guid_t privileged_pp_guid; /**< 如果此PP依赖于另一个PP进行SEDP写入 (If this PP depends on
                                     another PP for its SEDP writing) */
  struct ddsi_plist* plist; /**< 此参与者的设置/QoS (Settings/QoS for this participant) */
  ddsrt_atomic_voidp_t minl_auto; /**< min(leaseheap_auto)的克隆 (Clone of min(leaseheap_auto)) */
  ddsrt_fibheap_t
      leaseheap_auto; /**< 保存此proxypp的租约以及具有活动性自动的pwrs的租约 (Keeps leases for this
                         proxypp and leases for pwrs (with liveliness automatic)) */
  ddsrt_atomic_voidp_t minl_man; /**< min(leaseheap_man)的克隆 (Clone of min(leaseheap_man)) */
  ddsrt_fibheap_t leaseheap_man; /**< 保存此proxypp的租约以及具有活动性手动按参与者的pwrs的租约
                                    (Keeps leases for this proxypp and leases for pwrs (with
                                    liveliness manual-by-participant)) */
  struct ddsi_lease* lease; /**< 此proxypp的租约 (Lease for this proxypp) */
  struct ddsi_addrset* as_default; /**< 用于用户数据流量的默认地址集 (Default address set to use for
                                      user data traffic) */
  struct ddsi_addrset*
      as_meta; /**< 用于发现流量的默认地址集 (Default address set to use for discovery traffic) */
  struct ddsi_proxy_endpoint_common*
      endpoints; /**< 所有代理端点都可以从这里访问 (All proxy endpoints can be reached from here) */
#ifdef DDS_HAS_TOPIC_DISCOVERY
  ddsrt_avl_tree_t topics; /**< 主题树 (Topic tree) */
#endif
  ddsi_seqno_t seq; /**< 最近SPDP消息的序列号 (Sequence number of most recent SPDP message) */
  uint32_t
      receive_buffer_size; /**< 接收缓冲区的假定大小，用于限制涉及此proxypp的突发 (Assumed size of
                              receive buffer, used to limit bursts involving this proxypp) */
  unsigned implicitly_created : 1; /**< 对于Cloud/Fog发现的端点，参与者是隐式创建的 (Participants
                                      are implicitly created for Cloud/Fog discovered endpoints) */
  unsigned is_ddsi2_pp : 1; /**< 如果这是远程节点上的联邦领导者 (If this is the federation-leader on
                               the remote node) */
  unsigned minimal_bes_mode : 1;  /**< 最小BES模式标志 (Minimal BES mode flag) */
  unsigned lease_expired : 1;     /**< 租约过期标志 (Lease expired flag) */
  unsigned deleting : 1;          /**< 删除标志 (Deleting flag) */
  unsigned proxypp_have_spdp : 1; /**< ProxyPP具有SPDP标志 (ProxyPP have SPDP flag) */
  unsigned owns_lease : 1;        /**< 拥有租约标志 (Owns lease flag) */
  unsigned redundant_networking : 1; /**< 如果在所有广告接口上请求接收数据，则为1 (1 iff requests
                                        receiving data on all advertised interfaces) */
#ifdef DDS_HAS_SECURITY
  ddsi_security_info_t security_info; /**< 安全信息 (Security information) */
  struct ddsi_proxy_participant_sec_attributes* sec_attr; /**< 安全属性 (Security attributes) */
#endif
};

#ifdef DDS_HAS_TOPIC_DISCOVERY
extern const ddsrt_avl_treedef_t ddsi_proxypp_proxytp_treedef;
#endif

#if defined(__cplusplus)
}
#endif

#endif /* DDSI_PROXY_PARTICIPANT_H */
