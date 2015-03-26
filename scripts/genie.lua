premake.check_paths = true
premake.make.override = { "TARGET" }
MAME_DIR = (path.getabsolute("..") .. "/")
local MAME_BUILD_DIR = (MAME_DIR .. "build/")
local naclToolchain = ""


function includeosd()
	includedirs {
		MAME_DIR .. "src/osd",
	}
	if _OPTIONS["osd"]=="windows" then
		includedirs {
			MAME_DIR .. "src/osd/windows",
		}
	else
		includedirs {
			MAME_DIR .. "src/osd/sdl",
		}
	end
end

function str_to_version (str)
	local val = 0
	if (str == nil or str == '') then
		return val
	end
	local cnt = 10000
	for word in string.gmatch(str, '([^.]+)') do
		val = val + tonumber(word) * cnt
		cnt = cnt / 100
	end
    return val
end

CPUS = {}
SOUNDS  = {}
MACHINES  = {}
VIDEOS = {}
BUSES  = {}

newoption {
	trigger = "with-tools",
	description = "Enable building tools.",
} 

newoption {
	trigger = "osd",
	description = "Choose target OSD",
	allowed = {
		{ "osdmini",   "mini dummy OSD"         },
		{ "sdl",       "SDL"			        },
		{ "windows",   "Windows"                },
	},
}

newoption {
	trigger = "targetos",
	description = "Choose target OS",
	allowed = {
		{ "android",   "Android"          		},
		{ "asmjs",     "Emscripten/asm.js"      },
		{ "freebsd",   "FreeBSD"                },
		{ "linux",     "Linux"   				},
		{ "ios",       "iOS"              		},
		{ "nacl",      "Native Client"          },
		{ "macosx",    "OSX"                    },
		{ "windows",   "Windows"                },
	},
}

newoption {
	trigger = "distro",
	description = "Choose distribution",
	allowed = {
		{ "generic", 		   "generic"         	},
		{ "debian-stable",     "debian-stable"      },
		{ "ubuntu-intrepid",   "ubuntu-intrepid"    },
		{ "gcc44-generic",     "gcc44-generic"   	},
		{ "gcc45-generic",     "gcc45-generic"     	},
		{ "gcc46-generic",     "gcc46-generic" 		},
		{ "gcc47-generic",     "gcc47-generic"      },
	},
}

newoption {
	trigger = "target",
	description = "Building target",
}

newoption {
	trigger = "subtarget",
	description = "Building subtarget",
}

newoption {
	trigger = "gcc_version",
	description = "GCC compiler version",
}

newoption {
	trigger = "os_version",
	description = "OS version",
	value = "",
}

newoption {
	trigger = "CC",
	description = "CC replacement",
}

newoption {
	trigger = "CXX",
	description = "CXX replacement",
}

newoption {
	trigger = "LD",
	description = "LD replacement",
}

newoption {
	trigger = "PROFILE",
	description = "Enable profiling.",
} 

newoption {
	trigger = "SYMBOLS",
	description = "Enable symbols.",
} 

newoption {
	trigger = "SYMLEVEL",
	description = "Symbols level.",
} 

newoption {
	trigger = "PROFILER",
	description = "Include the internal profiler.",
} 

newoption {
	trigger = "OPTIMIZE",
	description = "Optimization level.",
} 

newoption {
	trigger = "ARCHOPTS",
	description = "ARCHOPTS.",
} 

newoption {
	trigger = "MAP",
	description = "Generate a link map.",
} 

newoption {
	trigger = "FORCE_DRC_C_BACKEND",
	description = "Force DRC C backend.",
} 


local os_version = str_to_version(_OPTIONS["os_version"])
USE_BGFX = 1
if (_OPTIONS["targetos"]=="macosx" and  os_version < 100700) then
	USE_BGFX = 0
end
GEN_DIR = MAME_BUILD_DIR .. "generated/"

if (_OPTIONS["target"] == nil) then return false end
if (_OPTIONS["subtarget"] == nil) then return false end

if (_OPTIONS["target"] == _OPTIONS["subtarget"]) then
	solution (_OPTIONS["target"])
