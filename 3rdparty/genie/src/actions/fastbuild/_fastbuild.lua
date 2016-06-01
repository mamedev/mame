	premake.fastbuild = { }
	local fastbuild = premake.fastbuild

	newaction
	{
		trigger = "vs2015-fastbuild",
		shortname = "FASTBuild VS2015",
		description = "Generate FASTBuild configuration files for Visual Studio 2015.",

		valid_kinds = {
			"ConsoleApp",
			"WindowedApp",
			"SharedLib",
			"StaticLib"
		},

		valid_languages = {
			"C",
			"C++"
		},

		valid_tools = {
			cc = {
				"msc"
			},
		},

		onsolution = function(sln)
			premake.generate(sln, "fbuild.bff", premake.fastbuild.solution)
		end,

		onproject = function(prj)
			premake.generate(prj, "%%.bff", premake.fastbuild.project)
		end,

		oncleansolution = function(sln)
			premake.clean.file(sln, "fbuild.bff")
		end,

		oncleanproject  = function(prj)
			premake.clean.file(prj, "%%.bff")
		end,
	}
