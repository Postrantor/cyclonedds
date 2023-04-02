/*
 * Copyright(c) 2021 Apex.AI Inc. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v. 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0, or the Eclipse Distribution License
 * v. 1.0 which is available at
 * http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * SPDX-License-Identifier: EPL-2.0 OR BSD-3-Clause
 */

#include "dds/ddsi/ddsi_entity.h"
#include "dds/ddsi/ddsi_entity_index.h"
#include "dds/ddsi/ddsi_rhc.h"
#include "dds/ddsi/ddsi_serdata.h"
#include "dds/ddsi/ddsi_tkmap.h"
#include "dds/ddsi/ddsi_xmsg.h"
#include "dds__entity.h"
#include "dds__reader.h"
#include "dds__shm_monitor.h"
#include "dds__types.h"
#include "iceoryx_binding_c/chunk.h"

#if defined(__cplusplus)
extern "C" {
#endif

// 定义一个静态函数，用于处理共享内存订阅器的回调
static void shm_subscriber_callback(iox_sub_t subscriber, void *context_data);

// 初始化共享内存监视器
void dds_shm_monitor_init(shm_monitor_t *monitor) {
  // 初始化互斥锁
  ddsrt_mutex_init(&monitor->m_lock);

  // 初始化监听器，尽管现在内部忽略了存储，但我们不能传递空指针
  monitor->m_listener = iox_listener_init(&(iox_listener_storage_t){0});
  // 初始化唤醒触发器
  monitor->m_wakeup_trigger = iox_user_trigger_init(&(iox_user_trigger_storage_t){0});

  // 设置监视器状态为运行中
  monitor->m_state = SHM_MONITOR_RUNNING;
}

// 销毁共享内存监视器
void dds_shm_monitor_destroy(shm_monitor_t *monitor) {
  // 唤醒并禁用监视器
  dds_shm_monitor_wake_and_disable(monitor);
  // 不需要等待读取器分离，因为它们会在监听器销毁时分离
  // 监听器的反初始化将等待内部监听器线程加入
  // 任何剩余的回调将被执行

  // 反初始化监听器
  iox_listener_deinit(monitor->m_listener);
  // 反初始化用户触发器
  iox_user_trigger_deinit(monitor->m_wakeup_trigger);
  // 销毁互斥锁
  ddsrt_mutex_destroy(&monitor->m_lock);
}

// 唤醒并禁用共享内存监视器
dds_return_t dds_shm_monitor_wake_and_disable(shm_monitor_t *monitor) {
  // 设置监视器状态为不运行
  monitor->m_state = SHM_MONITOR_NOT_RUNNING;
  // 触发用户触发器
  iox_user_trigger_trigger(monitor->m_wakeup_trigger);
  // 返回成功代码
  return DDS_RETCODE_OK;
}

// 唤醒并启用共享内存监视器
dds_return_t dds_shm_monitor_wake_and_enable(shm_monitor_t *monitor) {
  // 设置监视器状态为运行中
  monitor->m_state = SHM_MONITOR_RUNNING;
  // 触发用户触发器
  iox_user_trigger_trigger(monitor->m_wakeup_trigger);
  // 返回成功代码
  return DDS_RETCODE_OK;
}

// 将读取器附加到共享内存监视器
dds_return_t dds_shm_monitor_attach_reader(shm_monitor_t *monitor, struct dds_reader *reader) {
  // 使用上下文数据将订阅器事件附加到监听器，如果不成功则记录错误并返回资源不足的错误代码
  if (iox_listener_attach_subscriber_event_with_context_data(
          monitor->m_listener, reader->m_iox_sub, SubscriberEvent_DATA_RECEIVED,
          shm_subscriber_callback, &reader->m_iox_sub_context) != ListenerResult_SUCCESS) {
    DDS_CLOG(DDS_LC_SHM, &reader->m_rd->e.gv->logconfig, "error attaching reader\n");
    return DDS_RETCODE_OUT_OF_RESOURCES;
  }
  // 增加已附加读取器的数量
  ++monitor->m_number_of_attached_readers;

  // 返回成功代码
  return DDS_RETCODE_OK;
}

// 从共享内存监视器中分离读取器
dds_return_t dds_shm_monitor_detach_reader(shm_monitor_t *monitor, struct dds_reader *reader) {
  // 从监听器中分离订阅器事件
  iox_listener_detach_subscriber_event(monitor->m_listener, reader->m_iox_sub,
                                       SubscriberEvent_DATA_RECEIVED);
  // 减少已附加读取器的数量
  --monitor->m_number_of_attached_readers;
  // 返回成功代码
  return DDS_RETCODE_OK;
}

// 定义一个静态函数，用于处理接收到的数据
static void receive_data_wakeup_handler(struct dds_reader *rd) {
  // 定义一个指向void类型的指针chunk，并初始化为NULL
  void *chunk = NULL;
  // 获取dds_reader中的ddsi_domaingv结构体指针
  struct ddsi_domaingv *gv = rd->m_rd->e.gv;
  // 唤醒对应的线程状态
  ddsi_thread_state_awake(ddsi_lookup_thread_state(), gv);

  // 使用while循环不断处理接收到的数据
  while (true) {
    // 对iox_sub进行加锁操作
    shm_lock_iox_sub(rd->m_iox_sub);
    // 尝试从iox_sub中获取数据块
    enum iox_ChunkReceiveResult take_result =
        iox_sub_take_chunk(rd->m_iox_sub, (const void **const)&chunk);
    // 对iox_sub进行解锁操作
    shm_unlock_iox_sub(rd->m_iox_sub);

    // 注意：如果无法获取数据块（样本），用户可能会丢失数据。
    // 因为订阅者队列可能会溢出，并且会清除最近的样本。
    // 这完全取决于生产者和消费者的频率（以及它们之间的队列大小）。
    // 这里的消费者实际上是读者历史缓存。
    if (ChunkReceiveResult_SUCCESS != take_result) {
      // 根据take_result的值进行不同的处理
      switch (take_result) {
        case ChunkReceiveResult_TOO_MANY_CHUNKS_HELD_IN_PARALLEL: {
          // 我们持有太多的数据块，无法获取更多
          DDS_CLOG(
              DDS_LC_WARNING | DDS_LC_SHM, &rd->m_entity.m_domain->gv.logconfig,
              "DDS reader with topic %s : iceoryx subscriber - TOO_MANY_CHUNKS_HELD_IN_PARALLEL -"
              "could not take sample\n",
              rd->m_topic->m_name);
          break;
        }
        case ChunkReceiveResult_NO_CHUNK_AVAILABLE: {
          // 没有更多的数据块可用，正常情况
          break;
        }
        default: {
          // 发生了一些未知错误
          DDS_CLOG(DDS_LC_WARNING | DDS_LC_SHM, &rd->m_entity.m_domain->gv.logconfig,
                   "DDS reader with topic %s : iceoryx subscriber - UNKNOWN ERROR -"
                   "could not take sample\n",
                   rd->m_topic->m_name);
        }
      }

      break;
    }

    // 从数据块中获取iceoryx_header_t结构体指针
    const iceoryx_header_t *ice_hdr = iceoryx_header_from_chunk(chunk);

    // 获取写入者或代理写入者实体
    struct ddsi_entity_common *e =
        ddsi_entidx_lookup_guid_untyped(gv->entity_index, &ice_hdr->guid);
    if (e == NULL || (e->kind != DDSI_EK_PROXY_WRITER && e->kind != DDSI_EK_WRITER)) {
      // 忽略不匹配已知写入者或代理写入者的实体
      DDS_CLOG(DDS_LC_SHM, &gv->logconfig, "unknown source entity, ignore.\n");
      shm_lock_iox_sub(rd->m_iox_sub);
      iox_sub_release_chunk(rd->m_iox_sub, chunk);
      chunk = NULL;
      shm_unlock_iox_sub(rd->m_iox_sub);
      continue;
    }

    // 创建ddsi_serdata结构体
    struct ddsi_serdata *d =
        ddsi_serdata_from_iox(rd->m_topic->m_stype, ice_hdr->data_kind, &rd->m_iox_sub, chunk);
    d->timestamp.v = ice_hdr->tstamp;
    d->statusinfo = ice_hdr->statusinfo;

    // 获取ddsi_tkmap_instance结构体
    struct ddsi_tkmap_instance *tk;
    if ((tk = ddsi_tkmap_lookup_instance_ref(gv->m_tkmap, d)) == NULL) {
      DDS_CLOG(DDS_LC_SHM, &gv->logconfig, "ddsi_tkmap_lookup_instance_ref failed.\n");
      goto release;
    }

    // 生成writer_info结构体
    struct ddsi_writer_info wrinfo;
    struct dds_qos *xqos;
    if (e->kind == DDSI_EK_PROXY_WRITER)
      xqos = ((struct ddsi_proxy_writer *)e)->c.xqos;
    else
      xqos = ((struct ddsi_writer *)e)->xqos;
    ddsi_make_writer_info(&wrinfo, e, xqos, d->statusinfo);
    (void)ddsi_rhc_store(rd->m_rd->rhc, &wrinfo, d, tk);

  release:
    // 释放tk和d的引用
    if (tk) ddsi_tkmap_instance_unref(gv->m_tkmap, tk);
    if (d) ddsi_serdata_unref(d);
  }
  // 设置线程状态为休眠
  ddsi_thread_state_asleep(ddsi_lookup_thread_state());
}
// 定义一个静态函数，用于处理共享内存订阅者的回调
static void shm_subscriber_callback(iox_sub_t subscriber, void *context_data) {
  // 将传入的订阅者参数置为空，因为在这个函数中我们不需要使用它
  (void)subscriber;

  // 我们知道 context_data 实际上是扩展存储中的数据，因为我们创建时就是这样设置的
  iox_sub_context_t *context = (iox_sub_context_t *)context_data;

  // 如果共享内存监视器的状态为运行中（SHM_MONITOR_RUNNING）
  if (context->monitor->m_state == SHM_MONITOR_RUNNING) {
    // 调用接收数据唤醒处理程序，传入父级读取器作为参数
    receive_data_wakeup_handler(context->parent_reader);
  }
}

#if defined(__cplusplus)
}
#endif
