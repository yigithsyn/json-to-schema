# json-to-schema

A small, allocation-free C library that infers a JSON Schema draft-07 schema
from one JSON value. It is designed for embedded applications and builds as
strict C89 (and therefore also works with C99).

The project includes a compact jsmn-compatible tokenizer. The caller supplies
fixed-size token and output buffers; the library does not use heap allocation.

## Build and test

With CMake:

```sh
cmake -S . -B build/cmake
cmake --build build/cmake
ctest --test-dir build/cmake --output-on-failure
```

Or with the existing Makefile:

```sh
make
make test
```

The CMake build provides the `json_to_schema` library target and the
`json_to_schema::json_to_schema` alias. Tests can be disabled with
`-DBUILD_TESTING=OFF`.

## CLI

The `json-to-schema` command accepts a JSON value as one quoted command-line
argument, or reads JSON from standard input when called without an argument.
Use `-` to explicitly select standard input:

```sh
./build/json-to-schema --help
./build/json-to-schema --version
./build/json-to-schema '{"name":"Ada","age":37}'
echo '{"name":"Ada","age":37}' | ./build/json-to-schema
./build/json-to-schema - < input.json
```

`-h` and `--help` print usage; `--version` prints the CLI version.

With the CMake build directory shown above, the executable is
`build/cmake/json-to-schema`.

The generated JSON Schema is written to standard output. Usage and conversion
errors are written to standard error and return a nonzero exit status.

## API

Include `json_schema.h` and link `src/json_schema.c` and `src/jsmn.c`. The
converter takes a NUL-terminated JSON string:

```c
#include "json_schema.h"

jsmntok_t tokens[128];
char schema[2048];
size_t schema_length;
jts_status status;

status = jts_json_to_schema(json_text, tokens, 128U, schema,
                            sizeof(schema), &schema_length, NULL);
if (status == JTS_OK) {
    /* schema contains a NUL-terminated JSON Schema document. */
}
```

`output_length` reports the required schema length excluding the terminating
NUL. `error_position`, when provided, reports the parser position on JSON or
token-capacity errors. `JTS_ERROR_OUTPUT` and `JTS_ERROR_TOKENS` indicate that
the respective caller-provided buffer is too small. Parser and converter
nesting are limited to 64 levels.

## Inference behavior

* JSON objects become `type: object`; observed properties are included in
  `properties` and listed as `required`.
* JSON arrays infer `items` from their elements. Multiple elements are
  represented with `anyOf` so heterogeneous arrays retain every observed
  value shape. Empty arrays use an unconstrained `{}` item schema.
* Strings, booleans, null, integers and non-integer numbers map to their
  corresponding JSON Schema types.
* String formats and semantic constraints are not inferred from a single
  example.

## Documentation

The MkDocs guide covers building, the C API, CLI usage, inference rules, and
CMake integration. Its Markdown sources are in [docs/public](docs/public/index.md).
The configured website is <https://yigithsyn.github.io/json-to-schema/>; it
requires an initial successful GitHub Pages deployment.

To build or preview the guide locally:

```sh
python -m pip install -r docs/requirements.txt
python -m mkdocs build --strict
python -m mkdocs serve
```

The generated site is written to `site/`. The Makefile also provides `make docs`
and `make docs-serve`. Documentation dependencies are separate from the C build.

See the [documentation guide](docs/public/documentation.md) for environment
setup, GitHub Pages activation, and Read the Docs hosting. The Documentation
workflow checks documentation pull requests and deploys documentation changes
on `main` after Pages is configured to use **GitHub Actions**.
