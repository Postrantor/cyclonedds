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
#ifndef DDSI_DOMAINGV_H
#define DDSI_DOMAINGV_H

#include <stdio.h>

#include "dds/ddsi/ddsi_ownip.h"
#include "dds/ddsi/ddsi_plist.h"
#include "dds/ddsi/ddsi_protocol.h"
#include "dds/ddsi/ddsi_sockwaitset.h"
#include "dds/ddsrt/atomics.h"
#include "dds/ddsrt/avl.h"
#include "dds/ddsrt/fibheap.h"
#include "dds/ddsrt/random.h"
#include "dds/ddsrt/sockets.h"
#include "dds/ddsrt/sync.h"
#include "dds/export.h"
#include "dds/features.h"

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @file ddsi_structs.h
 *
 * @brief 定义了一些在 cyclone dds 项目中使用的结构体。
 *        Defines several structures used in the cyclone dds project.
 */

/**
 * @struct ddsi_xmsgpool
 * @brief 消息池，用于存储和管理消息。
 *        Message pool, used for storing and managing messages.
 */
struct ddsi_xmsgpool;

/**
 * @struct ddsi_dqueue
 * @brief 双向队列，用于存储和管理数据。
 *        Double-ended queue, used for storing and managing data.
 */
struct ddsi_dqueue;

/**
 * @struct ddsi_reorder
 * @brief 重新排序模块，用于对接收到的数据进行重新排序。
 *        Reordering module, used for reordering received data.
 */
struct ddsi_reorder;

/**
 * @struct ddsi_defrag
 * @brief 数据分片重组模块，用于将分片的数据重新组合成完整的数据。
 *        Data defragmentation module, used for reassembling fragmented data into complete data.
 */
struct ddsi_defrag;

/**
 * @struct ddsi_addrset
 * @brief 地址集合，用于存储和管理地址信息。
 *        Address set, used for storing and managing address information.
 */
struct ddsi_addrset;

/**
 * @struct ddsi_xeventq
 * @brief 事件队列，用于存储和管理事件。
 *        Event queue, used for storing and managing events.
 */
struct ddsi_xeventq;

/**
 * @struct ddsi_gcreq_queue
 * @brief 垃圾回收请求队列，用于存储和管理垃圾回收请求。
 *        Garbage collection request queue, used for storing and managing garbage collection
 * requests.
 */
struct ddsi_gcreq_queue;

/**
 * @struct ddsi_entity_index
 * @brief 实体索引，用于存储和管理实体信息。
 *        Entity index, used for storing and managing entity information.
 */
struct ddsi_entity_index;

/**
 * @struct ddsi_lease
 * @brief 租约结构，用于管理实体的生命周期。
 *        Lease structure, used for managing the lifecycle of entities.
 */
struct ddsi_lease;

/**
 * @struct ddsi_tran_conn
 * @brief 传输连接，用于管理通信连接。
 *        Transport connection, used for managing communication connections.
 */
struct ddsi_tran_conn;

/**
 * @struct ddsi_tran_listener
 * @brief 传输监听器，用于监听通信连接。
 *        Transport listener, used for listening to communication connections.
 */
struct ddsi_tran_listener;

/**
 * @struct ddsi_tran_factory
 * @brief 传输工厂，用于创建和管理传输对象。
 *        Transport factory, used for creating and managing transport objects.
 */
struct ddsi_tran_factory;

/**
 * @struct ddsi_debug_monitor
 * @brief 调试监视器，用于监视调试信息。
 *        Debug monitor, used for monitoring debug information.
 */
struct ddsi_debug_monitor;

/**
 * @struct ddsi_tkmap
 * @brief 主题键映射，用于存储和管理主题键信息。
 *        Topic key mapping, used for storing and managing topic key information.
 */
struct ddsi_tkmap;

/**
 * @struct dds_security_context
 * @brief 安全上下文，用于存储和管理安全相关的信息。
 *        Security context, used for storing and managing security-related information.
 */
