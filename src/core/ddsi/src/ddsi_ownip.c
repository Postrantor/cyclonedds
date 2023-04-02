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
#include <ctype.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "dds/ddsi/ddsi_domaingv.h"
#include "dds/ddsi/ddsi_feature_check.h"
#include "dds/ddsi/ddsi_log.h"
#include "dds/ddsi/ddsi_unused.h"
#include "dds/ddsrt/avl.h"
#include "dds/ddsrt/heap.h"
#include "dds/ddsrt/ifaddrs.h"
#include "dds/ddsrt/md5.h"
#include "dds/ddsrt/sockets.h"
#include "dds/ddsrt/string.h"
#include "ddsi__addrset.h" /* unspec locator */
#include "ddsi__ipaddr.h"
#include "ddsi__misc.h"
#include "ddsi__ownip.h"
#include "ddsi__tran.h"

#ifdef __linux
/* FIMXE: HACK HACK */
#include <linux/if_packet.h>
#endif

/**
 * @enum find_interface_result
 * @brief 枚举类型，表示查找接口的结果（Enumeration type representing the result of finding an
 * interface）
 */
enum find_interface_result {
  FIR_OK,       /**< 查找成功（Found successfully） */
  FIR_NOTFOUND, /**< 未找到（Not found） */
  FIR_INVALID   /**< 无效（Invalid） */
};

/**
 * @brief 通过名称查找网络接口（Find a network interface by its name）
 *
 * @param[in] reqname 请求的接口名称（The requested interface name）
 * @param[in] n_interfaces 网络接口数组中的接口数量（Number of interfaces in the network interface
 * array）
 * @param[in] interfaces 网络接口数组（Array of network interfaces）
 * @param[out] match 匹配到的接口在数组中的索引（Index of the matched interface in the array）
 * @return 查找结果（Result of the search）
 * @retval FIR_OK 成功找到接口（Interface found successfully）
 * @retval FIR_NOTFOUND 未找到指定名称的接口（Interface with specified name not found）
 * @retval FIR_INVALID 无效参数或其他错误（Invalid arguments or other errors）
 */
static enum find_interface_result find_interface_by_name(
    const char *reqname,
    size_t n_interfaces,
    const struct ddsi_network_interface *interfaces,
    size_t *match) {
  // 遍历所有接口（Iterate through all interfaces）
  for (size_t k = 0; k < n_interfaces; k++) {
    // 比较接口名称是否与请求名称相同（Compare if the interface name is the same as the requested
    // name）
    if (strcmp(reqname, interfaces[k].name) == 0) {
      // 设置匹配到的接口索引（Set the index of the matched interface）
      *match = k;
      // 返回查找成功（Return that the search was successful）
      return FIR_OK;
    }
  }
  // 如果没有找到匹配的接口，返回未找到（If no matching interface is found, return not found）
  return FIR_NOTFOUND;
}

/**
 * @brief 根据地址查找网络接口 (Find network interface by address)
 *
 * @param[in] gv 域全局变量 (Domain global variables)
 * @param[in] reqip 请求的IP地址字符串 (Requested IP address string)
 * @param[in] n_interfaces 网络接口数组的长度 (Length of the network interfaces array)
 * @param[in] interfaces 网络接口数组 (Network interfaces array)
 * @param[out] match 匹配到的网络接口在数组中的索引 (Index of the matched network interface in the
 * array)
 * @return 查找结果，包括：FIR_INVALID（无效地址）、FIR_OK（找到匹配的接口）和
 * FIR_NOTFOUND（未找到匹配的接口）(Find result, including: FIR_INVALID (invalid address), FIR_OK
 * (matched interface found) and FIR_NOTFOUND (no matched interface found))
 */
