--
-- tests/test_platforms.lua
-- Automated test suite for platform handling functions.
-- Copyright (c) 2009 Jason Perkins and the Premake project
--

	T.platforms = { }


	local testmap = { Native="Win32", x32="Win32", x64="x64" }
	
	local sln, r
	function T.platforms.setup()
		sln = solution "MySolution"
		configurations { "Debug", "Release" }
	end


	function T.platforms.filter_OnNoSolutionPlatforms()
		premake.bake.buildconfigs()
		r = premake.filterplatforms(sln, testmap)
		test.isequal("", table.concat(r, ":"))
	end
	
	function T.platforms.filter_OnNoSolutionPlatformsAndDefault()
		premake.bake.buildconfigs()
		r = premake.filterplatforms(sln, testmap, "x32")
		test.isequal("x32", table.concat(r, ":"))
	end
	
	function T.platforms.filter_OnIntersection()
		platforms { "x32", "x64", "Xbox360" }
		premake.bake.buildconfigs()
		r = premake.filterplatforms(sln, testmap, "x32")
		test.isequal("x32:x64", table.concat(r, ":"))
	end
	
	function T.platforms.filter_OnNoIntersection()
		platforms { "Universal", "Xbox360" }
		premake.bake.buildconfigs()
		r = premake.filterplatforms(sln, testmap)
		test.isequal("", table.concat(r, ":"))
	end
	
	function T.platforms.filter_OnNoIntersectionAndDefault()
		platforms { "Universal", "Xbox360" }
		premake.bake.buildconfigs()
		r = premake.filterplatforms(sln, testmap, "x32")
		test.isequal("x32", table.concat(r, ":"))
	end
	
	function T.platforms.filter_OnDuplicateKeys()
		platforms { "Native", "x32" }
		premake.bake.buildconfigs()
		r = premake.filterplatforms(sln, testmap, "x32")
		test.isequal("Native", table.concat(r, ":"))
	end
		
