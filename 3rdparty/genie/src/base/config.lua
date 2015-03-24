--
-- configs.lua
--
-- Functions for working with configuration objects (which can include
-- projects and solutions).
--
-- Copyright (c) 2008-2011 Jason Perkins and the Premake project
--

	premake.config = { }
	local config = premake.config


-- 
-- Determine if a configuration represents a "debug" or "release" build.
-- This controls the runtime library selected for Visual Studio builds
-- (and might also be useful elsewhere).
--

	function premake.config.isdebugbuild(cfg)
		-- If any of the optimize flags are set, it's a release a build
		if cfg.flags.Optimize or cfg.flags.OptimizeSize or cfg.flags.OptimizeSpeed then
			return false
		end
		-- If symbols are not defined, it's a release build
		if not cfg.flags.Symbols then
			return false
		end
		return true
	end


--
-- Determines if this configuration can be linked incrementally.
-- 
	
	function premake.config.isincrementallink(cfg)
		if cfg.kind == "StaticLib" 
				or config.isoptimizedbuild(cfg.flags)
				or cfg.flags.NoIncrementalLink then
			return false
		end
		return true
	end


--
-- Determine if this configuration uses one of the optimize flags. 
-- Optimized builds get different treatment, such as full linking 
-- instead of incremental.
--
	
	function premake.config.isoptimizedbuild(flags)
		return flags.Optimize or flags.OptimizeSize or flags.OptimizeSpeed
	end

