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
			local temp = {}
			local j = 1
			for i = start, start + size do
				if j < 65536 then
					temp[j] = string.pack("B", space:read_u8(i))
					j = j + 1
				else
					block = block .. table.concat(temp) .. string.pack("B", space:read_u8(i))
					temp = {}
					j = 1
				end
			end
			block = block .. table.concat(temp)
			data.block = block
		end
		return data
	end

	-- compare two data blocks, format is as lua string.unpack, bne and beq val is table of masks
	function cheat.comp(newdata, olddata, oper, format, val, bcd)
		local ret = {}
		local ref = {} -- this is a helper for comparing two match lists
		local bitmask = nil

		local function bne(a, b, val, addr)
			if type(val) ~= "table" then
				bitmask = a ~ b
				return bitmask ~= 0
			elseif not val[addr] then
				return false
			else
				bitmask = (a ~ b) & val[addr]
				return bitmask ~= 0
			end
		end

		local function beq(a, b, val, addr)
			if type(val) ~= "table" then
				bitmask = ~a ~ b
				return bitmask ~= 0
			elseif not val[addr] then
				return false
			else
				bitmask = (~a ~ b) & val[addr]
				return bitmask ~= 0
			end
		end

		local cfoper = { 
			lt = function(a, b, val) return (a < b and val == 0) or (val > 0 and (a + val) == b) end,
			gt = function(a, b, val) return (a > b and val == 0) or (val > 0 and (a - val) == b) end,
			eq = function(a, b, val) return a == b end,
			ne = function(a, b, val) return (a ~= b and val == 0) or
				(val > 0 and ((a - val) == b or (a + val) == b)) end,
			ltv = function(a, b, val) return a < val end,
			gtv = function(a, b, val) return a > val end,
			eqv = function(a, b, val) return a == val end,
			nev = function(a, b, val) return a ~= val end,
			bne = bne, beq = beq }

		local function check_bcd(val)
			local a = val + 0x0666666666666666
			a = a ~ val
			return (a & 0x1111111111111110) == 0
		end

		local function frombcd(val)
			local result = 0
			local mul = 1
			while val ~= 0 do
				result = result + ((val % 16) * mul)
				val = val >> 4
				mul = mul * 10
			end
			return result
		end

		if not newdata and oper:sub(3, 3) == "v" then
			newdata = olddata
		end
		if olddata.start ~= newdata.start or olddata.size ~= newdata.size or not cfoper[oper] then
			return {} 
		end
		if not val then
			val = 0
		end

		for i = 1, olddata.size do
			local old = string.unpack(format, olddata.block, i)
			local new = string.unpack(format, newdata.block, i)
			local oldc, newc = old, new
			local comp = false
			local addr = olddata.start + i - 1
			if not bcd or (check_bcd(old) and check_bcd(new)) then
				if bcd then
					oldc = frombcd(old)
					newc = frombcd(new)
				end
				if cfoper[oper](newc, oldc, val, addr) then
					ret[#ret + 1] = { addr = addr,
					oldval = old,
					newval = new,
					bitmask = bitmask }
					ref[ret[#ret].addr] = #ret
				end
			end
		end
		return ret, ref
	end

	local function check_val(oper, val, matches)
		if oper ~= "beq" and oper ~= "bne" then
			return val
		elseif not matches or not matches[1].bitmask then
			return nil
		end
		local masks = {}
		for num, match in pairs(matches) do
			masks[match.addr] = match.bitmask
		end
		return masks
	end

	-- compare two blocks and filter by table of previous matches
	function cheat.compnext(newdata, olddata, oldmatch, oper, format, val, bcd)
		local matches, refs = cheat.comp(newdata, olddata, oper, format, check_val(oper, val, oldmatch), bcd)
		local nonmatch = {}
		local oldrefs = {}
		for num, match in pairs(oldmatch) do
			oldrefs[match.addr] = num
		end
		for addr, ref in pairs(refs) do
			if not oldrefs[addr] then
				nonmatch[ref] = true
				refs[addr] = nil
			else
				matches[ref].oldval = oldmatch[oldrefs[addr]].oldval
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
	function cheat.compcur(olddata, oper, format, val, bcd)
		local newdata = cheat.save(olddata.space, olddata.start, olddata.size, olddata.space)
		return cheat.comp(newdata, olddata, oper, format, val, bcd)
	end

	-- compare a data block to the current state and filter
	function cheat.compcurnext(olddata, oldmatch, oper, format, val, bcd)
		local newdata = cheat.save(olddata.space, olddata.start, olddata.size, olddata.space)
		return cheat.compnext(newdata, olddata, oldmatch, oper, format, val, bcd)
	end


	_G.cf = cheat

	local devtable = {}
	local devsel = 1
	local devcur = 1
	local formtable = { "B", "b", "<H", ">H", "<h", ">h", "<L", ">L", "<l", ">l", "<J", ">J", "<j", ">j" }
	local formname = { "u8", "s8", "little u16", "big u16", "little s16", "big s16",
			   "little u32", "big u32", "little s32", "big s32", "little u64", "big u64", "little s64", "big s64" }
	local width = 1
	local bcd = 0
	local optable = { "lt", "gt", "eq", "ne", "beq", "bne", "ltv", "gtv", "eqv", "nev" }
	local opsel = 1
	local value = 0
	local leftop = 2
	local rightop = 1
	local matches = {}
	local matchsel = 0
	local menu_blocks = {}
	local midx = { region = 1, init = 2, save = 4, comp = 5, lop = 6, op = 7, rop = 8, val = 9,
		       width = 11,  bcd = 12, undo = 13,  match = 15, watch = 0 }
	local watches = {}

	local function start()
		devtable = {}
		devsel = 1
		devcur = 1
		width = 1
		bcd = 0
		opsel = 1
		value = 0
		leftop = 2
		rightop = 1
		matches = {}
		matchsel = 0
		menu_blocks = {}
		watches = {}

		local space_table = cheat.getspaces()
		for tag, list in pairs(space_table) do
			if list.program then
				local ram = {}
				for num, entry in pairs(list.program.map) do
					if entry.writetype == "ram" then
						ram[#ram + 1] = { offset = entry.offset, size = entry.endoff - entry.offset }
					end
				end
				if next(ram) then
					if tag == ":maincpu" then
						table.insert(devtable, 1, { tag = tag, space = list.program, ram = ram })
					else
						devtable[#devtable + 1] = { tag = tag, space = list.program, ram = ram }
					end
				end
			end
		end
		space_table = cheat.getram()
		for tag, ram in pairs(space_table) do
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
		menu[midx.init] = { "Start new search", "", 0 }
		if #menu_blocks ~= 0 then
			menu[midx.init + 1] = { "---", "", "off" }
			menu[midx.save] = { "Save current -- #" .. #menu_blocks[1] + 1, "", 0 }
			menu[midx.comp] = { "Compare", "", 0 }
			menu[midx.lop] = { "Left operand", leftop, "" }
			menu_lim(leftop, 1, #menu_blocks[1] + 1, menu[midx.lop])
			if leftop == #menu_blocks[1] + 1 then
				menu[midx.lop][2] = "Current"
			end
			menu[midx.op] = { "Operator", optable[opsel], "" }
			menu_lim(opsel, 1, #optable, menu[midx.op])
			menu[midx.rop] = { "Right operand", rightop, "" }
			menu_lim(rightop, 1, #menu_blocks[1], menu[midx.rop])
			menu[midx.val] = { "Value", value, "" }
			menu_lim(value, 0, 100, menu[midx.val]) -- max value?
			if value == 0 and optable[opsel]:sub(3, 3) ~= "v" then
				menu[midx.val][2] = "Any"
			end
			menu[midx.val + 1] = { "---", "", "off" }
			menu[midx.width] = { "Data Format", formname[width], 0 }
			menu_lim(width, 1, #formtable, menu[midx.width])
			menu[midx.bcd] = { "BCD", "Off", 0 }
			menu_lim(bcd, 0, 1, menu[midx.bcd])
			if bcd == 1 then
				menu[midx.bcd][2] = "On"
			end
			menu[midx.undo] = { "Undo last search -- #" .. #matches, "", 0 }
			if #matches ~= 0 then
				menu[midx.undo + 1] = { "---", "", "off" }
				menu[midx.match] = { "Match block", matchsel, "" }
				menu_lim(matchsel, 0, #matches[#matches], menu[midx.match])
				if matchsel == 0 then
					menu[midx.match][2] = "All"
				end
				local function mpairs(sel, list)
					if #list == 0 then
						return function() end, nil, nil
					end
					if sel ~= 0 then
						return ipairs(list[sel])
					end
					local function mpairs_it(list, i)
						local match
						i = i + 1
						local sel = i
						for j = 1, #list do
							if sel <= #list[j] then
								match = list[j][sel]
								break
							else
								sel = sel - #list[j]
							end
						end
						if not match then
							return
						end
						return i, match
					end
					return mpairs_it, list, 0
				end
				for num2, match in mpairs(matchsel, matches[#matches]) do
					if #menu > 100 then
						break
					end
					local numform = ""
					local bitwidth = formtable[width]:sub(2, 2):lower()
					if bitwidth == "h" then
						numform = " %04x"
					elseif bitwidth == "l" then
						numform = " %08x"
					elseif bitwidth == "j" then
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
					elseif match.mode == 2 then
						menu[#menu][2] = "Write"
					else
						menu[#menu][2] = "Watch"
					end
					menu_lim(match.mode, 1, 3, menu[#menu])
				end
			end
			if #watches ~= 0 then
				menu[#menu + 1] = { "Clear Watches", "", 0 }
				midx.watch = #menu
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
				manager:machine():popmessage("Changes to this only take effect when \"Start new search\" is selected")
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
				watches = {}
				leftop = 2
				rightop = 1
				matchsel = 0
				ret = true
			end
			devcur = devsel
		elseif index == midx.undo then
			if event == "select" and #matches > 0 then
				matches[#matches] = nil
			end
			ret = true
		elseif index == midx.save then
			if event == "select" then
				for num, region in ipairs(devtable[devcur].ram) do
					menu_blocks[num][#menu_blocks[num] + 1] = cheat.save(devtable[devcur].space, region.offset, region.size)
				end
				manager:machine():popmessage("Current state saved")
				leftop = (leftop == #menu_blocks[1]) and #menu_blocks[1] + 1 or leftop
				rightop = (rightop == #menu_blocks[1] - 1) and #menu_blocks[1] or rightop
				ret = true
			end
		elseif index == midx.op then
			opsel = incdec(opsel, 1, #optable)
			if event == "left" or event == "right" or event == "comment" then
				if optable[opsel] == "lt" then
					manager:machine():popmessage("Left less than right, value is difference")
				elseif optable[opsel] == "gt" then
					manager:machine():popmessage("Left greater than right, value is difference")
				elseif optable[opsel] == "eq" then
					manager:machine():popmessage("Left equal to right, value is ignored")
				elseif optable[opsel] == "ne" then
					manager:machine():popmessage("Left not equal to right, value is difference")
				elseif optable[opsel] == "beq" then
					manager:machine():popmessage("Left equal to right with bitmask, value is ignored")
				elseif optable[opsel] == "bne" then
					manager:machine():popmessage("Left not equal to right with bitmask, value is ignored")
				elseif optable[opsel] == "ltv" then
					manager:machine():popmessage("Left less than value, right is ignored")
				elseif optable[opsel] == "gtv" then
					manager:machine():popmessage("Left greater than value, right is ignored")
				elseif optable[opsel] == "eqv" then
					manager:machine():popmessage("Left equal to value, right is ignored")
				elseif optable[opsel] == "nev" then
					manager:machine():popmessage("Left not equal to value, right is ignored")
				end
			end
			ret = true
		elseif index == midx.val then
			value = incdec(value, 0, 100)
		elseif index == midx.lop then
			leftop = incdec(leftop, 1, #menu_blocks[1] + 1)
		elseif index == midx.rop then
			rightop = incdec(rightop, 1, #menu_blocks[1])
		elseif index == midx.width then
			width = incdec(width, 1, #formtable)
		elseif index == midx.bcd then
			bcd = incdec(bcd, 0, 1)
		elseif index == midx.comp then
			if event == "select" then
				local count = 0
				if #matches == 0 then
					matches[1] = {}
					for num = 1, #menu_blocks do
						if leftop == #menu_blocks[1] + 1 then
							matches[1][num] = cheat.compcur(menu_blocks[num][rightop], optable[opsel],
										formtable[width], value, bcd == 1)
						else
							matches[1][num] = cheat.comp(menu_blocks[num][leftop], menu_blocks[num][rightop],
										   optable[opsel], formtable[width], value, bcd == 1)
						end
						count = count + #matches[1][num]
					end
				else
					lastmatch = matches[#matches]
					matches[#matches + 1] = {}
					for num = 1, #menu_blocks do
						if leftop == #menu_blocks[1] + 1 then
							matches[#matches][num] = cheat.compcurnext(menu_blocks[num][rightop], lastmatch[num],
											optable[opsel], formtable[width], value, bcd == 1)
						else
							matches[#matches][num] = cheat.compnext(menu_blocks[num][leftop], menu_blocks[num][rightop],
											lastmatch[num], optable[opsel], formtable[width], value, bcd == 1)
						end
						count = count + #matches[#matches][num]
					end
				end
				manager:machine():popmessage(count .. " total matches found")
				ret = true
			end
		elseif index == midx.match then
			matchsel = incdec(matchsel, 0, #matches[#matches])
		elseif index == midx.watch then
			watches = {}
			ret = true
		elseif index > midx.match then 
			local match
			if matchsel == 0 then
				local sel = index - midx.match
				for i = 1, #matches[#matches] do
					if sel <= #matches[#matches][i] then
						match = matches[#matches][i][sel]
						break
					else
						sel = sel - #matches[#matches][i]
					end
				end
			else
				match = matches[#matches][matchsel][index - midx.match]
			end
			match.mode = incdec(match.mode, 1, 3)
			if event == "select" then
				local dev = devtable[devcur]
				local cheat = { desc = string.format("Test cheat at addr %08X", match.addr), script = {} }
				local wid = formtable[width]:sub(2, 2):lower()
				local xmlcheat
				local form
				if wid == "h" then
					wid = "u16"
					form = "%08x %04x"
					xmlcheat = "pw"
				elseif wid == "l" then
					wid = "u32"
					form = "%08x %08x"
					xmlcheat = "pd"
				elseif wid == "j" then
					wid = "u64"
					form = "%08x %016x"
					xmlcheat = "pq"
				else
					wid = "u8"
					form = "%08x %02x"
					xmlcheat = "pb"
				end
				xmlcheat = string.format("<mamecheat version=1>\n<cheat desc=\"%s\">\n<script state=\"run\">\n<action>%s.%s@%X=%X</action>\n</script>\n</cheat>\n</mamecheat>", cheat.desc, dev.tag:sub(2), xmlcheat, match.addr, match.newval)

				if dev.space.shortname then
					cheat.ram = { ram = dev.tag }
					cheat.script.on = "ram:write(" .. match.addr .. "," .. match.newval .. ")"
				else
					cheat.space = { cpu = { tag = dev.tag, type = "program" } }
					cheat.script.run = "cpu:write_" .. wid .. "(" .. match.addr .. "," .. match.newval .. ")"
				end
				if match.mode == 1 then
					if not _G.ce then
						manager:machine():popmessage("Cheat engine not available")
					else
						_G.ce.inject(cheat)
					end
				elseif match.mode == 2 then
					local filename = string.format("%s/%s_%08X_cheat", manager:machine():options().entries.cheatpath:value():match("([^;]+)"), emu.romname(), match.addr)
					local json = require("json")
					local file = io.open(filename .. ".json", "w")
					if file then
						file:write(json.stringify({[1] = cheat}, {indent = true}))
						file:close()
						file = io.open(filename .. ".xml", "w")
						file:write(xmlcheat)
						file:close()
						manager:machine():popmessage("Cheat written to " .. filename)
					else
						manager:machine():popmessage("Unable to write file\nCheck cheatpath dir exists")
					end
				else
					local func = "return space:read"
					local env = { space = devtable[devcur].space }
					if not dev.space.shortname then
						func = func .. "_" .. wid
					end
					func = func .. "(" .. match.addr .. ")"
					watches[#watches + 1] = { addr = match.addr, func = load(func, func, "t", env), format = form }
				end
			end
			ret = true
		end
		devsel = devcur
		return ret
	end
	emu.register_menu(menu_callback, menu_populate, "Cheat Finder")
	emu.register_frame_done(function () 
			local tag, screen = next(manager:machine().screens)
			local height = mame_manager:ui():get_line_height()
			for num, watch in ipairs(watches) do
				screen:draw_text("left", num * height, string.format(watch.format, watch.addr, watch.func()))
			end
		end)
end

return exports
