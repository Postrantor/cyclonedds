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
#include <string.h>

#include "dds/ddsc/dds_rhc.h"
#include "dds/ddsi/ddsi_domaingv.h"
#include "dds/ddsi/ddsi_entity.h"
#include "dds/ddsi/ddsi_gc.h"
#include "dds/ddsi/ddsi_iid.h"
#include "dds/ddsi/ddsi_init.h"
#include "dds/ddsi/ddsi_serdata.h"
#include "dds/ddsi/ddsi_threadmon.h"
#include "dds/ddsi/ddsi_tkmap.h"
#include "dds/ddsi/ddsi_typelib.h"
#include "dds/ddsrt/heap.h"
#include "dds/ddsrt/hopscotch.h"
#include "dds/ddsrt/process.h"
#include "dds/features.h"
#include "dds__builtin.h"
#include "dds__domain.h"
#include "dds__entity.h"
#include "dds__init.h"
#include "dds__serdata_default.h"
#include "dds__whc_builtintopic.h"

#ifdef DDS_HAS_SHM
#include "dds__shm_monitor.h"
#endif

/**
 * @brief 释放DDS域实体
 *
 * @param vdomain 指向要处理的dds_entity结构体的指针
 * @return 返回DDS返回代码，成功时为DDS_RETCODE_OK
 */
static dds_return_t dds_domain_free(dds_entity *vdomain);

// 定义DDS实体派生器结构体，用于处理DDS域实体的操作
const struct dds_entity_deriver dds_entity_deriver_domain = {
    .interrupt = dds_entity_deriver_dummy_interrupt,
    .close = dds_entity_deriver_dummy_close,
    .delete = dds_domain_free,
    .set_qos = dds_entity_deriver_dummy_set_qos,
    .validate_status = dds_entity_deriver_dummy_validate_status,
    .create_statistics = dds_entity_deriver_dummy_create_statistics,
    .refresh_statistics = dds_entity_deriver_dummy_refresh_statistics};

/**
 * @brief 比较两个DDS域ID
 *
 * @param va 指向第一个要比较的dds_domainid_t的指针
 * @param vb 指向第二个要比较的dds_domainid_t的指针
 * @return 如果相等返回0，如果a小于b返回-1，否则返回1
 */
static int dds_domain_compare(const void *va, const void *vb) {
  const dds_domainid_t *a = va;
  const dds_domainid_t *b = vb;
  return (*a == *b) ? 0 : (*a < *b) ? -1 : 1;
}

// 定义DDS域树结构体
static const ddsrt_avl_treedef_t dds_domaintree_def = DDSRT_AVL_TREEDEF_INITIALIZER(
    offsetof(dds_domain, m_node), offsetof(dds_domain, m_id), dds_domain_compare, 0);

// 定义配置源结构体
struct config_source {
  enum {
    CFGKIND_XML,  // XML类型的配置
    CFGKIND_RAW   // 原始类型的配置
  } kind;
  union {
    const char *xml;                // XML配置字符串
    const struct ddsi_config *raw;  // 原始配置结构体指针
  } u;
};

/**
 * @brief 初始化域实例
 *
 * @param[in] domain        域指针，用于存储初始化后的域实例
 * @param[in] domain_id     域ID，用于标识特定的域实例
 * @param[in] config        配置源结构体，包含配置信息
 * @param[in] implicit      是否为隐式创建的域
 * @return dds_entity_t     返回初始化后的域实例句柄
 */