static enum find_interface_result find_interface_by_address(
    const struct ddsi_domaingv *gv,
    const char *reqip,
    size_t n_interfaces,
    const struct ddsi_network_interface *interfaces,
    size_t *match) {
  // 尝试根据地址进行匹配 (Try matching on address)
  ddsi_locator_t req;
  // 将请求的IP地址字符串转换为ddsi_locator_t结构体 (Convert the requested IP address string to a
  // ddsi_locator_t structure)
  if (ddsi_locator_from_string(gv, &req, reqip, gv->m_factory) != AFSR_OK) {
    return FIR_INVALID;
  }
  // 尝试根据地址进行精确匹配 (Try an exact match on the address)
  for (size_t k = 0; k < n_interfaces; k++) {
    // 比较两个地址是否相等 (Compare whether the two addresses are equal)
    if (ddsi_compare_locators(&interfaces[k].loc, &req) == 0) {
      *match = k;
      return FIR_OK;
    }
  }

  // 对于IPv4，尝试仅匹配网络部分，其中网络部分基于正在考虑的接口的子网掩码 (For IPv4, try matching
  // on network portion only, where the network portion is based on the netmask of the interface
  // under consideration)
  if (req.kind == DDSI_LOCATOR_KIND_UDPv4) {
    for (size_t k = 0; k < n_interfaces; k++) {
      // 跳过非UDPv4类型的接口 (Skip non-UDPv4 type interfaces)
      if (interfaces[k].loc.kind != DDSI_LOCATOR_KIND_UDPv4) continue;
      uint32_t req1, ip1, nm1;
      // 提取请求地址、接口地址和子网掩码的后四个字节 (Extract the last four bytes of the requested
      // address, interface address and subnet mask)
      memcpy(&req1, req.address + 12, sizeof(req1));
      memcpy(&ip1, interfaces[k].loc.address + 12, sizeof(ip1));
      memcpy(&nm1, interfaces[k].netmask.address + 12, sizeof(nm1));

      // 如果请求地址的主机部分不为零，则跳过此接口 (If the host portion of the requested address is
      // non-zero, skip this interface)
      if (req1 & ~nm1) continue;

      // 如果请求地址和接口地址的网络部分相同，则匹配成功 (If the network portion of the requested
      // address and interface address are the same, the match is successful)
      if ((req1 & nm1) == (ip1 & nm1)) {
        *match = k;
        return FIR_OK;
      }
    }
  }
  // 未找到匹配的接口 (No matched interface found)
  return FIR_NOTFOUND;
}

/**
 * @brief 网络接口优先级结构体 (Network interface priority structure)
 *
 * @param match 匹配到的网络接口在数组中的索引 (Index of the matched network interface in the array)
 * @param priority 优先级值，数值越大优先级越高 (Priority value, the larger the value, the higher
 * the priority)
 */
struct interface_priority {
  size_t match;
  int32_t priority;
};

/**
 * @brief 比较两个接口优先级的函数 (Compare function for interface priorities)
 * @param va 第一个接口优先级结构体指针 (Pointer to the first interface_priority structure)
 * @param vb 第二个接口优先级结构体指针 (Pointer to the second interface_priority structure)
 * @return 返回比较结果，0表示相等，1表示第一个优先级低于第二个，-1表示第一个优先级高于第二个
 * (Returns 0 if equal, 1 if the first priority is lower than the second, -1 if the first priority
 * is higher than the second)
 */
static int compare_interface_priority(const void *va, const void *vb) {
  // 比较函数用于按降序优先级排序 (compare function is used for sorting in descending priority
  // order)

  // 将void指针转换为interface_priority结构体指针 (Convert void pointers to interface_priority
  // structure pointers)
  const struct interface_priority *a = va;
  const struct interface_priority *b = vb;

  // 如果优先级相等返回0，如果a的优先级小于b的优先级返回1，否则返回-1 (Return 0 if priorities are
  // equal, return 1 if a's priority is less than b's priority, otherwise return -1)
  return (a->priority == b->priority) ? 0 : (a->priority < b->priority) ? 1 : -1;
}

// 定义maybe_add_interface_result枚举类型，包含三个值：MAI_IGNORED, MAI_ADDED, MAI_OUT_OF_MEMORY
// (Define the maybe_add_interface_result enumeration with three values: MAI_IGNORED, MAI_ADDED,
// MAI_OUT_OF_MEMORY)
enum maybe_add_interface_result {
  MAI_IGNORED,       // 表示接口被忽略 (Indicates that the interface is ignored)
  MAI_ADDED,         // 表示接口已添加 (Indicates that the interface has been added)
  MAI_OUT_OF_MEMORY  // 表示内存不足 (Indicates out of memory)
};

