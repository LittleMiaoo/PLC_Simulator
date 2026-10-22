project(PLC_Simulator LANGUAGES CXX)

# 设置目标名称（Debug版本加'd'后缀，与原项目一致）
set(TARGET_NAME "PLC_Simulator")

# 按原项目过滤器结构组织文件
set(SOURCE_FILES
    main.cpp
    I18n/LanguageManager.cpp
)

# 根目录工具头文件
set(ROOT_HEADERS
    I18n/LanguageManager.h
)

set(RESOURCE_FILES
    Resource_Files/resources.qrc
    Resource_Files/PLC_Simulator.ico
    app.rc
    version.h
)


# Comm 模块
set(COMM_SOURCES
    Comm/CommBase.cpp
    Comm/CommInfoFactory.cpp
)

set(COMM_HEADERS
    Comm/CommBase.h
    Comm/CommEvent.h
    Comm/CommInfoFactory.h
)

# Comm\Socket 模块
set(COMM_SOCKET_SOURCES
    Comm/Socket/CommSocket.cpp
)

set(COMM_SOCKET_HEADERS
    Comm/Socket/CommSocket.h
)

# Comm\Serial 模块（根据你的过滤器，可能未来扩展）

# Comm\Protocol 模块
set(COMM_PROTOCOL_SOURCES
    Comm/Protocol/CommProtocolKeyencePCLink.cpp
    Comm/Protocol/CommProtocolMitsubishiQBinary.cpp
    Comm/Protocol/CommProtocolFactory.cpp
)

set(COMM_PROTOCOL_HEADERS
    Comm/Protocol/CommProtocolBase.h
    Comm/Protocol/CommProtocolMitsubishiQBinary.h
    Comm/Protocol/CommProtocolKeyencePCLink.h
    Comm/Protocol/CommProtocolFactory.h
)

# Gui\MainWindow 模块(主窗口及其管理器)
set(GUI_MAINWINDOW_SOURCES
    Gui/MainWindow/MainWindow.cpp
    Gui/MainWindow/MainWindow.ui
    Gui/MainWindow/QuickPanel.cpp
    Gui/MainWindow/RegisterTable/RegisterTableController.cpp
    Gui/MainWindow/RegisterTable/RegisterItemDelegate.cpp
    Gui/MainWindow/RegisterTable/RegisterTableModel.cpp
    Gui/MainWindow/ScriptManager.cpp
    Gui/MainWindow/AuxDialogs.cpp
    Gui/MainWindow/StatusBar/StatusSegment.cpp
    Gui/MainWindow/StatusBar/StatusBarController.cpp
)

set(GUI_MAINWINDOW_HEADERS
    Gui/MainWindow/MainWindow.h
    Gui/MainWindow/QuickPanel.h
    Gui/MainWindow/RegisterTable/RegisterTableController.h
    Gui/MainWindow/RegisterTable/RegisterTableModel.h
    Gui/MainWindow/RegisterTable/RegisterTableTypes.h
    Gui/MainWindow/RegisterTable/RegisterItemDelegate.h
    Gui/MainWindow/ScriptManager.h
    Gui/MainWindow/AuxDialogs.h
    Gui/MainWindow/StatusBar/StatusSegment.h
    Gui/MainWindow/StatusBar/StatusBarController.h
)

# Gui\Theme 模块
set(GUI_THEME_SOURCES
    Gui/Theme/ThemeManager.cpp
)
set(GUI_THEME_HEADERS
    Gui/Theme/ThemeManager.h
)

# Gui\Widgets 模块
set(GUI_WIDGETS_SOURCES
    Gui/Widgets/CollapsibleGroupBox.cpp
)
set(GUI_WIDGETS_HEADERS
    Gui/Widgets/CollapsibleGroupBox.h
)

# Gui\ScriptEditor 模块
set(GUI_SCRIPTEDITOR_SOURCES
    Gui/ScriptEditor/ScriptEditor.cpp
    Gui/ScriptEditor/CodeEditor.cpp
    Gui/ScriptEditor/LuaHighlighter.cpp
)

set(GUI_SCRIPTEDITOR_HEADERS
    Gui/ScriptEditor/ScriptEditor.h
    Gui/ScriptEditor/CodeEditor.h
    Gui/ScriptEditor/LuaHighlighter.h
)

