#ifndef PRODUCT_H
#define PRODUCT_H

#include <sqlite3.h>
#include "db.h"

typedef struct {
    int id;          // Unique identifier for the product
    char *name;  // Name of the product
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

/**
 * @brief Convenience function to retrieve a product by only its ID
 * @param db Pointer to the SQLite database connection.
 * @param productId The ID of the product to retrieve.
 * @param product Pointer to a Product structure that will be filled with the retrieved data.
 */
int GetProductById(sqlite3 *db, int productId, Product *product);
int GetMatchedProducts(sqlite3 *db, Product *searchProduct, GenericWrapper *productWrapper);

/**
 * @brief Prompts the user for product details and retrieves matching products from the database for user to select.
 * @param db Pointer to the SQLite database connection.
 * @param outProduct Pointer to a Product structure where the selected product will be stored.
 * 
 * @returns 1 - if user succesfully selected, 0 - user cancelled, -1 - error or sqlite3 status code.
 */
int PromptUserForProduct(sqlite3 *db, Product **outProduct);
void FreeProduct(void *pProduct);
void *GetProductAt(void *pWrapper, size_t index);
void InitProductWrapper(GenericWrapper *wrapper);
void PrintProduct(Product *product);

#endif // PRODUCT_H