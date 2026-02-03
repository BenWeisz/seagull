#ifndef SEAGULL_DATABASE_H
#define SEAGULL_DATABASE_H

#include "sqlite3.h"

sqlite3* DATABASE_setup();
void DATABASE_cleanup(sqlite3* db);

#endif //SEAGULL_DATABASE_H