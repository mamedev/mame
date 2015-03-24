
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


require "lsqlite3"

require "lunit"

lunit.setprivfenv()
lunit.import "assertions"
lunit.import "checks"






-------------------------------
-- Basic open and close test --
-------------------------------

lunit.wrap("open_memory", function()
  local db = assert_userdata( sqlite3.open_memory() )
  assert( db:close() )
end)

lunit.wrap("open", function()
  local filename = "/tmp/__lua-sqlite3-20040906135849." .. os.time()
  local db = assert_userdata( sqlite3.open(filename) )
  assert( db:close() )
  os.remove(filename)
end)



-------------------------------------
-- Presence of db member functions --
-------------------------------------

local db_funcs = lunit.TestCase("Database Member Functions")

function db_funcs:setup()
  self.db = assert( sqlite3.open_memory() )
end

function db_funcs:teardown()
  assert( self.db:close() )
end

function db_funcs:test()
  local db = self.db
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

local stmt_funcs = lunit.TestCase("Statement Member Functions")

function stmt_funcs:setup()
  self.db = assert( sqlite3.open_memory() )
  self.stmt = assert( self.db:prepare("CREATE TABLE test (id, content)") )
end

function stmt_funcs:teardown()
--e-  assert( self.stmt:close() )
  assert( self.stmt:finalize() ) --e+
  assert( self.db:close() )
end

function stmt_funcs:test()
  local stmt = self.stmt
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

local basics = lunit.TestCase("Basics")

function basics:setup()
  self.db = assert_userdata( sqlite3.open_memory() )
end

function basics:teardown()
  assert_number( self.db:close() )
end

function basics:create_table()
  assert_number( self.db:exec("CREATE TABLE test (id, name)") )
end

function basics:drop_table()
  assert_number( self.db:exec("DROP TABLE test") )
end

function basics:insert(id, name)
  assert_number( self.db:exec("INSERT INTO test VALUES ("..id..", '"..name.."')") )
end

function basics:update(id, name)
  assert_number( self.db:exec("UPDATE test SET name = '"..name.."' WHERE id = "..id) )
end

function basics:test_create_drop()
  self:create_table()
  self:drop_table()
end

function basics:test_multi_create_drop()
  self:create_table()
  self:drop_table()
  self:create_table()
  self:drop_table()
end

function basics:test_insert()
  self:create_table()
  self:insert(1, "Hello World")
  self:insert(2, "Hello Lua")
  self:insert(3, "Hello sqlite3")
end

function basics:test_update()
  self:create_table()
  self:insert(1, "Hello Home")
  self:insert(2, "Hello Lua")
  self:update(1, "Hello World")
end


---------------------------------
-- Statement Column Info Tests --
---------------------------------

