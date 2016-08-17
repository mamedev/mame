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
local dir = lfs.env_replace(manager:options().entries.pluginspath:value())

package.path = dir .. "/?.lua;" .. dir .. "/?/init.lua"

local json = require('json')
local function readAll(file)
    local f = io.open(file, "rb")
    local content = f:read("*all")
    f:close()
    return content
end

for file in lfs.dir(dir) do
	if (file~="." and file~=".." and lfs.attributes(dir .. "/" .. file,"mode")=="directory") then
		local filename = dir .. "/" .. file .. "/plugin.json"
		local meta = json.parse(readAll(filename))
		if (meta["plugin"]["type"]=="plugin") and (mame_manager:plugins().entries[meta["plugin"]["name"]]~=nil) then
			local entry = mame_manager:plugins().entries[meta["plugin"]["name"]]	
			if (entry:value()==true) then
				emu.print_verbose("Starting plugin " .. meta["plugin"]["name"] .. "...")
				plugin = require(meta["plugin"]["name"])
				if plugin.set_folder~=nil then plugin.set_folder(dir .. "/" .. file) end
				plugin.startplugin();
			end
		end
	end
end