/**
 * @brief 尝试添加网络接口，返回结果表示是否成功添加。
 * @param[in] gv 指向 ddsi_domaingv 结构体的指针
 * @param[out] dst 指向 ddsi_network_interface 结构体的指针，用于存储网络接口信息
 * @param[in] ifa 指向 ddsrt_ifaddrs_t 结构体的指针，包含要添加的网络接口的信息
 * @param[out] qout 用于存储网络接口质量评分的整数指针
 * @return maybe_add_interface_result 枚举类型，表示添加网络接口的结果
 *
 * @brief Attempt to add a network interface, returning the result indicating whether it was
 * successfully added.
 * @param[in] gv Pointer to the ddsi_domaingv structure
 * @param[out] dst Pointer to the ddsi_network_interface structure for storing network interface
 * information
 * @param[in] ifa Pointer to the ddsrt_ifaddrs_t structure containing information about the network
 * interface to be added
 * @param[out] qout Integer pointer for storing the network interface quality score
 * @return maybe_add_interface_result Enumeration type indicating the result of adding the network
 * interface
 */
static enum maybe_add_interface_result maybe_add_interface(struct ddsi_domaingv *const gv,
                                                           struct ddsi_network_interface *dst,
                                                           const ddsrt_ifaddrs_t *ifa,
                                                           int *qout) {
  // 返回质量评分 >= 0 表示已添加，< 0 表示未添加
  // Returns quality >= 0 if added, < 0 otherwise
  char addrbuf[DDSI_LOCSTRLEN];
  int q = 0;

  // 接口必须是启用状态
  // Interface must be up
  if ((ifa->flags & IFF_UP) == 0) {
    GVLOG(DDS_LC_CONFIG, " (interface down)");
    return MAI_IGNORED;
  } else if (ddsrt_sockaddr_isunspecified(ifa->addr)) {
    GVLOG(DDS_LC_CONFIG, " (address unspecified)");
    return MAI_IGNORED;
  }

  switch (ifa->type) {
    case DDSRT_IFTYPE_WIFI:
      GVLOG(DDS_LC_CONFIG, " wireless");
      break;
    case DDSRT_IFTYPE_WIRED:
      GVLOG(DDS_LC_CONFIG, " wired");
      break;
    case DDSRT_IFTYPE_UNKNOWN:
      break;
  }

  if (ddsi_locator_from_sockaddr(gv->m_factory, &dst->loc, ifa->addr) < 0) return MAI_IGNORED;
  ddsi_locator_to_string_no_port(addrbuf, sizeof(addrbuf), &dst->loc);
  GVLOG(DDS_LC_CONFIG, " %s(", addrbuf);

  bool link_local = false;
  bool loopback = false;
  if (ifa->flags & IFF_LOOPBACK) {
    // 回环设备的优先级低于其他可用接口，因为其他接口至少原则上允许与其他机器通信。
    // Loopback device has the lowest priority of every interface available,
    // because the other interfaces at least in principle allow communicating with other machines.
    loopback = true;
    q += 0;
#if DDSRT_HAVE_IPV6
    if (ifa->addr->sa_family == AF_INET6 &&
        IN6_IS_ADDR_LINKLOCAL(&((struct sockaddr_in6 *)ifa->addr)->sin6_addr))
      link_local = true;
    else
      q += 1;
#endif
  } else {
#if DDSRT_HAVE_IPV6
    // 我们接受链路本地 IPv6 地址，但具有链路本地地址的接口在排序中会低于具有全局地址的接口。
    // 当被迫使用链路本地地址时，我们仅限于在该接口上操作，并假定任何广告（传入）链路本地地址属于该接口。
    // We accept link-local IPv6 addresses, but an interface with a link-local address will
    // end up lower in the ordering than one with a global address. When forced to use a
    // link-local address, we restrict ourselves to operating on that one interface only and
    // assume any advertised (incoming) link-local address belongs to that interface.
    if (ifa->addr->sa_family == AF_INET6 &&
        IN6_IS_ADDR_LINKLOCAL(&((struct sockaddr_in6 *)ifa->addr)->sin6_addr))
      link_local = true;
    else
      q += 5;
#endif

    // 我们强烈倾向于能够组播的接口，如果没有可用的组播接口，那么任何非点对点的接口都可以，
    // 否则我们希望 IP 路由能解决问题。
    // We strongly prefer a multicast capable interface, if that's not available anything
    // that's not point-to-point, or else we hope IP routing will take care of the issues.
    if (ifa->flags & IFF_MULTICAST)
      q += 4;
    else if (!(ifa->flags & IFF_POINTOPOINT))
      q += 3;
    else
      q += 2;
  }

  GVLOG(DDS_LC_CONFIG, "q%d)", q);

  if (ifa->addr->sa_family == AF_INET && ifa->netmask) {
    if (ddsi_locator_from_sockaddr(gv->m_factory, &dst->netmask, ifa->netmask) < 0)
      return MAI_IGNORED;
  } else {
    dst->netmask.kind = dst->loc.kind;
    dst->netmask.port = DDSI_LOCATOR_PORT_INVALID;
    memset(&dst->netmask.address, 0, sizeof(dst->netmask.address));
  }
  // 默认情况下，外部（即在发现中宣告的）地址为实际接口地址。这可以通过配置进行覆盖。
  // Default external (i.e., advertised in discovery) address to the actual interface
  // address.  This can subsequently be overridden by the configuration.
  dst->extloc = dst->loc;
  dst->mc_capable = ((ifa->flags & IFF_MULTICAST) != 0);
  dst->mc_flaky = ((ifa->type == DDSRT_IFTYPE_WIFI) != 0);
  dst->point_to_point = ((ifa->flags & IFF_POINTOPOINT) != 0);
  dst->loopback = loopback ? 1 : 0;
  dst->link_local = link_local ? 1 : 0;
  dst->if_index = ifa->index;
  if ((dst->name = ddsrt_strdup(ifa->name)) == NULL) return MAI_OUT_OF_MEMORY;
  dst->priority = loopback ? 2 : 0;
  dst->prefer_multicast = 0;
  *qout = q;
  return MAI_ADDED;
}

