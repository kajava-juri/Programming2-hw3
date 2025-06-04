#include <stdlib.h>
#include <stdio.h>
#include <sqlite3.h>
#include <string.h>
#include "db_api/db.h"
#include "main.h"
#include "db_api/product.h"
#include "db_api/orders.h"
#include "main.h"
#include "menu.h"

int main(void)
{

    sqlite3 *db = NULL;
    db_init(&db);

    int option;
    // Get menu selection and check if it's not 0
    while ((option = GetMenuSelection()) != 0)
    {
        switch (option) // Use option as the switch expression
        {
        case 1:
            // Product product = {
            //     .id = 0, // Assuming 0 means no specific ID
            //     .name = {0}};
            // GenericWrapper pw;
            // InitProductWrapper(&pw);
            // PromptUserForProduct(db, &pw, &product);
            CreateOrder(db);
            break;

        case 2:

            break;

        case 3:

            break;

        default:

            break;
        }
    }

    // Product product = {
    //     .id = 1,
    //     . name = NULL
    // };

    // GetProduct(db, &product);
    // if (product.name[0] != '\0') {
    //     printf("Product found: ID = %d, Name = %s\n", product.id, product.name);
    // } else {
    //     printf("No product found with ID = %d\n", product.id);
    // }
    // sqlite3_close(db);
    // printf("Database connection closed.\n");

    return 0;
}

char *strdup(const char *src) {
    char *dst = malloc(strlen (src) + 1);  // Space for length plus nul
    if (dst == NULL) return NULL;          // No memory
    strcpy(dst, src);                      // Copy the characters
    return dst;                            // Return the new string
}