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

int GetOrderById(sqlite3* db, int orderId, Order *order);
int PromptUserForOrder(sqlite3 *db, Order *order);
void PrintOrder(Order *order);
void PrintOrdersGroupedByClient(sqlite3 *db);
void PrintAllOrdersByClientOrderCount(sqlite3 *db);
void PrintCheapestOffersForAllClientOrders(sqlite3 *db);
void FindCheapestShopPerClient(sqlite3 *db);
int ModifyOrder(sqlite3 *db, Order *order);
int DeleteOrder(sqlite3 *db, int orderId);

#endif // ORDERS_H