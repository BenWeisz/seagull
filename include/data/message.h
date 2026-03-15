#ifndef SEAGULL_MESSAGE_H
#define SEAGULL_MESSAGE_H

#include "types.h"
#include "list/list.h"

typedef enum {
    MESSAGE_BOX_INBOX,
    MESSAGE_BOX_SENTBOX,
    MESSAGE_BOX_DRAFTBOX,
    MESSAGE_BOX_UNKNOWN
} MESSAGE_BOX;

typedef struct {
    char* first_name;
    char* last_name;
    char* phone_number;
} MESSAGE_CONTACT;

MESSAGE_CONTACT MESSAGE_CONTACT_make_blank();

typedef struct {
    s32 year;
    s32 month;
    s32 day;
    s32 hour;
    s32 minute;
} MESSAGE_DATE;

MESSAGE_DATE MESSAGE_make_date();

typedef struct {
    MESSAGE_CONTACT contact;
    MESSAGE_BOX box;
    MESSAGE_DATE date;
    char* body;
    u32 hash;
} MESSAGE;

LIST_DECLARE(MESSAGE)

MESSAGE MESSAGE_make_blank();

#endif //SEAGULL_MESSAGE_H