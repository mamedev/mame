	T.vs2010_vcxproj = { }
	local vs10_vcxproj = T.vs2010_vcxproj
	local include_directory = "bar/foo"
	local include_directory2 = "baz/foo"
	local debug_define = "I_AM_ALIVE_NUMBER_FIVE"
	local vc2010 = premake.vstudio.vc2010

	local sln, prj
	function vs10_vcxproj.teardown()
		sln = nil
		prj = nil
	end
	function vs10_vcxproj.setup()
		_ACTION = "vs2010"

		sln = solution "MySolution"
		configurations { "Debug", "Release" }
		platforms {}

		prj = project "MyProject"
		language "C++"
		kind "ConsoleApp"
		uuid "AE61726D-187C-E440-BD07-2556188A6565"

		includedirs
		{
			include_directory,
			include_directory2
		}
		files
		{
			"foo/dummyHeader.h",
			"foo/dummySource.cpp",
			"../src/host/*h",
			"../src/host/*.c",
			"dummyResourceScript.rc"
		}

		configuration("Release")
			flags {"Optimize"}
			links{"foo","bar"}

		configuration("Debug")
			defines {debug_define}
			links{"foo_d"}

	end

	local function get_buffer()
		premake.bake.buildconfigs()
		sln.vstudio_configs = premake.vstudio.buildconfigs(sln)
		prj = premake.solution.getproject(sln, 1)
		premake.vs2010_vcxproj(prj)
		local buffer = io.endcapture()
		return buffer
	end



