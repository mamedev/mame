-- license:BSD-3-Clause
-- copyright-holders:MAMEdev Team

---------------------------------------------------------------------------
--
--   main.lua
--
--   Rules for building main binary
--
---------------------------------------------------------------------------

function mainProject(_target, _subtarget)
local projname
if (_OPTIONS["SOURCES"] == nil) and (_OPTIONS["SOURCEFILTER"] == nil) then
	if (_target == _subtarget) then
		projname = _target
	else
		projname = _target .. _subtarget
	end
else
	projname = _subtarget
end
	project (projname)
	uuid (os.uuid(_target .. "_" .. _subtarget))
	kind "ConsoleApp"

	configuration { "android*" }
		targetprefix "lib"
		targetname "main"
		targetextension ".so"
		linkoptions {
			"-shared",
			"-Wl,-soname,libmain.so"
		}
		links {
			"EGL",
			"GLESv1_CM",
			"GLESv2",
			"SDL2",
		}

	configuration {  }

	addprojectflags()
	flags {
		"NoManifest",
		"Symbols", -- always include minimum symbols for executables
	}

	if _OPTIONS["SYMBOLS"] then
		configuration { "mingw*" }
			postbuildcommands {
				"$(SILENT) echo Dumping symbols.",
				"$(SILENT) objdump --section=.text --line-numbers --syms --demangle $(TARGET) >$(subst .exe,.sym,$(TARGET))"
			}
	end

	configuration { "Release" }
		targetsuffix ""
		if _OPTIONS["PROFILE"] then
			targetsuffix "p"
		end

	configuration { "Debug" }
		targetsuffix "d"
		if _OPTIONS["PROFILE"] then
			targetsuffix "dp"
		end

	configuration { "mingw*" or "vs20*" }
		targetextension ".exe"

	configuration { "asmjs" }
		targetextension ".html"

	configuration { }

	if _OPTIONS["targetos"]=="android" then
		files {
			MAME_DIR .. "src/osd/sdl/android_main.cpp",
		}
		targetsuffix ""
		if _OPTIONS["SEPARATE_BIN"]~="1" then
			if _OPTIONS["PLATFORM"]=="arm" then
				targetdir(MAME_DIR .. "android-project/app/src/main/libs/armeabi-v7a")
				os.copyfile(_OPTIONS["SDL_INSTALL_ROOT"] .. "/lib/libSDL2.so", MAME_DIR .. "android-project/app/src/main/libs/armeabi-v7a/libSDL2.so")
				os.copyfile(androidToolchainRoot() .. "/sysroot/usr/lib/arm-linux-androideabi/libc++_shared.so", MAME_DIR .. "android-project/app/src/main/libs/armeabi-v7a/libc++_shared.so")
			end
			if _OPTIONS["PLATFORM"]=="arm64" then
				targetdir(MAME_DIR .. "android-project/app/src/main/libs/arm64-v8a")
				os.copyfile(_OPTIONS["SDL_INSTALL_ROOT"] .. "/lib/libSDL2.so", MAME_DIR .. "android-project/app/src/main/libs/arm64-v8a/libSDL2.so")
				os.copyfile(androidToolchainRoot() .. "/sysroot/usr/lib/aarch64-linux-android/libc++_shared.so", MAME_DIR .. "android-project/app/src/main/libs/arm64-v8a/libc++_shared.so")
			end
			if _OPTIONS["PLATFORM"]=="x86" then
				targetdir(MAME_DIR .. "android-project/app/src/main/libs/x86")
				os.copyfile(_OPTIONS["SDL_INSTALL_ROOT"] .. "/lib/libSDL2.so", MAME_DIR .. "android-project/app/src/main/libs/x86/libSDL2.so")
				os.copyfile(androidToolchainRoot() .. "/sysroot/usr/lib/i686-linux-android/libc++_shared.so", MAME_DIR .. "android-project/app/src/main/libs/x86/libc++_shared.so")
			end
			if _OPTIONS["PLATFORM"]=="x64" then
				targetdir(MAME_DIR .. "android-project/app/src/main/libs/x86_64")
				os.copyfile(_OPTIONS["SDL_INSTALL_ROOT"] .. "/lib/libSDL2.so", MAME_DIR .. "android-project/app/src/main/libs/x86_64/libSDL2.so")
				os.copyfile(androidToolchainRoot() .. "/sysroot/usr/lib/x86_64-linux-android/libc++_shared.so", MAME_DIR .. "android-project/app/src/main/libs/x86_64/libc++_shared.so")
			end
		end
	else
		if _OPTIONS["SEPARATE_BIN"]~="1" then
			targetdir(MAME_DIR)
		end
	end

