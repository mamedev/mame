-- license:BSD-3-Clause
-- copyright-holders:MAMEdev Team

---------------------------------------------------------------------------
--
--   mame.lua
--
--   MAME target makefile
--
---------------------------------------------------------------------------

-- Set all the device flag setting commands from the block headers

local function selectors_get(path)
	local selector = ""
	for l in io.lines(path) do
		if l:sub(1, 3) == "--@" then
			local pos = l:find(",")
			selector = selector .. l:sub(pos+1) .. "\n"
		end
	end
	return selector
end

local selectors =
		selectors_get(MAME_DIR .. "scripts/src/cpu.lua") ..
		selectors_get(MAME_DIR .. "scripts/src/sound.lua") ..
		selectors_get(MAME_DIR .. "scripts/src/video.lua") ..
		selectors_get(MAME_DIR .. "scripts/src/machine.lua") ..
		selectors_get(MAME_DIR .. "scripts/src/bus.lua") ..
		selectors_get(MAME_DIR .. "scripts/src/formats.lua")

load(selectors)()


--------------------------------------------------
-- this is the list of driver libraries that
-- comprise MAME
--------------------------------------------------

function linkProjects_mame_mame(_target, _subtarget)
	local projects = {}
	for x, dir in pairs(os.matchdirs(path.join(MAME_DIR, "src", _target, "*"))) do
		local name = path.getname(dir)
		if name ~= "shared" then
			if 0 < #os.matchfiles(path.join(dir, "**.cpp")) then
				table.insert(projects, name)
			elseif 0 < #os.matchfiles(path.join(dir, "**.h")) then
				table.insert(projects, name)
			elseif 0 < #os.matchfiles(path.join(dir, "**.ipp")) then
				table.insert(projects, name)
			end
		end
	end
	table.insert(projects, "shared") -- must stay at the end
	links(projects)
end


function createMAMEProjects(_target, _subtarget, _name)
	project (_name)
	targetsubdir(_target .."_" .. _subtarget)
	kind (LIBTYPE)
	uuid (os.uuid("drv-" .. _target .."_" .. _subtarget .. "_" .._name))
	addprojectflags()
	precompiledheaders_novs()

	includedirs {
		MAME_DIR .. "src/osd",
		MAME_DIR .. "src/emu",
		MAME_DIR .. "src/devices",
		MAME_DIR .. "src/mame/shared",
		MAME_DIR .. "src/lib",
		MAME_DIR .. "src/lib/util",
		MAME_DIR .. "3rdparty",
		GEN_DIR  .. "mame/layout",
	}

	includedirs {
		ext_includedir("asio"),
		ext_includedir("flac"),
		ext_includedir("glm"),
		ext_includedir("jpeg"),
		ext_includedir("rapidjson"),
		ext_includedir("zlib")
	}

end


function createProjects_mame_mame(_target, _subtarget)
	for x, dir in pairs(os.matchdirs(path.join(MAME_DIR, "src", _target, "*"))) do
		local name = path.getname(dir)

		local sources = {}
		if 0 < #os.matchfiles(path.join(dir, "**.cpp")) then
			table.insert(sources, MAME_DIR .. "src/" .. _target .. "/" .. name .. "/**.cpp")
		end
		if 0 < #os.matchfiles(path.join(dir, "**.h")) then
			table.insert(sources, MAME_DIR .. "src/" .. _target .. "/" .. name .. "/**.h")
		end
		if 0 < #os.matchfiles(path.join(dir, "**.ipp")) then
			table.insert(sources, MAME_DIR .. "src/" .. _target .. "/" .. name .. "/**.ipp")
		end

		if 0 < #sources then
			createMAMEProjects(_target, _subtarget, name)
			files(sources)
		end
	end
end
