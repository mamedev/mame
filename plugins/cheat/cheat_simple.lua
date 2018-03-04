-- converter for simple cheats
-- simple cheats are single/linked address every frame ram, rom or gg,ar cheats in one file called cheat.simple
--
-- ram/rom cheat format:  <set name>,<cputag|regiontag>,<hex offset>,<b|w|d|q - size>,<hex value>,<desc>
-- only program address space is supported, comments are prepended with ;
-- size is b - u8, w - u16, d - u32, q - u64
--
-- gg,ar cheat format: <set name>,<gg|ar - type>,<code>,<desc> like "nes/smb,gg,SXIOPO,Infinite Lives"
-- gg for game genie -- nes, snes, megadriv, gamegear, gameboy
-- ar for action replay -- nes, snes, megadriv, gamegear, sms
--
-- use "^" as description to link to previous cheat
-- set name is <softlist>/<entry> like "nes/smb" for softlist items
-- Don't use commas in the description

local simple = {}

simple.romset = "????"

function simple.filename(name)
	simple.romset = name
	return "cheat.simple"
end

local codefuncs = {}
local currcheat

local function prepare_rom_cheat(desc, region, addr, val, size, banksize, comp)
	local cheat
	if desc:sub(1,1) ~= "^" then
		currcheat = { desc = desc, region = { rom = region } }
		currcheat.script = { off = string.format([[
			if on then
				for k, v in pairs(addrs) do
					rom:write_u%d(v.addr, v.save)
				end
			end]], size),
			on = string.format([[
			addrs = {
			--flag
			}
			on = true
			for k, v in pairs(addrs) do
				v.save = rom:read_u%d(v.addr)
				rom:write_u%d(v.addr, v.val)
			end]], size, size) }
		cheat = currcheat

	end
	if banksize and comp then
		local rom = manager:machine():memory().regions[region]
		local bankaddr = addr & (banksize - 1)
		addr = nil
		if not rom then
			error("rom cheat invalid region " .. desc)
		end
		for i = 0, rom.size, banksize do
			if rom:read_u8(i + bankaddr) == comp then
				addr = i + bankaddr
				break
			end
		end
		if not addr then
			error("rom cheat compare value not found " .. desc)
		end
	end
	currcheat.script.on = currcheat.script.on:gsub("%-%-flag", string.format("{addr = %d, val = %d},\n--flag", addr, val), 1)
	return cheat
end

local function prepare_ram_cheat(desc, tag, addr, val, size)
	local cheat
	if desc:sub(1,1) ~= "^" then
		currcheat = { desc = desc, space = { cpup = { tag = tag, type = "program" } }, script = { run = "" } }
		cheat = currcheat
	end
	currcheat.script.run = currcheat.script.run .. " cpup:write_u" .. size .. "(" .. addr .. "," .. val .. ", true)"
	return cheat
end

function codefuncs.nes_gg(desc, code)
	local xlate = { A = 0, P = 1, Z = 2, L = 3, G = 4, I = 5, T = 6, Y = 7, E = 8,
			O = 9, X = 10, U = 11, K = 12, S = 13, V = 14, N = 15 }
	local value = 0
	code:upper():gsub("(.)", function(s)
			if not xlate[s] then
				error("error parsing game genie cheat " .. desc)
			end
			value = (value << 4) | xlate[s]
		end)
	local addr, newval, comp
	if #code == 6 then
		addr = ((value >> 4) & 7) | ((value >> 8) & 0x78) | ((value >> 12) & 0x80) | ((value << 8) & 0x700) | ((value << 4) & 0x7800)
		newval = ((value >> 20) & 7) | (value & 8) | ((value >> 12) & 0x70) | ((value >> 16) & 0x80)
		return prepare_rom_cheat(desc, ":nes_slot:cart:prg_rom", addr, newval, 8)
	elseif #code == 8 then
		addr = ((value >> 12) & 7) | ((value >> 16) & 0x78) | ((value >> 20) & 0x80) | (value & 0x700) | ((value >> 4) & 0x7800)
		newval = ((value >> 28) & 7) | (value & 8) | ((value >> 20) & 0x70) | ((value >> 24) & 0x80)
		comp = ((value >> 4) & 7) | ((value >> 8) & 8) | ((value << 4) & 0x70) | (value & 0x80)
		-- assume 8K banks, 32K also common but is an easy multiple of 8K
		return prepare_rom_cheat(desc, ":nes_slot:cart:prg_rom", addr, newval, 8, 8192, comp)
	else
		error("error game genie cheat incorrect length " .. desc)
	end
