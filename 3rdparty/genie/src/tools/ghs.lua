--
-- ghs.lua
-- Provides Green Hills Software-specific configuration strings.
--

	premake.ghs = { }
	premake.ghs.namestyle = "PS3"


--
-- Set default tools
--

	premake.ghs.cc     = "ccppc"
	premake.ghs.cxx    = "cxppc"
	premake.ghs.ar     = "cxppc"
	

--
-- Translation of Premake flags into GHS flags
--

	local cflags =
	{
		FatalWarnings  = "--quit_after_warnings",
		Optimize       = "-Ogeneral",
		OptimizeSize   = "-Osize",
		OptimizeSpeed  = "-Ospeed",
		Symbols        = "-g",
	}

	local cxxflags =
	{
		NoExceptions   = "--no_exceptions",
		NoRTTI         = "--no_rtti",
		UnsignedChar   = "--unsigned_chars",
	}


--
-- Map platforms to flags
--

	premake.ghs.platforms =
	{
		Native = {
			cppflags = "-MMD",
		},
		PowerPC = {
			cc         = "ccppc",
			cxx	       = "cxppc",
			ar	       = "cxppc",
			cppflags   = "-MMD",
			arflags    = "-archive -o",
		},
		ARM = {
			cc         = "ccarm",
			cxx        = "cxarm",
			ar         = "cxarm",
			cppflags   = "-MMD",
			arflags    = "-archive -o",
		}
	}

	local platforms = premake.ghs.platforms


--
-- Returns a list of compiler flags, based on the supplied configuration.
--

	function premake.ghs.getcppflags(cfg)
		local flags = { }
		table.insert(flags, platforms[cfg.platform].cppflags)
		return flags
	end


	function premake.ghs.getcflags(cfg)
		local result = table.translate(cfg.flags, cflags)
		table.insert(result, platforms[cfg.platform].flags)
		return result
	end


	function premake.ghs.getcxxflags(cfg)
		local result = table.translate(cfg.flags, cxxflags)
		return result
	end


--
-- Returns a list of linker flags, based on the supplied configuration.
--

	function premake.ghs.getldflags(cfg)
		local result = { }
		
		local platform = platforms[cfg.platform]
		table.insert(result, platform.flags)
		table.insert(result, platform.ldflags)

		return result
	end


--
-- Return a list of library search paths. Technically part of LDFLAGS but need to
-- be separated because of the way Visual Studio calls GCC for the PS3. See bug
-- #1729227 for background on why library paths must be split.
--

	function premake.ghs.getlibdirflags(cfg)
		local result = { }
		for _, value in ipairs(premake.getlinks(cfg, "all", "directory")) do
			table.insert(result, '-L' .. _MAKE.esc(value))
		end
		return result
	end

-- Returns a list of project-relative paths to external library files.
-- This function should examine the linker flags and return any that seem to be
-- a real path to a library file (e.g. "path/to/a/library.a", but not "GL").
-- Useful for adding to targets to trigger a relink when an external static
-- library gets updated.
-- Not currently supported on this toolchain.
--
	function premake.ghs.getlibfiles(cfg)
		local result = {}
		return result
	end

--
-- This is poorly named: returns a list of linker flags for external
-- (i.e. system, or non-sibling) libraries. See bug #1729227 for
-- background on why the path must be split.
--

	function premake.ghs.getlinkflags(cfg)
		local result = {}
		for _, value in ipairs(premake.getlinks(cfg, "system", "name")) do
			table.insert(result, '-lnk=' .. _MAKE.esc(value))
		end
		return result
	end
	
--
-- Get flags for passing to AR before the target is appended to the commandline
--  prj: project
--  cfg: configuration
--  ndx: true if the final step of a split archive
--

	function premake.ghs.getarchiveflags(prj, cfg, ndx)
		if prj.options.ArchiveSplit then
			error("ghs tool does not support split archives")
		end

		local result = {}
		local platform = platforms[cfg.platform]
		table.insert(result, platform.arflags)
		return result
	end



--
-- Decorate defines for the GHS command line.
--

	function premake.ghs.getdefines(defines)
		local result = { }
		for _,def in ipairs(defines) do
			table.insert(result, '-D' .. def)
		end
		return result
	end



--
-- Decorate include file search paths for the GCC command line.
--

	function premake.ghs.getincludedirs(includedirs)
		local result = { }
		for _,dir in ipairs(includedirs) do
			table.insert(result, "-I" .. _MAKE.esc(dir))
		end
		return result
	end


--
-- Return platform specific project and configuration level
-- makesettings blocks.
--

	function premake.ghs.getcfgsettings(cfg)
		return platforms[cfg.platform].cfgsettings
	end
