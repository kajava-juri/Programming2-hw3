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

/**
 * @brief Retrieves an order from the database by its ID.
 * @param db Pointer to the SQLite database connection.
 * @param orderId The ID of the order to retrieve.
 * @param order Pointer to an Order structure where the retrieved data will be stored.
 * @returns sqlite3 result code or -1 on error.
 */
int GetOrderById(sqlite3* db, int orderId, Order *order);

/**
 * @brief Prompts the user to enter order details and validates the input.
 * @param db Pointer to the SQLite database connection.
 * @param order Pointer to an Order structure where the user input will be stored.
 * @returns 1 on success, 0 if user cancelled, -1 on error.
 */
int PromptUserForOrder(sqlite3 *db, Order *order);

/**
 * @brief Prints the details of a single order to the console.
 * @param order Pointer to the Order structure to print.
 */
void PrintOrder(Order *order);

/**
 * @brief Prints all orders grouped by client to the console.
 * @param db Pointer to the SQLite database connection.
 */
void PrintOrdersGroupedByClient(sqlite3 *db);

/**
 * @brief Prints all orders sorted by client order count to the console.
 * @param db Pointer to the SQLite database connection.
 */
void PrintAllOrdersByClientOrderCount(sqlite3 *db);

/**
 * @brief Prints the cheapest offers for all client orders to the console.
 * @param db Pointer to the SQLite database connection.
 */
void PrintCheapestOffersForAllClientOrders(sqlite3 *db);

/**
 * @brief Prints potential savings per client to the console.
 * @param db Pointer to the SQLite database connection.
 */
void PrintPotentialSavingsPerClient(sqlite3 *db);

/**
 * @brief Finds and prints the cheapest shop per client to the console.
 * @param db Pointer to the SQLite database connection.
 */
void FindCheapestShopPerClient(sqlite3 *db);

/**
 * @brief Modifies an existing order in the database.
 * @param db Pointer to the SQLite database connection.
 * @param order Pointer to an Order structure containing the updated order details.
 * @returns sqlite3 result code or -1 on error.
 */
int ModifyOrder(sqlite3 *db, Order *order);

/**
 * @brief Deletes an order from the database by its ID.
 * @param db Pointer to the SQLite database connection.
 * @param orderId The ID of the order to delete.
 * @returns sqlite3 result code or -1 on error.
 */
int DeleteOrder(sqlite3 *db, int orderId);

#endif // ORDERS_H