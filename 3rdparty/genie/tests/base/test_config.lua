--
-- tests/test_config.lua
-- Automated test suite for the configuration handling functions.
-- Copyright (c) 2010 Jason Perkins and the Premake project
--

	T.config = { }
	local suite = T.config


--
-- Setup/Teardown
--

	function suite.setup()
		sln = test.createsolution()
	end

	local cfg
	local function prepare()
		premake.bake.buildconfigs()
		cfg = premake.solution.getproject(sln, 1)
	end


--
-- Debug/Release build testing
--
	function suite.IsDebug_ReturnsFalse_EnglishSpellingOfOptimiseFlag()
		flags { "Optimise" }
		prepare()
		return test.isfalse(premake.config.isdebugbuild(cfg))
	end
	
	function suite.IsDebug_ReturnsFalse_EnglishSpellingOfOptimiseSizeFlag()
		flags { "OptimiseSize" }
		prepare()
		return test.isfalse(premake.config.isdebugbuild(cfg))
	end

	function suite.IsDebug_ReturnsFalse_EnglishSpellingOfOptimiseSpeedFlag()
		flags { "OptimiseSpeed" }
		prepare()
		return test.isfalse(premake.config.isdebugbuild(cfg))
	end
	
	function suite.IsDebug_ReturnsFalse_OnOptimizeFlag()
		flags { "Optimize" }
		prepare()
		return test.isfalse(premake.config.isdebugbuild(cfg))
	end

	function suite.IsDebug_ReturnsFalse_OnOptimizeSizeFlag()
		flags { "OptimizeSize" }
		prepare()
		return test.isfalse(premake.config.isdebugbuild(cfg))
	end

	function suite.IsDebug_ReturnsFalse_OnOptimizeSpeedFlag()
		flags { "OptimizeSpeed" }
		prepare()
		return test.isfalse(premake.config.isdebugbuild(cfg))
	end

	function suite.IsDebug_ReturnsFalse_OnNoSymbolsFlag()
		prepare()
		return test.isfalse(premake.config.isdebugbuild(cfg))
	end

	function suite.IsDebug_ReturnsTrue_OnSymbolsFlag()
		flags { "Symbols" }
		prepare()
		return test.istrue(premake.config.isdebugbuild(cfg))
	end

	function suite.shouldIncrementallyLink_staticLib_returnsFalse()
		kind "StaticLib"
		prepare()
		return test.isfalse(premake.config.isincrementallink(cfg))
	end
	
	function suite.shouldIncrementallyLink_optimizeFlagSet_returnsFalse()
		flags { "Optimize" }
		prepare()
		return test.isfalse(premake.config.isincrementallink(cfg))
	end
	
	function suite.shouldIncrementallyLink_NoIncrementalLinkFlag_returnsFalse()
		flags { "NoIncrementalLink" }
		prepare()
		return test.isfalse(premake.config.isincrementallink(cfg))
	end
	
	function suite.shouldIncrementallyLink_notStaticLib_NoIncrementalLinkFlag_noOptimiseFlag_returnsTrue()
		prepare()
		return test.istrue(premake.config.isincrementallink(cfg))
	end
	
