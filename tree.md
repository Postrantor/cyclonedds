Folder PATH listing for volume RED
Volume serial number is B8A5-947D
C:.
├───core
│   ├───cdr // CDR 序列化和反序列化库
│   │   ├───include // CDR 库的头文件
│   │   │   └───dds
│   │   │       └───cdr
│   │   └───src // CDR 库的源代码
│   ├───ddsc // 核心DDS客户端库
│   │   ├───include // DDS客户端库的头文件
│   │   │   ├───dds
│   │   │   │   └───ddsc
│   │   │   └───ddsc
│   │   ├───src // DDS客户端库的源代码
│   │   └───tests // DDS客户端库的测试代码
│   ├───ddsi // DDSI协议实现
│   │   ├───idl // IDL编译器相关文件
│   │   ├───include // DDSI协议实现的头文件
│   │   │   └───dds
│   │   │       └───ddsi
│   │   ├───src // DDSI协议实现的源代码
│   │   └───tests // DDSI协议实现的测试代码
│   └───xtests // 扩展测试
│       ├───cdrtest // CDR 库的扩展测试
│       │   └───cmake // CMake 构建脚本
│       ├───initsampledeliv // 初始化样本传递的测试
│       ├───rhc_torture // RHC（可靠历史缓存）压力测试
│       └───symbol_export // 符号导出测试
├───ddsrt // ddsrt 是 Cyclone DDS 运行时库
│   ├───include // 包含头文件
│   │   └───dds // DDS 相关头文件
│   │       └───ddsrt // ddsrt 运行时库头文件
│   │           ├───atomics // 原子操作相关头文件
│   │           ├───filesystem // 文件系统相关头文件
│   │           ├───sockets // 套接字相关头文件
│   │           ├───sync // 同步原语相关头文件
│   │           ├───threads // 线程相关头文件
│   │           ├───time // 时间相关头文件
│   │           └───types // 类型定义相关头文件
│   ├───src // 源代码
│   │   ├───dynlib // 动态库加载相关源代码
│   │   │   ├───posix // POSIX 平台动态库加载源代码
│   │   │   └───windows // Windows 平台动态库加载源代码
│   │   ├───environ // 环境变量相关源代码
│   │   │   ├───posix // POSIX 平台环境变量源代码
│   │   │   ├───solaris2.6 // Solaris 2.6 平台环境变量源代码
│   │   │   ├───windows // Windows 平台环境变量源代码
│   │   │   └───zephyr // Zephyr 平台环境变量源代码
│   │   ├───filesystem // 文件系统相关源代码
│   │   │   ├───posix // POSIX 平台文件系统源代码
│   │   │   └───windows // Windows 平台文件系统源代码
│   │   ├───heap // 内存分配相关源代码
│   │   │   ├───freertos // FreeRTOS 平台内存分配源代码
│   │   │   ├───posix // POSIX 平台内存分配源代码
│   │   │   └───vxworks // VxWorks 平台内存分配源代码
│   │   ├───ifaddrs // 网络接口地址相关源代码
│   │   │   ├───lwip // lwIP 平台网络接口地址源代码
│   │   │   ├───posix // POSIX 平台网络接口地址源代码
│   │   │   ├───solaris2.6 // Solaris 2.6 平台网络接口地址源代码
│   │   │   ├───windows // Windows 平台网络接口地址源代码
│   │   │   └───zephyr // Zephyr 平台网络接口地址源代码
│   │   ├───netstat // 网络状态相关源代码
│   │   │   ├───darwin // Darwin 平台网络状态源代码
│   │   │   ├───linux // Linux 平台网络状态源代码
│   │   │   └───windows // Windows 平台网络状态源代码
│   │   ├───process // 进程相关源代码
│   │   │   ├───freertos // FreeRTOS 平台进程源代码
│   │   │   ├───posix // POSIX 平台进程源代码
│   │   │   └───windows // Windows 平台进程源代码
│   │   ├───random // 随机数生成相关源代码
│   │   │   ├───posix // POSIX 平台随机数生成源代码
│   │   │   └───windows // Windows 平台随机数生成源代码
│   │   ├───rusage // 资源使用情况相关源代码
│   │   │   ├───freertos // FreeRTOS 平台资源使用情况源代码
│   │   │   ├───posix // POSIX 平台资源使用情况源代码
│   │   │   └───windows // Windows 平台资源使用情况源代码
│   │   ├───sockets // 套接字相关源代码
│   │   │   ├───posix // POSIX 平台套接字源代码
│   │   │   └───windows // Windows 平台套接字源代码
│   │   ├───sync // 同步原语相关源代码
│   │   │   ├───freertos // FreeRTOS 平台同步原语源代码
│   │   │   ├───posix // POSIX 平台同步原语源代码
│   │   │   └───windows // Windows 平台同步原语源代码
│   │   ├───threads // 线程相关源代码
│   │   │   ├───freertos // FreeRTOS 平台线程源代码
│   │   │   ├───posix // POSIX 平台线程源代码
│   │   │   └───windows // Windows 平台线程源代码
│   │   └───time // 时间相关源代码
│   │       ├───darwin // Darwin 平台时间源代码
│   │       ├───freertos // FreeRTOS 平台时间源代码
│   │       ├───posix // POSIX 平台时间源代码
│   │       ├───solaris2.6 // Solaris 2.6 平台时间源代码
│   │       └───windows // Windows 平台时间源代码
│   └───tests // 测试代码
├───idl
│   ├───include          // 包含 IDL 编译器需要的头文件
│   │   └───idl
│   ├───src              // IDL 编译器的源代码
│   └───tests            // IDL 编译器的测试用例
├───security // 安全模块
│   ├───api // API 目录
│   │   └───include // 包含头文件
│   │       └───dds // DDS 目录
│   │           └───security // 安全相关头文件
│   ├───builtin_plugins // 内置插件
│   │   ├───access_control // 访问控制插件
│   │   │   └───src // 源代码
│   │   ├───authentication // 认证插件
│   │   │   └───src // 源代码
│   │   ├───cryptographic // 加密插件
│   │   │   └───src // 源代码
│   │   ├───include // 插件相关头文件
│   │   └───tests // 测试用例
│   │       ├───access_control_fnmatch // 访问控制匹配测试
│   │       │   └───src // 源代码
│   │       ├───common // 通用测试
│   │       │   └───src // 源代码
│   │       ├───create_local_datareader_crypto_tokens // 创建本地 DataReader 加密令牌测试
│   │       │   └───src // 源代码
│   │       ├───create_local_datawriter_crypto_tokens // 创建本地 DataWriter 加密令牌测试
│   │       │   └───src // 源代码
│   │       ├───create_local_participant_crypto_tokens // 创建本地参与者加密令牌测试
│   │       │   └───src // 源代码
│   │       ├───decode_datareader_submessage // 解码 DataReader 子消息测试
│   │       │   └───src // 源代码
│   │       ├───decode_datawriter_submessage // 解码 DataWriter 子消息测试
│   │       │   └───src // 源代码
│   │       ├───decode_rtps_message // 解码 RTPS 消息测试
│   │       │   └───src // 源代码
│   │       ├───decode_serialized_payload // 解码序列化负载测试
│   │       │   └───src // 源代码
│   │       ├───encode_datareader_submessage // 编码 DataReader 子消息测试
│   │       │   └───src // 源代码
│   │       ├───encode_datawriter_submessage // 编码 DataWriter 子消息测试
│   │       │   └───src // 源代码
│   │       ├───encode_rtps_message // 编码 RTPS 消息测试
│   │       │   └───src // 源代码
│   │       ├───encode_serialized_payload // 编码序列化负载测试
│   │       │   └───src // 源代码
│   │       ├───get_authenticated_peer_credential_token // 获取认证的对等凭据令牌测试
│   │       │   └───src // 源代码
│   │       ├───get_permissions_credential_token // 获取权限凭据令牌测试
│   │       │   ├───etc // 配置文件
│   │       │   └───src // 源代码
│   │       ├───get_permissions_token // 获取权限令牌测试
│   │       │   ├───etc // 配置文件
│   │       │   └───src // 源代码
│   │       ├───get_xxx_sec_attributes // 获取安全属性测试
│   │       │   ├───etc // 配置文件
│   │       │   └───src // 源代码
│   │       ├───listeners_access_control // 访问控制监听器测试
│   │       │   ├───etc // 配置文件
│   │       │   └───src // 源代码
│   │       ├───listeners_authentication // 认证监听器测试
│   │       │   ├───etc // 配置文件
│   │       │   └───src // 源代码
│   │       ├───preprocess_secure_submsg // 预处理安全子消息测试
│   │       │   └───src // 源代码
│   │       ├───process_handshake // 处理握手测试
│   │       │   ├───etc // 配置文件
│   │       │   │   ├───trusted_ca_dir // 可信 CA 目录
│   │       │   │   └───trusted_ca_dir_not_matching // 不匹配的可信 CA 目录
│   │       │   └───src // 源代码
│   │       ├───register_local_datareader // 注册本地 DataReader 测试
│   │       │   └───src // 源代码
│   │       ├───register_local_datawriter // 注册本地 DataWriter 测试
│   │       │   └───src // 源代码
│   │       ├───register_local_participant // 注册本地参与者测试
│   │       │   └───src // 源代码
│   │       ├───register_matched_remote_datareader // 注册匹配的远程 DataReader 测试
│   │       │   └───src // 源代码
│   │       ├───register_matched_remote_datawriter // 注册匹配的远程 DataWriter 测试
│   │       │   └───src // 源代码
│   │       ├───register_matched_remote_participant // 注册匹配的远程参与者测试
│   │       │   └───src // 源代码
│   │       ├───set_remote_datareader_crypto_tokens // 设置远程 DataReader 加密令牌测试
│   │       │   └───src // 源代码
│   │       ├───set_remote_datawriter_crypto_tokens // 设置远程 DataWriter 加密令牌测试
│   │       │   └───src // 源代码
│   │       ├───set_remote_participant_crypto_tokens // 设置远程参与者加密令牌测试
│   │       │   └───src // 源代码
│   │       ├───validate_begin_handshake_reply // 验证握手回复测试
│   │       │   ├───etc // 配置文件
│   │       │   │   └───trusted_ca_dir // 可信 CA 目录
│   │       │   └───src // 源代码
│   │       ├───validate_begin_handshake_request // 验证握手请求测试
│   │       │   └───src // 存放验证握手请求测试的源代码
│   │       ├───validate_local_identity // 验证本地身份模块
│   │       │   ├───etc // 存放配置文件
│   │       │   │   ├───new // 新配置文件目录
│   │       │   │   ├───old // 旧配置文件目录
│   │       │   │   ├───trusted_ca_dir // 可信任的CA证书目录
│   │       │   │   └───trusted_ca_dir_not_matching // 不匹配的可信任CA证书目录
│   │       │   └───src // 存放验证本地身份的源代码
│   │       ├───validate_local_permissions // 验证本地权限模块
│   │       │   ├───etc // 存放配置文件
│   │       │   └───src // 存放验证本地权限的源代码
│   │       ├───validate_remote_identity // 验证远程身份模块
│   │       │   └───src // 存放验证远程身份的源代码
│   │       └───validate_remote_permissions // 验证远程权限模块
│   │           ├───etc // 存放配置文件
│   │           └───src // 存放验证远程权限的源代码
│   ├───core
│   │   ├───include                     // 核心头文件目录
│   │   │   └───dds
│   │   │       └───security           // 安全相关头文件
│   │   │           └───core           // 安全核心头文件
│   │   ├───src                         // 核心源代码目录
│   │   └───tests                       // 核心测试代码目录
│   │       ├───common                  // 通用测试代码和资源
│   │       │   └───etc                 // 配置文件等资源
│   │       └───plugin_loading          // 插件加载相关测试代码
│   └───openssl                         // OpenSSL 支持模块
│       ├───include                     // OpenSSL 模块头文件目录
│       │   └───dds
│       │       └───security            // 安全相关头文件
│       └───src                         // OpenSSL 模块源代码目录
└───tools                   // 工具目录，包含 cyclone dds 相关的工具
    ├───ddsperf             // ddsperf 工具，用于性能测试和基准测试
    ├───idlc                // idlc 编译器，用于将 IDL 文件编译成 C 代码
    │   ├───include          // idlc 头文件目录
    │   │   └───idlc         // idlc 特定的头文件目录
    │   ├───src              // idlc 源代码目录
    │   ├───tests            // idlc 测试代码目录
    │   └───xtests           // idlc 扩展测试代码目录
    ├───idlpp               // idlpp 工具，用于处理 IDL 文件
    │   └───src              // idlpp 源代码目录
    └───_confgen            // _confgen 工具，用于生成配置文件
