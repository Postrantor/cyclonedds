#
# Copyright(c) 2022 ZettaScale Technology and others
#
# This program and the accompanying materials are made available under the terms
# of the Eclipse Public License v. 2.0 which is available at
# http://www.eclipse.org/legal/epl-2.0, or the Eclipse Distribution License v.
# 1.0 which is available at http://www.eclipse.org/org/documents/edl-v10.php.
#
# SPDX-License-Identifier: EPL-2.0 OR BSD-3-Clause
#

# 设置 srcs_cdr 变量，包含 dds_cdrstream.c 文件
set(srcs_cdr dds_cdrstream.c)

# 设置 hdrs_private_cdr 变量，包含 dds_cdrstream.h 文件
set(hdrs_private_cdr dds_cdrstream.h)

# 使用 prepend 函数将 "${CMAKE_CURRENT_LIST_DIR}/include/dds/cdr/" 添加到
# hdrs_private_cdr 变量的每个元素前面
prepend(hdrs_private_cdr "${CMAKE_CURRENT_LIST_DIR}/include/dds/cdr/"
        ${hdrs_private_cdr})

# 使用 prepend 函数将 "${CMAKE_CURRENT_LIST_DIR}/src/" 添加到 srcs_cdr 变量的每个元素前面
prepend(srcs_cdr "${CMAKE_CURRENT_LIST_DIR}/src/" ${srcs_cdr})

# 将 srcs_cdr 和 hdrs_private_cdr 变量中的文件添加到 ddsc 目标的私有源文件和头文件中
target_sources(ddsc PRIVATE ${srcs_cdr} ${hdrs_private_cdr})

# 将 "${CMAKE_CURRENT_LIST_DIR}/include" 添加到 ddsc 目标的私有包含目录中
target_include_directories(ddsc PRIVATE "${CMAKE_CURRENT_LIST_DIR}/include")

# 安装 include 目录下的所有文件到 CMAKE_INSTALL_INCLUDEDIR 目录，并设置组件为 dev
install(
  DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/include/"
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
  COMPONENT dev)
