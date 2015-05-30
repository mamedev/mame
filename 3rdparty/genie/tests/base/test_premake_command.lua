--
-- tests/base/test_premake_command.lua
-- Test the initialization of the _PREMAKE_COMMAND global.
-- Copyright (c) 2012 Jason Perkins and the Premake project
--

	T.premake_command = { }
	local suite = T.premake_command


	function suite.valueIsSet()
		local filename = iif(os.is("windows"), "premake4.exe", "premake4")
		test.isequal(path.getabsolute("../bin/debug/" .. filename), _PREMAKE_COMMAND)
	end
