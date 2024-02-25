// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    inputdev.cpp

    Input devices, items and device classes.

***************************************************************************/

#include "emu.h"
#include "inputdev.h"

#include "corestr.h"
#include "emuopts.h"


namespace {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> input_device_switch_item

// derived input item representing a switch input
class input_device_switch_item : public input_device_item
{
public:
	// construction/destruction
	input_device_switch_item(
			input_device &device,
			std::string_view name,
			std::string_view tokenhint,
			void *internal,
			input_item_id itemid,
			item_get_state_func getstate);

	// readers
	virtual s32 read_as_switch(input_item_modifier modifier) override;
	virtual s32 read_as_relative(input_item_modifier modifier) override;
	virtual s32 read_as_absolute(input_item_modifier modifier) override;
	virtual bool item_check_axis(input_item_modifier modifiers, s32 memory) override;

	// steadykey helper
	bool steadykey_changed();
	void steadykey_update_to_current() { m_steadykey = m_current; }

private:
	// internal state
	s32                     m_steadykey;            // the live steadykey state
	s32                     m_oldkey;               // old live state
};


// ======================> input_device_switch_item

// derived input item representing a relative axis input
class input_device_relative_item : public input_device_item
{
public:
	// construction/destruction
	input_device_relative_item(
			input_device &device,
			std::string_view name,
			std::string_view tokenhint,
			void *internal,
			input_item_id itemid,
			item_get_state_func getstate);

	// readers
	virtual s32 read_as_switch(input_item_modifier modifier) override;
	virtual s32 read_as_relative(input_item_modifier modifier) override;
	virtual s32 read_as_absolute(input_item_modifier modifier) override;
	virtual bool item_check_axis(input_item_modifier modifier, s32 memory) override;
};


// ======================> input_device_switch_item

// derived input item representing an absolute axis input
class input_device_absolute_item : public input_device_item
{
public:
	// construction/destruction
	input_device_absolute_item(
			input_device &device,
			std::string_view name,
			std::string_view tokenhint,
			void *internal,
			input_item_id itemid,
			item_get_state_func getstate);

	// readers
	virtual s32 read_as_switch(input_item_modifier modifier) override;
	virtual s32 read_as_relative(input_item_modifier modifier) override;
	virtual s32 read_as_absolute(input_item_modifier modifier) override;
	virtual bool item_check_axis(input_item_modifier modifier, s32 memory) override;
};

} // anonymous namespace


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// standard joystick mappings
const char          input_class_joystick::map_8way[] = "7778...4445";
const char          input_class_joystick::map_4way_diagonal[] = "4444s8888..444458888.444555888.ss5.222555666.222256666.2222s6666.2222s6666";
// const char          input_class_joystick::map_4way_sticky[] = "s8.4s8.44s8.4445";


//**************************************************************************
//  JOYSTICK MAP
//**************************************************************************

//-------------------------------------------------
//  joystick_map - constructor
//-------------------------------------------------

joystick_map::joystick_map()
	: m_lastmap(JOYSTICK_MAP_NEUTRAL)
{
	// parse the standard 8-way map as default
	parse(input_class_joystick::map_8way);
}


//-------------------------------------------------
//  parse - parse a string into a joystick map
//-------------------------------------------------

