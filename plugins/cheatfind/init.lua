-- license:BSD-3-Clause
-- copyright-holders:Carl
-- This includes a library of functions to be used at the Lua console as cf.getspaces() etc...
local exports = {}
exports.name = "cheatfind"
exports.version = "0.0.1"
exports.description = "Cheat finder helper library"
exports.license = "BSD-3-Clause"
exports.author = { name = "Carl" }

local cheatfind = exports

function cheatfind.startplugin()
	local cheat = {}

	-- return table of devices and spaces
	function cheat.getspaces()
		local spaces = {}
		for tag, device in pairs(manager.machine.devices) do
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
		for tag, device in pairs(manager.machine.devices) do
			if device.shortname == "ram" then
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
		for tag, share in pairs(manager.machine.memory.shares) do
			shares[tag] = share
		end
		return shares
	end

	-- save data block
	function cheat.save(space, start, size)
		local data = { block = "", start = start, size = size, space = space, shift = 0 }
		if getmetatable(space).__name:match("addr_space") then
			data.shift = space.shift
		end
		if getmetatable(space).__name:match("device_t") then
			if space.shortname == "ram" then
				data.block = emu.item(space.items["0/m_pointer"]):read_block(start, size)
				if not data.block then
					return nil
				end
			end
		else
			local block = ""
			local temp = {}
			local j = 1
			if data.shift >= 0 then -- region or byte wide space
				for i = start, start + (size << data.shift), 1 << data.shift do
					if j < 65536 then
						temp[j] = string.pack("B", space:read_u8(i))
						j = j + 1
					else
						block = block .. table.concat(temp) .. string.pack("B", space:read_u8(i))
						temp = {}
						j = 1
					end
				end
			elseif data.shift < 0 then
				local s = -data.shift
				local read = (s == 1) and space.read_u16 or (s == 2) and space.read_u32 or (s == 3) and space.read_u64 or space.read_u8
				local pack = (s == 1) and "<I2" or (s == 2) and "<I4" or (s == 3) and "<I8" or "B"
				for i = start, start + (size >> s) do
					if j < 65536 then
						temp[j] = string.pack(pack, read(space, i))
						j = j + 1
					else
						block = block .. table.concat(temp) .. string.pack(pack, read(space, i))
						temp = {}
						j = 1
					end
				end
			end
			block = block .. table.concat(temp)
			data.block = block
		end
		return data
	end

	-- compare two data blocks, format is as lua string.unpack, bne and beq val is table of masks, step is address increment value
	function cheat.comp(newdata, olddata, oper, format, val, bcd, step)
		local ret = {}
		local ref = {} -- this is a helper for comparing two match lists
		local bitmask = nil
		if not step or step <= 0 then
			step = 1
		end
		if (olddata.shift < 0) and (step < (1 << -olddata.shift)) then
			step = 1 << -olddata.shift;
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

		for i = 1, olddata.size, step do
			local oldstat, old = pcall(string.unpack, format, olddata.block, i)
			local newstat, new = pcall(string.unpack, format, newdata.block, i)
			if oldstat and newstat then
				local oldc, newc = old, new
				local comp = false
				local addr = i - 1
				if olddata.shift ~= 0 then
					local s = olddata.shift
					addr = (s < 0) and addr >> -s or (s > 0) and addr << s
				end
				addr = addr + olddata.start
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
						ref[addr] = #ret
					end
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
	function cheat.compnext(newdata, olddata, oldmatch, oper, format, val, bcd, step)
		local matches, refs = cheat.comp(newdata, olddata, oper, format, check_val(oper, val, oldmatch), bcd, step)
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
	function cheat.compcur(olddata, oper, format, val, bcd, step)
		local newdata = cheat.save(olddata.space, olddata.start, olddata.size, olddata.space)
		return cheat.comp(newdata, olddata, oper, format, val, bcd, step)
	end

	-- compare a data block to the current state and filter
	function cheat.compcurnext(olddata, oldmatch, oper, format, val, bcd, step)
		local newdata = cheat.save(olddata.space, olddata.start, olddata.size, olddata.space)
		return cheat.compnext(newdata, olddata, oldmatch, oper, format, val, bcd, step)
	end

	_G.emu.plugin.cheatfind = cheat
	local devtable = {}
	local devsel = 1
	local devcur = 1
	local formname = { "u8", "big u16", "big u32", "big u64", "little u16", "little u32",
		"little u64", "s8", "big s16", "big s32", "big s64", "little s16", "little s32", "little s64", }
	local formtable = { " I1", ">I2", ">I4", ">I8", "<I2", "<I4", "<I8", " i1", ">i2", ">i4", ">i8", "<i2", "<i4", "<i8", }
				-- " <f", " >f", " <d", " >d" }
	local width = 1
	local bcd = 0
	local align = 0
	local optable = { "lt", "gt", "eq", "ne", "beq", "bne", "ltv", "gtv", "eqv", "nev" }
	local opsel = 1
	local value = 0
	local leftop = 1
	local rightop = 1
	local value_text = ""
	local pausesel = 1
	local pokevalsel = 1
	local matches = {}
	local matchsel = 0
	local matchpg = 0
	local menu_blocks = {}
	local watches = {}
	local menu_func
	local cheat_save
	local name = 1
	local name_player = 1
	local name_type = 1
	local name_other = ""

	local function start()
		devtable = {}
		devsel = 1
		devcur = 1
		width = 1
		bcd = 0
		opsel = 1
		value = 0
		leftop = 1
		rightop = 1
		matches = {}
		matchsel = 0
		matchpg = 0
		menu_blocks = {}
		watches = {}

		local space_table = cheat.getspaces()
		for tag, list in pairs(space_table) do
			for name, space in pairs(list) do
				local ram = {}
				for num, entry in pairs(space.map.entries) do
					if entry.write.handlertype == "ram" then
						ram[#ram + 1] = {
							offset = entry.address_start & space.address_mask,
							size = (entry.address_end & space.address_mask) - (entry.address_start & space.address_mask) }
						if space.shift > 0 then
							ram[#ram].size = ram[#ram].size >> space.shift
						elseif space.shift < 0 then
							ram[#ram].size = ram[#ram].size << -space.shift
						end
					end
				end
				if next(ram) then
					if tag == ":maincpu" and name == "program" then
						table.insert(devtable, 1, { name = tag .. ", " .. name, tag = tag, sname = name, space = space, ram = ram })
					else
						devtable[#devtable + 1] = { name = tag .. ", " .. name, tag = tag, sname = name, space = space, ram = ram }
					end
				end
			end
		end
		space_table = cheat.getram()
		for tag, ram in pairs(space_table) do
			devtable[#devtable + 1] = { tag = tag, name = "ram", space = ram.dev, ram = {{ offset = 0, size = ram.size }} }
		end
		space_table = cheat.getshares()
		for tag, share in pairs(space_table) do
			devtable[#devtable + 1] = { tag = tag, name = tag, space = share, ram = {{ offset = 0, size = share.size }} }
		end
	end

	emu.register_start(start)

	local menu_is_showing = false
	local tabbed_out = false

	local function menu_populate()
		if pausesel == 1 then
			emu.pause()
			menu_is_showing = true
		end
		local menu = {}

		local function menu_prepare()
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

		local function menu_lim(val, min, max, menuitem)
			if min == max then
				menuitem[3] = "on"
			elseif val == min then
				menuitem[3] = "r"
			elseif val == max then
				menuitem[3] = "l"
			else
				menuitem[3] = "lr"
			end
		end

		local function incdec(event, val, min, max)
			local ret = false
			if event == "left" and val ~= min then
				val = val - 1
				ret = true
			elseif event == "right" and val ~= max then
				val = val + 1
				ret = true
			end
			return val, ret
		end

		if cheat_save then
			local cplayer = { "All", "P1", "P2", "P3", "P4" }
			local ctype = { "Infinite Credits", "Infinite Time", "Infinite Lives", "Infinite Energy", "Infinite Ammo", "Infinite Bombs", "Invincibility", _("Other: ") }
			menu[#menu + 1] = function() return { _("Save Cheat"), "", "off" }, nil end
			menu[#menu + 1] = function() return { "---", "", "off" }, nil end
			menu[#menu + 1] = function()
				local c = { _("Default"), _("Custom") }
				local m = { _("Cheat Name"), c[name], "on" }
				menu_lim(name, 1, #c, m)
				local function f(event)
					local r
					name, r = incdec(event, name, 1, #c)
					if (event == "select" or event == "comment") and name == 1 then
						manager.machine:popmessage(string.format(_("Default name is %s"), cheat_save.name))
					end
					return r
				end
				return m, f
			end
			if name == 2 then
				menu[#menu + 1] = function()
					local m = { _("Player"), cplayer[name_player], "on" }
					menu_lim(name_player, 1, #cplayer, m)
					return m, function(event) local r name_player, r = incdec(event, name_player, 1, #cplayer) return r end
				end
				menu[#menu + 1] = function()
					local m = { _("Type"), ctype[name_type] .. (name_type == #ctype and (#name_other ~= 0 and name_other or _("(empty)")) or ""), "on" }
					menu_lim(name_type, 1, #ctype, m)
					local function f(event)
						local r
						name_type, r = incdec(event, name_type, 1, #ctype)
						if name_type == #ctype then
							local char = tonumber(event)
							if char then
								if #name_other > 0 and (char == 8 or char == 0x7f) then
									name_other = name_other:sub(1, utf8.offset(name_other, -1) - 1)
									r = true
								elseif char > 0x1f and (char & ~0x7f) ~= 0x80 and (char & ~0xf) ~= 0xfdd0 and (char & ~0xfffe) ~= 0xfffe then
									name_other = name_other .. utf8.char(char)
									r = true
								end
							elseif event == "select" or event == "comment" or event == "right" then
								manager.machine:popmessage(_("You can enter any type name"))
							end
						end
						return r
					end
					return m, f
				end
			end
			menu[#menu + 1] = function()
				local m = { _("Save"), "", "on" }
				local function f(event)
					if event == "select" then
						local desc
						local written = false
						if name == 2 then
							desc = name_type ~= #ctype and ctype[name_type] or name_other
							if #desc == 0 then
								manager.machine:popmessage(_("Type name is empty"))
								return
							end
							if cplayer[name_player] ~= "All" then
								desc = cplayer[name_player] .. " " .. desc
							end
						else
							desc = cheat_save.name
						end
						local filename = cheat_save.filename .. "_" .. desc
						local file = io.open(filename .. ".json", "w")
						if file then
							file:write(string.format(cheat_save.json, desc))
							file:close()
							-- xml or simple are program space only
							if not getmetatable(devtable[devcur].space).__name:match("device_t") and devtable[devcur].sname == "program" then
								file = io.open(filename .. ".xml", "w")
								file:write(string.format(cheat_save.xml, desc))
								file:close()
								file = io.open(cheat_save.path .. "/cheat.simple", "a")
								file:write(string.format(cheat_save.simple, desc))
								-- old cheat .dat format, write support only (for cheat forum posting of new cheats if posted in simple format)
								file:write(string.format(cheat_save.dat, desc))
								file:close()
								manager.machine:popmessage(string.format(_("Cheat written to %s and added to cheat.simple"), filename))
							end
							written = true
						elseif not getmetatable(devtable[devcur].space).__name:match("device_t") and devtable[devcur].sname == "program" then
							file = io.open(cheat_save.path .. "/cheat.simple", "a")
							if file then
								file:write(string.format(cheat_save.simple, desc))
								-- old cheat .dat format, write support only (for cheat forum posting of new cheats if posted in simple format)
								file:write(string.format(cheat_save.dat, desc))
								file:close()
								manager.machine:popmessage(_("Cheat added to cheat.simple"))
								written = true
							end
						end
						if not written then
							manager.machine:popmessage(_("Unable to write file\nEnsure that cheatpath folder exists"))
						end
						cheat_save = nil
						return true
					end
					return false
				end
				return m, f
			end
			menu[#menu + 1] = function() return { _("Cancel"), "", "on" }, function(event) if event == "select" then cheat_save = nil return true end end end
			return menu_prepare()
		end

		menu[#menu + 1] = function()
			local m = { _("CPU or RAM"), devtable[devsel].name, "on" }
			menu_lim(devsel, 1, #devtable, m)
			local function f(event)
				if (event == "left" or event == "right") and #menu_blocks ~= 0 then
					manager.machine:popmessage(_("Changes to this only take effect when \"Start new search\" is selected"))
				end
				local r
				devsel, r = incdec(event, devsel, 1, #devtable)
				return r
			end
			return m, f
		end

		menu[#menu + 1] = function()
			local pausetable = { _("Automatic"), _("Manual") }
			local m = { _("Pause Mode"), pausetable[pausesel], "on" }
			menu_lim(pausesel, 1, pausetable, m)
			local function f(event)
				if (event == "left" or event == "right") then
					if pausesel == 1 then
						pausesel = 2
						menu_is_showing = false
						manager.machine:popmessage(_("Manually toggle pause when needed"))
					else
						pausesel = 1
						manager.machine:popmessage(_("Automatically toggle pause with on-screen menus"))
						emu.pause()
					end
					return true
				end
				return false
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
					manager.machine:popmessage(_("All slots cleared and current state saved to Slot 1"))
					watches = {}
					opsel = 1
					value = 0
					leftop = 1
					rightop = 1
					value_text = ""
					matchsel = 0
					return true
				end
			end
			local opsel = 1
			return { _("Start new search"), "", "on" }, f
		end

		if #menu_blocks ~= 0 then
			menu[#menu + 1] = function() return { "---", "", "off" }, nil end
			menu[#menu + 1] = function()
				local function f(event)
					if event == "select" then
						for num, region in ipairs(devtable[devcur].ram) do
							menu_blocks[num][#menu_blocks[num] + 1] = cheat.save(devtable[devcur].space, region.offset, region.size)
						end
						manager.machine:popmessage(string.format(_("Memory state saved to Slot %d"), #menu_blocks[1]))
						if (leftop == #menu_blocks[1] - 1 and rightop == #menu_blocks[1] - 2 ) then
							leftop = #menu_blocks[1]
							rightop = #menu_blocks[1]-1
						elseif (leftop == #menu_blocks[1] - 2 and rightop == #menu_blocks[1] - 1 ) then
							leftop = #menu_blocks[1]-1
							rightop = #menu_blocks[1]
						elseif (leftop == #menu_blocks[1] - 1 ) then
							leftop = #menu_blocks[1]
						elseif (rightop == #menu_blocks[1] - 1) then
							rightop = #menu_blocks[1]
						end
						devsel = devcur
						return true
					end
					return false
				end
				return { string.format(_("Save current memory state to Slot %d"), #menu_blocks[1] + 1), "", "on" }, f
			end
			menu[#menu + 1] = function() return { "---", "", "off" }, nil end
			menu[#menu + 1] = function()
				local function f(event)
					if event == "select" then
						local count = 0
						local step = align == 1 and formtable[width]:sub(3, 3) or "1"
						if step == "f" then
							step = 4
						elseif step == "d" then
							step = 8
						else
							step = tonumber(step)
						end
						if #matches == 0 then
							matches[1] = {}
							for num = 1, #menu_blocks do
								matches[1][num] = cheat.comp(menu_blocks[num][leftop], menu_blocks[num][rightop],
								optable[opsel], formtable[width], value, bcd == 1, step)
								count = count + #matches[1][num]
							end
						else
							lastmatch = matches[#matches]
							matches[#matches + 1] = {}
							for num = 1, #menu_blocks do
								matches[#matches][num] = cheat.compnext(menu_blocks[num][leftop], menu_blocks[num][rightop],
								lastmatch[num], optable[opsel], formtable[width], value, bcd == 1, step)
								count = count + #matches[#matches][num]
							end
						end
						manager.machine:popmessage(string.format(_("%d total matches found"), count))
						matches[#matches].count = count
						matchpg = 0
						devsel = devcur
						return true
					end
					return false
				end

				local slot_slot_comp = _("Perform Compare : Slot %d %s Slot %d")
				local slot_slot_val_comp = _("Perform Compare  :  Slot %d %s Slot %d %s %d")
				local slot_slot_bit_comp = _("Perform Compare  :  Slot %d BITWISE%s Slot %d")
				local slot_val_comp = _("Perform Compare  :  Slot %d %s %d")
				local expression_text
				if optable[opsel] == "lt" then
					if (value == 0 ) then
						expression_text = string.format(slot_slot_comp, leftop, "<", rightop)
					else
						expression_text = string.format(slot_slot_val_comp, leftop, "==", rightop, "-", value)
					end
				elseif optable[opsel] == "gt" then
					if (value == 0 ) then
						expression_text = string.format(slot_slot_comp, leftop, ">", rightop)
					else
						expression_text = string.format(slot_slot_val_comp, leftop, "==", rightop, "+", value)
					end
				elseif optable[opsel] == "eq" then
					expression_text = string.format(slot_slot_comp, leftop, "==", rightop)
				elseif optable[opsel] == "ne" then
					if (value == 0 ) then
						expression_text = string.format(slot_slot_comp, leftop, "!=", rightop)
					else
						expression_text = string.format(slot_slot_val_comp, leftop, "==", rightop, "+/-", value)
					end
				elseif optable[opsel] == "beq" then
					expression_text = string.format(slot_slot_bit_comp, leftop, "==", rightop)
				elseif optable[opsel] == "bne" then
					expression_text = string.format(slot_slot_bit_comp, leftop, "!=", rightop)
				elseif optable[opsel] == "ltv" then
					expression_text = string.format(slot_val_comp, leftop, "<", value)
				elseif optable[opsel] == "gtv" then
					expression_text = string.format(slot_val_comp, leftop, ">", value)
				elseif optable[opsel] == "eqv" then
					expression_text = string.format(slot_val_comp, leftop, "==", value)
				elseif optable[opsel] == "nev" then
					expression_text = string.format(slot_val_comp, leftop, "!=", value)
				end
				return { expression_text, "", "on" }, f
			end
			menu[#menu + 1] = function() return { "---", "", "off" }, nil end
			menu[#menu + 1] = function()
				local m = { string.format("%d", leftop), "", "on" }
				menu_lim(leftop, 1, #menu_blocks[1], m)
				m[1] = string.format(_("Slot %d"), leftop)
				return m, function(event) local r leftop, r = incdec(event, leftop, 1, #menu_blocks[1]) return r end
			end
			menu[#menu + 1] = function()
				local m = { _(optable[opsel]), "", "on" }
				menu_lim(opsel, 1, #optable, m)
				local function f(event)
					local r
					opsel, r = incdec(event, opsel, 1, #optable)
					if event == "left" or event == "right" or event == "comment" then
						if optable[opsel] == "lt" then
							manager.machine:popmessage(_("Left less than right"))
						elseif optable[opsel] == "gt" then
							manager.machine:popmessage(_("Left greater than right"))
						elseif optable[opsel] == "eq" then
							manager.machine:popmessage(_("Left equal to right"))
						elseif optable[opsel] == "ne" then
							manager.machine:popmessage(_("Left not equal to right"))
						elseif optable[opsel] == "beq" then
							manager.machine:popmessage(_("Left equal to right with bitmask"))
						elseif optable[opsel] == "bne" then
							manager.machine:popmessage(_("Left not equal to right with bitmask"))
						elseif optable[opsel] == "ltv" then
							manager.machine:popmessage(_("Left less than value"))
						elseif optable[opsel] == "gtv" then
							manager.machine:popmessage(_("Left greater than value"))
						elseif optable[opsel] == "eqv" then
							manager.machine:popmessage(_("Left equal to value"))
						elseif optable[opsel] == "nev" then
							manager.machine:popmessage(_("Left not equal to value"))
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
				local m = { string.format("%d", rightop), "", "on" }
				menu_lim(rightop, 1, #menu_blocks[1], m)
				m[1] = string.format(_("Slot %d"), rightop)
				return m, function(event) local r rightop, r = incdec(event, rightop, 1, #menu_blocks[1]) return r end
			end
			menu[#menu + 1] = function()
				if optable[opsel] == "bne" or optable[opsel] == "beq" or optable[opsel] == "eq" then
					return nil
				end
				local m
				if optable[opsel] == "ltv" or optable[opsel] == "gtv" or optable[opsel] == "eqv" or optable[opsel] == "nev" then
					m = { _("Value"), value, "" }
				else
					m = { _("Difference"), value, "" }
				end
				local max = 100 -- max value?
				menu_lim(value, 0, max, m)
				if value == 0 and optable[opsel]:sub(3, 3) ~= "v" then
					m[2] = _("Any")
				end
				return m, function(event) local r value, r = incdec(event, value, 0, max) return r end
			end
			menu[#menu + 1] = function() return { "---", "", "off" }, nil end
			menu[#menu + 1] = function()
				local m = { _("Data Format"), formname[width], "on" }
				menu_lim(width, 1, #formtable, m)
				return m, function(event) local r width, r = incdec(event, width, 1, #formtable) return r end
			end

			menu[#menu + 1] = function()
				local pokevaltable = { _("Slot 1 Value"), _("Last Slot Value"), "0x00", "0x01", "0x02", "0x03", "0x04", "0x05", "0x06",
					"0x07", "0x08", "0x09", "0x63 (99)", "0x99", "0xFF (255)" , "0x3E7 (999)", "0x999", "0x270F (9999)",
					"0x9999", "0xFFFF (65535)" }
				local m = { _("Test/Write Poke Value"), pokevaltable[pokevalsel], "on" }
				menu_lim(pokevalsel, 1, #pokevaltable, m)
				local function f(event)
					local r
					pokevalsel, r = incdec(event, pokevalsel, 1, #pokevaltable)
					if event == "left" or event == "right" or event == "comment" then
						if pokevalsel == 1 then
							manager.machine:popmessage(_("Use this if you want to poke the Slot 1 value (eg. You started with something but lost it)"))
						elseif pokevalsel == 2 then
							manager.machine:popmessage(_("Use this if you want to poke the Last Slot value (eg. You started without an item but finally got it)"))
						else
							manager.machine:popmessage(string.format(_("Use this if you want to poke %s"), pokevaltable[pokevalsel]))
						end
					end
					return r
				end
				return m, f
			end

			menu[#menu + 1] = function()
				if optable[opsel] == "bne" or optable[opsel] == "beq" then
					return nil
				end
				local m = { "BCD", _("Off"), "on" }
				menu_lim(bcd, 0, 1, m)
				if bcd == 1 then
					m[2] = _("On")
				end
				return m, function(event) local r bcd, r = incdec(event, bcd, 0, 1) return r end
			end
			menu[#menu + 1] = function()
				if formtable[width]:sub(3, 3) == "1" then
					return nil
				end
				local m = { _("Aligned only"), _("Off"), "on" }
				menu_lim(align, 0, 1, m)
				if align == 1 then
					m[2] = _("On")
				end
				return m, function(event) local r align, r = incdec(event, align, 0, 1) return r end
			end
			if #matches ~= 0 then
				menu[#menu + 1] = function()
					local function f(event)
						if event == "select" then
							matches[#matches] = nil
							matchpg = 0
							return true
						end
						return false
					end
					return { _("Undo last search -- #") .. #matches, "", "on" }, f
				end
				menu[#menu + 1] = function() return { "---", "", "off" }, nil end
				menu[#menu + 1] = function()
					local m = { _("Match block"), matchsel, "" }
					menu_lim(matchsel, 0, #matches[#matches], m)
					if matchsel == 0 then
						m[2] = _("All")
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
				local bitwidth = formtable[width]:sub(3, 3)
				if bitwidth == "2" then
					bitwidth = " %04x"
				elseif bitwidth == "4" then
					bitwidth = " %08x"
				elseif bitwidth == "8" then
					bitwidth = " %016x"
				elseif bitwidth == "f" or bitwidth == "d" then
					bitwidth = " %010f"
				else
					bitwidth = " %02x"
				end

				local function match_exec(match)
					local dev = devtable[devcur]
					local wid = formtable[width]:sub(3, 3)
					local widchar
					local pokevalue
					local form
					if pokevalsel == 1 then
						pokevalue = match.oldval
					elseif pokevalsel == 2 then
						pokevalue = match.newval
					elseif pokevalsel == 3 then
						pokevalue = 0
					elseif pokevalsel == 4 then
						pokevalue = 1
					elseif pokevalsel == 5 then
						pokevalue = 2
					elseif pokevalsel == 6 then
						pokevalue = 3
					elseif pokevalsel == 7 then
						pokevalue = 4
					elseif pokevalsel == 8 then
						pokevalue = 5
					elseif pokevalsel == 9 then
						pokevalue = 6
					elseif pokevalsel == 10 then
						pokevalue = 7
					elseif pokevalsel == 11 then
						pokevalue = 8
					elseif pokevalsel == 12 then
						pokevalue = 9
					elseif pokevalsel == 13 then
						pokevalue = 99
					elseif pokevalsel == 14 then
						pokevalue = 153
					elseif pokevalsel == 15 then
						pokevalue = 255
					elseif pokevalsel == 16 and wid == "1" then
						pokevalue = 99
					elseif pokevalsel == 17 and wid == "1" then
						pokevalue = 153
					elseif pokevalsel == 18 and wid == "1" then
						pokevalue = 99
					elseif pokevalsel == 19 and wid == "1" then
						pokevalue = 153
					elseif pokevalsel == 20 and wid == "1" then
						pokevalue = 255
					elseif pokevalsel == 16 then
						pokevalue = 999
					elseif pokevalsel == 17 then
						pokevalue = 2457
					elseif pokevalsel == 18 then
						pokevalue = 9999
					elseif pokevalsel == 19 then
						pokevalue = 39321
					elseif pokevalsel == 20 then
						pokevalue = 65535
					end

					local cheat = { desc = string.format(_("Test Cheat %08X_%02X"), match.addr, pokevalue), script = {} }

					if wid == "2" then
						wid = "u16"
						form = "%08x %04x"
						widchar = "w"
					elseif wid == "4" then
						wid = "u32"
						form = "%08x %08x"
						widchar = "d"
					elseif wid == "8" then
						wid = "u64"
						form = "%08x %016x"
						widchar = "q"
					elseif wid == "f" then
						wid = "u32"
						form = "%08x %f"
						widchar = "d"
					elseif wid == "d" then
						wid = "u64"
						form = "%08x %f"
						widchar = "q"
					else
						wid = "u8"
						form = "%08x %02x"
						widchar = "b"
					end

					if getmetatable(dev.space).__name:match("device_t") then
						cheat.ram = { ram = dev.tag }
						cheat.script.run = "ram:write(" .. match.addr .. "," .. pokevalue .. ")"
					elseif getmetatable(dev.space).__name:match("memory_share") then
						cheat.share = { share = dev.tag }
						cheat.script.run = "share:write_" .. wid .. "(" .. match.addr .. "," .. pokevalue .. ")"
					else
						cheat.space = { cpu = { tag = dev.tag, type = dev.sname } }
						cheat.script.run = "cpu:write_" .. wid .. "(" .. match.addr .. "," .. pokevalue .. ")"
					end
					if match.mode == 1 then
						if not emu.plugin.cheat then
							manager.machine:popmessage(_("Cheat engine not available"))
						else
							emu.plugin.cheat.inject(cheat)
						end
					elseif match.mode == 2 then
						cheat_save = {}
						menu = 1
						menu_player = 1
						menu_type = 1
						local setname = emu.romname()
						if emu.softname() ~= "" then
							if emu.softname():find(":") then
								setname = emu.softname():gsub(":", "/")
							else
								for name, image in pairs(manager.machine.images) do
									if image.exists and image.software_list_name ~= "" then
										setname = image.software_list_name .. "/" .. emu.softname()
									end
								end
							end
						end
						cheat_save.path = emu.subst_env(manager.machine.options.entries.cheatpath:value()):match("([^;]+)")
						cheat_save.filename = string.format("%s/%s", cheat_save.path, setname)
						cheat_save.name = cheat.desc
						local json = require("json")
						cheat.desc = "%s"
						cheat_save.json = json.stringify({[1] = cheat}, {indent = true})
						cheat_save.xml = string.format("<mamecheat version=\"1\">\n  <cheat desc=\"%%s\">\n    <script state=\"run\">\n      <action>%s.pp%s@%X=%X</action>\n    </script>\n  </cheat>\n</mamecheat>", dev.tag:sub(2), widchar, match.addr, match.newval)
						cheat_save.simple = string.format("%s,%s,%X,%s,%X,%%s\n", setname, dev.tag, match.addr, widchar, pokevalue)
						cheat_save.dat = string.format(":%s:40000000:%X:%08X:FFFFFFFF:%%s\n", setname, match.addr, pokevalue)
						manager.machine:popmessage(string.format(_("Default name is %s"), cheat_save.name))
						return true
					else
						local func = "return space:read"
						local env = {}
						if not getmetatable(dev.space).__name:match("device_t") then
							env.space = devtable[devcur].space;
							func = func .. "_" .. wid
						else
							env.space = emu.item(dev.space.items["0/m_pointer"])
						end
						func = func .. "(" .. match.addr .. ")"
						watches[#watches + 1] = { addr = match.addr, func = load(func, func, "t", env), format = form }
						return true
					end
					return false
				end

				for num2, match in mpairs(matchsel, matches[#matches], matchpg * 100) do
					if num2 > 100 then
						break
					end
					menu[#menu + 1] = function()
						if not match.mode then
							match.mode = 1
						end
						local modes = { _("Test"), _("Write"), _("Watch") }
						local m = { string.format("%08x" .. bitwidth .. bitwidth, match.addr, match.oldval,
													  match.newval), modes[match.mode], "on" }
						menu_lim(match.mode, 1, #modes, m)
						local function f(event)
							local r
							match.mode, r = incdec(event, match.mode, 1, 3)
							if event == "select" then
								r = match_exec(match)
							end
							return r
						end
						return m, f
					end
				end
				if matches[#matches].count > 100 then
					menu[#menu + 1] = function()
						local m = { _("Page"), matchpg, "on" }
						local max
						if matchsel == 0 then
							max = math.ceil(matches[#matches].count / 100) - 1
						else
							max = #matches[#matches][matchsel]
						end
						menu_lim(matchpg, 0, max, m)
						local function f(event)
							local r
							matchpg, r = incdec(event, matchpg, 0, max)
							return r
						end
						return m, f
					end
				end
			end
			if #watches ~= 0 then
				menu[#menu + 1] = function()
					return { _("Clear Watches"), "", "on" }, function(event) if event == "select" then watches = {} return true end end
				end
			end
		end
		return menu_prepare()
	end

	local function menu_callback(index, event)
		if event == "cancel" and pausesel == 1 then
			emu.unpause()
			menu_is_showing = false
			return false -- return false so menu will be popped off the stack
		end
		if index == 0 then return false end
		return menu_func[index](event)
	end
	emu.register_menu(menu_callback, menu_populate, _("Cheat Finder"))
	emu.register_frame_done(function ()
			local screen = manager.machine.render.ui_container
			local height = mame_manager.ui.line_height
			for num, watch in ipairs(watches) do
				screen:draw_text("left", num * height, string.format(watch.format, watch.addr, watch.func()))
			end
			if tabbed_out and manager.ui.menu_active then
				emu.pause()
				menu_is_showing = true
				tabbed_out = false
			end
		end)
	emu.register_periodic(function ()
		if menu_is_showing and not manager.ui.menu_active then
			emu.unpause()
			menu_is_showing = false
			tabbed_out = true
		end
	end)
end

return exports
