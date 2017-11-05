
local sqlite3 = require("lsqlite3")

local db = sqlite3.open_memory()

db:exec[[
  CREATE TABLE test (
    id        INTEGER PRIMARY KEY,
    content   VARCHAR
  );
]]

local insert_stmt = assert( db:prepare("INSERT INTO test VALUES (NULL, ?)") )

local function insert(data)
  insert_stmt:bind_values(data)
  insert_stmt:step()
  insert_stmt:reset()
end

local select_stmt = assert( db:prepare("SELECT * FROM test") )

local function select()
  for row in select_stmt:nrows() do
    print(row.id, row.content)
  end
end

insert("Hello World")
print("First:")
select()

insert("Hello Lua")
print("Second:")
select()

insert("Hello Sqlite3")
print("Third:")
select()
