local dat = {}
local ver, info
local datread = require("data/load_dat")
datread, ver = datread.open("story.dat", "# version")

function dat.check(set, softlist)
	if softlist or not datread then
		return nil
	end
	local status, data = pcall(datread, "story", "info", set)
	if not status or not data then
		return nil
	end
	local lines = {}
	data = data:gsub("MAMESCORE records : ([^\n]+)", "MAMESCORE records :\t\n%1", 1)
	for line in data:gmatch("[^\n]*") do
		if not (#lines == 0 and line == "") and not (lines[#lines] == "" and line == "") then
			lines[#lines + 1] = line:gsub("^(.-)_+([0-9.]+)$", "%1\t%2")
		end
	end
	info = "#j2\n" .. table.concat(lines, "\n")
	return _("Mamescore")
end

function dat.get()
	return info
end

function dat.ver()
	return ver
end

return dat
