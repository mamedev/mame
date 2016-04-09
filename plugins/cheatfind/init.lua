-- license:BSD-3-Clause
-- copyright-holders:Carl
-- This is a library of functions to be used at the Lua console as cf.getspaces() etc...
local exports = {}
exports.name = "cheatfind"
exports.version = "0.0.1"
exports.description = "Cheat finder helper library"
exports.license = "The BSD 3-Clause License"
exports.author = { name = "Carl" }

local cheatfind = exports

function cheatfind.startplugin()
	local cheat = {}

	-- return table of devices and spaces
	function cheat.getspaces()
		local spaces = {}
		for tag, device in pairs(manager:machine().devices) do
			if device.spaces then
				spaces[tag] = {}
				for name, space in pairs(device.spaces) do
					spaces[tag][name] = space
				end
			end
		end
		return spaces
	end

	-- return table of ram devices
	function cheat.getram()
		local ram = {}
		for tag, device in pairs(manager:machine().devices) do
			if device:shortname() == "ram" then
				ram[tag] = device
			end
		end
	end

	-- save data block
	function cheat.save(space, start, size)
		local data = { block = "", dev = space, start = start, size = size, space = space }
		if space.shortname then
			if space:shortname() == "ram" then
				data.block = emu.item(device.items["0/m_pointer"]):read_block(start, size)
				if not data.block then
					return nil
				end
			end
		else
			local block = ""
			for i = start, start + size do
				block = block .. string.pack("B", space:read_u8(i))
			end
			data.block = block
		end
		return data
	end

	-- compare a data block to the current state
	function cheat.comp(olddata, oper, val)
		local newdata = cheat.save(olddata.dev, olddata.start, olddata.size, olddata.space)
		local ret = {}
		if not val then
			val = 0
		end
		if oper == "+" or oper == "inc" then
			for i = 1, olddata.size do
				local old = string.unpack("B", olddata.block, i)
				local new = string.unpack("B", newdata.block, i)
				if old < new then
					if (val > 0 and (old + val) == new) or val == 0 then
						ret[#ret + 1] = { addr = olddata.start + i,
								  oldval = old,
								  newval = new}
					end
				end
			end
		elseif oper == "-" or oper == "dec" then
			for i = 1, olddata.size do
				local old = string.unpack("B", olddata.block, i)
				local new = string.unpack("B", newdata.block, i)
				if old > new then
					if (val > 0 and (old - val) == new) or val == 0 then
						ret[#ret + 1] = { addr = olddata.start + i,
								  oldval = old,
								  newval = new}
					end
				end
			end
		end
		return ret
	end

	_G.cf = cheat
end

return exports
