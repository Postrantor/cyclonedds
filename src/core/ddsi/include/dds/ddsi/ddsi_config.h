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
#ifndef DDSI_CONFIG_H
#define DDSI_CONFIG_H

#include <stdio.h>

#include "dds/ddsi/ddsi_locator.h"
#include "dds/ddsi/ddsi_portmapping.h"
#include "dds/ddsi/ddsi_xqos.h"
#include "dds/ddsrt/random.h"
#include "dds/ddsrt/sched.h"
#include "dds/export.h"
#include "dds/features.h"

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @struct ddsi_config
 * @brief Cyclone DDS 配置结构体 (Cyclone DDS configuration structure)
 */

/**
 * @enum ddsi_standards_conformance
 * @brief DDSI 标准遵循级别枚举 (DDSI standards conformance level enumeration)
 */
enum ddsi_standards_conformance {
  DDSI_SC_PEDANTIC,  ///< 严格遵循标准 (Strictly conform to the standard)
  DDSI_SC_STRICT,    ///< 较为严格遵循标准 (More strictly conform to the standard)
  DDSI_SC_LAX        ///< 宽松遵循标准 (Loosely conform to the standard)
};

/**
 * @def DDSI_SC_PEDANTIC_P(config)
 * @brief 检查配置是否为严格遵循标准 (Check if the configuration is strictly conforming to the
 * standard)
 */
#define DDSI_SC_PEDANTIC_P(config) ((config).standards_conformance <= DDSI_SC_PEDANTIC)

/**
 * @def DDSI_SC_STRICT_P(config)
 * @brief 检查配置是否为较为严格遵循标准 (Check if the configuration is more strictly conforming to
 * the standard)
 */
#define DDSI_SC_STRICT_P(config) ((config).standards_conformance <= DDSI_SC_STRICT)

/**
 * @enum ddsi_besmode
 * @brief DDSI 最佳效果模式枚举 (DDSI best-effort mode enumeration)
 */
enum ddsi_besmode {
  DDSI_BESMODE_FULL,     ///< 完全模式 (Full mode)
  DDSI_BESMODE_WRITERS,  ///< 仅限写入者模式 (Writers-only mode)
  DDSI_BESMODE_MINIMAL   ///< 最小模式 (Minimal mode)
};

/**
 * @enum ddsi_retransmit_merging
 * @brief DDSI 重传合并策略枚举 (DDSI retransmit merging policy enumeration)
 */
enum ddsi_retransmit_merging {
  DDSI_REXMIT_MERGE_NEVER,     ///< 从不合并 (Never merge)
  DDSI_REXMIT_MERGE_ADAPTIVE,  ///< 自适应合并 (Adaptive merge)
  DDSI_REXMIT_MERGE_ALWAYS     ///< 总是合并 (Always merge)
};

/**
 * @enum ddsi_boolean_default
 * @brief DDSI 布尔值默认设置枚举 (DDSI boolean default setting enumeration)
 */
enum ddsi_boolean_default {
  DDSI_BOOLDEF_DEFAULT,  ///< 默认值 (Default value)
  DDSI_BOOLDEF_FALSE,    ///< 假 (False)
  DDSI_BOOLDEF_TRUE      ///< 真 (True)
};

#ifdef DDS_HAS_SHM
/**
 * @enum ddsi_shm_loglevel
 * @brief DDSI 共享内存日志级别枚举 (DDSI shared memory log level enumeration)
 */
enum ddsi_shm_loglevel {
  DDSI_SHM_OFF = 0,  ///< 关闭日志 (Turn off logging)
  DDSI_SHM_FATAL,    ///< 致命错误日志 (Fatal error logging)
  DDSI_SHM_ERROR,    ///< 错误日志 (Error logging)
  DDSI_SHM_WARN,     ///< 警告日志 (Warning logging)
  DDSI_SHM_INFO,     ///< 信息日志 (Information logging)
  DDSI_SHM_DEBUG,    ///< 调试日志 (Debug logging)
  DDSI_SHM_VERBOSE   ///< 详细日志 (Verbose logging)
};
#endif

