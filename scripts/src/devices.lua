-- license:BSD-3-Clause
-- copyright-holders:MAMEdev Team

function devicesProject(_target, _subtarget)

	disasm_files = { }
	disasm_dependency = { }
	disasm_custombuildtask = { }

	project ("optional")
	uuid (os.uuid("optional-" .. _target .."_" .. _subtarget))
	kind (LIBTYPE)
	targetsubdir(_target .."_" .. _subtarget)
	options {
		"ForceCPP",
		"ArchiveSplit",
	}

	includedirs {
		MAME_DIR .. "src/osd",
		MAME_DIR .. "src/emu",
		MAME_DIR .. "src/devices",
		MAME_DIR .. "src/lib/netlist",
		MAME_DIR .. "src/mame", -- used for sound amiga
		MAME_DIR .. "src/lib",
		MAME_DIR .. "src/lib/util",
		MAME_DIR .. "3rdparty",
		GEN_DIR  .. "emu",
		GEN_DIR  .. "emu/layout",
	}
	if _OPTIONS["with-bundled-expat"] then
		includedirs {
			MAME_DIR .. "3rdparty/expat/lib",
		}
	end
	if _OPTIONS["with-bundled-lua"] then
		includedirs {
			MAME_DIR .. "3rdparty/lua/src",
		}
	end

	dofile(path.join("src", "cpu.lua"))

	dofile(path.join("src", "sound.lua"))

	dofile(path.join("src", "video.lua"))

	dofile(path.join("src", "machine.lua"))

if (_OPTIONS["DRIVERS"] == nil) then
	project ("bus")
	uuid ("5d782c89-cf7e-4cfe-8f9f-0d4bfc16c91d")
	kind (LIBTYPE)
	targetsubdir(_target .."_" .. _subtarget)
	options {
		"ForceCPP",
		"ArchiveSplit",
	}

	includedirs {
		MAME_DIR .. "src/osd",
		MAME_DIR .. "src/emu",
		MAME_DIR .. "src/devices",
		MAME_DIR .. "src/lib/netlist",
		MAME_DIR .. "src/lib",
		MAME_DIR .. "src/lib/util",
		MAME_DIR .. "3rdparty",
		MAME_DIR .. "src/mame", -- used for nes bus devices,some mess bus devices need this
		GEN_DIR  .. "emu",
		GEN_DIR  .. "emu/layout",
	}
	if _OPTIONS["with-bundled-expat"] then
		includedirs {
			MAME_DIR .. "3rdparty/expat/lib",
		}
	end
	if _OPTIONS["with-bundled-lua"] then
		includedirs {
			MAME_DIR .. "3rdparty/lua/src",
		}
	end

	dofile(path.join("src", "bus.lua"))
else
	dofile(path.join("src", "bus.lua"))
end

	project ("dasm")
	uuid ("f2d28b0a-6da5-4f78-b629-d834aa00429d")
	kind (LIBTYPE)
	targetsubdir(_target .."_" .. _subtarget)
	options {
		"ForceCPP",
	}

	includedirs {
		MAME_DIR .. "src/osd",
		MAME_DIR .. "src/emu",
		MAME_DIR .. "src/devices",
		MAME_DIR .. "src/lib",
		MAME_DIR .. "src/lib/util",
		MAME_DIR .. "3rdparty",
		GEN_DIR  .. "emu",
	}
	if _OPTIONS["with-bundled-expat"] then
		includedirs {
			MAME_DIR .. "3rdparty/expat/lib",
		}
	end
	if _OPTIONS["with-bundled-lua"] then
		includedirs {
			MAME_DIR .. "3rdparty/lua/src",
		}
	end

	files {
		disasm_files
	}

	if #disasm_dependency > 0 then
		dependency {
			disasm_dependency[1]
		}
	end

	if #disasm_custombuildtask > 0 then
		custombuildtask {
			disasm_custombuildtask[1]
		}
	end
end