# Gui\SimulationPlatform 模块
set(GUI_SIMULATIONPLATFORM_SOURCES
    Gui/SimulationPlatform/SimulationPlatform.cpp
    Gui/SimulationPlatform/PlatformCanvas.cpp
    Gui/SimulationPlatform/ImageViewer.cpp
    Gui/SimulationPlatform/ImagePage.cpp
    Gui/SimulationPlatform/PlatformScene.cpp
    Gui/SimulationPlatform/PlatformControlPanel.cpp
    Gui/SimulationPlatform/PlatformParamsDialog.cpp
)

set(GUI_SIMULATIONPLATFORM_HEADERS
    Gui/SimulationPlatform/SimulationPlatform.h
    Gui/SimulationPlatform/PlatformCanvas.h
    Gui/SimulationPlatform/ImageViewer.h
    Gui/SimulationPlatform/ImagePage.h
    Gui/SimulationPlatform/PlatformScene.h
    Gui/SimulationPlatform/PlatformControlPanel.h
    Gui/SimulationPlatform/PlatformParamsDialog.h
)

# MainFlow 模块
set(MAINFLOW_SOURCES
    MainFlow/MainWorkflow.cpp
)

set(MAINFLOW_HEADERS
    MainFlow/MainWorkflow.h
)

# LuaScript 模块
set(LUA_ENGINE_SOURCES
    LuaScript/Engine/LuaEngine.cpp
    LuaScript/Engine/ScriptEngineHost.cpp
    LuaScript/Engine/LuaStaticCheck.cpp
)

set(LUA_ENGINE_HEADERS
    LuaScript/Engine/LuaEngine.h
    LuaScript/Engine/ILuaBinding.h
    LuaScript/Engine/ScriptEngineHost.h
    LuaScript/Engine/LuaStaticCheck.h
)

set(LUA_MODULE_SOURCES
    LuaScript/Modules/LuaBindingUtil.cpp
    LuaScript/Modules/RegisterBinding.cpp
    LuaScript/Modules/PlatformBinding.cpp
)

set(LUA_MODULE_HEADERS
    LuaScript/Modules/LuaBindingUtil.h
    LuaScript/Modules/RegisterBinding.h
    LuaScript/Modules/PlatformBinding.h
)

set(LUA_SOURCES ${LUA_ENGINE_SOURCES} ${LUA_MODULE_SOURCES})
set(LUA_HEADERS ${LUA_ENGINE_HEADERS} ${LUA_MODULE_HEADERS})
# Config 模块
set(CONFIG_SOURCES
    Config/ConfigStore.cpp
)

set(CONFIG_HEADERS
    Config/ConfigStore.h
)

# Core 模块
set(CORE_SOURCES
    Core/RegisterStore.cpp
    Core/PlatformController.cpp
)

set(CORE_HEADERS
    Core/DataTypeConvert.h
    Core/RegisterStore.h
    Core/PlatformController.h
    Core/PlatformTypes.h
)

# 合并所有文件
set(SOURCES
    ${SOURCE_FILES}
    ${COMM_SOURCES}
    ${COMM_SOCKET_SOURCES}
    ${COMM_PROTOCOL_SOURCES}
    ${GUI_MAINWINDOW_SOURCES}
    ${GUI_THEME_SOURCES}
    ${GUI_WIDGETS_SOURCES}
    ${GUI_SCRIPTEDITOR_SOURCES}
    ${GUI_SIMULATIONPLATFORM_SOURCES}
    ${MAINFLOW_SOURCES}
    ${LUA_SOURCES}
    ${CONFIG_SOURCES}
    ${CORE_SOURCES}
)

set(HEADERS
    ${ROOT_HEADERS}
    ${COMM_HEADERS}
    ${COMM_SOCKET_HEADERS}
    ${COMM_PROTOCOL_HEADERS}
    ${GUI_MAINWINDOW_HEADERS}
    ${GUI_THEME_HEADERS}
    ${GUI_WIDGETS_HEADERS}
    ${GUI_SCRIPTEDITOR_HEADERS}
    ${GUI_SIMULATIONPLATFORM_HEADERS}
    ${MAINFLOW_HEADERS}
    ${LUA_HEADERS}
    ${CONFIG_HEADERS}
    ${CORE_HEADERS}
)

