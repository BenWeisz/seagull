#include "parse/parse.h"

#include <stdio.h>
#include <string.h>

#include "io/io.h"
#include "io/log.h"

MESSAGE_CONTACT MESSAGE_CONTACT_make_blank() {
    const MESSAGE_CONTACT contact = { NULL, NULL, NULL };
    return contact;
}

MESSAGE MESSAGE_make_blank() {
    const MESSAGE_CONTACT contact = MESSAGE_CONTACT_make_blank();
    const MESSAGE_DATE date = { 0, 0, 0, 0, 0 };
    const MESSAGE message = { contact, UNKNOWN, date, NULL, 0 };
    return message;
}

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
    PARSE_patch_u16_surrogates(back_buffer, &back_buffer_len);

    /*
     * Combined Headers
     *
     * Inbox SMS:
     * EF BB BF     0D 0A       49 6E 62 6F 78 20 53 4D 53  0D 0A
     * (UTF-8 BOM)  (New Line)  Inbox SMS                   (New Line)
     *
     * Sentbox SMS:
     * EF BB BF     0D 0A       53 65 6E 74 62 6F 78 20 53 4D 53    0D 0A
     * (UTF-8 BOM)  (New Line)  Sentbox SMS                         (New Line)
     */
    u8* curr_buffer = back_buffer;

    const u8 utf8_bom[] = { 0xEF, 0xBB, 0xBF, 0x0D, 0x0A };
    const char* inbox_header = "Inbox SMS\r\n";
    const char* sentbox_header = "Sentbox SMS\r\n";

    // The main parsing loop
    MESSAGE_DIRECTION direction = UNKNOWN;
    PARSE_STATE state = FIND_DIRECTION;

    MESSAGE message = MESSAGE_make_blank();
    MESSAGE_CONTACT message_contact_cache = MESSAGE_CONTACT_make_blank();
    u8 is_first_message = 1;
    u8 signal_exit = 0;
    while (*curr_buffer != '\0') {
        switch (state) {
            case FIND_DIRECTION: {
                // Find the Inbox or Sentbox header
                u32 i = 0;
                u8 found_utf8_bom = 1;
                while (i < 5 && curr_buffer[i] != '\0') {
                    if (curr_buffer[i] != utf8_bom[i]) {
                        found_utf8_bom = 0;
                        break;
                    }
                    i++;
                }

                // If you haven't found the UTF8 bom, continue
                if (found_utf8_bom == 0 || curr_buffer[i] == '\0') {
                    curr_buffer++;
                    break;
                }

                const u32 ti = i;

                // Find the Inbox Header
                u8 found_inbox_header = 1;
                while (i < 16 && curr_buffer[i] != '\0') {
                    if (curr_buffer[i] != inbox_header[i - 5]) {
                        found_inbox_header = 0;
                        break;
                    }
                    i++;
                }

                if (curr_buffer[i] == '\0') {
                    curr_buffer++;
                    break;
                }

                // If we find the inbox header, move the curr_buffer forward
                // and change the parsing state
                if (found_inbox_header == 1) {
                    state = FIND_CONTACT;
                    direction = INCOMING;
                    curr_buffer += 16;
                    break;
                }

                // If we didnt find the inbox header, look for the sentbox header
                u8 found_sentbox_header = 1;
                i = ti;
                while (i < 18 && curr_buffer[i] != '\0') {
                    if (curr_buffer[i] != sentbox_header[i - 5]) {
                        found_sentbox_header = 0;
                        break;
                    }
                    i++;
                }

                if (curr_buffer[i] == '\0') {
                    curr_buffer++;
                    break;
                }

                // If we find the sentbox header, move the curr_buffer forward
                // and change the parsing state
                if (found_sentbox_header == 1) {
                    state = FIND_CONTACT;
                    direction = OUTGOING;
                    curr_buffer += 18;
                    break;
                }

                // Oh man, we couldn't find anything :(
                curr_buffer++;

                break;
            }
            case FIND_CONTACT: {
                // The assumption is that we've processed the previous message
                // before switching to the FIND_CONTACT state

                if (is_first_message == 1) {
                    MESSAGE_CONTACT contact;
                    s64 next_line_pos;
                    const u8 r = PARSE_get_line_contact_info((char*)curr_buffer, &contact, &next_line_pos);
                    if (r == 0) {
                        if (next_line_pos == -1) {
                            signal_exit = 1;
                            break;
                        }

                        curr_buffer += next_line_pos;
                        break;
                    }

                    is_first_message = 0;
                    message_contact_cache = contact;

                    // next_line_pos must be > -1 because otherwise this wouldn't be a contact line
                    curr_buffer += next_line_pos;
                }

                // We've found a contact line so create a new message
                message = MESSAGE_make_blank();
                message.direction = direction;
                message.contact = message_contact_cache;
                state = FIND_TIME;
                break;
            }
            case FIND_TIME: {
                // We are pretty much guaranteed to find the date line
                // because it is a requirement that it succeeds the contact line

                MESSAGE_DATE date;
                s64 next_line_pos;
                const u8 r = PARSE_get_line_date_info((char*)curr_buffer, &date, &next_line_pos);
                // If there's no next line or this line isn't a date this is a bad message
                if (r == 0 || next_line_pos == -1) {
                    // Something went wrong
                    signal_exit = 1;
                    break;
                }

                // We've got a proper message below
                message.date = date;
                state = FIND_BODY;

                curr_buffer += next_line_pos;
                break;
            }
            case FIND_BODY: {
                // Save the curr_buffer so we know where the start of the body is
                const u8* body_start = curr_buffer;

                // We want to find the next contact line
                s64 next_line_pos;
                u8 r = 0;
                u8* end_of_body = curr_buffer;
                while (r == 0) {
                    r = PARSE_get_line_contact_info((char*)curr_buffer, &message_contact_cache, &next_line_pos);
                    if (next_line_pos == -1) break;

                    if (r != 0) end_of_body = curr_buffer;
                    curr_buffer += next_line_pos;
                }

                // We've come to the end of the message body
                // Rewind all of the trailing \r\n and then drop a null terminator
                while (*(end_of_body - 2) == '\r' && *(end_of_body - 1) == '\n' && end_of_body != body_start) {
                    end_of_body -= 2;
                }

                // Cap off the body
                end_of_body[0] = 0x00;

                // Set the body
                message.body = (char*)body_start;
                r = LIST_MESSAGE_push(messages, message);
                if (r == 0) {
                    signal_exit = 1;
                    break;
                }

                state = FIND_CONTACT;

                break;
            }
        }

        if (signal_exit == 1) break;
    }

    // Set up the result
    result.messages = messages;
    result.back_buffer = back_buffer;
    return result;
}

