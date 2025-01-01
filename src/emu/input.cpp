// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    input.cpp

    Handle input from the user.

****************************************************************************

    To do:
        * auto-selecting joystick configs
        * per-joystick configs?
        * test half-axis selections
        * add input test menu
        * get rid of osd_customize_inputport_list

***************************************************************************/

#include "emu.h"
#include "inputdev.h"

#include "corestr.h"



namespace {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> code_string_table

// simple class to match codes to strings
struct code_string_table
{
	static inline constexpr u32 SENTINEL = ~u32(0);

	u32 operator[](std::string_view string) const
	{
		for (const code_string_table *current = this; current->m_code != SENTINEL; current++)
			if (current->m_string == string)
				return current->m_code;
		return SENTINEL;
	}

	const char *operator[](u32 code) const
	{
		for (const code_string_table *current = this; current->m_code != SENTINEL; current++)
			if (current->m_code == code)
				return current->m_string;
		return nullptr;
	}

	u32             m_code;
	const char *    m_string;
};



//**************************************************************************
//  TOKEN/STRING TABLES
//**************************************************************************

// token strings for device classes
const code_string_table devclass_token_table[] =
{
	{ DEVICE_CLASS_KEYBOARD,       "KEYCODE" },
	{ DEVICE_CLASS_MOUSE,          "MOUSECODE" },
	{ DEVICE_CLASS_LIGHTGUN,       "GUNCODE" },
	{ DEVICE_CLASS_JOYSTICK,       "JOYCODE" },
	{ code_string_table::SENTINEL, "UNKCODE" }
};

// friendly strings for device classes
const code_string_table devclass_string_table[] =
{
	{ DEVICE_CLASS_KEYBOARD,       "Kbd" },
	{ DEVICE_CLASS_MOUSE,          "Mouse" },
	{ DEVICE_CLASS_LIGHTGUN,       "Gun" },
	{ DEVICE_CLASS_JOYSTICK,       "Joy" },
	{ code_string_table::SENTINEL, "Unk" }
};

// token strings for item modifiers
const code_string_table modifier_token_table[] =
{
	{ ITEM_MODIFIER_REVERSE,       "REVERSE" },
	{ ITEM_MODIFIER_POS,           "POS" },
	{ ITEM_MODIFIER_NEG,           "NEG" },
	{ ITEM_MODIFIER_LEFT,          "LEFT" },
	{ ITEM_MODIFIER_RIGHT,         "RIGHT" },
	{ ITEM_MODIFIER_UP,            "UP" },
	{ ITEM_MODIFIER_DOWN,          "DOWN" },
	{ code_string_table::SENTINEL, "" }
};

// friendly strings for item modifiers
const code_string_table modifier_string_table[] =
{
	{ ITEM_MODIFIER_REVERSE,       "Reverse" },
	{ ITEM_MODIFIER_POS,           "+" },
	{ ITEM_MODIFIER_NEG,           "-" },
	{ ITEM_MODIFIER_LEFT,          "Left" },
	{ ITEM_MODIFIER_RIGHT,         "Right" },
	{ ITEM_MODIFIER_UP,            "Up" },
	{ ITEM_MODIFIER_DOWN,          "Down" },
	{ code_string_table::SENTINEL, "" }
};

// token strings for item classes
const code_string_table itemclass_token_table[] =
{
	{ ITEM_CLASS_SWITCH,           "SWITCH" },
	{ ITEM_CLASS_ABSOLUTE,         "ABSOLUTE" },
	{ ITEM_CLASS_RELATIVE,         "RELATIVE" },
	{ code_string_table::SENTINEL, "" }
};

// token strings for standard item ids
const code_string_table itemid_token_table[] =
{
	// standard keyboard codes
	{ ITEM_ID_A,             "A" },
	{ ITEM_ID_B,             "B" },
	{ ITEM_ID_C,             "C" },
	{ ITEM_ID_D,             "D" },
	{ ITEM_ID_E,             "E" },
	{ ITEM_ID_F,             "F" },
	{ ITEM_ID_G,             "G" },
	{ ITEM_ID_H,             "H" },
	{ ITEM_ID_I,             "I" },
	{ ITEM_ID_J,             "J" },
	{ ITEM_ID_K,             "K" },
	{ ITEM_ID_L,             "L" },
	{ ITEM_ID_M,             "M" },
	{ ITEM_ID_N,             "N" },
	{ ITEM_ID_O,             "O" },
	{ ITEM_ID_P,             "P" },
	{ ITEM_ID_Q,             "Q" },
	{ ITEM_ID_R,             "R" },
	{ ITEM_ID_S,             "S" },
	{ ITEM_ID_T,             "T" },
	{ ITEM_ID_U,             "U" },
	{ ITEM_ID_V,             "V" },
	{ ITEM_ID_W,             "W" },
	{ ITEM_ID_X,             "X" },
	{ ITEM_ID_Y,             "Y" },
	{ ITEM_ID_Z,             "Z" },
	{ ITEM_ID_0,             "0" },
	{ ITEM_ID_1,             "1" },
	{ ITEM_ID_2,             "2" },
	{ ITEM_ID_3,             "3" },
	{ ITEM_ID_4,             "4" },
	{ ITEM_ID_5,             "5" },
	{ ITEM_ID_6,             "6" },
	{ ITEM_ID_7,             "7" },
	{ ITEM_ID_8,             "8" },
	{ ITEM_ID_9,             "9" },
	{ ITEM_ID_F1,            "F1" },
	{ ITEM_ID_F2,            "F2" },
	{ ITEM_ID_F3,            "F3" },
	{ ITEM_ID_F4,            "F4" },
	{ ITEM_ID_F5,            "F5" },
	{ ITEM_ID_F6,            "F6" },
	{ ITEM_ID_F7,            "F7" },
	{ ITEM_ID_F8,            "F8" },
	{ ITEM_ID_F9,            "F9" },
	{ ITEM_ID_F10,           "F10" },
	{ ITEM_ID_F11,           "F11" },
	{ ITEM_ID_F12,           "F12" },
	{ ITEM_ID_F13,           "F13" },
	{ ITEM_ID_F14,           "F14" },
	{ ITEM_ID_F15,           "F15" },
	{ ITEM_ID_F16,           "F16" },
	{ ITEM_ID_F17,           "F17" },
	{ ITEM_ID_F18,           "F18" },
	{ ITEM_ID_F19,           "F19" },
	{ ITEM_ID_F20,           "F20" },
	{ ITEM_ID_ESC,           "ESC" },
	{ ITEM_ID_TILDE,         "TILDE" },
	{ ITEM_ID_MINUS,         "MINUS" },
	{ ITEM_ID_EQUALS,        "EQUALS" },
	{ ITEM_ID_BACKSPACE,     "BACKSPACE" },
	{ ITEM_ID_TAB,           "TAB" },
	{ ITEM_ID_OPENBRACE,     "OPENBRACE" },
	{ ITEM_ID_CLOSEBRACE,    "CLOSEBRACE" },
	{ ITEM_ID_ENTER,         "ENTER" },
	{ ITEM_ID_COLON,         "COLON" },
	{ ITEM_ID_QUOTE,         "QUOTE" },
	{ ITEM_ID_BACKSLASH,     "BACKSLASH" },
	{ ITEM_ID_BACKSLASH2,    "BACKSLASH2" },
	{ ITEM_ID_COMMA,         "COMMA" },
	{ ITEM_ID_STOP,          "STOP" },
	{ ITEM_ID_SLASH,         "SLASH" },
	{ ITEM_ID_SPACE,         "SPACE" },
	{ ITEM_ID_INSERT,        "INSERT" },
	{ ITEM_ID_DEL,           "DEL" },
	{ ITEM_ID_HOME,          "HOME" },
	{ ITEM_ID_END,           "END" },
	{ ITEM_ID_PGUP,          "PGUP" },
	{ ITEM_ID_PGDN,          "PGDN" },
	{ ITEM_ID_LEFT,          "LEFT" },
	{ ITEM_ID_RIGHT,         "RIGHT" },
	{ ITEM_ID_UP,            "UP" },
	{ ITEM_ID_DOWN,          "DOWN" },
	{ ITEM_ID_0_PAD,         "0PAD" },
	{ ITEM_ID_1_PAD,         "1PAD" },
	{ ITEM_ID_2_PAD,         "2PAD" },
	{ ITEM_ID_3_PAD,         "3PAD" },
	{ ITEM_ID_4_PAD,         "4PAD" },
	{ ITEM_ID_5_PAD,         "5PAD" },
	{ ITEM_ID_6_PAD,         "6PAD" },
	{ ITEM_ID_7_PAD,         "7PAD" },
	{ ITEM_ID_8_PAD,         "8PAD" },
	{ ITEM_ID_9_PAD,         "9PAD" },
	{ ITEM_ID_SLASH_PAD,     "SLASHPAD" },
	{ ITEM_ID_ASTERISK,      "ASTERISK" },
	{ ITEM_ID_MINUS_PAD,     "MINUSPAD" },
	{ ITEM_ID_PLUS_PAD,      "PLUSPAD" },
	{ ITEM_ID_DEL_PAD,       "DELPAD" },
	{ ITEM_ID_ENTER_PAD,     "ENTERPAD" },
	{ ITEM_ID_BS_PAD,        "BSPAD" },
	{ ITEM_ID_TAB_PAD,       "TABPAD" },
	{ ITEM_ID_00_PAD,        "00PAD" },
	{ ITEM_ID_000_PAD,       "000PAD" },
	{ ITEM_ID_PRTSCR,        "PRTSCR" },
	{ ITEM_ID_PAUSE,         "PAUSE" },
	{ ITEM_ID_LSHIFT,        "LSHIFT" },
	{ ITEM_ID_RSHIFT,        "RSHIFT" },
	{ ITEM_ID_LCONTROL,      "LCONTROL" },
	{ ITEM_ID_RCONTROL,      "RCONTROL" },
	{ ITEM_ID_LALT,          "LALT" },
	{ ITEM_ID_RALT,          "RALT" },
	{ ITEM_ID_SCRLOCK,       "SCRLOCK" },
	{ ITEM_ID_NUMLOCK,       "NUMLOCK" },
	{ ITEM_ID_CAPSLOCK,      "CAPSLOCK" },
	{ ITEM_ID_LWIN,          "LWIN" },
	{ ITEM_ID_RWIN,          "RWIN" },
	{ ITEM_ID_MENU,          "MENU" },
	{ ITEM_ID_CANCEL,        "CANCEL" },

	// standard mouse/joystick/gun codes
	{ ITEM_ID_XAXIS,         "XAXIS" },
	{ ITEM_ID_YAXIS,         "YAXIS" },
	{ ITEM_ID_ZAXIS,         "ZAXIS" },
	{ ITEM_ID_RXAXIS,        "RXAXIS" },
	{ ITEM_ID_RYAXIS,        "RYAXIS" },
	{ ITEM_ID_RZAXIS,        "RZAXIS" },
	{ ITEM_ID_SLIDER1,       "SLIDER1" },
	{ ITEM_ID_SLIDER2,       "SLIDER2" },
	{ ITEM_ID_BUTTON1,       "BUTTON1" },
	{ ITEM_ID_BUTTON2,       "BUTTON2" },
	{ ITEM_ID_BUTTON3,       "BUTTON3" },
	{ ITEM_ID_BUTTON4,       "BUTTON4" },
	{ ITEM_ID_BUTTON5,       "BUTTON5" },
	{ ITEM_ID_BUTTON6,       "BUTTON6" },
	{ ITEM_ID_BUTTON7,       "BUTTON7" },
	{ ITEM_ID_BUTTON8,       "BUTTON8" },
	{ ITEM_ID_BUTTON9,       "BUTTON9" },
	{ ITEM_ID_BUTTON10,      "BUTTON10" },
	{ ITEM_ID_BUTTON11,      "BUTTON11" },
	{ ITEM_ID_BUTTON12,      "BUTTON12" },
	{ ITEM_ID_BUTTON13,      "BUTTON13" },
	{ ITEM_ID_BUTTON14,      "BUTTON14" },
	{ ITEM_ID_BUTTON15,      "BUTTON15" },
	{ ITEM_ID_BUTTON16,      "BUTTON16" },
	{ ITEM_ID_BUTTON17,      "BUTTON17" },
	{ ITEM_ID_BUTTON18,      "BUTTON18" },
	{ ITEM_ID_BUTTON19,      "BUTTON19" },
	{ ITEM_ID_BUTTON20,      "BUTTON20" },
	{ ITEM_ID_BUTTON21,      "BUTTON21" },
	{ ITEM_ID_BUTTON22,      "BUTTON22" },
	{ ITEM_ID_BUTTON23,      "BUTTON23" },
	{ ITEM_ID_BUTTON24,      "BUTTON24" },
	{ ITEM_ID_BUTTON25,      "BUTTON25" },
	{ ITEM_ID_BUTTON26,      "BUTTON26" },
	{ ITEM_ID_BUTTON27,      "BUTTON27" },
	{ ITEM_ID_BUTTON28,      "BUTTON28" },
	{ ITEM_ID_BUTTON29,      "BUTTON29" },
	{ ITEM_ID_BUTTON30,      "BUTTON30" },
	{ ITEM_ID_BUTTON31,      "BUTTON31" },
	{ ITEM_ID_BUTTON32,      "BUTTON32" },
	{ ITEM_ID_START,         "START" },
	{ ITEM_ID_SELECT,        "SELECT" },

	// Hats
	{ ITEM_ID_HAT1UP,        "HAT1UP" },
	{ ITEM_ID_HAT1DOWN,      "HAT1DOWN" },
	{ ITEM_ID_HAT1LEFT,      "HAT1LEFT" },
	{ ITEM_ID_HAT1RIGHT,     "HAT1RIGHT" },
	{ ITEM_ID_HAT2UP,        "HAT2UP" },
	{ ITEM_ID_HAT2DOWN,      "HAT2DOWN" },
	{ ITEM_ID_HAT2LEFT,      "HAT2LEFT" },
	{ ITEM_ID_HAT2RIGHT,     "HAT2RIGHT" },
	{ ITEM_ID_HAT3UP,        "HAT3UP" },
	{ ITEM_ID_HAT3DOWN,      "HAT3DOWN" },
	{ ITEM_ID_HAT3LEFT,      "HAT3LEFT" },
	{ ITEM_ID_HAT3RIGHT,     "HAT3RIGHT" },
	{ ITEM_ID_HAT4UP,        "HAT4UP" },
	{ ITEM_ID_HAT4DOWN,      "HAT4DOWN" },
	{ ITEM_ID_HAT4LEFT,      "HAT4LEFT" },
	{ ITEM_ID_HAT4RIGHT,     "HAT4RIGHT" },

	// Additional IDs
	{ ITEM_ID_ADD_SWITCH1,   "ADDSW1" },
	{ ITEM_ID_ADD_SWITCH2,   "ADDSW2" },
	{ ITEM_ID_ADD_SWITCH3,   "ADDSW3" },
	{ ITEM_ID_ADD_SWITCH4,   "ADDSW4" },
	{ ITEM_ID_ADD_SWITCH5,   "ADDSW5" },
	{ ITEM_ID_ADD_SWITCH6,   "ADDSW6" },
	{ ITEM_ID_ADD_SWITCH7,   "ADDSW7" },
	{ ITEM_ID_ADD_SWITCH8,   "ADDSW8" },
	{ ITEM_ID_ADD_SWITCH9,   "ADDSW9" },
	{ ITEM_ID_ADD_SWITCH10,  "ADDSW10" },
	{ ITEM_ID_ADD_SWITCH11,  "ADDSW11" },
	{ ITEM_ID_ADD_SWITCH12,  "ADDSW12" },
	{ ITEM_ID_ADD_SWITCH13,  "ADDSW13" },
	{ ITEM_ID_ADD_SWITCH14,  "ADDSW14" },
	{ ITEM_ID_ADD_SWITCH15,  "ADDSW15" },
	{ ITEM_ID_ADD_SWITCH16,  "ADDSW16" },
	{ ITEM_ID_ADD_ABSOLUTE1, "ADDAXIS1" },
	{ ITEM_ID_ADD_ABSOLUTE2, "ADDAXIS2" },
	{ ITEM_ID_ADD_ABSOLUTE3, "ADDAXIS3" },
	{ ITEM_ID_ADD_ABSOLUTE4, "ADDAXIS4" },
	{ ITEM_ID_ADD_ABSOLUTE5, "ADDAXIS5" },
	{ ITEM_ID_ADD_ABSOLUTE6, "ADDAXIS6" },
	{ ITEM_ID_ADD_ABSOLUTE7, "ADDAXIS7" },
	{ ITEM_ID_ADD_ABSOLUTE8, "ADDAXIS8" },
	{ ITEM_ID_ADD_ABSOLUTE9, "ADDAXIS9" },
	{ ITEM_ID_ADD_ABSOLUTE10,"ADDAXIS10" },
	{ ITEM_ID_ADD_ABSOLUTE11,"ADDAXIS11" },
	{ ITEM_ID_ADD_ABSOLUTE12,"ADDAXIS12" },
	{ ITEM_ID_ADD_ABSOLUTE13,"ADDAXIS13" },
	{ ITEM_ID_ADD_ABSOLUTE14,"ADDAXIS14" },
	{ ITEM_ID_ADD_ABSOLUTE15,"ADDAXIS15" },
	{ ITEM_ID_ADD_ABSOLUTE16,"ADDAXIS16" },
	{ ITEM_ID_ADD_RELATIVE1, "ADDREL1" },
	{ ITEM_ID_ADD_RELATIVE2, "ADDREL2" },
	{ ITEM_ID_ADD_RELATIVE3, "ADDREL3" },
	{ ITEM_ID_ADD_RELATIVE4, "ADDREL4" },
	{ ITEM_ID_ADD_RELATIVE5, "ADDREL5" },
	{ ITEM_ID_ADD_RELATIVE6, "ADDREL6" },
	{ ITEM_ID_ADD_RELATIVE7, "ADDREL7" },
	{ ITEM_ID_ADD_RELATIVE8, "ADDREL8" },
	{ ITEM_ID_ADD_RELATIVE9, "ADDREL9" },
	{ ITEM_ID_ADD_RELATIVE10,"ADDREL10" },
	{ ITEM_ID_ADD_RELATIVE11,"ADDREL11" },
	{ ITEM_ID_ADD_RELATIVE12,"ADDREL12" },
	{ ITEM_ID_ADD_RELATIVE13,"ADDREL13" },
	{ ITEM_ID_ADD_RELATIVE14,"ADDREL14" },
	{ ITEM_ID_ADD_RELATIVE15,"ADDREL15" },
	{ ITEM_ID_ADD_RELATIVE16,"ADDREL16" },

	{ code_string_table::SENTINEL, nullptr }
};



//**************************************************************************
//  UTILITY FUNCTIONS
//**************************************************************************

inline void accumulate_axis_value(
		s32 &result,
		input_item_class &resultclass,
		input_item_class &resultclasszero,
		s32 value,
		input_item_class valueclass,
		input_item_class valueclasszero)
{
	if (!value)
	{
		// track the highest-priority zero
		if ((ITEM_CLASS_ABSOLUTE == valueclasszero) || (ITEM_CLASS_INVALID == resultclasszero))
			resultclasszero = valueclasszero;
	}
	else if (ITEM_CLASS_ABSOLUTE == valueclass)
	{
		// absolute values override relative values
		if (ITEM_CLASS_ABSOLUTE == resultclass)
			result += value;
		else
			result = value;
		resultclass = ITEM_CLASS_ABSOLUTE;
	}
	else if (ITEM_CLASS_RELATIVE == valueclass)
	{
		// relative values accumulate
		if (resultclass != ITEM_CLASS_ABSOLUTE)
		{
			result += value;
			resultclass = ITEM_CLASS_RELATIVE;
		}
	}
}

} // anonymous namespace



