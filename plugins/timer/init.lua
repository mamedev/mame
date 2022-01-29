-- license:BSD-3-Clause
-- copyright-holders:Vas Crabb
local exports = {
	name = 'timer',
	version = '0.0.3',
	description = 'Game play timer',
	license = 'BSD-3-Clause',
	author = { name = 'Vas Crabb' } }

local timer = exports

function timer.startplugin()
	local total_time = 0
	local start_time = 0
	local play_count = 0
	local emu_total = emu.attotime()

	local reference = 0
	local lastupdate
	local highlight -- hacky - workaround for the menu not remembering the selected item if its ref is nullptr


	local function sectohms(time)
		local hrs = time // 3600
		local min = (time % 3600) // 60
		local sec = time % 60
		return string.format(_p('plugin-timer', '%03d:%02d:%02d'), hrs, min, sec)
	end

	local function menu_populate()
		lastupdate = os.time()
		local refname = (reference == 0) and _p('plugin-timer', 'Wall clock') or _p('plugin-timer', 'Emulated time')
		local time = (reference == 0) and (lastupdate - start_time) or manager.machine.time.seconds
		local total = (reference == 0) and (total_time + time) or (manager.machine.time + emu_total).seconds
		return
			{
				{ _p("plugin-timer", "Reference"), refname, (reference == 0) and 'r' or 'l' },
				{ '---', '', '' },
				{ _p("plugin-timer", "Current time"), sectohms(time), "off" },
				{ _p("plugin-timer", "Total time"), sectohms(total), "off" },
				{ _p("plugin-timer", "Play Count"), play_count, "off" } },
			highlight,
			"idle"
	end

	local function menu_callback(index, event)
		if (index == 1) and ((event == 'left') or (event == 'right') or (event == 'select')) then
			reference = reference ~ 1
			return true
		end
		highlight = index
		return os.time() > lastupdate
	end


	emu.register_start(
		function()
			if emu.romname() ~= '___empty' then
				start_time = os.time()
				local persister = require('timer/timer_persist')
				total_time, play_count, emu_total = persister:start_session()
			end
		end)

	emu.register_stop(
		function()
			if emu.romname() ~= '___empty' then
				local persister = require('timer/timer_persist')
				persister:update_totals(start_time)
			end
		end)

	emu.register_menu(menu_callback, menu_populate, _p("plugin-timer", "Timer"))
end

return exports
