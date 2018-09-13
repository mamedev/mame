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

function addfiles(_filelist)
	for i,v in ipairs(_filelist) do
		table.insert(project_files, v)
	end
end

function adddependency(_dependency)
	for i,v in ipairs(_dependency) do
		table.insert(project_dependency, v)
	end
end

function addcustombuildtask(_buildtask)
	for i,v in ipairs(_buildtask) do
		table.insert(project_custombuildtask, v)
	end
end

	project_files = {}
	project_dependency = {}
	project_custombuildtask = {}

	dofile(path.join("src", _libraryscript))

	if #project_files > 0 then
		project (_libraryname)

		files(project_files)
		dependency(project_dependency)
		custombuildtask(project_custombuildtask)
		--print( " added ", #project_files, " files ", #project_dependency, " dependencies ", #project_custombuildtask, " custom build tasks" )

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
	end

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
