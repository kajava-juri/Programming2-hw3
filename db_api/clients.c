#include <sqlite3.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "clients.h"
#include "db.h"
#include "../main.h"

void InitClientWrapper(GenericWrapper *wrapper)
{
    wrapper->data = NULL;
    wrapper->freeData = FreeClient;
    wrapper->getElementAt = GetClientAt;
    wrapper->size = 0;
    wrapper->used = 0;
    wrapper->limit = 0;
}

void FreeClient(void *pClient)
{
    Client *client = (Client *)pClient;
    if (client == NULL)
    {
        return; // Nothing to free
    }
    FreeMemory((void **)&client->first_name);
    FreeMemory((void **)&client->last_name);
}

void *GetClientAt(void *pWrapper, size_t index)
{
    GenericWrapper *wrapper = (GenericWrapper *)pWrapper;
    if (index >= wrapper->used)
    {
        return NULL; // Index out of bounds
    }
    Client *pClient = (Client *)wrapper->data + index;
    return (void *)pClient;
}

int GetClient(sqlite3 *db, Client *client)
{

    sqlite3_stmt *stmt;

    const char *sql = "SELECT id, first_name, last_name FROM clients WHERE id = ?1 OR first_name LIKE '%' || ?2 || '%' OR last_name LIKE '%' || ?3 || '%';";
    int rs;
    if ((rs = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL)) != SQLITE_OK)
    {
        // Error preparing statement
        fprintf(stderr, "Error preparing statement: %s\n", sqlite3_errmsg(db));
        return rs;
    }
    sqlite3_bind_int(stmt, 1, client->id);
    sqlite3_bind_text(stmt, 2, client->first_name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, client->last_name, -1, SQLITE_STATIC);

    if ((rs = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        // Successfully retrieved a row
        client->id = sqlite3_column_int(stmt, 0);
        const unsigned char *firstName = sqlite3_column_text(stmt, 1);
        const unsigned char *lastName = sqlite3_column_text(stmt, 2);
        if (firstName)
        {
            // strncpy(product->name, (const char *)name, sizeof(product->name) - 1);
            client->first_name = strdup((const char *)firstName);
            client->first_name[sizeof(client->first_name) - 1] = '\0'; // Ensure null termination
        }
        else
        {
            client->first_name[0] = '\0'; // Handle NULL case
        }

        if (lastName)
        {
            client->last_name = strdup((const char *)lastName);
            client->last_name[sizeof(client->last_name) - 1] = '\0'; // Ensure null termination
        }
        else
        {
            client->last_name[0] = '\0'; // Handle NULL case
        }

        sqlite3_finalize(stmt);
    }
    else if (rs == SQLITE_DONE)
    {
        // No rows found
        sqlite3_finalize(stmt);
    }
    else
    {
        fprintf(stderr, "Error executing statement: %s - %s\n", sqlite3_errstr(rs), sqlite3_errmsg(db));
    }

    return rs;
}

int GetClientById(sqlite3 *db, int clientId, Client *client)
{
    if (client == NULL)
    {
        fprintf(stderr, "Product pointer is NULL.\n");
        return SQLITE_MISUSE; // Return an error code for misuse
    }

    client->id = clientId;        // Set the ID to search for
    client->first_name[0] = '\0'; // Initialize name to an empty string
    client->last_name[0] = '\0';

    int rs;
    const char *sql = "SELECT id, first_name, last_name clients products WHERE id = ?1;";
    sqlite3_stmt *stmt;
    // Prepare the SQL statement to select a client by ID
    if ((rs = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL)) != SQLITE_OK)
    {
        // Error preparing statement
        fprintf(stderr, "Error preparing statement: %s\n", sqlite3_errmsg(db));
        return rs;
    }
    sqlite3_bind_int(stmt, 1, clientId);
    if ((rs = sqlite3_step(stmt)) & (SQLITE_ROW | SQLITE_DONE))
    {
        int id = sqlite3_column_int(stmt, 0);
        if (id == clientId)
        {
            // Successfully retrieved a row
            client->id = id;
            const unsigned char *firstName = sqlite3_column_text(stmt, 1);
            const unsigned char *lastName = sqlite3_column_text(stmt, 2);
            // Set first name
            if (firstName)
            {
                client->first_name = strdup((const char *)firstName);
                client->first_name[sizeof(client->first_name) - 1] = '\0'; // Ensure null termination
            }
            else
            {
                client->first_name[0] = '\0'; // Handle NULL case
            }

            // Set last name
            if (lastName)
            {
                client->last_name = strdup((const char *)lastName);
                client->last_name[sizeof(client->last_name) - 1] = '\0'; // Ensure null termination
            }
            else
            {
                client->last_name[0] = '\0'; // Handle NULL case
            }
        }
        else
        {
            fprintf(stderr, "Client with ID %d not found.\n", clientId);
        }
        sqlite3_finalize(stmt);
    }

    return rs;
}

