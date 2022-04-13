// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    ioport.cpp

    Input/output port handling.

****************************************************************************

    Theory of operation

    ------------
    OSD controls
    ------------

    There are three types of controls that the OSD can provide as potential
    input devices: digital controls, absolute analog controls, and relative
    analog controls.

    Digital controls have only two states: on or off. They are generally
    mapped to buttons and digital joystick directions (like a gamepad or a
    joystick hat). The OSD layer must return either 0 (off) or 1 (on) for
    these types of controls.

    Absolute analog controls are analog in the sense that they return a
    range of values depending on how much a given control is moved, but they
    are physically bounded. This means that there is a minimum and maximum
    limit to how far the control can be moved. They are generally mapped to
    analog joystick axes, lightguns, most PC steering wheels, and pedals.
    The OSD layer must determine the minimum and maximum range of each
    analog device and scale that to a value between -65536 and +65536
    representing the position of the control. -65536 generally refers to
    the topmost or leftmost position, while +65536 refers to the bottommost
    or rightmost position. Note that pedals are a special case here, the
    OSD layer needs to return half axis as full -65536 to + 65536 range.

    Relative analog controls are analog as well, but are not physically
    bounded. They can be moved continually in one direction without limit.
    They are generally mapped to trackballs and mice. Because they are
    unbounded, the OSD layer can only return delta values since the last
    read. Because of this, it is difficult to scale appropriately. For
    MAME's purposes, when mapping a mouse devices to a relative analog
    control, one pixel of movement should correspond to 512 units. Other
    analog control types should be scaled to return values of a similar
    magnitude. Like absolute analog controls, negative values refer to
    upward or leftward movement, while positive values refer to downward
    or rightward movement.

    -------------
    Game controls
    -------------

    Similarly, the types of controls used by arcade games fall into the same
    three categories: digital, absolute analog, and relative analog. The
    tricky part is how to map any arbitrary type of OSD control to an
    arbitrary type of game control.

    Digital controls: used for game buttons and standard 4/8-way joysticks,
    as well as many other types of game controls. Mapping an OSD digital
    control to a game's OSD control is trivial. For OSD analog controls,
    the MAME core does not directly support mapping any OSD analog devices
    to digital controls. However, the OSD layer is free to enumerate digital
    equivalents for analog devices. For example, each analog axis in the
    Windows OSD code enumerates to two digital controls, one for the
    negative direction (up/left) and one for the position direction
    (down/right). When these "digital" inputs are queried, the OSD layer
    checks the axis position against the center, adding in a dead zone,
    and returns 0 or 1 to indicate its position.

    Absolute analog controls: used for analog joysticks, lightguns, pedals,
    and wheel controls. Mapping an OSD absolute analog control to this type
    is easy. OSD relative analog controls can be mapped here as well by
    accumulating the deltas and bounding the results. OSD digital controls
    are mapped to these types of controls in pairs, one for a decrement and
    one for an increment, but apart from that, operate the same as the OSD
    relative analog controls by accumulating deltas and applying bounds.
    The speed of the digital delta is user-configurable per analog input.
    In addition, most absolute analog control types have an autocentering
    feature that is activated when using the digital increment/decrement
    sequences, which returns the control back to the center at a user-
    controllable speed if no digital sequences are pressed.

    Relative analog controls: used for trackballs and dial controls. Again,
    mapping an OSD relative analog control to this type is straightforward.
    OSD absolute analog controls can't map directly to these, but if the OSD
    layer provides a digital equivalent for each direction, it can be done.
    OSD digital controls map just like they do for absolute analog controls,
    except that the accumulated deltas are not bounded, but rather wrap.

***************************************************************************/

#include "emu.h"
#include "emuopts.h"
#include "config.h"
#include "fileio.h"
#include "xmlfile.h"
#include "profiler.h"
#include "ui/uimain.h"
#include "inputdev.h"
#include "natkeyboard.h"

#include "util/corestr.h"
#include "util/ioprocsfilter.h"
#include "util/language.h"
#include "util/unicode.h"

#include "osdepend.h"

#include <cctype>
#include <ctime>


namespace {

// temporary: set this to 1 to enable the originally defined behavior that
// a field specified via PORT_MODIFY which intersects a previously-defined
// field completely wipes out the previous definition
#define INPUT_PORT_OVERRIDE_FULLY_NUKES_PREVIOUS    1


//**************************************************************************
//  CONSTANTS
//**************************************************************************

const int SPACE_COUNT = 3;



//**************************************************************************
//  INLINE FUNCTIONS
//**************************************************************************

//-------------------------------------------------
//  compute_scale -- compute an 8.24 scale value
//  from a numerator and a denominator
//-------------------------------------------------

inline s64 compute_scale(s32 num, s32 den)
{
	return (s64(num) << 24) / den;
}


//-------------------------------------------------
//  recip_scale -- compute an 8.24 reciprocal of
//  an 8.24 scale value
//-------------------------------------------------

inline s64 recip_scale(s64 scale)
{
	return (s64(1) << 48) / scale;
}


//-------------------------------------------------
//  apply_scale -- apply an 8.24 scale value to
//  a 32-bit value
//-------------------------------------------------

inline s32 apply_scale(s32 value, s64 scale)
{
	return (s64(value) * scale) / (1 << 24);
}



//**************************************************************************
//  COMMON SHARED STRINGS
//**************************************************************************

const struct
{
	u32 id;
	const char *string;
} input_port_default_strings[] =
{
	{ INPUT_STRING_Off, "Off" },
	{ INPUT_STRING_On, "On" },
	{ INPUT_STRING_No, "No" },
	{ INPUT_STRING_Yes, "Yes" },
	{ INPUT_STRING_Lives, "Lives" },
	{ INPUT_STRING_Bonus_Life, "Bonus Life" },
	{ INPUT_STRING_Difficulty, "Difficulty" },
	{ INPUT_STRING_Demo_Sounds, "Demo Sounds" },
	{ INPUT_STRING_Coinage, "Coinage" },
	{ INPUT_STRING_Coin_A, "Coin A" },
	{ INPUT_STRING_Coin_B, "Coin B" },
	{ INPUT_STRING_9C_1C, "9 Coins/1 Credit" },
	{ INPUT_STRING_8C_1C, "8 Coins/1 Credit" },
	{ INPUT_STRING_7C_1C, "7 Coins/1 Credit" },
	{ INPUT_STRING_6C_1C, "6 Coins/1 Credit" },
	{ INPUT_STRING_5C_1C, "5 Coins/1 Credit" },
	{ INPUT_STRING_4C_1C, "4 Coins/1 Credit" },
	{ INPUT_STRING_3C_1C, "3 Coins/1 Credit" },
	{ INPUT_STRING_8C_3C, "8 Coins/3 Credits" },
	{ INPUT_STRING_4C_2C, "4 Coins/2 Credits" },
	{ INPUT_STRING_5C_2C, "5 Coins/2 Credits" },
	{ INPUT_STRING_2C_1C, "2 Coins/1 Credit" },
	{ INPUT_STRING_5C_3C, "5 Coins/3 Credits" },
	{ INPUT_STRING_3C_2C, "3 Coins/2 Credits" },
	{ INPUT_STRING_4C_3C, "4 Coins/3 Credits" },
	{ INPUT_STRING_4C_4C, "4 Coins/4 Credits" },
	{ INPUT_STRING_3C_3C, "3 Coins/3 Credits" },
	{ INPUT_STRING_2C_2C, "2 Coins/2 Credits" },
	{ INPUT_STRING_1C_1C, "1 Coin/1 Credit" },
	{ INPUT_STRING_3C_5C, "3 Coins/5 Credits" },
	{ INPUT_STRING_4C_5C, "4 Coins/5 Credits" },
	{ INPUT_STRING_3C_4C, "3 Coins/4 Credits" },
	{ INPUT_STRING_2C_3C, "2 Coins/3 Credits" },
	{ INPUT_STRING_4C_7C, "4 Coins/7 Credits" },
	{ INPUT_STRING_2C_4C, "2 Coins/4 Credits" },
	{ INPUT_STRING_1C_2C, "1 Coin/2 Credits" },
	{ INPUT_STRING_2C_5C, "2 Coins/5 Credits" },
	{ INPUT_STRING_2C_6C, "2 Coins/6 Credits" },
	{ INPUT_STRING_1C_3C, "1 Coin/3 Credits" },
	{ INPUT_STRING_2C_7C, "2 Coins/7 Credits" },
	{ INPUT_STRING_2C_8C, "2 Coins/8 Credits" },
	{ INPUT_STRING_1C_4C, "1 Coin/4 Credits" },
	{ INPUT_STRING_1C_5C, "1 Coin/5 Credits" },
	{ INPUT_STRING_1C_6C, "1 Coin/6 Credits" },
	{ INPUT_STRING_1C_7C, "1 Coin/7 Credits" },
	{ INPUT_STRING_1C_8C, "1 Coin/8 Credits" },
	{ INPUT_STRING_1C_9C, "1 Coin/9 Credits" },
	{ INPUT_STRING_Free_Play, "Free Play" },
	{ INPUT_STRING_Cabinet, "Cabinet" },
	{ INPUT_STRING_Upright, "Upright" },
	{ INPUT_STRING_Cocktail, "Cocktail" },
	{ INPUT_STRING_Flip_Screen, "Flip Screen" },
	{ INPUT_STRING_Service_Mode, "Service Mode" },
	{ INPUT_STRING_Pause, "Pause" },
	{ INPUT_STRING_Test, "Test" },
	{ INPUT_STRING_Tilt, "Tilt" },
	{ INPUT_STRING_Version, "Version" },
	{ INPUT_STRING_Region, "Region" },
	{ INPUT_STRING_International, "International" },
	{ INPUT_STRING_Japan, "Japan" },
	{ INPUT_STRING_USA, "USA" },
	{ INPUT_STRING_Europe, "Europe" },
	{ INPUT_STRING_Asia, "Asia" },
	{ INPUT_STRING_China, "China" },
	{ INPUT_STRING_Hong_Kong, "Hong Kong" },
	{ INPUT_STRING_Korea, "Korea" },
	{ INPUT_STRING_Southeast_Asia, "Southeast Asia" },
	{ INPUT_STRING_Taiwan, "Taiwan" },
	{ INPUT_STRING_World, "World" },
	{ INPUT_STRING_Language, "Language" },
	{ INPUT_STRING_English, "English" },
	{ INPUT_STRING_Japanese, "Japanese" },
	{ INPUT_STRING_Chinese, "Chinese" },
	{ INPUT_STRING_French, "French" },
	{ INPUT_STRING_German, "German" },
	{ INPUT_STRING_Italian, "Italian" },
	{ INPUT_STRING_Korean, "Korean" },
	{ INPUT_STRING_Spanish, "Spanish" },
	{ INPUT_STRING_Very_Easy, "Very Easy" },
	{ INPUT_STRING_Easiest, "Easiest" },
	{ INPUT_STRING_Easier, "Easier" },
	{ INPUT_STRING_Easy, "Easy" },
	{ INPUT_STRING_Medium_Easy, "Medium Easy" },
	{ INPUT_STRING_Normal, "Normal" },
	{ INPUT_STRING_Medium, "Medium" },
	{ INPUT_STRING_Medium_Hard, "Medium Hard" },
	{ INPUT_STRING_Hard, "Hard" },
	{ INPUT_STRING_Harder, "Harder" },
	{ INPUT_STRING_Hardest, "Hardest" },
	{ INPUT_STRING_Very_Hard, "Very Hard" },
	{ INPUT_STRING_Medium_Difficult, "Medium Difficult" },
	{ INPUT_STRING_Difficult, "Difficult" },
	{ INPUT_STRING_Very_Difficult, "Very Difficult" },
	{ INPUT_STRING_Very_Low, "Very Low" },
	{ INPUT_STRING_Low, "Low" },
	{ INPUT_STRING_High, "High" },
	{ INPUT_STRING_Higher, "Higher" },
	{ INPUT_STRING_Highest, "Highest" },
	{ INPUT_STRING_Very_High, "Very High" },
	{ INPUT_STRING_Players, "Players" },
	{ INPUT_STRING_Controls, "Controls" },
	{ INPUT_STRING_Dual, "Dual" },
	{ INPUT_STRING_Single, "Single" },
	{ INPUT_STRING_Game_Time, "Game Time" },
	{ INPUT_STRING_Continue_Price, "Continue Price" },
	{ INPUT_STRING_Controller, "Controller" },
	{ INPUT_STRING_Light_Gun, "Light Gun" },
	{ INPUT_STRING_Joystick, "Joystick" },
	{ INPUT_STRING_Trackball, "Trackball" },
	{ INPUT_STRING_Continues, "Continues" },
	{ INPUT_STRING_Allow_Continue, "Allow Continue" },
	{ INPUT_STRING_Level_Select, "Level Select" },
	{ INPUT_STRING_Infinite, "Infinite" },
	{ INPUT_STRING_Stereo, "Stereo" },
	{ INPUT_STRING_Mono, "Mono" },
	{ INPUT_STRING_Unused, "Unused" },
	{ INPUT_STRING_Unknown, "Unknown" },
	{ INPUT_STRING_Standard, "Standard" },
	{ INPUT_STRING_Reverse, "Reverse" },
	{ INPUT_STRING_Alternate, "Alternate" },
	{ INPUT_STRING_None, "None" },
};


inline bool input_seq_good(running_machine &machine, input_seq const &seq)
{
	if (INPUT_CODE_INVALID == seq[0])
		return false;
	else if (seq.empty())
		return true;
	else
		return input_seq::end_code != machine.input().seq_clean(seq)[0];
}


std::string substitute_player(std::string_view name, u8 player)
{
	using util::lang_translate;

	std::string result;
	while (!name.empty())
	{
		auto const found = name.find('%');
		if ((std::string_view::npos == found) || (name.length() == found + 1))
		{
			result.append(name);
			break;
		}
		switch (name[found + 1])
		{
		case '%':
			result.append(name.substr(0, found + 1));
			break;
		case 'p':
			result.append(name.substr(0, found));
			result.append(util::string_format(_("input-name", "P%1$u"), player + 1));
			break;
		default:
			result.append(name.substr(0, found + 2));
		}
		name.remove_prefix(found + 2);
	}
	return result;
}



// ======================> inp_header

// header at the front of INP files
class inp_header
{
public:
	// parameters
	static constexpr unsigned MAJVERSION = 3;
	static constexpr unsigned MINVERSION = 0;

	bool read(emu_file &f)
	{
		return f.read(m_data, sizeof(m_data)) == sizeof(m_data);
	}
	bool write(emu_file &f) const
	{
		return f.write(m_data, sizeof(m_data)) == sizeof(m_data);
	}

	bool check_magic() const
	{
		return 0 == std::memcmp(MAGIC, m_data + OFFS_MAGIC, OFFS_BASETIME - OFFS_MAGIC);
	}
	u64 get_basetime() const
	{
		return
				(u64(m_data[OFFS_BASETIME + 0]) << (0 * 8)) |
				(u64(m_data[OFFS_BASETIME + 1]) << (1 * 8)) |
				(u64(m_data[OFFS_BASETIME + 2]) << (2 * 8)) |
				(u64(m_data[OFFS_BASETIME + 3]) << (3 * 8)) |
				(u64(m_data[OFFS_BASETIME + 4]) << (4 * 8)) |
				(u64(m_data[OFFS_BASETIME + 5]) << (5 * 8)) |
				(u64(m_data[OFFS_BASETIME + 6]) << (6 * 8)) |
				(u64(m_data[OFFS_BASETIME + 7]) << (7 * 8));
	}
	unsigned get_majversion() const
	{
		return m_data[OFFS_MAJVERSION];
	}
	unsigned get_minversion() const
	{
		return m_data[OFFS_MINVERSION];
	}
	std::string get_sysname() const
	{
		return get_string<OFFS_SYSNAME, OFFS_APPDESC>();
	}
	std::string get_appdesc() const
	{
		return get_string<OFFS_APPDESC, OFFS_END>();
	}

