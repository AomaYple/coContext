cmake_minimum_required(VERSION 3.30)

project(coContext VERSION 1.1.0)

add_library(${PROJECT_NAME} SHARED)

set_target_properties(${PROJECT_NAME}
        PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
        CXX_STANDARD ${CMAKE_CXX_STANDARD_LATEST}
        CXX_STANDARD_REQUIRED ON
        COMPILE_WARNING_AS_ERROR ON
        INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE
)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
        src/*.cpp
)

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    find_package(mimalloc REQUIRED)
else ()
    list(REMOVE_ITEM SOURCES
            ${CMAKE_CURRENT_SOURCE_DIR}/src/memory/MiMallocResource.cpp
    )
endif ()

target_sources(${PROJECT_NAME}
        PRIVATE
        ${SOURCES}
)

target_include_directories(${PROJECT_NAME}
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

option(NATIVE "Enable native optimization")
target_compile_options(${PROJECT_NAME}
        PRIVATE
        -Wall -Wextra -Wpedantic
        $<$<CONFIG:Debug>:-g3 -ggdb3 -Og -fsanitize=address -fsanitize=leak -fsanitize=undefined>
        $<$<CONFIG:Release>:-Ofast>
        $<$<AND:$<CONFIG:Release>,$<BOOL:${NATIVE}>>:-march=native>
)

target_link_options(${PROJECT_NAME}
        PRIVATE
        $<$<CONFIG:Debug>:-fsanitize=address -fsanitize=leak -fsanitize=undefined>
)

target_link_libraries(${PROJECT_NAME}
        PRIVATE
        uring
        $<$<CONFIG:Release>:mimalloc>
)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

configure_package_config_file(cmake/${PROJECT_NAME}Config.cmake.in ${PROJECT_NAME}Config.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROcJECT_NAME}
)

write_basic_package_version_file(${PROJECT_NAME}ConfigVersion.cmake
        COMPATIBILITY SameMinorVersion
)

install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake

        DESTINATION
        ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

install(DIRECTORY include/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(FILES LICENSE
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/licenses/${PROJECT_NAME}
)

install(TARGETS ${PROJECT_NAME}
        EXPORT ${PROJECT_NAME}Targets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(EXPORT ${PROJECT_NAME}Targets
        FILE ${PROJECT_NAME}Targets.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

add_subdirectory(example)
