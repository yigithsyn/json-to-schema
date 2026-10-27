cmake_minimum_required(VERSION 3.10)

project(json_to_schema
    VERSION 0.1.0
    DESCRIPTION "Allocation-free JSON to JSON Schema converter for C"
    LANGUAGES C
)

add_library(json_to_schema
    src/jsmn.c
    src/json_schema.c
)
add_library(json_to_schema::json_to_schema ALIAS json_to_schema)

target_include_directories(json_to_schema
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
set_target_properties(json_to_schema PROPERTIES
    C_STANDARD 90
    C_STANDARD_REQUIRED YES
    C_EXTENSIONS NO
)

add_executable(json-to-schema src/json_to_schema_cli.c)
target_link_libraries(json-to-schema PRIVATE json_to_schema)
set_target_properties(json-to-schema PROPERTIES
    C_STANDARD 90
    C_STANDARD_REQUIRED YES
    C_EXTENSIONS NO
)

include(CTest)
if(BUILD_TESTING)
    add_executable(test_json_schema tests/test_json_schema.c)
    target_link_libraries(test_json_schema PRIVATE json_to_schema)
    set_target_properties(test_json_schema PROPERTIES
        C_STANDARD 90
        C_STANDARD_REQUIRED YES
        C_EXTENSIONS NO
    )
    add_test(NAME json_schema COMMAND test_json_schema)
    add_test(NAME cli_help_short COMMAND json-to-schema -h)
    add_test(NAME cli_help_long COMMAND json-to-schema --help)
    add_test(NAME cli_version COMMAND json-to-schema --version)
endif()