	void set_magic()
	{
		std::memcpy(m_data + OFFS_MAGIC, MAGIC, OFFS_BASETIME - OFFS_MAGIC);
	}
	void set_basetime(u64 time)
	{
		m_data[OFFS_BASETIME + 0] = u8((time >> (0 * 8)) & 0x00ff);
		m_data[OFFS_BASETIME + 1] = u8((time >> (1 * 8)) & 0x00ff);
		m_data[OFFS_BASETIME + 2] = u8((time >> (2 * 8)) & 0x00ff);
		m_data[OFFS_BASETIME + 3] = u8((time >> (3 * 8)) & 0x00ff);
		m_data[OFFS_BASETIME + 4] = u8((time >> (4 * 8)) & 0x00ff);
		m_data[OFFS_BASETIME + 5] = u8((time >> (5 * 8)) & 0x00ff);
		m_data[OFFS_BASETIME + 6] = u8((time >> (6 * 8)) & 0x00ff);
		m_data[OFFS_BASETIME + 7] = u8((time >> (7 * 8)) & 0x00ff);
	}
	void set_version()
	{
		m_data[OFFS_MAJVERSION] = MAJVERSION;
		m_data[OFFS_MINVERSION] = MINVERSION;
	}
	void set_sysname(std::string const &name)
	{
		set_string<OFFS_SYSNAME, OFFS_APPDESC>(name);
	}
	void set_appdesc(std::string const &desc)
	{
		set_string<OFFS_APPDESC, OFFS_END>(desc);
	}

private:
	template <std::size_t BEGIN, std::size_t END> void set_string(std::string const &str)
	{
		std::size_t const used = (std::min<std::size_t>)(str.size() + 1, END - BEGIN);
		std::memcpy(m_data + BEGIN, str.c_str(), used);
		if ((END - BEGIN) > used)
			std::memset(m_data + BEGIN + used, 0, (END - BEGIN) - used);
	}
	template <std::size_t BEGIN, std::size_t END> std::string get_string() const
	{
		char const *const begin = reinterpret_cast<char const *>(m_data + BEGIN);
		return std::string(begin, std::find(begin, reinterpret_cast<char const *>(m_data + END), '\0'));
	}

	static constexpr std::size_t    OFFS_MAGIC       = 0x00;    // 0x08 bytes
	static constexpr std::size_t    OFFS_BASETIME    = 0x08;    // 0x08 bytes (little-endian binary integer)
	static constexpr std::size_t    OFFS_MAJVERSION  = 0x10;    // 0x01 bytes (binary integer)
	static constexpr std::size_t    OFFS_MINVERSION  = 0x11;    // 0x01 bytes (binary integer)
																// 0x02 bytes reserved
	static constexpr std::size_t    OFFS_SYSNAME     = 0x14;    // 0x0c bytes (ASCII)
	static constexpr std::size_t    OFFS_APPDESC     = 0x20;    // 0x20 bytes (ASCII)
	static constexpr std::size_t    OFFS_END         = 0x40;

	static u8 const                 MAGIC[OFFS_BASETIME - OFFS_MAGIC];

	u8                              m_data[OFFS_END];
};

} // anonymous namespace


// XML attributes for the different types
const char *const ioport_manager::seqtypestrings[] = { "standard", "increment", "decrement" };


u8 const inp_header::MAGIC[inp_header::OFFS_BASETIME - inp_header::OFFS_MAGIC] = { 'M', 'A', 'M', 'E', 'I', 'N', 'P', 0 };



//**************************************************************************
//  BUILT-IN CORE MAPPINGS
//**************************************************************************

#include "inpttype.ipp"



//**************************************************************************
//  PORT CONFIGURATIONS
//**************************************************************************

//**************************************************************************
//  I/O PORT LIST
//**************************************************************************

//-------------------------------------------------
//  append - append the given device's input ports
//  to the current list
//-------------------------------------------------

void ioport_list::append(device_t &device, std::string &errorbuf)
{
	// no constructor, no list
	ioport_constructor constructor = device.input_ports();
	if (constructor == nullptr)
		return;

	// reset error buffer
	errorbuf.clear();

	// detokenize into the list
	(*constructor)(device, *this, errorbuf);

	// collapse fields and sort the list
	for (auto &port : *this)
		port.second->collapse_fields(errorbuf);
}



//**************************************************************************
//  INPUT TYPE ENTRY
//**************************************************************************

//-------------------------------------------------
//  input_type_entry - constructors
//-------------------------------------------------

input_type_entry::input_type_entry(ioport_type type, ioport_group group, int player, const char *token, const char *name, input_seq standard) noexcept
	: m_type(type),
		m_group(group),
		m_player(player),
		m_token(token),
		m_name(name)
{
	m_defseq[SEQ_TYPE_STANDARD] = m_seq[SEQ_TYPE_STANDARD] = standard;
}

input_type_entry::input_type_entry(ioport_type type, ioport_group group, int player, const char *token, const char *name, input_seq standard, input_seq decrement, input_seq increment) noexcept
	: m_type(type),
		m_group(group),
		m_player(player),
		m_token(token),
		m_name(name)
{
	m_defseq[SEQ_TYPE_STANDARD] = m_seq[SEQ_TYPE_STANDARD] = standard;
	m_defseq[SEQ_TYPE_INCREMENT] = m_seq[SEQ_TYPE_INCREMENT] = increment;
	m_defseq[SEQ_TYPE_DECREMENT] = m_seq[SEQ_TYPE_DECREMENT] = decrement;
}


//-------------------------------------------------
//  name - gets the display name for the input
//  type
//-------------------------------------------------

std::string input_type_entry::name() const
{
	using util::lang_translate;

	if (!m_name)
		return std::string();
	else if ((group() < IPG_PLAYER1) || (group() > IPG_PLAYER10))
		return _("input-name", m_name);
	else
		return substitute_player(_("input-name", m_name), player());
}


//-------------------------------------------------
//  replace_code - replace all instances of
//   oldcodewith newcode in all sequences
//-------------------------------------------------

void input_type_entry::replace_code(input_code oldcode, input_code newcode) noexcept
{
	for (input_seq &seq : m_seq)
		seq.replace(oldcode, newcode);
}


//-------------------------------------------------
//  configure_osd - set the token and name of an
//  OSD entry
//-------------------------------------------------

void input_type_entry::configure_osd(const char *token, const char *name) noexcept
{
	assert(m_type >= IPT_OSD_1 && m_type <= IPT_OSD_16);
	m_token = token;
	m_name = name;
}


//-------------------------------------------------
//  restore_default_seq - restores the sequence
//  from the default
//-------------------------------------------------

void input_type_entry::restore_default_seq() noexcept
{
	m_seq = m_defseq;
}


//**************************************************************************
//  DIGITAL JOYSTICKS
//**************************************************************************

//-------------------------------------------------
//  digital_joystick - constructor
//-------------------------------------------------

digital_joystick::digital_joystick(int player, int number)
	:   m_player(player),
		m_number(number),
		m_current(0),
		m_current4way(0),
		m_previous(0)
{
}


//-------------------------------------------------
//  set_axis - configure a single axis of a
//  digital joystick
//-------------------------------------------------

digital_joystick::direction_t digital_joystick::add_axis(ioport_field &field)
{
	direction_t direction = direction_t((field.type() - (IPT_DIGITAL_JOYSTICK_FIRST + 1)) % 4);
	m_field[direction].emplace_front(field);
	return direction;
}


//-------------------------------------------------
//  frame_update - update the state of digital
//  joysticks prior to accumulating the results
//  in a port
//-------------------------------------------------

void digital_joystick::frame_update()
{
	// remember previous state and reset current state
	m_previous = m_current;
	m_current = 0;

	// read all the associated ports
	running_machine *machine = nullptr;
	for (direction_t direction = JOYDIR_UP; direction < JOYDIR_COUNT; ++direction)
		for (const std::reference_wrapper<ioport_field> &i : m_field[direction])
		{
			machine = &i.get().machine();
			if (machine->input().seq_pressed(i.get().seq(SEQ_TYPE_STANDARD)))
				m_current |= 1 << direction;
		}

	// lock out opposing directions (left + right or up + down)
	if ((m_current & (UP_BIT | DOWN_BIT)) == (UP_BIT | DOWN_BIT))
		m_current &= ~(UP_BIT | DOWN_BIT);
	if ((m_current & (LEFT_BIT | RIGHT_BIT)) == (LEFT_BIT | RIGHT_BIT))
		m_current &= ~(LEFT_BIT | RIGHT_BIT);

	// only update 4-way case if joystick has moved
	if (m_current != m_previous)
	{
		m_current4way = m_current;

		//
		//  If joystick is pointing at a diagonal, acknowledge that the player moved
		//  the joystick by favoring a direction change.  This minimizes frustration
		//  and maximizes responsiveness.
		//
		//  For example, if you are holding "left" then switch to "up" (where both left
		//  and up are briefly pressed at the same time), we'll transition immediately
		//  to "up."
		//
		//  Zero any switches that didn't change from the previous to current state.
		//
		if ((m_current4way & (UP_BIT | DOWN_BIT)) &&
			(m_current4way & (LEFT_BIT | RIGHT_BIT)))
		{
			m_current4way ^= m_current4way & m_previous;
		}

		//
		//  If we are still pointing at a diagonal, we are in an indeterminant state.
		//
		//  This could happen if the player moved the joystick from the idle position directly
		//  to a diagonal, or from one diagonal directly to an extreme diagonal.
		//
		//  The chances of this happening with a keyboard are slim, but we still need to
		//  constrain this case. Let's pick the horizontal axis.
		//
		if ((m_current4way & (UP_BIT | DOWN_BIT)) &&
			(m_current4way & (LEFT_BIT | RIGHT_BIT)))
		{
			m_current4way &= ~(UP_BIT | DOWN_BIT);
		}
	}
}



//**************************************************************************
//  I/O PORT CONDITION
//**************************************************************************

//-------------------------------------------------
//  eval - evaluate condition
//-------------------------------------------------

bool ioport_condition::eval() const
{
	// always condition is always true
	if (m_condition == ALWAYS)
		return true;

	// otherwise, read the referenced port and switch off the condition type
	ioport_value const condvalue = m_port->read();
	switch (m_condition)
	{
		case ALWAYS:            return true;
		case EQUALS:            return ((condvalue & m_mask) == m_value);
		case NOTEQUALS:         return ((condvalue & m_mask) != m_value);
		case GREATERTHAN:       return ((condvalue & m_mask) > m_value);
		case NOTGREATERTHAN:    return ((condvalue & m_mask) <= m_value);
		case LESSTHAN:          return ((condvalue & m_mask) < m_value);
		case NOTLESSTHAN:       return ((condvalue & m_mask) >= m_value);
	}
	return true;
}


//-------------------------------------------------
//  initialize - create the live state
//-------------------------------------------------

void ioport_condition::initialize(device_t &device)
{
	if (m_tag != nullptr)
		m_port = device.ioport(m_tag);
}



//**************************************************************************
//  I/O PORT SETTING
//**************************************************************************

//-------------------------------------------------
//  ioport_setting - constructor
//-------------------------------------------------

ioport_setting::ioport_setting(ioport_field &field, ioport_value _value, const char *_name)
	: m_field(field),
		m_value(_value),
		m_name(_name)
{
}



//**************************************************************************
//  I/O PORT DIP LOCATION
//**************************************************************************

//-------------------------------------------------
//  ioport_diplocation - constructor
//-------------------------------------------------

ioport_diplocation::ioport_diplocation(const char *name, u8 swnum, bool invert)
	: m_name(name),
		m_number(swnum),
		m_invert(invert)
{
}



//**************************************************************************
//  I/O PORT FIELD
//**************************************************************************

//-------------------------------------------------
//  ioport_field - constructor
//-------------------------------------------------

ioport_field::ioport_field(ioport_port &port, ioport_type type, ioport_value defvalue, ioport_value maskbits, const char *name)
	: m_next(nullptr),
		m_port(port),
		m_modcount(port.modcount()),
		m_mask(maskbits),
		m_defvalue(defvalue & maskbits),
		m_type(type),
		m_player(0),
		m_flags(0),
		m_impulse(0),
		m_name(name),
		m_read(port.device()),
		m_write(port.device()),
		m_write_param(0),
		m_digital_value(false),
		m_min(0),
		m_max(maskbits),
		m_sensitivity(0),
		m_delta(0),
		m_centerdelta(0),
		m_crosshair_axis(CROSSHAIR_AXIS_NONE),
		m_crosshair_scale(1.0),
		m_crosshair_offset(0),
		m_crosshair_altaxis(0),
		m_crosshair_mapper(port.device()),
		m_full_turn_count(0),
		m_remap_table(nullptr),
		m_way(0)
{
	// reset sequences and chars
	for (input_seq_type seqtype = SEQ_TYPE_STANDARD; seqtype < SEQ_TYPE_TOTAL; ++seqtype)
		m_seq[seqtype].set_default();

	for (int i = 0; i < std::size(m_chars); i++)
		std::fill(std::begin(m_chars[i]), std::end(m_chars[i]), char32_t(0));

	// for DIP switches and configs, look for a default value from the owner
	if (type == IPT_DIPSWITCH || type == IPT_CONFIG)
	{
		const input_device_default *def = device().input_ports_defaults();
		if (def != nullptr)
		{
			const char *fulltag = port.tag();
			for ( ; def->tag != nullptr; def++)
				if (device().subtag(def->tag) == fulltag && def->mask == m_mask)
					m_defvalue = def->defvalue & m_mask;
		}

		m_flags |= FIELD_FLAG_TOGGLE;
	}
}


//-------------------------------------------------
//  ~ioport_field - destructor
//-------------------------------------------------

ioport_field::~ioport_field()
{
}


//-------------------------------------------------
//  set_value - programmatically set field value
//-------------------------------------------------

void ioport_field::set_value(ioport_value value)
{
	if (is_analog())
		live().analog->set_value(s32(value));
	else
		m_digital_value = value != 0;
}


//-------------------------------------------------
//  clear_value - clear programmatic override
//-------------------------------------------------

void ioport_field::clear_value()
{
	if (is_analog())
		live().analog->clear_value();
	else
		m_digital_value = false;
}


//-------------------------------------------------
//  name - return the field name for a given input
//  field (this must never return nullptr)
//-------------------------------------------------

std::string ioport_field::name() const
{
	using util::lang_translate;

	// if we have an overridden name, use that
	if (m_live && !m_live->name.empty())
		return m_live->name;

	// if no specific name, use the generic name for the type
	if (!m_name)
		return manager().type_name(m_type, m_player);

	// return name for non-controller fields as-is
	ioport_group const group = manager().type_group(m_type, m_player);
	if ((group < IPG_PLAYER1) || (group > IPG_PLAYER10))
		return m_name;

	// substitute the player number in if necessary
	return substitute_player(m_name, m_player);
}


//-------------------------------------------------
//  seq - return the live input sequence for the
//  given input field
//-------------------------------------------------

