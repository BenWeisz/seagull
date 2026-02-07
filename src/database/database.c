#include "database/database.h"

#include <stdio.h>
#include <stdlib.h>

#include "types.h"
#include "io/io.h"
#include "io/log.h"

sqlite3* DATABASE_setup() {
    sqlite3* db = NULL;

    // Load the db
    const s32 r = sqlite3_open("seagull.db", &db);
    if (r != SQLITE_OK) {
        LOG_error("Failed to open sqlite database:\n %s", sqlite3_errmsg(db));
        return NULL;
    }

    // Load the seeding seed_query
    char* seed_query = IO_read_char(IO_RESOURCE("sql/seed.sql"), NULL);
    if (seed_query == NULL) {
        LOG_warn("Seed query could not be loaded from disk\n");
    }
    else {
        char* error_message = NULL;
        sqlite3_exec(db, seed_query, NULL, NULL, &error_message);

        if (error_message != NULL) {
            LOG_error("Seed query failed with the message below:\n%s\n", error_message);
            sqlite3_free(error_message);
        }
        else {
            LOG("Seeded database\n");
        }

        free(seed_query);
    }

    return db;
}

void DATABASE_cleanup(sqlite3* db) {
    const s32 status = sqlite3_close(db);
    if (status != SQLITE_OK) {
        LOG_error("Failed to close sqlite database:\n %s", sqlite3_errmsg(db));
    }
}