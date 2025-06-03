#ifndef PRODUCT_H
#define PRODUCT_H

#include <sqlite3.h>
#include "db.h"

typedef struct {
    int id;          // Unique identifier for the product
    char name[128];  // Name of the product
} Product;

/**
 * @brief Retrieves a product from the database.
 * @param db Pointer to the SQLite database connection.
 * @param product Pointer to a Product structure that contains values to search for
 * and retrieved data will be stored in the same structure.
 * 
 * @returns sqlite3 result code
 * 
 *  - SQLITE_ROW if product is found,
 * 
 *  - SQLITE_DONE if no product is found but query executed successfully,
 * 
 *  - or an error code if the query failed.
 */
int GetProduct(sqlite3 *db, Product *product);
int PromptUserForProduct(sqlite3 *db, Product *product);
void FreeProduct(void **pProduct);
void *GetProductAt(void *pWrapper, size_t index);
void InitProductWrapper(GenericWrapper *wrapper);
void FreeProduct(void **pProduct);

#endif // PRODUCT_H