/**
 * @brief 收集网络接口信息 (Gather network interface information)
 *
 * @param[in] gv 域全局变量指针 (Pointer to domain global variables)
 * @param[out] n_interfaces 网络接口数量指针 (Pointer to the number of network interfaces)
 * @param[out] interfaces 网络接口结构体数组指针 (Pointer to the array of network interface
 * structures)
 * @param[out] maxq_count 最大质量接口数量指针 (Pointer to the count of maximum quality interfaces)
 * @param[out] maxq_list 最大质量接口索引列表指针 (Pointer to the list of indices of maximum quality
 * interfaces)
 *
 * @return 成功返回 true，失败返回 false (Return true on success, false on failure)
 */
static bool gather_interfaces(struct ddsi_domaingv *const gv,
                              size_t *n_interfaces,
                              struct ddsi_network_interface **interfaces,
                              size_t *maxq_count,
                              size_t **maxq_list) {
  ddsrt_ifaddrs_t *ifa_root = NULL;
  // 枚举网络接口 (Enumerate network interfaces)
  const int ret =
      ddsi_enumerate_interfaces(gv->m_factory, gv->config.transport_selector, &ifa_root);
  if (ret < 0) {
    GVERROR("failed to enumerate interfaces for \"%s\": %d\n", gv->m_factory->m_typename, ret);
    return false;
  }

  *maxq_count = 0;
  *n_interfaces = 0;
  size_t max_interfaces = 8;
  *interfaces = NULL;
  // 分配内存 (Allocate memory)
  if ((*maxq_list = ddsrt_malloc(max_interfaces * sizeof(**maxq_list))) == NULL ||
      (*interfaces = ddsrt_malloc(max_interfaces * sizeof(**interfaces))) == NULL) {
    goto fail;
  }
  const char *last_if_name = "";
  const char *sep = " ";
  int quality = -1;
  // 遍历网络接口 (Iterate through network interfaces)
  for (ddsrt_ifaddrs_t *ifa = ifa_root; ifa != NULL; ifa = ifa->next) {
    if (strcmp(ifa->name, last_if_name)) GVLOG(DDS_LC_CONFIG, "%s%s", sep, ifa->name);
    last_if_name = ifa->name;

    // 扩展数组大小 (Expand array size)
    if (*n_interfaces == max_interfaces) {
      max_interfaces *= 2;
      size_t *new_maxq_list;
      if ((new_maxq_list = ddsrt_realloc(*maxq_list, max_interfaces * sizeof(**maxq_list))) == NULL)
        goto fail;
      *maxq_list = new_maxq_list;
      struct ddsi_network_interface *new_interfaces;
      if ((new_interfaces = ddsrt_realloc(*interfaces, max_interfaces * sizeof(**interfaces))) ==
          NULL)
        goto fail;
      *interfaces = new_interfaces;
    }

    int q;
    // 尝试添加网络接口 (Attempt to add the network interface)
    switch (maybe_add_interface(gv, &(*interfaces)[*n_interfaces], ifa, &q)) {
      case MAI_IGNORED:
        break;
      case MAI_OUT_OF_MEMORY:
        goto fail;
      case MAI_ADDED:
        // 更新最大质量接口列表 (Update the list of maximum quality interfaces)
        if (q == quality) {
          (*maxq_list)[(*maxq_count)++] = *n_interfaces;
        } else if (q > quality) {
          (*maxq_list)[0] = *n_interfaces;
          *maxq_count = 1;
          quality = q;
        }
        (*n_interfaces)++;
        break;
    }
  }
  GVLOG(DDS_LC_CONFIG, "\n");
  ddsrt_freeifaddrs(ifa_root);
  assert((*n_interfaces > 0) == (*maxq_count > 0));
  if (*n_interfaces == 0) {
    GVERROR("failed to find interfaces for \"%s\"\n", gv->m_factory->m_typename);
    goto fail;
  }
  return true;

fail:
  // 释放内存 (Free memory)
  ddsrt_free(*maxq_list);
  ddsrt_free(*interfaces);
  return false;
}

