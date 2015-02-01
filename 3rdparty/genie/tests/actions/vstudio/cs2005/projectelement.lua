--
-- tests/actions/vstudio/cs2005/projectelement.lua
-- Validate generation of <Project/> element in Visual Studio 2005+ .csproj
-- Copyright (c) 2009-2011 Jason Perkins and the Premake project
--

	T.vstudio_cs2005_projectelement = { }
	local suite = T.vstudio_cs2005_projectelement
	local cs2005 = premake.vstudio.cs2005


--
-- Setup
--

	local sln, prj

	function suite.setup()
		sln = test.createsolution()
	end

	local function prepare()
		premake.bake.buildconfigs()
		prj = premake.solution.getproject(sln, 1)
		cs2005.projectelement(prj)
	end


--
-- Tests
--

	function suite.On2005()
		_ACTION = "vs2005"
		prepare()
		test.capture [[
<Project DefaultTargets="Build" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
		]]
	end

	function suite.On2008()
		_ACTION = "vs2008"
		prepare()
		test.capture [[
<Project ToolsVersion="3.5" DefaultTargets="Build" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
		]]
	end

	function suite.On2010()
		_ACTION = "vs2010"
		prepare()
		test.capture [[
<?xml version="1.0" encoding="utf-8"?>
<Project ToolsVersion="4.0" DefaultTargets="Build" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
		]]
	end

	function suite.On2012()
		_ACTION = "vs2012"
		prepare()
		test.capture [[
<?xml version="1.0" encoding="utf-8"?>
<Project ToolsVersion="4.0" DefaultTargets="Build" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
		]]
	end

	function suite.On2013()
		_ACTION = "vs2013"
		prepare()
		test.capture [[
<?xml version="1.0" encoding="utf-8"?>
<Project ToolsVersion="12.0" DefaultTargets="Build" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
		]]
	end
