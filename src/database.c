#include "database.h"

#include <stdio.h>

#include "types.h"

sqlite3* DATABASE_setup() {
    sqlite3* db;

    // Load the db
    const i32 r = sqlite3_open("seagull.db", &db);
    if (r != SQLITE_OK) {
        fprintf(stderr,"ERROR: Failed to open sqlite database:\n %s", sqlite3_errmsg(db));
        return NULL;
    }

    // Seed the db

    return db;
}

void DATABASE_cleanup(sqlite3* db) {

}