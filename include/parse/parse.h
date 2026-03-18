#ifndef SEAGULL_PARSE2_H
#define SEAGULL_PARSE2_H

#include "types.h"
#include "data/message.h"

typedef struct {
    LIST_MESSAGE* messages;
    const u8* back_buffer;
} PARSE_RESULT;

PARSE_RESULT PARSE_parse(const char* path);
void PARSE_free(PARSE_RESULT* result);

#endif //SEAGULL_PARSE2_H