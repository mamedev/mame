--
-- tests/actions/vstudio/vc2010/test_mfc.lua
-- Validate MFC support in Visual Studio 2010 C/C++ projects.
-- Copyright (c) 2011 Jason Perkins and the Premake project
--

	T.vstudio_vs2010_mfc = { }
	local suite = T.vstudio_vs2010_mfc
	local vc2010 = premake.vstudio.vc2010


--
-- Setup
--

	local sln, prj, cfg

	function suite.setup()
		_ACTION = "vs2010"
		sln, prj = test.createsolution()
	end

	local function prepare(platform)
		premake.bake.buildconfigs()
		sln.vstudio_configs = premake.vstudio.buildconfigs(sln)
		cfg = premake.getconfig(prj, "Debug", platform)
		vc2010.configurationPropertyGroup(cfg, sln.vstudio_configs[1])
	end


--
-- When MFC is enabled, it should match the runtime library linking
-- method (static or dynamic).
--

	function suite.useOfMfc_isDynamic_onSharedRuntime()
		flags { "MFC" }
		prepare()
		test.capture [[
	<PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'" Label="Configuration">
		<ConfigurationType>Application</ConfigurationType>
		<UseDebugLibraries>true</UseDebugLibraries>
		<CharacterSet>MultiByte</CharacterSet>
		<UseOfMfc>Dynamic</UseOfMfc>
	</PropertyGroup>
		]]
	end
	
	function suite.useOfMfc_isStatic_onStaticRuntime()	
		flags { "MFC", "StaticRuntime" }
		prepare()
		test.capture [[
	<PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'" Label="Configuration">
		<ConfigurationType>Application</ConfigurationType>
		<UseDebugLibraries>true</UseDebugLibraries>
		<CharacterSet>MultiByte</CharacterSet>
		<UseOfMfc>Static</UseOfMfc>
	</PropertyGroup>
		]]
	end
