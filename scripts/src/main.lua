function mainProject(_target, _subtarget)
	if (_target == _subtarget) then
		project (_target)
	else
		project (_target .. _subtarget)
	end	
	uuid (os.uuid(_target .."_" .. _subtarget))
	kind "ConsoleApp"	

	options {
		"ForceCPP",
	}
	flags {
		"NoManifest",
	}

	if _OPTIONS["SYMBOLS"] then
		configuration { "mingw*" }
			postbuildcommands {
				"$(SILENT) echo Dumping symbols.",
				"$(SILENT) objdump --section=.text --line-numbers --syms --demangle $(TARGET) >$(subst .exe,.sym,$(TARGET))"
			}
	end

	configuration { "osx*" }
		linkoptions {
			"-sectcreate __TEXT __info_plist " .. GEN_DIR .. "/osd/sdl/" .. _OPTIONS["target"] .. "-Info.plist"
		}

	configuration { "mingw*" }
		if _OPTIONS["osd"]=="sdl" then
			targetprefix "sdl"
		end
		targetextension ".exe"
		
	configuration { "vs*" }
		if _OPTIONS["osd"]=="sdl" then
			targetprefix "sdl"
		end
		targetextension ".exe"
		
	configuration { "asmjs" }
		targetextension ".bc"  
		
	configuration { }
		targetdir(MAME_DIR)

	linkProjects(_target, _subtarget)
	links {
		"osd_" .. _OPTIONS["osd"],
		"bus",
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
		"portmidi",
	}
	if (USE_BGFX == 1) then
		links {
			"bgfx"
		}
	end
	links{
		"ocore_" .. _OPTIONS["osd"],
	}
	dofile("src/osd_cfg.lua")
	
	includedirs {
		MAME_DIR .. "src/emu",
		MAME_DIR .. "src/mame",
		MAME_DIR .. "src/lib",
		MAME_DIR .. "src/lib/util",
		MAME_DIR .. "3rdparty",
		MAME_DIR .. "3rdparty/zlib",
		GEN_DIR  .. "mame/layout",
		GEN_DIR  .. "ldplayer/layout",
		GEN_DIR  .. "osd/windows",
	}

	includeosd()

	if _OPTIONS["osd"]=="windows" then
		local rcfile = MAME_DIR .. "src/" .. _target .. "/osd/windows/" .. _target ..".rc"
		
		if os.isfile(rcfile) then
			files {
				rcfile,
			}
		else
		files {
			MAME_DIR .. "src/osd/windows/mame.rc",
		}
		end
	end

	files {
		MAME_DIR .. "src/".._target .."/" .. _target ..".c",
		MAME_DIR .. "src/version.c",
		GEN_DIR  .. _target .. "/" .. _subtarget .."/drivlist.c",
	}
	debugdir (MAME_DIR)
	debugargs ("-window")
end