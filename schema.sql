CREATE TABLE shops (id INTEGER PRIMARY KEY, name TEXT);
CREATE TABLE products (id INTEGER PRIMARY KEY, name TEXT);
CREATE TABLE offers (id INTEGER PRIMARY KEY, shop_id INTEGER, product_id INTEGER, price REAL, FOREIGN KEY(shop_id) REFERENCES shops(id), FOREIGN KEY(product_id) REFERENCES products(id));
CREATE TABLE clients (id INTEGER PRIMARY KEY, first_name TEXT, last_name TEXT);
CREATE TABLE IF NOT EXISTS "orders" (
	"id"	INTEGER NOT NULL UNIQUE,
	"client_id"	INTEGER,
	"product_id"	INTEGER,
	"amount"	INTEGER,
	CONSTRAINT "fk_orders_clients" FOREIGN KEY("client_id") REFERENCES clients("id"),
	CONSTRAINT "fk_orders_products" FOREIGN KEY("product_id") REFERENCES products("id"),
	PRIMARY KEY("id" AUTOINCREMENT)
);
CREATE TABLE sqlite_sequence(name,seq);