static dds_entity_t dds_domain_init(dds_domain *domain,
                                    dds_domainid_t domain_id,
                                    const struct config_source *config,
                                    bool implicit) {
  // 定义域实例句柄变量
  dds_entity_t domh;

  // 初始化域实例并检查是否成功
  if ((domh = dds_entity_init(&domain->m_entity, &dds_global.m_entity, DDS_KIND_DOMAIN, implicit,
                              true, NULL, NULL, 0)) < 0)
    return domh;
  // 设置域实例的域和实例ID
  domain->m_entity.m_domain = domain;
  domain->m_entity.m_iid = ddsi_iid_gen();

  // 获取当前时间作为域实例启动时间
  domain->gv.tstart = ddsrt_time_wallclock();

  // 根据配置模型设置域ID
  switch (config->kind) {
    case CFGKIND_RAW:
      domain->cfgst = NULL;
      memcpy(&domain->gv.config, config->u.raw, sizeof(domain->gv.config));
      if (domain_id != DDS_DOMAIN_DEFAULT) domain->gv.config.domainId = domain_id;
      break;

    case CFGKIND_XML:
      domain->cfgst = ddsi_config_init(config->u.xml, &domain->gv.config, domain_id);
      if (domain->cfgst == NULL) {
        DDS_ILOG(DDS_LC_CONFIG, domain_id, "Failed to parse configuration\n");
        domh = DDS_RETCODE_ERROR;
        goto fail_config;
      }
      assert(domain_id == DDS_DOMAIN_DEFAULT || domain_id == domain->gv.config.domainId);
      break;
  }
  // 设置域实例的ID
  domain->m_id = domain->gv.config.domainId;

  // 准备配置并检查是否成功
  if (ddsi_config_prep(&domain->gv, domain->cfgst) != 0) {
    DDS_ILOG(DDS_LC_CONFIG, domain->m_id, "Failed to configure RTPS\n");
    domh = DDS_RETCODE_ERROR;
    goto fail_ddsi_config;
  }

  // 初始化RTPS并检查是否成功
  if (ddsi_init(&domain->gv) < 0) {
    DDS_ILOG(DDS_LC_CONFIG, domain->m_id, "Failed to initialize RTPS\n");
    domh = DDS_RETCODE_ERROR;
    goto fail_ddsi_init;
  }

  // 创建序列化数据池
  domain->serpool = dds_serdatapool_new();

#ifdef DDS_HAS_SHM
  // 如果启用了共享内存，初始化基于iceoryx的共享内存监视器
  if (domain->gv.config.enable_shm) {
    dds_shm_monitor_init(&domain->m_shm_monitor);
  }
#endif

  /**
   * @brief 如果启用了线程活跃度监控，初始化线程活跃度监视器
   * @param domain 指向 dds_domain 的指针
   * @param domh 返回的域句柄
   */
  if (domain->gv.config.liveliness_monitoring) {
    // 如果启用了生命周期监控
    if (dds_global.threadmon_count++ == 0) {
      // 如果全局线程监视器计数为 0，则创建新的线程监视器并增加计数
      dds_global.threadmon = ddsi_threadmon_new(DDS_MSECS(333), true);
      // 判断线程监视器是否创建成功
      if (dds_global.threadmon == NULL) {
        // 记录错误日志：无法创建线程生命周期监视器
        DDS_ILOG(DDS_LC_CONFIG, domain->m_id, "Failed to create a thread liveliness monitor\n");
        // 设置错误码为 OUT_OF_RESOURCES
        domh = DDS_RETCODE_OUT_OF_RESOURCES;
        // 跳转到错误处理标签
        goto fail_threadmon_new;
      }

      // 启动线程生命周期监视器
      if (ddsi_threadmon_start(dds_global.threadmon, "threadmon") < 0) {
        // 记录错误日志：无法启动线程生命周期监视器
        DDS_ILOG(DDS_LC_ERROR, domain->m_id, "Failed to start the thread liveliness monitor\n");
        // 设置错误码为 ERROR
        domh = DDS_RETCODE_ERROR;
        // 跳转到错误处理标签
        goto fail_threadmon_start;
      }
    }
  }

  // 初始化内置实体
  dds__builtin_init(domain);

  // 启动RTPS并检查是否成功
  if (ddsi_start(&domain->gv) < 0) {
    DDS_ILOG(DDS_LC_CONFIG, domain->m_id, "Failed to start RTPS\n");
    domh = DDS_RETCODE_ERROR;
    goto fail_ddsi_start;
  }

  // 如果启用了线程活跃度监控，注册域到线程活跃度监视器
  if (domain->gv.config.liveliness_monitoring)
    ddsi_threadmon_register_domain(dds_global.threadmon, &domain->gv);
  // 完成域实例的初始化
  dds_entity_init_complete(&domain->m_entity);
  return domh;

/**
 * @brief 处理失败的情况，释放资源并返回错误代码
 *
 * @param[in] domain 指向dds_domain结构体的指针，包含了DDS域的相关信息
 * @return 返回domh，表示错误代码
 */
fail_ddsi_start:
  // 结束内置实体并释放资源
  dds__builtin_fini(domain);
  // 如果启用了生命周期监控且线程监视器计数为1，则停止线程监视器
  if (domain->gv.config.liveliness_monitoring && dds_global.threadmon_count == 1)
    ddsi_threadmon_stop(dds_global.threadmon);
fail_threadmon_start:
  // 如果启用了生命周期监控且递减线程监视器计数后为0，则释放线程监视器资源并将其设置为NULL
  if (domain->gv.config.liveliness_monitoring && --dds_global.threadmon_count == 0) {
    ddsi_threadmon_free(dds_global.threadmon);
    dds_global.threadmon = NULL;
  }
fail_threadmon_new:
  // 结束DDSI服务并释放资源
  ddsi_fini(&domain->gv);
  // 释放序列化数据池资源
  dds_serdatapool_free(domain->serpool);
fail_ddsi_init:
fail_ddsi_config:
  // 如果存在配置状态，则结束配置并释放资源
  if (domain->cfgst) ddsi_config_fini(domain->cfgst);
fail_config:
  // 删除实体句柄并释放资源
  dds_handle_delete(&domain->m_entity.m_hdllink);
  // 返回错误代码
  return domh;
}

