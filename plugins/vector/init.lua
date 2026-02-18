-- license:BSD-3-Clause
-- copyright-holders:Ryan Holtz
local exports = {
	name = 'vector',
	version = '0.0.1',
	description = 'Vector-hook helper plugin',
	license = 'BSD-3-Clause',
	author = { name = 'Ryan Holtz' } }


local vector = exports

local frame_begin_subscription

function vector.startplugin()
	local function frame_begin(index)
		print("frame begin\n");
	end

	local function start()
		print("starting\n")
		local vector_device = manager.machine.vector_devices:at(1)
		if vector_device then
			print("vector device found with tag %s\n", vector_device.tag)
			frame_begin_subscription = vector_device.add_frame_begin_notifier(frame_begin)
		end
	end

	emu.add_machine_reset_notifier(start)
end

return exports