int GetMatchedClients(sqlite3 *db, Client *searchClient, GenericWrapper *clientWrapper)
{
    sqlite3_stmt *stmt;

    const char *sql = "SELECT id, first_name, last_name FROM clients WHERE id = ?1 OR first_name LIKE '%' || ?2 || '%' OR last_name LIKE '%' || ?3 || '%';";
    // For readability, define indices for the columns, not sure if there is a better way to do this
    const int idIdx = 0;
    const int firstNameIdx = 1;
    const int lastNameIdx = 2;

    int rs;
    if ((rs = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL)) != SQLITE_OK)
    {
        // Error preparing statement
        fprintf(stderr, "Error preparing statement: %s\n", sqlite3_errmsg(db));
        return rs;
    }
    sqlite3_bind_int(stmt, 1, searchClient->id);
    sqlite3_bind_text(stmt, 2, searchClient->first_name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, searchClient->last_name, -1, SQLITE_STATIC);

    int count = 0;
    int allocated = 4; // Initial allocation size
    Client *clients = malloc(allocated * sizeof(Client));
    Client *tempClient = NULL;
    if (!clients)
    {
        fprintf(stderr, "Memory allocation failed.\n");
        sqlite3_finalize(stmt);
        exit(EXIT_FAILURE);
    }

    while ((rs = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        if (count >= allocated)
        {
            // Resize the array if needed
            allocated *= 2;
            tempClient = realloc(clients, allocated * sizeof(Client));
            if (!tempClient)
            {
                fprintf(stderr, "Memory reallocation failed.\n");
                // Free all the products in array before exiting
                for (int i = 0; i < count; i++)
                {
                    FreeClient((void **)&clients[i]);
                }
                // then free the array itself
                FreeMemory((void **)&clients);
                sqlite3_finalize(stmt);
                exit(EXIT_FAILURE);
            }

            clients = tempClient;
        }

        // Successfully retrieved a row
        (clients + count)->id = sqlite3_column_int(stmt, idIdx);
        const unsigned char *lastName = sqlite3_column_text(stmt, lastNameIdx);
        const unsigned char *firstName = sqlite3_column_text(stmt, firstNameIdx);
        if (firstName)
        {
            // strncpy((products + count)->name, (const char *)name, sizeof((products + count)->name) - 1);
            (clients + count)->first_name = strdup((const char *)firstName);
            // null termination?
        }
        else
        {
            // Handle NULL case
            (clients + count)->first_name = NULL;
        }

        if(lastName) 
        {
            (clients + count)->last_name = strdup((const char *)lastName);
        }
        else
        {
            // Handle NULL case
            (clients + count)->last_name = NULL;
        }

        count++;
    }

    clientWrapper->data = clients;
    clientWrapper->used = count;
    clientWrapper->limit = allocated;
    clientWrapper->size = sizeof(Client);

    if (rs == SQLITE_DONE)
    {
        // No rows found
        sqlite3_finalize(stmt);
    }
    else
    {
        fprintf(stderr, "Error executing statement: %s - %s\n", sqlite3_errstr(rs), sqlite3_errmsg(db));
    }

    return rs;
}