# 创建可执行目标
add_executable(${TARGET_NAME}
    ${SOURCES}
    ${HEADERS}
    ${RESOURCE_FILES}
)

# 界面翻译:构建时 lrelease 生成 .qm 并以 :/i18n 前缀嵌入资源。
# 新增/修改 tr() 文案后执行 cmake --build <构建目录> --target update_translations 更新 .ts,再用 Qt Linguist 补译文;
# -locations none:.ts 不记源码行号,避免代码行变动造成 .ts 无关改动
qt_add_translations(${TARGET_NAME}
    TS_FILES translations/plc_simulator_en.ts
    RESOURCE_PREFIX "/i18n"
    LUPDATE_OPTIONS -locations none
)

# 设置目标属性
set_target_properties(${TARGET_NAME} PROPERTIES
    OUTPUT_NAME "PLCSimulator$<$<CONFIG:Debug>:d>"
    WIN32_EXECUTABLE TRUE
)

# 设置MSVC编译选项
# 创建Visual Studio过滤器
if(MSVC)

target_compile_options(${TARGET_NAME} PRIVATE /MP)
message(STATUS "为 ${TARGET_NAME} 启用多处理器编译 (/MP)")

source_group("Source Files" FILES ${SOURCE_FILES})
source_group("Resource Files" FILES ${RESOURCE_FILES})
source_group("Comm" FILES ${COMM_SOURCES} ${COMM_HEADERS})
source_group("Comm\\\\Socket" FILES ${COMM_SOCKET_SOURCES} ${COMM_SOCKET_HEADERS})
source_group("Comm\\\\Protocol" FILES ${COMM_PROTOCOL_SOURCES} ${COMM_PROTOCOL_HEADERS})
source_group("Gui\\\\MainWindow" FILES ${GUI_MAINWINDOW_SOURCES} ${GUI_MAINWINDOW_HEADERS})
source_group("Gui\\\\Theme" FILES ${GUI_THEME_SOURCES} ${GUI_THEME_HEADERS})
source_group("Gui\\\\Widgets" FILES ${GUI_WIDGETS_SOURCES} ${GUI_WIDGETS_HEADERS})
source_group("Gui\\\\ScriptEditor" FILES ${GUI_SCRIPTEDITOR_SOURCES} ${GUI_SCRIPTEDITOR_HEADERS})
source_group("Gui\\\\SimulationPlatform" FILES ${GUI_SIMULATIONPLATFORM_SOURCES} ${GUI_SIMULATIONPLATFORM_HEADERS})
source_group("MainFlow" FILES ${MAINFLOW_SOURCES} ${MAINFLOW_HEADERS})
source_group("LuaScript\\\\Engine" FILES ${LUA_ENGINE_SOURCES} ${LUA_ENGINE_HEADERS})
source_group("LuaScript\\\\Modules" FILES ${LUA_MODULE_SOURCES} ${LUA_MODULE_HEADERS})
source_group("Config" FILES ${CONFIG_SOURCES} ${CONFIG_HEADERS})
source_group("Core" FILES ${CORE_SOURCES} ${CORE_HEADERS})
source_group("Header Files" FILES ${ROOT_HEADERS})

endif()

# 链接Qt库
target_link_libraries(${TARGET_NAME} PRIVATE
    Qt${QT_VERSION}::Core
    Qt${QT_VERSION}::Gui
    Qt${QT_VERSION}::Widgets
    Qt${QT_VERSION}::Network
    Qt${QT_VERSION}::SerialPort
)

# Windows DWM(深色原生标题栏 DwmSetWindowAttribute)
if(WIN32)
    target_link_libraries(${TARGET_NAME} PRIVATE dwmapi)
endif()


