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

/**
 * @brief Retrieves products that match the search criteria from the database.
 * @param db Pointer to the SQLite database connection.
 * @param searchProduct Pointer to a Product structure containing search criteria.
 * @param productWrapper Pointer to a GenericWrapper structure where matched products will be stored.
 * 
 * @returns sqlite3 result code or -1 on error.
 */
int GetMatchedProducts(sqlite3 *db, Product *searchProduct, GenericWrapper *productWrapper);

/**
 * @brief Prompts the user for product details and retrieves matching products from the database for user to select.
 * @param db Pointer to the SQLite database connection.
 * @param outProduct Pointer to a Product structure where the selected product will be stored.
 * 
 * @returns 1 - if user succesfully selected, 0 - user cancelled, -1 - error or sqlite3 status code.
 */
int PromptUserForProduct(sqlite3 *db, Product **outProduct);

/**
 * @brief Frees memory allocated for a Product structure.
 * @param pProduct Pointer to the Product structure to be freed.
 */
void FreeProduct(void *pProduct);

/**
 * @brief Gets a product at the specified index from a wrapper.
 * @param pWrapper Pointer to the wrapper containing products.
 * @param index The index of the product to retrieve.
 * @returns Pointer to the Product at the specified index, or NULL if index is out of bounds.
 */
void *GetProductAt(void *pWrapper, size_t index);

/**
 * @brief Initializes a GenericWrapper for handling Product structures.
 * @param wrapper Pointer to the GenericWrapper to initialize.
 */
void InitProductWrapper(GenericWrapper *wrapper);

/**
 * @brief Prints the details of a single product to the console.
 * @param product Pointer to the Product structure to print.
 */
void PrintProduct(Product *product);

#endif // PRODUCT_H