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
	local write                 -- whether to record a timecode on the next emulated frame
	local text                  -- name of current part
	local frame_count           -- emulated frame counter
	local start_frame           -- start frame count for current part
	local start_time            -- start time for current part
	local total_time            -- total time of parts so far this session
	local count                 -- current timecode number
	local show_counter          -- whether to show elapsed time since last timecode
	local show_total            -- whether to show the total time of parts

	local frame_mode            -- 0 to count frames, 1 to assume 60 Hz
	local hotkey_seq            -- input sequence to record timecode
	local hotkey_pressed        -- whether the hotkey was pressed on the last frame update
	local hotkey_cfg            -- configuration string for the hotkey

	local item_framemode        -- menu index of frame mode item
	local item_hotkey           -- menu index of hotkey item
	local commonui              -- common UI helpers
	local hotkey_poller         -- helper for configuring hotkey


	local function get_settings_path()
		return emu.subst_env(manager.machine.options.entries.homepath:value():match('([^;]+)')) .. '/timecode'
	end


	local function set_default_hotkey()
		hotkey_seq = manager.machine.input:seq_from_tokens('KEYCODE_F12 NOT KEYCODE_LSHIFT NOT KEYCODE_RSHIFT NOT KEYCODE_LALT NOT KEYCODE_RALT')
		hotkey_cfg = nil
	end


	local function load_settings()
		-- set defaults
		frame_mode = 1
		set_default_hotkey()

		-- try to open configuration file
		local cfgname = get_settings_path() .. '/plugin.cfg'
		local cfgfile = io.open(cfgname, 'r')
		if not cfgfile then
			return -- probably harmless, configuration just doesn't exist yet
		end

		-- parse settings as JSON
		local json = require('json')
		local settings = json.parse(cfgfile:read('a'))
		cfgfile:close()
		if not settings then
			emu.print_error(string.format('Error loading timecode recorder settings: error parsing file "%s" as JSON', cfgname))
			return
		end

		-- recover frame mode
		local count_frames = settings.count_frames
		if count_frames ~= nil then
			frame_mode = count_frames and 0 or 1
		end

		-- recover hotkey assignment
		hotkey_cfg = settings.hotkey
		if hotkey_cfg then
			local seq = manager.machine.input:seq_from_tokens(hotkey_cfg)
			if seq then
				hotkey_seq = seq
			end
		end
	end


	local function save_settings()
		local path = get_settings_path()
		local attr = lfs.attributes(path)
		if not attr then
			lfs.mkdir(path)
		elseif attr.mode ~= 'directory' then
			emu.print_error(string.format('Error saving timecode recorder settings: "%s" is not a directory', path))
			return
		end
		local json = require('json')
		local settings = { count_frames = frame_mode == 0 }
		if hotkey_cfg then
			settings.hotkey = hotkey_cfg
		end
		local data = json.stringify(settings, { indent = true })
		local cfgname = path .. '/plugin.cfg'
		local cfgfile = io.open(cfgname, 'w')
		if not cfgfile then
			emu.print_error(string.format('Error saving timecode recorder settings: error opening file "%s" for writing', cfgname))
			return
		end
		cfgfile:write(data)
		cfgfile:close()
	end


	local function process_frame()
		if (not file) or manager.machine.paused then
			return
		end
		if write then
			write = false
			count = count + 1
			show_total = true

			-- time from beginning of playback in milliseconds, HH:MM:SS.fff and frames
			local curtime = manager.machine.time
			local sec_start = curtime.seconds
			local msec_start = (sec_start * 1000) + curtime.msec
			local msec_start_str = string.format('%015d', msec_start)
			local curtime_str = string.format(
					'%02d:%02d:%02d.%03d',
					sec_start // (60 * 60),
					(sec_start // 60) % 60,
					sec_start % 60,
					msec_start % 1000)
			local frame_start_str = string.format('%015d', (frame_mode == 0) and frame_count or (msec_start * 60 // 1000))

			-- elapsed from previous timecode in milliseconds, HH:MM:SS.fff and frames
			local elapsed = curtime - start_time
			local sec_elapsed = elapsed.seconds
			local msec_elapsed = (sec_elapsed * 1000) + elapsed.msec
			local msec_elapsed_str = string.format('%015d', msec_elapsed)
			local elapsed_str = string.format(
					'%02d:%02d:%02d.%03d',
					sec_elapsed // (60 * 60),
					(sec_elapsed // 60) % 60,
					sec_elapsed % 60,
					msec_elapsed % 1000)
			local frame_elapsed_str = string.format('%015d', (frame_mode == 0) and (frame_count - start_frame) or (msec_elapsed * 60 // 1000))

			-- update start of part
			start_frame = frame_count
			start_time = curtime

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
		frame_count = frame_count + 1
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
		if file then
			local pressed = machine.input:seq_pressed(hotkey_seq)
			if (not hotkey_pressed) and pressed then
				write = true
			end
			hotkey_pressed = pressed
		end
	end


	local function start()
		file = nil
		show_counter = false
		show_total = false
		load_settings()

		-- only do timecode recording if we're doing input recording
		local options = manager.machine.options.entries
		local filename = options.record:value()
		if #filename > 0 then
			filename = filename .. '.timecode'
			emu.print_info(string.format('Record input timecode file: %s', filename))
			file = emu.file(options.input_directory:value(), 0x0e) -- FIXME: magic number for flags
			local openerr = file:open(filename)
			if openerr then
				-- TODO: this used to throw a fatal error and log the error description
				emu.print_error('Failed to open file for input timecode recording')
				file = nil
			else
				write = false
				text = ''
				frame_count = 0
				start_frame = 0
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
		save_settings()
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
		elseif index == item_framemode then
			if (event == 'select') or (event == 'left') or (event == 'right') then
				frame_mode = (frame_mode ~= 0) and 0 or 1
				return true
			end
		elseif index == item_hotkey then
			if event == 'select' then
				if not commonui then
					commonui = require('commonui')
				end
				hotkey_poller = commonui.switch_polling_helper()
				return true
			elseif event == 'clear' then
				set_default_hotkey()
				return true
			end
		end
		return false
	end


	local function menu_populate()
		local result = { }
		table.insert(result, { _p('plugin-timecode', 'Timecode Recorder'), '', 'off' })
		table.insert(result, { '---', '', '' })

		local frame_mode_val = (frame_mode > 0) and _p('plugin-timecode', 'Assume 60 Hz') or _p('plugins-timecode', 'Count emulated frames')
		table.insert(result, { _p('plugin-timecode', 'Frame numbers'), frame_mode_val, (frame_mode > 0) and 'l' or 'r' })
		item_framemode = #result

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
