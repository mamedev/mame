--
-- vs15.lua
-- Baseline support for Visual Studio 15.
--

	local vstudio = premake.vstudio


---
-- Register a command-line action for Visual Studio 15.
---

	newaction
	{
		trigger         = "vs15",
		shortname       = "Visual Studio 15",
		description     = "Generate Microsoft Visual Studio 15 project files",
		os              = "windows",

		valid_kinds     = { "ConsoleApp", "WindowedApp", "StaticLib", "SharedLib" },

		valid_languages = { "C", "C++", "C#" },

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
				vstudio.needAppxManifest = false
				premake.generate(prj, "%%.vcxproj", premake.vs2010_vcxproj)
				premake.generate(prj, "%%.vcxproj.user", premake.vs2010_vcxproj_user)
				premake.generate(prj, "%%.vcxproj.filters", vstudio.vc2010.generate_filters)

				if vstudio.needAppxManifest then
					premake.generate(prj, "%%.appxmanifest", premake.vs2010_appxmanifest)
				end
			end
		end,


		oncleansolution = vstudio.cleansolution,
		oncleanproject  = vstudio.cleanproject,
		oncleantarget   = vstudio.cleantarget,

		vstudio = {
			solutionVersion = "12",
			targetFramework = "4.5.2",
			toolsVersion    = "15.0",
			windowsTargetPlatformVersion = "8.1",
			supports64bitEditContinue = true,
		}
	}
