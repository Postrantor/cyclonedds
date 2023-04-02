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
#ifndef DDS__TYPES_H
#define DDS__TYPES_H

/* DDS internal type definitions */

#include "dds/dds.h"
#include "dds/ddsi/ddsi_domaingv.h"
#include "dds/ddsi/ddsi_protocol.h"
#include "dds/ddsrt/sync.h"
#ifdef DDS_HAS_TOPIC_DISCOVERY
#include "dds/ddsi/ddsi_typewrap.h"
#endif
#include "dds/ddsi/ddsi_builtin_topic_if.h"
#include "dds/ddsrt/avl.h"
#include "dds__handles.h"

#ifdef DDS_HAS_SHM
#include "dds/ddsi/ddsi_shm_transport.h"
#include "dds__shm_monitor.h"
#include "iceoryx_binding_c/publisher.h"
#include "iceoryx_binding_c/subscriber.h"
#define MAX_PUB_LOANS 8
#endif

#if defined(__cplusplus)
extern "C" {
#endif

struct dds_domain;
struct dds_entity;
struct dds_participant;
struct dds_reader;
struct dds_writer;
struct dds_publisher;
struct dds_subscriber;
struct dds_topic;
struct dds_ktopic;
struct dds_readcond;
struct dds_guardcond;
struct dds_statuscond;

struct ddsi_sertype;
struct ddsi_rhc;

typedef uint16_t status_mask_t;
typedef ddsrt_atomic_uint32_t status_and_enabled_t;
#define SAM_STATUS_MASK 0xffffu
#define SAM_ENABLED_MASK 0xffff0000u
#define SAM_ENABLED_SHIFT 16

/**
 * @brief 轮询等待超时时间定义
 *
 * 这可以在轮询各种状态时使用。显然，鼓励使用条件变量等。
 * 但是有时候它不会产生太大的差异，采取简单的方法是比较务实的。
 */
#define DDS_HEADBANG_TIMEOUT (DDS_MSECS(10))

typedef bool (*dds_querycondition_filter_with_ctx_fn)(const void* sample, const void* ctx);

/* The listener struct. */

/**
 * @brief dds_listener 结构体，用于存储 DDS 监听器的回调函数和参数。
 */
struct dds_listener {
  uint32_t inherited;        ///< 继承标志位，表示哪些回调函数被继承

  uint32_t reset_on_invoke;  ///< 重置标志位，在调用回调函数时是否重置监听器

  // 不一致主题回调函数及其参数
  dds_on_inconsistent_topic_fn on_inconsistent_topic;  ///< 当检测到不一致的主题时触发的回调函数
  void* on_inconsistent_topic_arg;                     ///< 不一致主题回调函数的参数

  // 生命期丢失回调函数及其参数
  dds_on_liveliness_lost_fn on_liveliness_lost;  ///< 当生命期丢失时触发的回调函数
  void* on_liveliness_lost_arg;                  ///< 生命期丢失回调函数的参数

  // 提供的截止日期未满足回调函数及其参数
  dds_on_offered_deadline_missed_fn
      on_offered_deadline_missed;  ///< 当提供的截止日期未满足时触发的回调函数
  void* on_offered_deadline_missed_arg;  ///< 提供的截止日期未满足回调函数的参数

  // 提供的不兼容 QoS 回调函数及其参数
  dds_on_offered_incompatible_qos_fn
      on_offered_incompatible_qos;        ///< 当提供的 QoS 不兼容时触发的回调函数
  void* on_offered_incompatible_qos_arg;  ///< 提供的不兼容 QoS 回调函数的参数

  // 数据读取器回调函数及其参数
  dds_on_data_on_readers_fn on_data_on_readers;  ///< 当数据可用于读取器时触发的回调函数
  void* on_data_on_readers_arg;                  ///< 数据读取器回调函数的参数

  // 样本丢失回调函数及其参数
  dds_on_sample_lost_fn on_sample_lost;  ///< 当样本丢失时触发的回调函数
  void* on_sample_lost_arg;              ///< 样本丢失回调函数的参数

  // 数据可用回调函数及其参数
  dds_on_data_available_fn on_data_available;  ///< 当数据可用时触发的回调函数
  void* on_data_available_arg;                 ///< 数据可用回调函数的参数

  // 样本拒绝回调函数及其参数
  dds_on_sample_rejected_fn on_sample_rejected;  ///< 当样本被拒绝时触发的回调函数
  void* on_sample_rejected_arg;                  ///< 样本拒绝回调函数的参数

  // 生命期改变回调函数及其参数
  dds_on_liveliness_changed_fn on_liveliness_changed;  ///< 当生命期状态改变时触发的回调函数
  void* on_liveliness_changed_arg;                     ///< 生命期改变回调函数的参数

  // 请求的截止日期未满足回调函数及其参数
  dds_on_requested_deadline_missed_fn
      on_requested_deadline_missed;  ///< 当请求的截止日期未满足时触发的回调函数
  void* on_requested_deadline_missed_arg;  ///< 请求的截止日期未满足回调函数的参数

  // 请求的不兼容 QoS 回调函数及其参数
  dds_on_requested_incompatible_qos_fn
      on_requested_incompatible_qos;        ///< 当请求的 QoS 不兼容时触发的回调函数
  void* on_requested_incompatible_qos_arg;  ///< 请求的不兼容 QoS 回调函数的参数

  // 出版匹配回调函数及其参数
  dds_on_publication_matched_fn on_publication_matched;  ///< 当出版物匹配时触发的回调函数
  void* on_publication_matched_arg;                      ///< 出版匹配回调函数的参数

  // 订阅匹配回调函数及其参数
  dds_on_subscription_matched_fn on_subscription_matched;  ///< 当订阅匹配时触发的回调函数
  void* on_subscription_matched_arg;                       ///< 订阅匹配回调函数的参数
};

/* Entity flag values */

#define DDS_ENTITY_ENABLED ((uint32_t)0x1) /* DDS "enabled" state */
#define DDS_ENTITY_IMPLICIT \
  ((uint32_t)0x2) /* implicit ones get deleted when the last child is deleted */

struct dds_domain;
struct dds_entity;

// 定义一个名为 dds_entity_deriver 的结构体类型
typedef struct dds_entity_deriver {
  // 中断函数，用于在实际删除实体之前终止对实体的（阻塞）操作
  // 参数 e: 需要中断的实体指针
  void (*interrupt)(struct dds_entity* e) ddsrt_nonnull_all;

  // 关闭函数，用于执行...
  // 参数 e: 需要关闭的实体指针
  void (*close)(struct dds_entity* e) ddsrt_nonnull_all;

  // 删除函数，用于实际释放实体
  // 参数 e: 需要删除的实体指针
  // 返回值: 操作结果，成功返回 DDS_RETCODE_OK
  dds_return_t (*delete)(struct dds_entity* e) ddsrt_nonnull_all;

  // 设置 QoS 函数
  // 参数 e: 需要设置 QoS 的实体指针
  // 参数 qos: QoS 设置
  // 参数 enabled: 是否启用 QoS
  // 返回值: 操作结果，成功返回 DDS_RETCODE_OK
  dds_return_t (*set_qos)(struct dds_entity* e,
                          const dds_qos_t* qos,
                          bool enabled) ddsrt_nonnull_all;

  // 验证状态函数
  // 参数 mask: 状态掩码
  // 返回值: 操作结果，成功返回 DDS_RETCODE_OK
  dds_return_t (*validate_status)(uint32_t mask);

  // 创建统计信息函数
  // 参数 e: 需要创建统计信息的实体指针
  // 返回值: 创建的统计信息结构指针
  struct dds_statistics* (*create_statistics)(const struct dds_entity* e);

  // 刷新统计信息函数
  // 参数 e: 需要刷新统计信息的实体指针
  // 参数 s: 统计信息结构指针
  void (*refresh_statistics)(const struct dds_entity* e, struct dds_statistics* s);
} dds_entity_deriver;

// 定义一个名为 dds_waitset 的结构体类型
struct dds_waitset;

// 定义实体回调函数类型
// 参数 observer: 观察者指针
// 参数 observed: 被观察的实体
// 参数 status: 状态值
typedef void (*dds_entity_callback_t)(struct dds_waitset* observer,
                                      dds_entity_t observed,
                                      uint32_t status);

// 定义实体附加回调函数类型
// 参数 observer: 观察者指针
// 参数 observed: 被观察的实体指针
// 参数 attach_arg: 附加参数
// 返回值: 是否成功附加
typedef bool (*dds_entity_attach_callback_t)(struct dds_waitset* observer,
                                             struct dds_entity* observed,
                                             void* attach_arg);

// 定义实体删除回调函数类型
// 参数 observer: 观察者指针
// 参数 observed: 被观察的实体
typedef void (*dds_entity_delete_callback_t)(struct dds_waitset* observer, dds_entity_t observed);

// 定义一个名为 dds_entity_observer 的结构体类型
typedef struct dds_entity_observer {
  // 实体回调函数指针
  dds_entity_callback_t m_cb;

  // 实体删除回调函数指针
  dds_entity_delete_callback_t m_delete_cb;

  // 观察者指针
  struct dds_waitset* m_observer;

  // 下一个实体观察者指针
  struct dds_entity_observer* m_next;
} dds_entity_observer;

/**
 * @struct dds_entity
 * @brief 定义了一个DDS实体结构，包含实体的基本信息和状态。
 */
typedef struct dds_entity {
  struct dds_handle_link m_hdllink; /**< handle是常量，cnt_flags私有于dds_handle.c */
  dds_entity_kind_t m_kind;         /**< 常量，表示实体类型 */
  struct dds_entity* m_next;        /**< [m_mutex] 指向下一个实体 */
  struct dds_entity* m_parent;      /**< 常量，指向父实体 */
  ddsrt_avl_node_t m_avlnode_child; /**< [m_mutex of m_parent] 子节点的AVL树节点 */
  ddsrt_avl_tree_t m_children;      /**< [m_mutex] 使用m_avlnode_child的子节点AVL树 */
  struct dds_domain* m_domain;      /**< 常量，指向实体所属的域 */
  dds_qos_t*
      m_qos;                        /**< [m_mutex]
                                       实体的QoS设置；对于主题（topic）为null（它们依赖于对应的"ktopic"）(+waitset,domain,&c.)
                                     */
  ddsi_guid_t
      m_guid; /**< 唯一（如果非0）且常量；FIXME: 在创建过程中设置，但可能在变得可见之后才设置 */
  dds_instance_handle_t m_iid; /**< 唯一且常量的实例句柄；FIXME: 类似GUID */
  uint32_t m_flags;            /**< [m_mutex] 标志位 */

  /**
   * 允许的操作：
   * - 在持有m_mutex的情况下锁定parent->...->m_mutex
   * - 在持有{publisher,subscriber}::m_mutex的情况下锁定topic::m_mutex（没有层次关系）
   * - 在持有{reader,writer}::m_mutex的情况下锁定topic::m_mutex
   * - 在持有m_mutex的情况下锁定observers_lock
   * - 锁定waitset::wait_lock
   */
  ddsrt_mutex_t m_mutex; /**< 实体互斥锁 */
  ddsrt_cond_t m_cond;   /**< 实体条件变量 */

  union {
    status_and_enabled_t
        m_status_and_mask; /**< 对于大多数实体；读者在mask中以一种奇怪的方式使用DATA_ON_READERS */
    ddsrt_atomic_uint32_t m_trigger; /**< 对于条件和等待集合 */
  } m_status;                        /**< 实体状态 */

  ddsrt_mutex_t
      m_observers_lock; /**< 观察者互斥锁，允许在持有它的情况下锁定parent->...->m_observers_lock */
  ddsrt_cond_t m_observers_cond;    /**< 观察者条件变量 */
  dds_listener_t m_listener;        /**< [m_observers_lock] 监听器 */
  uint32_t m_cb_count;              /**< [m_observers_lock] 回调计数 */
  uint32_t m_cb_pending_count;      /**< [m_observers_lock] 待处理回调计数 */
  dds_entity_observer* m_observers; /**< [m_observers_lock] 观察者列表 */
} dds_entity;

extern const ddsrt_avl_treedef_t dds_topictree_def;
extern const ddsrt_avl_treedef_t dds_entity_children_td;

extern const struct dds_entity_deriver dds_entity_deriver_topic;
extern const struct dds_entity_deriver dds_entity_deriver_participant;
extern const struct dds_entity_deriver dds_entity_deriver_reader;
extern const struct dds_entity_deriver dds_entity_deriver_writer;
extern const struct dds_entity_deriver dds_entity_deriver_subscriber;
extern const struct dds_entity_deriver dds_entity_deriver_publisher;
extern const struct dds_entity_deriver dds_entity_deriver_readcondition;
extern const struct dds_entity_deriver dds_entity_deriver_guardcondition;
extern const struct dds_entity_deriver dds_entity_deriver_waitset;
extern const struct dds_entity_deriver dds_entity_deriver_domain;
extern const struct dds_entity_deriver dds_entity_deriver_cyclonedds;
extern const struct dds_entity_deriver* dds_entity_deriver_table[];

/** @notincomponent
 *  @brief 中断虚拟实体的操作
 *  @param[in] e 指向dds_entity结构体的指针
 */
void dds_entity_deriver_dummy_interrupt(struct dds_entity* e);

/** @notincomponent
 *  @brief 关闭虚拟实体
 *  @param[in] e 指向dds_entity结构体的指针
 */
void dds_entity_deriver_dummy_close(struct dds_entity* e);

/** @notincomponent
 *  @brief 删除虚拟实体
 *  @param[in] e 指向dds_entity结构体的指针
 *  @return 返回dds_return_t类型的结果
 */
dds_return_t dds_entity_deriver_dummy_delete(struct dds_entity* e);

/** @notincomponent
 *  @brief 设置虚拟实体的QoS
 *  @param[in] e 指向dds_entity结构体的指针
 *  @param[in] qos 指向dds_qos_t结构体的指针
 *  @param[in] enabled 布尔值，表示是否启用
 *  @return 返回dds_return_t类型的结果
 */
dds_return_t dds_entity_deriver_dummy_set_qos(struct dds_entity* e,
                                              const dds_qos_t* qos,
                                              bool enabled);

/** @notincomponent
 *  @brief 验证状态掩码
 *  @param[in] mask 32位无符号整数，表示状态掩码
 *  @return 返回dds_return_t类型的结果
 */
dds_return_t dds_entity_deriver_dummy_validate_status(uint32_t mask);

/** @notincomponent
 *  @brief 创建虚拟实体的统计信息
 *  @param[in] e 指向dds_entity结构体的指针
 *  @return 返回指向dds_statistics结构体的指针
 */
struct dds_statistics* dds_entity_deriver_dummy_create_statistics(const struct dds_entity* e);

/** @notincomponent
 *  @brief 刷新虚拟实体的统计信息
 *  @param[in] e 指向dds_entity结构体的指针
 *  @param[in] s 指向dds_statistics结构体的指针
 */
void dds_entity_deriver_dummy_refresh_statistics(const struct dds_entity* e,
                                                 struct dds_statistics* s);

/** @component generic_entity
 *  @brief 中断实体操作
 *  @param[in] e 指向dds_entity结构体的指针
 */
inline void dds_entity_deriver_interrupt(struct dds_entity* e) {
  (dds_entity_deriver_table[e->m_kind]->interrupt)(e);
}

/** @component generic_entity
 *  @brief 关闭实体
 *  @param[in] e 指向dds_entity结构体的指针
 */
inline void dds_entity_deriver_close(struct dds_entity* e) {
  (dds_entity_deriver_table[e->m_kind]->close)(e);
}

/** @component generic_entity
 *  @brief 删除实体
 *  @param[in] e 指向dds_entity结构体的指针
 *  @return 返回dds_return_t类型的结果
 */
inline dds_return_t dds_entity_deriver_delete(struct dds_entity* e) {
  return dds_entity_deriver_table[e->m_kind]->delete (e);
}

/** @component generic_entity
 *  @brief 设置实体的QoS
 *  @param[in] e 指向dds_entity结构体的指针
 *  @param[in] qos 指向dds_qos_t结构体的指针
 *  @param[in] enabled 布尔值，表示是否启用
 *  @return 返回dds_return_t类型的结果
 */
inline dds_return_t dds_entity_deriver_set_qos(struct dds_entity* e,
                                               const dds_qos_t* qos,
                                               bool enabled) {
  return dds_entity_deriver_table[e->m_kind]->set_qos(e, qos, enabled);
}

/** @component generic_entity */
// 内联函数：验证实体状态
// @param e 指向dds_entity结构的指针
// @param mask 状态掩码
// @return 返回dds_return_t类型的结果
inline dds_return_t dds_entity_deriver_validate_status(struct dds_entity* e, uint32_t mask) {
  // 调用实体派生表中对应实体类型的validate_status方法
  return dds_entity_deriver_table[e->m_kind]->validate_status(mask);
}

/** @component generic_entity */
// 内联函数：检查实体是否支持设置QoS
// @param e 指向dds_entity结构的指针
// @return 如果支持设置QoS，则返回true，否则返回false
inline bool dds_entity_supports_set_qos(struct dds_entity* e) {
  // 检查实体派生表中对应实体类型的set_qos方法是否为dummy_set_qos
  return dds_entity_deriver_table[e->m_kind]->set_qos != dds_entity_deriver_dummy_set_qos;
}

/** @component generic_entity */
// 内联函数：检查实体是否支持验证状态
// @param e 指向dds_entity结构的指针
// @return 如果支持验证状态，则返回true，否则返回false
inline bool dds_entity_supports_validate_status(struct dds_entity* e) {
  // 检查实体派生表中对应实体类型的validate_status方法是否为dummy_validate_status
  return dds_entity_deriver_table[e->m_kind]->validate_status !=
         dds_entity_deriver_dummy_validate_status;
}

/** @component statistics */
// 内联函数：创建实体的统计信息
// @param e 指向dds_entity结构的指针
// @return 返回一个指向dds_statistics结构的指针
inline struct dds_statistics* dds_entity_deriver_create_statistics(const struct dds_entity* e) {
  // 调用实体派生表中对应实体类型的create_statistics方法
  return dds_entity_deriver_table[e->m_kind]->create_statistics(e);
}

/** @component statistics */
// 内联函数：刷新实体的统计信息
// @param e 指向dds_entity结构的指针
// @param s 指向dds_statistics结构的指针
inline void dds_entity_deriver_refresh_statistics(const struct dds_entity* e,
                                                  struct dds_statistics* s) {
  // 调用实体派生表中对应实体类型的refresh_statistics方法
  dds_entity_deriver_table[e->m_kind]->refresh_statistics(e, s);
}

// 定义dds_cyclonedds_entity结构
typedef struct dds_cyclonedds_entity {
  struct dds_entity m_entity;        // dds_entity结构

  ddsrt_mutex_t m_mutex;             // 互斥锁
  ddsrt_cond_t m_cond;               // 条件变量
  ddsrt_avl_tree_t m_domains;        // 域的AVL树
  uint32_t threadmon_count;          // 线程监视器计数
  struct ddsi_threadmon* threadmon;  // 指向ddsi_threadmon结构的指针
} dds_cyclonedds_entity;

// 定义dds_domain结构
typedef struct dds_domain {
  struct dds_entity m_entity;  // dds_entity结构

  ddsrt_avl_node_t m_node;     // 用于dds_global.m_domains的AVL节点
  dds_domainid_t m_id;         // 域ID

#ifdef DDS_HAS_SHM
  shm_monitor_t m_shm_monitor;  // 共享内存监视器
#endif

  struct ddsi_cfgst* cfgst;  // 配置状态指针，如果提供了配置初始化器，则为NULL

  // 内置类型定义
  struct ddsi_sertype* builtin_participant_type;
#ifdef DDS_HAS_TOPIC_DISCOVERY
  struct ddsi_sertype* builtin_topic_type;
#endif
  struct ddsi_sertype* builtin_reader_type;
  struct ddsi_sertype* builtin_writer_type;

  // 内置主题写入器定义
  struct ddsi_local_orphan_writer* builtintopic_writer_participant;
  struct ddsi_local_orphan_writer* builtintopic_writer_publications;
  struct ddsi_local_orphan_writer* builtintopic_writer_subscriptions;
#ifdef DDS_HAS_TOPIC_DISCOVERY
  struct ddsi_local_orphan_writer* builtintopic_writer_topics;
#endif

  struct ddsi_builtin_topic_interface btif;  // 内置主题接口
  struct ddsi_domaingv gv;                   // 域全局变量

  // 传输端：序列化和传输消息的池
  struct dds_serdatapool* serpool;
} dds_domain;

/**
 * @brief 定义dds_subscriber结构体
 */
typedef struct dds_subscriber {
  /**
   * @brief 包含实体信息的dds_entity结构体
   */
  struct dds_entity m_entity;

  /**
   * @brief materialize_data_on_readers变量，包括：
   *        - 最低有效31位（MASK）中的计数器
   *        - 最高有效位（FLAG）中的标志
   *
   * @details 计数器用于跟踪是否需要实现订阅者的DATA_ON_READERS状态。
   *          当且仅当它实现并且所有读取器在其状态掩码中都设置了DATA_ON_READERS位时，才设置标志。
   *          读取器状态掩码中的DATA_ON_READERS位表示它们必须考虑可能的实现DATA_ON_READERS标志。
   *          在订阅者中设置FLAG，而其中一些读取器在其状态掩码中未设置DATA_ON_READERS，则为错误。
   *
   * @note 受m_entity.m_observers_lock保护。
   */
  uint32_t materialize_data_on_readers;
} dds_subscriber;

/**
 * @brief
 * 定义DDS_SUB_MATERIALIZE_DATA_ON_READERS_MASK，用于获取materialize_data_on_readers的计数器部分
 */
#define DDS_SUB_MATERIALIZE_DATA_ON_READERS_MASK 0x7fffffffu

/**
 * @brief
 * 定义DDS_SUB_MATERIALIZE_DATA_ON_READERS_FLAG，用于获取materialize_data_on_readers的标志部分
 */
#define DDS_SUB_MATERIALIZE_DATA_ON_READERS_FLAG 0x80000000u

/**
 * @brief 定义dds_publisher结构体
 */
typedef struct dds_publisher {
  /**
   * @brief 包含实体信息的dds_entity结构体
   */
  struct dds_entity m_entity;
} dds_publisher;

#ifdef DDS_HAS_TOPIC_DISCOVERY
/**
 * @brief 定义ktopic_type_guid结构体，用于存储完整的类型ID到<topic guid, ddsi topic>映射
 */
struct ktopic_type_guid {
  /**
   * @brief 类型ID指针
   */
  ddsi_typeid_t* type_id;

  /**
   * @brief 引用计数
   */
  uint32_t refc;

  /**
   * @brief GUID（全局唯一标识符）
   */
  ddsi_guid_t guid;

  /**
   * @brief ddsi_topic结构体指针
   */
  struct ddsi_topic* tp;
};
#endif

/**
 * @brief dds_ktopic 结构体定义
 *
 * 定义了一个dds_ktopic结构体，用于存储与主题相关的信息。
 */
typedef struct dds_ktopic {
  /* name -> <QoS> 映射的主题，属于参与者
       并受参与者锁保护（包括实际的 QoS 设置）

       defer_set_qos 用于实现有意设计的不公平单写入者/
       多读取者锁，使用参与者的锁和条件变量：set_qos
       "写锁定"它，create_reader 和 create_writer "读锁定"它。 */
  ddsrt_avl_node_t pp_ktopics_avlnode;  ///< 参与者的 AVL 树节点
  uint32_t refc;                        ///< 引用计数
  uint32_t defer_set_qos;               ///< set_qos 必须等待此值为0
  dds_qos_t* qos;                       ///< QoS 指针
  char* name;                           ///< 主题名称（常量）
#ifdef DDS_HAS_TOPIC_DISCOVERY
  struct ddsrt_hh* topic_guid_map;      ///< 此 ktopic 到 ddsi 主题的映射
#endif
} dds_ktopic;

/**
 * @brief dds_participant 结构体定义
 *
 * 定义了一个dds_participant结构体，用于存储与参与者相关的信息。
 */
typedef struct dds_participant {
  struct dds_entity m_entity;         ///< 实体结构体
  dds_entity_t m_builtin_subscriber;  ///< 内置订阅者
  ddsrt_avl_tree_t m_ktopics;         ///< ktopics 的 AVL 树（受 m_entity.m_mutex 保护）
} dds_participant;

/**
 * @brief dds_reader 结构体定义
 *
 * 定义了一个dds_reader结构体，用于存储与读取者相关的信息。
 */
typedef struct dds_reader {
  struct dds_entity m_entity;  ///< 实体结构体
  struct dds_topic* m_topic;  ///< 主题指针（引用计数，常量，允许 lock(rd) -> lock(tp)）
  struct dds_rhc* m_rhc;  ///< 别名 m_rd->rhc，具有更广泛的接口，FIXME：但 m_rd 拥有它以进行资源管理
  struct ddsi_reader* m_rd;             ///< ddsi_reader 结构体指针
  bool m_loan_out;                      ///< 借出标志
  void* m_loan;                         ///< 借出指针
  uint32_t m_loan_size;                 ///< 借出大小
#ifdef DDS_HAS_SHM
  iox_sub_context_t m_iox_sub_context;  ///< Iceoryx 订阅者上下文
  iox_sub_t m_iox_sub;                  ///< Iceoryx 订阅者
#endif
} dds_reader;

/* Status metrics */
dds_sample_rejected_status_t m_sample_rejected_status;
dds_liveliness_changed_status_t m_liveliness_changed_status;
dds_requested_deadline_missed_status_t m_requested_deadline_missed_status;
dds_requested_incompatible_qos_status_t m_requested_incompatible_qos_status;
dds_sample_lost_status_t m_sample_lost_status;
dds_subscription_matched_status_t m_subscription_matched_status;
}
dds_reader;

