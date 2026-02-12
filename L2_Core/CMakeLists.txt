file(GLOB_RECURSE L2_3RD_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/third_party/*.c")
file(GLOB_RECURSE L2_PORT_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/port/*.c")

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/utils/src L2_UTILS_SRCS)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/hal/src L2_HAL_SRCS)

add_library(l2_core OBJECT ${L2_3RD_SRCS} ${L2_UTILS_SRCS} ${L2_PORT_SRCS} ${L2_HAL_SRCS})

target_include_directories(l2_core PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/utils/inc
   ${CMAKE_CURRENT_SOURCE_DIR}/hal/inc
   ${CMAKE_CURRENT_SOURCE_DIR}/port/inc
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
