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
#include "dds/dds.h"
#include "dds/ddsi/ddsi_domaingv.h"
#include "dds/ddsi/ddsi_plist.h"
#include "dds/ddsrt/heap.h"
#include "dds/ddsrt/string.h"
#include "dds/ddsrt/sync.h"
#include "dds__qos.h"
#include <assert.h>
#include <stdbool.h>
#include <string.h>
/**
 * @brief 复制数据到ddsi_octetseq_t结构体中
 *
 * @param[out] data      指向ddsi_octetseq_t结构体的指针，用于存储复制后的数据
 * @param[in]  value     需要复制的原始数据的指针
 * @param[in]  sz        原始数据的大小（字节数）
 * @param[in]  overwrite 是否覆盖data中已有的数据
 */
static void dds_qos_data_copy_in(ddsi_octetseq_t *data, const void *__restrict value, size_t sz, bool overwrite)
{
  // 如果允许覆盖并且data中已经有值，则释放内存
  if (overwrite && data->value)
    ddsrt_free(data->value);

  // 设置data的长度为sz
  data->length = (uint32_t)sz;

  // 根据value是否为空，分别进行内存复制或设置为空
  data->value = value ? ddsrt_memdup(value, sz) : NULL;
}

/**
 * @brief 从ddsi_octetseq_t结构体中复制数据
 *
 * @param[in]  data  指向ddsi_octetseq_t结构体的指针，包含需要复制的数据
 * @param[out] value 存储复制出的数据的指针的地址
 * @param[out] sz    存储复制出的数据大小（字节数）的指针
 * @return           成功返回true，失败返回false
 */
static bool dds_qos_data_copy_out(const ddsi_octetseq_t *data, void **value, size_t *sz)
{
  // 检查data的长度是否小于UINT32_MAX
  assert(data->length < UINT32_MAX);

  // 如果sz为NULL且value不为NULL，返回false
  if (sz == NULL && value != NULL)
    return false;

  // 将data的长度赋值给sz指向的变量
  if (sz)
    *sz = data->length;

  // 如果value非空，进行数据复制操作
  if (value)
  {
    // 如果data的长度为0，将value指向的指针设置为NULL
    if (data->length == 0)
      *value = NULL;
    else
    {
      // 确保data的value非空
      assert(data->value);

      // 为value分配内存，并将data的value复制到value指向的内存中
      *value = dds_alloc(data->length + 1);
      memcpy(*value, data->value, data->length);

      // 在复制后的数据末尾添加一个空字符
      ((char *)(*value))[data->length] = 0;
    }
  }

  // 返回true表示成功
  return true;
}
/**
 * @brief 创建一个dds_qos_t结构体实例
 *
 * @return 返回指向新创建的dds_qos_t结构体的指针
 */
dds_qos_t *dds_create_qos(void)
{
  // 分配内存并初始化一个dds_qos_t结构体
  dds_qos_t *qos = ddsrt_malloc(sizeof(dds_qos_t));
  ddsi_xqos_init_empty(qos);

  // 返回新创建的结构体指针
  return qos;
}

/**
 * @brief 重置dds_qos_t结构体
 *
 * @param[in,out] qos 指向dds_qos_t结构体的指针，需要被重置
 */
void dds_reset_qos(dds_qos_t *__restrict qos)
{
  // 如果qos非空，则进行重置操作
  if (qos)
  {
    ddsi_xqos_fini(qos);
    ddsi_xqos_init_empty(qos);
  }
}

/**
 * @brief 删除dds_qos_t结构体实例
 *
 * @param[in] qos 指向需要删除的dds_qos_t结构体的指针
 */
void dds_delete_qos(dds_qos_t *__restrict qos)
{
  // 如果qos非空，则进行删除操作
  if (qos)
  {
    ddsi_xqos_fini(qos);
    ddsrt_free(qos);
  }
}

/**
 * @brief 复制dds_qos_t结构体
 *
 * @param[out] dst 指向目标dds_qos_t结构体的指针
 * @param[in]  src 指向源dds_qos_t结构体的指针
 * @return         成功返回DDS_RETCODE_OK，失败返回DDS_RETCODE_BAD_PARAMETER
 */
dds_return_t dds_copy_qos(dds_qos_t *__restrict dst, const dds_qos_t *__restrict src)
{
  // 检查输入参数是否为空
  if (src == NULL || dst == NULL)
    return DDS_RETCODE_BAD_PARAMETER;

  // 复制源结构体到目标结构体
  ddsi_xqos_copy(dst, src);

  // 返回成功状态码
  return DDS_RETCODE_OK;
}

/**
 * @brief 合并两个dds_qos_t结构体
 *
 * @param[in,out] dst 指向目标dds_qos_t结构体的指针，将被合并
 * @param[in]     src 指向源dds_qos_t结构体的指针，用于合并
 */
void dds_merge_qos(dds_qos_t *__restrict dst, const dds_qos_t *__restrict src)
{
  // 如果源和目标结构体都非空，则进行合并操作
  if (src != NULL && dst != NULL)
    ddsi_xqos_mergein_missing(dst, src, ~(uint64_t)0);
}
/**
 * @brief 比较两个dds_qos_t结构体是否相等
 *
 * @param[in] a 第一个dds_qos_t结构体指针
 * @param[in] b 第二个dds_qos_t结构体指针
 * @return 如果两个结构体相等，则返回true，否则返回false
 */
bool dds_qos_equal(const dds_qos_t *__restrict a, const dds_qos_t *__restrict b)
{
  // 如果a和b都是空指针，认为它们相等
  if (a == NULL && b == NULL)
    return true;
  // 如果a和b中有一个为空指针，认为它们不相等
  else if (a == NULL || b == NULL)
    return false;
  // 使用ddsi_xqos_delta函数比较两个结构体的差异，如果差异为0，则认为它们相等
  else
    return ddsi_xqos_delta(a, b, ~(DDSI_QP_TYPE_INFORMATION)) == 0;
}

/**
 * @brief 设置用户数据
 *
 * @param[in,out] qos dds_qos_t结构体指针
 * @param[in] value 用户数据指针
 * @param[in] sz 用户数据大小
 */
void dds_qset_userdata(dds_qos_t *__restrict qos, const void *__restrict value, size_t sz)
{
  // 如果qos为空指针或者sz大于0且value为空指针，直接返回
  if (qos == NULL || (sz > 0 && value == NULL))
    return;
  // 将用户数据复制到qos->user_data中
  dds_qos_data_copy_in(&qos->user_data, value, sz, qos->present & DDSI_QP_USER_DATA);
  // 设置qos->present的DDSI_QP_USER_DATA位
  qos->present |= DDSI_QP_USER_DATA;
}

/**
 * @brief 设置主题数据
 *
 * @param[in,out] qos dds_qos_t结构体指针
 * @param[in] value 主题数据指针
 * @param[in] sz 主题数据大小
 */
void dds_qset_topicdata(dds_qos_t *__restrict qos, const void *__restrict value, size_t sz)
{
  // 如果qos为空指针或者sz大于0且value为空指针，直接返回
  if (qos == NULL || (sz > 0 && value == NULL))
    return;
  // 将主题数据复制到qos->topic_data中
  dds_qos_data_copy_in(&qos->topic_data, value, sz, qos->present & DDSI_QP_TOPIC_DATA);
  // 设置qos->present的DDSI_QP_TOPIC_DATA位
  qos->present |= DDSI_QP_TOPIC_DATA;
}

/**
 * @brief 设置组数据
 *
 * @param[in,out] qos dds_qos_t结构体指针
 * @param[in] value 组数据指针
 * @param[in] sz 组数据大小
 */
void dds_qset_groupdata(dds_qos_t *__restrict qos, const void *__restrict value, size_t sz)
{
  // 如果qos为空指针或者sz大于0且value为空指针，直接返回
  if (qos == NULL || (sz > 0 && value == NULL))
    return;
  // 将组数据复制到qos->group_data中
  dds_qos_data_copy_in(&qos->group_data, value, sz, qos->present & DDSI_QP_GROUP_DATA);
  // 设置qos->present的DDSI_QP_GROUP_DATA位
  qos->present |= DDSI_QP_GROUP_DATA;
}