/**
 * @struct dds_writer
 * @brief 定义了一个DDS写入器的结构体
 */
typedef struct dds_writer {
  struct dds_entity m_entity;  ///< DDS实体对象
  struct dds_topic* m_topic;  ///< 指向DDS主题的指针，引用计数，常量，允许lock(wr) -> lock(tp)

  struct ddsi_xpack* m_xp;   ///< 指向DDSI xpack的指针
  struct ddsi_writer* m_wr;  ///< 指向DDSI writer的指针
  struct ddsi_whc*
      m_whc;  ///< 指向DDSI whc的指针，注意：所有权仍在底层DDSI writer（因为DDSI内置writer）

  bool whc_batch;  ///< 注意：通道 + 延迟预算

#ifdef DDS_HAS_SHM
  iox_pub_t m_iox_pub;                   ///< Iceoryx发布者对象
  void* m_iox_pub_loans[MAX_PUB_LOANS];  ///< Iceoryx发布者贷款数组
#endif

  /* 状态指标 */

  dds_liveliness_lost_status_t m_liveliness_lost_status;  ///< 生命期丧失状态
  dds_offered_deadline_missed_status_t
      m_offered_deadline_missed_status;                   ///< 提供的截止日期未满足状态
  dds_offered_incompatible_qos_status_t m_offered_incompatible_qos_status;  ///< 提供的不兼容QoS状态
  dds_publication_matched_status_t m_publication_matched_status;            ///< 发布匹配状态
} dds_writer;