end

function codefuncs.nes_ar(desc, code)
	code = code:gsub("[: %-]", "")
	if #code ~= 8 then
		error("error action replay cheat incorrect length " .. desc)
	end
	local newval = tonumber(code:sub(7, 8), 16)
	local addr = tonumber(code:sub(3, 6), 16)
	if not newval or not addr then
		error("error parsing action replay cheat " .. desc)
	end
	return prepare_ram_cheat(desc, ":maincpu", addr, newval, 8)
end

local function snes_prepare_cheat(desc, addr, val)
	local bank = addr >> 16
	local offset = addr & 0xffff
	if ((bank <= 0x3f) and (offset < 0x2000)) or ((bank & 0xfe) == 0x7e) then
		return prepare_ram_cheat(desc, ":maincpu", addr, val, 8)
	end
	if (manager:machine().devices[":maincpu"].spaces["program"]:read_u8(0xffd5) & 1) == 1 then --hirom
		if (bank & 0x7f) <= 0x3f and offset >= 0x8000 then
			-- direct map
		elseif (bank & 0x7f) >= 0x40 and (bank & 0x7f) <= 0x7d then
			addr = addr & 0x3fffff
		elseif bank >= 0xfe then
			addr = addr & 0x3fffff
		else
			error("error cheat not rom or ram addr " .. desc)
		end
	else --lorom
		if (bank & 0x7f) <= 0x3f and offset >= 0x8000 then
			addr = ((addr >> 1) & 0x3f8000) | (addr & 0x7fff)
		elseif (bank & 0x7f) >= 0x40 and (bank & 0x7f) <= 0x6f then
			addr = ((addr >> 1) & 0x3f8000) | (addr & 0x7fff)
		elseif (bank & 0x7f) >= 0x70 and (bank & 0x7f) <= 0x7d and offset >= 0x8000 then
			addr = ((addr >> 1) & 0x3f8000) | (addr & 0x7fff)
		elseif bank >= 0xfe and offset >= 0x8000 then
			addr = ((addr >> 1) & 0x3f8000) | (addr & 0x7fff)
		else
			error("error cheat not rom or ram addr " .. desc)
		end
	end
	return prepare_rom_cheat(desc, ":snsslot:cart:rom", addr, val, 8)
end

function codefuncs.snes_gg(desc, code)
	local xlate = { D = 0, F = 1, ["4"] = 2, ["7"] = 3, ["0"] = 4, ["9"] = 5, ["1"] = 6, ["5"] = 7,
			["6"] = 8, B = 9, C = 10, ["8"] = 11, A = 12, ["2"] = 13, ["3"] = 14, E = 15 }
	local value = 0
	local count = 0
	code:upper():gsub("(.)", function(s)
			if s == "-" then
				return
			elseif not xlate[s] then
				error("error parsing game genie cheat " .. desc)
			end
			count = count + 1
			value = (value << 4) | xlate[s]
		end)
	if count ~= 8 then
		error("error game genie cheat incorrect length " .. desc)
	end
	local newval = (value >> 24) & 0xff
	local addr = ((value >> 6) & 0xf) | ((value >> 12) & 0xf0) | ((value >> 6) & 0x300) | ((value << 10) & 0xc00) |
			((value >> 8) & 0xf000) | ((value << 14) & 0xf0000) | ((value << 10) & 0xf00000)
	return snes_prepare_cheat(desc, addr, newval)
end

function codefuncs.snes_ar(desc, code)
	code = code:gsub("[: %-]", "")
	if #code ~= 8 then
		error("error action replay cheat incorrect length " .. desc)
	end
	local addr = tonumber(code:sub(1, 6), 16)
	local val = tonumber(code:sub(7, 8), 16)
	if not addr or not val then
		error("error parsing action replay cheat " .. desc)
	end
	return snes_prepare_cheat(desc, addr, val)
end