//**************************************************************************
//  INPUT MANAGER
//**************************************************************************

//-------------------------------------------------
//  input_manager - constructor
//-------------------------------------------------

input_manager::input_manager(running_machine &machine) : m_machine(machine)
{
	// reset code memory
	reset_memory();

	// create pointers for the classes
	m_class[DEVICE_CLASS_KEYBOARD] = std::make_unique<input_class_keyboard>(*this);
	m_class[DEVICE_CLASS_MOUSE] = std::make_unique<input_class_mouse>(*this);
	m_class[DEVICE_CLASS_LIGHTGUN] = std::make_unique<input_class_lightgun>(*this);
	m_class[DEVICE_CLASS_JOYSTICK] = std::make_unique<input_class_joystick>(*this);

#ifdef MAME_DEBUG
	for (input_device_class devclass = DEVICE_CLASS_FIRST_VALID; devclass <= DEVICE_CLASS_LAST_VALID; ++devclass)
	{
		assert(m_class[devclass] != nullptr);
		assert(m_class[devclass]->devclass() == devclass);
	}
#endif
}


//-------------------------------------------------
//  input_manager - destructor
//-------------------------------------------------

input_manager::~input_manager()
{
}


//-------------------------------------------------
//  class_enabled - return whether input device
//  class is enabled
//-------------------------------------------------

