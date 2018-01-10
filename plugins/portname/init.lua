-- license:BSD-3-Clause
-- copyright-holders:Carl
-- use plugin options to save the input port list to a gettext formatted file
-- the file is saved in the ctrlrpath dir
-- use #import <filename.po> to load names from a different file
-- use #set <set1>,<set2>,... to override names for a child set, common names should be listed at the top before any #set tags
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
			filename = emu.romname() .. ".po"
		end
		return filename
	end
	
	emu.register_start(function()
		local reclevel = 0
		local function parse_names(names)
			local orig, rep
			local setfound = true
			reclevel = reclevel + 1
			if reclevel == 3 then
				emu.print_verbose("portname: too many import levels\n")
				return
			end
			names:gsub("[^\n\r]*", function (line)
				if line:find("^#import") then
					local impfile = emu.file(ctrlrpath .. "/portname", "r")
					local ret = impfile:open(line:match("^#import (.*)"))
					if ret then
						emu.print_verbose("portname: import file not found\n")
					else
						parse_names(impfile:read(impfile:size()))
					end
				elseif line:find("^#set") then
					local sets = line:match("^#set (.*)")
					setfound = false
					for s in sets:gmatch("[^,]*") do
						if s == emu.romname() then
							setfound = true
							break
						end
					end
				elseif line:find("^msgid") then
					orig = line:match("^msgid \"(.+)\"")
				elseif line:find("^msgstr") then
					rep = line:match("^msgstr \"(.+)\"")
					if rep and rep ~= "" and setfound then
						rep = rep:gsub("\\(.)", "%1")
						orig = orig:gsub("\\(.)", "%1")
						for pname, port in pairs(manager:machine():ioport().ports) do
							if port.fields[orig] then
								port.fields[orig].live.name = rep
							end
						end
					end
				end
				return line
			end)
			reclevel = reclevel - 1
		end
		local file = emu.file(ctrlrpath .. "/portname", "r")
		local ret = file:open(get_filename())
		if ret then
			ret = file:open(get_filename(true))
			if ret then
				ret = file:open(manager:machine():system().parent .. ".po")
				if ret then
					return
				end
			end
		end
		parse_names(file:read(file:size()))
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
			local function check_path(path)
				local attr = lfs.attributes(path)
				if not attr then
					lfs.mkdir(path)
					if not lfs.attributes(path) then
						manager:machine():popmessage(_("Failed to save input name file"))
						emu.print_verbose("portname: unable to create path " .. path .. "\n")
						return false
				end
				elseif attr.mode ~= "directory" then
					manager:machine():popmessage(_("Failed to save input name file"))
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
				manager:machine():popmessage(_("Failed to save input name file"))
				file:close()
				return false
			end
			file = io.open(ctrlrpath .. "/portname/" .. filename, "w")
			for def, custom in pairs(fields) do
				def = def:gsub("[\\\"]", function (s) return "\\" .. s end)
				custom = custom:gsub("[\\\"]", function (s) return "\\" .. s end)
				file:write("msgid \"" .. def .."\"\nmsgstr \"" .. custom .. "\"\n\n")
			end
			file:close()
			manager:machine():popmessage(_("Input port name file saved to ") .. ctrlrpath .. "/portname/" .. filename)
		end
		return false
	end

	emu.register_menu(menu_callback, menu_populate, _("Input ports"))
end

return exports
