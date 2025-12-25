# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## 项目概述

CommTest_Qt 是基于 Qt6 和 CMake 构建的工业通信协议测试仿真工具，用于模拟 PLC（可编程逻辑控制器）通信协议。项目最初在 Visual Studio 2022 开发，后迁移至 CMake。仅在 Windows 平台测试过。

## 构建命令

```bash
# 配置（在项目根目录）
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022" -A x64

# 构建
cmake --build . --config Release   # 或 Debug

# 输出位置
# 可执行文件: Bin/x64/PLC_Simulator.exe (Release) 或 PLC_Simulatord.exe (Debug)
```

**环境要求：**
- CMake 3.16+
- Qt6 SDK（Core, Gui, Widgets, Network, SerialPort）
- Visual Studio 2022 或支持 C++17 的编译器
- Lua 库（ScriptRunner.lib 位于 Lua/Lib/）

## 架构设计

### 核心模块

**MainFlow (`MainFlow/`)** - 单例调度器
- 管理协议创建和生命周期
- 持有寄存器数组：100,000 个 16 位原子寄存器（`std::vector<std::atomic_int16_t>`）
- 支持最多 6 个并发 Lua 脚本
- 通过线程池路由请求，每个端点独立队列

**Comm (`Comm/`)** - 通信层
- `CommBase` - 抽象基类，Qt 信号槽机制，请求队列管理
- `CommSocket` - TCP/IP Socket 实现（服务端/客户端模式）
- `Protocol/` - 协议实现：
  - `CommProMitsubishiQBinary` - 三菱 Q 系列二进制协议（3E 帧格式）
  - `CommProKeyencePCLink` - 基恩士 PC-LINK 协议

**LuaScript (`LuaScript/`)** - 脚本引擎
- 使用 Lua 5.4，通过 ScriptRunner 库封装
- `IDataProvider` 接口解耦脚本与 MainWorkFlow
- 注册函数：寄存器读写、循环控制、平台移动

**Config (`Config/`)** - JSON 配置持久化

**Gui (`Gui/`)** - Qt 界面组件
- `CommTest_Qt` - 主窗口（寄存器表格、协议选择、日志查看）
- `SubMainWindow` - 浮动工具栏，脚本快捷启动
- `ScriptEditor` - Lua 语法高亮编辑器
- `SimulationPlatform` - 2D 平台运动可视化

### 设计模式

1. **单例模式**：`MainWorkFlow`、`LuaScript` - 静态互斥锁保证线程安全
2. **提供者模式**：`IDataProvider` 抽象寄存器访问
3. **控制器接口**：`IBaseController` 用于平台运动注入
4. **线程池**：端点内串行化，跨端点并行化
5. **原子容器**：`std::atomic_int16_t` 实现无锁寄存器读取

### 信号流转

```
CommBase (dataReceived, dataSend, CommLogRecord)
    └── MainWorkFlow (转发信号)
            └── GUI (连接信号更新界面)
```

## Lua 脚本 API

寄存器函数：
- `SetInt16/32`、`SetFloat`、`SetDouble`、`SetString(addr, value)`
- `GetInt16/32`、`GetFloat`、`GetDouble`、`GetString(addr)`

控制函数：
- `IsLoopValid()` - 检查脚本循环是否启用
- `sleep(ms)` - 延时

平台函数：
- `MoveAbsInt32/Float(xAddr, yAddr, angleAddr)` - 绝对位置移动
- `MoveRelativeInt32/Float(xAddr, yAddr, angleAddr)` - 相对位置移动
- `WriteCurrentPosInt32/Float(xAddr, yAddr, angleAddr)` - 写入当前位置

寄存器地址范围：0-99999

## 开发说明

- Debug 构建链接 Visual Leak Detector (VLD) 进行内存泄漏检测
- 构建后自动复制到 `Bin/x64/` 并执行 `windeployqt` 部署 Qt 运行时
- main() 中通过共享内存 + 信号量实现单实例运行
- 串口通信基础设施已存在但未完全实现