bool joystick_map::parse(const char *mapstring)
{
	// save a copy of the original string
	m_origstring = mapstring;

	// iterate over rows
	for (int rownum = 0; rownum < 9; rownum++)
	{
		// if we're done, copy from another row
		if (*mapstring == 0 || *mapstring == '.')
		{
			bool symmetric = (rownum >= 5 && *mapstring == 0);
			const u8 *srcrow = &m_map[symmetric ? (8 - rownum) : (rownum - 1)][0];

			// if this is row 0, we don't have a source row -- invalid
			if (rownum == 0)
				return false;

			// copy from the srcrow, applying up/down symmetry if in the bottom half
			for (int colnum = 0; colnum < 9; colnum++)
			{
				u8 val = srcrow[colnum];
				if (symmetric)
					val = (val & (JOYSTICK_MAP_LEFT | JOYSTICK_MAP_RIGHT)) | ((val & JOYSTICK_MAP_UP) << 1) | ((val & JOYSTICK_MAP_DOWN) >> 1);
				m_map[rownum][colnum] = val;
			}
		}

		// otherwise, parse this column
		else
		{
			for (int colnum = 0; colnum < 9; colnum++)
			{
				// if we're at the end of row, copy previous to the middle, then apply left/right symmetry
				if (colnum > 0 && (*mapstring == 0 || *mapstring == '.'))
				{
					bool symmetric = (colnum >= 5);
					u8 val = m_map[rownum][symmetric ? (8 - colnum) : (colnum - 1)];
					if (symmetric)
						val = (val & (JOYSTICK_MAP_UP | JOYSTICK_MAP_DOWN)) | ((val & JOYSTICK_MAP_LEFT) << 1) | ((val & JOYSTICK_MAP_RIGHT) >> 1);
					m_map[rownum][colnum] = val;
				}

				// otherwise, convert the character to its value
				else
				{
					static const u8 charmap[] =
					{
						JOYSTICK_MAP_UP | JOYSTICK_MAP_LEFT,
						JOYSTICK_MAP_UP,
						JOYSTICK_MAP_UP | JOYSTICK_MAP_RIGHT,
						JOYSTICK_MAP_LEFT,
						JOYSTICK_MAP_NEUTRAL,
						JOYSTICK_MAP_RIGHT,
						JOYSTICK_MAP_DOWN | JOYSTICK_MAP_LEFT,
						JOYSTICK_MAP_DOWN,
						JOYSTICK_MAP_DOWN | JOYSTICK_MAP_RIGHT,
						JOYSTICK_MAP_STICKY
					};
					static const char validchars[] = "789456123s";
					const char *ptr = strchr(validchars, *mapstring++);

					// invalid characters exit immediately
					if (ptr == nullptr)
						return false;
					m_map[rownum][colnum] = charmap[ptr - validchars];
				}
			}
		}

		// if we ended with a period, advance to the next row
		if (*mapstring == '.')
			mapstring++;
	}
	return true;
}


//-------------------------------------------------
//  to_string - output the map as a string for
//  friendly display
//-------------------------------------------------

std::string joystick_map::to_string() const
{
	std::string str(m_origstring);
	str.append("\n");
	for (auto & elem : m_map)
	{
		str.append("  ");
		for (auto & elem_colnum : elem)
			switch (elem_colnum)
			{
				case JOYSTICK_MAP_UP | JOYSTICK_MAP_LEFT:   str.append("7");  break;
				case JOYSTICK_MAP_UP:                       str.append("8");  break;
				case JOYSTICK_MAP_UP | JOYSTICK_MAP_RIGHT:  str.append("9");  break;
				case JOYSTICK_MAP_LEFT:                     str.append("4");  break;
				case JOYSTICK_MAP_NEUTRAL:                  str.append("5");  break;
				case JOYSTICK_MAP_RIGHT:                    str.append("6");  break;
				case JOYSTICK_MAP_DOWN | JOYSTICK_MAP_LEFT: str.append("1");  break;
				case JOYSTICK_MAP_DOWN:                     str.append("2");  break;
				case JOYSTICK_MAP_DOWN | JOYSTICK_MAP_RIGHT:str.append("3");  break;
				case JOYSTICK_MAP_STICKY:                   str.append("s");  break;
				default:                                    str.append("?");  break;
			}

		str.append("\n");
	}
	return str;
}


//-------------------------------------------------
//  update - update the state of the joystick
//  map based on the given X/Y axis values
//-------------------------------------------------

u8 joystick_map::update(s32 xaxisval, s32 yaxisval)
{
	// now map the X and Y axes to a 9x9 grid using the raw values
	xaxisval = ((xaxisval - osd::input_device::ABSOLUTE_MIN) * 9) / (osd::input_device::ABSOLUTE_MAX - osd::input_device::ABSOLUTE_MIN + 1);
	yaxisval = ((yaxisval - osd::input_device::ABSOLUTE_MIN) * 9) / (osd::input_device::ABSOLUTE_MAX - osd::input_device::ABSOLUTE_MIN + 1);
	u8 mapval = m_map[yaxisval][xaxisval];

	// handle stickiness
	if (mapval == JOYSTICK_MAP_STICKY)
		mapval = m_lastmap;
	else
		m_lastmap = mapval;

	// return based on whether the appropriate bit is set
	return mapval;
}