struct dds_security_context;

/**
 * @struct dds_security_match_index
 * @brief 安全匹配索引，用于存储和管理安全匹配信息。
 *        Security match index, used for storing and managing security match information.
 */
struct dds_security_match_index;

/**
 * @struct ddsi_hsadmin
 * @brief 握手管理器，用于管理安全握手过程。
 *        Handshake manager, used for managing the secure handshake process.
 */
struct ddsi_hsadmin;

/**
 * @brief ddsi_config_in_addr_node 结构体定义
 * @struct ddsi_config_in_addr_node
 *
 * @param loc 定位器，用于存储 IP 地址和端口信息 (Locator, used to store IP address and port
 * information)
 * @param next 指向下一个 ddsi_config_in_addr_node 结构体的指针 (Pointer to the next
 * ddsi_config_in_addr_node structure)
 */
struct ddsi_config_in_addr_node {
  ddsi_locator_t loc;
  struct ddsi_config_in_addr_node* next;
};

/**
 * @brief ddsi_recvips_mode 枚举定义，表示接收 IP 的模式 (Enumeration definition of
 * ddsi_recvips_mode, representing the mode of receiving IPs)
 *
 * @enum ddsi_recvips_mode
 */
enum ddsi_recvips_mode {
  DDSI_RECVIPS_MODE_ALL, /**< 所有支持多播的接口 (All multicast-capable interfaces) */
  DDSI_RECVIPS_MODE_ANY, /**< 内核默认接口 (Kernel-default interface) */
  DDSI_RECVIPS_MODE_PREFERRED, /**< 仅选择的接口 (Selected interface only) */
  DDSI_RECVIPS_MODE_NONE,      /**< 不使用任何接口 (No interfaces at all) */
  DDSI_RECVIPS_MODE_SOME /**< 显示接口列表；只需要 recvips 的一个 (Explicit list of interfaces; only
                            one requiring recvips) */
};

/**
 * @brief ddsi_recv_thread_mode 枚举定义，表示接收线程的模式 (Enumeration definition of
 * ddsi_recv_thread_mode, representing the mode of receiving threads)
 *
 * @enum ddsi_recv_thread_mode
 */
enum ddsi_recv_thread_mode { DDSI_RTM_SINGLE, DDSI_RTM_MANY };

/**
 * @brief ddsi_recv_thread_arg 结构体定义，表示接收线程的参数 (Structure definition of
 * ddsi_recv_thread_arg, representing the parameters of receiving threads)
 *
 * @struct ddsi_recv_thread_arg
 *
 * @param mode 接收线程模式，可以是单个或多个 (Receiving thread mode, can be single or multiple)
 * @param rbpool 指向 ddsi_rbufpool 结构体的指针 (Pointer to the ddsi_rbufpool structure)
 * @param gv 指向 ddsi_domaingv 结构体的指针 (Pointer to the ddsi_domaingv structure)
 * @param u 匿名联合，包含单个和多个接收线程的参数 (Anonymous union, containing parameters for
 * single and multiple receiving threads)
 */
struct ddsi_recv_thread_arg {
  enum ddsi_recv_thread_mode mode;
  struct ddsi_rbufpool* rbpool;
  struct ddsi_domaingv* gv;
  union {
    struct {
      const ddsi_locator_t*
          loc;  /**< 单个接收线程的定位器 (Locator for a single receiving thread) */
      struct ddsi_tran_conn*
          conn; /**< 单个接收线程的连接 (Connection for a single receiving thread) */
    } single;
    struct {
      struct ddsi_sock_waitset*
          ws; /**< 多个接收线程的套接字等待集 (Socket waitset for multiple receiving threads) */
    } many;
  } u;
};

struct ddsi_deleted_participants_admin;

/**
 * @struct ddsi_domaingv
 * @brief DDSI domain global variables structure.
 */
