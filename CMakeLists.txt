cmake_minimum_required(VERSION 3.16)

project(EmbeddedKit VERSION 0.1.0 LANGUAGES C)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(EK_COMPONENT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/EK_Component")

set(EK_COMPONENT_SOURCES
    ${EK_COMPONENT_DIR}/EK_Common.c
    ${EK_COMPONENT_DIR}/Bsp/EK_Serial.c
    ${EK_COMPONENT_DIR}/DataStruct/EK_List.c
    ${EK_COMPONENT_DIR}/DataStruct/EK_Queue.c
    ${EK_COMPONENT_DIR}/DataStruct/EK_Stack.c
    ${EK_COMPONENT_DIR}/EK_Corotinue/EK_CoroMessage.c
    ${EK_COMPONENT_DIR}/EK_Corotinue/EK_CoroSemaphore.c
    ${EK_COMPONENT_DIR}/EK_Corotinue/EK_CoroTask.c
    ${EK_COMPONENT_DIR}/EK_Corotinue/Heap.c
    ${EK_COMPONENT_DIR}/EK_Corotinue/Kernel.c
    ${EK_COMPONENT_DIR}/EK_Corotinue/List.c
    ${EK_COMPONENT_DIR}/MemPool/EK_MemPool.c
    ${EK_COMPONENT_DIR}/Task/EK_Task.c
)

add_library(EmbeddedKit STATIC ${EK_COMPONENT_SOURCES})
add_library(EmbeddedKit::EmbeddedKit ALIAS EmbeddedKit)

target_include_directories(EmbeddedKit
    PUBLIC
        $<BUILD_INTERFACE:${EK_COMPONENT_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/EmbeddedKit>
)

target_compile_features(EmbeddedKit PUBLIC c_std_99)

set_target_properties(EmbeddedKit PROPERTIES
    OUTPUT_NAME embeddedkit
    EXPORT_NAME EmbeddedKit
)

install(TARGETS EmbeddedKit
    EXPORT EmbeddedKitTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/EmbeddedKit
)

install(DIRECTORY ${EK_COMPONENT_DIR}/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/EmbeddedKit
    FILES_MATCHING PATTERN "*.h"
)

install(EXPORT EmbeddedKitTargets
    NAMESPACE EmbeddedKit::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/EmbeddedKit
)

configure_package_config_file(
    cmake/EmbeddedKitConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/EmbeddedKitConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/EmbeddedKit
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/EmbeddedKitConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/EmbeddedKitConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/EmbeddedKitConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/EmbeddedKit
)

include(CPack)
