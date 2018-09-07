-- license:BSD-3-Clause
-- copyright-holders:MAMEdev Team

---------------------------------------------------------------------------
--
--   devices.lua
--
--   Rules for building device cores
--
---------------------------------------------------------------------------

function library_project(_libraryname, _libraryscript, _target, _subtarget)

	project (_libraryname)
	uuid (os.uuid(_libraryname .. "-" .. _target .."_" .. _subtarget))
	kind (LIBTYPE)
	targetsubdir(_target .."_" .. _subtarget)

	if (_OPTIONS["targetos"] ~= "asmjs") then
		options {
			"ArchiveSplit",
		}
	end

	addprojectflags()
	precompiledheaders()

	includedirs {
		MAME_DIR .. "src/osd",
		MAME_DIR .. "src/emu",
		MAME_DIR .. "src/devices",
		MAME_DIR .. "src/mame", -- used for sound amiga
		MAME_DIR .. "src/lib",
		MAME_DIR .. "src/lib/util",
		MAME_DIR .. "3rdparty",
		GEN_DIR  .. "emu",
		GEN_DIR  .. "emu/layout",
		ext_includedir("asio"),
		ext_includedir("expat"),
		ext_includedir("flac"),
	}

	dofile(path.join("src", _libraryscript))
end

function devicesProject(_target, _subtarget)

	disasm_files = { }
	disasm_dependency = { }
	disasm_custombuildtask = { }

	library_project("devicescpu", "cpu.lua", _target, _subtarget)

	library_project("devicessound", "sound.lua", _target, _subtarget)

	library_project("devicesvideo", "video.lua", _target, _subtarget)

	library_project("devicesmachine", "machine.lua", _target, _subtarget)

	library_project("devicesbus", "bus.lua", _target, _subtarget)

if #disasm_files > 0 then
	project ("dasm")
	uuid ("f2d28b0a-6da5-4f78-b629-d834aa00429d")
	kind (LIBTYPE)
	targetsubdir(_target .."_" .. _subtarget)
	addprojectflags()
	precompiledheaders()

	includedirs {
		MAME_DIR .. "src/osd",
		MAME_DIR .. "src/emu",
		MAME_DIR .. "src/devices",
		MAME_DIR .. "src/lib",
		MAME_DIR .. "src/lib/util",
		MAME_DIR .. "3rdparty",
		GEN_DIR  .. "emu",
		ext_includedir("asio"),
		ext_includedir("expat"),
	}

	files {
		disasm_files
	}

	if #disasm_dependency > 0 then
		dependency(disasm_dependency)
	end

	if #disasm_custombuildtask > 0 then
		custombuildtask(disasm_custombuildtask)
	end
end

end