struct ddsi_domaingv {
  volatile int terminate;  ///< 终止标志，非零表示需要终止 (Termination flag, non-zero indicates
                           ///< termination is needed)
  volatile int deaf;  ///< 失聪标志，非零表示不接收任何数据 (Deaf flag, non-zero indicates not
                      ///< receiving any data)
  volatile int mute;  ///< 静音标志，非零表示不发送任何数据 (Mute flag, non-zero indicates not
                      ///< sending any data)

  struct ddsrt_log_cfg
      logconfig;  ///< 日志配置，用于控制日志输出的详细程度和目标 (Log configuration, used to
                  ///< control the verbosity and target of log output)
  struct ddsi_config config;  ///< DDSI 配置，包含了与域相关的各种设置 (DDSI configuration, contains
                              ///< various settings related to the domain)

  struct ddsi_tkmap*
      m_tkmap;  ///< 主题键值映射表，用于在主题和实例之间建立映射关系 (Topic key map, used to
                ///< establish mapping relationships between topics and instances)

  /**
   * @brief Hash tables for various entities in Cyclone DDS project.
   *
   * 以下代码段为 Cyclone DDS 项目中的实体哈希表。
   */
  struct ddsi_entity_index* entity_index;

  /**
   * @brief Timed events admin.
   *
   * 定时事件管理。
   */
  struct ddsi_xeventq* xevents;

  /**
   * @brief Queue for garbage collection requests.
   *
   * 垃圾回收请求队列。
   */
  struct ddsi_gcreq_queue* gcreq_queue;

  /**
   * @brief Lease junk.
   *
   * 租约管理相关。
   */
  ddsrt_mutex_t leaseheap_lock;  ///< Mutex for protecting lease heap. (保护租约堆的互斥锁)
  ddsrt_fibheap_t leaseheap;  ///< Fibonacci heap for managing leases. (用于管理租约的斐波那契堆)

  /**
   * @brief Transport factories & selected factory.
   *
   * 传输工厂及所选工厂。
   */
  struct ddsi_tran_factory*
      ddsi_tran_factories;  ///< List of available transport factories. (可用传输工厂列表)
  struct ddsi_tran_factory* m_factory;  ///< Selected transport factory. (所选传输工厂)

  /**
   * @brief Connections for multicast discovery & data, and those that correspond
   * to the one DDSI participant index that the DDSI service uses. The
   * DCPS participant of DDSI itself will be mirrored in a DDSI
   * participant, and in multi-socket mode that one gets its own
   * socket.
   *
   * 用于多播发现和数据的连接，以及与 DDSI 服务使用的一个 DDSI 参与者索引相对应的连接。
   * DDSI 本身的 DCPS 参与者将在 DDSI
   * 参与者中得到反映，在多套接字模式下，该参与者将获得自己的套接字。
   */
  struct ddsi_tran_conn* disc_conn_mc;  ///< Multicast discovery connection. (多播发现连接)
  struct ddsi_tran_conn* data_conn_mc;  ///< Multicast data connection. (多播数据连接)
  struct ddsi_tran_conn* disc_conn_uc;  ///< Unicast discovery connection. (单播发现连接)
  struct ddsi_tran_conn* data_conn_uc;  ///< Unicast data connection. (单播数据连接)

/* 用于所有输出的连接（针对无连接传输），这里曾经只是 data_conn_uc，但有以下情况：
 *
 * - Windows 存在一个怪癖，如果传输套接字绑定到 0.0.0.0，那么机器内的多播传输会非常不可靠
 *   （尽管所有套接字都正确设置了多播接口），
 *   但显然仅在存在已绑定到非 0.0.0.0 的套接字传输到相同多播组的情况下...
 * - 至少 Fast-RTPS 和 Connext 无法遵循广告地址集，并将广告 IP 地址替换为 127.0.0.1，
 *   并期望其正常工作。
 * - Fast-RTPS（至少）将用于传输多播的套接字绑定到非 0.0.0.0
 *
 * 因此，绑定到 0.0.0.0 意味着来自 Fast-RTPS 和 Connext 的单播消息将到达，
 * 但是当试图进行互操作时，Cyclone 中的多播消息在 Windows 上经常被丢弃；
 * 而绑定到 IP 地址意味着来自其他节点的单播消息无法到达（因为它们无法到达）。
 *
 * 唯一的解决方法是使用单独的套接字进行发送。很遗憾，Cyclone 需要解决其他节点的错误，
 * 但这似乎是让用户获得他们期望的唯一方法。 */
#define MAX_XMIT_CONNS 4 /**< 最大传输连接数 */

