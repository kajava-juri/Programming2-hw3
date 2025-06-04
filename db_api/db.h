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

void FreeMemory(void **p);
void CreateOrder(sqlite3 *db);
void UpdateOrder(sqlite3 *db);

#endif // DB_H