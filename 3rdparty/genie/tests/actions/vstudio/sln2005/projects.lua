--
-- tests/actions/vstudio/sln2005/projects.lua
-- Validate generation of Visual Studio 2005+ solution project entries.
-- Copyright (c) 2009-2011 Jason Perkins and the Premake project
--

	T.vstudio_sln2005_projects = { }
	local suite = T.vstudio_sln2005_projects
	local sln2005 = premake.vstudio.sln2005


--
-- Setup 
--

	local sln, prj
	
	function suite.setup()
		_ACTION = "vs2005"
		sln = test.createsolution()
		uuid "AE61726D-187C-E440-BD07-2556188A6565"
	end
	
	local function prepare()
		premake.bake.buildconfigs()
		prj = premake.solution.getproject(sln, 1)
		sln2005.project(prj)
	end


--
-- C/C++ project reference tests
--

	function suite.On2005_CppProject()
		_ACTION = "vs2005"
		prepare()
		test.capture [[
Project("{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}") = "MyProject", "MyProject.vcproj", "{AE61726D-187C-E440-BD07-2556188A6565}"
EndProject
		]]
	end


	function suite.On2010_CppProject()
		_ACTION = "vs2010"
		prepare()
		test.capture [[
Project("{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}") = "MyProject", "MyProject.vcxproj", "{AE61726D-187C-E440-BD07-2556188A6565}"
EndProject
		]]
	end


--
-- C# project reference tests
--

	function suite.On2005_CsProject()
		_ACTION = "vs2005"
		language "C#"
		prepare()
		test.capture [[
Project("{FAE04EC0-301F-11D3-BF4B-00C04F79EFBC}") = "MyProject", "MyProject.csproj", "{AE61726D-187C-E440-BD07-2556188A6565}"
EndProject
		]]
	end
