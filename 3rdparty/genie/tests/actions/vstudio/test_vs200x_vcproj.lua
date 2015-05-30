--
-- tests/test_vs200x_vcproj.lua
-- Automated test suite for Visual Studio 2002-2008 C/C++ project generation.
-- Copyright (c) 2009-2013 Jason Perkins and the Premake project
--

	T.vs200x_vcproj = { }
	local suite = T.vs200x_vcproj
	local vc200x = premake.vstudio.vc200x
	

--
-- Configure a solution for testing
--

	local sln, prj
	function suite.setup()
		_ACTION = "vs2005"

		sln = solution "MySolution"
		configurations { "Debug", "Release" }
		platforms {}
		
		project "DotNetProject"   -- to test handling of .NET platform in solution
		language "C#"
		kind "ConsoleApp"
		
		prj = project "MyProject"
		language "C++"
		kind "ConsoleApp"
		uuid "AE61726D-187C-E440-BD07-2556188A6565"		
	end

	local function prepare()
		premake.bake.buildconfigs()
		sln.vstudio_configs = premake.vstudio.buildconfigs(sln)

		local cfg = premake.getconfig(sln.projects[2])
		cfg.name = prj.name
		cfg.blocks = prj.blocks
		prj = cfg
	end
	

--
-- Make sure I've got the basic layout correct
--

	function suite.BasicLayout()
		prepare()
		vc200x.generate(prj)
		test.capture [[
<?xml version="1.0" encoding="Windows-1252"?>
<VisualStudioProject
	ProjectType="Visual C++"
	Version="8.00"
	Name="MyProject"
	ProjectGUID="{AE61726D-187C-E440-BD07-2556188A6565}"
	RootNamespace="MyProject"
	Keyword="Win32Proj"
	>
	<Platforms>
		<Platform
			Name="Win32"
		/>
	</Platforms>
	<ToolFiles>
	</ToolFiles>
	<Configurations>
		<Configuration
			Name="Debug|Win32"
			OutputDirectory="."
			IntermediateDirectory="obj\Debug\MyProject"
			ConfigurationType="1"
			CharacterSet="2"
			>
			<Tool
				Name="VCPreBuildEventTool"
			/>
			<Tool
				Name="VCCustomBuildTool"
			/>
			<Tool
				Name="VCXMLDataGeneratorTool"
			/>
			<Tool
				Name="VCWebServiceProxyGeneratorTool"
			/>
			<Tool
				Name="VCMIDLTool"
			/>
			<Tool
				Name="VCCLCompilerTool"
				Optimization="0"
				BasicRuntimeChecks="3"
				RuntimeLibrary="2"
				EnableFunctionLevelLinking="true"
				UsePrecompiledHeader="0"
				WarningLevel="3"
				Detect64BitPortabilityProblems="true"
				ProgramDataBaseFileName="$(OutDir)\MyProject.pdb"
				DebugInformationFormat="0"
			/>
			<Tool
				Name="VCManagedResourceCompilerTool"
			/>
			<Tool
				Name="VCResourceCompilerTool"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
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
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCManifestTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCAppVerifierTool"
			/>
			<Tool
				Name="VCWebDeploymentTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
		<Configuration
			Name="Release|Win32"
			OutputDirectory="."
			IntermediateDirectory="obj\Release\MyProject"
			ConfigurationType="1"
			CharacterSet="2"
			>
			<Tool
				Name="VCPreBuildEventTool"
			/>
			<Tool
				Name="VCCustomBuildTool"
			/>
			<Tool
				Name="VCXMLDataGeneratorTool"
			/>
			<Tool
				Name="VCWebServiceProxyGeneratorTool"
			/>
			<Tool
				Name="VCMIDLTool"
			/>
			<Tool
				Name="VCCLCompilerTool"
				Optimization="0"
				BasicRuntimeChecks="3"
				RuntimeLibrary="2"
				EnableFunctionLevelLinking="true"
				UsePrecompiledHeader="0"
				WarningLevel="3"
				Detect64BitPortabilityProblems="true"
				ProgramDataBaseFileName="$(OutDir)\MyProject.pdb"
				DebugInformationFormat="0"
			/>
			<Tool
				Name="VCManagedResourceCompilerTool"
			/>
			<Tool
				Name="VCResourceCompilerTool"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
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
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCManifestTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCAppVerifierTool"
			/>
			<Tool
				Name="VCWebDeploymentTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
	</Configurations>
	<References>
	</References>
	<Files>
	</Files>
	<Globals>
	</Globals>
</VisualStudioProject>
		]]
	end