#define DDSI_PARTICIPANT_INDEX_AUTO -1  // 自动参与者索引值（Automatic participant index value）
#define DDSI_PARTICIPANT_INDEX_NONE -2  // 无参与者索引值（No participant index value）

/* ddsi_config_listelem 必须是所有使用的 listelem 类型的覆盖 */
/* ddsi_config_listelem must be an overlay for all used listelem types */
struct ddsi_config_listelem {
  struct ddsi_config_listelem* next;  // 指向下一个元素的指针（Pointer to the next element）
};

#ifdef DDS_HAS_NETWORK_PARTITIONS
struct ddsi_networkpartition_address {
  struct ddsi_networkpartition_address*
      next;            // 指向下一个地址的指针（Pointer to the next address）
  ddsi_locator_t loc;  // 定位器（Locator）
};

struct ddsi_config_networkpartition_listelem {
  struct ddsi_config_networkpartition_listelem*
      next;  // 指向下一个网络分区元素的指针（Pointer to the next network partition element）
  char* name;             // 网络分区名称（Network partition name）
  char* address_string;   // 地址字符串（Address string）
  char* interface_names;  // 接口名称（Interface names）
  struct ddsi_networkpartition_address* uc_addresses;  // 单播地址列表（Unicast addresses list）
  struct ddsi_networkpartition_address* asm_addresses;  // ASM 地址列表（ASM addresses list）
#ifdef DDS_HAS_SSM
  struct ddsi_networkpartition_address* ssm_addresses;  // SSM 地址列表（SSM addresses list）
#endif
};

struct ddsi_config_ignoredpartition_listelem {
  struct ddsi_config_ignoredpartition_listelem*
      next;  // 指向下一个忽略分区元素的指针（Pointer to the next ignored partition element）
  char* DCPSPartitionTopic;  // 忽略的分区主题（Ignored partition topic）
};

struct ddsi_config_partitionmapping_listelem {
  struct ddsi_config_partitionmapping_listelem*
      next;  // 指向下一个分区映射元素的指针（Pointer to the next partition mapping element）
  char* networkPartition;    // 网络分区名称（Network partition name）
  char* DCPSPartitionTopic;  // 映射的分区主题（Mapped partition topic）
  struct ddsi_config_networkpartition_listelem*
      partition;  // 对应的网络分区元素（Corresponding network partition element）
};
#endif /* DDS_HAS_NETWORK_PARTITIONS */

struct ddsi_config_maybe_int32 {
  int isdefault;  // 标识是否为默认值（Indicates whether it is a default value）
  int32_t value;  // 存储的整数值（Stored integer value）
};

struct ddsi_config_maybe_uint32 {
  int isdefault;   // 标识是否为默认值（Indicates whether it is a default value）
  uint32_t value;  // 存储的无符号整数值（Stored unsigned integer value）
};

struct ddsi_config_thread_properties_listelem {
  struct ddsi_config_thread_properties_listelem*
      next;  // 指向下一个线程属性元素的指针（Pointer to the next thread properties element）
  char* name;                 // 线程名称（Thread name）
  ddsrt_sched_t sched_class;  // 调度类（Scheduling class）
  struct ddsi_config_maybe_int32
      schedule_priority;  // 可选的调度优先级（Optional scheduling priority）
  struct ddsi_config_maybe_uint32 stack_size;  // 可选的栈大小（Optional stack size）
};

/** ddsi_config_peer_listelem 结构体定义了一个用于存储 peer 列表的元素。
 *  The ddsi_config_peer_listelem structure defines an element for storing a list of peers.
 */
struct ddsi_config_peer_listelem {
  struct ddsi_config_peer_listelem*
      next;   /**< 下一个列表元素的指针。Pointer to the next list element. */
  char* peer; /**< 存储 peer 地址的字符串。String storing the peer address. */
};

/** ddsi_config_prune_deleted_ppant 结构体定义了与删除参与者相关的配置选项。
 *  The ddsi_config_prune_deleted_ppant structure defines configuration options related to pruning
 * deleted participants.
 */