/**
 * @brief 设置持久性
 *
 * @param[in,out] qos dds_qos_t结构体指针
 * @param[in] kind 持久性类型（dds_durability_kind_t枚举值）
 */
void dds_qset_durability(dds_qos_t *__restrict qos, dds_durability_kind_t kind)
{
  // 如果qos为空指针，直接返回
  if (qos == NULL)
    return;
  // 设置qos->durability.kind为传入的kind值
  qos->durability.kind = kind;
  // 设置qos->present的DDSI_QP_DURABILITY位
  qos->present |= DDSI_QP_DURABILITY;
}
/**
 * @brief 设置历史记录参数
 *
 * @param[in] qos 指向dds_qos_t结构体的指针，用于存储QoS设置
 * @param[in] kind 历史记录类型，表示数据保存方式
 * @param[in] depth 历史记录深度，表示保存的数据数量
 */
void dds_qset_history(dds_qos_t *__restrict qos, dds_history_kind_t kind, int32_t depth)
{
  if (qos == NULL) // 如果qos为空，则直接返回
    return;
  qos->history.kind = kind;        // 设置历史记录类型
  qos->history.depth = depth;      // 设置历史记录深度
  qos->present |= DDSI_QP_HISTORY; // 更新QoS设置标志位
}

/**
 * @brief 设置资源限制参数
 *
 * @param[in] qos 指向dds_qos_t结构体的指针，用于存储QoS设置
 * @param[in] max_samples 最大样本数
 * @param[in] max_instances 最大实例数
 * @param[in] max_samples_per_instance 每个实例的最大样本数
 */
void dds_qset_resource_limits(dds_qos_t *__restrict qos, int32_t max_samples, int32_t max_instances, int32_t max_samples_per_instance)
{
  if (qos == NULL) // 如果qos为空，则直接返回
    return;
  qos->resource_limits.max_samples = max_samples;                           // 设置最大样本数
  qos->resource_limits.max_instances = max_instances;                       // 设置最大实例数
  qos->resource_limits.max_samples_per_instance = max_samples_per_instance; // 设置每个实例的最大样本数
  qos->present |= DDSI_QP_RESOURCE_LIMITS;                                  // 更新QoS设置标志位
}

/**
 * @brief 设置表示参数
 *
 * @param[in] qos 指向dds_qos_t结构体的指针，用于存储QoS设置
 * @param[in] access_scope 访问范围类型
 * @param[in] coherent_access 是否进行一致性访问
 * @param[in] ordered_access 是否进行有序访问
 */
void dds_qset_presentation(dds_qos_t *__restrict qos, dds_presentation_access_scope_kind_t access_scope, bool coherent_access, bool ordered_access)
{
  if (qos == NULL) // 如果qos为空，则直接返回
    return;
  qos->presentation.access_scope = access_scope;       // 设置访问范围类型
  qos->presentation.coherent_access = coherent_access; // 设置是否进行一致性访问
  qos->presentation.ordered_access = ordered_access;   // 设置是否进行有序访问
  qos->present |= DDSI_QP_PRESENTATION;                // 更新QoS设置标志位
}

/**
 * @brief 设置生命周期参数
 *
 * @param[in] qos 指向dds_qos_t结构体的指针，用于存储QoS设置
 * @param[in] lifespan 生命周期时长
 */
void dds_qset_lifespan(dds_qos_t *__restrict qos, dds_duration_t lifespan)
{
  if (qos == NULL) // 如果qos为空，则直接返回
    return;
  qos->lifespan.duration = lifespan; // 设置生命周期时长
  qos->present |= DDSI_QP_LIFESPAN;  // 更新QoS设置标志位
}

/**
 * @brief 设置截止时间参数
 *
 * @param[in] qos 指向dds_qos_t结构体的指针，用于存储QoS设置
 * @param[in] deadline 截止时间时长
 */
void dds_qset_deadline(dds_qos_t *__restrict qos, dds_duration_t deadline)
{
  if (qos == NULL) // 如果qos为空，则直接返回
    return;
  qos->deadline.deadline = deadline; // 设置截止时间时长
  qos->present |= DDSI_QP_DEADLINE;  // 更新QoS设置标志位
}
/**
 * @brief 设置延迟预算参数
 *
 * @param[in] qos 指向dds_qos_t结构体的指针，用于存储QoS设置
 * @param[in] duration 延迟预算时长
 */
void dds_qset_latency_budget(dds_qos_t *__restrict qos, dds_duration_t duration)
{
  if (qos == NULL) // 如果qos为空，则直接返回
    return;
  qos->latency_budget.duration = duration; // 设置延迟预算时长
  qos->present |= DDSI_QP_LATENCY_BUDGET;  // 更新QoS设置标志位
}

/**
 * @brief 设置所有权参数
 *
 * @param[in] qos 指向dds_qos_t结构体的指针，用于存储QoS设置
 * @param[in] kind 所有权类型
 */
void dds_qset_ownership(dds_qos_t *__restrict qos, dds_ownership_kind_t kind)
{
  if (qos == NULL) // 如果qos为空，则直接返回
    return;
  qos->ownership.kind = kind;        // 设置所有权类型
  qos->present |= DDSI_QP_OWNERSHIP; // 更新QoS设置标志位
}

/**
 * @brief 设置所有权强度参数
 *
 * @param[in] qos 指向dds_qos_t结构体的指针，用于存储QoS设置
 * @param[in] value 所有权强度值
 */
void dds_qset_ownership_strength(dds_qos_t *__restrict qos, int32_t value)
{
  if (qos == NULL) // 如果qos为空，则直接返回
    return;
  qos->ownership_strength.value = value;      // 设置所有权强度值
  qos->present |= DDSI_QP_OWNERSHIP_STRENGTH; // 更新QoS设置标志位
}

/**
 * @brief 设置活跃度参数
 *
 * @param[in] qos 指向dds_qos_t结构体的指针，用于存储QoS设置
 * @param[in] kind 活跃度类型
 * @param[in] lease_duration 租约时长
 */
void dds_qset_liveliness(dds_qos_t *__restrict qos, dds_liveliness_kind_t kind, dds_duration_t lease_duration)
{
  if (qos == NULL) // 如果qos为空，则直接返回
    return;
  qos->liveliness.kind = kind;                     // 设置活跃度类型
  qos->liveliness.lease_duration = lease_duration; // 设置租约时长
  qos->present |= DDSI_QP_LIVELINESS;              // 更新QoS设置标志位
}

/**
 * @brief 设置基于时间的过滤器参数
 *
 * @param[in] qos 指向dds_qos_t结构体的指针，用于存储QoS设置
 * @param[in] minimum_separation 最小分离时间
 */
void dds_qset_time_based_filter(dds_qos_t *__restrict qos, dds_duration_t minimum_separation)
{
  if (qos == NULL) // 如果qos为空，则直接返回
    return;
  qos->time_based_filter.minimum_separation = minimum_separation; // 设置最小分离时间
  qos->present |= DDSI_QP_TIME_BASED_FILTER;                      // 更新QoS设置标志位
}
/**
 * @brief 设置分区参数
 *
 * @param[in] qos 限制性指针，指向dds_qos_t类型的结构体
 * @param[in] n 分区数量
 * @param[in] ps 限制性指针，指向字符串数组，表示分区名称列表
 */