--
-- Test multiple platforms
--

	function suite.Platforms_OnMultiplePlatforms()
		platforms { "x32", "x64" }
		prepare()

		vc200x.generate(prj)
		local result = io.endcapture()		
		test.istrue(result:find '<Configuration\r\n\t\t\tName="Debug|Win32"\r\n')
		test.istrue(result:find '<Configuration\r\n\t\t\tName="Release|Win32"\r\n')
		test.istrue(result:find '<Configuration\r\n\t\t\tName="Debug|x64"\r\n')
		test.istrue(result:find '<Configuration\r\n\t\t\tName="Release|x64"\r\n')
	end



--
-- Test x64 handling
--

	function suite.PlatformsList_OnX64()
		platforms { "Native", "x64" }
		prepare()
		vc200x.Platforms(prj)
		test.capture [[
	<Platforms>
		<Platform
			Name="Win32"
		/>
		<Platform
			Name="x64"
		/>
	</Platforms>
		]]		
	end



--
-- Test Xbox360 handling
--

	function suite.PlatformsList_OnXbox360()
		platforms { "Native", "Xbox360" }
		prepare()
		vc200x.Platforms(prj)
		test.capture [[
	<Platforms>
		<Platform
			Name="Win32"
		/>
		<Platform
			Name="Xbox 360"
		/>
	</Platforms>
		]]		
	end
	
	function suite.CompilerBlock_OnXbox360()
		platforms { "Xbox360" }
		prepare()
		vc200x.VCCLCompilerTool(premake.getconfig(prj, "Debug", "Xbox360"))
		test.capture [[
			<Tool
				Name="VCCLX360CompilerTool"
				Optimization="0"
				BasicRuntimeChecks="3"
				RuntimeLibrary="2"
				EnableFunctionLevelLinking="true"
				UsePrecompiledHeader="0"
				WarningLevel="3"
				Detect64BitPortabilityProblems="true"
				ProgramDataBaseFileName="$(OutDir)\MyProject.pdb"
				DebugInformationFormat="0"
			/>
		]]
	end


--
-- Test PS3 handling
--

	function suite.PlatformsList_OnPS3()
		platforms { "Native", "PS3" }
		prepare()
		vc200x.Platforms(prj)
		test.capture [[
	<Platforms>
		<Platform
			Name="Win32"
		/>
	</Platforms>
		]]		
	end
	
	function suite.CompilerBlock_OnPS3()
		platforms { "PS3" }
		flags { "Symbols" }
		includedirs { "include/pkg1", "include/pkg2" }
		defines { "DEFINE1", "DEFINE2" }
		prepare()
		vc200x.VCCLCompilerTool_PS3(premake.getconfig(prj, "Debug", "PS3"))
		test.capture [[
			<Tool
				Name="VCCLCompilerTool"
				UsePrecompiledHeader="0"
				AdditionalOptions=""
				AdditionalIncludeDirectories="include\pkg1;include\pkg2"
				PreprocessorDefinitions="DEFINE1;DEFINE2"
				ProgramDataBaseFileName="$(OutDir)\MyProject.pdb"
				DebugInformationFormat="0"
				CompileAs="0"
			/>
		]]
	end
	
	function suite.LinkerBlock_OnPS3ConsoleApp()
		platforms { "PS3" }
		prepare()
		vc200x.VCLinkerTool_PS3(premake.getconfig(prj, "Debug", "PS3"))
		test.capture [[
			<Tool
				Name="VCLinkerTool"
				AdditionalOptions="-s"
				OutputFile="$(OutDir)\MyProject.elf"
				LinkIncremental="0"
				AdditionalLibraryDirectories=""
				GenerateManifest="false"
				ProgramDatabaseFile=""
				RandomizedBaseAddress="1"
				DataExecutionPrevention="0"
			/>
		]]
	end

	function suite.LinkerBlock_OnPS3StaticLib()
		platforms { "PS3" }
		kind "StaticLib"
		prepare()
		vc200x.VCLinkerTool_PS3(premake.getconfig(prj, "Debug", "PS3"))
		test.capture [[
			<Tool
				Name="VCLibrarianTool"
				AdditionalOptions="-s"
				OutputFile="$(OutDir)\libMyProject.a"
			/>
		]]
	end

	function suite.LinkerBlock_OnPS3SharedLink()
		platforms { "PS3" }
		links { "MyLibrary" }
		project "MyLibrary"
		language "C++"
		kind "SharedLib"
		prepare()
		vc200x.VCLinkerTool_PS3(premake.getconfig(prj, "Debug", "PS3"))

		test.capture [[
			<Tool
				Name="VCLinkerTool"
				AdditionalOptions="-s"
				AdditionalDependencies="libMyLibrary.a"
				OutputFile="$(OutDir)\MyProject.elf"
				LinkIncremental="0"
				AdditionalLibraryDirectories=""
				GenerateManifest="false"
				ProgramDatabaseFile=""
				RandomizedBaseAddress="1"
				DataExecutionPrevention="0"
			/>
		]]
	end

		
