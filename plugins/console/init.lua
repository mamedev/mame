-- license:MIT
-- copyright-holders:Carl, Patrick Rapin, Reuben Thomas
-- completion from https://github.com/rrthomas/lua-rlcompleter
local exports = {}
exports.name = "console"
exports.version = "0.0.1"
exports.description = "Console plugin"
exports.license = "BSD-3-Clause"
exports.author = { name = "Carl" }

local console = exports
local history_file = "console_history"

local history_fullpath = nil

function console.startplugin()
	local conth = emu.thread()
	local ln_started = false
	local started = false
	local stopped = false
	local ln = require("linenoise")
	local preload = false
	local matches = {}
	local lastindex = 0
	local consolebuf
	_G.history = function (index)
		local history = ln.historyget()
		if index then
			ln.preload(history[index])
			return
		end
		for num, line in ipairs(history) do
			print(num, line)
		end
	end
	print("       /|  /|    /|     /|  /|    _______")
	print("      / | / |   / |    / | / |   /      /")
	print("     /  |/  |  /  |   /  |/  |  /  ____/ ")
	print("    /       | /   |  /       | /  /_     ")
	print("   /        |/    | /        |/  __/     ")
	print("  /  /|  /|    /| |/  /|  /|    /____    ")
	print(" /  / | / |   / |    / | / |        /    ")
	print("/ _/  |/  /  /  |___/  |/  /_______/     ")
	print("         /  /                            ")
	print("        / _/                             \n")
	print(emu.app_name() .. " " .. emu.app_version(), "\nCopyright (C) Nicola Salmoria and the MAME team\n");
	print(_VERSION, "\nCopyright (C) Lua.org, PUC-Rio\n");
	-- linenoise isn't thread safe but that means history can handled here
	-- that also means that bad things will happen if anything outside lua tries to use it
	-- especially the completion callback
	ln.historysetmaxlen(50)
	local scr = [[
		local ln = require('linenoise')
		ln.setcompletion(function(c, str, pos)
				status = str .. "\x01" .. tostring(pos)
				yield()
				ln.addcompletion(c, status:match("([^\x01]*)\x01(.*)"))
		end)
		local ret = ln.linenoise('$PROMPT')
		if ret == nil then
			return "\n"
		end
		return ret
	]]
	local keywords = {
		'and', 'break', 'do', 'else', 'elseif', 'end', 'false', 'for',
		'function', 'if', 'in', 'local', 'nil', 'not', 'or', 'repeat',
		'return', 'then', 'true', 'until', 'while'
	}
	local cmdbuf = ""

	-- Main completion function. It evaluates the current sub-expression
	-- to determine its type. Currently supports tables fields, global
	-- variables and function prototype completion.
	local function contextual_list(expr, sep, str, word)
		local function add(value)
			value = tostring(value)
			if value:match("^" .. word) then
				matches[#matches + 1] = value
			end
		end

		-- This function is called in a context where a keyword or a global
		-- variable can be inserted. Local variables cannot be listed!
		local function  add_globals()
			for _, k in ipairs(keywords) do
				add(k)
			end
			for k in pairs(_G) do
				add(k)
			end
		end

		if expr and expr ~= "" then
			local v = load("return " .. expr)
			if v then
				err, v = pcall(v)
				if (not err) or (not v) then
					add_globals()
					return
				end
				local t = type(v)
				if sep == '.' or sep == ':' then
					if t == 'table' then
						for k, v in pairs(v) do
							if type(k) == 'string' and (sep ~= ':' or type(v) == "function") then
								add(k)
							end
						end
					elseif t == 'userdata' then
						for k, v in pairs(getmetatable(v)) do
							if type(k) == 'string' and (sep ~= ':' or type(v) == "function") then
								add(k)
							end
						end
					end
				elseif sep == '[' then
					if t == 'table' then
						for k in pairs(v) do
							if type(k) == 'number' then
								add(k .. "]")
							end
						end
						if word ~= "" then add_globals() end
					end
				end
			end
		end
		if #matches == 0 then
			add_globals()
		end
	end

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

	-- This complex function tries to simplify the input line, by removing
	-- literal strings, full table constructors and balanced groups of
	-- parentheses. Returns the sub-expression preceding the word, the
	-- separator item ( '.', ':', '[', '(' ) and the current string in case
	-- of an unfinished string literal.
	local function simplify_expression(expr, word)
		-- Replace annoying sequences \' and \" inside literal strings
		expr = expr:gsub("\\(['\"])", function (c)
				return string.format("\\%03d", string.byte(c))
			end)
		local curstring
		-- Remove (finished and unfinished) literal strings
		while true do
			local idx1, _, equals = expr:find("%[(=*)%[")
			local idx2, _, sign = expr:find("(['\"])")
			if idx1 == nil and idx2 == nil then
				break
			end
			local idx, startpat, endpat
			if (idx1 or math.huge) < (idx2 or math.huge) then
				idx, startpat, endpat = idx1, "%[" .. equals .. "%[", "%]" .. equals .. "%]"
			else
				idx, startpat, endpat = idx2, sign, sign
			end
			if expr:sub(idx):find("^" .. startpat .. ".-" .. endpat) then
				expr = expr:gsub(startpat .. "(.-)" .. endpat, " STRING ")
			else
				expr = expr:gsub(startpat .. "(.*)", function (str)
						curstring = str
						return "(CURSTRING "
					end)
			end
		end
		-- crop string at unmatched open paran
		expr = find_unmatch(expr, "%(", "%b()")
		expr = find_unmatch(expr, "%[", "%b[]")
		--expr = expr:gsub("%b()"," PAREN ") -- Remove groups of parentheses
		expr = expr:gsub("%b{}"," TABLE ") -- Remove table constructors
		-- Avoid two consecutive words without operator
		expr = expr:gsub("(%w)%s+(%w)","%1|%2")
		expr = expr:gsub("%s+", "") -- Remove now useless spaces
		-- This main regular expression looks for table indexes and function calls.
		return curstring, expr:match("([%.:%w%(%)%[%]_]-)([:%.%[%(])" .. word .. "$")
	end

	local function get_completions(line, endpos)
		matches = {}
		local endstr = line:sub(endpos + 1, -1)
		line = line:sub(1, endpos)
		endstr = endstr or ""
		local start, word = line:match("^(.*[ \t\n\"\\'><=;:%+%-%*/%%^~#{}%(%)%[%].,])(.-)$")
		if not start then
			start = ""
			word = word or line
		else
			word = word or ""
		end

		local str, expr, sep = simplify_expression(line, word)
		contextual_list(expr, sep, str, word)
		if #matches > 1 then
			print("\n")
			for k, v in pairs(matches) do
				print(v)
			end
			return "\x01" .. "-1"
		elseif #matches == 1 then
			return start .. matches[1] .. endstr .. "\x01" .. (#start + #matches[1])
		end
		return "\x01" .. "-1"
	end

	emu.register_start(function()
		if not consolebuf and manager.machine.debugger then
			consolebuf = manager.machine.debugger.consolelog
			lastindex = 0
		end
	end)

	emu.register_stop(function() consolebuf = nil end)

	emu.register_periodic(function()
		if stopped then
			return
		end
		if (not started) then
			-- options are not available in startplugin, so we load the history here
			local homepath = emu.subst_env(manager.options.entries.homepath:value():match("([^;]+)"))
			history_fullpath = homepath .. '/' .. history_file
			ln.loadhistory(history_fullpath)
			started = true
		end
		local prompt = "\x1b[1;36m[MAME]\x1b[0m> "
		if consolebuf and (#consolebuf > lastindex) then
			local last = #consolebuf
			print("\n")
			while lastindex < last do
				lastindex = lastindex + 1
				print(consolebuf[lastindex])
			end
			ln.refresh()
		end
		if conth.yield then
			conth:continue(get_completions(conth.result:match("([^\x01]*)\x01(.*)")))
			return
		elseif conth.busy then
			return
		elseif ln_started then
			local cmd = conth.result
			if cmd == "\n" then
				stopped = true
				return
			elseif cmd == "" then
				if cmdbuf ~= "" then
					print("Incomplete command")
					cmdbuf = ""
				end
			else
				cmdbuf = cmdbuf .. "\n" .. cmd
				local func, err = load(cmdbuf)
				if not func then
					if err:match("<eof>") then
						prompt = "\x1b[1;36m[MAME]\x1b[0m>> "
					else
						print("error: ", err)
						cmdbuf = ""
					end
				else
					local status
					status, err = pcall(func)
					if not status then
						print("error: ", err)
					end
					cmdbuf = ""
				end
				ln.historyadd(cmd)
			end
		end
		conth:start(scr:gsub("$PROMPT", prompt))
		ln_started = true
	end)
end

setmetatable(console, {
			 __gc = function ()
				 if history_fullpath then
					 local ln = require("linenoise")
					 ln.savehistory(history_fullpath)
				 end
end})

return exports
