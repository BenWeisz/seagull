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

    // TODO!!! MANAGE BOX TYPE SWITCHING IN THE PARSER

    return 0;
}
