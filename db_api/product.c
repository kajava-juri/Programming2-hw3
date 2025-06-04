#include <sqlite3.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "product.h"
#include "db.h"
#include "../main.h"

void InitProductWrapper(GenericWrapper *wrapper)
{
    wrapper->data = NULL;
    wrapper->freeData = FreeProduct;
    wrapper->getElementAt = GetProductAt;
    wrapper->size = 0;
    wrapper->used = 0;
    wrapper->limit = 0;
}

void FreeProduct(void *pProduct)
{
    Product *product = (Product *)pProduct;
    if (product == NULL)
    {
        return; // Nothing to free
    }
    FreeMemory((void **)&product->name); // Free the name string
}

void *GetProductAt(void *pWrapper, size_t index)
{
    GenericWrapper *wrapper = (GenericWrapper *)pWrapper;
    if (index >= wrapper->used)
    {
        return NULL; // Index out of bounds
    }
    Product *pProduct = (Product *)wrapper->data + index;
    return (void *)pProduct;
}

int GetProduct(sqlite3 *db, Product *product)
{

    sqlite3_stmt *stmt;

    const char *sql = "SELECT id, name FROM products WHERE id = ?1 OR name LIKE '%' || ?2 || '%';";
    int rs;
    if ((rs = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL)) != SQLITE_OK)
    {
        // Error preparing statement
        fprintf(stderr, "Error preparing statement: %s\n", sqlite3_errmsg(db));
        return rs;
    }
    sqlite3_bind_int(stmt, 1, product->id);
    sqlite3_bind_text(stmt, 2, product->name, -1, SQLITE_STATIC);

    if ((rs = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        // Successfully retrieved a row
        product->id = sqlite3_column_int(stmt, 0);
        const unsigned char *name = sqlite3_column_text(stmt, 1);
        if (name)
        {
            // strncpy(product->name, (const char *)name, sizeof(product->name) - 1);
            product->name = strdup((const char *)name);
            product->name[sizeof(product->name) - 1] = '\0'; // Ensure null termination
        }
        else
        {
            product->name[0] = '\0'; // Handle NULL case
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

int GetProductById(sqlite3 *db, int productId, Product *product)
{
    if (product == NULL)
    {
        fprintf(stderr, "Product pointer is NULL.\n");
        return SQLITE_MISUSE; // Return an error code for misuse
    }

    product->id = productId; // Set the ID to search for
    product->name[0] = '\0'; // Initialize name to an empty string

    int rs;
    const char *sql = "SELECT id, name FROM products WHERE id = ?1;";
    sqlite3_stmt *stmt;
    // Prepare the SQL statement to select a product by ID
    if ((rs = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL)) != SQLITE_OK)
    {
        // Error preparing statement
        fprintf(stderr, "Error preparing statement: %s\n", sqlite3_errmsg(db));
        return rs;
    }
    sqlite3_bind_int(stmt, 1, productId);
    if ((rs = sqlite3_step(stmt)) & (SQLITE_ROW | SQLITE_DONE))
    {
        int id = sqlite3_column_int(stmt, 0);
        if (id == productId)
        {
            // Successfully retrieved a row
            product->id = id;
            const unsigned char *name = sqlite3_column_text(stmt, 1);
            if (name)
            {
                // strncpy(product->name, (const char *)name, sizeof(product->name) - 1);
                product->name = strdup((const char *)name);
            }
            else
            {
                product->name[0] = '\0'; // Handle NULL case
            }
        }
        else
        {
            fprintf(stderr, "Product with ID %d not found.\n", productId);
        }
        sqlite3_finalize(stmt);
    }

    return rs;
}

int GetMatchedProducts(sqlite3 *db, Product *searchProduct, GenericWrapper *productWrapper)
{
    sqlite3_stmt *stmt;

    const char *sql = "SELECT id, name FROM products WHERE id = ?1 OR name LIKE '%' || ?2 || '%';";
    // For readability, define indices for the columns, not sure if there is a better way to do this
    const int idIdx = 0;
    const int nameIdx = 1;

    int rs;
    if ((rs = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL)) != SQLITE_OK)
    {
        // Error preparing statement
        fprintf(stderr, "Error preparing statement: %s\n", sqlite3_errmsg(db));
        PrintProduct(searchProduct);
        return rs;
    }
    sqlite3_bind_int(stmt, 1, searchProduct->id);
    sqlite3_bind_text(stmt, 2, searchProduct->name, -1, SQLITE_STATIC);

    int count = 0;
    int allocated = 4; // Initial allocation size
    Product *products = malloc(allocated * sizeof(Product));
    if (!products)
    {
        fprintf(stderr, "Memory allocation failed.\n");
        sqlite3_finalize(stmt);
        exit(EXIT_FAILURE);
    }
    Product *tempProduct = NULL;

    while ((rs = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        if (count >= allocated)
        {
            // Resize the array if needed
            allocated *= 2;
            tempProduct = realloc(products, allocated * sizeof(Product));
            if (!tempProduct)
            {
                fprintf(stderr, "Memory reallocation failed.\n");
                // Free all the products in array before exiting
                for (int i = 0; i < count; i++)
                {
                    FreeProduct((void **)&products[i]);
                }
                FreeMemory((void **)&products);
                sqlite3_finalize(stmt);
                exit(EXIT_FAILURE);
            }

            products = tempProduct;
        }

        // Successfully retrieved a row
        (products + count)->id = sqlite3_column_int(stmt, idIdx);
        const unsigned char *name = sqlite3_column_text(stmt, nameIdx);
        if (name)
        {
            // strncpy((products + count)->name, (const char *)name, sizeof((products + count)->name) - 1);
            (products + count)->name = strdup((const char *)name);
            // null termination?
        }
        else
        {
            // Handle NULL case
            (products + count)->name = NULL;
        }

        count++;
    }

    productWrapper->data = products;
    productWrapper->used = count;
    productWrapper->limit = allocated;
    productWrapper->size = sizeof(Product);

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

int PromptUserForProduct(sqlite3 *db, Product **outProduct)
{
    GenericWrapper *productWrapper = malloc(sizeof(GenericWrapper));
    if(productWrapper == NULL)
    {
        fprintf(stderr, "Memory allocation failed for product wrapper.\n");
        exit(EXIT_FAILURE);
    }
    InitProductWrapper(productWrapper);
    // First get the product name from the user
    char product_name[128];
    printf("\nSearch for products by name: ");

    // Read exactly product_name size of bytes from user
    // fgets prevents buffer overflow
    fgets(product_name, sizeof(product_name), stdin);

    // Remove the trailing newline character if present
    size_t len = strlen(product_name);
    if (len > 0 && product_name[len - 1] == '\n')
    {
        product_name[len - 1] = '\0';
    }

    Product product = {
        .id = 0,     // Assuming 0 means no specific ID
        .name = NULL // Initialize name to an empty string
    };
    // strncpy(product.name, product_name, sizeof(product.name) - 1);
    product.name = strdup(product_name);

    int rs;
    // Get matched products in generic wrapper
    // do not forhet to free the wrapper after use
    if ((rs = GetMatchedProducts(db, &product, productWrapper)) & (SQLITE_ROW | SQLITE_DONE))
    {
        for (size_t i = 0; i < productWrapper->used; i++)
        {
            Product *pProduct = (Product *)productWrapper->getElementAt(productWrapper, i);
            PrintProduct(pProduct);
        }
        printf("Found %zu products matching '%s':\n", productWrapper->used, product.name);
        printf("\nType ID of the product you want to select or 0 to cancel: ");
        int productId;
        // Read the product ID from user input
        scanf("%d", &productId);
        // Clear the input buffer
        while (getchar() != '\n' && getchar() != EOF);
        if (productId == 0)
        {
            printf("Product selection cancelled.\n");
            FreeProduct(&product); // Free the product name
            FreeWrapper(productWrapper); // Free the wrapper after use
            FreeMemory((void **)&productWrapper);
            return 0; // User cancelled the selection
        }

        // Check if picked ID is in fetched products and set it to outProduct
        for (size_t i = 0; i < productWrapper->used; i++)
        {
            Product *pProduct = (Product *)productWrapper->getElementAt(productWrapper, i);
            if (pProduct->id == productId)
            {
                // Set the output product to the found one
                Product *newPProduct = (Product *)malloc(sizeof(Product));
                if (newPProduct == NULL)
                {
                    fprintf(stderr, "Memory allocation failed.\n");
                    exit(EXIT_FAILURE);
                }
                // Copy the found product data
                newPProduct->id = pProduct->id;
                newPProduct->name = strdup(pProduct->name);

                *outProduct = newPProduct; // Set the output product to the selected one
                printf("Selected product: ");
                PrintProduct(newPProduct);
                FreeWrapper(productWrapper); // Free the wrapper after use
                FreeMemory((void **)&productWrapper); // Free the product name
                FreeProduct(&product);
                // FreeMemory((void **)&productWrapper);
                return 1; // Successfully selected a product
            }
        }

        // ====================================
        // Note: after this point instead of returning, set 'rs' to the desired value so that it will free the allocated memory in the end
        // ====================================

        // If we reach here, it means the product ID was not found in the fetched products
        // promt the user if he wants to search the database
        printf("Product with ID %d not found in the fetched products.\n", productId);
        printf("\nDo you want to search the database for this product? (y/n): ");
        char choice;
        scanf(" %c", &choice);
        // Clear the input buffer
        while (getchar() != '\n' && getchar() != EOF);
        if (tolower(choice) == 'n')
        {
            printf("Product selection cancelled.\n");
            rs = 0; // User cancelled the selection
        }
        else if (tolower(choice) == 'y')
        {
            printf("Searching for product with ID %d in the database...\n", productId);
            rs = GetProductById(db, productId, &product);
            if (rs == SQLITE_ROW)
            {
                // Set the output product to the found one
                Product *newPProduct = (Product *)malloc(sizeof(Product));
                if (newPProduct == NULL)
                {
                    fprintf(stderr, "Memory allocation failed.\n");
                    exit(EXIT_FAILURE);
                }
                newPProduct->id = product.id;
                newPProduct->name = strdup(product.name);

                *outProduct = newPProduct;
                printf("Found product: ");
                PrintProduct(&product);
                rs = 1; // Successfully selected a product
            }
            else if (rs == SQLITE_DONE)
            {
                printf("No product found with ID %d.\n", productId);
            }
            else
            {
                fprintf(stderr, "Error searching for product (%d): %s\n", rs, sqlite3_errstr(rs));
            }
        }
    }
    else
    {
        Product *firstMatchedProduct = (Product *)productWrapper->getElementAt(productWrapper, 0);
        printf("%s - searched for product '%s'\n", sqlite3_errstr(rs), firstMatchedProduct->name);
    }

    FreeWrapper(productWrapper); // Free the wrapper after use
    FreeMemory((void **)&productWrapper);
    return rs;
}

void PrintProduct(Product *product)
{
    if (product == NULL)
    {
        printf("No product data available.\n");
        return;
    }
    printf("Product ID: %d, Name: %s\n", product->id, product->name);
}