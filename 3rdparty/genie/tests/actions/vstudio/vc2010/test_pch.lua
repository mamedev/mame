--
-- tests/actions/vstudio/vc2010/test_pch.lua
-- Validate generation of files block in Visual Studio 2010 C/C++ projects.
-- Copyright (c) 2011 Jason Perkins and the Premake project
--

	T.vstudio_vs2010_pch = { }
	local suite = T.vstudio_vs2010_pch
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
-- Tests
--

	function suite.pch_OnProject()
		files { "afxwin.cpp" }
		pchheader "afxwin.h"
		pchsource "afxwin.cpp"
		prepare()
		test.capture [[
	<ItemGroup>
		<ClCompile Include="afxwin.cpp">
			<PrecompiledHeader Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">Create</PrecompiledHeader>
			<PrecompiledHeader Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">Create</PrecompiledHeader>
		</ClCompile>
	</ItemGroup>
		]]
	end


	function suite.pch_OnSingleConfig()
		files { "afxwin.cpp" }
		configuration "Debug"
		pchheader "afxwin.h"
		pchsource "afxwin.cpp"
		prepare()
		test.capture [[
	<ItemGroup>
		<ClCompile Include="afxwin.cpp">
			<PrecompiledHeader Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">Create</PrecompiledHeader>
		</ClCompile>
	</ItemGroup>
		]]
	end
