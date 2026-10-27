#include "json_schema.h"

#include <string.h>

#define JTS_SCHEMA_DEPTH 64

typedef struct {
    char *data;
    size_t capacity;
    size_t length;
    int overflow;
} jts_writer;

static void jts_putc(jts_writer *writer, char ch)
{
    if (writer->capacity > 0U && writer->length < writer->capacity - 1U) {
        writer->data[writer->length] = ch;
    } else {
        writer->overflow = 1;
    }
    writer->length++;
}

static void jts_write(jts_writer *writer, const char *text)
{
    while (*text != '\0') {
        jts_putc(writer, *text);
        text++;
    }
}

static unsigned int jts_skip_token(const jsmntok_t *tokens,
                                   unsigned int token_count,
                                   unsigned int index)
{
    int end;
    unsigned int next;

    end = tokens[index].end;
    next = index + 1U;
    while (next < token_count && tokens[next].start < end) {
        next++;
    }
    return next;
}

static int jts_is_integer(const char *json, const jsmntok_t *token)
{
    int index;

    for (index = token->start; index < token->end; index++) {
        if (json[index] == '.' || json[index] == 'e' || json[index] == 'E') {
            return 0;
        }
    }
    return 1;
}

static int jts_write_value_schema(jts_writer *writer, const char *json,
                                  const jsmntok_t *tokens,
                                  unsigned int token_count,
                                  unsigned int index, unsigned int depth)
{
    const jsmntok_t *token;
    unsigned int child;
    int item;

    if (index >= token_count || depth > JTS_SCHEMA_DEPTH) {
        return 0;
    }
    token = &tokens[index];
    if (token->type == JSMN_OBJECT) {
        if (depth == 0U) {
            jts_write(writer, "\"type\":\"object\",\"properties\":{");
        } else {
            jts_write(writer, "{\"type\":\"object\",\"properties\":{");
        }
        child = index + 1U;
        item = 0;
        while (child < token_count &&
               tokens[child].start < token->end - 1) {
            unsigned int value;
            unsigned int next;
            if (tokens[child].type != JSMN_STRING) {
                return 0;
            }
            if (item != 0) {
                jts_putc(writer, ',');
            }
            jts_putc(writer, '"');
            if (tokens[child].end > tokens[child].start) {
                int pos;
                for (pos = tokens[child].start; pos < tokens[child].end;
                     pos++) {
                    jts_putc(writer, json[pos]);
                }
            }
            jts_write(writer, "\":");
            value = child + 1U;
            if (value >= token_count ||
                !jts_write_value_schema(writer, json, tokens, token_count,
                                        value, depth + 1U)) {
                return 0;
            }
            next = jts_skip_token(tokens, token_count, value);
            child = next;
            item++;
        }
        jts_write(writer, "}");
        if (item != 0) {
            jts_write(writer, ",\"required\":[");
            child = index + 1U;
            item = 0;
            while (child < token_count &&
                   tokens[child].start < token->end - 1) {
                unsigned int value;
                unsigned int next;
                if (item != 0) {
                    jts_putc(writer, ',');
                }
                jts_putc(writer, '"');
                if (tokens[child].end > tokens[child].start) {
                    int pos;
                    for (pos = tokens[child].start; pos < tokens[child].end;
                         pos++) {
                        jts_putc(writer, json[pos]);
                    }
                }
                jts_putc(writer, '"');
                value = child + 1U;
                next = jts_skip_token(tokens, token_count, value);
                child = next;
                item++;
            }
            jts_putc(writer, ']');
        }
        jts_putc(writer, '}');
        return 1;
    }
    if (token->type == JSMN_ARRAY) {
        if (depth == 0U) {
            jts_write(writer, "\"type\":\"array\",\"items\":");
        } else {
            jts_write(writer, "{\"type\":\"array\",\"items\":");
        }
        child = index + 1U;
        if (token->size == 0) {
            jts_write(writer, "{}");
        } else if (token->size == 1) {
            if (!jts_write_value_schema(writer, json, tokens, token_count,
                                        child, depth + 1U)) {
                return 0;
            }
        } else {
            jts_write(writer, "{\"anyOf\":[");
            item = 0;
            while (child < token_count &&
                   tokens[child].start < token->end - 1) {
                unsigned int next;
                if (item != 0) {
                    jts_putc(writer, ',');
                }
                if (!jts_write_value_schema(writer, json, tokens,
                                            token_count, child, depth + 1U)) {
                    return 0;
                }
                next = jts_skip_token(tokens, token_count, child);
                child = next;
                item++;
            }
            jts_write(writer, "]}");
        }
        jts_putc(writer, '}');
        return 1;
    }
    if (token->type == JSMN_STRING) {
        if (depth == 0U) {
            jts_write(writer, "\"type\":\"string\"");
        } else {
            jts_write(writer, "{\"type\":\"string\"}");
        }
        if (depth == 0U) {
            jts_putc(writer, '}');
        }
        return 1;
    }
    if (token->type == JSMN_PRIMITIVE) {
        int length;
        length = token->end - token->start;
        if (length == 4 && memcmp(json + token->start, "null", 4U) == 0) {
            jts_write(writer, depth == 0U ?
                      "\"type\":\"null\"" : "{\"type\":\"null\"}");
        } else if (length == 4 &&
                   memcmp(json + token->start, "true", 4U) == 0) {
            jts_write(writer, depth == 0U ?
                      "\"type\":\"boolean\"" : "{\"type\":\"boolean\"}");
        } else if (length == 5 &&
                   memcmp(json + token->start, "false", 5U) == 0) {
            jts_write(writer, depth == 0U ?
                      "\"type\":\"boolean\"" : "{\"type\":\"boolean\"}");
        } else if (jts_is_integer(json, token)) {
            jts_write(writer, depth == 0U ?
                      "\"type\":\"integer\"" : "{\"type\":\"integer\"}");
        } else {
            jts_write(writer, depth == 0U ?
                      "\"type\":\"number\"" : "{\"type\":\"number\"}");
        }
        if (depth == 0U) {
            jts_putc(writer, '}');
        }
        return 1;
    }
    return 0;
}

