cmake_minimum_required(VERSION 3.20)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(CMAKE_PROJECT_NAME demo0)

project(${CMAKE_PROJECT_NAME})

enable_language(C CXX ASM)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "Export compile commands" FORCE)

if("${CMAKE_GENERATOR}" MATCHES "Ninja")
    message(STATUS "Build Generator is Ninja, forcing colored diagnostics.")
    add_compile_options($<$<COMPILE_LANGUAGE:C,CXX>:-fdiagnostics-color=always>)
endif()

# 全局宏定义 (Global Macros)
add_library(global_macros INTERFACE)

target_include_directories(global_macros INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(global_macros INTERFACE
    $<$<CONFIG:Debug>:DEBUG>  
    $<$<CONFIG:Release>:NDEBUG>
    # 在这里添加你的全局宏
)

# 全局编译选项
add_library(global_options INTERFACE)
target_compile_options(global_options INTERFACE
    -Wall
    -Wextra
    
    $<$<COMPILE_LANGUAGE:C>:-std=c11>
    $<$<COMPILE_LANGUAGE:CXX>:-std=c++17>
    
    # 可以在这里添加 MCU 架构参数，或者在 Toolchain 文件中定义
    
    $<$<CONFIG:Debug>:-O0 -g3>
    $<$<CONFIG:Release>:-O3>
)

# 统一输出目录配置
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# 生成最终固件
set(EMPTY_OBJ ${CMAKE_CURRENT_SOURCE_DIR}/ek_empty_obj.c)

add_executable(${CMAKE_PROJECT_NAME} ${EMPTY_OBJ})

# 设置输出文件名
set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES OUTPUT_NAME ${CMAKE_PROJECT_NAME})

# 添加子目录 
add_subdirectory(L0_Assets)
add_subdirectory(L1_MCU)
add_subdirectory(L2_Core)  
add_subdirectory(L3_Middlewares)  
add_subdirectory(L4_Components)
add_subdirectory(L5_App)

target_link_libraries(${CMAKE_PROJECT_NAME}
    $<TARGET_OBJECTS:l5_app>
    $<TARGET_OBJECTS:l4_components>
    l3_middlewares
    $<TARGET_OBJECTS:l2_core>
    $<TARGET_OBJECTS:l1_mcu>
    $<TARGET_OBJECTS:l0_assets>

    global_macros
    global_options
)

# 生成 Hex/Bin/Size
add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${CMAKE_PROJECT_NAME}> ${CMAKE_PROJECT_NAME}.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${CMAKE_PROJECT_NAME}> ${CMAKE_PROJECT_NAME}.bin
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${CMAKE_PROJECT_NAME}>
    COMMENT "Generating BIN/HEX and printing size"
)
