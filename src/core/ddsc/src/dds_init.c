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
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "dds/ddsc/dds_rhc.h"
#include "dds/ddsi/ddsi_domaingv.h"
#include "dds/ddsi/ddsi_entity.h"
#include "dds/ddsi/ddsi_gc.h"
#include "dds/ddsi/ddsi_iid.h"
#include "dds/ddsi/ddsi_serdata.h"
#include "dds/ddsi/ddsi_threadmon.h"
#include "dds/ddsi/ddsi_tkmap.h"
#include "dds/ddsrt/cdtors.h"
#include "dds/ddsrt/environ.h"
#include "dds/ddsrt/heap.h"
#include "dds/ddsrt/process.h"
#include "dds/version.h"
#include "dds__builtin.h"
#include "dds__domain.h"
#include "dds__entity.h"
#include "dds__init.h"
#include "dds__whc_builtintopic.h"

// 声明静态函数dds_close
static void dds_close(struct dds_entity* e);
// 声明静态函数dds_fini
static dds_return_t dds_fini(struct dds_entity* e);

// 定义dds_entity_deriver_cyclonedds结构体，包含实体操作的函数指针
const struct dds_entity_deriver dds_entity_deriver_cyclonedds = {
    .interrupt = dds_entity_deriver_dummy_interrupt,              // 中断操作的函数指针
    .close = dds_close,                                           // 关闭操作的函数指针
    .delete = dds_fini,                                           // 删除操作的函数指针
    .set_qos = dds_entity_deriver_dummy_set_qos,                  // 设置QoS操作的函数指针
    .validate_status = dds_entity_deriver_dummy_validate_status,  // 验证状态操作的函数指针
    .create_statistics = dds_entity_deriver_dummy_create_statistics,  // 创建统计信息操作的函数指针
    .refresh_statistics = dds_entity_deriver_dummy_refresh_statistics  // 刷新统计信息操作的函数指针
};

// 定义全局CycloneDDS实体变量
dds_cyclonedds_entity dds_global;

// 定义CycloneDDS状态常量
#define CDDS_STATE_ZERO 0u      // 初始状态
#define CDDS_STATE_STARTING 1u  // 启动中状态
#define CDDS_STATE_READY 2u     // 就绪状态
#define CDDS_STATE_STOPPING 3u  // 停止中状态

// 定义原子类型的dds_state变量，并初始化为CDDS_STATE_ZERO
static ddsrt_atomic_uint32_t dds_state = DDSRT_ATOMIC_UINT32_INIT(CDDS_STATE_ZERO);

/**
 * @file common_cleanup.c
 *
 * 该文件包含两个函数：common_cleanup 和 cyclonedds_entity_ready。
 */

/**
 * @brief 清理公共资源。
 *
 * 释放分配的资源并销毁全局变量。
 */
static void common_cleanup(void) {
  // 如果线程状态成功结束，则调用 dds_handle_server_fini()
  if (ddsi_thread_states_fini()) dds_handle_server_fini();

  // 结束 ddsi_iid
  ddsi_iid_fini();

  // 销毁全局条件变量
  ddsrt_cond_destroy(&dds_global.m_cond);

  // 销毁全局互斥锁
  ddsrt_mutex_destroy(&dds_global.m_mutex);

  // 将 dds_state 设置为 CDDS_STATE_ZERO
  ddsrt_atomic_st32(&dds_state, CDDS_STATE_ZERO);

  // 广播单例条件变量
  ddsrt_cond_broadcast(ddsrt_get_singleton_cond());
}

/**
 * @brief 检查 CycloneDDS 实体是否准备就绪。
 *
 * 根据给定的状态值，判断实体是否处于可用状态。
 *
 * @param s 状态值，用于判断实体是否准备就绪。
 * @return 如果实体准备就绪，则返回 true；否则返回 false。
 */
static bool cyclonedds_entity_ready(uint32_t s) {
  // 断言状态值不等于 CDDS_STATE_ZERO
  assert(s != CDDS_STATE_ZERO);

  // 如果状态值为 CDDS_STATE_STARTING 或 CDDS_STATE_STOPPING，则返回 false
  if (s == CDDS_STATE_STARTING || s == CDDS_STATE_STOPPING)
    return false;
  else {
    // 定义一个 dds_handle_link 指针 x
    struct dds_handle_link* x;

    // 如果 dds_handle_pin_and_ref_with_origin 返回 DDS_RETCODE_OK，则返回 true；否则返回 false
    return dds_handle_pin_and_ref_with_origin(DDS_CYCLONEDDS_HANDLE, false, &x) == DDS_RETCODE_OK;
  }
}
/**
 * @brief 初始化 CycloneDDS 系统
 *
 * 该函数负责初始化 CycloneDDS 系统，包括内部数据结构、线程状态、实体等。
 *
 * @return 返回 dds_return_t 类型的结果，表示操作成功或失败。
 */
