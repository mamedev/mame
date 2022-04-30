-- license:BSD-3-Clause
-- copyright-holders:Miodrag Milanovic
require('lfs')

_G._ = emu.lang_translate
_G._p = emu.lang_translate
_G.N_ = function (message) return message end
_G.N_p = function (context, message) return message end
_G.emu.plugin = {} -- table to contain plugin interfaces
-- substitute environment variables in the plugins path from options
local dirs = emu.subst_env(manager.options.entries.pluginspath:value())

-- and split the paths apart and make them suitable for package.path
package.path = ""
for dir in string.gmatch(dirs, "([^;]+)") do
	if (package.path ~= "") then
		package.path = package.path .. ";"
	end
	package.path = package.path .. dir .. "/?.lua;" .. dir .. "/?/init.lua"
end

for _,entry in pairs(manager.plugins) do
	if (entry.type == "plugin" and entry.start) then
		emu.print_verbose("Starting plugin " .. entry.name .. "...")
		plugin = require(entry.name)
		if plugin.set_folder~=nil then plugin.set_folder(entry.directory) end
		plugin.startplugin();
	end
end
