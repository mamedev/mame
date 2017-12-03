local dat = {}
local ver, info
local datread = require("data/load_dat")
datread, ver = datread.open("history.dat", "## REVISION:")

function dat.check(set, softlist)
	if not datread then
		return nil
	end
	local status
	status, info = pcall(datread, "bio", softlist or "info", set)
	if not status or not info then
		return nil
	end
	return _("History")
end

function dat.get()
	return info
end

function dat.ver()
	return ver
end

return dat