//**************************************************************************
//  INPUT DEVICE
//**************************************************************************

//-------------------------------------------------
//  input_device - constructor
//-------------------------------------------------

input_device::input_device(input_manager &manager, std::string_view name, std::string_view id, void *internal)
	: m_manager(manager)
	, m_name(name)
	, m_id(id)
	, m_devindex(-1)
	, m_maxitem(input_item_id(0))
	, m_internal(internal)
	, m_threshold(std::max<s32>(s32(manager.machine().options().joystick_threshold() * osd::input_device::ABSOLUTE_MAX), 1))
	, m_steadykey_enabled(manager.machine().options().steadykey())
	, m_lightgun_reload_button(manager.machine().options().offscreen_reload())
{
}


//-------------------------------------------------
//  input_device - destructor
//-------------------------------------------------

input_device::~input_device()
{
}


//-------------------------------------------------
//  add_item - add a new item to an input device
//-------------------------------------------------

input_item_id input_device::add_item(
		std::string_view name,
		std::string_view tokenhint,
		input_item_id itemid,
		item_get_state_func getstate,
		void *internal)
{
	if (machine().phase() != machine_phase::INIT)
		throw emu_fatalerror("Can only call input_device::add_item at init time!");
	assert(itemid > ITEM_ID_INVALID && itemid < ITEM_ID_MAXIMUM);
	assert(getstate != nullptr);

	// if we have a generic ID, pick a new internal one
	input_item_id originalid = itemid;
	if (itemid >= ITEM_ID_OTHER_SWITCH && itemid <= ITEM_ID_OTHER_AXIS_RELATIVE)
		for (itemid = (input_item_id)(ITEM_ID_MAXIMUM + 1); itemid <= ITEM_ID_ABSOLUTE_MAXIMUM; ++itemid)
			if (m_item[itemid] == nullptr)
				break;
	assert(itemid <= ITEM_ID_ABSOLUTE_MAXIMUM);

	// make sure we don't have any overlap
	assert(m_item[itemid] == nullptr);

	// determine the class and create the appropriate item class
	switch (m_manager.device_class(devclass()).standard_item_class(originalid))
	{
		case ITEM_CLASS_SWITCH:
			m_item[itemid] = std::make_unique<input_device_switch_item>(*this, name, tokenhint, internal, itemid, getstate);
			break;

		case ITEM_CLASS_RELATIVE:
			m_item[itemid] = std::make_unique<input_device_relative_item>(*this, name, tokenhint, internal, itemid, getstate);
			break;

		case ITEM_CLASS_ABSOLUTE:
			m_item[itemid] = std::make_unique<input_device_absolute_item>(*this, name, tokenhint, internal, itemid, getstate);
			break;

		default:
			m_item[itemid] = nullptr;
			assert(false);
	}

	// assign the new slot and update the maximum
	m_maxitem = std::max(m_maxitem, itemid);
	return itemid;
}


//-------------------------------------------------
//  set_default_assignments - set additional input
//  assignments suitable for device
//-------------------------------------------------

void input_device::set_default_assignments(assignment_vector &&assignments)
{
	m_default_assignments = std::move(assignments);
}


//-------------------------------------------------
//  match_device_id - match device id via
//  substring search
//-------------------------------------------------

bool input_device::match_device_id(std::string_view deviceid) const
{
	std::string deviceidupper(strmakeupper(deviceid));
	std::string idupper(strmakeupper(m_id));

	return std::string::npos == idupper.find(deviceidupper) ? false : true;
}


//**************************************************************************
//  SPECIFIC INPUT DEVICES
//**************************************************************************

//-------------------------------------------------
//  input_device_keyboard - constructor
//-------------------------------------------------

input_device_keyboard::input_device_keyboard(input_manager &manager, std::string_view _name, std::string_view _id, void *_internal)
	: input_device(manager, _name, _id, _internal)
{
}


//-------------------------------------------------
//  apply_steadykey - apply steadykey option
//-------------------------------------------------