# ========== 第二步：仅在 Debug 配置下链接 VLD（适配多配置生成器） ==========
if(WIN32 AND VLD_FOUND)
    # 生成器表达式 $<CONFIG:Debug>：仅在 Debug 配置时生效
    target_link_libraries(${TARGET_NAME}
        PRIVATE
        $<$<CONFIG:Debug>:VLD::VLD>  # 核心：动态判断 Debug 配置
    )
    # 定义 VLD_ENABLED 宏，让代码知道 VLD 可用
    target_compile_definitions(${TARGET_NAME} PRIVATE
        $<$<CONFIG:Debug>:VLD_ENABLED>
    )

    # 复制VLD的DLL和manifest到输出目录和Bin/dbg_env目录（仅Debug需要）
    get_filename_component(VLD_BIN_DIR "${VLD_INCLUDE_DIR}/../bin/Win64" ABSOLUTE)
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
        # 确保dbg_env目录存在
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_SOURCE_DIR}/Bin/dbg_env"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${VLD_BIN_DIR}/vld_x64.dll"
            "$<TARGET_FILE_DIR:${TARGET_NAME}>"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${VLD_BIN_DIR}/dbghelp.dll"
            "$<TARGET_FILE_DIR:${TARGET_NAME}>"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${VLD_BIN_DIR}/Microsoft.DTfW.DHL.manifest"
            "$<TARGET_FILE_DIR:${TARGET_NAME}>"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${VLD_BIN_DIR}/vld_x64.dll"
            "${CMAKE_SOURCE_DIR}/Bin/dbg_env/"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${VLD_BIN_DIR}/dbghelp.dll"
            "${CMAKE_SOURCE_DIR}/Bin/dbg_env/"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${VLD_BIN_DIR}/Microsoft.DTfW.DHL.manifest"
            "${CMAKE_SOURCE_DIR}/Bin/dbg_env/"
        COMMENT "Copying VLD DLLs and manifest to output and Bin/dbg_env directories"
    )

    # 可选：打印验证信息（构建阶段才会输出，因为依赖配置）
    message(STATUS "VLD will be linked in Debug configuration")
    message(STATUS "VLD DLLs will be copied from: ${VLD_BIN_DIR}")
endif()

# 包含目录
target_include_directories(${TARGET_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Gui
    ${CMAKE_CURRENT_SOURCE_DIR}/Gui/MainWindow
    ${CMAKE_CURRENT_SOURCE_DIR}/Gui/MainWindow/RegisterTable
    ${CMAKE_CURRENT_SOURCE_DIR}/Gui/MainWindow/StatusBar
    ${CMAKE_CURRENT_SOURCE_DIR}/Gui/Theme
    ${CMAKE_CURRENT_SOURCE_DIR}/Gui/Widgets
    ${CMAKE_CURRENT_SOURCE_DIR}/Gui/ScriptEditor
    ${CMAKE_CURRENT_SOURCE_DIR}/Gui/SimulationPlatform
    ${CMAKE_CURRENT_SOURCE_DIR}/Comm
    ${CMAKE_CURRENT_SOURCE_DIR}/Comm/Socket
    ${CMAKE_CURRENT_SOURCE_DIR}/Comm/Protocol
    ${CMAKE_CURRENT_SOURCE_DIR}/MainFlow
    ${CMAKE_CURRENT_SOURCE_DIR}/LuaScript/Engine
    ${CMAKE_CURRENT_SOURCE_DIR}/LuaScript/Modules
    ${CMAKE_SOURCE_DIR}/thirdparty/Lua/Include
)

# 链接目录和库（Lua相关）
target_link_directories(${TARGET_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/thirdparty/Lua/Lib
)

target_link_libraries(${TARGET_NAME} PRIVATE ScriptRunner.lib)

# 设置编译选项（与原项目保持一致）
target_compile_options(${TARGET_NAME} PRIVATE
    $<$<CONFIG:Debug>:/MDd>
    $<$<CONFIG:Release>:/MD>
)

target_compile_definitions(${TARGET_NAME} PRIVATE
    UNICODE
    _UNICODE
)

# 设置链接选项
target_link_options(${TARGET_NAME} PRIVATE
    /SUBSYSTEM:WINDOWS
    $<$<CONFIG:Debug>:/DEBUG>
    $<$<CONFIG:Release>:/OPT:REF /OPT:ICF>
)

if(MSVC)
set_target_properties(${TARGET_NAME} PROPERTIES
    VS_DEBUGGER_COMMAND "${CMAKE_SOURCE_DIR}/Bin/$<IF:$<CONFIG:Debug>,dbg_env,x64>/$<TARGET_FILE_NAME:${TARGET_NAME}>"
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/Bin/$<IF:$<CONFIG:Debug>,dbg_env,x64>"
)
endif()

# 后期构建步骤：根据配置复制到不同目录
# Debug版本复制到Bin/dbg_env，Release版本复制到Bin/x64
add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
    # 先确保目标目录存在
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/Bin/$<IF:$<CONFIG:Debug>,dbg_env,x64>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:${TARGET_NAME}>
        ${CMAKE_SOURCE_DIR}/Bin/$<IF:$<CONFIG:Debug>,dbg_env,x64>/
    COMMENT "Copying executable to Bin/$<IF:$<CONFIG:Debug>,dbg_env,x64> directory"
)

# 自动部署 Qt 运行时与插件（优先使用 windeployqt）
find_program(WINDEPLOYQT_PATH
    NAMES windeployqt windeployqt.exe
    HINTS
        "$ENV{Qt6_DIR}/bin"
        "$ENV{QTDIR}/bin"
        "${Qt6_DIR}/bin"
        "${Qt6_DIR}/../../../bin"
)

if(WINDEPLOYQT_PATH)
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
        COMMAND "${WINDEPLOYQT_PATH}"
            $<$<CONFIG:Debug>:--debug>
            $<$<CONFIG:Release>:--release>
            --no-compiler-runtime
            --dir "${CMAKE_SOURCE_DIR}/Bin/$<IF:$<CONFIG:Debug>,dbg_env,x64>"
            "$<TARGET_FILE:${TARGET_NAME}>"
        COMMENT "Deploying Qt runtime with windeployqt"
    )
