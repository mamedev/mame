
local sqlite3 = require("lsqlite3")

local db = sqlite3.open_memory()

optbl = { [sqlite3.UPDATE] = "UPDATE";
          [sqlite3.INSERT] = "INSERT";
          [sqlite3.DELETE] = "DELETE"
        }
setmetatable(optbl,
	{__index=function(t,n) return string.format("Unknown op %d",n) end})

udtbl = {0, 0, 0}

db:update_hook( function(ud, op, dname, tname, rowid)
  print("Sqlite Update Hook:", optbl[op], dname, tname, rowid)
end, udtbl)

db:exec[[
  CREATE TABLE test ( id INTEGER PRIMARY KEY, content VARCHAR );

  INSERT INTO test VALUES (NULL, 'Hello World');
  INSERT INTO test VALUES (NULL, 'Hello Lua');
  INSERT INTO test VALUES (NULL, 'Hello Sqlite3');
  UPDATE test SET content = 'Hello Again World' WHERE id = 1;
  DELETE FROM test WHERE id = 2;
]]

for row in db:nrows("SELECT * FROM test") do
  print(row.id, row.content)
end
