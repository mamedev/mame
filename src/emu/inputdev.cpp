// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    inputdev.cpp

    Input devices, items and device classes.

***************************************************************************/

#include "emu.h"
#include "emuopts.h"
#include "inputdev.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> input_device_switch_item

// derived input item representing a switch input
class input_device_switch_item : public input_device_item
{
public:
	// construction/destruction
	input_device_switch_item(input_device &device, const char *name, void *internal, input_item_id itemid, item_get_state_func getstate);

	// readers
	virtual s32 read_as_switch(input_item_modifier modifier) override;
	virtual s32 read_as_relative(input_item_modifier modifier) override;
	virtual s32 read_as_absolute(input_item_modifier modifier) override;

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
	input_device_relative_item(input_device &device, const char *name, void *internal, input_item_id itemid, item_get_state_func getstate);

	// readers
	virtual s32 read_as_switch(input_item_modifier modifier) override;
	virtual s32 read_as_relative(input_item_modifier modifier) override;
	virtual s32 read_as_absolute(input_item_modifier modifier) override;
};


// ======================> input_device_switch_item

// derived input item representing an absolute axis input
class input_device_absolute_item : public input_device_item
{
public:
	// construction/destruction
	input_device_absolute_item(input_device &device, const char *name, void *internal, input_item_id itemid, item_get_state_func getstate);

	// readers
	virtual s32 read_as_switch(input_item_modifier modifier) override;
	virtual s32 read_as_relative(input_item_modifier modifier) override;
	virtual s32 read_as_absolute(input_item_modifier modifier) override;
};


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
	xaxisval = ((xaxisval - INPUT_ABSOLUTE_MIN) * 9) / (INPUT_ABSOLUTE_MAX - INPUT_ABSOLUTE_MIN + 1);
	yaxisval = ((yaxisval - INPUT_ABSOLUTE_MIN) * 9) / (INPUT_ABSOLUTE_MAX - INPUT_ABSOLUTE_MIN + 1);
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

input_device::input_device(input_manager &manager, const char *name, const char *id, void *internal)
	: m_manager(manager),
		m_name(name),
		m_id(id),
		m_devindex(-1),
		m_maxitem(input_item_id(0)),
		m_internal(internal),
		m_steadykey_enabled(manager.machine().options().steadykey()),
		m_lightgun_reload_button(manager.machine().options().offscreen_reload())
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

input_item_id input_device::add_item(const char *name, input_item_id itemid, item_get_state_func getstate, void *internal)
{
	assert_always(machine().phase() == MACHINE_PHASE_INIT, "Can only call input_device::add_item at init time!");
	assert(name != nullptr);
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
			m_item[itemid] = std::make_unique<input_device_switch_item>(*this, name, internal, itemid, getstate);
			break;

		case ITEM_CLASS_RELATIVE:
			m_item[itemid] = std::make_unique<input_device_relative_item>(*this, name, internal, itemid, getstate);
			break;

		case ITEM_CLASS_ABSOLUTE:
			m_item[itemid] = std::make_unique<input_device_absolute_item>(*this, name, internal, itemid, getstate);
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
//  match_device_id - match device id via
//  substring search
//-------------------------------------------------

bool input_device::match_device_id(const char *deviceid)
{
	std::string deviceidupper(deviceid);
	std::string idupper(m_id);

	strmakeupper(deviceidupper);
	strmakeupper(idupper);

	return std::string::npos == idupper.find(deviceidupper) ? false : true;
}


//**************************************************************************
//  SPECIFIC INPUT DEVICES
//**************************************************************************

//-------------------------------------------------
//  input_device_keyboard - constructor
//-------------------------------------------------

input_device_keyboard::input_device_keyboard(input_manager &manager, const char *_name, const char *_id, void *_internal)
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

input_device_mouse::input_device_mouse(input_manager &manager, const char *_name, const char *_id, void *_internal)
	: input_device(manager, _name, _id, _internal)
{
}


//-------------------------------------------------
//  input_device_lightgun - constructor
//-------------------------------------------------

input_device_lightgun::input_device_lightgun(input_manager &manager, const char *_name, const char *_id, void *_internal)
	: input_device(manager, _name, _id, _internal)
{
}


//-------------------------------------------------
//  input_device_joystick - constructor
//-------------------------------------------------

input_device_joystick::input_device_joystick(input_manager &manager, const char *_name, const char *_id, void *_internal)
	: input_device(manager, _name, _id, _internal),
		m_joystick_deadzone(s32(manager.machine().options().joystick_deadzone() * INPUT_ABSOLUTE_MAX)),
		m_joystick_saturation(s32(manager.machine().options().joystick_saturation() * INPUT_ABSOLUTE_MAX))
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
		osd_printf_verbose("Input: Default joystick map = %s\n", m_joymap.to_string().c_str());
}


//-------------------------------------------------
//  adjust_absolute_value - apply joystick device
//  deadzone and saturation parameters to an
//  absolute value
//-------------------------------------------------

s32 input_device_joystick::adjust_absolute_value(s32 result) const
{
	// properties are symmetric
	bool negative = false;
	if (result < 0)
	{
		negative = true;
		result = -result;
	}

	// if in the deadzone, return 0
	if (result < m_joystick_deadzone)
		result = 0;

	// if saturated, return the max
	else if (result > m_joystick_saturation)
		result = INPUT_ABSOLUTE_MAX;

	// otherwise, scale
	else
		result = s64(result - m_joystick_deadzone) * s64(INPUT_ABSOLUTE_MAX) / s64(m_joystick_saturation - m_joystick_deadzone);

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

input_device *input_class::add_device(const char *name, const char *id, void *internal)
{
	assert_always(machine().phase() == MACHINE_PHASE_INIT, "Can only call input_class::add_device at init time!");
	assert(name != nullptr);
	assert(id != nullptr);

	// allocate a new device and add it to the index
	return add_device(make_device(name, id, internal));
}

input_device *input_class::add_device(std::unique_ptr<input_device> &&new_device)
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
				osd_printf_verbose("Input: Adding %s #%d: %s\n", m_name, devindex, new_device->name());
			else
				osd_printf_verbose("Input: Adding %s #%d: %s (device id: %s)\n", m_name, devindex, new_device->name(), new_device->id());

			m_device[devindex] = std::move(new_device);
			return m_device[devindex].get();
		}

	throw emu_fatalerror("Input: Too many %s devices\n", m_name);
}