/**
 * @brief 在已锁定的情况下查找指定ID的DDS域
 *
 * @param[in] id 要查找的DDS域的ID
 * @return 返回指向dds_domain结构体的指针，如果找到了对应ID的DDS域；否则返回NULL
 */
dds_domain *dds_domain_find_locked(dds_domainid_t id) {
  // 使用ddsrt_avl_lookup在dds_global.m_domains中查找具有给定ID的DDS域
  return ddsrt_avl_lookup(&dds_domaintree_def, &dds_global.m_domains, &id);
}

/**
 * @brief 创建域实体，支持 XML 或原始配置。
 *
 * @param[out] domain_out 输出参数，返回创建的域指针。
 * @param[in] id 域 ID，可以是 DDS_DOMAIN_DEFAULT。
 * @param[in] implicit 是否隐式创建域。
 * @param[in] config 配置源，包含 XML 或原始配置数据。
 * @return 返回创建的域实体句柄，如果失败则返回错误码。
 */
static dds_entity_t dds_domain_create_internal_xml_or_raw(dds_domain **domain_out,
                                                          dds_domainid_t id,
                                                          bool implicit,
                                                          const struct config_source *config) {
  struct dds_domain *dom;
  dds_entity_t domh = DDS_RETCODE_ERROR;

  // FIXME: 应该像其他地方一样锁定父对象
  ddsrt_mutex_lock(&dds_global.m_mutex);
retry:
  // 如果 id 不是默认域，则查找对应的域
  if (id != DDS_DOMAIN_DEFAULT)
    dom = dds_domain_find_locked(id);
  else
    // 否则查找最小的域
    dom = ddsrt_avl_find_min(&dds_domaintree_def, &dds_global.m_domains);

  // 如果找到了域
  if (dom) {
    // 如果 dom 不为空
    if (!implicit) {
      // 如果不是隐式创建域，则设置错误码为 PRECONDITION_NOT_MET
      domh = DDS_RETCODE_PRECONDITION_NOT_MET;
    } else {
      // 锁定实体互斥锁
      ddsrt_mutex_lock(&dom->m_entity.m_mutex);
      // 判断实体句柄是否已关闭
      if (dds_handle_is_closed(&dom->m_entity.m_hdllink)) {
        // 解锁实体互斥锁
        ddsrt_mutex_unlock(&dom->m_entity.m_mutex);
        // 等待全局条件变量
        ddsrt_cond_wait(&dds_global.m_cond, &dds_global.m_mutex);
        // 跳转到重试标签
        goto retry;
      }
      // 增加实体引用计数
      dds_entity_add_ref_locked(&dom->m_entity);
      // 重新锁定实体句柄
      dds_handle_repin(&dom->m_entity.m_hdllink);
      // 获取实体句柄
      domh = dom->m_entity.m_hdllink.hdl;
      // 解锁实体互斥锁
      ddsrt_mutex_unlock(&dom->m_entity.m_mutex);
      // 设置输出参数 domain_out
      *domain_out = dom;
    }
  } else {
    // 如果 dom 为空，分配内存空间
    dom = dds_alloc(sizeof(*dom));
    // 初始化域，并获取域句柄
    if ((domh = dds_domain_init(dom, id, config, implicit)) < 0) {
      // 如果初始化失败，释放内存空间
      dds_free(dom);
    } else {
      // 锁定实体互斥锁
      ddsrt_mutex_lock(&dom->m_entity.m_mutex);
      // 将新创建的域插入到全局域树中
      ddsrt_avl_insert(&dds_domaintree_def, &dds_global.m_domains, dom);
      // 注册子实体
      dds_entity_register_child(&dds_global.m_entity, &dom->m_entity);
      // 如果是隐式创建域
      if (implicit) {
        // 增加实体引用计数
        dds_entity_add_ref_locked(&dom->m_entity);
        // 重新锁定实体句柄
        dds_handle_repin(&dom->m_entity.m_hdllink);
      }
      // 获取实体句柄
      domh = dom->m_entity.m_hdllink.hdl;
      // 解锁实体互斥锁
      ddsrt_mutex_unlock(&dom->m_entity.m_mutex);
      // 设置输出参数 domain_out
      *domain_out = dom;
    }
  }
  ddsrt_mutex_unlock(&dds_global.m_mutex);
  return domh;
}

