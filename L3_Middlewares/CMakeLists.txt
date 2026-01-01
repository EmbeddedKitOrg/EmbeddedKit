# 中间件源文件可能分布在子文件夹，使用 GLOB_RECURSE
file(GLOB_RECURSE L3_SRCS "src/*.c")

add_library(l3_middlewares STATIC ${L3_SRCS})

# 中间件的头文件路径可能比较多
target_include_directories(l3_middlewares PUBLIC 
    inc
    # inc/FreeRTOS
    # inc/LwIP
)

target_link_libraries(l3_middlewares PUBLIC 
    l2_core         # 可能依赖 Core 提供的定时器或内存管理
    # l1_hal        # 有些中间件直接操纵 HAL
    global_macros 
    global_options
)