/**
 * @brief 匹配配置接口 (Match configuration interface)
 *
 * @param[in] gv 指向ddsi_domaingv结构的指针 (Pointer to a ddsi_domaingv structure)
 * @param[in] n_interfaces 网络接口数量 (Number of network interfaces)
 * @param[in] interfaces 网络接口数组 (Array of network interfaces)
 * @param[in] iface 配置网络接口列表元素 (Configuration network interface list element)
 * @param[out] match 匹配的网络接口索引 (Index of the matched network interface)
 * @param[in] required 是否需要匹配成功 (Whether the match is required to succeed)
 * @return 如果匹配成功返回true，否则返回false (Returns true if the match is successful, false
 * otherwise)
 */
static bool match_config_interface(struct ddsi_domaingv *const gv,
                                   size_t n_interfaces,
                                   struct ddsi_network_interface const *const interfaces,
                                   const struct ddsi_config_network_interface_listelem *iface,
                                   size_t *match,
                                   bool required) {
  // 检查名称是否存在 (Check if the name exists)
  const bool has_name = iface->cfg.name != NULL && iface->cfg.name[0] != '\0';
  // 检查地址是否存在 (Check if the address exists)
  const bool has_address = iface->cfg.address != NULL && iface->cfg.address[0] != '\0';
  *match = SIZE_MAX;
  // 如果名称和地址都存在 (If both name and address exist)
  if (has_name && has_address) {
    // 用户指定的名称和IP必须匹配相同的接口 (The user-specified name and IP must match the same
    // interface)
    size_t name_match = SIZE_MAX;
    size_t address_match = SIZE_MAX;
    enum find_interface_result name_result =
        find_interface_by_name(iface->cfg.name, n_interfaces, interfaces, &name_match);
    enum find_interface_result address_result =
        find_interface_by_address(gv, iface->cfg.address, n_interfaces, interfaces, &address_match);

    // 如果名称或地址匹配失败 (If the name or address match fails)
    if (name_result != FIR_OK || address_result != FIR_OK) {
      if (required) {
        GVERROR("%s/%s: does not match an available interface\n", iface->cfg.name,
                iface->cfg.address);
        return false;
      }

      GVWARNING("%s/%s: optional interface was not found.\n", iface->cfg.name, iface->cfg.address);
      return true;
    } else if (name_match != address_match) {
      GVERROR("%s/%s: do not match the same interface\n", iface->cfg.name, iface->cfg.address);
      return false;
    } else {
      *match = name_match;
    }
  } else if (has_name) {  // 如果只有名称存在 (If only the name exists)
    enum find_interface_result name_result =
        find_interface_by_name(iface->cfg.name, n_interfaces, interfaces, match);
    if (name_result != FIR_OK) {
      if (required) {
        GVERROR("%s: does not match an available interface.\n", iface->cfg.name);
        return false;
      }
      GVWARNING("%s: optional interface was not found.\n", iface->cfg.name);
      return true;
    }
  } else if (has_address) {  // 如果只有地址存在 (If only the address exists)
    enum find_interface_result address_result =
        find_interface_by_address(gv, iface->cfg.address, n_interfaces, interfaces, match);
    if (address_result != FIR_OK) {
      if (required) {
        GVERROR("%s: does not match an available interface\n", iface->cfg.address);
        return false;
      }
      GVWARNING("%s: optional interface was not found.\n", iface->cfg.address);
      return true;
    }
  } else {  // 如果名称和地址都不存在 (If both name and address do not exist)
    GVERROR("Nameless and address-less interface listed in interfaces.\n");
    return false;
  }
  return true;
}

