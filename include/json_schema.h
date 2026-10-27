#ifndef JSON_SCHEMA_H
#define JSON_SCHEMA_H

#include <stddef.h>
#include "jsmn.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    JTS_OK = 0,
    JTS_ERROR_ARGUMENT = -1,
    JTS_ERROR_JSON = -2,
    JTS_ERROR_TOKENS = -3,
    JTS_ERROR_OUTPUT = -4,
    JTS_ERROR_DEPTH = -5
} jts_status;

/*
 * Infers a JSON Schema draft-07 document from one JSON value.
 * The caller supplies both work buffers; this function allocates no memory.
 * output_length receives the required output size excluding the NUL byte.
 */
jts_status jts_json_to_schema(const char *json,
                              jsmntok_t *tokens,
                              unsigned int token_capacity,
                              char *output,
                              size_t output_capacity,
                              size_t *output_length,
                              size_t *error_position);

#ifdef __cplusplus
}
#endif

#endif
