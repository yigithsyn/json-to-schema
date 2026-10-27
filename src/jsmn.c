#include "jsmn.h"

#include <limits.h>
#include <string.h>

static void jsmn_skip_space(jsmn_parser *parser, const char *json, int length)
{
    while (parser->pos < (unsigned int)length) {
        char ch;
        ch = json[parser->pos];
        if (ch != ' ' && ch != '\t' && ch != '\r' && ch != '\n') {
            break;
        }
        parser->pos++;
    }
}

static int jsmn_new_token(jsmn_parser *parser, jsmntok_t *tokens,
                          unsigned int capacity, jsmntype_t type,
                          int parent, unsigned int start)
{
    unsigned int index;

    if (parser->toknext >= capacity) {
        return JSMN_ERROR_NOMEM;
    }
    index = parser->toknext++;
    tokens[index].type = type;
    tokens[index].start = (int)start;
    tokens[index].end = -1;
    tokens[index].size = 0;
    if (parent >= 0) {
        tokens[parent].size++;
    }
    return (int)index;
}

static int jsmn_parse_string(jsmn_parser *parser, const char *json, int length,
                             jsmntok_t *tokens, unsigned int capacity,
                             int parent)
{
    unsigned int start;
    int token;

    start = parser->pos;
    parser->pos++;
    token = jsmn_new_token(parser, tokens, capacity, JSMN_STRING,
                           parent, parser->pos);
    if (token < 0) {
        return token;
    }

    while (parser->pos < (unsigned int)length) {
        unsigned char ch;
        ch = (unsigned char)json[parser->pos];
        if (ch == '"') {
            tokens[token].end = (int)parser->pos;
            parser->pos++;
            return token;
        }
        if (ch < 0x20) {
            return JSMN_ERROR_INVAL;
        }
        if (ch == '\\') {
            parser->pos++;
            if (parser->pos >= (unsigned int)length) {
                return JSMN_ERROR_PART;
            }
            ch = (unsigned char)json[parser->pos];
            if (ch == '"' || ch == '\\' || ch == '/' || ch == 'b' ||
                ch == 'f' || ch == 'n' || ch == 'r' || ch == 't') {
                parser->pos++;
            } else if (ch == 'u') {
                int digit;
                parser->pos++;
                for (digit = 0; digit < 4; digit++) {
                    if (parser->pos >= (unsigned int)length) {
                        return JSMN_ERROR_PART;
                    }
                    ch = (unsigned char)json[parser->pos];
                    if (!((ch >= '0' && ch <= '9') ||
                          (ch >= 'a' && ch <= 'f') ||
                          (ch >= 'A' && ch <= 'F'))) {
                        return JSMN_ERROR_INVAL;
                    }
                    parser->pos++;
                }
            } else {
                return JSMN_ERROR_INVAL;
            }
        } else {
            parser->pos++;
        }
    }
    (void)start;
    return JSMN_ERROR_PART;
}

static int jsmn_parse_value(jsmn_parser *parser, const char *json, int length,
                            jsmntok_t *tokens, unsigned int capacity,
                            int parent, unsigned int depth);

static int jsmn_parse_compound(jsmn_parser *parser, const char *json,
                               int length, jsmntok_t *tokens,
                               unsigned int capacity, int parent,
                               unsigned int depth, char opening)
{
    int token;
    char closing;
    int is_object;
    int has_item;

    if (depth >= JSMN_MAX_DEPTH) {
        return JSMN_ERROR_DEPTH;
    }
    is_object = opening == '{';
    closing = is_object ? '}' : ']';
    token = jsmn_new_token(parser, tokens, capacity,
                           is_object ? JSMN_OBJECT : JSMN_ARRAY,
                           parent, parser->pos);
    if (token < 0) {
        return token;
    }
    parser->pos++;
    jsmn_skip_space(parser, json, length);
    has_item = 0;

    if (parser->pos < (unsigned int)length &&
        json[parser->pos] == closing) {
        tokens[token].end = (int)(parser->pos + 1U);
        parser->pos++;
        return token;
    }

    for (;;) {
        int result;
        jsmn_skip_space(parser, json, length);
        if (parser->pos >= (unsigned int)length) {
            return JSMN_ERROR_PART;
        }
        if (is_object) {
            if (json[parser->pos] != '"') {
                return JSMN_ERROR_INVAL;
            }
            result = jsmn_parse_string(parser, json, length, tokens,
                                       capacity, token);
            if (result < 0) {
                return result;
            }
            jsmn_skip_space(parser, json, length);
            if (parser->pos >= (unsigned int)length) {
                return JSMN_ERROR_PART;
            }
            if (json[parser->pos] != ':') {
                return JSMN_ERROR_INVAL;
            }
            parser->pos++;
            result = jsmn_parse_value(parser, json, length, tokens,
                                      capacity, token, depth + 1U);
        } else {
            result = jsmn_parse_value(parser, json, length, tokens,
                                      capacity, token, depth + 1U);
        }
        if (result < 0) {
            return result;
        }
        has_item = 1;
        jsmn_skip_space(parser, json, length);
        if (parser->pos >= (unsigned int)length) {
            return JSMN_ERROR_PART;
        }
        if (json[parser->pos] == closing) {
            tokens[token].end = (int)(parser->pos + 1U);
            parser->pos++;
            return token;
        }
        if (json[parser->pos] != ',') {
            return JSMN_ERROR_INVAL;
        }
        parser->pos++;
        jsmn_skip_space(parser, json, length);
        if (parser->pos < (unsigned int)length &&
            json[parser->pos] == closing) {
            return JSMN_ERROR_INVAL;
        }
        if (!has_item) {
            return JSMN_ERROR_INVAL;
        }
    }
}