function codefuncs.megadriv_gg(desc, code)
	local xlate = { A = 0, B = 1, C = 2, D = 3, E = 4, F = 5, G = 6, H = 7, J = 8, K = 9, L = 10, M = 11, N = 12,
			P = 13, R = 14, S = 15, T = 16, V = 17,  W = 18, X = 19, Y = 20, Z = 21, ["0"] = 22, ["1"] = 23,
			["2"] = 24, ["3"] = 25, ["4"] = 26, ["5"] = 27, ["6"] = 28, ["7"] = 29, ["8"] = 30, ["9"] = 31 }
	local value = 0
	local count = 0
	code:upper():gsub("(.)", function(s)
			if s == "-" then
				return
			elseif not xlate[s] then
				error("error parsing game genie cheat " .. desc)
			end
			count = count + 1
			value = (value << 5) | xlate[s]
		end)
	if count ~= 8 then
		error("error game genie cheat incorrect length " .. desc)
	end
	local newval = ((value >> 32) & 0xff) | ((value >> 3) & 0x1f00) | ((value << 5) & 0xe000)
	local addr = (value & 0xff00ff) | ((value >> 16) & 0xff00)
	return prepare_rom_cheat(desc, ":mdslot:cart:rom", addr, newval, 16)
end

function codefuncs.megadriv_ar(desc, code)
	code = code:gsub("[: %-]", "")
	if #code ~= 10 then
		error("error action replay cheat incorrect length " .. desc)
	end
	local addr = tonumber(code:sub(1, 6), 16)
	local val = tonumber(code:sub(7, 10), 16)
	if addr < 0xff0000 then
		error("error action replay cheat not ram addr " .. desc)
	end
	return prepare_ram_cheat(desc, ":maincpu", addr, val, 16)
end

local function gbgg_ggcodes(desc, code, region)
	code = code:gsub("%-", "")
	local comp
	if #code == 6 then
		comp = -1
	elseif #code == 9 then
		comp = ~tonumber(code:sub(7, 7) .. code:sub(9, 9), 16) & 0xff
		comp = ((comp >> 2) | ((comp << 6) & 0xc0)) ~ 0x45
	else
		error("error game genie cheat incorrect length " .. desc)
	end
	local newval = tonumber(code:sub(1, 2), 16)
	local addr = tonumber(code:sub(6, 6) .. code:sub(3, 5), 16)
	if not newval or not addr or not comp then
		error("error parsing game genie cheat " .. desc)
	end
	addr = (~addr & 0xf000) | (addr & 0xfff)
	if addr > 0x7fff then
		error("error game genie cheat bad addr " .. desc)
	end
	if comp == -1 then
		return prepare_rom_cheat(desc, region, addr, newval, 8)
	else
		-- assume 8K banks
		return prepare_rom_cheat(desc, region, addr, newval, 8, 8192, comp)
	end
	return cheat
end

function codefuncs.gameboy_gg(desc, code)
	return gbgg_ggcodes(desc, code, ":gbslot:cart:rom")
end

function codefuncs.gamegear_gg(desc, code)
	return gbgg_ggcodes(desc, code, ":slot:cart:rom")
end

function codefuncs.gamegear_ar(desc, code)
	code = code:gsub("[: %-]", "")
	if #code ~= 8 then
		error("error action replay cheat incorrect length " .. desc)
	end
	local addr = tonumber(code:sub(1, 6), 16)
	local val = tonumber(code:sub(7, 8), 16)
	if addr < 0xc000 or addr >= 0xe000 then
		error("error action replay cheat not ram addr " .. desc)
	end
	return prepare_ram_cheat(desc, ":maincpu", addr, val, 8)
end

codefuncs.sms_ar = codefuncs.gamegear_ar

function simple.conv_cheat(data)
	local cheats = {}
	for line in data:gmatch('([^\n;]+)') do
		local set, cputag, offset, size, val, desc = line:match('([^,]+),([^,]+),([^,]+),?([^,]*),?([^,]*),(.*)')
		if set == simple.romset then
			local cheat
			if cputag:sub(1,1) ~= ":" then
				local list, name = set:match('([^/]+)/(.+)')
				local func = list .. "_" .. cputag
				if list and desc and codefuncs[func] then
					local status
					status, cheat = pcall(codefuncs[func], desc, offset)
					if not status then
						emu.print_error(cheat)
						cheat = nil
					end
				end
			elseif size and val then
				if size == "w" then
					size = 16
				elseif size == "d" then
					size = 32
				elseif size == "q" then
					size = 64
				else
					size = 8
				end
				offset = tonumber(offset, 16)
				val = tonumber(val, 16)
				if manager:machine().devices[cputag] then
					cheat = prepare_ram_cheat(desc, cputag, offset, val, size)
				else
					cheat = prepare_rom_cheat(desc, cputag, offset, val, size)
				end
			end
			if cheat then
				cheats[#cheats + 1] = cheat
			end
		end
	end
	currcheat = nil
	return cheats
end

return simple

