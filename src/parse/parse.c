#include "parse/parse.h"

#include <string.h>

#include "io/io.h"
#include "io/log.h"

LIST_DEFINE(MESSAGE)

PARSE_RESULT PARSE_load(const char* path) {
    PARSE_RESULT result = { NULL, NULL };

    // Load in the message data
    u32 back_buffer_len;
    u8* back_buffer = (u8*)IO_read_char(path, &back_buffer_len);
    if (back_buffer == NULL) {
        LOG_error("Failed to read message data\n");
        return result;
    }

    // Setup MESSAGE list
    LIST_MESSAGE* messages = LIST_MESSAGE_create(32);
    if (messages == NULL) {
        free(back_buffer);
        LOG_error("Failed to create list for messages\n");
        return result;
    }

    // Check for the UTF-8 BOM
    if (!(back_buffer[0] == 0xEF && back_buffer[1] == 0xBB && back_buffer[2] == 0xBF)) {
        free(back_buffer);
        LIST_MESSAGE_destroy(messages);
        LOG_error("The input text messages aren't UTF-8\n");
        return result;
    }

    // Patch up the 16 surrogates
    PARSE_patch_u16_surrogates(back_buffer, back_buffer_len);

    // A number line:
    // Just numbers on a line
    //

    // Set up the result
    result.messages = messages;
    result.back_buffer = back_buffer;
    return result;
}

void PARSE_free(const PARSE_RESULT result) {
    if (result.messages != NULL) LIST_MESSAGE_destroy(result.messages);
    if (result.back_buffer != NULL) free((void*)(result.back_buffer));
}

void PARSE_patch_u16_surrogates(u8* buffer, const u32 len) {
    // len - 5 is the bound because we are only looking for surrogate pairs
    u32 curr_len = len;
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
        }
        else {
            i++;
        }
    }
}