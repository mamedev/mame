--
-- tests/project/test_eachfile.lua
-- Automated test suite for the file iteration function.
-- Copyright (c) 2011 Jason Perkins and the Premake project
--

	T.project_eachfile = { }
	local suite = T.project_eachfile
	local project = premake.project


--
-- Setup and teardown
--

	local sln, prj
	function suite.setup()
		sln = test.createsolution()
	end

	local function prepare()
		premake.bake.buildconfigs()
		prj = premake.solution.getproject(sln, 1)
	end


--
-- Tests
--

	function suite.ReturnsAllFiles()
		files { "hello.h", "hello.c" }
		prepare()
		local iter = project.eachfile(prj)
		test.isequal("hello.h", iter().name)
		test.isequal("hello.c", iter().name)
		test.isnil(iter())
	end


	function suite.ReturnedObjectIncludesVpath()
		files { "hello.h", "hello.c" }
		prepare()
		local iter = project.eachfile(prj)
		test.isequal("hello.h", iter().vpath)
	end