else
	solution (_OPTIONS["target"] .. _OPTIONS["subtarget"])
end	
	configurations {
		"Debug",
		"Release",
	}

	platforms {
		"x32",
		"x64",
		"Native", -- for targets where bitness is not specified
	}

	language "C++"

	flags {
		"StaticRuntime",
		"Unicode",
		"NoPCH",
	}
	
	configuration { "vs*" }
	flags {
		"ExtraWarnings",
		"FatalWarnings",
	}
	configuration { "Debug", "vs*" }
		flags {
			"Symbols",
		}	
	configuration {}
	
msgcompile ("Compiling $(subst ../../../,,$<)...")

msgcompile_objc ("Objective-C compiling $(subst ../../../,,$<)...")

msgresource ("Compiling resources $(subst ../../../,,$<)...")

msglinking ("Linking $(notdir $@)...")

msgarchiving ("Archiving $(notdir $@)...")

messageskip { "SkipCreatingMessage", "SkipBuildingMessage", "SkipCleaningMessage" }

if (not os.isfile(path.join("target", _OPTIONS["target"],_OPTIONS["subtarget"] .. ".lua"))) then
	error("File definition for TARGET=" .. _OPTIONS["target"] .. " SUBTARGET=" .. _OPTIONS["subtarget"] .. " does not exist")
end
dofile (path.join("target", _OPTIONS["target"],_OPTIONS["subtarget"] .. ".lua"))
	
configuration { "gmake" }
	flags {
		"SingleOutputDir",
	}

configuration { "x64", "Release" }
	targetsuffix "64"
	if _OPTIONS["PROFILE"] then
		targetsuffix "64p"
	end

configuration { "x64", "Debug" }
	targetsuffix "64d"
	if _OPTIONS["PROFILE"] then
		targetsuffix "64dp"
	end
	
configuration { "x32", "Release" }
	targetsuffix ""
	if _OPTIONS["PROFILE"] then
		targetsuffix "p"
	end
	
configuration { "x32", "Debug" }
	targetsuffix "d"
	if _OPTIONS["PROFILE"] then
		targetsuffix "dp"
	end
	
configuration { "Native", "Release" }
	targetsuffix ""
	if _OPTIONS["PROFILE"] then
		targetsuffix "p"
	end

configuration { "Native", "Debug" }
	targetsuffix "d"
	if _OPTIONS["PROFILE"] then
		targetsuffix "dp"
	end

configuration { }

dofile ("toolchain.lua")


if _OPTIONS["osd"]=="windows" then
	forcedincludes {
		MAME_DIR .. "src/osd/windows/winprefix.h"
	}
elseif _OPTIONS["osd"]=="sdl" then
	forcedincludes {
		MAME_DIR .. "src/osd/sdl/sdlprefix.h"
	}
end

-- Avoid error when invoking genie --help.
if (_ACTION == nil) then return false end

-- define PTR64 if we are a 64-bit target
configuration { "x64" }
	defines { "PTR64=1" }

-- map the INLINE to something digestible by GCC
configuration { "gmake" }
	buildoptions_cpp {
		"-DINLINE=\"static inline\"",
	}
	buildoptions_objc {
		"-DINLINE=\"static inline\"",
	}
configuration { "vs*" }
	defines {
		"INLINE=static inline",
	}

-- define MAME_DEBUG if we are a debugging build
configuration { "Debug" }
	defines {
		"MAME_DEBUG",
	}
	if _OPTIONS["PROFILER"] then
		defines{
			"MAME_PROFILER", -- define MAME_PROFILER if we are a profiling build
		}
	end

configuration { "Release" }
	defines {
		"NDEBUG",
	}