const input_seq &ioport_field::seq(input_seq_type seqtype) const noexcept
{
	// if the sequence is not the special default code, return it
	if (m_live && !m_live->seq[seqtype].is_default())
		return m_live->seq[seqtype];

	// otherwise return the default sequence
	return defseq(seqtype);
}


//-------------------------------------------------
//  defseq - return the default input sequence for
//  the given input field
//-------------------------------------------------

const input_seq &ioport_field::defseq(input_seq_type seqtype) const noexcept
{
	// if the sequence is the special default code, return the expanded default value
	if (m_seq[seqtype].is_default())
		return manager().type_seq(m_type, m_player, seqtype);

	// otherwise, return the sequence as-is
	return m_seq[seqtype];
}


//-------------------------------------------------
//  set_defseq - dynamically alter the default
//  input sequence for the given input field
//-------------------------------------------------

void ioport_field::set_defseq(input_seq_type seqtype, const input_seq &newseq)
{
	// set the new sequence
	m_seq[seqtype] = newseq;
}


//-------------------------------------------------
//  type_class - return the type class for this
//  field
//-------------------------------------------------

ioport_type_class ioport_field::type_class() const noexcept
{
	// inputs associated with specific players
	ioport_group const group = manager().type_group(m_type, m_player);
	if ((group >= IPG_PLAYER1) && (group <= IPG_PLAYER10))
		return INPUT_CLASS_CONTROLLER;

	// keys (names derived from character codes)
	if (m_type == IPT_KEYPAD || m_type == IPT_KEYBOARD)
		return INPUT_CLASS_KEYBOARD;

	// configuration settings (specific names required)
	if (m_type == IPT_CONFIG)
		return INPUT_CLASS_CONFIG;

	// DIP switches (specific names required)
	if (m_type == IPT_DIPSWITCH)
		return INPUT_CLASS_DIPSWITCH;

	// miscellaneous non-player inputs (named and user-mappable)
	if (group == IPG_OTHER || (group == IPG_INVALID && m_name != nullptr))
		return INPUT_CLASS_MISC;

	// internal inputs (these may be anonymous)
	return INPUT_CLASS_INTERNAL;
}


//-------------------------------------------------
//  keyboard_codes - accesses a particular keyboard
//  code list
//-------------------------------------------------

std::vector<char32_t> ioport_field::keyboard_codes(int which) const
{
	if (which >= std::size(m_chars))
		throw emu_fatalerror("Tried to access keyboard_code with out-of-range index %d\n", which);

	std::vector<char32_t> result;
	for (int i = 0; i < std::size(m_chars[which]) && m_chars[which][i] != 0; i++)
		result.push_back(m_chars[which][i]);

	return result;
}


//-------------------------------------------------
//  key_name - returns the name of a specific key
//-------------------------------------------------

std::string ioport_field::key_name(int which) const
{
	std::vector<char32_t> codes = keyboard_codes(which);
	char32_t ch = codes.empty() ? 0 : codes[0];

	// attempt to get the string from the character info table
	switch (ch)
	{
	case 8: return "Backspace";
	case 9: return "Tab";
	case 12: return "Clear";
	case 13: return "Enter";
	case 27: return "Esc";
	case 32: return "Space";
	case UCHAR_SHIFT_1: return "Shift";
	case UCHAR_SHIFT_2: return "Ctrl";
	case UCHAR_MAMEKEY(ESC): return "Esc";
	case UCHAR_MAMEKEY(INSERT): return "Insert";
	case UCHAR_MAMEKEY(DEL): return "Delete";
	case UCHAR_MAMEKEY(HOME): return "Home";
	case UCHAR_MAMEKEY(END): return "End";
	case UCHAR_MAMEKEY(PGUP): return "Page Up";
	case UCHAR_MAMEKEY(PGDN): return "Page Down";
	case UCHAR_MAMEKEY(LEFT): return "Cursor Left";
	case UCHAR_MAMEKEY(RIGHT): return "Cursor Right";
	case UCHAR_MAMEKEY(UP): return "Cursor Up";
	case UCHAR_MAMEKEY(DOWN): return "Cursor Down";
	case UCHAR_MAMEKEY(SLASH_PAD): return "Keypad /";
	case UCHAR_MAMEKEY(ASTERISK): return "Keypad *";
	case UCHAR_MAMEKEY(MINUS_PAD): return "Keypad -";
	case UCHAR_MAMEKEY(PLUS_PAD): return "Keypad +";
	case UCHAR_MAMEKEY(DEL_PAD): return "Keypad .";
	case UCHAR_MAMEKEY(ENTER_PAD): return "Keypad Enter";
	case UCHAR_MAMEKEY(BS_PAD): return "Keypad Backspace";
	case UCHAR_MAMEKEY(TAB_PAD): return "Keypad Tab";
	case UCHAR_MAMEKEY(00_PAD): return "Keypad 00";
	case UCHAR_MAMEKEY(000_PAD): return "Keypad 000";
	case UCHAR_MAMEKEY(COMMA_PAD): return "Keypad ,";
	case UCHAR_MAMEKEY(EQUALS_PAD): return "Keypad =";
	case UCHAR_MAMEKEY(PRTSCR): return "Print Screen";
	case UCHAR_MAMEKEY(PAUSE): return "Pause";
	case UCHAR_MAMEKEY(LSHIFT): return "Left Shift";
	case UCHAR_MAMEKEY(RSHIFT): return "Right Shift";
	case UCHAR_MAMEKEY(LCONTROL): return "Left Ctrl";
	case UCHAR_MAMEKEY(RCONTROL): return "Right Ctrl";
	case UCHAR_MAMEKEY(LALT): return "Left Alt";
	case UCHAR_MAMEKEY(RALT): return "Right Alt";
	case UCHAR_MAMEKEY(SCRLOCK): return "Scroll Lock";
	case UCHAR_MAMEKEY(NUMLOCK): return "Num Lock";
	case UCHAR_MAMEKEY(CAPSLOCK): return "Caps Lock";
	case UCHAR_MAMEKEY(LWIN): return "Left Win";
	case UCHAR_MAMEKEY(RWIN): return "Right Win";
	case UCHAR_MAMEKEY(MENU): return "Menu";
	case UCHAR_MAMEKEY(CANCEL): return "Break";
	default: break;
	}

	// handle function keys
	if (ch >= UCHAR_MAMEKEY(F1) && ch <= UCHAR_MAMEKEY(F20))
		return util::string_format("F%d", ch - UCHAR_MAMEKEY(F1) + 1);

	// handle 0-9 on numeric keypad
	if (ch >= UCHAR_MAMEKEY(0_PAD) && ch <= UCHAR_MAMEKEY(9_PAD))
		return util::string_format("Keypad %d", ch - UCHAR_MAMEKEY(0_PAD));

	// if that doesn't work, convert to UTF-8
	if (ch > 0x7F || isprint(ch))
		return utf8_from_uchar(ch);

	// otherwise, opt for question marks
	return "???";
}


//-------------------------------------------------
//  get_user_settings - return the current
//  settings for the given input field
//-------------------------------------------------

void ioport_field::get_user_settings(user_settings &settings) const
{
	// zap the entire structure
	settings = user_settings();

	// copy the basics
	for (input_seq_type seqtype = SEQ_TYPE_STANDARD; seqtype < SEQ_TYPE_TOTAL; ++seqtype)
	{
		settings.seq[seqtype] = seq(seqtype);
		if (m_live)
			settings.cfg[seqtype] = m_live->cfg[seqtype];
	}

	// if there's a list of settings or we're an adjuster, copy the current value
	if (!m_settinglist.empty() || m_type == IPT_ADJUSTER)
		settings.value = m_live->value;

	if (m_live->analog != nullptr)
	{
		// if there's analog data, extract the analog settings
		settings.sensitivity = m_live->analog->sensitivity();
		settings.delta = m_live->analog->delta();
		settings.centerdelta = m_live->analog->centerdelta();
		settings.reverse = m_live->analog->reverse();
	}
	else
	{
		// non-analog settings
		settings.toggle = m_live->toggle;
	}
}


//-------------------------------------------------
//  set_user_settings - modify the current
//  settings for the given input field
//-------------------------------------------------

void ioport_field::set_user_settings(const user_settings &settings)
{
	// copy the basics
	for (input_seq_type seqtype = SEQ_TYPE_STANDARD; seqtype < SEQ_TYPE_TOTAL; ++seqtype)
	{
		if (settings.seq[seqtype].is_default())
			m_live->seq[seqtype].set_default();
		else
			m_live->seq[seqtype] = settings.seq[seqtype];
		m_live->cfg[seqtype] = settings.cfg[seqtype];
	}

	// if there's a list of settings or we're an adjuster, copy the current value
	if (!m_settinglist.empty() || m_type == IPT_ADJUSTER)
		m_live->value = settings.value;

	if (m_live->analog)
	{
		// if there's analog data, extract the analog settings
		m_live->analog->m_sensitivity = settings.sensitivity;
		m_live->analog->m_delta = settings.delta;
		m_live->analog->m_centerdelta = settings.centerdelta;
		m_live->analog->m_reverse = settings.reverse;
	}
	else
	{
		// non-analog settings
		m_live->toggle = settings.toggle;
	}
}


//-------------------------------------------------
//  setting_name - return the expanded setting
//  name for a field
//-------------------------------------------------

const char *ioport_field::setting_name() const
{
	// only makes sense if we have settings
	assert(!m_settinglist.empty());

	// scan the list of settings looking for a match on the current value
	for (ioport_setting const &setting : m_settinglist)
		if (setting.enabled())
			if (setting.value() == m_live->value)
				return setting.name();

	return "INVALID";
}


//-------------------------------------------------
//  has_previous_setting - return true if the
//  given field has a "previous" setting
//-------------------------------------------------

bool ioport_field::has_previous_setting() const
{
	// only makes sense if we have settings
	assert(!m_settinglist.empty());

	// scan the list of settings looking for a match on the current value
	for (ioport_setting const &setting : m_settinglist)
		if (setting.enabled())
			return (setting.value() != m_live->value);

	return false;
}


//-------------------------------------------------
//  select_previous_setting - select the previous
//  item for a DIP switch or configuration field
//-------------------------------------------------

void ioport_field::select_previous_setting()
{
	// only makes sense if we have settings
	assert(!m_settinglist.empty());

	// scan the list of settings looking for a match on the current value
	auto prevsetting = m_settinglist.end();
	bool found_match = false;
	for (auto setting = m_settinglist.begin(); m_settinglist.end() != setting; ++setting)
	{
		if (setting->enabled())
		{
			if (setting->value() == m_live->value)
			{
				found_match = true;
				if (m_settinglist.end() != prevsetting)
					break;
			}
			prevsetting = setting;
		}
	}

	// if we didn't find a matching value, select the first
	if (!found_match)
	{
		prevsetting = m_settinglist.begin();
		while ((m_settinglist.end() != prevsetting) && !prevsetting->enabled())
			++prevsetting;
	}

	// update the value to the previous one
	if (m_settinglist.end() != prevsetting)
		m_live->value = prevsetting->value();
}


//-------------------------------------------------
//  has_next_setting - return true if the given
//  field has a "next" setting
//-------------------------------------------------

bool ioport_field::has_next_setting() const
{
	// only makes sense if we have settings
	assert(!m_settinglist.empty());

	// scan the list of settings looking for a match on the current value
	bool found = false;
	for (ioport_setting const &setting : m_settinglist)
	{
		if (setting.enabled())
		{
			if (found)
				return true;
			if (setting.value() == m_live->value)
				found = true;
		}
	}
	return false;
}


//-------------------------------------------------
//  select_next_setting - select the next item for
//  a DIP switch or configuration field
//-------------------------------------------------

void ioport_field::select_next_setting()
{
	// only makes sense if we have settings
	assert(!m_settinglist.empty());

	// scan the list of settings looking for a match on the current value
	auto setting = m_settinglist.begin();
	while ((m_settinglist.end() != setting) && (!setting->enabled() || (setting->value() != m_live->value)))
		++setting;

	// if we found one, scan forward for the next valid one
	auto nextsetting = setting;
	if (m_settinglist.end() != nextsetting)
	{
		++nextsetting;
		while ((m_settinglist.end() != nextsetting) && !nextsetting->enabled())
			++nextsetting;
	}

	// if we hit the end, search from the beginning
	if (m_settinglist.end() == nextsetting)
	{
		nextsetting = m_settinglist.begin();
		while ((m_settinglist.end() != nextsetting) && !nextsetting->enabled())
			++nextsetting;
	}

	// update the value to the previous one
	if (m_settinglist.end() != nextsetting)
		m_live->value = nextsetting->value();
}


//-------------------------------------------------
//  frame_update_digital - get the state of a
//  digital field
//-------------------------------------------------

void ioport_field::frame_update(ioport_value &result)
{
	// skip if not enabled
	if (!enabled())
		return;

	// handle analog inputs first
	if (m_live->analog != nullptr)
	{
		m_live->analog->frame_update(machine());
		return;
	}

	// if UI is active, ignore digital inputs
	if (machine().ui().is_menu_active())
		return;

	// if user input is locked out here, bail
	if (m_live->lockout)
	{
		// use just the digital value
		if (m_digital_value)
			result |= m_mask;
		return;
	}

	// if the state changed, look for switch down/switch up
	bool curstate = m_digital_value || machine().input().seq_pressed(seq());
	bool changed = false;
	if (curstate != m_live->last)
	{
		m_live->last = curstate;
		changed = true;
	}

	// coin impulse option
	int effective_impulse = m_impulse;
	int impulse_option_val = machine().options().coin_impulse();
	if (impulse_option_val != 0)
	{
		if (impulse_option_val < 0)
			effective_impulse = 0;
		else if ((m_type >= IPT_COIN1 && m_type <= IPT_COIN12) || m_impulse != 0)
			effective_impulse = impulse_option_val;
	}

	// if this is a switch-down event, handle impulse and toggle
	if (changed && curstate)
	{
		// impulse controls: reset the impulse counter
		if (effective_impulse != 0 && m_live->impulse == 0)
			m_live->impulse = effective_impulse;

		// toggle controls: flip the toggle state or advance to the next setting
		if (m_live->toggle)
		{
			if (m_settinglist.empty())
				m_live->value ^= m_mask;
			else
				select_next_setting();
		}
	}

	// update the current state with the impulse state
	if (effective_impulse != 0)
	{
		curstate = (m_live->impulse != 0);
		if (curstate)
			m_live->impulse--;
	}

	// for toggle switches, the current value is folded into the port's default value
	// so we always return false here
	if (m_live->toggle)
		curstate = false;

	// additional logic to restrict digital joysticks
	if (curstate && !m_digital_value && m_live->joystick != nullptr && m_way != 16 && !machine().options().joystick_contradictory())
	{
		u8 mask = (m_way == 4) ? m_live->joystick->current4way() : m_live->joystick->current();
		if (!(mask & (1 << m_live->joydir)))
			curstate = false;
	}

	// skip locked-out coin inputs
	if (curstate && m_type >= IPT_COIN1 && m_type <= IPT_COIN12 && machine().bookkeeping().coin_lockout_get_state(m_type - IPT_COIN1))
	{
		bool verbose = machine().options().verbose();
#ifdef MAME_DEBUG
		verbose = true;
#endif
		if (machine().options().coin_lockout())
		{
			if (verbose)
				machine().ui().popup_time(3, "Coinlock disabled %s.", name());
			curstate = false;
		}
		else
			if (verbose)
				machine().ui().popup_time(3, "Coinlock disabled, but broken through %s.", name());
	}

	// if we're active, set the appropriate bits in the digital state
	if (curstate)
		result |= m_mask;
}


