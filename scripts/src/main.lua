-- license:BSD-3-Clause
-- copyright-holders:MAMEdev Team

function mainProject(_target, _subtarget)
	if (_target == _subtarget) then
		project (_target)
	else
		if (_subtarget=="mess") then
			project (_subtarget)
		else
			project (_target .. _subtarget)
		end
	end	
	uuid (os.uuid(_target .."_" .. _subtarget))
	kind "ConsoleApp"

	options {
		"ForceCPP",
	}
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
	-- BEGIN libretro overrides to MAME's GENie build
	configuration { not "libretrodso" and "mingw*" or "vs*" }
	-- END libretro overrides to MAME's GENie build
		targetextension ".exe"

	configuration { "asmjs" }
		targetextension ".bc"  

	configuration { }

	if _OPTIONS["SEPARATE_BIN"]~="1" then 
		targetdir(MAME_DIR)
	end
	
	findfunction("linkProjects_" .. _OPTIONS["target"] .. "_" .. _OPTIONS["subtarget"])(_OPTIONS["target"], _OPTIONS["subtarget"])
	links {
		"osd_" .. _OPTIONS["osd"],
		"bus",
		"netlist",
		"optional",
		"emu",
		"dasm",
		"utils",
		"expat",
		"softfloat",
		"jpeg",
		"flac",
		"7z",
		"formats",
		"lua",
		"lsqlite3",
		"sqllite3",
		"zlib",
		"jsoncpp",
		"mongoose",
	}
	if _OPTIONS["NO_USE_MIDI"]~="1" then
		links {
			"portmidi",
		}
	end
	if (USE_BGFX == 1) then
		links {
			"bgfx"
		}
	end
	links{
		"ocore_" .. _OPTIONS["osd"],
	}
	
	override_resources = false;
	
	maintargetosdoptions(_target,_subtarget)

	includedirs {
		MAME_DIR .. "src/osd",
		MAME_DIR .. "src/emu",
		MAME_DIR .. "src/" .. _target,
		MAME_DIR .. "src/lib",
		MAME_DIR .. "src/lib/util",
		MAME_DIR .. "3rdparty",
		MAME_DIR .. "3rdparty/zlib",
		GEN_DIR  .. _target .. "/layout",
		GEN_DIR  .. "resource",
	}

	if _OPTIONS["targetos"]=="macosx" and (not override_resources) then
		linkoptions {
			"-sectcreate __TEXT __info_plist " .. GEN_DIR .. "/resource/" .. _subtarget .. "-Info.plist"
		}
		custombuildtask {
			{ MAME_DIR .. "src/version.c" ,  GEN_DIR  .. "/resource/" .. _subtarget .. "-Info.plist",    {  MAME_DIR .. "src/build/verinfo.py" }, {"@echo Emitting " .. _subtarget .. "-Info.plist" .. "...",    PYTHON .. " $(1)  -p -b " .. _subtarget .. " $(<) > $(@)" }},
		}
		dependency {
			{ "$(TARGET)" ,  GEN_DIR  .. "/resource/" .. _subtarget .. "-Info.plist", true  },
		}

	end
	local rctarget = _subtarget

	if _OPTIONS["targetos"]=="windows" and (not override_resources) then
		local rcfile = MAME_DIR .. "src/" .. _target .. "/osd/".._OPTIONS["osd"].."/"  .. _subtarget .. "/" .. rctarget ..".rc"
		if not os.isfile(rcfile) then
			rcfile = MAME_DIR .. "src/" .. _target .. "/osd/windows/" .. _subtarget .. "/" .. rctarget ..".rc"
		end
		if os.isfile(rcfile) then
			files {
				rcfile,
			}
			dependency {
				{ "$(OBJDIR)/".._subtarget ..".res" ,  GEN_DIR  .. "/resource/" .. rctarget .. "vers.rc", true  },
			}
		else
			rctarget = "mame"
			files {
				MAME_DIR .. "src/mame/osd/windows/mame/mame.rc",
			}
			dependency {
				{ "$(OBJDIR)/mame.res" ,  GEN_DIR  .. "/resource/" .. rctarget .. "vers.rc", true  },
			}
		end	
	end

	local mainfile = MAME_DIR .. "src/".._target .."/" .. _subtarget ..".c"
	if not os.isfile(mainfile) then
		mainfile = MAME_DIR .. "src/".._target .."/" .. _target ..".c"
	end
	files {
		mainfile,
		MAME_DIR .. "src/version.c",
		GEN_DIR  .. _target .. "/" .. _subtarget .."/drivlist.c",
	}
	dependency {
		{ "../../../../generated/mame/mame/drivlist.c",  MAME_DIR .. "src/mame/mess.lst", true },
		{ "../../../../generated/mame/mame/drivlist.c" , MAME_DIR .. "src/mame/arcade.lst", true},
	}
	custombuildtask {
		{ MAME_DIR .. "src/".._target .."/" .. _subtarget ..".lst" ,  GEN_DIR  .. _target .. "/" .. _subtarget .."/drivlist.c",    {  MAME_DIR .. "src/build/makelist.py" }, {"@echo Building driver list...",    PYTHON .. " $(1) $(<) > $(@)" }},
	}
	
	configuration { "mingw*" }
		custombuildtask {	
			{ MAME_DIR .. "src/version.c" ,  GEN_DIR  .. "/resource/" .. rctarget .. "vers.rc",    {  MAME_DIR .. "src/build/verinfo.py" }, {"@echo Emitting " .. rctarget .. "vers.rc" .. "...",    PYTHON .. " $(1)  -r -b " .. rctarget .. " $(<) > $(@)" }},
		}	
	
	configuration { "vs*" }
		prebuildcommands {	
			"mkdir " .. path.translate(GEN_DIR  .. "/resource/","\\") .. " 2>NUL",
			"@echo Emitting ".. rctarget .. "vers.rc...",
			PYTHON .. " " .. path.translate(MAME_DIR .. "src/build/verinfo.py","\\") .. " -r -b " .. rctarget .. " " .. path.translate(MAME_DIR .. "src/version.c","\\") .. " > " .. path.translate(GEN_DIR  .. "/resource/" .. rctarget .. "vers.rc", "\\") ,
		}	
	
	 
	configuration { }

	debugdir (MAME_DIR)
	debugargs ("-window")

	-- BEGIN libretro overrides to MAME's GENie build
	if _OPTIONS["osd"]=="retro" then
		newoption {
			trigger = "platform",
			description = "libretro OS/platform variable",
		}

		-- $ARCH means something to Apple/Clang, so we can't use it here.
		-- Instead, use ARCH="" LIBRETRO_ARCH="$ARCH" on the make cmdline.
		newoption {
			trigger = "LIBRETRO_ARCH",
			description = "libretro CPU/architecture variable",
		}

		kind "SharedLib"
		targetsuffix "_libretro"
		targetprefix ""
		links {
			"libco",
		}

		if _OPTIONS["platform"]~=nil then
			retro_platform=_OPTIONS["platform"]
		end
		if _OPTIONS["LIBRETRO_ARCH"]~=nil then
			retro_arch=_OPTIONS["ARCH"]
		end

		if retro_platform~=nil then
			if retro_platform=="unix" then
				_OPTIONS["TARGETOS"] = "linux"
			elseif retro_platform=="android" then
				_OPTIONS["TARGETOS"] = "linux"
			elseif retro_platform=="qnx" then
				_OPTIONS["TARGETOS"] = "linux"
			elseif retro_platform:sub(1, 4)=="armv" then
				_OPTIONS["TARGETOS"] = "linux"
			elseif retro_platform=="osx" then
				_OPTIONS["TARGETOS"] = "macosx"
			elseif retro_platform=="ios" then
				_OPTIONS["TARGETOS"] = "macosx"
				targetsuffix "_libretro_ios"
			elseif retro_platform:sub(1, 3)=="win" then
				_OPTIONS["TARGETOS"] = "win32"
			end
		end
		-- FIXME: set BIGENDIAN and dynarec based on retro_platform/retro_arch


		-- "linux" for pretty much any Linux/BSD/Android...
		if _OPTIONS["targetos"]=="linux" then
			linkoptions {
				"-Wl,--version-script=" .. MAME_DIR .. "src/osd/retro/link.T",
			}
		end

		-- If we compile this into the OSD rather than the main shared library, the
		-- linker will "helpfully" strip out the "unused" libretro API...
		includedirs {
			MAME_DIR .. "src/osd/retro/libretro-common/include",
		}
		files {
			MAME_DIR .. "src/osd/retro/libretro.c"
		}
	end
	-- END libretro overrides to MAME's GENie build
end

