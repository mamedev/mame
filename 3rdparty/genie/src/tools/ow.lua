--
-- ow.lua
-- Provides Open Watcom-specific configuration strings.
-- Copyright (c) 2008 Jason Perkins and the Premake project
--

	premake.ow = { }
	premake.ow.namestyle = "windows"
	
	
--
-- Set default tools
--

	premake.ow.cc     = "WCL386"
	premake.ow.cxx    = "WCL386"
	premake.ow.ar     = "ar"
	
	
--
-- Translation of Premake flags into OpenWatcom flags
--

	local cflags =
	{
		ExtraWarnings  = "-wx",
		FatalWarning   = "-we",
		FloatFast      = "-omn",
		FloatStrict    = "-op",
		Optimize       = "-ox",
		OptimizeSize   = "-os",
		OptimizeSpeed  = "-ot",
		Symbols        = "-d2",
	}

	local cxxflags =
	{
		NoExceptions   = "-xd",
		NoRTTI         = "-xr",
	}
	


--
-- No specific platform support yet
--

	premake.ow.platforms = 
	{
		Native = { 
			flags = "" 
		},
	}


	
--
-- Returns a list of compiler flags, based on the supplied configuration.
--

	function premake.ow.getcppflags(cfg)
		return {}
	end

	function premake.ow.getcflags(cfg)
		local result = table.translate(cfg.flags, cflags)		
		if (cfg.flags.Symbols) then
			table.insert(result, "-hw")   -- Watcom debug format for Watcom debugger
		end
		return result		
	end
	
	function premake.ow.getcxxflags(cfg)
		local result = table.translate(cfg.flags, cxxflags)
		return result
	end
	


--
-- Returns a list of linker flags, based on the supplied configuration.
--

	function premake.ow.getldflags(cfg)
		local result = { }
		
		if (cfg.flags.Symbols) then
			table.insert(result, "op symf")
		end
				
		return result
	end
		
	
--
-- Returns a list of linker flags for library search directories and 
-- library names.
--

	function premake.ow.getlinkflags(cfg)
		local result = { }
		return result
	end
	
	

--
-- Decorate defines for the command line.
--

	function premake.ow.getdefines(defines)
		local result = { }
		for _,def in ipairs(defines) do
			table.insert(result, '-D' .. def)
		end
		return result
	end


	
--
-- Decorate include file search paths for the command line.
--

	function premake.ow.getincludedirs(includedirs)
		local result = { }
		for _,dir in ipairs(includedirs) do
			table.insert(result, '-I "' .. dir .. '"')
		end
		return result
	end

