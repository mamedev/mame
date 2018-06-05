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
	local socket = emu.file("rw")
	local nonce = 1
	local starttime = 0

	local function init()
		local err = socket:open("socket.127.0.0.1:6463")
		if err then
			error("discord: unable to connect, " .. err .. "\n")
		end
		socket:write("GET /\n\n")
		local time = os.time()
		local data = ""
		repeat
			local res = socket:read(100)
			data = data .. res
		until #res == 0 and #data > 0 or time + 1 < os.time()
		--print(data)
		if not data:find("Authorization Required", 1, true) then
			error("discord: bad RPC reply\n")
		end
	end

	local function update(stop)
		local json = require("json")
		local activity
		if not stop then
			local running = emu.romname() ~= "___empty"
			local state = not running and "In menu" or "Playing"
			local details = running and manager:machine():system().description or nil
			if emu.softname() ~= "" then
				for name, dev in pairs(manager:machine().images) do
					if dev:longname() then
						details = details .. " (" .. dev:longname() .. ")"
						break
					end
				end
			end
			activity = {
				state = state,
				details = details,
				timestamps = {
					start = starttime
				}
			}
		end
		local status = {
			cmd = "SET_ACTIVITY",
			args = {
				pid = 1234, -- we have no access to the pid here, except on linux
				activity = activity,
			},
			nonce = tostring(nonce)
		}
		nonce = nonce + 1
		local err = socket:open("socket.127.0.0.1:6463")
		if err then
			return
		end
		local output = json.stringify(status)
		output = "POST /rpc?v=1&client_id=453309506152169472\nContent-Type: application/json\nContent-Length: "
						.. tostring(#output) .. "\n\n" .. output
		--print(output)
		socket:write(output)
		local time = os.time()
		local data = ""
		repeat
			local res = socket:read(100)
			data = data .. res
		until #res == 0 and #data > 0 or time + 1 < os.time()
		--print(data)
	end
		
	do
		local stat, err = pcall(init)
		if not stat then
			emu.print_error(err)
			return
		end
	end

	emu.register_start(function()
		starttime = os.time()
		update(false)
	end)

	emu.register_stop(function()
		update(true)
	end)

end

return exports