  /**
   * @brief xmit_conns 是一个指向 ddsi_tran_conn 结构体的指针数组，用于存储最多 MAX_XMIT_CONNS
   * 个传输连接。
   */
  struct ddsi_tran_conn* xmit_conns[MAX_XMIT_CONNS];

  /**
   * @brief intf_xlocators 是一个 ddsi_xlocator_t 类型的数组，用于存储与 xmit_conns 对应的最多
   * MAX_XMIT_CONNS 个传输定位器。
   */
  ddsi_xlocator_t intf_xlocators[MAX_XMIT_CONNS];

  /**
   * @struct ddsi_tran_listener
   * @brief TCP监听器 (TCP listener)
   */
  struct ddsi_tran_listener* listener;

  /**
   * @var ddsrt_atomic_uint32_t participant_set_generation
   * @brief
   * 在多套接字模式下，接收线程维护一个包含参与者GUID和套接字的本地数组，participant_set_generation用于通知它们。
   * In many sockets mode, the receive threads maintain a local array with participant GUIDs and
   * sockets, participant_set_generation is used to notify them.
   */
  ddsrt_atomic_uint32_t participant_set_generation;

  /**
   * @var ddsrt_mutex_t participant_set_lock
   * @brief 用于限制活动参与者数量的互斥锁 (Mutex for limiting the number of active participants)
   */
  ddsrt_mutex_t participant_set_lock;

  /**
   * @var ddsrt_cond_t participant_set_cond
   * @brief 用于限制活动参与者数量的条件变量 (Condition variable for limiting the number of active
   * participants)
   */
  ddsrt_cond_t participant_set_cond;

  /**
   * @var uint32_t nparticipants
   * @brief 参与者数量，主要用于限制活动参与者数量，但也在关闭期间用于确定何时可以安全地停止GC线程。
   * Number of participants, primarily used for limiting the number of active participants, but also
   * during shutdown to determine when it is safe to stop the GC thread.
   */
  uint32_t nparticipants;

  /**
   * @var struct ddsi_participant* privileged_pp
   * @brief
   * 对于没有（某些）内置写入器的参与者，我们回退到此参与者，这是创建所有内置写入器的第一个参与者。在需要它的任何地方弹出之前，必须创建它！
   * For participants without (some) built-in writers, we fall back to this participant, which is
   * the first one created with all built-in writers present. It MUST be created before any in need
   * of it pops up!
   */
  struct ddsi_participant* privileged_pp;

  /**
   * @var ddsrt_mutex_t privileged_pp_lock
   * @brief 用于保护privileged_pp的互斥锁 (Mutex for protecting privileged_pp)
   */
  ddsrt_mutex_t privileged_pp_lock;

  /**
   * @var struct ddsi_deleted_participants_admin* deleted_participants
   * @brief 用于跟踪（最近）删除的参与者 (For tracking (recently) deleted participants)
   */
  struct ddsi_deleted_participants_admin* deleted_participants;

  /**
   * @var struct ddsi_guid ppguid_base
   * @brief 在下一次调用new_participant时要使用的GUID；也受privileged_pp_lock保护。
   * GUID to be used in next call to new_participant; also protected by privileged_pp_lock.
   */
  struct ddsi_guid ppguid_base;

