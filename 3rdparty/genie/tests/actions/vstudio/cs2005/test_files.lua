--
-- tests/actions/vstudio/cs2005/test_files.lua
-- Validate generation of <Files/> block in Visual Studio 2005 .csproj
-- Copyright (c) 2009-2012 Jason Perkins and the Premake project
--

	T.vstudio_cs2005_files = { }
	local suite = T.vstudio_cs2005_files
	local cs2005 = premake.vstudio.cs2005


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
		cs2005.files(prj)
	end


--
-- Test grouping and nesting
--

	function suite.SimpleSourceFile()
		files { "Hello.cs" }
		prepare()
		test.capture [[
    <Compile Include="Hello.cs" />
		]]
	end

	function suite.NestedSourceFile()
		files { "Src/Hello.cs" }
		prepare()
		test.capture [[
    <Compile Include="Src\Hello.cs" />
		]]
	end


--
-- The relative path to the file is correct for files that live outside
-- the project's folder.
--

	function suite.filesUseRelativePath_onOutOfTreePath()
		files { "../Src/Hello.cs" }
		prepare()
		test.capture [[
    <Compile Include="..\Src\Hello.cs" />
		]]
	end


--
-- Test file dependencies
--

	function suite.SimpleResourceDependency()
		files { "Resources.resx", "Resources.Designer.cs" }
		prepare()
		test.capture [[
    <Compile Include="Resources.Designer.cs">
      <AutoGen>True</AutoGen>
      <DependentUpon>Resources.resx</DependentUpon>
    </Compile>
    <EmbeddedResource Include="Resources.resx">
      <SubType>Designer</SubType>
      <Generator>ResXFileCodeGenerator</Generator>
      <LastGenOutput>Resources.Designer.cs</LastGenOutput>
    </EmbeddedResource>
		]]
	end
