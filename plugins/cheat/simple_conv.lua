local simple = {}

-- converter for simple cheats
-- simple cheats are single address every frame ram cheats in one file called cheat.simple
-- format:  <set name>,<cputag>,<hex offset>,<hex value>,<desc>
-- only program address space is supported, comments are prepended with ;
function simple.conv_cheat(romset, data)
	local cheats = {}
	for line in data:gmatch('([^\n;]+)') do
		local set, cputag, offset, val, name = line:match('([^,]+),([^,]+),([^,]+),([^,]+),(.+)')
		if set == romset then
			local cheat = {}
			cheat.desc = name
			cheat.space = { cpup = { tag = cputag, type = "program" } }
			cheat.script = { run = "cpup:write_u8(0x" .. offset .. ",0x" .. val .. ", true)" }
			cheats[#cheats + 1] = cheat
		end
	end
	return cheats
end

return simple

