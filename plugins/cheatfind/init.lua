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
		local ref = {} -- this is a helper for comparing two match lists
		local format = ""
		local size = olddata.size

		if endian == 1 then
			format = format .. ">"
		else
			format = format .. "<"
		end
		if width == 16 then
			size = size & 0xfffe
			if signed == 1 then
				format = format .. "h"
			else
				format = format .. "H"
			end
		elseif width == 32 then
			size = size & 0xfffc
			if signed == 1 then
				format = format .. "l"
			else
				format = format .. "L"
			end
		elseif width == 64 then
			size = size & 0xfff8
			if signed == 1 then
				format = format .. "j"
			else
				format = format .. "J"
			end
		else
			if signed == 1 then
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
						ref[ret[#ret].addr] = #ret
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
						ref[ret[#ret].addr] = #ret
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
					ref[ret[#ret].addr] = #ret
				end
			end
		elseif oper == "!" or oper == "ne" then
			for i = 1, size do
				local old = string.unpack(format, olddata.block, i)
				local new = string.unpack(format, newdata.block, i)
				if old ~= new then
					ret[#ret + 1] = { addr = olddata.start + i - 1,
					oldval = old,
					newval = new}
					ref[ret[#ret].addr] = #ret
				end
			end
		elseif oper == "^" or oper == "xor" then
			if val >= width then
				return {}
			end
			for i = 1, size do
				local old = string.unpack(format, olddata.block, i)
				local new = string.unpack(format, newdata.block, i)
				if ((old ~ new) & (1 << val)) ~= 0 then
					ret[#ret + 1] = { addr = olddata.start + i - 1,
					oldval = old,
					newval = new}
					ref[ret[#ret].addr] = #ret
				end
			end
		end
		return ret, ref
	end

	-- compare successive pairs of blocks from a table returning only addresses that match every comparison
	function cheat.compall(dattable, oper, val, signed, width, endian)
		if #dattable < 2 then
			return {}
		end
		local matches, refs = cheat.comp(dattable[1], dattable[2], oper, val, signed, width, endian)
		local nonmatch = {}
		for i = 2, #dattable - 1 do
			local matchnext, refsnext = cheat.comp(dattable[i], dattable[i+1], oper, val, signed, width, endian)
			for addr, ref in pairs(refs) do
				if not refsnext[addr] then
					nonmatch[ref] = true
					refs[addr] = nil
				else
					matches[ref].newval = matchnext[refsnext[addr]].newval
				end
			end
		end
		local resort = {}
		for num, match in pairs(matches) do
			if not nonmatch[num] then
				resort[#resort + 1] = match
			end
		end
		return resort 
	end
			 

	-- compare a data block to the current state
	function cheat.compcur(olddata, oper, val, signed, width, endian)
		local newdata = cheat.save(olddata.space, olddata.start, olddata.size, olddata.space)
		return cheat.comp(olddata, newdata, oper, val, signed, width, endian)
	end

	_G.cf = cheat

	local devtable = {}
	local devsel = 1
	local devcur = 1
	local bitwidth = 3
	local signed = 0
	local endian = 0
	local optable = { "<", ">", "=", "!", "^" }
	local opsel = 1
	local bit = 0
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
		devcur = 1
		bitwidth = 3
		signed = 0
		endian = 0
		opsel = 1
		bit = 0
		value = 0
		leftop = 1
		rightop = 1
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

		menu[midx.region] = { "CPU or RAM", devtable[devsel].tag, 0 }
		if #devtable == 1 then
			menu[midx.region][3] = 0
		else
			menu_lim(devsel, 1, #devtable, menu[midx.region])
		end
		menu[midx.init] = { "Init", "", 0 }
		if #menu_blocks ~= 0 then
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
			if optable[opsel] == "^" then
				menu[midx.val] = { "Bit", bit, "" }
				menu_lim(bit, 0, (1 << bitwidth), menu[midx.val])
			else
				menu[midx.val] = { "Value", value, "" }
				menu_lim(value, 0, 100, menu[midx.val]) -- max value?
				if value == 0 then
					menu[midx.val][2] = "Any"
				end
			end
			menu[midx.lop] = { "Left operand", leftop, "" }
			menu_lim(leftop, 1, #menu_blocks[1] + 1, menu[midx.lop])
			if leftop == #menu_blocks[1] + 1 then
				menu[midx.lop][2] = "All"
			end
			menu[midx.rop] = { "Right operand", rightop, "" }
			menu_lim(rightop, 1, #menu_blocks[1] + 1, menu[midx.rop])
			if rightop == #menu_blocks[1] + 1 then
				menu[midx.rop][2] = "Current"
			end
			menu[midx.comp] = { "Compare", "", 0 }
			if #matches ~= 0 then
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
					if not match.mode then
						match.mode = 1
					end
					if match.mode == 1 then
						menu[#menu][2] = "Test"
					else
						menu[#menu][2] = "Write"
					end
					menu_lim(match.mode, 1, 2, menu[#menu])
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
			if (event == "left" or event == "right") and #menu_blocks ~= 0 then
				manager:machine():popmessage("Changes to this only take effect when Init is selected")
			end
			devsel = incdec(devsel, 1, #devtable)
			return true
		elseif index == midx.init then
			if event == "select" then
				menu_blocks = {}
				matches = {}
				for num, region in ipairs(devtable[devcur].ram) do
					menu_blocks[num] = {}
					menu_blocks[num][1] = cheat.save(devtable[devcur].space, region.offset, region.size)
				end
				manager:machine():popmessage("Data cleared and current state saved")
				ret = true
			end
			devcur = devsel
		elseif index == midx.save then
			if event == "select" then
				for num, region in ipairs(devtable[devcur].ram) do
					menu_blocks[num][#menu_blocks[num] + 1] = cheat.save(devtable[devcur].space, region.offset, region.size)
				end
				manager:machine():popmessage("Current state saved")
				ret = true
			end
		elseif index == midx.width then
			bitwidth = incdec(bitwidth, 3, 6)
			if event == "left" or event == "right" then
				bit = 0
			end
		elseif index == midx.signed then
			signed = incdec(signed, 0, 1)
		elseif index == midx.endian then
			endian = incdec(endian, 0, 1)
		elseif index == midx.op then
			if event == "left" or event == "right" or event == "comment" then
				if optable[opsel] == "<" then
					manager:machine():popmessage("Left less than right, value is difference")
				elseif optable[opsel] == ">" then
					manager:machine():popmessage("Left greater than right, value is difference")
				elseif optable[opsel] == "=" then
					manager:machine():popmessage("Left equal to right, value is ignored")
				elseif optable[opsel] == "!" then
					manager:machine():popmessage("Left not equal to right, value is ignored")
				elseif optable[opsel] == "^" then
					manager:machine():popmessage("Left bit different than right")
				end
			end
			opsel = incdec(opsel, 1, #optable)
		elseif index == midx.val then
			if optable[opsel] == "^" then
				bit = incdec(bit, 0, (1 << bitwidth))
			else
				value = incdec(value, 0, 100)
			end
		elseif index == midx.lop then
			leftop = incdec(leftop, 1, #menu_blocks[1] + 1)
			if (event == "left" or event == "right" or event == "comment") and leftop == #menu_blocks[1] + 1 then
				manager:machine():popmessage("Any compares every saved data pair and returns only matches that are true for all.  Right operand is ignored.")
			end
		elseif index == midx.rop then
			rightop = incdec(rightop, 1, #menu_blocks[1] + 1)
		elseif index == midx.comp then
			if event == "select" then
				matches = {}
				local val
				if optable[opsel] == "^" then
					val = bit
				else
					val = value
				end
				for num = 1, #menu_blocks do
					if leftop == #menu_blocks[1] + 1 then
						matches[#matches + 1] = cheat.compall(menu_blocks[num], optable[opsel], val, signed,
										      1 << bitwidth, endian)
					elseif rightop == #menu_blocks[1] + 1 then
						matches[#matches + 1] = cheat.compcur(menu_blocks[num][leftop], optable[opsel], val,
										      signed, 1 << bitwidth, endian)
					else
						matches[#matches + 1] = cheat.comp(menu_blocks[num][leftop], menu_blocks[num][rightop],
										   optable[opsel], val, signed, 1 << bitwidth, endian)
					end
				end
				ret = true
			end
		elseif index == midx.match then
			matchsel = incdec(matchsel, 0, #matches)
		elseif index > midx.match then 
			local match = matches[matchsel][index - midx.match]
			match.mode = incdec(match.mode, 1, 2)
			if event == "select" then
				local dev = devtable[devcur]
				local cheat = { desc = string.format("Test cheat at addr %08x", match.addr), script = {} }
				if dev.space.shortname then
					cheat.ram = dev.tag
					cheat.script.on = "ram:write(" .. match.addr .. "," .. match.newval .. ")"
				else
					cheat.space = { cpu = { tag = dev.tag, type = "program" } }
					cheat.script.on = "cpu:write_u8(" .. match.addr .. "," .. match.newval .. ")"
				end
				if match.mode == 1 then
					if not _G.ce then
						manager:machine():popmessage("Cheat engine not available")
					else
						_G.ce.inject(cheat)
					end
				else

				end

			end
		end
		devsel = devcur
		return ret
	end
	emu.register_menu(menu_callback, menu_populate, "Cheat Finder")
end

return exports
