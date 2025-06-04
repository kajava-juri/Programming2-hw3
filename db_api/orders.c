#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>

#include <sqlite3.h>
#include "orders.h"
#include "db.h"
#include "../main.h"

void InitOrdersWrapper(GenericWrapper *wrapper)
{
    wrapper->data = NULL;
    wrapper->freeData = NULL;
    wrapper->getElementAt = NULL;
    wrapper->size = 0;
    wrapper->used = 0;
    wrapper->limit = 0;
}

void *GetOrderAt(GenericWrapper *wrapper, size_t index)
{
    if (index >= wrapper->used)
    {
        return NULL; // Index out of bounds
    }
    return ((Order *)wrapper->data) + index;
}

void PrintOrder(Order *order)
{
    if (order == NULL)
    {
        printf("Order is NULL.\n");
        return;
    }
    printf("Order ID: %d, Client ID: %d, Product ID: %d, Amount: %d\n", order->id, order->client_id, order->product_id, order->amount);
}

int InsertOrder(sqlite3 *db, Order *order)
{

    // Sanity check for existing order values
    if (order == NULL || !(order->client_id > 0) || !(order->product_id > 0) || !(order->amount > 0))
    {
        fprintf(stderr, "Invalid order data provided.\n");
        return -1;
    }

    sqlite3_stmt *stmt;
    const char *sql = "INSERT INTO orders (client_id, product_id, amount) VALUES (?, ?, ?);";
    int rs;

    if ((rs = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL)) != SQLITE_OK)
    {
        fprintf(stderr, "Error preparing statement: %s\n", sqlite3_errmsg(db));
        return rs;
    }

    sqlite3_bind_int(stmt, 1, order->client_id);
    sqlite3_bind_int(stmt, 2, order->product_id);
    sqlite3_bind_int(stmt, 3, order->amount);

    rs = sqlite3_step(stmt);
    if (rs != SQLITE_DONE)
    {
        fprintf(stderr, "Error executing statement: %s - %s\n", sqlite3_errstr(rs), sqlite3_errmsg(db));
    }
    else
    {
        order->id = (int)sqlite3_last_insert_rowid(db);
    }

    sqlite3_finalize(stmt);
    return rs;
}

int DeleteOrder(sqlite3 *db, int orderId)
{
    if (orderId <= 0)
    {
        fprintf(stderr, "Invalid order ID provided.\n");
        return -1;
    }

    sqlite3_stmt *stmt;
    const char *sql = "DELETE FROM orders WHERE id = ?1;";
    int rs;

    if ((rs = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL)) != SQLITE_OK)
    {
        fprintf(stderr, "Error preparing statement: %s\n", sqlite3_errmsg(db));
        return rs;
    }

    sqlite3_bind_int(stmt, 1, orderId);
    rs = sqlite3_step(stmt);

    if (rs != SQLITE_DONE)
    {
        fprintf(stderr, "Error executing statement: %s - %s\n", sqlite3_errstr(rs), sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);
    return rs;
}

int ModifyOrder(sqlite3 *db, Order *order)
{
    // Sanity check for existing order values
    if (order == NULL || !(order->id > 0) || !(order->client_id > 0) || !(order->product_id > 0) || !(order->amount > 0))
    {
        fprintf(stderr, "Invalid order data provided.\n");
        return -1;
    }

    sqlite3_stmt *stmt;
    const char *sql = "UPDATE orders SET client_id = ?, product_id = ?, amount = ? WHERE id = ?;";
    int rs;

    if ((rs = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL)) != SQLITE_OK)
    {
        fprintf(stderr, "Error preparing statement: %s\n", sqlite3_errmsg(db));
        return rs;
    }

    sqlite3_bind_int(stmt, 1, order->client_id);
    sqlite3_bind_int(stmt, 2, order->product_id);
    sqlite3_bind_int(stmt, 3, order->amount);
    sqlite3_bind_int(stmt, 4, order->id);

    rs = sqlite3_step(stmt);

    if (rs != SQLITE_DONE)
    {
        fprintf(stderr, "Error executing statement: %s - %s\n", sqlite3_errstr(rs), sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);
    return rs;
}