bool input_manager::class_enabled(input_device_class devclass) const
{
	assert(devclass >= DEVICE_CLASS_FIRST_VALID && devclass <= DEVICE_CLASS_LAST_VALID);
	return m_class[devclass]->enabled();
}


//-------------------------------------------------
//  add_device - add a representation of a host
//  input device
//-------------------------------------------------

osd::input_device &input_manager::add_device(input_device_class devclass, std::string_view name, std::string_view id, void *internal)
{
	return device_class(devclass).add_device(name, id, internal);
}


//-------------------------------------------------
//  code_value - return the value of a given
//  input code
//-------------------------------------------------

s32 input_manager::code_value(input_code code)
{
	auto profile = g_profiler.start(PROFILER_INPUT);

	// return 0 for any invalid devices
	input_device *const device = device_from_code(code);
	if (!device)
		return 0;

	// also return 0 if the device class is disabled
	input_class &devclass = *m_class[code.device_class()];
	if (!devclass.enabled())
		return 0;

	// if this is not a multi device, only return data for item 0 and iterate over all
	int startindex = code.device_index();
	int stopindex = startindex;
	if (!devclass.multi())
	{
		if (startindex != 0)
			return 0;
		stopindex = devclass.maxindex();
	}

	// iterate over all device indices
	s32 result = 0;
	input_item_class targetclass = code.item_class();
	for (int curindex = startindex; curindex <= stopindex; curindex++)
	{
		// lookup the item for the appropriate index
		code.set_device_index(curindex);
		input_device_item *const item = item_from_code(code);
		if (item)
		{
			// process items according to their native type
			switch (targetclass)
			{
			case ITEM_CLASS_ABSOLUTE:
				if (result == 0)
					result = item->read_as_absolute(code.item_modifier());
				break;

			case ITEM_CLASS_RELATIVE:
				result += item->read_as_relative(code.item_modifier());
				break;

			case ITEM_CLASS_SWITCH:
				result |= item->read_as_switch(code.item_modifier());
				break;

			default:
				break;
			}
		}
	}

	return result;
}


