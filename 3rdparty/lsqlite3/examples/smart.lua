
local sqlite3 = require("lsqlite3")

local db = sqlite3.open_memory()

db:exec[[ CREATE TABLE test (id INTEGER PRIMARY KEY, content) ]]

local stmt = db:prepare[[ INSERT INTO test VALUES (:key, :value) ]]

stmt:bind_names{  key = 1,  value = "Hello World"    }
stmt:step()
stmt:reset()
stmt:bind_names{  key = 2,  value = "Hello Lua"      }
stmt:step()
stmt:reset()
stmt:bind_names{  key = 3,  value = "Hello Sqlite3"  }
stmt:step()
stmt:finalize()

for row in db:nrows("SELECT * FROM test") do
  print(row.id, row.content)
end
