-- license:BSD-3-Clause
-- copyright-holders:Carl
-- This is a library of functions to be used at the Lua console as cf.getspaces() etc...
local exports = {}
exports.name = "cheatfind"
exports.version = "0.0.1"
exports.description = "Cheat finder helper library"
exports.license = "The BSD 3-Clause License"
exports.author = { name = "Carl" }

local cheatfind = exports

function cheatfind.startplugin()
	local cheat = {}

	-- return table of devices and spaces
	function cheat.getspaces()
		local spaces = {}
		for tag, device in pairs(manager:machine().devices) do
			if device.spaces then
				spaces[tag] = {}
				for name, space in pairs(device.spaces) do
					spaces[tag][name] = space
				end
			end
		end
		return spaces
	end

	-- return table of ram devices
	function cheat.getram()
		local ram = {}
		for tag, device in pairs(manager:machine().devices) do
			if device:shortname() == "ram" then
				ram[tag] = {}
				ram[tag].dev = device
				ram[tag].size = emu.item(device.items["0/m_size"]:read(0))
			end
		end
		return ram
	end

	-- save data block
	function cheat.save(space, start, size)
		local data = { block = "", dev = space, start = start, size = size, space = space }
		if space.shortname then
			if space:shortname() == "ram" then
				data.block = emu.item(device.items["0/m_pointer"]):read_block(start, size)
				if not data.block then
					return nil
				end
			end
		else
			local block = ""
			for i = start, start + size do
				block = block .. string.pack("B", space:read_u8(i))
			end
			data.block = block
		end
		return data
	end

	-- compare a data block to the current state
	function cheat.comp(olddata, oper, val)
		local newdata = cheat.save(olddata.dev, olddata.start, olddata.size, olddata.space)
		local ret = {}
		if not val then
			val = 0
		end
		if oper == "+" or oper == "inc" then
			for i = 1, olddata.size do
				local old = string.unpack("B", olddata.block, i)
				local new = string.unpack("B", newdata.block, i)
				if old < new then
					if (val > 0 and (old + val) == new) or val == 0 then
						ret[#ret + 1] = { addr = olddata.start + i,
								  oldval = old,
								  newval = new}
					end
				end
			end
		elseif oper == "-" or oper == "dec" then
			for i = 1, olddata.size do
				local old = string.unpack("B", olddata.block, i)
				local new = string.unpack("B", newdata.block, i)
				if old > new then
					if (val > 0 and (old - val) == new) or val == 0 then
						ret[#ret + 1] = { addr = olddata.start + i,
								  oldval = old,
								  newval = new}
					end
				end
			end
		end
		return ret
	end
	
	_G.cf = cheat
	
	local devtable = {}
	local devsel = 1
	local optable = { "+", "-" }
	local opsel = 1
	local change = 0
	local matches = {}
	local menu_blocks = {}

	local function start()
		devtable = {}
		menu_blocks = {}
		matches = {}
		local table = cheat.getspaces()
		for tag, list in pairs(table) do
			if list.program then
				local ram = {}
				for num, entry in pairs(list.program.map) do
					if entry.writetype == "ram" then
						ram[#ram + 1] = { offset = entry.offset, size = entry.endoff - entry.offset }
					end
				end
				if next(ram) then
					devtable[#devtable + 1] = { tag = tag, space = list.program, ram = ram }
				end
			end
		end
		table = cheat.getram()
		for tag, ram in pairs(table) do
			devtable[#devtable + 1] = { tag = tag, space = ram.dev, ram = { offset = 0, size = ram.size } }
		end
	end

	emu.register_start(start)

	local function menu_populate()
		local menu = {}
		menu[1] = {}
		menu[1][1] = "Region"
		menu[1][2] = devtable[devsel].tag
		if #devtable == 1 then
			menu[1][3] = 0
		elseif devsel == 1 then
			menu[1][3] = 2
		elseif devsel == #devtable then
			menu[1][3] = 1
		else
			menu[1][3] = 3
		end
		menu[2] = { "Init", "", 0 }
		if next(menu_blocks) then
			menu[3] = { "---", "", 32 }
			menu[4] = {}
			menu[4][1] = "Operator"
			menu[4][2] = optable[opsel]
			if opsel == 1 then
				menu[4][3] = 2
			elseif opsel == #optable then
				menu[4][3] = 1
			else
				menu[4][3] = 3
			end
			menu[5] = {}
			menu[5][1] = "Change"
			menu[5][2] = change
			if change == 0 then
				menu[5][2] = "Any"
				menu[5][3] = 2
			elseif change == 100 then --?
				menu[5][3] = 1
			else
				menu[5][3] = 3
			end
			menu[6] = { "Compare", "", 0 }
			menu[7] = { "---", "", 32 }
			for num, list in ipairs(matches) do
				for num2, match in ipairs(list) do
					if #menu > 50 then
						break
					end
					menu[#menu + 1] = { string.format("%x %x %x", match.addr, match.oldval, match.newval), "", 0 }
				end
			end
		end
		return menu
	end

	local function menu_callback(index, event)
		if index == 1 then
			if event == "left" then
				if devsel ~= 1 then
					devsel = devsel - 1
					return true
				end
			elseif event == "right" then
				if devsel ~= #devtable then
					devsel = devsel + 1
					return true
				end
			end
		elseif index == 2 then
			if event == "select" then
				menu_blocks = {}
				matches = {}
				for num, region in ipairs(devtable[devsel].ram) do
					menu_blocks[num] = cheat.save(devtable[devsel].space, region.offset, region.size)
				end
				return true
			end
		elseif index == 4 then
			if event == "left" then
				if opsel ~= 1 then
					opsel = opsel - 1
					return true
				end
			elseif event == "right" then
				if opsel ~= #optable then
					opsel = opsel + 1
					return true
				end
			end
		elseif index == 5 then
			if event == "left" then
				if change ~= 0 then
					change = change - 1
					return true
				end
			elseif event == "right" then
				if change ~= 100 then
					change = change + 1
					return true
				end
			end
		elseif index == 6 then
			if event == "select" then
				matches = {}
				for num, block in ipairs(menu_blocks) do
					matches[#matches + 1] = cheat.comp(block, optable[opsel], change)
				end
				return true
			end
		elseif index > 7 then 
			if event == "select" then
				-- write out a script?
			end
		end
		return false
	end
	emu.register_menu(menu_callback, menu_populate, "Cheat Finder")
end

return exports