//-------------------------------------------------
//  code_pressed_once - return non-zero if a given
//  input code has transitioned from off to on
//  since the last call
//-------------------------------------------------

bool input_manager::code_pressed_once(input_code code)
{
	// look for the code in the memory
	bool curvalue = code_pressed(code);
	int empty = -1;
	for (int memnum = 0; memnum < std::size(m_switch_memory); memnum++)
	{
		// were we previous pressed on the last time through here?
		if (m_switch_memory[memnum] == code)
		{
			// if no longer pressed, clear entry
			if (!curvalue)
				m_switch_memory[memnum] = INPUT_CODE_INVALID;

			// always return false
			return false;
		}

		// remember the first empty entry
		if (empty == -1 && m_switch_memory[memnum] == INPUT_CODE_INVALID)
			empty = memnum;
	}

	// if we get here, we were not previously pressed; if still not pressed, return false
	if (!curvalue)
		return false;

	// otherwise, add the code to the memory and return true
	assert(empty != -1);
	if (empty != -1)
		m_switch_memory[empty] = code;
	return true;
}


//-------------------------------------------------
//  device_from_code - given an input_code return
//  a pointer to the associated device
//-------------------------------------------------

input_device *input_manager::device_from_code(input_code code) const
{
	// if the class is valid, return the appropriate device pointer
	input_device_class devclass = code.device_class();
	if (devclass >= DEVICE_CLASS_FIRST_VALID && devclass <= DEVICE_CLASS_LAST_VALID)
		return m_class[devclass]->device(code.device_index());

	// otherwise, return nullptr
	return nullptr;
}


