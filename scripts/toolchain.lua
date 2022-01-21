--
-- Copyright 2010-2021 Branimir Karadzic. All rights reserved.
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
		{ "android-x86",   "Android - x86"          },
		{ "android-x64",   "Android - x64"          },
		{ "asmjs",         "Emscripten/asm.js"      },
		{ "freebsd",       "FreeBSD"                },
		{ "freebsd-clang", "FreeBSD (clang compiler)"},
		{ "linux-gcc",     "Linux (GCC compiler)"   },
		{ "linux-clang",   "Linux (Clang compiler)" },
		{ "mingw32-gcc",   "MinGW32"                },
		{ "mingw64-gcc",   "MinGW64"                },
		{ "mingw-clang",   "MinGW (clang compiler)" },
		{ "netbsd",        "NetBSD"                },
		{ "netbsd-clang",  "NetBSD (clang compiler)"},
		{ "openbsd",       "OpenBSD"                },
		{ "osx",           "OSX (GCC compiler)"     },
		{ "osx-clang",     "OSX (Clang compiler)"   },
		{ "solaris",       "Solaris"                },
	},
}

newoption {
	trigger = "vs",
	value = "toolset",
	description = "Choose VS toolset",
	allowed = {
		{ "intel-15",      "Intel C++ Compiler XE 15.0" },
		{ "clangcl",       "Visual Studio 2019 using Clang/LLVM" },
	},
}

newoption {
	trigger = "with-android",
	value   = "#",
	description = "Set Android platform version (default: android-21).",
}

