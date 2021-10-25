-- license:BSD-3-Clause
-- copyright-holders:Carl
-- data files are json files named <romname>.json
-- {
--   "import":"<import filename>"
--   "ports":{
--     "<ioport name>":{
--       "labels":{
--         "<field mask>":{
--           "player":<int player number>,
--           "name":"<field label>"
--         }
--     },{
--       ...
--     }
--   }
-- }
-- any additional metadata can be included for other usage
-- and will be ignored
local exports = {}
exports.name = "portname"
exports.version = "0.0.1"
exports.description = "IOPort name/translation plugin"
exports.license = "BSD-3-Clause"
exports.author = { name = "Carl" }

local portname = exports

function portname.startplugin()
	local json = require("json")
	local ctrlrpath = emu.subst_env(manager.options.entries.ctrlrpath:value():match("([^;]+)"))
	local function get_filename(nosoft)
		local filename
		if emu.softname() ~= "" and not nosoft then
			local soft = emu.softname():match("([^:]*)$")
			filename = emu.romname() .. "_" .. soft .. ".json"
		else
			filename = emu.romname() .. ".json"
		end
		return filename
	end

	local function parse_names(ctable, depth)
		if depth >= 5 then
			emu.print_error("portname: max import depth exceeded\n")
			return
		end
		if ctable.import then
			local file = emu.file(ctrlrpath .. "/portname", "r")
			local ret = file:open(ctable.import)
			if not ret then
				parse_names(json.parse(file:read(file:size())), depth + 1)
			end
		end
		if not ctable.ports then
			return
		end
		for pname, port in pairs(ctable.ports) do
			local ioport = manager.machine.ioport.ports[pname]
			if ioport then
				for mask, label in pairs(port.labels) do
					for num3, field in pairs(ioport.fields) do
						local nummask = tonumber(mask, 16)
						if nummask == field.mask and label.player == field.player then
							field.live.name = label.name
						end
					end
				end
			end
		end
	end

	emu.register_start(function()
		local file = emu.file(ctrlrpath .. "/portname", "r")
		local ret = file:open(get_filename())
		if ret then
			if emu.softname() ~= "" then
				local parent
				for tag, image in pairs(manager.machine.images) do
					parent = image.software_parent
					if parent then
						break
					end
				end
				if parent then
					ret = file:open(emu.romname() .. "_" .. parent:match("([^:]*)$")  .. ".json")
				end
			end
			if ret then
				ret = file:open(get_filename(true))
				if ret then
					ret = file:open(manager.machine.system.parent .. ".json")
					if ret then
						return
					end
				end
			end
		end
		parse_names(json.parse(file:read(file:size())), 0)
	end)

	local function menu_populate()
		return {{ _("Save input names to file"), "", 0 }}
	end

	local function menu_callback(index, event)
		if event == "select" then
			local ports = {}
			for pname, port in pairs(manager.machine.ioport.ports) do
				local labels = {}
				local sort = {}
				for fname, field in pairs(port.fields) do
					local mask = string.format("%x", field.mask)
					if not labels[mask] then
						sort[#sort + 1] = mask
						labels[mask] = { name = fname, player = field.player }
						setmetatable(labels[mask], { __tojson = function(v,s)
							local label = { name = v.name, player = v.player }
							setmetatable(label, { __jsonorder = { "player", "name" }})
							return json.stringify(label) end })
					end
				end
				if #sort > 0 then
					table.sort(sort, function(i, j) return tonumber(i, 16) < tonumber(j, 16) end)
					setmetatable(labels, { __jsonorder = sort })
					ports[pname] = { labels = labels }
				end
			end
			local function check_path(path)
				local attr = lfs.attributes(path)
				if not attr then
					lfs.mkdir(path)
					if not lfs.attributes(path) then
						manager.machine:popmessage(_("Failed to save input name file"))
						emu.print_verbose("portname: unable to create path " .. path .. "\n")
						return false
				end
				elseif attr.mode ~= "directory" then
					manager.machine:popmessage(_("Failed to save input name file"))
					emu.print_verbose("portname: path exists but isn't directory " .. path .. "\n")
					return false
				end
				return true
			end
			if not check_path(ctrlrpath) then
				return false
			end
			if not check_path(ctrlrpath .. "/portname") then
				return false
			end
			local filename = get_filename()
			local file = io.open(ctrlrpath .. "/portname/" .. filename, "r")
			if file then
				emu.print_verbose("portname: input name file exists " .. filename .. "\n")
				manager.machine:popmessage(_("Failed to save input name file"))
				file:close()
				return false
			end
			file = io.open(ctrlrpath .. "/portname/" .. filename, "w")
			local ctable = { romname = emu.romname(), ports = ports }
			if emu.softname() ~= "" then
				ctable.softname = emu.softname()
			end
			setmetatable(ctable, { __jsonorder = { "romname", "softname", "ports" }})
			file:write(json.stringify(ctable, { indent = true }))
			file:close()
			manager.machine:popmessage(string.format(_("Input port name file saved to %s"), ctrlrpath .. "/portname/" .. filename))
		end
		return false
	end

	emu.register_menu(menu_callback, menu_populate, _("Input ports"))
end

return exports