//-------------------------------------------------
//  item_from_code - given an input_code return
//  a pointer to the appropriate input_device_item
//-------------------------------------------------

input_device_item *input_manager::item_from_code(input_code code) const
{
	// first get the device; if none, then we don't have an item
	input_device *device = device_from_code(code);
	if (device == nullptr)
		return nullptr;

	// then return the device's item
	return device->item(code.item_id());
}


//-------------------------------------------------
//  reset_memory - reset the array of memory for
//  pressed switches
//-------------------------------------------------

void input_manager::reset_memory()
{
	// reset all entries in switch memory to invalid
	for (auto & elem : m_switch_memory)
		elem = INPUT_CODE_INVALID;
}


//-------------------------------------------------
//  code_from_itemid - translates an input_item_id
//  to an input_code
//-------------------------------------------------

input_code input_manager::code_from_itemid(input_item_id itemid) const
{
	// iterate over device classes and devices
	for (input_device_class devclass = DEVICE_CLASS_FIRST_VALID; devclass <= DEVICE_CLASS_LAST_VALID; ++devclass)
		for (int devnum = 0; devnum <= m_class[devclass]->maxindex(); devnum++)
		{
			input_device *device = m_class[devclass]->device(devnum);
			if (device != nullptr)
			{
				input_device_item *item = device->item(itemid);
				if (item != nullptr)
					return item->code();
			}
		}

	return INPUT_CODE_INVALID;
}


//-------------------------------------------------
//  code_name - convert an input code into a
//  friendly name
//-------------------------------------------------

