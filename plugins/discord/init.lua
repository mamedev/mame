-- license:BSD-3-Clause
-- copyright-holders:Carl
local exports = {}
exports.name = "discord"
exports.version = "0.0.1"
exports.description = "Discord presence"
exports.license = "The BSD 3-Clause License"
exports.author = { name = "Carl" }

local discord = exports

function discord.startplugin()
	local pipe = emu.file("rw")
	local json = require("json")
	local nonce = 1
	local starttime = 0

	local function init()
		local path
		if package.config:sub(1,1) == '\\' then
			path = "\\\\.\\pipe\\discord-ipc-0"
		else
			path = os.getenv("XDG_RUNTIME_DIR") or os.getenv("TMPDIR") or os.getenv("TMP") or os.getenv("TEMP") or '/tmp'
			path = "domain." .. path .. "/discord-ipc-0"
		end
		local err = pipe:open(path)
		if err then
			error("discord: unable to connect, " .. err .. "\n")
		end
		local output = json.stringify({v = 1, client_id = "453309506152169472"})
		--print(output)
		pipe:write(string.pack("<I4I4", 0, #output) .. output)
		local time = os.time()
		local data = ""
		repeat
			local res = pipe:read(100)
			data = data .. res
		until #res == 0 and #data > 0 or time + 1 < os.time()
		--print(data)
		if data:find("code", 1, true) then
			error("discord: bad RPC reply, " .. data:sub(8) .. "\n")
		end
		if #data == 0 then
			error("discord: timed out waiting for response\n");
		end
	end

	local function update(status)
		if not pipe then return end
		local running = emu.romname() ~= "___empty"
		local state = not running and "In menu" or status
		local details = running and manager:machine():system().description or nil
		if emu.softname() ~= "" then
			for name, dev in pairs(manager:machine().images) do
				if dev:longname() then
					details = details .. " (" .. dev:longname() .. ")"
					break
				end
			end
		end
		local status = {
			cmd = "SET_ACTIVITY",
			args = {
				pid = emu.pid(),
				activity = {
					state = state,
					details = details,
					timestamps = {
						start = starttime
					}
				}
			},
			nonce = nonce
		}
		nonce = nonce + 1
		local output = json.stringify(status)
		--print(output)
		pipe:write(string.pack("<I4I4", 1, #output) .. output)
		local time = os.time()
		local data = ""
		repeat
			local res = pipe:read(100)
			data = data .. res
		until #res == 0 and #data > 0 or time + 1 < os.time()
		if #data == 0 then
			emu.print_verbose("discord: timed out waiting for response, closing connection\n");
			pipe = nil
		end
		--print(data)
	end

	do
		local stat, err = pcall(init)
		if not stat then
			emu.print_verbose(err)
			pipe = nil
			return
		end
	end

	emu.register_start(function()
		starttime = os.time()
		update("Playing")
	end)

	emu.register_pause(function()
		update("Paused")
	end)

	emu.register_resume(function()
		update("Playing")
	end)
end

return exports
