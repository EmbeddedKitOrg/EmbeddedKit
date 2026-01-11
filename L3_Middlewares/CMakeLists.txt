option(USE_FREERTOS "Enable FreeRTOS" OFF)
option(USE_FATFS "ENABLE FATFS" OFF)
option(USE_LVGL "ENABLE LVGL" OFF)
option(USE_LVGL_THORVG "Enable LVGL ThorVG vector graphics library" OFF)

# FreeRTOS 集成
if(USE_FREERTOS)
    add_subdirectory(FreeRTOS)
endif()

# FatFS 集成
if(USE_FATFS)
    add_subdirectory(FatFS)
endif()

# LVGL 集成
if(USE_LVGL)
    if(USE_FREERTOS)
        target_compile_definitions(l3_middlewares INTERFACE LVGL_USE_FREERTOS=1)
    endif()

    if(NOT USE_LVGL_THORVG)
        set(LV_CONF_BUILD_DISABLE_THORVG_INTERNAL 1 CACHE INTERNAL "Disable ThorVG")
    endif()

    add_subdirectory(LVGL)

    add_library(lvgl_config INTERFACE)
    target_include_directories(lvgl_config INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/LVGL)

    target_compile_definitions(lvgl_config INTERFACE
        LV_CONF_BUILD_DISABLE_EXAMPLES=1
        LV_CONF_BUILD_DISABLE_DEMOS=1
    )

    if(NOT USE_LVGL_THORVG)
        target_compile_definitions(lvgl_config INTERFACE
            LV_CONF_BUILD_DISABLE_THORVG_INTERNAL=1
        )
    endif()
endif()

# 创建 l3_middlewares 汇总库
add_library(l3_middlewares INTERFACE)

if(USE_FREERTOS)
    target_link_libraries(l3_middlewares INTERFACE freertos_kernel)
endif()

if(USE_FATFS)
    target_link_libraries(l3_middlewares INTERFACE fatfs)
endif()

if(USE_LVGL)
    target_link_libraries(l3_middlewares INTERFACE lvgl lvgl_config)

    if(USE_LVGL_THORVG)
        target_link_libraries(l3_middlewares INTERFACE lvgl::thorvg)
    endif()
endif()

# 依赖关系：L3 层可以依赖 L2 层（包括 utils 和 hal）
target_link_libraries(l3_middlewares INTERFACE
    l2_core
    global_macros
    global_options
)