std::string input_manager::code_name(input_code code) const
{
	// if nothing there, return an empty string
	input_device_item const *const item = item_from_code(code);
	if (!item)
		return std::string();

	std::string str;

	// keyboard 0 doesn't show a class or index if it is the only one
	input_device_class const device_class = item->device().devclass();
	if ((device_class != DEVICE_CLASS_KEYBOARD) || (m_class[device_class]->maxindex() > 0))
	{
		// determine the devclass part
		str = (*devclass_string_table)[code.device_class()];

		// if we're unifying all devices, don't display a number
		if (m_class[code.device_class()]->multi())
			str.append(util::string_format(" %d ", code.device_index() + 1));
		else
			str.append(" ");
	}

	// append item name - redundant with joystick switch left/right/up/down
	bool const joydir =
			(device_class == DEVICE_CLASS_JOYSTICK) &&
			(code.item_class() == ITEM_CLASS_SWITCH) &&
			(code.item_modifier() >= ITEM_MODIFIER_LEFT) &&
			(code.item_modifier() <= ITEM_MODIFIER_DOWN);
	if (joydir)
	{
		str.append((*modifier_string_table)[code.item_modifier()]);
	}
	else
	{
		str.append(item->name());
		char const *const modifier = (*modifier_string_table)[code.item_modifier()];
		if (modifier && *modifier)
			str.append(" ").append(modifier);
	}

	return str;
}


//-------------------------------------------------
//  code_to_token - create a token for a given code
//-------------------------------------------------

std::string input_manager::code_to_token(input_code code) const
{
	using namespace std::literals;

	// determine the devclass part
	const char *devclass = (*devclass_token_table)[code.device_class()];
	if (!devclass)
		return "INVALID";

	// determine the devindex part; keyboard 0 doesn't show an index
	std::string devindex = string_format("%d", code.device_index() + 1);
	if (code.device_class() == DEVICE_CLASS_KEYBOARD && code.device_index() == 0)
		devindex.clear();

	// determine the itemid part; look up in the table if we don't have a token
	input_device_item *item = item_from_code(code);
	std::string_view devcode = item ? item->token() : "UNKNOWN"sv;

	// determine the modifier part
	const char *modifier = (*modifier_token_table)[code.item_modifier()];

	// determine the itemclass part; if we match the native class, we don't include this
	const char *itemclass = "";
	if (!item || (item->itemclass() != code.item_class()))
		itemclass = (*itemclass_token_table)[code.item_class()];

	// concatenate the strings
	std::string str(devclass);
	if (!devindex.empty())
		str.append("_").append(devindex);
	if (!devcode.empty())
		str.append("_").append(devcode);
	if (modifier != nullptr)
		str.append("_").append(modifier);
	if (itemclass != nullptr && itemclass[0] != 0)
		str.append("_").append(itemclass);
	return str;
}


//-------------------------------------------------
//  code_from_token - extract an input code from a
//  token
//-------------------------------------------------

input_code input_manager::code_from_token(std::string_view _token)
{
	// copy the token and break it into pieces
	std::string token[6];
	int numtokens = 0;
	while (numtokens < std::size(token))
	{
		// make a token up to the next underscore
		std::string_view::size_type score = _token.find('_');
		token[numtokens++].assign(_token, 0, (std::string_view::npos == score) ? _token.length() : score);

		// if we hit the end, we're done, else advance our pointer
		if (std::string_view::npos == score)
			break;
		_token.remove_prefix(score + 1);
	}

	// first token should be the devclass
	int curtok = 0;
	input_device_class const devclass = input_device_class((*devclass_token_table)[token[curtok++]]);
	if (devclass == input_device_class(code_string_table::SENTINEL))
		return INPUT_CODE_INVALID;

	// second token might be index; look for number
	int devindex = 0;
	if (numtokens > 2 && sscanf(token[curtok].c_str(), "%d", &devindex) == 1)
	{
		curtok++;
		devindex--;
	}
	if (curtok >= numtokens)
		return INPUT_CODE_INVALID;

	// next token is the item ID
	input_item_id itemid = input_item_id((*itemid_token_table)[token[curtok]]);
	bool const standard = (itemid != input_item_id(code_string_table::SENTINEL));

	input_item_class itemclass = ITEM_CLASS_INVALID;
	if (standard)
	{
		// if we're a standard code, default the itemclass based on it
		itemclass = m_class[devclass]->standard_item_class(itemid);
	}
	else
	{
		// otherwise, keep parsing

		// if this is an invalid device, we have nothing to look up
		input_device *device = m_class[devclass]->device(devindex);
		if (!device)
			return INPUT_CODE_INVALID;

		// if not a standard code, look it up in the device specific codes
		for (itemid = ITEM_ID_FIRST_VALID; itemid <= device->maxitem(); ++itemid)
		{
			input_device_item *item = device->item(itemid);
			if (item && !token[curtok].compare(item->token()))
			{
				// take the itemclass from the item
				itemclass = item->itemclass();
				break;
			}
		}

		// bail on fail
		if (itemid > device->maxitem())
			return INPUT_CODE_INVALID;
	}
	curtok++;

	// if we have another token, it is probably a modifier
	input_item_modifier modifier = ITEM_MODIFIER_NONE;
	if (curtok < numtokens)
	{
		modifier = input_item_modifier((*modifier_token_table)[token[curtok]]);
		if (modifier != input_item_modifier(code_string_table::SENTINEL))
			curtok++;
		else
			modifier = ITEM_MODIFIER_NONE;
	}

	// if we have another token, it is the item class
	if (curtok < numtokens)
	{
		u32 const temp = (*itemclass_token_table)[token[curtok]];
		if (temp != code_string_table::SENTINEL)
		{
			curtok++;
			itemclass = input_item_class(temp);
		}
	}

	// we should have consumed all tokens
	if (curtok != numtokens)
		return INPUT_CODE_INVALID;

	// assemble the final code
	return input_code(devclass, devindex, itemclass, modifier, itemid);
}


