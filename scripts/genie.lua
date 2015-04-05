premake.check_paths = true
premake.make.linkoptions_after = true
premake.make.override = { "TARGET" }
MAME_DIR = (path.getabsolute("..") .. "/")
local MAME_BUILD_DIR = (MAME_DIR .. "build/")
local naclToolchain = ""


function backtick(cmd)
	result = string.gsub(string.gsub(os.outputof(cmd), "\r?\n$", ""), " $", "")
	return result
end

function str_to_version(str)
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

function findfunction(x)
  assert(type(x) == "string")
  local f=_G
  for v in x:gmatch("[^%.]+") do
    if type(f) ~= "table" then
       return nil, "looking for '"..v.."' expected table, not "..type(f)
    end
    f=f[v]
  end
  if type(f) == "function" then
    return f
  else
    return nil, "expected function, not "..type(f)
  end
end

function layoutbuildtask(_folder, _name)
	return { MAME_DIR .. "src/".._folder.."/".. _name ..".lay" ,    GEN_DIR .. _folder .. "/".._name..".lh",    {  MAME_DIR .. "src/build/file2str.py" }, {"@echo Converting src/".._folder.."/".._name..".lay...",    "python $(1) $(<) $(@) layout_".._name }};
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
	description = "Choose OSD layer implementation",
}

newoption {
	trigger = "targetos",
	description = "Choose target OS",
	allowed = {
		{ "android-arm",   "Android - ARM"          },
		{ "android-mips",  "Android - MIPS"         },
		{ "android-x86",   "Android - x86"          },
		{ "asmjs",         "Emscripten/asm.js"      },
		{ "freebsd",       "FreeBSD"                },
		{ "netbsd",        "NetBSD"                 },
		{ "openbsd",       "OpenBSD"                },
		{ "nacl",          "Native Client"          },
		{ "nacl-arm",      "Native Client - ARM"    },
		{ "pnacl",         "Native Client - PNaCl"  },
		{ "linux",     	   "Linux"                  },
		{ "ios",           "iOS"                    },
		{ "macosx",        "OSX"                    },
		{ "windows",       "Windows"                },
		{ "os2",           "OS/2 eComStation"       },
		{ "haiku",         "Haiku"                  },
	},
}

newoption {
	trigger = "distro",
	description = "Choose distribution",
	allowed = {
		{ "generic", 		   "generic"         	},
		{ "debian-stable",     "debian-stable"      },
		{ "ubuntu-intrepid",   "ubuntu-intrepid"    },
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
	trigger = "LDOPTS",
	description = "Additional linker options",
}

newoption {
	trigger = "MAP",
	description = "Generate a link map.",
}

newoption {
	trigger = "NOASM",
	description = "Disable implementations based on assembler code",
	allowed = {
		{ "0",  "Enable assembler code"   },
		{ "1",  "Disable assembler code"  },
	},
}

newoption {
	trigger = "BIGENDIAN",
	description = "Build for big endian target",
	allowed = {
		{ "0",  "Little endian target"   },
		{ "1",  "Big endian target"  },
	},
}

newoption {
	trigger = "FORCE_DRC_C_BACKEND",
	description = "Force DRC C backend.",
}

newoption {
	trigger = "NOWERROR",
	description = "NOWERROR",
}

newoption {
	trigger = "USE_BGFX",
	description = "Use of BGFX.",
	allowed = {
		{ "0",   "Disabled" 	},
		{ "1",   "Enabled"      },
	}
}

if not _OPTIONS["BIGENDIAN"] then
	_OPTIONS["BIGENDIAN"] = "0"
end

if not _OPTIONS["NOASM"] then
	if _OPTIONS["targetos"]=="emscripten" then
		_OPTIONS["NOASM"] = "1"
	else
		_OPTIONS["NOASM"] = "0"
	end
end

if _OPTIONS["NOASM"]=="1" and not _OPTIONS["FORCE_DRC_C_BACKEND"] then
	_OPTIONS["FORCE_DRC_C_BACKEND"] = "1"
end

USE_BGFX = 1
if(_OPTIONS["USE_BGFX"]~=nil) then
	USE_BGFX = tonumber(_OPTIONS["USE_BGFX"])
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
	"NoPCH",
}

configuration { "vs*" }
	flags {
		"ExtraWarnings",
	}
	if not _OPTIONS["NOWERROR"] then
		flags{
			"FatalWarnings",
		}
	end


configuration { "Debug", "vs*" }
	flags {
		"Symbols",
	}
	
configuration { "Release", "vs*" }
	flags {
		"Optimize",
	}

configuration {}

local AWK = ""
if (os.is("windows")) then
	AWK_TEST = backtick("awk --version 2> NUL")
	if (AWK_TEST~='') then
		AWK = "awk"
	else
		AWK_TEST = backtick("gawk --version 2> NUL")
		if (AWK_TEST~='') then
			AWK = "gawk"
		end
	end
else
	AWK_TEST = backtick("awk --version 2> /dev/null")
	if (AWK_TEST~='') then
		AWK = "awk"
	else
		AWK_TEST = backtick("gawk --version 2> /dev/null")
		if (AWK_TEST~='') then
			AWK = "gawk"
		end
	end
end

if (AWK~='') then
	postcompiletasks {
		AWK .. " -f ../../../../../scripts/depfilter.awk $(@:%.o=%.d) > $(@:%.o=%.dep)",
		"mv $(@:%.o=%.dep) $(@:%.o=%.d)",
	}
end

msgcompile ("Compiling $(subst ../,,$<)...")

