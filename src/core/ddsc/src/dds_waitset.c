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

#include "dds/ddsc/dds_rhc.h"
#include "dds/ddsi/ddsi_iid.h"
#include "dds/ddsrt/heap.h"
#include "dds/ddsrt/log.h"
#include "dds__entity.h"
#include "dds__init.h"
#include "dds__participant.h"
#include "dds__readcond.h"
#include "dds__subscriber.h"  // only for (de)materializing data_on_readers

/**
 * @brief 判断实体是否触发
 *
 * @param[in] e 指向dds_entity结构体的指针
 * @return 如果实体被触发，返回true；否则返回false
 */
static bool is_triggered(struct dds_entity *e) {
  bool t;
  // 根据实体类型进行判断
  switch (e->m_kind) {
    case DDS_KIND_COND_READ:
    case DDS_KIND_COND_QUERY:
    case DDS_KIND_COND_GUARD:
    case DDS_KIND_WAITSET:
      // 判断实体的触发状态
      t = ddsrt_atomic_ld32(&e->m_status.m_trigger) != 0;
      break;
    default: {
      const uint32_t sm = ddsrt_atomic_ld32(&e->m_status.m_status_and_mask);
      t = (sm & (sm >> SAM_ENABLED_SHIFT)) != 0;
      break;
    }
  }
  return t;
}

/**
 * @brief 实现等待集合的等待操作
 *
 * @param[in] waitset 等待集合实体
 * @param[out] xs 附件数组
 * @param[in] nxs 附件数组的大小
 * @param[in] abstimeout 绝对超时时间
 * @return 成功时返回DDS_RETCODE_OK，失败时返回相应的错误代码
 */
static dds_return_t dds_waitset_wait_impl(dds_entity_t waitset,
                                          dds_attach_t *xs,
                                          size_t nxs,
                                          dds_time_t abstimeout) {
  dds_waitset *ws;
  dds_return_t ret;

  // 检查参数有效性
  if ((xs == NULL) != (nxs == 0)) return DDS_RETCODE_BAD_PARAMETER;

  // 锁定等待集合，防止在操作过程中被删除
  {
    dds_entity *ent;
    if ((ret = dds_entity_pin(waitset, &ent)) != DDS_RETCODE_OK) return ret;
    if (dds_entity_kind(ent) != DDS_KIND_WAITSET) {
      dds_entity_unpin(ent);
      return DDS_RETCODE_ILLEGAL_OPERATION;
    }
    ws = (dds_waitset *)ent;
  }

  // 将不再触发的实体移回到观察列表中
  ddsrt_mutex_lock(&ws->wait_lock);
  ws->ntriggered = 0;
  for (size_t i = 0; i < ws->nentities; i++) {
    if (is_triggered(ws->entities[i].entity)) {
      dds_attachment tmp = ws->entities[i];
      ws->entities[i] = ws->entities[ws->ntriggered];
      ws->entities[ws->ntriggered++] = tmp;
    }
  }

  // 只有在有实体可观察且没有触发器时才进行等待/继续等待，并检查等待集合是否已关闭
  while (ws->nentities > 0 && ws->ntriggered == 0 && !dds_handle_is_closed(&ws->m_entity.m_hdllink))
    // 如果条件变量在指定的超时时间内未被唤醒，则跳出循环
    if (!ddsrt_cond_waituntil(&ws->wait_cond, &ws->wait_lock, abstimeout)) break;

  // 返回触发的实体数量
  ret = (int32_t)ws->ntriggered;
  // 将触发的实体的参数存储到输出数组xs中，最多存储nxs个
  for (size_t i = 0; i < ws->ntriggered && i < nxs; i++) xs[i] = ws->entities[i].arg;
  // 解锁等待集合的互斥锁
  ddsrt_mutex_unlock(&ws->wait_lock);
  // 解除对等待集合实体的引用
  dds_entity_unpin(&ws->m_entity);
  // 返回触发的实体数量
  return ret;
}

/**
 * @brief 中断等待集
 *
 * @param[in] e 指向dds_entity结构的指针
 */
static void dds_waitset_interrupt(struct dds_entity *e) {
  dds_waitset *ws = (dds_waitset *)e;                     // 将实体转换为等待集类型
  ddsrt_mutex_lock(&ws->wait_lock);                       // 锁定等待集互斥锁
  assert(dds_handle_is_closed(&ws->m_entity.m_hdllink));  // 断言等待集句柄已关闭
  ddsrt_cond_broadcast(&ws->wait_cond);                   // 广播等待集条件变量
  ddsrt_mutex_unlock(&ws->wait_lock);                     // 解锁等待集互斥锁
}

