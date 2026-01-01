file(GLOB L2_SRCS "src/*.c")

add_library(l2_core STATIC ${L2_SRCS})

target_include_directories(l2_core PUBLIC inc)

target_link_libraries(l2_core PUBLIC 
    l1_hal          # 依赖 HAL
    global_macros 
    global_options
)
