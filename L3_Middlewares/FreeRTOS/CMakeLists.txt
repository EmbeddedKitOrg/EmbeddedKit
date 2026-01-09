file(GLOB FRTOS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/*.c")

set(FRTOS_PORT
    ${CMAKE_CURRENT_SOURCE_DIR}/portable/port.c

    # 堆管理实现 - 只能选择一个
    # ${CMAKE_CURRENT_SOURCE_DIR}/Mem/heap_1.c
    # ${CMAKE_CURRENT_SOURCE_DIR}/Mem/heap_2.c
    # ${CMAKE_CURRENT_SOURCE_DIR}/Mem/heap_3.c
    ${CMAKE_CURRENT_SOURCE_DIR}/mem/heap_4.c
    # ${CMAKE_CURRENT_SOURCE_DIR}/Mem/heap_5.c
)

add_library(freertos_kernel STATIC ${FRTOS_SRC} ${FRTOS_PORT})

target_include_directories(freertos_kernel PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/conf
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/portable
)

target_link_libraries(freertos_kernel PUBLIC
    global_macros
    global_options
)
