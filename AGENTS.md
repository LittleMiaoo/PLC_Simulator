# AGENTS

本仓库是一个基于 Qt 6 的 PLC 模拟器与通信测试项目，采用 CMake 组织构建，目标名为 `PLC_Simulator`（Debug 版输出名为 `PLC_Simulatord`）。文档按“Agent（模块/职责单元）”视角，概述关键组件、职责边界、交互关系与构建信息。

## 构建 Agent
- 顶层工程：`CMakeLists.txt` 设置项目名 `CommTestSolution`、C++17、并行编译、VS 启动项目与 Qt 自动处理，添加子目录 `CommTest_Qt`（根/CMakeLists.txt:1–50）。
- 目标与依赖：子工程生成可执行 `PLC_Simulator`，链接 `Qt6::Core/Gui/Widgets/Network/SerialPort` 与外部 `ScriptRunner.lib`，包含 Lua 头文件目录（CommTest_Qt/CMakeLists.txt:108–172, 152–173）。
- 资源与表单：启用 `AUTOMOC/AUTOUIC/AUTORCC`，资源文件与 `.ui` 表单随目标一起编译（CommTest_Qt/CMakeLists.txt:111–114）。
- 输出与发布：后期构建拷贝到 `Bin/x64/`，与 Qt 运行时及插件目录并置（CommTest_Qt/CMakeLists.txt:192–198；根目录 `Bin/x64/...`）。

## 应用 Agent（UI 主程序）
- 入口：`main.cpp:11` 创建 `QApplication`，实例化并显示 `CommTest_Qt` 主窗口，Windows Debug 集成内存泄漏检测（CommTest_Qt/main.cpp:11–31）。
- 主窗体：`Gui/CommTest_Qt.h` 管理脚本编辑器、子窗口、模拟平台、寄存器表格与协议选择，负责信号连接、脚本初始化与寄存器显示/校验（CommTest_Qt/Gui/CommTest_Qt.h:29–145）。

## 工作流 Agent（核心协调）
- 单例：`MainFlow/MainWorkFlow.h` 通过 `InitialWorkFlow/ReleaseWorkFlow` 管理唯一实例与线程安全（MainFlow/MainWorkFlow.h:38–86）。
- 职责：
  - 通信信息设置与通信打开/关闭（MainWorkFlow/MainWorkFlow.h:42–49）。
  - 协议创建与数据处理入口 `WorkProcess`，解析指令并完成寄存器读写、发送回复（MainWorkFlow/MainWorkFlow.h:51–79）。
  - 寄存器存储与访问，使用 `std::atomic_int16_t` 向量维护 100000 个寄存器（MainWorkFlow/MainWorkFlow.h:60–69, 95–104）。
  - Lua 脚本管理与信号槽连接（MainWorkFlow/MainWorkFlow.h:66–93）。

## 通信 Agent
- 抽象层：`Comm/CommBase.h` 定义通信类型与状态、基础信号（日志/收发）以及统一接口（Open/Close/IsOpen/SendData）（CommTest_Qt/Comm/CommBase.h:4–58）。
- Socket 实现：`Comm/Socket/CommSocket.h` 支持服务端与客户端模式，管理连接、请求队列与超时定时器；提供 `Open/Close/SendData/IsOpen` 具体实现（CommTest_Qt/Comm/Socket/CommSocket.h:14–108）。
- 扩展点：串口等其他通信方式可按 `CommBase` 约定扩展，当前未提供具体实现文件夹但保留类型枚举（CommTest_Qt/Comm/CommBase.h:16–21）。

## 协议 Agent
- 抽象层：`Comm/Protocol/CommProtocolBase.h` 统一指令枚举、寄存器数据类型、收发处理流程；声明读/写寄存器解析与回复打包的纯虚接口（CommTest_Qt/Comm/Protocol/CommProtocolBase.h:5–103）。
- 三菱 MC 3E（Binary）：`CommProMitsubishiQBinary` 负责校验与解析/打包读写寄存器指令（CommTest_Qt/Comm/Protocol/CommProMitsubishiQBinary.h:20–53）。
- 基恩士 PC-Link：`CommProKeyencePCLink` 提供读写寄存器指令解析与回复打包（CommTest_Qt/Comm/Protocol/CommProKeyencePCLink.h:20–36）。
- 选择与创建：由工作流根据 `ProtocolType` 创建具体协议实例并在 `WorkProcess` 中使用（MainFlow/MainWorkFlow.h:51–55, 103–104）。

