#include "json_schema.h"

#include <stdio.h>
#include <string.h>

#define TOKEN_CAPACITY 128
#define OUTPUT_CAPACITY 4096

static int check_schema(const char *json, const char *expected)
{
    jsmntok_t tokens[TOKEN_CAPACITY];
    char output[OUTPUT_CAPACITY];
    size_t output_length;
    jts_status status;

    status = jts_json_to_schema(json, tokens, TOKEN_CAPACITY, output,
                                sizeof(output), &output_length, NULL);
    if (status != JTS_OK) {
        fprintf(stderr, "conversion failed: %d\n", (int)status);
        return 0;
    }
    if (strcmp(output, expected) != 0 || output_length != strlen(expected)) {
        fprintf(stderr, "unexpected schema:\n%s\n", output);
        return 0;
    }
    return 1;
}

int main(void)
{
    const char *object_schema;
    const char *array_schema;
    const char *primitive_schema;
    const char *empty_schema;
    const char *escaped_schema;
    jsmntok_t tokens[4];
    char output[8];
    size_t output_length;
    int failures;

    object_schema =
        "{\"$schema\":\"http://json-schema.org/draft-07/schema#\","
        "\"type\":\"object\",\"properties\":{\"name\":{\"type\":\"string\"},"
        "\"age\":{\"type\":\"integer\"},\"items\":{\"type\":\"array\","
        "\"items\":{\"anyOf\":[{\"type\":\"integer\"},{\"type\":\"null\"}]}}},"
        "\"required\":[\"name\",\"age\",\"items\"]}";
    array_schema =
        "{\"$schema\":\"http://json-schema.org/draft-07/schema#\","
        "\"type\":\"array\",\"items\":{\"anyOf\":[{\"type\":\"string\"},"
        "{\"type\":\"string\"}]}}";
    primitive_schema =
        "{\"$schema\":\"http://json-schema.org/draft-07/schema#\","
        "\"type\":\"boolean\"}";
    empty_schema =
        "{\"$schema\":\"http://json-schema.org/draft-07/schema#\","
        "\"type\":\"object\",\"properties\":{}}";
    escaped_schema =
        "{\"$schema\":\"http://json-schema.org/draft-07/schema#\","
        "\"type\":\"object\",\"properties\":{\"a\\\"b\":{\"type\":\"array\","
        "\"items\":{}},\"value\":{\"type\":\"number\"}},"
        "\"required\":[\"a\\\"b\",\"value\"]}";
    failures = 0;
    if (!check_schema(
            "{\"name\":\"Ada\",\"age\":37,\"items\":[1,null]}",
            object_schema)) {
        failures++;
    }
    if (!check_schema("[\"a\",\"b\"]", array_schema)) {
        failures++;
    }
    if (!check_schema("true", primitive_schema)) {
        failures++;
    }
    if (!check_schema("{}", empty_schema)) {
        failures++;
    }
    if (!check_schema("{\"a\\\"b\":[],\"value\":1.25}", escaped_schema)) {
        failures++;
    }
    if (jts_json_to_schema("{\"x\":1}", tokens, 4U, output,
                           sizeof(output), &output_length, NULL) !=
        JTS_ERROR_OUTPUT) {
        fprintf(stderr, "expected output buffer error\n");
        failures++;
    }
    if (jts_json_to_schema("{\"x\":1}", tokens, 1U, output,
                           sizeof(output), &output_length, NULL) !=
        JTS_ERROR_TOKENS) {
        fprintf(stderr, "expected token buffer error\n");
        failures++;
    }
    if (jts_json_to_schema("{\"x\":}", tokens, TOKEN_CAPACITY, output,
                           sizeof(output), &output_length, NULL) !=
        JTS_ERROR_JSON) {
        fprintf(stderr, "expected invalid JSON error\n");
        failures++;
    }
    return failures == 0 ? 0 : 1;
}
