local dat = {}
local ver, info
local datread = require("data/load_dat")
datread, ver = datread.open("sysinfo.dat", "# This file was generated on")

function dat.check(set, softlist)
	if softlist or not datread then
		return nil
	end
	local status
	status, info = pcall(datread, "bio", "info", set)
	if not status or not info then
		return nil
	end
	return _p("plugin-data", "Sysinfo")
end

function dat.get()
	return info
end

function dat.ver()
	return ver
end

return dat