/**
 * @brief 创建内部域实体
 *
 * @param[out] domain_out 输出的域指针
 * @param[in] id 域ID
 * @param[in] implicit 是否隐式创建
 * @param[in] config_xml 配置文件的XML字符串
 * @return 返回创建的实体
 */
dds_entity_t dds_domain_create_internal(dds_domain **domain_out,
                                        dds_domainid_t id,
                                        bool implicit,
                                        const char *config_xml) {
  // 初始化配置结构体
  const struct config_source config = {.kind = CFGKIND_XML, .u = {.xml = config_xml}};

  // 调用另一个函数创建内部域实体
  return dds_domain_create_internal_xml_or_raw(domain_out, id, implicit, &config);
}

/**
 * @brief 创建域
 *
 * @param[in] domain 域ID
 * @param[in] config_xml 配置文件的XML字符串
 * @return 返回创建的实体
 */
dds_entity_t dds_create_domain(const dds_domainid_t domain, const char *config_xml) {
  dds_domain *dom;
  dds_entity_t ret;

  // 检查是否为默认域
  if (domain == DDS_DOMAIN_DEFAULT) return DDS_RETCODE_BAD_PARAMETER;

  // 如果配置文件为空，则设置为空字符串
  if (config_xml == NULL) config_xml = "";

  // 确保DDS实例已初始化
  if ((ret = dds_init()) < 0) return ret;

  // 初始化配置结构体
  const struct config_source config = {.kind = CFGKIND_XML, .u = {.xml = config_xml}};

  // 调用另一个函数创建内部域实体
  ret = dds_domain_create_internal_xml_or_raw(&dom, domain, false, &config);

  // 解除引用并删除实体
  dds_entity_unpin_and_drop_ref(&dds_global.m_entity);

  return ret;
}

/**
 * @brief 创建一个具有原始配置的DDS域实体 (Create a DDS domain entity with raw configuration)
 *
 * @param[in] domain 域ID，不能为DDS_DOMAIN_DEFAULT (Domain ID, cannot be DDS_DOMAIN_DEFAULT)
 * @param[in] config_raw 指向ddsi_config结构的指针，包含原始配置信息 (Pointer to ddsi_config
 * structure containing the raw configuration information)
 * @return 成功时返回创建的域实体，否则返回错误代码 (Returns the created domain entity on success,
 * otherwise returns an error code)
 */
