--
-- tests/actions/vstudio/sln2005/dependencies.lua
-- Validate generation of Visual Studio 2005+ solution project dependencies.
-- Copyright (c) 2009-2011 Jason Perkins and the Premake project
--

	T.vstudio_sln2005_dependencies = { }
	local suite = T.vstudio_sln2005_dependencies
	local sln2005 = premake.vstudio.sln2005


--
-- Setup 
--

	local sln, prj1, prj2
	
	function suite.setup()
		_ACTION = "vs2005"
		sln, prj1 = test.createsolution()
		uuid "AE61726D-187C-E440-BD07-2556188A6565"
		prj2 = test.createproject(sln)
		uuid "2151E83B-997F-4A9D-955D-380157E88C31"
		links "MyProject"
	end
	
	local function prepare(language)
		prj1.language = language
		prj2.language = language
		premake.bake.buildconfigs()
		prj1 = premake.solution.getproject(sln, 1)
		prj2 = premake.solution.getproject(sln, 2)
		sln2005.projectdependencies(prj2)
	end


--
-- Tests
--

	function suite.On2005_Cpp()
		prepare("C++")
		test.capture [[
	ProjectSection(ProjectDependencies) = postProject
		{AE61726D-187C-E440-BD07-2556188A6565} = {AE61726D-187C-E440-BD07-2556188A6565}
	EndProjectSection
		]]
	end


	function suite.On2005_Cs()
		prepare("C#")
		test.capture [[
	ProjectSection(ProjectDependencies) = postProject
		{AE61726D-187C-E440-BD07-2556188A6565} = {AE61726D-187C-E440-BD07-2556188A6565}
	EndProjectSection
		]]
	end
