cmake_minimum_required(VERSION 3.20)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(CMAKE_PROJECT_NAME EK_DEMO)
project(${CMAKE_PROJECT_NAME})

enable_language(C CXX ASM)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "Export compile commands" FORCE)

if("${CMAKE_GENERATOR}" MATCHES "Ninja")
    add_compile_options($<$<COMPILE_LANGUAGE:C,CXX>:-fdiagnostics-color=always>)
endif()

# 全局配置库
add_library(global_macros INTERFACE)
target_include_directories(global_macros INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(global_macros INTERFACE
    $<$<CONFIG:Debug>:DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
)

add_library(global_options INTERFACE)
target_compile_options(global_options INTERFACE
    -Wall -Wextra
    $<$<COMPILE_LANGUAGE:C>:-std=c11>
    $<$<COMPILE_LANGUAGE:CXX>:-std=c++17>
    $<$<CONFIG:Debug>:-O0 -g3>
    $<$<CONFIG:Release>:-O3>
)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# 可执行文件
set(EMPTY_OBJ ${CMAKE_CURRENT_SOURCE_DIR}/ek_empty_obj.c)
add_executable(${CMAKE_PROJECT_NAME} ${EMPTY_OBJ})

add_subdirectory(L0_Assets)
add_subdirectory(L1_MCU)
add_subdirectory(L2_Core)
add_subdirectory(L3_Middlewares)
add_subdirectory(L4_Components)
add_subdirectory(L5_App)

# 强制链接器解析 ek_main 符号（跨层调用 L1->L5）
target_link_options(${CMAKE_PROJECT_NAME} PRIVATE "-Wl,--undefined=ek_main")

target_link_libraries(${CMAKE_PROJECT_NAME}
    l5_app
    l4_components
    l3_middlewares
    l2_core
    "-Wl,--whole-archive" l1_mcu "-Wl,--no-whole-archive"  # 包含 syscalls
    l0_assets

    global_macros
    global_options
)

add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${CMAKE_PROJECT_NAME}> ${CMAKE_PROJECT_NAME}.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${CMAKE_PROJECT_NAME}> ${CMAKE_PROJECT_NAME}.bin
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${CMAKE_PROJECT_NAME}>
    COMMENT "Generating BIN/HEX and printing size"
)