dds_return_t dds_init(void) {
  // 定义返回值变量
  dds_return_t ret;

  // 初始化 ddsrt 层
  ddsrt_init();
  // 获取单例互斥锁
  ddsrt_mutex_t* const init_mutex = ddsrt_get_singleton_mutex();
  // 获取单例条件变量
  ddsrt_cond_t* const init_cond = ddsrt_get_singleton_cond();

  // 锁定互斥锁
  ddsrt_mutex_lock(init_mutex);
  // 加载并获取 dds_state 的值
  uint32_t s = ddsrt_atomic_ld32(&dds_state);
  // 判断 dds_state 是否为 CDDS_STATE_ZERO 或实体是否已准备好
  while (s != CDDS_STATE_ZERO && !cyclonedds_entity_ready(s)) {
    // 等待条件变量
    ddsrt_cond_wait(init_cond, init_mutex);
    // 重新加载并获取 dds_state 的值
    s = ddsrt_atomic_ld32(&dds_state);
  }
  // 根据 dds_state 的值进行相应处理
  switch (s) {
    case CDDS_STATE_READY:
      assert(dds_global.m_entity.m_hdllink.hdl == DDS_CYCLONEDDS_HANDLE);
      ddsrt_mutex_unlock(init_mutex);
      return DDS_RETCODE_OK;
    case CDDS_STATE_ZERO:
      ddsrt_atomic_st32(&dds_state, CDDS_STATE_STARTING);
      break;
    default:
      ddsrt_mutex_unlock(init_mutex);
      ddsrt_fini();
      return DDS_RETCODE_ERROR;
  }

  // 初始化全局互斥锁和条件变量
  ddsrt_mutex_init(&dds_global.m_mutex);
  ddsrt_cond_init(&dds_global.m_cond);
  // 初始化实例标识生成器
  ddsi_iid_init();
  // 初始化线程状态
  ddsi_thread_states_init();

  // 初始化内部句柄服务器
  if (dds_handle_server_init() != DDS_RETCODE_OK) {
    DDS_ERROR("Failed to initialize internal handle server\n");
    ret = DDS_RETCODE_ERROR;
    goto fail_handleserver;
  }

  // 初始化 CycloneDDS 全局实体
  dds_entity_init(&dds_global.m_entity, NULL, DDS_KIND_CYCLONEDDS, true, true, NULL, NULL, 0);
  // 为全局实体生成实例标识
  dds_global.m_entity.m_iid = ddsi_iid_gen();
  // 重新绑定全局实体的句柄
  dds_handle_repin(&dds_global.m_entity.m_hdllink);
  // 增加全局实体的引用计数
  dds_entity_add_ref_locked(&dds_global.m_entity);
  // 完成全局实体的初始化
  dds_entity_init_complete(&dds_global.m_entity);
  // 将 dds_state 设置为 CDDS_STATE_READY
  ddsrt_atomic_st32(&dds_state, CDDS_STATE_READY);
  // 解锁互斥锁
  ddsrt_mutex_unlock(init_mutex);
  // 返回操作成功
  return DDS_RETCODE_OK;

// 处理句柄服务器初始化失败的情况
fail_handleserver:
  common_cleanup();
  ddsrt_mutex_unlock(init_mutex);
  ddsrt_fini();
  // 返回操作结果
  return ret;
}
/**
 * @brief 关闭实体 e
 *
 * 该函数将 dds_state 设置为 CDDS_STATE_STOPPING，表示实体正在关闭。
 *
 * @param[in] e 指向要关闭的实体的指针。
 */
static void dds_close(struct dds_entity* e) {
  (void)e;
  // 确保 dds_state 的值为 CDDS_STATE_READY
  assert(ddsrt_atomic_ld32(&dds_state) == CDDS_STATE_READY);
  // 将 dds_state 设置为 CDDS_STATE_STOPPING
  ddsrt_atomic_st32(&dds_state, CDDS_STATE_STOPPING);
}

/**
 * @brief 终止实体 e
 *
 * 该函数负责在实体 e 完全关闭之前执行最终的清理工作，并释放相关资源。
 *
 * @param[in] e 指向要终止的实体的指针。
 * @return 返回 dds_return_t 类型的结果，表示操作成功或失败。
 */
static dds_return_t dds_fini(struct dds_entity* e) {
  (void)e;
  // 获取单例互斥锁
  ddsrt_mutex_t* const init_mutex = ddsrt_get_singleton_mutex();
  /* 如果有多个域同时关闭，那么删除顶级实体（从而到达这里）的一个可能已经超过了另一个仍在删除其域对象的线程。
     对于大多数实体，这种竞争不是问题，但在这里我们拆除运行时，所以在这里我们必须等待其他人都出去。
   */
  // 锁定全局互斥锁
  ddsrt_mutex_lock(&dds_global.m_mutex);
  // 等待所有域被删除
  while (!ddsrt_avl_is_empty(&dds_global.m_domains))
    ddsrt_cond_wait(&dds_global.m_cond, &dds_global.m_mutex);
  // 解锁全局互斥锁
  ddsrt_mutex_unlock(&dds_global.m_mutex);

  // 锁定单例互斥锁
  ddsrt_mutex_lock(init_mutex);
  // 确保 dds_state 的值为 CDDS_STATE_STOPPING
  assert(ddsrt_atomic_ld32(&dds_state) == CDDS_STATE_STOPPING);
  // 在释放实体之前执行最终的清理工作
  dds_entity_final_deinit_before_free(e);
  // 执行通用清理操作
  common_cleanup();
  // 解锁单例互斥锁
  ddsrt_mutex_unlock(init_mutex);
  // 结束 ddsrt 层
  ddsrt_fini();
  // 返回操作成功，但没有数据
  return DDS_RETCODE_NO_DATA;
}
