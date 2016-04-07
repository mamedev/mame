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
if (_OPTIONS["SOURCES"] == nil) then
	if (_target == _subtarget) then
		project (_target)
	else
		if (_subtarget=="mess") then
			project (_subtarget)
		else
			project (_target .. _subtarget)
		end
	end
else
	project (_subtarget)
end
	uuid (os.uuid(_target .."_" .. _subtarget))
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
	configuration { "pnacl" }
		kind "ConsoleApp"
		targetextension ".pexe"
		links {
			"ppapi",
			"ppapi_gles2",
			"pthread",
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

	configuration { "vs*" }
	flags {
		"Unicode",
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

	configuration { "mingw*" or "vs*" }
		targetextension ".exe"

	configuration { "rpi" }
		targetextension ""

	configuration { "asmjs" }
		targetextension ".bc"
		if os.getenv("EMSCRIPTEN") then
			local emccopts = ""
			emccopts = emccopts .. " -O3"
			emccopts = emccopts .. " -s USE_SDL=2"
			emccopts = emccopts .. " -s USE_SDL_TTF=2"
			emccopts = emccopts .. " --memory-init-file 0"
			emccopts = emccopts .. " -s ALLOW_MEMORY_GROWTH=0"
			emccopts = emccopts .. " -s TOTAL_MEMORY=268435456"
			emccopts = emccopts .. " -s DISABLE_EXCEPTION_CATCHING=2"
			emccopts = emccopts .. " -s EXCEPTION_CATCHING_WHITELIST='[\"__ZN15running_machine17start_all_devicesEv\",\"__ZN12cli_frontend7executeEiPPc\"]'"
			emccopts = emccopts .. " -s EXPORTED_FUNCTIONS=\"['_main', '_malloc', '__Z14js_get_machinev', '__Z9js_get_uiv', '__Z12js_get_soundv', '__ZN10ui_manager12set_show_fpsEb', '__ZNK10ui_manager8show_fpsEv', '__ZN13sound_manager4muteEbh', '_SDL_PauseAudio']\""
			emccopts = emccopts .. " --pre-js " .. _MAKE.esc(MAME_DIR) .. "src/osd/modules/sound/js_sound.js"
			emccopts = emccopts .. " --post-js " .. _MAKE.esc(MAME_DIR) .. "scripts/resources/emscripten/emscripten_post.js"
			emccopts = emccopts .. " --embed-file " .. _MAKE.esc(MAME_DIR) .. "bgfx/chains@bgfx/chains"
			emccopts = emccopts .. " --embed-file " .. _MAKE.esc(MAME_DIR) .. "bgfx/effects@bgfx/effects"
			emccopts = emccopts .. " --embed-file " .. _MAKE.esc(MAME_DIR) .. "bgfx/shaders/gles@bgfx/shaders/gles"
			emccopts = emccopts .. " --embed-file " .. _MAKE.esc(MAME_DIR) .. "artwork/slot-mask.png@artwork/slot-mask.png"
			postbuildcommands {
				os.getenv("EMSCRIPTEN") .. "/emcc " .. emccopts .. " $(TARGET) -o " .. _MAKE.esc(MAME_DIR) .. _OPTIONS["target"] .. _OPTIONS["subtarget"] .. ".js",
			}
		end

	configuration { }

	if _OPTIONS["targetos"]=="android" then
		includedirs {
			MAME_DIR .. "3rdparty/SDL2/include",
		}

		files {
			MAME_DIR .. "3rdparty/SDL2/src/main/android/SDL_android_main.c",
		}
		targetsuffix ""
		if _OPTIONS["SEPARATE_BIN"]~="1" then
			if _OPTIONS["PLATFORM"]=="arm" then
				targetdir(MAME_DIR .. "android-project/app/src/main/libs/armeabi-v7a")
			end
			if _OPTIONS["PLATFORM"]=="arm64" then
				targetdir(MAME_DIR .. "android-project/app/src/main/libs/arm64-v8a")
			end
			if _OPTIONS["PLATFORM"]=="mips" then
				targetdir(MAME_DIR .. "android-project/app/src/main/libs/mips")
			end
			if _OPTIONS["PLATFORM"]=="mips64" then
				targetdir(MAME_DIR .. "android-project/app/src/main/libs/mips64")
			end
			if _OPTIONS["PLATFORM"]=="x86" then
				targetdir(MAME_DIR .. "android-project/app/src/main/libs/x86")
			end
			if _OPTIONS["PLATFORM"]=="x64" then
				targetdir(MAME_DIR .. "android-project/app/src/main/libs/x86_64")
			end
		end
	else
		if _OPTIONS["SEPARATE_BIN"]~="1" then
			targetdir(MAME_DIR)
		end
	end

	findfunction("linkProjects_" .. _OPTIONS["target"] .. "_" .. _OPTIONS["subtarget"])(_OPTIONS["target"], _OPTIONS["subtarget"])
	links {
		"osd_" .. _OPTIONS["osd"],
	}
	links {
		"qtdbg_" .. _OPTIONS["osd"],
	}
	links {
		"netlist",
		"optional",
		"emu",
		"formats",
	}
if #disasm_files > 0 then
	links {
		"dasm",
	}
end
	links {
		"utils",
		ext_lib("expat"),
		"softfloat",
		ext_lib("jpeg"),
		"7z",
		ext_lib("lua"),
		"lualibs",
	}

	if _OPTIONS["USE_LIBUV"]=="1" then
		links {
			ext_lib("uv"),
			"http-parser",
		}
	end
	links {
		ext_lib("zlib"),
		ext_lib("flac"),
		ext_lib("sqlite3"),
	}

	if _OPTIONS["NO_USE_MIDI"]~="1" then
		links {
			ext_lib("portmidi"),
		}
	end
	links {
		"bgfx",
		"ocore_" .. _OPTIONS["osd"],
	}

	override_resources = false;

	maintargetosdoptions(_target,_subtarget)

	includedirs {
		MAME_DIR .. "src/osd",
		MAME_DIR .. "src/emu",
		MAME_DIR .. "src/devices",
		MAME_DIR .. "src/" .. _target,
		MAME_DIR .. "src/lib",
		MAME_DIR .. "src/lib/util",
		MAME_DIR .. "3rdparty",
		GEN_DIR  .. _target .. "/layout",
		GEN_DIR  .. "resource",
		ext_includedir("zlib"),
		ext_includedir("flac"),
	}

	if _OPTIONS["targetos"]=="macosx" and (not override_resources) then
		linkoptions {
			"-sectcreate __TEXT __info_plist " .. _MAKE.esc(GEN_DIR) .. "resource/" .. _subtarget .. "-Info.plist"
		}
		custombuildtask {
			{ MAME_DIR .. "src/version.cpp" ,  GEN_DIR .. "resource/" .. _subtarget .. "-Info.plist",    {  MAME_DIR .. "scripts/build/verinfo.py" }, {"@echo Emitting " .. _subtarget .. "-Info.plist" .. "...",    PYTHON .. " $(1)  -p -b " .. _subtarget .. " $(<) > $(@)" }},
		}
		dependency {
			{ "$(TARGET)" ,  GEN_DIR  .. "resource/" .. _subtarget .. "-Info.plist", true  },
		}

	end
	local rctarget = _subtarget

	if _OPTIONS["targetos"]=="windows" and (not override_resources) then
		rcfile = MAME_DIR .. "scripts/resources/windows/" .. _subtarget .. "/" .. rctarget ..".rc"
		if os.isfile(rcfile) then
			files {
				rcfile,
			}
			dependency {
				{ "$(OBJDIR)/".._subtarget ..".res" ,  GEN_DIR  .. "resource/" .. rctarget .. "vers.rc", true  },
			}
		else
			rctarget = "mame"
			files {
				MAME_DIR .. "scripts/resources/windows/mame/mame.rc",
			}
			dependency {
				{ "$(OBJDIR)/mame.res" ,  GEN_DIR  .. "resource/" .. rctarget .. "vers.rc", true  },
			}
		end
	end

	local mainfile = MAME_DIR .. "src/".._target .."/" .. _subtarget ..".cpp"
	if not os.isfile(mainfile) then
		mainfile = MAME_DIR .. "src/".._target .."/" .. _target ..".cpp"
	end
	files {
		mainfile,
		MAME_DIR .. "src/version.cpp",
		GEN_DIR  .. _target .. "/" .. _subtarget .."/drivlist.cpp",
	}

if (_OPTIONS["SOURCES"] == nil) then

	if os.isfile(MAME_DIR .. "src/".._target .."/" .. _subtarget ..".flt") then
		dependency {
		{
			GEN_DIR  .. _target .. "/" .. _subtarget .."/drivlist.cpp",  MAME_DIR .. "src/".._target .."/" .. _target ..".lst", true },
		}
		custombuildtask {
			{ MAME_DIR .. "src/".._target .."/" .. _subtarget ..".flt" ,  GEN_DIR  .. _target .. "/" .. _subtarget .."/drivlist.cpp",    {  MAME_DIR .. "scripts/build/makelist.py", MAME_DIR .. "src/".._target .."/" .. _target ..".lst"  }, {"@echo Building driver list...",    PYTHON .. " $(1) $(2) $(<) > $(@)" }},
		}
	else
		if os.isfile(MAME_DIR .. "src/".._target .."/" .. _subtarget ..".lst") then
			custombuildtask {
				{ MAME_DIR .. "src/".._target .."/" .. _subtarget ..".lst" ,  GEN_DIR  .. _target .. "/" .. _subtarget .."/drivlist.cpp",    {  MAME_DIR .. "scripts/build/makelist.py" }, {"@echo Building driver list...",    PYTHON .. " $(1) $(<) > $(@)" }},
			}
		else
			dependency {
			{
				GEN_DIR  .. _target .. "/" .. _target .."/drivlist.cpp",  MAME_DIR .. "src/".._target .."/" .. _target ..".lst", true },
			}
			custombuildtask {
				{ MAME_DIR .. "src/".._target .."/" .. _target ..".lst" ,  GEN_DIR  .. _target .. "/" .. _target .."/drivlist.cpp",    {  MAME_DIR .. "scripts/build/makelist.py" }, {"@echo Building driver list...",    PYTHON .. " $(1) $(<) > $(@)" }},
			}
		end
	end
end

if (_OPTIONS["SOURCES"] ~= nil) then
		dependency {
		{
			GEN_DIR  .. _target .. "/" .. _subtarget .."/drivlist.cpp",  MAME_DIR .. "src/".._target .."/" .. _target ..".lst", true },
		}
		custombuildtask {
			{ GEN_DIR .. _target .."/" .. _subtarget ..".flt" ,  GEN_DIR  .. _target .. "/" .. _subtarget .."/drivlist.cpp",    {  MAME_DIR .. "scripts/build/makelist.py", MAME_DIR .. "src/".._target .."/" .. _target ..".lst"  }, {"@echo Building driver list...",    PYTHON .. " $(1) $(2) $(<) > $(@)" }},
		}
end

if _OPTIONS["FORCE_VERSION_COMPILE"]=="1" then
	configuration { "gmake" }
		dependency {
			{ ".PHONY", ".FORCE", true },
			{ "$(OBJDIR)/src/version.o", ".FORCE", true },
		}
end
	configuration { "mingw*" }
		custombuildtask {
			{ MAME_DIR .. "src/version.cpp" ,  GEN_DIR  .. "resource/" .. rctarget .. "vers.rc",    {  MAME_DIR .. "scripts/build/verinfo.py" }, {"@echo Emitting " .. rctarget .. "vers.rc" .. "...",    PYTHON .. " $(1)  -r -b " .. rctarget .. " $(<) > $(@)" }},
		}

	configuration { "vs*" }
		prebuildcommands {
			"mkdir " .. path.translate(GEN_DIR  .. "resource/","\\") .. " 2>NUL",
			"@echo Emitting ".. rctarget .. "vers.rc...",
			PYTHON .. " " .. path.translate(MAME_DIR .. "scripts/build/verinfo.py","\\") .. " -r -b " .. rctarget .. " " .. path.translate(MAME_DIR .. "src/version.cpp","\\") .. " > " .. path.translate(GEN_DIR  .. "resource/" .. rctarget .. "vers.rc", "\\") ,
		}

	configuration { }

	debugdir (MAME_DIR)
	debugargs ("-window")
end
