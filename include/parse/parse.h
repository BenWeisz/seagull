#ifndef SEAGULL_PARSE_H
#define SEAGULL_PARSE_H

#include "types.h"
#include "nesquik/list.h"

typedef enum {
    INCOMING,
    OUTGOING,
    UNKNOWN
} MESSAGE_DIRECTION;

typedef struct {
    char* first_name;
    char* last_name;
    char* phone_number;
    MESSAGE_DIRECTION direction;
    char* time;
    char* body;
    u32 hash;
} MESSAGE;

LIST_DECLARE(MESSAGE)

typedef struct {
    LIST_MESSAGE* messages;
    const u8* back_buffer;
} PARSE_RESULT;

PARSE_RESULT PARSE_load(const char* path);
void PARSE_free(const PARSE_RESULT result);

void PARSE_patch_u16_surrogates(u8* buffer, const u32 len);

#endif //SEAGULL_PARSE_H