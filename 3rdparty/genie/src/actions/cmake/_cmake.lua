--
-- _cmake.lua
-- Define the CMake action(s).
-- Copyright (c) 2015 Miodrag Milanovic
--

premake.cmake = { }

newaction {
	trigger         = "cmake",
	shortname       = "CMake",
	description     = "Generate CMake project files",
	valid_kinds     = { "ConsoleApp", "WindowedApp", "StaticLib", "SharedLib" },
	valid_languages = { "C", "C++" },
	valid_tools     = {
	cc   = { "gcc" },
	},
	onsolution = function(sln)
		premake.generate(sln, "CMakeLists.txt", premake.cmake.workspace)
	end,
	onproject = function(prj)
		premake.generate(prj, "%%/CMakeLists.txt", premake.cmake.project)
	end,
	oncleansolution = function(sln)
		premake.clean.file(sln, "CMakeLists.txt")
	end,
	oncleanproject = function(prj)
		premake.clean.file(prj, "%%/CMakeLists.txt")
	end
}

