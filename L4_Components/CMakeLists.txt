file(GLOB L4_SRCS "src/*.c")

add_library(l4_components STATIC ${L4_SRCS})

target_include_directories(l4_components PUBLIC inc)

target_link_libraries(l4_components PUBLIC 
    l3_middlewares
    l2_core
    l1_mcu
    l0_assets
    global_macros 
    global_options
)
