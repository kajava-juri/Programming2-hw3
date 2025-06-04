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
            CreateOrder(db);
            break;

        case 2:
            UpdateOrder(db);
            break;

        case 3:
            Order order = {0};
            PromptUserForOrder(db, &order);
            int rs = DeleteOrder(db, order.id);
            if(rs & (SQLITE_OK | SQLITE_DONE))
            {
                printf("Order with ID %d deleted successfully.\n", order.id);
            }
            
            break;
        case 4:
            PrintOrdersGroupedByClient(db);
            break;
        case 5:
            PrintAllOrdersByClientOrderCount(db);
            break;
        case 6:
            PrintCheapestOffersForAllClientOrders(db);
            break;
        case 7:
            FindCheapestShopPerClient(db);
            break;
        default:

            break;
        }
    }

    sqlite3_close(db); // Close the database connection

    return 0;
}

char *strdup(const char *src) {
    char *dst = malloc(strlen (src) + 1);  // Space for length plus nul
    if (dst == NULL) return NULL;          // No memory
    strcpy(dst, src);                      // Copy the characters
    return dst;                            // Return the new string
}