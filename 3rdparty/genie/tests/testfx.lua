--
-- tests/testfx.lua
-- Automated test framework for Premake.
-- Copyright (c) 2008-2009 Jason Perkins and the Premake project
--


--
-- Define a namespace for the testing functions
--

	test = { }


--
-- Assertion functions
--
	function test.string_contains(buffer, expected)
		if not string.find(buffer,expected) then
			test.fail("\n==Fail==: Expected to find :\n%s\nyet it was not found in buffer:\n%s\n", expected,buffer)
		end
	end

	function test.string_does_not_contain(buffer, expected)
		if string.find(buffer,expected) then
			test.fail("\n==Fail==: Did not expected to find :\n%s\nyet it was found in buffer:\n%s\n", expected,buffer)
		end
	end

	function test.capture(expected)
		local actual = io.endcapture()

		local ait = actual:gfind("(.-)" .. io.eol)
		local eit = expected:gfind("(.-)\n")

		local linenum = 1
		local atxt = ait()
		local etxt = eit()
		while etxt do
			if (etxt ~= atxt) then
				test.fail("(%d) expected:\n%s\n...but was:\n%s", linenum, etxt, atxt)
			end

			linenum = linenum + 1
			atxt = ait()
			etxt = eit()
		end
	end


	function test.closedfile(expected)
		if expected and not test.value_closedfile then
			test.fail("expected file to be closed")
		elseif not expected and test.value_closedfile then
			test.fail("expected file to remain open")
		end
	end


	function test.contains(value, expected)
		if not table.contains(value, expected) then
			test.fail("expected value %s not found", expected)
		end
	end


	function test.fail(format, ...)
		-- convert nils into something more usefuls
		for i = 1, arg.n do
			if (arg[i] == nil) then
				arg[i] = "(nil)"
			elseif (type(arg[i]) == "table") then
				arg[i] = "{" .. table.concat(arg[i], ", ") .. "}"
			end
		end
		error(string.format(format, unpack(arg)), 3)
	end


	function test.filecontains(expected, fn)
		local f = io.open(fn)
		local actual = f:read("*a")
		f:close()
		if (expected ~= actual) then
			test.fail("expected %s but was %s", expected, actual)
		end
	end


	function test.isemptycapture()
		local actual = io.endcapture()
		if actual ~= "" then
			test.fail("expected empty capture, but was %s", actual);
		end
	end


	function test.isequal(expected, actual)
		if (type(expected) == "table") then
			for k,v in pairs(expected) do
				if not (test.isequal(expected[k], actual[k])) then
					test.fail("expected %s but was %s", expected, actual)
				end
			end
		else
			if (expected ~= actual) then
				test.fail("expected %s but was %s", expected, actual)
			end
		end
		return true
	end


	function test.isfalse(value)
		if (value) then
			test.fail("expected false but was true")
		end
	end


	function test.isnil(value)
		if (value ~= nil) then
			test.fail("expected nil but was " .. tostring(value))
		end
	end


	function test.isnotnil(value)
		if (value == nil) then
			test.fail("expected not nil")
		end
	end


	function test.istrue(value)
		if (not value) then
			test.fail("expected true but was false")
		end
	end


	function test.openedfile(fname)
		if fname ~= test.value_openedfilename then
			local msg = "expected to open file '" .. fname .. "'"
			if test.value_openedfilename then
				msg = msg .. ", got '" .. test.value_openedfilename .. "'"
			end
			test.fail(msg)
		end
	end


	function test.success(fn, ...)
		local ok, err = pcall(fn, unpack(arg))
		if not ok then
			test.fail("call failed: " .. err)
		end
	end



--
-- Test stubs
--

	local function stub_io_open(fname, mode)
		test.value_openedfilename = fname
		test.value_openedfilemode = mode
		return {
			close = function()
				test.value_closedfile = true
			end
		}
	end

	local function stub_io_output(f)
	end

	local function stub_print(s)
	end


--
-- Define a collection for the test suites
--

	T = { }



--
-- Test execution function
--
	local _OS_host = _OS
	local function test_setup(suite, fn)
		-- clear out some important globals
		_ACTION = "test"
		_ARGS = { }
		_OPTIONS = { }
		_OS = _OS_host
		premake.solution.list = { }
		io.indent = nil
		io.eol = "\n"

		-- reset captured I/O values
		test.value_openedfilename = nil
		test.value_openedfilemode = nil
		test.value_closedfile = false

		if suite.setup then
			return pcall(suite.setup)
		else
			return true
		end
	end


	local function test_run(suite, fn)
		io.capture()
		return pcall(fn)
	end


	local function test_teardown(suite, fn)
		if suite.teardown then
			return pcall(suite.teardown)
		else
			return true
		end
	end


	function test.runall(suitename, testname)
		test.print = print

		print      = stub_print
		io.open    = stub_io_open
		io.output  = stub_io_output

		local numpassed = 0
		local numfailed = 0
		local start_time = os.clock()

		function runtest(suitename, suitetests, testname, testfunc)
			if suitetests.setup ~= testfunc and suitetests.teardown ~= testfunc then
				local ok, err = test_setup(suitetests, testfunc)

				if ok then
					ok, err = test_run(suitetests, testfunc)
				end

				local tok, terr = test_teardown(suitetests, testfunc)
				ok = ok and tok
				err = err or terr

				if (not ok) then
					test.print(string.format("%s.%s: %s", suitename, testname, err))
					numfailed = numfailed + 1
				else
					numpassed = numpassed + 1
				end
			end
		end

		function runsuite(suitename, suitetests, testname)
			if testname then
				runtest(suitename, suitetests, testname, suitetests[testname])
			else
				for testname, testfunc in pairs(suitetests) do
					runtest(suitename, suitetests, testname, testfunc)
				end
			end
		end

		if suitename then
			runsuite(suitename, T[suitename], testname)
		else
			for suitename, suitetests in pairs(T) do
				runsuite(suitename, suitetests, testname)
			end
		end

        io.write('running time : ',  os.clock() - start_time,'\n')
		print = test.print
		return numpassed, numfailed
	end

