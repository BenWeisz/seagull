#include "parse/parse.h"

#include <string.h>
#include <stdio.h>

#include "types.h"
#include "utils.h"
#include "io/log.h"
#include "io/io.h"
#include "parse/parse.ph"

STATE_CREATE(STATE_FIND_BOX,    STATE_TYPE_START)
STATE_CREATE(STATE_CONTACT,     STATE_TYPE_INTERNAL)
STATE_CREATE(STATE_TIME,        STATE_TYPE_INTERNAL)
STATE_CREATE(STATE_BODY,        STATE_TYPE_INTERNAL)
STATE_CREATE(STATE_END,         STATE_TYPE_END)
STATE_CREATE(STATE_ERROR,       STATE_TYPE_END)

PARSE_RESULT PARSE_parse(const char* path) {
    PARSE_RESULT result = { NULL, NULL };

    // Read in the back buffer
    u32 back_buffer_len;
    u8* back_buffer = (u8*)IO_read_char(path, &back_buffer_len);
    if (back_buffer == NULL) {
        LOG_error("Failed to read file %s\n", path);
        return result;
    }

    // Patch up the UTF-16 surrogates
    PARSE_patch_u16_surrogates(back_buffer, &back_buffer_len);

    // Set up the data parsing state machine
    STATE_MACHINE* state_machine = STATE_MACHINE_create(back_buffer, back_buffer_len);
    if (state_machine == NULL) {
        free(back_buffer);
        LOG_error("Failed to create state machine\n");
        return result;
    }

    // Add the parsing states
    u8 r = STATE_MACHINE_add_state(state_machine, &STATE_FIND_BOX, STATE_FIND_BOX_action);
    r &= STATE_MACHINE_add_state(state_machine, &STATE_CONTACT, STATE_CONTACT_action);
    r &= STATE_MACHINE_add_state(state_machine, &STATE_TIME, STATE_TIME_action);
    r &= STATE_MACHINE_add_state(state_machine, &STATE_BODY, STATE_BODY_action);
    if (r == 0) {
        free(back_buffer);
        STATE_MACHINE_destroy(state_machine);
        LOG_error("Failed to add parsing states\n");
        return result;
    }

    // Set up the message list
    LIST_MESSAGE* messages = LIST_MESSAGE_create(32);
    if (messages == NULL) {
        free(back_buffer);
        STATE_MACHINE_destroy(state_machine);
        LOG_error("Failed to create LIST_MESSAGE\n");
        return result;
    }

    // Parse in the line start indices
    LIST_u32* line_starts = PARSE_find_line_starts(back_buffer, back_buffer_len);
    if (line_starts == NULL) {
        free(back_buffer);
        STATE_MACHINE_destroy(state_machine);
        LIST_MESSAGE_destroy(messages);
        LOG_error("Failed to parse line starts\n");
        return result;
    }

    // Set up the state machine context
    PARSE_CONTEXT context;
    context.messages = messages;
    context.curr_box = MESSAGE_BOX_UNKNOWN;
    context.line_starts = line_starts;
    context.line_i = 0;

    // Run the state machine
    const STATE* state_r = STATE_MACHINE_run(state_machine, &context);
    if (state_r == NULL || strcmp(state_r->name, "STATE_ERROR") == 0) return result;

    // Set up the result
    result.messages = messages;
    result.back_buffer = back_buffer;

    // Clean up the things we no longer need
    STATE_MACHINE_destroy(state_machine);
    LIST_u32_destroy(line_starts);

    return result;
}

void PARSE_free(PARSE_RESULT* result) {
    if (result == NULL) return;

    LIST_MESSAGE_destroy(result->messages);
    result->messages = NULL;

    free((void*)result->back_buffer);
    result->back_buffer = NULL;
}

/////////////////////////////////////////////////////////////////////////
// parse.ph
/////////////////////////////////////////////////////////////////////////

