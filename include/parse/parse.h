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
    s32 year;
    s32 month;
    s32 day;
    s32 hour;
    s32 minute;
} MESSAGE_DATE;

typedef struct {
    char* first_name;
    char* last_name;
    char* phone_number;
} MESSAGE_CONTACT;

MESSAGE_CONTACT MESSAGE_CONTACT_make_blank();

typedef struct {
    MESSAGE_CONTACT contact;
    MESSAGE_DIRECTION direction;
    MESSAGE_DATE date;
    char* body;
    u32 hash;
} MESSAGE;

MESSAGE MESSAGE_make_blank();

LIST_DECLARE(MESSAGE)

typedef struct {
    LIST_MESSAGE* messages;
    const u8* back_buffer;
} PARSE_RESULT;

typedef enum {
    FIND_DIRECTION,
    FIND_CONTACT,
    FIND_TIME,
    FIND_BODY
} PARSE_STATE;

PARSE_RESULT PARSE_load(const char* path);
void PARSE_free(const PARSE_RESULT result);

void PARSE_patch_u16_surrogates(u8* buffer, u32* len);

u8 PARSE_get_line_contact_info(char* buffer, MESSAGE_CONTACT* contact, s64* next_line_pos);
u8 PARSE_get_line_date_info(const char* buffer, MESSAGE_DATE* date, s64* next_line_pos);
s64 PARSE_get_next_line_pos(const char* buffer);

#endif //SEAGULL_PARSE_H