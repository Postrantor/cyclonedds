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
#ifndef DDS__GET_STATUS_H
#define DDS__GET_STATUS_H

/**
 * @brief 获取实体的状态（已锁定）
 * Get the status of an entity (locked)
 *
 * @param[in] ent_type_ 实体类型，例如：reader、writer 等
 * Entity type, such as: reader, writer, etc.
 * @param[in] status_ 要获取的状态类型，例如：inconsistent_topic、sample_lost 等
 * The status type to get, such as: inconsistent_topic, sample_lost, etc.
 * @param[in] STATUS_ 状态宏，例如：INCONSISTENT_TOPIC、SAMPLE_LOST 等
 * Status macro, such as: INCONSISTENT_TOPIC, SAMPLE_LOST, etc.
 * @param[in] ... 可变参数列表，用于重置状态字段
 * Variable argument list for resetting status fields
 */
#define DDS_GET_STATUS_LOCKED(ent_type_, status_, STATUS_, ...)                                          \
  /**                                                                                                    \
   * @brief 静态函数：获取指定状态的实体（已锁定）                                    \
   * Static function: Get the entity with the specified status (locked)                                  \
   *                                                                                                     \
   * @param[in] ent 指向 dds_##ent_type_ 类型的指针                                               \
   * Pointer to a dds_##ent_type_ type                                                                   \
   * @param[out] status 指向 dds_##status_##_status_t 类型的指针，用于存储获取到的状态 \
   * Pointer to a dds_##status_##_status_t type, used to store the obtained status                       \
   */                                                                                                    \
  static void dds_get_##status_##_status_locked(dds_##ent_type_* ent,                                    \
                                                dds_##status_##_status_t* status) {                      \
    /**                                                                                                  \
     * @brief 如果 status 非空，则将实体的状态赋值给 status                               \
     * If status is not NULL, assign the entity's status to status                                       \
     */                                                                                                  \
    if (status) *status = ent->m_##status_##_status;                                                     \
    /**                                                                                                  \
     * @brief 重置指定的状态字段                                                                \
     * Reset the specified status fields                                                                 \
     */                                                                                                  \
    do {                                                                                                 \
      DDS_RESET_STATUS_FIELDS_N(DDSRT_COUNT_ARGS(__VA_ARGS__), ent, status_, __VA_ARGS__)                \
    } while (0);                                                                                         \
    /**                                                                                                  \
     * @brief 重置实体的状态                                                                      \
     * Reset the entity's status                                                                         \
     */                                                                                                  \
    dds_entity_status_reset(&ent->m_entity, DDS_##STATUS_##_STATUS);                                     \
  }

/**
 * @brief 定义一个宏，用于获取 DDS 状态。
 * Define a macro for getting DDS status.
 *
 * @param ent_type_ 实体类型。Entity type.
 * @param status_ 状态类型。Status type.
 */
#define DDS_GET_STATUS_COMMON(ent_type_, status_)                                                   \
  dds_return_t dds_get_##status_##_status(dds_entity_t entity, dds_##status_##_status_t* status) {  \
    /* 定义实体指针。Define the entity pointer. */                                           \
    dds_##ent_type_* ent;                                                                           \
    /* 定义返回值。Define the return value. */                                                \
    dds_return_t ret;                                                                               \
    /* 尝试锁定实体并检查返回值。Try to lock the entity and check the return value. */ \
    if ((ret = dds_##ent_type_##_lock(entity, &ent)) != DDS_RETCODE_OK) return ret;                 \
    /* 锁定观察者互斥锁。Lock the observers mutex. */                                      \
    ddsrt_mutex_lock(&ent->m_entity.m_observers_lock);                                              \
    /* 获取状态（已锁定）。Get the status (locked). */                                    \
    dds_get_##status_##_status_locked(ent, status);                                                 \
    /* 解锁观察者互斥锁。Unlock the observers mutex. */                                    \
    ddsrt_mutex_unlock(&ent->m_entity.m_observers_lock);                                            \
    /* 解锁实体。Unlock the entity. */                                                         \
    dds_##ent_type_##_unlock(ent);                                                                  \
    /* 返回成功状态码。Return the success status code. */                                   \
    return DDS_RETCODE_OK;                                                                          \
  }

/**
 * @brief 定义一个宏，用于获取 DDS 状态（包括锁定和常规状态）。
 * Define a macro for getting DDS status (including locked and common status).
 *
 * @param ent_type_ 实体类型。Entity type.
 * @param status_ 状态类型。Status type.
 * @param STATUS_ 大写状态类型。Uppercase status type.
 * @param ... 可变参数。Variable arguments.
 */
#define DDS_GET_STATUS(ent_type_, status_, STATUS_, ...)          \
  /* 获取已锁定的状态。Get the locked status. */         \
  DDS_GET_STATUS_LOCKED(ent_type_, status_, STATUS_, __VA_ARGS__) \
  /* 获取常规状态。Get the common status. */               \
  DDS_GET_STATUS_COMMON(ent_type_, status_)

#endif /* DDS__GET_STATUS_H */