//-------------------------------------------------
//  crosshair_read - compute the crosshair
//  position
//-------------------------------------------------

float ioport_field::crosshair_read() const
{
	float value = m_live->analog->crosshair_read();

	// apply the scale and offset
	if (m_crosshair_scale < 0)
		value = -(1.0f - value) * m_crosshair_scale;
	else
		value *= m_crosshair_scale;
	value += m_crosshair_offset;

	// apply custom mapping if necessary
	if (!m_crosshair_mapper.isnull())
		value = m_crosshair_mapper(value);

	return value;
}


//-------------------------------------------------
//  expand_diplocation - expand a string-based
//  DIP location into a linked list of
//  descriptions
//-------------------------------------------------

void ioport_field::expand_diplocation(const char *location, std::string &errorbuf)
{
	// if nothing present, bail
	if (!location)
		return;

	m_diploclist.clear();

	// parse the string
	std::string name; // Don't move this variable inside the loop, lastname's lifetime depends on it being outside
	const char *lastname = nullptr;
	const char *curentry = location;
	int entries = 0;
	while (*curentry != 0)
	{
		// find the end of this entry
		const char *comma = strchr(curentry, ',');
		if (comma == nullptr)
			comma = curentry + strlen(curentry);

		// extract it to tempbuf
		std::string tempstr(curentry, comma - curentry);

		// first extract the switch name if present
		const char *number = tempstr.c_str();
		const char *colon = strchr(tempstr.c_str(), ':');

		if (colon != nullptr)
		{
			// allocate and copy the name if it is present
			lastname = name.assign(number, colon - number).c_str();
			number = colon + 1;
		}
		else
		{
			// otherwise, just copy the last name
			if (lastname == nullptr)
			{
				errorbuf.append(string_format("Switch location '%s' missing switch name!\n", location));
				lastname = (char *)"UNK";
			}
			name.assign(lastname);
		}

		// if the number is preceded by a '!' it's active high
		bool invert = false;
		if (*number == '!')
		{
			invert = true;
			number++;
		}

		// now scan the switch number
		int swnum = -1;
		if (sscanf(number, "%d", &swnum) != 1)
			errorbuf.append(string_format("Switch location '%s' has invalid format!\n", location));

		// allocate a new entry
		m_diploclist.emplace_back(name.c_str(), swnum, invert);
		entries++;

		// advance to the next item
		curentry = comma;
		if (*curentry != 0)
			curentry++;
	}

	// then verify the number of bits in the mask matches
	ioport_value temp;
	int bits;
	for (bits = 0, temp = m_mask; temp != 0 && bits < 32; bits++)
		temp &= temp - 1;
	if (bits != entries)
		errorbuf.append(string_format("Switch location '%s' does not describe enough bits for mask %X\n", location, m_mask));
}


//-------------------------------------------------
//  init_live_state - create live state structures
//-------------------------------------------------

void ioport_field::init_live_state(analog_field *analog)
{
	// resolve callbacks
	m_read.resolve();
	m_write.resolve();
	m_crosshair_mapper.resolve();

	// allocate live state
	m_live = std::make_unique<ioport_field_live>(*this, analog);

	m_condition.initialize(device());

	for (ioport_setting &setting : m_settinglist)
		setting.condition().initialize(setting.device());
}



//**************************************************************************
//  I/O PORT FIELD LIVE
//**************************************************************************

//-------------------------------------------------
//  ioport_field_live - constructor
//-------------------------------------------------

ioport_field_live::ioport_field_live(ioport_field &field, analog_field *analog)
	: analog(analog),
		joystick(nullptr),
		value(field.defvalue()),
		impulse(0),
		last(0),
		toggle(field.toggle()),
		joydir(digital_joystick::JOYDIR_COUNT),
		lockout(false)
{
	// fill in the basic values
	for (input_seq_type seqtype = SEQ_TYPE_STANDARD; seqtype < SEQ_TYPE_TOTAL; ++seqtype)
		seq[seqtype] = field.defseq_unresolved(seqtype);

	// if this is a digital joystick field, make a note of it
	if (field.is_digital_joystick())
	{
		joystick = &field.manager().digjoystick(field.player(), (field.type() - (IPT_DIGITAL_JOYSTICK_FIRST + 1)) / 4);
		joydir = joystick->add_axis(field);
	}

	// Name keyboard key names
	if (field.type_class() == INPUT_CLASS_KEYBOARD && field.specific_name() == nullptr)
	{
		// loop through each character on the field
		for (int which = 0; which < (1 << (UCHAR_SHIFT_END - UCHAR_SHIFT_BEGIN + 1)); which++)
		{
			std::vector<char32_t> const codes = field.keyboard_codes(which);
			if (codes.empty())
				break;
			name.append(string_format("%-*s ", std::max(SPACE_COUNT - 1, 0), field.key_name(which)));
		}

		// trim extra spaces
		name = strtrimspace(name);

		// special case
		if (name.empty())
			name.assign("Unnamed Key");
	}
}



//**************************************************************************
//  I/O PORT
//**************************************************************************

//-------------------------------------------------
//  ioport_port - constructor
//-------------------------------------------------

ioport_port::ioport_port(device_t &owner, const char *tag)
	: m_next(nullptr),
		m_device(owner),
		m_tag(tag),
		m_modcount(0),
		m_active(0)
{
}


//-------------------------------------------------
//  ~ioport_port - destructor
//-------------------------------------------------

ioport_port::~ioport_port()
{
}


//-------------------------------------------------
//  machine - return a reference to the running
//  machine
//-------------------------------------------------

running_machine &ioport_port::machine() const
{
	return m_device.machine();
}


//-------------------------------------------------
//  manager - return a reference to the
//  ioport_manager on the running machine
//-------------------------------------------------

ioport_manager &ioport_port::manager() const
{
	return machine().ioport();
}


//-------------------------------------------------
//  field - return a pointer to the first field
//  that intersects the given mask
//-------------------------------------------------

ioport_field *ioport_port::field(ioport_value mask) const
{
	// if we got the port, look for the field
	for (ioport_field &field : fields())
		if ((field.mask() & mask) != 0 && field.enabled())
			return &field;
	return nullptr;
}


//-------------------------------------------------
//  read - return the value of an I/O port
//-------------------------------------------------

ioport_value ioport_port::read()
{
	if (!manager().safe_to_read())
		throw emu_fatalerror("Input ports cannot be read at init time!");

	// start with the digital state
	ioport_value result = m_live->digital;

	// insert dynamic read values
	for (dynamic_field &dynfield : m_live->readlist)
		dynfield.read(result);

	// apply active high/low state to digital and dynamic read inputs
	result ^= m_live->defvalue;

	// insert analog portions
	for (analog_field &analog : m_live->analoglist)
		analog.read(result);

	return result;
}


//-------------------------------------------------
//  write - write a value to a port
//-------------------------------------------------

void ioport_port::write(ioport_value data, ioport_value mem_mask)
{
	// call device line write handlers
	COMBINE_DATA(&m_live->outputvalue);
	for (dynamic_field &dynfield : m_live->writelist)
		if (dynfield.field().type() == IPT_OUTPUT)
			dynfield.write(m_live->outputvalue ^ dynfield.field().defvalue());
}


//-------------------------------------------------
//  frame_update - once/frame update
//-------------------------------------------------

void ioport_port::frame_update()
{
	// start with 0 values for the digital bits
	m_live->digital = 0;

	// now loop back and modify based on the inputs
	for (ioport_field &field : m_fieldlist)
		field.frame_update(m_live->digital);
}


//-------------------------------------------------
//  collapse_fields - remove any fields that are
//  wholly overlapped by other fields
//-------------------------------------------------

void ioport_port::collapse_fields(std::string &errorbuf)
{
	ioport_value maskbits = 0;
	int lastmodcount = -1;

	// remove the whole list and start from scratch
	ioport_field *field = m_fieldlist.detach_all();
	while (field != nullptr)
	{
		// if this modcount doesn't match, reset
		if (field->modcount() != lastmodcount)
		{
			lastmodcount = field->modcount();
			maskbits = 0;
		}

		// reinsert this field
		ioport_field *current = field;
		field = field->next();
		insert_field(*current, maskbits, errorbuf);
	}
}


//-------------------------------------------------
//  insert_field - insert a new field, checking
//  for errors
//-------------------------------------------------

void ioport_port::insert_field(ioport_field &newfield, ioport_value &disallowedbits, std::string &errorbuf)
{
	// verify against the disallowed bits, but only if we are condition-free
	if (newfield.condition().none())
	{
		if ((newfield.mask() & disallowedbits) != 0)
			errorbuf.append(string_format("INPUT_TOKEN_FIELD specifies duplicate port bits (port=%s mask=%X)\n", tag(), newfield.mask()));
		disallowedbits |= newfield.mask();
	}

	// first modify/nuke any entries that intersect our maskbits
	ioport_field *nextfield;
	for (ioport_field *field = m_fieldlist.first(); field != nullptr; field = nextfield)
	{
		nextfield = field->next();
		if ((field->mask() & newfield.mask()) != 0 &&
			(newfield.condition().none() || field->condition().none() || field->condition() == newfield.condition()))
		{
			// reduce the mask of the field we found
			field->reduce_mask(newfield.mask());

			// if the new entry fully overrides the previous one, we nuke
			if (INPUT_PORT_OVERRIDE_FULLY_NUKES_PREVIOUS || field->mask() == 0)
				m_fieldlist.remove(*field);
		}
	}

	// make a mask of just the low bit
	ioport_value lowbit = (newfield.mask() ^ (newfield.mask() - 1)) & newfield.mask();

	// scan forward to find where to insert ourselves
	ioport_field *field;
	for (field = m_fieldlist.first(); field != nullptr; field = field->next())
		if (field->mask() > lowbit)
			break;

	// insert it into the list
	m_fieldlist.insert_before(newfield, field);
}


//-------------------------------------------------
//  init_live_state - create the live state
//-------------------------------------------------

void ioport_port::init_live_state()
{
	m_live = std::make_unique<ioport_port_live>(*this);
}


//-------------------------------------------------
//  update_defvalue - force an update to the input
//  port values based on current conditions
//-------------------------------------------------

void ioport_port::update_defvalue(bool flush_defaults)
{
	// only clear on the first pass
	if (flush_defaults)
		m_live->defvalue = 0;

	// recompute the default value for the entire port
	for (ioport_field &field : m_fieldlist)
		if (field.enabled())
			m_live->defvalue = (m_live->defvalue & ~field.mask()) | (field.live().value & field.mask());
}



//**************************************************************************
//  I/O PORT LIVE STATE
//**************************************************************************

//-------------------------------------------------
//  ioport_port_live - constructor
//-------------------------------------------------

ioport_port_live::ioport_port_live(ioport_port &port)
	: defvalue(0),
		digital(0),
		outputvalue(0)
{
	// iterate over fields
	for (ioport_field &field : port.fields())
	{
		// allocate analog state if it's analog
		analog_field *analog = nullptr;
		if (field.is_analog())
			analog = &analoglist.emplace_back(field);

		// allocate a dynamic field for reading
		if (field.has_dynamic_read())
			readlist.emplace_back(field);

		// allocate a dynamic field for writing
		if (field.has_dynamic_write())
			writelist.emplace_back(field);

		// let the field initialize its live state
		field.init_live_state(analog);
	}
}



//**************************************************************************
//  I/O PORT MANAGER
//**************************************************************************

//-------------------------------------------------
//  ioport_manager - constructor
//-------------------------------------------------

ioport_manager::ioport_manager(running_machine &machine)
	: m_machine(machine)
	, m_safe_to_read(false)
	, m_last_frame_time(attotime::zero)
	, m_last_delta_nsec(0)
	, m_playback_accumulated_speed(0)
	, m_playback_accumulated_frames(0)
	, m_deselected_card_config()
{
	for (auto &entries : m_type_to_entry)
		std::fill(std::begin(entries), std::end(entries), nullptr);
}


//-------------------------------------------------
//  initialize - walk the configured ports and
//  create live state information
//-------------------------------------------------

time_t ioport_manager::initialize()
{
	// add an exit callback and a frame callback
	machine().add_notifier(MACHINE_NOTIFY_EXIT, machine_notify_delegate(&ioport_manager::exit, this));
	machine().add_notifier(MACHINE_NOTIFY_FRAME, machine_notify_delegate(&ioport_manager::frame_update_callback, this));

	// initialize the default port info from the OSD
	init_port_types();

	// if we have a token list, proceed
	device_enumerator iter(machine().root_device());
	for (device_t &device : iter)
	{
		std::string errors;
		m_portlist.append(device, errors);
		if (!errors.empty())
			osd_printf_error("Input port errors:\n%s", errors);
	}

	// renumber player numbers for controller ports
	int player_offset = 0;
	for (device_t &device : iter)
	{
		int players = 0;
		for (auto &port : m_portlist)
		{
			if (&port.second->device() == &device)
			{
				for (ioport_field &field : port.second->fields())
				{
					if (field.type_class() == INPUT_CLASS_CONTROLLER)
					{
						if (players < field.player() + 1)
							players = field.player() + 1;
						field.set_player(field.player() + player_offset);
					}
				}
			}
		}
		player_offset += players;
	}

	// allocate live structures to mirror the configuration
	for (auto &port : m_portlist)
		port.second->init_live_state();

	// handle autoselection of devices
	init_autoselect_devices(IPT_AD_STICK_X,  IPT_AD_STICK_Y,   IPT_AD_STICK_Z, OPTION_ADSTICK_DEVICE,    "analog joystick");
	init_autoselect_devices(IPT_PADDLE,      IPT_PADDLE_V,     0,              OPTION_PADDLE_DEVICE,     "paddle");
	init_autoselect_devices(IPT_PEDAL,       IPT_PEDAL2,       IPT_PEDAL3,     OPTION_PEDAL_DEVICE,      "pedal");
	init_autoselect_devices(IPT_LIGHTGUN_X,  IPT_LIGHTGUN_Y,   0,              OPTION_LIGHTGUN_DEVICE,   "lightgun");
	init_autoselect_devices(IPT_POSITIONAL,  IPT_POSITIONAL_V, 0,              OPTION_POSITIONAL_DEVICE, "positional");
	init_autoselect_devices(IPT_DIAL,        IPT_DIAL_V,       0,              OPTION_DIAL_DEVICE,       "dial");
	init_autoselect_devices(IPT_TRACKBALL_X, IPT_TRACKBALL_Y,  0,              OPTION_TRACKBALL_DEVICE,  "trackball");
	init_autoselect_devices(IPT_MOUSE_X,     IPT_MOUSE_Y,      0,              OPTION_MOUSE_DEVICE,      "mouse");

	// look for 4-way diagonal joysticks and change the default map if we find any
	const char *joystick_map_default = machine().options().joystick_map();
	if (joystick_map_default[0] == 0 || strcmp(joystick_map_default, "auto") == 0)
		for (auto &port : m_portlist)
			for (ioport_field const &field : port.second->fields())
				if (field.live().joystick != nullptr && field.rotated())
				{
					input_class_joystick &devclass = downcast<input_class_joystick &>(machine().input().device_class(DEVICE_CLASS_JOYSTICK));
					devclass.set_global_joystick_map(input_class_joystick::map_4way_diagonal);
					break;
				}

	// register callbacks for when we load configurations
	machine().configuration().config_register(
			"input",
			configuration_manager::load_delegate(&ioport_manager::load_config, this),
			configuration_manager::save_delegate(&ioport_manager::save_config, this));

	// open playback and record files if specified
	time_t basetime = playback_init();
	record_init();
	return basetime;
}