  /**
   * @var int n_interfaces
   * @brief 选定接口的数量 (Number of selected interfaces)
   */
  int n_interfaces;

  /**
   * @var struct ddsi_network_interface interfaces[MAX_XMIT_CONNS]
   * @brief 网络接口数组 (Array of network interfaces)
   */
  struct ddsi_network_interface interfaces[MAX_XMIT_CONNS];

  /**
   * @var int using_link_local_intf
   * @brief 是否使用本地链路地址（因此仅在该接口上侦听多播）
   * Whether we're using a link-local address (and therefore only listening to multicasts on that
   * interface)
   */
  int using_link_local_intf;

  /**
   * @var enum ddsi_recvips_mode recvips_mode
   * @brief 接收IP模式 (Receiving IP mode)
   */
  enum ddsi_recvips_mode recvips_mode;

  /**
   * @var struct ddsi_config_in_addr_node* recvips
   * @brief 接收IP地址列表 (List of receiving IP addresses)
   */
  struct ddsi_config_in_addr_node* recvips;

  /**
   * @var ddsi_locator_t extmask
   * @brief 用于发现消息中的广告IP地址（以便在NAT上的外部IP地址可能被广告），以及DDSI多播地址。
   * Addressing: actual own (preferred) IP address, IP address advertised in discovery messages (so
   * that an external IP address on a NAT may be advertised), and the DDSI multi-cast address.
   */
  ddsi_locator_t extmask;

  /* Locators */

  // 定义 SPDP（简单参与者发现协议）多播地址 (Define the Simple Participant Discovery Protocol
  // multicast address)
  ddsi_locator_t loc_spdp_mc;

  // 定义元数据多播地址 (Define the metadata multicast address)
  ddsi_locator_t loc_meta_mc;

  // 定义元数据单播地址 (Define the metadata unicast address)
  ddsi_locator_t loc_meta_uc;

  // 定义默认多播地址 (Define the default multicast address)
  ddsi_locator_t loc_default_mc;

  // 定义默认单播地址 (Define the default unicast address)
  ddsi_locator_t loc_default_uc;

#ifdef DDS_HAS_SHM
  // 如果定义了 DDS_HAS_SHM，则定义 Iceoryx 地址 (If DDS_HAS_SHM is defined, define the Iceoryx
  // address)
  ddsi_locator_t loc_iceoryx_addr;
#endif

  /**
   * @brief 初始发现地址集和当前发现地址集。这些地址是 SPDP ping 发送到的地址。
   *        Initial discovery address set, and the current discovery address
   *        set. These are the addresses that SPDP pings get sent to.
   */
  struct ddsi_addrset* as_disc;

  /**
   * @brief 互斥锁，用于同步对共享资源的访问。
   *        Mutex for synchronizing access to shared resources.
   */
  ddsrt_mutex_t lock;

/**
 * @brief 接收线程。(我们目前只能有一个，因为信号触发套接字。) 接收缓冲池是每个接收线程的，
 *        它只是一个全局变量，因为它需要在接收线程本身终止后很久才释放。
 *        Receive thread. (We can only has one for now, cos of the signal
 *        trigger socket.) Receive buffer pool is per receive thread,
 *        it is only a global variable because it needs to be freed way later
 *        than the receive thread itself terminates.
 */
#define MAX_RECV_THREADS 3
  uint32_t n_recv_threads;
  struct recv_thread {
    const char* name;
    struct ddsi_thread_state* thrst;
    struct ddsi_recv_thread_arg arg;
  } recv_threads[MAX_RECV_THREADS];

  /**
   * @brief 基于连接传输的监听器线程。
   *        Listener thread for connection based transports.
   */
  struct ddsi_thread_state* listen_ts;

  /**
   * @brief 当停止时清除标志(接收线程)。FIXME。
   *        Flag cleared when stopping (receive threads). FIXME.
   */
  ddsrt_atomic_uint32_t rtps_keepgoing;

