--
-- Copyright 2010-2015 Branimir Karadzic. All rights reserved.
-- License: https://github.com/bkaradzic/bx#license-bsd-2-clause
--

local naclToolchain = ""
local toolchainPrefix = ""

if _OPTIONS['TOOLCHAIN'] then
	toolchainPrefix = _OPTIONS["TOOLCHAIN"]
end

newoption {
	trigger = "gcc",
	value = "GCC",
	description = "Choose GCC flavor",
	allowed = {
		{ "android-arm",   "Android - ARM"          },
		{ "android-arm64", "Android - ARM64"        },
		{ "android-mips",  "Android - MIPS"         },
		{ "android-mips64","Android - MIPS64"       },
		{ "android-x86",   "Android - x86"          },
		{ "android-x64",   "Android - x64"          },
		{ "asmjs",         "Emscripten/asm.js"      },
		{ "freebsd",       "FreeBSD"                },
		{ "freebsd-clang", "FreeBSD (clang compiler)"},
		{ "linux-gcc",     "Linux (GCC compiler)"   },
		{ "linux-clang",   "Linux (Clang compiler)" },
		{ "ios-arm",       "iOS - ARM"              },
		{ "ios-simulator", "iOS - Simulator"        },
		{ "mingw32-gcc",   "MinGW32"                },
		{ "mingw64-gcc",   "MinGW64"                },
		{ "mingw-clang",   "MinGW (clang compiler)" },
		{ "netbsd",        "NetBSD"                },
		{ "netbsd-clang",  "NetBSD (clang compiler)"},
		{ "openbsd",       "OpenBSD"                },
		{ "osx",           "OSX (GCC compiler)"     },
		{ "osx-clang",     "OSX (Clang compiler)"   },
		{ "pnacl",         "Native Client - PNaCl"  },
		{ "rpi",           "RaspberryPi"            },
		{ "solaris",       "Solaris"                },
		{ "steamlink",     "Steam Link"             },
		{ "ci20",          "Creator-Ci20"           },
	},
}

newoption {
	trigger = "vs",
	value = "toolset",
	description = "Choose VS toolset",
	allowed = {
		{ "intel-14",      "Intel C++ Compiler XE 14.0" },
		{ "intel-15",      "Intel C++ Compiler XE 15.0" },
		{ "vs2015-clang",  "Clang 3.6"         },
		{ "vs2015-xp",     "Visual Studio 2015 targeting XP" },
		{ "vs2017-clang",  "Clang 3.6"         },
		{ "vs2017-xp",     "Visual Studio 2017 targeting XP" },
		{ "winphone8",     "Windows Phone 8.0" },
		{ "winphone81",    "Windows Phone 8.1" },
		{ "winstore81",    "Windows Store 8.1" },
		{ "winstore82",    "Universal Windows App" }
	},
}

newoption {
	trigger = "xcode",
	value = "xcode_target",
	description = "Choose XCode target",
	allowed = {
		{ "osx", "OSX" },
		{ "ios", "iOS" },
	}
}

newoption {
	trigger = "with-android",
	value   = "#",
	description = "Set Android platform version (default: android-21).",
}

newoption {
	trigger = "with-ios",
	value   = "#",
	description = "Set iOS target version (default: 8.0).",
}

newoption {
	trigger = "with-windows",
	value = "#",
	description = "Set the Windows target platform version (default: 10.0.10240.0).",
}

