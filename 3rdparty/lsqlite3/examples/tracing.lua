
local sqlite3 = require("lsqlite3")

local db = sqlite3.open_memory()

db:trace( function(ud, sql)
  print("Sqlite Trace:", sql)
end )

db:exec[[
  CREATE TABLE test ( id INTEGER PRIMARY KEY, content VARCHAR );

  INSERT INTO test VALUES (NULL, 'Hello World');
  INSERT INTO test VALUES (NULL, 'Hello Lua');
  INSERT INTO test VALUES (NULL, 'Hello Sqlite3');
]]

for row in db:rows("SELECT * FROM test") do
  -- NOP
end
