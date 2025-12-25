cmake_minimum_required(VERSION 3.16)
project(CommTestSolution VERSION 1.0.0 LANGUAGES CXX) 

# 只保留 Debug 和 Release 配置
if(CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_CONFIGURATION_TYPES "Debug;Release")
    set(CMAKE_CONFIGURATION_TYPES "${CMAKE_CONFIGURATION_TYPES}" CACHE STRING 
        "Reset the configurations to what we need" FORCE)
    message(STATUS "限制配置类型为: ${CMAKE_CONFIGURATION_TYPES}")
endif()

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# 设置输出目录（使用生成器表达式区分配置）
set(OUTPUT_DIR ${CMAKE_BINARY_DIR}/Temp/${PROJECT_NAME})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIR}/lib/$<CONFIG>)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR}/lib/$<CONFIG>)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}/bin/$<CONFIG>)

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

# ========== 通用查找 VLD（适配 Windows + VS） ==========
if(WIN32)
    # 手动指定 VLD 可能的安装路径（优先级高于系统默认）
    set(VLD_PATHS 
        "C:/Program Files (x86)/Visual Leak Detector"
        "C:/Program Files/Visual Leak Detector"
    )

    # 查找 VLD 的头文件和库文件
    find_path(VLD_INCLUDE_DIR 
        NAMES vld.h
        PATHS ${VLD_PATHS}
        PATH_SUFFIXES include
    )
    find_library(VLD_LIBRARY 
        NAMES vld.lib
        PATHS ${VLD_PATHS}
        PATH_SUFFIXES lib/Win64  # 针对 x64 平台，Win32 用 lib/Win32
    )

    # 标记 VLD 是否找到
    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(VLD 
        DEFAULT_MSG 
        VLD_INCLUDE_DIR 
        VLD_LIBRARY
    )

    # 若找到 VLD，创建导入库目标（方便链接）
    if(VLD_FOUND)
        add_library(VLD::VLD UNKNOWN IMPORTED)
        set_target_properties(VLD::VLD PROPERTIES
            IMPORTED_LOCATION "${VLD_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${VLD_INCLUDE_DIR}"
        )
        message(STATUS "VLD found: ${VLD_LIBRARY}")
    else()
        message(WARNING "VLD not found (check VLD_PATHS), memory leak detection disabled")
    endif()
endif()


# 设置默认的启动项目(MSVC有效)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT CommTest_Qt)

# 添加子项目
add_subdirectory(CommTest_Qt)