/**
 * @struct dds_topic
 * @brief DDS主题结构体定义
 */
typedef struct dds_topic {
  struct dds_entity m_entity;    ///< 实体对象，用于表示DDS实体的基本信息
  char* m_name;                  ///< 主题名称，用于标识一个特定的主题
  struct ddsi_sertype* m_stype;  ///< 序列化类型，用于处理数据序列化和反序列化
  struct dds_ktopic* m_ktopic;   ///< 内核主题，引用计数，常量
  struct dds_topic_filter m_filter;  ///< 主题过滤器，用于对接收到的数据进行过滤
  dds_inconsistent_topic_status_t
      m_inconsistent_topic_status;   ///< 不一致主题状态，用于记录状态指标
} dds_topic;

/**
 * @typedef dds_querycond_mask_t
 * @brief 查询条件掩码类型定义，用于表示查询条件的状态
 */
typedef uint32_t dds_querycond_mask_t;

/**
 * @struct dds_readcond
 * @brief DDS读取条件结构体定义
 */
typedef struct dds_readcond {
  dds_entity m_entity;         ///< 实体对象，用于表示DDS实体的基本信息
  uint32_t m_qminv;            ///< 最小查询间隔，用于限制查询操作的频率
  uint32_t m_sample_states;    ///< 样本状态，用于表示数据样本的状态
  uint32_t m_view_states;      ///< 视图状态，用于表示数据视图的状态
  uint32_t m_instance_states;  ///< 实例状态，用于表示数据实例的状态
  struct dds_readcond* m_next;  ///< 指向下一个读取条件的指针，用于组织多个读取条件

  /**
   * @brief 查询条件结构体定义
   */
  struct {
    dds_querycondition_filter_fn m_filter;  ///< 查询条件过滤器函数，用于对数据进行过滤
    dds_querycond_mask_t m_qcmask;  ///< 查询条件掩码，用于表示查询条件在RHC中的状态
  } m_query;
} dds_readcond;

