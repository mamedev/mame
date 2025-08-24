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
		-- If any of the specific runtime flags are set
		if cfg.flags.DebugRuntime then
			return true
		end

		if cfg.flags.ReleaseRuntime then
			return false
		end

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
-- Return an iterator over each file included in this configuration.
--

	function premake.config.eachfile(cfg)
		local i = 0
		local t = cfg.files
		return function ()
			i = i + 1
			if (i <= #t) then
				local fcfg = cfg.__fileconfigs[t[i]]
				fcfg.vpath = premake.project.getvpath(cfg.project, fcfg.name)
				return fcfg
			end
		end
	end


--
-- Determines if this configuration can be linked incrementally.
--

	function premake.config.isincrementallink(cfg)
		if cfg.kind == "StaticLib" then
			return false
		end
		return not config.islinkeroptimizedbuild(cfg.flags) and not cfg.flags.NoIncrementalLink
	end


--
-- Determine if this configuration uses one of the optimize flags.
--

	function premake.config.isoptimizedbuild(flags)
		return flags.Optimize or flags.OptimizeSize or flags.OptimizeSpeed
	end


--
-- Determine if this configuration uses one of the optimize flags.
-- Optimized builds get different treatment, such as full linking
-- instead of incremental.
--

	function premake.config.islinkeroptimizedbuild(flags)
		return config.isoptimizedbuild(flags) and not flags.NoOptimizeLink
	end


--
-- Determines if this configuration uses edit and continue.
--

	function premake.config.iseditandcontinue(cfg)
		if cfg.flags.NoEditAndContinue
				or cfg.flags.Managed
				or (cfg.kind ~= "StaticLib" and not config.isincrementallink(cfg))
				or config.islinkeroptimizedbuild(cfg.flags) then
			return false
		end
		return true
	end