--
-- Tests
--

	function vs10_vcxproj.xmlDeclarationPresent()
		local buffer = get_buffer()
		test.istrue(string.startswith(buffer, '<?xml version=\"1.0\" encoding=\"utf-8\"?>'))
	end

	function vs10_vcxproj.projectBlocksArePresent()
		local buffer = get_buffer()
		test.string_contains(buffer,'<Project.*</Project>')
	end

	function vs10_vcxproj.projectOpenTagIsCorrect()
		local buffer = get_buffer()
		test.string_contains(buffer,'<Project DefaultTargets="Build" ToolsVersion="4.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">.*</Project>')
	end

	function vs10_vcxproj.configItemGroupPresent()
		local buffer = get_buffer()
		test.string_contains(buffer,'<ItemGroup Label="ProjectConfigurations">.*</ItemGroup>')
	end

	function vs10_vcxproj.configBlocksArePresent()
		local buffer = get_buffer()
		test.string_contains(buffer,'<ProjectConfiguration.*</ProjectConfiguration>')
	end

	function vs10_vcxproj.configTypeBlockPresent()
		local buffer = get_buffer()
		test.string_contains(buffer,'<PropertyGroup Condition="\'%$%(Configuration%)|%$%(Platform%)\'==\'.*\'" Label="Configuration">.*</PropertyGroup>')
	end

	function vs10_vcxproj.twoConfigTypeBlocksPresent()
		local buffer = get_buffer()
		test.string_contains(buffer,'<PropertyGroup Condition.*</PropertyGroup>.*<PropertyGroup Condition=.*</PropertyGroup>')
	end

	function vs10_vcxproj.propsDefaultForCppProjArePresent()
		local buffer = get_buffer()
		test.string_contains(buffer,'<Import Project="$%(VCTargetsPath%)\\Microsoft.Cpp.Default.props" />')
	end


	function vs10_vcxproj.propsForCppProjArePresent()
		local buffer = get_buffer()
		test.string_contains(buffer,'<Import Project="%$%(VCTargetsPath%)\\Microsoft.Cpp.props" />')
	end

	function vs10_vcxproj.extensionSettingArePresent()
		local buffer = get_buffer()
		test.string_contains(buffer,'<ImportGroup Label="ExtensionSettings">.*</ImportGroup>')
	end

	function vs10_vcxproj.userMacrosPresent()
		local buffer = get_buffer()
		test.string_contains(buffer,'<PropertyGroup Label="UserMacros" />')
	end

	function vs10_vcxproj.projectWithDebugAndReleaseConfig_twoOutDirsAndTwoIntDirs()
		local buffer = get_buffer()
		test.string_contains(buffer,'<OutDir.*</OutDir>.*<IntDir.*</IntDir>.*<OutDir.*</OutDir>.*<IntDir.*</IntDir>')
	end

	function vs10_vcxproj.containsItemDefinition()
		local buffer = get_buffer()
		test.string_contains(buffer,'<ItemDefinitionGroup Condition="\'%$%(Configuration%)|%$%(Platform%)\'==\'.*\'">.*</ItemDefinitionGroup>')
	end


	function vs10_vcxproj.containsClCompileBlock()
		local buffer = get_buffer()
		test.string_contains(buffer,'<ClCompile>.*</ClCompile>')
	end

	function vs10_vcxproj.containsAdditionalOptions()
		buildoptions {"/Gm"}
		local buffer = get_buffer()
		test.string_contains(buffer,'<AdditionalOptions>/Gm %%%(AdditionalOptions%)</AdditionalOptions>')
	end

	local function cl_compile_string(version)
		return '<ItemDefinitionGroup Condition="\'%$%(Configuration%)|%$%(Platform%)\'==\''..version..'|Win32\'">.*<ClCompile>'
	end

	function vs10_vcxproj.debugHasNoOptimisation()
		local buffer = get_buffer()
		test.string_contains(buffer, cl_compile_string('Debug').. '.*<Optimization>Disabled</Optimization>.*</ItemDefinitionGroup>')
	end

	function vs10_vcxproj.releaseHasFullOptimisation()
		local buffer = get_buffer()
		test.string_contains(buffer, cl_compile_string('Release').. '.*<Optimization>Full</Optimization>.*</ItemDefinitionGroup>')
	end

	function vs10_vcxproj.includeDirectories_debugEntryContains_include_directory()
		local buffer = get_buffer()
		test.string_contains(buffer,cl_compile_string('Debug').. '.*<AdditionalIncludeDirectories>'.. path.translate(include_directory, '\\') ..'.*</AdditionalIncludeDirectories>')
	end

	function vs10_vcxproj.includeDirectories_debugEntryContains_include_directory2PrefixWithSemiColon()
		local buffer = get_buffer()
		test.string_contains(buffer,cl_compile_string('Debug').. '.*<AdditionalIncludeDirectories>.*;'.. path.translate(include_directory2, '\\') ..'.*</AdditionalIncludeDirectories>')
	end

	function vs10_vcxproj.includeDirectories_debugEntryContains_include_directory2PostfixWithSemiColon()
		local buffer = get_buffer()
		test.string_contains(buffer,cl_compile_string('Debug').. '.*<AdditionalIncludeDirectories>.*'.. path.translate(include_directory2, '\\') ..';.*</AdditionalIncludeDirectories>')
	end

	function vs10_vcxproj.debugContainsPreprossorBlock()
		local buffer = get_buffer()
		test.string_contains(buffer,cl_compile_string('Debug').. '.*<PreprocessorDefinitions>.*</PreprocessorDefinitions>')
	end

	function vs10_vcxproj.debugHasDebugDefine()
		local buffer = get_buffer()
		test.string_contains(buffer,cl_compile_string('Debug')..'.*<PreprocessorDefinitions>.*'..debug_define..'.*</PreprocessorDefinitions>')
	end

	function vs10_vcxproj.releaseHasStringPoolingOn()
		local buffer = get_buffer()
		test.string_contains(buffer,cl_compile_string('Release')..'.*<StringPooling>true</StringPooling>')
	end

	function vs10_vcxproj.hasItemGroupSection()
		local buffer = get_buffer()
		test.string_contains(buffer,'<ItemGroup>.*</ItemGroup>')
	end

	function vs10_vcxproj.itemGroupSection_hasResourceCompileSection()
		--for some reason this does not work here and it needs to be in
		--the project setting at the top ?
		--files{"dummyResourceScript.rc"}
		local buffer = get_buffer()
		test.string_contains(buffer,'<ItemGroup>.*<ResourceCompile.*</ItemGroup>')
	end

	function vs10_vcxproj.checkProjectConfigurationOpeningTag_hasACloseingAngleBracket()
		local buffer = get_buffer()
		test.string_contains(buffer,'<ProjectConfiguration Include="Debug|Win32">')
	end

	function vs10_vcxproj.postBuildEvent_isPresent()
		postbuildcommands { "doSomeThing" }
		local buffer = get_buffer()
		test.string_contains(buffer,'<PostBuildEvent>.*<Command>.*</Command>.*</PostBuildEvent>')
	end

	function vs10_vcxproj.postBuildEvent_containsCorrectInformationBetweenCommandTag()
		postbuildcommands { "doSomeThing" }
		local buffer = get_buffer()
		test.string_contains(buffer,'<PostBuildEvent>.*<Command>doSomeThing</Command>.*</PostBuildEvent>')
	end

	function vs10_vcxproj.postBuildEvent_eventEncloseByQuotes_containsCorrectInformationBetweenCommandTag()
		postbuildcommands { "\"doSomeThing\"" }
		local buffer = get_buffer()
		test.string_contains(buffer,'<PostBuildEvent>.*<Command>&quot;doSomeThing&quot;</Command>.*</PostBuildEvent>')
	end

	function vs10_vcxproj.noExtraWarnings_bufferDoesNotContainSmallerTypeCheck()
		local buffer = get_buffer()
		test.string_does_not_contain(buffer,'<SmallerTypeCheck>')
	end

	function vs10_vcxproj.debugAndExtraWarnings_bufferContainsSmallerTypeCheck()
		configuration("Debug")
		flags {"ExtraWarnings"}
		local buffer = get_buffer()
		test.string_contains(buffer,'<SmallerTypeCheck>true</SmallerTypeCheck>')
	end

	function vs10_vcxproj.releaseAndExtraWarnings_bufferDoesNotContainSmallerTypeCheck()
		configuration("Release")
		flags {"ExtraWarnings"}
		local buffer = get_buffer()
		test.string_does_not_contain(buffer,'<SmallerTypeCheck>')
	end

	function vs10_vcxproj.onlyOneProjectConfigurationBlockWhenMultipleConfigs()
		local buffer = get_buffer()
		test.string_does_not_contain(buffer,'<ItemGroup Label="ProjectConfigurations">.*<ItemGroup Label="ProjectConfigurations">')
	end

	function vs10_vcxproj.languageC_bufferContainsCompileAsC()
		language "C"
		local buffer = get_buffer()
		test.string_contains(buffer,'<CompileAs>CompileAsC</CompileAs>')
	end

	local debug_config_pch_string = '<PrecompiledHeader Condition="\'%$%(Configuration%)|%$%(Platform%)\'==\'Debug|Win32\'">Create</PrecompiledHeader>'
	local release_config_pch_string = debug_config_pch_string:gsub('Debug','Release')

	function vs10_vcxproj.noPchFlagSet_bufferDoesNotContainPchCreate()
		configuration("Debug")
		flags{"NoPCH"}
		local buffer = get_buffer()
		test.string_does_not_contain(buffer,debug_config_pch_string)
	end

	function vs10_vcxproj.pchHeaderSetYetPchSourceIsNot_bufferDoesNotContainPchCreate()
		configuration("Debug")
		pchheader "foo/dummyHeader.h"
		local buffer = get_buffer()
		test.string_does_not_contain(buffer,debug_config_pch_string)
	end

	function vs10_vcxproj.pchHeaderAndSourceSet_yetAlsoNoPch_bufferDoesNotContainpchCreate()
		configuration('Debug')
			pchheader "foo/dummyHeader.h"
			pchsource "foo/dummySource.cpp"
			flags{"NoPCH"}
		local buffer = get_buffer()
		test.string_does_not_contain(buffer,debug_config_pch_string)
	end

	function vs10_vcxproj.pchHeaderAndPchSourceSet_bufferContainPchCreate()
		configuration("Debug")
			pchheader "foo/dummyHeader.h"
			pchsource "foo/dummySource.cpp"
		local buffer = get_buffer()
		test.string_contains(buffer,debug_config_pch_string)
	end

	function vs10_vcxproj.wholeProgramOptimizationIsNotSetByDefault_bufferDoesNotContainWholeProgramOptimization()
		local buffer = get_buffer()
		test.string_does_not_contain(buffer,"WholeProgramOptimization")
	end
