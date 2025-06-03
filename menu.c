#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "menu.h"
#include "db_api/product.h"
#include "db_api/orders.h"
#include "db_api/db.h"

void DisplayMenu() {
    printf("\n\nMenu:\n");
    printf("1. Create order\n");
    printf("3. Modify order\n");
    printf("4. Delete order\n");
    printf("5. Print orders grouped by clients\n");
    printf("0. Exit\n");

}

int GetMenuSelection() {
    DisplayMenu();
    
    int menuOption;
    do
    {
        printf("  Select an option (1-...): ");
        scanf("%d", &menuOption);
        if (menuOption < 0 || menuOption > 5)
        {
            printf("  Invalid option. Please select a number between 1 and ... .\n");
            // Clear the input buffer
            while(getchar() != '\n' && getchar() != EOF);
        }
    }
    while (menuOption < 0 || menuOption > 5);
    
    if(menuOption == 0)
    {
        printf("\n==========================================================\n");
        printf("                        E X I T I N G                     \n");
        printf("==========================================================\n\n");
        return 0;
    }

    // Clear the input buffer
    while (getchar() != '\n');

    return menuOption;
}