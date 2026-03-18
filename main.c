#include <sqlite3.h>
#include <string.h>

#include "database/database.h"
#include "io/io.h"
#include "parse/parse.h"
#include "state_machine/state_machine.h"
#include "parse/parse.h"
#include "io/log.h"

int main() {
    //sqlite3* db = DATABASE_setup();
    //DATABASE_cleanup(db);

    PARSE_RESULT result = PARSE_parse(IO_RESOURCE("data/20260131_142549.txt"));
    PARSE_free(&result);

    // TODO!!! MANAGE BOX TYPE SWITCHING IN THE PARSER
    return 0;
}
