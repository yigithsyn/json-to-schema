# Command-line interface

The `json-to-schema` executable converts one JSON value to a compact JSON Schema
document followed by a newline.

```text
Usage: json-to-schema [JSON | -]
```

## Options

| Argument | Behavior |
| --- | --- |
| `-h`, `--help` | Print usage and exit successfully. |
| `--version` | Print the CLI version and exit successfully. |
| `-` | Read JSON from standard input until EOF. |
| No argument | Read JSON from standard input until EOF. |
| One other argument | Interpret that entire argument as JSON. |

The CLI reads a JSON value, not a file path. Redirect a file or pipe its contents
to standard input.

## Windows PowerShell

For a Visual Studio Release build:

```powershell
.\build\Release\json-to-schema.exe --help
.\build\Release\json-to-schema.exe --version
Get-Content -Raw .\input.json | .\build\Release\json-to-schema.exe -
```

## Unix-like shells

For a Makefile build:

```sh
./build/json-to-schema '{"name":"Ada","age":37}'
echo '{"name":"Ada","age":37}' | ./build/json-to-schema
./build/json-to-schema - < input.json > schema.json
```

With CMake, adjust the executable path to match your build directory and
generator. Shell quoting rules differ; file input avoids quoting embedded JSON.

## Output and errors

Schemas go to standard output. Conversion and usage errors go to standard error.

| Exit status | Meaning |
| --- | --- |
| 0 | Successful conversion, help, or version request. |
| 1 | Input, allocation, conversion, or output-write failure. |
| 2 | Too many command-line arguments. |

Conversion errors include the numeric [C API status code](api.md#status-codes).
The CLI allocates input, token, and output buffers dynamically; unlike the
library API, it is not allocation-free. Invalid JSON, embedded NUL bytes in
standard input, oversized input, and excessive nesting are rejected.
