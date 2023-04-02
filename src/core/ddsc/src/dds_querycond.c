/*
 * Copyright(c) 2006 to 2020 ZettaScale Technology and others
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

#include "dds/ddsi/ddsi_serdata.h"
#include "dds/ddsrt/atomics.h"
#include "dds/ddsrt/log.h"
#include "dds__entity.h"
#include "dds__readcond.h"
#include "dds__reader.h"
#include "dds__topic.h"

/**
 * @brief 创建一个查询条件实体 (Create a query condition entity)
 *
 * @param reader 读取器实体，用于创建查询条件 (Reader entity for which the query condition is
 * created)
 * @param mask 状态变化的掩码，用于过滤数据 (Mask for state changes, used to filter data)
 * @param filter 自定义过滤函数，用于进一步过滤数据 (Custom filter function for further filtering of
 * data)
 * @return 返回创建的查询条件实体的句柄 (Returns the handle of the created query condition entity)
 */
dds_entity_t dds_create_querycondition(dds_entity_t reader,
                                       uint32_t mask,
                                       dds_querycondition_filter_fn filter) {
  dds_return_t rc;  // 定义返回值变量 (Define return value variable)
  dds_reader* r;    // 定义读取器指针 (Define reader pointer)

  // 尝试锁定读取器并检查返回值 (Try to lock the reader and check the return value)
  if ((rc = dds_reader_lock(reader, &r)) != DDS_RETCODE_OK)
    return rc;  // 锁定失败，返回错误代码 (Lock failed, return error code)
  else {
    dds_entity_t hdl;  // 定义查询条件实体句柄变量 (Define query condition entity handle variable)

    // 创建查询条件实现并获取读取条件指针 (Create query condition implementation and get read
    // condition pointer)
    dds_readcond* cond = dds_create_readcond_impl(r, DDS_KIND_COND_QUERY, mask, filter);
    assert(cond);  // 断言读取条件不为空 (Assert that the read condition is not null)

    // 获取查询条件实体句柄 (Get the query condition entity handle)
    hdl = cond->m_entity.m_hdllink.hdl;

    // 完成查询条件实体的初始化 (Complete the initialization of the query condition entity)
    dds_entity_init_complete(&cond->m_entity);

    // 解锁读取器 (Unlock the reader)
    dds_reader_unlock(r);

    return hdl;  // 返回查询条件实体句柄 (Return the query condition entity handle)
  }
}
