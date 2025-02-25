-- license:BSD-3-Clause
-- copyright-holders:m1macrophage

local slider_manager = {}

-- Creates an instance of a slider_manager.
-- * view: The layout view that contains the sliders to be managed.
--         Typically: `file.views["{view layout name}"]`.
-- * device: Typically `file.device`.
function slider_manager:new(view, device)
	local manager = {}
	setmetatable(manager, self)
	self.__index = self
	self.view = view
	self.device = device
	self.sliders = {}  -- Stores slider information.
	self.pointers = {}  -- Tracks pointer state.
	return manager
end

-- Adds a vertical slider to the manager.
-- A slider comprises a click area, a knob and the associated input port.
-- The knob's Y position must be animated using <animate inputtag="{port_name}">.
-- The click area's vertical size must exactly span the range of the knob's
-- movement.
-- There are no requirements on horizontal dimensions.
--  _____
-- | _|_ |
-- ||___|<-- Slider knob.
-- |  |  |
-- |  |  | <-- Slider click area.
-- |  |  |
-- |__|__|
--
-- * clickarea_id: Element ID of the slider click area.
-- * knob_id: Element ID of the slider knob.
-- * port_name: Name of the associated IPT_ADJUSTER port.
function slider_manager:add_slider(clickarea_id, knob_id, port_name)
	local slider = {}

	slider.clickarea = self.view.items[clickarea_id]
	if slider.clickarea == nil then
		emu.print_error("Slider element: '" .. clickarea_id .. "' not found.")
		return
	end

	slider.knob = self.view.items[knob_id]
	if slider.knob == nil then
		emu.print_error("Slider knob element: '" .. knob_id .. "' not found.")
		return
	end

	local port = self.device:ioport(port_name)
	if port == nil then
		emu.print_error("Port: '" .. port_name .. "' not found.")
		return
	end

	slider.field = nil
	for k, val in pairs(port.fields) do
		slider.field = val
		break
	end
	if slider.field == nil then
		emu.print_error("Port: '" .. port_name .."' does not seem to be an IPT_ADJUSTER.")
		return
	end

	table.insert(self.sliders, slider)
end

-- Installs all callbacks.
-- If a layout script needs to install callbacks of its own, then it cannot use
-- this function. It will need to invoke the slider_manager's callbacks
-- from within its own callbacks.
function slider_manager:install_callbacks()
	self.view:set_pointer_updated_callback(
		function(type, id, dev, x, y, btn, dn, up, cnt)
			self:pointer_updated(type, id, dev, x, y, btn, dn, up, cnt)
		end)

	self.view:set_pointer_left_callback(
		function(type, id, dev, x, y, up, cnt)
			self:pointer_left(type, id, dev, x, y, up, cnt)
		end)

	self.view:set_pointer_aborted_callback(
		function(type, id, dev, x, y, up, cnt)
			self:pointer_aborted(type, id, dev, x, y, up, cnt)
		end)

	self.view:set_forget_pointers_callback(
		function() self:forget_pointers() end)
end

-- Pointer device callbacks.

function slider_manager:pointer_updated(type, id, dev, x, y, btn, dn, up, cnt)
	-- If a button is not pressed, reset the state of the current pointer.
	if btn & 1 == 0 then
		self.pointers[id] = nil
		return
	end

	-- If a button was just pressed, find the affected slider, if any.
	if dn & 1 ~= 0 then
		for i = 1, #self.sliders do
			if self.sliders[i].knob.bounds:includes(x, y) then
				self.pointers[id] = {
					selected_slider = i,
					relative = true,
					start_y = y,
					start_value = self.sliders[i].field.user_value }
				break
			elseif self.sliders[i].clickarea.bounds:includes(x, y) then
				self.pointers[id] = {
					selected_slider = i,
					relative = false }
				break
			end
		end
	end

	-- If there is no slider selected by the current pointer, we are done.
	if self.pointers[id] == nil then
		return
	end

	-- A slider is selected. Update state and, indirectly, slider knob position,
	-- based on the pointer's Y position. It is assumed the attached IO field is
	-- an IPT_ADJUSTER with a range of 0-100 (the default).

	local pointer = self.pointers[id]
	local slider = self.sliders[pointer.selected_slider]

	local knob_half_height = slider.knob.bounds.height / 2
	local min_y = slider.clickarea.bounds.y0 + knob_half_height
	local max_y = slider.clickarea.bounds.y1 - knob_half_height

	local new_value = 0
	if pointer.relative then
		-- User clicked on the knob. The new value will depend on how much the
		-- knob was dragged.
		new_value = pointer.start_value - 100 * (y - pointer.start_y) / (max_y - min_y)
	else
		-- User clicked elsewhere on the slider. The new value will depend on
		-- the absolute position of the click.
		new_value = 100 - 100 * (y - min_y) / (max_y - min_y)
	end

	new_value = math.floor(new_value + 0.5)
	if new_value < 0 then new_value = 0 end
	if new_value > 100 then new_value = 100 end
	slider.field.user_value = new_value
end

function slider_manager:pointer_left(type, id, dev, x, y, up, cnt)
	self.pointers[id] = nil
end

function slider_manager:pointer_aborted(type, id, dev, x, y, up, cnt)
	self.pointers[id] = nil
end

function slider_manager:forget_pointers()
	self.pointers = {}
end

return slider_manager