/**
 * @brief 添加匹配的网络接口到已匹配列表中 (Add a matching network interface to the list of matched
 * interfaces)
 *
 * @param[in] gv        ddsi_domaingv 结构体指针 (Pointer to ddsi_domaingv structure)
 * @param[out] matches  匹配的网络接口优先级数组 (Array of matched network interface priorities)
 * @param[in,out] num_matches 当前匹配的网络接口数量 (Current number of matched network interfaces)
 * @param[in] act_iface 实际网络接口信息 (Actual network interface information)
 * @param[in] xx_idx    配置文件中的网络接口索引 (Index of network interface in configuration file)
 * @param[in] cfg_iface 配置文件中的网络接口信息 (Network interface information from configuration
 * file)
 * @return 成功添加返回 true，否则返回 false (Returns true if successfully added, otherwise returns
 * false)
 */
static bool add_matching_interface(struct ddsi_domaingv *gv,
                                   struct interface_priority *matches,
                                   size_t *num_matches,
                                   struct ddsi_network_interface *act_iface,
                                   size_t xx_idx,
                                   const struct ddsi_config_network_interface_listelem *cfg_iface) {
  // 遍历已匹配的网络接口 (Iterate through the matched network interfaces)
  for (size_t i = 0; i < *num_matches; i++) {
    // 如果找到相同的接口，则返回错误 (If the same interface is found, return an error)
    if (matches[i].match == xx_idx) {
      GVERROR("%s: the same interface may not be selected twice\n", act_iface->name);
      return false;
    }
  }

  // 设置实际网络接口的多播首选项 (Set the multicast preference of the actual network interface)
  act_iface->prefer_multicast = ((unsigned)cfg_iface->cfg.prefer_multicast) & 1;

  // 如果优先级不是默认值，则设置实际网络接口的优先级 (If priority is not default, set the priority
  // of the actual network interface)
  if (!cfg_iface->cfg.priority.isdefault) act_iface->priority = cfg_iface->cfg.priority.value;

  // 根据配置文件中的多播设置，更新实际网络接口的多播能力 (Update the multicast capability of the
  // actual network interface based on the multicast setting in the configuration file)
  if (cfg_iface->cfg.multicast != DDSI_BOOLDEF_DEFAULT) {
    if (act_iface->mc_capable && cfg_iface->cfg.multicast == DDSI_BOOLDEF_FALSE) {
      GVLOG(DDS_LC_CONFIG, "disabling multicast on interface %s.", act_iface->name);
      act_iface->mc_capable = 0;
    } else if (!act_iface->mc_capable && cfg_iface->cfg.multicast == DDSI_BOOLDEF_TRUE) {
      GVLOG(DDS_LC_CONFIG, "assuming multicast capable interface %s.", act_iface->name);
      act_iface->mc_capable = 1;
    }
  }

  // 如果已匹配的网络接口数量达到最大值，则返回错误 (If the number of matched network interfaces
  // reaches the maximum, return an error)
  if (*num_matches == MAX_XMIT_CONNS) {
    GVERROR("too many interfaces specified\n");
    return false;
  }
  // 将新匹配的网络接口添加到匹配列表中，并更新匹配数量 (Add the newly matched network interface to
  // the list of matches and update the number of matches)
  matches[*num_matches].match = xx_idx;
  matches[*num_matches].priority = act_iface->priority;
  (*num_matches)++;
  return true;
}

/**
 * @brief 打印选择的网络接口信息
 *        Log the selected network interface information.
 *
 * @param[in] gv 指向 ddsi_domaingv 结构体的指针
 *               Pointer to the ddsi_domaingv structure.
 * @param[in] interfaces 网络接口结构体数组
 *                       Array of network interface structures.
 * @param[in] maxq_list 最大质量列表，包含最佳网络接口的索引
 *                      Max quality list, containing indices of the best network interfaces.
 * @param[in] maxq_count 最大质量列表中的元素数量
 *                       Number of elements in the max quality list.
 */
