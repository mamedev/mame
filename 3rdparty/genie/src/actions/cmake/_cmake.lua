--
-- _cmake.lua
-- Define the CMake action(s).
-- Copyright (c) 2015 Miodrag Milanovic
--

premake.cmake = { }
premake.cmake.cmake_minimum_version = "3.28.3"

--
-- Register the "cmake" action
--

newaction {
	trigger         = "cmake",
	shortname       = "CMake",
	description     = "Generate CMake project files",
	valid_kinds     = { "ConsoleApp", "WindowedApp", "StaticLib", "SharedLib", "Bundle" },
	valid_languages = { "C", "C++" },
	valid_tools     = {
		cc   = { "gcc" },
	},
	onsolution = function(sln)
		premake.generate(sln, "CMakeLists.txt", premake.cmake.workspace)
		premake.generate(sln, "cmake/" .. sln.name .. "/CMakeLists.txt", premake.cmake.gateway)
		premake.generate(sln, "CMakePresets.json", premake.cmake.presets)
	end,
	onproject = function(prj)
		local raw = prj.project or prj
		local sln_name = raw.solution.name
		premake.generate(prj, "cmake/" .. sln_name .. "/%%.cmake", premake.cmake.project)
	end,
	oncleansolution = function(sln)
		premake.clean.file(sln, "CMakeLists.txt")
		premake.clean.file(sln, "cmake/" .. sln.name .. "/CMakeLists.txt")
		premake.clean.file(sln, "CMakePresets.json")
	end,
	oncleanproject = function(prj)
		local raw = prj.project or prj
		local sln_name = raw.solution.name
		premake.clean.file(prj, "cmake/" .. sln_name .. "/%%.cmake")
	end
}