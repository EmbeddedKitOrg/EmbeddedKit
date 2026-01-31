option(USE_FREERTOS "Enable FreeRTOS" OFF)
option(USE_FATFS "ENABLE FATFS" OFF)
option(USE_LVGL "ENABLE LVGL" OFF)

# 汇总库
add_library(l3_middlewares INTERFACE)

# 中间件集成
if(USE_FREERTOS)
    add_subdirectory(FreeRTOS)
endif()

if(USE_FATFS)
    add_subdirectory(FatFS)
endif()

if(USE_LVGL)
    if(USE_FREERTOS)
        target_compile_definitions(l3_middlewares INTERFACE LVGL_USE_FREERTOS=1)
    endif()
    add_subdirectory(LVGL)

    add_library(lvgl_config INTERFACE)
    target_include_directories(lvgl_config INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/LVGL)
    target_compile_definitions(lvgl_config INTERFACE
        LV_CONF_BUILD_DISABLE_EXAMPLES=1
        LV_CONF_BUILD_DISABLE_DEMOS=1
    )
endif()

# 链接依赖
if(USE_FREERTOS)
    target_link_libraries(l3_middlewares INTERFACE freertos_kernel)
endif()

if(USE_FATFS)
    target_link_libraries(l3_middlewares INTERFACE fatfs)
endif()

if(USE_LVGL)
    target_link_libraries(l3_middlewares INTERFACE lvgl lvgl_config)
endif()

target_link_libraries(l3_middlewares INTERFACE l2_core global_macros global_options)
