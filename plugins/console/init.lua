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
	local scr =  "local ln = require('linenoise')\n"
	scr = scr .. "ln.setcompletion(function(c, str) status = str\n"
	scr = scr .. "	yield()\n" -- coroutines can't yield in the middle of a callback so this is a real thread
	scr = scr .. "	status:gsub('[^,]*', function(s) if s ~= '' then ln.addcompletion(c, s) end end)\n"
	scr = scr .. "end)\n"
	scr = scr .. "return ln.linenoise('\x1b[1;36m[MAME]\x1b[0m> ')"

	function get_completions(str)
		local function is_pair_iterable(t)
			local mt = getmetatable(t)
			return type(t) == 'table' or (mt and mt.__pairs)
		end
		local comps = ","
		local table = str:match("([(]?[%w.:()]-)[:.]?[%w_]*$")
		local rest, last = str:match("(.-[:.]?)([%w_]*)$")
		local err
		if table == "" then
			table = "_G"
		end
		err, tablef = pcall(load("return " .. table))
		if (not err) or (not tablef) then
			return comps
		end
		if is_pair_iterable(tablef) then
			for k, v in pairs(tablef) do
				if k:match("^" .. last) then
					comps = comps .. "," .. rest .. k
				end
			end
		end
		local tablef = getmetatable(tablef)
		if is_pair_iterable(tablef) then
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
			ln.historyadd(cmd)
			local func, err = load(cmd)
			if not func then
				print("error: ", err)
			else
				local status
				status, err = pcall(func)
				if not status then
					print("error: ", err)
				end
			end
		end
		conth:start(scr)
		started = true
	end)
end

return exports
