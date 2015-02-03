	T.vs2010_project_kinds= { }
	local vs10_project_kinds = T.vs2010_project_kinds
	local sln, prj

	function vs10_project_kinds.setup()
		_ACTION = "vs2010"

		sln = solution "MySolution"
		configurations { "Debug" }
		platforms {}

		prj = project "MyProject"
		language "C++"
	end

	local function get_buffer(platform)
		premake.bake.buildconfigs()
		sln.vstudio_configs = premake.vstudio.buildconfigs(sln)
		prj = premake.solution.getproject(sln, 1)
		premake.vs2010_vcxproj(prj)
		buffer = io.endcapture()
		return buffer
	end


	function vs10_project_kinds.staticLib_containsLibSection()
		kind "StaticLib"
		local buffer = get_buffer()
		test.string_contains(buffer,'<ItemDefinitionGroup.*<Lib>.*</Lib>.*</ItemDefinitionGroup>')
	end
	function vs10_project_kinds.staticLib_libSection_containsProjectNameDotLib()
		kind "StaticLib"
		local buffer = get_buffer()
		test.string_contains(buffer,'<Lib>.*<OutputFile>.*MyProject.lib.*</OutputFile>.*</Lib>')
	end


	function vs10_project_kinds.staticLib_valueInMinimalRebuildIsTrue()
		kind "StaticLib"
		flags  {"Symbols"}
		local buffer = get_buffer()
		test.string_contains(buffer,'<ClCompile>.*<MinimalRebuild>true</MinimalRebuild>.*</ClCompile>')
	end
	function vs10_project_kinds.sharedLib_valueInMinimalRebuildIsTrue()
		kind "SharedLib"
		flags  {"Symbols"}
		local buffer = get_buffer()
		test.string_contains(buffer,'<ClCompile>.*<MinimalRebuild>true</MinimalRebuild>.*</ClCompile>')
	end
	function vs10_project_kinds.sharedLib_valueDebugInformationFormatIsEditAndContinue()
		kind "SharedLib"
		flags  {"Symbols"}
		local buffer = get_buffer()
		test.string_contains(buffer,'<ClCompile>.*<DebugInformationFormat>EditAndContinue</DebugInformationFormat>.*</ClCompile>')
	end
	function vs10_project_kinds.sharedLib_valueGenerateDebugInformationIsTrue()
		kind "SharedLib"
		flags  {"Symbols"}
		local buffer = get_buffer()
		test.string_contains(buffer,'<Link>.*<GenerateDebugInformation>true</GenerateDebugInformation>.*</Link>')
	end
	function vs10_project_kinds.sharedLib_linkSectionContainsImportLibrary()
		kind "SharedLib"
		local buffer = get_buffer()
		test.string_contains(buffer,'<Link>.*<ImportLibrary>.*</ImportLibrary>.*</Link>')
	end

	function vs10_project_kinds.sharedLib_withoutOptimisation_linkIncrementalValueIsTrue()
		kind "SharedLib"
		local buffer = get_buffer()
		test.string_contains(buffer,'<LinkIncremental.*true</LinkIncremental>')
	end

	function vs10_project_kinds.sharedLib_withOptimisation_linkIncrementalValueIsFalse()
		kind "SharedLib"
		flags{"Optimize"}
		local buffer = get_buffer()
		test.string_contains(buffer,'<LinkIncremental.*false</LinkIncremental>')
	end

	function vs10_project_kinds.kindDoesNotMatter_noAdditionalDirectoriesSpecified_bufferDoesNotContainAdditionalIncludeDirectories()
		kind "SharedLib"
		local buffer = get_buffer()
		test.string_does_not_contain(buffer,'<ClCompile>.*<AdditionalIncludeDirectories>.*</ClCompile>')
	end

	function vs10_project_kinds.configType_configIsWindowedApp_resultComparesEqualToApplication()
		local t = { kind = "WindowedApp"}
		local result = premake.vstudio.vc2010.config_type(t)
		test.isequal('Application',result)
	end

	function vs10_project_kinds.linkOptions_staticLib_bufferContainsAdditionalOptionsInSideLibTag()
		kind "StaticLib"
		linkoptions{'/dummyOption'}

		test.string_contains(get_buffer(),
			'<AdditionalOptions>.*%%%(AdditionalOptions%)</AdditionalOptions>.*</Lib>')
	end

	function vs10_project_kinds.noLinkOptions_staticLib_bufferDoesNotContainAdditionalOptionsInSideLibTag()
		kind "StaticLib"

		test.string_does_not_contain(get_buffer(),
			'<AdditionalOptions>.*%%%(AdditionalOptions%)</AdditionalOptions>.*</Lib>')
	end

	function vs10_project_kinds.linkOptions_staticLib_bufferContainsPassedOption()
		kind "StaticLib"
		linkoptions{'/dummyOption'}

		test.string_contains(get_buffer(),
			'<AdditionalOptions>/dummyOption %%%(AdditionalOptions%)</AdditionalOptions>.*</Lib>')
	end

	function vs10_project_kinds.linkOptions_windowedApp_bufferContainsAdditionalOptionsInSideLinkTag()
		kind "WindowedApp"
		linkoptions{'/dummyOption'}

		test.string_contains(get_buffer(),
			'<AdditionalOptions>.* %%%(AdditionalOptions%)</AdditionalOptions>.*</Link>')
	end
	function vs10_project_kinds.linkOptions_consoleApp_bufferContainsAdditionalOptionsInSideLinkTag()
		kind "ConsoleApp"
		linkoptions{'/dummyOption'}

		test.string_contains(get_buffer(),
			'<AdditionalOptions>.* %%%(AdditionalOptions%)</AdditionalOptions>.*</Link>')
	end

	function vs10_project_kinds.linkOptions_sharedLib_bufferContainsAdditionalOptionsInSideLinkTag()
		kind "SharedLib"
		linkoptions{'/dummyOption'}

		test.string_contains(get_buffer(),
			'<AdditionalOptions>.* %%%(AdditionalOptions%)</AdditionalOptions>.*</Link>')
	end


	function vs10_project_kinds.staticLibX64_TargetMachineSetInLib()
		kind "StaticLib"
		platforms{'x64'}
		local buffer = get_buffer()
		test.string_contains(buffer,'<Lib>.*<TargetMachine>.*</TargetMachine>.*</Lib>')
	end

	function vs10_project_kinds.staticLibX64_TargetMachineInLibSetToMachineX64()
		kind "StaticLib"
		platforms{'x64'}
		local buffer = get_buffer()
		test.string_contains(buffer,'<Lib>.*<TargetMachine>MachineX64</TargetMachine>.*</Lib>')
	end

	function vs10_project_kinds.staticLibX32_TargetMachineSetInLib()
		kind "StaticLib"
		platforms{'x32'}
		local buffer = get_buffer()
		test.string_contains(buffer,'<Lib>.*<TargetMachine>.*</TargetMachine>.*</Lib>')
	end

	function vs10_project_kinds.staticLibX32_TargetMachineInLibSetToMachineX86()
		kind "StaticLib"
		platforms{'x32'}
		local buffer = get_buffer()
		test.string_contains(buffer,'<Lib>.*<TargetMachine>MachineX86</TargetMachine>.*</Lib>')
	end
