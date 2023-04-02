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
#ifndef _DDS_RHC_H_
#define _DDS_RHC_H_

#include "dds/ddsi/ddsi_rhc.h"
#include "dds/ddsrt/static_assert.h"

#define NO_STATE_MASK_SET (DDS_ANY_STATE + 1)

#if defined(__cplusplus)
extern "C" {
#endif

// 结构体定义：dds_rhc (Define the structure: dds_rhc)
struct dds_rhc;

// 结构体定义：dds_readcond (Define the structure: dds_readcond)
struct dds_readcond;

// 结构体定义：dds_reader (Define the structure: dds_reader)
struct dds_reader;

// 结构体定义：ddsi_tkmap (Define the structure: ddsi_tkmap)
struct ddsi_tkmap;

/**
 * @brief 定义一个函数指针类型：dds_rhc_associate_t，用于关联 dds_rhc 和 dds_reader
 *        (Define a function pointer type: dds_rhc_associate_t, used for associating dds_rhc and
 * dds_reader)
 *
 * @param rhc 指向 dds_rhc 的指针 (Pointer to dds_rhc)
 * @param reader 指向 dds_reader 的指针 (Pointer to dds_reader)
 * @param type 指向 ddsi_sertype 的常量指针 (Constant pointer to ddsi_sertype)
 * @param tkmap 指向 ddsi_tkmap 的指针 (Pointer to ddsi_tkmap)
 * @return dds_return_t 返回操作结果 (Return operation result)
 */
typedef dds_return_t (*dds_rhc_associate_t)(struct dds_rhc* rhc,
                                            struct dds_reader* reader,
                                            const struct ddsi_sertype* type,
                                            struct ddsi_tkmap* tkmap);

/**
 * @brief 定义一个函数指针类型：dds_rhc_read_take_t，用于读取或获取数据
 *        (Define a function pointer type: dds_rhc_read_take_t, used for reading or taking data)
 *
 * @param rhc 指向 dds_rhc 的指针 (Pointer to dds_rhc)
 * @param lock 布尔值，表示是否锁定 (Boolean value indicating whether to lock)
 * @param values 数据值的指针数组 (Array of pointers to data values)
 * @param info_seq 采样信息序列 (Sample info sequence)
 * @param max_samples 最大采样数量 (Maximum number of samples)
 * @param mask 掩码 (Mask)
 * @param handle 实例句柄 (Instance handle)
 * @param cond 指向 dds_readcond 的指针 (Pointer to dds_readcond)
 * @return int32_t 返回读取或获取到的数据数量 (Return the number of data read or taken)
 */
typedef int32_t (*dds_rhc_read_take_t)(struct dds_rhc* rhc,
                                       bool lock,
                                       void** values,
                                       dds_sample_info_t* info_seq,
                                       uint32_t max_samples,
                                       uint32_t mask,
                                       dds_instance_handle_t handle,
                                       struct dds_readcond* cond);

/**
 * @brief 定义一个函数指针类型：dds_rhc_read_take_cdr_t，用于以 CDR 格式读取或获取数据
 *        (Define a function pointer type: dds_rhc_read_take_cdr_t, used for reading or taking data
 * in CDR format)
 *
 * @param rhc 指向 dds_rhc 的指针 (Pointer to dds_rhc)
 * @param lock 布尔值，表示是否锁定 (Boolean value indicating whether to lock)
 * @param values 指向 ddsi_serdata 指针数组 (Pointer to an array of ddsi_serdata pointers)
 * @param info_seq 采样信息序列 (Sample info sequence)
 * @param max_samples 最大采样数量 (Maximum number of samples)
 * @param sample_states 采样状态 (Sample states)
 * @param view_states 视图状态 (View states)
 * @param instance_states 实例状态 (Instance states)
 * @param handle 实例句柄 (Instance handle)
 * @return int32_t 返回以 CDR 格式读取或获取到的数据数量 (Return the number of data read or taken in
 * CDR format)
 */
typedef int32_t (*dds_rhc_read_take_cdr_t)(struct dds_rhc* rhc,
                                           bool lock,
                                           struct ddsi_serdata** values,
                                           dds_sample_info_t* info_seq,
                                           uint32_t max_samples,
                                           uint32_t sample_states,
                                           uint32_t view_states,
                                           uint32_t instance_states,
                                           dds_instance_handle_t handle);

/**
 * @brief 定义一个函数指针类型：dds_rhc_add_readcondition_t，用于添加读取条件
 *        (Define a function pointer type: dds_rhc_add_readcondition_t, used for adding read
 * conditions)
 *
 * @param rhc 指向 dds_rhc 的指针 (Pointer to dds_rhc)
 * @param cond 指向 dds_readcond 的指针 (Pointer to dds_readcond)
 * @return bool 返回是否成功添加读取条件 (Return whether the read condition was added successfully)
 */
typedef bool (*dds_rhc_add_readcondition_t)(struct dds_rhc* rhc, struct dds_readcond* cond);

/**
 * @brief 定义一个函数指针类型：dds_rhc_remove_readcondition_t，用于移除读取条件
 *        (Define a function pointer type: dds_rhc_remove_readcondition_t, used for removing read
 * conditions)
 *
 * @param rhc 指向 dds_rhc 的指针 (Pointer to dds_rhc)
 * @param cond 指向 dds_readcond 的指针 (Pointer to dds_readcond)
 */
typedef void (*dds_rhc_remove_readcondition_t)(struct dds_rhc* rhc, struct dds_readcond* cond);

/**
 * @brief 定义一个函数指针类型：dds_rhc_lock_samples_t，用于锁定样本
 *        (Define a function pointer type: dds_rhc_lock_samples_t, used for locking samples)
 *
 * @param rhc 指向 dds_rhc 的指针 (Pointer to dds_rhc)
 * @return uint32_t 返回锁定的样本数量 (Return the number of locked samples)
 */
typedef uint32_t (*dds_rhc_lock_samples_t)(struct dds_rhc* rhc);

/**
 * @struct dds_rhc_ops
 * @brief 数据结构，包含dds_rhc所需的操作函数 (Data structure containing operation functions needed
 * by dds_rhc)
 */
struct dds_rhc_ops {
  // DDSI rhc操作的副本，我们可以在不使用额外间接的情况下使用任一接口 (A copy of DDSI rhc
  // operations, so we can use either interface without additional indirections)
  struct ddsi_rhc_ops rhc_ops;
  dds_rhc_read_take_t read;         ///< 读操作函数 (Read operation function)
  dds_rhc_read_take_t take;         ///< 取操作函数 (Take operation function)
  dds_rhc_read_take_cdr_t readcdr;  ///< 读取CDR操作函数 (Read CDR operation function)
  dds_rhc_read_take_cdr_t takecdr;  ///< 取CDR操作函数 (Take CDR operation function)
  dds_rhc_add_readcondition_t
      add_readcondition;  ///< 添加读条件操作函数 (Add read condition operation function)
  dds_rhc_remove_readcondition_t
      remove_readcondition;  ///< 移除读条件操作函数 (Remove read condition operation function)
  dds_rhc_lock_samples_t lock_samples;  ///< 锁定样本操作函数 (Lock samples operation function)
  dds_rhc_associate_t associate;        ///< 关联操作函数 (Associate operation function)
};

/**
 * @struct dds_rhc
 * @brief 存储dds_rhc操作的数据结构 (Data structure to store dds_rhc operations)
 */
struct dds_rhc {
  union {
    const struct dds_rhc_ops* ops;  ///< 指向dds_rhc_ops的指针 (Pointer to dds_rhc_ops)
    struct ddsi_rhc rhc;            ///< ddsi_rhc实例 (ddsi_rhc instance)
  } common;
};

// 静态断言，确保偏移量正确 (Static assertion to ensure correct offset)
DDSRT_STATIC_ASSERT(offsetof(struct dds_rhc, common.ops) == offsetof(struct ddsi_rhc, ops));

/**
 * @brief 将dds_reader、ddsi_sertype和ddsi_tkmap与dds_rhc关联 (Associates dds_reader, ddsi_sertype
 * and ddsi_tkmap with dds_rhc)
 * @param[in] rhc dds_rhc指针 (Pointer to dds_rhc)
 * @param[in] reader dds_reader指针 (Pointer to dds_reader)
 * @param[in] type ddsi_sertype指针 (Pointer to ddsi_sertype)
 * @param[in] tkmap ddsi_tkmap指针 (Pointer to ddsi_tkmap)
 * @return 操作结果 (Operation result)
 */
DDS_INLINE_EXPORT inline dds_return_t dds_rhc_associate(struct dds_rhc* rhc,
                                                        struct dds_reader* reader,
                                                        const struct ddsi_sertype* type,
                                                        struct ddsi_tkmap* tkmap) {
  return rhc->common.ops->associate(rhc, reader, type, tkmap);
}

/**
 * @brief 存储dds_rhc的样本 (Stores sample in dds_rhc)
 * @param[in] rhc dds_rhc指针 (Pointer to dds_rhc)
 * @param[in] wrinfo ddsi_writer_info指针 (Pointer to ddsi_writer_info)
 * @param[in] sample ddsi_serdata指针 (Pointer to ddsi_serdata)
 * @param[in] tk ddsi_tkmap_instance指针 (Pointer to ddsi_tkmap_instance)
 * @return 存储成功返回true，否则返回false (Returns true if stored successfully, false otherwise)
 */
DDS_INLINE_EXPORT inline bool dds_rhc_store(struct dds_rhc* __restrict rhc,
                                            const struct ddsi_writer_info* __restrict wrinfo,
                                            struct ddsi_serdata* __restrict sample,
                                            struct ddsi_tkmap_instance* __restrict tk) {
  return rhc->common.ops->rhc_ops.store(&rhc->common.rhc, wrinfo, sample, tk);
}

/**
 * @brief 取消注册dds_rhc中的写入器 (Unregisters writer in dds_rhc)
 * @param[in] rhc dds_rhc指针 (Pointer to dds_rhc)
 * @param[in] wrinfo ddsi_writer_info指针 (Pointer to ddsi_writer_info)
 */
DDS_INLINE_EXPORT inline void dds_rhc_unregister_wr(
    struct dds_rhc* __restrict rhc, const struct ddsi_writer_info* __restrict wrinfo) {
  rhc->common.ops->rhc_ops.unregister_wr(&rhc->common.rhc, wrinfo);
}

/**
 * @brief 放弃dds_rhc中的所有权 (Relinquishes ownership in dds_rhc)
 * @param[in] rhc dds_rhc指针 (Pointer to dds_rhc)
 * @param[in] wr_iid 写入器实例ID (Writer instance ID)
 */
DDS_INLINE_EXPORT inline void dds_rhc_relinquish_ownership(struct dds_rhc* __restrict rhc,
                                                           const uint64_t wr_iid) {
  rhc->common.ops->rhc_ops.relinquish_ownership(&rhc->common.rhc, wr_iid);
}

/**
 * @brief 设置RHC的QoS（Quality of Service，服务质量）
 * Set the QoS (Quality of Service) of RHC.
 *
 * @param[in] rhc 指向dds_rhc结构体的指针
 *               Pointer to dds_rhc structure.
 * @param[in] qos 指向dds_qos结构体的指针
 *               Pointer to dds_qos structure.
 */
DDS_INLINE_EXPORT inline void dds_rhc_set_qos(struct dds_rhc* rhc, const struct dds_qos* qos) {
  // 调用rhc_ops.set_qos操作设置RHC的QoS
  // Call the rhc_ops.set_qos operation to set the QoS of RHC.
  rhc->common.ops->rhc_ops.set_qos(&rhc->common.rhc, qos);
}

/**
 * @brief 释放RHC资源
 * Free the RHC resources.
 *
 * @param[in] rhc 指向dds_rhc结构体的指针
 *               Pointer to dds_rhc structure.
 */
DDS_INLINE_EXPORT inline void dds_rhc_free(struct dds_rhc* rhc) {
  // 调用rhc_ops.free操作释放RHC资源
  // Call the rhc_ops.free operation to free the RHC resources.
  rhc->common.ops->rhc_ops.free(&rhc->common.rhc);
}

/**
 * @brief 从RHC读取数据
 * Read data from RHC.
 *
 * @param[in] rhc 指向dds_rhc结构体的指针
 *               Pointer to dds_rhc structure.
 * @param[in] lock 是否锁定RHC以防止并发访问
 *                 Whether to lock the RHC to prevent concurrent access.
 * @param[out] values 读取到的数据值数组
 *                    Array of read data values.
 * @param[out] info_seq 读取到的样本信息序列
 *                      Sequence of read sample information.
 * @param[in] max_samples 要读取的最大样本数
 *                        Maximum number of samples to read.
 * @param[in] mask 过滤条件掩码
 *                 Filter condition mask.
 * @param[in] handle 实例句柄
 *                   Instance handle.
 * @param[in] cond 指向dds_readcond结构体的指针
 *                 Pointer to dds_readcond structure.
 * @return 返回读取到的样本数
 *         The number of samples read.
 */
DDS_INLINE_EXPORT inline int32_t dds_rhc_read(struct dds_rhc* rhc,
                                              bool lock,
                                              void** values,
                                              dds_sample_info_t* info_seq,
                                              uint32_t max_samples,
                                              uint32_t mask,
                                              dds_instance_handle_t handle,
                                              struct dds_readcond* cond) {
  // 调用read操作从RHC读取数据
  // Call the read operation to read data from RHC.
  return (rhc->common.ops->read)(rhc, lock, values, info_seq, max_samples, mask, handle, cond);
}

/**
 * @brief 从RHC获取（并删除）数据
 * Take (and remove) data from RHC.
 *
 * @param[in] rhc 指向dds_rhc结构体的指针
 *               Pointer to dds_rhc structure.
 * @param[in] lock 是否锁定RHC以防止并发访问
 *                 Whether to lock the RHC to prevent concurrent access.
 * @param[out] values 获取到的数据值数组
 *                    Array of taken data values.
 * @param[out] info_seq 获取到的样本信息序列
 *                      Sequence of taken sample information.
 * @param[in] max_samples 要获取的最大样本数
 *                        Maximum number of samples to take.
 * @param[in] mask 过滤条件掩码
 *                 Filter condition mask.
 * @param[in] handle 实例句柄
 *                   Instance handle.
 * @param[in] cond 指向dds_readcond结构体的指针
 *                 Pointer to dds_readcond structure.
 * @return 返回获取到的样本数
 *         The number of samples taken.
 */
DDS_INLINE_EXPORT inline int32_t dds_rhc_take(struct dds_rhc* rhc,
                                              bool lock,
                                              void** values,
                                              dds_sample_info_t* info_seq,
                                              uint32_t max_samples,
                                              uint32_t mask,
                                              dds_instance_handle_t handle,
                                              struct dds_readcond* cond) {
  // 调用take操作从RHC获取（并删除）数据
  // Call the take operation to take (and remove) data from RHC.
  return rhc->common.ops->take(rhc, lock, values, info_seq, max_samples, mask, handle, cond);
}

/**
 * @brief 以CDR格式从RHC读取数据
 * Read data from RHC in CDR format.
 *
 * @param[in] rhc 指向dds_rhc结构体的指针
 *               Pointer to dds_rhc structure.
 * @param[in] lock 是否锁定RHC以防止并发访问
 *                 Whether to lock the RHC to prevent concurrent access.
 * @param[out] values 读取到的CDR格式数据值数组
 *                    Array of read data values in CDR format.
 * @param[out] info_seq 读取到的样本信息序列
 *                      Sequence of read sample information.
 * @param[in] max_samples 要读取的最大样本数
 *                        Maximum number of samples to read.
 * @param[in] sample_states 要读取的样本状态
 *                          Sample states to read.
 * @param[in] view_states 要读取的视图状态
 *                         View states to read.
 * @param[in] instance_states 要读取的实例状态
 *                            Instance states to read.
 * @param[in] handle 实例句柄
 *                   Instance handle.
 * @return 返回读取到的样本数
 *         The number of samples read.
 */
DDS_INLINE_EXPORT inline int32_t dds_rhc_readcdr(struct dds_rhc* rhc,
                                                 bool lock,
                                                 struct ddsi_serdata** values,
                                                 dds_sample_info_t* info_seq,
                                                 uint32_t max_samples,
                                                 uint32_t sample_states,
                                                 uint32_t view_states,
                                                 uint32_t instance_states,
                                                 dds_instance_handle_t handle) {
  // 调用readcdr操作从RHC读取CDR格式数据
  // Call the readcdr operation to read data from RHC in CDR format.
  return rhc->common.ops->readcdr(rhc, lock, values, info_seq, max_samples, sample_states,
                                  view_states, instance_states, handle);
}

/** @component rhc */
// 以下函数用于从缓存中获取CDR样本数据（The following function is used to take CDR sample data from
// the cache）
DDS_INLINE_EXPORT inline int32_t dds_rhc_takecdr(
    struct dds_rhc* rhc,           // RHC对象指针（Pointer to the RHC object）
    bool lock,                     // 是否锁定RHC（Whether to lock the RHC）
    struct ddsi_serdata** values,  // 输出的序列化数据数组（Output array of serialized data）
    dds_sample_info_t* info_seq,  // 输出的样本信息数组（Output array of sample information）
    uint32_t max_samples,         // 最大样本数（Maximum number of samples）
    uint32_t sample_states,       // 要获取的样本状态（Sample states to be taken）
    uint32_t view_states,         // 要获取的视图状态（View states to be taken）
    uint32_t instance_states,     // 要获取的实例状态（Instance states to be taken）
    dds_instance_handle_t handle  // 实例句柄（Instance handle）
) {
  // 调用RHC操作结构体中的takecdr函数（Call the takecdr function in the RHC operation structure）
  return rhc->common.ops->takecdr(rhc, lock, values, info_seq, max_samples, sample_states,
                                  view_states, instance_states, handle);
}

/** @component rhc */
// 以下函数用于向RHC添加读取条件（The following function is used to add a read condition to the
// RHC）
DDS_INLINE_EXPORT inline bool dds_rhc_add_readcondition(
    struct dds_rhc* rhc,       // RHC对象指针（Pointer to the RHC object）
    struct dds_readcond* cond  // 读取条件对象指针（Pointer to the read condition object）
) {
  // 调用RHC操作结构体中的add_readcondition函数（Call the add_readcondition function in the RHC
  // operation structure）
  return rhc->common.ops->add_readcondition(rhc, cond);
}

/** @component rhc */
// 以下函数用于从RHC移除读取条件（The following function is used to remove a read condition from the
// RHC）
DDS_INLINE_EXPORT inline void dds_rhc_remove_readcondition(
    struct dds_rhc* rhc,       // RHC对象指针（Pointer to the RHC object）
    struct dds_readcond* cond  // 读取条件对象指针（Pointer to the read condition object）
) {
  // 调用RHC操作结构体中的remove_readcondition函数（Call the remove_readcondition function in the
  // RHC operation structure）
  rhc->common.ops->remove_readcondition(rhc, cond);
}

/** @component rhc */
// 以下函数用于锁定RHC中的样本数据（The following function is used to lock samples in the RHC）
DDS_INLINE_EXPORT inline uint32_t dds_rhc_lock_samples(
    struct dds_rhc* rhc  // RHC对象指针（Pointer to the RHC object）
) {
  // 调用RHC操作结构体中的lock_samples函数（Call the lock_samples function in the RHC operation
  // structure）
  return rhc->common.ops->lock_samples(rhc);
}

/** @component rhc */
// 当读取器有可用数据时，调用以下回调函数（The following callback function is called when the reader
// has data available）
DDS_EXPORT void dds_reader_data_available_cb(
    struct dds_reader* rd  // 读取器对象指针（Pointer to the reader object）
);

#if defined(__cplusplus)
}
#endif
#endif