//-------------------------------------------------
//  init_port_types - initialize the default
//  type list
//-------------------------------------------------

void ioport_manager::init_port_types()
{
	// convert the array into a list of type states that can be modified
	emplace_core_types(m_typelist);

	// ask the OSD to customize the list
	machine().osd().customize_input_type_list(m_typelist);

	// now iterate over the OSD-modified types
	for (input_type_entry &curtype : m_typelist)
	{
		// first copy all the OSD-updated sequences into our current state
		curtype.restore_default_seq();

		// also make a lookup table mapping type/player to the appropriate type list entry
		m_type_to_entry[curtype.type()][curtype.player()] = &curtype;
	}
}


//-------------------------------------------------
//  init_autoselect_devices - autoselect a single
//  device based on the input port list passed
//  in and the corresponding option
//-------------------------------------------------

void ioport_manager::init_autoselect_devices(int type1, int type2, int type3, const char *option, const char *ananame)
{
	// if nothing specified, ignore the option
	const char *stemp = machine().options().value(option);
	if (stemp[0] == 0 || strcmp(stemp, "none") == 0)
		return;

	// extract valid strings
	input_class *autoenable_class = nullptr;
	for (input_device_class devclass = DEVICE_CLASS_FIRST_VALID; devclass <= DEVICE_CLASS_LAST_VALID; ++devclass)
		if (strcmp(stemp, machine().input().device_class(devclass).name()) == 0)
		{
			autoenable_class = &machine().input().device_class(devclass);
			break;
		}
	if (autoenable_class == nullptr)
	{
		osd_printf_error("Invalid %s value %s; reverting to keyboard\n", option, stemp);
		autoenable_class = &machine().input().device_class(DEVICE_CLASS_KEYBOARD);
	}

	// only scan the list if we haven't already enabled this class of control
	if (!autoenable_class->enabled())
		for (auto &port : m_portlist)
			for (ioport_field const &field : port.second->fields())

				// if this port type is in use, apply the autoselect criteria
				if ((type1 != 0 && field.type() == type1) || (type2 != 0 && field.type() == type2) || (type3 != 0 && field.type() == type3))
				{
					osd_printf_verbose("Input: Autoenabling %s due to presence of a %s\n", autoenable_class->name(), ananame);
					autoenable_class->enable();
					break;
				}
}


//-------------------------------------------------
//  exit - exit callback to ensure we clean up
//  and close our files
//-------------------------------------------------

void ioport_manager::exit()
{
	// close any playback or recording files
	playback_end();
	record_end();
}


//-------------------------------------------------
//  ~ioport_manager - destructor
//-------------------------------------------------

ioport_manager::~ioport_manager()
{
}


//-------------------------------------------------
//  type_name - return the name for the given
//  type/player
//-------------------------------------------------

std::string ioport_manager::type_name(ioport_type type, u8 player) const
{
	using util::lang_translate;

	// if we have a machine, use the live state and quick lookup
	input_type_entry const *const entry = m_type_to_entry[type][player];
	if (entry)
	{
		std::string name = entry->name();
		if (!name.empty())
			return name;
	}

	// if we find nothing, return a default string (not a null pointer)
	return _("input-name", "???");
}


//-------------------------------------------------
//  type_group - return the group for the given
//  type/player
//-------------------------------------------------

ioport_group ioport_manager::type_group(ioport_type type, int player) const noexcept
{
	input_type_entry *entry = m_type_to_entry[type][player];
	if (entry != nullptr)
		return entry->group();

	// if we find nothing, return an invalid group
	return IPG_INVALID;
}


//-------------------------------------------------
//  type_seq - return the input sequence for the
//  given type/player
//-------------------------------------------------

const input_seq &ioport_manager::type_seq(ioport_type type, int player, input_seq_type seqtype) const noexcept
{
	assert(type >= 0 && type < IPT_COUNT);
	assert(player >= 0 && player < MAX_PLAYERS);

	// if we have a machine, use the live state and quick lookup
	input_type_entry *entry = m_type_to_entry[type][player];
	if (entry != nullptr)
		return entry->seq(seqtype);

	// if we find nothing, return an empty sequence
	return input_seq::empty_seq;
}


//-------------------------------------------------
//  set_type_seq - change the input sequence for
//  the given type/player
//-------------------------------------------------

void ioport_manager::set_type_seq(ioport_type type, int player, input_seq_type seqtype, const input_seq &newseq) noexcept
{
	input_type_entry *const entry = m_type_to_entry[type][player];
	if (entry)
	{
		if (newseq.is_default())
		{
			entry->set_seq(seqtype, entry->defseq(seqtype));
			entry->set_cfg(seqtype, "");
		}
		else
		{
			entry->set_seq(seqtype, newseq);
			if (!newseq.length())
				entry->set_cfg(seqtype, "NONE");
			else
				entry->set_cfg(seqtype, machine().input().seq_to_tokens(newseq));
		}
	}
}


//-------------------------------------------------
//  type_pressed - return true if the sequence for
//  the given input type/player is pressed
//-------------------------------------------------

bool ioport_manager::type_pressed(ioport_type type, int player)
{
	return machine().input().seq_pressed(type_seq(type, player));
}


//-------------------------------------------------
//  type_class_present - return true if the given
//  ioport_type_class exists in at least one port
//-------------------------------------------------

bool ioport_manager::type_class_present(ioport_type_class inputclass) const noexcept
{
	for (auto &port : m_portlist)
		for (ioport_field const &field : port.second->fields())
			if (field.type_class() == inputclass)
				return true;
	return false;
}


//-------------------------------------------------
//  count_players - counts the number of active
//  players
//-------------------------------------------------

int ioport_manager::count_players() const noexcept
{
	int max_player = 0;
	for (auto &port : m_portlist)
		for (ioport_field const &field : port.second->fields())
			if (field.type_class() == INPUT_CLASS_CONTROLLER && max_player <= field.player() + 1)
				max_player = field.player() + 1;

	return max_player;
}


//-------------------------------------------------
//  frame_update - core logic for per-frame input
//  port updating
//-------------------------------------------------

digital_joystick &ioport_manager::digjoystick(int player, int number)
{
	// find it in the list
	for (digital_joystick &joystick : m_joystick_list)
		if (joystick.player() == player && joystick.number() == number)
			return joystick;

	// create a new one
	return m_joystick_list.emplace_back(player, number);
}


//-------------------------------------------------
//  frame_update - callback for once/frame updating
//-------------------------------------------------

void ioport_manager::frame_update_callback()
{
	// if we're paused, don't do anything
	if (!machine().paused())
		frame_update();
}


//-------------------------------------------------
//  frame_update_internal - core logic for
//  per-frame input port updating
//-------------------------------------------------

void ioport_manager::frame_update()
{
	g_profiler.start(PROFILER_INPUT);

	// record/playback information about the current frame
	attotime curtime = machine().time();
	playback_frame(curtime);
	record_frame(curtime);

	// track the duration of the previous frame
	m_last_delta_nsec = (curtime - m_last_frame_time).as_attoseconds() / ATTOSECONDS_PER_NANOSECOND;
	m_last_frame_time = curtime;

	// update the digital joysticks
	for (digital_joystick &joystick : m_joystick_list)
		joystick.frame_update();

	// compute default values for all the ports
	// two passes to catch conditionals properly
	for (auto &port : m_portlist)
		port.second->update_defvalue(true);
	for (auto &port : m_portlist)
		port.second->update_defvalue(false);

	// loop over all input ports
	for (auto &port : m_portlist)
	{
		port.second->frame_update();

		// handle playback/record
		playback_port(*port.second.get());
		record_port(*port.second.get());

		// call device line write handlers
		ioport_value newvalue = port.second->read();
		for (dynamic_field &dynfield : port.second->live().writelist)
			if (dynfield.field().type() != IPT_OUTPUT)
				dynfield.write(newvalue);
	}

	g_profiler.stop();
}


//-------------------------------------------------
//  frame_interpolate - interpolate between two
//  values based on the time between frames
//-------------------------------------------------

s32 ioport_manager::frame_interpolate(s32 oldval, s32 newval)
{
	// if no last delta, just use new value
	if (m_last_delta_nsec == 0)
		return newval;

	// otherwise, interpolate
	attoseconds_t nsec_since_last = (machine().time() - m_last_frame_time).as_attoseconds() / ATTOSECONDS_PER_NANOSECOND;
	return oldval + (s64(newval - oldval) * nsec_since_last / m_last_delta_nsec);
}


//-------------------------------------------------
//  load_config - callback to extract configuration
//  data from the XML nodes
//-------------------------------------------------

void ioport_manager::load_config(config_type cfg_type, config_level cfg_level, util::xml::data_node const *parentnode)
{
	// in the completion phase, we finish the initialization with the final ports
	if (cfg_type == config_type::FINAL)
	{
		m_safe_to_read = true;
		frame_update();
	}

	// early exit if no data to parse
	if (!parentnode)
		return;

	// load device map table for controller configs only
	if (cfg_type == config_type::CONTROLLER)
	{
		// iterate over all the remap nodes
		load_remap_table(*parentnode);

		input_manager::devicemap_table devicemap;
		for (util::xml::data_node const *mapdevice_node = parentnode->get_child("mapdevice"); mapdevice_node != nullptr; mapdevice_node = mapdevice_node->get_next_sibling("mapdevice"))
		{
			char const *const devicename = mapdevice_node->get_attribute_string("device", nullptr);
			char const *const controllername = mapdevice_node->get_attribute_string("controller", nullptr);
			if (devicename && controllername)
				devicemap.emplace(devicename, controllername);
		}

		// map device to controller if we have a device map
		if (!devicemap.empty())
			machine().input().map_device_to_controller(devicemap);
	}

	// iterate over all the port nodes
	for (util::xml::data_node const *portnode = parentnode->get_child("port"); portnode; portnode = portnode->get_next_sibling("port"))
	{
		// get the basic port info from the attributes
		int player;
		int type = token_to_input_type(portnode->get_attribute_string("type", ""), player);

		// initialize sequences to invalid defaults
		std::pair<input_seq, char const *> newseq[SEQ_TYPE_TOTAL];
		for (auto &seq : newseq)
		{
			seq.first.set(INPUT_CODE_INVALID);
			seq.second = "";
		}

		// loop over new sequences
		for (util::xml::data_node const *seqnode = portnode->get_child("newseq"); seqnode; seqnode = seqnode->get_next_sibling("newseq"))
		{
			// with a valid type, parse out the new sequence
			input_seq_type seqtype = token_to_seq_type(seqnode->get_attribute_string("type", ""));
			if ((seqtype != -1) && seqnode->get_value())
			{
				if (!strcmp(seqnode->get_value(), "NONE"))
					newseq[seqtype].first.reset();
				else
					machine().input().seq_from_tokens(newseq[seqtype].first, seqnode->get_value());
				if (config_type::CONTROLLER != cfg_type)
					newseq[seqtype].second = seqnode->get_value();
			}
		}

		// load into the appropriate place for the config type/level
		if (config_type::SYSTEM == cfg_type)
			load_system_config(*portnode, type, player, newseq);
		else if ((config_type::CONTROLLER == cfg_type) && (config_level::DEFAULT != cfg_level))
			load_controller_config(*portnode, type, player, newseq);
		else
			load_default_config(type, player, newseq);
	}

	// after applying the controller config, push that back into the backup, since that is
	// what we will diff against
	if (cfg_type == config_type::CONTROLLER)
		for (input_type_entry &entry : m_typelist)
			for (input_seq_type seqtype = SEQ_TYPE_STANDARD; seqtype < SEQ_TYPE_TOTAL; ++seqtype)
				entry.defseq(seqtype) = entry.seq(seqtype);

	// load keyboard enable/disable state
	if (cfg_type == config_type::SYSTEM)
	{
		std::vector<bool> kbd_enable_set;
		bool keyboard_enabled = false, missing_enabled = false;
		natural_keyboard &natkbd = machine().natkeyboard();
		for (util::xml::data_node const *kbdnode = parentnode->get_child("keyboard"); kbdnode; kbdnode = kbdnode->get_next_sibling("keyboard"))
		{
			char const *const tag = kbdnode->get_attribute_string("tag", nullptr);
			int const enabled = kbdnode->get_attribute_int("enabled", -1);
			if (tag && (0 <= enabled))
			{
				size_t i;
				for (i = 0; natkbd.keyboard_count() > i; ++i)
				{
					if (!strcmp(natkbd.keyboard_device(i).tag(), tag))
					{
						if (kbd_enable_set.empty())
							kbd_enable_set.resize(natkbd.keyboard_count(), false);
						kbd_enable_set[i] = true;
						if (enabled)
						{
							if (!natkbd.keyboard_is_keypad(i))
								keyboard_enabled = true;
							natkbd.enable_keyboard(i);
						}
						else
						{
							natkbd.disable_keyboard(i);
						}
						break;
					}
				}
				missing_enabled = missing_enabled || (enabled && (natkbd.keyboard_count() <= i));
			}
		}

		// if keyboard enable configuration was loaded, patch it up for principle of least surprise
		if (!kbd_enable_set.empty())
		{
			for (size_t i = 0; natkbd.keyboard_count() > i; ++i)
			{
				if (!natkbd.keyboard_is_keypad(i))
				{
					if (!keyboard_enabled && missing_enabled)
					{
						natkbd.enable_keyboard(i);
						keyboard_enabled = true;
					}
					else if (!kbd_enable_set[i])
					{
						if (keyboard_enabled)
							natkbd.disable_keyboard(i);
						else
							natkbd.enable_keyboard(i);
						keyboard_enabled = true;
					}
				}
			}
		}
	}
}


//-------------------------------------------------
//  load_remap_table - extract and apply the
//  global remapping table
//-------------------------------------------------

void ioport_manager::load_remap_table(util::xml::data_node const &parentnode)
{
	// count items first so we can allocate
	int count = 0;
	for (util::xml::data_node const *remapnode = parentnode.get_child("remap"); remapnode != nullptr; remapnode = remapnode->get_next_sibling("remap"))
		count++;

	// if we have some, deal with them
	if (count > 0)
	{
		// allocate tables
		std::vector<input_code> oldtable(count);
		std::vector<input_code> newtable(count);

		// build up the remap table
		count = 0;
		for (util::xml::data_node const *remapnode = parentnode.get_child("remap"); remapnode != nullptr; remapnode = remapnode->get_next_sibling("remap"))
		{
			input_code origcode = machine().input().code_from_token(remapnode->get_attribute_string("origcode", ""));
			input_code newcode = machine().input().code_from_token(remapnode->get_attribute_string("newcode", ""));
			if (origcode != INPUT_CODE_INVALID && newcode != INPUT_CODE_INVALID)
			{
				oldtable[count] = origcode;
				newtable[count] = newcode;
				count++;
			}
		}

		// loop over the remapping table, then over default ports, replacing old with new
		for (int remapnum = 0; remapnum < count; remapnum++)
			for (input_type_entry &entry : m_typelist)
				entry.replace_code(oldtable[remapnum], newtable[remapnum]);
	}
}


//-------------------------------------------------
//  load_default_config - apply input settings
//  to defaults for all systems
//-------------------------------------------------

