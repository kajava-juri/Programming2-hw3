#include <sqlite3.h>
#include <stdlib.h>
#include <stdio.h>
#include "db.h"

void db_init(sqlite3 **pdb) {
    int conn = sqlite3_open_v2("shop2.db", pdb, SQLITE_OPEN_READWRITE, NULL);
    if (conn != SQLITE_OK) {
        fprintf(stderr, "Error opening database: %s\n", sqlite3_errmsg(*pdb));
        sqlite3_close(*pdb);
        exit(EXIT_FAILURE);
    }

    // If database could not be opened in read/write mode, it tries to open it in read-only mode
    // we do not want that so check if it is opened in read/write mode
    if(sqlite3_db_readonly(*pdb, "main") != 0) {
        fprintf(stderr, "Could not open database in read/write mode: %s\n", sqlite3_errmsg(*pdb));
        sqlite3_close(*pdb);
        exit(EXIT_FAILURE);
    }

    // Test db connection
    char buffer[256];
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(*pdb, "SELECT 1;", -1, &stmt, NULL) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_DONE) {
            const unsigned char *text = sqlite3_column_text(stmt, 0);
            if (text) strcpy(buffer, (const char*)text);
        }
        int rs = sqlite3_finalize(stmt);
        if(rs != SQLITE_OK) {
            fprintf(stderr, "Error finalizing statement: %s\n", sqlite3_errstr(rs));
            sqlite3_close(*pdb);
            exit(EXIT_FAILURE);
        }
        
    }
    if (sqlite3_errcode(*pdb) != SQLITE_OK) {
        fprintf(stderr, "Error executing test query: %s\n", sqlite3_errmsg(*pdb));
        sqlite3_close(*pdb);
        exit(EXIT_FAILURE);
    }
    printf("Test query output: %s\n", buffer);
    printf("Database opened successfully in read/write mode.\n");
}