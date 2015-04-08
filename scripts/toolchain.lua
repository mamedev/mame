--
-- Copyright 2010-2015 Branimir Karadzic. All rights reserved.
-- License: https://github.com/bkaradzic/bx#license-bsd-2-clause
--

local naclToolchain = ""

newoption {
	trigger = "gcc",
	value = "GCC",
	description = "Choose GCC flavor",
	allowed = {
		{ "android-arm",   "Android - ARM"          },
		{ "android-mips",  "Android - MIPS"         },
		{ "android-x86",   "Android - x86"          },
		{ "asmjs",         "Emscripten/asm.js"      },
		{ "freebsd",       "FreeBSD"                },
		{ "linux-gcc",     "Linux (GCC compiler)"   },
		{ "linux-clang",   "Linux (Clang compiler)" },
		{ "ios-arm",       "iOS - ARM"              },
		{ "ios-simulator", "iOS - Simulator"        },
		{ "mingw32-gcc",   "MinGW32"                },
		{ "mingw64-gcc",   "MinGW64"                },
		{ "mingw-clang",   "MinGW (clang compiler)" },
		{ "nacl",          "Native Client"          },
		{ "nacl-arm",      "Native Client - ARM"    },
		{ "osx",           "OSX (GCC compiler)"     },
		{ "osx-clang",     "OSX (Clang compiler)"   },
		{ "pnacl",         "Native Client - PNaCl"  },
		{ "qnx-arm",       "QNX/Blackberry - ARM"   },
		{ "rpi",           "RaspberryPi"            },
	},
}