msgcompile_objc ("Objective-C compiling $(subst ../,,$<)...")

msgresource ("Compiling resources $(subst ../,,$<)...")

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


if _OPTIONS["targetos"]=="windows" then
	configuration { "x64" }
		defines {
			"X64_WINDOWS_ABI",
		}
	configuration { }
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
configuration { "xcode4*" }
	buildoptions {
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
	if _OPTIONS["PROFILER"]=="1" then
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


if _OPTIONS["BIGENDIAN"]=="1" then
	if _OPTIONS["targetos"]=="macosx" then
		defines {
			"OSX_PPC",
		}
		buildoptions {
			"-Wno-unused-label",
		}
		if _OPTIONS["SYMBOLS"] then
			buildoptions {
				"-mlong-branch",
			}
		end
		configuration { "x64" }
			buildoptions {
				"-arch ppc64",
			}
			linkoptions {
				"-arch ppc64",
			}
		configuration { "x32" }
			buildoptions {
				"-arch ppc",
			}
			linkoptions {
				"-arch ppc",
			}
		configuration { }
	end
else
	defines {
		"LSB_FIRST",
	}
	if _OPTIONS["targetos"]=="macosx" then
		configuration { "x64" }
			buildoptions {
				"-arch x86_64",
			}
			linkoptions {
				"-arch x86_64",
			}
		configuration { "x32" }
			buildoptions {
				"-arch i386",
			}
			linkoptions {
				"-arch i386",
			}
	end
end

-- need to ensure FLAC functions are statically linked
defines {
	"FLAC__NO_DLL",
}

if _OPTIONS["NOASM"]=="1" then
	defines {
		"MAME_NOASM"
	}
end

if not _OPTIONS["FORCE_DRC_C_BACKEND"] then
	if _OPTIONS["BIGENDIAN"]~="1" then
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
if _OPTIONS["NOWERROR"]==nil then
	buildoptions {
		"-Werror",
	}
end

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

if _OPTIONS["LDOPTS"] then
		linkoptions {
			_OPTIONS["LDOPTS"]
		}
end

if _OPTIONS["MAP"] then
	if (_OPTIONS["target"] == _OPTIONS["subtarget"]) then
		linkoptions {
			"-Wl,-Map," .. "../../../../" .. _OPTIONS["target"] .. ".map"
		}
	else
		linkoptions {
			"-Wl,-Map," .. "../../../../"  .. _OPTIONS["target"] .. _OPTIONS["subtarget"] .. ".map"
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
			if (version >= 30200) then
				buildoptions {
					"-Wno-unused-value",
				}
			end
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
			if (version == 40201) then
				buildoptions {
					"-Wno-cast-align"
				}
			end
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

local subdir
if (_OPTIONS["target"] == _OPTIONS["subtarget"]) then
	subdir = _OPTIONS["osd"] .. "/" .. _OPTIONS["target"]
else
	subdir = _OPTIONS["osd"] .. "/" .. _OPTIONS["target"] .. _OPTIONS["subtarget"]
end

if not toolchain(MAME_BUILD_DIR, subdir) then
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
		links {
			"dl",
		}
		if _OPTIONS["distro"]=="debian-stable" then
			defines
			{
				"NO_AFFINITY_NP",
			}
		end


configuration { "osx*" }
		links {
			"pthread",
		}
		flags {
			"Symbols",
		}


configuration { "mingw*" }
		linkoptions {
			"-static-libgcc",
			"-static-libstdc++",
		}
		links {
			"user32",
			"winmm",
			"advapi32",
			"shlwapi",
			"wsock32",
		}

configuration { "vs*" }
		defines {
			"XML_STATIC",
			"WIN32",
			"_WIN32",
			"_CRT_NONSTDC_NO_DEPRECATE",
			"_CRT_SECURE_NO_DEPRECATE",
		}
		links {
			"user32",
			"winmm",
			"advapi32",
			"shlwapi",
			"wsock32",
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
if _OPTIONS["vs"]=="intel-15" then
		buildoptions {
			"/Qwd9",
			"/Qwd82",
			"/Qwd111",
			"/Qwd128",
			"/Qwd177",
			"/Qwd181",
			"/Qwd185",
			"/Qwd280",
			"/Qwd344",
			"/Qwd411",
			"/Qwd869",
			"/Qwd2545",
			"/Qwd2553",
			"/Qwd2557",
			"/Qwd3280",

			"/Qwd170",
			"/Qwd188",

			"/Qwd63",
			"/Qwd177",
			"/Qwd186",
			"/Qwd488",
			"/Qwd1478",
			"/Qwd1879",
			"/Qwd3291",
			"/Qwd1195",
			"/Qwd1786",
			"/Qwd592", -- For lua, false positive?
		}
end

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

if (not os.isfile(path.join("src", "osd",  _OPTIONS["osd"] .. ".lua"))) then
	error("Unsupported value '" .. _OPTIONS["osd"] .. "' for OSD")
end
dofile(path.join("src", "osd", _OPTIONS["osd"] .. ".lua"))
dofile(path.join("src", "lib.lua"))

group "3rdparty"
dofile(path.join("src", "3rdparty.lua"))


group "core"

dofile(path.join("src", "emu.lua"))
emuProject(_OPTIONS["target"],_OPTIONS["subtarget"])

group "drivers"
findfunction("createProjects_" .. _OPTIONS["target"] .. "_" .. _OPTIONS["subtarget"])(_OPTIONS["target"], _OPTIONS["subtarget"])

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