bool ioport_manager::load_default_config(
		int type,
		int player,
		const std::pair<input_seq, char const *> (&newseq)[SEQ_TYPE_TOTAL])
{
	// find a matching port in the list
	for (input_type_entry &entry : m_typelist)
	{
		if (entry.type() == type && entry.player() == player)
		{
			for (input_seq_type seqtype = SEQ_TYPE_STANDARD; seqtype < SEQ_TYPE_TOTAL; ++seqtype)
			{
				if (input_seq_good(machine(), newseq[seqtype].first))
					entry.set_seq(seqtype, newseq[seqtype].first);
				entry.set_cfg(seqtype, newseq[seqtype].second);
			}
			return true;
		}
	}
	return false;
}


//-------------------------------------------------
//  load_controller_config - apply controler
//  profile settings to defaults
//-------------------------------------------------

bool ioport_manager::load_controller_config(
		util::xml::data_node const &portnode,
		int type,
		int player,
		const std::pair<input_seq, char const *> (&newseq)[SEQ_TYPE_TOTAL])
{
	// without a tag, apply to the defaults for all systems
	char const *const tag = portnode.get_attribute_string("tag", nullptr);
	if (!tag)
		return load_default_config(type, player, newseq);

	// ensure the port actually exists
	auto const port(m_portlist.find(tag));
	if (m_portlist.end() == port)
		return false;
	ioport_value const mask = portnode.get_attribute_int("mask", 0);
	if (!mask)
		return false;

	// find the matching field
	ioport_value const defvalue = portnode.get_attribute_int("defvalue", 0);
	for (ioport_field &field : port->second->fields())
	{
		// find the matching mask and default value
		if (field.type() == type && field.player() == player &&
			field.mask() == mask && (field.defvalue() & mask) == (defvalue & mask))
		{
			// if a sequence was specified, override the developer-specified default for the field
			for (input_seq_type seqtype = SEQ_TYPE_STANDARD; seqtype < SEQ_TYPE_TOTAL; ++seqtype)
			{
				if (input_seq_good(machine(), newseq[seqtype].first))
				{
					field.live().seq[seqtype] = newseq[seqtype].first;
					field.set_defseq(seqtype, newseq[seqtype].first);
				}
			}

			// fetch configurable attributes
			if (!field.live().analog)
			{
				// for non-analog fields

				// can't practically set value here

				// fetch yes/no for toggle setting
				char const *const togstring = portnode.get_attribute_string("toggle", nullptr);
				if (togstring && !strcmp(togstring, "yes"))
				{
					field.live().toggle = true;
					field.m_flags |= ioport_field::FIELD_FLAG_TOGGLE;
				}
				else if (togstring && !strcmp(togstring, "no"))
				{
					field.live().toggle = false;
					field.m_flags &= ~ioport_field::FIELD_FLAG_TOGGLE;
				}
			}
			else
			{
				// for analog fields

#if 0 // changing this stuff causes issues because of the way it's tied up with the analog_field object
				// get base attributes
				field.live().analog->m_delta = field.m_delta = portnode.get_attribute_int("keydelta", field.delta());
				field.live().analog->m_centerdelta = field.m_centerdelta = portnode.get_attribute_int("centerdelta", field.centerdelta());
				field.live().analog->m_sensitivity = field.m_sensitivity = portnode.get_attribute_int("sensitivity", field.sensitivity());

				// fetch yes/no for reverse setting
				char const *const revstring = portnode.get_attribute_string("reverse", nullptr);
				if (revstring && !strcmp(revstring, "yes"))
				{
					field.live().analog->m_reverse = true;
					field.m_flags |= ioport_field::ANALOG_FLAG_REVERSE;
				}
				else if (revstring && !strcmp(revstring, "no"))
				{
					field.live().analog->m_reverse = false;
					field.m_flags &= ~ioport_field::ANALOG_FLAG_REVERSE;
				}
#endif
			}

			// successfully applied
			return true;
		}
	}

	// no matching field
	return false;
}


//-------------------------------------------------
//  load_system_config - apply saved input
//  configuration for the current system
//-------------------------------------------------

void ioport_manager::load_system_config(
		util::xml::data_node const &portnode,
		int type,
		int player,
		const std::pair<input_seq, char const *> (&newseq)[SEQ_TYPE_TOTAL])
{
	// system-specific configuration should always apply by port/field
	char const *const tag = portnode.get_attribute_string("tag", nullptr);
	ioport_value const mask = portnode.get_attribute_int("mask", 0);
	ioport_value const defvalue = portnode.get_attribute_int("defvalue", 0);
	if (!tag || !mask)
		return;

	// find the port we want
	auto const port(m_portlist.find(tag));
	if (m_portlist.end() != port)
	{
		for (ioport_field &field : port->second->fields())
		{
			// find the matching mask and default value
			if (field.type() == type && field.player() == player &&
				field.mask() == mask && (field.defvalue() & mask) == (defvalue & mask))
			{
				// if a sequence was specified, copy it in
				for (input_seq_type seqtype = SEQ_TYPE_STANDARD; seqtype < SEQ_TYPE_TOTAL; ++seqtype)
				{
					if (input_seq_good(machine(), newseq[seqtype].first))
						field.live().seq[seqtype] = newseq[seqtype].first;
					field.live().cfg[seqtype] = newseq[seqtype].second;
				}

				// fetch configurable attributes
				if (!field.live().analog)
				{
					// for non-analog fields

					// fetch the value
					field.live().value = portnode.get_attribute_int("value", field.defvalue());

					// fetch yes/no for toggle setting
					char const *const togstring = portnode.get_attribute_string("toggle", nullptr);
					if (togstring && !strcmp(togstring, "yes"))
						field.live().toggle = true;
					else if (togstring && !strcmp(togstring, "no"))
						field.live().toggle = false;
				}
				else
				{
					// for analog fields

					// get base attributes
					field.live().analog->m_delta = portnode.get_attribute_int("keydelta", field.delta());
					field.live().analog->m_centerdelta = portnode.get_attribute_int("centerdelta", field.centerdelta());
					field.live().analog->m_sensitivity = portnode.get_attribute_int("sensitivity", field.sensitivity());

					// fetch yes/no for reverse setting
					char const *const revstring = portnode.get_attribute_string("reverse", nullptr);
					if (revstring && !strcmp(revstring, "yes"))
						field.live().analog->m_reverse = true;
					else if (revstring && !strcmp(revstring, "no"))
						field.live().analog->m_reverse = false;
				}
				break;
			}
		}
	}
	else
	{
		// see if this belongs to a slot card that isn't inserted
		std::string_view parent_tag(tag);
		auto pos(parent_tag.rfind(':'));
		if (pos && (std::string_view::npos != pos))
		{
			parent_tag = parent_tag.substr(0, pos);
			if (!machine().root_device().subdevice(parent_tag))
			{
				for (pos = parent_tag.rfind(':'); pos && (std::string_view::npos != pos); pos = parent_tag.rfind(':'))
				{
					std::string_view const child_tag(parent_tag.substr(pos + 1));
					parent_tag = parent_tag.substr(0, pos);
					device_t const *const parent_device(machine().root_device().subdevice(parent_tag));
					if (parent_device)
					{
						device_slot_interface const *slot;
						if (parent_device->interface(slot) && (slot->option_list().find(std::string(child_tag)) != slot->option_list().end()))
						{
							if (!m_deselected_card_config)
								m_deselected_card_config = util::xml::file::create();
							portnode.copy_into(*m_deselected_card_config);
						}
						break;
					}
				}
			}
		}
	}
}



//**************************************************************************
//  SETTINGS SAVE
//**************************************************************************

//-------------------------------------------------
//  save_config - config callback for saving input
//  port configuration
//-------------------------------------------------

void ioport_manager::save_config(config_type cfg_type, util::xml::data_node *parentnode)
{
	// if no parentnode, ignore
	if (!parentnode)
		return;

	// default ports save differently
	if (cfg_type == config_type::DEFAULT)
		save_default_inputs(*parentnode);
	else if (cfg_type == config_type::SYSTEM)
		save_game_inputs(*parentnode);
}


//-------------------------------------------------
//  save_this_input_field_type - determine if the
//  given port type is worth saving
//-------------------------------------------------

bool ioport_manager::save_this_input_field_type(ioport_type type)
{
	switch (type)
	{
		case IPT_UNUSED:
		case IPT_END:
		case IPT_PORT:
		case IPT_UNKNOWN:
			return false;

		default:
			break;
	}
	return true;
}


//-------------------------------------------------
//  save_default_inputs - add nodes for any default
//  mappings that have changed
//-------------------------------------------------

void ioport_manager::save_default_inputs(util::xml::data_node &parentnode)
{
	// iterate over ports
	for (input_type_entry &entry : m_typelist)
	{
		// only save if this port is a type we save
		if (save_this_input_field_type(entry.type()))
		{
			// see if any of the sequences have changed
			input_seq_type seqtype;
			for (seqtype = SEQ_TYPE_STANDARD; seqtype < SEQ_TYPE_TOTAL; ++seqtype)
				if (!entry.cfg(seqtype).empty())
					break;

			// if so, we need to add a node
			if (seqtype < SEQ_TYPE_TOTAL)
			{
				// add a new port node
				util::xml::data_node *const portnode = parentnode.add_child("port", nullptr);
				if (portnode)
				{
					// add the port information and attributes
					portnode->set_attribute("type", input_type_to_token(entry.type(), entry.player()).c_str());

					// add only the sequences that have changed from the defaults
					for (input_seq_type type = SEQ_TYPE_STANDARD; type < SEQ_TYPE_TOTAL; ++type)
					{
						if (!entry.cfg(type).empty())
						{
							util::xml::data_node *const seqnode = portnode->add_child("newseq", entry.cfg(type).c_str());
							if (seqnode)
								seqnode->set_attribute("type", seqtypestrings[type]);
						}
					}
				}
			}
		}
	}
}


//-------------------------------------------------
//  save_game_inputs - add nodes for any game
//  mappings that have changed
//-------------------------------------------------

void ioport_manager::save_game_inputs(util::xml::data_node &parentnode)
{
	// save keyboard enable/disable state
	natural_keyboard &natkbd = machine().natkeyboard();
	for (size_t i = 0; natkbd.keyboard_count() > i; ++i)
	{
		util::xml::data_node *const kbdnode = parentnode.add_child("keyboard", nullptr);
		kbdnode->set_attribute("tag", natkbd.keyboard_device(i).tag());
		kbdnode->set_attribute_int("enabled", natkbd.keyboard_enabled(i));
	}

	// iterate over ports
	for (auto &port : m_portlist)
		for (ioport_field const &field : port.second->fields())
			if (save_this_input_field_type(field.type()))
			{
				// determine if we changed
				bool changed = false;
				for (input_seq_type seqtype = SEQ_TYPE_STANDARD; (seqtype < SEQ_TYPE_TOTAL) && !changed; ++seqtype)
					changed = !field.live().cfg[seqtype].empty();

				if (!field.is_analog())
				{
					// non-analog changes
					changed = changed || ((field.live().value & field.mask()) != (field.defvalue() & field.mask()));
					changed = changed || (field.live().toggle != field.toggle());
				}
				else
				{
					// analog changes
					changed = changed || (field.live().analog->m_delta != field.delta());
					changed = changed || (field.live().analog->m_centerdelta != field.centerdelta());
					changed = changed || (field.live().analog->m_sensitivity != field.sensitivity());
					changed = changed || (field.live().analog->m_reverse != field.analog_reverse());
				}

				// if we did change, add a new node
				if (changed)
				{
					// add a new port node
					util::xml::data_node *const portnode = parentnode.add_child("port", nullptr);
					if (portnode)
					{
						// add the identifying information and attributes
						portnode->set_attribute("tag", port.second->tag());
						portnode->set_attribute("type", input_type_to_token(field.type(), field.player()).c_str());
						portnode->set_attribute_int("mask", field.mask());
						portnode->set_attribute_int("defvalue", field.defvalue() & field.mask());

						// add sequences if changed
						for (input_seq_type seqtype = SEQ_TYPE_STANDARD; seqtype < SEQ_TYPE_TOTAL; ++seqtype)
						{
							if (!field.live().cfg[seqtype].empty())
							{
								util::xml::data_node *const seqnode = portnode->add_child("newseq", field.live().cfg[seqtype].c_str());
								if (seqnode)
									seqnode->set_attribute("type", seqtypestrings[seqtype]);
							}
						}

						if (!field.is_analog())
						{
							// write out non-analog changes
							if ((field.live().value & field.mask()) != (field.defvalue() & field.mask()))
								portnode->set_attribute_int("value", field.live().value & field.mask());
							if (field.live().toggle != field.toggle())
								portnode->set_attribute("toggle", field.live().toggle ? "yes" : "no");
						}
						else
						{
							// write out analog changes
							if (field.live().analog->m_delta != field.delta())
								portnode->set_attribute_int("keydelta", field.live().analog->m_delta);
							if (field.live().analog->m_centerdelta != field.centerdelta())
								portnode->set_attribute_int("centerdelta", field.live().analog->m_centerdelta);
							if (field.live().analog->m_sensitivity != field.sensitivity())
								portnode->set_attribute_int("sensitivity", field.live().analog->m_sensitivity);
							if (field.live().analog->m_reverse != field.analog_reverse())
								portnode->set_attribute("reverse", field.live().analog->m_reverse ? "yes" : "no");
						}
					}
				}
			}

	// preserve configuration for deselected slot cards
	if (m_deselected_card_config)
	{
		for (util::xml::data_node const *node = m_deselected_card_config->get_first_child(); node; node = node->get_next_sibling())
			node->copy_into(parentnode);
	}
}



//**************************************************************************
//  INPUT PLAYBACK
//**************************************************************************

//-------------------------------------------------
//  playback_read - read a value from the playback
//  file
//-------------------------------------------------

template<typename Type>
Type ioport_manager::playback_read(Type &result)
{
	// protect against nullptr handles if previous reads fail
	if (!m_playback_stream)
		return result = Type(0);

	// read the value; if we fail, end playback
	size_t read;
	m_playback_stream->read(&result, sizeof(result), read);
	if (sizeof(result) != read)
	{
		playback_end("End of file");
		return result = Type(0);
	}

	// normalize byte order
	if (sizeof(result) == 8)
		result = little_endianize_int64(result);
	else if (sizeof(result) == 4)
		result = little_endianize_int32(result);
	else if (sizeof(result) == 2)
		result = little_endianize_int16(result);

	return result;
}

template<>
bool ioport_manager::playback_read<bool>(bool &result)
{
	u8 temp;
	playback_read(temp);
	return result = bool(temp);
}


//-------------------------------------------------
//  playback_init - initialize INP playback
//-------------------------------------------------

