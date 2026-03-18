#include <sqlite3.h>
#include <string.h>

#include "database/database.h"
#include "io/io.h"
#include "parse/parse.h"
#include "state_machine/state_machine.h"
#include "parse/parse.h"
#include "parse/parse.ph"
#include "io/log.h"

int main() {
    //sqlite3* db = DATABASE_setup();
    //DATABASE_cleanup(db);

    PARSE_RESULT result = PARSE_parse(IO_RESOURCE("data/20260131_142549.txt"));
    PARSE_free(&result);

    u32 len;
    char* data = IO_read_char(IO_RESOURCE("data/test.txt"), &len);

    MESSAGE_CONTACT contact;
    u8 r = PARSE_get_line_contact_info(data, &contact);

    // TODO!!! MANAGE BOX TYPE SWITCHING IN THE PARSER
    return 0;
}
