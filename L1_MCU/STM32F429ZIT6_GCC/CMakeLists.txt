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

# 直接将源文件添加到 l1_mcu OBJECT 库中
# CMake 不支持 OBJECT 库的嵌套对象文件传递，所以不创建 cubemx_hal 子库
target_sources(l1_mcu PRIVATE
    ${MX_Start_S}
    ${MX_Core_Src}
    ${MX_Driver_Src}
    ${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/ST/ARM/DSP/Lib/libarm_cortexM4lf_math.a
)

# 将头文件路径和宏定义传递给 l1_mcu
target_include_directories(l1_mcu PUBLIC
    ${MX_Include_Dirs}
)

target_compile_definitions(l1_mcu PUBLIC
    USE_HAL_DRIVER
    STM32F429xx
    $<$<CONFIG:Debug>:DEBUG>
)

# 链接脚本内存参数
set(EK_FLASH_ORIGIN  "0x08000000" PARENT_SCOPE)
set(EK_FLASH_LENGTH  "2048K"      PARENT_SCOPE)
set(EK_RAM_ORIGIN    "0x20000000" PARENT_SCOPE)
set(EK_RAM_LENGTH    "192K"       PARENT_SCOPE)
set(EK_CCMRAM_ORIGIN "0x10000000" PARENT_SCOPE)
set(EK_CCMRAM_LENGTH "64K"        PARENT_SCOPE)
set(EK_SDRAM1_ORIGIN "0xD0000000" PARENT_SCOPE)
set(EK_SDRAM1_LENGTH "8M"         PARENT_SCOPE)
