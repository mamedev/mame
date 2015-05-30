--
-- tests/test_dofile.lua
-- Automated test suite for the extended dofile() functions.
-- Copyright (c) 2008 Jason Perkins and the Premake project
--


	T.dofile = { }

	
	local os_getenv
	
	function T.dofile.setup()
		os_getenv = os.getenv
	end
	
	function T.dofile.teardown()
		os.getenv = os_getenv
	end
	
	
--
-- dofile() tests
--

	function T.dofile.SearchesPath()
		os.getenv = function() return os.getcwd().."/folder" end
		result = dofile("ok.lua")
		test.isequal("ok", result)
	end

	function T.dofile.SearchesScriptsOption()
		_OPTIONS["scripts"] = os.getcwd().."/folder"
		result = dofile("ok.lua")
		test.isequal("ok", result)
	end
