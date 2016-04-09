-- license:BSD-3-Clause
-- copyright-holders:Carl
-- This includes a library of functions to be used at the Lua console as cf.getspaces() etc...
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

	-- compare two data blocks
	function cheat.comp(olddata, newdata, oper, val)
		local ret = {}
		if olddata.start ~= newdata.start or olddata.size ~= newdata.size then
			return {} 
		end
		if not val then
			val = 0
		end
		if oper == "+" or oper == "inc" or oper == "<" or oper == "lt" then
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
		elseif oper == "-" or oper == "dec" or oper == ">" or oper == "gt" then
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
		elseif oper == "=" or oper == "eq" then
			for i = 1, olddata.size do
				local old = string.unpack("B", olddata.block, i)
				local new = string.unpack("B", newdata.block, i)
				if old == new then
					ret[#ret + 1] = { addr = olddata.start + i,
							  oldval = old,
							  newval = new}
				end
			end
		end
		return ret
	end

	-- compare a data block to the current state
	function cheat.compcur(olddata, oper, val)
		local newdata = cheat.save(olddata.dev, olddata.start, olddata.size, olddata.space)
		return cheat.comp(olddata, newdata, oper, val)
	end
	
	_G.cf = cheat
	
	local devtable = {}
	local devsel = 1
	local optable = { "<", ">", "=" }
	local opsel = 1
	local value = 0
	local leftop = 1
	local rightop = 0
	local matches = {}
	local matchsel = 1
	local menu_blocks = {}
	local midx = { region = 1, init = 2, save = 3, op = 5, val = 6, lop = 7, rop = 8, comp = 9, match = 11 }

	local function start()
		devtable = {}
		devsel = 1
		opsel = 1
		value = 0
		leftop = 1
		rightop = 0
		matches = {}
		matchsel = 1
		menu_blocks = {}

		table = cheat.getspaces()
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
		menu[midx.region] = { "CPU or RAM", devtable[devsel].tag, "" }
		if #devtable == 1 then
			menu[midx.region][3] = 0
		elseif devsel == 1 then
			menu[midx.region][3] = "r"
		elseif devsel == #devtable then
			menu[midx.region][3] = "l"
		else
			menu[midx.region][3] = "lr"
		end
		menu[midx.init] = { "Init", "", 0 }
		if next(menu_blocks) then
			menu[midx.save] = { "Save current", "", 0 }
			menu[midx.save + 1] = { "---", "", "off" }
			menu[midx.op] = { "Operator", optable[opsel], "" }
			if opsel == 1 then
				menu[midx.op][3] = "r"
			elseif opsel == #optable then
				menu[midx.op][3] = "l"
			else
				menu[midx.op][3] = "lr"
			end
			menu[midx.val] = { "Value", value, "" }
			if value == 0 then
				menu[midx.val][2] = "Any"
				menu[midx.val][3] = "r"
			elseif value == 100 then --?
				menu[midx.val][3] = "l"
			else
				menu[midx.val][3] = "lr"
			end
			menu[midx.lop] = { "Left operand", leftop, "" }
			if #menu_blocks == 1 then
				menu[midx.lop][3] = 0
			elseif leftop == 1 then
				menu[midx.lop][3] = "r"
			elseif leftop == #menu_blocks then
				menu[midx.lop][3] = "l"
			else
				menu[midx.lop][3] = "lr"
			end
			menu[midx.rop] = { "Right operand", leftop, "" }
			if rightop == 0 then
				menu[midx.rop][2] = "Current"
				menu[midx.rop][3] = "r"
			elseif rightop == #menu_blocks then
				menu[midx.rop][3] = "l"
			else
				menu[midx.rop][3] = "lr"
			end
			menu[midx.comp] = { "Compare", "", 0 }
			if next(matches) then
				menu[midx.comp + 1] = { "---", "", "off" }
				menu[midx.match] = { "Match block", matchsel, "" }
				if #matches == 1 then
					menu[midx.match][3] = 0
				elseif matchsel == 1 then
					menu[midx.match][3] = "r"
				elseif matchsel == #matches then
					menu[midx.match][3] = "l"
				else
					menu[midx.match][3] = "lr"
				end
				for num2, match in ipairs(matches[matchsel]) do
					if #menu > 50 then
						break
					end
					menu[#menu + 1] = { string.format("%08x %02x %02x", match.addr, match.oldval, match.newval), "", 0 }
				end
			end
		end
		return menu
	end

	local function menu_callback(index, event)
		if index == midx.region then
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
		elseif index == midx.init then
			if event == "select" then
				menu_blocks = {{}}
				matches = {}
				for num, region in ipairs(devtable[devsel].ram) do
					menu_blocks[1][num] = cheat.save(devtable[devsel].space, region.offset, region.size)
				end
				manager:machine():popmessage("Data cleared and current state saved")
				return true
			end
		elseif index == midx.save then
			if event == "select" then
				for num, region in ipairs(devtable[devsel].ram) do
					menu_blocks[#menu_blocks][num] = cheat.save(devtable[devsel].space, region.offset, region.size)
				end
				manager:machine():popmessage("Current state saved")
			return false
		end
		elseif index == midx.op then
			if event == "up" or event == "down" or event == "comment" then
				if optable[opsel] == "<" then
					manager:machine():popmessage("Left less than right, value is difference")
				elseif optable[opsel] == ">" then
					manager:machine():popmessage("Left greater than right, value is difference")
				elseif optable[opsel] == "=" then
					manager:machine():popmessage("Left equal to right, value is ignored")
				end
			elseif event == "left" then
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
		elseif index == midx.val then
			if event == "left" then
				if value ~= 0 then
					value = value - 1
					return true
				end
			elseif event == "right" then
				if value ~= 100 then
					value = value + 1
					return true
				end
			end
		elseif index == midx.lop then
			if event == "left" then
				if leftop ~= 1 then
					leftop = leftop - 1
					return true
				end
			elseif event == "right" then
				if leftop ~= #menu_blocks then
					leftop = leftop + 1
					return true
				end
			end
		elseif index == midx.rop then
			if event == "left" then
				if rightop ~= 0 then
					rightop = rightop - 1
					return true
				end
			elseif event == "right" then
				if rightop ~= #menu_blocks then
					rightop = rightop + 1
					return true
				end
			end
		elseif index == midx.comp then
			if event == "select" then
				matches = {}
				for num = 1, #menu_blocks[1] do
					if rightop == 0 then
						matches[#matches + 1] = cheat.compcur(menu_blocks[leftop][num], optable[opsel], value)
					else
						matches[#matches + 1] = cheat.comp(menu_blocks[leftop][num], menu_blocks[rightop][num],
										   optable[opsel], value)
					end
				end
				return true
			end
		elseif index == midx.match then
			if event == "left" then
				if matchsel ~= 1 then
					matchsel = matchsel - 1
					return true
				end
			elseif event == "right" then
				if matchsel ~= #matches then
					matchsel = matchsel + 1
					return true
				end
			end
		elseif index > midx.match then 
			if event == "select" then
				-- write out a script?
			end
		end
		return false
	end
	emu.register_menu(menu_callback, menu_populate, "Cheat Finder")
end

return exports
