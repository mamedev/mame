--
-- xcode15.lua
-- Define the Apple XCode 15.0 action and support functions.
--

	local premake = premake
	premake.xcode15 = { }

	local xcode  = premake.xcode
	local xcode10 = premake.xcode10
	local xcode11 = premake.xcode11
	local xcode14 = premake.xcode14
	local xcode15 = premake.xcode15

	function xcode15.XCBuildConfiguration_Target(tr, target, cfg)
		local options = xcode11.XCBuildConfiguration_Target(tr, target, cfg)
		options.CODE_SIGN_IDENTITY = "-"

		local action = premake.action.current()
		xcode.setdeploymenttarget(cfg, action.xcode, options)

		local iosversion = options.IPHONEOS_DEPLOYMENT_TARGET
		local macosversion = options.MACOSX_DEPLOYMENT_TARGET
		local tvosversion = options.TVOS_DEPLOYMENT_TARGET
		local xrosversion = options.XROS_DEPLOYMENT_TARGET

		if iosversion and not xcode.versionge(iosversion, "12") then
		   error("XCode15 does not support deployment for iOS older than 12")
		elseif macosversion and not xcode.versionge(macosversion, "13.5") then
		   error("XCode15 does not support deployment for macOS older than 13.5")
		elseif tvosversion and not xcode.versionge(tvosversion, "12") then
		   error("XCode15 does not support deployment for tvOS older than 12")
		elseif xrosversion and not xcode.versionge(xrosversion, "1.0") then
		   error("XCode15 does not support deployment for visionOS older than 1.0")
		end

		return options
	end

	function xcode15.XCBuildConfiguration_Project(tr, prj, cfg)
		local options = xcode10.XCBuildConfiguration_Project(tr, prj, cfg)

		-- We need to set the deployment target for both target
		-- and project. XCode will complain, otherwise.
		local action = premake.action.current()
		xcode.setdeploymenttarget(cfg, action.xcode, options)

		return options
	end

	function xcode15.project(prj)
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
			ontarget = xcode15.XCBuildConfiguration_Target,
			onproject = xcode15.XCBuildConfiguration_Project,
		})
		xcode.XCBuildConfigurationList(tr)
		xcode.Footer(tr)
	end
	--]]


--
-- xcode15 action
--

	newaction
	{
		trigger         = "xcode15",
		shortname       = "Xcode 15",
		description     = "Generate Apple Xcode 15 project files",
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
			visionOSTargetPlatformVersion = nil,
		},
	}
