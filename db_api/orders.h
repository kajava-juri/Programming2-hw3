#ifndef ORDERS_H
#define ORDERS_H

#include <sqlite3.h>
#include "db.h"

typedef struct {
    int id;        
    int client_id; 
    int product_id;
    int amount;
} Order;

/**
 * @brief Creates a new order in the database.
 * @param db Pointer to the SQLite database connection.
 * @param order Pointer to an Order structure that contains the order details.
 * @returns sqlite3 result code or -1 on error.
 */
int InsertOrder(sqlite3 *db, Order *order);


#endif // ORDERS_H