CREATE TABLE "orders" (
	"id"	INTEGER NOT NULL UNIQUE,
	"client_id"	INTEGER,
	"product_id"	INTEGER,
	"amount"	INTEGER,
	CONSTRAINT "fk_orders_clients" FOREIGN KEY("client_id") REFERENCES clients("id"),
	CONSTRAINT "fk_orders_products" FOREIGN KEY("product_id") REFERENCES products("id"),
	PRIMARY KEY("id" AUTOINCREMENT)
);

INSERT INTO orders (client_id, product_id, amount) VALUES (?, ?, ?);

UPDATE orders SET client_id = ?1, product_id = ?2, amount = ?3 WHERE id = ?4;

-- ORDERS GROUPED BY CLIENTS

SELECT cl.id, cl.first_name, cl.last_name, prd.name AS product_name
FROM orders AS ord
LEFT JOIN clients AS cl ON cl.id = ord.client_id
LEFT JOIN products AS prd ON prd.id = ord.product_id
GROUP BY cl.id, prd.id
ORDER BY cl.last_name ASC, cl.first_name ASC;
--

-- CLIENTS BY ORDER COUNT
SELECT cl.id, cl.first_name, cl.last_name, 
o.id as order_id, o.product_id, o.amount, p.name as product_name, 
(SELECT COUNT(*) FROM orders WHERE client_id = cl.id) as order_count 
FROM clients AS cl
INNER JOIN orders o ON cl.id = o.client_id 
LEFT JOIN products p ON o.product_id = p.id 
ORDER BY order_count DESC, cl.last_name ASC, cl.first_name ASC, o.id ASC;
--

-- CHEAPEST OFFER FOR EACH ORDER
SELECT cl.id, cl.first_name, cl.last_name, prd.name, off.product_id AS product_id, off.id AS offer_id,
off.price, o.id AS order_id, sh.name
FROM clients AS cl
INNER JOIN orders AS o ON o.client_id = cl.id
LEFT JOIN products AS prd ON prd.id = o.product_id
LEFT JOIN offers AS off ON off.product_id = prd.id
LEFT JOIN shops AS sh ON sh.id = off.shop_id
WHERE off.price = (
	SELECT MIN(price) FROM offers WHERE product_id = prd.id
)
ORDER BY cl.last_name ASC, cl.first_name ASC, o.id ASC;
--

-- CHEAPEST SHOP FOR INDIVIDUAL CLIENT
SELECT cl.id AS client_id, cl.first_name, cl.last_name, SUM(off.price * o.amount) AS total_cost_for_shop,
sh.name AS shop_name, sh.id AS shop_id, COUNT(o.id) AS orders_count
FROM clients AS cl
INNER JOIN orders AS o ON o.client_id = cl.id
LEFT JOIN products AS prd ON prd.id = o.product_id
LEFT JOIN offers AS off ON off.product_id = prd.id
LEFT JOIN shops AS sh ON sh.id = off.shop_id
GROUP BY sh.id, cl.id
ORDER BY cl.id
--