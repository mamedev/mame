--
-- tests/actions/vstudio/sln2005/layout.lua
-- Validate the overall layout of VS 2005-2010 solutions.
-- Copyright (c) 2009-2011 Jason Perkins and the Premake project
--

	T.vstudio_sln2005_layout = { }
	local suite = T.vstudio_sln2005_layout
	local sln2005 = premake.vstudio.sln2005


	local sln

	function suite.setup()
		_ACTION = "vs2005"
		sln = test.createsolution()
		uuid "AE61726D-187C-E440-BD07-2556188A6565"
	end

	local function prepare()
		premake.bake.buildconfigs()
		sln.vstudio_configs = premake.vstudio.buildconfigs(sln)
		sln2005.generate(sln)
	end	

	
	function suite.BasicLayout()
		prepare()
		test.capture ('\239\187\191' .. [[

Microsoft Visual Studio Solution File, Format Version 9.00
# Visual Studio 2005
Project("{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}") = "MyProject", "MyProject.vcproj", "{AE61726D-187C-E440-BD07-2556188A6565}"
EndProject
Global
	GlobalSection(SolutionConfigurationPlatforms) = preSolution
		Debug|Win32 = Debug|Win32
		Release|Win32 = Release|Win32
	EndGlobalSection
	GlobalSection(ProjectConfigurationPlatforms) = postSolution
		{AE61726D-187C-E440-BD07-2556188A6565}.Debug|Win32.ActiveCfg = Debug|Win32
		{AE61726D-187C-E440-BD07-2556188A6565}.Debug|Win32.Build.0 = Debug|Win32
		{AE61726D-187C-E440-BD07-2556188A6565}.Release|Win32.ActiveCfg = Release|Win32
		{AE61726D-187C-E440-BD07-2556188A6565}.Release|Win32.Build.0 = Release|Win32
	EndGlobalSection
	GlobalSection(SolutionProperties) = preSolution
		HideSolutionNode = FALSE
	EndGlobalSection
EndGlobal
		]])
	end
