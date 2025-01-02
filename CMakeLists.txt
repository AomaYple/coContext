cmake_minimum_required(VERSION 3.30)

project(coContext)

add_library(${PROJECT_NAME} SHARED)

set_target_properties(${PROJECT_NAME}
        PROPERTIES
        CXX_STANDARD ${CMAKE_CXX_STANDARD_LATEST}
        CXX_STANDARD_REQUIRED ON
        COMPILE_WARNING_AS_ERROR ON
        INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE
)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
        src/*.cpp
)
if (NOT CMAKE_BUILD_TYPE STREQUAL "Release")
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

target_compile_options(${PROJECT_NAME}
        PRIVATE
        -Wall -Wextra -Wpedantic
        $<$<CONFIG:Debug>:-g3 -ggdb3 -Og -fsanitize=address -fsanitize=leak -fsanitize=undefined>
        $<$<CONFIG:Release>:-Ofast>
)

if (CMAKE_BUILD_TYPE STREQUAL "Release" AND DEFINED CO_CONTEXT_NATIVE AND CO_CONTEXT_NATIVE)
    target_compile_options(${PROJECT_NAME}
            PRIVATE
            -march=native
    )
endif ()

target_link_options(${PROJECT_NAME}
        PRIVATE
        $<$<CONFIG:Debug>:-fsanitize=address -fsanitize=leak -fsanitize=undefined>
)

find_package(mimalloc REQUIRED)
target_link_libraries(${PROJECT_NAME}
        PRIVATE
        uring
        $<$<CONFIG:Release>:mimalloc>
)

set(installTargets ${PROJECT_NAME}Targets)
install(TARGETS ${PROJECT_NAME}
        EXPORT ${installTargets}
)

install(EXPORT ${installTargets}
        FILE ${PROJECT_NAME}Config.cmake
        DESTINATION lib/cmake/${PROJECT_NAME}
)

install(DIRECTORY include/
        DESTINATION include
)
