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

#include <assert.h>
#include <string.h>

#include "dds/ddsi/ddsi_entity.h"
#include "dds/ddsi/ddsi_entity_index.h"
#include "dds/ddsi/ddsi_iid.h"
#include "dds/ddsi/ddsi_thread.h"
#include "dds__guardcond.h"
#include "dds__init.h"
#include "dds__participant.h"
#include "dds__reader.h"

/**
 * @file
 * @brief 定义了dds_entity_deriver_guardcondition结构体和dds_create_guardcondition函数
 */
DECL_ENTITY_LOCK_UNLOCK(dds_guardcond)

/**
 * @brief dds_entity_deriver_guardcondition 结构体，包含了一系列用于处理 guard condition
 * 实体的函数指针。
 */
const struct dds_entity_deriver dds_entity_deriver_guardcondition = {
    .interrupt = dds_entity_deriver_dummy_interrupt,                   ///< 中断函数
    .close = dds_entity_deriver_dummy_close,                           ///< 关闭函数
    .delete = dds_entity_deriver_dummy_delete,                         ///< 删除函数
    .set_qos = dds_entity_deriver_dummy_set_qos,                       ///< 设置QoS函数
    .validate_status = dds_entity_deriver_dummy_validate_status,       ///< 验证状态函数
    .create_statistics = dds_entity_deriver_dummy_create_statistics,   ///< 创建统计信息函数
    .refresh_statistics = dds_entity_deriver_dummy_refresh_statistics  ///< 刷新统计信息函数
};

/**
 * @brief 创建一个 guard condition 实体。
 *
 * @param[in] owner 该实体的所有者，可以是 CycloneDDS、Domain 或 Participant 类型的实体。
 * @return 成功时返回新创建的 guard condition 实体的句柄，失败时返回错误代码。
 */
dds_entity_t dds_create_guardcondition(dds_entity_t owner) {
  dds_entity* e;
  dds_return_t rc;

  // 如果 owner 是允许的普通实体，则库已经初始化，此处调用 init 是低成本的。
  // 如果它是 DDS_CYCLONEDDS_HANDLE，我们可能需要初始化库，因此必须调用它。
  // 如果它是一些错误的值且库尚未初始化...那就这样吧。
  // 当然，这要求我们在 DDS_CYCLONEDDS_HANDLE 上调用 delete。
  if ((rc = dds_init()) < 0) return rc;

  // 锁定 owner 实体
  if ((rc = dds_entity_lock(owner, DDS_KIND_DONTCARE, &e)) != DDS_RETCODE_OK) goto err_entity_lock;

  // 检查实体类型
  switch (dds_entity_kind(e)) {
    case DDS_KIND_CYCLONEDDS:
    case DDS_KIND_DOMAIN:
    case DDS_KIND_PARTICIPANT:
      break;
    default:
      rc = DDS_RETCODE_ILLEGAL_OPERATION;
      goto err_entity_kind;
  }

  // 分配内存并初始化 guard condition 实体
  dds_guardcond* gcond = dds_alloc(sizeof(*gcond));
  dds_entity_t hdl =
      dds_entity_init(&gcond->m_entity, e, DDS_KIND_COND_GUARD, false, true, NULL, NULL, 0);
  gcond->m_entity.m_iid = ddsi_iid_gen();
  dds_entity_register_child(e, &gcond->m_entity);
  dds_entity_init_complete(&gcond->m_entity);

  // 解锁 owner 实体并返回新创建的 guard condition 实体的句柄
  dds_entity_unlock(e);
  dds_entity_unpin_and_drop_ref(&dds_global.m_entity);
  return hdl;

err_entity_kind:
  dds_entity_unlock(e);
err_entity_lock:
  dds_entity_unpin_and_drop_ref(&dds_global.m_entity);
  return rc;
}
/**
 * @brief 设置保护条件的触发状态
 *
 * @param condition 保护条件实体
 * @param triggered 触发状态，true表示触发，false表示未触发
 * @return dds_return_t 返回操作结果，成功返回DDS_RETCODE_OK
 */
