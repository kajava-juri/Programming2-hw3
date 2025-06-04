#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "menu.h"
#include "db_api/product.h"
#include "db_api/orders.h"
#include "db_api/db.h"

void DisplayMenu()
{
    printf("\n\nMenu:\n");
    printf("1. Create order\n");
    printf("2. Modify order\n");
    printf("3. Delete order\n");
    printf("4. Print orders grouped by clients\n");
    printf("5. Print clients by order count\n");
    printf("6. Print clients' orders with cheapest offer\n");
    printf("7. Find cheapest shop per client\n");
    printf("0. Exit\n");
}

int GetMenuSelection()
{
    DisplayMenu();

    int menuOption;
    int maxOption = 7; // Maximum option number
    do
    {
        printf("  Select an option (1-...): ");
        scanf("%d", &menuOption);
        if (menuOption < 0 || menuOption > maxOption)
        {
            printf("  Invalid option. Please select a number between 1 and ... .\n");
            // Clear the input buffer
            while (getchar() != '\n' && getchar() != EOF)
                ;
        }
    } while (menuOption < 0 || menuOption > maxOption);

    if (menuOption == 0)
    {
        printf("\n==========================================================\n");
        printf("                        E X I T I N G                     \n");
        printf("==========================================================\n\n");
        return 0;
    }

    // Clear the input buffer
    while (getchar() != '\n')
        ;

    return menuOption;
}