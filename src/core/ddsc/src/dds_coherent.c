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
#include <assert.h>

#include "dds/dds.h"
#include "dds__entity.h"
#include "dds__publisher.h"
#include "dds__subscriber.h"

/**
 * @brief 开始一致性操作
 *
 * @param entity 实体，可以是读取器、写入器、发布者或订阅者
 * @return dds_return_t 返回操作结果，当前未实现，返回DDS_RETCODE_UNSUPPORTED
 */
dds_return_t dds_begin_coherent(dds_entity_t entity) {
  // 定义支持的实体类型数组
  static const dds_entity_kind_t kinds[] = {DDS_KIND_READER, DDS_KIND_WRITER, DDS_KIND_PUBLISHER,
                                            DDS_KIND_SUBSCRIBER};
  // 调用通用未实现操作函数，并传入实体类型数组
  return dds_generic_unimplemented_operation_manykinds(entity, sizeof(kinds) / sizeof(kinds[0]),
                                                       kinds);
}

/**
 * @brief 结束一致性操作
 *
 * @param entity 实体，可以是读取器、写入器、发布者或订阅者
 * @return dds_return_t 返回操作结果，当前未实现，返回DDS_RETCODE_UNSUPPORTED
 */
dds_return_t dds_end_coherent(dds_entity_t entity) {
  // 定义支持的实体类型数组
  static const dds_entity_kind_t kinds[] = {DDS_KIND_READER, DDS_KIND_WRITER, DDS_KIND_PUBLISHER,
                                            DDS_KIND_SUBSCRIBER};
  // 调用通用未实现操作函数，并传入实体类型数组
  return dds_generic_unimplemented_operation_manykinds(entity, sizeof(kinds) / sizeof(kinds[0]),
                                                       kinds);
}