else()
    message(WARNING "windeployqt not found. Qt runtime will not be auto-deployed.")
endif()

# 复制 ScriptRunner.dll（若存在）到发布目录
if(EXISTS "${CMAKE_SOURCE_DIR}/thirdparty/Lua/Lib/ScriptRunner.dll")
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_SOURCE_DIR}/thirdparty/Lua/Lib/ScriptRunner.dll"
            "${CMAKE_SOURCE_DIR}/Bin/$<IF:$<CONFIG:Debug>,dbg_env,x64>/"
        COMMENT "Copying ScriptRunner.dll to Bin/$<IF:$<CONFIG:Debug>,dbg_env,x64>"
    )
endif()

# 复制文档文件到发布目录
if(EXISTS "${CMAKE_SOURCE_DIR}/THIRD_PARTY_LICENSES.txt")
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_SOURCE_DIR}/THIRD_PARTY_LICENSES.txt"
            "${CMAKE_SOURCE_DIR}/Bin/$<IF:$<CONFIG:Debug>,dbg_env,x64>/"
        COMMENT "Copying THIRD_PARTY_LICENSES.txt to Bin/$<IF:$<CONFIG:Debug>,dbg_env,x64>"
    )
endif()

if(EXISTS "${CMAKE_SOURCE_DIR}/LICENSE")
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_SOURCE_DIR}/LICENSE"
            "${CMAKE_SOURCE_DIR}/Bin/$<IF:$<CONFIG:Debug>,dbg_env,x64>/"
        COMMENT "Copying LICENSE to Bin/$<IF:$<CONFIG:Debug>,dbg_env,x64>"
    )
endif()

if(EXISTS "${CMAKE_SOURCE_DIR}/ChangeLog.txt")
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_SOURCE_DIR}/ChangeLog.txt"
            "${CMAKE_SOURCE_DIR}/Bin/$<IF:$<CONFIG:Debug>,dbg_env,x64>/"
        COMMENT "Copying ChangeLog.txt to Bin/$<IF:$<CONFIG:Debug>,dbg_env,x64>"
    )
endif()

# Release构建后清理不需要的文件，减小发布包体积
add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND}
        -DBUILD_CONFIG=$<CONFIG>
        -DBIN_DIR=${CMAKE_SOURCE_DIR}/Bin
        -P ${CMAKE_CURRENT_SOURCE_DIR}/cleanup_release_unnecessary.cmake
    COMMENT "Cleaning unnecessary files from Release package (if Release mode)"
)