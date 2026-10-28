# json-to-schema

json-to-schema is a small C library that infers a JSON Schema draft-07 document
from one JSON value. It targets embedded applications and builds as strict
C89/C90, so it can also be used from C99 projects.

The library includes a compact jsmn-compatible tokenizer. The caller supplies
token and output buffers; the library performs no heap allocation. The optional
use of the supplied command-line tool is different: it allocates buffers
dynamically to accommodate its input.

## Example

Input:

```json
{"name":"Ada","age":37}
```

Generated schema, formatted here for readability:

```json
{
  "$schema": "http://json-schema.org/draft-07/schema#",
  "type": "object",
  "properties": {
    "name": {"type": "string"},
    "age": {"type": "integer"}
  },
  "required": ["name", "age"]
}
```

The output describes the observed value, not every possible value in your
application. Review the [inference rules and limitations](inference.md) before
using a generated schema for validation.

## Guides

- [Build and test](getting-started.md) the library and command-line tool.
- Call the [C API](api.md) with caller-owned buffers.
- Convert files or pipeline input with the [CLI](cli.md).
- Embed the library in a [CMake project](cmake-integration.md).
- [Build, preview, and publish this documentation](documentation.md).
