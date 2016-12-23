-- license:BSD-3-Clause
-- copyright-holders:Carl
-- A data script should contain two functions check which takes a set name and returns the data
-- heading if it supports the set otherwise nil and get which returns the data
-- the script should be named data_<name>.lua
-- this is set default on in the plugin.json
local exports = {}
exports.name = "data"
exports.version = "0.0.1"
exports.description = "Data plugin"
exports.license = "The BSD 3-Clause License"
exports.author = { name = "Carl" }

local data = exports

function data.set_folder(path)
	data.path = path
end

function data.startplugin()
	local data_scr = {}
	local valid_lst = {}
	local cur_set
	local cur_list
	emu.register_start(function()
		data_scr = {}
		for file in lfs.dir(data.path) do
			local name = string.match(file, "^(data_.*).lua$")
			if name then
				local script = require("data/" .. name)
				if script then
					data_scr[#data_scr + 1] = script
				end
			end
		end
	end)
	emu.register_callback(function(set)
		local ret = {}
		if set == "" then
			set = emu.romname()
		end
		if set == cur_set then
			return cur_list
		end
		cur_set = set
		if not set then
			return nil
		end
		valid_lst = {}
		for num, scr in ipairs(data_scr) do
			local setname, softname = set:match("^([^,]+),?(.*)$")
			if softname == "" then
				softname = nil
			end
			local name = scr.check(setname, softname)
			if name then
				ret[#ret + 1] = name
				valid_lst[#valid_lst + 1] = scr
			end
		end
		cur_list = ret
		return ret
	end, "data_list")

	emu.register_callback(function(num)
		return valid_lst[num + 1].get()
	end, "data")

	emu.register_callback(function(num)
		local ver
		if valid_lst[num + 1].ver then
			ver = valid_lst[num + 1].ver()
		end
		if ver then
			return ver
		end
		return ""
	end, "data_version")
end

return exports