lunit.wrap("Column Info Test", function()
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

st = lunit.TestCase("Statement Tests")

function st:setup()
  self.db = assert( sqlite3.open_memory() )
  assert_equal( sqlite3.OK, self.db:exec("CREATE TABLE test (id, name)") )
  assert_equal( sqlite3.OK, self.db:exec("INSERT INTO test VALUES (1, 'Hello World')") )
  assert_equal( sqlite3.OK, self.db:exec("INSERT INTO test VALUES (2, 'Hello Lua')") )
  assert_equal( sqlite3.OK, self.db:exec("INSERT INTO test VALUES (3, 'Hello sqlite3')") )
end

function st:teardown()
  assert_equal( sqlite3.OK, self.db:close() )
end

function st:check_content(expected)
  local stmt = assert( self.db:prepare("SELECT * FROM test ORDER BY id") )
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

function st:test_setup()
  assert_pass(function() self:check_content{ "Hello World", "Hello Lua", "Hello sqlite3" } end)
  assert_error(function() self:check_content{ "Hello World", "Hello Lua" } end)
  assert_error(function() self:check_content{ "Hello World", "Hello Lua", "Hello sqlite3", "To much" } end)
  assert_error(function() self:check_content{ "Hello World", "Hello Lua", "Wrong" } end)
  assert_error(function() self:check_content{ "Hello World", "Wrong", "Hello sqlite3" } end)
  assert_error(function() self:check_content{ "Wrong", "Hello Lua", "Hello sqlite3" } end)
end

function st:test_questionmark_args()
  local stmt = assert_userdata( self.db:prepare("INSERT INTO test VALUES (?, ?)")  )
  assert_number( stmt:bind_values(0, "Test") )
  assert_error(function() stmt:bind_values("To few") end)
  assert_error(function() stmt:bind_values(0, "Test", "To many") end)
end

function st:test_questionmark()
  local stmt = assert_userdata( self.db:prepare("INSERT INTO test VALUES (?, ?)")  )
  assert_number( stmt:bind_values(4, "Good morning") )
  assert_number( stmt:step() )
  assert_number( stmt:reset() )
  self:check_content{ "Hello World", "Hello Lua", "Hello sqlite3", "Good morning" }
  assert_number( stmt:bind_values(5, "Foo Bar") )
  assert_number( stmt:step() )
  assert_number( stmt:reset() )
  self:check_content{ "Hello World", "Hello Lua", "Hello sqlite3", "Good morning", "Foo Bar" }
  assert_number( stmt:finalize() )
end

--[===[
function st:test_questionmark_multi()
  local stmt = assert_userdata( self.db:prepare([[
    INSERT INTO test VALUES (?, ?); INSERT INTO test VALUES (?, ?) ]]))
  assert( stmt:bind_values(5, "Foo Bar", 4, "Good morning") )
  assert_number( stmt:step() )
  assert_number( stmt:reset() )
  self:check_content{ "Hello World", "Hello Lua", "Hello sqlite3", "Good morning", "Foo Bar" }
  assert_number( stmt:finalize() )
end
]===]

function st:test_identifiers()
  local stmt = assert_userdata( self.db:prepare("INSERT INTO test VALUES (:id, :name)")  )
  assert_number( stmt:bind_values(4, "Good morning") )
  assert_number( stmt:step() )
  assert_number( stmt:reset() )
  self:check_content{ "Hello World", "Hello Lua", "Hello sqlite3", "Good morning" }
  assert_number( stmt:bind_values(5, "Foo Bar") )
  assert_number( stmt:step() )
  assert_number( stmt:reset() )
  self:check_content{ "Hello World", "Hello Lua", "Hello sqlite3", "Good morning", "Foo Bar" }
  assert_number( stmt:finalize() )
end

--[===[
function st:test_identifiers_multi()
  local stmt = assert_table( self.db:prepare([[
    INSERT INTO test VALUES (:id1, :name1); INSERT INTO test VALUES (:id2, :name2) ]]))
  assert( stmt:bind_values(5, "Foo Bar", 4, "Good morning") )
  assert( stmt:exec() )
  self:check_content{ "Hello World", "Hello Lua", "Hello sqlite3", "Good morning", "Foo Bar" }
end
]===]

function st:test_identifiers_names()
  --local stmt = assert_userdata( self.db:prepare({"name", "id"}, "INSERT INTO test VALUES (:id, $name)")  )
  local stmt = assert_userdata( self.db:prepare("INSERT INTO test VALUES (:id, $name)")  )
  assert_number( stmt:bind_names({name="Good morning", id=4}) )
  assert_number( stmt:step() )
  assert_number( stmt:reset() )
  self:check_content{ "Hello World", "Hello Lua", "Hello sqlite3", "Good morning" }
  assert_number( stmt:bind_names({name="Foo Bar", id=5}) )
  assert_number( stmt:step() )
  assert_number( stmt:reset() )
  self:check_content{ "Hello World", "Hello Lua", "Hello sqlite3", "Good morning", "Foo Bar" }
  assert_number( stmt:finalize() )
end

--[===[
function st:test_identifiers_multi_names()
  local stmt = assert_table( self.db:prepare( {"name", "id1", "id2"},[[
    INSERT INTO test VALUES (:id1, $name); INSERT INTO test VALUES ($id2, :name) ]]))
  assert( stmt:bind_values("Hoho", 4, 5) )
  assert( stmt:exec() )
  self:check_content{ "Hello World", "Hello Lua", "Hello sqlite3", "Hoho", "Hoho" }
end
]===]

function st:test_colon_identifiers_names()
  local stmt = assert_userdata( self.db:prepare("INSERT INTO test VALUES (:id, :name)")  )
  assert_number( stmt:bind_names({name="Good morning", id=4}) )
  assert_number( stmt:step() )
  assert_number( stmt:reset() )
  self:check_content{ "Hello World", "Hello Lua", "Hello sqlite3", "Good morning" }
  assert_number( stmt:bind_names({name="Foo Bar", id=5}) )
  assert_number( stmt:step() )
  assert_number( stmt:reset() )
  self:check_content{ "Hello World", "Hello Lua", "Hello sqlite3", "Good morning", "Foo Bar" }
  assert_number( stmt:finalize() )
end

--[===[
function st:test_colon_identifiers_multi_names()
  local stmt = assert_table( self.db:prepare( {":name", ":id1", ":id2"},[[
    INSERT INTO test VALUES (:id1, $name); INSERT INTO test VALUES ($id2, :name) ]]))
  assert( stmt:bind_values("Hoho", 4, 5) )
  assert( stmt:exec() )
  self:check_content{ "Hello World", "Hello Lua", "Hello sqlite3", "Hoho", "Hoho" }
end


function st:test_dollar_identifiers_names()
  local stmt = assert_table( self.db:prepare({"$name", "$id"}, "INSERT INTO test VALUES (:id, $name)")  )
  assert_table( stmt:bind_values("Good morning", 4) )
  assert_table( stmt:exec() )
  self:check_content{ "Hello World", "Hello Lua", "Hello sqlite3", "Good morning" }
  assert_table( stmt:bind_values("Foo Bar", 5) )
  assert_table( stmt:exec() )
  self:check_content{ "Hello World", "Hello Lua", "Hello sqlite3", "Good morning", "Foo Bar" }
end

function st:test_dollar_identifiers_multi_names()
  local stmt = assert_table( self.db:prepare( {"$name", "$id1", "$id2"},[[
    INSERT INTO test VALUES (:id1, $name); INSERT INTO test VALUES ($id2, :name) ]]))
  assert( stmt:bind_values("Hoho", 4, 5) )
  assert( stmt:exec() )
  self:check_content{ "Hello World", "Hello Lua", "Hello sqlite3", "Hoho", "Hoho" }
end
]===]

function st:test_bind_by_names()
  local stmt = assert_userdata( self.db:prepare("INSERT INTO test VALUES (:id, :name)")  )
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
  self:check_content{ "Hello World", "Hello Lua", "Hello sqlite3",  "Hello boys", "Hello girls" }
  assert_number( stmt:finalize() )
end



--------------------------------
-- Tests binding of arguments --
--------------------------------

b = lunit.TestCase("Binding Tests")

function b:setup()
  self.db = assert( sqlite3.open_memory() )
  assert_number( self.db:exec("CREATE TABLE test (id, name, u, v, w, x, y, z)") )
end

function b:teardown()
  assert_number( self.db:close() )
end

function b:test_auto_parameter_names()
  local stmt = assert_userdata( self.db:prepare("INSERT INTO test VALUES(:a, $b, :a2, :b2, $a, :b, $a3, $b3)") )
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

function b:test_auto_parameter_names()
  local stmt = assert_userdata( self.db:prepare("INSERT INTO test VALUES($a, $b, $a2, $b2, $a, $b, $a3, $b3)") )
  local parameters = assert_number( stmt:bind_parameter_count() )
  assert_equal( 6, parameters )
  assert_equal( "$a", stmt:bind_parameter_name(1) )
  assert_equal( "$b", stmt:bind_parameter_name(2) )
  assert_equal( "$a2", stmt:bind_parameter_name(3) )
  assert_equal( "$b2", stmt:bind_parameter_name(4) )
  assert_equal( "$a3", stmt:bind_parameter_name(5) )
  assert_equal( "$b3", stmt:bind_parameter_name(6) )
end

function b:test_no_parameter_names_1()
  local stmt = assert_userdata( self.db:prepare([[ SELECT * FROM test ]]))
  local parameters = assert_number( stmt:bind_parameter_count() )
  assert_equal( 0, (parameters) )
end

function b:test_no_parameter_names_2()
  local stmt = assert_userdata( self.db:prepare([[ INSERT INTO test VALUES(?, ?, ?, ?, ?, ?, ?, ?) ]]))
  local parameters = assert_number( stmt:bind_parameter_count() )
  assert_equal( 8, (parameters) )
  assert_nil( stmt:bind_parameter_name(1) )
end







--------------------------------------------
-- Tests loop break and statement reusage --
--------------------------------------------



----------------------------
-- Test for bugs reported --
----------------------------

bug = lunit.TestCase("Bug-Report Tests")

function bug:setup()
  self.db = assert( sqlite3.open_memory() )
end

function bug:teardown()
  assert_number( self.db:close() )
end

--[===[
function bug:test_1()
  self.db:exec("CREATE TABLE test (id INTEGER PRIMARY KEY, value TEXT)")
  
  local query = assert_userdata( self.db:prepare("SELECT id FROM test WHERE value=?") )
  
  assert_table ( query:bind_values("1") )
  assert_nil   ( query:first_cols() )
  assert_table ( query:bind_values("2") )
  assert_nil   ( query:first_cols() )
end
]===]

function bug:test_nils()   -- appeared in lua-5.1 (holes in arrays)
  local function check(arg1, arg2, arg3, arg4, arg5)
    assert_equal(1, arg1)
    assert_equal(2, arg2)
    assert_nil(arg3)
    assert_equal(4, arg4)
    assert_nil(arg5)
  end
  
  self.db:create_function("test_nils", 5, function(arg1, arg2, arg3, arg4, arg5)
    check(arg1, arg2, arg3, arg4, arg5)
  end, {})
  
  assert_number( self.db:exec([[ SELECT test_nils(1, 2, NULL, 4, NULL) ]]) )
  
  for arg1, arg2, arg3, arg4, arg5 in self.db:urows([[ SELECT 1, 2, NULL, 4, NULL ]])
  do check(arg1, arg2, arg3, arg4, arg5) 
  end
  
  for row in self.db:rows([[ SELECT 1, 2, NULL, 4, NULL ]])
  do assert_table( row ) 
     check(row[1], row[2], row[3], row[4], row[5])
  end
end

----------------------------
-- Test for collation fun --
----------------------------

colla = lunit.TestCase("Collation Tests")

function colla:setup()
    local function collate(s1,s2)
        -- if p then print("collation callback: ",s1,s2) end
        s1=s1:lower()
        s2=s2:lower()
        if s1==s2 then return 0
        elseif s1<s2 then return -1
        else return 1 end
    end
    self.db = assert( sqlite3.open_memory() )
    assert_nil(self.db:create_collation('CINSENS',collate))
    self.db:exec[[
      CREATE TABLE test(id INTEGER PRIMARY KEY,content COLLATE CINSENS);
      INSERT INTO test VALUES(NULL,'hello world');
      INSERT INTO test VALUES(NULL,'Buenos dias');
      INSERT INTO test VALUES(NULL,'HELLO WORLD');
      INSERT INTO test VALUES(NULL,'Guten Tag');
      INSERT INTO test VALUES(NULL,'HeLlO WoRlD');
      INSERT INTO test VALUES(NULL,'Bye for now');
    ]]
end

function colla:teardown()
  assert_number( self.db:close() )
end

function colla:test()
    --for row in db:nrows('SELECT * FROM test') do
    --  print(row.id,row.content)
    --end
    local n = 0
    for row in self.db:nrows('SELECT * FROM test WHERE content="hElLo wOrLd"') do
      -- print(row.id,row.content)
      assert_equal (row.content:lower(), "hello world")
      n = n + 1
    end
    assert_equal (n, 3)
end

lunit.run()