function toolchain(_buildDir, _subDir)

	location (_buildDir .. "projects/" .. _subDir .. "/".. _ACTION)

	local androidPlatform = "android-24"
	if _OPTIONS["with-android"] then
		androidPlatform = "android-" .. _OPTIONS["with-android"]
	elseif _OPTIONS["PLATFORM"]:find("64", -2) then
		androidPlatform = "android-24"
	end

	local iosPlatform = ""
	if _OPTIONS["with-ios"] then
		iosPlatform = _OPTIONS["with-ios"]
	end

	local windowsPlatform = "10.0.10240.0"
	if _OPTIONS["with-windows"] then
		windowsPlatform = _OPTIONS["with-windows"]
	end

	if _ACTION == "gmake" or _ACTION == "ninja" then

		if nil == _OPTIONS["gcc"] or nil == _OPTIONS["gcc_version"] then
			print("GCC flavor and version must be specified!")
			os.exit(1)
		end

		if string.find(_OPTIONS["gcc"], "android") then
			-- 64-bit android platform requires >= 21
			if _OPTIONS["PLATFORM"]:find("64", -2) and tonumber(androidPlatform:sub(9)) < 21 then
				error("64-bit android requires platform 21 or higher")
			end
			if not os.getenv("ANDROID_NDK_ROOT") then
				print("Set ANDROID_NDK_ROOT environment variable.")
			end
			if not os.getenv("ANDROID_NDK_LLVM") then
				print("Set ANDROID_NDK_LLVM envrionment variable.")
			end
			platform_ndk_env = "ANDROID_NDK_" .. _OPTIONS["PLATFORM"]:upper()
			if not os.getenv(platform_ndk_env) then
				print("Set " .. platform_ndk_env .. " environment variable.")
			end

			local platformToolchainMap = {
				['arm']    = "arm-linux-androideabi",
				['arm64']  = "aarch64-linux-android",
				['mips64'] = "mips64el-linux-android",
				['mips']   = "mipsel-linux-android",
				['x86']    = "i686-linux-android",
				['x64']    = "x86_64-linux-android",
			}

			toolchainPrefix = os.getenv(platform_ndk_env) .. "/bin/" .. platformToolchainMap[_OPTIONS["PLATFORM"]] .. "-"

			premake.gcc.cc  = "$(ANDROID_NDK_LLVM)/bin/clang"
			premake.gcc.cxx = "$(ANDROID_NDK_LLVM)/bin/clang++"
			premake.gcc.ar  = toolchainPrefix .. "ar"
			premake.gcc.llvm = true

			location (_buildDir .. "projects/" .. _subDir .. "/".. _ACTION .. "-android-" .. _OPTIONS["PLATFORM"])
		end

		if "asmjs" == _OPTIONS["gcc"] then

			if not os.getenv("EMSCRIPTEN") then
				print("Set EMSCRIPTEN enviroment variables.")
			end

			premake.gcc.cc   = "$(EMSCRIPTEN)/emcc"
			premake.gcc.cxx  = "$(EMSCRIPTEN)/em++"
			premake.gcc.ar   = "$(EMSCRIPTEN)/emar"
			premake.gcc.llvm = true
			location (_buildDir .. "projects/" .. _subDir .. "/".. _ACTION .. "-asmjs")
		end

		if "freebsd" == _OPTIONS["gcc"] then
			location (_buildDir .. "projects/" .. _subDir .. "/".. _ACTION .. "-freebsd")
		end

		if "freebsd-clang" == _OPTIONS["gcc"] then
			location (_buildDir .. "projects/" .. _subDir .. "/".. _ACTION .. "-freebsd-clang")
		end

		if "netbsd" == _OPTIONS["gcc"] then
			location (_buildDir .. "projects/" .. _subDir .. "/".. _ACTION .. "-netbsd")
		end

		if "netbsd-clang" == _OPTIONS["gcc"] then
			location (_buildDir .. "projects/" .. _subDir .. "/".. _ACTION .. "-netbsd-clang")
		end

		if "openbsd" == _OPTIONS["gcc"] then
			location (_buildDir .. "projects/" .. _subDir .. "/".. _ACTION .. "-openbsd")
		end

		if "ios-arm" == _OPTIONS["gcc"] then
			premake.gcc.cc  = "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang"
			premake.gcc.cxx = "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang++"
			premake.gcc.ar  = "ar"
			location (_buildDir .. "projects/" .. _subDir .. "/".. _ACTION .. "-ios-arm")
		end

		if "ios-simulator" == _OPTIONS["gcc"] then
			premake.gcc.cc  = "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang"
			premake.gcc.cxx = "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang++"
			premake.gcc.ar  = "ar"
			location (_buildDir .. "projects/" .. _subDir .. "/".. _ACTION .. "-ios-simulator")
		end

		if "linux-gcc" == _OPTIONS["gcc"] then
			-- Force gcc-4.2 on ubuntu-intrepid
			if _OPTIONS["distro"]=="ubuntu-intrepid" then
				premake.gcc.cc   = "@gcc -V 4.2"
				premake.gcc.cxx  = "@g++-4.2"
			end
			premake.gcc.ar  = "ar"
			location (_buildDir .. "projects/" .. _subDir .. "/".. _ACTION .. "-linux")
		end

		if "solaris" == _OPTIONS["gcc"] then
			location (_buildDir .. "projects/" .. _subDir .. "/".. _ACTION .. "-solaris")
		end


		if "linux-clang" == _OPTIONS["gcc"] then
			premake.gcc.cc  = "clang"
			premake.gcc.cxx = "clang++"
			premake.gcc.ar  = "ar"
			location (_buildDir .. "projects/" .. _subDir .. "/".. _ACTION .. "-linux-clang")
		end

		if "steamlink" == _OPTIONS["gcc"] then
			if not os.getenv("MARVELL_SDK_PATH") then
				print("Set MARVELL_SDK_PATH envrionment variable.")
			end
			premake.gcc.cc  = "$(MARVELL_SDK_PATH)/toolchain/bin/armv7a-cros-linux-gnueabi-gcc"
			premake.gcc.cxx = "$(MARVELL_SDK_PATH)/toolchain/bin/armv7a-cros-linux-gnueabi-g++"
			premake.gcc.ar  = "$(MARVELL_SDK_PATH)/toolchain/bin/armv7a-cros-linux-gnueabi-ar"
			location (_buildDir .. "projects/" .. _subDir .. "/".. _ACTION .. "-steamlink")
		end

		if "rpi" == _OPTIONS["gcc"] then
			if not os.getenv("RASPBERRY_SDK_PATH") then
				print("Set RASPBERRY_SDK_PATH envrionment variable.")
			end
			premake.gcc.cc  = "$(RASPBERRY_SDK_PATH)/bin/arm-linux-gnueabihf-gcc"
			premake.gcc.cxx = "$(RASPBERRY_SDK_PATH)/bin/arm-linux-gnueabihf-g++"
			premake.gcc.ar  = "$(RASPBERRY_SDK_PATH)/bin/arm-linux-gnueabihf-ar"
			location (_buildDir .. "projects/" .. _subDir .. "/".. _ACTION .. "-rpi")
		end

		if "ci20" == _OPTIONS["gcc"] then
			if not os.getenv("MIPS_LINUXGNU_ROOT") then
				print("Set MIPS_LINUXGNU_ROOT envrionment variable.")
			end
			premake.gcc.cc  = "$(MIPS_LINUXGNU_ROOT)/bin/mips-mti-linux-gnu-gcc"
			premake.gcc.cxx = "$(MIPS_LINUXGNU_ROOT)/bin/mips-mti-linux-gnu-g++"
			premake.gcc.ar  = "$(MIPS_LINUXGNU_ROOT)/bin/mips-mti-linux-gnu-ar"
			location (_buildDir .. "projects/" .. _subDir .. "/".. _ACTION .. "-ci20")
		end

		if "mingw32-gcc" == _OPTIONS["gcc"] then
			if not os.getenv("MINGW32") then
				print("Set MINGW32 envrionment variable.")
			end
			if toolchainPrefix == nil or toolchainPrefix == "" then
				toolchainPrefix = "$(MINGW32)/bin/i686-w64-mingw32-"
			end
			premake.gcc.cc  = toolchainPrefix .. "gcc"
			premake.gcc.cxx = toolchainPrefix .. "g++"
