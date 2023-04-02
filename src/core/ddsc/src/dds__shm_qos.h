/*
 * Copyright(c) 2023 ZettaScale Technology and others
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v. 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0, or the Eclipse Distribution License
 * v. 1.0 which is available at
 * http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * SPDX-License-Identifier: EPL-2.0 OR BSD-3-Clause
 */
#ifndef DDS__SHM_QOS_H
#define DDS__SHM_QOS_H

#if defined(__cplusplus)
extern "C"
{
#endif

  struct dds_qos;
  struct dds_topic;

  /**
   * @brief 检查DDS QoS设置和主题是否与Iceoryx兼容
   * @component iceoryx_support
   *
   * @param[in] qos 要检查兼容性的QoS
   * @param[in] tp 主题
   * @param[in] check_durability_service 是否包括此项
   *
   * @return 如果兼容，则返回true
   */
  bool dds_shm_compatible_qos_and_topic(const struct dds_qos *qos, const struct dds_topic *tp, bool check_durability_service);

  /**
   * @brief 为Iceoryx中的分区和主题字符串构建一个字符串表示形式
   * @component iceoryx_support
   *
   * @param[in] qos 从中获取分区的QoS
   * @param[in] tp 主题
   *
   * @return 如果分区兼容且有足够的内存可用，则返回一个已分配的空终止字符串
   */
  char *dds_shm_partition_topic(const struct dds_qos *qos, const struct dds_topic *tp);

#if defined(__cplusplus)
}
#endif

#endif /* DDS__SHM_MONITOR_H */