struct ddsi_config_prune_deleted_ppant {
  int64_t delay; /**< 延迟删除参与者的时间（以纳秒为单位）。Time in nanoseconds to delay deletion of
                    participants. */
  int enforce_delay; /**< 是否强制执行延迟删除。Whether to enforce delay deletion or not. */
};

/* 允许多播位（默认取决于网络类型）：
 * Allow multicast bits (default depends on network type):
 */
#define DDSI_AMC_FALSE 0u
#define DDSI_AMC_SPDP 1u
#define DDSI_AMC_ASM 2u
#ifdef DDS_HAS_SSM
#define DDSI_AMC_SSM 4u
#define DDSI_AMC_TRUE (DDSI_AMC_SPDP | DDSI_AMC_ASM | DDSI_AMC_SSM)
#else
#define DDSI_AMC_TRUE (DDSI_AMC_SPDP | DDSI_AMC_ASM)
#endif
#define DDSI_AMC_DEFAULT 0x80000000u

/* FIXME: 这应该是完全动态的...但这对于快速实现来说更容易
 * FIXME: this should be fully dynamic ... but this is easier for a quick hack
 */
enum ddsi_transport_selector {
  DDSI_TRANS_DEFAULT, /* 实际上是 UDP，但这样我们可以知道哪些已经设置了。Actually UDP, but this is
                         so we can tell what has been set. */
  DDSI_TRANS_UDP,
  DDSI_TRANS_UDP6,
  DDSI_TRANS_TCP,
  DDSI_TRANS_TCP6,
  DDSI_TRANS_RAWETH,
  DDSI_TRANS_NONE /* FIXME: 参见上面的 FIXME... :( See FIXME above ... :( */
};

enum ddsi_many_sockets_mode { DDSI_MSM_NO_UNICAST, DDSI_MSM_SINGLE_UNICAST, DDSI_MSM_MANY_UNICAST };

#ifdef DDS_HAS_SECURITY
/** ddsi_plugin_library_properties 结构体定义了插件库属性。
 *  The ddsi_plugin_library_properties structure defines plugin library properties.
 */
struct ddsi_plugin_library_properties {
  char* library_path; /**< 插件库路径。Plugin library path. */
  char* library_init; /**< 插件库初始化函数名。Plugin library initialization function name. */
  char* library_finalize; /**< 插件库结束函数名。Plugin library finalization function name. */
};

/** ddsi_authentication_properties 结构体定义了认证属性。
 *  The ddsi_authentication_properties structure defines authentication properties.
 */
struct ddsi_authentication_properties {
  char* identity_certificate; /**< 身份证书文件路径。Identity certificate file path. */
  char* identity_ca; /**< 身份认证机构文件路径。Identity Certificate Authority file path. */
  char* private_key; /**< 私钥文件路径。Private key file path. */
  char* password;    /**< 密码。Password. */
  char* trusted_ca_dir; /**< 受信任的认证机构目录。Trusted Certificate Authority directory. */
  char* crl; /**< 吊销证书列表文件路径。Certificate Revocation List file path. */
  int include_optional_fields; /**< 是否包含可选字段。Whether to include optional fields or not. */
};
#endif

/**
 * @brief ddsi_access_control_properties 结构体
 * @param permissions 权限字符串
 * @param permissions_ca 权限证书颁发机构
 * @param governance 管理策略
 */
struct ddsi_access_control_properties {
  char* permissions;     ///< 权限字符串 (Permissions string)
  char* permissions_ca;  ///< 权限证书颁发机构 (Permissions Certificate Authority)
  char* governance;      ///< 管理策略 (Governance policy)
};

/**
 * @brief ddsi_config_omg_security 结构体
 * @param authentication_properties 认证属性
 * @param access_control_properties 访问控制属性
 * @param authentication_plugin 认证插件
 * @param access_control_plugin 访问控制插件
 * @param cryptography_plugin 加密插件
 */
