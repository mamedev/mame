-- license:BSD-3-Clause
-- copyright-holders:Carl
--
-- json cheat file format
-- [{
--   "desc": "text",
--   "parameter": {
--     "min": "minval(0)",
--     "max": "maxval(numitems)",
--     "step": "stepval(1)",
--     "item" [{
--       "value": "itemval(index*stepval+minval)",
--       "text": "text"
--     },
--     ... ]
--   },
--   "cpu": {
--      "varname": "tag"
--      ...
--   }
--   "space": {
--     "varname": {
--       "tag": "tag",
--       "type": "program|data|io"
--     },
--     ...
--   },
--   "screen": {
--     "varname": "tag",
--     ...
--   },
--   "region": {
--     "varname": "tag",
--     ...
--   },
--   "ram": {
--     "varname": "tag",
--     ...
--   },
--   "share": {
--     "varname": "tag",
--     ...
--   },
--   "script": {
--     "on|off|run|change": "script",
--      ...
--   },
--   "comment": "text"
-- },
-- ... ]
--
-- Scripts are lua scripts with a limited api. Most library functions are unavailable.
-- Like the XML cheats, param is the current parameter value and variables are shared between scripts within a cheat
-- Differences from XML cheats:
-- - actions are only one line which include the entire script
-- - "condexpr" is replaced with lua control statements (if-then-else-end)
-- - variables are only limited by the limits of the lua interperter, you can have strings and tables
-- - the address spaces in the "space" blocks are accessible to the script if included,
--      same with regions (the "m" space in debug expr)
-- - frame is replaced by screen:frame_number() so if you use frame a screen needs to be in the device section
-- - output is a function and argindex isn't supported, output args need to be explicit and a screen device
--      must be provided
-- - cpu is only used for break and watch points, if it is defined and the debugger is not enabled (-debugger none is enough)
--  it will disable the cheat only if a point is set, check var for nil first
-- - watch points require the address space that you want to set the watch on, wptype is "r"-read, "w"-write or "rw"-both

local exports = {}
exports.name = "cheat"
exports.version = "0.0.1"
exports.description = "Cheat plugin"
exports.license = "BSD-3-Clause"
exports.author = { name = "Carl" }

local cheat = exports

function cheat.set_folder(path)
	cheat.path = path
end

