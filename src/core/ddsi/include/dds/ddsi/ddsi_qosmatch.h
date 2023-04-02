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
#ifndef DDSI_QOSMATCH_H
#define DDSI_QOSMATCH_H

#include "dds/ddsi/ddsi_typelib.h"
#include "dds/ddsi/ddsi_xqos.h"
#include "dds/features.h"
#if defined(__cplusplus)
extern "C" {
#endif

struct ddsi_domaingv;

/**
 * @brief 执行读写器QoS（以及主题名称、类型名称、分区）匹配 / Perform reader/writer QoS (and topic
 * name, type name, partition) matching
 * @component qos_matching
 *
 * @param gv 域全局变量 / Domain globals
 * @param rd_qos 读取器QoS / Reader QoS
 * @param wr_qos 写入器QoS / Writer QoS
 * @param mask 可用于排除其中一些（包括主题名称和类型名称，所以要小心！）/ Can be used to exclude
 * some of these (including topic name and type name, so be careful!)
 * @param reason
 * 将设置为不匹配QoS之一的策略ID，如果没有不匹配或者不匹配在主题或类型名称中（这些不是真正的QoS并且没有策略ID），则设置为DDS_INVALID_QOS_POLICY_ID
 * / Will be set to the policy id of one of the mismatching QoS, or to DDS_INVALID_QOS_POLICY_ID if
 * there is no mismatch or if the mismatch is in topic or type name (those are not really QoS and
 * don't have a policy id)
 * @param rd_type_pair 读取器的最小和完整ddsi类型 / Minimal and complete ddsi type for the reader
 * @param wr_type_pair 写入器的最小和完整ddsi类型 / Minimal and complete ddsi type for the writer
 * @param rd_typeid_req_lookup
 * 如果由于缺少类型信息而无法完成匹配，则设置为true。需要进行类型查找请求以获取类型的详细信息以进行QoS匹配（例如检查可分配性）/
 * Is set to true in case the matching cannot be completed because of missing type information. A
 * type-lookup request is required to get the details of the type to do the qos matching (e.g. check
 * assignability)
 * @param wr_typeid_req_lookup 参见rd_typeid_req_lookup / See rd_typeid_req_lookup
 *
 * @returns 如果匹配，则为true，否则为false / True in case of a match, false otherwise
 */
bool ddsi_qos_match_mask_p(struct ddsi_domaingv* gv,
                           const dds_qos_t* rd_qos,
                           const dds_qos_t* wr_qos,
                           uint64_t mask,
                           dds_qos_policy_id_t* reason
#ifdef DDS_HAS_TYPE_DISCOVERY
                           ,
                           const struct ddsi_type_pair* rd_type_pair,
                           const struct ddsi_type_pair* wr_type_pair,
                           bool* rd_typeid_req_lookup,
                           bool* wr_typeid_req_lookup
#endif
);

/** @component qos_matching */
bool ddsi_qos_match_p(struct ddsi_domaingv* gv,
                      const dds_qos_t* rd_qos,
                      const dds_qos_t* wr_qos,
                      dds_qos_policy_id_t* reason
#ifdef DDS_HAS_TYPE_DISCOVERY
                      ,
                      const struct ddsi_type_pair* rd_type_pair,
                      const struct ddsi_type_pair* wr_type_pair,
                      bool* rd_typeid_req_lookup,
                      bool* wr_typeid_req_lookup
#endif
);

#if defined(__cplusplus)
}
#endif

#endif /* DDSI_QOSMATCH_H */