--
-- Test manifest file handling.
--

	function suite.VCManifestTool_OnNoManifests()
		files { "hello.c", "goodbye.c" }
		prepare()
		vc200x.VCManifestTool(premake.getconfig(prj, "Debug"))
		test.capture [[
			<Tool
				Name="VCManifestTool"
			/>
		]]
	end


	function suite.VCManifestTool_OnNoManifests()
		files { "hello.c", "project1.manifest", "goodbye.c", "project2.manifest" }
		prepare()
		vc200x.VCManifestTool(premake.getconfig(prj, "Debug"))
		test.capture [[
			<Tool
				Name="VCManifestTool"
				AdditionalManifestFiles="project1.manifest;project2.manifest"
			/>
		]]
	end


--
-- Test precompiled header handling; the header should be treated as
-- a plain string value, with no path manipulation applied, since it
-- needs to match the value of the #include statement used in the
-- project code.
--

	function suite.CompilerBlock_OnPCH()
		location "build/MyProject"
		pchheader "include/common.h"
		pchsource "source/common.cpp"
		prepare()
		vc200x.VCCLCompilerTool(premake.getconfig(prj, "Debug"))
		test.capture [[
			<Tool
				Name="VCCLCompilerTool"
				Optimization="0"
				BasicRuntimeChecks="3"
				RuntimeLibrary="2"
				EnableFunctionLevelLinking="true"
				UsePrecompiledHeader="2"
				PrecompiledHeaderThrough="include/common.h"
				WarningLevel="3"
				Detect64BitPortabilityProblems="true"
				ProgramDataBaseFileName="$(OutDir)\MyProject.pdb"
				DebugInformationFormat="0"
			/>
		]]
	end


--
-- Floating point flag tests
--

	function suite.CompilerBlock_OnFpFast()
		flags { "FloatFast" }
		prepare()
		vc200x.VCCLCompilerTool(premake.getconfig(prj, "Debug"))
		test.capture [[
			<Tool
				Name="VCCLCompilerTool"
				Optimization="0"
				BasicRuntimeChecks="3"
				RuntimeLibrary="2"
				EnableFunctionLevelLinking="true"
				FloatingPointModel="2"
				UsePrecompiledHeader="0"
				WarningLevel="3"
				Detect64BitPortabilityProblems="true"
				ProgramDataBaseFileName="$(OutDir)\MyProject.pdb"
				DebugInformationFormat="0"
			/>
		]]
	end

	function suite.CompilerBlock_OnFpStrict()
		flags { "FloatStrict" }
		prepare()
		vc200x.VCCLCompilerTool(premake.getconfig(prj, "Debug"))
		test.capture [[
			<Tool
				Name="VCCLCompilerTool"
				Optimization="0"
				BasicRuntimeChecks="3"
				RuntimeLibrary="2"
				EnableFunctionLevelLinking="true"
				FloatingPointModel="1"
				UsePrecompiledHeader="0"
				WarningLevel="3"
				Detect64BitPortabilityProblems="true"
				ProgramDataBaseFileName="$(OutDir)\MyProject.pdb"
				DebugInformationFormat="0"
			/>
		]]
	end


--
-- PDB file naming tests
--

	function suite.CompilerBlock_OnTargetName()
		targetname "foob"
		prepare()
		vc200x.VCCLCompilerTool(premake.getconfig(prj, "Debug"))
		test.capture [[
			<Tool
				Name="VCCLCompilerTool"
				Optimization="0"
				BasicRuntimeChecks="3"
				RuntimeLibrary="2"
				EnableFunctionLevelLinking="true"
				UsePrecompiledHeader="0"
				WarningLevel="3"
				Detect64BitPortabilityProblems="true"
				ProgramDataBaseFileName="$(OutDir)\foob.pdb"
				DebugInformationFormat="0"
			/>
		]]
	end


