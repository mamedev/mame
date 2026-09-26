local dat = {}

local ver, info
local datread = require('data/load_dat')
datread, ver = datread.open('story.dat', '# version')

function dat.check(set, softlist)
	if softlist or not datread then
		return nil
	end
	local status, data = pcall(datread, 'story', 'info', set)
	if not status or not data then
		return nil
	end
	local str_gsub = string.gsub
	local table_ins = table.insert
	local lines = {}
	data = str_gsub(data, 'MAMESCORE records : ([^\n]+)', 'MAMESCORE records :\t\n%1', 1)
	for line in string.gmatch(data, '[^\n]*') do
		if (line ~= '') or ((#lines ~= 0) and (lines[#lines] ~= '')) then
			local reformatted = str_gsub(line, '^(.-)_+([0-9.]+)$', '%1\t%2')
			table_ins(lines, reformatted)
		end
	end
	info = '#j2\n' .. table.concat(lines, '\n')
	return _p('plugin-data', 'Mamescore')
end

function dat.get()
	return info
end

function dat.ver()
	return ver
end

return dat