/**
 * @brief 关闭等待集
 *
 * @param[in] e 指向dds_entity结构的指针
 */
static void dds_waitset_close(struct dds_entity *e) {
  dds_waitset *ws = (dds_waitset *)e;  // 将实体转换为等待集类型
  ddsrt_mutex_lock(&ws->wait_lock);    // 锁定等待集互斥锁
  while (ws->nentities > 0)            // 当存在实体时
  {
    dds_entity *observed;
    if (dds_entity_pin(ws->entities[0].handle, &observed) < 0)  // 如果无法固定实体
    {
      /* can't be pinned => being deleted => will be removed from wait set soon enough
       and go through delete_observer (which will trigger the condition variable) */
      ddsrt_cond_wait(&ws->wait_cond, &ws->wait_lock);  // 等待条件变量
    } else                                              // 如果可以固定实体
    {
      /* entity will remain in existence */
      ddsrt_mutex_unlock(&ws->wait_lock);                        // 解锁等待集互斥锁
      (void)dds_entity_observer_unregister(observed, ws, true);  // 注销观察者
      ddsrt_mutex_lock(&ws->wait_lock);                          // 重新锁定等待集互斥锁
      assert(ws->nentities == 0 || ws->entities[0].entity != observed);  // 断言实体已被移除
      dds_entity_unpin(observed);                                        // 解除实体固定
    }
  }
  ddsrt_mutex_unlock(&ws->wait_lock);  // 解锁等待集互斥锁
}

/**
 * @brief 删除等待集
 *
 * @param[in] e 指向dds_entity结构的指针
 * @return 返回DDS_RETCODE_OK表示成功删除等待集
 */
static dds_return_t dds_waitset_delete(struct dds_entity *e) {
  dds_waitset *ws = (dds_waitset *)e;   // 将实体转换为等待集类型
  ddsrt_mutex_destroy(&ws->wait_lock);  // 销毁等待集互斥锁
  ddsrt_cond_destroy(&ws->wait_cond);   // 销毁等待集条件变量
  ddsrt_free(ws->entities);             // 释放实体内存
  return DDS_RETCODE_OK;                // 返回操作成功
}

const struct dds_entity_deriver dds_entity_deriver_waitset = {
    .interrupt = dds_waitset_interrupt,
    .close = dds_waitset_close,
    .delete = dds_waitset_delete,
    .set_qos = dds_entity_deriver_dummy_set_qos,
    .validate_status = dds_entity_deriver_dummy_validate_status,
    .create_statistics = dds_entity_deriver_dummy_create_statistics,
    .refresh_statistics = dds_entity_deriver_dummy_refresh_statistics};
/**
 * @brief 获取 waitset 中的实体列表
 *
 * @param waitset    [in]  等待集合的实体标识符
 * @param entities   [out] 实体数组，用于存储从 waitset 中获取的实体
 * @param size       [in]  实体数组的大小
 * @return dds_return_t    成功时返回实体数量，失败时返回错误代码
 */
dds_return_t dds_waitset_get_entities(dds_entity_t waitset, dds_entity_t *entities, size_t size) {
  // 定义返回值变量
  dds_return_t ret;
  // 定义指向实体的指针
  dds_entity *wsent;

  // 尝试获取并锁定实体，如果失败则返回错误代码
  if ((ret = dds_entity_pin(waitset, &wsent)) < 0) return ret;
  // 检查实体类型是否为 DDS_KIND_WAITSET，如果不是则解锁实体并返回错误代码
  else if (dds_entity_kind(wsent) != DDS_KIND_WAITSET) {
    dds_entity_unpin(wsent);
    return DDS_RETCODE_ILLEGAL_OPERATION;
  }
  // 如果实体类型正确，则执行以下操作
  else {
    // 将实体转换为等待集合类型
    dds_waitset *ws = (dds_waitset *)wsent;
    // 锁定等待集合的互斥锁
    ddsrt_mutex_lock(&ws->wait_lock);

    // 如果实体数组不为空，则将等待集合中的实体复制到实体数组中
    if (entities != NULL) {
      for (size_t i = 0; i < ws->nentities && i < size; i++) entities[i] = ws->entities[i].handle;
    }

    // 将等待集合中的实体数量赋值给返回值变量
    ret = (int32_t)ws->nentities;
    // 解锁等待集合的互斥锁
    ddsrt_mutex_unlock(&ws->wait_lock);
    // 解锁实体
    dds_entity_unpin(&ws->m_entity);

    // 返回实体数量
    return ret;
  }
}

