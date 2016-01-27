--
-- gcc.lua
-- Provides GCC-specific configuration strings.
-- Copyright (c) 2002-2011 Jason Perkins and the Premake project
--


	premake.gcc = { }


--
-- Set default tools
--

	premake.gcc.cc     = "gcc"
	premake.gcc.cxx    = "g++"
	premake.gcc.ar     = "ar"
	premake.gcc.llvm   = false


--
-- Translation of Premake flags into GCC flags
--

	local cflags =
	{
		EnableSSE      = "-msse",
		EnableSSE2     = "-msse2",
		ExtraWarnings  = "-Wall -Wextra",
		FatalWarnings  = "-Werror",
		FloatFast      = "-ffast-math",
		FloatStrict    = "-ffloat-store",
		NoFramePointer = "-fomit-frame-pointer",
		Optimize       = "-O2",
		OptimizeSize   = "-Os",
		OptimizeSpeed  = "-O3",
		Symbols        = "-g",
	}

	local cxxflags =
	{
		NoExceptions   = "-fno-exceptions",
		NoRTTI         = "-fno-rtti",
		UnsignedChar   = "-funsigned-char",
	}


--
-- Map platforms to flags
--

	premake.gcc.platforms =
	{
		Native = {
			cppflags = "-MMD -MP",
		},
		x32 = {
			cppflags = "-MMD -MP",
			flags    = "-m32",
		},
		x64 = {
			cppflags = "-MMD -MP",
			flags    = "-m64",
		},
		Universal = {
			ar       = "libtool",
			cppflags = "-MMD -MP",
			flags    = "-arch i386 -arch x86_64 -arch ppc -arch ppc64",
		},
		Universal32 = {
			ar       = "libtool",
			cppflags = "-MMD -MP",
			flags    = "-arch i386 -arch ppc",
		},
		Universal64 = {
			ar       = "libtool",
			cppflags = "-MMD -MP",
			flags    = "-arch x86_64 -arch ppc64",
		},
		PS3 = {
			cc         = "ppu-lv2-g++",
			cxx        = "ppu-lv2-g++",
			ar         = "ppu-lv2-ar",
			cppflags   = "-MMD -MP",
		},
		WiiDev = {
			cppflags    = "-MMD -MP -I$(LIBOGC_INC) $(MACHDEP)",
			ldflags		= "-L$(LIBOGC_LIB) $(MACHDEP)",
			cfgsettings = [[
  ifeq ($(strip $(DEVKITPPC)),)
    $(error "DEVKITPPC environment variable is not set")'
  endif
  include $(DEVKITPPC)/wii_rules']],
		},
		Orbis = {
			cc         = "orbis-clang",
			cxx        = "orbis-clang++",
			ar         = "orbis-ar",
			cppflags   = "-MMD -MP",
		}
	}

	local platforms = premake.gcc.platforms


--
-- Returns a list of compiler flags, based on the supplied configuration.
--

	function premake.gcc.getcppflags(cfg)
		local flags = { }
		table.insert(flags, platforms[cfg.platform].cppflags)

		-- We want the -MP flag for dependency generation (creates phony rules
		-- for headers, prevents make errors if file is later deleted)
		if flags[1]:startswith("-MMD") then
			table.insert(flags, "-MP")
		end

		return flags
	end


	function premake.gcc.getcflags(cfg)
		local result = table.translate(cfg.flags, cflags)
		table.insert(result, platforms[cfg.platform].flags)
		if cfg.system ~= "windows" and cfg.kind == "SharedLib" then
			table.insert(result, "-fPIC")
		end
		return result
	end


	function premake.gcc.getcxxflags(cfg)
		local result = table.translate(cfg.flags, cxxflags)
		return result
	end


--
-- Returns a list of linker flags, based on the supplied configuration.
--

	function premake.gcc.getldflags(cfg)
		local result = { }

		-- OS X has a bug, see http://lists.apple.com/archives/Darwin-dev/2006/Sep/msg00084.html
		if not cfg.flags.Symbols then
			if cfg.system == "macosx" then
-- Issue#80
-- https://github.com/bkaradzic/genie/issues/80#issuecomment-100664007
--				table.insert(result, "-Wl,-x")
			else
				table.insert(result, "-s")
			end
		end

		if cfg.kind == "SharedLib" then
			if cfg.system == "macosx" then
				table.insert(result, "-dynamiclib")
			else
				table.insert(result, "-shared")
			end

			if cfg.system == "windows" and not cfg.flags.NoImportLib then
				table.insert(result, '-Wl,--out-implib="' .. cfg.linktarget.fullpath .. '"')
			end
		end

		if cfg.kind == "WindowedApp" and cfg.system == "windows" then
			table.insert(result, "-mwindows")
		end

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

	function premake.gcc.getlibdirflags(cfg)
		local result = { }
		for _, value in ipairs(premake.getlinks(cfg, "all", "directory")) do
			table.insert(result, '-L' .. _MAKE.esc(value))
		end
		return result
	end



--
-- This is poorly named: returns a list of linker flags for external
-- (i.e. system, or non-sibling) libraries. See bug #1729227 for
-- background on why the path must be split.
--

	function premake.gcc.getlinkflags(cfg)
		local result = {}
		for _, value in ipairs(premake.getlinks(cfg, "system", "name")) do
			if path.getextension(value) == ".framework" then
				table.insert(result, '-framework ' .. _MAKE.esc(path.getbasename(value)))
			else
				table.insert(result, '-l' .. _MAKE.esc(value))
			end
		end
		return result
	end



--
-- Get flags for passing to AR before the target is appended to the commandline
--  prj: project
--  cfg: configuration
--  ndx: true if the final step of a split archive
--

	function premake.gcc.getarchiveflags(prj, cfg, ndx)
		local result = {}
		if cfg.platform:startswith("Universal") then
				if prj.options.ArchiveSplit then
					error("gcc tool 'Universal*' platforms do not support split archives")
				end
				table.insert(result, '-o')
		else
			if (not prj.options.ArchiveSplit) then
				if premake.gcc.llvm then
					table.insert(result, 'rcs')
				else
					table.insert(result, '-rcs')
				end
			else
				if premake.gcc.llvm then
					if (not ndx) then
						table.insert(result, 'qc')
					else
						table.insert(result, 'cs')
					end
				else
					if (not ndx) then
						table.insert(result, '-qc')
					else
						table.insert(result, '-cs')
					end
				end
			end
		end
		return result
	end


--
-- Decorate defines for the GCC command line.
--

	function premake.gcc.getdefines(defines)
		local result = { }
		for _,def in ipairs(defines) do
			table.insert(result, '-D' .. def)
		end
		return result
	end



--
-- Decorate include file search paths for the GCC command line.
--

	function premake.gcc.getincludedirs(includedirs)
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

	function premake.gcc.getcfgsettings(cfg)
		return platforms[cfg.platform].cfgsettings
	end
