-- license:BSD-3-Clause
-- copyright-holders:Carl
require('lfs')
local exports = {}
exports.name = "timer"
exports.version = "0.0.1"
exports.description = "Game play timer"
exports.license = "The BSD 3-Clause License"
exports.author = { name = "Carl" }

local timer = exports

function timer.startplugin()
	local timer_path = "timer"
	local timer_started = false
	local total_time = 0
	local start_time = 0

	local function get_filename()
		local path
		if emu.softname() ~= "" then
			path = timer_path .. '/' .. emu.romname() .. "_" .. emu.softname() .. ".time"
		else
			path = timer_path .. '/' .. emu.romname() .. ".time"
		end
		return path
	end

	emu.register_start(function()
		local file
		if timer_started then
			total_time = total_time + (os.time() - start_time)
			os.remove(get_filename()) -- truncate file
			file = io.open(get_filename(), "w")
			if not file then
				lfs.mkdir(timer_path)
				file = io.open(get_filename(), "w")
			end
			if file then
				file:write(total_time)
				file:close()
			end
		end
		timer_started = true
		local file = io.open(get_filename(), "r")
		if file then
			total_time = file:read("n")
			file:close()
		end
		start_time = os.time()
	end)

	local function sectohms(time)
		local hrs = math.floor(time / 3600)
		local min = math.floor((time % 3600) / 60)
		local sec = math.floor(time % 60)
		return string.format("%02d:%02d:%02d", hrs, min, sec)
	end

	local function menu_populate()
		local time = os.time() - start_time
		return {{ "Current time", "", 32 },
			{ sectohms(time), "", 32 },
			{ "Total time", "", 32 },
			{ sectohms(total_time + time), "", 32 }}
	end

	local function menu_callback(index, event)
		return true
	end

	emu.register_menu(menu_callback, menu_populate, "Timer")
end

return exports
