# CommTest_Qt - 工业通信协议测试仿真工具

## 项目简介

CommTest_Qt 是一个基于 Qt 和 CMake 构建的工业通信协议测试与仿真工具。该项目主要用于模拟和测试各种工业设备通信协议，特别是 PLC（可编程逻辑控制器）相关的通信协议。

本项目最初基于Visual Studio 2022开发，后迁移至CMake。仅在windows下做过测试&使用，其他平台暂未测试。


## 功能特点

- 支持多种工业通信协议仿真：
  - Keyence PCLink 协议
  - Mitsubishi Q 系列二进制协议
- 图形化用户界面，便于操作和监控
- 支持 TCP/IP 网络通信和串口通信(待实现)
- 集成 Lua 脚本引擎，支持自定义测试脚本
- 模拟运动平台控制功能


## 技术栈

- **编程语言**: C++17
- **框架**: Qt6 (Core, Gui, Widgets, Network, SerialPort)
- **构建系统**: CMake 3.16+
- **脚本支持**: Lua
- **开发环境**: Visual Studio 2022 或 VSCode

## 项目结构

```
CommTest_Qt/
├── Comm/                  # 通信模块
│   ├── Protocol/          # 各种通信协议实现
│   ├── Socket/            # Socket通信封装
│   ├── CommBase.h/cpp     # 通信基础类
│   └── CommDefine.h       # 通信相关定义
├── Config/                # 配置管理模块
├── Gui/                   # 用户界面模块
├── LuaScript/             # Lua脚本接口模块
├── MainFlow/              # 主工作流程控制模块
├── Resource_Files/        # 资源文件
└── CMakeLists.txt         # CMake构建配置
```

## 构建说明

### 环境要求

1. CMake 3.16 或更高版本
2. Qt6 开发环境（包含 Core, Gui, Widgets, Network, SerialPort 模块）
3. Visual Studio 2022 或支持 C++17 的编译器
4. Lua 库文件

### 构建步骤

```bash
# 克隆或下载项目到本地
cd CommTest_Qt

# 创建构建目录
mkdir build
cd build

# 配置项目
cmake .. -G "Visual Studio 17 2022" -A x64

# 编译项目
cmake --build . --config Release
```

编译后的可执行文件将位于 `Bin/x64/` 目录下。

## 使用说明

1. 启动应用程序后，可以通过图形界面选择不同的通信协议进行测试
2. 支持配置网络连接参数或串口参数
3. 可以通过 Lua 脚本编写自定义测试逻辑
4. 提供模拟运动平台功能，可用于测试设备控制指令
