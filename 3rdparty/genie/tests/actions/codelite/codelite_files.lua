--
-- tests/actions/codelite/codelite_files.lua
-- Validate generation of files block in CodeLite C/C++ projects.
-- Copyright (c) 2011 Jason Perkins and the Premake project
--

	T.codelite_files = { }
	local suite = T.codelite_files
	local codelite = premake.codelite


--
-- Setup
--

	local sln, prj

	function suite.setup()
		sln = test.createsolution()
	end

	local function prepare()
		io.indent = "  "
		premake.bake.buildconfigs()
		prj = premake.solution.getproject(sln, 1)
		codelite.files(prj)
	end


--
-- Test grouping and nesting
--

	function suite.SimpleSourceFile()
		files { "hello.c" }
		prepare()
		test.capture [[
  <VirtualDirectory Name="MyProject">
    <File Name="hello.c"/>
  </VirtualDirectory>
		]]
	end


	function suite.SingleFolderLevel()
		files { "src/hello.c" }
		prepare()
		test.capture [[
  <VirtualDirectory Name="MyProject">
    <VirtualDirectory Name="src">
      <File Name="src/hello.c"/>
    </VirtualDirectory>
  </VirtualDirectory>
		]]
	end


	function suite.MultipleFolderLevels()
		files { "src/greetings/hello.c" }
		prepare()
		test.capture [[
  <VirtualDirectory Name="MyProject">
    <VirtualDirectory Name="src">
      <VirtualDirectory Name="greetings">
        <File Name="src/greetings/hello.c"/>
      </VirtualDirectory>
    </VirtualDirectory>
  </VirtualDirectory>
		]]
	end
