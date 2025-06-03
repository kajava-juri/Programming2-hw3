#include <sqlite3.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "product.h"
#include "db.h"

void InitProductWrapper(GenericWrapper *wrapper) {
    wrapper->data = NULL;
    wrapper->freeData = FreeProduct;
    wrapper->getElementAt = GetProductAt;
    wrapper->size = 0;
    wrapper->used = 0;
    wrapper->limit = 0;
}

void FreeProduct(void **pProduct) {
    FreeMemory((void**)pProduct);
}

void *GetProductAt(void *pWrapper, size_t index) {
    GenericWrapper *wrapper = (GenericWrapper *)pWrapper;
    if (index >= wrapper->used) {
        return NULL; // Index out of bounds
    }
    Product *pProduct = (Product *)wrapper->data + index;
    return (void *) pProduct;
}

int GetProduct(sqlite3 *db, Product *product) {

    sqlite3_stmt *stmt;

    const char *sql = "SELECT id, name FROM products WHERE id = ?1 OR name LIKE '%' || ?2 || '%';";
    int rs;
    if ((rs = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL)) != SQLITE_OK) {
        // Error preparing statement
        fprintf(stderr, "Error preparing statement: %s\n", sqlite3_errmsg(db));
        return rs;
    }
    sqlite3_bind_int(stmt, 1, product->id);
    sqlite3_bind_text(stmt, 2, product->name, -1, SQLITE_STATIC);

    if((rs = sqlite3_step(stmt)) == SQLITE_ROW) {
        // Successfully retrieved a row
        product->id = sqlite3_column_int(stmt, 0);
        const unsigned char *name = sqlite3_column_text(stmt, 1);
        if (name) {
            strncpy(product->name, (const char*)name, sizeof(product->name) - 1);
            product->name[sizeof(product->name) - 1] = '\0'; // Ensure null termination
        } else {
            product->name[0] = '\0'; // Handle NULL case
        }
        sqlite3_finalize(stmt);
    }
    else if (rs == SQLITE_DONE) {
        // No rows found
        sqlite3_finalize(stmt);
    }
    else {
        fprintf(stderr, "Error executing statement: %s - %s\n", sqlite3_errstr(rs), sqlite3_errmsg(db));
    }

    return rs;
}

int PromptUserForProduct(sqlite3 *db, Product *product) {
    // First get the product name from the user
    char product_name[128];
    printf("Search for product by name: ");

    // Read exactly product_name size of bytes from user
    // fgets prevents buffer overflow
    fgets(product_name, sizeof(product_name), stdin);

    // Remove the trailing newline character if present
    size_t len = strlen(product_name);
    if (len > 0 && product_name[len - 1] == '\n')
    {
        product_name[len - 1] = '\0';
    }

    strncpy(product->name, product_name, sizeof(product->name) - 1);

    int rs;
    // Ideally GetProduct would return SQLITE_DONE if all the rows were processed
    // because SQLITE_ROW means that there are more rows that were found
    if((rs = GetProduct(db, product)) & (SQLITE_ROW | SQLITE_DONE)) {
        printf("Product found: ID = %d, Name = %s\n", product->id, product->name);
    } else {
        printf("%s - searched for product '%s'\n", sqlite3_errstr(rs), product->name);
    }

    return rs;
}