void dds_qset_partition(dds_qos_t *__restrict qos, uint32_t n, const char **__restrict ps)
{
  // 检查输入参数是否有效
  if (qos == NULL || (n > 0 && ps == NULL))
    return;

  // 如果已经存在分区信息，则释放内存
  if (qos->present & DDSI_QP_PARTITION)
  {
    for (uint32_t i = 0; i < qos->partition.n; i++)
      ddsrt_free(qos->partition.strs[i]);
    ddsrt_free(qos->partition.strs);
  }

  // 设置新的分区数量
  qos->partition.n = n;

  // 根据分区数量设置分区字符串数组
  if (qos->partition.n == 0)
    qos->partition.strs = NULL;
  else
  {
    qos->partition.strs = ddsrt_malloc(n * sizeof(*qos->partition.strs));
    for (uint32_t i = 0; i < n; i++)
      qos->partition.strs[i] = ddsrt_strdup(ps[i]);
  }

  // 更新QoS标志位
  qos->present |= DDSI_QP_PARTITION;
}

/**
 * @brief 设置单个分区参数
 *
 * @param[in] qos 限制性指针，指向dds_qos_t类型的结构体
 * @param[in] name 限制性指针，指向字符串，表示分区名称
 */
void dds_qset_partition1(dds_qos_t *__restrict qos, const char *__restrict name)
{
  if (name == NULL)
    dds_qset_partition(qos, 0, NULL);
  else
    dds_qset_partition(qos, 1, (const char **)&name);
}

/**
 * @brief 设置可靠性参数
 *
 * @param[in] qos 限制性指针，指向dds_qos_t类型的结构体
 * @param[in] kind 可靠性类型，dds_reliability_kind_t枚举值
 * @param[in] max_blocking_time 最大阻塞时间，dds_duration_t类型
 */
void dds_qset_reliability(dds_qos_t *__restrict qos, dds_reliability_kind_t kind, dds_duration_t max_blocking_time)
{
  if (qos == NULL)
    return;

  // 设置可靠性类型和最大阻塞时间
  qos->reliability.kind = kind;
  qos->reliability.max_blocking_time = max_blocking_time;

  // 更新QoS标志位
  qos->present |= DDSI_QP_RELIABILITY;
}

/**
 * @brief 设置传输优先级参数
 *
 * @param[in] qos 限制性指针，指向dds_qos_t类型的结构体
 * @param[in] value 传输优先级值，int32_t类型
 */
void dds_qset_transport_priority(dds_qos_t *__restrict qos, int32_t value)
{
  if (qos == NULL)
    return;

  // 设置传输优先级值
  qos->transport_priority.value = value;

  // 更新QoS标志位
  qos->present |= DDSI_QP_TRANSPORT_PRIORITY;
}
/**
 * @brief 设置目标排序策略
 *
 * @param[in] qos 质量服务对象指针
 * @param[in] kind 目标排序类型
 */
void dds_qset_destination_order(dds_qos_t *__restrict qos, dds_destination_order_kind_t kind)
{
  // 检查 qos 是否为空，如果为空则直接返回
  if (qos == NULL)
    return;
  // 设置目标排序类型
  qos->destination_order.kind = kind;
  // 更新 qos 的 present 标志位
  qos->present |= DDSI_QP_DESTINATION_ORDER;
}

/**
 * @brief 设置写入者数据生命周期
 *
 * @param[in] qos 质量服务对象指针
 * @param[in] autodispose 是否自动处理未注册实例
 */
void dds_qset_writer_data_lifecycle(dds_qos_t *__restrict qos, bool autodispose)
{
  // 检查 qos 是否为空，如果为空则直接返回
  if (qos == NULL)
    return;
  // 设置是否自动处理未注册实例
  qos->writer_data_lifecycle.autodispose_unregistered_instances = autodispose;
  // 更新 qos 的 present 标志位
  qos->present |= DDSI_QP_ADLINK_WRITER_DATA_LIFECYCLE;
}

/**
 * @brief 设置读取者数据生命周期
 *
 * @param[in] qos 质量服务对象指针
 * @param[in] autopurge_nowriter_samples_delay 无写入者样本自动清除延迟
 * @param[in] autopurge_disposed_samples_delay 废弃样本自动清除延迟
 */
void dds_qset_reader_data_lifecycle(dds_qos_t *__restrict qos, dds_duration_t autopurge_nowriter_samples_delay, dds_duration_t autopurge_disposed_samples_delay)
{
  // 检查 qos 是否为空，如果为空则直接返回
  if (qos == NULL)
    return;
  // 设置无写入者样本自动清除延迟
  qos->reader_data_lifecycle.autopurge_nowriter_samples_delay = autopurge_nowriter_samples_delay;
  // 设置废弃样本自动清除延迟
  qos->reader_data_lifecycle.autopurge_disposed_samples_delay = autopurge_disposed_samples_delay;
  // 更新 qos 的 present 标志位
  qos->present |= DDSI_QP_ADLINK_READER_DATA_LIFECYCLE;
}

/**
 * @brief 设置持久化服务
 *
 * @param[in] qos 质量服务对象指针
 * @param[in] service_cleanup_delay 服务清理延迟
 * @param[in] history_kind 历史记录类型
 * @param[in] history_depth 历史记录深度
 * @param[in] max_samples 最大样本数
 * @param[in] max_instances 最大实例数
 * @param[in] max_samples_per_instance 每个实例的最大样本数
 */
void dds_qset_durability_service(dds_qos_t *__restrict qos, dds_duration_t service_cleanup_delay, dds_history_kind_t history_kind, int32_t history_depth, int32_t max_samples, int32_t max_instances, int32_t max_samples_per_instance)
{
  // 检查 qos 是否为空，如果为空则直接返回
  if (qos == NULL)
    return;
  // 设置服务清理延迟
  qos->durability_service.service_cleanup_delay = service_cleanup_delay;
  // 设置历史记录类型和深度
  qos->durability_service.history.kind = history_kind;
  qos->durability_service.history.depth = history_depth;
  // 设置资源限制
  qos->durability_service.resource_limits.max_samples = max_samples;
  qos->durability_service.resource_limits.max_instances = max_instances;
  qos->durability_service.resource_limits.max_samples_per_instance = max_samples_per_instance;
  // 更新 qos 的 present 标志位
  qos->present |= DDSI_QP_DURABILITY_SERVICE;
}

/**
 * @brief 设置忽略本地策略
 *
 * @param[in] qos 质量服务对象指针
 * @param[in] ignore 忽略本地类型
 */
void dds_qset_ignorelocal(dds_qos_t *__restrict qos, dds_ignorelocal_kind_t ignore)
{
  // 检查 qos 是否为空，如果为空则直接返回
  if (qos == NULL)
    return;
  // 设置忽略本地类型
  qos->ignorelocal.value = ignore;
  // 更新 qos 的 present 标志位
  qos->present |= DDSI_QP_CYCLONE_IGNORELOCAL;
}

/**
 * @brief 初始化质量属性列表
 *
 * @param[in] qos 质量服务对象指针
 */
static void dds_qprop_init(dds_qos_t *qos)
{
  // 检查 qos 的 present 标志位是否包含属性列表，如果不包含则进行初始化
  if (!(qos->present & DDSI_QP_PROPERTY_LIST))
  {
    // 初始化属性值和二进制属性值
    qos->property.value.n = 0;
    qos->property.value.props = NULL;
    qos->property.binary_value.n = 0;
    qos->property.binary_value.props = NULL;
    // 更新 qos 的 present 标志位
    qos->present |= DDSI_QP_PROPERTY_LIST;
  }
}
/**
 * @brief 获取指定属性的索引
 * @param[in] prop_type_ 属性类型
 * @param[in] prop_field_ 属性字段
 * @return 返回一个布尔值，表示是否成功获取索引
 */
