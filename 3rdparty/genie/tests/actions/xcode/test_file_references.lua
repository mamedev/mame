--
-- tests/actions/xcode/test_file_references.lua
-- Verify generation of PBXFileReference section of Xcode project
-- Copyright (c) 2011 Jason Perkins and the Premake project
--

	T.xcode3_filerefs = { }
	local suite = T.xcode3_filerefs
	local xcode = premake.xcode


--
-- Setup and teardown
--

	local sln
	
	function suite.setup()
		_ACTION = "xcode3"
		xcode.used_ids = { } -- reset the list of generated IDs
		sln = test.createsolution()
	end

	local function prepare()
		premake.bake.buildconfigs()
		xcode.preparesolution(sln)
		local prj = premake.solution.getproject(sln, 1)
		tr = xcode.buildprjtree(prj)
		xcode.PBXFileReference(tr)
	end


--
-- Bug #3410213: regression in xcode generation in 4.4 beta3
-- When a location has been set on a project, a file at the top of
-- the source tree (i.e. not in a folder) should use the relative
-- path to the project.
--

	function suite.canFindFile_onLocationSet()
		location "build"
		files "hello.c"
		prepare()
		test.capture [[
/* Begin PBXFileReference section */
		[hello.c] /* hello.c */ = {isa = PBXFileReference; lastKnownFileType = sourcecode.c.c; name = "hello.c"; path = "../hello.c"; sourceTree = "<group>"; };
		]]
	end