/**
 * @struct dds_guardcond
 * @brief DDS守护条件结构体定义
 */
typedef struct dds_guardcond {
  dds_entity m_entity;  ///< 实体对象，用于表示DDS实体的基本信息
} dds_guardcond;

/**
 * @struct dds_attachment
 * @brief 附件结构，用于存储实体、句柄和参数信息
 */
typedef struct dds_attachment {
  dds_entity* entity;   ///< 实体指针
  dds_entity_t handle;  ///< 实体句柄
  dds_attach_t arg;     ///< 附件参数
} dds_attachment;

/**
 * @struct dds_waitset
 * @brief 等待集结构，用于管理实体、锁、条件变量等信息
 */
typedef struct dds_waitset {
  dds_entity m_entity;  ///< 等待集中的实体

  /* 需要一个不同于m_entity.m_mutex的锁，因为在持有祖先锁的情况下不能获取实体锁，
     但是等待集必须能够触发其父级上的事件 */
  ddsrt_mutex_t wait_lock;  ///< 等待锁
  ddsrt_cond_t wait_cond;   ///< 等待条件变量
  size_t nentities;         ///< [wait_lock] 实体数量
  size_t ntriggered;        ///< [wait_lock] 触发的实体数量
  /**
   * @brief [wait_lock] 存储实体的数组，0 .. ntriggered 是触发的，ntriggred .. nentities 是未触发的
   */
  dds_attachment* entities;
} dds_waitset;

extern dds_cyclonedds_entity dds_global;

#if defined(__cplusplus)
}
#endif
#endif /* DDS__TYPES_H */
