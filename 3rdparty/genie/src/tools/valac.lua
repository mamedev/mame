--
-- valac.lua
-- Provides valac-specific configuration strings.
--


	premake.valac = { }


--
-- Set default tools
--

	premake.valac.valac  = "valac"
	premake.valac.cc     = premake.gcc.cc
	premake.valac.glibrc = "glib-compile-resources"


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
		FatalWarnings             = "--fatal-warnings",               -- Treat warnings as fatal
		HideInternal              = "--hide-internal",                -- Hide symbols marked as internal
		NoStdPkg                  = "--nostdpkg",                     -- Do not include standard packages
		Symbols                   = "-g",                             -- Produce debug information
	}

	local valaccflags =
	{
		Optimize                  = "-O2",
		OptimizeSize              = "-Os",
		OptimizeSpeed             = "-O3",
		Symbols                   = "-g",                             -- Produce debug information
	}

--
-- Map platforms to flags
--

	premake.valac.platforms =
	{
		Native = {
		},
		x64 = {
			flags = "-m64"
		},
	}



--
-- Returns a list of compiler flags for `valac`, based on the supplied configuration.
--

	function premake.valac.getvalaflags(cfg)
		return table.translate(cfg.flags, valaflags)
	end



--
-- Returns a list of compiler flags for `cc`, based on the supplied configuration.
--

	function premake.valac.getvalaccflags(cfg)
		return table.translate(cfg.flags, valaccflags)
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
-- Decorate vapidirs for the Vala command line.
--

	function premake.valac.getvapidirs(vapidirs)
		local result = { }
		for _, def in ipairs(vapidirs) do
			table.insert(result, '--vapidir=' .. def)
		end
		return result
	end
