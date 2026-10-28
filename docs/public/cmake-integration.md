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
- Installation includes the library, CLI, headers, and exported CMake targets.
  There is no package configuration file, so
  `find_package(json_to_schema CONFIG REQUIRED)` is not supported.
- Without CMake, compile both C source files and add the public include
  directory as described in [Build and test](getting-started.md).

## Installation

For Windows PowerShell, configure a user-writable prefix, build, and install:

```powershell
cmake -S . -B build -DCMAKE_INSTALL_PREFIX="$env:LOCALAPPDATA\Programs\json-to-schema"
cmake --build build --config Release
cmake --install build --config Release
```

Fresh Windows configurations use this prefix by default when `LOCALAPPDATA` is
available. Explicit prefixes are not overridden. Existing build directories keep
their cached prefix, so rerun the configure command above if installation attempts
to write to `C:\Program Files\json_to_schema`. No administrator privileges are
needed for the user-local destination.

On Linux and macOS, install without `sudo`:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$HOME/.local"
cmake --build build --config Release
cmake --install build --config Release
export PATH="$HOME/.local/bin:$PATH"
```

Fresh Linux and macOS configurations default to `$HOME/.local` when `HOME` is
available. Explicit prefixes remain unchanged; the configure command above also
replaces a cached `/usr/local` prefix in existing builds. Persist the `PATH`
setting in your shell startup file (such as `~/.bashrc` or `~/.zshrc`) to run the
installed CLI in future sessions.

If `LOCALAPPDATA` (Windows) or `HOME` (Linux and macOS) is missing or empty, CMake
keeps its standard prefix. In that case, specify a writable prefix explicitly.

For a one-time destination override, use
`cmake --install build --config Release --prefix "<writable-directory>"`.
The prefix contains `bin`, `lib`, and `include` directories (subject to
`GNUInstallDirs` overrides).
