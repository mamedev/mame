--
-- tests/actions/vstudio/sln2005/projectplatforms.lua
-- Validate generation of Visual Studio 2005+ ProjectConfigurationPlatforms block.
-- Copyright (c) 2009-2011 Jason Perkins and the Premake project
--

	T.vstudio_sln2005_projectplatforms = { }
	local suite = T.vstudio_sln2005_projectplatforms
	local sln2005 = premake.vstudio.sln2005


--
-- Setup 
--

	local sln, prj
	
	function suite.setup()
		sln, prj = test.createsolution()
		uuid "C9135098-6047-8142-B10E-D27E7F73FCB3"
	end
	
	local function prepare(language)
		prj.language = language
		premake.bake.buildconfigs()
		sln.vstudio_configs = premake.vstudio.buildconfigs(sln)
		sln2005.project_platforms(sln)
	end


	function suite.ProjectPlatforms_OnMixedLanguages()
		_ACTION = "vs2005"
		test.createproject(sln)
		uuid "AE61726D-187C-E440-BD07-2556188A6565"
		prepare("C#")
		test.capture [[
	GlobalSection(ProjectConfigurationPlatforms) = postSolution
		{C9135098-6047-8142-B10E-D27E7F73FCB3}.Debug|Any CPU.ActiveCfg = Debug|Any CPU
		{C9135098-6047-8142-B10E-D27E7F73FCB3}.Debug|Any CPU.Build.0 = Debug|Any CPU
		{C9135098-6047-8142-B10E-D27E7F73FCB3}.Debug|Mixed Platforms.ActiveCfg = Debug|Any CPU
		{C9135098-6047-8142-B10E-D27E7F73FCB3}.Debug|Mixed Platforms.Build.0 = Debug|Any CPU
		{C9135098-6047-8142-B10E-D27E7F73FCB3}.Debug|Win32.ActiveCfg = Debug|Any CPU
		{C9135098-6047-8142-B10E-D27E7F73FCB3}.Release|Any CPU.ActiveCfg = Release|Any CPU
		{C9135098-6047-8142-B10E-D27E7F73FCB3}.Release|Any CPU.Build.0 = Release|Any CPU
		{C9135098-6047-8142-B10E-D27E7F73FCB3}.Release|Mixed Platforms.ActiveCfg = Release|Any CPU
		{C9135098-6047-8142-B10E-D27E7F73FCB3}.Release|Mixed Platforms.Build.0 = Release|Any CPU
		{C9135098-6047-8142-B10E-D27E7F73FCB3}.Release|Win32.ActiveCfg = Release|Any CPU
		{AE61726D-187C-E440-BD07-2556188A6565}.Debug|Any CPU.ActiveCfg = Debug|Win32
		{AE61726D-187C-E440-BD07-2556188A6565}.Debug|Mixed Platforms.ActiveCfg = Debug|Win32
		{AE61726D-187C-E440-BD07-2556188A6565}.Debug|Mixed Platforms.Build.0 = Debug|Win32
		{AE61726D-187C-E440-BD07-2556188A6565}.Debug|Win32.ActiveCfg = Debug|Win32
		{AE61726D-187C-E440-BD07-2556188A6565}.Debug|Win32.Build.0 = Debug|Win32
		{AE61726D-187C-E440-BD07-2556188A6565}.Release|Any CPU.ActiveCfg = Release|Win32
		{AE61726D-187C-E440-BD07-2556188A6565}.Release|Mixed Platforms.ActiveCfg = Release|Win32
		{AE61726D-187C-E440-BD07-2556188A6565}.Release|Mixed Platforms.Build.0 = Release|Win32
		{AE61726D-187C-E440-BD07-2556188A6565}.Release|Win32.ActiveCfg = Release|Win32
		{AE61726D-187C-E440-BD07-2556188A6565}.Release|Win32.Build.0 = Release|Win32
	EndGlobalSection
		]]
	end


	function suite.ProjectPlatforms_OnMultiplePlatformsAndMixedModes()
		_ACTION = "vs2005"
		platforms { "x32", "x64" }
		test.createproject(sln)
		uuid "AE61726D-187C-E440-BD07-2556188A6565"
		prepare("C#")
		test.capture [[
	GlobalSection(ProjectConfigurationPlatforms) = postSolution
		{C9135098-6047-8142-B10E-D27E7F73FCB3}.Debug|Any CPU.ActiveCfg = Debug|Any CPU
		{C9135098-6047-8142-B10E-D27E7F73FCB3}.Debug|Any CPU.Build.0 = Debug|Any CPU
		{C9135098-6047-8142-B10E-D27E7F73FCB3}.Debug|Mixed Platforms.ActiveCfg = Debug|Any CPU
		{C9135098-6047-8142-B10E-D27E7F73FCB3}.Debug|Mixed Platforms.Build.0 = Debug|Any CPU
		{C9135098-6047-8142-B10E-D27E7F73FCB3}.Debug|Win32.ActiveCfg = Debug|Any CPU
		{C9135098-6047-8142-B10E-D27E7F73FCB3}.Debug|x64.ActiveCfg = Debug|Any CPU
		{C9135098-6047-8142-B10E-D27E7F73FCB3}.Release|Any CPU.ActiveCfg = Release|Any CPU
		{C9135098-6047-8142-B10E-D27E7F73FCB3}.Release|Any CPU.Build.0 = Release|Any CPU
		{C9135098-6047-8142-B10E-D27E7F73FCB3}.Release|Mixed Platforms.ActiveCfg = Release|Any CPU
		{C9135098-6047-8142-B10E-D27E7F73FCB3}.Release|Mixed Platforms.Build.0 = Release|Any CPU
		{C9135098-6047-8142-B10E-D27E7F73FCB3}.Release|Win32.ActiveCfg = Release|Any CPU
		{C9135098-6047-8142-B10E-D27E7F73FCB3}.Release|x64.ActiveCfg = Release|Any CPU
		{AE61726D-187C-E440-BD07-2556188A6565}.Debug|Any CPU.ActiveCfg = Debug|Win32
		{AE61726D-187C-E440-BD07-2556188A6565}.Debug|Mixed Platforms.ActiveCfg = Debug|Win32
		{AE61726D-187C-E440-BD07-2556188A6565}.Debug|Mixed Platforms.Build.0 = Debug|Win32
		{AE61726D-187C-E440-BD07-2556188A6565}.Debug|Win32.ActiveCfg = Debug|Win32
		{AE61726D-187C-E440-BD07-2556188A6565}.Debug|Win32.Build.0 = Debug|Win32
		{AE61726D-187C-E440-BD07-2556188A6565}.Debug|x64.ActiveCfg = Debug|x64
		{AE61726D-187C-E440-BD07-2556188A6565}.Debug|x64.Build.0 = Debug|x64
		{AE61726D-187C-E440-BD07-2556188A6565}.Release|Any CPU.ActiveCfg = Release|Win32
		{AE61726D-187C-E440-BD07-2556188A6565}.Release|Mixed Platforms.ActiveCfg = Release|Win32
		{AE61726D-187C-E440-BD07-2556188A6565}.Release|Mixed Platforms.Build.0 = Release|Win32
		{AE61726D-187C-E440-BD07-2556188A6565}.Release|Win32.ActiveCfg = Release|Win32
		{AE61726D-187C-E440-BD07-2556188A6565}.Release|Win32.Build.0 = Release|Win32
		{AE61726D-187C-E440-BD07-2556188A6565}.Release|x64.ActiveCfg = Release|x64
		{AE61726D-187C-E440-BD07-2556188A6565}.Release|x64.Build.0 = Release|x64
	EndGlobalSection
			]]
	end