  /**
   * @brief DDSI 服务的开始时间，用于记录相对时间戳。
   *        Start time of the DDSI service, for logging relative time stamps.
   */
  ddsrt_wctime_t tstart;

  /**
   * @brief 参与者、读者和写者的默认 QoS (需要消除传出发现包中的默认值，
   *        并为传入发现包中缺失的 QoS 设置提供值); 以及内置端点所需的实际 QoS。
   *        Default QoSs for participant, readers and writers (needed for
   *        eliminating default values in outgoing discovery packets, and for
   *        supplying values for missing QoS settings in incoming discovery
   *        packets); plus the actual QoSs needed for the builtin
   *        endpoints.
   */
  dds_qos_t default_local_xqos_pp;
  dds_qos_t spdp_endpoint_xqos;
  dds_qos_t builtin_endpoint_xqos_rd;
  dds_qos_t builtin_endpoint_xqos_wr;
#ifdef DDS_HAS_TYPE_DISCOVERY
  dds_qos_t builtin_volatile_xqos_rd;
  dds_qos_t builtin_volatile_xqos_wr;
#endif
#ifdef DDS_HAS_SECURITY
  dds_qos_t builtin_secure_volatile_xqos_rd;
  dds_qos_t builtin_secure_volatile_xqos_wr;
  dds_qos_t builtin_stateless_xqos_rd;
  dds_qos_t builtin_stateless_xqos_wr;
#endif

  /* SPDP 包需要特殊处理（它们是我们接受的来自未知写入者的唯一包），
     并且有它们自己的无操作碎片重组和重新排序设备，以及一个全局互斥锁，
     代替代理写入者锁。 */
  /* SPDP packets get very special treatment (they're the only packets
     we accept from writers we don't know) and have their very own
     do-nothing defragmentation and reordering thingummies, as well as a
     global mutex to in lieu of the proxy writer lock. */
  ddsrt_mutex_t spdp_lock;
  struct ddsi_defrag* spdp_defrag;
  struct ddsi_reorder* spdp_reorder;

  /* 除 SPDP 外的内置内容通过内置传递队列进行；
     目前仅为 SEDP 和 PMD */
  /* Built-in stuff other than SPDP gets funneled through the builtins
     delivery queue; currently just SEDP and PMD */
  struct ddsi_dqueue* builtins_dqueue;

  struct ddsi_debug_monitor* debmon;

  uint32_t networkQueueId;
  struct ddsi_thread_state* channel_reader_thrst;

  /* 应用程序数据具有自己的传递队列 */
  /* Application data gets its own delivery queue */
  struct ddsi_dqueue* user_dqueue;

  /* 发送端：传输队列池 */
  /* Transmit side: pool for transmit queue*/
  struct ddsi_xmsgpool* xmsgpool;
  struct ddsi_sertype* spdp_type;           /* key = participant GUID */
  struct ddsi_sertype* sedp_reader_type;    /* key = endpoint GUID */
  struct ddsi_sertype* sedp_writer_type;    /* key = endpoint GUID */
  struct ddsi_sertype* sedp_topic_type;     /* key = topic GUID */
  struct ddsi_sertype* pmd_type;            /* participant message data */
#ifdef DDS_HAS_TYPE_DISCOVERY
  struct ddsi_sertype* tl_svc_request_type; /* TypeLookup service request, no key */
  struct ddsi_sertype* tl_svc_reply_type;   /* TypeLookup service reply, no key */
#endif
#ifdef DDS_HAS_SECURITY
  struct ddsi_sertype* spdp_secure_type;        /* key = participant GUID */
  struct ddsi_sertype* sedp_reader_secure_type; /* key = endpoint GUID */
  struct ddsi_sertype* sedp_writer_secure_type; /* key = endpoint GUID */
  struct ddsi_sertype* pmd_secure_type;         /* participant message data */
  struct ddsi_sertype* pgm_stateless_type;      /* participant generic message */
  struct ddsi_sertype* pgm_volatile_type;       /* participant generic message */
#endif

