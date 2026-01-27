# STM32 头文件路径
set(MX_Include_Dirs
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32F4xx_HAL_Driver/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32F4xx_HAL_Driver/Inc/Legacy
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/CMSIS/Device/ST/STM32F4xx/Include
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/CMSIS/Include
    ${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/ST/ARM/DSP/Inc
)

# 收集源文件
file(GLOB MX_Start_S ${CMAKE_CURRENT_SOURCE_DIR}/*.s)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/Core/Src MX_Core_Src)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32F4xx_HAL_Driver/Src MX_Driver_Src)

# 创建 cubemx_hal OBJECT 库（不生成独立静态库，对象将被添加到 l1_mcu）
add_library(cubemx_hal OBJECT ${MX_Start_S} ${MX_Core_Src} ${MX_Driver_Src})

# ARM DSP 数学库 - 作为源文件添加
target_sources(cubemx_hal PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/ST/ARM/DSP/Lib/libarm_cortexM4lf_math.a
)

# 头文件路径 - PUBLIC（传递给使用者）
target_include_directories(cubemx_hal PUBLIC
    ${MX_Include_Dirs}
)

# STM32 宏定义 - PUBLIC（传递给使用者）
target_compile_definitions(cubemx_hal PUBLIC
    USE_HAL_DRIVER
    STM32F407xx
    $<$<CONFIG:Debug>:DEBUG>
)

# 链接全局配置
target_link_libraries(cubemx_hal PUBLIC
    global_macros
    global_options
)

# 将 cubemx_hal 的对象文件添加到 l1_mcu（整合到 l1_mcu 静态库中）
# 这样上层只需要链接 l1_mcu，不需要知道 cubemx_hal 的存在
target_sources(l1_mcu PRIVATE
    $<TARGET_OBJECTS:cubemx_hal>
)

# 将 cubemx_hal 的头文件和宏定义传递给 l1_mcu 的使用者
target_include_directories(l1_mcu PUBLIC
    $<TARGET_PROPERTY:cubemx_hal,INTERFACE_INCLUDE_DIRECTORIES>
)

target_compile_definitions(l1_mcu PUBLIC
    $<TARGET_PROPERTY:cubemx_hal,INTERFACE_COMPILE_DEFINITIONS>
)
