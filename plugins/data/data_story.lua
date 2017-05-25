local dat = {}
local ver, info
local datread = require("data/load_dat")
local space_wid = mame_manager:ui():get_char_width(0x200a)
datread, ver = datread.open("story.dat", "# version")

function dat.check(set, softlist)
	if softlist or not datread then
		return nil
	end
	local status
	status, info = pcall(datread, "story", "info", set)
	if not status or not info then
		return nil
	end
	info = "#jf\n" .. info:gsub("([^%s]-)(%f[_]_+%f[0-9])([0-9.]+)",
		function(name, sep, score)
			local wid = mame_manager:ui():get_string_width(name .. score, 1.0)
			return name .. string.rep(utf8.char(0x200a), math.floor((.4 - wid) / space_wid)) .. score
		end)
	return "Mamescore"
end

function dat.get()
	return info
end

function dat.ver()
	return ver
end

return dat
