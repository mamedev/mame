--
-- xcode14.lua
-- Define the Apple XCode 14.0 action and support functions.
--

	local premake = premake
	premake.xcode14 = { }

	local xcode  = premake.xcode
	local xcode10 = premake.xcode10
	local xcode11 = premake.xcode11
	local xcode14 = premake.xcode14

	function xcode14.XCBuildConfiguration_Target(tr, target, cfg)
		local options = xcode11.XCBuildConfiguration_Target(tr, target, cfg)
		options.CODE_SIGN_IDENTITY = "-"

		local action = premake.action.current()
		xcode.setdeploymenttarget(cfg, action.xcode, options)

		local iosversion = options.IPHONEOS_DEPLOYMENT_TARGET
		local macosversion = options.MACOSX_DEPLOYMENT_TARGET
		local tvosversion = options.TVOS_DEPLOYMENT_TARGET

		if iosversion and not xcode.versionge(iosversion, "11") then
		   error("XCode14 does not support deployment for iOS older than 11")
		elseif macosversion and not xcode.versionge(macosversion, "10.13") then
		   error("XCode14 does not support deployment for macOS older than 10.13")
		elseif tvosversion and not xcode.versionge(tvosversion, "11") then
		   error("XCode14 does not support deployment for tvOS older than 11")
		end

		return options
	end

	function xcode14.XCBuildConfiguration_Project(tr, prj, cfg)
		local options = xcode10.XCBuildConfiguration_Project(tr, prj, cfg)

		options.ENABLE_BITCODE = "NO" -- Bitcode is now deprecated.

		-- We need to set the deployment target for both target
		-- and project. XCode will complain, otherwise.
		local action = premake.action.current()
		xcode.setdeploymenttarget(cfg, action.xcode, options)

		return options
	end

	function xcode14.project(prj)
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
			ontarget = xcode14.XCBuildConfiguration_Target,
			onproject = xcode14.XCBuildConfiguration_Project,
		})
		xcode.XCBuildConfigurationList(tr)
		xcode.Footer(tr)
	end
	--]]


--
-- xcode14 action
--

	newaction
	{
		trigger         = "xcode14",
		shortname       = "Xcode 14",
		description     = "Generate Apple Xcode 14 project files",
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
			premake.generate(prj, "%%.xcodeproj/project.pbxproj", xcode14.project)
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
