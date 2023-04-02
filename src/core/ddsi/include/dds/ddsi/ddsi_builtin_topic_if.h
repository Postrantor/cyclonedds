/*
 * Copyright(c) 2019 to 2021 ZettaScale Technology and others
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v. 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0, or the Eclipse Distribution License
 * v. 1.0 which is available at
 * http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * SPDX-License-Identifier: EPL-2.0 OR BSD-3-Clause
 */
#ifndef DDSI_BUILTIN_TOPIC_IF_H
#define DDSI_BUILTIN_TOPIC_IF_H

#if defined(__cplusplus)
extern "C" {
#endif

struct ddsi_entity_common;
struct ddsi_tkmap_instance;
struct ddsi_sertype;
struct ddsi_guid;
struct ddsi_topic_definition;

/**
 * @struct ddsi_builtin_topic_interface
 * @brief 内置主题接口结构体，用于处理内置主题相关操作 (Structure for builtin topic interface, used to handle operations related to builtin topics)
 */
struct ddsi_builtin_topic_interface {
  void* arg; /**< 参数，传递给回调函数 (Argument passed to the callback functions) */

  /**
   * @brief 判断类型是否为内置主题 (Determine if the type is a builtin topic)
   * @param[in] type 序列化类型对象 (Serialized type object)
   * @param[in] arg 参数 (Argument)
   * @return 如果是内置主题，则返回 true；否则返回 false (Returns true if it's a builtin topic, otherwise returns false)
   */
  bool (*builtintopic_is_builtintopic)(const struct ddsi_sertype* type, void* arg);

  /**
   * @brief 判断 GUID 是否可见 (Determine if the GUID is visible)
   * @param[in] guid GUID 对象 (GUID object)
   * @param[in] vendorid 供应商 ID (Vendor ID)
   * @param[in] arg 参数 (Argument)
   * @return 如果可见，则返回 true；否则返回 false (Returns true if it's visible, otherwise returns false)
   */
  bool (*builtintopic_is_visible)(const struct ddsi_guid* guid,
                                  ddsi_vendorid_t vendorid,
                                  void* arg);

  /**
   * @brief 获取 tkmap_entry 对象 (Get the tkmap_entry object)
   * @param[in] guid GUID 对象 (GUID object)
   * @param[in] arg 参数 (Argument)
   * @return 返回 tkmap_entry 对象 (Returns the tkmap_entry object)
   */
  struct ddsi_tkmap_instance* (*builtintopic_get_tkmap_entry)(const struct ddsi_guid* guid,
                                                              void* arg);

  /**
   * @brief 写入端点信息 (Write endpoint information)
   * @param[in] e 实体对象 (Entity object)
   * @param[in] timestamp 时间戳 (Timestamp)
   * @param[in] alive 是否存活 (Alive status)
   * @param[in] arg 参数 (Argument)
   */
  void (*builtintopic_write_endpoint)(const struct ddsi_entity_common* e,
                                      ddsrt_wctime_t timestamp,
                                      bool alive,
                                      void* arg);

  /**
   * @brief 写入主题定义信息 (Write topic definition information)
   * @param[in] tpd 主题定义对象 (Topic definition object)
   * @param[in] timestamp 时间戳 (Timestamp)
   * @param[in] alive 是否存活 (Alive status)
   * @param[in] arg 参数 (Argument)
   */
  void (*builtintopic_write_topic)(const struct ddsi_topic_definition* tpd,
                                   ddsrt_wctime_t timestamp,
                                   bool alive,
                                   void* arg);
};

/** @component builtintopic_if */
inline bool ddsi_builtintopic_is_visible(const struct ddsi_builtin_topic_interface* btif,
                                         const struct ddsi_guid* guid,
                                         ddsi_vendorid_t vendorid) {
  // 如果 btif 存在，则调用回调函数，否则返回 false (If btif exists, call the callback function, otherwise return false)
  return btif ? btif->builtintopic_is_visible(guid, vendorid, btif->arg) : false;
}

/** @component builtintopic_if */
inline bool ddsi_builtintopic_is_builtintopic(const struct ddsi_builtin_topic_interface* btif,
                                              const struct ddsi_sertype* type) {
  // 如果 btif 存在，则调用回调函数，否则返回 false (If btif exists, call the callback function, otherwise return false)
  return btif ? btif->builtintopic_is_builtintopic(type, btif->arg) : false;
}

/** @component builtintopic_if */
inline struct ddsi_tkmap_instance* ddsi_builtintopic_get_tkmap_entry(
    const struct ddsi_builtin_topic_interface* btif, const struct ddsi_guid* guid) {
  // 如果 btif 存在，则调用回调函数，否则返回 NULL (If btif exists, call the callback function, otherwise return NULL)
  return btif ? btif->builtintopic_get_tkmap_entry(guid, btif->arg) : NULL;
}

/** @component builtintopic_if */
inline void ddsi_builtintopic_write_endpoint(const struct ddsi_builtin_topic_interface* btif,
                                             const struct ddsi_entity_common* e,
                                             ddsrt_wctime_t timestamp,
                                             bool alive) {
  // 如果 btif 存在，则调用回调函数 (If btif exists, call the callback function)
  if (btif) btif->builtintopic_write_endpoint(e, timestamp, alive, btif->arg);
}

/** @component builtintopic_if */

/**
 * @brief 写入内置主题 (Write builtin topic)
 *
 * @param[in] btif 内置主题接口指针 (Pointer to the builtin topic interface)
 * @param[in] tpd 主题定义指针 (Pointer to the topic definition)
 * @param[in] timestamp 写操作的时间戳 (Timestamp of the write operation)
 * @param[in] alive 主题是否存活 (Whether the topic is alive or not)
 */
inline void ddsi_builtintopic_write_topic(const struct ddsi_builtin_topic_interface* btif,
                                          const struct ddsi_topic_definition* tpd,
                                          ddsrt_wctime_t timestamp,
                                          bool alive) {
  // 如果内置主题接口存在 (If the builtin topic interface exists)
  if (btif)
    // 调用内置主题接口的写入主题函数，传入参数 (Call the write_topic function of the builtin topic interface with the given parameters)
    btif->builtintopic_write_topic(tpd, timestamp, alive, btif->arg);
}

#if defined(__cplusplus)
}
#endif

#endif
