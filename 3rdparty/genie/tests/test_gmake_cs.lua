--
-- tests/test_gmake_cs.lua
-- Automated test suite for GNU Make C/C++ project generation.
-- Copyright (c) 2009 Jason Perkins and the Premake project
--

	T.gmake_cs = { }

--
-- Configure a solution for testing
--

	local sln, prj
	function T.gmake_cs.setup()
		_ACTION = "gmake"
		_OPTIONS.os = "linux"

		sln = solution "MySolution"
		configurations { "Debug", "Release" }
		platforms { "native" }
		
		prj = project "MyProject"
		language "C#"
		kind "ConsoleApp"		
	end

	local function prepare()
		premake.bake.buildconfigs()
	end
	


--
-- Test configuration blocks
--

	function T.gmake_cs.BasicCfgBlock()
		prepare()
		local cfg = premake.getconfig(prj, "Debug")
		premake.gmake_cs_config(cfg, premake.dotnet, {[cfg]={}})
		test.capture [[
ifneq (,$(findstring debug,$(config)))
  TARGETDIR  := .
  OBJDIR     := obj/Debug
  DEPENDS    := 
  REFERENCES := 
  FLAGS      +=  
  define PREBUILDCMDS
  endef
  define PRELINKCMDS
  endef
  define POSTBUILDCMDS
  endef
endif
		]]
	end


	function T.gmake_cs.OnBuildOptions()
		buildoptions { "/define:SYMBOL" }
		prepare()
		local cfg = premake.getconfig(prj, "Debug")
		premake.gmake_cs_config(cfg, premake.dotnet, {[cfg]={}})
		test.capture [[
ifneq (,$(findstring debug,$(config)))
  TARGETDIR  := .
  OBJDIR     := obj/Debug
  DEPENDS    := 
  REFERENCES := 
  FLAGS      +=  /define:SYMBOL
  define PREBUILDCMDS
  endef
  define PRELINKCMDS
  endef
  define POSTBUILDCMDS
  endef
endif
		]]
	end
