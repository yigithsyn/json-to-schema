# C API

Include `json_schema.h`. The header also includes the tokenizer types and has
an `extern "C"` guard for C++ consumers.

## Conversion

```c
jts_status jts_json_to_schema(const char *json,
                              jsmntok_t *tokens,
                              unsigned int token_capacity,
                              char *output,
                              size_t output_capacity,
                              size_t *output_length,
                              size_t *error_position);
```

| Parameter | Meaning |
| --- | --- |
| `json` | NUL-terminated input containing one JSON value. |
| `tokens` | Caller-owned tokenizer workspace. Must not be NULL. |
| `token_capacity` | Number of token entries, not bytes; must be greater than zero. |
| `output` | Caller-owned output buffer. Must not be NULL. |
| `output_capacity` | Output capacity in bytes, including room for the terminating NUL; must be greater than zero. |
| `output_length` | Optional pointer receiving the schema length excluding the terminating NUL. |
| `error_position` | Optional pointer receiving the parser's byte position on JSON, token-capacity, or parser-depth errors. |

The function does not allocate memory. Keep input, token workspace, and output
in separate buffers. Only use the output as a complete schema when the return
value is `JTS_OK`.

## Example

```c
#include <stdio.h>
#include "json_schema.h"

int main(void)
{
    const char *json;
    jsmntok_t tokens[128];
    char schema[2048];
    size_t schema_length;
    size_t error_position;
    jts_status status;

    json = "{\"name\":\"Ada\",\"age\":37}";
    status = jts_json_to_schema(json, tokens, 128U, schema,
                                sizeof(schema), &schema_length,
                                &error_position);
    if (status != JTS_OK) {
        fprintf(stderr, "Conversion failed: %d; parser position: %lu\n",
                (int)status, (unsigned long)error_position);
        return 1;
    }
    puts(schema);
    return 0;
}
```

## Status codes

| Status | Value | Meaning / action |
| --- | --- | --- |
| `JTS_OK` | 0 | Output is a complete, NUL-terminated schema. |
| `JTS_ERROR_ARGUMENT` | -1 | A required pointer is NULL or a capacity is zero. Correct the arguments. |
| `JTS_ERROR_JSON` | -2 | Input is invalid or incomplete JSON. Inspect the input and parser position. |
| `JTS_ERROR_TOKENS` | -3 | Token workspace is too small. Provide more token entries and retry. |
| `JTS_ERROR_OUTPUT` | -4 | Output buffer is too small. Provide at least `output_length + 1` bytes and retry. |
| `JTS_ERROR_DEPTH` | -5 | Parser or converter nesting limit exceeded. Reduce nesting. |

Parser and converter nesting are limited to 64 levels. The parser position is
not a general diagnostic for all statuses; for example, it does not identify an
output-buffer sizing problem.

## Buffer sizing

On `JTS_OK` and `JTS_ERROR_OUTPUT`, `output_length` gives the full required
schema length excluding the NUL. Other errors do not guarantee a complete size.
The function initializes optional length and position outputs to zero on entry.

To probe the output size, supply sufficient tokens and a one-byte output buffer.
On `JTS_ERROR_OUTPUT`, check that adding one to the returned length will not
overflow `size_t`, then retry with a larger buffer. A NULL output or zero
capacity is not a supported size probe.

After an output-capacity error, the buffer contains a terminated but incomplete
prefix, not a usable schema. Parser errors clear the output buffer. Do not rely
on the output contents after an argument or depth error.

Token demand depends on the input structure: object keys, values, and containers
all consume tokens. Increase the workspace on `JTS_ERROR_TOKENS`; increasing only
the output buffer cannot fix a token-capacity error.