  // 定义发送队列锁，用于保护发送队列的访问
  // Define send queue lock, used to protect access to the send queue
  ddsrt_mutex_t sendq_lock;

  // 定义发送队列条件变量，用于通知发送线程有新数据可发送
  // Define send queue condition variable, used to notify the send thread that there is new data to
  // send
  ddsrt_cond_t sendq_cond;

  // 发送队列长度
  // Send queue length
  unsigned sendq_length;

  // 发送队列头指针
  // Send queue head pointer
  struct ddsi_xpack* sendq_head;

  // 发送队列尾指针
  // Send queue tail pointer
  struct ddsi_xpack* sendq_tail;

  // 发送队列停止标志，用于通知发送线程退出
  // Send queue stop flag, used to notify the send thread to exit
  int sendq_stop;

  // 发送队列线程状态
  // Send queue thread state
  struct ddsi_thread_state* sendq_ts;

  // 发送队列运行状态
  // Send queue running status
  bool sendq_running;

  // 发送队列运行状态锁
  // Send queue running status lock
  ddsrt_mutex_t sendq_running_lock;

  // 用于存储捕获的数据包的文件，如果禁用则为 NULL
  // File for storing captured packets, NULL if disabled
  FILE* pcap_fp;

  // pcap 文件锁
  // pcap file lock
  ddsrt_mutex_t pcap_lock;

  // 内置主题接口
  // Built-in topic interface
  struct ddsi_builtin_topic_interface* builtin_topic_interface;

  // 组成员关系
  // Group membership
  struct ddsi_mcgroup_membership* mship;

  // 序列化类型锁
  // Serialization types lock
  ddsrt_mutex_t sertypes_lock;

  // 序列化类型哈希表
  // Serialization types hash table
  struct ddsrt_hh* sertypes;

#ifdef DDS_HAS_TYPE_DISCOVERY
  // 类型库锁
  // Type library lock
  ddsrt_mutex_t typelib_lock;

  // 类型库 AVL 树
  // Type library AVL tree
  ddsrt_avl_tree_t typelib;

  // 类型依赖 AVL 树
  // Type dependencies AVL tree
  ddsrt_avl_tree_t typedeps;

  // 类型依赖反向 AVL 树
  // Type dependencies reverse AVL tree
  ddsrt_avl_tree_t typedeps_reverse;

  // 类型库已解析条件变量
  // Type library resolved condition variable
  ddsrt_cond_t typelib_resolved_cond;
#endif

#ifdef DDS_HAS_TOPIC_DISCOVERY
  // 主题定义锁
  // Topic definitions lock
  ddsrt_mutex_t topic_defs_lock;

  // 主题定义哈希表
  // Topic definitions hash table
  struct ddsrt_hh* topic_defs;
#endif

  // 新主题锁
  // New topic lock
  ddsrt_mutex_t new_topic_lock;

  // 新主题条件变量
  // New topic condition variable
  ddsrt_cond_t new_topic_cond;

  // 新主题版本
  // New topic version
  uint32_t new_topic_version;

// 安全相关全局变量
// Security globals
#ifdef DDS_HAS_SECURITY
  // 安全上下文
  // Security context
  struct dds_security_context* security_context;

  // 握手管理
  // Handshake administration
  struct ddsi_hsadmin* hsadmin;

  // 握手是否包含可选字段
  // Whether handshake includes optional fields
  bool handshake_include_optional;
#endif

  // 命名相关
  // Naming related
  ddsrt_mutex_t naming_lock;

  // 命名随机数生成器
  // Naming random number generator
  ddsrt_prng_t naming_rng;
};

#if defined(__cplusplus)
}
#endif

#endif /* DDSI_DOMAINGV_H */