time_t ioport_manager::playback_init()
{
	// if no file, nothing to do
	const char *filename = machine().options().playback();
	if (filename[0] == 0)
		return 0;

	// open the playback file
	m_playback_file = std::make_unique<emu_file>(machine().options().input_directory(), OPEN_FLAG_READ);
	std::error_condition const filerr = m_playback_file->open(filename);

	// return an explicit error if file isn't found in given path
	if (filerr == std::errc::no_such_file_or_directory)
		fatalerror("Input file %s not found\n",filename);

	// TODO: bail out any other error laconically for now
	if (filerr)
		fatalerror("Failed to open file %s for playback (%s:%d %s)\n", filename, filerr.category().name(), filerr.value(), filerr.message());

	// read the header and verify that it is a modern version; if not, print an error
	inp_header header;
	if (!header.read(*m_playback_file))
		fatalerror("Input file is corrupt or invalid (missing header)\n");
	if (!header.check_magic())
		fatalerror("Input file invalid or in an older, unsupported format\n");
	if (header.get_majversion() != inp_header::MAJVERSION)
		fatalerror("Input file format version mismatch\n");

	// output info to console
	osd_printf_info("Input file: %s\n", filename);
	osd_printf_info("INP version %u.%u\n", header.get_majversion(), header.get_minversion());
	time_t basetime = header.get_basetime();
	osd_printf_info("Created %s\n", ctime(&basetime));
	osd_printf_info("Recorded using %s\n", header.get_appdesc());

	// verify the header against the current game
	std::string const sysname = header.get_sysname();
	if (sysname != machine().system().name)
		osd_printf_info("Input file is for machine '%s', not for current machine '%s'\n", sysname, machine().system().name);

	// enable compression
	m_playback_stream = util::zlib_read(*m_playback_file, 16386);
	return basetime;
}


//-------------------------------------------------
//  playback_end - end INP playback
//-------------------------------------------------

void ioport_manager::playback_end(const char *message)
{
	// only applies if we have a live file
	if (m_playback_stream)
	{
		// close the file
		m_playback_stream.reset();
		m_playback_file.reset();

		// pop a message
		if (message != nullptr)
			machine().popmessage("Playback Ended\nReason: %s", message);

		// display speed stats
		if (m_playback_accumulated_speed > 0)
			m_playback_accumulated_speed /= m_playback_accumulated_frames;
		osd_printf_info("Total playback frames: %d\n", u32(m_playback_accumulated_frames));
		osd_printf_info("Average recorded speed: %d%%\n", u32((m_playback_accumulated_speed * 200 + 1) >> 21));

		// close the program at the end of inp file playback
		if (machine().options().exit_after_playback())
		{
			osd_printf_info("Exiting MAME now...\n");
			machine().schedule_exit();
		}
	}
}


//-------------------------------------------------
//  playback_frame - start of frame callback for
//  playback
//-------------------------------------------------

void ioport_manager::playback_frame(const attotime &curtime)
{
	// if playing back, fetch the information and verify
	if (m_playback_stream)
	{
		// first the absolute time
		seconds_t seconds_temp;
		attoseconds_t attoseconds_temp;
		playback_read(seconds_temp);
		playback_read(attoseconds_temp);
		attotime readtime(seconds_temp, attoseconds_temp);
		if (readtime != curtime)
			playback_end("Out of sync");

		// then the speed
		u32 curspeed;
		m_playback_accumulated_speed += playback_read(curspeed);
		m_playback_accumulated_frames++;
	}
}


//-------------------------------------------------
//  playback_port - per-port callback for playback
//-------------------------------------------------

void ioport_manager::playback_port(ioport_port &port)
{
	// if playing back, fetch information about this port
	if (m_playback_stream)
	{
		// read the default value and the digital state
		playback_read(port.live().defvalue);
		playback_read(port.live().digital);

		// loop over analog ports and save their data
		for (analog_field &analog : port.live().analoglist)
		{
			// read current and previous values
			playback_read(analog.m_accum);
			playback_read(analog.m_previous);

			// read configuration information
			playback_read(analog.m_sensitivity);
			playback_read(analog.m_reverse);
		}
	}
}


//-------------------------------------------------
//  record_write - write a value to the record file
//-------------------------------------------------

template<typename Type>
void ioport_manager::record_write(Type value)
{
	// protect against nullptr handles if previous reads fail
	if (!m_record_stream)
		return;

	// normalize byte order
	if (sizeof(value) == 8)
		value = little_endianize_int64(value);
	else if (sizeof(value) == 4)
		value = little_endianize_int32(value);
	else if (sizeof(value) == 2)
		value = little_endianize_int16(value);

	// write the value; if we fail, end recording
	size_t written;
	if (m_record_stream->write(&value, sizeof(value), written) || (sizeof(value) != written))
		record_end("Out of space");
}

template<>
void ioport_manager::record_write<bool>(bool value)
{
	u8 byte = u8(value);
	record_write(byte);
}


//-------------------------------------------------
//  record_init - initialize INP recording
//-------------------------------------------------