newoption {
	trigger = "vs",
	value = "toolset",
	description = "Choose VS toolset",
	allowed = {
		{ "intel-14",	   "Intel C++ Compiler XE 14.0" },
		{ "intel-15",	   "Intel C++ Compiler XE 15.0" },
		{ "vs2012-clang",  "Clang 3.6"         },
		{ "vs2013-clang",  "Clang 3.6"         },
		{ "vs2012-xp", 	   "Visual Studio 2012 targeting XP" },
		{ "vs2013-xp", 	   "Visual Studio 2013 targeting XP" },
		{ "winphone8",     "Windows Phone 8.0" },
		{ "winphone81",    "Windows Phone 8.1" },
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
	description = "Set Android platform version (default: android-14).",
}

newoption {
	trigger = "with-ios",
	value   = "#",
	description = "Set iOS target version (default: 8.0).",
}

function toolchain(_buildDir, _subDir)

	location (_buildDir .. "projects/" .. _subDir .. "/".. _ACTION)

	local androidPlatform = "android-14"
	if _OPTIONS["with-android"] then
		androidPlatform = "android-" .. _OPTIONS["with-android"]
	end

	local iosPlatform = ""
	if _OPTIONS["with-ios"] then
		iosPlatform = _OPTIONS["with-ios"]
	end

	if _ACTION == "gmake" then

		if nil == _OPTIONS["gcc"] or nil == _OPTIONS["gcc_version"] then
			print("GCC flavor and version must be specified!")
			os.exit(1)
		end

		if "android-arm" == _OPTIONS["gcc"] then

			if not os.getenv("ANDROID_NDK_ARM") or not os.getenv("ANDROID_NDK_ROOT") then
				print("Set ANDROID_NDK_ARM and ANDROID_NDK_ROOT envrionment variables.")
			end

			premake.gcc.cc  = "$(ANDROID_NDK_ARM)/bin/arm-linux-androideabi-gcc"
			premake.gcc.cxx = "$(ANDROID_NDK_ARM)/bin/arm-linux-androideabi-g++"
			premake.gcc.ar  = "$(ANDROID_NDK_ARM)/bin/arm-linux-androideabi-ar"
			location (_buildDir .. "projects/" .. _subDir .. "/".. _ACTION .. "-android-arm")
		end

		if "android-mips" == _OPTIONS["gcc"] then

			if not os.getenv("ANDROID_NDK_MIPS") or not os.getenv("ANDROID_NDK_ROOT") then
				print("Set ANDROID_NDK_MIPS and ANDROID_NDK_ROOT envrionment variables.")
			end

			premake.gcc.cc  = "$(ANDROID_NDK_MIPS)/bin/mipsel-linux-android-gcc"
			premake.gcc.cxx = "$(ANDROID_NDK_MIPS)/bin/mipsel-linux-android-g++"
			premake.gcc.ar  = "$(ANDROID_NDK_MIPS)/bin/mipsel-linux-android-ar"
			location (_buildDir .. "projects/" .. _subDir .. "/".. _ACTION .. "-android-mips")
		end

		if "android-x86" == _OPTIONS["gcc"] then

			if not os.getenv("ANDROID_NDK_X86") or not os.getenv("ANDROID_NDK_ROOT") then
				print("Set ANDROID_NDK_X86 and ANDROID_NDK_ROOT envrionment variables.")
			end

			premake.gcc.cc  = "$(ANDROID_NDK_X86)/bin/i686-linux-android-gcc"
			premake.gcc.cxx = "$(ANDROID_NDK_X86)/bin/i686-linux-android-g++"
			premake.gcc.ar  = "$(ANDROID_NDK_X86)/bin/i686-linux-android-ar"
			location (_buildDir .. "projects/" .. _subDir .. "/".. _ACTION .. "-android-x86")
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

		if "linux-clang" == _OPTIONS["gcc"] then
			premake.gcc.cc  = "clang"
			premake.gcc.cxx = "clang++"
			premake.gcc.ar  = "ar"
			location (_buildDir .. "projects/" .. _subDir .. "/".. _ACTION .. "-linux-clang")
		end

		if "mingw32-gcc" == _OPTIONS["gcc"] then
			if not os.getenv("MINGW32") or not os.getenv("MINGW32") then
				print("Set MINGW32 envrionment variable.")
			end		
			premake.gcc.cc  = "$(MINGW32)/bin/i686-w64-mingw32-gcc"
			premake.gcc.cxx = "$(MINGW32)/bin/i686-w64-mingw32-g++"
			premake.gcc.ar  = "$(MINGW32)/bin/ar"
			location (_buildDir .. "projects/" .. _subDir .. "/".. _ACTION .. "-mingw32-gcc")
		end

		if "mingw64-gcc" == _OPTIONS["gcc"] then
			if not os.getenv("MINGW64") or not os.getenv("MINGW64") then
				print("Set MINGW64 envrionment variable.")
			end				
			premake.gcc.cc  = "$(MINGW64)/bin/x86_64-w64-mingw32-gcc"
			premake.gcc.cxx = "$(MINGW64)/bin/x86_64-w64-mingw32-g++"
			premake.gcc.ar  = "$(MINGW64)/bin/ar"
			location (_buildDir .. "projects/" .. _subDir .. "/".. _ACTION .. "-mingw64-gcc")
		end


		if "mingw-clang" == _OPTIONS["gcc"] then
			premake.gcc.cc   = "$(CLANG)/bin/clang"
			premake.gcc.cxx  = "$(CLANG)/bin/clang++"
			premake.gcc.ar   = "$(CLANG)/bin/llvm-ar"
			premake.gcc.llvm = true
			location (_buildDir .. "projects/" .. _subDir .. "/".. _ACTION .. "-mingw-clang")
		end

		if "nacl" == _OPTIONS["gcc"] then

			if not os.getenv("NACL_SDK_ROOT") then
				print("Set NACL_SDK_ROOT enviroment variables.")
			end

			naclToolchain = "$(NACL_SDK_ROOT)/toolchain/win_x86_newlib/bin/x86_64-nacl-"
			if os.is("macosx") then
				naclToolchain = "$(NACL_SDK_ROOT)/toolchain/mac_x86_newlib/bin/x86_64-nacl-"
			elseif os.is("linux") then
				naclToolchain = "$(NACL_SDK_ROOT)/toolchain/linux_x86_newlib/bin/x86_64-nacl-"
			end

			premake.gcc.cc  = naclToolchain .. "gcc"
			premake.gcc.cxx = naclToolchain .. "g++"
			premake.gcc.ar  = naclToolchain .. "ar"
			location (_buildDir .. "projects/" .. _subDir .. "/".. _ACTION .. "-nacl")
		end

		if "nacl-arm" == _OPTIONS["gcc"] then

			if not os.getenv("NACL_SDK_ROOT") then
				print("Set NACL_SDK_ROOT enviroment variables.")
			end

			naclToolchain = "$(NACL_SDK_ROOT)/toolchain/win_arm_newlib/bin/arm-nacl-"
			if os.is("macosx") then
				naclToolchain = "$(NACL_SDK_ROOT)/toolchain/mac_arm_newlib/bin/arm-nacl-"
			elseif os.is("linux") then
				naclToolchain = "$(NACL_SDK_ROOT)/toolchain/linux_arm_newlib/bin/arm-nacl-"
			end

			premake.gcc.cc  = naclToolchain .. "gcc"
			premake.gcc.cxx = naclToolchain .. "g++"
			premake.gcc.ar  = naclToolchain .. "ar"
			location (_buildDir .. "projects/" .. _subDir .. "/".. _ACTION .. "-nacl-arm")
		end

		if "osx" == _OPTIONS["gcc"] then
			if os.is("linux") then
				local osxToolchain = "x86_64-apple-darwin13-"
				premake.gcc.cc  = osxToolchain .. "clang"
				premake.gcc.cxx = osxToolchain .. "clang++"
				premake.gcc.ar  = osxToolchain .. "ar"
			end
			location (_buildDir .. "projects/" .. _subDir .. "/".. _ACTION .. "-osx")
		end

		if "osx-clang" == _OPTIONS["gcc"] then
			premake.gcc.cc  = "clang"
			premake.gcc.cxx = "clang++"
			premake.gcc.ar  = "ar"
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

		if "qnx-arm" == _OPTIONS["gcc"] then

			if not os.getenv("QNX_HOST") then
				print("Set QNX_HOST enviroment variables.")
			end

			premake.gcc.cc  = "$(QNX_HOST)/usr/bin/arm-unknown-nto-qnx8.0.0eabi-gcc"
			premake.gcc.cxx = "$(QNX_HOST)/usr/bin/arm-unknown-nto-qnx8.0.0eabi-g++"
			premake.gcc.ar  = "$(QNX_HOST)/usr/bin/arm-unknown-nto-qnx8.0.0eabi-ar"
			location (_buildDir .. "projects/" .. _subDir .. "/".. _ACTION .. "-qnx-arm")
		end

		if "rpi" == _OPTIONS["gcc"] then
			location (_buildDir .. "projects/" .. _subDir .. "/".. _ACTION .. "-rpi")
		end
	elseif _ACTION == "vs2012" or _ACTION == "vs2013" or _ACTION == "vs2015" then

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

		if "intel-14" == _OPTIONS["vs"] then
			premake.vstudio.toolset = "Intel C++ Compiler XE 14.0"
			location (_buildDir .. "projects/" .. _subDir .. "/".. _ACTION .. "-intel")
		end

		if "intel-15" == _OPTIONS["vs"] then
			premake.vstudio.toolset = "Intel C++ Compiler XE 15.0"
			location (_buildDir .. "projects/" .. _subDir .. "/".. _ACTION .. "-intel")
		end

		if ("vs2012-xp") == _OPTIONS["vs"] then
			premake.vstudio.toolset = ("v110_xp")
			location (_buildDir .. "projects/" .. _subDir .. "/".. _ACTION .. "-xp")
		end
		
		if ("vs2013-xp") == _OPTIONS["vs"] then
			premake.vstudio.toolset = ("v120_xp")
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
		targetdir (_buildDir .. "win32_" .. _ACTION .. "/bin/x32")
		objdir (_buildDir .. "win32_" .. _ACTION .. "/obj")

	configuration { "x64", "vs*" }
		defines { "_WIN64" }
		targetdir (_buildDir .. "win64_" .. _ACTION .. "/bin/x64")
		objdir (_buildDir .. "win64_" .. _ACTION .. "/obj")

	configuration { "ARM", "vs*" }
		targetdir (_buildDir .. "arm_" .. _ACTION .. "/bin/ARM")
		objdir (_buildDir .. "arm_" .. _ACTION .. "/obj")

	configuration { "x32", "vs*-clang" }
		targetdir (_buildDir .. "win32_" .. _ACTION .. "-clang/bin/x32")
		objdir (_buildDir .. "win32_" .. _ACTION .. "-clang/obj")

	configuration { "x64", "vs*-clang" }
		targetdir (_buildDir .. "win64_" .. _ACTION .. "-clang/bin/x64")
		objdir (_buildDir .. "win64_" .. _ACTION .. "-clang/obj")

	configuration { "vs*-clang" }
		buildoptions {
			"-Qunused-arguments",
		} 
		
	configuration { "mingw*" }
		defines { "WIN32" }

	configuration { "x32", "mingw32-gcc" }
		targetdir (_buildDir .. "win32_mingw-gcc" .. "/bin/x32")
		objdir (_buildDir .. "win32_mingw-gcc" .. "/obj")
		buildoptions { "-m32" }

	configuration { "x64", "mingw64-gcc" }
		targetdir (_buildDir .. "win64_mingw-gcc" .. "/bin/x64")
		objdir (_buildDir .. "win64_mingw-gcc" .. "/obj")
		buildoptions { "-m64" }
		
	configuration { "mingw-clang" }
		linkoptions {
			"-Qunused-arguments",
			"-Wno-error=unused-command-line-argument-hard-error-in-future",
			"-Wl,--allow-multiple-definition",
		}

	configuration { "x32", "mingw-clang" }
		targetdir (_buildDir .. "win32_mingw-clang/bin/x32")
		objdir ( _buildDir .. "win32_mingw-clang/obj")
		buildoptions { "-m32" }
		buildoptions {
			"-isystem$(MINGW32)/i686-w64-mingw32/include/c++",
			"-isystem$(MINGW32)/i686-w64-mingw32/include/c++/i686-w64-mingw32",
			"-isystem$(MINGW32)/i686-w64-mingw32/include",
		}
		
	configuration { "x64", "mingw-clang" }
		targetdir (_buildDir .. "win64_mingw-clang/bin/x64")
		objdir (_buildDir .. "win64_mingw-clang/obj")
		buildoptions { "-m64" }
		buildoptions {
			"-isystem$(MINGW64)/x86_64-w64-mingw32/include/c++",
			"-isystem$(MINGW64)/x86_64-w64-mingw32/include/c++/x86_64-w64-mingw32",
			"-isystem$(MINGW64)/x86_64-w64-mingw32/include",
		}		

	configuration { "linux-gcc", "x32" }
		targetdir (_buildDir .. "linux32_gcc" .. "/bin/x32")
		objdir (_buildDir .. "linux32_gcc" .. "/obj")
		buildoptions {
			"-m32",
		}

	configuration { "linux-gcc", "x64" }
		targetdir (_buildDir .. "linux64_gcc" .. "/bin/x64")
		objdir (_buildDir .. "linux64_gcc" .. "/obj")
		buildoptions {
			"-m64",
		}

	configuration { "linux-clang", "x32" }
		targetdir (_buildDir .. "linux32_clang" .. "/bin/x32")
		objdir (_buildDir .. "linux32_clang" .. "/obj")
		buildoptions {
			"-m32",
		}

	configuration { "linux-clang", "x64" }
		targetdir (_buildDir .. "linux64_clang" .. "/bin/x64")
		objdir (_buildDir .. "linux64_clang" .. "/obj")
		buildoptions {
			"-m64",
		}
		
	configuration { "android-*" }
		includedirs {
			"$(ANDROID_NDK_ROOT)/sources/cxx-stl/gnu-libstdc++/4.8/include",
			"$(ANDROID_NDK_ROOT)/sources/android/native_app_glue",
		}
		linkoptions {
			"-nostdlib",
			"-static-libgcc",
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
			"gnustl_static",
			"gcc",
		}
		buildoptions {
			"-fPIC",
			"-no-canonical-prefixes",
			"-Wa,--noexecstack",
			"-fstack-protector",
			"-ffunction-sections",
			"-Wno-cast-align",
			"-Wno-psabi", -- note: the mangling of 'va_list' has changed in GCC 4.4.0
			"-Wunused-value",
			"-Wundef",
		}
		linkoptions {
			"-no-canonical-prefixes",
			"-Wl,--no-undefined",
			"-Wl,-z,noexecstack",
			"-Wl,-z,relro",
			"-Wl,-z,now",
		}


	configuration { "android-arm" }
		targetdir (_buildDir .. "android-arm" .. "/bin")
		objdir (_buildDir .. "android-arm" .. "/obj")
			libdirs {
				"$(ANDROID_NDK_ROOT)/sources/cxx-stl/gnu-libstdc++/4.8/libs/armeabi-v7a",
				"$(ANDROID_NDK_ROOT)/platforms/" .. androidPlatform .. "/arch-arm/usr/lib",
			}
			includedirs {
				"$(ANDROID_NDK_ROOT)/sources/cxx-stl/gnu-libstdc++/4.8/libs/armeabi-v7a/include",
				"$(ANDROID_NDK_ROOT)/platforms/" .. androidPlatform .. "/arch-arm/usr/include",
			}
			buildoptions {
				"-mthumb",
				"-march=armv7-a",
				"-mfloat-abi=softfp",
				"-mfpu=neon",
				"-Wunused-value",
				"-Wundef",
			}
			linkoptions {
				"$(ANDROID_NDK_ROOT)/platforms/" .. androidPlatform .. "/arch-arm/usr/lib/crtbegin_so.o",
				"$(ANDROID_NDK_ROOT)/platforms/" .. androidPlatform .. "/arch-arm/usr/lib/crtend_so.o",
				"-march=armv7-a",
				"-Wl,--fix-cortex-a8",
			}

	configuration { "android-mips" }
		targetdir (_buildDir .. "android-mips" .. "/bin")
		objdir (_buildDir .. "android-mips" .. "/obj")
		libdirs {
			"$(ANDROID_NDK_ROOT)/sources/cxx-stl/gnu-libstdc++/4.8/libs/mips",
			"$(ANDROID_NDK_ROOT)/platforms/" .. androidPlatform .. "/arch-mips/usr/lib/",
		}
		includedirs {
			"$(ANDROID_NDK_ROOT)/sources/cxx-stl/gnu-libstdc++/4.8/libs/mips/include",
			"$(ANDROID_NDK_ROOT)/platforms/" .. androidPlatform .. "/arch-mips/usr/include",
		}
		buildoptions {
			"-Wunused-value",
			"-Wundef",
		}
		linkoptions {
			"$(ANDROID_NDK_ROOT)/platforms/" .. androidPlatform .. "/arch-mips/usr/lib/crtbegin_so.o",
			"$(ANDROID_NDK_ROOT)/platforms/" .. androidPlatform .. "/arch-mips/usr/lib/crtend_so.o",
		}

	configuration { "android-x86" }
		targetdir (_buildDir .. "android-x86" .. "/bin")
		objdir (_buildDir .. "android-x86" .. "/obj")
		libdirs {
			"$(ANDROID_NDK_ROOT)/sources/cxx-stl/gnu-libstdc++/4.8/libs/x86",
			"$(ANDROID_NDK_ROOT)/platforms/" .. androidPlatform .. "/arch-x86/usr/lib",
		}
		includedirs {
			"$(ANDROID_NDK_ROOT)/sources/cxx-stl/gnu-libstdc++/4.8/libs/x86/include",
			"$(ANDROID_NDK_ROOT)/platforms/" .. androidPlatform .. "/arch-x86/usr/include",
		}
		buildoptions {
			"-march=i686",
			"-mtune=atom",
			"-mstackrealign",
			"-msse3",
			"-mfpmath=sse",
			"-Wunused-value",
			"-Wundef",
		}
		linkoptions {
			"$(ANDROID_NDK_ROOT)/platforms/" .. androidPlatform .. "/arch-x86/usr/lib/crtbegin_so.o",
			"$(ANDROID_NDK_ROOT)/platforms/" .. androidPlatform .. "/arch-x86/usr/lib/crtend_so.o",
		}


	configuration { "asmjs" }
		targetdir (_buildDir .. "asmjs" .. "/bin")
		objdir (_buildDir .. "asmjs" .. "/obj")
		buildoptions {
			"-isystem$(EMSCRIPTEN)/system/include",
			"-isystem$(EMSCRIPTEN)/system/include/compat",
			"-isystem$(EMSCRIPTEN)/system/include/libc",
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

	configuration { "freebsd" }
		targetdir (_buildDir .. "freebsd" .. "/bin")
		objdir (_buildDir .. "freebsd" .. "/obj")

	configuration { "nacl or nacl-arm or pnacl" }
		buildoptions {
			"-U__STRICT_ANSI__", -- strcasecmp, setenv, unsetenv,...
			"-fno-stack-protector",
			"-fdiagnostics-show-option",
			"-fdata-sections",
			"-ffunction-sections",
			"-Wunused-value",			
		}
	configuration { "nacl or nacl-arm" }
		includedirs {
			"$(NACL_SDK_ROOT)/include",
			"$(NACL_SDK_ROOT)/include/newlib",
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

	configuration { "x32", "nacl" }
		targetdir (_buildDir .. "nacl-x86" .. "/bin/x32")
		objdir (_buildDir .. "nacl-x86" .. "/obj")

	configuration { "x32", "nacl", "Debug" }
		libdirs { "$(NACL_SDK_ROOT)/lib/newlib_x86_32/Debug" }

	configuration { "x32", "nacl", "Release" }
		libdirs { "$(NACL_SDK_ROOT)/lib/newlib_x86_32/Release" }

	configuration { "x64", "nacl" }
		targetdir (_buildDir .. "nacl-x64" .. "/bin/x64")
		objdir (_buildDir .. "nacl-x64" .. "/obj")

	configuration { "x64", "nacl", "Debug" }
		libdirs { "$(NACL_SDK_ROOT)/lib/newlib_x86_64/Debug" }

	configuration { "x64", "nacl", "Release" }
		libdirs { "$(NACL_SDK_ROOT)/lib/newlib_x86_64/Release" }

	configuration { "nacl-arm" }
		targetdir (_buildDir .. "nacl-arm" .. "/bin")
		objdir (_buildDir .. "nacl-arm" .. "/obj")

	configuration { "nacl-arm", "Debug" }
		libdirs { "$(NACL_SDK_ROOT)/lib/newlib_arm/Debug" }

	configuration { "nacl-arm", "Release" }
		libdirs { "$(NACL_SDK_ROOT)/lib/newlib_arm/Release" }

	configuration { "pnacl" }
		targetdir (_buildDir .. "pnacl" .. "/bin")
		objdir (_buildDir .. "pnacl" .. "/obj")

	configuration { "pnacl", "Debug" }
		libdirs { "$(NACL_SDK_ROOT)/lib/pnacl/Debug" }

	configuration { "pnacl", "Release" }
		libdirs { "$(NACL_SDK_ROOT)/lib/pnacl/Release" }

	configuration { "osx*", "x32" }
		targetdir (_buildDir .. "osx32_clang" .. "/bin")
		objdir (_buildDir .. "osx32_clang" .. "/obj")
		buildoptions {
			"-m32",
		}

	configuration { "osx*", "x64" }
		targetdir (_buildDir .. "osx64_clang" .. "/bin")
		objdir (_buildDir .. "osx64_clang" .. "/obj")
		buildoptions {
			"-m64",
		}

	configuration { "ios-arm" }
		targetdir (_buildDir .. "ios-arm" .. "/bin")
		objdir (_buildDir .. "ios-arm" .. "/obj")

	configuration { "ios-simulator" }
		targetdir (_buildDir .. "ios-simulator" .. "/bin")
		objdir (_buildDir .. "ios-simulator" .. "/obj")

	configuration { "qnx-arm" }
		targetdir (_buildDir .. "qnx-arm" .. "/bin")
		objdir (_buildDir .. "qnx-arm" .. "/obj")

	configuration { "rpi" }
		targetdir (_buildDir .. "rpi" .. "/bin")
		objdir (_buildDir .. "rpi" .. "/obj")

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

	configuration { "linux-* or rpi", "Release" }
		postbuildcommands {
			"$(SILENT) echo Stripping symbols.",
			"$(SILENT) strip -s \"$(TARGET)\""
		}

	configuration { "mingw*", "x64", "Release" }
		postbuildcommands {
			"$(SILENT) echo Stripping symbols.",
			"$(SILENT) $(MINGW64)/bin/strip -s \"$(TARGET)\"",
		}

	configuration { "mingw*", "x32", "Release" }
		postbuildcommands {
			"$(SILENT) echo Stripping symbols.",
			"$(SILENT) $(MINGW32)/bin/strip -s \"$(TARGET)\""
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
			"$(SILENT) $(EMSCRIPTEN)/emcc -O2 -s TOTAL_MEMORY=268435456 \"$(TARGET)\" -o \"$(TARGET)\".html"
			-- ALLOW_MEMORY_GROWTH
		}

	configuration {} -- reset configuration
end

