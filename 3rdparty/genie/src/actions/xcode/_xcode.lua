--
-- _xcode.lua
-- Define the Apple XCode action and support functions.
-- Copyright (c) 2009 Jason Perkins and the Premake project
--

	premake.xcode = { }

--
-- Set default toolset
--

	premake.xcode.toolset = "macosx"
	
	newaction 
	{
		trigger         = "xcode3",
		shortname       = "Xcode 3",
		description     = "Generate Apple Xcode 3 project files (experimental)",
		os              = "macosx",

		valid_kinds     = { "ConsoleApp", "WindowedApp", "SharedLib", "StaticLib" },
		
		valid_languages = { "C", "C++" },
		
		valid_tools     = {
			cc     = { "gcc" },
		},

		valid_platforms = { 
			Native = "Native", 
			x32 = "Native 32-bit", 
			x64 = "Native 64-bit", 
			Universal32 = "32-bit Universal", 
			Universal64 = "64-bit Universal", 
			Universal = "Universal",
		},
		
		default_platform = "Universal",
		
		onsolution = function(sln)
			-- Assign IDs needed for inter-project dependencies
			premake.xcode.preparesolution(sln)
		end,
		
		onproject = function(prj)
			premake.generate(prj, "%%.xcodeproj/project.pbxproj", premake.xcode.project)
		end,
		
		oncleanproject = function(prj)
			premake.clean.directory(prj, "%%.xcodeproj")
		end,
		
		oncheckproject = function(prj)
			-- Xcode can't mix target kinds within a project
			local last
			for cfg in premake.eachconfig(prj) do
				if last and last ~= cfg.kind then
					error("Project '" .. prj.name .. "' uses more than one target kind; not supported by Xcode", 0)
				end
				last = cfg.kind
			end
		end,
	}

	newaction 
	{
		trigger         = "xcode4",
		shortname       = "Xcode 4",
		description     = "Generate Apple Xcode 4 project files (experimental)",
		os              = "macosx",

		valid_kinds     = { "ConsoleApp", "WindowedApp", "SharedLib", "StaticLib" },
		
		valid_languages = { "C", "C++" },
		
		valid_tools     = {
			cc     = { "gcc" },
		},

		valid_platforms = { 
			Native = "Native", 
			x32 = "Native 32-bit", 
			x64 = "Native 64-bit", 
			Universal32 = "32-bit Universal", 
			Universal64 = "64-bit Universal", 
			Universal = "Universal",
		},
		
		default_platform = "Universal",
		
		onsolution = function(sln)
			premake.generate(sln, "%%.xcworkspace/contents.xcworkspacedata", premake.xcode4.workspace_generate)
		end,
		
		onproject = function(prj)
			premake.generate(prj, "%%.xcodeproj/project.pbxproj", premake.xcode.project)
		end,
		
		oncleanproject = function(prj)
			premake.clean.directory(prj, "%%.xcodeproj")
			premake.clean.directory(prj, "%%.xcworkspace")
		end,
		
		oncheckproject = function(prj)
			-- Xcode can't mix target kinds within a project
			local last
			for cfg in premake.eachconfig(prj) do
				if last and last ~= cfg.kind then
					error("Project '" .. prj.name .. "' uses more than one target kind; not supported by Xcode", 0)
				end
				last = cfg.kind
			end
		end,
	}


