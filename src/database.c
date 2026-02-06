#include "database.h"

#include <stdio.h>
#include <stdlib.h>

#include "types.h"
#include "io.h"

sqlite3* DATABASE_setup() {
    sqlite3* db = NULL;

    // Load the db
    const i32 r = sqlite3_open("seagull.db", &db);
    if (r != SQLITE_OK) {
        fprintf(stderr,"ERROR: Failed to open sqlite database:\n %s", sqlite3_errmsg(db));
        return NULL;
    }

    // Load the seeding seed_query
    char* seed_query = IO_read_char(IO_RESOURCE("sql/seed.sql"), NULL);
    if (seed_query == NULL) {
        fprintf(stdout, "WARNING: Seed query could not be loaded from disk\n");
    }
    else {
        char* error_message = NULL;
        sqlite3_exec(db, seed_query, NULL, NULL, &error_message);

        if (error_message != NULL) {
            fprintf(stderr,"ERROR: Seed query failed with the message below:\n%s\n", error_message);
            sqlite3_free(error_message);
        }

        free(seed_query);
    }

    return db;
}

void DATABASE_cleanup(sqlite3* db) {
    const i32 status = sqlite3_close(db);
    if (status != SQLITE_OK) {
        fprintf(stderr,"ERROR: Failed to close sqlite database:\n %s", sqlite3_errmsg(db));
    }
}