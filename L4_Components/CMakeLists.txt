file(GLOB_RECURSE L4_SRCS "src/*.c")

add_library(l4_components OBJECT ${L4_SRCS})

target_include_directories(l4_components PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

target_link_libraries(l4_components PUBLIC 
    l3_middlewares
    l2_core
    l1_mcu
    l0_assets
    global_macros 
    global_options
)
