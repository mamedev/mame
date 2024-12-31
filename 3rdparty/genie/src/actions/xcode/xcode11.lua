--
-- xcode11.lua
-- Define the Apple XCode 11.0 action and support functions.
--

	local premake = premake
	premake.xcode11 = { }

	local xcode  = premake.xcode
	local xcode10 = premake.xcode10
	local xcode11 = premake.xcode11

	function xcode11.XCBuildConfiguration_Target(tr, target, cfg)
		local options = xcode10.XCBuildConfiguration_Target(tr, target, cfg)
		options.CODE_SIGN_IDENTITY = "-"
		return options
	end

	function xcode11.project(prj)
		local tr = xcode.buildprjtree(prj)
		xcode.Header(tr, 48)
		xcode.PBXBuildFile(tr)
		xcode.PBXContainerItemProxy(tr)
		xcode.PBXFileReference(tr,prj)
		xcode.PBXFrameworksBuildPhase(tr)
		xcode.PBXGroup(tr)
		xcode.PBXNativeTarget(tr)
		xcode.PBXProject(tr, "8.0")
		xcode.PBXReferenceProxy(tr)
		xcode.PBXResourcesBuildPhase(tr)
		xcode.PBXShellScriptBuildPhase(tr)
		xcode.PBXCopyFilesBuildPhase(tr)
		xcode.PBXSourcesBuildPhase(tr,prj)
		xcode.PBXVariantGroup(tr)
		xcode.PBXTargetDependency(tr)
		xcode.XCBuildConfiguration(tr, prj, {
			ontarget = xcode11.XCBuildConfiguration_Target,
			onproject = xcode10.XCBuildConfiguration_Project,
		})
		xcode.XCBuildConfigurationList(tr)
		xcode.Footer(tr)
	end
	--]]


--
-- xcode11 action
--

	newaction
	{
		trigger         = "xcode11",
		shortname       = "Xcode 11",
		description     = "Generate Apple Xcode 11 project files",
		os              = "macosx",

		valid_kinds     = { "ConsoleApp", "WindowedApp", "StaticLib", "SharedLib", "Bundle" },

		valid_languages = { "C", "C++" },

		valid_tools     = {
			cc     = { "gcc" },
		},

		valid_platforms = { Native = "Native" },
		default_platform = "Native",

		onsolution = function(sln)
			premake.generate(sln, "%%.xcworkspace/contents.xcworkspacedata", xcode.workspace_generate)
			premake.generate(sln, "%%.xcworkspace/xcshareddata/WorkspaceSettings.xcsettings", xcode.workspace_settings)
			premake.generate(sln, "%%.xcworkspace/xcshareddata/xcschemes/-ALL-.xcscheme", xcode.workspace_scheme)
		end,

		onproject = function(prj)
			premake.generate(prj, "%%.xcodeproj/project.pbxproj", xcode11.project)
			xcode.generate_schemes(prj, "%%.xcodeproj/xcshareddata/xcschemes")
		end,

		oncleanproject = function(prj)
			premake.clean.directory(prj, "%%.xcodeproj")
			premake.clean.directory(prj, "%%.xcworkspace")
		end,

		oncheckproject = xcode.checkproject,

		xcode = {
			iOSTargetPlatformVersion = nil,
			macOSTargetPlatformVersion = nil,
			tvOSTargetPlatformVersion = nil,
		},
	}
