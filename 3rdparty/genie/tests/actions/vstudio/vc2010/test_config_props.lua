--
-- tests/actions/vstudio/vc2010/test_config_props.lua
-- Validate generation of the configuration property group.
-- Copyright (c) 2011-2013 Jason Perkins and the Premake project
--

	T.vstudio_vs2010_config_props = { }
	local suite = T.vstudio_vs2010_config_props
	local vc2010 = premake.vstudio.vc2010
	local project = premake.project


--
-- Setup
--

	local sln, prj

	function suite.setup()
		sln, prj = test.createsolution()
	end

	local function prepare(platform)
		premake.bake.buildconfigs()
		sln.vstudio_configs = premake.vstudio.buildconfigs(sln)
		local cfginfo = sln.vstudio_configs[1]
		local cfg = premake.getconfig(prj, cfginfo.src_buildcfg, cfginfo.src_platform)
		vc2010.configurationPropertyGroup(cfg, cfginfo)
	end


--
-- Check the structure with the default project values.
--

	function suite.structureIsCorrect_onDefaultValues()
		prepare()
		test.capture [[
	<PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'" Label="Configuration">
		<ConfigurationType>Application</ConfigurationType>
		<UseDebugLibraries>true</UseDebugLibraries>
		<CharacterSet>MultiByte</CharacterSet>
	</PropertyGroup>
		]]
	end




--
-- Visual Studio 2012 adds a platform toolset.
--

	function suite.structureIsCorrect_onDefaultValues()
		_ACTION = "vs2012"
		prepare()
		test.capture [[
	<PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'" Label="Configuration">
		<ConfigurationType>Application</ConfigurationType>
		<UseDebugLibraries>true</UseDebugLibraries>
		<CharacterSet>MultiByte</CharacterSet>
		<PlatformToolset>v110</PlatformToolset>
	</PropertyGroup>
		]]
	end

	function suite.structureIsCorrect_onDefaultValues_on2013()
		_ACTION = "vs2013"
		prepare()
		test.capture [[
	<PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'" Label="Configuration">
		<ConfigurationType>Application</ConfigurationType>
		<UseDebugLibraries>true</UseDebugLibraries>
		<CharacterSet>MultiByte</CharacterSet>
		<PlatformToolset>v120</PlatformToolset>
	</PropertyGroup>
		]]
	end
