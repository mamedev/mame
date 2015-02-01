--
-- tests/actions/vstudio/test_vs200x_vcproj_linker.lua
-- Automated tests for Visual Studio 2002-2008 C/C++ linker block.
-- Copyright (c) 2009-2011 Jason Perkins and the Premake project
--

	T.vs200x_vcproj_linker = { }
	local suite = T.vs200x_vcproj_linker
	local vc200x = premake.vstudio.vc200x


--
-- Setup/Teardown
--

	local sln, prj
	function suite.setup()
		_ACTION = "vs2005"
		sln, prj = test.createsolution()
	end

	local function prepare()
		premake.bake.buildconfigs()
	end


--
-- Test default linker blocks for each target kind
-- (ConsoleApp, StaticLib, etc.)
--

	function suite.DefaultLinkerBlock_OnConsoleApp()
		kind "ConsoleApp"
		prepare()
		vc200x.VCLinkerTool(premake.getconfig(prj, "Debug"))
		test.capture [[
			<Tool
				Name="VCLinkerTool"
				OutputFile="$(OutDir)\MyProject.exe"
				LinkIncremental="2"
				AdditionalLibraryDirectories=""
				GenerateDebugInformation="false"
				SubSystem="1"
				EntryPointSymbol="mainCRTStartup"
				TargetMachine="1"
			/>
		]]
	end

	function suite.DefaultLinkerBlock_OnWindowedApp()
		kind "WindowedApp"
		prepare()
		vc200x.VCLinkerTool(premake.getconfig(prj, "Debug"))
		test.capture [[
			<Tool
				Name="VCLinkerTool"
				OutputFile="$(OutDir)\MyProject.exe"
				LinkIncremental="2"
				AdditionalLibraryDirectories=""
				GenerateDebugInformation="false"
				SubSystem="2"
				EntryPointSymbol="mainCRTStartup"
				TargetMachine="1"
			/>
		]]
	end

	function suite.DefaultLinkerBlock_OnSharedLib()
		kind "SharedLib"
		prepare()
		vc200x.VCLinkerTool(premake.getconfig(prj, "Debug"))
		test.capture [[
			<Tool
				Name="VCLinkerTool"
				OutputFile="$(OutDir)\MyProject.dll"
				LinkIncremental="2"
				AdditionalLibraryDirectories=""
				GenerateDebugInformation="false"
				SubSystem="2"
				ImportLibrary="MyProject.lib"
				TargetMachine="1"
			/>
		]]
	end

	function suite.DefaultLinkerBlock_OnStaticLib()
		kind "StaticLib"
		prepare()
		vc200x.VCLinkerTool(premake.getconfig(prj, "Debug"))
		test.capture [[
			<Tool
				Name="VCLibrarianTool"
				OutputFile="$(OutDir)\MyProject.lib"
			/>
		]]
	end


--
-- linkoptions tests
--

	function suite.AdditionalOptions_OnStaticLib()
		kind "StaticLib"
		linkoptions { "/ltcg", "/lZ" }
		prepare()
		vc200x.VCLinkerTool(premake.getconfig(prj, "Debug"))
		test.capture [[
			<Tool
				Name="VCLibrarianTool"
				OutputFile="$(OutDir)\MyProject.lib"
				AdditionalOptions="/ltcg /lZ"
			/>
		]]
	end