/**
 * @brief 创建一个等待集 (waitset) 实体
 *
 * @param owner 等待集的所有者实体，可以是 DDS_CYCLONEDDS_HANDLE、域 (domain) 或参与者 (participant)
 * @return 成功时返回创建的等待集实体句柄，失败时返回错误代码
 */
dds_entity_t dds_create_waitset(dds_entity_t owner) {
  dds_entity *e;    // 定义一个指向 dds_entity 结构体的指针
  dds_return_t rc;  // 定义一个用于存储返回值的变量

  // 如果 owner 是允许的普通实体，则库已经初始化，此处调用 init 是低成本的。
  // 如果它是 DDS_CYCLONEDDS_HANDLE，我们可能需要初始化库，因此必须调用它。
  // 如果它是一些错误的值并且库尚未初始化...那就这样吧。
  // 当然，这要求我们在之后对 DDS_CYCLONEDDS_HANDLE 调用 delete。
  if ((rc = dds_init()) < 0) return rc;

  // 尝试锁定 owner 实体
  if ((rc = dds_entity_lock(owner, DDS_KIND_DONTCARE, &e)) != DDS_RETCODE_OK) goto err_entity_lock;

  // 检查实体类型是否合法
  switch (dds_entity_kind(e)) {
    case DDS_KIND_CYCLONEDDS:
    case DDS_KIND_DOMAIN:
    case DDS_KIND_PARTICIPANT:
      break;
    default:
      rc = DDS_RETCODE_ILLEGAL_OPERATION;
      goto err_entity_kind;
  }

  // 为等待集分配内存并初始化
  dds_waitset *waitset = dds_alloc(sizeof(*waitset));
  dds_entity_t hdl =
      dds_entity_init(&waitset->m_entity, e, DDS_KIND_WAITSET, false, true, NULL, NULL, 0);
  ddsrt_mutex_init(&waitset->wait_lock);
  ddsrt_cond_init(&waitset->wait_cond);
  waitset->m_entity.m_iid = ddsi_iid_gen();
  dds_entity_register_child(e, &waitset->m_entity);
  waitset->nentities = 0;
  waitset->ntriggered = 0;
  waitset->entities = NULL;

  // 完成等待集实体的初始化
  dds_entity_init_complete(&waitset->m_entity);

  // 解锁 owner 实体
  dds_entity_unlock(e);

  // 取消对全局实体的引用
  dds_entity_unpin_and_drop_ref(&dds_global.m_entity);

  // 返回创建的等待集实体句柄
  return hdl;

// 错误处理部分
err_entity_kind:
  dds_entity_unlock(e);
err_entity_lock:
  dds_entity_unpin_and_drop_ref(&dds_global.m_entity);
  return rc;
}

/**
 * @brief 获取等待集中的实体列表
 *
 * 该函数用于获取指定等待集中的实体列表。
 *
 * @param waitset    [in] 等待集实体
 * @param entities   [out] 实体数组，用于存储获取到的实体列表
 * @param size       [in] 实体数组的大小
 * @return 返回实际获取到的实体数量，或错误代码（负数）
 */
dds_return_t dds_waitset_get_entities(dds_entity_t waitset, dds_entity_t *entities, size_t size) {
  // 定义返回值变量
  dds_return_t ret;
  // 定义等待集实体指针
  dds_entity *wsent;

  // 尝试获取等待集实体，如果失败则返回错误代码
  if ((ret = dds_entity_pin(waitset, &wsent)) < 0) return ret;
  // 检查实体类型是否为等待集，如果不是则解锁实体并返回错误代码
  else if (dds_entity_kind(wsent) != DDS_KIND_WAITSET) {
    dds_entity_unpin(wsent);
    return DDS_RETCODE_ILLEGAL_OPERATION;
  }
  // 如果实体类型正确，则执行以下操作
  else {
    // 将实体指针转换为等待集指针
    dds_waitset *ws = (dds_waitset *)wsent;
    // 锁定等待集的互斥锁
    ddsrt_mutex_lock(&ws->wait_lock);

    // 如果实体数组非空，则将等待集中的实体复制到实体数组中
    if (entities != NULL) {
      for (size_t i = 0; i < ws->nentities && i < size; i++) entities[i] = ws->entities[i].handle;
    }

    // 获取等待集中实体的数量，并赋值给返回值变量
    ret = (int32_t)ws->nentities;
    // 解锁等待集的互斥锁
    ddsrt_mutex_unlock(&ws->wait_lock);
    // 解锁等待集实体
    dds_entity_unpin(&ws->m_entity);

    // 返回实际获取到的实体数量
    return ret;
  }
}