if (STANDALONE~=true) then
	findfunction("linkProjects_" .. _OPTIONS["target"] .. "_" .. _OPTIONS["subtarget"])(_OPTIONS["target"], _OPTIONS["subtarget"])
end
if (STANDALONE~=true) then
	links {
		"frontend",
	}
end
	links {
		"optional",
		"emu",
	}
	links {
		"osd_" .. _OPTIONS["osd"],
	}
	links {
		"qtdbg_" .. _OPTIONS["osd"],
	}
--if (STANDALONE~=true) then
	links {
		"formats",
	}
--end
if #disasm_files > 0 then
	links {
		"dasm",
	}
end
if (MACHINES["NETLIST"]~=null) then
	links {
		"netlist",
	}
end
	links {
		"utils",
		"mame_srcdbg_static",
		ext_lib("expat"),
		"softfloat",
		"softfloat3",
		"wdlfft",
		"ymfm",
		ext_lib("jpeg"),
		"7z",
	}
if CPU_INCLUDE_DRC_NATIVE then
	links {
		"asmjit",
	}
end
if (STANDALONE~=true) then
	links {
		ext_lib("lua"),
		"lualibs",
		"linenoise",
	}
end
	links {
		ext_lib("zlib"),
		ext_lib("zstd"),
		ext_lib("flac"),
		ext_lib("utf8proc"),
	}
if (STANDALONE~=true) then
	links {
		ext_lib("sqlite3"),
	}
end

	if _OPTIONS["NO_USE_PORTAUDIO"]~="1" then
		links {
			ext_lib("portaudio"),
		}
		if _OPTIONS["targetos"]=="windows" then
			links {
				"setupapi",
			}
		end
	end
	if _OPTIONS["NO_USE_MIDI"]~="1" then
		links {
			ext_lib("portmidi"),
		}
	end
	links {
		"bgfx",
		"bimg",
		"bx",
		"ocore_" .. _OPTIONS["osd"],
	}

	override_resources = false;

	maintargetosdoptions(_target, _subtarget)
	local exename = projname -- FIXME: should include the OSD prefix if any

	includedirs {
		MAME_DIR .. "src/osd",
		MAME_DIR .. "src/emu",
		MAME_DIR .. "src/devices",
		MAME_DIR .. "src/" .. _target,
		MAME_DIR .. "src/lib",
		MAME_DIR .. "src/lib/util",
		MAME_DIR .. "3rdparty",
		GEN_DIR  .. _target .. "/layout",
		ext_includedir("zlib"),
		ext_includedir("flac"),
	}

	resincludedirs {
		MAME_DIR .. "scripts/resources/windows/" .. _target,
		GEN_DIR  .. "resource",
	}


if (STANDALONE==true) then
	standalone();
end

