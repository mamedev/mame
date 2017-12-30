-- license:BSD-3-Clause
-- copyright-holders:Carl
-- use plugin options to save the input port list to a gettext formatted file
-- the file is saved in the ctrlrpath dir
local exports = {}
exports.name = "portname"
exports.version = "0.0.1"
exports.description = "IOPort name/translation plugin"
exports.license = "The BSD 3-Clause License"
exports.author = { name = "Carl" }

local portname = exports

function portname.startplugin()
	local ctrlrpath = lfs.env_replace(manager:options().entries.ctrlrpath:value():match("([^;]+)"))
	local function get_filename(nosoft)
		local filename
		if emu.softname() ~= "" and not nosoft then
			filename = emu.romname() .. "_" .. emu.softname() .. ".po"
		else
			filename =  emu.romname() .. ".po"
		end
		return filename
	end
	
	emu.register_start(function()
		local file = io.open(ctrlrpath .. "/" .. get_filename(), "r")
		if not file then
			file = io.open(ctrlrpath .. "/" .. get_filename(true), "r")
			if not file then
				return
			end
		end
		local orig, rep
		for line in file:lines() do
			if line:find("^msgid") then
				orig = line:match("^msgid \"(.+)\"")
			elseif line:find("^msgstr") then
				rep = line:match("^msgstr \"(.+)\"")
				if rep and rep ~= "" then
					rep = rep:gsub("\\(.)", "%1")
					orig = orig:gsub("\\(.)", "%1")
					for pname, port in pairs(manager:machine():ioport().ports) do
						if port.fields[orig] then
							port.fields[orig].live.name = rep
						end
					end
				end
			end
		end
		file:close()
	end)

	local function menu_populate()
		return {{ _("Save input names to file"), "", 0 }}
	end

	local function menu_callback(index, event)
		if event == "select" then
			local fields = {}
			for pname, port in pairs(manager:machine():ioport().ports) do
				for fname, field in pairs(port.fields) do
					local dname = field.default_name
					if not fields[dname] then
						fields[dname] = ""
					end
					if fname ~= dname then
						fields[dname] = fname
					end
				end
			end
			local attr = lfs.attributes(ctrlrpath)
			if not attr then
				lfs.mkdir(ctrlrpath)
				if not lfs.attributes(ctrlrpath) then
					manager:machine():popmessage(_("Failed to save input name file"))
					emu.print_verbose("portname: unable to create ctrlrpath " .. ctrlrpath .. "\n")
					return false
				end
			elseif attr.mode ~= "directory" then
				manager:machine():popmessage(_("Failed to save input name file"))
				emu.print_verbose("portname: ctrlrpath exists but isn't directory " .. ctrlrpath .. "\n")
				return false
			end
			local filename = get_filename()
			local file = io.open(ctrlrpath .. "/" .. filename, "r")
			if file then
				emu.print_verbose("portname: input name file exists " .. filename .. "\n")
				manager:machine():popmessage(_("Failed to save input name file"))
				file:close()
				return false
			end
			file = io.open(ctrlrpath .. "/" .. filename, "w")
			for def, custom in pairs(fields) do
				def = def:gsub("[\\\"]", function (s) return "\\" .. s end)
				custom = custom:gsub("[\\\"]", function (s) return "\\" .. s end)
				file:write("msgid \"" .. def .."\"\nmsgstr \"" .. custom .. "\"\n\n")
			end
			file:close()
			manager:machine():popmessage(_("Input port name file saved to ") .. ctrlrpath .. "/" .. filename)
		end
		return false
	end

	emu.register_menu(menu_callback, menu_populate, _("Input ports"))
end

return exports