//-------------------------------------------------
//  standard_token - return the standard token for
//  the given input item ID
//-------------------------------------------------

const char *input_manager::standard_token(input_item_id itemid) const
{
	return (itemid <= ITEM_ID_MAXIMUM) ? (*itemid_token_table)[itemid] : nullptr;
}


//-------------------------------------------------
//  seq_pressed - return true if the given sequence
//  of switch inputs is "pressed"
//-------------------------------------------------

bool input_manager::seq_pressed(const input_seq &seq)
{
	// iterate over all of the codes
	bool result = false;
	bool invert = false;
	bool first = true;
	for (int codenum = 0; ; codenum++)
	{
		input_code code = seq[codenum];
		if (code == input_seq::not_code)
		{
			// handle NOT
			invert = true;
		}
		else if (code == input_seq::or_code || code == input_seq::end_code)
		{
			// handle OR and END

			// if we have a positive result from the previous set, we're done
			if (result || code == input_seq::end_code)
				break;

			// otherwise, reset our state
			result = false;
			invert = false;
			first = true;
		}
		else
		{
			// handle everything else as a series of ANDs

			// if this is the first in the sequence, result is set equal
			if (first)
				result = code_pressed(code) ^ invert;

			// further values are ANDed
			else if (result)
				result &= code_pressed(code) ^ invert;

			// no longer first, and clear the invert flag
			first = invert = false;
		}
	}

	// return the result if we queried at least one switch
	return result;
}


//-------------------------------------------------
//  seq_axis_value - return the value of an axis
//  defined in an input sequence
//-------------------------------------------------

s32 input_manager::seq_axis_value(const input_seq &seq, input_item_class &itemclass)
{
	// start with zero result and no valid classes
	s32 result = 0;
	itemclass = ITEM_CLASS_INVALID;
	input_item_class itemclasszero = ITEM_CLASS_INVALID;

	// iterate over all of the codes
	s32 groupval = 0;
	input_item_class groupclass = ITEM_CLASS_INVALID;
	input_item_class groupclasszero = ITEM_CLASS_INVALID;
	bool invert = false;
	bool enable = true;
	for (int codenum = 0; ; codenum++)
	{
		input_code const code = seq[codenum];
		if (code == input_seq::not_code)
		{
			// handle NOT - invert the next code
			invert = true;
		}
		else if (code == input_seq::end_code)
		{
			// handle END - commit group and break out of loop
			accumulate_axis_value(
					result, itemclass, itemclasszero,
					groupval, groupclass, groupclasszero);
			break;
		}
		else if (code == input_seq::or_code)
		{
			// handle OR - commit group and reset for the next group
			accumulate_axis_value(
					result, itemclass, itemclasszero,
					groupval, groupclass, groupclasszero);
			groupval = 0;
			groupclasszero = ITEM_CLASS_INVALID;
			groupclass = ITEM_CLASS_INVALID;
			invert = false;
			enable = true;
		}
		else if (enable)
		{
			// handle everything else only if we're still enabled
			input_item_class const codeclass = code.item_class();

			// switch codes serve as enables
			if (ITEM_CLASS_SWITCH == codeclass)
			{
				// AND against previous digital codes
				if (enable)
				{
					enable = code_pressed(code) ^ invert;
					if (!enable)
					{
						// clear current group if enable became false - only way out is an OR code
						groupval = 0;
						groupclasszero = ITEM_CLASS_INVALID;
						groupclass = ITEM_CLASS_INVALID;
					}
				}
			}
			else
			{
				// non-switch codes are analog values
				accumulate_axis_value(
						groupval, groupclass, groupclasszero,
						code_value(code), codeclass, codeclass);
			}

			// clear the invert flag - it only applies to one item
			invert = false;
		}
	}

	// saturate mixed absolute values, report neutral type
	if (ITEM_CLASS_ABSOLUTE == itemclass)
		result = std::clamp(result, osd::input_device::ABSOLUTE_MIN, osd::input_device::ABSOLUTE_MAX);
	else if (ITEM_CLASS_INVALID == itemclass)
		itemclass = itemclasszero;
	return result;
}


//-------------------------------------------------
//  seq_clean - clean the sequence, removing
//  any invalid bits
//-------------------------------------------------

input_seq input_manager::seq_clean(const input_seq &seq) const
{
	// make a copy of our sequence, removing any invalid bits
	input_seq clean_codes;
	int clean_index = 0;
	for (int codenum = 0; seq[codenum] != input_seq::end_code; codenum++)
	{
		// if this is a code item which is not valid, don't copy it and remove any preceding ORs/NOTs
		input_code code = seq[codenum];
		if (!code.internal() && (((code.device_index() > 0) && !m_class[code.device_class()]->multi()) || !item_from_code(code)))
		{
			while (clean_index > 0 && clean_codes[clean_index - 1].internal())
			{
				clean_codes.backspace();
				clean_index--;
			}
		}
		else if (clean_index > 0 || !code.internal() || code == input_seq::not_code)
		{
			clean_codes += code;
			clean_index++;
		}
	}
	return clean_codes;
}


