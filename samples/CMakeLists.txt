macro(add_sample example)
    add_executable(${example} ${example}.cpp)
    target_include_directories(${example}
        PUBLIC ${CMAKE_SOURCE_DIR}/include
    )
    set_target_properties(${example}
        PROPERTIES
            CXX_STANDARD 17
            CXX_STANDARD_REQUIRED ON
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/samples
    )
endmacro()

add_sample(00_basic_usage)
add_sample(01_allow_abbrev)
add_sample(02_help_argument)
add_sample(03_exit_on_error)
add_sample(04_argument_name)
add_sample(05_actions)
add_sample(06_custom_action)
add_sample(07_nargs)
add_sample(08_choices)
add_sample(09_dest)
add_sample(10_subcommand_1)
add_sample(11_subcommand_2)