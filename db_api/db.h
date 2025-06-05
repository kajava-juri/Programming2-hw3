#ifndef DB_H
#define DB_H

#include <sqlite3.h>

typedef struct {
    void *data;

    // These function must be implemented in the respective data types
    void (*freeData)(void *); // a function provided that frees the data
    void *(*getElementAt)(void *, size_t); // a function provided that gets the element at index

    size_t size;
    size_t used;
    size_t limit;
} GenericWrapper;

/**
 * @brief Initializes the SQLite database connection.
 * @param pdb Pointer to a pointer that will hold the database connection.
 */
void db_init(sqlite3 **pdb);

/**
 * @brief Frees resources associated with a wrapper object.
 *
 * This function deallocates memory and releases any resources that were
 * allocated by the wrapper.
 *
 * @param wrapper Pointer to the wrapper object to be freed.
 * @return void
 */
void FreeWrapper(GenericWrapper *wrapper);

/**
 * @brief Frees memory pointed to by a pointer and sets the pointer to NULL.
 * @param p Pointer to a pointer that will be freed and set to NULL.
 */
void FreeMemory(void **p);

/**
 * @brief Creates a new order by prompting the user and inserting it into the database.
 * @param db Pointer to the SQLite database connection.
 */
void CreateOrder(sqlite3 *db);

/**
 * @brief Updates an existing order by prompting the user for modifications.
 * @param db Pointer to the SQLite database connection.
 */
void UpdateOrder(sqlite3 *db);

#endif // DB_H