void input_device_keyboard::apply_steadykey() const
{
	// ignore if not enabled
	if (!steadykey_enabled())
		return;

	// update the state of all the keys and see if any changed state
	bool anything_changed = false;
	for (input_item_id itemid = ITEM_ID_FIRST_VALID; itemid <= maxitem(); ++itemid)
	{
		input_device_item *itm = item(itemid);
		if (itm != nullptr && itm->itemclass() == ITEM_CLASS_SWITCH)
			if (downcast<input_device_switch_item &>(*itm).steadykey_changed())
				anything_changed = true;
	}

	// if the keyboard state is stable, flush the current state
	if (!anything_changed)
		for (input_item_id itemid = ITEM_ID_FIRST_VALID; itemid <= maxitem(); ++itemid)
		{
			input_device_item *itm = item(itemid);
			if (itm != nullptr && itm->itemclass() == ITEM_CLASS_SWITCH)
				downcast<input_device_switch_item &>(*itm).steadykey_update_to_current();
		}
}


//-------------------------------------------------
//  input_device_mouse - constructor
//-------------------------------------------------

input_device_mouse::input_device_mouse(input_manager &manager, std::string_view _name, std::string_view _id, void *_internal)
	: input_device(manager, _name, _id, _internal)
{
}


//-------------------------------------------------
//  input_device_lightgun - constructor
//-------------------------------------------------

input_device_lightgun::input_device_lightgun(input_manager &manager, std::string_view _name, std::string_view _id, void *_internal)
	: input_device(manager, _name, _id, _internal)
{
}


//-------------------------------------------------
//  input_device_joystick - constructor
//-------------------------------------------------

input_device_joystick::input_device_joystick(input_manager &manager, std::string_view _name, std::string_view _id, void *_internal)
	: input_device(manager, _name, _id, _internal)
	, m_deadzone(s32(manager.machine().options().joystick_deadzone() * osd::input_device::ABSOLUTE_MAX))
	, m_saturation(s32(manager.machine().options().joystick_saturation() * osd::input_device::ABSOLUTE_MAX))
	, m_range(m_saturation - m_deadzone)
{
	// get the default joystick map
	const char *mapstring = machine().options().joystick_map();
	if (mapstring[0] == 0 || strcmp(mapstring, "auto") == 0)
		mapstring = input_class_joystick::map_8way;

	// parse it
	if (!m_joymap.parse(mapstring))
	{
		osd_printf_error("Invalid joystick map: %s\n", mapstring);
		m_joymap.parse(input_class_joystick::map_8way);
	}
	else if (mapstring != input_class_joystick::map_8way)
		osd_printf_verbose("Input: Default joystick map = %s\n", m_joymap.to_string());
}


//-------------------------------------------------
//  adjust_absolute_value - apply joystick device
//  deadzone and saturation parameters to an
//  absolute value
//-------------------------------------------------

s32 input_device_joystick::adjust_absolute_value(s32 result) const
{
	// properties are symmetric
	bool const negative = result < 0;
	if (negative)
		result = -result;

	if (result < m_deadzone)            // if in the deadzone, return 0
		result = 0;
	else if (result >= m_saturation)    // if saturated, return the max
		result = osd::input_device::ABSOLUTE_MAX;
	else                                // otherwise, scale
		result = s64(result - m_deadzone) * s64(osd::input_device::ABSOLUTE_MAX) / m_range;

	// re-apply sign and return
	return negative ? -result : result;
}


//**************************************************************************
//  INPUT CLASS
//**************************************************************************

//-------------------------------------------------
//  input_class - constructor
//-------------------------------------------------

input_class::input_class(input_manager &manager, input_device_class devclass, const char *name, bool enabled, bool multi)
	: m_manager(manager),
		m_devclass(devclass),
		m_name(name),
		m_maxindex(0),
		m_enabled(enabled),
		m_multi(multi)
{
	assert(m_name != nullptr);
}


//-------------------------------------------------
//  input_class - destructor
//-------------------------------------------------

input_class::~input_class()
{
}


//-------------------------------------------------
//  add_device - add a new input device
//-------------------------------------------------

input_device &input_class::add_device(std::string_view name, std::string_view id, void *internal)
{
	if (machine().phase() != machine_phase::INIT)
		throw emu_fatalerror("Can only call input_class::add_device at init time!");

	// allocate a new device and add it to the index
	return add_device(make_device(name, id, internal));
}