-- work around GCC 4.9.2 not having proper linker for LTO=1 usage
			local version_4_ar = str_to_version(_OPTIONS["gcc_version"])
			if (version_4_ar < 50000) then
				premake.gcc.ar  = toolchainPrefix .. "ar"
			end
			if (version_4_ar >= 50000) then
				premake.gcc.ar  = toolchainPrefix .. "gcc-ar"
			end
			location (_buildDir .. "projects/" .. _subDir .. "/".. _ACTION .. "-mingw32-gcc")
		end

		if "mingw64-gcc" == _OPTIONS["gcc"] then
			if not os.getenv("MINGW64") then
				print("Set MINGW64 envrionment variable.")
			end
			if toolchainPrefix == nil or toolchainPrefix == "" then
				toolchainPrefix = "$(MINGW64)/bin/x86_64-w64-mingw32-"
			end
			premake.gcc.cc  = toolchainPrefix .. "gcc"
			premake.gcc.cxx = toolchainPrefix .. "g++"
-- work around GCC 4.9.2 not having proper linker for LTO=1 usage
			local version_4_ar = str_to_version(_OPTIONS["gcc_version"])
			if (version_4_ar < 50000) then
				premake.gcc.ar  = toolchainPrefix .. "ar"
			end
			if (version_4_ar >= 50000) then
				premake.gcc.ar  = toolchainPrefix .. "gcc-ar"
			end
			location (_buildDir .. "projects/" .. _subDir .. "/".. _ACTION .. "-mingw64-gcc")
		end

		if "mingw-clang" == _OPTIONS["gcc"] then
			premake.gcc.cc   = "clang"
			premake.gcc.cxx  = "clang++"
			premake.gcc.ar   = "llvm-ar"
			premake.gcc.llvm = true
			location (_buildDir .. "projects/" .. _subDir .. "/".. _ACTION .. "-mingw-clang")
		end

		if "osx" == _OPTIONS["gcc"] then
			if os.is("linux") then
				premake.gcc.cc  = toolchainPrefix .. "clang"
				premake.gcc.cxx = toolchainPrefix .. "clang++"
				premake.gcc.ar  = toolchainPrefix .. "ar"
			end
			location (_buildDir .. "projects/" .. _subDir .. "/".. _ACTION .. "-osx")
		end

		if "osx-clang" == _OPTIONS["gcc"] then
			premake.gcc.cc  = toolchainPrefix .. "clang"
			premake.gcc.cxx = toolchainPrefix .. "clang++"
			premake.gcc.ar  = toolchainPrefix .. "ar"
			location (_buildDir .. "projects/" .. _subDir .. "/".. _ACTION .. "-osx-clang")
		end

		if "pnacl" == _OPTIONS["gcc"] then

			if not os.getenv("NACL_SDK_ROOT") then
				print("Set NACL_SDK_ROOT enviroment variables.")
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
			location (_buildDir .. "projects/" .. _subDir .. "/".. _ACTION .. "-pnacl")
		end

		if "rpi" == _OPTIONS["gcc"] then
			location (_buildDir .. "projects/" .. _subDir .. "/".. _ACTION .. "-rpi")
		end

		if "ci20" == _OPTIONS["gcc"] then
			location (_buildDir .. "projects/" .. _subDir .. "/".. _ACTION .. "-ci20")
		end
	elseif _ACTION == "vs2015" or _ACTION == "vs2015-fastbuild" then

		if (_ACTION .. "-clang") == _OPTIONS["vs"] then
			premake.vstudio.toolset = ("LLVM-" .. _ACTION)
			location (_buildDir .. "projects/" .. _subDir .. "/".. _ACTION .. "-clang")
		end

		if "winphone8" == _OPTIONS["vs"] then
			premake.vstudio.toolset = "v110_wp80"
			location (_buildDir .. "projects/" .. _subDir .. "/".. _ACTION .. "-winphone8")
		end

		if "winphone81" == _OPTIONS["vs"] then
			premake.vstudio.toolset = "v120_wp81"
			platforms { "ARM" }
			location (_buildDir .. "projects/" .. _subDir .. "/".. _ACTION .. "-winphone81")
		end

		if "winstore81" == _OPTIONS["vs"] then
			premake.vstudio.toolset = "v120"
			premake.vstudio.storeapp = "8.1"
			platforms { "ARM" }
			location (_buildDir .. "projects/" .. _subDir .. "/".. _ACTION .. "-winstore81")
		end

		if "winstore82" == _OPTIONS["vs"] then
			premake.vstudio.toolset = "v140"
			premake.vstudio.storeapp = "8.2"

			-- If needed, depending on GENie version, enable file-level configuration
			if enablefilelevelconfig ~= nil then
				enablefilelevelconfig()
			end

			local action = premake.action.current()
			action.vstudio.windowsTargetPlatformVersion = windowsPlatform

			platforms { "ARM" }
			location (_buildDir .. "projects/" .. _subDir .. "/".. _ACTION .. "-winstore82")
		end

		if "intel-14" == _OPTIONS["vs"] then
			premake.vstudio.toolset = "Intel C++ Compiler XE 14.0"
			location (_buildDir .. "projects/" .. _subDir .. "/".. _ACTION .. "-intel")
		end

		if "intel-15" == _OPTIONS["vs"] then
			premake.vstudio.toolset = "Intel C++ Compiler XE 15.0"
			location (_buildDir .. "projects/" .. _subDir .. "/".. _ACTION .. "-intel")
		end

		if ("vs2015-xp") == _OPTIONS["vs"] then
			premake.vstudio.toolset = ("v140_xp")
			location (_buildDir .. "projects/" .. _subDir .. "/".. _ACTION .. "-xp")
		end
	elseif _ACTION == "vs2017" or _ACTION == "vs2017-fastbuild" then

		if (_ACTION .. "-clang") == _OPTIONS["vs"] then
			premake.vstudio.toolset = ("LLVM-" .. _ACTION)
			location (_buildDir .. "projects/" .. _subDir .. "/".. _ACTION .. "-clang")
		end

		if "winphone8" == _OPTIONS["vs"] then
			premake.vstudio.toolset = "v110_wp80"
			location (_buildDir .. "projects/" .. _subDir .. "/".. _ACTION .. "-winphone8")
		end

		if "winphone81" == _OPTIONS["vs"] then
			premake.vstudio.toolset = "v120_wp81"
			platforms { "ARM" }
			location (_buildDir .. "projects/" .. _subDir .. "/".. _ACTION .. "-winphone81")
		end

		if "winstore81" == _OPTIONS["vs"] then
			premake.vstudio.toolset = "v120"
			premake.vstudio.storeapp = "8.1"
			platforms { "ARM" }
			location (_buildDir .. "projects/" .. _subDir .. "/".. _ACTION .. "-winstore81")
		end

		if "winstore82" == _OPTIONS["vs"] then
			premake.vstudio.toolset = "v141"
			premake.vstudio.storeapp = "8.2"

			-- If needed, depending on GENie version, enable file-level configuration
			if enablefilelevelconfig ~= nil then
				enablefilelevelconfig()
			end

			local action = premake.action.current()
			action.vstudio.windowsTargetPlatformVersion = windowsPlatform

			platforms { "ARM" }
			location (_buildDir .. "projects/" .. _subDir .. "/".. _ACTION .. "-winstore82")
		end

		if "intel-14" == _OPTIONS["vs"] then
			premake.vstudio.toolset = "Intel C++ Compiler XE 14.0"
			location (_buildDir .. "projects/" .. _subDir .. "/".. _ACTION .. "-intel")
		end

		if "intel-15" == _OPTIONS["vs"] then
			premake.vstudio.toolset = "Intel C++ Compiler XE 15.0"
			location (_buildDir .. "projects/" .. _subDir .. "/".. _ACTION .. "-intel")
		end

		if ("vs2017-xp") == _OPTIONS["vs"] then
			premake.vstudio.toolset = ("v141_xp")
			location (_buildDir .. "projects/" .. _subDir .. "/".. _ACTION .. "-xp")
		end
	elseif _ACTION == "xcode4" then


		if "osx" == _OPTIONS["xcode"] then
			premake.xcode.toolset = "macosx"
			location (path.join(_buildDir, "projects", _ACTION .. "-osx"))

		elseif "ios" == _OPTIONS["xcode"] then
			premake.xcode.toolset = "iphoneos"
			location (path.join(_buildDir, "projects", _ACTION .. "-ios"))
		end
	end

	if (_OPTIONS["CC"] ~= nil) then
		premake.gcc.cc  = _OPTIONS["CC"]
	end
	if (_OPTIONS["CXX"] ~= nil) then
		premake.gcc.cxx  = _OPTIONS["CXX"]
	end
	if (_OPTIONS["LD"] ~= nil) then
		premake.gcc.ld  = _OPTIONS["LD"]
	end

	configuration {} -- reset configuration


	configuration { "x32", "vs*" }
		objdir (_buildDir .. _ACTION .. "/obj")

	configuration { "x32", "vs*", "Release" }
		targetdir (_buildDir .. _ACTION .. "/bin/x32/Release")

	configuration { "x32", "vs*", "Debug" }
		targetdir (_buildDir .. _ACTION .. "/bin/x32/Debug")

	configuration { "x64", "vs*" }
		defines { "_WIN64" }
		objdir (_buildDir .. _ACTION .. "/obj")

	configuration { "x64", "vs*", "Release" }
		targetdir (_buildDir .. _ACTION .. "/bin/x64/Release")

	configuration { "x64", "vs*", "Debug" }
		targetdir (_buildDir .. _ACTION .. "/bin/x64/Debug")

	configuration { "ARM", "vs*" }
		targetdir (_buildDir .. _ACTION .. "/bin/ARM")
		objdir (_buildDir .. _ACTION .. "/obj")

	configuration { "ARM", "vs*", "Release" }
		targetdir (_buildDir .. _ACTION .. "/bin/ARM/Release")

	configuration { "ARM", "vs*", "Debug" }
		targetdir (_buildDir .. _ACTION .. "/bin/ARM/Debug")

	configuration { "x32", "vs*-clang" }
		objdir (_buildDir .. _ACTION .. "-clang/obj")

	configuration { "x32", "vs*-clang", "Release" }
		targetdir (_buildDir .. _ACTION .. "-clang/bin/x32/Release")

	configuration { "x32", "vs*-clang", "Debug" }
		targetdir (_buildDir .. _ACTION .. "-clang/bin/x32/Debug")

	configuration { "x64", "vs*-clang" }
		objdir (_buildDir .. _ACTION .. "-clang/obj")

	configuration { "x64", "vs*-clang", "Release" }
		targetdir (_buildDir .. _ACTION .. "-clang/bin/x64/Release")

	configuration { "x64", "vs*-clang", "Debug" }
		targetdir (_buildDir .. _ACTION .. "-clang/bin/x64/Debug")

	configuration { "vs*-clang" }
		buildoptions {
			"-Qunused-arguments",
		}

	configuration { "winphone8* or winstore8*" }
		removeflags {
			"StaticRuntime",
			"NoExceptions",
		}
		flags {
			"WinMain"
		}

	configuration { "mingw*" }
		defines { "WIN32" }

	configuration { "x32", "mingw32-gcc" }
		objdir (_buildDir .. "mingw-gcc" .. "/obj")
		buildoptions { "-m32" }

	configuration { "x32", "mingw32-gcc", "Release" }
		targetdir (_buildDir .. "mingw-gcc" .. "/bin/x32/Release")

	configuration { "x32", "mingw32-gcc", "Debug" }
		targetdir (_buildDir .. "mingw-gcc" .. "/bin/x32/Debug")

	configuration { "x64", "mingw64-gcc" }
		objdir (_buildDir .. "mingw-gcc" .. "/obj")
		buildoptions { "-m64" }

	configuration { "x64", "mingw64-gcc", "Release" }
		targetdir (_buildDir .. "mingw-gcc" .. "/bin/x64/Release")

	configuration { "x64", "mingw64-gcc", "Debug" }
		targetdir (_buildDir .. "mingw-gcc" .. "/bin/x64/Debug")

	configuration { "steamlink" }
		objdir ( _buildDir .. "steamlink/obj")
		defines {
			"__STEAMLINK__=1", -- There is no special prefedined compiler symbol to detect SteamLink, faking it.
		}
		buildoptions {
			"-marm",
			"-mfloat-abi=hard",
			"--sysroot=$(MARVELL_SDK_PATH)/rootfs",
		}
		linkoptions {
			"-static-libgcc",
			"-static-libstdc++",
			"--sysroot=$(MARVELL_SDK_PATH)/rootfs",
		}

	configuration { "steamlink", "Release" }
		targetdir (_buildDir .. "steamlink/bin/Release")

	configuration { "steamlink", "Debug" }
		targetdir (_buildDir .. "steamlink/bin/Debug")

	configuration { "rpi" }
		objdir ( _buildDir .. "rpi/obj")
		libdirs {
			"$(RASPBERRY_SYSROOT)/opt/vc/lib",
		}
		includedirs {
			"$(RASPBERRY_SYSROOT)/opt/vc/include",
			"$(RASPBERRY_SYSROOT)/opt/vc/include/interface/vcos/pthreads",
			"$(RASPBERRY_SYSROOT)/opt/vc/include/interface/vmcs_host/linux",
		}
		defines {
			"__VCCOREVER__=0x04000000", -- There is no special prefedined compiler symbol to detect RaspberryPi, faking it.
		}
		linkoptions {
			"-Wl,--gc-sections",
		}
		buildoptions {
			"--sysroot=$(RASPBERRY_SYSROOT)",
		}
		linkoptions {
			"-static-libgcc",
			"-static-libstdc++",
			"--sysroot=$(RASPBERRY_SYSROOT)",
		}

	configuration { "rpi", "Release" }
		targetdir (_buildDir .. "rpi/bin/Release")

	configuration { "rpi", "Debug" }
		targetdir (_buildDir .. "rpi/bin/Debug")

	configuration { "ci20" }
		objdir ( _buildDir .. "ci20/obj")
		includedirs {
			"$(CI20_SYSROOT)/mipsel-r2-hard/usr/include/c++/4.9",
			"$(CI20_SYSROOT)/mipsel-r2-hard/usr/include/mipsel-linux-gnu/c++/4.9",
			"$(CI20_SYSROOT)/mipsel-r2-hard/usr/include/c++/4.9/backward",
			"$(CI20_SYSROOT)/mipsel-r2-hard/usr/lib/gcc/mipsel-linux-gnu/4.9/include",
			"$(CI20_SYSROOT)/mipsel-r2-hard/usr/local/include",
			"$(CI20_SYSROOT)/mipsel-r2-hard/usr/lib/gcc/mipsel-linux-gnu/4.9/include-fixed",
			"$(CI20_SYSROOT)/mipsel-r2-hard/usr/include/mipsel-linux-gnu",
			"$(CI20_SYSROOT)/mipsel-r2-hard/usr/include",
		}
		links {
			"c",
			"dl",
			"m",
			"gcc",
			"stdc++",
			"gcc_s",
		}

		buildoptions {
			"--sysroot=$(CI20_SYSROOT)",
			"-Wno-pragmas",
			"-Wno-undef",
			"-EL",
			"-mel",
			"-march=mips32r2",
			"-mllsc",
			"-mabi=32",
		}
		linkoptions {
			"--sysroot=$(CI20_SYSROOT)",
			"-Wl,-rpath=$(CI20_SYSROOT)/mipsel-r2-hard/usr/lib/mipsel-linux-gnu/",
			"-Wl,-rpath=$(CI20_SYSROOT)/mipsel-r2-hard/lib/mipsel-linux-gnu/",
			"-nostdlib",
			"-EL",
			"-mel",
			"-march=mips32r2",
			"-mllsc",
			"-mabi=32",
			"$(MIPS_LINUXGNU_ROOT)/lib/gcc/mips-mti-linux-gnu/4.9.2/mipsel-r2-hard/lib/crtbegin.o",
			"$(MIPS_LINUXGNU_ROOT)/lib/gcc/mips-mti-linux-gnu/4.9.2/mipsel-r2-hard/lib/crtend.o",
			"-L$(CI20_SYSROOT)/mipsel-r2-hard/usr/lib/gcc/mipsel-linux-gnu/4.9",
			"-L$(CI20_SYSROOT)/mipsel-r2-hard/usr/lib/mipsel-linux-gnu",
			"-L$(CI20_SYSROOT)/mipsel-r2-hard/usr/lib",
			"-L$(CI20_SYSROOT)/mipsel-r2-hard/lib/mipsel-linux-gnu",
			"-L$(CI20_SYSROOT)/mipsel-r2-hard/lib",
		}

	configuration { "ci20", "Release" }
		targetdir (_buildDir .. "ci20/bin/Release")

	configuration { "ci20", "Debug" }
		targetdir (_buildDir .. "ci20/bin/Debug")

	configuration { "mingw-clang" }
		linkoptions {
			"-Wl,--allow-multiple-definition",
		}

	configuration { "x32", "mingw-clang" }
		objdir ( _buildDir .. "mingw-clang/obj")
		buildoptions { "-m32" }

	configuration { "x32", "mingw-clang", "Release" }
		targetdir (_buildDir .. "mingw-clang/bin/x32/Release")

	configuration { "x32", "mingw-clang", "Debug" }
		targetdir (_buildDir .. "mingw-clang/bin/x32/Debug")

	configuration { "x64", "mingw-clang" }
		objdir (_buildDir .. "mingw-clang/obj")
		buildoptions { "-m64" }

	configuration { "x64", "mingw-clang", "Release" }
		targetdir (_buildDir .. "mingw-clang/bin/x64/Release")

	configuration { "x64", "mingw-clang", "Debug" }
		targetdir (_buildDir .. "mingw-clang/bin/x64/Debug")

	configuration { "linux-gcc", "x32" }
		objdir (_buildDir .. "linux_gcc" .. "/obj")
		buildoptions {
			"-m32",
		}

	configuration { "linux-gcc", "x32", "Release" }
		targetdir (_buildDir .. "linux_gcc" .. "/bin/x32/Release")

	configuration { "linux-gcc", "x32", "Debug" }
		targetdir (_buildDir .. "linux_gcc" .. "/bin/x32/Debug")

	configuration { "linux-gcc", "x64" }
		objdir (_buildDir .. "linux_gcc" .. "/obj")
		buildoptions {
			"-m64",
		}

	configuration { "linux-gcc", "x64", "Release" }
		targetdir (_buildDir .. "linux_gcc" .. "/bin/x64/Release")

	configuration { "linux-gcc", "x64", "Debug" }
		targetdir (_buildDir .. "linux_gcc" .. "/bin/x64/Debug")

	configuration { "linux-clang", "x32" }
		objdir (_buildDir .. "linux_clang" .. "/obj")
		buildoptions {
			"-m32",
		}

	configuration { "linux-clang", "x32", "Release" }
		targetdir (_buildDir .. "linux_clang" .. "/bin/x32/Release")

	configuration { "linux-clang", "x32", "Debug" }
		targetdir (_buildDir .. "linux_clang" .. "/bin/x32/Debug")

	configuration { "linux-clang", "x64" }
		objdir (_buildDir .. "linux_clang" .. "/obj")
		buildoptions {
			"-m64",
		}

	configuration { "linux-clang", "x64", "Release" }
		targetdir (_buildDir .. "linux_clang" .. "/bin/x64/Release")

	configuration { "linux-clang", "x64", "Debug" }
		targetdir (_buildDir .. "linux_clang" .. "/bin/x64/Debug")

	configuration { "solaris", "x32" }
		objdir (_buildDir .. "solaris" .. "/obj")
		buildoptions {
			"-m32",
		}

	configuration { "solaris", "x32", "Release" }
		targetdir (_buildDir .. "solaris" .. "/bin/x32/Release")

	configuration { "solaris", "x32", "Debug" }
		targetdir (_buildDir .. "solaris" .. "/bin/x32/Debug")

	configuration { "solaris", "x64" }
		objdir (_buildDir .. "solaris" .. "/obj")
		buildoptions {
			"-m64",
		}

	configuration { "solaris", "x64", "Release" }
		targetdir (_buildDir .. "solaris" .. "/bin/x64/Release")

	configuration { "solaris", "x64", "Debug" }
		targetdir (_buildDir .. "solaris" .. "/bin/x64/Debug")

	configuration { "freebsd", "x32" }
		objdir (_buildDir .. "freebsd" .. "/obj")
		buildoptions {
			"-m32",
		}

	configuration { "freebsd", "x32", "Release" }
		targetdir (_buildDir .. "freebsd" .. "/bin/x32/Release")

	configuration { "freebsd", "x32", "Debug" }
		targetdir (_buildDir .. "freebsd" .. "/bin/x32/Debug")

	configuration { "freebsd", "x64" }
		objdir (_buildDir .. "freebsd" .. "/obj")
		buildoptions {
			"-m64",
		}
	configuration { "freebsd", "x64", "Release" }
		targetdir (_buildDir .. "freebsd" .. "/bin/x64/Release")

	configuration { "freebsd", "x64", "Debug" }
		targetdir (_buildDir .. "freebsd" .. "/bin/x64/Debug")

	configuration { "netbsd", "x32" }
		objdir (_buildDir .. "netbsd" .. "/obj")
		buildoptions {
			"-m32",
		}
	configuration { "netbsd", "x32", "Release" }
		targetdir (_buildDir .. "netbsd" .. "/bin/x32/Release")

	configuration { "netbsd", "x32", "Debug" }
		targetdir (_buildDir .. "netbsd" .. "/bin/x32/Debug")

	configuration { "netbsd", "x64" }
		objdir (_buildDir .. "netbsd" .. "/obj")
		buildoptions {
			"-m64",
		}
	configuration { "netbsd", "x64", "Release" }
		targetdir (_buildDir .. "netbsd" .. "/bin/x64/Release")

	configuration { "netbsd", "x64", "Debug" }
		targetdir (_buildDir .. "netbsd" .. "/bin/x64/Debug")

	configuration { "openbsd", "x32" }
		objdir (_buildDir .. "openbsd" .. "/obj")
		buildoptions {
			"-m32",
		}
	configuration { "openbsd", "x32", "Release" }
		targetdir (_buildDir .. "openbsd" .. "/bin/x32/Release")

	configuration { "openbsd", "x32", "Debug" }
		targetdir (_buildDir .. "openbsd" .. "/bin/x32/Debug")

	configuration { "openbsd", "x64" }
		objdir (_buildDir .. "openbsd" .. "/obj")
		buildoptions {
			"-m64",
		}
	configuration { "openbsd", "x64", "Release" }
		targetdir (_buildDir .. "openbsd" .. "/bin/x64/Release")

	configuration { "openbsd", "x64", "Debug" }
		targetdir (_buildDir .. "openbsd" .. "/bin/x64/Debug")

	configuration { "android-*", "Release" }
		targetdir (_buildDir .. "android/bin/" .. _OPTIONS["PLATFORM"] .. "/Release")

	configuration { "android-*", "Debug" }
		targetdir (_buildDir .. "android/bin/" .. _OPTIONS["PLATFORM"] .. "/Debug")

	configuration { "android-*" }
		objdir (_buildDir .. "android/obj/" .. _OPTIONS["PLATFORM"])
		includedirs {
			MAME_DIR .. "3rdparty/bgfx/3rdparty/khronos",
			"$(ANDROID_NDK_ROOT)/sources/cxx-stl/llvm-libc++/libcxx/include",
			"$(ANDROID_NDK_ROOT)/sources/cxx-stl/llvm-libc++/include",
			"$(ANDROID_NDK_ROOT)/sysroot/usr/include",
			"$(ANDROID_NDK_ROOT)/sources/android/support/include",
			"$(ANDROID_NDK_ROOT)/sources/android/native_app_glue",
		}
		linkoptions {
			"-nostdlib",
		}
		flags {
			"NoImportLib",
		}
		links {
			"c",
			"dl",
			"m",
			"android",
			"log",
			"c++_static",
			"c++abi",
			"stdc++",
			"gcc",
		}
		buildoptions_c {
			"-Wno-strict-prototypes",
		}
		buildoptions {
			"-fpic",
			"-ffunction-sections",
			"-funwind-tables",
			"-fstack-protector-strong",
			"-no-canonical-prefixes",
			"-fno-integrated-as",
			"-Wunused-value",
			"-Wundef",
			"-Wno-cast-align",
			"-Wno-unknown-attributes",
			"-Wno-macro-redefined",
			"-DASIO_HAS_STD_STRING_VIEW",
			"-Wno-unused-function",
		}
		linkoptions {
			"-no-canonical-prefixes",
			"-Wl,--no-undefined",
			"-Wl,-z,noexecstack",
			"-Wl,-z,relro",
			"-Wl,-z,now",
		}


	configuration { "android-arm" }
			libdirs {
				"$(ANDROID_NDK_ROOT)/sources/cxx-stl/llvm-libc++/libs/armeabi-v7a",
				"$(ANDROID_NDK_ROOT)/platforms/" .. androidPlatform .. "/arch-arm/usr/lib",
			}
			includedirs {
				"$(ANDROID_NDK_ROOT)/sysroot/usr/include/arm-linux-androideabi",
			}
			buildoptions {
				"-gcc-toolchain $(ANDROID_NDK_ARM)",
				"-target armv7-none-linux-androideabi",
				"-march=armv7-a",
				"-mfloat-abi=softfp",
				"-mfpu=vfpv3-d16",
				"-mthumb",
			}
			links {
				"unwind",
			}
			linkoptions {
				"-gcc-toolchain $(ANDROID_NDK_ARM)",
				"--sysroot=$(ANDROID_NDK_ROOT)/platforms/" .. androidPlatform .. "/arch-arm",
				"$(ANDROID_NDK_ROOT)/platforms/" .. androidPlatform .. "/arch-arm/usr/lib/crtbegin_so.o",
				"$(ANDROID_NDK_ROOT)/platforms/" .. androidPlatform .. "/arch-arm/usr/lib/crtend_so.o",
				"-target armv7-none-linux-androideabi",
				"-march=armv7-a",
				"-mthumb",
			}

	configuration { "android-arm64" }
			libdirs {
				"$(ANDROID_NDK_ROOT)/sources/cxx-stl/llvm-libc++/libs/arm64-v8a",
				"$(ANDROID_NDK_ROOT)/platforms/" .. androidPlatform .. "/arch-arm64/usr/lib64",
			}
			includedirs {
				"$(ANDROID_NDK_ROOT)/sysroot/usr/include/aarch64-linux-android",
			}
			buildoptions {
				"-gcc-toolchain $(ANDROID_NDK_ARM64)",
				"-target aarch64-none-linux-android",
			}
			linkoptions {
				"-gcc-toolchain $(ANDROID_NDK_ARM64)",
				"--sysroot=$(ANDROID_NDK_ROOT)/platforms/" .. androidPlatform .. "/arch-arm64",
				"$(ANDROID_NDK_ROOT)/platforms/" .. androidPlatform .. "/arch-arm64/usr/lib/crtbegin_so.o",
				"$(ANDROID_NDK_ROOT)/platforms/" .. androidPlatform .. "/arch-arm64/usr/lib/crtend_so.o",
				"-target aarch64-none-linux-android",
			}

	configuration { "android-x86" }
		libdirs {
			"$(ANDROID_NDK_ROOT)/sources/cxx-stl/llvm-libc++/libs/x86",
			"$(ANDROID_NDK_ROOT)/platforms/" .. androidPlatform .. "/arch-x86/usr/lib",
		}
		includedirs {
			"$(ANDROID_NDK_ROOT)/sysroot/usr/include/i686-linux-android",
		}
		buildoptions {
			"-gcc-toolchain $(ANDROID_NDK_X86)",
			"-target i686-none-linux-android",
			"-mssse3"
		}
		linkoptions {
			"-gcc-toolchain $(ANDROID_NDK_X86)",
			"-target i686-none-linux-android",
			"-mssse3",
			"--sysroot=$(ANDROID_NDK_ROOT)/platforms/" .. androidPlatform .. "/arch-x86",
			"$(ANDROID_NDK_ROOT)/platforms/" .. androidPlatform .. "/arch-x86/usr/lib/crtbegin_so.o",
			"$(ANDROID_NDK_ROOT)/platforms/" .. androidPlatform .. "/arch-x86/usr/lib/crtend_so.o",
		}

	configuration { "android-x64" }
		libdirs {
			"$(ANDROID_NDK_ROOT)/sources/cxx-stl/llvm-libc++/libs/x86_64",
			"$(ANDROID_NDK_ROOT)/platforms/" .. androidPlatform .. "/arch-x86_64/usr/lib64",
		}
		includedirs {
			"$(ANDROID_NDK_ROOT)/sysroot/usr/include/x86_64-linux-android",
		}
		buildoptions {
			"-gcc-toolchain $(ANDROID_NDK_X64)",
			"-target x86_64-none-linux-android",
		}
		linkoptions {
			"-gcc-toolchain $(ANDROID_NDK_X64)",
			"-target x86_64-none-linux-android",
			"--sysroot=$(ANDROID_NDK_ROOT)/platforms/" .. androidPlatform .. "/arch-x86_64",
			"$(ANDROID_NDK_ROOT)/platforms/" .. androidPlatform .. "/arch-x86_64/usr/lib64/crtbegin_so.o",
			"$(ANDROID_NDK_ROOT)/platforms/" .. androidPlatform .. "/arch-x86_64/usr/lib64/crtend_so.o",
		}

	configuration { "asmjs" }
		targetdir (_buildDir .. "asmjs" .. "/bin")
		objdir (_buildDir .. "asmjs" .. "/obj")
		includedirs {
			"$(EMSCRIPTEN)/system/include",
			"$(EMSCRIPTEN)/system/include/compat",
			"$(EMSCRIPTEN)/system/include/libc",
			"$(EMSCRIPTEN)/system/lib/libcxxabi/include",
		}
		buildoptions {
			"-Wno-cast-align",
			"-Wno-tautological-compare",
			"-Wno-self-assign-field",
			"-Wno-format-security",
			"-Wno-inline-new-delete",
			"-Wno-constant-logical-operand",
			"-Wno-absolute-value",
			"-Wno-unknown-warning-option",
			"-Wno-extern-c-compat",
		}

	configuration { "pnacl" }
		buildoptions {
			"-U__STRICT_ANSI__", -- strcasecmp, setenv, unsetenv,...
			"-fno-stack-protector",
			"-fdiagnostics-show-option",
			"-fdata-sections",
			"-ffunction-sections",
			"-Wunused-value",
		}

	configuration { "pnacl" }
		buildoptions {
			"-Wno-tautological-undefined-compare",
			"-Wno-cast-align",
		}
		includedirs {
			"$(NACL_SDK_ROOT)/include",
			"$(NACL_SDK_ROOT)/include/pnacl",
		}

	configuration { "pnacl" }
		targetdir (_buildDir .. "pnacl" .. "/bin")
		objdir (_buildDir .. "pnacl" .. "/obj")

	configuration { "pnacl", "Debug" }
		libdirs { "$(NACL_SDK_ROOT)/lib/pnacl/Debug" }

	configuration { "pnacl", "Release" }
		libdirs { "$(NACL_SDK_ROOT)/lib/pnacl/Release" }

	configuration { "osx*", "x32" }
		objdir (_buildDir .. "osx_clang" .. "/obj")
		buildoptions {
			"-m32",
		}
	configuration { "osx*", "x32", "Release" }
		targetdir (_buildDir .. "osx_clang" .. "/bin/x32/Release")

	configuration { "osx*", "x32", "Debug" }
		targetdir (_buildDir .. "osx_clang" .. "/bin/x32/Debug")

	configuration { "osx*", "x64" }
		objdir (_buildDir .. "osx_clang" .. "/obj")
		buildoptions {
			"-m64", "-DHAVE_IMMINTRIN_H=1",
		}

	configuration { "osx*", "x64", "Release" }
		targetdir (_buildDir .. "osx_clang" .. "/bin/x64/Release")

	configuration { "osx*", "x64", "Debug" }
		targetdir (_buildDir .. "osx_clang" .. "/bin/x64/Debug")

	configuration { "ios-arm" }
		targetdir (_buildDir .. "ios-arm" .. "/bin")
		objdir (_buildDir .. "ios-arm" .. "/obj")

	configuration { "ios-simulator" }
		targetdir (_buildDir .. "ios-simulator" .. "/bin")
		objdir (_buildDir .. "ios-simulator" .. "/obj")

	configuration { "rpi" }
		targetdir (_buildDir .. "rpi" .. "/bin")
		objdir (_buildDir .. "rpi" .. "/obj")

	configuration {} -- reset configuration

	return true
