#include <sqlite3.h>
#include <string.h>

#include "database/database.h"
#include "io/io.h"
#include "parse/parse.h"
#include "state_machine/state_machine.h"
#include "parse/parse.h"
#include "parse/parse.ph"
#include "io/log.h"
#include <stdio.h>

int main() {
    //sqlite3* db = DATABASE_setup();
    //DATABASE_cleanup(db);

    PARSE_RESULT result = PARSE_parse(IO_RESOURCE("data/20251117_211356.txt"));
    PARSE_free(&result);

    // u32 len;
    // char* data = IO_read_char(IO_RESOURCE("data/20251117_211356.txt"), &len);
    //
    // MESSAGE_CONTACT contact;
    // u8 r = PARSE_get_line_contact_info(data, &contact, 0);
    // printf("%d\n", r);

    // TODO!!! implement a parsing log that takes the file name from the p_context;

    // TODO!!! MANAGE BOX TYPE SWITCHING IN THE PARSER
    return 0;
}
