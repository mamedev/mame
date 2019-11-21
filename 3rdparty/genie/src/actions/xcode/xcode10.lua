--
-- xcode10.lua
-- Define the Apple XCode 10.0 action and support functions.
--

	local premake = premake
	premake.xcode10 = { }

	local xcode  = premake.xcode
	local xcode8 = premake.xcode8
	local xcode9 = premake.xcode9
	local xcode10 = premake.xcode10

	function xcode10.XCBuildConfiguration_Project(tr, prj, cfg)
		local options = xcode9.XCBuildConfiguration_Project(tr, prj, cfg)

		return table.merge(options, {
			CLANG_WARN_DEPRECATED_OBJC_IMPLEMENTATIONS = "YES",
			CLANG_WARN_OBJC_IMPLICIT_RETAIN_SELF = "YES",
			CLANG_WARN_BLOCK_CAPTURE_AUTORELEASING = "YES",
			CLANG_WARN_COMMA = "YES",
			CLANG_WARN_NON_LITERAL_NULL_CONVERSION = "YES",
			CLANG_WARN_OBJC_LITERAL_CONVERSION = "YES",
			CLANG_WARN_RANGE_LOOP_ANALYSIS = "YES",
			CLANG_WARN_STRICT_PROTOTYPES = "YES",
		})
	end

	function xcode10.XCBuildConfiguration_Target(tr, target, cfg)
		local options = xcode8.XCBuildConfiguration_Target(tr, target, cfg)

		if not cfg.flags.ObjcARC then
			options.CLANG_ENABLE_OBJC_WEAK = "YES"
		end

		return options
	end

	function xcode10.project(prj)
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
			ontarget = xcode10.XCBuildConfiguration_Target,
			onproject = xcode10.XCBuildConfiguration_Project,
		})
		xcode.XCBuildConfigurationList(tr)
		xcode.Footer(tr)
	end


--
-- xcode10 action
--

	newaction
	{
		trigger         = "xcode10",
		shortname       = "Xcode 10",
		description     = "Generate Apple Xcode 10 project files (experimental)",
		os              = "macosx",

		valid_kinds     = { "ConsoleApp", "WindowedApp", "StaticLib", "SharedLib", "Bundle" },

		valid_languages = { "C", "C++" },

		valid_tools     = {
			cc     = { "gcc" },
		},

		valid_platforms = {
			Native = "Native",
			x32 = "Native 32-bit",
			x64 = "Native 64-bit",
			Universal = "Universal",
		},

		default_platform = "Native",

		onsolution = function(sln)
			premake.generate(sln, "%%.xcworkspace/contents.xcworkspacedata", xcode.workspace_generate)
			premake.generate(sln, "%%.xcworkspace/xcshareddata/WorkspaceSettings.xcsettings", xcode.workspace_settings)
			premake.generate(sln, "%%.xcworkspace/xcshareddata/xcschemes/-ALL-.xcscheme", xcode.workspace_scheme)
		end,

		onproject = function(prj)
			premake.generate(prj, "%%.xcodeproj/project.pbxproj", xcode10.project)
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
