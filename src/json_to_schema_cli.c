#include "json_schema.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define JTS_INPUT_CHUNK 1024U
#define JTS_VERSION "1.0.0"

static int allocation_size_overflows(size_t count, size_t element_size)
{
    return element_size != 0U && count > (size_t)-1 / element_size;
}

static int read_stdin(char **data, size_t *length)
{
    char chunk[JTS_INPUT_CHUNK];
    char *buffer;
    size_t used;
    size_t capacity;
    size_t count;
    char *resized;

    buffer = NULL;
    used = 0U;
    capacity = 0U;
    for (;;) {
        count = fread(chunk, 1U, sizeof(chunk), stdin);
        if (count > 0U) {
            if (used > (size_t)-1 - count - 1U) {
                free(buffer);
                return 0;
            }
            if (used + count + 1U > capacity) {
                size_t needed;
                size_t next_capacity;
                needed = used + count + 1U;
                next_capacity = capacity == 0U ? JTS_INPUT_CHUNK : capacity;
                while (next_capacity < needed) {
                    if (next_capacity > ((size_t)-1) / 2U) {
                        next_capacity = needed;
                        break;
                    }
                    next_capacity *= 2U;
                }
                resized = (char *)realloc(buffer, next_capacity);
                if (resized == NULL) {
                    free(buffer);
                    return 0;
                }
                buffer = resized;
                capacity = next_capacity;
            }
            memcpy(buffer + used, chunk, count);
            used += count;
        }
        if (count < sizeof(chunk)) {
            if (ferror(stdin)) {
                free(buffer);
                return 0;
            }
            break;
        }
    }

    if (buffer == NULL) {
        buffer = (char *)malloc(1U);
        if (buffer == NULL) {
            return 0;
        }
    }
    buffer[used] = '\0';
    *data = buffer;
    *length = used;
    return 1;
}

static void print_usage(FILE *stream, const char *program)
{
    fprintf(stream, "Usage: %s [JSON | -]\n", program);
    fprintf(stream,
            "Pass JSON as one argument or read it from standard input.\n");
    fprintf(stream, "Options:\n");
    fprintf(stream, "  -h, --help     Show this help message\n");
    fprintf(stream, "  --version      Show version information\n");
}

int main(int argc, char **argv)
{
    char *json;
    size_t json_length;
    jsmntok_t *tokens;
    char output_probe[1];
    char *output;
    size_t output_length;
    jts_status status;
    unsigned int token_capacity;

    json = NULL;
    json_length = 0U;
    if (argc == 2 &&
        (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)) {
        print_usage(stdout, argv[0]);
        return 0;
    }
    if (argc == 2 && strcmp(argv[1], "--version") == 0) {
        printf("json-to-schema %s\n", JTS_VERSION);
        return 0;
    }
    if (argc > 2) {
        print_usage(stderr, argv[0]);
        return 2;
    }
    if (argc == 2 && strcmp(argv[1], "-") != 0) {
        json = argv[1];
        json_length = strlen(json);
    } else if (!read_stdin(&json, &json_length)) {
        fprintf(stderr, "json-to-schema: failed to read standard input\n");
        return 1;
    }

    if (json_length > (size_t)INT_MAX ||
        json_length > (size_t)UINT_MAX) {
        fprintf(stderr, "json-to-schema: input is too large\n");
        if (argc != 2 || strcmp(argv[1], "-") == 0) {
            free(json);
        }
        return 1;
    }
    if (memchr(json, '\0', json_length) != NULL) {
        fprintf(stderr, "json-to-schema: input contains a NUL byte\n");
        if (argc != 2 || strcmp(argv[1], "-") == 0) {
            free(json);
        }
        return 1;
    }
    token_capacity = json_length == 0U ? 1U : (unsigned int)json_length;
    if (allocation_size_overflows((size_t)token_capacity, sizeof(*tokens))) {
        fprintf(stderr, "json-to-schema: input is too large\n");
        if (argc != 2 || strcmp(argv[1], "-") == 0) {
            free(json);
        }
        return 1;
    }
    tokens = (jsmntok_t *)malloc((size_t)token_capacity * sizeof(*tokens));
    if (tokens == NULL) {
        fprintf(stderr, "json-to-schema: unable to allocate token buffer\n");
        if (argc != 2 || strcmp(argv[1], "-") == 0) {
            free(json);
        }
        return 1;
    }

    status = jts_json_to_schema(json, tokens, token_capacity, output_probe,
                                sizeof(output_probe), &output_length, NULL);
    if (status != JTS_ERROR_OUTPUT && status != JTS_OK) {
        fprintf(stderr, "json-to-schema: conversion failed (error %d)\n",
                (int)status);
        free(tokens);
        if (argc != 2 || strcmp(argv[1], "-") == 0) {
            free(json);
        }
        return 1;
    }
    if (output_length == (size_t)-1) {
        fprintf(stderr, "json-to-schema: generated schema is too large\n");
        free(tokens);
        if (argc != 2 || strcmp(argv[1], "-") == 0) {
            free(json);
        }
        return 1;
    }
    output = (char *)malloc(output_length + 1U);
    if (output == NULL) {
        fprintf(stderr, "json-to-schema: unable to allocate output buffer\n");
        free(tokens);
        if (argc != 2 || strcmp(argv[1], "-") == 0) {
            free(json);
        }
        return 1;
    }
    status = jts_json_to_schema(json, tokens, token_capacity, output,
                                output_length + 1U, &output_length, NULL);
    if (status != JTS_OK) {
        fprintf(stderr, "json-to-schema: conversion failed (error %d)\n",
                (int)status);
        free(output);
        free(tokens);
        if (argc != 2 || strcmp(argv[1], "-") == 0) {
            free(json);
        }
        return 1;
    }
    if (fputs(output, stdout) == EOF || fputc('\n', stdout) == EOF) {
        fprintf(stderr, "json-to-schema: failed to write standard output\n");
        free(output);
        free(tokens);
        if (argc != 2 || strcmp(argv[1], "-") == 0) {
            free(json);
        }
        return 1;
    }

    free(output);
    free(tokens);
    if (argc != 2 || strcmp(argv[1], "-") == 0) {
        free(json);
    }
    return 0;
}