void ioport_manager::record_init()
{
	// if no file, nothing to do
	const char *filename = machine().options().record();
	if (filename[0] == 0)
		return;

	// open the record file
	m_record_file = std::make_unique<emu_file>(machine().options().input_directory(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
	std::error_condition const filerr = m_record_file->open(filename);
	if (filerr)
		throw emu_fatalerror("ioport_manager::record_init: Failed to open file for recording (%s:%d %s)", filerr.category().name(), filerr.value(), filerr.message());

	// get the base time
	system_time systime;
	machine().base_datetime(systime);

	// fill in the header
	inp_header header;
	header.set_magic();
	header.set_basetime(systime.time);
	header.set_version();
	header.set_sysname(machine().system().name);
	header.set_appdesc(util::string_format("%s %s", emulator_info::get_appname(), emulator_info::get_build_version()));

	// write it
	header.write(*m_record_file);

	// enable compression
	m_record_stream = util::zlib_write(*m_record_file, 6, 16384);
}


//-------------------------------------------------
//  record_end - end INP recording
//-------------------------------------------------

void ioport_manager::record_end(const char *message)
{
	// only applies if we have a live file
	if (m_record_stream)
	{
		// close the file
		m_record_stream.reset(); // TODO: check for errors flushing the last compressed block before doing this
		m_record_file.reset();

		// pop a message
		if (message != nullptr)
			machine().popmessage("Recording Ended\nReason: %s", message);
	}
}


//-------------------------------------------------
//  record_frame - start of frame callback for
//  recording
//-------------------------------------------------

void ioport_manager::record_frame(const attotime &curtime)
{
	// if recording, record information about the current frame
	if (m_record_stream)
	{
		// first the absolute time
		record_write(curtime.seconds());
		record_write(curtime.attoseconds());

		// then the current speed
		record_write(u32(machine().video().speed_percent() * double(1 << 20)));
	}
}


//-------------------------------------------------
//  record_port - per-port callback for record
//-------------------------------------------------

void ioport_manager::record_port(ioport_port &port)
{
	// if recording, store information about this port
	if (m_record_stream)
	{
		// store the default value and digital state
		record_write(port.live().defvalue);
		record_write(port.live().digital);

		// loop over analog ports and save their data
		for (analog_field &analog : port.live().analoglist)
		{
			// store current and previous values
			record_write(analog.m_accum);
			record_write(analog.m_previous);

			// store configuration information
			record_write(analog.m_sensitivity);
			record_write(analog.m_reverse);
		}
	}
}



//**************************************************************************
//  I/O PORT CONFIGURER
//**************************************************************************

//-------------------------------------------------
//  ioport_configurer - constructor
//-------------------------------------------------

ioport_configurer::ioport_configurer(device_t &owner, ioport_list &portlist, std::string &errorbuf)
	: m_owner(owner),
		m_portlist(portlist),
		m_errorbuf(errorbuf),
		m_curport(nullptr),
		m_curfield(nullptr),
		m_cursetting(nullptr)
{
}


//-------------------------------------------------
//  string_from_token - convert an
//  ioport_token to a default string
//-------------------------------------------------

const char *ioport_configurer::string_from_token(const char *string)
{
	// 0 is an invalid index
	if (string == nullptr)
		return nullptr;

	// if the index is greater than the count, assume it to be a pointer
	if (uintptr_t(string) >= INPUT_STRING_COUNT)
		return string;

#if false // Set true, If you want to take care missing-token or wrong-sorting

	// otherwise, scan the list for a matching string and return it
	for (int index = 0; index < std::size(input_port_default_strings); index++)
		if (input_port_default_strings[index].id == uintptr_t(string))
			return input_port_default_strings[index].string;
	return "(Unknown Default)";

#else

	return input_port_default_strings[uintptr_t(string)-1].string;

#endif
}


//-------------------------------------------------
//  port_alloc - allocate a new port
//-------------------------------------------------

ioport_configurer& ioport_configurer::port_alloc(const char *tag)
{
	// create the full tag
	std::string fulltag = m_owner.subtag(tag);

	// add it to the list, and reset current field/setting
	if (m_portlist.count(fulltag) != 0) throw tag_add_exception(fulltag.c_str());
	m_portlist.emplace(std::make_pair(fulltag, std::make_unique<ioport_port>(m_owner, fulltag.c_str())));
	m_curport = m_portlist.find(fulltag)->second.get();
	m_curfield = nullptr;
	m_cursetting = nullptr;
	return *this;
}


//-------------------------------------------------
//  port_modify - find an existing port and
//  modify it
//-------------------------------------------------

ioport_configurer& ioport_configurer::port_modify(const char *tag)
{
	// create the full tag
	std::string fulltag = m_owner.subtag(tag);

	// find the existing port
	m_curport = m_portlist.find(fulltag)->second.get();
	if (m_curport == nullptr)
		throw emu_fatalerror("Requested to modify nonexistent port '%s'", fulltag);

	// bump the modification count, and reset current field/setting
	m_curport->m_modcount++;
	m_curfield = nullptr;
	m_cursetting = nullptr;
	return *this;
}


//-------------------------------------------------
//  field_alloc - allocate a new field
//-------------------------------------------------

ioport_configurer& ioport_configurer::field_alloc(ioport_type type, ioport_value defval, ioport_value mask, const char *name)
{
	// make sure we have a port
	if (m_curport == nullptr)
		throw emu_fatalerror("alloc_field called with no active port (mask=%X defval=%X)\n", mask, defval);
	// append the field
	if (type != IPT_UNKNOWN && type != IPT_UNUSED)
		m_curport->m_active |= mask;
	m_curfield = &m_curport->m_fieldlist.append(*new ioport_field(*m_curport, type, defval, mask, string_from_token(name)));

	// reset the current setting
	m_cursetting = nullptr;
	return *this;
}


//-------------------------------------------------
//  field_add_char - add a character to a field
//-------------------------------------------------

ioport_configurer& ioport_configurer::field_add_char(std::initializer_list<char32_t> charlist)
{
	for (int index = 0; index < std::size(m_curfield->m_chars); index++)
		if (m_curfield->m_chars[index][0] == 0)
		{
			const size_t char_count = std::size(m_curfield->m_chars[index]);
			assert(charlist.size() > 0 && charlist.size() <= char_count);

			for (size_t i = 0; i < char_count; i++)
				m_curfield->m_chars[index][i] = i < charlist.size() ? *(charlist.begin() + i) : 0;
			return *this;
		}

	std::ostringstream s;
	bool is_first = true;
	for (char32_t ch : charlist)
	{
		util::stream_format(s, "%s%d", is_first ? "" : ",", (int)ch);
		is_first = false;
	}
	throw emu_fatalerror("PORT_CHAR(%s) could not be added - maximum amount exceeded\n", s.str());
}


//-------------------------------------------------
//  field_add_code - add a character to a field
//-------------------------------------------------

ioport_configurer& ioport_configurer::field_add_code(input_seq_type which, input_code code)
{
	m_curfield->m_seq[which] |= code;
	return *this;
}


//-------------------------------------------------
//  setting_alloc - allocate a new setting
//-------------------------------------------------

ioport_configurer& ioport_configurer::setting_alloc(ioport_value value, const char *name)
{
	// make sure we have a field
	if (!m_curfield)
		throw emu_fatalerror("alloc_setting called with no active field (value=%X name=%s)\n", value, name);

	// append a new setting
	m_cursetting = &m_curfield->m_settinglist.emplace_back(*m_curfield, value & m_curfield->mask(), string_from_token(name));
	return *this;
}


//-------------------------------------------------
//  set_condition - set the condition for either
//  the current setting or field
//-------------------------------------------------

ioport_configurer& ioport_configurer::set_condition(ioport_condition::condition_t condition, const char *tag, ioport_value mask, ioport_value value)
{
	ioport_condition &target = m_cursetting ? m_cursetting->condition() : m_curfield->condition();
	target.set(condition, tag, mask, value);
	return *this;
}


//-------------------------------------------------
//  onoff_alloc - allocate an on/off DIP switch
//-------------------------------------------------

ioport_configurer& ioport_configurer::onoff_alloc(const char *name, ioport_value defval, ioport_value mask, const char *diplocation)
{
	// allocate a field normally
	field_alloc(IPT_DIPSWITCH, defval, mask, name);

	// expand the diplocation
	if (diplocation != nullptr)
		field_set_diplocation(diplocation);

	// allocate settings
	setting_alloc(defval & mask, DEF_STR(Off));
	setting_alloc(~defval & mask, DEF_STR(On));
	// clear cursettings set by setting_alloc
	m_cursetting = nullptr;
	return *this;
}



/***************************************************************************
    MISCELLANEOUS
***************************************************************************/

//-------------------------------------------------
//  dynamic_field - constructor
//-------------------------------------------------

dynamic_field::dynamic_field(ioport_field &field)
	: m_field(field),
		m_shift(0),
		m_oldval(field.defvalue())
{
	// fill in the data
	for (ioport_value mask = field.mask(); !(mask & 1); mask >>= 1)
		m_shift++;
	m_oldval >>= m_shift;
}


//-------------------------------------------------
//  read - read the updated value and merge it
//  into the target
//-------------------------------------------------

void dynamic_field::read(ioport_value &result)
{
	// skip if not enabled
	if (m_field.enabled())
	{
		// call the callback to read a new value
		ioport_value newval = m_field.m_read();
		m_oldval = newval;

		// merge in the bits (don't invert yet, as all digitals are inverted together)
		result = (result & ~m_field.mask()) | ((newval << m_shift) & m_field.mask());
	}
}


//-------------------------------------------------
//  write - track a change to a value and call
//  the write callback if there's something new
//-------------------------------------------------

void dynamic_field::write(ioport_value newval)
{
	// skip if not enabled
	if (m_field.enabled())
	{
		// if the bits have changed, call the handler
		newval = (newval & m_field.mask()) >> m_shift;
		if (m_oldval != newval)
		{
			m_field.m_write(m_field, m_field.m_write_param, m_oldval, newval);
			m_oldval = newval;
		}
	}
}


//-------------------------------------------------
//  analog_field - constructor
//-------------------------------------------------

analog_field::analog_field(ioport_field &field)
	: m_field(field),
		m_shift(0),
		m_adjdefvalue(field.defvalue() & field.mask()),
		m_adjmin(field.minval() & field.mask()),
		m_adjmax(field.maxval() & field.mask()),
		m_adjoverride(field.defvalue() & field.mask()),
		m_sensitivity(field.sensitivity()),
		m_reverse(field.analog_reverse()),
		m_delta(field.delta()),
		m_centerdelta(field.centerdelta()),
		m_accum(0),
		m_previous(0),
		m_previousanalog(0),
		m_minimum(INPUT_ABSOLUTE_MIN),
		m_maximum(INPUT_ABSOLUTE_MAX),
		m_center(0),
		m_reverse_val(0),
		m_scalepos(0),
		m_scaleneg(0),
		m_keyscalepos(0),
		m_keyscaleneg(0),
		m_positionalscale(0),
		m_absolute(false),
		m_wraps(false),
		m_autocenter(false),
		m_single_scale(false),
		m_interpolate(false),
		m_lastdigital(false),
		m_use_adjoverride(false)
{
	// compute the shift amount and number of bits
	for (ioport_value mask = field.mask(); !(mask & 1); mask >>= 1)
		m_shift++;

	// initialize core data
	m_adjdefvalue >>= m_shift;
	m_adjmin >>= m_shift;
	m_adjmax >>= m_shift;

	// set basic parameters based on the configured type
	switch (field.type())
	{
		// paddles and analog joysticks are absolute and autocenter
		case IPT_AD_STICK_X:
		case IPT_AD_STICK_Y:
		case IPT_AD_STICK_Z:
		case IPT_PADDLE:
		case IPT_PADDLE_V:
			m_absolute = true;
			m_autocenter = true;
			m_interpolate = !field.analog_reset();
			break;

		// pedals start at and autocenter to the min range
		case IPT_PEDAL:
		case IPT_PEDAL2:
		case IPT_PEDAL3:
			m_center = INPUT_ABSOLUTE_MIN;
			m_accum = apply_inverse_sensitivity(m_center);
			m_absolute = true;
			m_autocenter = true;
			m_interpolate = !field.analog_reset();
			break;

		// lightguns are absolute as well, but don't autocenter and don't interpolate their values
		case IPT_LIGHTGUN_X:
		case IPT_LIGHTGUN_Y:
			m_absolute = true;
			m_autocenter = false;
			m_interpolate = false;
			break;

		// positional devices are absolute, but can also wrap like relative devices
		// set each position to be 512 units
		case IPT_POSITIONAL:
		case IPT_POSITIONAL_V:
			m_positionalscale = compute_scale(field.maxval(), INPUT_ABSOLUTE_MAX - INPUT_ABSOLUTE_MIN);
			m_adjmin = 0;
			m_adjmax = field.maxval() - 1;
			m_wraps = field.analog_wraps();
			m_autocenter = !m_wraps;
			break;

		// dials, mice and trackballs are relative devices
		// these have fixed "min" and "max" values based on how many bits are in the port
		// in addition, we set the wrap around min/max values to 512 * the min/max values
		// this takes into account the mapping that one mouse unit ~= 512 analog units
		case IPT_DIAL:
		case IPT_DIAL_V:
		case IPT_TRACKBALL_X:
		case IPT_TRACKBALL_Y:
		case IPT_MOUSE_X:
		case IPT_MOUSE_Y:
			m_absolute = false;
			m_wraps = true;
			m_interpolate = !field.analog_reset();
			break;

		default:
			fatalerror("Unknown analog port type -- don't know if it is absolute or not\n");
	}

	// further processing for absolute controls
	if (m_absolute)
	{
		// if the default value is pegged at the min or max, use a single scale value for the whole axis
		m_single_scale = (m_adjdefvalue == m_adjmin) || (m_adjdefvalue == m_adjmax);

		// if not "single scale", compute separate scales for each side of the default
		if (!m_single_scale)
		{
			// unsigned
			m_scalepos = compute_scale(m_adjmax - m_adjdefvalue, INPUT_ABSOLUTE_MAX - 0);
			m_scaleneg = compute_scale(m_adjdefvalue - m_adjmin, 0 - INPUT_ABSOLUTE_MIN);

			if (m_adjmin > m_adjmax)
				m_scaleneg = -m_scaleneg;

			// reverse point is at center
			m_reverse_val = 0;
		}
		else
		{
			// single axis that increases from default
			m_scalepos = compute_scale(m_adjmax - m_adjmin, INPUT_ABSOLUTE_MAX - INPUT_ABSOLUTE_MIN);

			// make the scaling the same for easier coding when we need to scale
			m_scaleneg = m_scalepos;

			// reverse point is at max
			m_reverse_val = m_maximum;
		}
	}

	// relative and positional controls all map directly with a 512x scale factor
	else
	{
		// The relative code is set up to allow specifing PORT_MINMAX and default values.
		// The validity checks are purposely set up to not allow you to use anything other
		// a default of 0 and PORT_MINMAX(0,mask).  This is in case the need arises to use
		// this feature in the future.  Keeping the code in does not hurt anything.
		if (m_adjmin > m_adjmax)
			// adjust for signed
			m_adjmin = -m_adjmin;

		if (m_wraps)
			m_adjmax++;

		m_minimum = (m_adjmin - m_adjdefvalue) * INPUT_RELATIVE_PER_PIXEL;
		m_maximum = (m_adjmax - m_adjdefvalue) * INPUT_RELATIVE_PER_PIXEL;

		// make the scaling the same for easier coding when we need to scale
		m_scaleneg = m_scalepos = compute_scale(1, INPUT_RELATIVE_PER_PIXEL);

		if (m_field.analog_reset())
			// delta values reverse from center
			m_reverse_val = 0;
		else
		{
			// positional controls reverse from their max range
			m_reverse_val = m_maximum + m_minimum;

			// relative controls reverse from 1 past their max range
			if (m_wraps)
			{
				// FIXME: positional needs -1, using INPUT_RELATIVE_PER_PIXEL skips a position (and reads outside the table array)
				if(field.type() == IPT_POSITIONAL || field.type() == IPT_POSITIONAL_V)
					m_reverse_val --;
				else
					m_reverse_val -= INPUT_RELATIVE_PER_PIXEL;
			}
		}
	}

	// compute scale for keypresses
	m_keyscalepos = recip_scale(m_scalepos);
	m_keyscaleneg = recip_scale(m_scaleneg);
}


//-------------------------------------------------
//  apply_min_max - clamp the given input value to
//  the appropriate min/max for the analog control
//-------------------------------------------------

inline s32 analog_field::apply_min_max(s32 value) const
{
	// take the analog minimum and maximum values and apply the inverse of the
	// sensitivity so that we can clamp against them before applying sensitivity
	s32 adjmin = apply_inverse_sensitivity(m_minimum);
	s32 adjmax = apply_inverse_sensitivity(m_maximum);

	// clamp to the bounds absolutely
	if (value > adjmax)
		value = adjmax;
	else if (value < adjmin)
		value = adjmin;

	return value;
}


//-------------------------------------------------
//  apply_sensitivity - apply a sensitivity
//  adjustment for a current value
//-------------------------------------------------

inline s32 analog_field::apply_sensitivity(s32 value) const
{
	return lround((s64(value) * m_sensitivity) / 100.0);
}


//-------------------------------------------------
//  apply_inverse_sensitivity - reverse-apply the
//  sensitivity adjustment for a current value
//-------------------------------------------------

inline s32 analog_field::apply_inverse_sensitivity(s32 value) const
{
	return s32((s64(value) * 100) / m_sensitivity);
}


//-------------------------------------------------
//  apply_settings - return the value of an
//  analog input
//-------------------------------------------------

s32 analog_field::apply_settings(s32 value) const
{
	// apply the min/max and then the sensitivity
	if (!m_wraps)
		value = apply_min_max(value);
	value = apply_sensitivity(value);

	// apply reversal if needed
	if (m_reverse)
		value = m_reverse_val - value;
	else if (m_single_scale)
		// it's a pedal or the default value is equal to min/max
		// so we need to adjust the center to the minimum
		value -= INPUT_ABSOLUTE_MIN;

	// map differently for positive and negative values
	if (value >= 0)
		value = apply_scale(value, m_scalepos);
	else
		value = apply_scale(value, m_scaleneg);
	value += m_adjdefvalue;

	// for relative devices, wrap around when we go past the edge
	// (this is done last to prevent rounding errors)
	if (m_wraps)
	{
		s32 range = m_adjmax - m_adjmin;
		// rolls to other end when 1 position past end.
		value = (value - m_adjmin) % range;
		if (value < 0)
			value += range;
		value += m_adjmin;
	}

	return value;
}


//-------------------------------------------------
//  set_value - override the value that will be
//  read from the field
//-------------------------------------------------

void analog_field::set_value(s32 value)
{
	m_use_adjoverride = true;
	m_adjoverride = std::clamp(value, m_adjmin, m_adjmax);
}


//-------------------------------------------------
//  clear_value - clear programmatic override
//-------------------------------------------------

void analog_field::clear_value()
{
	m_use_adjoverride = false;
	m_adjoverride = m_adjdefvalue;
}


//-------------------------------------------------
//  frame_update - update the internals of a
//  single analog field periodically
//-------------------------------------------------

void analog_field::frame_update(running_machine &machine)
{
	// clamp the previous value to the min/max range
	if (!m_wraps)
		m_accum = apply_min_max(m_accum);

	// remember the previous value in case we need to interpolate
	m_previous = m_accum;

	// get the new raw analog value and its type
	input_item_class itemclass;
	s32 rawvalue = machine.input().seq_axis_value(m_field.seq(SEQ_TYPE_STANDARD), itemclass);

	// if we got an absolute input, it overrides everything else
	if (itemclass == ITEM_CLASS_ABSOLUTE)
	{
		if (!m_absolute && !m_positionalscale)
		{
			// if port is relative, we use the value to simulate the speed of relative movement
			// sensitivity adjustment is allowed for this mode
			if (rawvalue)
			{
				if (m_field.analog_reset())
					m_accum = rawvalue / 8;
				else
					m_accum += rawvalue / 8;

				// do not bother with other control types if the analog data is changing
				m_lastdigital = false;
				return;
			}
		}
		else if (m_previousanalog != rawvalue)
		{
			// only update if analog value changed
			m_previousanalog = rawvalue;

			// apply the inverse of the sensitivity to the raw value so that
			// it will still cover the full min->max range requested after
			// we apply the sensitivity adjustment
			if (m_absolute || m_field.analog_reset())
			{
				// if port is absolute, then just return the absolute data supplied
				m_accum = apply_inverse_sensitivity(rawvalue);
			}
			else
			{
				assert(m_positionalscale); // only way to get here due to previous if

				// if port is positional, we will take the full analog control and divide it
				// into positions, that way as the control is moved full scale,
				// it moves through all the positions
				rawvalue = apply_scale(rawvalue - INPUT_ABSOLUTE_MIN, m_positionalscale) * INPUT_RELATIVE_PER_PIXEL + m_minimum;

				// clamp the high value so it does not roll over
				rawvalue = std::min(rawvalue, m_maximum);
				m_accum = apply_inverse_sensitivity(rawvalue);
			}

			// do not bother with other control types if the analog data is changing
			m_lastdigital = false;
			return;
		}
	}

	// if we got it from a relative device, use that as the starting delta
	// also note that the last input was not a digital one
	s32 delta = 0;
	if (itemclass == ITEM_CLASS_RELATIVE && rawvalue)
	{
		delta = rawvalue;
		m_lastdigital = false;
	}

	s64 keyscale = (m_accum >= 0) ? m_keyscalepos : m_keyscaleneg;

	// if the decrement code sequence is pressed, add the key delta to
	// the accumulated delta; also note that the last input was a digital one
	bool keypressed = false;
	if (machine.input().seq_pressed(m_field.seq(SEQ_TYPE_DECREMENT)))
	{
		keypressed = true;
		if (m_delta != 0)
			delta -= apply_scale(m_delta, keyscale);
		else if (!m_lastdigital)
			// decrement only once when first pressed
			delta -= apply_scale(1, keyscale);
		m_lastdigital = true;
	}

	// same for the increment code sequence
	if (machine.input().seq_pressed(m_field.seq(SEQ_TYPE_INCREMENT)))
	{
		keypressed = true;
		if (m_delta)
			delta += apply_scale(m_delta, keyscale);
		else if (!m_lastdigital)
			// increment only once when first pressed
			delta += apply_scale(1, keyscale);
		m_lastdigital = true;
	}

	// if resetting is requested, clear the accumulated position to 0 before
	// applying the deltas so that we only return this frame's delta
	// note that centering only works for relative controls
	// no need to check if absolute here because it is checked by the validity tests
	if (m_field.analog_reset())
		m_accum = 0;

	// apply the delta to the accumulated value
	m_accum += delta;

	// if our last movement was due to a digital input, and if this control
	// type autocenters, and if neither the increment nor the decrement seq
	// was pressed, apply autocentering
	if (m_autocenter)
	{
		s32 center = apply_inverse_sensitivity(m_center);
		if (m_lastdigital && !keypressed)
		{
			if (m_accum >= center)
			{
				// autocenter from positive values
				m_accum -= apply_scale(m_centerdelta, m_keyscalepos);
				if (m_accum < center)
				{
					m_accum = center;
					m_lastdigital = false;
				}
			}
			else
			{
				// autocenter from negative values
				m_accum += apply_scale(m_centerdelta, m_keyscaleneg);
				if (m_accum > center)
				{
					m_accum = center;
					m_lastdigital = false;
				}
			}
		}
	}
	else if (!keypressed)
		m_lastdigital = false;
}


//-------------------------------------------------
//  read - read the current value and insert into
//  the provided ioport_value
//-------------------------------------------------

void analog_field::read(ioport_value &result)
{
	// do nothing if we're not enabled
	if (!m_field.enabled())
		return;

	// if set programmatically, only use the override value
	if (m_use_adjoverride)
	{
		result = m_adjoverride;
		return;
	}

	// start with the raw value
	s32 value = m_accum;

	// interpolate if appropriate and if time has passed since the last update
	if (m_interpolate)
		value = manager().frame_interpolate(m_previous, m_accum);

	// apply standard analog settings
	value = apply_settings(value);

	// remap the value if needed
	if (m_field.remap_table() != nullptr)
		value = m_field.remap_table()[value];

	// invert bits if needed
	if (m_field.analog_invert())
		value = ~value;

	// insert into the port
	result = (result & ~m_field.mask()) | ((value << m_shift) & m_field.mask());
}


//-------------------------------------------------
//  crosshair_read - read a value for crosshairs,
//  scaled between 0 and 1
//-------------------------------------------------

float analog_field::crosshair_read()
{
	s32 rawvalue = apply_settings(m_accum) & (m_field.mask() >> m_shift);
	return float(rawvalue - m_adjmin) / float(m_adjmax - m_adjmin);
}



/***************************************************************************
    TOKENIZATION HELPERS
***************************************************************************/

//-------------------------------------------------
//  token_to_input_type - convert a string token
//  to an input field type and player
//-------------------------------------------------

ioport_type ioport_manager::token_to_input_type(const char *string, int &player) const
{
	// check for our failsafe case first
	int ipnum;
	if (sscanf(string, "TYPE_OTHER(%d,%d)", &ipnum, &player) == 2)
		return ioport_type(ipnum);

	// find the token in the list
	for (const input_type_entry &entry : m_typelist)
		if (entry.token() != nullptr && !strcmp(entry.token(), string))
		{
			player = entry.player();
			return entry.type();
		}

	// if we fail, return IPT_UNKNOWN
	player = 0;
	return IPT_UNKNOWN;
}


//-------------------------------------------------
//  input_type_to_token - convert an input field
//  type and player to a string token
//-------------------------------------------------

std::string ioport_manager::input_type_to_token(ioport_type type, int player)
{
	// look up the port and return the token
	input_type_entry *entry = m_type_to_entry[type][player];
	if (entry != nullptr)
		return std::string(entry->token());

	// if that fails, carry on
	return string_format("TYPE_OTHER(%d,%d)", type, player);
}


//-------------------------------------------------
//  token_to_seq_type - convert a string to
//  a sequence type
//-------------------------------------------------

input_seq_type ioport_manager::token_to_seq_type(const char *string)
{
	// look up the string in the table of possible sequence types and return the index
	for (int seqindex = 0; seqindex < std::size(seqtypestrings); seqindex++)
		if (!core_stricmp(string, seqtypestrings[seqindex]))
			return input_seq_type(seqindex);
	return SEQ_TYPE_INVALID;
}
