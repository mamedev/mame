--
-- Copyright 2010-2017 Branimir Karadzic. All rights reserved.
-- License: https://github.com/bkaradzic/bx#license-bsd-2-clause
--

local bxDir = path.getabsolute("..")
local naclToolchain = ""

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

function toolchain(_buildDir, _libDir)

	newoption {
		trigger = "gcc",
		value = "GCC",
		description = "Choose GCC flavor",
		allowed = {
			{ "android-arm",     "Android - ARM"              },
			{ "android-mips",    "Android - MIPS"             },
			{ "android-x86",     "Android - x86"              },
			{ "asmjs",           "Emscripten/asm.js"          },
			{ "freebsd",         "FreeBSD"                    },
			{ "linux-gcc",       "Linux (GCC compiler)"       },
			{ "linux-gcc-afl",   "Linux (GCC + AFL fuzzer)"   },
			{ "linux-gcc-6",     "Linux (GCC-6 compiler)"     },
			{ "linux-clang",     "Linux (Clang compiler)"     },
			{ "linux-clang-afl", "Linux (Clang + AFL fuzzer)" },
			{ "linux-mips-gcc",  "Linux (MIPS, GCC compiler)" },
			{ "linux-arm-gcc",   "Linux (ARM, GCC compiler)"  },
			{ "ios-arm",         "iOS - ARM"                  },
			{ "ios-arm64",       "iOS - ARM64"                },
			{ "ios-simulator",   "iOS - Simulator"            },
			{ "tvos-arm64",      "tvOS - ARM64"               },
			{ "tvos-simulator",  "tvOS - Simulator"           },
			{ "mingw-gcc",       "MinGW"                      },
			{ "mingw-clang",     "MinGW (clang compiler)"     },
			{ "nacl",            "Native Client"              },
			{ "nacl-arm",        "Native Client - ARM"        },
			{ "netbsd",          "NetBSD"                     },
			{ "osx",             "OSX"                        },
			{ "pnacl",           "Native Client - PNaCl"      },
			{ "orbis",           "Orbis"                      },
			{ "qnx-arm",         "QNX/Blackberry - ARM"       },
			{ "riscv",           "RISC-V"                     },
			{ "rpi",             "RaspberryPi"                },
		},
	}

	newoption {
		trigger = "vs",
		value = "toolset",
		description = "Choose VS toolset",
		allowed = {
			{ "vs2012-clang",  "Clang 3.6"                       },
			{ "vs2013-clang",  "Clang 3.6"                       },
			{ "vs2015-clang",  "Clang 3.9"                       },
			{ "vs2017-clang",  "Clang with MS CodeGen"           },
			{ "vs2012-xp",     "Visual Studio 2012 targeting XP" },
			{ "vs2013-xp",     "Visual Studio 2013 targeting XP" },
			{ "vs2015-xp",     "Visual Studio 2015 targeting XP" },
			{ "vs2017-xp",     "Visual Studio 2017 targeting XP" },
			{ "winphone8",     "Windows Phone 8.0"               },
			{ "winphone81",    "Windows Phone 8.1"               },
			{ "winstore81",    "Windows Store 8.1"               },
			{ "winstore82",    "Universal Windows App"           },
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
		}
	}

	newoption {
		trigger     = "with-android",
		value       = "#",
		description = "Set Android platform version (default: android-14).",
	}

	newoption {
		trigger     = "with-ios",
		value       = "#",
		description = "Set iOS target version (default: 8.0).",
	}

	newoption {
		trigger     = "with-tvos",
		value       = "#",
		description = "Set tvOS target version (default: 9.0).",
	}

	newoption {
		trigger = "with-windows",
		value = "#",
		description = "Set the Windows target platform version (default: 10.0.10240.0).",
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

	local androidPlatform = "android-24"
	if _OPTIONS["with-android"] then
		androidPlatform = "android-" .. _OPTIONS["with-android"]
	end

	local iosPlatform = ""
	if _OPTIONS["with-ios"] then
		iosPlatform = _OPTIONS["with-ios"]
	end

	local tvosPlatform = ""
	if _OPTIONS["with-tvos"] then
		tvosPlatform = _OPTIONS["with-tvos"]
	end

	local windowsPlatform = "10.0.10240.0"
	if _OPTIONS["with-windows"] then
		windowsPlatform = _OPTIONS["with-windows"]
	end

	local compiler32bit = false
	if _OPTIONS["with-32bit-compiler"] then
		compiler32bit = true
	end

	flags {
		"ExtraWarnings",
	}

	if _ACTION == "gmake" or _ACTION == "ninja" then

		if nil == _OPTIONS["gcc"] then
			print("GCC flavor must be specified!")
			os.exit(1)
		end

		if "android-arm" == _OPTIONS["gcc"] then

			if not os.getenv("ANDROID_NDK_ARM")
			or not os.getenv("ANDROID_NDK_CLANG")
			or not os.getenv("ANDROID_NDK_ROOT") then
				print("Set ANDROID_NDK_CLANG, ANDROID_NDK_ARM, and ANDROID_NDK_ROOT envrionment variables.")
			end

			premake.gcc.cc   = "$(ANDROID_NDK_CLANG)/bin/clang"
			premake.gcc.cxx  = "$(ANDROID_NDK_CLANG)/bin/clang++"
			premake.gcc.ar   = "$(ANDROID_NDK_ARM)/bin/arm-linux-androideabi-ar"

			premake.gcc.llvm = true
			location (path.join(_buildDir, "projects", _ACTION .. "-android-arm"))

		elseif "android-mips" == _OPTIONS["gcc"] then

			if not os.getenv("ANDROID_NDK_MIPS")
			or not os.getenv("ANDROID_NDK_CLANG")
			or not os.getenv("ANDROID_NDK_ROOT") then
				print("Set ANDROID_NDK_CLANG, ANDROID_NDK_ARM, and ANDROID_NDK_ROOT envrionment variables.")
			end

			premake.gcc.cc   = "$(ANDROID_NDK_CLANG)/bin/clang"
			premake.gcc.cxx  = "$(ANDROID_NDK_CLANG)/bin/clang++"
			premake.gcc.llvm = true
			location (path.join(_buildDir, "projects", _ACTION .. "-android-mips"))

		elseif "android-x86" == _OPTIONS["gcc"] then

			if not os.getenv("ANDROID_NDK_X86")
			or not os.getenv("ANDROID_NDK_CLANG")
			or not os.getenv("ANDROID_NDK_ROOT") then
				print("Set ANDROID_NDK_CLANG, ANDROID_NDK_ARM, and ANDROID_NDK_ROOT envrionment variables.")
			end

			premake.gcc.cc   = "$(ANDROID_NDK_CLANG)/bin/clang"
			premake.gcc.cxx  = "$(ANDROID_NDK_CLANG)/bin/clang++"
			premake.gcc.llvm = true
			location (path.join(_buildDir, "projects", _ACTION .. "-android-x86"))

		elseif "asmjs" == _OPTIONS["gcc"] then

			if not os.getenv("EMSCRIPTEN") then
				print("Set EMSCRIPTEN enviroment variable.")
			end

			premake.gcc.cc   = "\"$(EMSCRIPTEN)/emcc\""
			premake.gcc.cxx  = "\"$(EMSCRIPTEN)/em++\""
			premake.gcc.ar   = "\"$(EMSCRIPTEN)/emar\""
			premake.gcc.llvm = true
			location (path.join(_buildDir, "projects", _ACTION .. "-asmjs"))

		elseif "freebsd" == _OPTIONS["gcc"] then
			location (path.join(_buildDir, "projects", _ACTION .. "-freebsd"))

		elseif "ios-arm"   == _OPTIONS["gcc"]
			or "ios-arm64" == _OPTIONS["gcc"] then
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
			location (path.join(_buildDir, "projects", _ACTION .. "-linux"))

		elseif "linux-gcc-afl" == _OPTIONS["gcc"] then
			premake.gcc.cc  = "afl-gcc"
			premake.gcc.cxx = "afl-g++"
			premake.gcc.ar  = "ar"
			location (path.join(_buildDir, "projects", _ACTION .. "-linux"))

		elseif "linux-gcc-6" == _OPTIONS["gcc"] then
			premake.gcc.cc  = "gcc-6"
			premake.gcc.cxx = "g++-6"
			premake.gcc.ar  = "ar"
			location (path.join(_buildDir, "projects", _ACTION .. "-linux"))

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

		elseif "linux-mips-gcc" == _OPTIONS["gcc"] then
			location (path.join(_buildDir, "projects", _ACTION .. "-linux-mips-gcc"))

		elseif "linux-arm-gcc" == _OPTIONS["gcc"] then
			location (path.join(_buildDir, "projects", _ACTION .. "-linux-arm-gcc"))

		elseif "linux-steamlink" == _OPTIONS["gcc"] then
			if not os.getenv("MARVELL_SDK_PATH") then
				print("Set MARVELL_SDK_PATH enviroment variable.")
			end

			premake.gcc.cc  = "$(MARVELL_SDK_PATH)/toolchain/bin/armv7a-cros-linux-gnueabi-gcc"
			premake.gcc.cxx = "$(MARVELL_SDK_PATH)/toolchain/bin/armv7a-cros-linux-gnueabi-g++"
			premake.gcc.ar  = "$(MARVELL_SDK_PATH)/toolchain/bin/armv7a-cros-linux-gnueabi-ar"
			location (path.join(_buildDir, "projects", _ACTION .. "-linux-steamlink"))

		elseif "mingw-gcc" == _OPTIONS["gcc"] then
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
			premake.gcc.cc   = "$(CLANG)/bin/clang"
			premake.gcc.cxx  = "$(CLANG)/bin/clang++"
			premake.gcc.ar   = "$(MINGW)/bin/ar"
--			premake.gcc.ar   = "$(CLANG)/bin/llvm-ar"
--			premake.gcc.llvm = true
			location (path.join(_buildDir, "projects", _ACTION .. "-mingw-clang"))

		elseif "nacl" == _OPTIONS["gcc"] then

			if not os.getenv("NACL_SDK_ROOT") then
				print("Set NACL_SDK_ROOT enviroment variable.")
			end

			naclToolchain = "$(NACL_SDK_ROOT)/toolchain/win_x86_glibc/bin/x86_64-nacl-"
			if os.is("macosx") then
				naclToolchain = "$(NACL_SDK_ROOT)/toolchain/mac_x86_glibc/bin/x86_64-nacl-"
			elseif os.is("linux") then
				naclToolchain = "$(NACL_SDK_ROOT)/toolchain/linux_x86_glibc/bin/x86_64-nacl-"
			end

			premake.gcc.cc  = naclToolchain .. "gcc"
			premake.gcc.cxx = naclToolchain .. "g++"
			premake.gcc.ar  = naclToolchain .. "ar"
			location (path.join(_buildDir, "projects", _ACTION .. "-nacl"))

		elseif "nacl-arm" == _OPTIONS["gcc"] then

			if not os.getenv("NACL_SDK_ROOT") then
				print("Set NACL_SDK_ROOT enviroment variable.")
			end

			naclToolchain = "$(NACL_SDK_ROOT)/toolchain/win_arm_glibc/bin/arm-nacl-"
			if os.is("macosx") then
				naclToolchain = "$(NACL_SDK_ROOT)/toolchain/mac_arm_glibc/bin/arm-nacl-"
			elseif os.is("linux") then
				naclToolchain = "$(NACL_SDK_ROOT)/toolchain/linux_arm_glibc/bin/arm-nacl-"
			end

			premake.gcc.cc  = naclToolchain .. "gcc"
			premake.gcc.cxx = naclToolchain .. "g++"
			premake.gcc.ar  = naclToolchain .. "ar"
			location (path.join(_buildDir, "projects", _ACTION .. "-nacl-arm"))

		elseif "netbsd" == _OPTIONS["gcc"] then
			location (path.join(_buildDir, "projects", _ACTION .. "-netbsd"))

		elseif "osx" == _OPTIONS["gcc"] then

			if os.is("linux") then
				if not os.getenv("OSXCROSS") then
					print("Set OSXCROSS enviroment variable.")
				end

				local osxToolchain = "x86_64-apple-darwin15-"
				premake.gcc.cc  = "$(OSXCROSS)/target/bin/" .. osxToolchain .. "clang"
				premake.gcc.cxx = "$(OSXCROSS)/target/bin/" .. osxToolchain .. "clang++"
				premake.gcc.ar  = "$(OSXCROSS)/target/bin/" .. osxToolchain .. "ar"
			end
			location (path.join(_buildDir, "projects", _ACTION .. "-osx"))

		elseif "pnacl" == _OPTIONS["gcc"] then

			if not os.getenv("NACL_SDK_ROOT") then
				print("Set NACL_SDK_ROOT enviroment variable.")
			end

			naclToolchain = "$(NACL_SDK_ROOT)/toolchain/win_pnacl/bin/pnacl-"
			if os.is("macosx") then
				naclToolchain = "$(NACL_SDK_ROOT)/toolchain/mac_pnacl/bin/pnacl-"
			elseif os.is("linux") then
				naclToolchain = "$(NACL_SDK_ROOT)/toolchain/linux_pnacl/bin/pnacl-"
			end

			premake.gcc.cc  = naclToolchain .. "clang"
			premake.gcc.cxx = naclToolchain .. "clang++"
			premake.gcc.ar  = naclToolchain .. "ar"
			location (path.join(_buildDir, "projects", _ACTION .. "-pnacl"))

		elseif "orbis" == _OPTIONS["gcc"] then

			if not os.getenv("SCE_ORBIS_SDK_DIR") then
				print("Set SCE_ORBIS_SDK_DIR enviroment variable.")
			end

			orbisToolchain = "$(SCE_ORBIS_SDK_DIR)/host_tools/bin/orbis-"

			premake.gcc.cc  = orbisToolchain .. "clang"
			premake.gcc.cxx = orbisToolchain .. "clang++"
			premake.gcc.ar  = orbisToolchain .. "ar"
			location (path.join(_buildDir, "projects", _ACTION .. "-orbis"))

		elseif "qnx-arm" == _OPTIONS["gcc"] then

			if not os.getenv("QNX_HOST") then
				print("Set QNX_HOST enviroment variable.")
			end

			premake.gcc.cc  = "$(QNX_HOST)/usr/bin/arm-unknown-nto-qnx8.0.0eabi-gcc"
			premake.gcc.cxx = "$(QNX_HOST)/usr/bin/arm-unknown-nto-qnx8.0.0eabi-g++"
			premake.gcc.ar  = "$(QNX_HOST)/usr/bin/arm-unknown-nto-qnx8.0.0eabi-ar"
			location (path.join(_buildDir, "projects", _ACTION .. "-qnx-arm"))

		elseif "rpi" == _OPTIONS["gcc"] then
			location (path.join(_buildDir, "projects", _ACTION .. "-rpi"))

		elseif "riscv" == _OPTIONS["gcc"] then
			premake.gcc.cc  = "$(FREEDOM_E_SDK)/toolchain/bin/riscv32-unknown-elf-gcc"
			premake.gcc.cxx = "$(FREEDOM_E_SDK)/toolchain/bin/riscv32-unknown-elf-g++"
			premake.gcc.ar  = "$(FREEDOM_E_SDK)/toolchain/bin/riscv32-unknown-elf-ar"
			location (path.join(_buildDir, "projects", _ACTION .. "-riscv"))

		end
	elseif _ACTION == "vs2012"
		or _ACTION == "vs2013"
		or _ACTION == "vs2015"
		or _ACTION == "vs2017"
		then

		if (_ACTION .. "-clang") == _OPTIONS["vs"] then
			if "vs2017-clang" == _OPTIONS["vs"] then
				premake.vstudio.toolset = "v141_clang_c2"
			elseif "vs2015-clang" == _OPTIONS["vs"] then
				premake.vstudio.toolset = "LLVM-vs2014"
			else
				premake.vstudio.toolset = ("LLVM-" .. _ACTION)
			end
			location (path.join(_buildDir, "projects", _ACTION .. "-clang"))

		elseif "winphone8" == _OPTIONS["vs"] then
			premake.vstudio.toolset = "v110_wp80"
			location (path.join(_buildDir, "projects", _ACTION .. "-winphone8"))

		elseif "winphone81" == _OPTIONS["vs"] then
			premake.vstudio.toolset = "v120_wp81"
			premake.vstudio.storeapp = "8.1"
			platforms { "ARM" }
			location (path.join(_buildDir, "projects", _ACTION .. "-winphone81"))

		elseif "winstore81" == _OPTIONS["vs"] then
			premake.vstudio.toolset = "v120"
			premake.vstudio.storeapp = "8.1"
			platforms { "ARM" }
			location (path.join(_buildDir, "projects", _ACTION .. "-winstore81"))

		elseif "winstore82" == _OPTIONS["vs"] then
			premake.vstudio.toolset = "v140"
			premake.vstudio.storeapp = "8.2"

			local action = premake.action.current()
			action.vstudio.windowsTargetPlatformVersion = windowsPlatform
			action.vstudio.windowsTargetPlatformMinVersion = windowsPlatform

			platforms { "ARM" }
			location (path.join(_buildDir, "projects", _ACTION .. "-winstore82"))

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
				print("Set SCE_ORBIS_SDK_DIR enviroment variable.")
			end

			platforms { "Orbis" }
			location (path.join(_buildDir, "projects", _ACTION .. "-orbis"))

		end

		elseif ("vs2012-xp") == _OPTIONS["vs"] then
			premake.vstudio.toolset = ("v110_xp")
			location (path.join(_buildDir, "projects", _ACTION .. "-xp"))

		elseif "vs2013-xp" == _OPTIONS["vs"] then
			premake.vstudio.toolset = ("v120_xp")
			location (path.join(_buildDir, "projects", _ACTION .. "-xp"))

		elseif "vs2015-xp" == _OPTIONS["vs"] then
			premake.vstudio.toolset = ("v140_xp")
			location (path.join(_buildDir, "projects", _ACTION .. "-xp"))

		elseif "vs2015-xp" == _OPTIONS["vs"] then
			premake.vstudio.toolset = ("v141_xp")
			location (path.join(_buildDir, "projects", _ACTION .. "-xp"))

	elseif _ACTION == "xcode4" then

		if "osx" == _OPTIONS["xcode"] then
			premake.xcode.toolset = "macosx"
			location (path.join(_buildDir, "projects", _ACTION .. "-osx"))

		elseif "ios" == _OPTIONS["xcode"] then
			premake.xcode.toolset = "iphoneos"
			location (path.join(_buildDir, "projects", _ACTION .. "-ios"))

		elseif "tvos" == _OPTIONS["xcode"] then
			premake.xcode.toolset = "appletvos"
			location (path.join(_buildDir, "projects", _ACTION .. "-tvos"))
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

	configuration { "qbs" }
		flags {
			"ExtraWarnings",
		}

	configuration { "vs*", "x32" }
		flags {
			"EnableSSE2",
		}

	configuration { "vs*", "not orbis" }
		includedirs { path.join(bxDir, "include/compat/msvc") }
		defines {
			"WIN32",
			"_WIN32",
			"_HAS_EXCEPTIONS=0",
			"_HAS_ITERATOR_DEBUGGING=0",
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
		}
		linkoptions {
			"/ignore:4221", -- LNK4221: This object file does not define any previously undefined public symbols, so it will not be used by any link operation that consumes this library
		}

	configuration { "vs2008" }
		includedirs { path.join(bxDir, "include/compat/msvc/pre1600") }

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

	configuration { "winphone8* or winstore8*" }
		removeflags {
			"StaticRuntime",
			"NoExceptions",
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
			"-Wunused-value",
			"-fdata-sections",
			"-ffunction-sections",
			"-msse2",
			"-Wunused-value",
			"-Wundef",
		}
		buildoptions_cpp {
			"-std=c++11",
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
			"-isystem$(MINGW)/lib/gcc/x86_64-w64-mingw32/4.8.1/include/c++",
			"-isystem$(MINGW)/lib/gcc/x86_64-w64-mingw32/4.8.1/include/c++/x86_64-w64-mingw32",
			"-isystem$(MINGW)/x86_64-w64-mingw32/include",
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

	configuration { "linux-clang" }

	configuration { "linux-gcc-6" }
		buildoptions {
--			"-fno-omit-frame-pointer",
--			"-fsanitize=address",
--			"-fsanitize=undefined",
--			"-fsanitize=float-divide-by-zero",
--			"-fsanitize=float-cast-overflow",
		}
		links {
--			"asan",
--			"ubsan",
		}

	configuration { "linux-gcc" }
		buildoptions {
			"-mfpmath=sse",
		}

	configuration { "linux-gcc* or linux-clang*" }
		buildoptions {
			"-msse2",
			"-Wunused-value",
			"-Wundef",
		}
		buildoptions_cpp {
			"-std=c++11",
		}
		links {
			"rt",
			"dl",
		}
		linkoptions {
			"-Wl,--gc-sections",
			"-Wl,--as-needed",
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

	configuration { "linux-mips-gcc" }
		targetdir (path.join(_buildDir, "linux32_mips_gcc/bin"))
		objdir (path.join(_buildDir, "linux32_mips_gcc/obj"))
		libdirs { path.join(_libDir, "lib/linux32_mips_gcc") }
		buildoptions {
			"-Wunused-value",
			"-Wundef",
		}
		buildoptions_cpp {
			"-std=c++11",
		}
		links {
			"rt",
			"dl",
		}
		linkoptions {
			"-Wl,--gc-sections",
		}

	configuration { "linux-arm-gcc" }
		targetdir (path.join(_buildDir, "linux32_arm_gcc/bin"))
		objdir (path.join(_buildDir, "linux32_arm_gcc/obj"))
		libdirs { path.join(_libDir, "lib/linux32_arm_gcc") }
		buildoptions {
			"-Wunused-value",
			"-Wundef",
		}
		buildoptions_cpp {
			"-std=c++11",
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
		includedirs {
			"$(ANDROID_NDK_ROOT)/sources/cxx-stl/llvm-libc++/include",
			"$(ANDROID_NDK_ROOT)/sources/android/native_app_glue",
		}
		linkoptions {
			"-nostdlib",
		}
		links {
			"c",
			"dl",
			"m",
			"android",
			"log",
			"c++",
			"gcc",
		}
		buildoptions {
			"-fPIC",
			"-no-canonical-prefixes",
			"-Wa,--noexecstack",
			"-fstack-protector-strong",
			"-ffunction-sections",
			"-Wunused-value",
			"-Wundef",
		}
		buildoptions_cpp {
			"-std=c++11",
		}
		linkoptions {
			"-no-canonical-prefixes",
			"-Wl,--no-undefined",
			"-Wl,-z,noexecstack",
			"-Wl,-z,relro",
			"-Wl,-z,now",
		}

	configuration { "linux-steamlink" }
		targetdir (path.join(_buildDir, "steamlink/bin"))
		objdir (path.join(_buildDir, "steamlink/obj"))
		libdirs { path.join(_libDir, "lib/steamlink") }
		includedirs { path.join(bxDir, "include/compat/linux") }
		defines {
			"__STEAMLINK__=1", -- There is no special prefedined compiler symbol to detect SteamLink, faking it.
		}
		buildoptions {
			"-std=c++11",
			"-Wfatal-errors",
			"-Wunused-value",
			"-Wundef",
			"-pthread",
			"-marm",
			"-mfloat-abi=hard",
			"--sysroot=$(MARVELL_SDK_PATH)/rootfs",
		}
		linkoptions {
			"-static-libgcc",
			"-static-libstdc++",
			"--sysroot=$(MARVELL_SDK_PATH)/rootfs",
		}

	configuration { "android-arm" }
		targetdir (path.join(_buildDir, "android-arm/bin"))
		objdir (path.join(_buildDir, "android-arm/obj"))
		libdirs {
			path.join(_libDir, "lib/android-arm"),
			"$(ANDROID_NDK_ROOT)/sources/cxx-stl/llvm-libc++/libs/armeabi-v7a",
		}
		includedirs {
			"$(ANDROID_NDK_ROOT)/sources/cxx-stl/llvm-libc++/libs/armeabi-v7a/include",
			"$(ANDROID_NDK_ROOT)/sources/cxx-stl/llvm-libc++/include",
		}
		buildoptions {
			"-gcc-toolchain $(ANDROID_NDK_ARM)",
			"--sysroot=" .. path.join("$(ANDROID_NDK_ROOT)/platforms", androidPlatform, "arch-arm"),
			"-target armv7-none-linux-androideabi",
			"-mthumb",
			"-march=armv7-a",
			"-mfloat-abi=softfp",
			"-mfpu=neon",
			"-Wunused-value",
			"-Wundef",
		}
		linkoptions {
			"-gcc-toolchain $(ANDROID_NDK_ARM)",
			"--sysroot=" .. path.join("$(ANDROID_NDK_ROOT)/platforms", androidPlatform, "arch-arm"),
			path.join("$(ANDROID_NDK_ROOT)/platforms", androidPlatform, "arch-arm/usr/lib/crtbegin_so.o"),
			path.join("$(ANDROID_NDK_ROOT)/platforms", androidPlatform, "arch-arm/usr/lib/crtend_so.o"),
			"-target armv7-none-linux-androideabi",
			"-march=armv7-a",
			"-Wl,--fix-cortex-a8",
		}

	configuration { "android-mips" }
		targetdir (path.join(_buildDir, "android-mips/bin"))
		objdir (path.join(_buildDir, "android-mips/obj"))
		libdirs {
			path.join(_libDir, "lib/android-mips"),
			"$(ANDROID_NDK_ROOT)/sources/cxx-stl/llvm-libc++/libs/mips",
		}
		includedirs {
			"$(ANDROID_NDK_ROOT)/sources/cxx-stl/llvm-libc++/libs/mips/include",
		}
		buildoptions {
			"-gcc-toolchain $(ANDROID_NDK_MIPS)",
			"--sysroot=" .. path.join("$(ANDROID_NDK_ROOT)/platforms", androidPlatform, "arch-mips"),
			"-target mipsel-none-linux-android",
			"-mips32",
			"-Wunused-value",
			"-Wundef",
		}
		linkoptions {
			"-gcc-toolchain $(ANDROID_NDK_MIPS)",
			"--sysroot=" .. path.join("$(ANDROID_NDK_ROOT)/platforms", androidPlatform, "arch-mips"),
			path.join("$(ANDROID_NDK_ROOT)/platforms", androidPlatform, "arch-mips/usr/lib/crtbegin_so.o"),
			path.join("$(ANDROID_NDK_ROOT)/platforms", androidPlatform, "arch-mips/usr/lib/crtend_so.o"),
			"-target mipsel-none-linux-android",
			"-mips32",
		}

	configuration { "android-x86" }
		targetdir (path.join(_buildDir, "android-x86/bin"))
		objdir (path.join(_buildDir, "android-x86/obj"))
		libdirs {
			path.join(_libDir, "lib/android-x86"),
			"$(ANDROID_NDK_ROOT)/sources/cxx-stl/llvm-libc++/libs/x86",
		}
		includedirs {
			"$(ANDROID_NDK_ROOT)/sources/cxx-stl/llvm-libc++/libs/x86/include",
		}
		buildoptions {
			"-gcc-toolchain $(ANDROID_NDK_X86)",
			"--sysroot=" .. path.join("$(ANDROID_NDK_ROOT)/platforms", androidPlatform, "arch-x86"),
			"-target i686-none-linux-android",
			"-march=i686",
			"-mtune=atom",
			"-mstackrealign",
			"-msse3",
			"-mfpmath=sse",
			"-Wunused-value",
			"-Wundef",
		}
		linkoptions {
			"-gcc-toolchain $(ANDROID_NDK_X86)",
			"--sysroot=" .. path.join("$(ANDROID_NDK_ROOT)/platforms", androidPlatform, "arch-x86"),
			path.join("$(ANDROID_NDK_ROOT)/platforms", androidPlatform, "arch-x86/usr/lib/crtbegin_so.o"),
			path.join("$(ANDROID_NDK_ROOT)/platforms", androidPlatform, "arch-x86/usr/lib/crtend_so.o"),
			"-target i686-none-linux-android",
		}

	configuration { "asmjs" }
		targetdir (path.join(_buildDir, "asmjs/bin"))
		objdir (path.join(_buildDir, "asmjs/obj"))
		libdirs { path.join(_libDir, "lib/asmjs") }
		buildoptions {
			"-i\"system$(EMSCRIPTEN)/system/include\"",
			"-i\"system$(EMSCRIPTEN)/system/include/libc\"",
			"-Wunused-value",
			"-Wundef",
		}

	configuration { "freebsd" }
		targetdir (path.join(_buildDir, "freebsd/bin"))
		objdir (path.join(_buildDir, "freebsd/obj"))
		libdirs { path.join(_libDir, "lib/freebsd") }
		includedirs {
			path.join(bxDir, "include/compat/freebsd"),
		}

	configuration { "nacl or nacl-arm or pnacl" }
		buildoptions {
			"-U__STRICT_ANSI__", -- strcasecmp, setenv, unsetenv,...
			"-fno-stack-protector",
			"-fdiagnostics-show-option",
			"-fdata-sections",
			"-ffunction-sections",
			"-Wunused-value",
			"-Wundef",
		}
		buildoptions_cpp {
			"-std=c++11",
		}
		includedirs {
			"$(NACL_SDK_ROOT)/include",
			path.join(bxDir, "include/compat/nacl"),
		}

	configuration { "nacl" }
		buildoptions {
			"-pthread",
			"-mfpmath=sse", -- force SSE to get 32-bit and 64-bit builds deterministic.
			"-msse2",
		}
		linkoptions {
			"-Wl,--gc-sections",
		}

	configuration { "x32", "nacl" }
		targetdir (path.join(_buildDir, "nacl-x86/bin"))
		objdir (path.join(_buildDir, "nacl-x86/obj"))
		libdirs { path.join(_libDir, "lib/nacl-x86") }
		linkoptions { "-melf32_nacl" }

	configuration { "x32", "nacl", "Debug" }
		libdirs { "$(NACL_SDK_ROOT)/lib/glibc_x86_32/Debug" }

	configuration { "x32", "nacl", "Release" }
		libdirs { "$(NACL_SDK_ROOT)/lib/glibc_x86_32/Release" }

	configuration { "x64", "nacl" }
		targetdir (path.join(_buildDir, "nacl-x64/bin"))
		objdir (path.join(_buildDir, "nacl-x64/obj"))
		libdirs { path.join(_libDir, "lib/nacl-x64") }
		linkoptions { "-melf64_nacl" }

	configuration { "x64", "nacl", "Debug" }
		libdirs { "$(NACL_SDK_ROOT)/lib/glibc_x86_64/Debug" }

	configuration { "x64", "nacl", "Release" }
		libdirs { "$(NACL_SDK_ROOT)/lib/glibc_x86_64/Release" }

	configuration { "nacl-arm" }
		buildoptions {
			"-Wno-psabi", -- note: the mangling of 'va_list' has changed in GCC 4.4.0
		}
		targetdir (path.join(_buildDir, "nacl-arm/bin"))
		objdir (path.join(_buildDir, "nacl-arm/obj"))
		libdirs { path.join(_libDir, "lib/nacl-arm") }

	configuration { "nacl-arm", "Debug" }
		libdirs { "$(NACL_SDK_ROOT)/lib/glibc_arm/Debug" }

	configuration { "nacl-arm", "Release" }
		libdirs { "$(NACL_SDK_ROOT)/lib/glibc_arm/Release" }

	configuration { "pnacl" }
		targetdir (path.join(_buildDir, "pnacl/bin"))
		objdir (path.join(_buildDir, "pnacl/obj"))
		libdirs { path.join(_libDir, "lib/pnacl") }

	configuration { "pnacl", "Debug" }
		libdirs { "$(NACL_SDK_ROOT)/lib/pnacl/Debug" }

	configuration { "pnacl", "Release" }
		libdirs { "$(NACL_SDK_ROOT)/lib/pnacl/Release" }

	configuration { "xbox360" }
		targetdir (path.join(_buildDir, "xbox360/bin"))
		objdir (path.join(_buildDir, "xbox360/obj"))
		includedirs { path.join(bxDir, "include/compat/msvc") }
		libdirs { path.join(_libDir, "lib/xbox360") }
		defines {
			"NOMINMAX",
		}

	configuration { "durango" }
		targetdir (path.join(_buildDir, "durango/bin"))
		objdir (path.join(_buildDir, "durango/obj"))
		includedirs { path.join(bxDir, "include/compat/msvc") }
		libdirs { path.join(_libDir, "lib/durango") }
		removeflags { "StaticRuntime" }
		defines {
			"NOMINMAX",
		}

	configuration { "netbsd" }
		targetdir (path.join(_buildDir, "netbsd/bin"))
		objdir (path.join(_buildDir, "netbsd/obj"))
		libdirs { path.join(_libDir, "lib/netbsd") }
		includedirs {
			path.join(bxDir, "include/compat/freebsd"),
		}

	configuration { "osx", "x32" }
		targetdir (path.join(_buildDir, "osx32_clang/bin"))
		objdir (path.join(_buildDir, "osx32_clang/obj"))
		--libdirs { path.join(_libDir, "lib/osx32_clang") }
		buildoptions {
			"-m32",
		}

	configuration { "osx", "x64" }
		targetdir (path.join(_buildDir, "osx64_clang/bin"))
		objdir (path.join(_buildDir, "osx64_clang/obj"))
		--libdirs { path.join(_libDir, "lib/osx64_clang") }
		buildoptions {
			"-m64",
		}

	configuration { "osx", "Universal" }
		targetdir (path.join(_buildDir, "osx_universal/bin"))
		objdir (path.join(_buildDir, "osx_universal/bin"))

	configuration { "osx" }
		buildoptions_cpp {
			"-std=c++11",
		}
		buildoptions {
			"-Wfatal-errors",
			"-msse2",
			"-Wunused-value",
			"-Wundef",
		}
		includedirs { path.join(bxDir, "include/compat/osx") }

	configuration { "ios*" }
		linkoptions {
			"-lc++",
		}
		buildoptions_cpp {
			"-std=c++11",
		}
		buildoptions {
			"-Wfatal-errors",
			"-Wunused-value",
			"-Wundef",
		}
		includedirs { path.join(bxDir, "include/compat/ios") }

	configuration { "xcode4", "ios*" }
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
			"-miphoneos-version-min=7.0",
			"--sysroot=/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS" ..iosPlatform .. ".sdk",
			"-L/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS" ..iosPlatform .. ".sdk/usr/lib/system",
			"-F/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS" ..iosPlatform .. ".sdk/System/Library/Frameworks",
			"-F/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS" ..iosPlatform .. ".sdk/System/Library/PrivateFrameworks",
		}
		buildoptions {
			"-miphoneos-version-min=7.0",
			"--sysroot=/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS" ..iosPlatform .. ".sdk",
		}

	configuration { "ios-simulator" }
		targetdir (path.join(_buildDir, "ios-simulator/bin"))
		objdir (path.join(_buildDir, "ios-simulator/obj"))
		libdirs { path.join(_libDir, "lib/ios-simulator") }
		linkoptions {
			"-mios-simulator-version-min=7.0",
			"-arch i386",
			"--sysroot=/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator" ..iosPlatform .. ".sdk",
			"-L/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator" ..iosPlatform .. ".sdk/usr/lib/system",
			"-F/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator" ..iosPlatform .. ".sdk/System/Library/Frameworks",
			"-F/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator" ..iosPlatform .. ".sdk/System/Library/PrivateFrameworks",
		}
		buildoptions {
			"-mios-simulator-version-min=7.0",
			"-arch i386",
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

	configuration { "xcode4", "tvos*" }
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
			"-mtvos-simulator-version-min=9.0",
			"-arch i386",
			"--sysroot=/Applications/Xcode.app/Contents/Developer/Platforms/AppleTVSimulator.platform/Developer/SDKs/AppleTVSimulator" ..tvosPlatform .. ".sdk",
			"-L/Applications/Xcode.app/Contents/Developer/Platforms/AppleTVSimulator.platform/Developer/SDKs/AppleTVSimulator" ..tvosPlatform .. ".sdk/usr/lib/system",
			"-F/Applications/Xcode.app/Contents/Developer/Platforms/AppleTVSimulator.platform/Developer/SDKs/AppleTVSimulator" ..tvosPlatform .. ".sdk/System/Library/Frameworks",
			"-F/Applications/Xcode.app/Contents/Developer/Platforms/AppleTVSimulator.platform/Developer/SDKs/AppleTVSimulator" ..tvosPlatform .. ".sdk/System/Library/PrivateFrameworks",
		}
		buildoptions {
			"-mtvos-simulator-version-min=9.0",
			"-arch i386",
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
		buildoptions {
		}
		buildoptions_cpp {
			"-std=c++11",
		}
		linkoptions {
		}

	configuration { "qnx-arm" }
		targetdir (path.join(_buildDir, "qnx-arm/bin"))
		objdir (path.join(_buildDir, "qnx-arm/obj"))
		libdirs { path.join(_libDir, "lib/qnx-arm") }
--		includedirs { path.join(bxDir, "include/compat/qnx") }
		buildoptions {
			"-Wno-psabi", -- note: the mangling of 'va_list' has changed in GCC 4.4.0
			"-Wunused-value",
			"-Wundef",
		}
		buildoptions_cpp {
			"-std=c++11",
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
		buildoptions_cpp {
			"-std=c++11",
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
			"$(FREEDOM_E_SDK)/toolchain/riscv32-unknown-elf/include",
			path.join(bxDir, "include/compat/riscv"),
		}
		buildoptions {
			"-Wunused-value",
			"-Wundef",
			"--sysroot=$(FREEDOM_E_SDK)/toolchain/riscv32-unknown-elf",
		}
		buildoptions_cpp {
			"-std=c++11",
		}

	configuration {} -- reset configuration

	return true
end

function strip()

	configuration { "android-arm", "Release" }
		postbuildcommands {
			"$(SILENT) echo Stripping symbols.",
			"$(SILENT) $(ANDROID_NDK_ARM)/bin/arm-linux-androideabi-strip -s \"$(TARGET)\""
		}

	configuration { "android-mips", "Release" }
		postbuildcommands {
			"$(SILENT) echo Stripping symbols.",
			"$(SILENT) $(ANDROID_NDK_MIPS)/bin/mipsel-linux-android-strip -s \"$(TARGET)\""
		}

	configuration { "android-x86", "Release" }
		postbuildcommands {
			"$(SILENT) echo Stripping symbols.",
			"$(SILENT) $(ANDROID_NDK_X86)/bin/i686-linux-android-strip -s \"$(TARGET)\""
		}

	configuration { "linux-steamlink", "Release" }
		postbuildcommands {
			"$(SILENT) echo Stripping symbols.",
			"$(SILENT) $(MARVELL_SDK_PATH)/toolchain/bin/armv7a-cros-linux-gnueabi-strip -s \"$(TARGET)\""
		}

	configuration { "linux-* or rpi", "not linux-steamlink", "Release" }
		postbuildcommands {
			"$(SILENT) echo Stripping symbols.",
			"$(SILENT) strip -s \"$(TARGET)\""
		}

	configuration { "mingw*", "Release" }
		postbuildcommands {
			"$(SILENT) echo Stripping symbols.",
			"$(SILENT) $(MINGW)/bin/strip -s \"$(TARGET)\""
		}

	configuration { "pnacl" }
		postbuildcommands {
			"$(SILENT) echo Running pnacl-finalize.",
			"$(SILENT) " .. naclToolchain .. "finalize \"$(TARGET)\""
		}

	configuration { "*nacl*", "Release" }
		postbuildcommands {
			"$(SILENT) echo Stripping symbols.",
			"$(SILENT) " .. naclToolchain .. "strip -s \"$(TARGET)\""
		}

	configuration { "asmjs" }
		postbuildcommands {
			"$(SILENT) echo Running asmjs finalize.",
			"$(SILENT) \"$(EMSCRIPTEN)/emcc\" -O2 "
--				.. "-s EMTERPRETIFY=1 "
--				.. "-s EMTERPRETIFY_ASYNC=1 "
				.. "-s TOTAL_MEMORY=268435456 "
--				.. "-s ALLOW_MEMORY_GROWTH=1 "
--				.. "-s USE_WEBGL2=1 "
				.. "--memory-init-file 1 "
				.. "\"$(TARGET)\" -o \"$(TARGET)\".html "
--				.. "--preload-file ../../../examples/runtime@/"
		}

	configuration { "riscv" }
		postbuildcommands {
			"$(SILENT) echo Stripping symbols.",
			"$(SILENT) $(FREEDOM_E_SDK)/toolchain/bin/riscv32-unknown-elf-strip -s \"$(TARGET)\""
		}

	configuration {} -- reset configuration
end
