--
-- tests/actions/vstudio/vc2010/test_output_props.lua
-- Validate generation of the output property groups.
-- Copyright (c) 2011-2013 Jason Perkins and the Premake project
--

	T.vstudio_vs2010_output_props = {}
	local suite = T.vstudio_vs2010_output_props
	local vc2010 = premake.vstudio.vc2010


--
-- Setup
--

	local sln

	function suite.setup()
		_ACTION = "vs2010"
		sln = test.createsolution()
	end

	local function prepare()
		premake.bake.buildconfigs()
		sln.vstudio_configs = premake.vstudio.buildconfigs(sln)
		local prj = premake.solution.getproject(sln, 1)
		vc2010.outputProperties(prj)
	end


--
-- Check the structure with the default project values.
--

	function suite.structureIsCorrect_onDefaultValues()
		prepare()
		test.capture [[
	<PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">
		<OutDir>.\</OutDir>
		<IntDir>obj\Debug\</IntDir>
		<TargetName>MyProject</TargetName>
		<TargetExt>.exe</TargetExt>
		<LinkIncremental>true</LinkIncremental>
	</PropertyGroup>
	<PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">
		]]
	end


--
-- Static libraries should omit the link incremental element entirely.
--

	function suite.omitLinkIncremental_onStaticLib()
		kind "StaticLib"
		prepare()
		test.capture [[
	<PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">
		<OutDir>.\</OutDir>
		<IntDir>obj\Debug\</IntDir>
		<TargetName>MyProject</TargetName>
		<TargetExt>.lib</TargetExt>
	</PropertyGroup>
		]]
	end

--
-- Optimized builds should not link incrementally.
--

	function suite.noIncrementalLink_onOptimizedBuild()
		flags "Optimize"
		prepare()
		test.capture [[
	<PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">
		<OutDir>.\</OutDir>
		<IntDir>obj\Debug\</IntDir>
		<TargetName>MyProject</TargetName>
		<TargetExt>.exe</TargetExt>
		<LinkIncremental>false</LinkIncremental>
	</PropertyGroup>
		]]
	end

--
-- The target directory is applied, if specified.
--

	function suite.outDir_onTargetDir()
		targetdir "../bin"
		prepare()
		test.capture [[
	<PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">
		<OutDir>..\bin\</OutDir>
		]]
	end

--
-- The objeccts directory is applied, if specified.
--

	function suite.intDir_onTargetDir()
		objdir "../tmp"
		prepare()
		test.capture [[
	<PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">
		<OutDir>.\</OutDir>
		<IntDir>..\tmp\Debug\</IntDir>
		]]
	end

--
-- The target name is applied, if specified.
--

	function suite.targetName_onTargetName()
		targetname "MyTarget"
		prepare()
		test.capture [[
	<PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">
		<OutDir>.\</OutDir>
		<IntDir>obj\Debug\</IntDir>
		<TargetName>MyTarget</TargetName>
		]]
	end

--
-- A target extension should be used if specified.
--

	function suite.targetExt_onTargetExtension()
		targetextension ".delta"
		prepare()
		test.capture [[
	<PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">
		<OutDir>.\</OutDir>
		<IntDir>obj\Debug\</IntDir>
		<TargetName>MyProject</TargetName>
		<TargetExt>.delta</TargetExt>
		]]
	end

--
-- If the NoImportLib flag is set, add the IgnoreImportLibrary element.
--

	function suite.ignoreImportLib_onNoImportLib()
		kind "SharedLib"
		flags "NoImportLib"
		prepare()
		test.capture [[
	<PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">
		<OutDir>.\</OutDir>
		<IntDir>obj\Debug\</IntDir>
		<TargetName>MyProject</TargetName>
		<TargetExt>.dll</TargetExt>
		<IgnoreImportLibrary>true</IgnoreImportLibrary>
		]]
	end


--
-- If the NoManifest flag is set, add the GenerateManifest element.
--

	function suite.generateManifest_onNoManifest()
		flags "NoManifest"
		prepare()
		test.capture [[
	<PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">
		<OutDir>.\</OutDir>
		<IntDir>obj\Debug\</IntDir>
		<TargetName>MyProject</TargetName>
		<TargetExt>.exe</TargetExt>
		<LinkIncremental>true</LinkIncremental>
		<GenerateManifest>false</GenerateManifest>
	</PropertyGroup>
		]]
	end
