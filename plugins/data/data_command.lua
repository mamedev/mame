local dat = {}
local info, ver

local datread = require("data/load_dat")
datread, ver = datread.open("command.dat", nil)

function dat.check(set, softlist)
	if softlist or not datread then
		return nil
	end
	local status
	status, info = pcall(datread, "cmd", "info", set)
	if not status or not info then
		return nil
	end
	local convert = require("data/button_char")
	info = "#jf\n" .. convert(info)
	return "Command"
end

function dat.get()
	return info
end

return dat
