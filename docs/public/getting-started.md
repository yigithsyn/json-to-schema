# Build and test

## Requirements

- A C89/C90-capable compiler.
- CMake 3.10 or newer for the CMake build.
- Alternatively, GNU Make and a Unix-like shell for the existing Makefile.

Python and MkDocs are required only for [building the documentation](documentation.md),
not for compiling or using the C library.

## CMake

From the repository root:

```powershell
cmake -S . -B build
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

The default build creates the `json_to_schema` library, the `json-to-schema`
executable, and tests. On Windows with a Visual Studio generator, the CLI is
typically `build\Release\json-to-schema.exe`. Single-configuration generators
typically place it directly in the build directory.

For a single-configuration Release build, configure with
`-DCMAKE_BUILD_TYPE=Release`.

The tests cover schema generation, buffer errors, invalid JSON, and the CLI's
help and version options. Disable tests with:

```powershell
cmake -S . -B build -DBUILD_TESTING=OFF
```

## GNU Make

In a Unix-like shell with a C compiler:

```sh
make
make test
```

The CLI is written to `build/json-to-schema`. Use `make cli` to build only the
CLI and `make clean` to remove the build directory.

## Use the library

Include `json_schema.h`, provide the project's include directory to your
compiler, and compile/link both `src/json_schema.c` and `src/jsmn.c`.

For a complete caller-owned-buffer example, see the [C API](api.md). CMake
consumers can use [the provided library target](cmake-integration.md).
