--
-- Copyright 2010-2024 Branimir Karadzic. All rights reserved.
-- License: https://github.com/bkaradzic/bx/blob/master/LICENSE
--

local bxDir = path.getabsolute("..")

local function crtNone()

	defines {
		"BX_CRT_NONE=1",
	}

	buildoptions {
		"-nostdlib",
		"-nodefaultlibs",
		"-nostartfiles",
		"-Wa,--noexecstack",
		"-ffreestanding",
	}

	linkoptions {
		"-nostdlib",
		"-nodefaultlibs",
		"-nostartfiles",
		"-Wa,--noexecstack",
		"-ffreestanding",
	}

	configuration { "linux-*" }

		buildoptions {
			"-mpreferred-stack-boundary=4",
			"-mstackrealign",
		}

		linkoptions {
			"-mpreferred-stack-boundary=4",
			"-mstackrealign",
		}

	configuration {}
end

local android = {};

local function androidToolchainRoot()
	if android.toolchainRoot == nil then
		local hostTags = {
			windows = "windows-x86_64",
			linux   = "linux-x86_64",
			macosx  = "darwin-x86_64"
		}
		android.toolchainRoot = "$(ANDROID_NDK_ROOT)/toolchains/llvm/prebuilt/" .. hostTags[os.get()]
	end

	return android.toolchainRoot;
end

