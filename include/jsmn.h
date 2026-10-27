#ifndef JSMN_H
#define JSMN_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    JSMN_UNDEFINED = 0,
    JSMN_OBJECT = 1,
    JSMN_ARRAY = 2,
    JSMN_STRING = 3,
    JSMN_PRIMITIVE = 4
} jsmntype_t;

typedef struct {
    jsmntype_t type;
    int start;
    int end;
    int size;
} jsmntok_t;

typedef struct {
    unsigned int pos;
    unsigned int toknext;
    int toksuper;
} jsmn_parser;

#define JSMN_ERROR_NOMEM (-1)
#define JSMN_ERROR_INVAL (-2)
#define JSMN_ERROR_PART (-3)
#define JSMN_ERROR_DEPTH (-4)
#define JSMN_MAX_DEPTH 64

void jsmn_init(jsmn_parser *parser);
int jsmn_parse(jsmn_parser *parser, const char *json, size_t length,
               jsmntok_t *tokens, unsigned int token_capacity);

#ifdef __cplusplus
}
#endif

#endif