## Lua Agent（脚本扩展）
- 封装类：`LuaScript` 管理 `lua_State`，注册寄存器读写、平台控制与循环状态等函数，并提供文件/编辑器脚本执行（CommTest_Qt/LuaScript/LuaScript.h:7–141）。
- 依赖：头文件来自 `Lua/Include`，链接 `Lua/Lib/ScriptRunner.lib` 与同目录 `ScriptRunner.dll`（CommTest_Qt/CMakeLists.txt:152–173；根/Lua/Include, Lua/Lib）。
- 集成：工作流持有并连接多个 `LuaScript` 实例，主窗体提供脚本编辑器与调用入口（MainFlow/MainWorkFlow.h:87–93；Gui/CommTest_Qt.h:44–48, 97–101）。

## GUI Agent（界面与交互）
- 主窗体：`CommTest_Qt` 负责寄存器表格显示、输入校验、协议选择和脚本编辑等（CommTest_Qt/Gui/CommTest_Qt.h:65–145）。
- 子窗口：`SubMainWindow` 提供辅助 UI（CommTest_Qt/Gui/SubMainWindow.h）。
- 模拟平台：`SimulationPlatform` 绘制坐标系/平台/Mark，暴露绝对/相对移动与数据获取接口，用于脚本或 UI 控制的可视化（CommTest_Qt/Gui/SimulationPlatform.h:31–130）。
- 脚本编辑器：`ScriptEditor` 与 Lua 集成，支持从 UI 打开与执行脚本（CommTest_Qt/Gui/ScriptEditor.h）。

## 资源 Agent
- 资源集合：`Resource_Files/CommTest_Qt.qrc` 管理图标与界面资源；`CommTest_Qt.rc` 配置 Windows 资源与应用图标（CommTest_Qt/Resource_Files/*）。

## 运行时交互总览
- 数据接收：通信层触发 `CommBase::dataReceived` 信号（CommTest_Qt/Comm/CommBase.h:10）。
- 工作流处理：`MainWorkFlow::WorkProcess` 解析为读/写指令并调用具体协议实现（MainFlow/MainWorkFlow.h:54–79）。
- 寄存器更新：工作流维护寄存器并发出 `RegisterDataUpdate`，UI 侧刷新表格（MainFlow/MainWorkFlow.h:110–113；Gui/CommTest_Qt.h:65–140）。
- 数据发送：工作流通过通信层发送回复，触发 `CommBase::dataSend`（CommTest_Qt/Comm/CommBase.h:11）。
- 脚本与平台：Lua 函数读写寄存器、控制 `SimulationPlatform` 的位置与角度，形成闭环模拟（LuaScript.h:64–88；Gui/CommTest_Qt.h:50–54, 74）。

## 关键外部依赖
- Qt 6：`Core/Gui/Widgets/Network/SerialPort`，已通过 `find_package` 引入（根/CMakeLists.txt:24–31；CommTest_Qt/CMakeLists.txt:141–148）。
- Lua/ScriptRunner：头文件与静态库路径由工程提供（CommTest_Qt/CMakeLists.txt:152–173；根/Lua/*）。
- Windows 子系统：链接选项指定 `SUBSYSTEM:WINDOWS`，Debug 打开 `/DEBUG`（CommTest_Qt/CMakeLists.txt:185–190）。

## 产物与运行
- 构建后可执行拷贝至 `Bin/x64/`，依赖 Qt DLL 与插件目录同级；Debug 版输出名含 `d` 后缀。
- 在 VS 中将启动项目设为 `CommTest_Qt`（根/CMakeLists.txt:46–48），或使用 CMake 配置生成 Visual Studio 方案后启动 `PLC_Simulator`。

