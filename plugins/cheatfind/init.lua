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

	-- return table of share regions
	function cheat.getshares()
		local shares = {}
		for tag, share in pairs(manager:machine():memory().shares) do
			shares[tag] = share
		end
		return shares
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
					temp[j] = string.pack("B", space:read_u8(i, true))
					j = j + 1
				else
					block = block .. table.concat(temp) .. string.pack("B", space:read_u8(i, true))
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

		local cfoper = { 
			lt = function(a, b, val) return (a < b and val == 0) or (val > 0 and (a + val) == b) end,
			gt = function(a, b, val) return (a > b and val == 0) or (val > 0 and (a - val) == b) end,
			eq = function(a, b, val) return a == b end,
			ne = function(a, b, val) return (a ~= b and val == 0) or
				(val > 0 and ((a - val) == b or (a + val) == b)) end,
			ltv = function(a, b, val) return a < val end,
			gtv = function(a, b, val) return a > val end,
			eqv = function(a, b, val) return a == val end,
			nev = function(a, b, val) return a ~= val end }

		function cfoper.bne(a, b, val, addr)
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

		function cfoper.beq(a, b, val, addr)
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
	local matchpg = 0
	local menu_blocks = {}
	local watches = {}
	local menu_func

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
		matchpg = 0
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
		space_table = cheat.getshares()
		for tag, share in pairs(space_table) do
			devtable[#devtable + 1] = { tag = tag, space = share, ram = {{ offset = 0, size = share.size }} }
		end
	end

	emu.register_start(start)

	local function menu_populate()
		local menu = {}

		local function menu_lim(val, min, max, menuitem)
			if min == max then
				menuitem[3] = 0
			elseif val == min then
				menuitem[3] = "r"
			elseif val == max then
				menuitem[3] = "l"
			else
				menuitem[3] = "lr"
			end
		end

		local function incdec(event, val, min, max)
			local ret
			if event == "left" and val ~= min then
				val = val - 1
				ret = true
			elseif event == "right" and val ~= max then
				val = val + 1
				ret = true
			end
			return val, ret
		end

	
		menu[#menu + 1] = function()
			local m = { "CPU or RAM", devtable[devsel].tag, 0 }
			menu_lim(devsel, 1, #devtable, m)
			local function f(event)
				if (event == "left" or event == "right") and #menu_blocks ~= 0 then
					manager:machine():popmessage("Changes to this only take effect when \"Start new search\" is selected")
				end
				devsel = incdec(event, devsel, 1, #devtable)
				return true
			end
			return m, f
		end

		menu[#menu + 1] = function()
			local function f(event)
				local ret = false
				if event == "select" then
					menu_blocks = {}
					matches = {}
					devcur = devsel
					for num, region in ipairs(devtable[devcur].ram) do
						menu_blocks[num] = {}
						menu_blocks[num][1] = cheat.save(devtable[devcur].space, region.offset, region.size)
					end
					manager:machine():popmessage("Data cleared and current state saved")
					watches = {}
					leftop = 2
					rightop = 1
					matchsel = 0
					return true
				end
			end
			return { "Start new search", "", 0 }, f
		end
		if #menu_blocks ~= 0 then
			menu[#menu + 1] = function() return { "---", "", "off" }, nil end
			menu[#menu + 1] = function()
				local function f(event)
					if event == "select" then
						for num, region in ipairs(devtable[devcur].ram) do
							menu_blocks[num][#menu_blocks[num] + 1] = cheat.save(devtable[devcur].space, region.offset, region.size)
						end
						manager:machine():popmessage("Current state saved")
						leftop = (leftop == #menu_blocks[1]) and #menu_blocks[1] + 1 or leftop
						rightop = (rightop == #menu_blocks[1] - 1) and #menu_blocks[1] or rightop
						devsel = devcur
						return true
					end
				end
				return { "Save current -- #" .. #menu_blocks[1] + 1, "", 0 }, f
			end
			menu[#menu + 1] = function()
				local function f(event)
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
						matches[#matches].count = count
						matchpg = 0
						devsel = devcur
						return true
					end
				end
				return { "Compare", "", 0 }, f
			end
			menu[#menu + 1] = function()
				local m = { "Left operand", leftop, "" }
				menu_lim(leftop, 1, #menu_blocks[1] + 1, m)
				if leftop == #menu_blocks[1] + 1 then
					m[2] = "Current"
				end
				return m, function(event) local r leftop, r = incdec(event, leftop, 1, #menu_blocks[1] + 1) return r end
			end
			menu[#menu + 1] = function()
				local m = { "Operator", optable[opsel], "" }
				menu_lim(opsel, 1, #optable, m)
				local function f(event)
					local r
					opsel, r = incdec(event, opsel, 1, #optable)
					if event == "left" or event == "right" or event == "comment" then
						if optable[opsel] == "lt" then
							manager:machine():popmessage("Left less than right, value is difference")
						elseif optable[opsel] == "gt" then
							manager:machine():popmessage("Left greater than right, value is difference")
						elseif optable[opsel] == "eq" then
							manager:machine():popmessage("Left equal to right")
						elseif optable[opsel] == "ne" then
							manager:machine():popmessage("Left not equal to right, value is difference")
						elseif optable[opsel] == "beq" then
							manager:machine():popmessage("Left equal to right with bitmask")
						elseif optable[opsel] == "bne" then
							manager:machine():popmessage("Left not equal to right with bitmask")
						elseif optable[opsel] == "ltv" then
							manager:machine():popmessage("Left less than value")
						elseif optable[opsel] == "gtv" then
							manager:machine():popmessage("Left greater than value")
						elseif optable[opsel] == "eqv" then
							manager:machine():popmessage("Left equal to value")
						elseif optable[opsel] == "nev" then
							manager:machine():popmessage("Left not equal to value")
						end
					end
					return r
				end
				return m, f
			end
			menu[#menu + 1] = function()
				if optable[opsel]:sub(3, 3) == "v" then
					return nil
				end
				local m = { "Right operand", rightop, "" }
				menu_lim(rightop, 1, #menu_blocks[1], m)
				return m, function(event) local r rightop, r = incdec(event, rightop, 1, #menu_blocks[1]) return r end
			end
			menu[#menu + 1] = function() 
				if optable[opsel] == "bne" or optable[opsel] == "beq" or optable[opsel] == "eq" then
					return nil
				end
				local m = { "Value", value, "" }
				local max = 100 -- max value?
				menu_lim(value, 0, max, m)
				if value == 0 and optable[opsel]:sub(3, 3) ~= "v" then
					m[2] = "Any"
				end
				return m, function(event) local r value, r = incdec(event, value, 0, max) return r end
			end
			menu[#menu + 1] = function() return { "---", "", "off" }, nil end
			menu[#menu + 1] = function() 
				local m = { "Data Format", formname[width], 0 }
				menu_lim(width, 1, #formtable, m)
				return m, function(event) local r width, r = incdec(event, width, 1, #formtable) return r end
			end
			menu[#menu + 1] = function()
				if optable[opsel] == "bne" or optable[opsel] == "beq" then
					return nil
				end
				local m = { "BCD", "Off", 0 }
				menu_lim(bcd, 0, 1, m)
				if bcd == 1 then
					m[2] = "On"
				end
				return m, function(event) local r bcd, r = incdec(event, bcd, 0, 1) return r end
			end
			if #matches ~= 0 then
				menu[#menu + 1] = function()
					local function f(event)
						if event == "select" then
							matches[#matches] = nil
							matchpg = 0
							return true
						end
					end
					return { "Undo last search -- #" .. #matches, "", 0 }, f
				end
				menu[#menu + 1] = function() return { "---", "", "off" }, nil end
				menu[#menu + 1] = function()
					local m = { "Match block", matchsel, "" }
					menu_lim(matchsel, 0, #matches[#matches], m)
					if matchsel == 0 then
						m[2] = "All"
					end
					local function f(event)
						local r
						matchsel, r = incdec(event, matchsel, 0, #matches[#matches])
						if r then
							matchpg = 0
						end
						return r
					end
					return m, f
				end
				local function mpairs(sel, list, start)
					if #list == 0 then
						return function() end, nil, nil
					end
					if sel ~= 0 then
						list = {list[sel]}
					end
					local function mpairs_it(list, i)
						local match
						i = i + 1
						local sel = i + start
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
				local bitwidth = formtable[width]:sub(2, 2):lower()
				if bitwidth == "h" then
					bitwidth = " %04x"
				elseif bitwidth == "l" then
					bitwidth = " %08x"
				elseif bitwidth == "j" then
					bitwidth = " %016x"
				else
					bitwidth = " %02x"
				end
				for num2, match in mpairs(matchsel, matches[#matches], matchpg * 100) do
					if num2 > 100 then
						break
					end
					menu[#menu + 1] = function()
						local m = { string.format("%08x" .. bitwidth .. bitwidth, match.addr, match.oldval, 
					                                  match.newval), "", 0 }
						if not match.mode then
							match.mode = 1
						end
						if match.mode == 1 then
							m[2] = "Test"
						elseif match.mode == 2 then
							m[2] = "Write"
						else
							m[2] = "Watch"
						end
						menu_lim(match.mode, 1, 3, m)
						local function f(event)
							local r
							match.mode, r = incdec(event, match.mode, 1, 3)
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
									cheat.script.run = "ram:write(" .. match.addr .. "," .. match.newval .. ")"
								else
									cheat.space = { cpu = { tag = dev.tag, type = "program" } }
									cheat.script.run = "cpu:write_" .. wid .. "(" .. match.addr .. "," .. match.newval .. ", true)"
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
									r = true
								end
							end
							return r
						end
						return m, f
					end
				end
				if matches[#matches].count > 100 then
					menu[#menu + 1] = function()
						local m = { "Page", matchpg, 0 }
						local max
						if matchsel == 0 then
							max = math.ceil(matches[#matches].count / 100)
						else
							max = #matches[#matches][matchsel]
						end
						menu_lim(matchpg, 0, max, m)
						local function f(event)
							matchpg, r = incdec(event, matchpg, 0, max)
							return r
						end
						return m, f
					end
				end
			end
			if #watches ~= 0 then
				menu[#menu + 1] = function()
					return { "Clear Watches", "", 0 }, function(event) if event == "select" then watches = {} return true end end
				end
			end
		end
		local menu_list = {}
		menu_func = {}
		for num, func in ipairs(menu) do
			local item, f = func()
			if item then
				menu_list[#menu_list + 1] = item
				menu_func[#menu_list] = f
			end
		end
		return menu_list
	end

	local function menu_callback(index, event)
		return menu_func[index](event)
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
