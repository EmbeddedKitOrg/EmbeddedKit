cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME STM32F407VGT6_JLC_TKX)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# 源文件
file(GLOB_RECURSE APP_SOURCES "App/Src/*.c")
file(GLOB_RECURSE BSP_SOURCES "Bsp/Src/*.c")
file(GLOB_RECURSE EK_COMPONENT_SOURCES "EK_Component/**/*.c")

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here
    ${APP_SOURCES}
    ${BSP_SOURCES}
    ${EK_COMPONENT_SOURCES}
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
    App/Inc
    Bsp/Inc
    EK_Component
    EK_Component/DataStruct/List
    EK_Component/DataStruct/Queue
    EK_Component/DataStruct/Stack
    EK_Component/MemPool
    EK_Component/Task

)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
    ARM_MATH_CM4
)

# 开启硬件浮点
target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE
    -mfloat-abi=hard
    -mfpu=fpv4-sp-d16
    -g  # 添加调试信息
)

# Remove wrong libob.a library dependency when using cpp files
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx

    # Add user defined libraries
)

# 开启打印浮点
target_link_options(${CMAKE_PROJECT_NAME} PRIVATE
    -u _printf_float
)
