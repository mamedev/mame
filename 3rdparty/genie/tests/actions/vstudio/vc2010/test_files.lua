--
-- tests/actions/vstudio/vc2010/test_files.lua
-- Validate generation of files block in Visual Studio 2010 C/C++ projects.
-- Copyright (c) 2011 Jason Perkins and the Premake project
--

	T.vstudio_vs2010_files = { }
	local suite = T.vstudio_vs2010_files
	local vc2010 = premake.vstudio.vc2010


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
		sln.vstudio_configs = premake.vstudio.buildconfigs(sln)
		vc2010.files(prj)
	end


--
-- Test file groups
--

	function suite.SimpleHeaderFile()
		files { "include/hello.h" }
		prepare()
		test.capture [[
	<ItemGroup>
		<ClInclude Include="include\hello.h" />
	</ItemGroup>
		]]
	end
	
	
	function suite.SimpleSourceFile()
		files { "hello.c" }
		prepare()
		test.capture [[
	<ItemGroup>
		<ClCompile Include="hello.c">
		</ClCompile>
	</ItemGroup>
		]]
	end
	
	
	function suite.SimpleNoneFile()
		files { "docs/hello.txt" }
		prepare()
		test.capture [[
	<ItemGroup>
		<None Include="docs\hello.txt" />
	</ItemGroup>
		]]
	end
	
	
	function suite.SimpleResourceFile()
		files { "resources/hello.rc" }
		prepare()
		test.capture [[
	<ItemGroup>
		<ResourceCompile Include="resources\hello.rc" />
	</ItemGroup>
		]]
	end


--
-- Test path handling
--

	function suite.MultipleFolderLevels()
		files { "src/greetings/hello.c" }
		prepare()
		test.capture [[
	<ItemGroup>
		<ClCompile Include="src\greetings\hello.c">
		</ClCompile>
	</ItemGroup>
		]]
	end
