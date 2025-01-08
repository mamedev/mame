--
-- _jcdb.lua
-- Define the compile_commands.json action(s).
-- Copyright (c) 2020 Johan Skoeld
--

newaction {
	trigger = "jcdb",
	shortname = "compile_commands.json",
	description = "Generate a compile_commands.json file.",

	valid_kinds = { "ConsoleApp", "WindowedApp", "StaticLib", "SharedLib", "Bundle" },
	valid_languages = { "C", "C++" },
	valid_tools = { cc = { "gcc" } },

	onsolution = function(sln)
		local jsonpath = path.join(sln.location, "compile_commands.json")
		premake.generate(sln, jsonpath, premake.jcdb.generate)
	end,
}

