--
-- tests/actions/vstudio/sln2005/platforms.lua
-- Validate generation of Visual Studio 2005+ SolutionConfigurationPlatforms block.
-- Copyright (c) 2009-2011 Jason Perkins and the Premake project
--

	T.vstudio_sln2005_platforms = { }
	local suite = T.vstudio_sln2005_platforms
	local sln2005 = premake.vstudio.sln2005


--
-- Setup 
--

	local sln, prj
	
	function suite.setup()
		sln, prj = test.createsolution()
	end
	
	local function prepare(language)
		prj.language = language
		premake.bake.buildconfigs()
		sln.vstudio_configs = premake.vstudio.buildconfigs(sln)
		sln2005.platforms(sln)
	end


--
-- C/C++ Tests
--

	function suite.On2005_Cpp()
		_ACTION = "vs2005"
		prepare("C++")
		test.capture [[
	GlobalSection(SolutionConfigurationPlatforms) = preSolution
		Debug|Win32 = Debug|Win32
		Release|Win32 = Release|Win32
	EndGlobalSection
		]]
	end


--
-- C# Tests
--

	function suite.On2005_Cs()
		_ACTION = "vs2005"
		prepare("C#")
		test.capture [[
	GlobalSection(SolutionConfigurationPlatforms) = preSolution
		Debug|Any CPU = Debug|Any CPU
		Debug|Win32 = Debug|Win32
		Release|Any CPU = Release|Any CPU
		Release|Win32 = Release|Win32
	EndGlobalSection
		]]
	end


	function suite.On2010_Cs()
		_ACTION = "vs2010"
		prepare("C#")
		test.capture [[
	GlobalSection(SolutionConfigurationPlatforms) = preSolution
		Debug|Any CPU = Debug|Any CPU
		Debug|Mixed Platforms = Debug|Mixed Platforms
		Debug|x86 = Debug|x86
		Release|Any CPU = Release|Any CPU
		Release|Mixed Platforms = Release|Mixed Platforms
		Release|x86 = Release|x86
	EndGlobalSection
		]]
	end


--
-- Mixed language tests
--

	function suite.On2005_MixedLanguages()
		_ACTION = "vs2005"
		test.createproject(sln)
		prepare("C#")
		test.capture [[
	GlobalSection(SolutionConfigurationPlatforms) = preSolution
		Debug|Any CPU = Debug|Any CPU
		Debug|Mixed Platforms = Debug|Mixed Platforms
		Debug|Win32 = Debug|Win32
		Release|Any CPU = Release|Any CPU
		Release|Mixed Platforms = Release|Mixed Platforms
		Release|Win32 = Release|Win32
	EndGlobalSection
		]]
	end


	function suite.On2010_MixedLanguages()
		_ACTION = "vs2010"
		test.createproject(sln)
		prepare("C#")
		test.capture [[
	GlobalSection(SolutionConfigurationPlatforms) = preSolution
		Debug|Mixed Platforms = Debug|Mixed Platforms
		Debug|Win32 = Debug|Win32
		Debug|x86 = Debug|x86
		Release|Mixed Platforms = Release|Mixed Platforms
		Release|Win32 = Release|Win32
		Release|x86 = Release|x86
	EndGlobalSection
		]]
	end


--
-- Test multiple platforms
--

	function suite.On2005_MixedPlatforms()
		_ACTION = "vs2005"
		platforms { "x32", "x64" }
		prepare("C++")
		test.capture [[
	GlobalSection(SolutionConfigurationPlatforms) = preSolution
		Debug|Win32 = Debug|Win32
		Debug|x64 = Debug|x64
		Release|Win32 = Release|Win32
		Release|x64 = Release|x64
	EndGlobalSection
		]]
	end