jts_status jts_json_to_schema(const char *json,
                              jsmntok_t *tokens,
                              unsigned int token_capacity,
                              char *output,
                              size_t output_capacity,
                              size_t *output_length,
                              size_t *error_position)
{
    jsmn_parser parser;
    int token_count;
    jts_writer writer;
    int success;

    if (output_length != NULL) {
        *output_length = 0U;
    }
    if (error_position != NULL) {
        *error_position = 0U;
    }
    if (json == NULL || tokens == NULL || token_capacity == 0U ||
        output == NULL || output_capacity == 0U) {
        return JTS_ERROR_ARGUMENT;
    }
    jsmn_init(&parser);
    token_count = jsmn_parse(&parser, json, strlen(json), tokens,
                             token_capacity);
    if (token_count == JSMN_ERROR_NOMEM) {
        if (error_position != NULL) {
            *error_position = parser.pos;
        }
        output[0] = '\0';
        return JTS_ERROR_TOKENS;
    }
    if (token_count == JSMN_ERROR_DEPTH) {
        if (error_position != NULL) {
            *error_position = parser.pos;
        }
        output[0] = '\0';
        return JTS_ERROR_DEPTH;
    }
    if (token_count < 0) {
        if (error_position != NULL) {
            *error_position = parser.pos;
        }
        output[0] = '\0';
        return JTS_ERROR_JSON;
    }

    writer.data = output;
    writer.capacity = output_capacity;
    writer.length = 0U;
    writer.overflow = 0;
    jts_write(&writer,
              "{\"$schema\":\"http://json-schema.org/draft-07/schema#\",");
    success = jts_write_value_schema(&writer, json, tokens,
                                     (unsigned int)token_count, 0U, 0U);
    if (output_capacity > 0U) {
        if (writer.length < output_capacity) {
            output[writer.length] = '\0';
        } else {
            output[output_capacity - 1U] = '\0';
        }
    }
    if (output_length != NULL) {
        *output_length = writer.length;
    }
    if (!success) {
        return JTS_ERROR_DEPTH;
    }
    if (writer.overflow) {
        return JTS_ERROR_OUTPUT;
    }
    return JTS_OK;
}
