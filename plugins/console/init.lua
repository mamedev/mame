-- license:BSD-3-Clause
-- copyright-holders:Carl
local exports = {}
exports.name = "console"
exports.version = "0.0.1"
exports.description = "Console plugin"
exports.license = "The BSD 3-Clause License"
exports.author = { name = "Carl" }

local console = exports

function console.startplugin()
	local conth = emu.thread()
	local started = false
	local ln = require("linenoise")
	local preload = false
	print("    _/      _/    _/_/    _/      _/  _/_/_/_/");
	print("   _/_/  _/_/  _/    _/  _/_/  _/_/  _/       ");
	print("  _/  _/  _/  _/_/_/_/  _/  _/  _/  _/_/_/    ");
	print(" _/      _/  _/    _/  _/      _/  _/         ");
	print("_/      _/  _/    _/  _/      _/  _/_/_/_/    \n");
	print(emu.app_name() .. " " .. emu.app_version(), "\nCopyright (C) Nicola Salmoria and the MAME team\n");
	print(_VERSION, "\nCopyright (C) Lua.org, PUC-Rio\n");
	-- linenoise isn't thread safe but that means history can handled here
	-- that also means that bad things will happen if anything outside lua tries to use it
	-- especially the completion callback
	ln.historysetmaxlen(10)
	local scr = [[
local ln = require('linenoise')
ln.setcompletion(function(c, str)
	status = str
	yield()
	status:gsub('[^,]*', function(s) if s ~= '' then ln.addcompletion(c, s) end end)
end)
return ln.linenoise('\x1b[1;36m[MAME]\x1b[0m> ')
]]

	local function find_unmatch(str, openpar, pair)
		local done = false
		if not str:match(openpar) then
			return str
		end
		local tmp = str:gsub(pair, "")
		if not tmp:match(openpar) then
			return str
		end
		repeat
			str = str:gsub(".-" .. openpar .. "(.*)", function (s)
				tmp = s:gsub(pair, "")
				if not tmp:match(openpar) then
					done = true
				end
				return s
			end)
		until done or str == ""
		return str
	end

	local function get_completions(str)
		local comps = ","
		local rest, dot, last = str:match("(.-)([.:]?)([^.:]*)$")
		str = find_unmatch(str, "%(", "%b()")
		str = find_unmatch(str, "%[", "%b[]")
		local table = str:match("([%w_%.:%(%)%[%]]-)[:.][%w_]*$")
		local err
		if rest == "" or not table then
			if dot == "" then
				table = "_G"
			else
				return comps
			end
		end
		err, tablef = pcall(load("return " .. table))
		if (not err) or (not tablef) then
			return comps
		end
		rest = rest .. dot
		if type(tablef) == 'table' then
			for k, v in pairs(tablef) do
				if k:match("^" .. last) then
					comps = comps .. "," .. rest .. k
				end
			end
		end
		if type(tablef) == "userdata" then
			local tablef = getmetatable(tablef)
			for k, v in pairs(tablef) do
				if k:match("^" .. last) then
					comps = comps .. "," .. rest .. k
				end
			end
		end
		return comps
	end

	emu.register_periodic(function()
		if conth.yield then
			conth:continue(get_completions(conth.result))
			return
		elseif conth.busy then
			return
		elseif started then
			local cmd = conth.result
			preload = false
			local func, err = load(cmd)
			if not func then
				if err:match("<eof>") then
					print("incomplete command")
					ln.preload(cmd)
					preload = true
				else
					print("error: ", err)
				end
			else
				local status
				status, err = pcall(func)
				if not status then
					print("error: ", err)
				end
			end
			if not preload then
				ln.historyadd(cmd)
			end
		end
		conth:start(scr)
		started = true
	end)
end

return exports
