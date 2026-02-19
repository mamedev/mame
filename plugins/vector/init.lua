-- license:BSD-3-Clause
-- copyright-holders:Ryan Holtz
local exports = {
	name = 'vector',
	version = '0.0.1',
	description = 'Vector-hook demonstration plugin',
	license = 'BSD-3-Clause',
	author = { name = 'Ryan Holtz' } }


local vector = exports

local reset_subscription, frame_begin_subscription, frame_end_subscription, move_subscription, line_subscription

function vector.startplugin()
	local function frame_begin(index)
		print("frame begin");
	end

	local function frame_end(index)
		print("frame end");
	end

	local function vector_move(x, y, color, width, height)
		print(string.format("beam move, x:%.1f y:%.1f color:%08x width:%d height:%d", x, y, color, width, height));
	end

	local function vector_line(lastx, lasty, x, y, color, intensity, width, height)
		print(string.format("line, from x:%.1f y:%.1f, to x:%.1f y:%.1f, color:%08x intensity:%d width:%d height:%d", lastx, lasty, x, y, color, intensity, width, height));
	end

	local function start()
		local vector_device = manager.machine.vector_devices:at(1)
		if vector_device then
			frame_begin_subscription = vector_device:add_frame_begin_notifier(frame_begin)
			frame_end_subscription = vector_device:add_frame_end_notifier(frame_end)
			move_subscription = vector_device:add_move_notifier(vector_move)
			line_subscription = vector_device:add_line_notifier(vector_line)
		end
	end

	reset_subscription = emu.add_machine_reset_notifier(start)
end

return exports