//-------------------------------------------------
//  standard_item_class - return the class of a
//  standard item
//-------------------------------------------------

input_item_class input_class::standard_item_class(input_item_id itemid) const
{
	// most everything standard is a switch, apart from the axes
	if (itemid == ITEM_ID_OTHER_SWITCH || itemid < ITEM_ID_XAXIS || (itemid > ITEM_ID_SLIDER2 && itemid < ITEM_ID_ADD_ABSOLUTE1))
		return ITEM_CLASS_SWITCH;

	// standard mouse axes are relative
	else if (m_devclass == DEVICE_CLASS_MOUSE || itemid == ITEM_ID_OTHER_AXIS_RELATIVE || (itemid >= ITEM_ID_ADD_RELATIVE1 && itemid <= ITEM_ID_ADD_RELATIVE16))
		return ITEM_CLASS_RELATIVE;

	// all other standard axes are absolute
	else
		return ITEM_CLASS_ABSOLUTE;
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

	// update the maximum index found, since newindex may
	// exceed current m_maxindex
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
//  class_add_type_seq - provide an input sequence
//  of this class for the given input type
//-------------------------------------------------

void input_class_keyboard::class_add_type_seq(input_type_entry &entry)
{
	switch (entry.group())
	{
		case IPG_PLAYER1:
			switch (entry.type())
			{
				case IPT_JOYSTICK_UP:
					entry.defseq() |= KEYCODE_UP;
					break;
				case IPT_JOYSTICK_DOWN:
					entry.defseq() |= KEYCODE_DOWN;
					break;
				case IPT_JOYSTICK_LEFT:
					entry.defseq() |= KEYCODE_LEFT;
					break;
				case IPT_JOYSTICK_RIGHT:
					entry.defseq() |= KEYCODE_RIGHT;
					break;
				case IPT_JOYSTICKRIGHT_UP:
					entry.defseq() |= KEYCODE_I;
					break;
				case IPT_JOYSTICKRIGHT_DOWN:
					entry.defseq() |= KEYCODE_K;
					break;
				case IPT_JOYSTICKRIGHT_LEFT:
					entry.defseq() |= KEYCODE_J;
					break;
				case IPT_JOYSTICKRIGHT_RIGHT:
					entry.defseq() |= KEYCODE_L;
					break;
				case IPT_JOYSTICKLEFT_UP:
					entry.defseq() |= KEYCODE_E;
					break;
				case IPT_JOYSTICKLEFT_DOWN:
					entry.defseq() |= KEYCODE_D;
					break;
				case IPT_JOYSTICKLEFT_LEFT:
					entry.defseq() |= KEYCODE_S;
					break;
				case IPT_JOYSTICKLEFT_RIGHT:
					entry.defseq() |= KEYCODE_F;
					break;
				case IPT_BUTTON1:
				case IPT_MAHJONG_KAN:
					entry.defseq() |= KEYCODE_LCONTROL;
					break;
				case IPT_BUTTON2:
				case IPT_MAHJONG_PON:
					entry.defseq() |= KEYCODE_LALT;
					break;
				case IPT_BUTTON3:
				case IPT_MAHJONG_CHI:
					entry.defseq() |= KEYCODE_SPACE;
					break;
				case IPT_BUTTON4:
				case IPT_MAHJONG_REACH:
					entry.defseq() |= KEYCODE_LSHIFT;
					break;
				case IPT_BUTTON5:
				case IPT_MAHJONG_RON:
				case IPT_POKER_HOLD1:
				case IPT_SLOT_STOP_ALL:
					entry.defseq() |= KEYCODE_Z;
					break;
				case IPT_BUTTON6:
				case IPT_POKER_HOLD2:
				case IPT_SLOT_STOP1:
					entry.defseq() |= KEYCODE_X;
					break;
				case IPT_BUTTON7:
				case IPT_POKER_HOLD3:
				case IPT_SLOT_STOP2:
					entry.defseq() |= KEYCODE_C;
					break;
				case IPT_BUTTON8:
				case IPT_POKER_HOLD4:
				case IPT_SLOT_STOP3:
					entry.defseq() |= KEYCODE_V;
					break;
				case IPT_BUTTON9:
				case IPT_POKER_HOLD5:
				case IPT_SLOT_STOP4:
					entry.defseq() |= KEYCODE_B;
					break;
				case IPT_BUTTON10:
				case IPT_POKER_CANCEL:
					entry.defseq() |= KEYCODE_N;
					break;
				case IPT_BUTTON11:
				case IPT_GAMBLE_BET:
					entry.defseq() |= KEYCODE_M;
					break;
				case IPT_BUTTON12:
					entry.defseq() |= KEYCODE_COMMA;
					break;
				case IPT_BUTTON13:
					entry.defseq() |= KEYCODE_STOP;
					break;
				case IPT_BUTTON14:
					entry.defseq() |= KEYCODE_SLASH;
					break;
				case IPT_BUTTON15:
					entry.defseq() |= KEYCODE_RSHIFT;
					break;
				case IPT_START:
				case IPT_POKER_BET:
					entry.defseq() |= KEYCODE_1;
					break;
				case IPT_SELECT:
					entry.defseq() |= KEYCODE_5;
					break;
				case IPT_MAHJONG_A:
				case IPT_HANAFUDA_A:
					entry.defseq() |= KEYCODE_A;
					break;
				case IPT_MAHJONG_B:
				case IPT_HANAFUDA_B:
					entry.defseq() |= KEYCODE_B;
					break;
				case IPT_MAHJONG_C:
				case IPT_HANAFUDA_C:
					entry.defseq() |= KEYCODE_C;
					break;
				case IPT_MAHJONG_D:
				case IPT_HANAFUDA_D:
					entry.defseq() |= KEYCODE_D;
					break;
				case IPT_MAHJONG_E:
				case IPT_HANAFUDA_E:
					entry.defseq() |= KEYCODE_E;
					break;
				case IPT_MAHJONG_F:
				case IPT_HANAFUDA_F:
					entry.defseq() |= KEYCODE_F;
					break;
				case IPT_MAHJONG_G:
				case IPT_HANAFUDA_G:
					entry.defseq() |= KEYCODE_G;
					break;
				case IPT_MAHJONG_H:
				case IPT_HANAFUDA_H:
					entry.defseq() |= KEYCODE_H;
					break;
				case IPT_MAHJONG_I:
					entry.defseq() |= KEYCODE_I;
					break;
				case IPT_MAHJONG_J:
					entry.defseq() |= KEYCODE_J;
					break;
				case IPT_MAHJONG_K:
					entry.defseq() |= KEYCODE_K;
					break;
				case IPT_MAHJONG_L:
					entry.defseq() |= KEYCODE_L;
					break;
				case IPT_MAHJONG_M:
				case IPT_HANAFUDA_YES:
					entry.defseq() |= KEYCODE_M;
					break;
				case IPT_MAHJONG_N:
				case IPT_HANAFUDA_NO:
					entry.defseq() |= KEYCODE_N;
					break;
				case IPT_MAHJONG_O:
					entry.defseq() |= KEYCODE_O;
					break;
				case IPT_MAHJONG_P:
					entry.defseq() |= KEYCODE_COLON;
					break;
				case IPT_MAHJONG_Q:
					entry.defseq() |= KEYCODE_Q;
					break;
				case IPT_MAHJONG_BET:
					entry.defseq() |= KEYCODE_3;
					break;
				case IPT_MAHJONG_LAST_CHANCE:
					entry.defseq() |= KEYCODE_RALT;
					break;
				case IPT_MAHJONG_SCORE:
					entry.defseq() |= KEYCODE_RCONTROL;
					break;
				case IPT_MAHJONG_DOUBLE_UP:
					entry.defseq() |= KEYCODE_RSHIFT;
					break;
				case IPT_MAHJONG_FLIP_FLOP:
					entry.defseq() |= KEYCODE_Y;
					break;
				case IPT_MAHJONG_BIG:
					entry.defseq() |= KEYCODE_ENTER;
					break;
				case IPT_MAHJONG_SMALL:
					entry.defseq() |= KEYCODE_BACKSPACE;
					break;
				case IPT_GAMBLE_HIGH:
					entry.defseq() |= KEYCODE_A;
					break;
				case IPT_GAMBLE_LOW:
					entry.defseq() |= KEYCODE_S;
					break;
				case IPT_GAMBLE_HALF:
					entry.defseq() |= KEYCODE_D;
					break;
				case IPT_GAMBLE_DEAL:
					entry.defseq() |= KEYCODE_2;
					break;
				case IPT_GAMBLE_D_UP:
					entry.defseq() |= KEYCODE_3;
					break;
				case IPT_GAMBLE_TAKE:
					entry.defseq() |= KEYCODE_4;
					break;
				case IPT_GAMBLE_STAND:
					entry.defseq() |= KEYCODE_L;
					break;
				case IPT_GAMBLE_KEYIN:
					entry.defseq() |= KEYCODE_Q;
					break;
				case IPT_GAMBLE_KEYOUT:
					entry.defseq() |= KEYCODE_W;
					break;
				case IPT_GAMBLE_PAYOUT:
					entry.defseq() |= KEYCODE_I;
					break;
				case IPT_GAMBLE_DOOR:
					entry.defseq() |= KEYCODE_O;
					break;
				case IPT_GAMBLE_SERVICE:
					entry.defseq() |= KEYCODE_9;
					break;
				case IPT_GAMBLE_BOOK:
					entry.defseq() |= KEYCODE_0;
					break;
				case IPT_PEDAL:
					entry.defseq(SEQ_TYPE_INCREMENT) |= KEYCODE_LCONTROL;
					break;
				case IPT_PEDAL2:
					entry.defseq(SEQ_TYPE_INCREMENT) |= KEYCODE_LALT;
					break;
				case IPT_PEDAL3:
					entry.defseq(SEQ_TYPE_INCREMENT) |= KEYCODE_SPACE;
					break;
				case IPT_PADDLE:
				case IPT_POSITIONAL:
				case IPT_DIAL:
				case IPT_TRACKBALL_X:
				case IPT_AD_STICK_X:
				case IPT_LIGHTGUN_X:
				case IPT_MOUSE_X:
					entry.defseq(SEQ_TYPE_DECREMENT) |= KEYCODE_LEFT;
					entry.defseq(SEQ_TYPE_INCREMENT) |= KEYCODE_RIGHT;
					break;
				case IPT_PADDLE_V:
				case IPT_POSITIONAL_V:
				case IPT_DIAL_V:
				case IPT_TRACKBALL_Y:
				case IPT_AD_STICK_Y:
				case IPT_LIGHTGUN_Y:
				case IPT_MOUSE_Y:
					entry.defseq(SEQ_TYPE_DECREMENT) |= KEYCODE_UP;
					entry.defseq(SEQ_TYPE_INCREMENT) |= KEYCODE_DOWN;
					break;
				case IPT_AD_STICK_Z:
					entry.defseq(SEQ_TYPE_DECREMENT) |= KEYCODE_A;
					entry.defseq(SEQ_TYPE_INCREMENT) |= KEYCODE_Z;
					break;
				default:
					break;
			}
			break;
		case IPG_PLAYER2:
			switch (entry.type())
			{
				case IPT_JOYSTICK_UP:
					entry.defseq() |= KEYCODE_R;
					break;
				case IPT_JOYSTICK_DOWN:
					entry.defseq() |= KEYCODE_F;
					break;
				case IPT_JOYSTICK_LEFT:
					entry.defseq() |= KEYCODE_D;
					break;
				case IPT_JOYSTICK_RIGHT:
					entry.defseq() |= KEYCODE_G;
					break;
				case IPT_BUTTON1:
					entry.defseq() |= KEYCODE_A;
					break;
				case IPT_BUTTON2:
					entry.defseq() |= KEYCODE_S;
					break;
				case IPT_BUTTON3:
					entry.defseq() |= KEYCODE_Q;
					break;
				case IPT_BUTTON4:
					entry.defseq() |= KEYCODE_W;
					break;
				case IPT_START:
					entry.defseq() |= KEYCODE_2;
					break;
				case IPT_SELECT:
					entry.defseq() |= KEYCODE_6;
					break;
				case IPT_PEDAL:
					entry.defseq(SEQ_TYPE_INCREMENT) |= KEYCODE_A;
					break;
				case IPT_PEDAL2:
					entry.defseq(SEQ_TYPE_INCREMENT) |= KEYCODE_S;
					break;
				case IPT_PEDAL3:
					entry.defseq(SEQ_TYPE_INCREMENT) |= KEYCODE_Q;
					break;
				case IPT_PADDLE:
				case IPT_POSITIONAL:
				case IPT_DIAL:
				case IPT_TRACKBALL_X:
				case IPT_AD_STICK_X:
				case IPT_LIGHTGUN_X:
				case IPT_MOUSE_X:
					entry.defseq(SEQ_TYPE_DECREMENT) |= KEYCODE_D;
					entry.defseq(SEQ_TYPE_INCREMENT) |= KEYCODE_G;
					break;
				case IPT_PADDLE_V:
				case IPT_POSITIONAL_V:
				case IPT_DIAL_V:
				case IPT_TRACKBALL_Y:
				case IPT_AD_STICK_Y:
				case IPT_LIGHTGUN_Y:
				case IPT_MOUSE_Y:
					entry.defseq(SEQ_TYPE_DECREMENT) |= KEYCODE_R;
					entry.defseq(SEQ_TYPE_INCREMENT) |= KEYCODE_F;
					break;
				default:
					break;
			}
			break;
		case IPG_PLAYER3:
			switch (entry.type())
			{
				case IPT_JOYSTICK_UP:
					entry.defseq() |= KEYCODE_I;
					break;
				case IPT_JOYSTICK_DOWN:
					entry.defseq() |= KEYCODE_K;
					break;
				case IPT_JOYSTICK_LEFT:
					entry.defseq() |= KEYCODE_J;
					break;
				case IPT_JOYSTICK_RIGHT:
					entry.defseq() |= KEYCODE_L;
					break;
				case IPT_BUTTON1:
					entry.defseq() |= KEYCODE_RCONTROL;
					break;
				case IPT_BUTTON2:
					entry.defseq() |= KEYCODE_RSHIFT;
					break;
				case IPT_BUTTON3:
					entry.defseq() |= KEYCODE_ENTER;
					break;
				case IPT_START:
					entry.defseq() |= KEYCODE_3;
					break;
				case IPT_SELECT:
					entry.defseq() |= KEYCODE_7;
					break;
				case IPT_PEDAL:
					entry.defseq(SEQ_TYPE_INCREMENT) |= KEYCODE_RCONTROL;
					break;
				case IPT_PEDAL2:
					entry.defseq(SEQ_TYPE_INCREMENT) |= KEYCODE_RSHIFT;
					break;
				case IPT_PEDAL3:
					entry.defseq(SEQ_TYPE_INCREMENT) |= KEYCODE_ENTER;
					break;
				case IPT_PADDLE:
				case IPT_POSITIONAL:
				case IPT_DIAL:
				case IPT_TRACKBALL_X:
				case IPT_AD_STICK_X:
				case IPT_LIGHTGUN_X:
				case IPT_MOUSE_X:
					entry.defseq(SEQ_TYPE_DECREMENT) |= KEYCODE_J;
					entry.defseq(SEQ_TYPE_INCREMENT) |= KEYCODE_L;
					break;
				case IPT_PADDLE_V:
				case IPT_POSITIONAL_V:
				case IPT_DIAL_V:
				case IPT_TRACKBALL_Y:
				case IPT_AD_STICK_Y:
				case IPT_LIGHTGUN_Y:
				case IPT_MOUSE_Y:
					entry.defseq(SEQ_TYPE_DECREMENT) |= KEYCODE_I;
					entry.defseq(SEQ_TYPE_INCREMENT) |= KEYCODE_K;
					break;
				default:
					break;
			}
			break;
		case IPG_PLAYER4:
			switch (entry.type())
			{
				case IPT_JOYSTICK_UP:
					entry.defseq() |= KEYCODE_8_PAD;
					break;
				case IPT_JOYSTICK_DOWN:
					entry.defseq() |= KEYCODE_2_PAD;
					break;
				case IPT_JOYSTICK_LEFT:
					entry.defseq() |= KEYCODE_4_PAD;
					break;
				case IPT_JOYSTICK_RIGHT:
					entry.defseq() |= KEYCODE_6_PAD;
					break;
				case IPT_BUTTON1:
					entry.defseq() |= KEYCODE_0_PAD;
					break;
				case IPT_BUTTON2:
					entry.defseq() |= KEYCODE_DEL_PAD;
					break;
				case IPT_BUTTON3:
					entry.defseq() |= KEYCODE_ENTER_PAD;
					break;
				case IPT_START:
					entry.defseq() |= KEYCODE_4;
					break;
				case IPT_SELECT:
					entry.defseq() |= KEYCODE_8;
					break;
				case IPT_PEDAL:
					entry.defseq(SEQ_TYPE_INCREMENT) |= KEYCODE_0_PAD;
					break;
				case IPT_PEDAL2:
					entry.defseq(SEQ_TYPE_INCREMENT) |= KEYCODE_DEL_PAD;
					break;
				case IPT_PEDAL3:
					entry.defseq(SEQ_TYPE_INCREMENT) |= KEYCODE_ENTER_PAD;
					break;
				default:
					break;
			}
			break;
		case IPG_OTHER:
			switch (entry.type())
			{
				case IPT_START1:
					entry.defseq() |= KEYCODE_1;
					break;
				case IPT_START2:
					entry.defseq() |= KEYCODE_2;
					break;
				case IPT_START3:
					entry.defseq() |= KEYCODE_3;
					break;
				case IPT_START4:
					entry.defseq() |= KEYCODE_4;
					break;
				case IPT_COIN1:
					entry.defseq() |= KEYCODE_5;
					break;
				case IPT_COIN2:
					entry.defseq() |= KEYCODE_6;
					break;
				case IPT_COIN3:
					entry.defseq() |= KEYCODE_7;
					break;
				case IPT_COIN4:
					entry.defseq() |= KEYCODE_8;
					break;
				case IPT_BILL1:
					entry.defseq() |= KEYCODE_BACKSPACE;
					break;
				case IPT_SERVICE1:
					entry.defseq() |= KEYCODE_9;
					break;
				case IPT_SERVICE2:
					entry.defseq() |= KEYCODE_0;
					break;
				case IPT_SERVICE3:
				case IPT_VOLUME_DOWN:
					entry.defseq() |= KEYCODE_MINUS;
					break;
				case IPT_SERVICE4:
				case IPT_VOLUME_UP:
					entry.defseq() |= KEYCODE_EQUALS;
					break;
				case IPT_TILT:
				case IPT_TILT1:
					entry.defseq() |= KEYCODE_T;
					break;
				case IPT_POWER_ON:
				case IPT_MEMORY_RESET:
					entry.defseq() |= KEYCODE_F1;
					break;
				case IPT_POWER_OFF:
				case IPT_SERVICE:
					entry.defseq() |= KEYCODE_F2;
					break;
				default:
					break;
			}
			break;
		case IPG_UI:
			switch (entry.type())
			{
				case IPT_UI_ON_SCREEN_DISPLAY:
				case IPT_UI_DEBUG_BREAK:
					entry.defseq() |= KEYCODE_TILDE;
					break;
				case IPT_UI_CONFIGURE:
					entry.defseq() |= KEYCODE_TAB;
					break;
				case IPT_UI_PAUSE:
					entry.defseq() |= KEYCODE_P;
					entry.defseq() += input_seq::not_code;
					entry.defseq() += KEYCODE_LSHIFT;
					entry.defseq() += input_seq::not_code;
					entry.defseq() += KEYCODE_RSHIFT;
					break;
				case IPT_UI_PAUSE_SINGLE:
					entry.defseq() |= KEYCODE_P;
					entry.defseq() += KEYCODE_LSHIFT;
					entry.defseq() |= KEYCODE_P;
					entry.defseq() += KEYCODE_RSHIFT;
					break;
				case IPT_UI_RESET_MACHINE:
					entry.defseq() |= KEYCODE_F3;
					entry.defseq() += KEYCODE_LSHIFT;
					break;
				case IPT_UI_SOFT_RESET:
					entry.defseq() |= KEYCODE_F3;
					entry.defseq() += input_seq::not_code;
					entry.defseq() += KEYCODE_LSHIFT;
					break;
				case IPT_UI_SHOW_GFX:
					entry.defseq() |= KEYCODE_F4;
					break;
				case IPT_UI_FRAMESKIP_DEC:
					entry.defseq() |= KEYCODE_F8;
					break;
				case IPT_UI_FRAMESKIP_INC:
					entry.defseq() |= KEYCODE_F9;
					break;
				case IPT_UI_THROTTLE:
					entry.defseq() |= KEYCODE_F10;
					break;
				case IPT_UI_FAST_FORWARD:
					entry.defseq() |= KEYCODE_INSERT;
					break;
				case IPT_UI_SHOW_FPS:
					entry.defseq() |= KEYCODE_F11;
					entry.defseq() += input_seq::not_code;
					entry.defseq() += KEYCODE_LSHIFT;
					break;
				case IPT_UI_SNAPSHOT:
				case IPT_UI_TIMECODE:
					entry.defseq() |= KEYCODE_F12;
					entry.defseq() += input_seq::not_code;
					entry.defseq() += KEYCODE_LSHIFT;
					break;
				case IPT_UI_RECORD_MOVIE:
					entry.defseq() |= KEYCODE_F12;
					entry.defseq() += KEYCODE_LSHIFT;
					break;
				case IPT_UI_TOGGLE_CHEAT:
					entry.defseq() |= KEYCODE_F6;
					break;
				case IPT_UI_UP:
					entry.defseq() |= KEYCODE_UP;
					break;
				case IPT_UI_DOWN:
					entry.defseq() |= KEYCODE_DOWN;
					break;
				case IPT_UI_LEFT:
					entry.defseq() |= KEYCODE_LEFT;
					break;
				case IPT_UI_RIGHT:
					entry.defseq() |= KEYCODE_RIGHT;
					break;
				case IPT_UI_HOME:
					entry.defseq() |= KEYCODE_HOME;
					break;
				case IPT_UI_END:
					entry.defseq() |= KEYCODE_END;
					break;
				case IPT_UI_PAGE_UP:
					entry.defseq() |= KEYCODE_PGUP;
					break;
				case IPT_UI_PAGE_DOWN:
					entry.defseq() |= KEYCODE_PGDN;
					break;
				case IPT_UI_SELECT:
					entry.defseq() |= KEYCODE_ENTER;
					entry.defseq() |= KEYCODE_ENTER_PAD;
					break;
				case IPT_UI_CANCEL:
					entry.defseq() |= KEYCODE_ESC;
					break;
				case IPT_UI_DISPLAY_COMMENT:
					entry.defseq() |= KEYCODE_SPACE;
					break;
				case IPT_UI_CLEAR:
					entry.defseq() |= KEYCODE_DEL;
					break;
				case IPT_UI_ZOOM_IN:
					entry.defseq() |= KEYCODE_EQUALS;
					break;
				case IPT_UI_ZOOM_OUT:
					entry.defseq() |= KEYCODE_MINUS;
					break;
				case IPT_UI_PREV_GROUP:
					entry.defseq() |= KEYCODE_OPENBRACE;
					break;
				case IPT_UI_NEXT_GROUP:
					entry.defseq() |= KEYCODE_CLOSEBRACE;
					break;
				case IPT_UI_ROTATE:
					entry.defseq() |= KEYCODE_R;
					break;
				case IPT_UI_SHOW_PROFILER:
					entry.defseq() |= KEYCODE_F11;
					entry.defseq() += KEYCODE_LSHIFT;
					break;
				case IPT_UI_TOGGLE_UI:
					entry.defseq() |= KEYCODE_SCRLOCK;
					entry.defseq() += input_seq::not_code;
					entry.defseq() += KEYCODE_LSHIFT;
					break;
				case IPT_UI_PASTE:
					entry.defseq() |= KEYCODE_SCRLOCK;
					entry.defseq() += KEYCODE_LSHIFT;
					break;
				case IPT_UI_TOGGLE_DEBUG:
					entry.defseq() |= KEYCODE_F5;
					break;
				case IPT_UI_SAVE_STATE:
					entry.defseq() |= KEYCODE_F7;
					entry.defseq() += KEYCODE_LSHIFT;
					break;
				case IPT_UI_LOAD_STATE:
					entry.defseq() |= KEYCODE_F7;
					entry.defseq() += input_seq::not_code;
					entry.defseq() += KEYCODE_LSHIFT;
					break;
				case IPT_UI_TAPE_START:
					entry.defseq() |= KEYCODE_F2;
					entry.defseq() += input_seq::not_code;
					entry.defseq() += KEYCODE_LSHIFT;
					break;
				case IPT_UI_TAPE_STOP:
					entry.defseq() |= KEYCODE_F2;
					entry.defseq() += KEYCODE_LSHIFT;
					break;
				case IPT_UI_DATS:
					entry.defseq() |= KEYCODE_LALT;
					entry.defseq() += KEYCODE_D;
					break;
				case IPT_UI_FAVORITES:
					entry.defseq() |= KEYCODE_LALT;
					entry.defseq() += KEYCODE_F;
					break;
				case IPT_UI_EXPORT:
					entry.defseq() |= KEYCODE_LALT;
					entry.defseq() += KEYCODE_E;
					break;
				case IPT_UI_AUDIT_FAST:
					entry.defseq() |= KEYCODE_F1;
					entry.defseq() += input_seq::not_code;
					entry.defseq() += KEYCODE_LSHIFT;
					break;
				case IPT_UI_AUDIT_ALL:
					entry.defseq() |= KEYCODE_F1;
					entry.defseq() += KEYCODE_LSHIFT;
					break;
				default:
					break;
			}
			break;
		default:
			break;
	}
}


//-------------------------------------------------
//  input_class_mouse - constructor
//-------------------------------------------------

input_class_mouse::input_class_mouse(input_manager &manager)
	: input_class(manager, DEVICE_CLASS_MOUSE, "mouse", manager.machine().options().mouse(), manager.machine().options().multi_mouse())
{
}


//-------------------------------------------------
//  class_add_type_seq - provide an input sequence
//  of this class for the given input type
//-------------------------------------------------

void input_class_mouse::class_add_type_seq(input_type_entry &entry)
{
	if (entry.group() >= IPG_PLAYER1 && entry.group() <= IPG_PLAYER10)
	{
		int p = entry.player();
		switch (entry.type())
		{
			case IPT_BUTTON1:
				if (p < 2)
					entry.defseq() |= MOUSECODE_BUTTON1_INDEXED(p);
				break;
			case IPT_BUTTON2:
				if (p < 2)
					entry.defseq() |= MOUSECODE_BUTTON3_INDEXED(p);
				break;
			case IPT_BUTTON3:
				if (p < 2)
					entry.defseq() |= MOUSECODE_BUTTON2_INDEXED(p);
				break;
			case IPT_PADDLE:
			case IPT_POSITIONAL:
			case IPT_DIAL:
			case IPT_TRACKBALL_X:
			case IPT_AD_STICK_X:
			case IPT_LIGHTGUN_X:
			case IPT_MOUSE_X:
				entry.defseq(SEQ_TYPE_STANDARD) |= MOUSECODE_X_INDEXED(p);
				break;
			case IPT_PADDLE_V:
			case IPT_POSITIONAL_V:
			case IPT_DIAL_V:
			case IPT_TRACKBALL_Y:
			case IPT_AD_STICK_Y:
			case IPT_LIGHTGUN_Y:
			case IPT_MOUSE_Y:
				entry.defseq(SEQ_TYPE_STANDARD) |= MOUSECODE_Y_INDEXED(p);
				break;
			default:
				break;
		}
	}
}


//-------------------------------------------------
//  input_class_lightgun - constructor
//-------------------------------------------------

input_class_lightgun::input_class_lightgun(input_manager &manager)
	: input_class(manager, DEVICE_CLASS_LIGHTGUN, "lightgun", manager.machine().options().lightgun(), true)
{
}


//-------------------------------------------------
//  class_add_type_seq - provide an input sequence
//  of this class for the given input type
//-------------------------------------------------

void input_class_lightgun::class_add_type_seq(input_type_entry &entry)
{
	if (entry.group() >= IPG_PLAYER1 && entry.group() <= IPG_PLAYER10)
	{
		int p = entry.player();
		switch (entry.type())
		{
			case IPT_BUTTON1:
				if (p < 3)
					entry.defseq() |= GUNCODE_BUTTON1_INDEXED(p);
				break;
			case IPT_BUTTON2:
				if (p < 3)
					entry.defseq() |= GUNCODE_BUTTON2_INDEXED(p);
				break;
			case IPT_LIGHTGUN_X:
				entry.defseq(SEQ_TYPE_STANDARD) |= GUNCODE_X_INDEXED(p);
				break;
			case IPT_LIGHTGUN_Y:
				entry.defseq(SEQ_TYPE_STANDARD) |= GUNCODE_Y_INDEXED(p);
				break;
			default:
				break;
		}
	}
}


//-------------------------------------------------
//  input_class_joystick - constructor
//-------------------------------------------------

input_class_joystick::input_class_joystick(input_manager &manager)
	: input_class(manager, DEVICE_CLASS_JOYSTICK, "joystick", manager.machine().options().joystick(), true)
{
}


//-------------------------------------------------
//  class_add_type_seq - provide an input sequence
//  of this class for the given input type
//-------------------------------------------------

void input_class_joystick::class_add_type_seq(input_type_entry &entry)
{
	if (entry.group() >= IPG_PLAYER1 && entry.group() <= IPG_PLAYER10)
	{
		int p = entry.player();
		switch (entry.type())
		{
			case IPT_JOYSTICK_UP:
				entry.defseq() |= JOYCODE_Y_UP_SWITCH_INDEXED(p);
				break;
			case IPT_JOYSTICK_DOWN:
				entry.defseq() |= JOYCODE_Y_DOWN_SWITCH_INDEXED(p);
				break;
			case IPT_JOYSTICK_RIGHT:
				entry.defseq() |= JOYCODE_X_LEFT_SWITCH_INDEXED(p);
				break;
			case IPT_JOYSTICK_LEFT:
				entry.defseq() |= JOYCODE_X_RIGHT_SWITCH_INDEXED(p);
				break;
			case IPT_JOYSTICKLEFT_UP:
				if (p == 0)
					entry.defseq() |= JOYCODE_Y_UP_SWITCH_INDEXED(p);
				break;
			case IPT_JOYSTICKLEFT_DOWN:
				if (p == 0)
					entry.defseq() |= JOYCODE_Y_DOWN_SWITCH_INDEXED(p);
				break;
			case IPT_JOYSTICKLEFT_RIGHT:
				if (p == 0)
					entry.defseq() |= JOYCODE_X_LEFT_SWITCH_INDEXED(p);
				break;
			case IPT_JOYSTICKLEFT_LEFT:
				if (p == 0)
					entry.defseq() |= JOYCODE_X_RIGHT_SWITCH_INDEXED(p);
				break;
			case IPT_JOYSTICKRIGHT_UP:
				if (p == 0)
					entry.defseq() |= JOYCODE_BUTTON2_INDEXED(p);
				break;
			case IPT_JOYSTICKRIGHT_DOWN:
				if (p == 0)
					entry.defseq() |= JOYCODE_BUTTON3_INDEXED(p);
				break;
			case IPT_JOYSTICKRIGHT_LEFT:
				if (p == 0)
					entry.defseq() |= JOYCODE_BUTTON1_INDEXED(p);
				break;
			case IPT_JOYSTICKRIGHT_RIGHT:
				if (p == 0)
					entry.defseq() |= JOYCODE_BUTTON4_INDEXED(p);
				break;
			case IPT_BUTTON1:
				entry.defseq() |= JOYCODE_BUTTON1_INDEXED(p);
				break;
			case IPT_BUTTON2:
				entry.defseq() |= JOYCODE_BUTTON2_INDEXED(p);
				break;
			case IPT_BUTTON3:
				entry.defseq() |= JOYCODE_BUTTON3_INDEXED(p);
				break;
			case IPT_BUTTON4:
				entry.defseq() |= JOYCODE_BUTTON4_INDEXED(p);
				break;
			case IPT_BUTTON5:
				entry.defseq() |= JOYCODE_BUTTON5_INDEXED(p);
				break;
			case IPT_BUTTON6:
				entry.defseq() |= JOYCODE_BUTTON6_INDEXED(p);
				break;
			case IPT_BUTTON7:
				entry.defseq() |= JOYCODE_BUTTON7_INDEXED(p);
				break;
			case IPT_BUTTON8:
				entry.defseq() |= JOYCODE_BUTTON8_INDEXED(p);
				break;
			case IPT_BUTTON9:
				entry.defseq() |= JOYCODE_BUTTON9_INDEXED(p);
				break;
			case IPT_BUTTON10:
				entry.defseq() |= JOYCODE_BUTTON10_INDEXED(p);
				break;
			case IPT_BUTTON11:
				entry.defseq() |= JOYCODE_BUTTON11_INDEXED(p);
				break;
			case IPT_BUTTON12:
				entry.defseq() |= JOYCODE_BUTTON12_INDEXED(p);
				break;
			case IPT_BUTTON13:
				entry.defseq() |= JOYCODE_BUTTON13_INDEXED(p);
				break;
			case IPT_BUTTON14:
				entry.defseq() |= JOYCODE_BUTTON14_INDEXED(p);
				break;
			case IPT_BUTTON15:
				entry.defseq() |= JOYCODE_BUTTON15_INDEXED(p);
				break;
			case IPT_BUTTON16:
				entry.defseq() |= JOYCODE_BUTTON16_INDEXED(p);
				break;
			case IPT_START:
				entry.defseq() |= JOYCODE_START_INDEXED(p);
				break;
			case IPT_SELECT:
				entry.defseq() |= JOYCODE_SELECT_INDEXED(p);
				break;
			case IPT_PEDAL:
				entry.defseq(SEQ_TYPE_STANDARD) |= JOYCODE_Z_NEG_ABSOLUTE_INDEXED(p);
				entry.defseq(SEQ_TYPE_INCREMENT) |= JOYCODE_BUTTON1_INDEXED(p);
				break;
			case IPT_PEDAL2:
				entry.defseq(SEQ_TYPE_STANDARD) |= JOYCODE_Z_POS_ABSOLUTE_INDEXED(p);
				entry.defseq(SEQ_TYPE_INCREMENT) |= JOYCODE_BUTTON2_INDEXED(p);
				break;
			case IPT_PEDAL3:
				entry.defseq(SEQ_TYPE_INCREMENT) |= JOYCODE_BUTTON3_INDEXED(p);
				break;
			case IPT_PADDLE:
			case IPT_POSITIONAL:
			case IPT_DIAL:
			case IPT_TRACKBALL_X:
			case IPT_AD_STICK_X:
			case IPT_LIGHTGUN_X:
				entry.defseq(SEQ_TYPE_STANDARD) |= JOYCODE_X_INDEXED(p);
				break;
			case IPT_PADDLE_V:
			case IPT_POSITIONAL_V:
			case IPT_DIAL_V:
			case IPT_TRACKBALL_Y:
			case IPT_AD_STICK_Y:
			case IPT_LIGHTGUN_Y:
				entry.defseq(SEQ_TYPE_STANDARD) |= JOYCODE_Y_INDEXED(p);
				break;
			case IPT_AD_STICK_Z:
				entry.defseq(SEQ_TYPE_STANDARD) |= JOYCODE_Z_INDEXED(p);
				break;
			default:
				break;
		}
	}
	else
	{
		switch (entry.type())
		{
			case IPT_START1:
				entry.defseq() |= JOYCODE_START_INDEXED(0);
				break;
			case IPT_START2:
				entry.defseq() |= JOYCODE_START_INDEXED(1);
				break;
			case IPT_START3:
				entry.defseq() |= JOYCODE_START_INDEXED(2);
				break;
			case IPT_START4:
				entry.defseq() |= JOYCODE_START_INDEXED(3);
				break;
			case IPT_COIN1:
				entry.defseq() |= JOYCODE_SELECT_INDEXED(0);
				break;
			case IPT_COIN2:
				entry.defseq() |= JOYCODE_SELECT_INDEXED(1);
				break;
			case IPT_COIN3:
				entry.defseq() |= JOYCODE_SELECT_INDEXED(2);
				break;
			case IPT_COIN4:
				entry.defseq() |= JOYCODE_SELECT_INDEXED(3);
				break;
			case IPT_UI_UP:
				entry.defseq() |= JOYCODE_Y_UP_SWITCH_INDEXED(0);
				break;
			case IPT_UI_DOWN:
				entry.defseq() |= JOYCODE_Y_DOWN_SWITCH_INDEXED(0);
				break;
			case IPT_UI_LEFT:
				entry.defseq() |= JOYCODE_X_LEFT_SWITCH_INDEXED(0);
				break;
			case IPT_UI_RIGHT:
				entry.defseq() |= JOYCODE_X_RIGHT_SWITCH_INDEXED(0);
				break;
			case IPT_UI_SELECT:
				entry.defseq() |= JOYCODE_BUTTON1_INDEXED(0);
				break;
			default:
				break;
		}
	}
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

	osd_printf_verbose("Input: Changing default joystick map = %s\n", map.to_string().c_str());

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

input_device_item::input_device_item(input_device &device, const char *name, void *internal, input_item_id itemid, item_get_state_func getstate, input_item_class itemclass)
	: m_device(device),
		m_name(name),
		m_internal(internal),
		m_itemid(itemid),
		m_itemclass(itemclass),
		m_getstate(getstate),
		m_current(0),
		m_memory(0)
{
	// use a standard token name for know item IDs
	const char *standard_token = manager().standard_token(itemid);
	if (standard_token != nullptr)
		m_token.assign(standard_token);

	// otherwise, create a tokenized name
	else {
		m_token.assign(name);
		strmakeupper(m_token);
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



//**************************************************************************
//  INPUT DEVICE SWITCH ITEM
//**************************************************************************

//-------------------------------------------------
//  input_device_switch_item - constructor
//-------------------------------------------------

input_device_switch_item::input_device_switch_item(input_device &device, const char *name, void *internal, input_item_id itemid, item_get_state_func getstate)
	: input_device_item(device, name, internal, itemid, getstate, ITEM_CLASS_SWITCH),
		m_steadykey(0),
		m_oldkey(0)
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

input_device_relative_item::input_device_relative_item(input_device &device, const char *name, void *internal, input_item_id itemid, item_get_state_func getstate)
	: input_device_item(device, name, internal, itemid, getstate, ITEM_CLASS_RELATIVE)
{
}


//-------------------------------------------------
//  read_as_switch - return the relative value as
//  a switch result based on the modifier
//-------------------------------------------------

s32 input_device_relative_item::read_as_switch(input_item_modifier modifier)
{
	// process according to modifiers
	if (modifier == ITEM_MODIFIER_POS || modifier == ITEM_MODIFIER_RIGHT || modifier == ITEM_MODIFIER_DOWN)
		return (update_value() > 0);
	else if (modifier == ITEM_MODIFIER_NEG || modifier == ITEM_MODIFIER_LEFT || modifier == ITEM_MODIFIER_UP)
		return (update_value() < 0);

	// all other cases just return 0
	return 0;
}


//-------------------------------------------------
//  read_as_relative - return the relative input
//  as a relative axis value
//-------------------------------------------------

s32 input_device_relative_item::read_as_relative(input_item_modifier modifier)
{
	// just return directly
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



//**************************************************************************
//  INPUT DEVICE ABSOLUTE ITEM
//**************************************************************************

//-------------------------------------------------
//  input_device_absolute_item - constructor
//-------------------------------------------------

input_device_absolute_item::input_device_absolute_item(input_device &device, const char *name, void *internal, input_item_id itemid, item_get_state_func getstate)
	: input_device_item(device, name, internal, itemid, getstate, ITEM_CLASS_ABSOLUTE)
{
}


//-------------------------------------------------
//  read_as_switch - return the absolute value as
//  a switch result based on the modifier
//-------------------------------------------------

s32 input_device_absolute_item::read_as_switch(input_item_modifier modifier)
{
	// start with the current value
	s32 result = m_device.adjust_absolute(update_value());
	assert(result >= INPUT_ABSOLUTE_MIN && result <= INPUT_ABSOLUTE_MAX);

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
			return (joymap.update(xaxis_item->current(), yaxis_item->current()) >> (modifier - ITEM_MODIFIER_LEFT)) & 1;
		}
	}

	// positive/negative: true if past the deadzone in either direction
	if (modifier == ITEM_MODIFIER_POS || modifier == ITEM_MODIFIER_RIGHT || modifier == ITEM_MODIFIER_DOWN)
		return (result > 0);
	else if (modifier == ITEM_MODIFIER_NEG || modifier == ITEM_MODIFIER_LEFT || modifier == ITEM_MODIFIER_UP)
		return (result < 0);

	// all other cases just return 0
	return 0;
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
	assert(result >= INPUT_ABSOLUTE_MIN && result <= INPUT_ABSOLUTE_MAX);

	// if we're doing a lightgun reload hack, override the value
	if (m_device.devclass() == DEVICE_CLASS_LIGHTGUN && m_device.lightgun_reload_button())
	{
		// if it is pressed, return (min,max)
		input_device_item *button2_item = m_device.item(ITEM_ID_BUTTON2);
		if (button2_item != nullptr && button2_item->update_value())
			result = (m_itemid == ITEM_ID_XAXIS) ? INPUT_ABSOLUTE_MIN : INPUT_ABSOLUTE_MAX;
	}

	// positive/negative: scale to full axis
	if (modifier == ITEM_MODIFIER_POS)
		result = std::max(result, 0) * 2 + INPUT_ABSOLUTE_MIN;
	if (modifier == ITEM_MODIFIER_NEG)
		result = std::max(-result, 0) * 2 + INPUT_ABSOLUTE_MIN;
	return result;
}
