#include <sqlite3.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "db.h"
#include "orders.h"
#include "product.h"

void db_init(sqlite3 **pdb)
{
    int conn = sqlite3_open_v2("shop2.db", pdb, SQLITE_OPEN_READWRITE, NULL);
    if (conn != SQLITE_OK)
    {
        fprintf(stderr, "Error opening database: %s\n", sqlite3_errmsg(*pdb));
        sqlite3_close(*pdb);
        exit(EXIT_FAILURE);
    }

    // If database could not be opened in read/write mode, it tries to open it in read-only mode
    // we do not want that so check if it is opened in read/write mode
    if (sqlite3_db_readonly(*pdb, "main") != 0)
    {
        fprintf(stderr, "Could not open database in read/write mode: %s\n", sqlite3_errmsg(*pdb));
        sqlite3_close(*pdb);
        exit(EXIT_FAILURE);
    }

    // Test db connection
    char buffer[256] = {0};
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(*pdb, "PRAGMA database_list;", -1, &stmt, NULL) == SQLITE_OK)
    {
        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            const unsigned char *text = sqlite3_column_text(stmt, 1);
            if (text)
                strcpy(buffer, (const char *)text);
        }
        int rs = sqlite3_finalize(stmt);
        if (rs != SQLITE_OK)
        {
            fprintf(stderr, "Error finalizing statement: %s\n", sqlite3_errstr(rs));
            sqlite3_close(*pdb);
            exit(EXIT_FAILURE);
        }
    }
    if (sqlite3_errcode(*pdb) != SQLITE_OK)
    {
        fprintf(stderr, "Error executing test query: %s\n", sqlite3_errmsg(*pdb));
        sqlite3_close(*pdb);
        exit(EXIT_FAILURE);
    }
    printf("Database opened successfully in read/write mode.\n");
    printf("Database name: '%s'\n", buffer);
}

void CreateOrder(sqlite3 *db)
{
    // Initialize product
    Product product = {
        .id = 0, // Assuming 0 means no specific ID
        .name = NULL};

    Product *product_ptr = &product;
    int product_res = PromptUserForProduct(db, &product_ptr);
    if (product_res == 0)
    {
        return;
    }
    else if (product_res < 0)
    {
        fprintf(stderr, "Error retrieving product: %d\n", product_res);
        return;
    }
    else if(product_res != 1)
    {
        fprintf(stderr, "Product selection returned non-succesful result: %s >> %s\n", sqlite3_errstr(product_res), sqlite3_errmsg(db));
    }

    printf("DEBUG>>> "); PrintProduct(product_ptr);
}

/**
 * @brief Frees resources associated with a wrapper object.
 *
 * This function deallocates memory and releases any resources that were
 * allocated by the wrapper.
 *
 * @param wrapper Pointer to the wrapper object to be freed.
 * @return void
 */
void FreeWrapper(GenericWrapper *wrapper)
{
    if (wrapper == NULL)
    {
        return;
    }
    if (wrapper->data != NULL)
    {
        if (wrapper->freeData != NULL)
        {
            for (size_t i = 0; i < wrapper->used; i++)
            {
                // get the data pointer at index i for the unknown type
                void *pData = wrapper->getElementAt(wrapper, i);
                // continue if pData is NULL
                if (pData == NULL)
                {
                    continue;
                }

                // call the struct specific memory deallocation function
                wrapper->freeData(pData);
            }
        }
        // free the data pointer itself
        FreeMemory(&wrapper->data);
    }
    wrapper->size = 0;
    wrapper->used = 0;
    wrapper->limit = 0;
}

void FreeMemory(void **p)
{
    if (*p)
    {
        free(*p);
        *p = NULL;
    }
}