static void log_arbitrary_selection(struct ddsi_domaingv *gv,
                                    const struct ddsi_network_interface *interfaces,
                                    const size_t *maxq_list,
                                    size_t maxq_count) {
  // 定义地址缓冲区
  // Define address buffer
  char addrbuf[DDSI_LOCSTRLEN];

  // 获取最佳网络接口的索引
  // Get the index of the best network interface
  const size_t idx = maxq_list[0];

  // 将选定网络接口的地址转换为字符串（不包括端口）
  // Convert the address of the selected network interface to a string (excluding port)
  ddsi_locator_to_string_no_port(addrbuf, sizeof(addrbuf), &interfaces[idx].loc);

  // 打印选定网络接口的名称和地址
  // Print the name and address of the selected network interface
  GVLOG(DDS_LC_INFO,
        "using network interface %s (%s) selected arbitrarily from: ", interfaces[idx].name,
        addrbuf);

  // 遍历最大质量列表，打印所有最佳网络接口的名称
  // Iterate through the max quality list, printing the names of all the best network interfaces
  for (size_t i = 0; i < maxq_count; i++)
    GVLOG(DDS_LC_INFO, "%s%s", (i == 0) ? "" : ", ", interfaces[maxq_list[i]].name);

  // 打印换行符
  // Print a newline character
  GVLOG(DDS_LC_INFO, "\n");
}

/**
 * @brief 在 Cyclone DDS 项目中查找本地 IP 地址的函数。
 *        Function to find the local IP address in the Cyclone DDS project.
 *
 * @param[in] gv 指向 ddsi_domaingv 结构体的指针，用于存储相关配置和状态信息。
 *              Pointer to the ddsi_domaingv structure, used to store related configuration and
 * state information.
 * @return 成功时返回 1，失败时返回 0。
 *         Returns 1 on success, 0 on failure.
 */