struct ddsi_config_omg_security {
  struct ddsi_authentication_properties
      authentication_properties;  ///< 认证属性 (Authentication properties)
  struct ddsi_access_control_properties
      access_control_properties;  ///< 访问控制属性 (Access control properties)
  struct ddsi_plugin_library_properties
      authentication_plugin;  ///< 认证插件 (Authentication plugin)
  struct ddsi_plugin_library_properties
      access_control_plugin;  ///< 访问控制插件 (Access control plugin)
  struct ddsi_plugin_library_properties cryptography_plugin;  ///< 加密插件 (Cryptography plugin)
};

/**
 * @brief ddsi_config_omg_security_listelem 结构体
 * @param next 指向下一个元素的指针
 * @param cfg ddsi_config_omg_security 配置
 */
struct ddsi_config_omg_security_listelem {
  struct ddsi_config_omg_security_listelem*
      next;  ///< 指向下一个元素的指针 (Pointer to the next element)
  struct ddsi_config_omg_security
      cfg;  ///< ddsi_config_omg_security 配置 (ddsi_config_omg_security configuration)
};
#endif /* DDS_HAS_SECURITY */

#ifdef DDS_HAS_SSL
/**
 * @brief ddsi_config_ssl_min_version 结构体
 * @param major 主版本号
 * @param minor 次版本号
 */
struct ddsi_config_ssl_min_version {
  int major;  ///< 主版本号 (Major version number)
  int minor;  ///< 次版本号 (Minor version number)
};
#endif

/**
 * @brief ddsi_config_socket_buf_size 结构体
 * @param min 最小缓冲区大小
 * @param max 最大缓冲区大小
 */
struct ddsi_config_socket_buf_size {
  struct ddsi_config_maybe_uint32 min;  ///< 最小缓冲区大小 (Minimum buffer size)
  struct ddsi_config_maybe_uint32 max;  ///< 最大缓冲区大小 (Maximum buffer size)
};

/**
 * @brief ddsi_config_network_interface 结构体
 * @param automatic 自动选择网络接口
 * @param name 网络接口名称
 * @param address 网络接口地址
 * @param prefer_multicast 是否优先使用多播
 * @param presence_required 是否需要网络接口存在
 * @param multicast 多播设置
 * @param priority 网络接口优先级
 */
struct ddsi_config_network_interface {
  int automatic;         ///< 自动选择网络接口 (Automatic network interface selection)
  char* name;            ///< 网络接口名称 (Network interface name)
  char* address;         ///< 网络接口地址 (Network interface address)
  int prefer_multicast;  ///< 是否优先使用多播 (Prefer multicast)
  int presence_required;  ///< 是否需要网络接口存在 (Presence required for the network interface)
  enum ddsi_boolean_default multicast;      ///< 多播设置 (Multicast setting)
  struct ddsi_config_maybe_int32 priority;  ///< 网络接口优先级 (Network interface priority)
};

/**
 * @brief ddsi_config_network_interface_listelem 结构体
 * @param next 指向下一个元素的指针
 * @param cfg ddsi_config_network_interface 配置
 */
struct ddsi_config_network_interface_listelem {
  struct ddsi_config_network_interface_listelem*
      next;  ///< 指向下一个元素的指针 (Pointer to the next element)
  struct ddsi_config_network_interface
      cfg;  ///< ddsi_config_network_interface 配置 (ddsi_config_network_interface configuration)
};

/**
 * @brief ddsi_config_entity_naming_mode 枚举
 * @param DDSI_ENTITY_NAMING_DEFAULT_EMPTY 默认为空
 * @param DDSI_ENTITY_NAMING_DEFAULT_FANCY 默认为精美命名
 */
enum ddsi_config_entity_naming_mode {
  DDSI_ENTITY_NAMING_DEFAULT_EMPTY,  ///< 默认为空 (Default empty)
  DDSI_ENTITY_NAMING_DEFAULT_FANCY   ///< 默认为精美命名 (Default fancy naming)
};

/* Expensive checks (compiled in when NDEBUG not defined, enabled only if flag set in xchecks) */
#define DDSI_XCHECK_WHC 1u  ///< 写入历史缓存检查 (Write history cache check)
#define DDSI_XCHECK_RHC 2u  ///< 读取历史缓存检查 (Read history cache check)
#define DDSI_XCHECK_XEV 4u  ///< 事件检查 (Event check)

