#include <stdlib.h>
#include <stdio.h>
#include <sqlite3.h>
#include "db_api/db.h"
#include "main.h"

int main(void) {

    sqlite3 *db = NULL;
    db_init(&db);

    return 0;
}