/**
 * @brief 当观察到的实体发出状态更改信号时调用此函数。
 *
 * @param ws 指向dds_waitset结构体的指针
 * @param observed 观察到的实体
 * @param status 状态值
 */
static void dds_waitset_observer(struct dds_waitset *ws, dds_entity_t observed, uint32_t status) {
  (void)status;

  // 锁定互斥量
  ddsrt_mutex_lock(&ws->wait_lock);

  // 将观察到的实体移动到触发列表中
  size_t i;
  for (i = 0; i < ws->nentities; i++)
    if (ws->entities[i].handle == observed) break;
  if (i < ws->nentities && i >= ws->ntriggered) {
    dds_attachment tmp = ws->entities[i];
    ws->entities[i] = ws->entities[ws->ntriggered];
    ws->entities[ws->ntriggered++] = tmp;
  }

  // 触发waitset唤醒
  ddsrt_cond_broadcast(&ws->wait_cond);
  ddsrt_mutex_unlock(&ws->wait_lock);
}

/**
 * @brief dds_waitset_attach_observer_arg 结构体定义
 */
struct dds_waitset_attach_observer_arg {
  dds_attach_t x;
};

/**
 * @brief 附加观察者到dds_waitset
 *
 * @param ws 指向dds_waitset结构体的指针
 * @param observed 指向dds_entity结构体的指针，表示观察到的实体
 * @param varg 指向dds_waitset_attach_observer_arg结构体的指针
 * @return bool 返回true表示成功，false表示失败
 */
static bool dds_waitset_attach_observer(struct dds_waitset *ws,
                                        struct dds_entity *observed,
                                        void *varg) {
  // 将 void 类型的指针转换为 dds_waitset_attach_observer_arg 类型的指针
  struct dds_waitset_attach_observer_arg *arg = varg;

  // 对等待集的互斥锁进行加锁，以确保线程安全
  ddsrt_mutex_lock(&ws->wait_lock);

  // 重新分配实体数组的内存空间，以容纳新的实体
  ws->entities = ddsrt_realloc(ws->entities, (ws->nentities + 1) * sizeof(*ws->entities));

  // 将新实体的参数、实体和句柄分别赋值给实体数组中的相应位置
  ws->entities[ws->nentities].arg = arg->x;
  ws->entities[ws->nentities].entity = observed;
  ws->entities[ws->nentities].handle = observed->m_hdllink.hdl;

  // 增加实体数量
  ws->nentities++;

  // 如果观察的实体已经被触发
  if (is_triggered(observed)) {
    // 获取新添加实体在数组中的索引
    const size_t i = ws->nentities - 1;

    // 交换新添加的实体与触发实体列表的第一个非触发实体
    dds_attachment tmp = ws->entities[i];
    ws->entities[i] = ws->entities[ws->ntriggered];
    ws->entities[ws->ntriggered++] = tmp;
  }

  // 广播等待条件，通知其他线程
  ddsrt_cond_broadcast(&ws->wait_cond);

  // 对等待集的互斥锁进行解锁
  ddsrt_mutex_unlock(&ws->wait_lock);

  // 返回 true 表示成功添加观察者
  return true;
}

/**
 * @brief 删除观察者实体从等待集中
 *
 * 该函数从给定的等待集中删除指定的观察者实体。
 *
 * @param ws        指向dds_waitset结构的指针，表示要操作的等待集
 * @param observed  要从等待集中删除的观察者实体的句柄
 */
static void dds_waitset_delete_observer(struct dds_waitset *ws, dds_entity_t observed) {
  size_t i;                          // 定义一个变量i用于循环遍历

  ddsrt_mutex_lock(&ws->wait_lock);  // 对等待集的互斥锁进行加锁，确保线程安全

  // 遍历等待集中的所有实体
  for (i = 0; i < ws->nentities; i++)
    // 如果找到了要删除的观察者实体
    if (ws->entities[i].handle == observed) break;  // 跳出循环

  // 如果找到了要删除的观察者实体
  if (i < ws->nentities) {
    // 如果要删除的实体在已触发的实体范围内
    if (i < ws->ntriggered) {
      // 将最后一个已触发的实体移动到要删除的实体位置
      ws->entities[i] = ws->entities[--ws->ntriggered];
      // 将最后一个实体移动到原先最后一个已触发的实体位置
      ws->entities[ws->ntriggered] = ws->entities[--ws->nentities];
    } else {
      // 将最后一个实体移动到要删除的实体位置
      ws->entities[i] = ws->entities[--ws->nentities];
    }
  }

  ddsrt_cond_broadcast(&ws->wait_cond);  // 广播等待条件变量，通知其他线程
  ddsrt_mutex_unlock(&ws->wait_lock);    // 对等待集的互斥锁进行解锁
}