dds_return_t dds_set_guardcondition(dds_entity_t condition, bool triggered) {
  dds_guardcond* gcond;  // 定义一个保护条件指针
  dds_return_t rc;       // 定义一个返回值变量

  // 尝试锁定保护条件，如果失败则返回错误码
  if ((rc = dds_guardcond_lock(condition, &gcond)) != DDS_RETCODE_OK)
    return rc;
  else {
    dds_entity* const e = &gcond->m_entity;  // 获取实体指针
    uint32_t oldst;                          // 定义一个旧状态变量
    ddsrt_mutex_lock(&e->m_observers_lock);  // 锁定观察者互斥锁

    // 循环直到原子操作成功
    do {
      oldst = ddsrt_atomic_ld32(&e->m_status.m_trigger);                      // 加载旧状态
    } while (!ddsrt_atomic_cas32(&e->m_status.m_trigger, oldst, triggered));  // 尝试原子比较并交换

    // 如果旧状态为0且新状态不为0，则通知观察者
    if (oldst == 0 && triggered != 0) dds_entity_observers_signal(e, triggered);

    ddsrt_mutex_unlock(&e->m_observers_lock);  // 解锁观察者互斥锁
    dds_guardcond_unlock(gcond);               // 解锁保护条件
    return DDS_RETCODE_OK;                     // 返回成功
  }
}

/**
 * @brief 读取保护条件的触发状态
 *
 * @param condition 保护条件实体
 * @param[out] triggered 用于存储触发状态的指针，true表示触发，false表示未触发
 * @return dds_return_t 返回操作结果，成功返回DDS_RETCODE_OK
 */
dds_return_t dds_read_guardcondition(dds_entity_t condition, bool* triggered) {
  dds_guardcond* gcond;  // 定义一个保护条件指针
  dds_return_t rc;       // 定义一个返回值变量

  // 检查传入的触发状态指针是否为空，如果为空则返回错误参数
  if (triggered == NULL) return DDS_RETCODE_BAD_PARAMETER;

  *triggered = false;  // 初始化触发状态为false
  // 尝试锁定保护条件，如果失败则返回错误码
  if ((rc = dds_guardcond_lock(condition, &gcond)) != DDS_RETCODE_OK)
    return rc;
  else {
    // 读取保护条件的触发状态并存储到传入的指针中
    *triggered = (ddsrt_atomic_ld32(&gcond->m_entity.m_status.m_trigger) != 0);
    dds_guardcond_unlock(gcond);  // 解锁保护条件
    return DDS_RETCODE_OK;        // 返回成功
  }
}

/**
 * @brief 获取并重置保护条件的触发状态
 *
 * @param condition 保护条件实体
 * @param[out] triggered 用于存储触发状态的指针，true表示触发，false表示未触发
 * @return dds_return_t 返回操作结果，成功返回DDS_RETCODE_OK
 */
dds_return_t dds_take_guardcondition(dds_entity_t condition, bool* triggered) {
  dds_guardcond* gcond;  // 定义一个保护条件指针
  dds_return_t rc;       // 定义一个返回值变量

  // 检查传入的触发状态指针是否为空，如果为空则返回错误参数
  if (triggered == NULL) return DDS_RETCODE_BAD_PARAMETER;

  *triggered = false;  // 初始化触发状态为false
  // 尝试锁定保护条件，如果失败则返回错误码
  if ((rc = dds_guardcond_lock(condition, &gcond)) != DDS_RETCODE_OK)
    return rc;
  else {
    // 获取并重置保护条件的触发状态，并将结果存储到传入的指针中
    *triggered = (ddsrt_atomic_and32_ov(&gcond->m_entity.m_status.m_trigger, 0) != 0);
    dds_guardcond_unlock(gcond);  // 解锁保护条件
    return DDS_RETCODE_OK;        // 返回成功
  }
}
