# GD32头文件
set(GD_Include_Dir
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/Driver/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/Driver/CMSIS/Device/GD/GD32F4xx/Include
    ${CMAKE_CURRENT_SOURCE_DIR}/Driver/CMSIS/Include
    ${CMAKE_CURRENT_SOURCE_DIR}/Bsp/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/Hal/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/Bsp
    ${CMAKE_CURRENT_SOURCE_DIR}/Hal
)

# 收集源文件
file(GLOB GD_Start_S ${CMAKE_CURRENT_SOURCE_DIR}/*.s)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/Core/Src GD_Core_Src)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/Driver/Src GD_Std_Src)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/Driver/CMSIS/Device/GD/GD32F4xx/Source GD_CMSIS_Src)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/Bsp/Src GD_Bsp_Src)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/Hal/Src GD_Hal_Src)


target_sources(l1_mcu PRIVATE
    ${GD_Start_S}
    ${GD_Core_Src}
    ${GD_Std_Src}
    ${GD_CMSIS_Src}
    ${GD_Bsp_Src}
    ${GD_Hal_Src}
)

target_include_directories(l1_mcu PUBLIC
    ${GD_Include_Dir}
)

target_compile_definitions(l1_mcu PUBLIC
    GD32F470
    $<$<CONFIG:Debug>:DEBUG>
)

# 链接脚本内存参数
set(EK_FLASH_ORIGIN  "0x08000000" PARENT_SCOPE)
set(EK_FLASH_LENGTH  "1024K"      PARENT_SCOPE)
set(EK_RAM_ORIGIN    "0x20000000" PARENT_SCOPE)
set(EK_RAM_LENGTH    "448K"       PARENT_SCOPE)
set(EK_CCMRAM_ORIGIN "0x10000000" PARENT_SCOPE)
set(EK_CCMRAM_LENGTH "64K"        PARENT_SCOPE)
set(EK_SDRAM1_ORIGIN "0xC0000000" PARENT_SCOPE)
set(EK_SDRAM1_LENGTH "32M"        PARENT_SCOPE)