input_device &input_class::add_device(std::unique_ptr<input_device> &&new_device)
{
	assert(new_device->devclass() == m_devclass);

	// find the next empty index
	for (int devindex = 0; devindex < DEVICE_INDEX_MAXIMUM; devindex++)
		if (m_device[devindex] == nullptr)
		{
			// update the device and maximum index found
			new_device->set_devindex(devindex);
			m_maxindex = std::max(m_maxindex, devindex);

			if (new_device->id()[0] == 0)
				osd_printf_verbose("Input: Adding %s #%d: %s\n", m_name, devindex + 1, new_device->name());
			else
				osd_printf_verbose("Input: Adding %s #%d: %s (device id: %s)\n", m_name, devindex + 1, new_device->name(), new_device->id());

			m_device[devindex] = std::move(new_device);
			return *m_device[devindex];
		}

	throw emu_fatalerror("Input: Too many %s devices\n", m_name);
}


//-------------------------------------------------
//  standard_item_class - return the class of a
//  standard item
//-------------------------------------------------

input_item_class input_class::standard_item_class(input_item_id itemid) const
{
	if (itemid == ITEM_ID_OTHER_SWITCH || itemid < ITEM_ID_XAXIS || (itemid > ITEM_ID_SLIDER2 && itemid < ITEM_ID_ADD_ABSOLUTE1))
	{
		// most everything standard is a switch, apart from the axes
		return ITEM_CLASS_SWITCH;
	}
	else if (m_devclass == DEVICE_CLASS_MOUSE || itemid == ITEM_ID_OTHER_AXIS_RELATIVE || (itemid >= ITEM_ID_ADD_RELATIVE1 && itemid <= ITEM_ID_ADD_RELATIVE16))
	{
		// standard mouse axes are relative
		return ITEM_CLASS_RELATIVE;
	}
	else
	{
		// all other standard axes are absolute
		return ITEM_CLASS_ABSOLUTE;
	}
}


//-------------------------------------------------
//  remap_device_index - remaps device index by
//  mapping oldindex to newindex
//-------------------------------------------------

void input_class::remap_device_index(int oldindex, int newindex)
{
	assert(oldindex >= 0 && oldindex < DEVICE_INDEX_MAXIMUM);
	assert(newindex >= 0 && newindex < DEVICE_INDEX_MAXIMUM);

	// swap indexes in m_device array
	m_device[oldindex].swap(m_device[newindex]);

	// update device indexes
	if (nullptr != m_device[oldindex].get())
		m_device[oldindex]->set_devindex(oldindex);

	if (nullptr != m_device[newindex].get())
		m_device[newindex]->set_devindex(newindex);

	// update the maximum index found, since newindex may exceed current m_maxindex
	m_maxindex = std::max(m_maxindex, newindex);
}


//**************************************************************************
//  SPECIFIC INPUT CLASSES
//**************************************************************************

//-------------------------------------------------
//  input_class_keyboard - constructor
//-------------------------------------------------

input_class_keyboard::input_class_keyboard(input_manager &manager)
	: input_class(manager, DEVICE_CLASS_KEYBOARD, "keyboard", true, manager.machine().options().multi_keyboard())
{
	// request a per-frame callback for the keyboard class
	machine().add_notifier(MACHINE_NOTIFY_FRAME, machine_notify_delegate(&input_class_keyboard::frame_callback, this));
}


//-------------------------------------------------
//  frame_callback - per-frame callback for various
//  bookkeeping
//-------------------------------------------------

void input_class_keyboard::frame_callback()
{
	// iterate over all devices in our class
	for (int devnum = 0; devnum <= maxindex(); devnum++)
		if (device(devnum) != nullptr)
			downcast<input_device_keyboard &>(*device(devnum)).apply_steadykey();
}


//-------------------------------------------------
//  input_class_mouse - constructor
//-------------------------------------------------

input_class_mouse::input_class_mouse(input_manager &manager)
	: input_class(manager, DEVICE_CLASS_MOUSE, "mouse", manager.machine().options().mouse(), manager.machine().options().multi_mouse())
{
}


//-------------------------------------------------
//  input_class_lightgun - constructor
//-------------------------------------------------

