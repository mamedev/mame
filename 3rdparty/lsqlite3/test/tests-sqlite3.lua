
--[[--------------------------------------------------------------------------

    Author: Michael Roth <mroth@nessie.de>

    Copyright (c) 2004, 2005 Michael Roth <mroth@nessie.de>

    Permission is hereby granted, free of charge, to any person
    obtaining a copy of this software and associated documentation
    files (the "Software"), to deal in the Software without restriction,
    including without limitation the rights to use, copy, modify, merge,
    publish, distribute, sublicense, and/or sell copies of the Software,
    and to permit persons to whom the Software is furnished to do so,
    subject to the following conditions:

    The above copyright notice and this permission notice shall be
    included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
    IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
    CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
    TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
    SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

--]]--------------------------------------------------------------------------

-- extended for LuaSQLite3, and for Lua 5.2 (using lunitx)
-- Copyright (c) 2005-13 Doug Currie
-- Same license as above

local sqlite3 = require(arg[1]) -- "lsqlite3complete" or "lsqlite3"

local os = os

--local lunit = require "lunitx"
local lunit = require "lunit"

local tests_sqlite3

if _VERSION >= 'Lua 5.2' then

    tests_sqlite3 = lunit.module('tests-sqlite3','seeall')
    _ENV = tests_sqlite3

else

    module('tests_sqlite3', lunit.testcase, package.seeall)
    tests_sqlite3 = _M

end

-- compat

function lunit_wrap (name, fcn)
   tests_sqlite3['test_o_'..name] = fcn
end

function lunit_TestCase (name)
   return lunit.module(name,'seeall')
end

-------------------------------
-- Print library versions    --
-------------------------------

print ("SQLite v", sqlite3.version())
print ("lsqlite v", sqlite3.lversion())

-------------------------------
-- Basic open and close test --
-------------------------------

lunit_wrap("open_memory", function()
  local db = assert_userdata( sqlite3.open_memory() )
  assert( db:close() )
end)

lunit_wrap("open", function()
  local filename = "/tmp/__lua-sqlite3-20040906135849." .. os.time()
  local db = assert_userdata( sqlite3.open(filename) )
  assert( db:close() )
  os.remove(filename)
end)

-------------------------------------
-- Presence of db member functions --
-------------------------------------

local db_funcs = lunit_TestCase("Database Member Functions")

function db_funcs.setup()
  db_funcs.db = assert( sqlite3.open_memory() )
end

function db_funcs.teardown()
  assert( db_funcs.db:close() )
end

function db_funcs.test()
  local db = db_funcs.db
  assert_function( db.close )
  assert_function( db.exec )
--e  assert_function( db.irows )
  assert_function( db.rows )
--e  assert_function( db.cols )
--e  assert_function( db.first_irow )
--e  assert_function( db.first_row )
--e  assert_function( db.first_cols )
  assert_function( db.prepare )
  assert_function( db.interrupt )
  assert_function( db.last_insert_rowid )
  assert_function( db.changes )
  assert_function( db.total_changes )
end


---------------------------------------
-- Presence of stmt member functions --
---------------------------------------

local stmt_funcs = lunit_TestCase("Statement Member Functions")

function stmt_funcs.setup()
  stmt_funcs.db = assert( sqlite3.open_memory() )
  stmt_funcs.stmt = assert( stmt_funcs.db:prepare("CREATE TABLE test (id, content)") )
end

function stmt_funcs.teardown()
--e-  assert( stmt_funcs.stmt:close() )
  assert( stmt_funcs.stmt:finalize() ) --e+
  assert( stmt_funcs.db:close() )
end

function stmt_funcs.test()
  local stmt = stmt_funcs.stmt
--e  assert_function( stmt.close )
  assert_function( stmt.reset )
--e  assert_function( stmt.exec )
  assert_function( stmt.bind )
--e  assert_function( stmt.irows )
--e  assert_function( stmt.rows )
--e  assert_function( stmt.cols )
--e  assert_function( stmt.first_irow )
--e  assert_function( stmt.first_row )
--e  assert_function( stmt.first_cols )
--e  assert_function( stmt.column_names )
--e  assert_function( stmt.column_decltypes )
--e  assert_function( stmt.column_count )
--e +
  assert_function( stmt.isopen )
  assert_function( stmt.step )
  assert_function( stmt.reset )
  assert_function( stmt.finalize )
  assert_function( stmt.columns )
  assert_function( stmt.bind )
  assert_function( stmt.bind_values )
  assert_function( stmt.bind_names )
  assert_function( stmt.bind_blob )
  assert_function( stmt.bind_parameter_count )
  assert_function( stmt.bind_parameter_name )
  assert_function( stmt.get_value )
  assert_function( stmt.get_values )
  assert_function( stmt.get_name )
  assert_function( stmt.get_names )
  assert_function( stmt.get_type )
  assert_function( stmt.get_types )
  assert_function( stmt.get_uvalues )
  assert_function( stmt.get_unames )
  assert_function( stmt.get_utypes )
  assert_function( stmt.get_named_values )
  assert_function( stmt.get_named_types )
  assert_function( stmt.idata )
  assert_function( stmt.inames )
  assert_function( stmt.itypes )
  assert_function( stmt.data )
  assert_function( stmt.type )
--e +
end



------------------
-- Tests basics --
------------------

local basics = lunit_TestCase("Basics")

function basics.setup()
  basics.db = assert_userdata( sqlite3.open_memory() )
end

function basics.teardown()
  assert_number( basics.db:close() )
end

function basics.create_table()
  assert_number( basics.db:exec("CREATE TABLE test (id, name)") )
end

function basics.drop_table()
  assert_number( basics.db:exec("DROP TABLE test") )
end

function basics.insert(id, name)
  assert_number( basics.db:exec("INSERT INTO test VALUES ("..id..", '"..name.."')") )
end

function basics.update(id, name)
  assert_number( basics.db:exec("UPDATE test SET name = '"..name.."' WHERE id = "..id) )
end

function basics.test_create_drop()
  basics.create_table()
  basics.drop_table()
end

function basics.test_multi_create_drop()
  basics.create_table()
  basics.drop_table()
  basics.create_table()
  basics.drop_table()
end

function basics.test_insert()
  basics.create_table()
  basics.insert(1, "Hello World")
  basics.insert(2, "Hello Lua")
  basics.insert(3, "Hello sqlite3")
end

function basics.test_update()
  basics.create_table()
  basics.insert(1, "Hello Home")
  basics.insert(2, "Hello Lua")
  basics.update(1, "Hello World")
end


---------------------------------
-- Statement Column Info Tests --
---------------------------------

