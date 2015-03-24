--
-- tests/base/test_location.lua
-- Automated tests for the location() function.
-- Copyright (c) 2011 Jason Perkins and the Premake project
--

	T.base_location = { }
	local suite = T.base_location


--
-- Setup/Teardown
--

	function suite.setup()
		sln = solution "MySolution"
		configurations { "Debug", "Release" }
		language "C"
	end

	local function prepare()
		premake.bake.buildconfigs()
		prj = premake.solution.getproject(sln, 1)
	end


--
-- Test no location set
--

	function suite.solutionUsesCwd_OnNoLocationSet()
		project "MyProject"
		prepare()
		test.isequal(os.getcwd(), sln.location)
	end

	function suite.projectUsesCwd_OnNoLocationSet()
		project "MyProject"
		prepare()
		test.isequal(os.getcwd(), prj.location)
	end


--
-- Test with location set on solution only
--

	function suite.solutionUsesLocation_OnSolutionOnly()
		location "build"
		project "MyProject"
		prepare()
		test.isequal(path.join(os.getcwd(), "build"), sln.location)
	end

	function suite.projectUsesLocation_OnSolutionOnly()
		location "build"
		project "MyProject"
		prepare()
		test.isequal(path.join(os.getcwd(), "build"), prj.location)
	end


--
-- Test with location set on project only
--

	function suite.solutionUsesCwd_OnProjectOnly()
		project "MyProject"
		location "build"
		prepare()
		test.isequal(os.getcwd(), sln.location)
	end

	function suite.projectUsesLocation_OnProjectOnly()
		project "MyProject"
		location "build"
		prepare()
		test.isequal(path.join(os.getcwd(), "build"), prj.location)
	end


--
-- Test with location set on both solution and project only
--

	function suite.solutionUsesCwd_OnProjectOnly()
		location "build/solution"
		project "MyProject"
		location "build/project"
		prepare()
		test.isequal(path.join(os.getcwd(), "build/solution"), sln.location)
	end

	function suite.projectUsesLocation_OnProjectOnly()
		location "build/solution"
		project "MyProject"
		location "build/project"
		prepare()
		test.isequal(path.join(os.getcwd(), "build/project"), prj.location)
	end