/**
 * @brief 将实体附加到dds_waitset
 *
 * @param waitset 等待集实体
 * @param entity 要附加的实体
 * @param x 附加参数
 * @return dds_return_t 返回操作结果
 */
dds_return_t dds_waitset_attach(dds_entity_t waitset, dds_entity_t entity, dds_attach_t x) {
  dds_entity *wsent;
  dds_entity *e;
  dds_return_t ret;

  // 验证并锁定waitset实体
  if ((ret = dds_entity_pin(waitset, &wsent)) < 0)
    return ret;
  else if (dds_entity_kind(wsent) != DDS_KIND_WAITSET) {
    dds_entity_unpin(wsent);
    return DDS_RETCODE_ILLEGAL_OPERATION;
  } else {
    dds_waitset *ws = (dds_waitset *)wsent;

    // 验证并锁定要附加的实体
    if ((ret = dds_entity_pin(entity, &e)) < 0) goto err_entity;

    // 检查实体是否在范围内
    if (!dds_entity_in_scope(e, ws->m_entity.m_parent)) {
      ret = DDS_RETCODE_BAD_PARAMETER;
      goto err_scope;
    }

    // 附加订阅者到waitset会强制实现DATA_ON_READERS
    if (dds_entity_kind(e) == DDS_KIND_SUBSCRIBER)
      dds_subscriber_adjust_materialize_data_on_readers((dds_subscriber *)e, true);

    // 注册观察者
    struct dds_waitset_attach_observer_arg attach_arg = {.x = x};
    ret = dds_entity_observer_register(e, ws, dds_waitset_observer, dds_waitset_attach_observer,
                                       &attach_arg, dds_waitset_delete_observer);

    // 如果注册失败，撤销对订阅者的DATA_ON_READERS更改
    if (ret < 0 && dds_entity_kind(e) == DDS_KIND_SUBSCRIBER)
      dds_subscriber_adjust_materialize_data_on_readers((dds_subscriber *)e, false);

  err_scope:
    dds_entity_unpin(e);
  err_entity:
    dds_entity_unpin(&ws->m_entity);
    return ret;
  }
}

/**
 * @brief 从等待集中分离实体。
 *
 * @param waitset 要从中分离实体的等待集。
 * @param entity 要分离的实体。
 * @return 成功时返回 DDS_RETCODE_OK，否则返回错误代码。
 */
dds_return_t dds_waitset_detach(dds_entity_t waitset, dds_entity_t entity) {
  // 定义实体指针 wsent
  dds_entity *wsent;
  // 定义返回值 ret
  dds_return_t ret;

  // 尝试将 waitset 实体固定到 wsent，如果失败，则返回错误代码
  if ((ret = dds_entity_pin(waitset, &wsent)) != DDS_RETCODE_OK) return ret;
  // 如果 wsent 的类型不是 DDS_KIND_WAITSET，则取消固定并返回错误代码
  else if (dds_entity_kind(wsent) != DDS_KIND_WAITSET) {
    dds_entity_unpin(wsent);
    return DDS_RETCODE_ILLEGAL_OPERATION;
  } else {
    // 将 wsent 转换为 dds_waitset 类型的指针 ws
    dds_waitset *ws = (dds_waitset *)wsent;
    // 定义实体指针 e
    dds_entity *e;
    // 当 waitset 和 entity 相同时，尝试注销观察者，可能会因为实体未附加而失败
    if (waitset == entity) ret = dds_entity_observer_unregister(&ws->m_entity, ws, true);
    // 尝试将 entity 实体固定到 e，如果失败，则实体无效
    else if ((ret = dds_entity_pin(entity, &e)) < 0)
      ; /* entity invalid */
    else {
      // 注销观察者
      ret = dds_entity_observer_unregister(e, ws, true);

      // 如果注销成功且实体类型为 DDS_KIND_SUBSCRIBER，则不再需要订阅者具有 DATA_ON_READERS
      if (ret >= 0 && dds_entity_kind(e) == DDS_KIND_SUBSCRIBER)
        dds_subscriber_adjust_materialize_data_on_readers((dds_subscriber *)e, false);

      // 取消固定实体 e
      dds_entity_unpin(e);
    }

    // 取消固定实体 ws->m_entity
    dds_entity_unpin(&ws->m_entity);
    // 如果返回值不是 DDS_RETCODE_OK 且不是 DDS_RETCODE_PRECONDITION_NOT_MET，则设置为
    // DDS_RETCODE_BAD_PARAMETER
    if (ret != DDS_RETCODE_OK && ret != DDS_RETCODE_PRECONDITION_NOT_MET)
      ret = DDS_RETCODE_BAD_PARAMETER;
    // 返回结果
    return ret;
  }
}