function toolchain(_buildDir, _libDir)

	newoption {
		trigger = "gcc",
		value = "GCC",
		description = "Choose GCC flavor",
		allowed = {
			{ "android-arm",     "Android - ARM"              },
			{ "android-arm64",   "Android - ARM64"            },
			{ "android-x86",     "Android - x86"              },
			{ "android-x86_64",  "Android - x86_64"           },
			{ "wasm2js",         "Emscripten/Wasm2JS"         },
			{ "wasm",            "Emscripten/Wasm"            },
			{ "linux-gcc",       "Linux (GCC compiler)"       },
			{ "linux-gcc-afl",   "Linux (GCC + AFL fuzzer)"   },
			{ "linux-clang",     "Linux (Clang compiler)"     },
			{ "linux-clang-afl", "Linux (Clang + AFL fuzzer)" },
			{ "linux-arm-gcc",   "Linux (ARM, GCC compiler)"  },
			{ "linux-ppc64le-gcc",  "Linux (PPC64LE, GCC compiler)"  },
			{ "linux-ppc64le-clang",  "Linux (PPC64LE, Clang compiler)"  },
			{ "linux-riscv64-gcc",  "Linux (RISC-V 64, GCC compiler)"  },
			{ "ios-arm",         "iOS - ARM"                  },
			{ "ios-arm64",       "iOS - ARM64"                },
			{ "ios-simulator",   "iOS - Simulator"            },
			{ "tvos-arm64",      "tvOS - ARM64"               },
			{ "xros-arm64",      "visionOS ARM64"             },
			{ "xros-simulator",  "visionOS - Simulator"       },
			{ "tvos-simulator",  "tvOS - Simulator"           },
			{ "mingw-gcc",       "MinGW"                      },
			{ "mingw-clang",     "MinGW (clang compiler)"     },
			{ "osx-x64",         "OSX - x64"                  },
			{ "osx-arm64",       "OSX - ARM64"                },
			{ "orbis",           "Orbis"                      },
			{ "riscv",           "RISC-V"                     },
			{ "rpi",             "RaspberryPi"                },
		},
	}

	newoption {
		trigger = "vs",
		value = "toolset",
		description = "Choose VS toolset",
		allowed = {
			{ "vs2017-clang",  "Clang with MS CodeGen"           },
			{ "vs2017-xp",     "Visual Studio 2017 targeting XP" },
			{ "winstore100",   "Universal Windows App 10.0"      },
			{ "durango",       "Durango"                         },
			{ "orbis",         "Orbis"                           },
		},
	}

	newoption {
		trigger = "xcode",
		value = "xcode_target",
		description = "Choose XCode target",
		allowed = {
			{ "osx", "OSX" },
			{ "ios", "iOS" },
			{ "tvos", "tvOS" },
			{ "xros", "visionOS" },
		}
	}

	newoption {
		trigger     = "with-android",
		value       = "#",
		description = "Set Android platform version (default: android-24).",
	}

	newoption {
		trigger     = "with-ios",
		value       = "#",
		description = "Set iOS target version (default: 13.0).",
	}

	newoption {
		trigger     = "with-macos",
		value       = "#",
		description = "Set macOS target version (default 13.0).",
	}

	newoption {
		trigger     = "with-tvos",
		value       = "#",
		description = "Set tvOS target version (default: 13.0).",
	}

	newoption {
		trigger     = "with-visionos",
		value       = "#",
		description = "Set visionOS target version (default: 1.0).",
	}

	newoption {
		trigger = "with-windows",
		value = "#",
		description = "Set the Windows target platform version (default: $WindowsSDKVersion or 8.1).",
	}

	newoption {
		trigger     = "with-dynamic-runtime",
		description = "Dynamically link with the runtime rather than statically",
	}

	newoption {
		trigger     = "with-32bit-compiler",
		description = "Use 32-bit compiler instead 64-bit.",
	}

	newoption {
		trigger     = "with-avx",
		description = "Use AVX extension.",
	}

	-- Avoid error when invoking genie --help.
	if (_ACTION == nil) then return false end

	location (path.join(_buildDir, "projects", _ACTION))

	if _ACTION == "clean" then
		os.rmdir(_buildDir)
		os.mkdir(_buildDir)
		os.exit(1)
	end

	local androidApiLevel = 24
	if _OPTIONS["with-android"] then
		androidApiLevel = _OPTIONS["with-android"]
	end

	local iosPlatform = ""
	if _OPTIONS["with-ios"] then
		iosPlatform = _OPTIONS["with-ios"]
	end

	local macosPlatform = ""
	if _OPTIONS["with-macos"] then
		macosPlatform = _OPTIONS["with-macos"]
	end

	local tvosPlatform = ""
	if _OPTIONS["with-tvos"] then
		tvosPlatform = _OPTIONS["with-tvos"]
	end

	local xrosPlatform = ""
	if _OPTIONS["with-xros"] then
		xrosPlatform = _OPTIONS["with-xros"]
	end

	local windowsPlatform = nil
	if _OPTIONS["with-windows"] then
		windowsPlatform = _OPTIONS["with-windows"]
	elseif nil ~= os.getenv("WindowsSDKVersion") then
		windowsPlatform = string.gsub(os.getenv("WindowsSDKVersion"), "\\", "")
	end

	local compiler32bit = false
	if _OPTIONS["with-32bit-compiler"] then
		compiler32bit = true
	end

	flags {
		"Cpp17",
		"ExtraWarnings",
		"FloatFast",
	}

	if _ACTION == "gmake" or _ACTION == "ninja" then

		if nil == _OPTIONS["gcc"] then
			print("GCC flavor must be specified!")
			os.exit(1)
		end

		if "android-arm"    == _OPTIONS["gcc"]
		or "android-arm64"  == _OPTIONS["gcc"]
		or "android-x86"    == _OPTIONS["gcc"]
		or "android-x86_64" == _OPTIONS["gcc"] then

			premake.gcc.cc   = androidToolchainRoot() .. "/bin/clang"
			premake.gcc.cxx  = androidToolchainRoot() .. "/bin/clang++"
			premake.gcc.ar   = androidToolchainRoot() .. "/bin/llvm-ar"

			premake.gcc.llvm = true
			location (path.join(_buildDir, "projects", _ACTION .. "-" .. _OPTIONS["gcc"]))

		elseif "wasm2js" == _OPTIONS["gcc"] or "wasm" == _OPTIONS["gcc"] then

			if not os.getenv("EMSCRIPTEN") then
				print("Set EMSCRIPTEN environment variable to root directory of your Emscripten installation. (e.g. by entering the EMSDK command prompt)")
			end

			premake.gcc.cc   = "\"$(EMSCRIPTEN)/emcc\""
			premake.gcc.cxx  = "\"$(EMSCRIPTEN)/em++\""
			premake.gcc.ar   = "\"$(EMSCRIPTEN)/emar\""
			premake.gcc.llvm = true
			premake.gcc.namestyle = "Emscripten"
			location (path.join(_buildDir, "projects", _ACTION .. "-" .. _OPTIONS["gcc"]))

		elseif "ios-arm"   == _OPTIONS["gcc"]
			or "ios-arm64" == _OPTIONS["gcc"] then
			premake.gcc.cc  = "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang"
			premake.gcc.cxx = "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang++"
			premake.gcc.ar  = "ar"
			location (path.join(_buildDir, "projects", _ACTION .. "-" .. _OPTIONS["gcc"]))

		elseif "xros-arm64"     == _OPTIONS["gcc"]
			or "xros-simulator" == _OPTIONS["gcc"] then
			premake.gcc.cc  = "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang"
			premake.gcc.cxx = "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang++"
			premake.gcc.ar  = "ar"
			location (path.join(_buildDir, "projects", _ACTION .. "-" .. _OPTIONS["gcc"]))

		elseif "ios-simulator" == _OPTIONS["gcc"] then
			premake.gcc.cc  = "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang"
			premake.gcc.cxx = "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang++"
			premake.gcc.ar  = "ar"
			location (path.join(_buildDir, "projects", _ACTION .. "-ios-simulator"))

		elseif "tvos-arm64" == _OPTIONS["gcc"] then
			premake.gcc.cc  = "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang"
			premake.gcc.cxx = "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang++"
			premake.gcc.ar  = "ar"
			location (path.join(_buildDir, "projects", _ACTION .. "-tvos-arm64"))

		elseif "tvos-simulator" == _OPTIONS["gcc"] then
			premake.gcc.cc  = "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang"
			premake.gcc.cxx = "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang++"
			premake.gcc.ar  = "ar"
			location (path.join(_buildDir, "projects", _ACTION .. "-tvos-simulator"))

		elseif "linux-gcc" == _OPTIONS["gcc"] then
			location (path.join(_buildDir, "projects", _ACTION .. "-linux-gcc"))

		elseif "linux-gcc-afl" == _OPTIONS["gcc"] then
			premake.gcc.cc  = "afl-gcc"
			premake.gcc.cxx = "afl-g++"
			premake.gcc.ar  = "ar"
			location (path.join(_buildDir, "projects", _ACTION .. "-linux-gcc"))

		elseif "linux-clang" == _OPTIONS["gcc"] then
			premake.gcc.cc  = "clang"
			premake.gcc.cxx = "clang++"
			premake.gcc.ar  = "ar"
			location (path.join(_buildDir, "projects", _ACTION .. "-linux-clang"))

		elseif "linux-clang-afl" == _OPTIONS["gcc"] then
			premake.gcc.cc  = "afl-clang"
			premake.gcc.cxx = "afl-clang++"
			premake.gcc.ar  = "ar"
			location (path.join(_buildDir, "projects", _ACTION .. "-linux-clang"))

		elseif "linux-arm-gcc" == _OPTIONS["gcc"] then
			location (path.join(_buildDir, "projects", _ACTION .. "-linux-arm-gcc"))

		elseif "linux-ppc64le-gcc" == _OPTIONS["gcc"] then
 			location (path.join(_buildDir, "projects", _ACTION .. "-linux-ppc64le-gcc"))

		elseif "linux-ppc64le-clang" == _OPTIONS["gcc"] then
			premake.gcc.cc  = "clang"
			premake.gcc.cxx = "clang++"
			premake.gcc.ar  = "ar"
			premake.gcc.llvm = true
			location (path.join(_buildDir, "projects", _ACTION .. "-linux-ppc64le-clang"))

		elseif "linux-riscv64-gcc" == _OPTIONS["gcc"] then
			location (path.join(_buildDir, "projects", _ACTION .. "-linux-riscv64-gcc"))

		elseif "mingw-gcc" == _OPTIONS["gcc"] then
			if not os.getenv("MINGW") then
				print("Set MINGW environment variable.")
			end

			local mingwToolchain = "x86_64-w64-mingw32"
			if compiler32bit then
				if os.is("linux") then
					mingwToolchain = "i686-w64-mingw32"
				else
					mingwToolchain = "mingw32"
				end
			end

			premake.gcc.cc  = "$(MINGW)/bin/" .. mingwToolchain .. "-gcc"
			premake.gcc.cxx = "$(MINGW)/bin/" .. mingwToolchain .. "-g++"
			premake.gcc.ar  = "$(MINGW)/bin/ar"
			location (path.join(_buildDir, "projects", _ACTION .. "-mingw-gcc"))

		elseif "mingw-clang" == _OPTIONS["gcc"] then
			if not os.getenv("MINGW") then
				print("Set MINGW environment variable.")
			end

			if not os.getenv("CLANG") then
				print("Set CLANG environment variable.")
			end

			premake.gcc.cc   = "$(CLANG)/bin/clang"
			premake.gcc.cxx  = "$(CLANG)/bin/clang++"
			premake.gcc.ar   = "$(MINGW)/bin/ar"
