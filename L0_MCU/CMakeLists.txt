# ==========================================================
# 定义 L0 库
# ==========================================================
add_library(l0_mcu STATIC)

target_link_libraries(l0_mcu PUBLIC
    global_macros
    global_options
)

target_include_directories(l0_mcu PUBLIC inc)

# ==============================================================================
# 加载具体 MCU 子模块 (关键步骤)
# ==============================================================================
# 设置默认 MCU 型号，允许命令行 -DMCU_MODEL=xxx 覆盖
set(MCU_MODEL "STM32F429VGT6" CACHE STRING "Select MCU Model")

message(STATUS "L0 Configuration: Integrating ${MCU_MODEL} into l0_mcu")

# 检查目录是否存在
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${MCU_MODEL}")
    message(FATAL_ERROR "MCU directory not found: ${CMAKE_CURRENT_SOURCE_DIR}/${MCU_MODEL}")
endif()

# 添加 stub 源文件（提供 ek_main 的弱定义）
target_sources(l0_mcu PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/stub/ek_app_stub.c
)

# 确保 ek_def.h 可以被找到
target_include_directories(l0_mcu PUBLIC
    ${CMAKE_SOURCE_DIR}/L5_App/inc
)

# 进入子目录。注意：此时 l0_mcu 目标已经存在，子目录可以向其添加文件
add_subdirectory(${MCU_MODEL})