end

function strip()
	if _OPTIONS["STRIP_SYMBOLS"]~="1" then
		return true
	end

	configuration { "osx-*" }
		postbuildcommands {
			"$(SILENT) echo Stripping symbols.",
			"$(SILENT) " .. (_OPTIONS['TOOLCHAIN'] and toolchainPrefix) .. "strip \"$(TARGET)\"",
		}

	configuration { "android-*" }
		postbuildcommands {
			"$(SILENT) echo Stripping symbols.",
			"$(SILENT) " .. toolchainPrefix .. "strip -s \"$(TARGET)\""
		}

	configuration { "linux-* or rpi" }
		postbuildcommands {
			"$(SILENT) echo Stripping symbols.",
			"$(SILENT) strip -s \"$(TARGET)\""
		}

	configuration { "mingw*", "x64" }
		postbuildcommands {
			"$(SILENT) echo Stripping symbols.",
			"$(SILENT) " .. (_OPTIONS['TOOLCHAIN'] or "$(MINGW64)/bin/") .. "strip -s \"$(TARGET)\"",
		}
	configuration { "mingw*", "x32" }
		postbuildcommands {
			"$(SILENT) echo Stripping symbols.",
			"$(SILENT) " .. (_OPTIONS['TOOLCHAIN'] or "$(MINGW32)/bin/") .. "strip -s \"$(TARGET)\"",
		}

	configuration { "pnacl" }
		postbuildcommands {
			"$(SILENT) echo Running pnacl-finalize.",
			"$(SILENT) " .. naclToolchain .. "finalize \"$(TARGET)\""
		}

	configuration { "asmjs" }
		postbuildcommands {
			"$(SILENT) echo Running asmjs finalize.",
			"$(SILENT) $(EMSCRIPTEN)/emcc -O2 -s TOTAL_MEMORY=268435456 \"$(TARGET)\" -o \"$(TARGET)\".html"
			-- ALLOW_MEMORY_GROWTH
		}

	configuration {} -- reset configuration
end