dds_entity_t dds_create_domain_with_rawconfig(const dds_domainid_t domain,
                                              const struct ddsi_config *config_raw) {
  // 定义一个指向dds_domain的指针
  dds_domain *dom;
  // 定义一个dds_entity_t类型的变量用于存储返回值
  dds_entity_t ret;

  // 检查domain是否为DDS_DOMAIN_DEFAULT，如果是，则返回错误代码
  if (domain == DDS_DOMAIN_DEFAULT) return DDS_RETCODE_BAD_PARAMETER;
  // 检查config_raw是否为空，如果是，则返回错误代码
  if (config_raw == NULL) return DDS_RETCODE_BAD_PARAMETER;

  // 确保DDS实例已初始化
  if ((ret = dds_init()) < 0) return ret;

  // 创建并初始化config_source结构
  const struct config_source config = {.kind = CFGKIND_RAW, .u = {.raw = config_raw}};
  // 调用内部函数创建域实体
  ret = dds_domain_create_internal_xml_or_raw(&dom, domain, false, &config);
  // 解除实体引用并删除
  dds_entity_unpin_and_drop_ref(&dds_global.m_entity);
  // 返回创建的域实体或错误代码
  return ret;
}

/**
 * @brief 释放DDS域实体 (Free a DDS domain entity)
 *
 * @param[in] vdomain 指向要释放的dds_entity结构的指针 (Pointer to the dds_entity structure to be
 * freed)
 * @return 返回DDS_RETCODE_NO_DATA
 */
static dds_return_t dds_domain_free(dds_entity *vdomain) {
  // 将vdomain转换为dds_domain类型的指针
  struct dds_domain *domain = (struct dds_domain *)vdomain;
  // 停止DDSI服务
  ddsi_stop(&domain->gv);
  // 清理内置实体
  dds__builtin_fini(domain);

  // 如果启用了生命周期监控，则取消注册域
  if (domain->gv.config.liveliness_monitoring)
    ddsi_threadmon_unregister_domain(dds_global.threadmon, &domain->gv);

#ifdef DDS_HAS_SHM
  // 如果启用了共享内存，则销毁共享内存监视器
  if (domain->gv.config.enable_shm) dds_shm_monitor_destroy(&domain->m_shm_monitor);
#endif

  // 结束DDSI服务
  ddsi_fini(&domain->gv);
  // 释放序列化数据池
  dds_serdatapool_free(domain->serpool);

  // 锁定全局互斥锁
  ddsrt_mutex_lock(&dds_global.m_mutex);
  // 如果启用了生命周期监控且所有域都已删除，则停止线程监视器并释放资源
  if (domain->gv.config.liveliness_monitoring && --dds_global.threadmon_count == 0) {
    ddsi_threadmon_stop(dds_global.threadmon);
    ddsi_threadmon_free(dds_global.threadmon);
  }

  // 从全局域树中删除域实体
  ddsrt_avl_delete(&dds_domaintree_def, &dds_global.m_domains, domain);
  // 在释放之前执行实体的最终反初始化
  dds_entity_final_deinit_before_free(vdomain);
  // 如果存在配置状态，则清理配置状态
  if (domain->cfgst) ddsi_config_fini(domain->cfgst);
  // 释放vdomain内存
  dds_free(vdomain);
  // 广播全局条件变量
  ddsrt_cond_broadcast(&dds_global.m_cond);
  // 解锁全局互斥锁
  ddsrt_mutex_unlock(&dds_global.m_mutex);
  // 返回DDS_RETCODE_NO_DATA
  return DDS_RETCODE_NO_DATA;
}

/**
 * @brief 设置实体的听力和发声状态
 *
 * @param entity 实体标识符
 * @param deaf 是否使实体变为聋（无法接收消息）
 * @param mute 是否使实体变为哑（无法发送消息）
 * @param reset_after 在此时间段后重置听力和发声状态
 * @return dds_return_t 操作结果代码
 */
