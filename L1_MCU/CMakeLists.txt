add_library(l1_mcu STATIC)

target_link_libraries(l1_mcu PUBLIC l0_assets global_macros global_options)
target_include_directories(l1_mcu PUBLIC inc)

# 选择 MCU 型号
set(MCU_MODEL "STM32F429ZIT6_GCC" CACHE STRING "Select MCU Model")
message(STATUS "L1 Configuration: Integrating ${MCU_MODEL} into l1_mcu")

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${MCU_MODEL}")
    message(FATAL_ERROR "MCU directory not found: ${CMAKE_CURRENT_SOURCE_DIR}/${MCU_MODEL}")
endif()

add_subdirectory(${MCU_MODEL})