if (STANDALONE~=true) then
	if _OPTIONS["targetos"]=="macosx" and (not override_resources) then
		local plistname = _target .. "_" .. _subtarget .. "-Info.plist"
		linkoptions {
			"-sectcreate __TEXT __info_plist " .. _MAKE.esc(GEN_DIR) .. "resource/" .. plistname
		}
		custombuildtask {
			{
				GEN_DIR .. "version.cpp",
				GEN_DIR .. "resource/" .. plistname,
				{ MAME_DIR .. "scripts/build/verinfo.py" },
				{
					"@echo Emitting " .. plistname .. "...",
					PYTHON .. " $(1) -f plist -t " .. _target .. " -s " .. _subtarget .. " -e " .. exename .. " -o $(@) $(<)"
				}
			},
		}
		dependency {
			{ "$(TARGET)" ,  GEN_DIR  .. "resource/" .. plistname, true },
		}

	end

	local rcversfile = GEN_DIR .. "resource/" .. _target .. "_" .. _subtarget .. "_vers.rc"
	if _OPTIONS["targetos"]=="windows" and (not override_resources) then
		files {
			rcversfile
		}
	end

	local rcincfile = MAME_DIR .. "scripts/resources/windows/" .. _target .. "/" .. _subtarget ..".rc"
	if not os.isfile(rcincfile) then
		rcincfile = MAME_DIR .. "scripts/resources/windows/mame/mame.rc"
		resincludedirs {
			MAME_DIR .. "scripts/resources/windows/mame",
		}
	end

	local mainfile = MAME_DIR .. "src/" .. _target .. "/" .. _subtarget .. ".cpp"
	if not os.isfile(mainfile) then
		mainfile = MAME_DIR .. "src/" .. _target .. "/" .. _target .. ".cpp"
	end
	files {
		mainfile,
		GEN_DIR .. "version.cpp",
		GEN_DIR .. _target .. "/" .. _subtarget .. "/drivlist.cpp",
	}

	local driverlist = MAME_DIR .. "src/" .. _target .. "/" .. _target .. ".lst"
	local driverssrc = GEN_DIR .. _target .. "/" .. _subtarget .. "/drivlist.cpp"
	if _OPTIONS["SOURCES"] ~= nil then
		dependency {
			{ driverssrc, driverlist, true },
		}
		custombuildtask {
			{
				GEN_DIR .. _target .."/" .. _subtarget .. ".flt" ,
				driverssrc,
				{ MAME_DIR .. "scripts/build/makedep.py", driverlist },
				{
					"@echo Building driver list...",
					PYTHON .. " $(1) -r " .. MAME_DIR .. " driverlist $(2) -f $(<) > $(@)"
				}
			},
		}
	elseif _OPTIONS["SOURCEFILTER"] ~= nil then
		dependency {
			{ driverssrc, driverlist, true },
		}
		custombuildtask {
			{
				MAME_DIR .. _OPTIONS["SOURCEFILTER"],
				driverssrc,
				{ MAME_DIR .. "scripts/build/makedep.py", driverlist },
				{
					"@echo Building driver list...",
					PYTHON .. " $(1) -r " .. MAME_DIR .. " driverlist $(2) -f $(<) > $(@)"
				}
			},
		}
	elseif os.isfile(MAME_DIR .. "src/" .. _target .."/" .. _subtarget ..".flt") then
		dependency {
			{ driverssrc, driverlist, true },
		}
		custombuildtask {
			{
				MAME_DIR .. "src/" .. _target .. "/" .. _subtarget .. ".flt",
				driverssrc,
				{ MAME_DIR .. "scripts/build/makedep.py", driverlist },
				{
					"@echo Building driver list...",
					PYTHON .. " $(1) -r " .. MAME_DIR .. " driverlist $(2) -f $(<) > $(@)"
				}
			},
		}
	elseif os.isfile(MAME_DIR .. "src/" .._target .. "/" .. _subtarget ..".lst") then
		custombuildtask {
			{
				MAME_DIR .. "src/" .. _target .. "/" .. _subtarget .. ".lst",
				driverssrc,
				{ MAME_DIR .. "scripts/build/makedep.py" },
				{
					"@echo Building driver list...",
					PYTHON .. " $(1) -r " .. MAME_DIR .. " driverlist $(<) > $(@)"
				}
			},
		}
	else
		dependency {
			{ driverssrc, driverlist, true },
		}
		custombuildtask {
			{
				driverlist,
				driverssrc,
				{ MAME_DIR .. "scripts/build/makedep.py" },
				{
					"@echo Building driver list...",
					PYTHON .. " $(1) -r " .. MAME_DIR .. " driverlist $(<) > $(@)"
				}
			},
		}
	end

	configuration { "mingw*" }
		dependency {
			{ "$(OBJDIR)/" .. _target .. "_" .. _subtarget .. "_vers.res", rcincfile, true },
		}
		custombuildtask {
			{
				GEN_DIR .. "version.cpp" ,
				rcversfile,
				{ MAME_DIR .. "scripts/build/verinfo.py" },
				{
					"@echo Emitting " .. _target .. "_" .. _subtarget .. "_vers.rc" .. "...",
					PYTHON .. " $(1) -f rc -t " .. _target .. " -s " .. _subtarget .. " -e " .. exename .. " -r " .. path.getname(rcincfile) .. " -o $(@) $(<)"
				}
			},
		}

	configuration { "vs20*" }
		dependency {
			{ "$(OBJDIR)/" .. _target .. "_" .. _subtarget .. "_vers.res", rcincfile, true },
		}
		prebuildcommands {
			"mkdir \"" .. path.translate(GEN_DIR .. "resource/", "\\") .. "\" 2>NUL",
			"mkdir \"" .. path.translate(GEN_DIR .. _target .. "/" .. _subtarget .. "/", "\\") .. "\" 2>NUL",
			"@echo Emitting " .. _target .. "_" .. _subtarget .. "_vers.rc" .. "...",
			PYTHON .. " \"" .. path.translate(MAME_DIR .. "scripts/build/verinfo.py", "\\") .. "\" -f rc -t " .. _target .. " -s " .. _subtarget .. " -e " .. exename .. " -o \"" .. path.translate(rcversfile) .. "\" -r \"" .. path.getname(rcincfile) .. "\" \"" .. path.translate(GEN_DIR .. "version.cpp", "\\") .. "\"",
		}
end

	configuration { }

	if _OPTIONS["DEBUG_DIR"]~=nil then
		debugdir(_OPTIONS["DEBUG_DIR"])
	else
		debugdir (MAME_DIR)
	end
	if _OPTIONS["DEBUG_ARGS"]~=nil then
		debugargs (_OPTIONS["DEBUG_ARGS"])
	else
		debugargs ("-window")
	end

end