--
-- Compilation option tests
--

	function suite.CompilerBlock_OnMinimalRebuild()
		flags { "Symbols", "EnableMinimalRebuild" }
		prepare()
		vc200x.VCCLCompilerTool(premake.getconfig(prj, "Debug"))
		test.capture [[
			<Tool
				Name="VCCLCompilerTool"
				Optimization="0"
				BasicRuntimeChecks="3"
				RuntimeLibrary="3"
				EnableFunctionLevelLinking="true"
				UsePrecompiledHeader="0"
				WarningLevel="3"
				Detect64BitPortabilityProblems="true"
				ProgramDataBaseFileName="$(OutDir)\MyProject.pdb"
				DebugInformationFormat="4"
			/>
		]]
	end


--
-- RuntimeLibrary tests
--

	function suite.CompilerBlock_RuntimeLibrary_IsDebug_OnSymbolsNoOptimize()
		flags { "Symbols" }
		prepare()
		vc200x.VCCLCompilerTool(premake.getconfig(prj, "Debug"))
		test.capture [[
			<Tool
				Name="VCCLCompilerTool"
				Optimization="0"
				MinimalRebuild="true"
				BasicRuntimeChecks="3"
				RuntimeLibrary="3"
				EnableFunctionLevelLinking="true"
				UsePrecompiledHeader="0"
				WarningLevel="3"
				Detect64BitPortabilityProblems="true"
				ProgramDataBaseFileName="$(OutDir)\MyProject.pdb"
				DebugInformationFormat="4"
			/>
		]]
	end

	function suite.CompilerBlock_RuntimeLibrary_IsRelease_OnOptimize()
		flags { "Symbols", "Optimize" }
		prepare()
		vc200x.VCCLCompilerTool(premake.getconfig(prj, "Debug"))
		test.capture [[
			<Tool
				Name="VCCLCompilerTool"
				Optimization="3"
				StringPooling="true"
				RuntimeLibrary="2"
				EnableFunctionLevelLinking="true"
				UsePrecompiledHeader="0"
				WarningLevel="3"
				Detect64BitPortabilityProblems="true"
				ProgramDataBaseFileName="$(OutDir)\MyProject.pdb"
				DebugInformationFormat="3"
			/>
		]]
	end


--
-- C language support
--

	function suite.CompilerBlock_RuntimeLibrary_IsDebug_OnSymbolsNoOptimize()
		language "C"
		flags { "Symbols" }
		prepare()
		vc200x.VCCLCompilerTool(premake.getconfig(prj, "Debug"))
		test.capture [[
			<Tool
				Name="VCCLCompilerTool"
				Optimization="0"
				MinimalRebuild="true"
				BasicRuntimeChecks="3"
				RuntimeLibrary="3"
				EnableFunctionLevelLinking="true"
				UsePrecompiledHeader="0"
				WarningLevel="3"
				Detect64BitPortabilityProblems="true"
				ProgramDataBaseFileName="$(OutDir)\MyProject.pdb"
				DebugInformationFormat="4"
				CompileAs="1"
			/>
		]]
	end
	
	
	function suite.noLinkIncrementalFlag_valueEqualsOne()
		flags { "NoIncrementalLink" }
		prepare()
		vc200x.VCLinkerTool(premake.getconfig(prj, "Debug"))
		local result = io.endcapture()		
		test.string_contains(result,'LinkIncremental="1"')
	end

	function suite.staticLib_platformX64_MachineX64SetInAdditionalOptions()
		local sln1 = solution "sol"
		configurations { "foo" }
		platforms {'x64'}

		local prj1 = project "prj"
		language 'C++'
		kind 'StaticLib'

		premake.bake.buildconfigs()
		sln1.vstudio_configs = premake.vstudio.buildconfigs(sln1)
		prj1= premake.getconfig(sln1.projects[1])
		vc200x.generate(prj1)
		local result = io.endcapture()		
		test.string_contains(result,'AdditionalOptions="/MACHINE:X64"')
	end

	function suite.staticLib_platformX32_MachineX86SetInAdditionalOptions()
		local sln1 = solution "sol"
		configurations { "foo" }
		platforms {'x32'}

		local prj1 = project "prj"
		language 'C++'
		kind 'StaticLib'

		premake.bake.buildconfigs()
		sln1.vstudio_configs = premake.vstudio.buildconfigs(sln1)
		prj1= premake.getconfig(sln1.projects[1])
		vc200x.generate(prj1)
		local result = io.endcapture()		
		test.string_contains(result,'AdditionalOptions="/MACHINE:X86"')
	end