#define DDS_QPROP_GET_INDEX(prop_type_, prop_field_)                                                 \
  static bool dds_q##prop_type_##_get_index(const dds_qos_t *qos, const char *name, uint32_t *index) \
  {                                                                                                  \
    /* 检查输入参数是否有效 */                                                             \
    if (qos == NULL || name == NULL || index == NULL || !(qos->present & DDSI_QP_PROPERTY_LIST))     \
      return false;                                                                                  \
    /* 遍历属性列表 */                                                                         \
    for (uint32_t i = 0; i < qos->property.prop_field_.n; i++)                                       \
    {                                                                                                \
      /* 如果找到匹配的属性名，则设置索引并返回 true */                           \
      if (strcmp(qos->property.prop_field_.props[i].name, name) == 0)                                \
      {                                                                                              \
        *index = i;                                                                                  \
        return true;                                                                                 \
      }                                                                                              \
    }                                                                                                \
    /* 如果没有找到匹配的属性名，则返回 false */                                     \
    return false;                                                                                    \
  }
DDS_QPROP_GET_INDEX(prop, value)
DDS_QPROP_GET_INDEX(bprop, binary_value)

/**
 * @brief 取消设置指定属性
 * @param[in] prop_type_ 属性类型
 * @param[in] prop_field_ 属性字段
 * @param[in] value_field_ 值字段
 */
