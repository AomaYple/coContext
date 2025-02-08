file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
        *.cpp
)

foreach (SOURCE ${SOURCES})
    get_filename_component(FILE_NAME ${SOURCE} NAME_WE)

    set(EXECUTION "benchmark-${FILE_NAME}")

    add_executable(${EXECUTION} ${SOURCE})

    set_target_properties(${EXECUTION}
            PROPERTIES
            CXX_STANDARD ${CMAKE_CXX_STANDARD_LATEST}
            CXX_STANDARD_REQUIRED ON
            COMPILE_WARNING_AS_ERROR ON
            INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE
    )

    target_compile_options(${EXECUTION}
            PRIVATE
            -Wall -Wextra -Wpedantic
            $<$<CONFIG:Debug>:-g3 -ggdb3 -Og -fsanitize=address -fsanitize=leak -fsanitize=undefined>
            $<$<CONFIG:Release>:-Ofast -march=native>
    )

    target_link_options(${EXECUTION}
            PRIVATE
            $<$<CONFIG:Debug>:-fsanitize=address -fsanitize=leak -fsanitize=undefined>
    )

    if (${FILE_NAME} STREQUAL ${PROJECT_NAME})
        target_link_libraries(${EXECUTION}
                PRIVATE
                ${PROJECT_NAME}
        )
    endif ()
endforeach ()
