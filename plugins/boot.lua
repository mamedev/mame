-- license:BSD-3-Clause
-- copyright-holders:Miodrag Milanovic
require('lfs')
local cwd = lfs.currentdir()
package.path = cwd .. "/plugins/?.lua;" .. cwd .. "/plugins/?/init.lua"

local json = require('json')
function readAll(file)
    local f = io.open(file, "rb")
    local content = f:read("*all")
    f:close()
    return content
end

for file in lfs.dir("plugins") do
	if (file~="." and file~=".." and lfs.attributes("plugins/" .. file,"mode")=="directory") then
		local filename = "plugins/" .. file .. "/plugin.json"
		local meta = json.parse(readAll(filename))
		if (meta["plugin"]["type"]=="plugin") and (manager:plugins().entries[meta["plugin"]["name"]]~=nil) then
			local entry = manager:plugins().entries[meta["plugin"]["name"]]	
			if (entry:value()==true) then
				print("Starting plugin " .. meta["plugin"]["name"] .. "...")
				plugin = require(meta["plugin"]["name"])
				if plugin.set_folder~=nil then plugin.set_folder("plugins/" .. file) end
				plugin.startplugin();
			end
		end
	end
end