/**
 * @brief 默认初始化配置（不稳定）
 * @component config
 *
 * @param[out]  cfg 要初始化的配置结构体。
 */
DDS_EXPORT void ddsi_config_init_default(struct ddsi_config* cfg);

// 定义ddsi_config结构体
struct ddsi_config {
  int valid;                 // 配置是否有效
  uint32_t tracemask;        // 跟踪掩码
  uint32_t enabled_xchecks;  // 启用的交叉检查
  char* pcap_file;           // PCAP文件路径

  /* 接口 */
  struct ddsi_config_network_interface_listelem* network_interfaces;  // 网络接口列表

  /* 废弃的接口支持 */
  char* depr_networkAddressString;    // 废弃的网络地址字符串
  int depr_prefer_multicast;          // 废弃的多播优先设置
  char* depr_assumeMulticastCapable;  // 废弃的假设多播能力设置

  char** networkRecvAddressStrings;                 // 网络接收地址字符串数组
  uint32_t allowMulticast;                          // 是否允许多播
  char* externalAddressString;                      // 外部地址字符串
  char* externalMaskString;                         // 外部掩码字符串
  FILE* tracefp;                                    // 跟踪文件指针
  char* tracefile;                                  // 跟踪文件名
  int tracingAppendToFile;                          // 是否将跟踪信息追加到文件中
  enum ddsi_transport_selector transport_selector;  // 传输选择器枚举
  enum ddsi_boolean_default compat_use_ipv6;        // 是否使用IPv6的设置
  enum ddsi_boolean_default compat_tcp_enable;      // 是否启用TCP的设置
  int dontRoute;                                    // 是否禁用路由
  int enableMulticastLoopback;                      // 是否启用多播环回
  uint32_t domainId;                                // 域ID
  struct ddsi_config_maybe_uint32 extDomainId;      // 发现中宣告的域ID
  char* domainTag;                                  // 域标签
  int participantIndex;                             // 参与者索引
  int maxAutoParticipantIndex;                      // 最大自动参与者索引
  char* spdpMulticastAddressString;                 // SPDP多播地址字符串
  char* defaultMulticastAddressString;              // 默认多播地址字符串
  int64_t spdp_interval;                            // SPDP间隔
  int64_t spdp_response_delay_max;                  // SPDP响应最大延迟
  int64_t lease_duration;                           // 租约持续时间
  int64_t const_hb_intv_sched;                      // 心跳间隔计划常量
  int64_t const_hb_intv_sched_min;                  // 心跳间隔计划最小值
  int64_t const_hb_intv_sched_max;                  // 心跳间隔计划最大值
  int64_t const_hb_intv_min;                        // 最小心跳间隔
  enum ddsi_retransmit_merging retransmit_merging;  // 重传合并枚举
  int64_t retransmit_merging_period;                // 重传合并周期
  int squash_participants;                          // 压缩参与者设置
  int liveliness_monitoring;                        // 活跃度监控设置
  int noprogress_log_stacktraces;                   // 不记录进度日志堆栈跟踪设置
  int64_t liveliness_monitoring_interval;           // 活跃度监控间隔
  int prioritize_retransmit;                        // 是否优先处理重传
  enum ddsi_boolean_default multiple_recv_threads;  // 是否使用多个接收线程的设置
  unsigned recv_thread_stop_maxretries;             // 接收线程停止的最大重试次数

  unsigned primary_reorder_maxsamples;    // 主要重排序最大样本数
  unsigned secondary_reorder_maxsamples;  // 次要重排序最大样本数

  unsigned delivery_queue_maxsamples;  // 交付队列最大样本数

  uint16_t fragment_size;            // 分片大小
  uint32_t max_msg_size;             // 最大消息大小
  uint32_t max_rexmit_msg_size;      // 最大重传消息大小
  uint32_t init_transmit_extra_pct;  // 初始传输额外百分比
  uint32_t max_rexmit_burst_size;    // 最大重传突发大小