dds_return_t dds_domain_set_deafmute(dds_entity_t entity,
                                     bool deaf,
                                     bool mute,
                                     dds_duration_t reset_after) {
  struct dds_entity *e;                       // 定义实体指针
  dds_return_t rc;                            // 定义返回值
  if ((rc = dds_entity_pin(entity, &e)) < 0)  // 尝试获取实体，如果失败则返回错误码
    return rc;
  if (e->m_domain == NULL)  // 如果实体没有关联域，则返回非法操作错误
    rc = DDS_RETCODE_ILLEGAL_OPERATION;
  else {
    ddsi_set_deafmute(&e->m_domain->gv, deaf, mute, reset_after);  // 设置实体的听力和发声状态
    rc = DDS_RETCODE_OK;  // 设置成功，返回 OK 状态码
  }
  dds_entity_unpin(e);  // 解除实体引用
  return rc;            // 返回操作结果
}

#include "dds__entity.h"

/**
 * @brief 向下传递批处理设置
 *
 * @param e 实体指针
 * @param enable 是否启用批处理
 */
static void pushdown_set_batch(struct dds_entity *e, bool enable) {
  // e 已经被引用，没有锁定
  dds_instance_handle_t last_iid = 0;  // 定义最后一个实例标识符
  struct dds_entity *c;                // 定义子实体指针
  ddsrt_mutex_lock(&e->m_mutex);       // 锁定实体互斥量

  // 遍历子实体
  while ((c = ddsrt_avl_lookup_succ(&dds_entity_children_td, &e->m_children, &last_iid)) != NULL) {
    struct dds_entity *x;
    last_iid = c->m_iid;
    if (dds_entity_pin(c->m_hdllink.hdl, &x) < 0)  // 尝试获取子实体，如果失败则跳过
      continue;
    assert(x == c);
    ddsrt_mutex_unlock(&e->m_mutex);  // 解锁实体互斥量

    // 根据子实体类型进行操作
    if (c->m_kind == DDS_KIND_PARTICIPANT)
      pushdown_set_batch(c, enable);  // 如果是参与者，则递归调用此函数
    else if (c->m_kind == DDS_KIND_WRITER) {
      struct dds_writer *w = (struct dds_writer *)c;  // 转换为写入器类型
      w->whc_batch = enable;                          // 设置批处理状态
    }

    ddsrt_mutex_lock(&e->m_mutex);  // 再次锁定实体互斥量
    dds_entity_unpin(c);            // 解除子实体引用
  }
  ddsrt_mutex_unlock(&e->m_mutex);  // 解锁实体互斥量
}

/**
 * @brief 设置DDS写入批处理模式
 *
 * @param enable 是否启用批处理模式
 */
void dds_write_set_batch(bool enable) {
  // FIXME: 获取通道和延迟预算并解决此问题；同时，任何丑陋的hack都可以。
  struct dds_domain *dom;
  dds_domainid_t next_id = 0;

  // 如果DDS初始化失败，则返回
  if (dds_init() < 0) return;
  // 锁定全局互斥锁
  ddsrt_mutex_lock(&dds_global.m_mutex);

  // 遍历所有域
  while ((dom = ddsrt_avl_lookup_succ_eq(&dds_domaintree_def, &dds_global.m_domains, &next_id)) !=
         NULL) {
    // 确保编译器不会从dom->m_id重新加载curr_id
    dds_domainid_t curr_id = *((volatile dds_domainid_t *)&dom->m_id);
    next_id = curr_id + 1;
    // 设置批处理模式
    dom->gv.config.whc_batch = enable;

    dds_instance_handle_t last_iid = 0;
    struct dds_entity *e;

    // 遍历实体
    while (dom && (e = ddsrt_avl_lookup_succ(&dds_entity_children_td, &dom->m_entity.m_children,
                                             &last_iid)) != NULL) {
      struct dds_entity *x;
      last_iid = e->m_iid;
      // 尝试锁定实体
      if (dds_entity_pin(e->m_hdllink.hdl, &x) < 0) continue;

      // 断言实体相等
      assert(x == e);
      // 解锁全局互斥锁
      ddsrt_mutex_unlock(&dds_global.m_mutex);
      // 设置批处理模式
      pushdown_set_batch(e, enable);
      // 重新锁定全局互斥锁
      ddsrt_mutex_lock(&dds_global.m_mutex);
      // 解锁实体
      dds_entity_unpin(e);
      // 查找当前域
      dom = ddsrt_avl_lookup(&dds_domaintree_def, &dds_global.m_domains, &curr_id);
    }
  }

  // 解锁全局互斥锁
  ddsrt_mutex_unlock(&dds_global.m_mutex);
  // 解锁并删除全局实体的引用
  dds_entity_unpin_and_drop_ref(&dds_global.m_entity);
}

