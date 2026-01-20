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

file(GLOB_RECURSE L2_TestSrc "${CMAKE_CURRENT_SOURCE_DIR}/../L2_Core/*.c")

add_executable(${CMAKE_PROJECT_NAME} ${TestSrc} ${L2_TestSrc})

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

