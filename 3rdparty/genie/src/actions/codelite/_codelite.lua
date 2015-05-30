--
-- _codelite.lua
-- Define the CodeLite action(s).
-- Copyright (c) 2008-2009 Jason Perkins and the Premake project
--

	premake.codelite = { }

	newaction {
		trigger         = "codelite",
		shortname       = "CodeLite",
		description     = "Generate CodeLite project files",
	
		valid_kinds     = { "ConsoleApp", "WindowedApp", "StaticLib", "SharedLib" },
		
		valid_languages = { "C", "C++" },
		
		valid_tools     = {
			cc   = { "gcc" },
		},
		
		onsolution = function(sln)
			premake.generate(sln, "%%.workspace", premake.codelite.workspace)
		end,
		
		onproject = function(prj)
			premake.generate(prj, "%%.project", premake.codelite.project)
		end,
		
		oncleansolution = function(sln)
			premake.clean.file(sln, "%%.workspace")
			premake.clean.file(sln, "%%_wsp.mk")
			premake.clean.file(sln, "%%.tags")
		end,
		
		oncleanproject = function(prj)
			premake.clean.file(prj, "%%.project")
			premake.clean.file(prj, "%%.mk")
			premake.clean.file(prj, "%%.list")
			premake.clean.file(prj, "%%.out")
		end
	}
