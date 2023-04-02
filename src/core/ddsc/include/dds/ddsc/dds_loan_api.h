/*
 * Copyright(c) 2021 ZettaScale Technology
 * Copyright(c) 2021 Apex.AI, Inc
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v. 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0, or the Eclipse Distribution License
 * v. 1.0 which is available at
 * http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * SPDX-License-Identifier: EPL-2.0 OR BSD-3-Clause
 */

// API extension
// defines functions needed for loaning and shared memory usage

#ifndef _DDS_LOAN_API_H_
#define _DDS_LOAN_API_H_

#include "dds/ddsc/dds_basic_types.h"
#include "dds/ddsrt/retcode.h"
#include "dds/export.h"

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @ingroup loan
 * @component read_data
 * @brief 检查读/写器是否有可用的Loan (Check if a Loan is available to reader/writer)
 * 如果启用了共享内存并满足启用共享内存的所有约束条件且类型是固定的，则贷款可用
 * (The loan is available if the shared memory is enabled and all the constraints
 * to enable shared memory are met and the type is fixed)
 * @note 仅当 dds_is_loan_available 返回 true 时，才能使用 dds_loan_sample
 * (dds_loan_sample can be used if and only if
 * dds_is_loan_available returns true.)
 *
 * @param[in] entity 实体句柄 (the handle of the entity)
 *
 * @returns 贷款是否可用 (loan available or not)
 */
DDS_EXPORT bool dds_is_loan_available(const dds_entity_t entity);

/**
 * @ingroup loan
 * @component read_data
 * @brief 检查读/写器是否有可用的共享内存 (Check if a shared memory is available to reader/writer)
 *
 * @note 仅当 dds_is_shared_memory_available 返回 true 时，才能使用 dds_loan_shared_memory_buffer
 * (dds_loan_shared_memory_buffer can be used if and only if
 * dds_is_shared_memory_available returns true.)
 *
 * @param[in] entity 实体句柄 (the handle of the entity)
 *
 * @returns 如果共享内存可用，则返回true；否则返回false (true if shared memory is available, false
 * otherwise)
 */
DDS_EXPORT bool dds_is_shared_memory_available(const dds_entity_t entity);

/**
 * @ingroup loan
 * @component read_data
 * @brief 从写入器中借用特定大小的共享内存缓冲区 (Loan a shared memory buffer of a specific size
 * from the writer)
 *
 * @note 当前，此功能将与 dds_writecdr 一起使用，将借来的缓冲区添加到 serdata 中作为 iox_chunk
 * (Currently this function is to be used with dds_writecdr by adding the
 * loaned buffer to serdata as iox_chunk.)
 * @note 只有当写入器的 dds_is_shared_memory_available 为 true 时，才能使用此函数
 * (The function can only be used if dds_is_shared_memory_available is
 *       true for the writer.)
 *
 * @param[in] writer 要从中借用缓冲区的写入器 (the writer to loan the buffer from)
 * @param[in] size 请求的缓冲区大小 (the requested buffer size)
 * @param[out] buffer 借来的缓冲区 (the loaned buffer)
 *
 * @returns 如果成功，则返回 DDS_RETCODE_OK；否则返回 DDS_RETCODE_ERROR
 * (DDS_RETCODE_OK if successful, DDS_RETCODE_ERROR otherwise)
 */
DDS_EXPORT dds_return_t dds_loan_shared_memory_buffer(dds_entity_t writer,
                                                      size_t size,
                                                      void** buffer);

/**
 * @ingroup loan
 * @component read_data
 * @brief 从写入器借用一个样本。 (Loan a sample from the writer.)
 *
 * @note 该函数将与 dds_write 一起使用以发布借用的样本。 (This function is to be used with dds_write
 * to publish the loaned sample.)
 * @note 仅当 dds_is_loan_available 对于写入器为 true 时，才能使用此功能。 (The function can only be
 * used if dds_is_loan_available is true for the writer.)
 *
 * @param[in] writer 要从中借用缓冲区的写入器 (the writer to loan the buffer from)
 * @param[out] sample 借用的样本 (the loaned sample)
 *
 * @returns 如果成功，则返回 DDS_RETCODE_OK，否则返回 DDS_RETCODE_ERROR (DDS_RETCODE_OK if
 * successful, DDS_RETCODE_ERROR otherwise)
 */
DDS_EXPORT dds_return_t dds_loan_sample(dds_entity_t writer, void** sample);

#if defined(__cplusplus)
}
#endif
#endif