function toolchain(_buildDir, _subDir)

	location (_buildDir .. "projects/" .. _subDir .. "/".. _ACTION)

	local androidPlatform = "android-24"
	if _OPTIONS["with-android"] then
		androidPlatform = "android-" .. _OPTIONS["with-android"]
	elseif _OPTIONS["PLATFORM"]:find("64", -2) then
		androidPlatform = "android-24"
	end
	androidPlatformNumber = androidPlatform:sub(9)

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

		if "mingw32-gcc" == _OPTIONS["gcc"] then
			if not os.getenv("MINGW32") then
				print("Set MINGW32 envrionment variable.")
			end
			if toolchainPrefix == nil or toolchainPrefix == "" then
				toolchainPrefix = "$(MINGW32)/bin/i686-w64-mingw32-"
			end
			premake.gcc.cc  = toolchainPrefix .. "gcc"
			premake.gcc.cxx = toolchainPrefix .. "g++"
			premake.gcc.ar  = toolchainPrefix .. "gcc-ar"
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
			premake.gcc.ar  = toolchainPrefix .. "gcc-ar"
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
	elseif _ACTION == "vs2019" then

		if "clangcl" == _OPTIONS["vs"] then
			premake.vstudio.toolset = ("ClangCL")
			location (_buildDir .. "projects/" .. _subDir .. "/".. _ACTION .. "-clang")
		end

		if "intel-15" == _OPTIONS["vs"] then
			premake.vstudio.toolset = "Intel C++ Compiler XE 15.0"
			location (_buildDir .. "projects/" .. _subDir .. "/".. _ACTION .. "-intel")
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
	if (_OPTIONS["AR"] ~= nil) then
		premake.gcc.ar  = _OPTIONS["AR"]
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

	configuration { "mingw-clang" }
		buildoptions {
			"-femulated-tls",
		}
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
			--  LIBRETRO: don't mess with NDK includir order
			-- "$(ANDROID_NDK_ROOT)/sources/cxx-stl/llvm-libc++/libcxx/include",
			-- "$(ANDROID_NDK_ROOT)/sources/cxx-stl/llvm-libc++/include",
			-- "$(ANDROID_NDK_ROOT)/sysroot/usr/include",
			-- "$(ANDROID_NDK_ROOT)/sources/android/support/include",
			-- "$(ANDROID_NDK_ROOT)/sources/android/native_app_glue",
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
				"$(ANDROID_NDK_LLVM)/sysroot/usr/lib/arm-linux-androideabi/" .. androidPlatformNumber,
			}
			includedirs {
			}
			buildoptions {
				"-gcc-toolchain $(ANDROID_NDK_ARM)",
				"-target armv7-linux-androideabi" .. androidPlatformNumber,
				"-march=armv7-a",
				"-mfloat-abi=softfp",
				"-mfpu=vfpv3-d16",
				"-mthumb",
			}
			links {
				"android_support",
				"unwind",
				"gcc",
			}
			linkoptions {
				"-gcc-toolchain $(ANDROID_NDK_ARM)",
				"--sysroot=$(ANDROID_NDK_LLVM)/sysroot/usr/lib/arm-linux-androideabi/" .. androidPlatformNumber,
				"$(ANDROID_NDK_LLVM)/sysroot/usr/lib/arm-linux-androideabi/" .. androidPlatformNumber .. "/crtbegin_so.o",
				"$(ANDROID_NDK_LLVM)/sysroot/usr/lib/arm-linux-androideabi/" .. androidPlatformNumber .. "/crtend_so.o",
				"-target armv7-linux-androideabi" .. androidPlatformNumber,
				"-march=armv7-a",
				"-mthumb",
			}

	configuration { "android-arm64" }
			libdirs {
				"$(ANDROID_NDK_ROOT)/sources/cxx-stl/llvm-libc++/libs/arm64-v8a",
				"$(ANDROID_NDK_LLVM)/sysroot/usr/lib/aarch64-linux-android/" .. androidPlatformNumber,
			}
			includedirs {
			}
			buildoptions {
				"-gcc-toolchain $(ANDROID_NDK_ARM64)",
				"-target aarch64-linux-android" .. androidPlatformNumber,
				"-march=armv8-a",
			}
			links {
				"gcc",
			}
			linkoptions {
				"-gcc-toolchain $(ANDROID_NDK_ARM64)",
				"--sysroot=$(ANDROID_NDK_LLVM)/sysroot/usr/lib/aarch64-linux-android/" .. androidPlatformNumber,
				"$(ANDROID_NDK_LLVM)/sysroot/usr/lib/aarch64-linux-android/" .. androidPlatformNumber .. "/crtbegin_so.o",
				"$(ANDROID_NDK_LLVM)/sysroot/usr/lib/aarch64-linux-android/" .. androidPlatformNumber .. "/crtend_so.o",
				"-target aarch64-linux-android" .. androidPlatformNumber,
				"-march=armv8-a",
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
			"$(ANDROID_NDK_LLVM)/sysroot/usr/lib/x86_64-linux-android/" .. androidPlatformNumber,
		}
		includedirs {
		}
		buildoptions {
			"-gcc-toolchain $(ANDROID_NDK_X64)",
			"-target x86_64-linux-android" .. androidPlatformNumber,
		}
		links {
			"gcc",
		}
		linkoptions {
			"-gcc-toolchain $(ANDROID_NDK_X64)",
			"-target x86_64-linux-android" .. androidPlatformNumber,
			"--sysroot=$(ANDROID_NDK_LLVM)/sysroot/usr/lib/x86_64-linux-android/" .. androidPlatformNumber,
			"$(ANDROID_NDK_LLVM)/sysroot/usr/lib/x86_64-linux-android/" .. androidPlatformNumber .. "/crtbegin_so.o",
			"$(ANDROID_NDK_LLVM)/sysroot/usr/lib/x86_64-linux-android/" .. androidPlatformNumber .. "/crtend_so.o",
			"-target x86_64-linux-android" .. androidPlatformNumber,
		}

	configuration { "asmjs" }
		targetdir (_buildDir .. "asmjs" .. "/bin")
		objdir (_buildDir .. "asmjs" .. "/obj")
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

	configuration { "osx*", "x32", "not arm64" }
		objdir (_buildDir .. "osx_clang" .. "/obj")
		buildoptions {
			"-m32",
		}
	configuration { "osx*", "x32", "not arm64", "Release" }
		targetdir (_buildDir .. "osx_clang" .. "/bin/x32/Release")

	configuration { "osx*", "x32", "not arm64", "Debug" }
		targetdir (_buildDir .. "osx_clang" .. "/bin/x32/Debug")

	configuration { "osx*", "x64", "not arm64" }
		objdir (_buildDir .. "osx_clang" .. "/obj")
		buildoptions {
			"-m64", "-DHAVE_IMMINTRIN_H=1",
		}

	configuration { "osx*", "x64", "not arm64", "Release" }
		targetdir (_buildDir .. "osx_clang" .. "/bin/x64/Release")

	configuration { "osx*", "x64", "not arm64", "Debug" }
		targetdir (_buildDir .. "osx_clang" .. "/bin/x64/Debug")

	configuration { "osx*", "arm64" }
		objdir (_buildDir .. "osx_clang" .. "/obj")
		buildoptions {
			"-m64", "-DHAVE_IMMINTRIN_H=0", "-DSDL_DISABLE_IMMINTRIN_H=1", "-DHAVE_SSE=0"
		}

	configuration { "osx*", "arm64", "Release" }
		targetdir (_buildDir .. "osx_clang" .. "/bin/x64/Release")

	configuration { "osx*", "arm64", "Debug" }
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
-- BEGIN libretro overrides to MAME's GENie build

	configuration { "libretro*" }
		objdir (_buildDir .. "libretro" .. "/obj")

	-- $ARCH means something to Apple/Clang, so we can't use it here.
		-- Instead, use ARCH="" LIBRETRO_CPU="$ARCH" on the make cmdline.
		--
		-- NB: $ARCH has caused libretro problems before, LIBRETRO_CPU will
		--     replace it everywhere at some point.
		newoption {
			trigger = "LIBRETRO_CPU",
			description = "libretro CPU/architecture variable",
		}
		if _OPTIONS["LIBRETRO_CPU"]~=nil then
			LIBRETRO_CPU=_OPTIONS["LIBRETRO_CPU"]
		end

		-- $platform we could keep using, but $LIBRETRO_OS seems safer.
		newoption {
			trigger = "LIBRETRO_OS",
			description = "libretro OS/platform variable",
		}
		if _OPTIONS["LIBRETRO_OS"]~=nil then
			LIBRETRO_OS=_OPTIONS["LIBRETRO_OS"]
		end

		-- Set TARGETOS based on LIBRETRO_OS if we know
		if LIBRETRO_OS~=nil then
			-- most things are "linux" (ish).
			local targetos = "linux"
			if LIBRETRO_OS=="osx" then
				targetos = "macosx"
			elseif LIBRETRO_OS:sub(1, 4)=="armv" then
				targetos = "android"
                        elseif LIBRETRO_OS=="ios" then
				targetos = "ios"
                        elseif LIBRETRO_OS=="win32" then
				targetos = "win32"
			end
			_OPTIONS["TARGETOS"] = targetos
		end

		-- FIXME: set BIGENDIAN and dynarec based on retro_platform/retro_arch
		if LIBRETRO_CPU~=nil then
			if LIBRETRO_CPU=="x86_64" or LIBRETRO_CPU=="ppc64" then
				defines { "PTR64=1" }
			end
		end


		-- MS and Apple don't need -fPIC, but pretty much everything else does.
		if _OPTIONS["targetos"] ~= "windows" and _OPTIONS["targetos"] ~= "macosx" then
			buildoptions { "-fPIC" }
			linkoptions { "-fPIC" }
		end

		-- Don't use BGFX (Defaults to 1 for Windows if unset)
		 USE_BGFX = 0
-- _OPTIONS["USE_BGFX"] = "1"
		-- libretro only supports the retro OSD
		_OPTIONS["osd"] = "retro"

		-- libretro does not (yet) support MIDI.
		_OPTIONS["NO_USE_MIDI"] = "1"

	configuration { "libretrodbg" }
		targetdir (_buildDir .. "libretro"  .. "/debug")
		flags {
			"Symbols",
		}
	configuration { "libretro" }
		targetdir (_buildDir .. "libretro" .. "/bin")
		flags {
			"Optimize",
		}


-- END   libretro overrides to MAME's GENie build

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

	configuration { "linux-*" }
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

	configuration { "asmjs" }
		postbuildcommands {
			"$(SILENT) echo Running asmjs finalize.",
			"$(SILENT) $(EMSCRIPTEN)/emcc -O2 -s TOTAL_MEMORY=268435456 \"$(TARGET)\" -o \"$(TARGET)\".html"
			-- ALLOW_MEMORY_GROWTH
		}

	configuration {} -- reset configuration
end

