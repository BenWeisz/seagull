#include <stdio.h>
#include <sqlite3.h>

#include "database.h"

int main() {
    sqlite3* db = DATABASE_setup();
    DATABASE_cleanup(db);

    return 0;
}