//-------------------------------------------------
//  seq_name - generate the friendly name of a
//  sequence
//-------------------------------------------------

std::string input_manager::seq_name(const input_seq &seq) const
{
	// make a copy of our sequence, removing any invalid bits
	input_seq cleaned_seq = seq_clean(seq);

	// special case: empty
	if (cleaned_seq[0] == input_seq::end_code)
		return std::string(seq.empty() ? "None" : "n/a");

	// start with an empty buffer
	std::string str;

	// loop until we hit the end
	for (int codenum = 0; cleaned_seq[codenum] != input_seq::end_code; codenum++)
	{
		// append a space if not the first code
		if (codenum != 0)
			str.append(" ");

		// handle OR/NOT codes here
		input_code code = cleaned_seq[codenum];
		if (code == input_seq::or_code)
			str.append("or");
		else if (code == input_seq::not_code)
			str.append("not");

		// otherwise, assume it is an input code and ask the input system to generate it
		else
			str.append(code_name(code));
	}
	return str;
}


//-------------------------------------------------
//  seq_to_tokens - generate the tokenized form of
//  a sequence
//-------------------------------------------------

std::string input_manager::seq_to_tokens(const input_seq &seq) const
{
	// start with an empty buffer
	std::string str;

	// loop until we hit the end
	for (int codenum = 0; seq[codenum] != input_seq::end_code; codenum++)
	{
		// append a space if not the first code
		if (codenum != 0)
			str.append(" ");

		// handle OR/NOT codes here
		input_code code = seq[codenum];
		if (code == input_seq::or_code)
			str.append("OR");
		else if (code == input_seq::not_code)
			str.append("NOT");
		else if (code == input_seq::default_code)
			str.append("DEFAULT");

		// otherwise, assume it is an input code and ask the input system to generate it
		else
			str.append(code_to_token(code));
	}
	return str;
}


//-------------------------------------------------
//  seq_from_tokens - generate the tokenized form
//  of a sequence
//-------------------------------------------------

void input_manager::seq_from_tokens(input_seq &seq, std::string_view string)
{
	// start with a blank sequence
	seq.reset();

	// loop until we're done
	unsigned operators = 0;
	while (1)
	{
		// trim any leading spaces
		while (!string.empty() && isspace(u8(string[0])))
			string.remove_prefix(1);

		// bail if we're done
		if (string.empty())
			return;

		// find the end of the token and make it upper-case along the way
		std::string token;
		while (!string.empty() && !isspace(u8(string[0])))
		{
			token.push_back(toupper(u8(string[0])));
			string.remove_prefix(1);
		}

		// look for common stuff
		input_code code;
		bool is_operator;
		if (token == "OR")
		{
			code = input_seq::or_code;
			is_operator = true;
		}
		else if (token == "NOT")
		{
			code = input_seq::not_code;
			is_operator = true;
		}
		else if (token == "DEFAULT")
		{
			code = input_seq::default_code;
			is_operator = false;
		}
		else
		{
			code = code_from_token(token);
			is_operator = false;
		}

		// translate and add to the sequence
		if (is_operator)
		{
			++operators;
			seq += code;
		}
		else
		{
			if (code.device_class() < DEVICE_CLASS_FIRST_VALID)
			{
				osd_printf_warning("Input: Dropping invalid input token %s\n", token);
				while (operators)
				{
					seq.backspace();
					--operators;
				}
			}
			else
			{
				operators = 0;
				seq += code;
			}
		}

		// advance
		if (string.empty())
			return;
		string.remove_prefix(1);
	}
}

//-------------------------------------------------
//  map_device_to_controller - map device to
//  controller based on device map table
//-------------------------------------------------

bool input_manager::map_device_to_controller(const devicemap_table &table)
{
	for (const auto &it : table)
	{
		std::string_view deviceid = it.first;
		std::string_view controllername = it.second;

		// tokenize the controller name into device class and index (i.e. controller name should be of the form "GUNCODE_1")
		std::string token[2];
		int numtokens = 0;
		std::string_view _token = controllername;
		while (numtokens < std::size(token))
		{
			// make a token up to the next underscore
			std::string_view::size_type score = _token.find('_');
			token[numtokens++].assign(_token, 0, (std::string_view::npos == score) ? _token.length() : score);

			// if we hit the end, we're done, else advance our pointer
			if (std::string_view::npos == score)
				break;
			_token.remove_prefix(score + 1);
		}
		if (2 != numtokens)
			return false;

		// first token should be the devclass
		input_device_class const devclass = input_device_class((*devclass_token_table)[strmakeupper(token[0])]);
		if (devclass == input_device_class(code_string_table::SENTINEL))
			return false;

		// second token should be the devindex
		int devindex = 0;
		if (1 != sscanf(token[1].c_str(), "%d", &devindex))
			return false;
		devindex--;

		if (devindex >= DEVICE_INDEX_MAXIMUM)
			return false;

		// enumerate through devices and look for a match
		input_class *input_devclass = m_class[devclass].get();
		for (int devnum = 0; devnum <= input_devclass->maxindex(); devnum++)
		{
			input_device *device = input_devclass->device(devnum);
			if (device && device->match_device_id(deviceid))
			{
				// remap devindex
				input_devclass->remap_device_index(device->devindex(), devindex);
				osd_printf_verbose("Input: Remapped %s #%d: %s (device id: %s)\n", input_devclass->name(), devindex + 1, device->name(), device->id());

				break;
			}
		}
	}

	return true;
}
