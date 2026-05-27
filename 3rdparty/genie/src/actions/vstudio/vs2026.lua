--
-- vs2026.lua
-- Baseline support for Visual Studio 2026.
--

	premake.vstudio.vc2026 = {}
	local vstudio = premake.vstudio


---
-- Register a command-line action for Visual Studio 2026.
---

	newaction
	{
		trigger         = "vs2026",
		shortname       = "Visual Studio 2026",
		description     = "Generate Microsoft Visual Studio 2026 project files",
		os              = "windows",

		valid_kinds     = { "ConsoleApp", "WindowedApp", "StaticLib", "SharedLib", "Bundle" },

		valid_languages = { "C", "C++", "C#" },

		valid_tools     = {
			cc     = { "msc"   },
			dotnet = { "msnet" },
		},

		onsolution = function(sln)
			premake.generate(sln, "%%.slnx", vstudio.slnx.generate)
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
					premake.generate(prj, "%%/Package.appxmanifest", premake.vs2010_appxmanifest)
				end
			end
		end,


		oncleansolution = premake.vstudio.cleansolution,
		oncleanproject  = premake.vstudio.cleanproject,
		oncleantarget   = premake.vstudio.cleantarget,

		vstudio = {
			solutionVersion = "slnx",
			targetFramework = "4.7.2",
			toolsVersion    = "16.0",
			windowsTargetPlatformVersion = "10.0",
			supports64bitEditContinue    = true,
			intDirAbsolute  = false,
		}
	}
