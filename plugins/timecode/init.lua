-- license:BSD-3-Clause
-- copyright-holders:Vas Crabb
local exports = {
	name = 'timecode',
	version = '0.0.1',
	description = 'Timecode recorder plugin',
	license = 'BSD-3-Clause',
	author = { name = 'Vas Crabb' } }


local timecode = exports

function timecode.startplugin()
	local file                  -- the timecode log file
	local enabled               -- whether timecode recording is enabled
	local write                 -- whether to record a timecode on the next emulated frame
	local text                  -- name of current part
	local start_time            -- start time for current part
	local total_time            -- total time of parts so far this session
	local count                 -- current timecode number
	local show_counter          -- whether to show elapsed time since last timecode
	local show_total            -- whether to show the total time of parts

	local hotkey_seq            -- input sequence to record timecode
	local hotkey_pressed        -- whether the hotkey was pressed on the last frame update
	local hotkey_cfg            -- configuration string for the hotkey

	local item_hotkey           -- menu index of hotkey item
	local commonui              -- common UI helpers
	local hotkey_poller         -- helper for configuring hotkey


	local function get_settings_path()
		return emu.subst_env(manager.machine.options.entries.homepath:value():match('([^;]+)')) .. '/timecode/'
	end


	local function process_frame()
		if (not manager.machine.paused) and file and write then
			write = false
			count = count + 1
			show_total = true

			-- milliseconds from beginning of playback
			local curtime = manager.machine.time
			local cursec = curtime.seconds
			local msec_start = (cursec * 1000) + curtime.msec
			local msec_start_str = string.format('%015d', msec_start)

			-- display the timecode
			local curtime_str = string.format(
					'%02d:%02d:%02d.%03d',
					cursec // (60 * 60),
					(cursec // 60) % 60,
					cursec % 60,
					msec_start % 1000)

			-- milliseconds from previous timecode
			local elapsed = curtime - start_time
			local elapsedsec = elapsed.seconds
			local msec_elapsed = (elapsedsec * 1000) + elapsed.msec
			local msec_elapsed_str = string.format('%015d', msec_elapsed)

			-- elapsed from previous timecode
			start_time = curtime
			local elapsed_str = string.format(
					'%02d:%02d:%02d.%03d',
					elapsedsec // (60 * 60),
					(elapsedsec // 60) % 60,
					elapsedsec % 60,
					msec_elapsed % 1000)

			-- number of frames from beginning of playback
			-- TODO: should this account for actual frame rate rather than assuming 60fps?
			local frame_start_str = string.format('%015d', msec_start * 60 // 1000)

			-- number of frames from previous timecode
			-- TODO: should this account for actual frame rate rather than assuming 60fps?
			local frame_elapsed_str = string.format('%015d', msec_elapsed * 60 // 1000)

			local message
			local key
			if count == 1 then
				text = 'INTRO'
				show_counter = true
				message = string.format(_p('plugin-timecode', 'TIMECODE: Intro started at %s'), curtime_str)
				key = 'INTRO_START'
			elseif count == 2 then
				total_time = total_time + elapsed
				show_counter = false
				message = string.format(_p('plugin-timecode', 'TIMECODE: Intro duration %s'), elapsed_str)
				key = 'INTRO_STOP'
			elseif count == 3 then
				text = 'GAMEPLAY'
				show_counter = true
				message = string.format(_p('plugin-timecode', 'TIMECODE: Gameplay started at %s'), curtime_str)
				key = 'GAMEPLAY_START'
			elseif count == 4 then
				total_time = total_time + elapsed
				show_counter = false
				message = string.format(_p('plugin-timecode', 'TIMECODE: Gameplay duration %s'), elapsed_str)
				key = 'GAMEPLAY_STOP'
			elseif (count % 2) == 1 then
				local extrano = (count - 3) // 2
				text = string.format('EXTRA %d', extrano)
				show_counter = true
				message = string.format(_p('plugin-timecode', 'TIMECODE: Extra %d started at %s'), extrano, curtime_str)
				key = string.format('EXTRA_START_%03d', extrano)
			else
				local extrano = (count - 4) // 2
				total_time = total_time + elapsed
				show_counter = false
				message = string.format(_p('plugin-timecode', 'TIMECODE: Extra %d duration %s'), extrano, elapsed_str)
				key = string.format('EXTRA_STOP_%03d', extrano)
			end

			emu.print_info(message)
			manager.machine:popmessage(message)

			file:write(
					string.format(
						'%-19s %s %s %s %s %s %s\n',
						key,
						curtime_str, elapsed_str,
						msec_start_str, msec_elapsed_str,
						frame_start_str, frame_elapsed_str))
		end
	end


	local function process_frame_done()
		local machine = manager.machine
		if show_counter then
			-- show duration of current part
			local counter = (machine.time - start_time).seconds
			local counter_str = string.format(
					machine.paused and _p('plugin-timecode', ' %s%s%02d:%02d [paused] ') or _p('plugin-timecode', ' %s%s%02d:%02d '),
					text,
					(#text > 0) and ' ' or '',
					(counter // 60) % 60,
					counter % 60)
			machine.render.ui_container:draw_text('right', 0, counter_str, 0xf0f01010, 0xff000000)
		end
		if show_total then
			-- show total time for all parts so far
			local total = ((show_counter and (machine.time - start_time) or emu.attotime()) + total_time).seconds
			total_str = string.format(_p('plugin-timecode', 'TOTAL %02d:%02d '), (total // 60) % 60, total % 60)
			machine.render.ui_container:draw_text('left', 0, total_str, 0xf010f010, 0xff000000)
		end
		if enabled then
			local pressed = machine.input:seq_pressed(hotkey_seq)
			if (not hotkey_pressed) and pressed then
				write = true
			end
			hotkey_pressed = pressed
		end
	end


	local function start()
		hotkey_seq = manager.machine.input:seq_from_tokens('KEYCODE_F12 NOT KEYCODE_LSHIFT NOT KEYCODE_RSHIFT NOT KEYCODE_LALT NOT KEYCODE_RALT')

		-- try to load configuration
		local cfgname = get_settings_path() .. 'plugin.cfg'
		local cfgfile = io.open(cfgname, 'r')
		if cfgfile then
			local json = require('json')
			local settings = json.parse(cfgfile:read('a'))
			cfgfile:close()
			if not settings then
				emu.print_error(string.format('Error loading timecode recorder settings: error parsing file "%s" as JSON', cfgname))
			else
				hotkey_cfg = settings.hotkey
				if hotkey_cfg then
					local seq = manager.machine.input:seq_from_tokens(hotkey_cfg)
					if seq then
						hotkey_seq = seq
					end
				end
			end
		end

		-- only do timecode recording if we're doing input recording
		local options = manager.machine.options.entries
		local filename = options.record:value()
		enabled = #filename > 0
		show_counter = false
		show_total = false
		if enabled then
			filename = filename .. '.timecode'
			emu.print_info(string.format('Record input timecode file: %s', filename))
			file = emu.file(options.input_directory:value(), 0x0e) -- FIXME: magic number for flags
			local openerr = file:open(filename)
			if openerr then
				-- TODO: this used to throw a fatal error and log the error description
				emu.print_error('Failed to open file for input timecode recording')
				enabled = false
			else
				write = false
				text = ''
				start_time = emu.attotime()
				total_time = emu.attotime()
				count = 0
				show_counter = false
				show_total = false
				hotkey_pressed = false

				file:write('# ==========================================\n')
				file:write('# TIMECODE FILE FOR VIDEO PREVIEW GENERATION\n')
				file:write('# ==========================================\n')
				file:write('#\n')
				file:write('# VIDEO_PART:     code of video timecode\n')
				file:write('# START:          start time (hh:mm:ss.mmm)\n')
				file:write('# ELAPSED:        elapsed time (hh:mm:ss.mmm)\n')
				file:write('# MSEC_START:     start time (milliseconds)\n')
				file:write('# MSEC_ELAPSED:   elapsed time (milliseconds)\n')
				file:write('# FRAME_START:    start time (frames)\n')
				file:write('# FRAME_ELAPSED:  elapsed time (frames)\n')
				file:write('#\n')
				file:write('# VIDEO_PART======= START======= ELAPSED===== MSEC_START===== MSEC_ELAPSED=== FRAME_START==== FRAME_ELAPSED==\n')
			end
		end
	end


	local function stop()
		-- close the file if we're recording
		if file then
			file:close()
			file = nil
		end

		-- try to save settings
		local path = get_settings_path()
		local attr = lfs.attributes(path)
		if not attr then
			lfs.mkdir(path)
		elseif attr.mode ~= 'directory' then
			emu.print_error(string.format('Error saving timecode recorder settings: "%s" is not a directory', path))
			return
		end
		if hotkey_cfg then
			local json = require('json')
			local settings = { hotkey = hotkey_cfg }
			local data = json.stringify(settings, { indent = true })
			local cfgname = path .. 'plugin.cfg'
			local cfgfile = io.open(cfgname, 'w')
			if not cfgfile then
				emu.print_error(string.format('Error saving timecode recorder settings: error opening file "%s" for writing', cfgname))
				return
			end
			cfgfile:write(data)
			cfgfile:close()
		end
	end


	local function menu_callback(index, event)
		if hotkey_poller then
			if hotkey_poller:poll() then
				if hotkey_poller.sequence then
					hotkey_seq = hotkey_poller.sequence
					hotkey_cfg = manager.machine.input:seq_to_tokens(hotkey_seq)
				end
				hotkey_poller = nil
				return true
			end
		elseif (index == item_hotkey) and (event == 'select') then
			if not commonui then
				commonui = require('commonui')
			end
			hotkey_poller = commonui.switch_polling_helper()
			return true
		end
		return false
	end


	local function menu_populate()
		local result = { }
		table.insert(result, { _p('plugin-timecode', 'Timecode Recorder'), '', 'off' })
		table.insert(result, { '---', '', '' })
		table.insert(result, { _p('plugin-timecode', 'Hotkey'), manager.machine.input:seq_name(hotkey_seq), hotkey_poller and 'lr' or '' })
		item_hotkey = #result
		if hotkey_poller then
			return hotkey_poller:overlay(result)
		else
			return result
		end
	end


	emu.register_frame(process_frame)
	emu.register_frame_done(process_frame_done)
	emu.register_prestart(start)
	emu.register_stop(stop)
	emu.register_menu(menu_callback, menu_populate, _p('plugin-timecode', 'Timecode Recorder'))
end

return exports
