#include <sqlite3.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "db.h"
#include "orders.h"
#include "product.h"
#include "clients.h"

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
    // This is not a great solution, because the product_ptr will be overwritten with allocated memory
    // TODO: fix product prompting to just take pointer to allocated memory, then reallocate instead
    Product *product_ptr = &product;
    int product_res = PromptUserForProduct(db, &product_ptr);
    if (product_res == 0)
    {
        FreeProduct(product_ptr);
        FreeMemory((void **)&product_ptr);
        return;
    }
    else if (product_res < 0)
    {
        FreeProduct(product_ptr);
        FreeMemory((void **)&product_ptr);
        fprintf(stderr, "Error retrieving product: %d\n", product_res);
        return;
    }
    else if(product_res != 1)
    {
        fprintf(stderr, "Product selection returned non-succesful result: %s >> %s\n", sqlite3_errstr(product_res), sqlite3_errmsg(db));
    }

    // Initialize client
    Client client = {
        .id = 0, // Assuming 0 means no specific ID
        .first_name = NULL,
        .last_name = NULL};
    Client *client_ptr = &client;
    // Note: client_ptr will be overwritten with allocated memory
    // TODO: allocate memory here instead, idk why I did it like this initally
    int client_res = PromptUserForClient(db, &client_ptr);
    if (client_res == 0)
    {
        FreeProduct(product_ptr);
        FreeMemory((void **)&product_ptr);
        FreeClient(client_ptr);
        FreeMemory((void **)&client_ptr);
        return;
    }
    else if (client_res < 0)
    {
        fprintf(stderr, "Error retrieving client: %d\n", client_res);
        FreeProduct(product_ptr);
        FreeMemory((void **)&product_ptr);
        FreeClient(client_ptr);
        FreeMemory((void **)&client_ptr);
        return;
    }
    else if(client_res != 1)
    {
        fprintf(stderr, "Client selection returned non-succesful result: %s >> %s\n", sqlite3_errstr(client_res), sqlite3_errmsg(db));
    }

    // Insert order
    Order order = {
        .id = 0, // Will be set by the database
        .client_id = client_ptr->id,
        .product_id = product_ptr->id,
        .amount = 1 // Default amount, can be modified later
    };
    int order_res = InsertOrder(db, &order);
    if(order_res != SQLITE_DONE)
    {
        fprintf(stderr, "Error inserting order: %s >> %s\n", sqlite3_errstr(order_res), sqlite3_errmsg(db));
    }
    else
    {
        printf("Order created successfully with ID: %d\n", order.id);
    }
    // Free resources
    FreeProduct(product_ptr);
    FreeMemory((void **)&product_ptr);
    FreeClient(client_ptr);
    FreeMemory((void **)&client_ptr);
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