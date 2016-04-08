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
--   "space": {
--     "varname": {
--       "tag": "tag",
--       "type": "program|data|io"
--     },
--     ...
--   },
--   "screen": {
--     "varname": "tag"
--     },
--     ...
--   "region": {
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

local exports = {}
exports.name = "cheat"
exports.version = "0.0.1"
exports.description = "Cheat plugin"
exports.license = "The BSD 3-Clause License"
exports.author = { name = "Carl" }

local cheat = exports

function cheat.startplugin()
	local cheats = {}
	local output = {}
	local line = 0

	local function load_cheats()
		local filename = emu.romname()
		local json = require("json")
		local file = emu.file(manager:machine():options().entries.cheatpath:value():gsub("([^;]+)", "%1;%1/cheat"), 1)

		if emu.softname() ~= "" then
			for name, image in pairs(manager:machine().images) do
				if image:exists() and image:software_list_name() ~= "" then
					filename = image:software_list_name() .. "/" .. emu.softname()
				end
			end
		end

		if file:open(filename .. ".json") then
			local xml = require("cheat/xml_conv")

			if file:open(filename .. ".xml") then
				return {}
			end
			return xml.conv_cheat(file:read(file:size()))
		end

		return json.parse(file:read(file:size()))
	end

	local function draw_text(screen, x, y, color, form, ...)
		local str = form:format(...)
		if y == "auto" then
			y = line
			line = line + 1
		end
		if not screen then
			print("draw_text: invalid screen")
			return
		end
		if type(x) == "string" then
			y = y * manager:machine():ui():get_line_height()
		end
		output[#output + 1] = { type = "text", scr = screen, x = x, y = y, str = str, color = color }
	end

	local function draw_line(screen, x1, y1, x2, y2, color)
		if not screen then
			print("draw_line: invalid screen")
			return
		end
		output[#output + 1] = { type = "line", scr = screen, x1 = x1, x2 = x2, y1 = y1, y2 = y2, color = color }
	end

	local function draw_box(screen, x1, y1, x2, y2, bgcolor, linecolor)
		if not screen then
			print("draw_box: invalid screen")
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

	local function parse_cheat(cheat)
		cheat.cheat_env = { draw_text = draw_text,
				    draw_line = draw_line,
				    draw_box = draw_box,
				    tobcd = tobcd,
				    frombcd = frombcd,
				    pairs = pairs }
		cheat.enabled = false
		-- verify scripts are valid first
		if not cheat.script then
			return
		end
		for name, script in pairs(cheat.script) do
			script = load(script, cheat.desc .. name, "t", cheat.cheat_env)
			if not script then
				print("error loading cheat script: " .. cheat.desc)
				cheat = { desc = cheat.desc .. "error" }
				return
			end
			cheat.script[name] = script
		end
		-- initialize temp[0-9] for backward compatbility reasons
		for i = 0, 9 do
			cheat.cheat_env["temp" .. i] = 0
		end
		if cheat.space then
			for name, space in pairs(cheat.space) do
				local cpu, mem
				cpu = manager:machine().devices[space.tag]
				if not cpu then
					print("error loading cheat script: " .. cheat.desc)
					cheat = { desc = cheat.desc .. "error" }
					return
				end
				if space.type then
					mem = cpu.spaces[space.type]
				else
					mem = cpu.spaces["program"]
				end
				if not mem then
					print("error loading cheat script: " .. cheat.desc)
					cheat = { desc = cheat.desc .. "error" }
					return
				end
				cheat.cheat_env[name] = mem
			end
		end
		if cheat.screen then
			for name, screen in pairs(cheat.screen) do
				local scr
				scr = manager:machine().screens[screen]
				if not scr then
					local tag
					tag, scr = next(manager:machine().screens) -- get any screen
				end
				cheat.cheat_env[name] = scr
			end
		end
		if cheat.region then
			for name, region in pairs(cheat.region) do
				local mem 
				mem = manager:machine():memory().regions[region]
				if not mem then
					print("error loading cheat script: " .. cheat.desc)
					cheat = nil
					return
				end
				cheat.cheat_env[name] = mem
			end
		end
		local param = cheat.parameter
		if not param then
			return
		end
		if not param.min then
			param.min = 0
		else
			param.min = tonumber(param.min)
		end
		if not param.max then
			param.max = #param.item
		else
			param.max = tonumber(param.max)
		end
		if not param.step then
			param.step = 1
		else
			param.step = tonumber(param.step)
		end
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

	local function menu_populate()
		local menu = {}
		for num, cheat in pairs(cheats) do
			menu[num] = {}
			menu[num][1] = cheat.desc
			if not cheat.parameter then
				if not cheat.script then
					if cheat.desc == "" then
						menu[num][1] = "---"
					end
					menu[num][2] = ""
					menu[num][3] = 32 -- MENU_FLAG_DISABLE
				elseif not cheat.script.run and not cheat.script.off then
					menu[num][2] = "Set"
					menu[num][3] = 0
				else
					if cheat.enabled then
						menu[num][2] = "On"
						menu[num][3] = 1 -- MENU_FLAG_LEFT_ARROW
					else
						menu[num][2] = "Off"
						menu[num][3] = 2 -- MENU_FLAG_RIGHT_ARROW
					end
				end
			else
				if cheat.parameter.index == 0 then
					if not cheat.script.run and not cheat.script.off then
						menu[num][2] = "Set"
					else
						menu[num][2] = "Off"
					end
					menu[num][3] = 2
				else
					if cheat.parameter.item then
						menu[num][2] = cheat.parameter.item[cheat.parameter.index].text
					else
						menu[num][2] = cheat.parameter.value
					end
					menu[num][3] = 1
					if cheat.parameter.index < cheat.parameter.last then
						menu[num][3] = 3
					end
				end
			end
		end
		return menu
	end

	local function menu_callback(index, event)
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

		local cheat = cheats[index]
		if not cheat then
			return false
		end
		if event == "up" or event == "down" or event == "comment" then
			if cheat.comment then
				manager:machine():popmessage("Cheat Comment:\n" .. cheat.comment)
			end
		elseif event == "left" then
			if cheat.parameter then
				local param = cheat.parameter
				if param.index == 1 then
					param.index = 0
					cheat.enabled = false
					cheat.cheat_env.param = param.min
					if cheat.script.off then
						cheat.script.off()
					end
					return true
				elseif param.index == 0 then
					return false
				end
				param.index = param.index - 1
				param_calc(param)
				cheat.cheat_env.param = param.value
				if cheat.script.change and (cheat.script.run or cheat.script.off) then
					cheat.script.change()
				end
				return true
			else
				if not cheat.script.run and not cheat.script.off then
					return false
				end
				if cheat.enabled then
					cheat.enabled = false
					if cheat.script.off then
						cheat.script.off()
					end
					return true
				end
				return false
			end
		elseif event == "right" then
			if cheat.parameter then
				local param = cheat.parameter
				if param.index == 0 then
					cheat.enabled = true
					if cheat.script.on then
						cheat.script.on()
					end
				elseif param.index == param.last then
					return false
				end
				param.index = param.index + 1
				param_calc(param)
				cheat.cheat_env.param = param.value
				if cheat.script.change and (cheat.script.run or cheat.script.off) then
					cheat.script.change()
				end
				return true
			else
				if not cheat.script.run and not cheat.script.off then
					return false
				end
				if not cheat.enabled then
					cheat.enabled = true
					if cheat.script.on then
						cheat.script.on()
					end
					return true
				end
				return false
			end
		elseif event == "select" then
			if not cheat.script.run and not cheat.script.off then
				if cheat.parameter and cheat.script.change and cheat.parameter.index ~= 0 then
					param_calc(cheat.parameter)
					cheat.cheat_env.param = cheat.parameter.value
					cheat.script.change()
					local subtext
					if cheat.parameter.item then
						subtext = cheat.parameter.item[cheat.parameter.index]
					else
						subtext = cheat.parameter.value
					end
					manager:machine():popmessage("Activated: " .. cheat.desc .. " = " .. subtext)
				elseif not cheat.parameter and cheat.script.on then
					cheat.script.on()
					manager:machine():popmessage("Activated: " .. cheat.desc)
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
			  end, "Cheat")

	emu.register_start(function()
		cheats = load_cheats()
		for num, cheat in pairs(cheats) do
			parse_cheat(cheat)
		end
	end)

	emu.register_frame(function()
		for num, cheat in pairs(cheats) do
			if cheat.enabled and cheat.script.run then
				cheat.script.run()
			end
		end
	end)

	emu.register_frame_done(function()
		line = 0
		for num, draw in pairs(output) do
			if draw.type == "text" then
				if not draw.color then
					draw.scr:draw_text(draw.x, draw.y, draw.str)
				else
					draw.scr:draw_text(draw.x, draw.y, draw.str, draw.color)
				end
			elseif draw.type == "line" then
				draw.scr:draw_line(draw.x1, draw.x2, draw.y1, draw.y2, draw.color)
			elseif draw.type == "box" then
				draw.scr:draw_box(draw.x1, draw.x2, draw.y1, draw.y2, draw.bgcolor, draw.linecolor)
			end
		end
		output = {}
	end)
end

return exports
