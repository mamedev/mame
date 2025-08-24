--
-- _xcode8.lua
-- Define the Apple XCode 8.0 action and support functions.
--

	local premake = premake
	premake.xcode8 = { }

	local xcode  = premake.xcode
	local xcode8 = premake.xcode8

	function xcode8.XCBuildConfiguration_Target(tr, target, cfg)
		local cfgname = xcode.getconfigname(cfg)
		local installpaths = {
			ConsoleApp = "/usr/local/bin",
			WindowedApp = "$(HOME)/Applications",
			SharedLib = "/usr/local/lib",
			StaticLib = "/usr/local/lib",
			Bundle    = "$(LOCAL_LIBRARY_DIR)/Bundles",
		}

		-- options table to return
		local options = {
			ALWAYS_SEARCH_USER_PATHS = "NO",
			GCC_DYNAMIC_NO_PIC = "NO",
			GCC_MODEL_TUNING = "G5",
			INSTALL_PATH = installpaths[cfg.kind],
			PRODUCT_NAME = cfg.buildtarget.basename,
		}

		if not cfg.flags.Symbols then
			options.DEBUG_INFORMATION_FORMAT = "dwarf-with-dsym"
		end

		if cfg.kind ~= "StaticLib" and cfg.buildtarget.prefix ~= "" then
			options.EXECUTABLE_PREFIX = cfg.buildtarget.prefix
		end

		if cfg.targetextension then
			local ext = cfg.targetextension
			options.EXECUTABLE_EXTENSION = iif(ext:startswith("."), ext:sub(2), ext)
		end

		if cfg.flags.ObjcARC then
			options.CLANG_ENABLE_OBJC_ARC = "YES"
		end

		local outdir = path.getdirectory(cfg.buildtarget.bundlepath)
		if outdir ~= "." then
			options.CONFIGURATION_BUILD_DIR = outdir
		end

		if tr.infoplist then
			options.INFOPLIST_FILE = tr.infoplist.cfg.name
		end

		local infoplist_file = nil

		for _, v in ipairs(cfg.files) do
			-- for any file named *info.plist, use it as the INFOPLIST_FILE
			if (string.find (string.lower (v), 'info.plist') ~= nil) then
				infoplist_file = string.format('$(SRCROOT)/%s', v)
			end
		end

		if infoplist_file ~= nil then
			options.INFOPLIST_FILE = infoplist_file
		end

		local action = premake.action.current()
		xcode.setdeploymenttarget(cfg, action.xcode, options)

		if cfg.kind == "Bundle" and not cfg.options.SkipBundling then
			options.PRODUCT_BUNDLE_IDENTIFIER = "genie." .. cfg.buildtarget.basename:gsub("%s+", ".") --replace spaces with .
			local ext = cfg.targetextension
			if ext then
				options.WRAPPER_EXTENSION = iif(ext:startswith("."), ext:sub(2), ext)
			else
				options.WRAPPER_EXTENSION = "bundle"
			end
		end

		return options
	end

	function xcode8.XCBuildConfiguration_Project(tr, prj, cfg)
		local cfgname = xcode.getconfigname(cfg)
		local archs = {
			Native      = nil,
			x32         = "i386",
			x64         = "x86_64",
			Universal32 = "$(ARCHS_STANDARD_32_BIT)",
			Universal64 = "$(ARCHS_STANDARD_64_BIT)",
			Universal   = "$(ARCHS_STANDARD_32_64_BIT)",
		}

		-- build list of "other" C/C++ flags
		local checks = {
			["-ffast-math"]          = cfg.flags.FloatFast,
			["-ffloat-store"]        = cfg.flags.FloatStrict,
			["-fomit-frame-pointer"] = cfg.flags.NoFramePointer,
		}

		local cflags = { }
		for flag, check in pairs(checks) do
			if check then
				table.insert(cflags, flag)
			end
		end

		-- build list of "other" linked flags. All libraries that aren't frameworks
		-- are listed here, so I don't have to try and figure out if they are ".a"
		-- or ".dylib", which Xcode requires to list in the Frameworks section
		local ldflags = { }
		for _, lib in ipairs(premake.getlinks(cfg, "system")) do
			if not xcode.isframework(lib) then
				table.insert(ldflags, "-l" .. lib)
			end
		end

		-- options table to return
		local options = {
			ARCHS                              = archs[cfg.platform],
			CLANG_WARN__DUPLICATE_METHOD_MATCH = "YES",
			CLANG_WARN_BOOL_CONVERSION         = "YES",
			CLANG_WARN_CONSTANT_CONVERSION     = "YES",
			CLANG_WARN_EMPTY_BODY              = "YES",
			CLANG_WARN_ENUM_CONVERSION         = "YES",
			CLANG_WARN_INFINITE_RECURSION      = "YES",
			CLANG_WARN_INT_CONVERSION          = "YES",
			CLANG_WARN_SUSPICIOUS_MOVE         = "YES",
			CLANG_WARN_UNREACHABLE_CODE        = "YES",
			CONFIGURATION_TEMP_DIR             = "$(OBJROOT)",
			ENABLE_STRICT_OBJC_MSGSEND         = "YES",
			ENABLE_TESTABILITY                 = "YES",
			GCC_C_LANGUAGE_STANDARD            = "gnu99",
			GCC_NO_COMMON_BLOCKS               = "YES",
			GCC_PREPROCESSOR_DEFINITIONS       = cfg.defines,
			GCC_SYMBOLS_PRIVATE_EXTERN         = "NO",
			GCC_WARN_64_TO_32_BIT_CONVERSION   = "YES",
			GCC_WARN_ABOUT_RETURN_TYPE         = "YES",
			GCC_WARN_UNDECLARED_SELECTOR       = "YES",
			GCC_WARN_UNINITIALIZED_AUTOS       = "YES",
			GCC_WARN_UNUSED_FUNCTION           = "YES",
			GCC_WARN_UNUSED_VARIABLE           = "YES",
			HEADER_SEARCH_PATHS                = table.join(cfg.includedirs, cfg.systemincludedirs),
			LIBRARY_SEARCH_PATHS               = cfg.libdirs,
			OBJROOT                            = cfg.objectsdir,
			ONLY_ACTIVE_ARCH                   = "YES",
			OTHER_CFLAGS                       = table.join(cflags, cfg.buildoptions, cfg.buildoptions_c),
			OTHER_CPLUSPLUSFLAGS               = table.join(cflags, cfg.buildoptions, cfg.buildoptions_cpp),
			OTHER_LDFLAGS                      = table.join(ldflags, cfg.linkoptions),
			SDKROOT                            = xcode.toolset,
			USER_HEADER_SEARCH_PATHS           = cfg.userincludedirs,
		}

		if tr.entitlements then
			options.CODE_SIGN_ENTITLEMENTS = tr.entitlements.cfg.name
		end

		local targetdir = path.getdirectory(cfg.buildtarget.bundlepath)
		if targetdir ~= "." then
			options.CONFIGURATION_BUILD_DIR = "$(SYMROOT)"
			options.SYMROOT = targetdir
		end

		if cfg.flags.Symbols then
			options.COPY_PHASE_STRIP = "NO"
		end

		local excluded = xcode.cfg_excluded_files(prj, cfg)
		if #excluded > 0 then
			options.EXCLUDED_SOURCE_FILE_NAMES = excluded
		end

		if cfg.flags.NoExceptions then
			options.GCC_ENABLE_CPP_EXCEPTIONS = "NO"
		end

		if cfg.flags.NoRTTI then
			options.GCC_ENABLE_CPP_RTTI = "NO"
		end

		if cfg.flags.Symbols and not cfg.flags.NoEditAndContinue then
			options.GCC_ENABLE_FIX_AND_CONTINUE = "YES"
		end

		if cfg.flags.NoExceptions then
			options.GCC_ENABLE_OBJC_EXCEPTIONS = "NO"
		end

		if cfg.flags.Optimize or cfg.flags.OptimizeSize then
			options.GCC_OPTIMIZATION_LEVEL = "s"
		elseif cfg.flags.OptimizeSpeed then
			options.GCC_OPTIMIZATION_LEVEL = 3
		else
			options.GCC_OPTIMIZATION_LEVEL = 0
		end

		if cfg.pchheader and not cfg.flags.NoPCH then
			options.GCC_PRECOMPILE_PREFIX_HEADER = "YES"

			-- Visual Studio requires the PCH header to be specified in the same way
			-- it appears in the #include statements used in the source code; the PCH
			-- source actual handles the compilation of the header. GCC compiles the
			-- header file directly, and needs the file's actual file system path in
			-- order to locate it.

			-- To maximize the compatibility between the two approaches, see if I can
			-- locate the specified PCH header on one of the include file search paths
			-- and, if so, adjust the path automatically so the user doesn't have
			-- add a conditional configuration to the project script.

			local pch = cfg.pchheader
			for _, incdir in ipairs(cfg.includedirs) do

				-- convert this back to an absolute path for os.isfile()
				local abspath = path.getabsolute(path.join(cfg.project.location, incdir))

				local testname = path.join(abspath, pch)
				if os.isfile(testname) then
					pch = path.getrelative(cfg.location, testname)
					break
				end
			end

			options.GCC_PREFIX_HEADER = pch
		end

		if cfg.flags.FatalWarnings then
			options.GCC_TREAT_WARNINGS_AS_ERRORS = "YES"
		end

		if cfg.kind == "Bundle" then
			options.MACH_O_TYPE = "mh_bundle"
		end

		if cfg.flags.StaticRuntime then
			options.STANDARD_C_PLUS_PLUS_LIBRARY_TYPE = "static"
		end

		if cfg.flags.PedanticWarnings or cfg.flags.ExtraWarnings then
			options.WARNING_CFLAGS = "-Wall"
		end

		if cfg.flags.Cpp11 then
			options.CLANG_CXX_LANGUAGE_STANDARD = "c++11"
		elseif cfg.flags.Cpp14 or cfg.flags.CppLatest then
			options.CLANG_CXX_LANGUAGE_STANDARD = "c++14"
		elseif cfg.flags.Cpp17 then
			-- XCode8 does not support C++17, but other actions use this as
			-- base that *do* support C++17, so check if this is the current
			-- action before erroring.
			if premake.action.current() == premake.action.get("xcode8") then
				error("XCode8 does not support C++17.")
			end
		end

		for _, val in ipairs(premake.xcode.parameters) do
			local eqpos = string.find(val, "=")
			if eqpos ~= nil then
				local key = string.trim(string.sub(val, 1, eqpos - 1))
				local value = string.trim(string.sub(val, eqpos + 1))
				options[key] = value
			end
		end

		return options
	end

	function xcode8.project(prj)
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
			ontarget = xcode8.XCBuildConfiguration_Target,
			onproject = xcode8.XCBuildConfiguration_Project,
		})
		xcode.XCBuildConfigurationList(tr)
		xcode.Footer(tr)
	end


--
-- xcode8 action
--

	newaction
	{
		trigger         = "xcode8",
		shortname       = "Xcode 8",
		description     = "Generate Apple Xcode 8 project files",
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
			premake.generate(prj, "%%.xcodeproj/project.pbxproj", xcode8.project)
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
