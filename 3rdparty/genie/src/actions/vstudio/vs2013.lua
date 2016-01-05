--
-- vs2013.lua
-- Baseline support for Visual Studio 2013.
-- Copyright (c) 2013 Jason Perkins and the Premake project
--

	premake.vstudio.vc2013 = {}
	local vc2013 = premake.vstudio.vc2013
	local vstudio = premake.vstudio


---
-- Register a command-line action for Visual Studio 2013.
---

	newaction
	{
		trigger         = "vs2013",
		shortname       = "Visual Studio 2013",
		description     = "Generate Microsoft Visual Studio 2013 project files",
		os              = "windows",

		valid_kinds     = { "ConsoleApp", "WindowedApp", "StaticLib", "SharedLib" },

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
				premake.vstudio.needAppxManifest = false
				premake.generate(prj, "%%.vcxproj", premake.vs2010_vcxproj)
				premake.generate(prj, "%%.vcxproj.user", premake.vs2010_vcxproj_user)
				premake.generate(prj, "%%.vcxproj.filters", vstudio.vc2010.generate_filters)

				if premake.vstudio.needAppxManifest then
					premake.generate(prj, "%%.appxmanifest", premake.vs2010_appxmanifest)
				end
			end
		end,


		oncleansolution = premake.vstudio.cleansolution,
		oncleanproject  = premake.vstudio.cleanproject,
		oncleantarget   = premake.vstudio.cleantarget,

		vstudio = {
			solutionVersion = "12",
			targetFramework = "4.5",
			toolsVersion    = "12.0",
			supports64bitEditContinue = false,
		}
	}
