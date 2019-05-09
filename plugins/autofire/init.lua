-- license:BSD-3-Clause
-- copyright-holders:Jack Li
local exports = {}
exports.name = "autofire"
exports.version = "0.0.1"
exports.description = "Autofire plugin"
exports.license = "The BSD 3-Clause License"
exports.author = { name = "Jack Li" }

local autofire = exports

function autofire.startplugin()
	local key = 'KEYCODE_Q'
	local on_frames = 1
	local off_frames = 60
	local counter = 0

	local function per_frame()
		if emu.romname() == '___empty' then
			return
		end
		if manager:machine().paused then
			return
		end
		local keycode = manager:machine():input():code_from_token(key)
		local pressed = manager:machine():input():code_pressed(keycode)
		local p1b1 = manager:machine():ioport().ports[':P1'].fields['P1 Button 1']
		if pressed then
			p1b1:set_value(counter < on_frames and 1 or 0)
			counter = (counter + 1) % (on_frames + off_frames)
		else
			counter = 0
			p1b1:set_value(0)
		end
	end

	emu.register_frame(per_frame)
end

return exports