void PARSE_patch_u16_surrogates(u8* buffer, u32* len) {
    // len - 5 is the bound because we are only looking for surrogate pairs
    u32 curr_len = *len;
    u32 i = 0;
    while (i < curr_len - 5) {
        // Found a surrogate pair!
        if (buffer[i] == 0xED && (buffer[i + 1] & 0xF0) == 0xA0) {
            // Compute the surrogate pair
            const u16 high = ((buffer[i + 1] & 0x0F) << 6) | (buffer[i + 2] & 0x3F);
            const u16 low = ((buffer[i + 4] & 0x0F) << 6) | (buffer[i + 5] & 0x3F);

            const u32 codepoint = 0x10000 + (high << 10) + low;

            // Update the buffer with the correct UTF-8 encoding
            buffer[i] = 0xF0 | (codepoint >> 18);
            buffer[i + 1] = 0x80 | ((codepoint >> 12) & 0x3F);
            buffer[i + 2] = 0x80 | ((codepoint >> 6) & 0x3F);
            buffer[i + 3] = 0x80 | (codepoint & 0x3F);

            // Move up the remaining data (sub 6) but add 1 because we need the null terminator
            memmove(buffer + i + 4, buffer + i + 6, curr_len - i - 6 + 1);
            curr_len -= 2;
            i += 4;

            continue;
        }

        i++;
    }

    *len = curr_len;
}

LIST_u32* PARSE_find_line_starts(const u8* buf, const u32 buf_len) {
    // Create a list containing the index of the start of each line in buf

    // Create the line list
    LIST_u32* lines = LIST_u32_create(256);
    if (lines == NULL) {
        LOG_error("Failed to allocate memory for LIST_u32");
        return NULL;
    }

    if (buf_len == 0) return lines;
    LIST_u32_push(lines, 0);

    u32 pos = 0;
    while (pos < buf_len) {
        if (buf[pos] == 0x0A && pos + 1 < buf_len) {
            LIST_u32_push(lines, pos + 1);
        }
        pos++;
    }

    return lines;
}

u8 PARSE_line_is_empty(const u8* buf, const u32 buf_len) {
    // Figure out if the line is empty
    if (buf_len < 2) return 1;
    if (buf[0] == 0x0D && buf[1] == 0x0A) return 1;
    return 0;
}

u8 PARSE_goto_next_non_empty_line(const u8* buf, const u32 buf_len,
    PARSE_CONTEXT* context) {

    const LIST_u32* line_starts = context->line_starts;
    if (line_starts == NULL) return 0;

    // Find the next non-empty line
    while (context->line_i < line_starts->size) {
        u32 line_loc;
        LIST_u32_get(line_starts, context->line_i, &line_loc);

        if (!PARSE_line_is_empty(buf + line_loc, buf_len - line_loc)) break;
        context->line_i++;
    }

    if (context->line_i == line_starts->size) return 0;

    return 1;
}

u8 PARSE_get_line_date_info(const char* buffer, MESSAGE_DATE* date) {
    date->year = -1;
    date->month = -1;
    date->day = -1;
    date->hour = -1;
    date->minute = -1;

    const s32 r = sscanf(buffer, "%d/%d/%d %d:%d",
        &(date->year), &(date->month), &(date->day),
        &(date->hour), &(date->minute));
    if (r != 5) return 0;
    return 1;
}

