--
-- tests/actions/codeblocks/test_filters.lua
-- Test CodeBlocks virtual path support.
-- Copyright (c) 2011 Jason Perkins and the Premake project
--

	T.codeblocks_filters = { }
	local suite = T.codeblocks_filters
	local codeblocks = premake.codeblocks


--
-- Setup 
--

	local sln, prj
	
	function suite.setup()
		sln = test.createsolution()
	end
	
	local function prepare()
		premake.bake.buildconfigs()
		prj = premake.solution.getproject(sln, 1)
		codeblocks.files(prj)
	end


--
-- File/filter assignment tests
--

	function suite.Filter_UsesVirtualForm_OnVpath()
		files { "src/hello.cpp" }
		vpaths { ["Source Files"] = "**.c" }		
		prepare()
		test.capture [[
		<Unit filename="src/hello.cpp">
			<Option virtualFolder="Source Files" />
		</Unit>
		]]
	end
