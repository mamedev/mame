--
-- dotnet.lua
-- Interface for the C# compilers, all of which are flag compatible.
-- Copyright (c) 2002-2009 Jason Perkins and the Premake project
--

	
	premake.dotnet = { }
	premake.dotnet.namestyle = "windows"
	

--
-- Translation of Premake flags into CSC flags
--

	local flags =
	{
		FatalWarning   = "/warnaserror",
		Optimize       = "/optimize",
		OptimizeSize   = "/optimize",
		OptimizeSpeed  = "/optimize",
		Symbols        = "/debug",
		Unsafe         = "/unsafe"
	}


--
-- Return the default build action for a given file, based on the file extension.
--

	function premake.dotnet.getbuildaction(fcfg)
		local ext = path.getextension(fcfg.name):lower()
		if fcfg.buildaction == "Compile" or ext == ".cs" then
			return "Compile"
		elseif fcfg.buildaction == "Embed" or ext == ".resx" then
			return "EmbeddedResource"
		elseif fcfg.buildaction == "Copy" or ext == ".asax" or ext == ".aspx" then
			return "Content"
		else
			return "None"
		end
	end
	
	
	
--
-- Returns the compiler filename (they all use the same arguments)
--

	function premake.dotnet.getcompilervar(cfg)
		if (_OPTIONS.dotnet == "msnet") then
			return "csc"
		elseif (_OPTIONS.dotnet == "mono") then
			if (cfg.framework <= "1.1") then
				return "mcs"
			elseif (cfg.framework >= "4.0") then
				return "dmcs"
			else 
				return "gmcs"
			end
		else
			return "cscc"
		end
	end



--
-- Returns a list of compiler flags, based on the supplied configuration.
--

	function premake.dotnet.getflags(cfg)
		local result = table.translate(cfg.flags, flags)
		return result		
	end



--
-- Translates the Premake kind into the CSC kind string.
--

	function premake.dotnet.getkind(cfg)
		if (cfg.kind == "ConsoleApp") then
			return "Exe"
		elseif (cfg.kind == "WindowedApp") then
			return "WinExe"
		elseif (cfg.kind == "SharedLib") then
			return "Library"
		end
	end