int GetOrderById(sqlite3 *db, int orderId, Order *order)
{
    if (order == NULL || orderId <= 0)
    {
        fprintf(stderr, "Invalid order ID or order pointer provided.\n");
        return -1;
    }

    sqlite3_stmt *stmt;
    const char *sql = "SELECT id, client_id, product_id, amount FROM orders WHERE id = ?1;";
    int rs;

    if ((rs = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL)) != SQLITE_OK)
    {
        fprintf(stderr, "Error preparing statement: %s\n", sqlite3_errmsg(db));
        return rs;
    }

    sqlite3_bind_int(stmt, 1, orderId);

    if ((rs = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        order->id = sqlite3_column_int(stmt, 0);
        order->client_id = sqlite3_column_int(stmt, 1);
        order->product_id = sqlite3_column_int(stmt, 2);
        order->amount = sqlite3_column_int(stmt, 3);
    }
    else if (rs == SQLITE_DONE)
    {
        fprintf(stderr, "No order found with ID %d.\n", orderId);
        rs = -1; // No rows found
    }
    else
    {
        fprintf(stderr, "Error executing statement: %s - %s\n", sqlite3_errstr(rs), sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);
    return rs;
}

int PromptUserForOrder(sqlite3 *db, Order *order)
{
    if (order == NULL)
    {
        fprintf(stderr, "Order pointer is NULL.\n");
        return -1;
    }

    printf("Enter order ID (0 to cancel): ");
    int orderId;
    while (((scanf("%d", &orderId) != 1) || (orderId < 0)) && orderId != 0)
    {
        printf("Invalid order ID. Please enter a positive integer: ");
        // Clear the input buffer
        while (getchar() != '\n' && getchar() != EOF);
    }

    if(orderId == 0)
    {
        printf("Order selection cancelled.\n");
        return 0; // User cancelled the selection
    }

    order->id = orderId;
    int rs = GetOrderById(db, orderId, order);
    return rs;
}

void PrintOrdersGroupedByClient(sqlite3 *db)
{
    sqlite3_stmt *stmt;
    const char *sql = "SELECT cl.id, cl.first_name, cl.last_name, o.id, o.product_id, o.amount, prd.name "
                      "FROM orders AS o "
                      "LEFT JOIN clients AS cl ON cl.id = o.client_id "
                      "LEFT JOIN products AS prd ON prd.id = o.product_id "
                      "GROUP BY cl.id, prd.id "
                      "ORDER BY cl.last_name ASC, cl.first_name ASC;";
    int rs;
    if ((rs = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL)) != SQLITE_OK)
    {
        fprintf(stderr, "Error preparing statement: %s\n", sqlite3_errmsg(db));
        return;
    }

    printf("\n=== Orders Grouped by Clients ===\n");

    // Since rows are sorted by client names, orders can be grouped under client until a new client is found
    int currentClientId = -1;
    while ((rs = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        int clientId = sqlite3_column_int(stmt, 0);
        const unsigned char *firstName = sqlite3_column_text(stmt, 1);
        const unsigned char *lastName = sqlite3_column_text(stmt, 2);
        int orderId = sqlite3_column_int(stmt, 3);
        int productId = sqlite3_column_int(stmt, 4);
        int amount = sqlite3_column_int(stmt, 5);
        const unsigned char *productName = sqlite3_column_text(stmt, 6);

        if (clientId != currentClientId)
        {
            // New client found, print client details
            if (currentClientId != -1)
            {
                printf("\n"); // Print a newline before the next client
            }
            printf("Client ID: %d, Name: %s %s\n", clientId, firstName, lastName);
            printf("────────────────────────────────────────\n");
            currentClientId = clientId;
        }

        // Print order details
        printf("    Order ID %-3d: %s (ID %-3d) Amount: %d\n", orderId, productName, productId, amount);
    }

    sqlite3_finalize(stmt);
}

void PrintAllOrdersByClientOrderCount(sqlite3 *db)
{
    sqlite3_stmt *stmt;
    const char *sql = "SELECT cl.id, cl.first_name, cl.last_name, "
                      "o.id as order_id, o.product_id, o.amount, p.name as product_name, "
                      "(SELECT COUNT(*) FROM orders WHERE client_id = cl.id) as orderCount "
                      "FROM clients AS cl "
                      "INNER JOIN orders o ON cl.id = o.client_id "
                      "LEFT JOIN products p ON o.product_id = p.id "
                      "ORDER BY orderCount DESC, cl.last_name ASC, cl.first_name ASC, o.id ASC;";
    int rs;

    if ((rs = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL)) != SQLITE_OK)
    {
        fprintf(stderr, "Error preparing statement: %s\n", sqlite3_errmsg(db));
        return;
    }

    printf("\n=== Clients by order count ===\n");

    int currentClientId = -1;
    int totalOrders = 0;

    while ((rs = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        int clientId = sqlite3_column_int(stmt, 0);
        const char *firstName = (const char *)sqlite3_column_text(stmt, 1);
        const char *lastName = (const char *)sqlite3_column_text(stmt, 2);
        int orderId = sqlite3_column_int(stmt, 3);
        int productId = sqlite3_column_int(stmt, 4);
        int amount = sqlite3_column_int(stmt, 5);
        const char *productName = (const char *)sqlite3_column_text(stmt, 6);
        int orderCount = sqlite3_column_int(stmt, 7);

        // Check if we're on a new client
        if (clientId != currentClientId)
        {
            if (currentClientId != -1)
            {
                printf("\n"); // Add spacing between clients
            }

            currentClientId = clientId;

            // Handle NULL names
            const char *fName = firstName ? firstName : "N/A";
            const char *lName = lastName ? lastName : "N/A";

            printf("Client ID %d: %s %s (%d orders)\n",
                   clientId, fName, lName, orderCount);
            printf("────────────────────────────────────────\n");
        }

        // Handle NULL product name
        const char *pName = productName ? productName : "Unknown Product";

        printf("  Order ID %-3d: %s (ID: %-3d) Amount: %d\n",
               orderId, pName, productId, amount);

        totalOrders++;
    }

    if (currentClientId == -1)
    {
        printf("No orders found in the database.\n");
    }
    else
    {
        printf("\n════════════════════════════════════════\n");
        printf("Total orders displayed: %d\n", totalOrders);
    }

    if (rs != SQLITE_DONE)
    {
        fprintf(stderr, "Error executing statement: %s - %s\n", sqlite3_errstr(rs), sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);
}

void PrintCheapestOffersForAllClientOrders(sqlite3 *db)
{
    sqlite3_stmt *stmt;
    const char *sql = "SELECT cl.id, cl.first_name, cl.last_name, prd.name, "
                      "off.product_id AS product_id, off.id AS offer_id, "
                      "off.price, o.id AS order_id, o.amount, sh.name "
                      "FROM clients AS cl "
                      "INNER JOIN orders AS o ON o.client_id = cl.id "
                      "LEFT JOIN products AS prd ON prd.id = o.product_id "
                      "LEFT JOIN offers AS off ON off.product_id = prd.id "
                      "LEFT JOIN shops AS sh ON sh.id = off.shop_id "
                      "WHERE off.price = ("
                      "    SELECT MIN(price) FROM offers WHERE product_id = prd.id"
                      ") "
                      "ORDER BY cl.last_name ASC, cl.first_name ASC, o.id ASC;";
    int rs;

    if ((rs = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL)) != SQLITE_OK)
    {
        fprintf(stderr, "Error preparing statement: %s\n", sqlite3_errmsg(db));
        return;
    }

    printf("\n=== Cheapest Offers for All Orders ===\n");

    // Your implementation here - similar to PrintAllOrdersByClientOrderCount
    // but showing offer details instead of just products
    int currentClientId = -1;
    int currentOrderId = -1;
    while ((rs = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        int clientId = sqlite3_column_int(stmt, 0);
        const char *firstName = (const char *)sqlite3_column_text(stmt, 1);
        const char *lastName = (const char *)sqlite3_column_text(stmt, 2);
        const char *productName = (const char *)sqlite3_column_text(stmt, 3);
        int productId = sqlite3_column_int(stmt, 4);
        int offerId = sqlite3_column_int(stmt, 5);
        double price = sqlite3_column_double(stmt, 6);
        int orderId = sqlite3_column_int(stmt, 7);
        int amount = sqlite3_column_int(stmt, 8);
        const char *shopName = (const char *)sqlite3_column_text(stmt, 9);

        if (clientId != currentClientId)
        {
            if (currentClientId != -1)
            {
                printf("\n"); // Print a newline before the next client
            }
            printf("Client ID: %d, Name: %s %s\n", clientId, firstName, lastName);
            printf("────────────────────────────────────────\n");
            currentClientId = clientId;
        }

        if (currentOrderId != orderId)
        {
            if (currentOrderId != -1)
            {
                printf("\n"); // Print a newline before the next order
            }
            printf("    Order ID: %d\n", orderId);
            currentOrderId = orderId;
        }

        printf("        Product '%s' (ID %-3d) - Offer ID: %-3d at Price: %.2f from Shop: %s "
               "Amount: %d\n",
               productName, productId, offerId, price, shopName ? shopName : "Unknown", amount);
    }
    if (rs != SQLITE_DONE)
    {
        fprintf(stderr, "Error executing statement: %s - %s\n", sqlite3_errstr(rs), sqlite3_errmsg(db));
    }
    sqlite3_finalize(stmt);
}

void FindCheapestShopPerClient(sqlite3 *db)
{
    sqlite3_stmt *stmt;
    const char *sql = "SELECT cl.id AS client_id, cl.first_name, cl.last_name, sh.id AS shop_id, SUM(off.price * o.amount) AS total_cost_for_shop, "
                      "sh.name AS shop_name, COUNT(o.id) AS orders_count "
                      "FROM clients AS cl "
                      "INNER JOIN orders AS o ON o.client_id = cl.id "
                      "LEFT JOIN products AS prd ON prd.id = o.product_id "
                      "LEFT JOIN offers AS off ON off.product_id = prd.id "
                      "LEFT JOIN shops AS sh ON sh.id = off.shop_id "
                      "GROUP BY sh.id, cl.id "
                      "ORDER BY cl.id ";

    int currentClientId = -1;
    double minCost = 0.0;
    int bestShopId = -1;
    char bestShopName[128] = "";
    char currentFirstName[128] = "";
    char currentLastName[128] = "";

    int rs;
    if ((rs = sqlite3_prepare_v2(db, (const char *)sql, -1, &stmt, NULL)) != SQLITE_OK)
    {
        fprintf(stderr, "Error preparing statement: %s\n", sqlite3_errmsg(db));
        return;
    }
    printf("\n=== Cheapest Shop per Client ===\n");
    while ((rs = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        int clientId = sqlite3_column_int(stmt, 0);
        int shopId = sqlite3_column_int(stmt, 3);
        double totalCost = sqlite3_column_double(stmt, 4);
        const unsigned char *shopName = sqlite3_column_text(stmt, 5);
        const unsigned char *firstName = sqlite3_column_text(stmt, 1);
        const unsigned char *lastName = sqlite3_column_text(stmt, 2);

        // printf("CHECKING SHOP: %s for client %d\n", shopName, clientId);

        if (clientId != currentClientId)
        {
            // Print previous client's best shop (if any)
            if (currentClientId != -1)
            {
                printf("Best shop for client %s %s (ID %d): Shop ID %d (%.2f €): %s\n",
                       currentFirstName, currentLastName, currentClientId, bestShopId, minCost, bestShopName);
            }

            // Reset for new client
            currentClientId = clientId;
            minCost = totalCost;
            bestShopId = shopId;
            strcpy(currentFirstName, (const char *)firstName);
            strcpy(currentLastName, (const char *)lastName);
            strcpy(bestShopName, (const char *)shopName);
        }
        else
        {
            // Same client - check if this shop is cheaper
            if (totalCost < minCost)
            {
                minCost = totalCost;
                bestShopId = shopId;
                strcpy(bestShopName, (const char *)shopName);
            }
        }
    }

    if (currentClientId != -1)
    {
        printf("Best shop for client %s %s (ID %d): Shop ID %d (%.2f €): %s\n",
               currentFirstName, currentLastName, currentClientId, bestShopId, minCost, bestShopName);
    }

    if (rs != SQLITE_DONE)
    {
        fprintf(stderr, "Error executing statement: %s - %s\n", sqlite3_errstr(rs), sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);
}

void PrintPotentialSavingsPerClient(sqlite3 *db)
{
    sqlite3_stmt *stmt;
    const char *sql = "SELECT cl.id AS client_id, cl.first_name, cl.last_name, sh.id AS shop_id, SUM(off.price * o.amount) AS total_cost_for_shop, "
                      "sh.name AS shop_name, COUNT(o.id) AS orders_count "
                      "FROM clients AS cl "
                      "INNER JOIN orders AS o ON o.client_id = cl.id "
                      "LEFT JOIN products AS prd ON prd.id = o.product_id "
                      "LEFT JOIN offers AS off ON off.product_id = prd.id "
                      "LEFT JOIN shops AS sh ON sh.id = off.shop_id "
                      "GROUP BY sh.id, cl.id "
                      "ORDER BY cl.id ";

    int currentClientId = -1;
    double minCost = 0.0;
    double maxCost = 0.0;
    int bestShopId = -1;
    int worstShopId = -1;
    char bestShopName[128] = "";
    char worstShopName[128] = "";
    char currentFirstName[128] = "";
    char currentLastName[128] = "";

    int rs;
    if ((rs = sqlite3_prepare_v2(db, (const char *)sql, -1, &stmt, NULL)) != SQLITE_OK)
    {
        fprintf(stderr, "Error preparing statement: %s\n", sqlite3_errmsg(db));
        return;
    }
    printf("\n=== Potential savings per client (best price vs wors price) ===\n");
    while ((rs = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        int clientId = sqlite3_column_int(stmt, 0);
        int shopId = sqlite3_column_int(stmt, 3);
        double totalCost = sqlite3_column_double(stmt, 4);
        const unsigned char *shopName = sqlite3_column_text(stmt, 5);
        const unsigned char *firstName = sqlite3_column_text(stmt, 1);
        const unsigned char *lastName = sqlite3_column_text(stmt, 2);

        if (clientId != currentClientId)
        {
            // Print previous client
            if (currentClientId != -1)
            {
                double potentialSavings = maxCost - minCost;
                printf("Client %s %s (ID %d) could save %.2f € by choosing shop ID %d (%s) instead of shop ID %d (%s)\n",
                       currentFirstName, currentLastName, currentClientId, potentialSavings,
                       bestShopId, bestShopName, worstShopId, worstShopName);
            }

            // Reset for new client
            currentClientId = clientId;
            minCost = totalCost;
            maxCost = totalCost;
            bestShopId = shopId;
            worstShopId = shopId;
            strcpy(currentFirstName, (const char *)firstName);
            strcpy(currentLastName, (const char *)lastName);
            strcpy(bestShopName, (const char *)shopName);
            strcpy(worstShopName, (const char *)shopName);
        }
        else
        {
            // Same client - check if this shop is cheaper
            if (totalCost < minCost)
            {
                minCost = totalCost;
                bestShopId = shopId;
                strcpy(bestShopName, (const char *)shopName);
            }
            else if (totalCost > maxCost)
            {
                maxCost = totalCost;
                worstShopId = shopId;
                strcpy(worstShopName, (const char *)shopName);
            }
        }
    }

    // Print the last client's info
    if (currentClientId != -1)
    {
        double potentialSavings = maxCost - minCost;
        printf("Client %s %s (ID %d) could save %.2f € by choosing shop ID %d (%s) instead of shop ID %d (%s)\n",
               currentFirstName, currentLastName, currentClientId, potentialSavings,
               bestShopId, bestShopName, worstShopId, worstShopName);
    }

    if (rs != SQLITE_DONE)
    {
        fprintf(stderr, "Error executing statement: %s - %s\n", sqlite3_errstr(rs), sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);
}