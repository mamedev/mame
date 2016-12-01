
local sqlite3 = require("lsqlite3")

local db = sqlite3.open_memory()

assert( db:exec "CREATE TABLE test (col1, col2)" )
assert( db:exec "INSERT INTO test VALUES (1, 2)" )
assert( db:exec "INSERT INTO test VALUES (2, 4)" )
assert( db:exec "INSERT INTO test VALUES (3, 6)" )
assert( db:exec "INSERT INTO test VALUES (4, 8)" )
assert( db:exec "INSERT INTO test VALUES (5, 10)" )

do

  local square_error_sum = 0

  local function step(ctx, a, b)
    local error        = a - b
    local square_error = error * error
    square_error_sum   = square_error_sum + square_error
  end

  local function final(ctx)
    ctx:result_number( square_error_sum / ctx:aggregate_count() )
  end

  assert( db:create_aggregate("my_stats", 2, step, final) )

end

--for a,b in db:urows("SELECT col1, col2 FROM test")
--do print("a b: ", a, b) end

for my_stats in db:urows("SELECT my_stats(col1, col2) FROM test")
do print("my_stats:", my_stats) end
