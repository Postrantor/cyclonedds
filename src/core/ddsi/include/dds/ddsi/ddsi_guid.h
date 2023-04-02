/*
 * Copyright(c) 2019 ZettaScale Technology and others
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v. 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0, or the Eclipse Distribution License
 * v. 1.0 which is available at
 * http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * SPDX-License-Identifier: EPL-2.0 OR BSD-3-Clause
 */
#ifndef DDSI_GUID_H
#define DDSI_GUID_H

#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @brief GUID前缀联合体 (GUID prefix union)
 * @details
 * 用于表示GUID前缀的联合体，包含一个12字节的无符号字符数组和一个3个元素的32位无符号整数数组。 (A
 * union representing the GUID prefix, containing a 12-byte unsigned character array and a 3-element
 * 32-bit unsigned integer array.)
 */
typedef union ddsi_guid_prefix {
  unsigned char s[12];  ///< 12字节的无符号字符数组 (12-byte unsigned character array)
  uint32_t u[3];  ///< 3个元素的32位无符号整数数组 (3-element 32-bit unsigned integer array)
} ddsi_guid_prefix_t;

/**
 * @brief 实体ID联合体 (Entity ID union)
 * @details 用于表示实体ID的联合体，包含一个32位无符号整数。
 *          (A union representing the entity ID, containing a 32-bit unsigned integer.)
 */
typedef union ddsi_entityid {
  uint32_t u;  ///< 32位无符号整数 (32-bit unsigned integer)
} ddsi_entityid_t;

/**
 * @brief DDSI GUID结构体 (DDSI GUID structure)
 * @details 包含GUID前缀和实体ID的结构体。
 *          (A structure containing the GUID prefix and entity ID.)
 */
typedef struct ddsi_guid {
  ddsi_guid_prefix_t prefix;  ///< GUID前缀 (GUID prefix)
  ddsi_entityid_t entityid;   ///< 实体ID (Entity ID)
} ddsi_guid_t;

/**
 * @brief 将GUID从主机字节序转换为网络字节序 (Converts a GUID from host byte order to network byte
 * order)
 * @param g 要转换的GUID (The GUID to convert)
 * @return 转换后的GUID (The converted GUID)
 */
/** @component misc */
ddsi_guid_t ddsi_hton_guid(ddsi_guid_t g);

/**
 * @brief 将GUID从网络字节序转换为主机字节序 (Converts a GUID from network byte order to host byte
 * order)
 * @param g 要转换的GUID (The GUID to convert)
 * @return 转换后的GUID (The converted GUID)
 */
/** @component misc */
ddsi_guid_t ddsi_ntoh_guid(ddsi_guid_t g);

/**
 * @brief 将GUID前缀从主机字节序转换为网络字节序 (Converts a GUID prefix from host byte order to
 * network byte order)
 * @param p 要转换的GUID前缀 (The GUID prefix to convert)
 * @return 转换后的GUID前缀 (The converted GUID prefix)
 */
/** @component misc */
ddsi_guid_prefix_t ddsi_hton_guid_prefix(ddsi_guid_prefix_t p);

/**
 * @brief 将GUID前缀从网络字节序转换为主机字节序 (Converts a GUID prefix from network byte order to
 * host byte order)
 * @param p 要转换的GUID前缀 (The GUID prefix to convert)
 * @return 转换后的GUID前缀 (The converted GUID prefix)
 */
/** @component misc */
ddsi_guid_prefix_t ddsi_ntoh_guid_prefix(ddsi_guid_prefix_t p);

/**
 * @brief 将实体ID从主机字节序转换为网络字节序 (Converts an entity ID from host byte order to
 * network byte order)
 * @param e 要转换的实体ID (The entity ID to convert)
 * @return 转换后的实体ID (The converted entity ID)
 */
/** @component misc */
ddsi_entityid_t ddsi_hton_entityid(ddsi_entityid_t e);

/**
 * @brief 将实体ID从网络字节序转换为主机字节序 (Converts an entity ID from network byte order to
 * host byte order)
 * @param e 要转换的实体ID (The entity ID to convert)
 * @return 转换后的实体ID (The converted entity ID)
 */
/** @component misc */
ddsi_entityid_t ddsi_ntoh_entityid(ddsi_entityid_t e);

#if defined(__cplusplus)
}
#endif

#endif