static int jsmn_parse_value(jsmn_parser *parser, const char *json, int length,
                            jsmntok_t *tokens, unsigned int capacity,
                            int parent, unsigned int depth)
{
    unsigned int start;
    int token;
    char ch;

    jsmn_skip_space(parser, json, length);
    if (parser->pos >= (unsigned int)length) {
        return JSMN_ERROR_PART;
    }
    ch = json[parser->pos];
    if (ch == '{' || ch == '[') {
        return jsmn_parse_compound(parser, json, length, tokens, capacity,
                                   parent, depth, ch);
    }
    if (ch == '"') {
        return jsmn_parse_string(parser, json, length, tokens, capacity,
                                 parent);
    }

    start = parser->pos;
    if (ch == 't' && parser->pos + 4U <= (unsigned int)length &&
        memcmp(json + parser->pos, "true", 4U) == 0) {
        parser->pos += 4U;
    } else if (ch == 'f' && parser->pos + 5U <= (unsigned int)length &&
               memcmp(json + parser->pos, "false", 5U) == 0) {
        parser->pos += 5U;
    } else if (ch == 'n' && parser->pos + 4U <= (unsigned int)length &&
               memcmp(json + parser->pos, "null", 4U) == 0) {
        parser->pos += 4U;
    } else {
        if (ch == '-') {
            parser->pos++;
            if (parser->pos >= (unsigned int)length) {
                return JSMN_ERROR_PART;
            }
        }
        if (parser->pos >= (unsigned int)length ||
            json[parser->pos] < '0' || json[parser->pos] > '9') {
            return JSMN_ERROR_INVAL;
        }
        if (json[parser->pos] == '0') {
            parser->pos++;
            if (parser->pos < (unsigned int)length &&
                json[parser->pos] >= '0' && json[parser->pos] <= '9') {
                return JSMN_ERROR_INVAL;
            }
        } else {
            while (parser->pos < (unsigned int)length &&
                   json[parser->pos] >= '0' && json[parser->pos] <= '9') {
                parser->pos++;
            }
        }
        if (parser->pos < (unsigned int)length &&
            json[parser->pos] == '.') {
            parser->pos++;
            if (parser->pos >= (unsigned int)length ||
                json[parser->pos] < '0' || json[parser->pos] > '9') {
                return JSMN_ERROR_INVAL;
            }
            while (parser->pos < (unsigned int)length &&
                   json[parser->pos] >= '0' && json[parser->pos] <= '9') {
                parser->pos++;
            }
        }
        if (parser->pos < (unsigned int)length &&
            (json[parser->pos] == 'e' || json[parser->pos] == 'E')) {
            parser->pos++;
            if (parser->pos < (unsigned int)length &&
                (json[parser->pos] == '+' || json[parser->pos] == '-')) {
                parser->pos++;
            }
            if (parser->pos >= (unsigned int)length ||
                json[parser->pos] < '0' || json[parser->pos] > '9') {
                return JSMN_ERROR_INVAL;
            }
            while (parser->pos < (unsigned int)length &&
                   json[parser->pos] >= '0' && json[parser->pos] <= '9') {
                parser->pos++;
            }
        }
    }

    token = jsmn_new_token(parser, tokens, capacity, JSMN_PRIMITIVE,
                           parent, start);
    if (token < 0) {
        return token;
    }
    tokens[token].end = (int)parser->pos;
    return token;
}

void jsmn_init(jsmn_parser *parser)
{
    if (parser != NULL) {
        parser->pos = 0U;
        parser->toknext = 0U;
        parser->toksuper = -1;
    }
}

int jsmn_parse(jsmn_parser *parser, const char *json, size_t length,
               jsmntok_t *tokens, unsigned int token_capacity)
{
    int result;
    int parser_length;

    if (parser == NULL || json == NULL || length > (size_t)INT_MAX ||
        (tokens == NULL && token_capacity != 0U)) {
        return JSMN_ERROR_INVAL;
    }
    parser_length = (int)length;
    parser->pos = 0U;
    parser->toknext = 0U;
    parser->toksuper = -1;
    jsmn_skip_space(parser, json, parser_length);
    if (parser->pos >= (unsigned int)parser_length) {
        return JSMN_ERROR_INVAL;
    }
    result = jsmn_parse_value(parser, json, parser_length, tokens,
                              token_capacity,
                              -1, 0U);
    if (result < 0) {
        return result;
    }
    jsmn_skip_space(parser, json, parser_length);
    if (parser->pos != (unsigned int)parser_length) {
        return JSMN_ERROR_INVAL;
    }
    return (int)parser->toknext;
}
