file(GLOB_RECURSE L0_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/*.c")

add_library(l0_assets STATIC ${L0_SRCS})

target_link_libraries(l0_assets PUBLIC
    global_macros
    global_options
)

target_include_directories(l0_assets PUBLIC inc)