input_class_lightgun::input_class_lightgun(input_manager &manager)
	: input_class(manager, DEVICE_CLASS_LIGHTGUN, "lightgun", manager.machine().options().lightgun(), true)
{
}


//-------------------------------------------------
//  input_class_joystick - constructor
//-------------------------------------------------

input_class_joystick::input_class_joystick(input_manager &manager)
	: input_class(manager, DEVICE_CLASS_JOYSTICK, "joystick", manager.machine().options().joystick(), true)
{
}


//-------------------------------------------------
//  set_global_joystick_map - set the map for all
//  joysticks
//-------------------------------------------------

bool input_class_joystick::set_global_joystick_map(const char *mapstring)
{
	// parse the map
	joystick_map map;
	if (!map.parse(mapstring))
		return false;

	osd_printf_verbose("Input: Changing default joystick map = %s\n", map.to_string());

	// iterate over joysticks and set the map
	for (int joynum = 0; joynum <= maxindex(); joynum++)
		if (device(joynum) != nullptr)
			downcast<input_device_joystick &>(*device(joynum)).set_joystick_map(map);
	return true;
}


//**************************************************************************
//  INPUT DEVICE ITEM
//**************************************************************************

//-------------------------------------------------
//  input_device_item - constructor
//-------------------------------------------------

input_device_item::input_device_item(
		input_device &device,
		std::string_view name,
		std::string_view tokenhint,
		void *internal,
		input_item_id itemid,
		item_get_state_func getstate,
		input_item_class itemclass)
	: m_device(device)
	, m_name(name)
	, m_internal(internal)
	, m_itemid(itemid)
	, m_itemclass(itemclass)
	, m_getstate(getstate)
	, m_current(0)
{
	const char *standard_token = manager().standard_token(itemid);
	if (standard_token)
	{
		// use a standard token name for known item IDs
		m_token = standard_token;
	}
	else if (!tokenhint.empty())
	{
		// fall back to token hint if supplied
		m_token = tokenhint;
	}
	else
	{
		// otherwise, create a tokenized name
		m_token = strmakeupper(name);
		strdelchr(m_token, ' ');
		strdelchr(m_token, '_');
	}
}


//-------------------------------------------------
//  input_device_item - destructor
//-------------------------------------------------

input_device_item::~input_device_item()
{
}


//-------------------------------------------------
//  check_axis - see if axis has moved far enough
//  to trigger a read when polling
//-------------------------------------------------

bool input_device_item::check_axis(input_item_modifier modifier, s32 memory)
{
	// use osd::INVALID_AXIS_VALUE as a short-circuit
	return (memory != osd::input_device::INVALID_AXIS_VALUE) && item_check_axis(modifier, memory);
}


//**************************************************************************
//  INPUT DEVICE SWITCH ITEM
//**************************************************************************

//-------------------------------------------------
//  input_device_switch_item - constructor
//-------------------------------------------------

input_device_switch_item::input_device_switch_item(
		input_device &device,
		std::string_view name,
		std::string_view tokenhint,
		void *internal,
		input_item_id itemid,
		item_get_state_func getstate)
	: input_device_item(device, name, tokenhint, internal, itemid, getstate, ITEM_CLASS_SWITCH)
	, m_steadykey(0)
	, m_oldkey(0)
{
}


//-------------------------------------------------
//  read_as_switch - return the raw switch value,
//  modified as necessary
//-------------------------------------------------

s32 input_device_switch_item::read_as_switch(input_item_modifier modifier)
{
	// if we're doing a lightgun reload hack, button 1 and 2 operate differently
	input_device_class devclass = m_device.devclass();
	if (devclass == DEVICE_CLASS_LIGHTGUN && m_device.lightgun_reload_button())
	{
		// button 1 is pressed if either button 1 or 2 are active
		if (m_itemid == ITEM_ID_BUTTON1)
		{
			input_device_item *button2_item = m_device.item(ITEM_ID_BUTTON2);
			if (button2_item != nullptr)
				return button2_item->update_value() | update_value();
		}

		// button 2 is never officially pressed
		if (m_itemid == ITEM_ID_BUTTON2)
			return 0;
	}

	// steadykey for keyboards
	if (devclass == DEVICE_CLASS_KEYBOARD && m_device.steadykey_enabled())
		return m_steadykey;

	// everything else is just the current value as-is
	return update_value();
}


