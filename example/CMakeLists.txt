file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
        *.cpp
)

foreach (SOURCE ${SOURCES})
    get_filename_component(FILE_NAME ${SOURCE} NAME_WE)

    add_executable(${FILE_NAME} ${SOURCE})

    set_target_properties(${FILE_NAME}
            PROPERTIES
            CXX_STANDARD ${CMAKE_CXX_STANDARD_LATEST}
            CXX_STANDARD_REQUIRED ON
            COMPILE_WARNING_AS_ERROR ON
            INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE
    )

    target_compile_options(${FILE_NAME}
            PRIVATE
            -Wall -Wextra -Wpedantic
            $<$<CONFIG:Debug>:-g3 -ggdb3 -Og -fsanitize=address -fsanitize=leak -fsanitize=undefined>
            $<$<CONFIG:Release>:-Ofast -march=native>
    )

    target_link_options(${FILE_NAME}
            PRIVATE
            $<$<CONFIG:Debug>:-fsanitize=address -fsanitize=leak -fsanitize=undefined>
    )

    target_link_libraries(${FILE_NAME}
            PRIVATE
            ${PROJECT_NAME}
    )
endforeach ()
