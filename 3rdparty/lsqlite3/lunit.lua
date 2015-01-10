
--[[--------------------------------------------------------------------------

    This file is part of lunit 0.4pre (alpha).

    For Details about lunit look at: http://www.nessie.de/mroth/lunit/

    Author: Michael Roth <mroth@nessie.de>

    Copyright (c) 2004 Michael Roth <mroth@nessie.de>

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




-----------------------
-- Intialize package --
-----------------------

local P = { }
lunit = P

-- Import
local type = type
local print = print
local ipairs = ipairs
local pairs = pairs
local string = string
local table = table
local pcall = pcall
local xpcall = xpcall
local traceback = debug.traceback
local error = error
local setmetatable = setmetatable
local rawset = rawset
local orig_assert = assert
local getfenv = getfenv
local setfenv = setfenv
local tostring = tostring


-- Start package scope
setfenv(1, P)




--------------------------------
-- Private data and functions --
--------------------------------

local run_testcase
local do_assert, check_msg
local stats = { }
local testcases = { }
local stats_inc, tc_mt




--------------------------
-- Type check functions --
--------------------------

function is_nil(x)
  return type(x) == "nil"
end

function is_boolean(x)
  return type(x) == "boolean"
end

function is_number(x)
  return type(x) == "number"
end

function is_string(x)
  return type(x) == "string"
end

function is_table(x)
  return type(x) == "table"
end

function is_function(x)
  return type(x) == "function"
end

function is_thread(x)
  return type(x) == "thread"
end

function is_userdata(x)
  return type(x) == "userdata"
end




----------------------
-- Assert functions --
----------------------

function assert(assertion, msg)
  stats_inc("assertions")
  check_msg("assert", msg)
  do_assert(not not assertion, "assertion failed (was: "..tostring(assertion)..")", msg)		-- (convert assertion to bool)
  return assertion
end


function assert_fail(msg)
  stats_inc("assertions")
  check_msg("assert_fail", msg)
  do_assert(false, "failure", msg)
end


function assert_true(actual, msg)
  stats_inc("assertions")
  check_msg("assert_true", msg)
  do_assert(is_boolean(actual), "true expected but was a "..type(actual), msg)
  do_assert(actual == true, "true expected but was false", msg)
  return actual
end


function assert_false(actual, msg)
  stats_inc("assertions")
  check_msg("assert_false", msg)
  do_assert(is_boolean(actual), "false expected but was a "..type(actual), msg)
  do_assert(actual == false, "false expected but was true", msg)
  return actual
end


function assert_equal(expected, actual, msg)
  stats_inc("assertions")
  check_msg("assert_equal", msg)
  do_assert(expected == actual, "expected '"..tostring(expected).."' but was '"..tostring(actual).."'", msg)
  return actual
end


function assert_not_equal(unexpected, actual, msg)
  stats_inc("assertions")
  check_msg("assert_not_equal", msg)
  do_assert(unexpected ~= actual, "'"..tostring(expected).."' not expected but was one", msg)
  return actual
end


function assert_match(pattern, actual, msg)
  stats_inc("assertions")
  check_msg("assert_match", msg)
  do_assert(is_string(pattern), "assert_match expects the pattern as a string")
  do_assert(is_string(actual), "expected a string to match pattern '"..pattern.."' but was a '"..type(actual).."'", msg)
  do_assert(not not string.find(actual, pattern), "expected '"..actual.."' to match pattern '"..pattern.."' but doesn't", msg)
  return actual
end


function assert_not_match(pattern, actual, msg)
  stats_inc("assertions")
  check_msg("assert_not_match", msg)
  do_assert(is_string(actual), "expected a string to not match pattern '"..pattern.."' but was a '"..type(actual).."'", msg)
  do_assert(string.find(actual, pattern) == nil, "expected '"..actual.."' to not match pattern '"..pattern.."' but it does", msg)
  return actual
end


function assert_nil(actual, msg)
  stats_inc("assertions")
  check_msg("assert_nil", msg)
  do_assert(is_nil(actual), "nil expected but was a "..type(actual), msg)
  return actual
end


function assert_not_nil(actual, msg)
  stats_inc("assertions")
  check_msg("assert_not_nil", msg)
  do_assert(not is_nil(actual), "nil not expected but was one", msg)
  return actual
end


function assert_boolean(actual, msg)
  stats_inc("assertions")
  check_msg("assert_boolean", msg)
  do_assert(is_boolean(actual), "boolean expected but was a "..type(actual), msg)
  return actual
end


function assert_not_boolean(actual, msg)
  stats_inc("assertions")
  check_msg("assert_not_boolean", msg)
  do_assert(not is_boolean(actual), "boolean not expected but was one", msg)
  return actual
end


function assert_number(actual, msg)
  stats_inc("assertions")
  check_msg("assert_number", msg)
  do_assert(is_number(actual), "number expected but was a "..type(actual), msg)
  return actual
end


function assert_not_number(actual, msg)
  stats_inc("assertions")
  check_msg("assert_not_number", msg)
  do_assert(not is_number(actual), "number not expected but was one", msg)
  return actual
end


function assert_string(actual, msg)
  stats_inc("assertions")
  check_msg("assert_string", msg)
  do_assert(is_string(actual), "string expected but was a "..type(actual), msg)
  return actual
end


function assert_not_string(actual, msg)
  stats_inc("assertions")
  check_msg("assert_not_string", msg)
  do_assert(not is_string(actual), "string not expected but was one", msg)
  return actual
end


function assert_table(actual, msg)
  stats_inc("assertions")
  check_msg("assert_table", msg)
  do_assert(is_table(actual), "table expected but was a "..type(actual), msg)
  return actual
end


function assert_not_table(actual, msg)
  stats_inc("assertions")
  check_msg("assert_not_table", msg)
  do_assert(not is_table(actual), "table not expected but was one", msg)
  return actual
end


function assert_function(actual, msg)
  stats_inc("assertions")
  check_msg("assert_function", msg)
  do_assert(is_function(actual), "function expected but was a "..type(actual), msg)
  return actual
end


function assert_not_function(actual, msg)
  stats_inc("assertions")
  check_msg("assert_not_function", msg)
  do_assert(not is_function(actual), "function not expected but was one", msg)
  return actual
end


function assert_thread(actual, msg)
  stats_inc("assertions")
  check_msg("assert_thread", msg)
  do_assert(is_thread(actual), "thread expected but was a "..type(actual), msg)
  return actual
end


function assert_not_thread(actual, msg)
  stats_inc("assertions")
  check_msg("assert_not_thread", msg)
  do_assert(not is_thread(actual), "thread not expected but was one", msg)
  return actual
end


function assert_userdata(actual, msg)
  stats_inc("assertions")
  check_msg("assert_userdata", msg)
  do_assert(is_userdata(actual), "userdata expected but was a "..type(actual), msg)
  return actual
end


function assert_not_userdata(actual, msg)
  stats_inc("assertions")
  check_msg("assert_not_userdata", msg)
  do_assert(not is_userdata(actual), "userdata not expected but was one", msg)
  return actual
end


function assert_error(msg, func)
  stats_inc("assertions")
  if is_nil(func) then func, msg = msg, nil end
  check_msg("assert_error", msg)
  do_assert(is_function(func), "assert_error expects a function as the last argument but it was a "..type(func))
  local ok, errmsg = pcall(func)
  do_assert(ok == false, "error expected but no error occurred", msg)
end


function assert_pass(msg, func)
  stats_inc("assertions")
  if is_nil(func) then func, msg = msg, nil end
  check_msg("assert_pass", msg)
  do_assert(is_function(func), "assert_pass expects a function as the last argument but it was a "..type(func))
  local ok, errmsg = pcall(func)
  if not ok then do_assert(ok == true, "no error expected but error was: "..errmsg, msg) end
end




-----------------------------------------------------------
-- Assert implementation that assumes it was called from --
-- lunit code which was called directly from user code.  --
-----------------------------------------------------------

function do_assert(assertion, base_msg, user_msg)
  orig_assert(is_boolean(assertion))
  orig_assert(is_string(base_msg))
  orig_assert(is_string(user_msg) or is_nil(user_msg))
  if not assertion then
    if user_msg then
      error(base_msg..": "..user_msg, 3)
    else
      error(base_msg.."!", 3)
    end
  end
end

-------------------------------------------
-- Checks the msg argument in assert_xxx --
-------------------------------------------

function check_msg(name, msg)
  orig_assert(is_string(name))
  if not (is_nil(msg) or is_string(msg)) then
    error("lunit."..name.."() expects the optional message as a string but it was a "..type(msg).."!" ,3)
  end
end




-------------------------------------
-- Creates a new TestCase 'Object' --
-------------------------------------

function TestCase(name)
  do_assert(is_string(name), "lunit.TestCase() needs a string as an argument")
  local tc = {
    __lunit_name = name;
    __lunit_setup = nil;
    __lunit_tests = { };
    __lunit_teardown = nil;
  }
  setmetatable(tc, tc_mt)
  table.insert(testcases, tc)
  return tc
end

tc_mt = {
  __newindex = function(tc, key, value)
    rawset(tc, key, value)
    if is_string(key) and is_function(value) then
      local name = string.lower(key)
      if string.find(name, "^test") or string.find(name, "test$") then
        table.insert(tc.__lunit_tests, key)
      elseif name == "setup" then
        tc.__lunit_setup = value
      elseif name == "teardown" then
        tc.__lunit_teardown = value
      end
    end
  end
}



-----------------------------------------
-- Wrap Functions in a TestCase object --
-----------------------------------------

function wrap(name, ...)
  if is_function(name) then
    table.insert({...}, 1, name)
    name = "Anonymous Testcase"
  end
  
  local tc = TestCase(name)
  for index, test in ipairs({...}) do
    tc["Test #"..index] = test
  end
  return tc
end






----------------------------------
-- Runs the complete Test Suite --
----------------------------------

function run()
  
  ---------------------------
  -- Initialize statistics --
  ---------------------------
  
  stats.testcases = 0	-- Total number of Test Cases
  stats.tests = 0	-- Total number of all Tests in all Test Cases
  stats.run = 0		-- Number of Tests run
  stats.notrun = 0	-- Number of Tests not run
  stats.failed = 0	-- Number of Tests failed
  stats.warnings = 0	-- Number of Warnings (teardown)
  stats.errors = 0	-- Number of Errors (setup)
  stats.passed = 0	-- Number of Test passed
  stats.assertions = 0	-- Number of all assertions made in all Test in all Test Cases
  
  --------------------------------
  -- Count Test Cases and Tests --
  --------------------------------
  
  stats.testcases = table.getn(testcases)
  
  for _, tc in ipairs(testcases) do
    stats_inc("tests" , table.getn(tc.__lunit_tests))
  end
  
  ------------------
  -- Print Header --
  ------------------
  
  print()
  print("#### Test Suite with "..stats.tests.." Tests in "..stats.testcases.." Test Cases loaded.")
  
  ------------------------
  -- Run all Test Cases --
  ------------------------
  
  for _, tc in ipairs(testcases) do
    run_testcase(tc)
  end
  
  ------------------
  -- Print Footer --
  ------------------
  
  print()
  print("#### Test Suite finished.")
  
  local msg_assertions = stats.assertions.." Assertions checked. "
  local msg_passed     = stats.passed == stats.tests and "All Tests passed" or  stats.passed.." Tests passed"
  local msg_failed     = stats.failed > 0 and ", "..stats.failed.." failed" or ""
  local msg_run	       = stats.notrun > 0 and ", "..stats.notrun.." not run" or ""
  local msg_warn       = stats.warnings > 0 and ", "..stats.warnings.." warnings" or ""
  
  print()
  print(msg_assertions..msg_passed..msg_failed..msg_run..msg_warn.."!")
  
  -----------------
  -- Return code --
  -----------------
  
  if stats.passed == stats.tests then
    return 0
  else
    return 1
  end
end




-----------------------------
-- Runs a single Test Case --
-----------------------------

function run_testcase(tc)
  
  orig_assert(is_table(tc))
  orig_assert(is_table(tc.__lunit_tests))
  orig_assert(is_string(tc.__lunit_name))
  orig_assert(is_nil(tc.__lunit_setup) or is_function(tc.__lunit_setup))
  orig_assert(is_nil(tc.__lunit_teardown) or is_function(tc.__lunit_teardown))
  
  ----------------------------------
  -- Protected call to a function --
  ----------------------------------
  
  local function call(errprefix, func)
    orig_assert(is_string(errprefix))
    orig_assert(is_function(func))
    local ok, errmsg = xpcall(function() func(tc) end, traceback)
    if not ok then
      print()
      print(errprefix..": "..errmsg)
    end
    return ok
  end
  
  ------------------------------------
  -- Calls setup() on the Test Case --
  ------------------------------------
  
  local function setup(testname)
    if tc.__lunit_setup then 
      return call("ERROR: "..testname..": setup() failed", tc.__lunit_setup)
    else
      return true
    end
  end
  
  ------------------------------------------
  -- Calls a single Test on the Test Case --
  ------------------------------------------
  
  local function run(testname)
    orig_assert(is_string(testname))
    orig_assert(is_function(tc[testname]))
    local ok = call("FAIL: "..testname, tc[testname])
    if not ok then
      stats_inc("failed")
    else
      stats_inc("passed")
    end
    return ok
  end
  
  ---------------------------------------
  -- Calls teardown() on the Test Case --
  ---------------------------------------
  
  local function teardown(testname)
     if tc.__lunit_teardown then
       if not call("WARNING: "..testname..": teardown() failed", tc.__lunit_teardown) then
         stats_inc("warnings")
       end
     end
  end
  
  ---------------------------------
  -- Run all Tests on a TestCase --
  ---------------------------------
  
  print()
  print("#### Running '"..tc.__lunit_name.."' ("..table.getn(tc.__lunit_tests).." Tests)...")
  
  for _, testname in ipairs(tc.__lunit_tests) do
    if setup(testname) then
      run(testname)
      stats_inc("run")
      teardown(testname)
    else
      print("WARN: Skipping '"..testname.."'...")
      stats_inc("notrun")
    end
  end
  
end




---------------------
-- Import function --
---------------------

function import(name)
  
  do_assert(is_string(name), "lunit.import() expects a single string as argument")
  
  local user_env = getfenv(2)
  
  --------------------------------------------------
  -- Installs a specific function in the user env --
  --------------------------------------------------
  
  local function install(funcname)
    user_env[funcname] = P[funcname]
  end
  
  
  ----------------------------------------------------------
  -- Install functions matching a pattern in the user env --
  ----------------------------------------------------------
  
  local function install_pattern(pattern)
    for funcname, _ in pairs(P) do
      if string.find(funcname, pattern) then
        install(funcname)
      end
    end
  end
  
  ------------------------------------------------------------
  -- Installs assert() and all assert_xxx() in the user env --
  ------------------------------------------------------------
  
  local function install_asserts()
    install_pattern("^assert.*")
  end
  
  -------------------------------------------
  -- Installs all is_xxx() in the user env --
  -------------------------------------------
  
  local function install_tests()
    install_pattern("^is_.+")
  end
  
  if name == "asserts" or name == "assertions" then
    install_asserts()
  elseif name == "tests" or name == "checks" then
    install_tests()
  elseif name == "all" then
    install_asserts()
    install_tests()
    install("TestCase")
  elseif string.find(name, "^assert.*") and P[name] then
    install(name)
  elseif string.find(name, "^is_.+") and P[name] then
    install(name)
  elseif name == "TestCase" then
    install("TestCase")
  else
    error("luniit.import(): invalid function '"..name.."' to import", 2)
  end
end




--------------------------------------------------
-- Installs a private environment on the caller --
--------------------------------------------------

function setprivfenv()
  local new_env = { }
  local new_env_mt = { __index = getfenv(2) }
  setmetatable(new_env, new_env_mt)
  setfenv(2, new_env)
end




--------------------------------------------------
-- Increments a counter in the statistics table --  
--------------------------------------------------

function stats_inc(varname, value)
  orig_assert(is_table(stats))
  orig_assert(is_string(varname))
  orig_assert(is_nil(value) or is_number(value))
  if not stats[varname] then return end
  stats[varname] = stats[varname] + (value or 1)
end




