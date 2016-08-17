--
-- valac.lua
-- Provides valac-specific configuration strings.
--


	premake.valac = { }


--
-- Set default tools
--

	premake.valac.valac  = "valac"
	premake.valac.cc     = "gcc"


--
-- Translation of Premake flags into GCC flags
--

	local valaflags =
	{
		DisableAssert             = "--disable-assert",               -- Disable assertions
		DisableSinceCheck         = "--disable-since-check",          -- Do not check whether used symbols exist in local packages
		DisableWarnings           = "--disable-warnings",             -- Disable warnings
		EnableChecking            = "--enable-checking",              -- Enable additional run-time checks
		EnableDeprecated          = "--enable-deprecated",            -- Enable deprecated features
		EnableExperimental        = "--enable-experimental",          -- Enable experimental features
		EnableExperimentalNonNull = "--enable-experimental-non-null", -- Enable experimental enhancements for non-null types
		EnableGObjectTracing      = "--enable-gobject-tracing",       -- Enable GObject creation tracing
		EnableMemProfiler         = "--enable-mem-profiler",          -- Enable GLib memory profiler
		EnableThreading           = "--thread",                       -- Enable multithreading support
		FatalWarnings             = "--fatal-warnings",               -- Treat warnings as fatal
		HideInternal              = "--hide-internal",                -- Hide symbols marked as internal
		NoStdPkg                  = "--nostdpkg",                     -- Do not include standard packages
		Optimize                  = "-X -O2",
		OptimizeSize              = "-X -Os",
		OptimizeSpeed             = "-X -O3",
		Symbols                   = "-g",                             -- Produce debug information
	}

	premake.valac.platforms = {}

--
-- Returns a list of compiler flags, based on the supplied configuration.
--

	function premake.valac.getvalaflags(cfg)
		return table.translate(cfg.flags, valaflags)
	end



--
-- Decorate pkgs for the Vala command line.
--

	function premake.valac.getlinks(links)
		local result = { }
		for _, pkg in ipairs(links) do
			table.insert(result, '--pkg ' .. pkg)
		end
		return result
	end



--
-- Decorate defines for the Vala command line.
--

	function premake.valac.getdefines(defines)
		local result = { }
		for _, def in ipairs(defines) do
			table.insert(result, '-D ' .. def)
		end
		return result
	end



--
-- Decorate C flags for the Vala command line.
--

	function premake.valac.getbuildoptions(buildoptions)
		local result = { }
		for _, def in ipairs(buildoptions) do
			table.insert(result, '-X ' .. def)
		end
		return result
	end
