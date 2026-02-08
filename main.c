#include <sqlite3.h>
#include <string.h>

#include "database/database.h"
#include "io/io.h"
#include "parse/parse.h"

int main() {
    sqlite3* db = DATABASE_setup();
    DATABASE_cleanup(db);

    const PARSE_RESULT result = PARSE_load(IO_RESOURCE("data/20241217_094850.txt"));
    PARSE_free(result);

    const char* test_seed = "+11234567890\r\n2025/12/08 16:33";
    char test[1024];
    strcpy(test, test_seed);

    MESSAGE_CONTACT contact;
    u8 r = PARSE_get_line_contact_info(test, &contact, NULL);

    return 0;
}
