--
-- tests/test_baking.lua
-- Automated test suite for the configuration baking functions.
-- Copyright (c) 2009, 2010 Jason Perkins and the Premake project
--

	T.baking = { }
	local suite = T.baking

--
-- Setup code
--

	local prj, cfg
	function suite.setup()
		_ACTION = "gmake"
		
		solution "MySolution"
		configurations { "Debug", "Release" }
		platforms { "x32", "ps3" }
		
		defines "SOLUTION"
		
		configuration "Debug"
		defines "SOLUTION_DEBUG"
		
		prj = project "MyProject"
		language "C"
		kind "SharedLib"
		targetdir "../bin"
		defines "PROJECT"
		
		configuration "Debug"
		defines "DEBUG"
		  
		configuration "Release"
		defines "RELEASE"

		configuration "native"
		defines "NATIVE"
		
		configuration "x32"
		defines "X86_32"
		
		configuration "x64"
		defines "X86_64"
	end

	local function prepare()
		premake.bake.buildconfigs()
		prj = premake.getconfig(prj)
		cfg = premake.getconfig(prj, "Debug")
	end
	

--
-- Tests
--

	function suite.ProjectWideSettings()
		prepare()
		test.isequal("SOLUTION:PROJECT:NATIVE", table.concat(prj.defines,":"))
	end

	
	function suite.BuildCfgSettings()
		prepare()
		test.isequal("SOLUTION:SOLUTION_DEBUG:PROJECT:DEBUG:NATIVE", table.concat(cfg.defines,":"))
	end


	function suite.PlatformSettings()
		prepare()
		local cfg = premake.getconfig(prj, "Debug", "x32")
		test.isequal("SOLUTION:SOLUTION_DEBUG:PROJECT:DEBUG:X86_32", table.concat(cfg.defines,":"))
	end

			
	function suite.SetsConfigName()
		prepare()
		local cfg = premake.getconfig(prj, "Debug", "x32")
		test.isequal("Debug", cfg.name)
	end

	
	function suite.SetsPlatformName()
		prepare()
		local cfg = premake.getconfig(prj, "Debug", "x32")
		test.isequal("x32", cfg.platform)
	end

	
	function suite.SetsPlatformNativeName()
		test.isequal("Native", cfg.platform)
	end

	
	function suite.SetsShortName()
		prepare()
		local cfg = premake.getconfig(prj, "Debug", "x32")
		test.isequal("debug32", cfg.shortname)
	end

	
	function suite.SetsNativeShortName()
		prepare()
		test.isequal("debug", cfg.shortname)
	end

	
	function suite.SetsLongName()
		prepare()
		local cfg = premake.getconfig(prj, "Debug", "x32")
		test.isequal("Debug|x32", cfg.longname)
	end

	
	function suite.SetsNativeLongName()
		prepare()
		test.isequal("Debug", cfg.longname)
	end

	
	function suite.SetsProject()
		prepare()
		local cfg = premake.getconfig(prj, "Debug", "x32")
		test.istrue(prj.project == cfg.project)
	end



--
-- Target system testing
--

	function suite.SetsTargetSystem_OnNative()
		prepare()
		test.isequal(os.get(), cfg.system)
	end

	function suite.SetTargetSystem_OnCrossCompiler()
		prepare()
		local cfg = premake.getconfig(prj, "Debug", "PS3")
		test.isequal("PS3", cfg.system)
	end


	
--
-- Configuration-specific kinds
--

	function suite.SetsConfigSpecificKind()
		configuration "Debug"
		kind "ConsoleApp"
		prepare()
		test.isequal("ConsoleApp", cfg.kind)
	end


--
-- Platform kind translation
--

	function suite.SetsTargetKind_OnSupportedKind()
		prepare()
		test.isequal("SharedLib", cfg.kind)
	end

	function suite.SetsTargetKind_OnUnsupportedKind()
		prepare()
		local cfg = premake.getconfig(prj, "Debug", "PS3")
		test.isequal("StaticLib", cfg.kind)
	end
