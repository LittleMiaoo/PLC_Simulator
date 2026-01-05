/*
 * PLC Simulator - Industrial Communication Protocol Testing Tool
 * Copyright (c) 2025-2026 Wang Mao <mao.wang.dev@foxmail.com>
 *
 * This file is part of PLC Simulator.
 * Licensed under the MIT License. See LICENSE file in the project root.
 */

#ifndef VERSION_H
#define VERSION_H

// 版本号定义
#define APP_VERSION_MAJOR   1
#define APP_VERSION_MINOR   5
#define APP_VERSION_PATCH   2
#define APP_VERSION_BUILD   0

// 版本字符串拼接宏
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define APP_VERSION_STRING  TOSTRING(APP_VERSION_MAJOR) "." TOSTRING(APP_VERSION_MINOR) "." TOSTRING(APP_VERSION_PATCH)

// Windows RC 资源文件使用的版本格式 (逗号分隔)
#define APP_VERSION_RC      APP_VERSION_MAJOR,APP_VERSION_MINOR,APP_VERSION_PATCH,APP_VERSION_BUILD
#define APP_VERSION_RC_STR  APP_VERSION_STRING

// Windows RC 资源文件使用的信息（仅 ASCII/英文，避免编码问题）
#define APP_NAME_RC         "PLC Simulator"
#define APP_DESCRIPTION_RC  "A Qt6-based PLC simulation communication test software."
#define APP_COPYRIGHT_RC    "Copyright (C) 2025-2026 WangMao. All rights reserved."
#define APP_INTERNAL_NAME   "PLC_Simulator"
#define APP_ORIGINAL_NAME   "PLC_Simulator.exe"
#define APP_ORGANIZATION    "WangMao<mao.wang.dev@foxmail.com>"

// 以下定义仅供 C/C++ 代码使用，RC 编译器不支持中文
#ifndef RC_INVOKED
#define APP_NAME            "PLC通信模拟器"
#define APP_VERSION         APP_VERSION_STRING
#define APP_AUTHOR          APP_ORGANIZATION
#define APP_DOMAIN          "https://github.com/LittleMiaoo/PLC_Simulator"
#define APP_DESCRIPTION     "一个基于Qt6的PLC模拟通信测试软件.\n"\
                            "支持基恩士上位链路协议&三菱Q系列MC协议二进制通信.\n"\
                            "支持通过Lua脚本语言实现自动化.\n"\
                            "本软件采用 MIT 许可证发布,详情查看 LICENSE 文件."

// 编译时间（自动生成）
#define APP_COMPILE_DATE    __DATE__
#define APP_COMPILE_TIME    __TIME__
#endif // RC_INVOKED

#endif // VERSION_H
