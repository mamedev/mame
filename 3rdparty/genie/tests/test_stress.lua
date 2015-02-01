--
-- tests/tests_stress.lua
-- Stress test for Premake.
-- Copyright (c) 2009 Jason Perkins and the Premake project
--

local numprojects = 100
local numfiles    = 100

dofile("pepperfish_profiler.lua")
profiler = newProfiler()
function dumpresults(sorttotal)
    local outfile = io.open("build/profile.txt", "w+" )
    profiler:report(outfile, sorttotal)
    outfile:close()
end


solution "MySolution"
	configurations { "Debug", "Release", "DebugDLL", "ReleaseDLL" }
	platforms { "Native", "x32", "x64" }
	location "build"
		
	configuration "Debug"
		defines { "_DEBUG" }
		flags   { "Symbols" }
		
	configuration "Release"
		defines { "NDEBUG" }
		flags   { "Optimize" }
		

for pi = 1, numprojects do
	
	project ("Project" .. pi)
	location "build"
	kind     "ConsoleApp"
	language "C++"
	
	for fi = 1, numfiles do
		files { "file" .. fi .. ".cpp" }
	end
		
end
