cmake_minimum_required(VERSION 3.30.5)

project(coRing)

add_library(${PROJECT_NAME} SHARED)

set(BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME})

set_target_properties(${PROJECT_NAME} PROPERTIES
        CXX_STANDARD 26
        CXX_STANDARD_REQUIRED ON
        COMPILE_WARNING_AS_ERROR ON
        LIBRARY_OUTPUT_DIRECTORY ${BINARY_DIR}/lib
        INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE
)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
        include/*.hpp
        src/*.cpp
)

target_sources(${PROJECT_NAME} PRIVATE
        ${SOURCES}
)

target_compile_options(${PROJECT_NAME} PRIVATE
        -Wall -Wextra -Wpedantic
        $<$<CONFIG:Debug>:-Og -fsanitize=address>
        $<$<CONFIG:Release>:-Ofast>
)

target_link_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:-fsanitize=address>
)

target_link_libraries(${PROJECT_NAME} PRIVATE
        uring
)

add_custom_command(TARGET ${PROJECT_NAME}
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${BINARY_DIR}/include

        VERBATIM
)