lunit_wrap("Column Info Test", function()
  local db = assert_userdata( sqlite3.open_memory() )
  assert_number( db:exec("CREATE TABLE test (id INTEGER, name TEXT)") )
  local stmt = assert_userdata( db:prepare("SELECT * FROM test") )

  assert_equal(2, stmt:columns(), "Wrong number of columns." )

  local names = assert_table( stmt:get_names() )
  assert_equal(2, #(names), "Wrong number of names.")
  assert_equal("id", names[1] )
  assert_equal("name", names[2] )

  local types = assert_table( stmt:get_types() )
  assert_equal(2, #(types), "Wrong number of declaration types.")
  assert_equal("INTEGER", types[1] )
  assert_equal("TEXT", types[2] )

  assert_equal( sqlite3.OK, stmt:finalize() )
  assert_equal( sqlite3.OK, db:close() )
end)



---------------------
-- Statement Tests --
---------------------

st = lunit_TestCase("Statement Tests")

function st.setup()
  st.db = assert( sqlite3.open_memory() )
  assert_equal( sqlite3.OK, st.db:exec("CREATE TABLE test (id, name)") )
  assert_equal( sqlite3.OK, st.db:exec("INSERT INTO test VALUES (1, 'Hello World')") )
  assert_equal( sqlite3.OK, st.db:exec("INSERT INTO test VALUES (2, 'Hello Lua')") )
  assert_equal( sqlite3.OK, st.db:exec("INSERT INTO test VALUES (3, 'Hello sqlite3')") )
end

function st.teardown()
  assert_equal( sqlite3.OK, st.db:close() )
end

function st.check_content(expected)
  local stmt = assert( st.db:prepare("SELECT * FROM test ORDER BY id") )
  local i = 0
  for row in stmt:rows() do
    i = i + 1
    assert( i <= #(expected), "Too many rows." )
    assert_equal(2, #(row), "Two result column expected.")
    assert_equal(i, row[1], "Wrong 'id'.")
    assert_equal(expected[i], row[2], "Wrong 'name'.")
  end
  assert_equal( #(expected), i, "Too few rows." )
  assert_number( stmt:finalize() )
end

function st.test_setup()
  assert_pass(function() st.check_content{ "Hello World", "Hello Lua", "Hello sqlite3" } end)
  assert_error(function() st.check_content{ "Hello World", "Hello Lua" } end)
  assert_error(function() st.check_content{ "Hello World", "Hello Lua", "Hello sqlite3", "To much" } end)
  assert_error(function() st.check_content{ "Hello World", "Hello Lua", "Wrong" } end)
  assert_error(function() st.check_content{ "Hello World", "Wrong", "Hello sqlite3" } end)
  assert_error(function() st.check_content{ "Wrong", "Hello Lua", "Hello sqlite3" } end)
end

function st.test_questionmark_args()
  local stmt = assert_userdata( st.db:prepare("INSERT INTO test VALUES (?, ?)")  )
  assert_number( stmt:bind_values(0, "Test") )
  assert_error(function() stmt:bind_values("To few") end)
  assert_error(function() stmt:bind_values(0, "Test", "To many") end)
end

function st.test_questionmark()
  local stmt = assert_userdata( st.db:prepare("INSERT INTO test VALUES (?, ?)")  )
  assert_number( stmt:bind_values(4, "Good morning") )
  assert_number( stmt:step() )
  assert_number( stmt:reset() )
  st.check_content{ "Hello World", "Hello Lua", "Hello sqlite3", "Good morning" }
  assert_number( stmt:bind_values(5, "Foo Bar") )
  assert_number( stmt:step() )
  assert_number( stmt:reset() )
  st.check_content{ "Hello World", "Hello Lua", "Hello sqlite3", "Good morning", "Foo Bar" }
  assert_number( stmt:finalize() )
end

--[===[
function st.test_questionmark_multi()
  local stmt = assert_userdata( st.db:prepare([[
    INSERT INTO test VALUES (?, ?); INSERT INTO test VALUES (?, ?) ]]))
  assert( stmt:bind_values(5, "Foo Bar", 4, "Good morning") )
  assert_number( stmt:step() )
  assert_number( stmt:reset() )
  st.check_content{ "Hello World", "Hello Lua", "Hello sqlite3", "Good morning", "Foo Bar" }
  assert_number( stmt:finalize() )
end
]===]

function st.test_identifiers()
  local stmt = assert_userdata( st.db:prepare("INSERT INTO test VALUES (:id, :name)")  )
  assert_number( stmt:bind_values(4, "Good morning") )
  assert_number( stmt:step() )
  assert_number( stmt:reset() )
  st.check_content{ "Hello World", "Hello Lua", "Hello sqlite3", "Good morning" }
  assert_number( stmt:bind_values(5, "Foo Bar") )
  assert_number( stmt:step() )
  assert_number( stmt:reset() )
  st.check_content{ "Hello World", "Hello Lua", "Hello sqlite3", "Good morning", "Foo Bar" }
  assert_number( stmt:finalize() )
end

--[===[
function st.test_identifiers_multi()
  local stmt = assert_table( st.db:prepare([[
    INSERT INTO test VALUES (:id1, :name1); INSERT INTO test VALUES (:id2, :name2) ]]))
  assert( stmt:bind_values(5, "Foo Bar", 4, "Good morning") )
  assert( stmt:exec() )
  st.check_content{ "Hello World", "Hello Lua", "Hello sqlite3", "Good morning", "Foo Bar" }
end
]===]

function st.test_identifiers_names()
  --local stmt = assert_userdata( st.db:prepare({"name", "id"}, "INSERT INTO test VALUES (:id, $name)")  )
  local stmt = assert_userdata( st.db:prepare("INSERT INTO test VALUES (:id, $name)")  )
  assert_number( stmt:bind_names({name="Good morning", id=4}) )
  assert_number( stmt:step() )
  assert_number( stmt:reset() )
  st.check_content{ "Hello World", "Hello Lua", "Hello sqlite3", "Good morning" }
  assert_number( stmt:bind_names({name="Foo Bar", id=5}) )
  assert_number( stmt:step() )
  assert_number( stmt:reset() )
  st.check_content{ "Hello World", "Hello Lua", "Hello sqlite3", "Good morning", "Foo Bar" }
  assert_number( stmt:finalize() )
end

--[===[
function st:test_identifiers_multi_names()
  local stmt = assert_table( st.db:prepare( {"name", "id1", "id2"},[[
    INSERT INTO test VALUES (:id1, $name); INSERT INTO test VALUES ($id2, :name) ]]))
  assert( stmt:bind_values("Hoho", 4, 5) )
  assert( stmt:exec() )
  st.check_content{ "Hello World", "Hello Lua", "Hello sqlite3", "Hoho", "Hoho" }
end
]===]

function st.test_colon_identifiers_names()
  local stmt = assert_userdata( st.db:prepare("INSERT INTO test VALUES (:id, :name)")  )
  assert_number( stmt:bind_names({name="Good morning", id=4}) )
  assert_number( stmt:step() )
  assert_number( stmt:reset() )
  st.check_content{ "Hello World", "Hello Lua", "Hello sqlite3", "Good morning" }
  assert_number( stmt:bind_names({name="Foo Bar", id=5}) )
  assert_number( stmt:step() )
  assert_number( stmt:reset() )
  st.check_content{ "Hello World", "Hello Lua", "Hello sqlite3", "Good morning", "Foo Bar" }
  assert_number( stmt:finalize() )
end

--[===[
function st:test_colon_identifiers_multi_names()
  local stmt = assert_table( st.db:prepare( {":name", ":id1", ":id2"},[[
    INSERT INTO test VALUES (:id1, $name); INSERT INTO test VALUES ($id2, :name) ]]))
  assert( stmt:bind_values("Hoho", 4, 5) )
  assert( stmt:exec() )
  st.check_content{ "Hello World", "Hello Lua", "Hello sqlite3", "Hoho", "Hoho" }
end


function st.test_dollar_identifiers_names()
  local stmt = assert_table( st.db:prepare({"$name", "$id"}, "INSERT INTO test VALUES (:id, $name)")  )
  assert_table( stmt:bind_values("Good morning", 4) )
  assert_table( stmt:exec() )
  st.check_content{ "Hello World", "Hello Lua", "Hello sqlite3", "Good morning" }
  assert_table( stmt:bind_values("Foo Bar", 5) )
  assert_table( stmt:exec() )
  st.check_content{ "Hello World", "Hello Lua", "Hello sqlite3", "Good morning", "Foo Bar" }
end

function st.test_dollar_identifiers_multi_names()
  local stmt = assert_table( st.db:prepare( {"$name", "$id1", "$id2"},[[
    INSERT INTO test VALUES (:id1, $name); INSERT INTO test VALUES ($id2, :name) ]]))
  assert( stmt:bind_values("Hoho", 4, 5) )
  assert( stmt:exec() )
  st.check_content{ "Hello World", "Hello Lua", "Hello sqlite3", "Hoho", "Hoho" }
end
]===]

function st.test_bind_by_names()
  local stmt = assert_userdata( st.db:prepare("INSERT INTO test VALUES (:id, :name)")  )
  local args = { }
  args.id = 5
  args.name = "Hello girls"
  assert( stmt:bind_names(args) )
  assert_number( stmt:step() )
  assert_number( stmt:reset() )
  args.id = 4
  args.name = "Hello boys"
  assert( stmt:bind_names(args) )
  assert_number( stmt:step() )
  assert_number( stmt:reset() )
  st.check_content{ "Hello World", "Hello Lua", "Hello sqlite3",  "Hello boys", "Hello girls" }
  assert_number( stmt:finalize() )
end

function st.test_last_insert_rowid()
  local stmt = assert_userdata( st.db:prepare("INSERT INTO test VALUES (:id, :name)")  )
  local args = { }
  args.id = 5
  args.name = "Hello girls"
  assert( stmt:bind_names(args) )
  assert_number( stmt:step() )
  assert_number( stmt:reset() )
  assert_number( stmt:last_insert_rowid() )
  assert_equal( stmt:last_insert_rowid(), 4 )
  args.id = 4
  args.name = "Hello boys"
  assert( stmt:bind_names(args) )
  assert_number( stmt:step() )
  assert_number( stmt:reset() )
  assert_number( stmt:last_insert_rowid() )
  assert_equal( stmt:last_insert_rowid(), 5 )
  st.check_content{ "Hello World", "Hello Lua", "Hello sqlite3",  "Hello boys", "Hello girls" }
  assert_number( stmt:finalize() )
end



--------------------------------
-- Tests binding of arguments --
--------------------------------

b = lunit_TestCase("Binding Tests")

function b.setup()
  b.db = assert( sqlite3.open_memory() )
  assert_number( b.db:exec("CREATE TABLE test (id, name, u, v, w, x, y, z)") )
end

function b.teardown()
  assert_number( b.db:close() )
end

function b.test_auto_parameter_names()
  local stmt = assert_userdata( b.db:prepare("INSERT INTO test VALUES(:a, $b, :a2, :b2, $a, :b, $a3, $b3)") )
  local parameters = assert_number( stmt:bind_parameter_count() )
  assert_equal( 8, parameters )
  assert_equal( ":a", stmt:bind_parameter_name(1) )
  assert_equal( "$b", stmt:bind_parameter_name(2) )
  assert_equal( ":a2", stmt:bind_parameter_name(3) )
  assert_equal( ":b2", stmt:bind_parameter_name(4) )
  assert_equal( "$a", stmt:bind_parameter_name(5) )
  assert_equal( ":b", stmt:bind_parameter_name(6) )
  assert_equal( "$a3", stmt:bind_parameter_name(7) )
  assert_equal( "$b3", stmt:bind_parameter_name(8) )
end

function b.test_auto_parameter_names()
  local stmt = assert_userdata( b.db:prepare("INSERT INTO test VALUES($a, $b, $a2, $b2, $a, $b, $a3, $b3)") )
  local parameters = assert_number( stmt:bind_parameter_count() )
  assert_equal( 6, parameters )
  assert_equal( "$a", stmt:bind_parameter_name(1) )
  assert_equal( "$b", stmt:bind_parameter_name(2) )
  assert_equal( "$a2", stmt:bind_parameter_name(3) )
  assert_equal( "$b2", stmt:bind_parameter_name(4) )
  assert_equal( "$a3", stmt:bind_parameter_name(5) )
  assert_equal( "$b3", stmt:bind_parameter_name(6) )
end

function b.test_no_parameter_names_1()
  local stmt = assert_userdata( b.db:prepare([[ SELECT * FROM test ]]))
  local parameters = assert_number( stmt:bind_parameter_count() )
  assert_equal( 0, (parameters) )
end

function b.test_no_parameter_names_2()
  local stmt = assert_userdata( b.db:prepare([[ INSERT INTO test VALUES(?, ?, ?, ?, ?, ?, ?, ?) ]]))
  local parameters = assert_number( stmt:bind_parameter_count() )
  assert_equal( 8, (parameters) )
  assert_nil( stmt:bind_parameter_name(1) )
end




----------------------------
-- Tests for update_hook  --
----------------------------

uh = lunit_TestCase("Update Hook")

function uh.setup()
  uh.db = assert( sqlite3.open_memory() )
  uh.udtbl = {[sqlite3.INSERT]=0, [sqlite3.UPDATE]=0, [sqlite3.DELETE]=0}
  uh.crtbl = {0, 0}
  assert_nil(uh.db:update_hook( function(ud, op, dname, tname, rowid)
    --print("Sqlite Update Hook:", op, dname, tname, rowid)
    ud[op] = ud[op] + 1
  end, uh.udtbl))
  uh.uttblsz = function () local sz = 0; for _,_ in pairs(uh.udtbl) do sz = sz + 1 end return sz end
  -- enable foreign keys!
  assert_number( uh.db:exec("PRAGMA foreign_keys = ON;") )
  assert_number( uh.db:exec("CREATE TABLE test ( id INTEGER PRIMARY KEY, content VARCHAR );") )
  assert_number( uh.db:exec("CREATE TABLE T1 ( id INTEGER PRIMARY KEY, content VARCHAR );") )
  assert_number( uh.db:exec("CREATE TABLE T2 ( id INTEGER PRIMARY KEY, content VARCHAR );") )
end

function uh.teardown()
  --for k,v in pairs(uh.udtbl) do print(k,v) end
  assert_equal( 3, uh.uttblsz() )
  assert_number( uh.db:close() )
end

function uh.test_insert1()
  assert_equal( sqlite3.OK, uh.db:exec("INSERT INTO test VALUES (NULL, 'Hello World');") )
  assert_equal( 1, uh.udtbl[sqlite3.INSERT] )
end

function uh.test_insert3()
  assert_equal( sqlite3.OK, uh.db:exec("INSERT INTO test VALUES (NULL, 'Hello World');") )
  assert_equal( sqlite3.OK, uh.db:exec("INSERT INTO test VALUES (NULL, 'Hello Lua');") )
  assert_equal( sqlite3.OK, uh.db:exec("INSERT INTO test VALUES (NULL, 'Hello Sqlite3');") )
  assert_equal( 3, uh.udtbl[sqlite3.INSERT] )
  assert_equal( 0, uh.udtbl[sqlite3.UPDATE] )
  assert_equal( 0, uh.udtbl[sqlite3.DELETE] )

end

function uh.test_insert3_update1()
  assert_equal( sqlite3.OK, uh.db:exec("INSERT INTO test VALUES (NULL, 'Hello World');") )
  assert_equal( sqlite3.OK, uh.db:exec("INSERT INTO test VALUES (NULL, 'Hello Lua');") )
  assert_equal( sqlite3.OK, uh.db:exec("INSERT INTO test VALUES (NULL, 'Hello Sqlite3');") )
  assert_equal( sqlite3.OK, uh.db:exec("UPDATE test SET content = 'Hello Again World' WHERE id = 1;") )
  assert_equal( 3, uh.udtbl[sqlite3.INSERT] )
  assert_equal( 1, uh.udtbl[sqlite3.UPDATE] )
  assert_equal( 0, uh.udtbl[sqlite3.DELETE] )
end

function uh.test_insert3_delete1()
  assert_equal( sqlite3.OK, uh.db:exec("INSERT INTO test VALUES (NULL, 'Hello World');") )
  assert_equal( sqlite3.OK, uh.db:exec("INSERT INTO test VALUES (NULL, 'Hello Lua');") )
  assert_equal( sqlite3.OK, uh.db:exec("INSERT INTO test VALUES (NULL, 'Hello Sqlite3');") )
  assert_equal( sqlite3.OK, uh.db:exec("DELETE FROM test WHERE id = 2;") )
  assert_equal( 3, uh.udtbl[sqlite3.INSERT] )
  assert_equal( 0, uh.udtbl[sqlite3.UPDATE] )
  assert_equal( 1, uh.udtbl[sqlite3.DELETE] )
end

function uh.test_insert3_update1_delete1()
  assert_equal( sqlite3.OK, uh.db:exec("INSERT INTO test VALUES (NULL, 'Hello World');") )
  assert_equal( sqlite3.OK, uh.db:exec("INSERT INTO test VALUES (NULL, 'Hello Lua');") )
  assert_equal( sqlite3.OK, uh.db:exec("INSERT INTO test VALUES (NULL, 'Hello Sqlite3');") )
  assert_equal( sqlite3.OK, uh.db:exec("UPDATE test SET content = 'Hello Again World' WHERE id = 1;") )
  assert_equal( sqlite3.OK, uh.db:exec("DELETE FROM test WHERE id = 2;") )
  assert_equal( 3, uh.udtbl[sqlite3.INSERT] )
  assert_equal( 1, uh.udtbl[sqlite3.UPDATE] )
  assert_equal( 1, uh.udtbl[sqlite3.DELETE] )
end

function uh.test_insert_select()
  assert_equal( sqlite3.OK, uh.db:exec("INSERT INTO T1 VALUES (NULL, 'Hello World');") )
  assert_equal( sqlite3.OK, uh.db:exec("INSERT INTO T1 VALUES (NULL, 'Hello Lua');") )
  assert_equal( sqlite3.OK, uh.db:exec("INSERT INTO T1 VALUES (NULL, 'Hello Sqlite3');") )
  assert_equal( sqlite3.OK, uh.db:exec("INSERT INTO T2 SELECT * FROM T1;") )
  assert_equal( 6, uh.udtbl[sqlite3.INSERT] )
  assert_equal( 0, uh.udtbl[sqlite3.UPDATE] )
  assert_equal( 0, uh.udtbl[sqlite3.DELETE] )
end

function uh.test_trigger_insert()
  assert_equal( sqlite3.OK, uh.db:exec([[
    CREATE TRIGGER after_insert_T1
    AFTER INSERT ON T1
    FOR EACH ROW
    BEGIN
      INSERT INTO T2 VALUES(NEW.id, NEW.content);
    END;
  ]]) )
  assert_equal( sqlite3.OK, uh.db:exec("INSERT INTO T1 VALUES (NULL, 'Hello World');") )
  assert_equal( sqlite3.OK, uh.db:exec("INSERT INTO T1 VALUES (NULL, 'Hello Lua');") )
  assert_equal( sqlite3.OK, uh.db:exec("INSERT INTO T1 VALUES (NULL, 'Hello Sqlite3');") )
  assert_equal( 6, uh.udtbl[sqlite3.INSERT] )
  assert_equal( 0, uh.udtbl[sqlite3.UPDATE] )
  assert_equal( 0, uh.udtbl[sqlite3.DELETE] )
end

function uh.test_cascade_update()
  assert_equal( sqlite3.OK, uh.db:exec("DROP TABLE T2;") )
  assert_equal( sqlite3.OK, uh.db:exec([[
    CREATE TABLE T2 ( id INTEGER PRIMARY KEY REFERENCES T1 ON UPDATE CASCADE, content VARCHAR );
  ]]) )

  assert_equal( sqlite3.OK, uh.db:exec("INSERT INTO T1 VALUES (NULL, 'a');") )
  assert_equal( sqlite3.OK, uh.db:exec("INSERT INTO T1 VALUES (NULL, 'b');") )
  assert_equal( sqlite3.OK, uh.db:exec("INSERT INTO T1 VALUES (NULL, 'c');") )
  assert_equal( sqlite3.OK, uh.db:exec("INSERT INTO T1 VALUES (NULL, 'd');") )
  assert_equal( sqlite3.OK, uh.db:exec("INSERT INTO T2 SELECT * FROM T1;") )
  assert_equal( sqlite3.OK, uh.db:exec("UPDATE T1 SET id = id + 10 WHERE id < 3;") )

  assert_equal( 8, uh.udtbl[sqlite3.INSERT] )
  assert_equal( 4, uh.udtbl[sqlite3.UPDATE] )
  assert_equal( 0, uh.udtbl[sqlite3.DELETE] )
end

function uh.test_cascade_update_restrict()
  assert_equal( sqlite3.OK, uh.db:exec("DROP TABLE T2;") )
  assert_equal( sqlite3.OK, uh.db:exec([[
    CREATE TABLE T2 ( id INTEGER PRIMARY KEY REFERENCES T1 ON UPDATE RESTRICT, content VARCHAR );
  ]]) )

  assert_equal( sqlite3.OK, uh.db:exec("INSERT INTO T1 VALUES (NULL, 'a');") )
  assert_equal( sqlite3.OK, uh.db:exec("INSERT INTO T1 VALUES (NULL, 'b');") )
  assert_equal( sqlite3.OK, uh.db:exec("INSERT INTO T1 VALUES (NULL, 'c');") )
  assert_equal( sqlite3.OK, uh.db:exec("INSERT INTO T1 VALUES (NULL, 'd');") )
  assert_equal( sqlite3.OK, uh.db:exec("INSERT INTO T2 SELECT * FROM T1;") )
  -- the update_hook with rowid=11 *DOES* get triggered before the RESTRICT constraint is enforced
  assert_equal( sqlite3.CONSTRAINT, uh.db:exec("UPDATE T1 SET id = id + 10 WHERE id < 3;") )

  assert_equal( 8, uh.udtbl[sqlite3.INSERT] )
  assert_equal( 1, uh.udtbl[sqlite3.UPDATE] )
  assert_equal( 0, uh.udtbl[sqlite3.DELETE] )
end

function uh.test_cascade_delete_restrict()
  assert_equal( sqlite3.OK, uh.db:exec("DROP TABLE T2;") )
  assert_equal( sqlite3.OK, uh.db:exec([[
    CREATE TABLE T2 ( id INTEGER PRIMARY KEY REFERENCES T1 ON DELETE RESTRICT, content VARCHAR );
  ]]) )

  assert_equal( sqlite3.OK, uh.db:exec("INSERT INTO T1 VALUES (NULL, 'a');") )
  assert_equal( sqlite3.OK, uh.db:exec("INSERT INTO T1 VALUES (NULL, 'b');") )
  assert_equal( sqlite3.OK, uh.db:exec("INSERT INTO T1 VALUES (NULL, 'c');") )
  assert_equal( sqlite3.OK, uh.db:exec("INSERT INTO T1 VALUES (NULL, 'd');") )
  assert_equal( sqlite3.OK, uh.db:exec("INSERT INTO T2 SELECT * FROM T1;") )
  -- the update_hook with rowid=1 *DOES* get triggered before the RESTRICT constraint is enforced
  assert_equal( sqlite3.CONSTRAINT, uh.db:exec("DELETE FROM T1 WHERE id < 3;") )

  assert_equal( 8, uh.udtbl[sqlite3.INSERT] )
  assert_equal( 0, uh.udtbl[sqlite3.UPDATE] )
  assert_equal( 1, uh.udtbl[sqlite3.DELETE] )
end

function uh.test_fk_violate_insert()
  assert_equal( sqlite3.OK, uh.db:exec("DROP TABLE T2;") )
  assert_equal( sqlite3.OK, uh.db:exec([[
    CREATE TABLE T2 ( id INTEGER PRIMARY KEY REFERENCES T1, content VARCHAR );
  ]]) )

  assert_equal( sqlite3.OK, uh.db:exec("INSERT INTO T1 VALUES (NULL, 'a');") )
  assert_equal( sqlite3.OK, uh.db:exec("INSERT INTO T1 VALUES (NULL, 'b');") )
  -- no update hook triggered here
  assert_equal( sqlite3.CONSTRAINT, uh.db:exec("INSERT INTO T2 VALUES(99, 'xxx')") )

  assert_equal( 2, uh.udtbl[sqlite3.INSERT] )
  assert_equal( 0, uh.udtbl[sqlite3.UPDATE] )
  assert_equal( 0, uh.udtbl[sqlite3.DELETE] )
end

function uh.test_fk_violate_update()
  assert_equal( sqlite3.OK, uh.db:exec("DROP TABLE T2;") )
  assert_equal( sqlite3.OK, uh.db:exec([[
    CREATE TABLE T2 ( id INTEGER PRIMARY KEY REFERENCES T1, content VARCHAR );
  ]]) )

  assert_equal( sqlite3.OK, uh.db:exec("INSERT INTO T1 VALUES (NULL, 'a');") )
  assert_equal( sqlite3.OK, uh.db:exec("INSERT INTO T1 VALUES (NULL, 'b');") )
  assert_equal( sqlite3.OK, uh.db:exec("INSERT INTO T2 VALUES (1, 'a');") )
  -- update hook triggered
  assert_equal( sqlite3.CONSTRAINT, uh.db:exec("UPDATE T2 SET id = 99 WHERE id = 1;") )

  assert_equal( 3, uh.udtbl[sqlite3.INSERT] )
  assert_equal( 1, uh.udtbl[sqlite3.UPDATE] )
  assert_equal( 0, uh.udtbl[sqlite3.DELETE] )
end

----------------------------------------------
-- Tests for commit_hook and rollback_hook  --
----------------------------------------------

crh = lunit_TestCase("Commit/Rollback Hook")

function crh.setup()
  crh.db = assert( sqlite3.open_memory() )
  crh.crtbl = {0, 0}
  assert_number( crh.db:exec("PRAGMA foreign_keys = ON;") )
  assert_number( crh.db:exec("CREATE TABLE T1 ( id INTEGER PRIMARY KEY, content VARCHAR );") )
  assert_number( crh.db:exec("CREATE TABLE T2 ( id INTEGER PRIMARY KEY REFERENCES T1, content VARCHAR );") )

  -- reset commit hook return value
  crh.commit_hook_returnvalue = false

  assert_nil( crh.db:commit_hook(function (ud)
    -- print('Commit hook')
    ud[1] = ud[1] + 1;
    return crh.commit_hook_returnvalue
  end, crh.crtbl))

  assert_nil( crh.db:rollback_hook(function (ud)
    -- print('Rollback hook')
    ud[2] = ud[2] + 1
  end, crh.crtbl))
end

function crh.teardown()
  assert_number( crh.db:close() )
end

function crh.test_simple_transaction_commit()
  assert_equal( sqlite3.OK, crh.db:exec("BEGIN;") )
  assert_equal( sqlite3.OK, crh.db:exec("INSERT INTO T1 VALUES (NULL, 'Hello World');") )
  assert_equal( sqlite3.OK, crh.db:exec("UPDATE T1 SET content = 'Hello Again World' WHERE id = 1;") )
  assert_equal( sqlite3.OK, crh.db:exec("DELETE FROM T1 WHERE id = 2;") )
  assert_equal( sqlite3.OK, crh.db:exec("COMMIT;") )
  assert_equal( 1, crh.crtbl[1] )
  assert_equal( 0, crh.crtbl[2] )
end

function crh.test_simple_transaction_prohibit()
  -- if the commit hook returns anything except false or nil, the commit would turn into rollback
  crh.commit_hook_returnvalue = 1

  assert_equal( sqlite3.OK, crh.db:exec("BEGIN;") )
  assert_equal( sqlite3.OK, crh.db:exec("INSERT INTO T1 VALUES (NULL, 'Hello World');") )
  assert_equal( sqlite3.OK, crh.db:exec("UPDATE T1 SET content = 'Hello Again World' WHERE id = 1;") )
  assert_equal( sqlite3.OK, crh.db:exec("DELETE FROM T1 WHERE id = 2;") )
  assert_equal( sqlite3.CONSTRAINT, crh.db:exec("COMMIT;") )
  -- commit hook gets called and returns 1 triggering a rollback
  assert_equal( 1, crh.crtbl[1] )
  assert_equal( 1, crh.crtbl[2] )
end

function crh.test_simple_transaction_rollback()
  assert_equal( sqlite3.OK, crh.db:exec("BEGIN;") )
  assert_equal( sqlite3.OK, crh.db:exec("INSERT INTO T1 VALUES (NULL, 'Hello World');") )
  assert_equal( sqlite3.OK, crh.db:exec("UPDATE T1 SET content = 'Hello Again World' WHERE id = 1;") )
  assert_equal( sqlite3.OK, crh.db:exec("DELETE FROM T1 WHERE id = 2;") )
  assert_equal( sqlite3.OK, crh.db:exec("ROLLBACK;") )
  assert_equal( 0, crh.crtbl[1] )
  assert_equal( 1, crh.crtbl[2] )
end

function crh.test_statement_level_transaction()
  assert_equal( sqlite3.OK, crh.db:exec("INSERT INTO T1 VALUES (NULL, 'Hello World');") )
  assert_equal( sqlite3.OK, crh.db:exec("UPDATE T1 SET content = 'Hello Again World' WHERE id = 1;") )
  assert_equal( sqlite3.OK, crh.db:exec("DELETE FROM T1 WHERE id = 2;") )
  assert_equal( 3, crh.crtbl[1] )
  assert_equal( 0, crh.crtbl[2] )
end

function crh.test_statement_level_fk_violate_insert()
  assert_equal( sqlite3.OK, crh.db:exec("INSERT INTO T1 VALUES (NULL, 'a');") )
  assert_equal( sqlite3.OK, crh.db:exec("INSERT INTO T1 VALUES (NULL, 'b');") )
  -- implicit statement-level transaction rollback + error
  assert_equal( sqlite3.CONSTRAINT, crh.db:exec("INSERT INTO T2 VALUES(99, 'xxx')") )
  assert_equal( 2, crh.crtbl[1] )
  assert_equal( 1, crh.crtbl[2] )
end

function crh.test_transaction_fk_violate_update()
  assert_equal( sqlite3.OK, crh.db:exec("BEGIN;") )
  assert_equal( sqlite3.OK, crh.db:exec("INSERT INTO T1 VALUES (NULL, 'Hello World');") )
  assert_equal( sqlite3.OK, crh.db:exec("INSERT INTO T2 VALUES (1, 'xyz');") )
  -- Doesn't trigger rollback hook because the implicit update statement transaction
  -- is nested inside our explicit transaction. However we *do* get an error.
  assert_equal( sqlite3.CONSTRAINT, crh.db:exec("UPDATE T2 SET id = 99 WHERE id = 1;") )
  -- rollback explicitly
  assert_equal( sqlite3.OK, crh.db:exec("ROLLBACK;") )

  assert_equal( 0, crh.crtbl[1] )
  assert_equal( 1, crh.crtbl[2] )
end

function crh.test_savepoint_nested_commit()
  assert_equal( sqlite3.OK, crh.db:exec("SAVEPOINT S1;") )
  assert_equal( sqlite3.OK, crh.db:exec(" INSERT INTO T1 VALUES (NULL, 'Hello World');") )
  assert_equal( sqlite3.OK, crh.db:exec(" INSERT INTO T1 VALUES (NULL, 'Hello Lua');") )
  assert_equal( sqlite3.OK, crh.db:exec(" SAVEPOINT S2;") )
  assert_equal( sqlite3.OK, crh.db:exec("  INSERT INTO T1 VALUES (NULL, 'Hello Sqlite3');") )
  -- nested transactions don't trigger commit/rollback hooks
  assert_equal( sqlite3.OK, crh.db:exec(" RELEASE S2;") )
  assert_equal( sqlite3.OK, crh.db:exec("RELEASE S1;") )
  assert_equal( 1, crh.crtbl[1] )
  assert_equal( 0, crh.crtbl[2] )
end

function crh.test_savepoint_nested_rollback()
  assert_equal( sqlite3.OK, crh.db:exec("SAVEPOINT S1;") )
  assert_equal( sqlite3.OK, crh.db:exec(" INSERT INTO T1 VALUES (NULL, 'Hello World');") )
  assert_equal( sqlite3.OK, crh.db:exec(" INSERT INTO T1 VALUES (NULL, 'Hello Lua');") )
  assert_equal( sqlite3.OK, crh.db:exec(" SAVEPOINT S2;") )
  assert_equal( sqlite3.OK, crh.db:exec("  INSERT INTO T1 VALUES (NULL, 'Hello Sqlite3');") )
  -- nested transactions don't trigger commit/rollback hooks
  assert_equal( sqlite3.OK, crh.db:exec(" ROLLBACK TO S2;") )
  assert_equal( sqlite3.OK, crh.db:exec("RELEASE S1;") )
  assert_equal( 1, crh.crtbl[1] )
  assert_equal( 0, crh.crtbl[2] )
end


--------------------------------------------
-- Tests loop break and statement reusage --
--------------------------------------------


----------------------------------------------------
-- Test garbage collected db with live statements --
----------------------------------------------------

gco = lunit_TestCase("Bug-Report: GC'd db with live prepared statements")

function gco.setup()
  gco.db = assert( sqlite3.open_memory() )
  assert_equal( sqlite3.OK, gco.db:exec("CREATE TABLE test (id, name)") )
  assert_equal( sqlite3.OK, gco.db:exec("INSERT INTO test VALUES (1, 'Hello World')") )
  assert_equal( sqlite3.OK, gco.db:exec("INSERT INTO test VALUES (2, 'Hello Lua')") )
  assert_equal( sqlite3.OK, gco.db:exec("INSERT INTO test VALUES (3, 'Hello sqlite3')") )
end

-- gco.db:close()
function gco.test_gc()
  local stmt = assert_userdata( gco.db:prepare("INSERT INTO test VALUES (?, ?)")  )
  assert_number( stmt:bind_values(4, "Good morning") )
  assert_number( stmt:step() )
  assert_number( stmt:reset() )
  --st.check_content{ "Hello World", "Hello Lua", "Hello sqlite3", "Good morning" }
  gco.db = nil -- reap the db if unreferenced from stmt
  collectgarbage()
  assert_number( stmt:bind_values(5, "Foo Bar") )
  assert_number( stmt:step() )
  assert_number( stmt:reset() )
  --st.check_content{ "Hello World", "Hello Lua", "Hello sqlite3", "Good morning", "Foo Bar" }
  assert_number( stmt:finalize() )
end

function gco.teardown()
  assert_true( (gco.db == nil) or (gco.db:close() == sqlite3.OK) )
end


if _VERSION >= 'Lua 5.3' then

-- this hack is avoid syntax errors in Lua < 5.3
local bitand = (load "return function (x, y) return x & y end")()
local bitshr = (load "return function (x, y) return x >> y end")()

l53 = lunit_TestCase("Lua 5.3 integers")

function l53.setup()
  l53.db = assert( sqlite3.open_memory() )
  assert_equal( sqlite3.OK, l53.db:exec("CREATE TABLE test (id, val)") )
  assert_equal( sqlite3.OK, l53.db:exec("INSERT INTO test VALUES (1, 0x12345678abcdef09)") )
  assert_equal( sqlite3.OK, l53.db:exec("INSERT INTO test VALUES (2, 5.1234567890123456)") )
  assert_equal( sqlite3.OK, l53.db:exec("INSERT INTO test VALUES (3, 42)") )
end

-- l53.db:close()
function l53.test_intfloat()
    local db = l53.db
    for row in db:nrows("SELECT val FROM test WHERE id = 1") do
      assert_equal (row.val, 0x12345678abcdef09)
      assert_equal (bitand(row.val, 0xffffffff), 0xabcdef09)
      assert_equal (bitshr(row.val, 32), 0x12345678)
    end
    for row in db:nrows("SELECT val FROM test WHERE id = 2") do
      assert_equal (row.val, 5.1234567890123456)
    end
    for row in db:nrows("SELECT val FROM test WHERE id = 3") do
      assert_equal (row.val, 42)
    end
end

function l53.teardown()
  assert_true( (l53.db == nil) or (l53.db:close() == sqlite3.OK) )
end

end

----------------------------
-- Test for bugs reported --
----------------------------

bug = lunit_TestCase("Bug-Report Tests")

function bug.setup()
  bug.db = assert( sqlite3.open_memory() )
end

function bug.teardown()
  assert_number( bug.db:close() )
end

--[===[
function bug.test_1()
  bug.db:exec("CREATE TABLE test (id INTEGER PRIMARY KEY, value TEXT)")

  local query = assert_userdata( bug.db:prepare("SELECT id FROM test WHERE value=?") )

  assert_table ( query:bind_values("1") )
  assert_nil   ( query:first_cols() )
  assert_table ( query:bind_values("2") )
  assert_nil   ( query:first_cols() )
end
]===]

function bug.test_nils()   -- appeared in lua-5.1 (holes in arrays)
  local function check(arg1, arg2, arg3, arg4, arg5)
    assert_equal(1, arg1)
    assert_equal(2, arg2)
    assert_nil(arg3)
    assert_equal(4, arg4)
    assert_nil(arg5)
  end

  bug.db:create_function("test_nils", 5, function(arg1, arg2, arg3, arg4, arg5)
    check(arg1, arg2, arg3, arg4, arg5)
  end, {})

  assert_number( bug.db:exec([[ SELECT test_nils(1, 2, NULL, 4, NULL) ]]) )

  for arg1, arg2, arg3, arg4, arg5 in bug.db:urows([[ SELECT 1, 2, NULL, 4, NULL ]])
  do check(arg1, arg2, arg3, arg4, arg5)
  end

  for row in bug.db:rows([[ SELECT 1, 2, NULL, 4, NULL ]])
  do assert_table( row )
     check(row[1], row[2], row[3], row[4], row[5])
  end
end

----------------------------
-- Test for collation fun --
----------------------------

colla = lunit_TestCase("Collation Tests")

function colla.setup()
    local function collate(s1,s2)
        -- if p then print("collation callback: ",s1,s2) end
        s1=s1:lower()
        s2=s2:lower()
        if s1==s2 then return 0
        elseif s1<s2 then return -1
        else return 1 end
    end
    colla.db = assert( sqlite3.open_memory() )
    assert_nil(colla.db:create_collation('CINSENS',collate))
    colla.db:exec[[
      CREATE TABLE test(id INTEGER PRIMARY KEY,content COLLATE CINSENS);
      INSERT INTO test VALUES(NULL,'hello world');
      INSERT INTO test VALUES(NULL,'Buenos dias');
      INSERT INTO test VALUES(NULL,'HELLO WORLD');
      INSERT INTO test VALUES(NULL,'Guten Tag');
      INSERT INTO test VALUES(NULL,'HeLlO WoRlD');
      INSERT INTO test VALUES(NULL,'Bye for now');
    ]]
end

function colla.teardown()
  assert_number( colla.db:close() )
end

function colla.test()
    --for row in db:nrows('SELECT * FROM test') do
    --  print(row.id,row.content)
    --end
    local n = 0
    for row in colla.db:nrows('SELECT * FROM test WHERE content="hElLo wOrLd"') do
      -- print(row.id,row.content)
      assert_equal (row.content:lower(), "hello world")
      n = n + 1
    end
    assert_equal (n, 3)
end

--------------------------------------
-- Tests for NULs in BLOBs and TEXT --
--------------------------------------

local db_blobNULs = lunit_TestCase("Blob NULs")

function db_blobNULs.setup()
  db_blobNULs.db = assert( sqlite3.open_memory() )
  assert_equal( sqlite3.OK, db_blobNULs.db:exec("CREATE TABLE test (id, name blob)") )
  assert_equal( sqlite3.OK, db_blobNULs.db:exec("INSERT INTO test VALUES (1, CAST('Hello' || x'00' || 'World' || x'00' AS BLOB))") )
  assert_equal( sqlite3.OK, db_blobNULs.db:exec("INSERT INTO test VALUES (2, CAST('Hello' || x'00' || 'Lua' || x'00' AS BLOB))") )
  assert_equal( sqlite3.OK, db_blobNULs.db:exec("INSERT INTO test VALUES (3, CAST('Hello' || x'00' || 'SQLite' || x'00' AS BLOB))") )
end

function db_blobNULs.teardown()
  assert( db_blobNULs.db:close() )
end

function db_blobNULs.test()
    local db = db_blobNULs.db
    for row in db:nrows("SELECT id as val FROM test WHERE name=CAST('Hello' || x'00' || 'World' || x'00' AS BLOB)") do
      assert_equal (row.val, 1)
    end
    for row in db:nrows("SELECT substr(name,7,4) as val FROM test WHERE id = 2") do
      assert_equal (row.val,'Lua\0')
      assert_equal (#row.val, 4)
    end
    for row in db:nrows("SELECT name as val FROM test WHERE id = 3") do
      assert_equal (row.val, 'Hello\0SQLite\0')
      assert_equal (#row.val, 13)
    end
end

local db_textNULs = lunit_TestCase("Text NULs")

function db_textNULs.setup()
  db_textNULs.db = assert( sqlite3.open_memory() )
  assert_equal( sqlite3.OK, db_textNULs.db:exec("CREATE TABLE test (id, name text)") )
  assert_equal( sqlite3.OK, db_textNULs.db:exec("INSERT INTO test VALUES (1, CAST('Hello' || x'00' || 'World' || x'00' AS TEXT))") )
  assert_equal( sqlite3.OK, db_textNULs.db:exec("INSERT INTO test VALUES (2, CAST('Hello' || x'00' || 'Lua' || x'00' AS TEXT))") )
  assert_equal( sqlite3.OK, db_textNULs.db:exec("INSERT INTO test VALUES (3, CAST('Hello' || x'00' || 'SQLite' || x'00' AS TEXT))") )
end

function db_textNULs.teardown()
  assert( db_textNULs.db:close() )
end

function db_textNULs.test()
    local db = db_textNULs.db
    for row in db:nrows("SELECT id as val FROM test WHERE name=CAST('Hello' || x'00' || 'World' || x'00' AS TEXT)") do
      assert_equal (row.val, 1)
    end
    for row in db:nrows("SELECT substr(CAST(name AS BLOB),7,4) as val FROM test WHERE id = 2") do
      assert_equal (row.val,'Lua\0')
      assert_equal (#row.val, 4)
    end
    for row in db:nrows("SELECT name as val FROM test WHERE id = 3") do
      assert_equal (row.val, 'Hello\0SQLite\0')
      assert_equal (#row.val, 13)
    end
end

-------------------------------------
--   Tests for Online Backup API   --
-------------------------------------

local db_bu = lunit_TestCase("Online Backup API")

function db_bu.setup()
  db_bu.db_src = assert( sqlite3.open_memory() )
  assert_equal( sqlite3.OK, db_bu.db_src:exec("CREATE TABLE test (id, name text)") )
  assert_equal( sqlite3.OK, db_bu.db_src:exec("INSERT INTO test VALUES (1, 'Hello World')") )
  assert_equal( sqlite3.OK, db_bu.db_src:exec("INSERT INTO test VALUES (2, 'Hello Lua')") )
  assert_equal( sqlite3.OK, db_bu.db_src:exec("INSERT INTO test VALUES (3, 'Hello SQLite')") )
  db_bu.filename = "/tmp/__lua-sqlite3-20161102233049." .. os.time()
  db_bu.db_tgt = assert_userdata( sqlite3.open(db_bu.filename) )
end

function db_bu.teardown()
  assert( db_bu.db_src:close() )
  assert( db_bu.db_tgt:close() )
  os.remove(db_bu.filename)
end

function db_bu.test()

  assert_function( sqlite3.backup_init )

  local bu = assert_userdata( sqlite3.backup_init(db_bu.db_tgt, 'main', db_bu.db_src, 'main') )

  assert_function ( bu.step )
  assert_function ( bu.remaining )
  assert_function ( bu.pagecount )
  assert_function ( bu.finish )

  if true then
    bu = nil
    collectgarbage()
    collectgarbage()
  else
    bu:finish()
  end

  bu = assert_userdata( sqlite3.backup_init(db_bu.db_tgt, 'main', db_bu.db_src, 'main') )

  assert_equal( sqlite3.DONE, bu:step(-1) )
  assert_equal( sqlite3.OK, bu:finish() )
  bu = nil

  local db = db_bu.db_tgt
  for row in db:nrows("SELECT id as val FROM test WHERE name='Hello World'") do
    assert_equal (row.val, 1)
  end
  for row in db:nrows("SELECT substr(name,7,3) as val FROM test WHERE id = 2") do
    assert_equal (row.val,'Lua')
    assert_equal (#row.val, 3)
  end
  for row in db:nrows("SELECT name as val FROM test WHERE id = 3") do
    assert_equal (row.val, 'Hello SQLite')
    assert_equal (#row.val, 12)
  end

end

local db_bu_gc = lunit_TestCase("Online Backup API GC")

function db_bu_gc.setup()
  db_bu_gc.db_src = assert( sqlite3.open_memory() )
  assert_equal( sqlite3.OK, db_bu_gc.db_src:exec("CREATE TABLE test (id, name text)") )
  assert_equal( sqlite3.OK, db_bu_gc.db_src:exec("INSERT INTO test VALUES (1, 'Hello World')") )
  assert_equal( sqlite3.OK, db_bu_gc.db_src:exec("INSERT INTO test VALUES (2, 'Hello Lua')") )
  assert_equal( sqlite3.OK, db_bu_gc.db_src:exec("INSERT INTO test VALUES (3, 'Hello SQLite')") )
  db_bu_gc.filename = "/tmp/__lua-sqlite3-20161103120909." .. os.time()
  db_bu_gc.db_tgt = assert_userdata( sqlite3.open(db_bu_gc.filename) )
end

function db_bu_gc.teardown()
  os.remove(db_bu_gc.filename)
end

function db_bu_gc.test()

  local bu = assert_userdata( sqlite3.backup_init(db_bu_gc.db_tgt, 'main', db_bu_gc.db_src, 'main') )

  db_bu_gc.db_src = nil
  db_bu_gc.db_tgt = nil

  collectgarbage()
  collectgarbage() -- should not close dbs even though db_bu_gc refs nil'd since they are referenced by bu

  assert_equal( sqlite3.DONE, bu:step(-1) )
  assert_equal( sqlite3.OK, bu:finish() )
  bu = nil

  collectgarbage()
  collectgarbage() -- should now close dbs

  local db = assert_userdata( sqlite3.open(db_bu_gc.filename) )

  for row in db:nrows("SELECT id as val FROM test WHERE name='Hello World'") do
    assert_equal (row.val, 1)
  end
  for row in db:nrows("SELECT substr(name,7,3) as val FROM test WHERE id = 2") do
    assert_equal (row.val,'Lua')
    assert_equal (#row.val, 3)
  end
  for row in db:nrows("SELECT name as val FROM test WHERE id = 3") do
    assert_equal (row.val, 'Hello SQLite')
    assert_equal (#row.val, 12)
  end

  assert( db:close() )

end

local db_bu_null = lunit_TestCase("Online Backup API NULL")

function db_bu_null.setup()
  db_bu_null.db = assert( sqlite3.open_memory() )
end

function db_bu_null.teardown()
  assert( db_bu_null.db:close() )
end

function db_bu_null.test()

  local bu = assert_nil( sqlite3.backup_init(db_bu_null.db, 'main', db_bu_null.db, 'main') )

end

--------------------------------------
-- Functions added 2016-11-xx 0.9.4 --
--------------------------------------

r094 = lunit_TestCase("Functions added 0.9.4")

function r094.setup()
  r094.db = assert( sqlite3.open_memory() )
  r094.filename = "/tmp/__lua-sqlite3-20161112163049." .. os.time()
  r094.db_fn = assert_userdata( sqlite3.open(r094.filename) )
end

function r094.teardown()
  assert_number( r094.db:close() )
  assert_number( r094.db_fn:close() )
end

function r094.test_db_filename()

  assert_nil( r094.db:db_filename("frob") )
  assert_equal( '', r094.db:db_filename("main") )

  assert_nil( r094.db_fn:db_filename("frob") )
  assert_equal( r094.filename, r094.db_fn:db_filename("main") )

  -- from Wolfgang Oertl
  local db_ptr = assert_userdata( r094.db:get_ptr() )
  local db2    = assert_userdata( sqlite3.open_ptr(db_ptr) )
  -- do something via connection 1
  r094.db:exec("CREATE TABLE test1(a, b)")
  r094.db:exec("INSERT INTO test1 VALUES(1, 2)")
  -- see result via connection 2
  for a, b in db2:urows("SELECT * FROM test1 ORDER BY a") do
        assert_equal(a, 1)
        assert_equal(b, 2)
  end
  assert_number( db2:close() )

end

lunit.main()

