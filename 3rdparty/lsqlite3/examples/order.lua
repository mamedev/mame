
local sqlite3 = require("lsqlite3")

local db = assert( sqlite3:open_memory() )

assert( db:exec[[

  CREATE TABLE customer (
    id		INTEGER PRIMARY KEY, 
    name	VARCHAR(40)
  );

  CREATE TABLE invoice (
    id		INTEGER PRIMARY KEY,
    customer	INTEGER NOT NULL,
    title	VARCHAR(80) NOT NULL,
    article1	VARCHAR(40) NOT NULL,
    price1	REAL NOT NULL,
    article2	VARCHAR(40),
    price2	REAL
  );

  CREATE TABLE invoice_overflow (
    id		INTEGER PRIMARY KEY,
    invoice	INTEGER NOT NULL,
    article	VARCHAR(40) NOT NULL,
    price	REAL NOT NULL
  );

  INSERT INTO customer VALUES(
    1, "Michael" );

  INSERT INTO invoice VALUES(
    1, 1, "Computer parts", "harddisc", 89.90, "floppy", 9.99 );

  INSERT INTO customer VALUES(
    2, "John" );

  INSERT INTO invoice VALUES(
    2, 2, "Somme food", "apples", 2.79, "pears", 5.99 );

  INSERT INTO invoice_overflow VALUES(
    NULL, 2, "grapes", 6.34 );

  INSERT INTO invoice_overflow VALUES(
    NULL, 2, "strawberries", 4.12 );

  INSERT INTO invoice_overflow VALUES(
    NULL, 2, "tomatoes", 6.17 );

  INSERT INTO invoice VALUES(
    3, 2, "A new car", "Cybercar XL-1000", 65000.00, NULL, NULL );

]] )


local function customer_name(id)
  local stmt = db:prepare("SELECT name FROM customer WHERE id = ?")
  stmt:bind_values(id)
  stmt:step()
  local r = stmt:get_uvalues()
  stmt:finalize()
  return r
end


local function all_invoices()
  return db:nrows("SELECT id, customer, title FROM invoice")
end


local function all_articles(invoice)

  local function iterator()
    local stmt, row

    -- Get the articles that are contained in the invoice table itself.
    stmt = db:prepare("SELECT article1, price1, article2, price2 FROM invoice WHERE id = ?")
    stmt:bind_values(invoice)
    stmt:step()
    row = stmt:get_named_values()

    -- Every Invoice has at least one article.
    coroutine.yield(row.article1, row.price1)

    -- Maybe the Invoice has a second article?
    if row.article2 then

      -- Yes, there is a second article, so return it.
      coroutine.yield(row.article2, row.price2)

      -- When there was an second article, maybe there are even
      -- more articles in the overflow table? We will see...

      stmt = db:prepare("SELECT article, price FROM invoice_overflow WHERE invoice = ? ORDER BY id")
      stmt:bind_values(invoice)
      
      for row in stmt:nrows() do
        coroutine.yield(row.article, row.price)
      end
    end
  end

  return coroutine.wrap(iterator)
end


for invoice in all_invoices() do
  local id    = invoice.id
  local name  = customer_name(invoice.customer)
  local title = invoice.title

  print()
  print("Invoice #"..id..", "..name..": '"..title.."'")
  print("----------------------------------------")

  for article, price in all_articles(id) do
    print( string.format("%20s  %8.2f", article, price) )
  end

  print()
end
