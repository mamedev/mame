-- license:BSD-3-Clause
-- copyright-holders:MAMEdev Team

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
if (_OPTIONS["SOURCES"] == nil) then 
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
end
	configuration { "mingw*" or "vs*" }
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
	}
	if (_OPTIONS["SOURCES"] == nil) then 
		links {
			"bus",
		}
	end
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
		"expat",
		"softfloat",
		"jpeg",
		"7z",
		"lua",
		"lsqlite3",
		"jsoncpp",
		"mongoose",
	}

	if _OPTIONS["with-bundled-zlib"] then
		links {
			"zlib",
		}
	else
		links {
			"z",
		}
	end

	if _OPTIONS["with-bundled-flac"] then
		links {
			"flac",
		}
	else
		links {
			"FLAC",
		}
	end

	if _OPTIONS["with-bundled-sqlite3"] then
		links {
			"sqllite3",
		}
	else
		links {
			"sqlite3",
		}
	end

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
		MAME_DIR .. "src/devices",
		MAME_DIR .. "src/" .. _target,
		MAME_DIR .. "src/lib",
		MAME_DIR .. "src/lib/util",
		MAME_DIR .. "3rdparty",
		GEN_DIR  .. _target .. "/layout",
		GEN_DIR  .. "resource",
	}

	if _OPTIONS["with-bundled-zlib"] then
		includedirs {
			MAME_DIR .. "3rdparty/zlib",
		}
	end

	if _OPTIONS["targetos"]=="macosx" and (not override_resources) then
		linkoptions {
			"-sectcreate __TEXT __info_plist " .. GEN_DIR .. "/resource/" .. _subtarget .. "-Info.plist"
		}
		custombuildtask {
			{ MAME_DIR .. "src/version.c" ,  GEN_DIR  .. "/resource/" .. _subtarget .. "-Info.plist",    {  MAME_DIR .. "scripts/build/verinfo.py" }, {"@echo Emitting " .. _subtarget .. "-Info.plist" .. "...",    PYTHON .. " $(1)  -p -b " .. _subtarget .. " $(<) > $(@)" }},
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
	
if (_OPTIONS["SOURCES"] == nil) then 	
	dependency {
		{ "../../../../generated/mame/mame/drivlist.c",  MAME_DIR .. "src/mame/mess.lst", true },
		{ "../../../../generated/mame/mame/drivlist.c" , MAME_DIR .. "src/mame/arcade.lst", true},
	}
	custombuildtask {
		{ MAME_DIR .. "src/".._target .."/" .. _subtarget ..".lst" ,  GEN_DIR  .. _target .. "/" .. _subtarget .."/drivlist.c",    {  MAME_DIR .. "scripts/build/makelist.py" }, {"@echo Building driver list...",    PYTHON .. " $(1) $(<) > $(@)" }},
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
			{ MAME_DIR .. "src/version.c" ,  GEN_DIR  .. "/resource/" .. rctarget .. "vers.rc",    {  MAME_DIR .. "scripts/build/verinfo.py" }, {"@echo Emitting " .. rctarget .. "vers.rc" .. "...",    PYTHON .. " $(1)  -r -b " .. rctarget .. " $(<) > $(@)" }},
		}	
	
	configuration { "vs*" }
		prebuildcommands {	
			"mkdir " .. path.translate(GEN_DIR  .. "/resource/","\\") .. " 2>NUL",
			"@echo Emitting ".. rctarget .. "vers.rc...",
			PYTHON .. " " .. path.translate(MAME_DIR .. "scripts/build/verinfo.py","\\") .. " -r -b " .. rctarget .. " " .. path.translate(MAME_DIR .. "src/version.c","\\") .. " > " .. path.translate(GEN_DIR  .. "/resource/" .. rctarget .. "vers.rc", "\\") ,
		}	
	
	 
	configuration { }

	debugdir (MAME_DIR)
	debugargs ("-window")
end