//-------------------------------------------------
//  read_as_relative - return the switch input as
//  a relative axis value
//-------------------------------------------------

s32 input_device_switch_item::read_as_relative(input_item_modifier modifier)
{
	// no translation to relative
	return 0;
}


//-------------------------------------------------
//  read_as_absolute - return the switch input as
//  an absolute axis value
//-------------------------------------------------

s32 input_device_switch_item::read_as_absolute(input_item_modifier modifier)
{
	// no translation to absolute
	return 0;
}


//-------------------------------------------------
//  item_check_axis - see if axis has moved far
//  enough to trigger a read when polling
//-------------------------------------------------

bool input_device_switch_item::item_check_axis(input_item_modifier modifier, s32 memory)
{
	return false;
}


//-------------------------------------------------
//  steadykey_changed - update for steadykey
//  behavior, returning true if the current state
//  has changed since the last call
//-------------------------------------------------

bool input_device_switch_item::steadykey_changed()
{
	s32 old = m_oldkey;
	m_oldkey = update_value();
	if (((m_current ^ old) & 1) == 0)
		return false;

	// if the keypress was missed, turn it on for one frame
	if (((m_current | m_steadykey) & 1) == 0)
		m_steadykey = 1;
	return true;
}



//**************************************************************************
//  INPUT DEVICE RELATIVE ITEM
//**************************************************************************

//-------------------------------------------------
//  input_device_relative_item - constructor
//-------------------------------------------------

input_device_relative_item::input_device_relative_item(
		input_device &device,
		std::string_view name,
		std::string_view tokenhint,
		void *internal,
		input_item_id itemid,
		item_get_state_func getstate)
	: input_device_item(device, name, tokenhint, internal, itemid, getstate, ITEM_CLASS_RELATIVE)
{
}


//-------------------------------------------------
//  read_as_switch - return the relative value as
//  a switch result based on the modifier
//-------------------------------------------------

s32 input_device_relative_item::read_as_switch(input_item_modifier modifier)
{
	// process according to modifiers
	switch (modifier)
	{
	case ITEM_MODIFIER_POS:
	case ITEM_MODIFIER_RIGHT:
	case ITEM_MODIFIER_DOWN:
		return update_value() > 0;
	case ITEM_MODIFIER_NEG:
	case ITEM_MODIFIER_LEFT:
	case ITEM_MODIFIER_UP:
		return update_value() < 0;
	// all other cases just return 0
	default:
		return 0;
	}
}


//-------------------------------------------------
//  read_as_relative - return the relative input
//  as a relative axis value
//-------------------------------------------------

s32 input_device_relative_item::read_as_relative(input_item_modifier modifier)
{
	// just return directly
	if (ITEM_MODIFIER_REVERSE == modifier)
		return -update_value();
	else
		return update_value();
}


//-------------------------------------------------
//  read_as_absolute - return the relative input
//  as an absolute axis value
//-------------------------------------------------

s32 input_device_relative_item::read_as_absolute(input_item_modifier modifier)
{
	// no translation to absolute
	return 0;
}


//-------------------------------------------------
//  item_check_axis - see if axis has moved far
//  enough to trigger a read when polling
//-------------------------------------------------

bool input_device_relative_item::item_check_axis(input_item_modifier modifier, s32 memory)
{
	const s32 curval = read_as_relative(modifier);

	// for relative axes, look for ~20 pixels movement
	return std::abs(curval - memory) > (20 * osd::input_device::RELATIVE_PER_PIXEL);
}



//**************************************************************************
//  INPUT DEVICE ABSOLUTE ITEM
//**************************************************************************

//-------------------------------------------------
//  input_device_absolute_item - constructor
//-------------------------------------------------

input_device_absolute_item::input_device_absolute_item(
		input_device &device,
		std::string_view name,
		std::string_view tokenhint,
		void *internal,
		input_item_id itemid,
		item_get_state_func getstate)
	: input_device_item(device, name, tokenhint, internal, itemid, getstate, ITEM_CLASS_ABSOLUTE)
{
}


//-------------------------------------------------
//  read_as_switch - return the absolute value as
//  a switch result based on the modifier
//-------------------------------------------------

