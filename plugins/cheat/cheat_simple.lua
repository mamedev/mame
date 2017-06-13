-- converter for simple cheats
-- simple cheats are single address every frame ram or gg,ar cheats in one file called cheat.simple
-- ram cheat format:  <set name>,<cputag>,<hex offset>,<b|w|d|q - size>,<hex value>,<desc>
-- gg,ar cheat format: <set name>,<gg|ar - type>,<code>,<desc> like "nes/smb,gg,SXIOPO,Infinite Lives"
-- gg for game genie -- nes, snes, megadriv
-- set name is <softlist>/<entry> like "nes/smb" for softlist items
-- only program address space is supported, comments are prepended with ;
-- size is b - u8, w - u16, d - u32, q - u64
-- Don't use commas in the description

local simple = {}

simple.romset = "????"

function simple.filename(name)
	simple.romset = name
	return "cheat.simple"
end

local codefuncs = {}

function codefuncs.nes_gg(desc, code)
	local xlate = { A = 0, P = 1, Z = 2, L = 3, G = 4, I = 5, T = 6, Y = 7, E = 8,
			O = 9, X = 10, U = 11, K = 12, S = 13, V = 14, N = 15 }
	local cheat = { desc = desc, region = { rom = ":nes_slot:cart:prg_rom" } }
	local value = 0
	code:upper():gsub("(.)", function(s)
			if not xlate[s] then
				error("error parsing game genie cheat " .. desc)
			end
			value = (value << 4) | xlate[s]
		end)
	local addr, newval, comp
	cheat.script = { off = "if on then rom:write_u8(addr, save) end" }
	if #code == 6 then
		addr = ((value >> 4) & 7) | ((value >> 8) & 0x78) | ((value >> 12) & 0x80) | ((value << 8) & 0x700) | ((value << 4) & 0x7800)
		newval = ((value >> 20) & 7) | (value & 8) | ((value >> 12) & 0x70) | ((value >> 16) & 0x80)
		cheat.script.on = string.format([[
					on = true
					addr = %d
					save = rom:read_u8(addr)
					rom:write_u8(addr, %d)]], addr, newval)
	elseif #code == 8 then
		addr = ((value >> 12) & 7) | ((value >> 16) & 0x78) | ((value >> 20) & 0x80) | (value & 0x700) | ((value >> 4) & 0x7800)
		newval = ((value >> 28) & 7) | (value & 8) | ((value >> 20) & 0x70) | ((value >> 24) & 0x80)
		comp = ((value >> 4) & 7) | ((value >> 8) & 8) | ((value << 4) & 0x70) | ((value << 1) & 0x80)
		-- assume 8K banks, 32K also common but is an easy multiple of 8K
		cheat.script.on = string.format([[
				addr = %d
				save = %d
				for i = 0, rom.size, 8192 do
					if rom:read_u8(i + addr) == save then
						on = true
						addr = i + addr
						rom:write_u8(addr, %d)
						break
					end
				end]], addr & 0x3fff, comp, newval)
	else
		error("error game genie cheat incorrect length " .. desc)
	end
	return cheat
end

function codefuncs.snes_gg(desc, code)
	local xlate = { D = 0, F = 1, ["4"] = 2, ["7"] = 3, ["0"] = 4, ["9"] = 5, ["1"] = 6, ["5"] = 7,
			["6"] = 8, B = 9, C = 10, ["8"] = 11, A = 12, ["2"] = 13, ["3"] = 14, E = 15 }
	local value = 0
	local count = 0
	local cheat = { desc = desc, region = { rom = ":snsslot:cart:rom" } } 
	cheat.script = { off = "if on then rom:write_u8(addr, save) end" }
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
	if (manager:machine().devices[":maincpu"].spaces["program"]:read_u8(0xffd5) & 1) == 1 then --hirom
		local bank = addr >> 16
		local offset = addr & 0xffff
		if (bank & 0x7f) <= 0x3f and offset >= 0x8000 then
			-- direct map
		elseif (bank & 0x7f) >= 0x40 and (bank & 0x7f) <= 0x7d then
			addr = addr & 0x3fffff
		elseif bank >= 0xfe then
			addr = addr & 0x3fffff
		else
			error("error game genie cheat not rom addr " .. desc)
		end
	else --lorom
		local bank = addr >> 16
		local offset = addr & 0xffff
		if (bank & 0x7f) <= 0x3f and offset >= 0x8000 then
			addr = ((addr >> 1) & 0x78000) | (addr & 0x7fff)
		elseif (bank & 0x7f) >= 0x40 and (bank & 0x7f) <= 0x6f then
			addr = ((addr >> 1) & 0x78000) | (addr & 0x7fff)
		elseif (bank & 0x7f) >= 0x70 and (bank & 0x7f) <= 0x7d and offset >= 0x8000 then
			addr = ((addr >> 1) & 0x78000) | (addr & 0x7fff)
		elseif bank >= 0xfe and offset >= 0x8000 then
			addr = ((addr >> 1) & 0x78000) | (addr & 0x7fff)
		else
			error("error game genie cheat not rom addr " .. desc)
		end
	end
	cheat.script.on = string.format([[
				addr = %d
				save = rom:read_u8(addr)
				on = true
				rom:write_u8(addr, %d)
				]], addr, newval)
	return cheat
end

function codefuncs.megadriv_gg(desc, code)
	local xlate = { A = 0, B = 1, C = 2, D = 3, E = 4, F = 5, G = 6, H = 7, J = 8, K = 9, L = 10, M = 11, N = 12,
			P = 13, R = 14, S = 15, T = 16, V = 17,  W = 18, X = 19, Y = 20, Z = 21, ["0"] = 22, ["1"] = 23,
			["2"] = 24, ["3"] = 25, ["4"] = 26, ["5"] = 27, ["6"] = 28, ["7"] = 29, ["8"] = 30, ["9"] = 31 }
	local value = 0
	local count = 0
	local cheat = { desc = desc, region = { rom = ":mdslot:cart:rom" } } 
	cheat.script = { off = "if on then rom:write_u16(addr, save) end" }
	code:upper():gsub("(.)", function(s)
			if s == "-" then
				return
			elseif not xlate[s] then
				error("error parsing game genie cheat " .. desc)
			end
			count = count + 1
			value = (value << 5) | xlate[s]
		end)
	local newval = ((value >> 32) & 0xff) | ((value >> 3) & 0x1f00) | ((value << 5) & 0xe000)
	local addr = (value & 0xff00ff) | ((value >> 16) & 0xff00)
	cheat.script.on = string.format([[
				addr = %d
				save = rom:read_u16(addr)
				on = true
				rom:write_u16(addr, %d)
				]], addr, newval)
	return cheat
end

function simple.conv_cheat(data)
	local cheats = {}
	for line in data:gmatch('([^\n;]+)') do
		local set, cputag, offset, size, val, desc = line:match('([^,]+),([^,]+),([^,]+),?([^,]*),?([^,]*),(.*)')
		if set == simple.romset then
			local cheat
			if cputag.sub(1,1) ~= ":" then
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
				cheat = { desc = desc, space = { cpup = { tag = cputag, type = "program" } } }
				if size == "w" then
					size  "u16"
				elseif size == "d" then
					size = "u32"
				elseif size == "q" then
					size = "u64"
				else
					size = "u8"
				end
				cheat.script = { run = "cpup:write_" .. size .. "(0x" .. offset .. ",0x" .. val .. ", true)" }
			end
			if cheat then
				cheats[#cheats + 1] = cheat
			end
		end
	end
	return cheats
end

return simple

