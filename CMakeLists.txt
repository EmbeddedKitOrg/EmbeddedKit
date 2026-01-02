cmake_minimum_required(VERSION 3.20)

# ==============================================================================
#  项目基本配置
# ==============================================================================
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# 设置工程名
set(CMAKE_PROJECT_NAME EK_DEMO)

project(${CMAKE_PROJECT_NAME})

# 允许使用的语言
enable_language(C CXX ASM)

# 默认生成 compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "Export compile commands" FORCE)

# Ninja 彩色输出支持
if("${CMAKE_GENERATOR}" MATCHES "Ninja")
    message(STATUS "Build Generator is Ninja, forcing colored diagnostics.")
    add_compile_options($<$<COMPILE_LANGUAGE:C,CXX>:-fdiagnostics-color=always>)
endif()

# ==============================================================================
# 全局宏定义 (Global Macros)
# ==============================================================================
# 使用 INTERFACE 库来管理全局宏，方便所有子层级继承
add_library(global_macros INTERFACE)
target_compile_definitions(global_macros INTERFACE
    $<$<CONFIG:Debug>:DEBUG>   # 仅在 Debug 模式下定义 DEBUG
    $<$<CONFIG:Release>:NDEBUG>
    # 在这里添加你的全局宏
)

# ==============================================================================
# 全局编译选项 (Global Compile Options)
# ==============================================================================
# 统一管理警告等级、优化等级、MCU架构参数
add_library(global_options INTERFACE)
target_compile_options(global_options INTERFACE
    # 通用警告
    -Wall
    -Wextra
    
    # C 标准与特性 (如果编译器支持)
    $<$<COMPILE_LANGUAGE:C>:-std=c11>
    $<$<COMPILE_LANGUAGE:CXX>:-std=c++17>
    
    # 可以在这里添加 MCU 架构参数，或者在 Toolchain 文件中定义
    # -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16
    
    # 调试与优化
    $<$<CONFIG:Debug>:-O0 -g3>
    $<$<CONFIG:Release>:-O3>
)

# 统一输出目录配置
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# ==============================================================================
# 生成最终固件 (Executable)
# ==============================================================================
# 虽然入口在 L0，业务在 L5，但最终的 ELF 需要链接所有层
# 注意：必须在 add_subdirectory 之前创建此目标，因为子目录中会引用它

add_executable(${CMAKE_PROJECT_NAME})

# 设置输出文件名
set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES OUTPUT_NAME ${CMAKE_PROJECT_NAME})

# ==============================================================================
# 添加子目录 (构建静态库)
# ==============================================================================
# 注意：在各层级的 CMakeLists.txt 中，记得链接 global_macros 和 global_options
add_subdirectory(L0_MCU)
# add_subdirectory(L1_HAL)
add_subdirectory(L2_Core)
# add_subdirectory(L3_Middlewares)
# add_subdirectory(L4_Components)
add_subdirectory(L5_App)

# 链接所有层级生成的库
# 注意顺序：通常 顶层 依赖 底层，但链接器有时需要注意符号解析顺序。
# 如果使用 .a 静态库，建议将依赖方放在前面 (L5 依赖 L4...)
target_link_libraries(${CMAKE_PROJECT_NAME}
    l5_app
    # l4_components
    # l3_middlewares
    l2_core
    # l1_hal
    l0_mcu

    global_macros   # 确保最终 ELF 也继承宏
    global_options  # 确保最终 ELF 也继承选项
)

# ==============================================================================
# Post-Build: 生成 Hex/Bin/Size
# ==============================================================================
add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${CMAKE_PROJECT_NAME}> ${CMAKE_PROJECT_NAME}.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${CMAKE_PROJECT_NAME}> ${CMAKE_PROJECT_NAME}.bin
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${CMAKE_PROJECT_NAME}>
    COMMENT "Generating BIN/HEX and printing size"
)
