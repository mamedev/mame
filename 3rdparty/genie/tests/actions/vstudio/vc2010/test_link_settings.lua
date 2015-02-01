--
-- tests/actions/vstudio/vc2010/test_link_settings.lua
-- Validate linker settings in Visual Studio 2010 C/C++ projects.
-- Copyright (c) 2011 Jason Perkins and the Premake project
--

	T.vstudio_vs2010_link_settings = { }
	local suite = T.vstudio_vs2010_link_settings
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
		vc2010.link(cfg)
	end


--
-- Check the basic element structure for a console application.
--

	function suite.writesCorrectSubsystem_onConsoleApp()
		kind "ConsoleApp"
		prepare()
		test.capture [[
		<Link>
			<SubSystem>Console</SubSystem>
			<GenerateDebugInformation>false</GenerateDebugInformation>
			<OutputFile>$(OutDir)MyProject.exe</OutputFile>
			<EntryPointSymbol>mainCRTStartup</EntryPointSymbol>
		</Link>
		]]
	end


--
-- Check the basic element structure for a windowed application.
--

	function suite.writesCorrectSubsystem_onWindowedApp()
		kind "WindowedApp"
		prepare()
		test.capture [[
		<Link>
			<SubSystem>Windows</SubSystem>
			<GenerateDebugInformation>false</GenerateDebugInformation>
			<OutputFile>$(OutDir)MyProject.exe</OutputFile>
			<EntryPointSymbol>mainCRTStartup</EntryPointSymbol>
		</Link>
		]]
	end


--
-- Check the basic element structure for a shared library.
--

	function suite.writesCorrectSubsystem_onSharedLib()
		kind "SharedLib"
		prepare()
		test.capture [[
		<Link>
			<SubSystem>Windows</SubSystem>
			<GenerateDebugInformation>false</GenerateDebugInformation>
			<OutputFile>$(OutDir)MyProject.dll</OutputFile>
			<ImportLibrary>MyProject.lib</ImportLibrary>
		</Link>
		]]
	end


--
-- Check the basic element structure for a static library.
--

	function suite.writesCorrectSubsystem_onStaticLib()
		kind "StaticLib"
		prepare()
		test.capture [[
		<Link>
			<SubSystem>Windows</SubSystem>
			<GenerateDebugInformation>false</GenerateDebugInformation>
		</Link>
		]]
	end


--
-- Check the structure of the additional library directories element.
--

	function suite.additionalLibraryDirectories()
		libdirs { "include/GL", "include/lua" }
		prepare()
		test.capture [[
		<Link>
			<SubSystem>Console</SubSystem>
			<GenerateDebugInformation>false</GenerateDebugInformation>
			<OutputFile>$(OutDir)MyProject.exe</OutputFile>
			<AdditionalLibraryDirectories>include\GL;include\lua;%(AdditionalLibraryDirectories)</AdditionalLibraryDirectories>
			<EntryPointSymbol>mainCRTStartup</EntryPointSymbol>
		</Link>
		]]
	end


--
-- Enable debug information if the Symbols flag is specified.
--

	function suite.generateDebugInformation_onSymbolsFlag()
		flags { "Symbols" }
		prepare()
		test.capture [[
		<Link>
			<SubSystem>Console</SubSystem>
			<GenerateDebugInformation>true</GenerateDebugInformation>
			<OutputFile>$(OutDir)MyProject.exe</OutputFile>
			<EntryPointSymbol>mainCRTStartup</EntryPointSymbol>
		</Link>
		]]
	end


--
-- Enable reference optimizing if Optimize flag is specified.
--

	function suite.optimizeReferences_onOptimizeFlag()
		flags { "Optimize" }
		prepare()
		test.capture [[
		<Link>
			<SubSystem>Console</SubSystem>
			<GenerateDebugInformation>false</GenerateDebugInformation>
			<EnableCOMDATFolding>true</EnableCOMDATFolding>
			<OptimizeReferences>true</OptimizeReferences>
			<OutputFile>$(OutDir)MyProject.exe</OutputFile>
			<EntryPointSymbol>mainCRTStartup</EntryPointSymbol>
		</Link>
		]]
	end


--
-- Skip the entry point override if the WinMain flag is specified.
--

	function suite.noEntryPointElement_onWinMainFlag()
		flags { "WinMain" }
		prepare()
		test.capture [[
		<Link>
			<SubSystem>Console</SubSystem>
			<GenerateDebugInformation>false</GenerateDebugInformation>
			<OutputFile>$(OutDir)MyProject.exe</OutputFile>
		</Link>
		]]
	end


--
-- Use the x86 target for Premake's x32 platform.
--

	function suite.writesCorrectTarget_onX32Platform()
		platforms "x32"
		prepare("x32")
		test.capture [[
		<Link>
			<SubSystem>Console</SubSystem>
			<GenerateDebugInformation>false</GenerateDebugInformation>
			<OutputFile>$(OutDir)MyProject.exe</OutputFile>
			<EntryPointSymbol>mainCRTStartup</EntryPointSymbol>
			<TargetMachine>MachineX86</TargetMachine>
		</Link>
		]]
	end


--
-- Use the x64 target for Premake's x64 platform.
--

	function suite.writesCorrectTarget_onX64Platform()
		platforms { "x64" }
		prepare("x64")
		test.capture [[
		<Link>
			<SubSystem>Console</SubSystem>
			<GenerateDebugInformation>false</GenerateDebugInformation>
			<OutputFile>$(OutDir)MyProject.exe</OutputFile>
			<EntryPointSymbol>mainCRTStartup</EntryPointSymbol>
			<TargetMachine>MachineX64</TargetMachine>
		</Link>
		]]
	end


--
-- Correctly handle module definition (.def) files.
--

	function suite.recognizesModuleDefinitionFile()
		files { "hello.cpp", "hello.def" }
		prepare()
		test.capture [[
		<Link>
			<SubSystem>Console</SubSystem>
			<GenerateDebugInformation>false</GenerateDebugInformation>
			<OutputFile>$(OutDir)MyProject.exe</OutputFile>
			<EntryPointSymbol>mainCRTStartup</EntryPointSymbol>
			<ModuleDefinitionFile>hello.def</ModuleDefinitionFile>
		</Link>
		]]
	end
