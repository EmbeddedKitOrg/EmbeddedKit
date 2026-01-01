file(GLOB L4_SRCS "src/*.c")

add_library(l4_components STATIC ${L4_SRCS})

target_include_directories(l4_components PUBLIC inc)

target_link_libraries(l4_components PUBLIC 
    l3_middlewares  # 可能依赖中间件 (如 USB)
    l2_core         # 依赖 delay 或 log
    l1_hal          # 依赖 GPIO/SPI/I2C
    global_macros 
    global_options
)
