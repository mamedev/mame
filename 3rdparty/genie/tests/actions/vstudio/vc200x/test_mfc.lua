--
-- tests/actions/vstudio/vc200x/test_mfc.lua
-- Validate ATL/MFC support in Visual Studio 200x C/C++ projects.
-- Copyright (c) 2011 Jason Perkins and the Premake project
--

	T.vstudio_vs200x_mfc = { }
	local suite = T.vstudio_vs200x_mfc
	local vc200x = premake.vstudio.vc200x


--
-- Setup
--

	local sln, prj, cfg

	function suite.setup()
		_ACTION = "vs2008"
		sln, prj = test.createsolution()
	end

	local function prepare(platform)
		premake.bake.buildconfigs()
		sln.vstudio_configs = premake.vstudio.buildconfigs(sln)
		cfg = premake.getconfig(prj, "Debug", platform)
		vc200x.Configuration("Debug|Win32", cfg)
	end


--
-- When MFC is enabled, it should match the runtime library linking
-- method (static or dynamic).
--

	function suite.useOfMfc_isDynamic_onSharedRuntime()
		flags { "MFC" }
		prepare()
		test.capture [[
		<Configuration
			Name="Debug|Win32"
			OutputDirectory="."
			IntermediateDirectory="obj\Debug"
			ConfigurationType="1"
			UseOfMFC="2"
			CharacterSet="2"
			>
		]]
	end
	
	function suite.useOfMfc_isStatic_onStaticRuntime()	
		flags { "MFC", "StaticRuntime" }
		prepare()
		test.capture [[
		<Configuration
			Name="Debug|Win32"
			OutputDirectory="."
			IntermediateDirectory="obj\Debug"
			ConfigurationType="1"
			UseOfMFC="1"
			CharacterSet="2"
			>
		]]
	end

--
-- Same as above for ATL.
--

	function suite.useOfAtl_isDynamic_onSharedRuntime()
		flags { "ATL" }
		prepare()
		test.capture [[
		<Configuration
			Name="Debug|Win32"
			OutputDirectory="."
			IntermediateDirectory="obj\Debug"
			ConfigurationType="1"
			UseOfATL="2"
			CharacterSet="2"
			>
		]]
	end
	
	function suite.useOfAtl_isStatic_onStaticRuntime()	
		flags { "StaticATL" }
		prepare()
		test.capture [[
		<Configuration
			Name="Debug|Win32"
			OutputDirectory="."
			IntermediateDirectory="obj\Debug"
			ConfigurationType="1"
			UseOfATL="1"
			CharacterSet="2"
			>
		]]
	end
