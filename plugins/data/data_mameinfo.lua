local dat = {}
local ver, info
local datread = require("data/load_dat")
datread, ver = datread.open("mameinfo.dat", "# MAMEINFO.DAT", function(str) return str:gsub("\n\n", "\n") end)

function dat.check(set, softlist)
	if softlist or not datread then
		return nil
	end
	local status, drvinfo
	status, info = pcall(datread, "mame", "info", set)
	if not status or not info then
		return nil
	end
	local sourcefile = emu.driver_find(set).source_file:match("[^/\\]*$")
	status, drvinfo = pcall(datread, "drv", "info", sourcefile)
	if drvinfo then
		info = info .. _("\n\n--- DRIVER INFO ---\nDriver: ") .. sourcefile .. "\n\n" .. drvinfo
	end
	return _("MAMEinfo")
end

function dat.get()
	return info
end

function dat.ver()
	return ver
end

return dat