  int publish_uc_locators; /* 发布发现单播定位器 */
  int enable_uc_locators;  /* 如果为false，则不尝试创建单播套接字 */

#ifdef DDS_HAS_TOPIC_DISCOVERY
  int enable_topic_discovery_endpoints;  // 是否启用主题发现端点的设置
#endif

  int tcp_nodelay;                  /**< 是否禁用 Nagle 算法 */
  int tcp_port;                     /**< TCP 端口号 */
  int64_t tcp_read_timeout;         /**< TCP 读取超时时间，单位：纳秒 */
  int64_t tcp_write_timeout;        /**< TCP 写入超时时间，单位：纳秒 */
  int tcp_use_peeraddr_for_unicast; /**< 是否使用对等地址进行单播 */

#ifdef DDS_HAS_SSL
  int ssl_enable;                                     /**< 是否启用 SSL */
  int ssl_verify;                                     /**< 是否验证 SSL 证书 */
  int ssl_verify_client;                              /**< 是否验证客户端 SSL 证书 */
  int ssl_self_signed;                                /**< 是否允许自签名证书 */
  char* ssl_keystore;                                 /**< SSL 密钥库文件路径 */
  char* ssl_rand_file;                                /**< SSL 随机数生成器文件路径 */
  char* ssl_key_pass;                                 /**< SSL 密钥密码 */
  char* ssl_ciphers;                                  /**< SSL 加密套件列表 */
  struct ddsi_config_ssl_min_version ssl_min_version; /**< SSL 最低版本要求 */
#endif

#ifdef DDS_HAS_NETWORK_PARTITIONS
  struct ddsi_config_networkpartition_listelem* networkPartitions;  /**< 网络分区列表 */
  unsigned nof_networkPartitions;                                   /**< 网络分区数量 */
  struct ddsi_config_ignoredpartition_listelem* ignoredPartitions;  /**< 被忽略的分区列表 */
  struct ddsi_config_partitionmapping_listelem* partitionMappings;  /**< 分区映射列表 */
#endif                                                              /* DDS_HAS_NETWORK_PARTITIONS */
  struct ddsi_config_peer_listelem* peers;                          /**< 对等节点列表 */
  struct ddsi_config_thread_properties_listelem* thread_properties; /**< 线程属性列表 */

  /* 调试/测试/未记录特性: */
  int xmit_lossiness;                          /**< 发送丢包率，单位：1e-3 */
  uint32_t rmsg_chunk_size;                    /**< 接收缓冲区中块的大小 */
  uint32_t rbuf_size;                          /**< 单个接收器缓冲区的大小 */
  enum ddsi_besmode besmode;                   /**< Builtin Endpoint Set 模式 */
  int meas_hb_to_ack_latency;                  /**< 是否测量心跳到确认延迟 */
  int unicast_response_to_spdp_messages;       /**< 是否对 SPDP 消息进行单播响应 */
  int synchronous_delivery_priority_threshold; /**< 同步传输优先级阈值 */
  int64_t synchronous_delivery_latency_bound;  /**< 同步传输延迟上限 */

  /* 写入缓存 */
  int whc_batch;               /**< 是否启用批处理写入历史缓存 */
  uint32_t whc_lowwater_mark;  /**< 写入历史缓存低水位标记 */
  uint32_t whc_highwater_mark; /**< 写入历史缓存高水位标记 */
  struct ddsi_config_maybe_uint32 whc_init_highwater_mark; /**< 写入历史缓存初始高水位标记 */
  int whc_adaptive; /**< 是否启用自适应写入历史缓存 */

