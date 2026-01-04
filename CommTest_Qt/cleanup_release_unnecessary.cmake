# 清理Release发布包中不需要的文件，减小体积
# 仅在Release配置下运行

if("${BUILD_CONFIG}" STREQUAL "Release")
    message(STATUS "Cleaning unnecessary files from Release package...")

    # 删除触摸设备插件（如果不需要触摸屏支持）
    file(REMOVE_RECURSE "${BIN_DIR}/x64/generic")

    # 删除TLS/SSL插件（如果不使用HTTPS或加密网络）
    file(REMOVE_RECURSE "${BIN_DIR}/x64/tls")

    # 删除网络信息插件（如果不需要网络状态检测）
    file(REMOVE_RECURSE "${BIN_DIR}/x64/networkinformation")

    # 删除多语言翻译文件（如果只需要中文或不需要翻译）
    # 注意：如果需要保留中文翻译，请注释掉下面这行
    file(REMOVE_RECURSE "${BIN_DIR}/x64/translations")

    # 删除不常用的图片格式插件
    file(REMOVE
        "${BIN_DIR}/x64/imageformats/qicns.dll"
        "${BIN_DIR}/x64/imageformats/qtga.dll"
        "${BIN_DIR}/x64/imageformats/qwbmp.dll"
        "${BIN_DIR}/x64/imageformats/qico.dll"
        "${BIN_DIR}/x64/imageformats/qpdf.dll"
    )

    # 删除软件OpenGL渲染器（如果目标机器有显卡驱动）
    # 注意：删除后在没有显卡驱动的机器上可能无法运行
    file(REMOVE "${BIN_DIR}/x64/opengl32sw.dll")

    # 删除DirectX着色器编译器（如果没有复杂3D渲染）
    file(REMOVE
        "${BIN_DIR}/x64/dxcompiler.dll"
        "${BIN_DIR}/x64/dxil.dll"
    )

    # 删除Qt6 PDF模块（如果不需要PDF阅读/渲染）
    file(REMOVE "${BIN_DIR}/x64/Qt6Pdf.dll")

    # 删除D3D编译器（如果使用QPainter而非OpenGL渲染）
    # 注意：如果删除后程序无法启动，请注释掉下面这行
    file(REMOVE "${BIN_DIR}/x64/D3Dcompiler_47.dll")

    message(STATUS "Release package cleanup completed.")
    #message(STATUS "Estimated space saved: ~50MB")
else()
    message(STATUS "Skipping release cleanup (Debug mode)")
endif()
