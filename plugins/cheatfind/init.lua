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
				ram[tag].size = emu.item(device.items["0/m_size"]):read(0)
			end
		end
		return ram
	end

	-- save data block
	function cheat.save(space, start, size)
		local data = { block = "", start = start, size = size, space = space }
		if space.shortname then
			if space:shortname() == "ram" then
				data.block = emu.item(space.items["0/m_pointer"]):read_block(start, size)
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
	function cheat.comp(olddata, newdata, oper, val, signed, width, endian)
		local ret = {}
		local format = ""
		local size = olddata.size

		if endian == 1 then
			format = format .. ">"
		else
			format = format .. "<"
		end
		if width == 16 then
			size = size & 0xfffe
			if signed == "signed" then
				format = format .. "h"
			else
				format = format .. "H"
			end
		elseif width == 32 then
			size = size & 0xfffc
			if signed == "signed" then
				format = format .. "l"
			else
				format = format .. "L"
			end
		elseif width == 64 then
			size = size & 0xfff8
			if signed == "signed" then
				format = format .. "j"
			else
				format = format .. "J"
			end
		else
			if signed == "signed" then
				format = format .. "b"
			else
				format = format .. "B"
			end
		end

		if olddata.start ~= newdata.start or olddata.size ~= newdata.size then
			return {} 
		end
		if not val then
			val = 0
		end
		if oper == "+" or oper == "inc" or oper == "<" or oper == "lt" then
			for i = 1, size do
				local old = string.unpack(format, olddata.block, i)
				local new = string.unpack(format, newdata.block, i)
				if old < new then
					if (val > 0 and (old + val) == new) or val == 0 then
						ret[#ret + 1] = { addr = olddata.start + i - 1,
						oldval = old,
						newval = new}
					end
				end
			end
		elseif oper == "-" or oper == "dec" or oper == ">" or oper == "gt" then
			for i = 1, size do
				local old = string.unpack(format, olddata.block, i)
				local new = string.unpack(format, newdata.block, i)
				if old > new then
					if (val > 0 and (old - val) == new) or val == 0 then
						ret[#ret + 1] = { addr = olddata.start + i - 1,
						oldval = old,
						newval = new}
					end
				end
			end
		elseif oper == "=" or oper == "eq" then
			for i = 1, size do
				local old = string.unpack(format, olddata.block, i)
				local new = string.unpack(format, newdata.block, i)
				if old == new then
					ret[#ret + 1] = { addr = olddata.start + i - 1,
					oldval = old,
					newval = new}
				end
			end
		end
		return ret
	end

	-- compare a data block to the current state
	function cheat.compcur(olddata, oper, val, signed, width, endian)
		local newdata = cheat.save(olddata.space, olddata.start, olddata.size, olddata.space)
		return cheat.comp(olddata, newdata, oper, val, signed, width, endian)
	end

	_G.cf = cheat

	local devtable = {}
	local devsel = 1
	local bitwidth = 3
	local signed = 0
	local endian = 0
	local optable = { "<", ">", "=" }
	local opsel = 1
	local value = 0
	local leftop = 1
	local rightop = 0
	local matches = {}
	local matchsel = 1
	local menu_blocks = {}
	local midx = { region = 1, init = 2, save = 3, width = 5, signed = 6, endian = 7, op = 8, val = 9,
	lop = 10, rop = 11, comp = 12, match = 14 }

	local function start()
		devtable = {}
		devsel = 1
		devsel = 1
		bitwidth = 3
		signed = 0
		endian = 0
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
			devtable[#devtable + 1] = { tag = tag, space = ram.dev, ram = {{ offset = 0, size = ram.size }} }
		end
	end

	emu.register_start(start)

	local function menu_populate()
		local menu = {}

		local function menu_lim(val, min, max, menuitem)
			if val == min then
				menuitem[3] = "r"
			elseif val == max then
				menuitem[3] = "l"
			else
				menuitem[3] = "lr"
			end
		end

		menu[midx.region] = { "CPU or RAM", devtable[devsel].tag, "" }
		if #devtable == 1 then
			menu[midx.region][3] = 0
		else
			menu_lim(devsel, 1, #devtable, menu[midx.region])
		end
		menu[midx.init] = { "Init", "", 0 }
		if next(menu_blocks) then
			menu[midx.save] = { "Save current", "", 0 }
			menu[midx.save + 1] = { "---", "", "off" }
			menu[midx.width] = { "Bit Width", 1 << bitwidth , 0 }
			menu_lim(bitwidth, 3, 6, menu[midx.width])
			menu[midx.signed] = { "Signed", "false", 0 }
			menu_lim(signed, 0, 1, menu[midx.signed])
			if signed == 1 then
				menu[midx.signed][2] = "true"
			end
			menu[midx.endian] = { "Endian", "little", 0 }
			menu_lim(endian, 0, 1, menu[midx.endian])
			if endian == 1 then
				menu[midx.endian][2] = "big"
			end
			menu[midx.op] = { "Operator", optable[opsel], "" }
			menu_lim(opsel, 1, #optable, menu[midx.op])
			menu[midx.val] = { "Value", value, "" }
			menu_lim(value, 0, 100, menu[midx.val]) -- max value?
			if value == 0 then
				menu[midx.val][2] = "Any"
			end
			menu[midx.lop] = { "Left operand", leftop, "" }
			if #menu_blocks == 1 then
				menu[midx.lop][3] = 0
			else
				menu_lim(leftop, 1, #menu_blocks, menu[midx.lop])
			end
			menu[midx.rop] = { "Right operand", rightop, "" }
			menu_lim(rightop, 0, #menu_blocks, menu[midx.rop])
			if rightop == 0 then
				menu[midx.rop][2] = "Current"
			end
			menu[midx.comp] = { "Compare", "", 0 }
			if next(matches) then
				menu[midx.comp + 1] = { "---", "", "off" }
				menu[midx.match] = { "Match block", matchsel, "" }
				if #matches == 1 then
					menu[midx.match][3] = 0
				else
					menu_lim(matchsel, 1, #matches, menu[midx.match])
				end
				for num2, match in ipairs(matches[matchsel]) do
					if #menu > 50 then
						break
					end
					local numform = ""
					if bitwidth == 4 then
						numform = " %04x"
					elseif bitwidth == 5 then
						numform = " %08x"
					elseif bitwidth == 6 then
						numform = " %016x"
					else
						numform = " %02x"
					end
					menu[#menu + 1] = { string.format("%08x" .. numform .. numform, match.addr, match.oldval, 
					                                  match.newval), "", 0 }
				end
			end
		end
		return menu
	end

	local function menu_callback(index, event)
		local ret = false

		local function incdec(val, min, max)
			if event == "left" and val ~= min then
				val = val - 1
				ret = true
			elseif event == "right" and val ~= max then
				val = val + 1
				ret = true
			end
			return val
		end

		if index == midx.region then
			devsel = incdec(devsel, 1, #devtable)
		elseif index == midx.init then
			if event == "select" then
				menu_blocks = {{}}
				matches = {}
				for num, region in ipairs(devtable[devsel].ram) do
					menu_blocks[1][num] = cheat.save(devtable[devsel].space, region.offset, region.size)
				end
				manager:machine():popmessage("Data cleared and current state saved")
				ret = true
			end
		elseif index == midx.init then
			if event == "select" then
				menu_blocks = {{}}
				matches = {}
				for num, region in ipairs(devtable[devsel].ram) do
					menu_blocks[1][num] = cheat.save(devtable[devsel].space, region.offset, region.size)
				end
				manager:machine():popmessage("Data cleared and current state saved")
				ret = true
			end
		elseif index == midx.save then
			if event == "select" then
				menu_blocks[#menu_blocks + 1] = {}
				for num, region in ipairs(devtable[devsel].ram) do
					menu_blocks[#menu_blocks][num] = cheat.save(devtable[devsel].space, region.offset, region.size)
				end
				manager:machine():popmessage("Current state saved")
				ret = true
			end
		elseif index == midx.width then
			bitwidth = incdec(bitwidth, 3, 6)
		elseif index == midx.signed then
			signed = incdec(signed, 0, 1)
		elseif index == midx.endian then
			endian = incdec(endian, 0, 1)
		elseif index == midx.op then
			if event == "up" or event == "down" or event == "comment" then
				if optable[opsel] == "<" then
					manager:machine():popmessage("Left less than right, value is difference")
				elseif optable[opsel] == ">" then
					manager:machine():popmessage("Left greater than right, value is difference")
				elseif optable[opsel] == "=" then
					manager:machine():popmessage("Left equal to right, value is ignored")
				end
			else
				opsel = incdec(opsel, 1, #optable)
			end
		elseif index == midx.val then
			value = incdec(value, 0, 100)
		elseif index == midx.lop then
			leftop = incdec(leftop, 1, #menu_blocks)
		elseif index == midx.rop then
			rightop = incdec(rightop, 0, #menu_blocks)
		elseif index == midx.comp then
			if event == "select" then
				matches = {}
				for num = 1, #menu_blocks[1] do
					if rightop == 0 then
						matches[#matches + 1] = cheat.compcur(menu_blocks[leftop][num], optable[opsel], value,
						signed, 1 << bitwidth, endian)
					else
						matches[#matches + 1] = cheat.comp(menu_blocks[leftop][num], menu_blocks[rightop][num],
						optable[opsel], value, signed, 1 << bitwidth, endian)
					end
				end
				ret = true
			end
		elseif index == midx.match then
			matchsel = incdec(matchsel, 0, #matches)
		elseif index > midx.match then 
			if event == "select" then
				-- write out a script?
			end
		end
		return ret
	end
	emu.register_menu(menu_callback, menu_populate, "Cheat Finder")
end

return exports