/**
 * @brief 在指定的绝对超时时间内等待 waitset。
 *
 * @param waitset 要等待的等待集。
 * @param xs 附加实体的数组。
 * @param nxs 附加实体数组的大小。
 * @param abstimeout 绝对超时时间。
 * @return 成功时返回 DDS_RETCODE_OK，否则返回错误代码。
 */
dds_return_t dds_waitset_wait_until(dds_entity_t waitset,
                                    dds_attach_t *xs,
                                    size_t nxs,
                                    dds_time_t abstimeout) {
  // 调用 dds_waitset_wait_impl 实现等待功能
  return dds_waitset_wait_impl(waitset, xs, nxs, abstimeout);
}

/**
 * @brief 在指定的相对超时时间内等待 waitset。
 *
 * @param waitset 要等待的等待集。
 * @param xs 附加实体的数组。
 * @param nxs 附加实体数组的大小。
 * @param reltimeout 相对超时时间。
 * @return 成功时返回 DDS_RETCODE_OK，否则返回错误代码。
 */
dds_return_t dds_waitset_wait(dds_entity_t waitset,
                              dds_attach_t *xs,
                              size_t nxs,
                              dds_duration_t reltimeout) {
  // 如果相对超时时间小于0，则返回错误代码
  if (reltimeout < 0) return DDS_RETCODE_BAD_PARAMETER;
  // 获取当前时间
  const dds_time_t tnow = dds_time();
  // 计算绝对超时时间
  const dds_time_t abstimeout =
      (DDS_INFINITY - reltimeout <= tnow) ? DDS_NEVER : (tnow + reltimeout);
  // 调用 dds_waitset_wait_impl 实现等待功能
  return dds_waitset_wait_impl(waitset, xs, nxs, abstimeout);
}

/**
 * @brief 设置 waitset 的触发状态。
 *
 * @param waitset 要设置触发状态的等待集。
 * @param trigger 触发状态（true 或 false）。
 * @return 成功时返回 DDS_RETCODE_OK，否则返回错误代码。
 */
dds_return_t dds_waitset_set_trigger(dds_entity_t waitset, bool trigger) {
  // 定义实体指针 ent
  dds_entity *ent;
  // 定义返回值 rc
  dds_return_t rc;
  // 尝试将 waitset 实体固定到 ent，如果失败，则返回错误代码
  if ((rc = dds_entity_pin(waitset, &ent)) != DDS_RETCODE_OK) return rc;
  // 如果 ent 的类型不是 DDS_KIND_WAITSET，则取消固定并返回错误代码
  else if (dds_entity_kind(ent) != DDS_KIND_WAITSET) {
    dds_entity_unpin(ent);
    return DDS_RETCODE_ILLEGAL_OPERATION;
  } else {
    // 定义旧的触发状态变量 oldst
    uint32_t oldst;
    // 锁定观察者锁
    ddsrt_mutex_lock(&ent->m_observers_lock);
    do {
      // 获取当前触发状态
      oldst = ddsrt_atomic_ld32(&ent->m_status.m_trigger);
    } while (!ddsrt_atomic_cas32(&ent->m_status.m_trigger, oldst, trigger));
    // 如果旧的触发状态为 0 且新的触发状态不为 0，则发送信号给实体的观察者
    if (oldst == 0 && trigger != 0) dds_entity_observers_signal(ent, trigger);
    // 解锁观察者锁
    ddsrt_mutex_unlock(&ent->m_observers_lock);
    // 取消固定实体 ent
    dds_entity_unpin(ent);
    // 返回成功代码
    return DDS_RETCODE_OK;
  }
}
