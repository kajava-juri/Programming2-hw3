#ifndef CLIENTS_H
#define CLIENTS_H

#include <sqlite3.h>
#include "db.h"
#include <inttypes.h>
#include <stdlib.h>

typedef struct {
    int id;        
    char *first_name;
    char *last_name;
} Client;

/**
 * @brief Retrieves a client from the database.
 * @param db Pointer to the SQLite database connection.
 * @param client Pointer to a Client structure that contains values to search for
 * and retrieved data will be stored in the same structure.
 * 
 * @returns sqlite3 result code
 * 
 *  - SQLITE_ROW if client is found,
 * 
 *  - SQLITE_DONE if no client is found but query executed successfully,
 * 
 *  - or an error code if the query failed.
 */
int GetClient(sqlite3 *db, Client *client);

/**
 * @brief Convenience function to retrieve a client by only its ID
 * @param db Pointer to the SQLite database connection.
 * @param clientId The ID of the client to retrieve.
 * @param client Pointer to a Client structure that will be filled with the retrieved data.
 */
int GetClientById(sqlite3 *db, int clientId, Client *client);
int GetMatchedClients(sqlite3 *db, Client *searchClient, GenericWrapper *clientWrapper);

/**
 * @brief Prompts the user for client details and retrieves matching products from the database for user to select.
 * @param db Pointer to the SQLite database connection.
 * @param outClient Pointer to a Client structure where the selected product will be stored.
 * 
 * @returns 1 - if user succesfully selected, 0 - user cancelled, -1 - error or sqlite3 status code.
 */
int PromptUserForClient(sqlite3 *db, Client **outClient);
void FreeClient(void *pClient);
void *GetClientAt(void *pWrapper, size_t index);
void InitClientWrapper(GenericWrapper *wrapper);
void PrintClient(Client *client);

#endif // CLIENTS_H