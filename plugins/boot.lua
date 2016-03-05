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
		if (meta["plugin"]["type"]=="plugin") and (meta["plugin"]["start"]=="true") then
			server = require(meta["plugin"]["name"])
			server.startplugin();
		end
	end
end