s32 input_device_absolute_item::read_as_switch(input_item_modifier modifier)
{
	// start with the current value
	s32 const result = update_value();

	// left/right/up/down: if this is a joystick, fetch the paired X/Y axis values and convert
	if (m_device.devclass() == DEVICE_CLASS_JOYSTICK && modifier >= ITEM_MODIFIER_LEFT && modifier <= ITEM_MODIFIER_DOWN)
	{
		input_device_item *xaxis_item = m_device.item(ITEM_ID_XAXIS);
		input_device_item *yaxis_item = m_device.item(ITEM_ID_YAXIS);
		if (xaxis_item != nullptr && yaxis_item != nullptr)
		{
			// determine which item we didn't update, and update it
			assert(this == xaxis_item || this == yaxis_item);
			if (this == xaxis_item)
				yaxis_item->update_value();
			else
				xaxis_item->update_value();

			// now map the X and Y axes to a 9x9 grid using the raw values
			joystick_map &joymap = downcast<input_device_joystick &>(m_device).joymap();
			return BIT(joymap.update(xaxis_item->current(), yaxis_item->current()), modifier - ITEM_MODIFIER_LEFT);
		}
	}

	// positive/negative: true if past the threshold in either direction, otherwise zero
	switch (modifier)
	{
	case ITEM_MODIFIER_POS:
	case ITEM_MODIFIER_RIGHT:
	case ITEM_MODIFIER_DOWN:
		return result >= m_device.threshold();
	case ITEM_MODIFIER_NEG:
	case ITEM_MODIFIER_LEFT:
	case ITEM_MODIFIER_UP:
		return -result >= m_device.threshold();
	default:
		return 0;
	}
}


//-------------------------------------------------
//  read_as_relative - return the absolute input
//  as a relative axis value
//-------------------------------------------------

s32 input_device_absolute_item::read_as_relative(input_item_modifier modifier)
{
	// no translation to relative
	return 0;
}


//-------------------------------------------------
//  read_as_absolute - return the absolute input
//  as an absolute axis value, with appropriate
//  tweaks
//-------------------------------------------------

s32 input_device_absolute_item::read_as_absolute(input_item_modifier modifier)
{
	// start with the current value
	s32 result = m_device.adjust_absolute(update_value());
	assert(result >= osd::input_device::ABSOLUTE_MIN && result <= osd::input_device::ABSOLUTE_MAX);

	// if we're doing a lightgun reload hack, override the value
	if (m_device.devclass() == DEVICE_CLASS_LIGHTGUN && m_device.lightgun_reload_button())
	{
		// if it is pressed, return (min,max)
		input_device_item *button2_item = m_device.item(ITEM_ID_BUTTON2);
		if (button2_item != nullptr && button2_item->update_value())
			result = (m_itemid == ITEM_ID_XAXIS) ? osd::input_device::ABSOLUTE_MIN : osd::input_device::ABSOLUTE_MAX;
	}

	// positive/negative: scale to full axis
	if (modifier == ITEM_MODIFIER_REVERSE)
		result = -result;
	else if (modifier == ITEM_MODIFIER_POS)
		result = std::max(result, 0) * 2 + osd::input_device::ABSOLUTE_MIN;
	else if (modifier == ITEM_MODIFIER_NEG)
		result = std::max(-result, 0) * 2 + osd::input_device::ABSOLUTE_MIN;
	return result;
}


//-------------------------------------------------
//  item_check_axis - see if axis has moved far
//  enough to trigger a read when polling
//-------------------------------------------------

bool input_device_absolute_item::item_check_axis(input_item_modifier modifier, s32 memory)
{
	// ignore min/max for lightguns
	// so the selection will not be affected by a gun going out of range
	const s32 curval = read_as_absolute(modifier);
	if (m_device.devclass() == DEVICE_CLASS_LIGHTGUN &&
		(curval == osd::input_device::ABSOLUTE_MAX || curval == osd::input_device::ABSOLUTE_MIN))
		return false;

	// for absolute axes, look for 25% of maximum
	return std::abs(curval - memory) > ((osd::input_device::ABSOLUTE_MAX - osd::input_device::ABSOLUTE_MIN) / 4);
}
