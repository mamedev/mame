-- license:BSD-3-Clause
-- copyright-holders:Miodrag Milanovic
require('lfs')

-- add helper to lfs for plugins to use
function lfs.env_replace(str)
	local pathsep = package.config:sub(1,1)
	local function dorep(val)
		ret = os.getenv(val)
		if ret then
			return ret
		end
		return val
	end

	if pathsep == '\\' then
		str = str:gsub("%%(%w+)%%", dorep)
	else
		str = str:gsub("%$(%w+)", dorep)
	end
	return str
end

_G._ = emu.lang_translate
local dir = lfs.env_replace(manager:options().entries.pluginspath:value())

package.path = dir .. "/?.lua;" .. dir .. "/?/init.lua"

for _,entry in pairs(manager:plugins()) do
	if (entry.type == "plugin" and entry.start) then
		emu.print_verbose("Starting plugin " .. entry.name .. "...")
		plugin = require(entry.name)
		if plugin.set_folder~=nil then plugin.set_folder(entry.directory) end
		plugin.startplugin();
	end
end