--			premake.gcc.ar   = "$(CLANG)/bin/llvm-ar"
--			premake.gcc.llvm = true
			location (path.join(_buildDir, "projects", _ACTION .. "-mingw-clang"))

		elseif "osx-x64"   == _OPTIONS["gcc"]
			or "osx-arm64" == _OPTIONS["gcc"] then

			if os.is("linux") then
				if not os.getenv("OSXCROSS") then
					print("Set OSXCROSS environment variable.")
				end

				local osxToolchain = "x86_64-apple-darwin15-"
				premake.gcc.cc  = "$(OSXCROSS)/target/bin/" .. osxToolchain .. "clang"
				premake.gcc.cxx = "$(OSXCROSS)/target/bin/" .. osxToolchain .. "clang++"
				premake.gcc.ar  = "$(OSXCROSS)/target/bin/" .. osxToolchain .. "ar"
			end

			location (path.join(_buildDir, "projects", _ACTION .. "-" .. _OPTIONS["gcc"]))

		elseif "orbis" == _OPTIONS["gcc"] then

			if not os.getenv("SCE_ORBIS_SDK_DIR") then
				print("Set SCE_ORBIS_SDK_DIR environment variable.")
			end

			orbisToolchain = "$(SCE_ORBIS_SDK_DIR)/host_tools/bin/orbis-"

			premake.gcc.cc  = orbisToolchain .. "clang"
			premake.gcc.cxx = orbisToolchain .. "clang++"
			premake.gcc.ar  = orbisToolchain .. "ar"
			location (path.join(_buildDir, "projects", _ACTION .. "-orbis"))

		elseif "rpi" == _OPTIONS["gcc"] then
			location (path.join(_buildDir, "projects", _ACTION .. "-rpi"))

		elseif "riscv" == _OPTIONS["gcc"] then
			premake.gcc.cc  = "$(FREEDOM_E_SDK)/work/build/riscv-gnu-toolchain/riscv64-unknown-elf/prefix/bin/riscv64-unknown-elf-gcc"
			premake.gcc.cxx = "$(FREEDOM_E_SDK)/work/build/riscv-gnu-toolchain/riscv64-unknown-elf/prefix/bin/riscv64-unknown-elf-g++"
			premake.gcc.ar  = "$(FREEDOM_E_SDK)/work/build/riscv-gnu-toolchain/riscv64-unknown-elf/prefix/bin/riscv64-unknown-elf-ar"
			location (path.join(_buildDir, "projects", _ACTION .. "-riscv"))

		end
	elseif _ACTION == "vs2017"
		or _ACTION == "vs2019"
		or _ACTION == "vs2022"
		then

		local action = premake.action.current()
		if nil ~= windowsPlatform then
			action.vstudio.windowsTargetPlatformVersion    = windowsPlatform
			action.vstudio.windowsTargetPlatformMinVersion = windowsPlatform
		end

		if (_ACTION .. "-clang") == _OPTIONS["vs"] then
			if "vs2017-clang" == _OPTIONS["vs"] then
				premake.vstudio.toolset = "v141_clang_c2"
			else
				premake.vstudio.toolset = ("LLVM-" .. _ACTION)
			end
			location (path.join(_buildDir, "projects", _ACTION .. "-clang"))

		elseif "winstore100" == _OPTIONS["vs"] then
			premake.vstudio.toolset = "v141"
			premake.vstudio.storeapp = "10.0"

			platforms { "ARM" }
			location (path.join(_buildDir, "projects", _ACTION .. "-winstore100"))

		elseif "durango" == _OPTIONS["vs"] then
			if not os.getenv("DurangoXDK") then
				print("DurangoXDK not found.")
			end

			premake.vstudio.toolset = "v140"
			premake.vstudio.storeapp = "durango"
			platforms { "Durango" }
			location (path.join(_buildDir, "projects", _ACTION .. "-durango"))
		elseif "orbis" == _OPTIONS["vs"] then

			if not os.getenv("SCE_ORBIS_SDK_DIR") then
				print("Set SCE_ORBIS_SDK_DIR environment variable.")
			end

			platforms { "Orbis" }
			location (path.join(_buildDir, "projects", _ACTION .. "-orbis"))

		end

	elseif _ACTION and _ACTION:match("^xcode.+$") then
		local action = premake.action.current()
		local str_or = function(str, def)
			return #str > 0 and str or def
		end

		if "osx" == _OPTIONS["xcode"] then
			action.xcode.macOSTargetPlatformVersion = str_or(macosPlatform, "13.0")
			premake.xcode.toolset = "macosx"
			location (path.join(_buildDir, "projects", _ACTION .. "-osx"))

		elseif "ios" == _OPTIONS["xcode"] then
			action.xcode.iOSTargetPlatformVersion = str_or(iosPlatform, "13.0")
			premake.xcode.toolset = "iphoneos"
			location (path.join(_buildDir, "projects", _ACTION .. "-ios"))

		elseif "tvos" == _OPTIONS["xcode"] then
			action.xcode.tvOSTargetPlatformVersion = str_or(tvosPlatform, "13.0")
			premake.xcode.toolset = "appletvos"
			location (path.join(_buildDir, "projects", _ACTION .. "-tvos"))

		elseif "xros" == _OPTIONS["xcode"] then
			action.xcode.visionOSTargetPlatformVersion = str_or(xrosPlatform, "1.0")
			premake.xcode.toolset = "xros"
			location (path.join(_buildDir, "projects", _ACTION .. "-xros"))
		end
	end

	if not _OPTIONS["with-dynamic-runtime"] then
		flags { "StaticRuntime" }
	end

	if _OPTIONS["with-avx"] then
		flags { "EnableAVX" }
	end

	if _OPTIONS["with-crtnone"] then
		crtNone()
	end

	flags {
		"NoPCH",
		"NativeWChar",
		"NoRTTI",
		"NoExceptions",
		"NoEditAndContinue",
		"NoFramePointer",
		"Symbols",
	}

	defines {
		"__STDC_LIMIT_MACROS",
		"__STDC_FORMAT_MACROS",
		"__STDC_CONSTANT_MACROS",
	}

	configuration { "Debug" }
		targetsuffix "Debug"
		defines {
			"_DEBUG",
		}

	configuration { "Release" }
		flags {
			"NoBufferSecurityCheck",
			"OptimizeSpeed",
		}
		defines {
			"NDEBUG",
		}
		targetsuffix "Release"

	configuration { "*-clang" }
		buildoptions {
			"-Wno-tautological-constant-compare",
		}

	configuration { "vs*", "not NX32", "not NX64" }
		flags {
			"EnableAVX",
		}

	configuration { "vs*", "not orbis", "not NX32", "not NX64" }
		includedirs { path.join(bxDir, "include/compat/msvc") }
		defines {
			"WIN32",
			"_WIN32",
			"_HAS_EXCEPTIONS=0",
			"_SCL_SECURE=0",
			"_SECURE_SCL=0",
			"_SCL_SECURE_NO_WARNINGS",
			"_CRT_SECURE_NO_WARNINGS",
			"_CRT_SECURE_NO_DEPRECATE",
		}
		buildoptions {
			"/wd4201", -- warning C4201: nonstandard extension used: nameless struct/union
			"/wd4324", -- warning C4324: '': structure was padded due to alignment specifier
			"/Ob2",    -- The Inline Function Expansion

			"/Zc:__cplusplus",  -- Enable updated __cplusplus macro.
			"/Zc:preprocessor", -- Enable preprocessor conformance mode.
		}
		linkoptions {
			"/ignore:4221", -- LNK4221: This object file does not define any previously undefined public symbols, so it will not be used by any link operation that consumes this library
		}

	configuration { "x32", "vs*" }
		targetdir (path.join(_buildDir, "win32_" .. _ACTION, "bin"))
		objdir (path.join(_buildDir, "win32_" .. _ACTION, "obj"))
		libdirs {
			path.join(_libDir, "lib/win32_" .. _ACTION),
		}

	configuration { "x64", "vs*" }
		defines { "_WIN64" }
		targetdir (path.join(_buildDir, "win64_" .. _ACTION, "bin"))
		objdir (path.join(_buildDir, "win64_" .. _ACTION, "obj"))
		libdirs {
			path.join(_libDir, "lib/win64_" .. _ACTION),
		}

	configuration { "x32", "vs2017" }
		targetdir (path.join(_buildDir, "win32_" .. _ACTION, "bin"))
		objdir (path.join(_buildDir, "win32_" .. _ACTION, "obj"))
		libdirs {
			path.join(_libDir, "lib/win32_" .. _ACTION),
		}

	configuration { "x64", "vs2017" }
		defines { "_WIN64" }
		targetdir (path.join(_buildDir, "win64_" .. _ACTION, "bin"))
		objdir (path.join(_buildDir, "win64_" .. _ACTION, "obj"))
		libdirs {
			path.join(_libDir, "lib/win64_" .. _ACTION),
		}

	configuration { "ARM", "vs*" }
		targetdir (path.join(_buildDir, "arm_" .. _ACTION, "bin"))
		objdir (path.join(_buildDir, "arm_" .. _ACTION, "obj"))

	configuration { "vs*-clang" }
		buildoptions {
			"-Qunused-arguments",
		}

	configuration { "x32", "vs*-clang" }
		targetdir (path.join(_buildDir, "win32_" .. _ACTION .. "-clang/bin"))
		objdir (path.join(_buildDir, "win32_" .. _ACTION .. "-clang/obj"))

	configuration { "x64", "vs*-clang" }
		targetdir (path.join(_buildDir, "win64_" .. _ACTION .. "-clang/bin"))
		objdir (path.join(_buildDir, "win64_" .. _ACTION .. "-clang/obj"))

	configuration { "winstore*" }
		removeflags {
			"StaticRuntime",
			"NoBufferSecurityCheck",
		}
		buildoptions {
			"/wd4530", -- vccorlib.h(1345): warning C4530: C++ exception handler used, but unwind semantics are not enabled. Specify /EHsc
		}
		linkoptions {
			"/ignore:4264" -- LNK4264: archiving object file compiled with /ZW into a static library; note that when authoring Windows Runtime types it is not recommended to link with a static library that contains Windows Runtime metadata
		}

	configuration { "*-gcc* or osx" }
		buildoptions {
			"-Wshadow",
		}

	configuration { "mingw-*" }
		defines { "WIN32" }
		includedirs { path.join(bxDir, "include/compat/mingw") }
		defines {
			"MINGW_HAS_SECURE_API=1",
		}
		buildoptions {
			"-Wa,-mbig-obj",
			"-Wundef",
			"-Wunused-value",
			"-fdata-sections",
			"-ffunction-sections",
			"-msse4.2",
		}
		linkoptions {
			"-Wl,--gc-sections",
			"-static",
			"-static-libgcc",
			"-static-libstdc++",
		}

	configuration { "x32", "mingw-gcc" }
		targetdir (path.join(_buildDir, "win32_mingw-gcc/bin"))
		objdir (path.join(_buildDir, "win32_mingw-gcc/obj"))
		libdirs {
			path.join(_libDir, "lib/win32_mingw-gcc"),
		}
		buildoptions {
			"-m32",
			"-mstackrealign",
		}

	configuration { "x64", "mingw-gcc" }
		targetdir (path.join(_buildDir, "win64_mingw-gcc/bin"))
		objdir (path.join(_buildDir, "win64_mingw-gcc/obj"))
		libdirs {
			path.join(_libDir, "lib/win64_mingw-gcc"),
		}
		buildoptions { "-m64" }

	configuration { "mingw-clang" }
		buildoptions {
			"-isystem $(MINGW)/lib/gcc/x86_64-w64-mingw32/4.8.1/include/c++",
			"-isystem $(MINGW)/lib/gcc/x86_64-w64-mingw32/4.8.1/include/c++/x86_64-w64-mingw32",
			"-isystem $(MINGW)/x86_64-w64-mingw32/include",
			"-Wno-nan-infinity-disabled",
		}
		linkoptions {
			"-Qunused-arguments",
			"-Wno-error=unused-command-line-argument-hard-error-in-future",
		}

	configuration { "x32", "mingw-clang" }
		targetdir (path.join(_buildDir, "win32_mingw-clang/bin"))
		objdir (path.join(_buildDir, "win32_mingw-clang/obj"))
		libdirs {
			path.join(_libDir, "lib/win32_mingw-clang"),
		}
		buildoptions { "-m32" }

	configuration { "x64", "mingw-clang" }
		targetdir (path.join(_buildDir, "win64_mingw-clang/bin"))
		objdir (path.join(_buildDir, "win64_mingw-clang/obj"))
		libdirs {
			path.join(_libDir, "lib/win64_mingw-clang"),
		}
		buildoptions { "-m64" }

	configuration { "linux-*" }
		includedirs { path.join(bxDir, "include/compat/linux") }

	configuration { "linux-gcc" }
		buildoptions {
			"-mfpmath=sse",
		}

	configuration { "linux-gcc* or linux-clang*" }
		buildoptions {
			"-msse4.2",
--			"-Wdouble-promotion",
--			"-Wduplicated-branches",
--			"-Wduplicated-cond",
--			"-Wjump-misses-init",
			"-Wshadow",
--			"-Wnull-dereference",
			"-Wunused-value",
			"-Wundef",
--			"-Wuseless-cast",
		}
		links {
			"rt",
			"dl",
		}
		linkoptions {
			"-Wl,--gc-sections",
			"-Wl,--as-needed",
		}

	configuration { "linux-gcc*" }
		buildoptions {
			"-Wlogical-op",
		}

	configuration { "linux-gcc*", "x32" }
		targetdir (path.join(_buildDir, "linux32_gcc/bin"))
		objdir (path.join(_buildDir, "linux32_gcc/obj"))
		libdirs { path.join(_libDir, "lib/linux32_gcc") }
		buildoptions {
			"-m32",
		}

	configuration { "linux-gcc*", "x64" }
		targetdir (path.join(_buildDir, "linux64_gcc/bin"))
		objdir (path.join(_buildDir, "linux64_gcc/obj"))
		libdirs { path.join(_libDir, "lib/linux64_gcc") }
		buildoptions {
			"-m64",
		}

	configuration { "linux-clang*", "x32" }
		targetdir (path.join(_buildDir, "linux32_clang/bin"))
		objdir (path.join(_buildDir, "linux32_clang/obj"))
		libdirs { path.join(_libDir, "lib/linux32_clang") }
		buildoptions {
			"-m32",
		}

	configuration { "linux-clang*", "x64" }
		targetdir (path.join(_buildDir, "linux64_clang/bin"))
		objdir (path.join(_buildDir, "linux64_clang/obj"))
		libdirs { path.join(_libDir, "lib/linux64_clang") }
		buildoptions {
			"-m64",
		}

	configuration { "linux-arm-gcc" }
		targetdir (path.join(_buildDir, "linux32_arm_gcc/bin"))
		objdir (path.join(_buildDir, "linux32_arm_gcc/obj"))
		libdirs { path.join(_libDir, "lib/linux32_arm_gcc") }
		buildoptions {
			"-Wunused-value",
			"-Wundef",
		}
		links {
			"rt",
			"dl",
		}
		linkoptions {
			"-Wl,--gc-sections",
		}

	configuration { "android-*" }
		targetprefix ("lib")
		flags {
			"NoImportLib",
		}
		links {
			"c",
			"dl",
			"m",
			"android",
			"log",
			"c++_shared",
		}
		buildoptions {
			"--gcc-toolchain=" .. androidToolchainRoot(),
			"--sysroot=" .. androidToolchainRoot() .. "/sysroot",
			"-DANDROID",
			"-fPIC",
			"-no-canonical-prefixes",
			"-Wa,--noexecstack",
			"-fstack-protector-strong",
			"-ffunction-sections",
			"-Wunused-value",
			"-Wundef",
		}
		linkoptions {
			"--gcc-toolchain=" .. androidToolchainRoot(),
			"--sysroot=" .. androidToolchainRoot() .. "/sysroot",
			"-no-canonical-prefixes",
			"-Wl,--no-undefined",
			"-Wl,-z,noexecstack",
			"-Wl,-z,relro",
			"-Wl,-z,now",
		}

	configuration { "android-arm" }
		targetdir (path.join(_buildDir, "android-arm/bin"))
		objdir (path.join(_buildDir, "android-arm/obj"))
		buildoptions {
			"--target=armv7-none-linux-android" .. androidApiLevel,
			"-mthumb",
			"-march=armv7-a",
			"-mfloat-abi=softfp",
			"-mfpu=neon",
		}
		linkoptions {
			"--target=armv7-none-linux-android" .. androidApiLevel,
			"-march=armv7-a",
		}

	configuration { "android-arm64" }
		targetdir (path.join(_buildDir, "android-arm64/bin"))
		objdir (path.join(_buildDir, "android-arm64/obj"))
		buildoptions {
			"--target=aarch64-none-linux-android" .. androidApiLevel,
		}
		linkoptions {
			"--target=aarch64-none-linux-android" .. androidApiLevel,
		}

	configuration { "android-x86" }
		targetdir (path.join(_buildDir, "android-x86/bin"))
		objdir (path.join(_buildDir, "android-x86/obj"))
		buildoptions {
			"--target=i686-none-linux-android" .. androidApiLevel,
			"-mtune=atom",
			"-mstackrealign",
			"-msse4.2",
			"-mfpmath=sse",
		}
		linkoptions {
			"--target=i686-none-linux-android" .. androidApiLevel,
		}

	configuration { "android-x86_64" }
		targetdir (path.join(_buildDir, "android-x86_64/bin"))
		objdir (path.join(_buildDir, "android-x86_64/obj"))
		buildoptions {
			"--target=x86_64-none-linux-android" .. androidApiLevel,
		}
		linkoptions {
			"--target=x86_64-none-linux-android" .. androidApiLevel,
		}

	configuration { "wasm*" }
		buildoptions {
			"-Wunused-value",
			"-Wundef",
		}

		linkoptions {
			"-s MAX_WEBGL_VERSION=2",
		}

	configuration { "linux-ppc64le*" }
		buildoptions {
			"-fsigned-char",
			"-Wunused-value",
			"-Wundef",
			"-mcpu=power8",
		}
		links {
			"rt",
			"dl",
		}
		linkoptions {
			"-Wl,--gc-sections",
		}

	configuration { "linux-riscv64*" }
		buildoptions {
			"-Wunused-value",
			"-Wundef",
			"-march=rv64g"
		}
		links {
			"rt",
			"dl",
		}
		linkoptions {
			"-Wl,--gc-sections",
		}

	configuration { "linux-ppc64le-gcc" }
		targetdir (path.join(_buildDir, "linux_ppc64le_gcc/bin"))
		objdir (path.join(_buildDir, "linux_ppc64le_gcc/obj"))
		libdirs { path.join(_libDir, "lib/linux_ppc64le_gcc") }

	configuration { "linux-ppc64le-clang" }
		targetdir (path.join(_buildDir, "linux_ppc64le_clang/bin"))
		objdir (path.join(_buildDir, "linux_ppc64le_clang/obj"))
		libdirs { path.join(_libDir, "lib/linux_ppc64le_clang") }

	configuration { "linux-riscv64-gcc" }
		targetdir (path.join(_buildDir, "linux_riscv64_gcc/bin"))
		objdir (path.join(_buildDir, "linux_riscv64_gcc/obj"))
		libdirs { path.join(_libDir, "lib/linux_riscv64_gcc") }

	configuration { "wasm2js" }
		targetdir (path.join(_buildDir, "wasm2js/bin"))
		objdir (path.join(_buildDir, "wasm2js/obj"))
		libdirs { path.join(_libDir, "lib/wasm2js") }
		linkoptions {
			"-s WASM=0",
		}

	configuration { "wasm" }
		targetdir (path.join(_buildDir, "wasm/bin"))
		objdir (path.join(_buildDir, "wasm/obj"))
		libdirs { path.join(_libDir, "lib/wasm") }

	configuration { "durango" }
		targetdir (path.join(_buildDir, "durango/bin"))
		objdir (path.join(_buildDir, "durango/obj"))
		includedirs { path.join(bxDir, "include/compat/msvc") }
		libdirs { path.join(_libDir, "lib/durango") }
		removeflags { "StaticRuntime" }
		defines {
			"NOMINMAX",
		}

	configuration { "osx-x64" }
		targetdir (path.join(_buildDir, "osx-x64/bin"))
		objdir (path.join(_buildDir, "osx-x64/obj"))
		linkoptions {
			"-arch x86_64",
		}
		buildoptions {
			"-arch x86_64",
			"-msse4.2",
			"-target x86_64-apple-macos" .. (#macosPlatform > 0 and macosPlatform or "13.0"),
		}

	configuration { "osx-arm64" }
		targetdir (path.join(_buildDir, "osx-arm64/bin"))
		objdir (path.join(_buildDir, "osx-arm64/obj"))
		linkoptions {
			"-arch arm64",
		}
		buildoptions {
			"-arch arm64",
			"-Wno-error=unused-command-line-argument",
			"-Wno-unused-command-line-argument",
		}

	configuration { "osx*" }
		buildoptions {
			"-Wfatal-errors",
			"-Wunused-value",
			"-Wundef",
--			"-Wno-overriding-t-option",
--			"-mmacosx-version-min=13.0",
		}
		includedirs { path.join(bxDir, "include/compat/osx") }

	configuration { "ios*" }
		linkoptions {
			"-lc++",
		}
		buildoptions {
			"-Wfatal-errors",
			"-Wunused-value",
			"-Wundef",
--			"-mios-version-min=16.0",
		}
		includedirs { path.join(bxDir, "include/compat/ios") }

	configuration { "xcode*", "ios*" }
		targetdir (path.join(_buildDir, "ios-arm/bin"))
		objdir (path.join(_buildDir, "ios-arm/obj"))

	configuration { "ios-arm" }
		targetdir (path.join(_buildDir, "ios-arm/bin"))
		objdir (path.join(_buildDir, "ios-arm/obj"))
		libdirs { path.join(_libDir, "lib/ios-arm") }
		linkoptions {
			"-arch armv7",
		}
		buildoptions {
			"-arch armv7",
		}

	configuration { "ios-arm64" }
		targetdir (path.join(_buildDir, "ios-arm64/bin"))
		objdir (path.join(_buildDir, "ios-arm64/obj"))
		libdirs { path.join(_libDir, "lib/ios-arm64") }
		linkoptions {
			"-arch arm64",
		}
		buildoptions {
			"-arch arm64",
		}

	configuration { "ios-arm*" }
		linkoptions {
			"--sysroot=/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS" ..iosPlatform .. ".sdk",
			"-L/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS" ..iosPlatform .. ".sdk/usr/lib/system",
			"-F/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS" ..iosPlatform .. ".sdk/System/Library/Frameworks",
			"-F/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS" ..iosPlatform .. ".sdk/System/Library/PrivateFrameworks",
		}
		buildoptions {
			"--sysroot=/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS" ..iosPlatform .. ".sdk",
			"-fembed-bitcode",
		}

	configuration { "xros*" }
		linkoptions {
			"-lc++",
		}
		buildoptions {
			"-Wfatal-errors",
			"-Wunused-value",
			"-Wundef",
		}
		includedirs { path.join(bxDir, "include/compat/ios") }

	configuration { "xros-arm64" }
		targetdir (path.join(_buildDir, "xros-arm64/bin"))
		objdir (path.join(_buildDir, "xros-arm64/obj"))
		libdirs { path.join(_libDir, "lib/xros-arm64") }
		linkoptions {
			"--sysroot=/Applications/Xcode.app/Contents/Developer/Platforms/XROS.platform/Developer/SDKs/XROS" ..xrosPlatform.. ".sdk",
			"-L/Applications/Xcode.app/Contents/Developer/Platforms/XROS.platform/Developer/SDKs/XROS" ..xrosPlatform .. ".sdk/usr/lib/system",
			"-F/Applications/Xcode.app/Contents/Developer/Platforms/XROS.platform/Developer/SDKs/XROS" ..xrosPlatform .. ".sdk/System/Library/Frameworks",
			"-F/Applications/Xcode.app/Contents/Developer/Platforms/XROS.platform/Developer/SDKs/XROS" ..xrosPlatform .. ".sdk/System/Library/PrivateFrameworks",
		}
		buildoptions {
			"--sysroot=/Applications/Xcode.app/Contents/Developer/Platforms/XROS.platform/Developer/SDKs/XROS" ..tvosPlatform .. ".sdk",
		}

	configuration { "xros-simulator" }
		targetdir (path.join(_buildDir, "xros-simulator/bin"))
		objdir (path.join(_buildDir, "xros-simulator/obj"))
		libdirs { path.join(_libDir, "lib/xros-simulator") }

		linkoptions {
			"--sysroot=/Applications/Xcode.app/Contents/Developer/Platforms/XRSimulator.platform/Developer/SDKs/XRSimulator" ..xrosPlatform.. ".sdk",
			"-L/Applications/Xcode.app/Contents/Developer/Platforms/XRSimulator.platform/Developer/SDKs/XRSimulator" ..xrosPlatform .. ".sdk/usr/lib/system",
			"-F/Applications/Xcode.app/Contents/Developer/Platforms/XRSimulator.platform/Developer/SDKs/XRSimulator" ..xrosPlatform .. ".sdk/System/Library/Frameworks",
		}
		buildoptions {
			"--sysroot=/Applications/Xcode.app/Contents/Developer/Platforms/XRSimulator.platform/Developer/SDKs/XRSimulator" ..xrosPlatform .. ".sdk",
		}

	configuration { "ios-simulator" }
		targetdir (path.join(_buildDir, "ios-simulator/bin"))
		objdir (path.join(_buildDir, "ios-simulator/obj"))
		libdirs { path.join(_libDir, "lib/ios-simulator") }
		linkoptions {
			"--sysroot=/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator" ..iosPlatform .. ".sdk",
			"-L/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator" ..iosPlatform .. ".sdk/usr/lib/system",
			"-F/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator" ..iosPlatform .. ".sdk/System/Library/Frameworks",
			"-F/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator" ..iosPlatform .. ".sdk/System/Library/PrivateFrameworks",
		}
		buildoptions {
			"--sysroot=/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator" ..iosPlatform .. ".sdk",
		}

	configuration { "tvos*" }
		linkoptions {
			"-lc++",
		}
		buildoptions {
			"-Wfatal-errors",
			"-Wunused-value",
			"-Wundef",
		}
		includedirs { path.join(bxDir, "include/compat/ios") }

	configuration { "xcode*", "tvos*" }
		targetdir (path.join(_buildDir, "tvos-arm64/bin"))
		objdir (path.join(_buildDir, "tvos-arm64/obj"))

	configuration { "tvos-arm64" }
		targetdir (path.join(_buildDir, "tvos-arm64/bin"))
		objdir (path.join(_buildDir, "tvos-arm64/obj"))
		libdirs { path.join(_libDir, "lib/tvos-arm64") }
		linkoptions {
			"-mtvos-version-min=9.0",
			"-arch arm64",
			"--sysroot=/Applications/Xcode.app/Contents/Developer/Platforms/AppleTVOS.platform/Developer/SDKs/AppleTVOS" ..tvosPlatform .. ".sdk",
			"-L/Applications/Xcode.app/Contents/Developer/Platforms/AppleTVOS.platform/Developer/SDKs/AppleTVOS" ..tvosPlatform .. ".sdk/usr/lib/system",
			"-F/Applications/Xcode.app/Contents/Developer/Platforms/AppleTVOS.platform/Developer/SDKs/AppleTVOS" ..tvosPlatform .. ".sdk/System/Library/Frameworks",
			"-F/Applications/Xcode.app/Contents/Developer/Platforms/AppleTVOS.platform/Developer/SDKs/AppleTVOS" ..tvosPlatform .. ".sdk/System/Library/PrivateFrameworks",
		}
		buildoptions {
			"-mtvos-version-min=9.0",
			"-arch arm64",
			"--sysroot=/Applications/Xcode.app/Contents/Developer/Platforms/AppleTVOS.platform/Developer/SDKs/AppleTVOS" ..tvosPlatform .. ".sdk",
		}

	configuration { "tvos-simulator" }
		targetdir (path.join(_buildDir, "tvos-simulator/bin"))
		objdir (path.join(_buildDir, "tvos-simulator/obj"))
		libdirs { path.join(_libDir, "lib/tvos-simulator") }
		linkoptions {
			"--sysroot=/Applications/Xcode.app/Contents/Developer/Platforms/AppleTVSimulator.platform/Developer/SDKs/AppleTVSimulator" ..tvosPlatform .. ".sdk",
			"-L/Applications/Xcode.app/Contents/Developer/Platforms/AppleTVSimulator.platform/Developer/SDKs/AppleTVSimulator" ..tvosPlatform .. ".sdk/usr/lib/system",
			"-F/Applications/Xcode.app/Contents/Developer/Platforms/AppleTVSimulator.platform/Developer/SDKs/AppleTVSimulator" ..tvosPlatform .. ".sdk/System/Library/Frameworks",
			"-F/Applications/Xcode.app/Contents/Developer/Platforms/AppleTVSimulator.platform/Developer/SDKs/AppleTVSimulator" ..tvosPlatform .. ".sdk/System/Library/PrivateFrameworks",
		}
		buildoptions {
			"--sysroot=/Applications/Xcode.app/Contents/Developer/Platforms/AppleTVSimulator.platform/Developer/SDKs/AppleTVSimulator" ..tvosPlatform .. ".sdk",
		}

	configuration { "orbis" }
		targetdir (path.join(_buildDir, "orbis/bin"))
		objdir (path.join(_buildDir, "orbis/obj"))
		libdirs { path.join(_libDir, "lib/orbis") }
		includedirs {
			path.join(bxDir, "include/compat/freebsd"),
			"$(SCE_ORBIS_SDK_DIR)/target/include",
			"$(SCE_ORBIS_SDK_DIR)/target/include_common",
		}

	configuration { "rpi" }
		targetdir (path.join(_buildDir, "rpi/bin"))
		objdir (path.join(_buildDir, "rpi/obj"))
		libdirs {
			path.join(_libDir, "lib/rpi"),
			"/opt/vc/lib",
		}
		defines {
			"__VCCOREVER__=0x04000000", -- There is no special prefedined compiler symbol to detect RaspberryPi, faking it.
			"__STDC_VERSION__=199901L",
		}
		buildoptions {
			"-Wunused-value",
			"-Wundef",
		}
		includedirs {
			"/opt/vc/include",
			"/opt/vc/include/interface/vcos/pthreads",
			"/opt/vc/include/interface/vmcs_host/linux",
		}
		links {
			"rt",
			"dl",
		}
		linkoptions {
			"-Wl,--gc-sections",
		}

	configuration { "riscv" }
		targetdir (path.join(_buildDir, "riscv/bin"))
		objdir (path.join(_buildDir, "riscv/obj"))
		defines {
			"__BSD_VISIBLE",
			"__MISC_VISIBLE",
		}
		includedirs {
			"$(FREEDOM_E_SDK)/work/build/riscv-gnu-toolchain/riscv64-unknown-elf/prefix/riscv64-unknown-elf/include",
			path.join(bxDir, "include/compat/riscv"),
		}
		buildoptions {
			"-Wunused-value",
			"-Wundef",
			"--sysroot=$(FREEDOM_E_SDK)/work/build/riscv-gnu-toolchain/riscv64-unknown-elf/prefix/riscv64-unknown-elf",
		}

	configuration {} -- reset configuration

	return true
end

function strip()

	configuration { "android-*", "Release" }
		postbuildcommands {
			"$(SILENT) echo Stripping symbols.",
			"$(SILENT) " .. androidToolchainRoot() .. "/bin/llvm-strip -s \"$(TARGET)\""
		}

	configuration { "linux-* or rpi", "Release" }
		postbuildcommands {
			"$(SILENT) echo Stripping symbols.",
			"$(SILENT) strip -s \"$(TARGET)\""
		}

	configuration { "mingw*", "Release" }
		postbuildcommands {
			"$(SILENT) echo Stripping symbols.",
			"$(SILENT) $(MINGW)/bin/strip -s \"$(TARGET)\""
		}

	configuration { "riscv" }
		postbuildcommands {
			"$(SILENT) echo Stripping symbols.",
			"$(SILENT) $(FREEDOM_E_SDK)/work/build/riscv-gnu-toolchain/riscv64-unknown-elf/prefix/bin/riscv64-unknown-elf-strip -s \"$(TARGET)\""
		}

	configuration {} -- reset configuration
end
