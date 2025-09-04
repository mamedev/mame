local dat = {}

local ver, info
local datread = require('data/load_dat')
datread, ver = datread.open(
	'messinfo.dat',
	'#     MESSINFO.DAT',
	function (str) return str:gsub('\n\n', '\n') end)

function dat.check(set, softlist)
	if softlist or not datread then
		return nil
	end
	local status, drvinfo
	status, info = pcall(datread, 'mame', 'info', set)
	if not status or not info then
		return nil
	end
	local sourcefile = emu.driver_find(set).source_file:match('[^/\\]+[/\\\\][^/\\]*$')
	status, drvinfo = pcall(datread, 'drv', 'info', sourcefile)
	if not drvinfo then
		status, drvinfo = pcall(datread, 'drv', 'info', sourcefile:match('[^/\\]*$'))
	end
	if drvinfo then
		info = info .. _p('plugin-data', '\n\n--- DRIVER INFO ---\nDriver: ') .. sourcefile .. '\n\n' .. drvinfo
	end
	return _p('plugin-data', 'MESSinfo')
end

function dat.get()
	return info
end

function dat.ver()
	return ver
end

return dat
