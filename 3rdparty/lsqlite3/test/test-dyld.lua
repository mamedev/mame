--
-- test for load_extension 
--
-- before running this script, you must compile extension-functions.c
-- e.g., in directory extras:
-- gcc -fno-common -dynamiclib extension-functions.c -o libsqlitefunctions.dylib
--
-- then run this script from the top level directory: lua test/test-dyld.lua 

local sqlite3 = require "lsqlite3"

local os = os

local lunit = require "lunitx"

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

local db_dyld = lunit_TestCase("Load Extension")

function db_dyld.setup()
  db_dyld.db = assert( sqlite3.open_memory() )
  assert_equal( sqlite3.OK, db_dyld.db:exec("CREATE TABLE test (id, name)") )
  assert_equal( sqlite3.OK, db_dyld.db:exec("INSERT INTO test VALUES (1, 'Hello World')") )
  assert_equal( sqlite3.OK, db_dyld.db:exec("INSERT INTO test VALUES (2, 'Hello Lua')") )
  assert_equal( sqlite3.OK, db_dyld.db:exec("INSERT INTO test VALUES (3, 'Hello sqlite3')") )
end

function db_dyld.teardown()
  assert( db_dyld.db:close() )
end

function db_dyld.test()
    local db = db_dyld.db
    assert_function( db.load_extension )
    assert_true( db:load_extension "extras/libsqlitefunctions" )
    for row in db:nrows("SELECT log10(id) as val FROM test WHERE name='Hello World'") do
      assert_equal (row.val, 0.0)
    end
    for row in db:nrows("SELECT reverse(name) as val FROM test WHERE id = 2") do
      assert_equal (row.val, 'auL olleH')
    end
    for row in db:nrows("SELECT padl(name, 16) as val FROM test WHERE id = 3") do
      assert_equal (row.val, '   Hello sqlite3')
    end
end