void PARSE_free(const PARSE_RESULT result) {
    if (result.messages != NULL) LIST_MESSAGE_destroy(result.messages);
    if (result.back_buffer != NULL) free((void*)(result.back_buffer));
}

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

/* Figure out if this is contact line
 *
 * If there is a next line, and it's a date line and,
 * you can find a phone number on this line, then this
 * is a contact line.
 *
 * Return:
 *      0   - If this is not a contact line
 *      1   - If this is a contact line
 *
 * If a return of 1 is given, the contact struct will
 * be filled out. The next_line_pos will be filled out
 * if there is a next line
 */
u8 PARSE_get_line_contact_info(char* buffer, MESSAGE_CONTACT* contact, s64* next_line_pos) {
    contact->first_name = NULL;
    contact->last_name = NULL;
    contact->phone_number = NULL;

    const s64 r_next_line = PARSE_get_next_line_pos(buffer);
    if (next_line_pos != NULL) *next_line_pos = r_next_line;
    if (r_next_line == -1) return 0;

    MESSAGE_DATE date;
    const u8 r_is_date = PARSE_get_line_date_info(buffer + r_next_line, &date, NULL);
    if (r_is_date == 0) return 0;

    // Figure out where the name ends
    u32 i = 0;
    while (buffer[i] != '(' && buffer[i] != '\n') i++;

    // If there's a bracket, check if there's a number after it
    if (buffer[i] == '(') {
        u32 j = i + 1;
        if (buffer[j] == '+') {
            j++;
        }

        const u32 phone_number_start = j;
        u32 phone_number_num_digits = 0;
        while (buffer[j] != ')' && buffer[j] != '\n') {
            if (buffer[j] >= '0' && buffer[j] <= '9') phone_number_num_digits++;
            // This isn't a phone number
            else return 0;
            j++;
        }

        // Welp, it looks like this isn't a well-formed contact line
        if (buffer[j] != ')' || phone_number_num_digits < 3) {
            return 0;
        }

        buffer[j] = '\0';
        contact->phone_number = buffer + phone_number_start;

        // So there is a phone number, so we can parse up the name too
        buffer[i] = '\0';

        // Find the last space because that's where
        // we assume the last name begins
        s64 last_space_pos = -1;
        i = 0;
        while (buffer[i] != '\0') {
            if (buffer[i] == ' ') last_space_pos = i;
            i++;
        }

        // There was no space, so this is a first name
        if (last_space_pos == -1) {
            contact->first_name = buffer;
        }
        else {
            buffer[last_space_pos] = '\0';
            contact->last_name = buffer + last_space_pos + 1;
            contact->first_name = buffer;
        }
    }
    // Handle the numbers only case
    else {
        i = 0;
        if (buffer[i] == '+') i++;

        const u32 phone_number_start = i;
        u32 phone_number_num_digits = 0;
        while (buffer[i] != '\r' && buffer[i] != '\n') {
            if (buffer[i] >= '0' && buffer[i] <= '9') phone_number_num_digits++;
            // This is not a phone number
            else return 0;
            i++;
        }

        buffer[i] = '\0';
        if (phone_number_num_digits < 3) return 0;
        contact->phone_number = buffer + phone_number_start;
    }

    return 1;
}

u8 PARSE_get_line_date_info(const char* buffer, MESSAGE_DATE* date, s64* next_line_pos) {
    date->year = -1;
    date->month = -1;
    date->day = -1;
    date->hour = -1;
    date->minute = -1;

    if (next_line_pos != NULL) {
        *next_line_pos = PARSE_get_next_line_pos(buffer);
    }

    const s32 r = sscanf(buffer, "%d/%d/%d %d:%d",
        &(date->year), &(date->month), &(date->day),
        &(date->hour), &(date->minute));
    if (r != 5) return 0;
    return 1;
}

s64 PARSE_get_next_line_pos(const char* buffer) {
    const char* curr_buffer = buffer;

    u8 found_newline = 0;

    while (*curr_buffer != '\0') {
        const u8 curr_char = *curr_buffer;
        if (found_newline == 0 && curr_char == '\n') {
            found_newline = 1;
        }
        else if (found_newline == 1) {
            return curr_buffer - buffer;
        }
        curr_buffer++;
    }

    return -1;
}