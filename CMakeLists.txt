cmake_minimum_required(VERSION 3.10)

# Read version from VERSION file
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" PROJECT_VERSION)
string(STRIP "${PROJECT_VERSION}" PROJECT_VERSION)

project(json_to_schema
VERSION ${PROJECT_VERSION}
DESCRIPTION "Allocation-free JSON to JSON Schema converter for C"
LANGUAGES C
)


# project() initializes the prefix, so only replace CMake's default.
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    if(WIN32 AND DEFINED ENV{LOCALAPPDATA} AND NOT "$ENV{LOCALAPPDATA}" STREQUAL "")
        file(TO_CMAKE_PATH "$ENV{LOCALAPPDATA}/Programs/json-to-schema" _default_install_prefix)
    elseif(UNIX AND DEFINED ENV{HOME} AND NOT "$ENV{HOME}" STREQUAL "")
        set(_default_install_prefix "$ENV{HOME}/.local")
    endif()
    if(DEFINED _default_install_prefix)
        set(CMAKE_INSTALL_PREFIX "${_default_install_prefix}" CACHE PATH
            "Install prefix for json-to-schema" FORCE)
        unset(_default_install_prefix)
    endif()
endif()

include(GNUInstallDirs)

add_library(json_to_schema
src/jsmn.c
src/json_schema.c
)
add_library(json_to_schema::json_to_schema ALIAS json_to_schema)

target_include_directories(json_to_schema
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
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

install(TARGETS json_to_schema
    EXPORT json_to_schema_targets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(TARGETS json-to-schema
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(FILES
    include/jsmn.h
    include/json_schema.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
install(EXPORT json_to_schema_targets
    NAMESPACE json_to_schema::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/json_to_schema
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

set(CPACK_PACKAGE_NAME "json-to-schema")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${PROJECT_DESCRIPTION})
set(CPACK_PACKAGE_FILE_NAME
    "json-to-schema-${PROJECT_VERSION}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")
set(CPACK_GENERATOR "ZIP")
include(CPack)
