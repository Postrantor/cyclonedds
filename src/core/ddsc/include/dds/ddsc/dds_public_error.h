/*
 * Copyright(c) 2006 to 2019 ZettaScale Technology and others
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v. 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0, or the Eclipse Distribution License
 * v. 1.0 which is available at
 * http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * SPDX-License-Identifier: EPL-2.0 OR BSD-3-Clause
 */

/** @file
 *
 * @brief DDS C Error API
 *
 * This header file defines the public API of error values and convenience
 * functions in the CycloneDDS C language binding.
 */
#ifndef DDS_ERROR_H
#define DDS_ERROR_H

#include "dds/ddsrt/log.h"
#include "dds/ddsrt/retcode.h"
#include "dds/export.h"

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @file rmw_error_handling.h
 * @brief RMW层错误处理相关的宏和函数 (RMW layer error handling related macros and functions)
 */

/* ** DEPRECATED ** */
#ifndef DOXYGEN_SHOULD_SKIP_THIS

/* Error masks for returned status values */

/** 错误号掩码 (Error number mask) */
#define DDS_ERR_NR_MASK 0x000000ff
/** 行号掩码 (Line number mask) */
#define DDS_ERR_LINE_MASK 0x003fff00
/** 文件ID掩码 (File ID mask) */
#define DDS_ERR_FILE_ID_MASK 0x7fc00000

/* Error code handling functions */

/**
 * @brief 提取错误号的宏 (Macro to extract error number)
 * @param e 错误代码 (Error code)
 * @return 错误号 (Error number)
 */
#define dds_err_nr(e) (e)

/**
 * @brief 提取行号的宏 (Macro to extract line number)
 * @param e 错误代码 (Error code)
 * @return 行号 (Line number)
 */
#define dds_err_line(e) (0)

/**
 * @brief 提取文件ID的宏 (Macro to extract file identifier)
 * @param e 错误代码 (Error code)
 * @return 文件ID (File identifier)
 */
#define dds_err_file_id(e) (0)

#endif  // DOXYGEN_SHOULD_SKIP_THIS

#if defined(__cplusplus)
}
#endif
#endif
