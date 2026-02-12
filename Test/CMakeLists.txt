cmake_minimum_required(VERSION 3.20)

set(CMAKE_PROJECT_NAME "test")

project(${CMAKE_PROJECT_NAME})

enable_language(C ASM)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if("${CMAKE_GENERATOR}" MATCHES "Ninja")
    message(STATUS "Build Generator is Ninja, forcing colored diagnostics.")
    add_compile_options($<$<COMPILE_LANGUAGE:C,CXX>:-fdiagnostics-color=always>)
endif()

set(CMAKE_BINARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/build)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} TestSrc)

file(GLOB_RECURSE L2_TestSrc "${CMAKE_CURRENT_SOURCE_DIR}/../L2_Core/utils/src/*.c")
file(GLOB_RECURSE L2_TestSrc_3rd "${CMAKE_CURRENT_SOURCE_DIR}/../L2_Core/third_party/*.c")


# 添加shell_symbols.c为非嵌入式平台提供链接器符号和内存管理函数
list(APPEND TestSrc "${CMAKE_CURRENT_SOURCE_DIR}/shell_symbols.c")

add_executable(${CMAKE_PROJECT_NAME} ${TestSrc} ${L2_TestSrc} ${L2_TestSrc_3rd}) 

include_directories( ${CMAKE_PROJECT_NAME} PRIVATE
    ${CMAKE_PROJECT_NAME}/../L2_Core/utils/inc
    ${CMAKE_PROJECT_NAME}/../L2_Core/third_party/tlsf
    ${CMAKE_PROJECT_NAME}/../L2_Core/third_party/lwprintf/inc
    ${CMAKE_PROJECT_NAME}/../L2_Core/third_party/letter_shell/inc
    ${CMAKE_PROJECT_NAME}
)

target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE 
    -Wall
    -Wextra

    $<$<COMPILE_LANGUAGE:C>:-std=c11>
    $<$<COMPILE_LANGUAGE:CXX>:-std=c++17>
  
    $<$<CONFIG:Debug>:-O0 -g3>
    $<$<CONFIG:Release>:-O3>
)

