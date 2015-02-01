--
-- tests/actions/vstudio/cs2005/propertygroup.lua
-- Validate configuration <PropertyGroup/> elements in Visual Studio 2005+ .csproj
-- Copyright (c) 2009-2011 Jason Perkins and the Premake project
--

	T.vstudio_cs2005_propertygroup = { }
	local suite = T.vstudio_cs2005_propertygroup
	local cs2005 = premake.vstudio.cs2005


--
-- Setup 
--

	local sln, prj, cfg
	
	function suite.setup()
		sln = test.createsolution()
		language "C#"
	end
	
	local function prepare()
		premake.bake.buildconfigs()
		prj = premake.solution.getproject(sln, 1)
		cfg = premake.getconfig(prj, "Debug")
		cs2005.propertygroup(cfg)
	end


--
-- Version Tests
--

	function suite.OnVs2005()
		_ACTION = "vs2005"
		prepare()
		test.capture [[
  <PropertyGroup Condition=" '$(Configuration)|$(Platform)' == 'Debug|AnyCPU' ">
		]]
	end


	function suite.OnVs2008()
		_ACTION = "vs2008"
		prepare()
		test.capture [[
  <PropertyGroup Condition=" '$(Configuration)|$(Platform)' == 'Debug|AnyCPU' ">
		]]
	end


	function suite.OnVs2010()
		_ACTION = "vs2010"
		prepare()
		test.capture [[
  <PropertyGroup Condition=" '$(Configuration)|$(Platform)' == 'Debug|AnyCPU' ">
		]]
	end
