#ifndef SEAGULL_PARSE2_PH

/*
    Private header for parsing
*/

#include "types.h"
#include "state_machine/state_machine.h"

void PARSE_patch_u16_surrogates(u8* buffer, u32* len);
u8 PARSE_line_is_empty(const u8* buf, const u32 buf_len);

LIST_DECLARE(u32)
LIST_u32* PARSE_find_line_starts(const u8* buf, const u32 buf_len);

typedef struct {
    LIST_MESSAGE* messages;

    MESSAGE_BOX curr_box;
    MESSAGE curr_message;

    LIST_u32* line_starts;
    u32 line_i;
} PARSE_CONTEXT;

u8 PARSE_goto_next_non_empty_line(const u8* buf, const u32 buf_len,
    PARSE_CONTEXT* context);
u8 PARSE_get_line_date_info(const char* buffer, MESSAGE_DATE* date);
u8 PARSE_get_line_contact_info(char* buffer, MESSAGE_CONTACT* contact, const u8 capture);

STATE* STATE_FIND_BOX_action(u8* buf, const u32 buf_len, void* context);
STATE* STATE_CONTACT_action(u8* buf, u32 buf_len, void* context);
STATE* STATE_DATE_action(u8* buf, u32 buf_len, void* context);
STATE* STATE_BODY_action(u8* buf, u32 buf_len, void* context);

#endif // SEAGULL_PARSE2_PH