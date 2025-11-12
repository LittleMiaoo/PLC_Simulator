#project(CommTest_Qt LANGUAGES CXX)

# 设置目标名称（Debug版本加'd'后缀，与原项目一致）
set(TARGET_NAME "CommTest_Qt")

# 按原项目过滤器结构组织文件
set(SOURCE_FILES
    main.cpp
)

set(RESOURCE_FILES
    Resource_Files/CommTest_Qt.qrc
    Resource_Files/CommTest_Qt.rc
    Resource_Files/CommTest_Qt.ico
)

set(FORM_FILES
    Gui/CommTest_Qt.ui
)


# Comm 模块
set(COMM_SOURCES
    Comm/CommBase.cpp
)

set(COMM_HEADERS
    Comm/CommBase.h
    Comm/CommDefine.h
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
    Comm/Protocol/CommProKeyencePCLink.cpp
    Comm/Protocol/CommProMitsubishiQBinary.cpp
)

set(COMM_PROTOCOL_HEADERS
    Comm/Protocol/CommProtocolBase.h
    Comm/Protocol/CommProMitsubishiQBinary.h
    Comm/Protocol/CommProKeyencePCLink.h
)

# Gui 模块
set(GUI_SOURCES
    Gui/CommTest_Qt.cpp
    Gui/SubMainWindow.cpp
    Gui/ScriptEditor.cpp
    Gui/SimulationPlatform.cpp
)

set(GUI_HEADERS
    Gui/CommTest_Qt.h
    Gui/SubMainWindow.h
    Gui/ScriptEditor.h
    Gui/SimulationPlatform.h
)

# MainFlow 模块
set(MAINFLOW_SOURCES
    MainFlow/MainWorkFlow.cpp
)

set(MAINFLOW_HEADERS
    MainFlow/MainWorkFlow.h
)

# LuaScript 模块
set(LUA_SOURCES
    LuaScript/LuaScript.cpp
)

set(LUA_HEADERS
    LuaScript/LuaScript.h
)

# 合并所有文件
set(SOURCES
    ${SOURCE_FILES}
    ${COMM_SOURCES}
    ${COMM_SOCKET_SOURCES}
    ${COMM_PROTOCOL_SOURCES}
    ${GUI_SOURCES}
    ${MAINFLOW_SOURCES}
    ${LUA_SOURCES}
)

set(HEADERS
    ${COMM_HEADERS}
    ${COMM_SOCKET_HEADERS}
    ${COMM_PROTOCOL_HEADERS}
    ${GUI_HEADERS}
    ${MAINFLOW_HEADERS}
    ${LUA_HEADERS}
)

# 创建可执行目标
add_executable(${TARGET_NAME}
    ${SOURCES}
    ${HEADERS}
    ${RESOURCE_FILES}
    ${FORM_FILES}
)

# 设置目标属性
set_target_properties(${TARGET_NAME} PROPERTIES
    OUTPUT_NAME "CommTest_Qt$<$<CONFIG:Debug>:d>"
    WIN32_EXECUTABLE TRUE
)

# 设置MSVC编译选项
# 创建Visual Studio过滤器
if(MSVC)

target_compile_options(${TARGET_NAME} PRIVATE /MP)
message(STATUS "为 ${TARGET_NAME} 启用多处理器编译 (/MP)")

source_group("Source Files" FILES ${SOURCE_FILES})
source_group("Resource Files" FILES ${RESOURCE_FILES})
source_group("Form Files" FILES ${FORM_FILES})
source_group("Comm" FILES ${COMM_SOURCES} ${COMM_HEADERS})
source_group("Comm\\\\Socket" FILES ${COMM_SOCKET_SOURCES} ${COMM_SOCKET_HEADERS})
source_group("Comm\\\\Protocol" FILES ${COMM_PROTOCOL_SOURCES} ${COMM_PROTOCOL_HEADERS})
source_group("Gui" FILES ${GUI_SOURCES} ${GUI_HEADERS})
source_group("MainFlow" FILES ${MAINFLOW_SOURCES} ${MAINFLOW_HEADERS})
source_group("LuaScript" FILES ${LUA_SOURCES} ${LUA_HEADERS})

endif()

# 链接Qt库
target_link_libraries(${TARGET_NAME} PRIVATE
    Qt${QT_VERSION}::Core
    Qt${QT_VERSION}::Gui
    Qt${QT_VERSION}::Widgets
    Qt${QT_VERSION}::Network
    Qt${QT_VERSION}::SerialPort
)



# 包含目录
target_include_directories(${TARGET_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Gui
    ${CMAKE_CURRENT_SOURCE_DIR}/Comm
    ${CMAKE_CURRENT_SOURCE_DIR}/Comm/Socket
    ${CMAKE_CURRENT_SOURCE_DIR}/Comm/Protocol
    ${CMAKE_CURRENT_SOURCE_DIR}/MainFlow
    ${CMAKE_CURRENT_SOURCE_DIR}/LuaScript
    ${CMAKE_SOURCE_DIR}/Lua/Include
)

# 链接目录和库（Lua相关）
target_link_directories(${TARGET_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/Lua/Lib
)

target_link_libraries(${TARGET_NAME} PRIVATE
    ScriptRunner.lib
)

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

# 后期构建步骤：复制到Bin目录
add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:${TARGET_NAME}>
        ${CMAKE_SOURCE_DIR}/Bin/x64/
    COMMENT "Copying executable to Bin/x64 directory"
)