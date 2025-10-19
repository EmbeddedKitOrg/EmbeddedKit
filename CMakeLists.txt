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
set(CMAKE_PROJECT_NAME EmbeddedKit)

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

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here
    EK_Component/EK_Common.c
    EK_Component/Bsp/Serial/EK_Serial.c
    EK_Component/DataStruct/List/EK_List.c
    EK_Component/DataStruct/Queue/EK_Queue.c
    EK_Component/DataStruct/Stack/EK_Stack.c
    EK_Component/EK_Corotinue/Kernel/Heap.c
    EK_Component/EK_Corotinue/Kernel/Kernel.c
    EK_Component/EK_Corotinue/Kernel/List.c
    EK_Component/EK_Corotinue/Message/EK_CoroMessage.c
    EK_Component/EK_Corotinue/Semaphore/EK_CoroSemaphore.c
    EK_Component/EK_Corotinue/Task/EK_CoroTask.c
    EK_Component/MemPool/EK_MemPool.c
    EK_Component/Task/EK_Task.c
    APP/Src/IT_App.c
    APP/Src/Task_App.c
    BSP/Src/Serial.c
    BSP/Src/Key_LED.c
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
    EK_Component
    EK_Component/Bsp/Serial
    EK_Component/DataStruct/List
    EK_Component/DataStruct/Queue
    EK_Component/DataStruct/Stack
    EK_Component/EK_Corotinue
    EK_Component/EK_Corotinue/Kernel
    EK_Component/EK_Corotinue/Message
    EK_Component/EK_Corotinue/Semaphore
    EK_Component/EK_Corotinue/Task
    EK_Component/MemPool
    EK_Component/Task
    APP/Inc
    BSP/Inc
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
)

# 添加编译选项
target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE
    # 开启硬件FPU
    -mfloat-abi=hard
    -mfpu=fpv4-sp-d16
    # 关闭所有警告
    -w
)

# Remove wrong libob.a library dependency when using cpp files
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx

    # Add user defined libraries
    # 使能printf浮点输出、输入
    -u _printf_float
    -u _scanf_float
)
