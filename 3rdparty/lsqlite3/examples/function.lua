
local sqlite3 = require("lsqlite3")


local db = sqlite3.open_memory()

assert( db:exec "CREATE TABLE test (col1, col2)" )
assert( db:exec "INSERT INTO test VALUES (1, 2)" )
assert( db:exec "INSERT INTO test VALUES (2, 4)" )
assert( db:exec "INSERT INTO test VALUES (3, 6)" )
assert( db:exec "INSERT INTO test VALUES (4, 8)" )
assert( db:exec "INSERT INTO test VALUES (5, 10)" )

assert( db:create_function("my_sum", 2, function(ctx, a, b)
  ctx:result_number( a + b )
end))


for col1, col2, sum in db:urows("SELECT *, my_sum(col1, col2) FROM test") do
  print(col1, col2, sum)
end