int PromptUserForClient(sqlite3 *db, Client **outClient)
{
    GenericWrapper *clientWrapper = malloc(sizeof(GenericWrapper));
    if(clientWrapper == NULL)
    {
        fprintf(stderr, "Memory allocation failed for client wrapper.\n");
        exit(EXIT_FAILURE);
    }
    InitClientWrapper(clientWrapper);
    // First get the product name from the user
    char firstName[128];
    char lastName[128];
    printf("Enter name of client to search for (separate by space if seaching for both first and last name):\n>> ");
    // Read user input and determine if it contains single word or two words
    // If single word then set first name and last name to the same value
    fgets(firstName, sizeof(firstName), stdin);
    // Remove the trailing newline character if present
    size_t len = strlen(firstName);
    if (len > 0 && firstName[len - 1] == '\n')
    {
        firstName[len - 1] = '\0';
    }

    // Check if the input contains a space
    char *spacePos = strchr(firstName, ' ');
    if(spacePos != NULL)
    {
        // Split the input into first name and last name
        *spacePos = '\0'; // Replace space with null terminator
        strncpy(lastName, spacePos + 1, sizeof(lastName) - 1);
        lastName[sizeof(lastName) - 1] = '\0'; // Ensure null termination
    }
    else
    {
        // If no space, set last name to same as first name
        strncpy(lastName, firstName, sizeof(lastName) - 1);
        lastName[sizeof(lastName) - 1] = '\0'; // Ensure null termination
        firstName[sizeof(firstName) - 1] = '\0'; // Ensure null termination
    }

    

    Client client = {
        .id = 0,
        .first_name = NULL,
        .last_name = NULL
    };
    // strncpy(product.name, product_name, sizeof(product.name) - 1);
    client.first_name = strdup(firstName);
    client.last_name = strdup(lastName);
    if (client.first_name == NULL || client.last_name == NULL)
    {
        fprintf(stderr, "Memory allocation failed for client names.\n");
        exit(EXIT_FAILURE);
    }

    int rs;
    // Get matched products in generic wrapper
    // do not forget to free the wrapper after use
    if ((rs = GetMatchedClients(db, &client, clientWrapper)) & (SQLITE_ROW | SQLITE_DONE))
    {
        // TODO: limit for the number of clients to display and ask the user to narrow down the search if there are too many results
        for (size_t i = 0; i < clientWrapper->used; i++)
        {
            Client *pClient = (Client *)clientWrapper->getElementAt(clientWrapper, i);
            PrintClient(pClient);
        }
        printf("Found %zu clients matching '%s %s':\n", clientWrapper->used, client.first_name, client.last_name);
        printf("Type ID of the client you want to select or 0 to cancel: ");
        int clientId;
        // Read the product ID from user input
        scanf("%d", &clientId);
        // Clear the input buffer
        while (getchar() != '\n' && getchar() != EOF);
        if (clientId == 0)
        {
            printf("Client selection cancelled.\n");
            FreeClient(&client); // Free the client names
            FreeWrapper(clientWrapper); // Free the wrapper after use
            FreeMemory((void **)&clientWrapper);
            return 0; // User cancelled the selection
        }

        // Check if picked ID is in fetched clients and set it to outClient
        for (size_t i = 0; i < clientWrapper->used; i++)
        {
            Client *pClient = (Client *)clientWrapper->getElementAt(clientWrapper, i);
            if (pClient->id == clientId)
            {
                // Set the output product to the found one
                Client *newPClient = (Client *)malloc(sizeof(Client));
                if (newPClient == NULL)
                {
                    fprintf(stderr, "Memory allocation failed.\n");
                    exit(EXIT_FAILURE);
                }
                // Copy the found product data
                newPClient->id = pClient->id;
                newPClient->first_name = strdup(pClient->first_name);
                newPClient->last_name = strdup(pClient->last_name);
                if (newPClient->first_name == NULL || newPClient->last_name == NULL)
                {
                    fprintf(stderr, "Memory allocation failed for client names.\n");
                    exit(EXIT_FAILURE);
                }

                *outClient = newPClient; // Set the output product to the selected one
                printf("Selected Client: ");
                PrintClient(newPClient);
                FreeWrapper(clientWrapper); // Free the wrapper after use
                FreeMemory((void **)&clientWrapper); // Free the client names
                FreeClient(&client); // Free the client names
                return 1; // Successfully selected a product
            }
        }

        // ====================================
        // Note: after this point instead of returning, set 'rs' to the desired value so that it will free the allocated memory in the end
        // ====================================

        // If we reach here, it means the client ID was not found in the fetched products
        // promt the user if he wants to search the database
        printf("Client with ID %d not found in the fetched clients.\n", clientId);
        printf("Do you want to search the database for this product? (y/n): ");
        char choice;
        scanf(" %c", &choice);
        // Clear the input buffer
        while (getchar() != '\n' && getchar() != EOF);
        if (tolower(choice) == 'n')
        {
            printf("Client selection cancelled.\n");
            rs = 0; // User cancelled the selection
        }
        else if (tolower(choice) == 'y')
        {
            printf("Searching for client with ID %d in the database...\n", clientId);
            rs = GetClientById(db, clientId, &client);
            if (rs == SQLITE_ROW)
            {
                // Set the output product to the found one
                Client *newPClient = (Client *)malloc(sizeof(Client));
                if (newPClient == NULL)
                {
                    fprintf(stderr, "Memory allocation failed.\n");
                    exit(EXIT_FAILURE);
                }
                newPClient->id = client.id;
                newPClient->first_name = strdup(client.first_name);
                newPClient->last_name = strdup(client.last_name);
                if (newPClient->first_name == NULL || newPClient->last_name == NULL)
                {
                    fprintf(stderr, "Memory allocation failed for client names.\n");
                    exit(EXIT_FAILURE);
                }

                *outClient = newPClient;
                printf("Found client: ");
                PrintClient(&client);
                rs = 1; // Successfully selected a product
            }
            else if (rs == SQLITE_DONE)
            {
                printf("No client found with ID %d.\n", clientId);
            }
            else
            {
                fprintf(stderr, "Error searching for client (%d): %s\n", rs, sqlite3_errstr(rs));
            }
        }
    }
    else
    {
        printf("%s - searched for client '%s %s'\n", sqlite3_errstr(rs), client.first_name, client.last_name);
    }

    FreeWrapper(clientWrapper); // Free the wrapper after use
    FreeMemory((void **)&clientWrapper);
    FreeClient(&client); // Free the client names
    return rs;
}

void PrintClient(Client *client)
{
    if (client == NULL)
    {
        printf("No client data available.\n");
        return;
    }
    printf("Client ID: %d, Name: %s %s\n", client->id, client->first_name, client->last_name);
}