#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>

#include <sqlite3.h>
#include "orders.h"
#include "db.h"
#include "../main.h"

void InitOrdersWrapper(GenericWrapper *wrapper)
{
    wrapper->data = NULL;
    wrapper->freeData = NULL;
    wrapper->getElementAt = NULL;
    wrapper->size = 0;
    wrapper->used = 0;
    wrapper->limit = 0;
}

void *GetOrderAt(GenericWrapper *wrapper, size_t index)
{
    if (index >= wrapper->used)
    {
        return NULL; // Index out of bounds
    }
    return ((Order *)wrapper->data) + index;
}

int InsertOrder(sqlite3 *db, Order *order)
{

    // Sanity check for existing order values
    if (order == NULL || !(order->client_id > 0) || !(order->product_id > 0) || !(order->amount > 0))
    {
        fprintf(stderr, "Invalid order data provided.\n");
        return -1;
    }

    sqlite3_stmt *stmt;
    const char *sql = "INSERT INTO orders (client_id, product_id, amount) VALUES (?, ?, ?);";
    int rs;

    if ((rs = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL)) != SQLITE_OK)
    {
        fprintf(stderr, "Error preparing statement: %s\n", sqlite3_errmsg(db));
        return rs;
    }

    sqlite3_bind_int(stmt, 1, order->client_id);
    sqlite3_bind_int(stmt, 2, order->product_id);
    sqlite3_bind_int(stmt, 3, order->amount);

    rs = sqlite3_step(stmt);
    if (rs != SQLITE_DONE)
    {
        fprintf(stderr, "Error executing statement: %s - %s\n", sqlite3_errstr(rs), sqlite3_errmsg(db));
    }
    else
    {
        order->id = (int)sqlite3_last_insert_rowid(db);
    }

    sqlite3_finalize(stmt);
    return rs;
}