#define DDS_QUNSET_PROP(prop_type_, prop_field_, value_field_)                                                                       \
  void dds_qunset_##prop_type_(dds_qos_t *__restrict qos, const char *name)                                                          \
  {                                                                                                                                  \
    uint32_t i;                                                                                                                      \
    /* 检查输入参数是否有效 */                                                                                             \
    if (qos == NULL || !(qos->present & DDSI_QP_PROPERTY_LIST) || qos->property.prop_field_.n == 0 || name == NULL)                  \
      return;                                                                                                                        \
    /* 如果成功获取属性索引 */                                                                                             \
    if (dds_q##prop_type_##_get_index(qos, name, &i))                                                                                \
    {                                                                                                                                \
      /* 释放属性名和值字段的内存 */                                                                                     \
      dds_free(qos->property.prop_field_.props[i].name);                                                                             \
      dds_free(qos->property.prop_field_.props[i].value_field_);                                                                     \
      /* 如果属性列表中有多个元素 */                                                                                     \
      if (qos->property.prop_field_.n > 1)                                                                                           \
      {                                                                                                                              \
        /* 如果当前索引不是最后一个元素，将后面的元素向前移动 */                                            \
        if (i < (qos->property.prop_field_.n - 1))                                                                                   \
          memmove(qos->property.prop_field_.props + i, qos->property.prop_field_.props + i + 1,                                      \
                  (qos->property.prop_field_.n - i - 1) * sizeof(*qos->property.prop_field_.props));                                 \
        /* 重新分配属性列表内存 */                                                                                         \
        qos->property.prop_field_.props = dds_realloc(qos->property.prop_field_.props,                                               \
                                                      (qos->property.prop_field_.n - 1) * sizeof(*qos->property.prop_field_.props)); \
      }                                                                                                                              \
      /* 如果属性列表中只有一个元素 */                                                                                  \
      else                                                                                                                           \
      {                                                                                                                              \
        /* 释放属性列表内存并设置为 NULL */                                                                              \
        dds_free(qos->property.prop_field_.props);                                                                                   \
        qos->property.prop_field_.props = NULL;                                                                                      \
      }                                                                                                                              \
      /* 更新属性列表的元素数量 */                                                                                        \
      qos->property.prop_field_.n--;                                                                                                 \
    }                                                                                                                                \
  }
DDS_QUNSET_PROP(prop, value, value)
DDS_QUNSET_PROP(bprop, binary_value, value.value)
/**
 * @brief 设置QoS属性值
 *
 * @param[in] qos QoS对象指针
 * @param[in] name 属性名
 * @param[in] value 属性值
 */
void dds_qset_prop(dds_qos_t *__restrict qos, const char *name, const char *value)
{
  uint32_t i;
  // 检查输入参数是否为空
  if (qos == NULL || name == NULL || value == NULL)
    return;

  // 初始化QoS属性
  dds_qprop_init(qos);
  // 获取属性索引
  if (dds_qprop_get_index(qos, name, &i))
  {
    // 断言检查
    assert(&qos->property.value.props[i] != NULL); /* for Clang static analyzer */
    // 释放旧的属性值内存
    dds_free(qos->property.value.props[i].value);
    // 分配新的属性值内存并复制
    qos->property.value.props[i].value = dds_string_dup(value);
  }
  else
  {
    // 重新分配属性数组内存
    qos->property.value.props = dds_realloc(qos->property.value.props,
                                            (qos->property.value.n + 1) * sizeof(*qos->property.value.props));
    // 设置新属性
    qos->property.value.props[qos->property.value.n].propagate = 0;
    qos->property.value.props[qos->property.value.n].name = dds_string_dup(name);
    qos->property.value.props[qos->property.value.n].value = dds_string_dup(value);
    // 更新属性数量
    qos->property.value.n++;
  }
}

/**
 * @brief 设置QoS二进制属性值
 *
 * @param[in] qos QoS对象指针
 * @param[in] name 属性名
 * @param[in] value 二进制属性值
 * @param[in] sz 二进制属性值大小
 */
void dds_qset_bprop(dds_qos_t *__restrict qos, const char *name, const void *value, const size_t sz)
{
  uint32_t i;
  // 检查输入参数是否为空
  if (qos == NULL || name == NULL || (value == NULL && sz > 0))
    return;

  // 初始化QoS属性
  dds_qprop_init(qos);
  // 获取属性索引
  if (dds_qbprop_get_index(qos, name, &i))
  {
    // 断言检查
    assert(&qos->property.binary_value.props[i].value != NULL); /* for Clang static analyzer */
    // 复制二进制属性值
    dds_qos_data_copy_in(&qos->property.binary_value.props[i].value, value, sz, true);
  }
  else
  {
    // 重新分配属性数组内存
    qos->property.binary_value.props = dds_realloc(qos->property.binary_value.props,
                                                   (qos->property.binary_value.n + 1) * sizeof(*qos->property.binary_value.props));
    // 设置新属性
    qos->property.binary_value.props[qos->property.binary_value.n].propagate = 0;
    qos->property.binary_value.props[qos->property.binary_value.n].name = dds_string_dup(name);
    // 复制二进制属性值
    dds_qos_data_copy_in(&qos->property.binary_value.props[qos->property.binary_value.n].value, value, sz, false);
    // 更新属性数量
    qos->property.binary_value.n++;
  }
}

/**
 * @brief 设置QoS实体名称
 *
 * @param[in] qos QoS对象指针
 * @param[in] name 实体名称
 */
void dds_qset_entity_name(dds_qos_t *__restrict qos, const char *name)
{
  // 检查输入参数是否为空
  if (qos == NULL || name == NULL)
    return;
  // 分配新的实体名称内存并复制
  qos->entity_name = dds_string_dup(name);
  // 更新QoS标志位
  qos->present |= DDSI_QP_ENTITY_NAME;
}

/**
 * @brief 设置QoS类型一致性
 *
 * @param[in] qos QoS对象指针
 * @param[in] kind 类型一致性种类
 * @param[in] ignore_sequence_bounds 是否忽略序列边界
 * @param[in] ignore_string_bounds 是否忽略字符串边界
 * @param[in] ignore_member_names 是否忽略成员名称
 * @param[in] prevent_type_widening 是否阻止类型扩展
 * @param[in] force_type_validation 是否强制类型验证
 */
void dds_qset_type_consistency(dds_qos_t *__restrict qos, dds_type_consistency_kind_t kind,
                               bool ignore_sequence_bounds, bool ignore_string_bounds, bool ignore_member_names, bool prevent_type_widening, bool force_type_validation)
{
  // 检查输入参数是否为空
  if (qos == NULL)
    return;
  // 设置类型一致性参数
  qos->type_consistency.kind = kind;
  qos->type_consistency.ignore_sequence_bounds = ignore_sequence_bounds;
  qos->type_consistency.ignore_string_bounds = ignore_string_bounds;
  qos->type_consistency.ignore_member_names = ignore_member_names;
  qos->type_consistency.prevent_type_widening = prevent_type_widening;
  qos->type_consistency.force_type_validation = force_type_validation;
  // 更新QoS标志位
  qos->present |= DDSI_QP_TYPE_CONSISTENCY_ENFORCEMENT;
}

/**
 * @brief 设置数据表示的质量参数
 *
 * @param[in] qos 质量参数结构体指针
 * @param[in] n 数据表示标识符数组的长度
 * @param[in] values 数据表示标识符数组
 */
void dds_qset_data_representation(dds_qos_t *__restrict qos, uint32_t n, const dds_data_representation_id_t *values)
{
  // 检查输入参数是否有效
  if (qos == NULL || (n && !values))
    return;

  // 如果已经设置过数据表示，释放之前分配的内存
  if ((qos->present & DDSI_QP_DATA_REPRESENTATION) && qos->data_representation.value.ids != NULL)
    ddsrt_free(qos->data_representation.value.ids);

  // 初始化数据表示值
  qos->data_representation.value.n = 0;
  qos->data_representation.value.ids = NULL;

  /* 去除提供的数据表示标识符列表中的重复项。重新分配内存的方法效率较低，
     但由于列表通常只包含少量值，因此不会造成太大问题 */
  for (uint32_t x = 0; x < n; x++)
  {
    bool duplicate = false;
    // 检查当前值是否已存在于列表中
    for (uint32_t c = 0; !duplicate && c < x; c++)
      if (qos->data_representation.value.ids[c] == values[x])
        duplicate = true;

    // 如果当前值不是重复项，则将其添加到列表中
    if (!duplicate)
    {
      qos->data_representation.value.n++;
      qos->data_representation.value.ids = dds_realloc(qos->data_representation.value.ids, qos->data_representation.value.n * sizeof(*qos->data_representation.value.ids));
      qos->data_representation.value.ids[qos->data_representation.value.n - 1] = values[x];
    }
  }

  // 更新质量参数的表示标志
  qos->present |= DDSI_QP_DATA_REPRESENTATION;
}

/**
 * @brief 获取用户数据质量参数
 *
 * @param[in] qos 质量参数结构体指针
 * @param[out] value 用户数据值指针
 * @param[out] sz 用户数据大小
 * @return 成功返回 true，失败返回 false
 */
bool dds_qget_userdata(const dds_qos_t *__restrict qos, void **value, size_t *sz)
{
  if (qos == NULL || !(qos->present & DDSI_QP_USER_DATA))
    return false;
  return dds_qos_data_copy_out(&qos->user_data, value, sz);
}

/**
 * @brief 获取主题数据质量参数
 *
 * @param[in] qos 质量参数结构体指针
 * @param[out] value 主题数据值指针
 * @param[out] sz 主题数据大小
 * @return 成功返回 true，失败返回 false
 */
bool dds_qget_topicdata(const dds_qos_t *__restrict qos, void **value, size_t *sz)
{
  if (qos == NULL || !(qos->present & DDSI_QP_TOPIC_DATA))
    return false;
  return dds_qos_data_copy_out(&qos->topic_data, value, sz);
}

/**
 * @brief 获取组数据质量参数
 *
 * @param[in] qos 质量参数结构体指针
 * @param[out] value 组数据值指针
 * @param[out] sz 组数据大小
 * @return 成功返回 true，失败返回 false
 */
bool dds_qget_groupdata(const dds_qos_t *__restrict qos, void **value, size_t *sz)
{
  if (qos == NULL || !(qos->present & DDSI_QP_GROUP_DATA))
    return false;
  return dds_qos_data_copy_out(&qos->group_data, value, sz);
}

/**
 * @brief 获取durability属性
 *
 * @param[in] qos 指向dds_qos_t结构体的指针
 * @param[out] kind 返回durability种类的指针
 * @return 如果成功获取durability属性，则返回true，否则返回false
 */
bool dds_qget_durability(const dds_qos_t *__restrict qos, dds_durability_kind_t *kind)
{
  // 检查qos是否为空或者durability属性是否存在
  if (qos == NULL || !(qos->present & DDSI_QP_DURABILITY))
    return false;
  // 如果kind不为空，将durability种类赋值给kind
  if (kind)
    *kind = qos->durability.kind;
  return true;
}

/**
 * @brief 获取history属性
 *
 * @param[in] qos 指向dds_qos_t结构体的指针
 * @param[out] kind 返回history种类的指针
 * @param[out] depth 返回history深度的指针
 * @return 如果成功获取history属性，则返回true，否则返回false
 */
bool dds_qget_history(const dds_qos_t *__restrict qos, dds_history_kind_t *kind, int32_t *depth)
{
  // 检查qos是否为空或者history属性是否存在
  if (qos == NULL || !(qos->present & DDSI_QP_HISTORY))
    return false;
  // 如果kind不为空，将history种类赋值给kind
  if (kind)
    *kind = qos->history.kind;
  // 如果depth不为空，将history深度赋值给depth
  if (depth)
    *depth = qos->history.depth;
  return true;
}

/**
 * @brief 获取resource_limits属性
 *
 * @param[in] qos 指向dds_qos_t结构体的指针
 * @param[out] max_samples 返回最大样本数的指针
 * @param[out] max_instances 返回最大实例数的指针
 * @param[out] max_samples_per_instance 返回每个实例的最大样本数的指针
 * @return 如果成功获取resource_limits属性，则返回true，否则返回false
 */
bool dds_qget_resource_limits(const dds_qos_t *__restrict qos, int32_t *max_samples, int32_t *max_instances, int32_t *max_samples_per_instance)
{
  // 检查qos是否为空或者resource_limits属性是否存在
  if (qos == NULL || !(qos->present & DDSI_QP_RESOURCE_LIMITS))
    return false;
  // 如果max_samples不为空，将最大样本数赋值给max_samples
  if (max_samples)
    *max_samples = qos->resource_limits.max_samples;
  // 如果max_instances不为空，将最大实例数赋值给max_instances
  if (max_instances)
    *max_instances = qos->resource_limits.max_instances;
  // 如果max_samples_per_instance不为空，将每个实例的最大样本数赋值给max_samples_per_instance
  if (max_samples_per_instance)
    *max_samples_per_instance = qos->resource_limits.max_samples_per_instance;
  return true;
}

/**
 * @brief 获取presentation属性
 *
 * @param[in] qos 指向dds_qos_t结构体的指针
 * @param[out] access_scope 返回访问范围种类的指针
 * @param[out] coherent_access 返回一致访问标志的指针
 * @param[out] ordered_access 返回有序访问标志的指针
 * @return 如果成功获取presentation属性，则返回true，否则返回false
 */
bool dds_qget_presentation(const dds_qos_t *__restrict qos, dds_presentation_access_scope_kind_t *access_scope, bool *coherent_access, bool *ordered_access)
{
  // 检查qos是否为空或者presentation属性是否存在
  if (qos == NULL || !(qos->present & DDSI_QP_PRESENTATION))
    return false;
  // 如果access_scope不为空，将访问范围种类赋值给access_scope
  if (access_scope)
    *access_scope = qos->presentation.access_scope;
  // 如果coherent_access不为空，将一致访问标志赋值给coherent_access
  if (coherent_access)
    *coherent_access = qos->presentation.coherent_access;
  // 如果ordered_access不为空，将有序访问标志赋值给ordered_access
  if (ordered_access)
    *ordered_access = qos->presentation.ordered_access;
  return true;
}

/**
 * @brief 获取lifespan属性
 *
 * @param[in] qos 指向dds_qos_t结构体的指针
 * @param[out] lifespan 返回生命周期时长的指针
 * @return 如果成功获取lifespan属性，则返回true，否则返回false
 */
bool dds_qget_lifespan(const dds_qos_t *__restrict qos, dds_duration_t *lifespan)
{
  // 检查qos是否为空或者lifespan属性是否存在
  if (qos == NULL || !(qos->present & DDSI_QP_LIFESPAN))
    return false;
  // 如果lifespan不为空，将生命周期时长赋值给lifespan
  if (lifespan)
    *lifespan = qos->lifespan.duration;
  return true;
}

/**
 * @brief 获取QoS的deadline参数
 *
 * @param qos 输入的QoS结构体指针
 * @param deadline 输出的deadline值指针
 * @return bool 如果成功获取到deadline参数，则返回true，否则返回false
 */
bool dds_qget_deadline(const dds_qos_t *__restrict qos, dds_duration_t *deadline)
{
  // 检查输入参数是否有效
  if (qos == NULL || !(qos->present & DDSI_QP_DEADLINE))
    return false;
  // 设置输出参数
  if (deadline)
    *deadline = qos->deadline.deadline;
  return true;
}

/**
 * @brief 获取QoS的latency_budget参数
 *
 * @param qos 输入的QoS结构体指针
 * @param duration 输出的duration值指针
 * @return bool 如果成功获取到latency_budget参数，则返回true，否则返回false
 */
bool dds_qget_latency_budget(const dds_qos_t *__restrict qos, dds_duration_t *duration)
{
  // 检查输入参数是否有效
  if (qos == NULL || !(qos->present & DDSI_QP_LATENCY_BUDGET))
    return false;
  // 设置输出参数
  if (duration)
    *duration = qos->latency_budget.duration;
  return true;
}

/**
 * @brief 获取QoS的ownership参数
 *
 * @param qos 输入的QoS结构体指针
 * @param kind 输出的ownership_kind值指针
 * @return bool 如果成功获取到ownership参数，则返回true，否则返回false
 */
bool dds_qget_ownership(const dds_qos_t *__restrict qos, dds_ownership_kind_t *kind)
{
  // 检查输入参数是否有效
  if (qos == NULL || !(qos->present & DDSI_QP_OWNERSHIP))
    return false;
  // 设置输出参数
  if (kind)
    *kind = qos->ownership.kind;
  return true;
}

/**
 * @brief 获取QoS的ownership_strength参数
 *
 * @param qos 输入的QoS结构体指针
 * @param value 输出的ownership_strength值指针
 * @return bool 如果成功获取到ownership_strength参数，则返回true，否则返回false
 */
bool dds_qget_ownership_strength(const dds_qos_t *__restrict qos, int32_t *value)
{
  // 检查输入参数是否有效
  if (qos == NULL || !(qos->present & DDSI_QP_OWNERSHIP_STRENGTH))
    return false;
  // 设置输出参数
  if (value)
    *value = qos->ownership_strength.value;
  return true;
}

/**
 * @brief 获取QoS的liveliness参数
 *
 * @param qos 输入的QoS结构体指针
 * @param kind 输出的liveliness_kind值指针
 * @param lease_duration 输出的lease_duration值指针
 * @return bool 如果成功获取到liveliness参数，则返回true，否则返回false
 */
bool dds_qget_liveliness(const dds_qos_t *__restrict qos, dds_liveliness_kind_t *kind, dds_duration_t *lease_duration)
{
  // 检查输入参数是否有效
  if (qos == NULL || !(qos->present & DDSI_QP_LIVELINESS))
    return false;
  // 设置输出参数
  if (kind)
    *kind = qos->liveliness.kind;
  if (lease_duration)
    *lease_duration = qos->liveliness.lease_duration;
  return true;
}

/**
 * @brief 获取基于时间的过滤器设置
 *
 * @param[in] qos 指向dds_qos_t结构体的指针
 * @param[out] minimum_separation 时间间隔的最小值
 * @return 如果成功获取，返回true，否则返回false
 */
bool dds_qget_time_based_filter(const dds_qos_t *__restrict qos, dds_duration_t *minimum_separation)
{
  // 检查qos是否为空或者不包含基于时间的过滤器设置
  if (qos == NULL || !(qos->present & DDSI_QP_TIME_BASED_FILTER))
    return false;
  // 如果minimum_separation不为空，则将qos中的最小时间间隔赋值给它
  if (minimum_separation)
    *minimum_separation = qos->time_based_filter.minimum_separation;
  return true;
}

/**
 * @brief 获取分区设置
 *
 * @param[in] qos 指向dds_qos_t结构体的指针
 * @param[out] n 分区数量
 * @param[out] ps 分区字符串数组
 * @return 如果成功获取，返回true，否则返回false
 */
bool dds_qget_partition(const dds_qos_t *__restrict qos, uint32_t *n, char ***ps)
{
  // 检查qos是否为空或者不包含分区设置
  if (qos == NULL || !(qos->present & DDSI_QP_PARTITION))
    return false;
  // 如果n为空且ps不为空，返回false
  if (n == NULL && ps != NULL)
    return false;
  // 如果n不为空，则将qos中的分区数量赋值给它
  if (n)
    *n = qos->partition.n;
  // 如果ps不为空
  if (ps)
  {
    // 如果分区数量为0，将ps设置为NULL
    if (qos->partition.n == 0)
      *ps = NULL;
    else
    {
      // 分配内存并复制分区字符串数组
      *ps = dds_alloc(sizeof(char *) * qos->partition.n);
      for (uint32_t i = 0; i < qos->partition.n; i++)
        (*ps)[i] = dds_string_dup(qos->partition.strs[i]);
    }
  }
  return true;
}

/**
 * @brief 获取可靠性设置
 *
 * @param[in] qos 指向dds_qos_t结构体的指针
 * @param[out] kind 可靠性类型
 * @param[out] max_blocking_time 最大阻塞时间
 * @return 如果成功获取，返回true，否则返回false
 */
bool dds_qget_reliability(const dds_qos_t *__restrict qos, dds_reliability_kind_t *kind, dds_duration_t *max_blocking_time)
{
  // 检查qos是否为空或者不包含可靠性设置
  if (qos == NULL || !(qos->present & DDSI_QP_RELIABILITY))
    return false;
  // 如果kind不为空，则将qos中的可靠性类型赋值给它
  if (kind)
    *kind = qos->reliability.kind;
  // 如果max_blocking_time不为空，则将qos中的最大阻塞时间赋值给它
  if (max_blocking_time)
    *max_blocking_time = qos->reliability.max_blocking_time;
  return true;
}

/**
 * @brief 获取传输优先级设置
 *
 * @param[in] qos 指向dds_qos_t结构体的指针
 * @param[out] value 传输优先级值
 * @return 如果成功获取，返回true，否则返回false
 */
bool dds_qget_transport_priority(const dds_qos_t *__restrict qos, int32_t *value)
{
  // 检查qos是否为空或者不包含传输优先级设置
  if (qos == NULL || !(qos->present & DDSI_QP_TRANSPORT_PRIORITY))
    return false;
  // 如果value不为空，则将qos中的传输优先级值赋值给它
  if (value)
    *value = qos->transport_priority.value;
  return true;
}

/**
 * @brief 获取目标顺序设置
 *
 * @param[in] qos 指向dds_qos_t结构体的指针
 * @param[out] kind 目标顺序类型
 * @return 如果成功获取，返回true，否则返回false
 */
bool dds_qget_destination_order(const dds_qos_t *__restrict qos, dds_destination_order_kind_t *kind)
{
  // 检查qos是否为空或者不包含目标顺序设置
  if (qos == NULL || !(qos->present & DDSI_QP_DESTINATION_ORDER))
    return false;
  // 如果kind不为空，则将qos中的目标顺序类型赋值给它
  if (kind)
    *kind = qos->destination_order.kind;
  return true;
}

/**
 * @brief 获取写入者数据生命周期设置
 *
 * @param[in] qos 指向dds_qos_t结构体的指针
 * @param[out] autodispose 是否自动处理未注册实例
 * @return 如果成功获取，返回true，否则返回false
 */
bool dds_qget_writer_data_lifecycle(const dds_qos_t *__restrict qos, bool *autodispose)
{
  // 检查qos是否为空或者不包含写入者数据生命周期设置
  if (qos == NULL || !(qos->present & DDSI_QP_ADLINK_WRITER_DATA_LIFECYCLE))
    return false;
  // 如果autodispose不为空，则将qos中的自动处理未注册实例标志赋值给它
  if (autodispose)
    *autodispose = qos->writer_data_lifecycle.autodispose_unregistered_instances;
  return true;
}

/**
 * @brief 获取读取者数据生命周期设置
 *
 * @param[in] qos 指向dds_qos_t结构体的指针
 * @param[out] autopurge_nowriter_samples_delay 无写入者样本自动清除延迟
 * @param[out] autopurge_disposed_samples_delay 废弃样本自动清除延迟
 * @return 如果成功获取，返回true，否则返回false
 */
bool dds_qget_reader_data_lifecycle(const dds_qos_t *__restrict qos, dds_duration_t *autopurge_nowriter_samples_delay, dds_duration_t *autopurge_disposed_samples_delay)
{
  // 检查qos是否为空或者不包含读取者数据生命周期设置
  if (qos == NULL || !(qos->present & DDSI_QP_ADLINK_READER_DATA_LIFECYCLE))
    return false;
  // 如果autopurge_nowriter_samples_delay不为空，则将qos中的无写入者样本自动清除延迟赋值给它
  if (autopurge_nowriter_samples_delay)
    *autopurge_nowriter_samples_delay = qos->reader_data_lifecycle.autopurge_nowriter_samples_delay;
  // 如果autopurge_disposed_samples_delay不为空，则将qos中的废弃样本自动清除延迟赋值给它
  if (autopurge_disposed_samples_delay)
    *autopurge_disposed_samples_delay = qos->reader_data_lifecycle.autopurge_disposed_samples_delay;
  return true;
}

/**
 * @brief 获取Durability Service的QoS设置
 *
 * @param[in] qos 输入的QoS结构体指针
 * @param[out] service_cleanup_delay 输出的清理延迟时间
 * @param[out] history_kind 输出的历史记录类型
 * @param[out] history_depth 输出的历史记录深度
 * @param[out] max_samples 输出的最大样本数
 * @param[out] max_instances 输出的最大实例数
 * @param[out] max_samples_per_instance 输出的每个实例的最大样本数
 * @return 如果成功获取到设置，则返回true，否则返回false
 */
bool dds_qget_durability_service(const dds_qos_t *__restrict qos, dds_duration_t *service_cleanup_delay, dds_history_kind_t *history_kind, int32_t *history_depth, int32_t *max_samples, int32_t *max_instances, int32_t *max_samples_per_instance)
{
  // 检查输入参数是否有效
  if (qos == NULL || !(qos->present & DDSI_QP_DURABILITY_SERVICE))
    return false;

  // 获取清理延迟时间
  if (service_cleanup_delay)
    *service_cleanup_delay = qos->durability_service.service_cleanup_delay;

  // 获取历史记录类型
  if (history_kind)
    *history_kind = qos->durability_service.history.kind;

  // 获取历史记录深度
  if (history_depth)
    *history_depth = qos->durability_service.history.depth;

  // 获取最大样本数
  if (max_samples)
    *max_samples = qos->durability_service.resource_limits.max_samples;

  // 获取最大实例数
  if (max_instances)
    *max_instances = qos->durability_service.resource_limits.max_instances;

  // 获取每个实例的最大样本数
  if (max_samples_per_instance)
    *max_samples_per_instance = qos->durability_service.resource_limits.max_samples_per_instance;

  return true;
}

/**
 * @brief 获取Ignore Local的QoS设置
 *
 * @param[in] qos 输入的QoS结构体指针
 * @param[out] ignore 输出的忽略本地设置
 * @return 如果成功获取到设置，则返回true，否则返回false
 */
bool dds_qget_ignorelocal(const dds_qos_t *__restrict qos, dds_ignorelocal_kind_t *ignore)
{
  // 检查输入参数是否有效
  if (qos == NULL || !(qos->present & DDSI_QP_CYCLONE_IGNORELOCAL))
    return false;

  // 获取忽略本地设置
  if (ignore)
    *ignore = qos->ignorelocal.value;

  return true;
}

// 定义宏函数，用于生成获取属性名称的函数
#define DDS_QGET_PROPNAMES(prop_type_, prop_field_)                                              \
  /**                                                                                            \
   * @brief 获取指定类型的属性名称列表                                              \
   *                                                                                             \
   * @param[in] qos 输入的QoS结构体指针                                                  \
   * @param[out] n 输出的属性名称数量                                                   \
   * @param[out] names 输出的属性名称字符串数组                                      \
   * @return 如果成功获取到属性名称列表，则返回true，否则返回false         \
   */                                                                                            \
  bool dds_qget_##prop_type_##names(const dds_qos_t *__restrict qos, uint32_t *n, char ***names) \
  {                                                                                              \
    bool props;                                                                                  \
    // 检查输入参数是否有效                                                                        \
    if (qos == NULL || (n == NULL && names == NULL))                                             \
      return false;                                                                              \
                                                                                                 \
    // 判断属性列表是否存在和有效                                                                  \
    props = (qos->present & DDSI_QP_PROPERTY_LIST) && qos->property.prop_field_.n > 0;           \
                                                                                                 \
    // 获取属性名称数量                                                                            \
    if (n != NULL)                                                                               \
      *n = props ? qos->property.prop_field_.n : 0;                                              \
                                                                                                 \
    // 获取属性名称字符串数组                                                                      \
    if (names != NULL)                                                                           \
    {                                                                                            \
      if (!props)                                                                                \
        *names = NULL;                                                                           \
      else                                                                                       \
      {                                                                                          \
        *names = dds_alloc(sizeof(char *) * qos->property.prop_field_.n);                        \
        for (uint32_t i = 0; i < qos->property.prop_field_.n; i++)                               \
          (*names)[i] = dds_string_dup(qos->property.prop_field_.props[i].name);                 \
      }                                                                                          \
    }                                                                                            \
    return props;                                                                                \
  }

// 使用宏函数生成获取属性名称的函数
DDS_QGET_PROPNAMES(prop, value)
DDS_QGET_PROPNAMES(bprop, binary_value)

/**
 * @brief 从给定的QoS中获取指定名称的属性值。
 *
 * @param[in] qos 指向dds_qos_t结构体的指针，用于存储QoS设置。
 * @param[in] name 要查询的属性名称。
 * @param[out] value 如果找到属性，则返回该属性的值；否则返回NULL。
 * @return 如果找到指定名称的属性，则返回true；否则返回false。
 */
bool dds_qget_prop(const dds_qos_t *__restrict qos, const char *name, char **value)
{
  uint32_t i;
  bool found;

  // 检查输入参数是否有效
  if (qos == NULL || name == NULL)
    return false;

  // 查找指定名称的属性，并获取其索引
  found = dds_qprop_get_index(qos, name, &i);
  // 如果找到属性且value非空，则将属性值复制到value中
  if (value != NULL)
    *value = found ? dds_string_dup(qos->property.value.props[i].value) : NULL;
  return found;
}

/**
 * @brief 从给定的QoS中获取指定名称的二进制属性值。
 *
 * @param[in] qos 指向dds_qos_t结构体的指针，用于存储QoS设置。
 * @param[in] name 要查询的属性名称。
 * @param[out] value 如果找到属性，则返回该属性的值；否则返回NULL。
 * @param[out] sz 返回找到的二进制属性值的大小。
 * @return 如果找到指定名称的属性，则返回true；否则返回false。
 */
bool dds_qget_bprop(const dds_qos_t *__restrict qos, const char *name, void **value, size_t *sz)
{
  uint32_t i;
  bool found;

  // 检查输入参数是否有效
  if (qos == NULL || name == NULL || (sz == NULL && value != NULL))
    return false;

  // 查找指定名称的二进制属性，并获取其索引
  found = dds_qbprop_get_index(qos, name, &i);
  if (found)
  {
    // 如果找到属性且value或sz非空，则将属性值复制到value中，大小复制到sz中
    if (value != NULL || sz != NULL)
      dds_qos_data_copy_out(&qos->property.binary_value.props[i].value, value, sz);
  }
  else
  {
    // 如果未找到属性，则设置value为NULL，sz为0
    if (value != NULL)
      *value = NULL;
    if (sz != NULL)
      *sz = 0;
  }
  return found;
}

/**
 * @brief 从给定的QoS中获取类型一致性设置。
 *
 * @param[in] qos 指向dds_qos_t结构体的指针，用于存储QoS设置。
 * @param[out] kind 返回类型一致性的种类。
 * @param[out] ignore_sequence_bounds 返回是否忽略序列边界。
 * @param[out] ignore_string_bounds 返回是否忽略字符串边界。
 * @param[out] ignore_member_names 返回是否忽略成员名称。
 * @param[out] prevent_type_widening 返回是否阻止类型扩展。
 * @param[out] force_type_validation 返回是否强制类型验证。
 * @return 如果找到类型一致性设置，则返回true；否则返回false。
 */
bool dds_qget_type_consistency(const dds_qos_t *__restrict qos, dds_type_consistency_kind_t *kind,
                               bool *ignore_sequence_bounds, bool *ignore_string_bounds, bool *ignore_member_names, bool *prevent_type_widening, bool *force_type_validation)
{
  // 检查输入参数是否有效
  if (qos == NULL || !(qos->present & DDSI_QP_TYPE_CONSISTENCY_ENFORCEMENT))
    return false;
  // 获取类型一致性设置的各个值
  if (kind)
    *kind = qos->type_consistency.kind;
  if (ignore_sequence_bounds)
    *ignore_sequence_bounds = qos->type_consistency.ignore_sequence_bounds;
  if (ignore_string_bounds)
    *ignore_string_bounds = qos->type_consistency.ignore_string_bounds;
  if (ignore_member_names)
    *ignore_member_names = qos->type_consistency.ignore_member_names;
  if (prevent_type_widening)
    *prevent_type_widening = qos->type_consistency.prevent_type_widening;
  if (force_type_validation)
    *force_type_validation = qos->type_consistency.force_type_validation;
  return true;
}

/**
 * @brief 从给定的QoS中获取数据表示设置。
 *
 * @param[in] qos 指向dds_qos_t结构体的指针，用于存储QoS设置。
 * @param[out] n 返回数据表示ID的数量。
 * @param[out] values 返回数据表示ID数组的指针。
 * @return 如果找到数据表示设置，则返回true；否则返回false。
 */
bool dds_qget_data_representation(const dds_qos_t *__restrict qos, uint32_t *n, dds_data_representation_id_t **values)
{
  // 检查输入参数是否有效
  if (qos == NULL || !(qos->present & DDSI_QP_DATA_REPRESENTATION))
    return false;
  if (n == NULL)
    return false;
  // 获取数据表示ID的数量
  if (qos->data_representation.value.n > 0)
    assert(qos->data_representation.value.ids != NULL);
  *n = qos->data_representation.value.n;
  // 如果values非空，则复制数据表示ID数组到values中
  if (values != NULL)
  {
    if (qos->data_representation.value.n > 0)
    {
      size_t sz = qos->data_representation.value.n * sizeof(*qos->data_representation.value.ids);
      *values = dds_alloc(sz);
      memcpy(*values, qos->data_representation.value.ids, sz);
    }
    else
    {
      *values = NULL;
    }
  }
  return true;
}

/**
 * @brief 确保QoS中的数据表示设置有效。
 *
 * @param[in,out] qos 指向dds_qos_t结构体的指针，用于存储QoS设置。
 * @param[in] allowed_data_representations 允许的数据表示标志。
 * @param[in] topicqos 是否为主题QoS。
 * @return 如果数据表示设置有效，则返回DDS_RETCODE_OK；否则返回相应的错误代码。
 */
dds_return_t dds_ensure_valid_data_representation(dds_qos_t *qos, uint32_t allowed_data_representations, bool topicqos)
{
  const bool allow1 = allowed_data_representations & DDS_DATA_REPRESENTATION_FLAG_XCDR1,
             allow2 = allowed_data_representations & DDS_DATA_REPRESENTATION_FLAG_XCDR2;

  // 如果QoS中存在数据表示设置且数量大于0
  if ((qos->present & DDSI_QP_DATA_REPRESENTATION) && qos->data_representation.value.n > 0)
  {
    assert(qos->data_representation.value.ids != NULL);
    // 遍历数据表示ID数组
    for (uint32_t n = 0; n < qos->data_representation.value.n; n++)
    {
      switch (qos->data_representation.value.ids[n])
      {
      case DDS_DATA_REPRESENTATION_XML:
        return DDS_RETCODE_UNSUPPORTED;
      case DDS_DATA_REPRESENTATION_XCDR1:
        if (!allow1)
          return DDS_RETCODE_BAD_PARAMETER;
        break;
      case DDS_DATA_REPRESENTATION_XCDR2:
        if (!allow2)
          return DDS_RETCODE_BAD_PARAMETER;
        break;
      default:
        return DDS_RETCODE_BAD_PARAMETER;
      }
    }
  }
  else
  {
    // 如果不允许XCDR1和XCDR2，则返回错误
    if (!allow1 && !allow2)
      return DDS_RETCODE_BAD_PARAMETER;
    // 设置默认的数据表示设置
    if (!allow1)
      dds_qset_data_representation(qos, 1, (dds_data_representation_id_t[]){DDS_DATA_REPRESENTATION_XCDR2});
    else if (!topicqos || !allow2)
      dds_qset_data_representation(qos, 1, (dds_data_representation_id_t[]){DDS_DATA_REPRESENTATION_XCDR1});
    else
      dds_qset_data_representation(qos, 2, (dds_data_representation_id_t[]){DDS_DATA_REPRESENTATION_XCDR1, DDS_DATA_REPRESENTATION_XCDR2});
  }
  return DDS_RETCODE_OK;
}

/**
 * @brief 从给定的QoS中获取实体名称。
 *
 * @param[in] qos 指向dds_qos_t结构体的指针，用于存储QoS设置。
 * @param[out] name 返回实体名称字符串。
 * @return 如果找到实体名称，则返回true；否则返回false。
 */
bool dds_qget_entity_name(const dds_qos_t *__restrict qos, char **name)
{
  // 检查输入参数是否有效
  if (qos == NULL || name == NULL || !(qos->present & DDSI_QP_ENTITY_NAME))
    return false;

  // 复制实体名称字符串
  *name = dds_string_dup(qos->entity_name);
  return *name != NULL;
}

/**
 * @brief 应用实体命名规则。
 *
 * @param[in,out] qos 指向dds_qos_t结构体的指针，用于存储QoS设置。
 * @param[in] parent_qos 指向父实体的dds_qos_t结构体的指针。
 * @param[in] gv 指向ddsi_domaingv结构体的指针，用于获取命名配置和锁。
 */
void dds_apply_entity_naming(dds_qos_t *qos, /* optional */ dds_qos_t *parent_qos, struct ddsi_domaingv *gv)
{
  // 如果实体命名模式为默认且QoS中不存在实体名称
  if (gv->config.entity_naming_mode == DDSI_ENTITY_NAMING_DEFAULT_FANCY && !(qos->present & DDSI_QP_ENTITY_NAME))
  {
    char name_buf[16];
    ddsrt_mutex_lock(&gv->naming_lock);
    ddsrt_prng_random_name(&gv->naming_rng, name_buf, sizeof(name_buf));
    ddsrt_mutex_unlock(&gv->naming_lock);
    // 如果存在父实体QoS，则复制父实体名称前缀
    if (parent_qos && parent_qos->present & DDSI_QP_ENTITY_NAME)
    {
      memcpy(name_buf, parent_qos->entity_name, strnlen(parent_qos->entity_name, 3));
    }
    // 设置实体名称
    dds_qset_entity_name(qos, name_buf);
  }
}
