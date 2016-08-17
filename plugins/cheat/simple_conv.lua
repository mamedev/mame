local simple = {}

-- converter for simple cheats
-- simple cheats are single address every frame ram cheats in one file called cheat.simple
-- format:  <set name>,<cputag>,<hex offset>,<b|w|d|q - size>,<hex value>,<desc>
-- only program address space is supported, comments are prepended with ;
-- size is b - u8, w - u16, d - u32, q - u64
function simple.conv_cheat(romset, data)
	local cheats = {}
	for line in data:gmatch('([^\n;]+)') do
		local set, cputag, offset, size, val, name = line:match('([^,]+),([^,]+),([^,]+),([^,]+),([^,]+),(.+)')
		if set == romset then
			local cheat = {}
			cheat.desc = name
			cheat.space = { cpup = { tag = cputag, type = "program" } }
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
			cheats[#cheats + 1] = cheat
		end
	end
	return cheats
end

return simple