int ddsi_find_own_ip(struct ddsi_domaingv *gv) {
  // 定义一个缓冲区，用于存储地址字符串
  // Define a buffer to store the address string
  char addrbuf[DDSI_LOCSTRLEN];

  // 记录接口信息
  // Log interface information
  GVLOG(DDS_LC_CONFIG, "interfaces:");

  size_t n_interfaces, maxq_count, *maxq_list;
  struct ddsi_network_interface *interfaces;
  // 收集网络接口信息
  // Gather network interface information
  if (!gather_interfaces(gv, &n_interfaces, &interfaces, &maxq_count, &maxq_list)) return 0;

#ifndef NDEBUG
  // 断言我们可以使用 `int` 标识所有接口，这样我们就可以将 size_t 类型转换为 int 类型
  // Assert that we can identify all interfaces with an `int`, so we can then just cast the size_t's
  // to int's
  assert(n_interfaces <= (size_t)INT_MAX);
  assert(maxq_count <= n_interfaces);
  for (size_t i = 0; i < maxq_count; i++) assert(maxq_list[i] < n_interfaces);
#endif

  bool ok = true;
  gv->n_interfaces = 0;

  // 检查接口数量是否大于 0 和最大队列计数是否大于 0
  // Check if the number of interfaces is greater than 0 and the maximum queue count is greater than
  // 0
  assert(n_interfaces > 0 && maxq_count > 0);
  if (gv->config.network_interfaces == NULL) {
    if (maxq_count > 1) log_arbitrary_selection(gv, interfaces, maxq_list, maxq_count);
    gv->n_interfaces = 1;
    gv->interfaces[0] = interfaces[maxq_list[0]];
    if ((gv->interfaces[0].name = ddsrt_strdup(gv->interfaces[0].name)) == NULL) ok = false;
  } else  // 获取优先级设置
  {
    /**
     * @param[in] n_interfaces 网络接口数量
     * @param[in] interfaces 网络接口数组
     * @param[in] gv 全局变量结构体指针
     * @return 无返回值
     */
    size_t num_matches =
        0;  // 初始化匹配接口的数量为0 / Initialize the number of matched interfaces to 0
    size_t maxq_index = 0;  // 初始化最大质量索引为0 / Initialize the maximum quality index to 0
    struct interface_priority *matches = ddsrt_malloc(
        n_interfaces *
        sizeof(*matches));  // 为匹配数组分配内存 / Allocate memory for the matches array
    if (matches == NULL)
      ok = false;  // 如果分配失败，设置ok为false / If allocation fails, set ok to false

    // 遍历配置文件中的网络接口 / Iterate through the network interfaces in the configuration file
    for (struct ddsi_config_network_interface_listelem *iface = gv->config.network_interfaces;
         iface && ok; iface = iface->next) {
      size_t match = SIZE_MAX;  // 初始化匹配索引为SIZE_MAX / Initialize the match index to SIZE_MAX
      bool has_name =
          iface->cfg.name != NULL &&
          iface->cfg.name[0] != '\0';  // 检查接口是否有名称 / Check if the interface has a name
      bool has_address = iface->cfg.address != NULL &&
                         iface->cfg.address[0] !=
                             '\0';  // 检查接口是否有地址 / Check if the interface has an address

      // 如果接口不是自动配置的 / If the interface is not automatically configured
      if (!iface->cfg.automatic) {
        ok = match_config_interface(gv, n_interfaces, interfaces, iface, &match,
                                    iface->cfg.presence_required);
      }
      // 如果接口是自动配置的，但同时具有名称或地址 / If the interface is automatically configured
      // but has a name or address at the same time
      else if (has_name || has_address) {
        GVERROR(
            "An autodetermined interface should not have its name or address property "
            "specified.\n");
        ok = false;
      }
      // 如果没有合适的接口用于自动选择 / If there are no suitable interfaces for auto-selection
      else if (maxq_index == maxq_count) {
        GVERROR("No appropriate interface remaining for autoselect.\n");
        ok = false;
      } else {
        match = maxq_list[maxq_index++];  // 获取最大质量接口索引 / Get the maximum quality
                                          // interface index
        ddsi_locator_to_string_no_port(
            addrbuf, sizeof(addrbuf),
            &interfaces[match].loc);  // 将匹配的接口地址转换为字符串 / Convert the matched
                                      // interface address to a string
        GVLOG(
            DDS_LC_INFO,
            "determined %s (%s) as highest quality interface, selected for automatic interface.\n",
            interfaces[match].name,
            addrbuf);  // 记录选择的自动接口 / Log the selected automatic interface
      }

      // 如果找到匹配的接口并且ok为true / If a matching interface is found and ok is true
      if (match != SIZE_MAX && ok) {
        ok = add_matching_interface(gv, matches, &num_matches, &interfaces[match], match,
                                    iface);  // 将匹配的接口添加到匹配数组中 / Add the matched
                                             // interface to the matches array
      }
    }
    if (num_matches == 0) {
      // gv->config.network_interfaces 不为空，match_... 和 add_matching_...
      // 在错误时打印，所以这里必须保持静默 gv->config.network_interfaces not empty, match_... and
      // add_matching_... print on error, so must be silent here
      ok = false;
    } else {
      qsort(matches, num_matches, sizeof(*matches), compare_interface_priority);
      for (size_t i = 0; i < num_matches && ok; ++i) {
        gv->interfaces[gv->n_interfaces] = interfaces[matches[i].match];
        if ((gv->interfaces[gv->n_interfaces].name =
                 ddsrt_strdup(gv->interfaces[gv->n_interfaces].name)) == NULL)
          ok = false;
        gv->n_interfaces++;
      }
    }
    ddsrt_free(matches);
  }

  gv->using_link_local_intf = false;
  for (int i = 0; i < gv->n_interfaces && ok; i++) {
    if (!gv->interfaces[i].link_local)
      continue;
    else if (!gv->using_link_local_intf)
      gv->using_link_local_intf = true;
    else {
      GVERROR("multiple interfaces selected with at least one having a link-local address\n");
      ok = false;
    }
  }

  // 释放接口名称和接口列表
  // Free interface names and interface list
  for (size_t i = 0; i < n_interfaces; i++)
    if (interfaces[i].name) ddsrt_free(interfaces[i].name);
  ddsrt_free(interfaces);
  ddsrt_free(maxq_list);

  if (!ok) {
    for (int i = 0; i < gv->n_interfaces; i++)
      if (gv->interfaces[i].name) ddsrt_free(gv->interfaces[i].name);
    gv->n_interfaces = 0;
    return 0;
  }

  assert(gv->n_interfaces > 0);
  assert(gv->n_interfaces <= MAX_XMIT_CONNS);

  GVLOG(DDS_LC_CONFIG, "selected interfaces: ");
  for (int i = 0; i < gv->n_interfaces; i++)
    GVLOG(DDS_LC_CONFIG, "%s%s (index %" PRIu32 " priority %" PRId32 ")", (i == 0) ? "" : ", ",
          gv->interfaces[i].name, gv->interfaces[i].if_index, gv->interfaces[i].priority);
  GVLOG(DDS_LC_CONFIG, "\n");
  return 1;
}
