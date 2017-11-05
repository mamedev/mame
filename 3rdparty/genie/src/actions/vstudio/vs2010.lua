--
-- vs2010.lua
-- Baseline support for Visual Studio 2010.
-- Copyright (c) 2013 Jason Perkins and the Premake project
--

	local vc2010 = premake.vstudio.vc2010
	local vstudio = premake.vstudio

---
-- Register a command-line action for Visual Studio 2010.
---

	newaction
	{
		trigger         = "vs2010",
		shortname       = "Visual Studio 2010",
		description     = "Generate Microsoft Visual Studio 2010 project files",
		os              = "windows",

		valid_kinds     = { "ConsoleApp", "WindowedApp", "StaticLib", "SharedLib", "Bundle" },

		valid_languages = { "C", "C++", "C#"},

		valid_tools     = {
			cc     = { "msc"   },
			dotnet = { "msnet" },
		},

		onsolution = function(sln)
			premake.generate(sln, "%%.sln", vstudio.sln2005.generate)
		end,

		onproject = function(prj)
			if premake.isdotnetproject(prj) then
				premake.generate(prj, "%%.csproj", vstudio.cs2005.generate)
				premake.generate(prj, "%%.csproj.user", vstudio.cs2005.generate_user)
			else
			premake.generate(prj, "%%.vcxproj", premake.vs2010_vcxproj)
			premake.generate(prj, "%%.vcxproj.user", premake.vs2010_vcxproj_user)
			premake.generate(prj, "%%.vcxproj.filters", vstudio.vc2010.generate_filters)
			end
		end,

		oncleansolution = premake.vstudio.cleansolution,
		oncleanproject  = premake.vstudio.cleanproject,
		oncleantarget   = premake.vstudio.cleantarget,

		vstudio = {
			productVersion  = "8.0.30703",
			solutionVersion = "11",
			targetFramework = "4.0",
			toolsVersion    = "4.0",
			supports64bitEditContinue = false,
			intDirAbsolute  = false,
		}
	}