function cheat.startplugin()
	local cheats = {}
	local output = {}
	local line = 0
	local start_time = 0
	local stop = true
	local cheatname = ""
	local consolelog = nil
	local consolelast = 0
	local perodicset = false
	local watches = {}
	local breaks = {}
	local inputs = {}

	local function load_cheats()
		local filename = emu.romname()
		local newcheats = {}
		local file = emu.file(manager.machine.options.entries.cheatpath:value():gsub("([^;]+)", "%1;%1/cheat") , 1)

		for name, image in pairs(manager.machine.images) do
			if image.exists and image.software_list_name ~= "" then
				filename = image.software_list_name .. "/" .. image.filename
			end
		end

		cheatname = filename
		local function add(addcheats)
			if not next(newcheats) then
				newcheats = addcheats
			else
				for num, cheat in pairs(addcheats) do
					newcheats[#newcheats + 1] = cheat
				end
			end
		end
		for scrfile in lfs.dir(cheat.path) do
			local name = string.match(scrfile, "^(cheat_.*).lua$")
			if name then
				local conv = require("cheat/" .. name)
				if conv then
					local ret = file:open(conv.filename(filename))
					while not ret do
						add(conv.conv_cheat(file:read(file:size())))
						ret = file:open_next()
					end
				end
			end
		end
		return newcheats
	end

	local function load_hotkeys()
		local json = require("json")
		local file = io.open(emu.subst_env(manager.machine.options.entries.cheatpath:value():match("([^;]+)")) .. "/" .. cheatname .. "_hotkeys.json", "r")
		if not file then
			return
		end
		local hotkeys = json.parse(file:read("a"))
		for num, val in ipairs(hotkeys) do
			for num, cheat in pairs(cheats) do
				if val.desc == cheat.desc then
					cheat.hotkeys = {pressed = false, keys = manager.machine.input:seq_from_tokens(val.keys)}
				end
			end
		end
	end

	local function save_hotkeys()
		local hotkeys = {}
		for num, cheat in ipairs(cheats) do
			if cheat.hotkeys then
				local hotkey = {desc = cheat.desc, keys = manager.machine.input:seq_to_tokens(cheat.hotkeys.keys)}
				if hotkey.keys ~= "" then
					hotkeys[#hotkeys + 1] = hotkey
				end
			end
		end
		if #hotkeys > 0 then
			local json = require("json")
			local path = emu.subst_env(manager.machine.options.entries.cheatpath:value():match("([^;]+)"))
			local attr = lfs.attributes(path)
			if not attr then
				lfs.mkdir(path)
			elseif attr.mode ~= "directory" then -- uhhh?
				return
			end
			if cheatname:find("/", 1, true) then
				local softpath = path .. "/" .. cheatname:match("([^/]+)")
				local attr = lfs.attributes(softpath)
				if not attr then
					lfs.mkdir(softpath)
				elseif attr.mode ~= "directory" then -- uhhh?
					return
				end
			end

			local file = io.open(path .. "/" .. cheatname .. "_hotkeys.json", "w+")
			if file then
				file:write(json.stringify(hotkeys, {indent = true}))
				file:close()
			end
		end
	end

	local function cheat_error(cheat, msg)
		emu.print_error("error cheat script error: \"" .. cheat.desc .. "\" " .. msg)
		cheat.desc = cheat.desc .. " error"
		cheat.script = nil
		cheat.enabled = nil
		return
	end

	local function run_if(cheat, func)
		if func then
			local stat, err = pcall(func)
			if not stat then
				cheat_error(cheat, err)
			end
			return func
		end
		return false
	end

	local function draw_text(screen, x, y, color, form, ...)
		local str = form:format(...)
		if y == "auto" then
			y = line
			line = line + 1
		end
		if not screen then
			emu.print_verbose("draw_text: invalid screen")
			return
		end
		if type(x) == "string" then
			y = y * mame_manager.ui.line_height
		end
		output[#output + 1] = { type = "text", scr = screen, x = x, y = y, str = str, color = color }
	end

	local function draw_line(screen, x1, y1, x2, y2, color)
		if not screen then
			emu.print_verbose("draw_line: invalid screen")
			return
		end
		output[#output + 1] = { type = "line", scr = screen, x1 = x1, x2 = x2, y1 = y1, y2 = y2, color = color }
	end

	local function draw_box(screen, x1, y1, x2, y2, bgcolor, linecolor)
		if not screen then
			emu.print_verbose("draw_box: invalid screen")
			return
		end
		output[#output + 1] = { type = "box", scr = screen, x1 = x1, x2 = x2, y1 = y1, y2 = y2, bgcolor = bgcolor, linecolor = linecolor }
	end

	local function tobcd(val)
		local result = 0
		local shift = 0
		while val ~= 0 do
			result = result + ((val % 10) << shift)
			val = val / 10
			shift = shift + 4
		end
		return result
	end

	local function frombcd(val)
		local result = 0
		local mul = 1
		while val ~= 0 do
			result = result + ((val % 16) * mul)
			val = val >> 4
			mul = mul * 10
		end
		return result
	end

	local function time()
		return emu.time() - start_time
	end

	local function periodiccb()
		local last = consolelast
		local msg = consolelog[#consolelog]
		consolelast = #consolelog
		if #consolelog > last and msg:find("Stopped at", 1, true) then
			local point = tonumber(msg:match("Stopped at breakpoint ([0-9]+)"))
			if not point then
				point = tonumber(msg:match("Stopped at watchpoint ([0-9]+"))
				if not point then
					return -- ??
				end
				local wp = watches[point]
				if wp then
					run_if(wp.cheat, wp.func)
					-- go in case a debugger other than "none" is enabled
					-- don't use an b/wpset action because that will supress the b/wp index
					manager.machine.debugger.execution_state = "run"
				end
			else
				local bp = breaks[point]
				if bp then
					run_if(bp.cheat, bp.func)
					manager.machine.debugger.execution_state = "run"
				end
			end
		end
	end

	local function bpset(cheat, dev, addr, func)
		if cheat.is_oneshot then
			error("bpset not permitted in oneshot cheat")
			return
		end
		local idx = dev.debug:bpset(addr)
		breaks[idx] = {cheat = cheat, func = func, dev = dev}
	end

	local function wpset(cheat, dev, space, wptype, addr, len, func)
		if cheat.is_oneshot then
			error("wpset not permitted in oneshot cheat")
			return
		end
		if not space.name then
			error("bad space in wpset")
			return
		end
		local idx = dev.debug:wpset(space, wptype, addr, len)
		watches[idx] = {cheat = cheat, func = func, dev = dev}
	end

	local function bwpclr(cheat)
		if not manager.machine.debugger then
			return
		end
		for num, bp in pairs(breaks) do
			if cheat == bp.cheat then
				bp.dev.debug:bpclr(num)
			end
		end
		for num, wp in pairs(watches) do
			if cheat == wp.cheat then
				wp.dev.debug:wpclr(num)
			end
		end
	end

	local function input_trans(list)
		local xlate = { start = {}, stop = {}, last = 0 }
		local function errout(port, field)
			cheat:set_enabled(false)
			error(port .. field .. " not found")
			return
		end

		for num, entry in ipairs(list) do
			if entry.port:sub(1, 1) ~= ":" then
				entry.port = ":" .. entry.port
			end
			local port = manager.machine.ioport.ports[entry.port]
			if not port then
				errout(entry.port, entry.field)
			end
			local field = port.fields[entry.field]
			if not field then
				errout(entry.port, entry.field)
			end
			if not xlate.start[entry.start] then
				xlate.start[entry.start] = {}
			end
			if not xlate.stop[entry.stop] then
				xlate.stop[entry.stop] = {}
			end
			local start = xlate.start[entry.start]
			local stop = xlate.stop[entry.stop]
			local ent = { port = port, field = field }
			stop[#stop + 1] = ent
			start[#start + 1] = ent
			if entry.stop > xlate.last then
				xlate.last = entry.stop
			end
		end
		return xlate
	end

	local function input_run(cheat, list)
		if not cheat.is_oneshot then
			cheat.enabled = false
			error("input_run only allowed in one shot cheats")
			return
		end
		local _, screen = next(manager.machine.screens)
		list.begin = screen:frame_number()
		inputs[#inputs + 1] = list
	end

	local function param_calc(param)
		if param.item then
			if not param.item[param.index] then -- uh oh
				param.index = 1
			end
			param.value = param.item[param.index].value
			return
		end
		param.value = param.min + (param.step * (param.index - 1))
		if param.value > param.max then
			param.value = param.max
		end
	end

	-- return is current state, ui change
	local function set_enabled(cheat, state)
		if cheat.is_oneshot then
			if state then
				if cheat.parameter and cheat.script.change and cheat.parameter.index ~= 0 then
					param_calc(cheat.parameter)
					cheat.cheat_env.param = cheat.parameter.value
					cheat.script.change()
				elseif not cheat.parameter and cheat.script.on then
					cheat.script.on()
				end
			end
			return false, false
		end
		if cheat.enabled == state then
			return state, false
		end
		if not state then
			cheat.enabled = false
			run_if(cheat, cheat.script.off)
			bwpclr(cheat)
		else
			cheat.enabled = true
			run_if(cheat, cheat.script.on)
		end
		return state, true
	end

	-- return is current index, ui change
	local function set_index(cheat, index)
		local param = cheat.parameter
		local oldindex = param.index
		if (index < 0) or (index > param.last) or (param.index == index) then
			return param.index, false
		end
		param.index = index
		if index == 0 then
			cheat.cheat_env.param = param.min
			cheat:set_enabled(false)
		else
			if oldindex == 0 then
				cheat:set_enabled(true)
			end
			param_calc(param)
			cheat.cheat_env.param = param.value
			if not cheat.is_oneshot then
				run_if(cheat, cheat.script.change)
			end
		end
		return index, true
	end

	local function parse_cheat(cheat)
		cheat.cheat_env = { draw_text = draw_text,
					draw_line = draw_line,
					draw_box = draw_box,
					tobcd = tobcd,
					frombcd = frombcd,
					pairs = pairs,
					ipairs = ipairs,
					outputs = manager.machine.output,
					time = time,
					input_trans = input_trans,
					input_run = function(list) input_run(cheat, list) end,
					os = { time = os.time, date = os.date, difftime = os.difftime },
					table =
					{ insert = table.insert,
						  remove = table.remove } }
		cheat.enabled = false
		cheat.set_enabled = set_enabled;
		cheat.get_enabled = function(cheat) return cheat.enabled end
		cheat.is_oneshot = cheat.script and not cheat.script.run and not cheat.script.off

		-- verify scripts are valid first
		if not cheat.script then
			return
		end
		for name, script in pairs(cheat.script) do
			script, err = load(script, cheat.desc .. name, "t", cheat.cheat_env)
			if not script then
				cheat_error(cheat, err)
				return
			end
			cheat.script[name] = script
		end
		-- initialize temp[0-9] for backward compatbility reasons
		for i = 0, 9 do
			cheat.cheat_env["temp" .. i] = 0
		end
		if cheat.cpu then
			cheat.cpudev = {}
			for name, tag in pairs(cheat.cpu) do
				if manager.machine.debugger then
					local dev = manager.machine.devices[tag]
					if not dev or not dev.debug then
						cheat_error(cheat, "missing or invalid device " .. tag)
						return
					end
					cheat.cheat_env[name] = {
						bpset = function(addr, func) bpset(cheat, dev, addr, func) end,
						wpset = function(space, wptype, addr, len, func) wpset(cheat, dev, space, wptype, addr, len, func) end,
						regs = dev.state }
					cheat.bp = {}
					cheat.wp = {}
					if not periodicset then
						emu.register_periodic(periodic_cb)
						periodicset = true
					end
				end
			end
		end
		if cheat.space then
			for name, space in pairs(cheat.space) do
				local cpu, mem
				cpu = manager.machine.devices[space.tag]
				if not cpu then
					cheat_error(cheat, "missing device " .. space.tag)
					return
				end
				if space.type then
					mem = cpu.spaces[space.type]
				else
					space.type = "program"
					mem = cpu.spaces["program"]
				end
				if not mem then
					cheat_error(cheat, "missing space " .. space.type)
					return
				end
				cheat.cheat_env[name] = mem
			end
		end
		if cheat.screen then
			for name, screen in pairs(cheat.screen) do
				local scr = manager.machine.screens[screen]
				if not scr then
					local tag
					local nxt, coll = manager.machine.screens:pairs()
					tag, scr = nxt(coll) -- get any screen
				end
				cheat.cheat_env[name] = scr
			end
		end
		if cheat.region then
			for name, region in pairs(cheat.region) do
				local mem = manager.machine.memory.regions[region]
				if not mem then
					cheat_error(cheat, "missing region " .. region)
					return
				end
				cheat.cheat_env[name] = mem
			end
		end
		if cheat.ram then
			for name, tag in pairs(cheat.ram) do
				local ram = manager.machine.devices[tag]
				if not ram then
					cheat_error(cheat, "missing ram device " .. tag)
					return
				end
				cheat.cheat_env[name] = emu.item(ram.items["0/m_pointer"])
			end
		end
		if cheat.share then
			for name, tag in pairs(cheat.share) do
				local share = manager.machine.memory.shares[tag]
				if not share then
					cheat_error(cheat, "missing share " .. share)
					return
				end
				cheat.cheat_env[name] = share
			end
		end
		local param = cheat.parameter
		if not param then
			return
		end
		cheat.set_index = set_index;
		cheat.set_value = function(cheat, value)
			local idx = ((value - cheat.parameter.min) / cheat.parameter.step) + 1
			local chg = false
			if math.integer(idx) == idx then
				idx, chg = cheat:set_index(idx)
			end
			return cheat.parameter.value, chg
		end
		cheat.get_index = function(cheat) return cheat.parameter.index end
		cheat.get_value = function(cheat) return cheat.parameter.value end
		param.min = tonumber(param.min) or 0
		param.max = tonumber(param.max) or #param.item
		param.step = tonumber(param.step) or 1
		if param.item then
			for count, item in pairs(param.item) do
				if not item.value then
					item.value = (count * param.step) + param.min
				else
				item.value = tonumber(item.value)
				end
			end
			param.last = #param.item
		else
			param.last = ((param.max - param.min) / param.step) + 1
		end
		param.index = 0
		param.value = param.min
		cheat.cheat_env.param = param.min
	end

	local hotkeymenu = false
	local hotkeylist = {}
	local commonui
	local poller

	local function menu_populate()
		local menu = {}
		if hotkeymenu then
			local ioport = manager.machine.ioport
			local input = manager.machine.input

			menu[1] = {_("Select cheat to set hotkey"), "", "off"}
			menu[2] = {string.format(_("Press %s to clear hotkey"), manager.ui:get_general_input_setting(ioport:token_to_input_type("UI_CLEAR"))), "", "off"}
			menu[3] = {"---", "", "off"}
			hotkeylist = {}

			local function hkcbfunc(cheat, event)
				if poller then
					if poller:poll() then
						if poller.sequence then
							cheat.hotkeys = { pressed = false, keys = poller.sequence }
						end
						poller = nil
						return true
					end
				elseif event == "clear" then
					cheat.hotkeys = nil
					return true
				elseif event == "select" then
					if not commonui then
						commonui = require('commonui')
					end
					poller = commonui.switch_polling_helper()
					return true
				end
				return false
			end

			for num, cheat in ipairs(cheats) do
				if cheat.script then
					local setting = cheat.hotkeys and input:seq_name(cheat.hotkeys.keys) or _("None")
					menu[#menu + 1] = {cheat.desc, setting, ""}
					hotkeylist[#hotkeylist + 1] = function(event) return hkcbfunc(cheat, event) end
				end
			end
			menu[#menu + 1] = {"---", "", ""}
			menu[#menu + 1] = {_("Done"), "", ""}
			if poller then
				return poller:overlay(menu)
			else
				return menu
			end
		end
		for num, cheat in ipairs(cheats) do
			menu[num] = {}
			menu[num][1] = cheat.desc
			if not cheat.parameter then
				if not cheat.script then
					if cheat.desc == "" then
						menu[num][1] = "---"
					end
					menu[num][2] = ""
					menu[num][3] = "off"
				elseif cheat.is_oneshot then
					menu[num][2] = _("Set")
					menu[num][3] = 0
				else
					if cheat.enabled then
						menu[num][2] = _("On")
						menu[num][3] = "l"
					else
						menu[num][2] = _("Off")
						menu[num][3] = "r"
					end
				end
			else
				if cheat.parameter.index == 0 then
					if cheat.is_oneshot then
						menu[num][2] = _("Set")
					else
						menu[num][2] = _("Off")
					end
					menu[num][3] = "r"
				else
					if cheat.parameter.item then
						menu[num][2] = cheat.parameter.item[cheat.parameter.index].text
					else
						menu[num][2] = cheat.parameter.value
					end
					menu[num][3] = "l"
					if cheat.parameter.index < cheat.parameter.last then
						menu[num][3] = "lr"
					end
				end
			end
		end
		menu[#menu + 1] = {"---", "", 0}
		menu[#menu + 1] = {_("Set hotkeys"), "", 0}
		menu[#menu + 1] = {_("Reset All"), "", 0}
		menu[#menu + 1] = {_("Reload All"), "", 0}
		return menu
	end

	local function menu_callback(index, event)
		manager.machine:popmessage()
		if hotkeymenu then
			if event == "cancel" then
				hotkeymenu = false
				return true
			else
				index = index - 3
				if index >= 1 and index <= #hotkeylist then
					hotkeylist[index](event)
					return true
				elseif index == #hotkeylist + 2 and event == "select" then
					hotkeymenu = false
					return true
				end
			end
			return false
		end
		if index > #cheats and event == "select" then
			index = index - #cheats
			if index == 2 then
				hotkeymenu = true
			elseif index == 3 then
				for num, cheat in pairs(cheats) do
					cheat:set_enabled(false)
					if cheat.parameter then
						cheat:set_index(0)
					end
				end
			elseif index == 4 then
				for num, cheat in pairs(cheats) do
					cheat:set_enabled(false)
				end
				cheats = load_cheats()
				for num, cheat in pairs(cheats) do
					parse_cheat(cheat)
				end
				load_hotkeys()
			end
			return true
		end

		local cheat = cheats[index]
		if not cheat then
			return false
		end
		if event == "up" or event == "down" or event == "comment" then
			if cheat.comment then
				manager.machine:popmessage(string.format(_("Cheat Comment:\n%s"), cheat.comment))
			end
		elseif event == "left" then
			if cheat.parameter then
				local idx, chg = cheat:set_index(cheat:get_index() - 1)
				return chg
			else
				if not cheat.is_oneshot then
					local state, chg = cheat:set_enabled(false)
					return chg
				end
				return false
			end
		elseif event == "right" then
			if cheat.parameter then
				local idx, chg = cheat:set_index(cheat:get_index() + 1)
				return chg
			else
				if not cheat.is_oneshot then
					local state, chg = cheat:set_enabled(true)
					return chg
				end
				return false
			end
		elseif event == "select" then
			if cheat.is_oneshot then
				cheat:set_enabled(true)
				if cheat.parameter and cheat.script.change and cheat:get_index() ~= 0 then
					local itemtext
					if cheat.parameter.item then
						itemtext = cheat.parameter.item[cheat.parameter.index].text
					else
						itemtext = cheat.parameter.value
					end
					manager.machine:popmessage(string.format(_("Activated: %s = %s"), cheat.desc, itemtext))
				elseif not cheat.parameter and cheat.script.on then
					manager.machine:popmessage(string.format(_("Activated: %s"), cheat.desc))
				end
			end
		end
		return false
	end

	emu.register_menu(function(index, event)
				return menu_callback(index, event)
			  end,
			  function()
				return menu_populate()
			  end, _("Cheat"))

	emu.register_start(function()
		if not stop then
			return
		end
		stop = false
		start_time = emu.time()
		cheats = load_cheats()
		local json = require("json")
		local file = io.open(manager.machine.options.entries.cheatpath:value():match("([^;]+)") .. "/output.json", "w")
		if file then
			file:write(json.stringify(cheats, {indent = true}))
			file:close()
		end
		for num, cheat in pairs(cheats) do
			parse_cheat(cheat)
		end
		load_hotkeys()
		if manager.machine.debugger then
			consolelog = manager.machine.debugger.consolelog
			consolelast = 0
		end
	end)

	emu.register_stop(function()
		stop = true
		consolelog = nil
		save_hotkeys()
	end)

	emu.register_frame(function()
		if stop then
			return
		end
		for num, cheat in pairs(cheats) do
			if cheat.enabled then
				run_if(cheat, cheat.script.run)
			end
			if cheat.hotkeys and cheat.hotkeys.keys then
				if manager.machine.input:seq_pressed(cheat.hotkeys.keys) then
					if not cheat.hotkeys.pressed then
						if cheat.is_oneshot then
							if not run_if(cheat, cheat.script.change) then
								run_if(cheat, cheat.script.on)
							end
							manager.machine:popmessage(string.format(_("Activated: %s"), cheat.desc))
						elseif not cheat.enabled then
							cheat.enabled = true
							run_if(cheat, cheat.script.on)
							manager.machine:popmessage(string.format(_("Enabled: %s"), cheat.desc))
						else
							cheat.enabled = false
							run_if(cheat, cheat.script.off)
							bwpclr(cheat)
							manager.machine:popmessage(string.format(_("Disabled: %s"), cheat.desc))
						end
					end
					cheat.hotkeys.pressed = true
				else
					cheat.hotkeys.pressed = false
				end
			end
		end
		for num, input in pairs(inputs) do
			local _, screen = next(manager.machine.screens)
			local framenum = screen:frame_number() - input.begin
			local enttab = input.start[framenum]
			if enttab then
				for num, entry in pairs(enttab) do
					entry.field:set_value(1)
				end
			end
			enttab = input.stop[framenum]
			if enttab then
				for num, entry in pairs(enttab) do
					entry.field:set_value(0)

				end
			end
			if framenum >= input.last then
				table.remove(inputs, num)
			end
		end

	end)

	emu.register_frame_done(function()
		if stop then
			return
		end
		line = 0
		for num, draw in pairs(output) do
			if draw.type == "text" then
				if not draw.color then
					draw.scr:draw_text(draw.x, draw.y, draw.str)
				else
					draw.scr:draw_text(draw.x, draw.y, draw.str, draw.color)
				end
			elseif draw.type == "line" then
				draw.scr:draw_line(draw.x1, draw.y1, draw.x2, draw.y2, draw.color)
			elseif draw.type == "box" then
				draw.scr:draw_box(draw.x1, draw.y1, draw.x2, draw.y2, draw.linecolor, draw.bgcolor)
			end
		end
		output = {}
	end)

	local ce = {}

	-- interface to script cheat engine
	function ce.inject(newcheat)
		cheats[#cheats + 1] = newcheat
		parse_cheat(newcheat)
		manager.machine:popmessage(string.format(_("%s added"), newcheat.desc))
	end

	function ce.get(index)
		local cheat = cheats[index]
		if not cheat then
			return nil
		end
		local intf = {
			get_enabled = function() return cheat:get_enabled() end,
			set_enabled = function(status) return cheat:set_enabled(status) end,
			desc = cheat.desc,
			is_oneshot = cheat.is_oneshot,
			comment = cheat.comment,
			get_hotkeys = function() if cheat.hotkeys then return cheat.hotkeys.keys end return nil end,
			set_hotkeys = function(seq) cheat.hotkeys = { pressed = false, keys = manager.machine.input:seq_clean(seq) } end
		}
		if cheat.script then
			intf.script = {}
			if cheat.script.on then intf.script.on = true end
			if cheat.script.off then intf.script.off = true end
			if cheat.script.run then intf.script.run = true end
			if cheat.script.change then intf.script.change = true end
		end

		if cheat.parameter then
			intf.parameter = {}
			intf.get_value = function() return cheat:get_value() end
			intf.set_value = function(value) return cheat:set_value(value) end
			intf.get_index = function() return cheat:get_index() end
			intf.set_index = function(index) return cheat:set_index(index) end
			intf.parameter.min = cheat.parameter.min
			intf.parameter.max = cheat.parameter.max
			intf.parameter.step = cheat.parameter.step
			if cheat.parameter.item then
				intf.parameter.item = {}
				for idx, item in pairs(cheat.parameter.item) do
					intf.parameter.item[idx] = {}
					intf.parameter.item[idx].text = cheat.parameter.item[idx].text
					intf.parameter.item[idx].value = cheat.parameter.item[idx].value
				end
			end
		end
		return intf
	end

	function ce.list()
		local list = {}
		for num, cheat in pairs(cheats) do
			list[num] = cheat.desc
		end
		return list
	end

	_G.emu.plugin.cheat = ce

end

return exports
