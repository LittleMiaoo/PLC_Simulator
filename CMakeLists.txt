cmake_minimum_required(VERSION 3.16)
project(CommTestSolution VERSION 1.0.0 LANGUAGES CXX) 

# 只保留 Debug 和 Release 配置,默认情况下有4个配置 Release, Debug, MinSizeRel, RelWithDebInfo
if(CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_CONFIGURATION_TYPES "Debug;Release")
    set(CMAKE_CONFIGURATION_TYPES "${CMAKE_CONFIGURATION_TYPES}" CACHE STRING 
        "Reset the configurations to what we need" FORCE)
    message(STATUS "限制配置类型为: ${CMAKE_CONFIGURATION_TYPES}")
endif()

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置输出目录（保持与原项目一致的结构）
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Temp/${PROJECT_NAME}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Temp/${PROJECT_NAME}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Temp/${PROJECT_NAME}/bin)

# 查找Qt包 - 使用与VS项目相同的版本
set(QT_VERSION 6)
find_package(Qt${QT_VERSION} REQUIRED COMPONENTS Core Gui Widgets Network SerialPort)

# 启用Qt自动处理
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# 设置并行编译
if(CMAKE_BUILD_PARALLEL_LEVEL)
    # 如果命令行已指定，则使用命令行值
    set(CMAKE_MAKE_PROGRAM "${CMAKE_MAKE_PROGRAM} -j${CMAKE_BUILD_PARALLEL_LEVEL}")
else()
    # 否则根据处理器核心数自动设置
    include(ProcessorCount)
    ProcessorCount(N)
    if(NOT N EQUAL 0)
        set(CMAKE_BUILD_PARALLEL_LEVEL ${N})
        message(STATUS "设置并行编译线程数: ${N}")
    endif()
endif()

# 设置默认的启动项目(MSVC有效)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT CommTest_Qt)

# 添加子项目
add_subdirectory(CommTest_Qt)