u8 PARSE_get_line_contact_info(char* buffer, MESSAGE_CONTACT* contact) {
    contact->first_name = NULL;
    contact->last_name = NULL;
    contact->phone_number = NULL;

    // Look for bracket pair first
    u8 found_open_bracket = 0;
    u32 i = 0;
    while (buffer[i] != '\r' && buffer[i] != '\n' && buffer[i] != '\0') {
        if (buffer[i] == '(') found_open_bracket = 1;
        if (buffer[i] == ')') {
            if (found_open_bracket == 0) return 0;
            break;
        }
        i++;
    }

    if ((buffer[i] == '\r' || buffer[i] == '\n' || buffer[i] == '\0')
        && found_open_bracket == 1) return 0;

    // With name
    if (found_open_bracket == 1) {
        i = 0;

        while (buffer[i] != '(' && buffer[i] != ' ') i++;
        const u8 has_last_name = (buffer[i] == ' ');

        // Capture the first name
        buffer[i] = '\0';
        contact->first_name = buffer;
        i++;

        // Capture the optional last name
        if (has_last_name) {
            buffer += i;
            i = 0;
            while (buffer[i] != '(') i++;
            buffer[i] = '\0';
            contact->last_name = buffer;
            i++;
        }

        // Capture the phone number
        buffer += i;
        i = 0;
        if (buffer[0] == '+') i++;
        while (buffer[i] != ')') {
            if (buffer[i] < '0' || buffer[i] > '9') return 0;
            i++;
        }

        buffer[i] = '\0';
        contact->phone_number = buffer;
    }
    // Without name
    else {
        i = 0;
        if (buffer[0] == '+') i++;
        while (buffer[i] != '\r' && buffer[i] != '\n' && buffer[i] != '\0') {
            if (buffer[i] < '0' || buffer[i] > '9') return 0;
            i++;
        }

        buffer[i] = '\0';
        contact->phone_number = buffer;
    }

    return 1;
}

STATE* STATE_FIND_BOX_action(u8* buf, const u32 buf_len, void* context) {
    PARSE_CONTEXT* p_context = (PARSE_CONTEXT*)context;

    // Make sure the context exists
    if (p_context == NULL) return &STATE_ERROR;

    const LIST_u32* line_starts = p_context->line_starts;
    if (line_starts == NULL) return &STATE_ERROR;

    // Find a message box
    p_context->curr_box = MESSAGE_BOX_UNKNOWN;
    while (p_context->line_i < line_starts->size) {
        if (!PARSE_goto_next_non_empty_line(buf, buf_len, p_context)) return &STATE_ERROR;

        u32 line_loc;
        LIST_u32_get(line_starts, p_context->line_i++, &line_loc);

        if (line_loc + 3 >= buf_len) return &STATE_ERROR;
        if (!(buf[line_loc] == 0xEF && buf[line_loc + 1] == 0xBB && buf[line_loc + 2] == 0xBF)) return &STATE_ERROR;
        if (p_context->line_i >= line_starts->size) return &STATE_ERROR;

        // Find the next non-empty line
        if (!PARSE_goto_next_non_empty_line(buf, buf_len, p_context)) return &STATE_ERROR;
        LIST_u32_get(line_starts, p_context->line_i++, &line_loc);

        u32 box_i = 0;
        while (box_i < 3) {
            const char* message_box_names[3] = { "Inbox SMS", "Sentbox SMS", "Draftbox SMS" };
            const u64 len = strlen(message_box_names[box_i]);
            const s32 sr = strncmp(message_box_names[box_i], (char*)buf + line_loc, MIN(len, buf_len - line_loc));
            if (sr == 0) {
                if (box_i == 0) p_context->curr_box = MESSAGE_BOX_INBOX;
                else if (box_i == 1) p_context->curr_box = MESSAGE_BOX_SENTBOX;
                else p_context->curr_box = MESSAGE_BOX_DRAFTBOX;
                break;
            }

            box_i++;
        }

        if (p_context->curr_box != MESSAGE_BOX_UNKNOWN) break;

    }

    // Find the next non-empty line
    if (!PARSE_goto_next_non_empty_line(buf, buf_len, p_context)) return &STATE_ERROR;

    return &STATE_CONTACT;
}

STATE* STATE_CONTACT_action(u8* buf, u32 buf_len, void* context) {
    return NULL;
}

STATE* STATE_TIME_action(u8* buf, u32 buf_len, void* context) {
    return NULL;
}

STATE* STATE_BODY_action(u8* buf, u32 buf_len, void* context) {
    return NULL;
}