configuration { }
	-- CR/LF setup: use both on win32/os2, CR only on everything else
	if _OPTIONS["targetos"]=="windows" or _OPTIONS["targetos"]=="os2" then
		defines {
			"CRLF=3",
		}
	else
		defines {
			"CRLF=2",
		}
	end


	-- define LSB_FIRST if we are a little-endian target
	defines {
		"LSB_FIRST",
	}

	-- define USE_NETWORK if networking is enabled (not OS/2 and hasn't been disabled)
	if not _OPTIONS["targetos"]=="os2" then
		defines {
			"USE_NETWORK",
		}
	end
	-- need to ensure FLAC functions are statically linked
	defines {
		"FLAC__NO_DLL",
	}

	-- fixme -- need to make this work for other target architectures (PPC)
	if _OPTIONS["FORCE_DRC_C_BACKEND"]==nil then
		configuration { "x64" }
			defines {
				"NATIVE_DRC=drcbe_x64",
			}
		configuration { "x32" }
			defines {
				"NATIVE_DRC=drcbe_x86",
			}
		configuration {  }
	end
	
-- define USE_SYSTEM_JPEGLIB if library shipped with MAME is not used
--ifneq ($(BUILD_JPEGLIB),1)
--DEFS += -DUSE_SYSTEM_JPEGLIB
--endif

--ifdef FASTDEBUG
--DEFS += -DMAME_DEBUG_FAST
--endif

	--To support casting in Lua 5.3
	defines {
		"LUA_COMPAT_APIINTCASTS",
	}

	if _ACTION == "gmake" then

	--we compile C-only to C89 standard with GNU extensions
	buildoptions_c {
		"-std=gnu89",

	}
	--we compile C++ code to C++98 standard with GNU extensions
	buildoptions_cpp {
		"-x c++",
		"-std=gnu++98",
	}

	buildoptions_objc {
		"-x objective-c++",
	}
--ifdef CPP11
--CPPONLYFLAGS += -x c++ -std=gnu++11
--else

-- this speeds it up a bit by piping between the preprocessor/compiler/assembler
	if not ("pnacl" == _OPTIONS["gcc"]) then
		buildoptions {
			"--pipe",
		}
	end
-- add -g if we need symbols, and ensure we have frame pointers
if _OPTIONS["SYMBOLS"]~=nil then
	buildoptions {
		"-g" .. _OPTIONS["SYMLEVEL"],
		"-fno-omit-frame-pointer",
		"-fno-optimize-sibling-calls",
	}
end

--# we need to disable some additional implicit optimizations for profiling
if _OPTIONS["PROFILE"] then
	buildoptions {
		"-mno-omit-leaf-frame-pointer",
	}
end
-- add -v if we need verbose build information
if _OPTIONS["VERBOSE"] then
	buildoptions {
		"-v",
	}
end

-- only show deprecation warnings when enabled
--ifndef DEPRECATED
	buildoptions {
		"-Wno-deprecated-declarations"
	}
--endif

-- add profiling information for the compiler
if _OPTIONS["PROFILE"] then
	buildoptions {
		"-pg",
	}
	linkoptions {
		"-pg",
	}
end

if _OPTIONS["SYMBOLS"]~=nil then
	flags {
		"Symbols",
	}	
end

--# add the optimization flag
	buildoptions {
		"-O".. _OPTIONS["OPTIMIZE"],
		"-fno-strict-aliasing"
	}

	-- add the error warning flag
	--ifndef NOWERROR
	buildoptions {
		"-Werror",
	}


-- if we are optimizing, include optimization options
--ifneq ($(),0)
if _OPTIONS["OPTIMIZE"] then
	buildoptions {
		"-fno-strict-aliasing" 
	}
	if _OPTIONS["ARCHOPTS"] then	
		buildoptions {
			_OPTIONS["ARCHOPTS"]
		}
	end
--ifdef LTO
--CCOMFLAGS += -flto
--endif
end

--ifdef SSE2
--CCOMFLAGS += -msse2
--endif

--ifdef OPENMP
--CCOMFLAGS += -fopenmp
--else
--CCOMFLAGS += -Wno-unknown-pragmas
--endif

if _OPTIONS["MAP"] then
	if (_OPTIONS["target"] == _OPTIONS["subtarget"]) then
		linkoptions {
			"-Wl,-Map," .. "../../../" .. _OPTIONS["target"] .. ".map"
		}
	else
		linkoptions {
			"-Wl,-Map," .. "../../../"  .. _OPTIONS["target"] .. _OPTIONS["subtarget"] .. ".map"
		}

	end	
end
	buildoptions {
		"-Wno-unknown-pragmas",
	}
-- add a basic set of warnings
	buildoptions {
		"-Wall",
		"-Wcast-align",
		"-Wundef",
		"-Wformat-security",
		"-Wwrite-strings",
		"-Wno-sign-compare",
		"-Wno-conversion",
	}
-- warnings only applicable to C compiles
	buildoptions_c {
		"-Wpointer-arith",
		"-Wbad-function-cast",
		"-Wstrict-prototypes",
	}

-- warnings only applicable to OBJ-C compiles
	buildoptions_objc {
		"-Wpointer-arith",
	}

-- warnings only applicable to C++ compiles
	buildoptions_cpp {
		"-Woverloaded-virtual",
	}

--ifdef SANITIZE
--CCOMFLAGS += -fsanitize=$(SANITIZE)

--ifneq (,$(findstring thread,$(SANITIZE)))
--CCOMFLAGS += -fPIE
--endif
--endif


		
		local version = str_to_version(_OPTIONS["gcc_version"])
		if string.find(_OPTIONS["gcc"], "clang") then
			buildoptions {
				"-Wno-cast-align",
				"-Wno-tautological-compare",
				"-Wno-dynamic-class-memaccess",
				"-Wno-self-assign-field",
			}
			
			if (version >= 30400) then
				buildoptions {
					"-Wno-inline-new-delete",
					"-Wno-constant-logical-operand",
				}
			end
			if (version >= 30500) then
				buildoptions {
					"-Wno-absolute-value",
					"-Wno-unknown-warning-option",
					"-Wno-extern-c-compat",
				}
			end
		else
			if (version >= 40400) then
				buildoptions {
					"-Wno-unused-result",
				}
			end 

			if (version >= 40700) then
				buildoptions {
					"-Wno-narrowing",
					"-Wno-attributes"
				}
			end
			if (version >= 40800) then
				-- array bounds checking seems to be buggy in 4.8.1 (try it on video/stvvdp1.c and video/model1.c without -Wno-array-bounds)
				buildoptions {
					"-Wno-unused-variable",
					"-Wno-array-bounds"
				}
			end
		end
	end
--ifeq ($(findstring arm,$(UNAME)),arm)
--	CCOMFLAGS += -Wno-cast-align
--endif

if not toolchain(MAME_BUILD_DIR) then
	return -- no action specified
end
	
configuration { "asmjs" }
	buildoptions {
		"-std=gnu89",
		"-Wno-implicit-function-declaration",
	}
	buildoptions_cpp {
		"-x c++",
		"-std=gnu++98",
	}
	archivesplit_size "20"

configuration { "android*" }
	buildoptions_cpp {
		"-x c++",
		"-std=gnu++98",
	}
	archivesplit_size "20"

configuration { "pnacl" }
	buildoptions {
		"-std=gnu89",
		"-Wno-inline-new-delete",
	}
	buildoptions_cpp {
		"-x c++",
		"-std=gnu++98",
	}
	archivesplit_size "20"

configuration { "nacl*" }
	buildoptions_cpp {
		"-x c++",
		"-std=gnu++98",
	}
	archivesplit_size "20"

configuration { "linux-*" }
		linkoptions {
			"`sdl2-config --libs`",
		}
		links {
			"pthread",
			"SDL2",
			"SDL2_ttf",
			"asound",
			"dl",
			"fontconfig",
			"freetype",
			"GL",
			"m",
			"util",
			"X11",
			"Xinerama",
		}
		defines 
		{
			"DISTRO=" .. _OPTIONS["distro"] ,
		}
		if _OPTIONS["distro"]=="debian-stable" then
			defines 
			{
				"NO_AFFINITY_NP",
			}
		end


configuration { "osx*" }
		links {
			"Cocoa.framework",
			"OpenGL.framework",
			"CoreAudio.framework",
			"CoreMIDI.framework",
			"SDL2.framework",
			"pthread",
		}


configuration { "mingw*" }
		defines {
			"main=utf8_main",
		}
		linkoptions {
			"-static-libgcc",
			"-static-libstdc++",
			"-municode",
		}
		if _OPTIONS["osd"]=="sdl" then
			linkoptions {
				"-Wl,--allow-multiple-definition",
				"-static"
			}
			links {
				"opengl32",
				"SDL2",
				"Imm32",
				"version",
				"ole32",
				"oleaut32",
			}
		end
		links {
			"user32",
			"gdi32",
			"dsound",
			"dxguid",
			"winmm",
			"advapi32",
			"comctl32",
			"shlwapi",
			"wsock32",
			"dinput8",
			"comdlg32",
		}

configuration { "vs*" }
		defines {
			"main=utf8_main",
		}
		defines {
			"XML_STATIC",
			"WIN32",
			"_WIN32",
			"_CRT_NONSTDC_NO_DEPRECATE",
			"_CRT_SECURE_NO_DEPRECATE",
		}
		links {
			"user32",
			"gdi32",
			"dsound",
			"dxguid",
			"winmm",
			"advapi32",
			"comctl32",
			"shlwapi",
			"wsock32",
			"dinput8",
			"comdlg32",
		}

		buildoptions {
			"/wd4025",
			"/wd4003",
			"/wd4018",
			"/wd4061",
			"/wd4100",
			"/wd4127",
			"/wd4131",
			"/wd4141",
			"/wd4146",
			"/wd4150",
			"/wd4189",
			"/wd4191",
			"/wd4201",
			"/wd4232",
			"/wd4242",
			"/wd4244",
			"/wd4250",
			"/wd4255",
			"/wd4296",
			"/wd4306",
			"/wd4310",
			"/wd4312",
			"/wd4324",
			"/wd4347",
			"/wd4435",
			"/wd4510",
			"/wd4512",
			"/wd4514",
			"/wd4571",
			"/wd4610",
			"/wd4611",
			"/wd4619",
			"/wd4625",
			"/wd4626",
			"/wd4640",
			"/wd4668",
			"/wd4702",
			"/wd4706",
			"/wd4710",
			"/wd4711",
			"/wd4805",
			"/wd4820",
			"/wd4826",
			"/wd4365",
			"/wd4389",
			"/wd4245",
			"/wd4388",
			"/wd4267",
			"/wd4005",
			"/wd4350",
			"/wd4996",
			"/wd4191",
			"/wd4060",
			"/wd4065",
			"/wd4640",
			"/wd4290",
			"/wd4355",
			"/wd4800",
			"/wd4371",
			"/wd4548",
		}
		linkoptions {
			"/ignore:4221", -- LNK4221: This object file does not define any previously undefined public symbols, so it will not be used by any link operation that consumes this library
		}
		includedirs {
			MAME_DIR .. "3rdparty/dxsdk/Include"
		}

configuration { "x32", "vs*" }
		libdirs {
			MAME_DIR .. "3rdparty/dxsdk/lib/x86",
		}

configuration { "x64", "vs*" }
		libdirs {
			MAME_DIR .. "3rdparty/dxsdk/lib/x64",
		}

configuration { }

group "libs"
dofile(path.join("src", "3rdparty.lua"))
dofile(path.join("src", "lib.lua"))

group "core"

dofile(path.join("src", "osd.lua"))
dofile(path.join("src", "emu.lua"))
emuProject(_OPTIONS["target"],_OPTIONS["subtarget"])

group "drivers"
createProjects(_OPTIONS["target"],_OPTIONS["subtarget"])
group "emulator"
dofile(path.join("src", "main.lua"))
if (_OPTIONS["target"] == _OPTIONS["subtarget"]) then
	startproject (_OPTIONS["target"])
else
	startproject (_OPTIONS["target"] .. _OPTIONS["subtarget"])
end	
mainProject(_OPTIONS["target"],_OPTIONS["subtarget"])

if _OPTIONS["with-tools"] then
	group "tools"
	dofile(path.join("src", "tools.lua"))
end

if (_ACTION == "gmake" and _OPTIONS["gcc"]=='asmjs') then 
	strip()
end