#ifdef DDS_HAS_TYPE_DISCOVERY

/**
 * @brief 获取类型对象
 *
 * @param entity 实体ID
 * @param type_id 类型ID指针
 * @param timeout 超时时间
 * @param type_obj 类型对象双重指针
 * @return dds_return_t 返回状态码
 *
 * @details 该函数用于获取类型对象，如果成功，将类型对象存储在type_obj中。
 */
dds_return_t dds_get_typeobj(dds_entity_t entity,
                             const dds_typeid_t *type_id,
                             dds_duration_t timeout,
                             dds_typeobj_t **type_obj) {
  // 定义返回值变量
  dds_return_t ret;
  // 定义实体结构体指针
  struct dds_entity *e;

  // 检查type_obj是否为空
  if (type_obj == NULL) return DDS_RETCODE_BAD_PARAMETER;
  // 尝试获取实体并检查返回值
  if ((ret = dds_entity_pin(entity, &e)) < 0) return ret;

  // 检查实体的域是否为空
  if (e->m_domain == NULL)
    ret = DDS_RETCODE_ILLEGAL_OPERATION;
  else {
    // 获取实体域的全局变量
    struct ddsi_domaingv *gv = &e->m_domain->gv;
    // 定义类型结构体指针
    struct ddsi_type *type;
    // 等待类型解析完成，并检查返回值
    if ((ret = ddsi_wait_for_type_resolved(gv, (const ddsi_typeid_t *)type_id, timeout, &type,
                                           DDSI_TYPE_IGNORE_DEPS, DDSI_TYPE_SEND_REQUEST)) ==
        DDS_RETCODE_OK) {
      // 获取类型对象并存储在type_obj中
      *type_obj = ddsi_type_get_typeobj(gv, type);
      // 释放类型引用
      ddsi_type_unref(gv, type);
    }
  }
  // 解除实体引用
  dds_entity_unpin(e);
  // 返回状态码
  return ret;
}

/**
 * @brief 释放类型对象
 *
 * @param type_obj 类型对象指针
 * @return dds_return_t 返回状态码
 *
 * @details 该函数用于释放类型对象，如果成功，将释放类型对象占用的内存。
 */
dds_return_t dds_free_typeobj(dds_typeobj_t *type_obj) {
  // 检查type_obj是否为空
  if (type_obj == NULL) return DDS_RETCODE_BAD_PARAMETER;
  // 销毁类型对象
  ddsi_typeobj_fini(type_obj);
  // 释放类型对象内存
  dds_free(type_obj);
  // 返回状态码
  return DDS_RETCODE_OK;
}

#else

/**
 * @brief 获取类型对象
 *
 * @param entity 实体
 * @param type_id 类型ID指针
 * @param timeout 超时时间
 * @param type_obj 类型对象双重指针
 * @return dds_return_t 返回状态码
 *
 * 该函数用于获取类型对象。
 */
dds_return_t dds_get_typeobj(dds_entity_t entity,
                             const dds_typeid_t *type_id,
                             dds_duration_t timeout,
                             dds_typeobj_t **type_obj) {
  (void)entity;                    // 忽略实体参数
  (void)type_id;                   // 忽略类型ID指针参数
  (void)timeout;                   // 忽略超时时间参数
  (void)type_obj;                  // 忽略类型对象双重指针参数
  return DDS_RETCODE_UNSUPPORTED;  // 返回不支持的状态码
}

/**
 * @brief 释放类型对象
 *
 * @param type_obj 类型对象指针
 * @return dds_return_t 返回状态码
 *
 * 该函数用于释放类型对象。
 */
dds_return_t dds_free_typeobj(dds_typeobj_t *type_obj) {
  (void)type_obj;                  // 忽略类型对象指针参数
  return DDS_RETCODE_UNSUPPORTED;  // 返回不支持的状态码
}

#endif /* DDS_HAS_TYPE_DISCOVERY */
