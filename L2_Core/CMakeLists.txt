file(GLOB_RECURSE L2_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/*.c")

add_library(l2_core OBJECT ${L2_SRCS})

target_include_directories(l2_core PUBLIC
    utils/inc
    hal/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/letter_shell/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/lwprintf/include
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/tlsf
)

target_link_libraries(l2_core PUBLIC
    l1_mcu
    l0_assets
    global_macros
    global_options
)
