file(GLOB L5_SRCS "src/*.c")

add_library(l5_app OBJECT ${L5_SRCS})

target_include_directories(l5_app PUBLIC inc)

# App 层通常需要访问所有下层的 API
target_link_libraries(l5_app PUBLIC
    l4_components
    l3_middlewares
    l2_core
    l1_mcu
    l0_assets
    global_macros
    global_options
)