  unsigned defrag_unreliable_maxsamples; /**< 不可靠分片重组最大样本数 */
  unsigned defrag_reliable_maxsamples;   /**< 可靠分片重组最大样本数 */
  unsigned accelerate_rexmit_block_size; /**< 加速重传块大小 */
  int64_t responsiveness_timeout;        /**< 响应超时时间 */
  uint32_t max_participants;             /**< 最大参与者数量 */
  int64_t writer_linger_duration;        /**< 写入器停留时间 */
  int multicast_ttl;                     /**< 多播 TTL 值 */
  struct ddsi_config_socket_buf_size socket_rcvbuf_size; /**< 套接字接收缓冲区大小 */
  struct ddsi_config_socket_buf_size socket_sndbuf_size; /**< 套接字发送缓冲区大小 */
  int64_t ack_delay;                                     /**< 确认延迟时间 */
  int64_t nack_delay;                                    /**< 否定确认延迟时间 */
  int64_t preemptive_ack_delay;                          /**< 抢占式确认延迟时间 */
  int64_t schedule_time_rounding;                        /**< 调度时间取整 */
  int64_t auto_resched_nack_delay;  /**< 自动重新调度否定确认延迟时间 */
  int64_t ds_grace_period;          /**< 数据序列宽限期 */
  uint32_t max_queued_rexmit_bytes; /**< 最大排队重传字节数 */
  unsigned max_queued_rexmit_msgs;  /**< 最大排队重传消息数 */
  int late_ack_mode;                /**< 延迟确认模式 */
  int retry_on_reject_besteffort;   /**< 是否在拒绝最佳努力时重试 */
  int generate_keyhash;             /**< 是否生成密钥哈希值 */
  uint32_t max_sample_size;         /**< 最大样本大小 */

  /* 兼容性选项 */
  enum ddsi_standards_conformance standards_conformance; /**< 标准兼容性模式 */
  int explicitly_publish_qos_set_to_default;             /**< 是否显式发布默认 QoS 设置 */
  enum ddsi_many_sockets_mode many_sockets_mode;         /**< 多套接字模式 */
  int assume_rti_has_pmd_endpoints;                      /**< 是否假定 RTI 具有 PMD 端点 */

  struct ddsi_portmapping ports; /**< 端口映射配置 */

  int monitor_port; /**< 监控端口号 */

  int enable_control_topic;        /**< 是否启用控制主题 */
  int initial_deaf;                /**< 初始失聪状态 */
  int initial_mute;                /**< 初始静音状态 */
  int64_t initial_deaf_mute_reset; /**< 初始失聪静音重置时间 */

  int use_multicast_if_mreqn;                                 /**< 是否使用多播接口 MREQN */
  struct ddsi_config_prune_deleted_ppant prune_deleted_ppant; /**< 删除的参与者修剪配置 */
  int redundant_networking;                                   /**< 冗余网络配置 */

#ifdef DDS_HAS_SECURITY
  struct ddsi_config_omg_security_listelem* omg_security_configuration; /**< OMG 安全配置列表 */
#endif

#ifdef DDS_HAS_SHM
  int enable_shm;                     /**< 是否启用共享内存 */
  char* shm_locator;                  /**< 共享内存定位器路径 */
  char* iceoryx_service;              /**< iceoryx 服务名称 */
  enum ddsi_shm_loglevel shm_log_lvl; /**< 共享内存日志级别 */
#endif

  enum ddsi_config_entity_naming_mode entity_naming_mode; /**< 实体命名模式 */
  ddsrt_prng_seed_t entity_naming_seed;                   /**< 实体命名随机数种子 */

#if defined(__cplusplus)
 public:
  ddsi_config() { ddsi_config_init_default(this); }
#endif
};

/**
 * @component config
 * @brief 初始化 ddsi 配置
 *
 * @param[in] config  配置文件的路径（不能为空）
 * @param[out] cfg    存储配置信息的结构体指针（不能为空）
 * @param[in] domid   域 ID
 *
 * @return 返回一个 ddsi_cfgst 结构体指针，用于存储解析后的配置信息
 */
struct ddsi_cfgst* ddsi_config_init(const char* config, struct ddsi_config* cfg, uint32_t domid)
    ddsrt_nonnull((1, 2));

/**
 * @component config
 * @brief 清理并释放 ddsi_cfgst 结构体资源
 *
 * @param[in] cfgst  需要清理的 ddsi_cfgst 结构体指针
 */
DDS_EXPORT void ddsi_config_fini(struct ddsi_cfgst* cfgst);

#if defined(__cplusplus)
}
#endif

#endif /* DDSI_CONFIG_H */
