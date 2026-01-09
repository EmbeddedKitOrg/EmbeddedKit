file(GLOB FATFS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/*.c")

add_library(fatfs STATIC ${FATFS_SRC})

target_include_directories(fatfs PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/conf
  ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

target_link_libraries(fatfs PUBLIC
    global_macros
    global_options
)
