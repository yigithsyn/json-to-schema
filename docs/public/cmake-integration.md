# CMake integration

The project exposes the `json_to_schema` library target and the
`json_to_schema::json_to_schema` alias. The target exports its include directory
to consumers.

## Embed a source checkout

Place a checkout under `external/json-to-schema` in your application, then add:

```cmake
cmake_minimum_required(VERSION 3.10)
project(example LANGUAGES C)

set(BUILD_TESTING OFF CACHE BOOL "Build tests")
add_subdirectory(external/json-to-schema)

add_executable(example main.c)
target_link_libraries(example PRIVATE json_to_schema::json_to_schema)
```

Your source can then include:

```c
#include "json_schema.h"
```

The library and CLI targets request standard C90 with compiler extensions
disabled. The consuming application can choose its own C language standard.

## Integration notes

- `BUILD_TESTING` controls the project's CTest executable and test registration.
  It is a shared CMake cache option, so coordinate its value with the parent
  project's testing policy.
- The project also defines the `json-to-schema` CLI target; there is currently
  no separate option to disable it.
- There are currently no installation rules or exported CMake package.
  `find_package(json_to_schema CONFIG REQUIRED)` is therefore not a supported
  integration method.
- Without CMake, compile both C source files and add the public include
  directory as described in [Build and test](getting-started.md).
