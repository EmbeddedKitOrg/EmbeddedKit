add_library(l1_mcu OBJECT)

target_link_libraries(l1_mcu PUBLIC
    l0_assets
    global_macros
    global_options
)

target_include_directories(l1_mcu PUBLIC inc)

# 设置默认 MCU 型号，允许命令行 -DMCU_MODEL=xxx 覆盖
set(MCU_MODEL "STM32F429ZIT6_GCC" CACHE STRING "Select MCU Model")

message(STATUS "L1 Configuration: Integrating ${MCU_MODEL} into l1_mcu")

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${MCU_MODEL}")
    message(FATAL_ERROR "MCU directory not found: ${CMAKE_CURRENT_SOURCE_DIR}/${MCU_MODEL}")
endif()

add_subdirectory(${MCU_MODEL})

# MCU 子目录通过 PARENT_SCOPE 设置了内存参数，生成链接脚本
include(EkLinkerScript)
ek_configure_linker_script()

# 用生成的链接脚本覆盖工具链文件设置的默认值
set(LINKER_SCRIPT "${EK_GENERATED_LINKER_SCRIPT}"
    CACHE FILEPATH "The path to the linker script" FORCE)
