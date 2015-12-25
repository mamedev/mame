// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    input.h

    Handle input from the user.

***************************************************************************/

#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef __INPUT_H__
#define __INPUT_H__


//**************************************************************************
//  CONSTANTS
//**************************************************************************

// relative devices return ~512 units per onscreen pixel
const INT32 INPUT_RELATIVE_PER_PIXEL = 512;

// absolute devices return values between -65536 and +65536
const INT32 INPUT_ABSOLUTE_MIN = -65536;
const INT32 INPUT_ABSOLUTE_MAX = 65536;

// maximum number of axis/buttons/hats with ITEM_IDs for use by osd layer
const int INPUT_MAX_AXIS = 8;
const int INPUT_MAX_BUTTONS = 32;
const int INPUT_MAX_HATS = 4;
const int INPUT_MAX_ADD_SWITCH = 16;
const int INPUT_MAX_ADD_ABSOLUTE = 16;
const int INPUT_MAX_ADD_RELATIVE = 16;


// device classes
enum input_device_class
{
	DEVICE_CLASS_INVALID,
	DEVICE_CLASS_FIRST_VALID,
	DEVICE_CLASS_KEYBOARD = DEVICE_CLASS_FIRST_VALID,
	DEVICE_CLASS_MOUSE,
	DEVICE_CLASS_LIGHTGUN,
	DEVICE_CLASS_JOYSTICK,
	DEVICE_CLASS_LAST_VALID = DEVICE_CLASS_JOYSTICK,
	DEVICE_CLASS_INTERNAL,
	DEVICE_CLASS_MAXIMUM
};
DECLARE_ENUM_OPERATORS(input_device_class)


// device index
const int DEVICE_INDEX_MAXIMUM = 0xff;


// input item classes
enum input_item_class
{
	ITEM_CLASS_INVALID,
	ITEM_CLASS_SWITCH,
	ITEM_CLASS_ABSOLUTE,
	ITEM_CLASS_RELATIVE,
	ITEM_CLASS_MAXIMUM
};


// input item modifiers
enum input_item_modifier
{
	ITEM_MODIFIER_NONE,
	ITEM_MODIFIER_POS,
	ITEM_MODIFIER_NEG,
	ITEM_MODIFIER_LEFT,
	ITEM_MODIFIER_RIGHT,
	ITEM_MODIFIER_UP,
	ITEM_MODIFIER_DOWN,
	ITEM_MODIFIER_MAXIMUM
};


// standard item IDs
enum input_item_id
{
	ITEM_ID_INVALID,
	ITEM_ID_FIRST_VALID,

	// standard keyboard IDs
	ITEM_ID_A = ITEM_ID_FIRST_VALID,
	ITEM_ID_B,
	ITEM_ID_C,
	ITEM_ID_D,
	ITEM_ID_E,
	ITEM_ID_F,
	ITEM_ID_G,
	ITEM_ID_H,
	ITEM_ID_I,
	ITEM_ID_J,
	ITEM_ID_K,
	ITEM_ID_L,
	ITEM_ID_M,
	ITEM_ID_N,
	ITEM_ID_O,
	ITEM_ID_P,
	ITEM_ID_Q,
	ITEM_ID_R,
	ITEM_ID_S,
	ITEM_ID_T,
	ITEM_ID_U,
	ITEM_ID_V,
	ITEM_ID_W,
	ITEM_ID_X,
	ITEM_ID_Y,
	ITEM_ID_Z,
	ITEM_ID_0,
	ITEM_ID_1,
	ITEM_ID_2,
	ITEM_ID_3,
	ITEM_ID_4,
	ITEM_ID_5,
	ITEM_ID_6,
	ITEM_ID_7,
	ITEM_ID_8,
	ITEM_ID_9,
	ITEM_ID_F1,
	ITEM_ID_F2,
	ITEM_ID_F3,
	ITEM_ID_F4,
	ITEM_ID_F5,
	ITEM_ID_F6,
	ITEM_ID_F7,
	ITEM_ID_F8,
	ITEM_ID_F9,
	ITEM_ID_F10,
	ITEM_ID_F11,
	ITEM_ID_F12,
	ITEM_ID_F13,
	ITEM_ID_F14,
	ITEM_ID_F15,
	ITEM_ID_ESC,
	ITEM_ID_TILDE,
	ITEM_ID_MINUS,
	ITEM_ID_EQUALS,
	ITEM_ID_BACKSPACE,
	ITEM_ID_TAB,
	ITEM_ID_OPENBRACE,
	ITEM_ID_CLOSEBRACE,
	ITEM_ID_ENTER,
	ITEM_ID_COLON,
	ITEM_ID_QUOTE,
	ITEM_ID_BACKSLASH,
	ITEM_ID_BACKSLASH2,
	ITEM_ID_COMMA,
	ITEM_ID_STOP,
	ITEM_ID_SLASH,
	ITEM_ID_SPACE,
	ITEM_ID_INSERT,
	ITEM_ID_DEL,
	ITEM_ID_HOME,
	ITEM_ID_END,
	ITEM_ID_PGUP,
	ITEM_ID_PGDN,
	ITEM_ID_LEFT,
	ITEM_ID_RIGHT,
	ITEM_ID_UP,
	ITEM_ID_DOWN,
	ITEM_ID_0_PAD,
	ITEM_ID_1_PAD,
	ITEM_ID_2_PAD,
	ITEM_ID_3_PAD,
	ITEM_ID_4_PAD,
	ITEM_ID_5_PAD,
	ITEM_ID_6_PAD,
	ITEM_ID_7_PAD,
	ITEM_ID_8_PAD,
	ITEM_ID_9_PAD,
	ITEM_ID_SLASH_PAD,
	ITEM_ID_ASTERISK,
	ITEM_ID_MINUS_PAD,
	ITEM_ID_PLUS_PAD,
	ITEM_ID_DEL_PAD,
	ITEM_ID_ENTER_PAD,
	ITEM_ID_PRTSCR,
	ITEM_ID_PAUSE,
	ITEM_ID_LSHIFT,
	ITEM_ID_RSHIFT,
	ITEM_ID_LCONTROL,
	ITEM_ID_RCONTROL,
	ITEM_ID_LALT,
	ITEM_ID_RALT,
	ITEM_ID_SCRLOCK,
	ITEM_ID_NUMLOCK,
	ITEM_ID_CAPSLOCK,
	ITEM_ID_LWIN,
	ITEM_ID_RWIN,
	ITEM_ID_MENU,
	ITEM_ID_CANCEL,

	// standard mouse/joystick/gun IDs
	ITEM_ID_XAXIS,
	ITEM_ID_YAXIS,
	ITEM_ID_ZAXIS,
	ITEM_ID_RXAXIS,
	ITEM_ID_RYAXIS,
	ITEM_ID_RZAXIS,
	ITEM_ID_SLIDER1,
	ITEM_ID_SLIDER2,
	ITEM_ID_BUTTON1,
	ITEM_ID_BUTTON2,
	ITEM_ID_BUTTON3,
	ITEM_ID_BUTTON4,
	ITEM_ID_BUTTON5,
	ITEM_ID_BUTTON6,
	ITEM_ID_BUTTON7,
	ITEM_ID_BUTTON8,
	ITEM_ID_BUTTON9,
	ITEM_ID_BUTTON10,
	ITEM_ID_BUTTON11,
	ITEM_ID_BUTTON12,
	ITEM_ID_BUTTON13,
	ITEM_ID_BUTTON14,
	ITEM_ID_BUTTON15,
	ITEM_ID_BUTTON16,
	ITEM_ID_BUTTON17,
	ITEM_ID_BUTTON18,
	ITEM_ID_BUTTON19,
	ITEM_ID_BUTTON20,
	ITEM_ID_BUTTON21,
	ITEM_ID_BUTTON22,
	ITEM_ID_BUTTON23,
	ITEM_ID_BUTTON24,
	ITEM_ID_BUTTON25,
	ITEM_ID_BUTTON26,
	ITEM_ID_BUTTON27,
	ITEM_ID_BUTTON28,
	ITEM_ID_BUTTON29,
	ITEM_ID_BUTTON30,
	ITEM_ID_BUTTON31,
	ITEM_ID_BUTTON32,
	ITEM_ID_START,
	ITEM_ID_SELECT,

	// Hats
	ITEM_ID_HAT1UP,
	ITEM_ID_HAT1DOWN,
	ITEM_ID_HAT1LEFT,
	ITEM_ID_HAT1RIGHT,
	ITEM_ID_HAT2UP,
	ITEM_ID_HAT2DOWN,
	ITEM_ID_HAT2LEFT,
	ITEM_ID_HAT2RIGHT,
	ITEM_ID_HAT3UP,
	ITEM_ID_HAT3DOWN,
	ITEM_ID_HAT3LEFT,
	ITEM_ID_HAT3RIGHT,
	ITEM_ID_HAT4UP,
	ITEM_ID_HAT4DOWN,
	ITEM_ID_HAT4LEFT,
	ITEM_ID_HAT4RIGHT,

	// Additional IDs
	ITEM_ID_ADD_SWITCH1,
	ITEM_ID_ADD_SWITCH2,
	ITEM_ID_ADD_SWITCH3,
	ITEM_ID_ADD_SWITCH4,
	ITEM_ID_ADD_SWITCH5,
	ITEM_ID_ADD_SWITCH6,
	ITEM_ID_ADD_SWITCH7,
	ITEM_ID_ADD_SWITCH8,
	ITEM_ID_ADD_SWITCH9,
	ITEM_ID_ADD_SWITCH10,
	ITEM_ID_ADD_SWITCH11,
	ITEM_ID_ADD_SWITCH12,
	ITEM_ID_ADD_SWITCH13,
	ITEM_ID_ADD_SWITCH14,
	ITEM_ID_ADD_SWITCH15,
	ITEM_ID_ADD_SWITCH16,

	ITEM_ID_ADD_ABSOLUTE1,
	ITEM_ID_ADD_ABSOLUTE2,
	ITEM_ID_ADD_ABSOLUTE3,
	ITEM_ID_ADD_ABSOLUTE4,
	ITEM_ID_ADD_ABSOLUTE5,
	ITEM_ID_ADD_ABSOLUTE6,
	ITEM_ID_ADD_ABSOLUTE7,
	ITEM_ID_ADD_ABSOLUTE8,
	ITEM_ID_ADD_ABSOLUTE9,
	ITEM_ID_ADD_ABSOLUTE10,
	ITEM_ID_ADD_ABSOLUTE11,
	ITEM_ID_ADD_ABSOLUTE12,
	ITEM_ID_ADD_ABSOLUTE13,
	ITEM_ID_ADD_ABSOLUTE14,
	ITEM_ID_ADD_ABSOLUTE15,
	ITEM_ID_ADD_ABSOLUTE16,

	ITEM_ID_ADD_RELATIVE1,
	ITEM_ID_ADD_RELATIVE2,
	ITEM_ID_ADD_RELATIVE3,
	ITEM_ID_ADD_RELATIVE4,
	ITEM_ID_ADD_RELATIVE5,
	ITEM_ID_ADD_RELATIVE6,
	ITEM_ID_ADD_RELATIVE7,
	ITEM_ID_ADD_RELATIVE8,
	ITEM_ID_ADD_RELATIVE9,
	ITEM_ID_ADD_RELATIVE10,
	ITEM_ID_ADD_RELATIVE11,
	ITEM_ID_ADD_RELATIVE12,
	ITEM_ID_ADD_RELATIVE13,
	ITEM_ID_ADD_RELATIVE14,
	ITEM_ID_ADD_RELATIVE15,
	ITEM_ID_ADD_RELATIVE16,

	// generic other IDs
	ITEM_ID_OTHER_SWITCH,
	ITEM_ID_OTHER_AXIS_ABSOLUTE,
	ITEM_ID_OTHER_AXIS_RELATIVE,
	ITEM_ID_MAXIMUM,

	// internal codes for sequences
	ITEM_ID_SEQ_END,
	ITEM_ID_SEQ_DEFAULT,
	ITEM_ID_SEQ_NOT,
	ITEM_ID_SEQ_OR,

	// absolute maximum ID
	ITEM_ID_ABSOLUTE_MAXIMUM = 0xfff
};
DECLARE_ENUM_OPERATORS(input_item_id)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// forward declarations
class input_device_item;
class input_device;
class input_class;
class input_manager;


// callback for getting the value of an item on a device
typedef INT32 (*item_get_state_func)(void *device_internal, void *item_internal);


// ======================> joystick_map

// a 9x9 joystick map
class joystick_map
{
public:
	// construction/destruction
	joystick_map();
	joystick_map(const joystick_map &src) { copy(src); }

	// operators
	joystick_map &operator=(const joystick_map &src) { if (this != &src) copy(src); return *this; }

	// parse from a string
	bool parse(const char *mapstring);

	// create a friendly string
	const char *to_string(std::string &str) const;

	// update the state of a live map
	UINT8 update(INT32 xaxisval, INT32 yaxisval);

	// joystick mapping codes
	static const UINT8 JOYSTICK_MAP_NEUTRAL = 0x00;
	static const UINT8 JOYSTICK_MAP_LEFT    = 0x01;
	static const UINT8 JOYSTICK_MAP_RIGHT   = 0x02;
	static const UINT8 JOYSTICK_MAP_UP      = 0x04;
	static const UINT8 JOYSTICK_MAP_DOWN    = 0x08;
	static const UINT8 JOYSTICK_MAP_STICKY  = 0x0f;

private:
	// internal helpers
	void copy(const joystick_map &src)
	{
		memcpy(m_map, src.m_map, sizeof(m_map));
		m_lastmap = JOYSTICK_MAP_NEUTRAL;
		m_origstring = src.m_origstring;
	}

	// internal state
	UINT8                   m_map[9][9];            // 9x9 grid
	UINT8                   m_lastmap;              // last value returned (for sticky tracking)
	std::string             m_origstring;           // originally parsed string
};


// ======================> input_code

// a combined code that describes a particular input on a particular device
class input_code
{
public:
	// construction/destruction
	input_code(input_device_class devclass = DEVICE_CLASS_INVALID, int devindex = 0, input_item_class itemclass = ITEM_CLASS_INVALID, input_item_modifier modifier = ITEM_MODIFIER_NONE, input_item_id itemid = ITEM_ID_INVALID)
		: m_internal(((devclass & 0xf) << 28) | ((devindex & 0xff) << 20) | ((itemclass & 0xf) << 16) | ((modifier & 0xf) << 12) | (itemid & 0xfff))
	{
		assert(devclass >= 0 && devclass < DEVICE_CLASS_MAXIMUM);
		assert(devindex >= 0 && devindex < DEVICE_INDEX_MAXIMUM);
		assert(itemclass >= 0 && itemclass < ITEM_CLASS_MAXIMUM);
		assert(modifier >= 0 && modifier < ITEM_MODIFIER_MAXIMUM);
		assert(itemid >= 0 && itemid < ITEM_ID_ABSOLUTE_MAXIMUM);
	}
	input_code(const input_code &src)
		: m_internal(src.m_internal) { }
	input_code(input_device &device, input_item_id itemid);

	// operators
	bool operator==(const input_code &rhs) const { return m_internal == rhs.m_internal; }
	bool operator!=(const input_code &rhs) const { return m_internal != rhs.m_internal; }

	// getters
	bool internal() const { return device_class() == DEVICE_CLASS_INTERNAL; }
	input_device_class device_class() const { return input_device_class((m_internal >> 28) & 0xf); }
	int device_index() const { return ((m_internal >> 20) & 0xff); }
	input_item_class item_class() const { return input_item_class((m_internal >> 16) & 0xf); }
	input_item_modifier item_modifier() const { return input_item_modifier((m_internal >> 12) & 0xf); }
	input_item_id item_id() const { return input_item_id(m_internal & 0xfff); }

	// setters
	void set_device_class(input_device_class devclass) { assert(devclass >= 0 && devclass <= 0xf); m_internal = (m_internal & ~(0xf << 28)) | ((devclass & 0xf) << 28); }
	void set_device_index(int devindex) { assert(devindex >= 0 && devindex <= 0xff); m_internal = (m_internal & ~(0xff << 20)) | ((devindex & 0xff) << 20); }
	void set_item_class(input_item_class itemclass) { assert(itemclass >= 0 && itemclass <= 0xf); m_internal = (m_internal & ~(0xf << 16)) | ((itemclass & 0xf) << 16); }
	void set_item_modifier(input_item_modifier modifier) { assert(modifier >= 0 && modifier <= 0xf); m_internal = (m_internal & ~(0xf << 12)) | ((modifier & 0xf) << 12); }
	void set_item_id(input_item_id itemid) { assert(itemid >= 0 && itemid <= 0xfff); m_internal = (m_internal & ~0xfff) | (itemid & 0xfff); }

private:
	// internal state
	UINT32      m_internal;
};


// ======================> input_seq

// a sequence of input_codes, supporting AND/OR and inversion
class input_seq
{
public:
	// construction/destruction
	input_seq(input_code code0 = input_seq::end_code, input_code code1 = input_seq::end_code, input_code code2 = input_seq::end_code, input_code code3 = input_seq::end_code, input_code code4 = input_seq::end_code, input_code code5 = input_seq::end_code, input_code code6 = input_seq::end_code)
		{ set(code0, code1, code2, code3, code4, code5, code6); }
	input_seq(const input_seq &rhs) { memcpy(m_code, rhs.m_code, sizeof(m_code)); }

	// operators
	bool operator==(const input_seq &rhs) const { return (memcmp(m_code, rhs.m_code, sizeof(m_code)) == 0); }
	bool operator!=(const input_seq &rhs) const { return (memcmp(m_code, rhs.m_code, sizeof(m_code)) != 0); }
	input_code operator[](int index) const { return (index >= 0 && index < ARRAY_LENGTH(m_code)) ? m_code[index] : input_seq::end_code; }
	input_seq &operator+=(input_code code);
	input_seq &operator|=(input_code code);

	// getters
	int length() const;
	bool is_valid() const;
	bool is_default() const { return m_code[0] == default_code; }

	// setters
	void set(input_code code0 = input_seq::end_code, input_code code1 = input_seq::end_code, input_code code2 = input_seq::end_code, input_code code3 = input_seq::end_code, input_code code4 = input_seq::end_code, input_code code5 = input_seq::end_code, input_code code6 = input_seq::end_code);
	void reset() { set(); }
	void set_default() { set(default_code); }
	void backspace();
	void replace(input_code oldcode, input_code newcode);

	// constant codes used in sequences
	static const input_code end_code;
	static const input_code default_code;
	static const input_code not_code;
	static const input_code or_code;

	// constant sequences
	static const input_seq empty_seq;

private:
	// internal state
	input_code  m_code[16];
};


// ======================> input_device_item

// a single item on an input device
class input_device_item
{
protected:
	// construction/destruction
	input_device_item(input_device &device, const char *name, void *internal, input_item_id itemid, item_get_state_func getstate, input_item_class itemclass);

public:
	virtual ~input_device_item();

	// getters
	input_device &device() const { return m_device; }
	input_manager &manager() const;
	running_machine &machine() const;
	const char *name() const { return m_name.c_str(); }
	void *internal() const { return m_internal; }
	input_item_id itemid() const { return m_itemid; }
	input_item_class itemclass() const { return m_itemclass; }
	const char *token() const { return m_token.c_str(); }
	INT32 current() const { return m_current; }
	INT32 memory() const { return m_memory; }

	// helpers
	INT32 update_value();
	void set_memory(INT32 value) { m_memory = value; }

	// readers
	virtual INT32 read_as_switch(input_item_modifier modifier) = 0;
	virtual INT32 read_as_relative(input_item_modifier modifier) = 0;
	virtual INT32 read_as_absolute(input_item_modifier modifier) = 0;

protected:
	// internal state
	input_device &          m_device;               // reference to our owning device
	std::string             m_name;                 // string name of item
	void *                  m_internal;             // internal callback pointer
	input_item_id           m_itemid;               // originally specified item id
	input_item_class        m_itemclass;            // class of the item
	item_get_state_func     m_getstate;             // get state callback
	std::string             m_token;                // tokenized name for non-standard items

	// live state
	INT32                   m_current;              // current raw value
	INT32                   m_memory;               // "memory" value, to remember where we started during polling
};


// ======================> input_device

// a logical device of a given class that can provide input
class input_device
{
	friend class input_class;

public:
	// construction/destruction
	input_device(input_class &_class, int _devindex, const char *_name, void *_internal);
	// getters
	input_class &device_class() const { return m_class; }
	input_manager &manager() const;
	running_machine &machine() const;
	input_device_class devclass() const;
	const char *name() const { return m_name.c_str(); }
	int devindex() const { return m_devindex; }
	input_device_item *item(input_item_id index) const { return m_item[index].get(); }
	input_item_id maxitem() const { return m_maxitem; }
	void *internal() const { return m_internal; }
	joystick_map &joymap() { return m_joymap; }
	bool steadykey_enabled() const { return m_steadykey_enabled; }
	bool lightgun_reload_button() const { return m_lightgun_reload_button; }

	// item management
	input_item_id add_item(const char *name, input_item_id itemid, item_get_state_func getstate, void *internal = nullptr);
	void set_joystick_map(const joystick_map &map) { m_joymap = map; }

	// helpers
	INT32 apply_deadzone_and_saturation(INT32 value) const;
	void apply_steadykey() const;

private:
	// internal state
	input_class &           m_class;                // reference to our class
	std::string             m_name;                 // string name of device
	int                     m_devindex;             // device index of this device
	std::unique_ptr<input_device_item> m_item[ITEM_ID_ABSOLUTE_MAXIMUM+1]; // array of pointers to items
	input_item_id           m_maxitem;              // maximum item index
	void *                  m_internal;             // internal callback pointer

	// joystick information
	joystick_map            m_joymap;               // joystick map for this device
	INT32                   m_joystick_deadzone;    // deadzone for joystick
	INT32                   m_joystick_saturation;  // saturation position for joystick
	bool                    m_steadykey_enabled;    // steadykey enabled for keyboards
	bool                    m_lightgun_reload_button; // lightgun reload hack
};


// ======================> input_class

// a class of device that provides input
class input_class
{
public:
	// construction/destruction
	input_class(input_manager &manager, input_device_class devclass, bool enabled = false, bool multi = false);

	// getters
	input_manager &manager() const { return m_manager; }
	running_machine &machine() const;
	input_device *device(int index) const { return (index <= m_maxindex) ? m_device[index].get() : nullptr; }
	input_device_class devclass() const { return m_devclass; }
	int maxindex() const { return m_maxindex; }
	bool enabled() const { return m_enabled; }
	bool multi() const { return m_multi; }

	// setters
	void enable(bool state = true) { m_enabled = state; }
	void set_multi(bool multi = true) { m_multi = multi; }

	// device management
	input_device *add_device(const char *name, void *internal = nullptr);
	input_device *add_device(int devindex, const char *name, void *internal = nullptr);

	// misc helpers
	input_item_class standard_item_class(input_item_id itemid);

private:
	// internal helpers
	void frame_callback();

	// internal state
	input_manager &         m_manager;              // reference to our manager
	std::unique_ptr<input_device> m_device[DEVICE_INDEX_MAXIMUM]; // array of devices in this class
	input_device_class      m_devclass;             // our device class
	int                     m_maxindex;             // maximum populated index
	bool                    m_enabled;              // is this class enabled?
	bool                    m_multi;                // are multiple instances of this class allowed?
};


// ======================> input_manager

// global machine-level information about devices
class input_manager
{
public:
	// construction/destruction
	input_manager(running_machine &machine);

	// getters
	running_machine &machine() const { return m_machine; }
	input_class &device_class(input_device_class devclass) { assert(devclass < ARRAY_LENGTH(m_class)); assert(m_class[devclass] != nullptr); return *m_class[devclass]; }

	// input code readers
	INT32 code_value(input_code code);
	bool code_pressed(input_code code) { return code_value(code) != 0; }
	bool code_pressed_once(input_code code);

	// input code polling
	void reset_polling();
	input_code poll_axes();
	input_code poll_switches();
	input_code poll_keyboard_switches();

	// input code helpers
	input_device *device_from_code(input_code code) const;
	input_device_item *item_from_code(input_code code) const;
	input_code code_from_itemid(input_item_id itemid) const;
	const char *code_name(std::string &str, input_code code) const;
	const char *code_to_token(std::string &str, input_code code) const;
	input_code code_from_token(const char *_token);

	// input sequence readers
	bool seq_pressed(const input_seq &seq);
	INT32 seq_axis_value(const input_seq &seq, input_item_class &itemclass);

	// input sequence polling
	void seq_poll_start(input_item_class itemclass, const input_seq *startseq = nullptr);
	bool seq_poll();
	const input_seq &seq_poll_final() const { return m_poll_seq; }

	// input sequence helpers
	const char *seq_name(std::string &str, const input_seq &seq) const;
	const char *seq_to_tokens(std::string &str, const input_seq &seq) const;
	void seq_from_tokens(input_seq &seq, const char *_token);

	// misc
	bool set_global_joystick_map(const char *mapstring);

private:
	// internal helpers
	void reset_memory();
	bool code_check_axis(input_device_item &item, input_code code);

	// internal state
	running_machine &   m_machine;
	input_code          m_switch_memory[64];

	// classes
	input_class         m_keyboard_class;
	input_class         m_mouse_class;
	input_class         m_joystick_class;
	input_class         m_lightgun_class;
	input_class *       m_class[DEVICE_CLASS_MAXIMUM];

	// sequence polling state
	input_seq           m_poll_seq;
	osd_ticks_t         m_poll_seq_last_ticks;
	input_item_class    m_poll_seq_class;
};



//**************************************************************************
//  MACROS
//**************************************************************************

// invalid codes
#define INPUT_CODE_INVALID input_code()

// keyboard codes
#define KEYCODE_A_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_A)
#define KEYCODE_B_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_B)
#define KEYCODE_C_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_C)
#define KEYCODE_D_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_D)
#define KEYCODE_E_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_E)
#define KEYCODE_F_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_F)
#define KEYCODE_G_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_G)
#define KEYCODE_H_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_H)
#define KEYCODE_I_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_I)
#define KEYCODE_J_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_J)
#define KEYCODE_K_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_K)
#define KEYCODE_L_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_L)
#define KEYCODE_M_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_M)
#define KEYCODE_N_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_N)
#define KEYCODE_O_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_O)
#define KEYCODE_P_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_P)
#define KEYCODE_Q_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_Q)
#define KEYCODE_R_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_R)
#define KEYCODE_S_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_S)
#define KEYCODE_T_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_T)
#define KEYCODE_U_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_U)
#define KEYCODE_V_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_V)
#define KEYCODE_W_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_W)
#define KEYCODE_X_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_X)
#define KEYCODE_Y_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_Y)
#define KEYCODE_Z_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_Z)
#define KEYCODE_0_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_0)
#define KEYCODE_1_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_1)
#define KEYCODE_2_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_2)
#define KEYCODE_3_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_3)
#define KEYCODE_4_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_4)
#define KEYCODE_5_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_5)
#define KEYCODE_6_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_6)
#define KEYCODE_7_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_7)
#define KEYCODE_8_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_8)
#define KEYCODE_9_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_9)
#define KEYCODE_F1_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_F1)
#define KEYCODE_F2_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_F2)
#define KEYCODE_F3_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_F3)
#define KEYCODE_F4_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_F4)
#define KEYCODE_F5_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_F5)
#define KEYCODE_F6_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_F6)
#define KEYCODE_F7_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_F7)
#define KEYCODE_F8_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_F8)
#define KEYCODE_F9_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_F9)
#define KEYCODE_F10_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_F10)
#define KEYCODE_F11_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_F11)
#define KEYCODE_F12_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_F12)
#define KEYCODE_F13_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_F13)
#define KEYCODE_F14_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_F14)
#define KEYCODE_F15_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_F15)
#define KEYCODE_ESC_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_ESC)
#define KEYCODE_TILDE_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_TILDE)
#define KEYCODE_MINUS_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_MINUS)
#define KEYCODE_EQUALS_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_EQUALS)
#define KEYCODE_BACKSPACE_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_BACKSPACE)
#define KEYCODE_TAB_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_TAB)
#define KEYCODE_OPENBRACE_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_OPENBRACE)
#define KEYCODE_CLOSEBRACE_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_CLOSEBRACE)
#define KEYCODE_ENTER_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_ENTER)
#define KEYCODE_COLON_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_COLON)
#define KEYCODE_QUOTE_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_QUOTE)
#define KEYCODE_BACKSLASH_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_BACKSLASH)
#define KEYCODE_BACKSLASH2_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_BACKSLASH2)
#define KEYCODE_COMMA_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_COMMA)
#define KEYCODE_STOP_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_STOP)
#define KEYCODE_SLASH_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_SLASH)
#define KEYCODE_SPACE_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_SPACE)
#define KEYCODE_INSERT_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_INSERT)
#define KEYCODE_DEL_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_DEL)
#define KEYCODE_HOME_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_HOME)
#define KEYCODE_END_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_END)
#define KEYCODE_PGUP_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_PGUP)
#define KEYCODE_PGDN_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_PGDN)
#define KEYCODE_LEFT_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_LEFT)
#define KEYCODE_RIGHT_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_RIGHT)
#define KEYCODE_UP_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_UP)
#define KEYCODE_DOWN_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_DOWN)
#define KEYCODE_0_PAD_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_0_PAD)
#define KEYCODE_1_PAD_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_1_PAD)
#define KEYCODE_2_PAD_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_2_PAD)
#define KEYCODE_3_PAD_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_3_PAD)
#define KEYCODE_4_PAD_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_4_PAD)
#define KEYCODE_5_PAD_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_5_PAD)
#define KEYCODE_6_PAD_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_6_PAD)
#define KEYCODE_7_PAD_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_7_PAD)
#define KEYCODE_8_PAD_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_8_PAD)
#define KEYCODE_9_PAD_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_9_PAD)
#define KEYCODE_SLASH_PAD_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_SLASH_PAD)
#define KEYCODE_ASTERISK_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_ASTERISK)
#define KEYCODE_MINUS_PAD_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_MINUS_PAD)
#define KEYCODE_PLUS_PAD_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_PLUS_PAD)
#define KEYCODE_DEL_PAD_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_DEL_PAD)
#define KEYCODE_ENTER_PAD_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_ENTER_PAD)
#define KEYCODE_PRTSCR_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_PRTSCR)
#define KEYCODE_PAUSE_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_PAUSE)
#define KEYCODE_LSHIFT_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_LSHIFT)
#define KEYCODE_RSHIFT_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_RSHIFT)
#define KEYCODE_LCONTROL_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_LCONTROL)
#define KEYCODE_RCONTROL_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_RCONTROL)
#define KEYCODE_LALT_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_LALT)
#define KEYCODE_RALT_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_RALT)
#define KEYCODE_SCRLOCK_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_SCRLOCK)
#define KEYCODE_NUMLOCK_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_NUMLOCK)
#define KEYCODE_CAPSLOCK_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_CAPSLOCK)
#define KEYCODE_LWIN_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_LWIN)
#define KEYCODE_RWIN_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_RWIN)
#define KEYCODE_MENU_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_MENU)
#define KEYCODE_CANCEL_INDEXED(n) input_code(DEVICE_CLASS_KEYBOARD, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_CANCEL)

#define KEYCODE_A KEYCODE_A_INDEXED(0)
#define KEYCODE_B KEYCODE_B_INDEXED(0)
#define KEYCODE_C KEYCODE_C_INDEXED(0)
#define KEYCODE_D KEYCODE_D_INDEXED(0)
#define KEYCODE_E KEYCODE_E_INDEXED(0)
#define KEYCODE_F KEYCODE_F_INDEXED(0)
#define KEYCODE_G KEYCODE_G_INDEXED(0)
#define KEYCODE_H KEYCODE_H_INDEXED(0)
#define KEYCODE_I KEYCODE_I_INDEXED(0)
#define KEYCODE_J KEYCODE_J_INDEXED(0)
#define KEYCODE_K KEYCODE_K_INDEXED(0)
#define KEYCODE_L KEYCODE_L_INDEXED(0)
#define KEYCODE_M KEYCODE_M_INDEXED(0)
#define KEYCODE_N KEYCODE_N_INDEXED(0)
#define KEYCODE_O KEYCODE_O_INDEXED(0)
#define KEYCODE_P KEYCODE_P_INDEXED(0)
#define KEYCODE_Q KEYCODE_Q_INDEXED(0)
#define KEYCODE_R KEYCODE_R_INDEXED(0)
#define KEYCODE_S KEYCODE_S_INDEXED(0)
#define KEYCODE_T KEYCODE_T_INDEXED(0)
#define KEYCODE_U KEYCODE_U_INDEXED(0)
#define KEYCODE_V KEYCODE_V_INDEXED(0)
#define KEYCODE_W KEYCODE_W_INDEXED(0)
#define KEYCODE_X KEYCODE_X_INDEXED(0)
#define KEYCODE_Y KEYCODE_Y_INDEXED(0)
#define KEYCODE_Z KEYCODE_Z_INDEXED(0)
#define KEYCODE_0 KEYCODE_0_INDEXED(0)
#define KEYCODE_1 KEYCODE_1_INDEXED(0)
#define KEYCODE_2 KEYCODE_2_INDEXED(0)
#define KEYCODE_3 KEYCODE_3_INDEXED(0)
#define KEYCODE_4 KEYCODE_4_INDEXED(0)
#define KEYCODE_5 KEYCODE_5_INDEXED(0)
#define KEYCODE_6 KEYCODE_6_INDEXED(0)
#define KEYCODE_7 KEYCODE_7_INDEXED(0)
#define KEYCODE_8 KEYCODE_8_INDEXED(0)
#define KEYCODE_9 KEYCODE_9_INDEXED(0)
#define KEYCODE_F1 KEYCODE_F1_INDEXED(0)
#define KEYCODE_F2 KEYCODE_F2_INDEXED(0)
#define KEYCODE_F3 KEYCODE_F3_INDEXED(0)
#define KEYCODE_F4 KEYCODE_F4_INDEXED(0)
#define KEYCODE_F5 KEYCODE_F5_INDEXED(0)
#define KEYCODE_F6 KEYCODE_F6_INDEXED(0)
#define KEYCODE_F7 KEYCODE_F7_INDEXED(0)
#define KEYCODE_F8 KEYCODE_F8_INDEXED(0)
#define KEYCODE_F9 KEYCODE_F9_INDEXED(0)
#define KEYCODE_F10 KEYCODE_F10_INDEXED(0)
#define KEYCODE_F11 KEYCODE_F11_INDEXED(0)
#define KEYCODE_F12 KEYCODE_F12_INDEXED(0)
#define KEYCODE_F13 KEYCODE_F13_INDEXED(0)
#define KEYCODE_F14 KEYCODE_F14_INDEXED(0)
#define KEYCODE_F15 KEYCODE_F15_INDEXED(0)
#define KEYCODE_ESC KEYCODE_ESC_INDEXED(0)
#define KEYCODE_TILDE KEYCODE_TILDE_INDEXED(0)
#define KEYCODE_MINUS KEYCODE_MINUS_INDEXED(0)
#define KEYCODE_EQUALS KEYCODE_EQUALS_INDEXED(0)
#define KEYCODE_BACKSPACE KEYCODE_BACKSPACE_INDEXED(0)
#define KEYCODE_TAB KEYCODE_TAB_INDEXED(0)
#define KEYCODE_OPENBRACE KEYCODE_OPENBRACE_INDEXED(0)
#define KEYCODE_CLOSEBRACE KEYCODE_CLOSEBRACE_INDEXED(0)
#define KEYCODE_ENTER KEYCODE_ENTER_INDEXED(0)
#define KEYCODE_COLON KEYCODE_COLON_INDEXED(0)
#define KEYCODE_QUOTE KEYCODE_QUOTE_INDEXED(0)
#define KEYCODE_BACKSLASH KEYCODE_BACKSLASH_INDEXED(0)
#define KEYCODE_BACKSLASH2 KEYCODE_BACKSLASH2_INDEXED(0)
#define KEYCODE_COMMA KEYCODE_COMMA_INDEXED(0)
#define KEYCODE_STOP KEYCODE_STOP_INDEXED(0)
#define KEYCODE_SLASH KEYCODE_SLASH_INDEXED(0)
#define KEYCODE_SPACE KEYCODE_SPACE_INDEXED(0)
#define KEYCODE_INSERT KEYCODE_INSERT_INDEXED(0)
#define KEYCODE_DEL KEYCODE_DEL_INDEXED(0)
#define KEYCODE_HOME KEYCODE_HOME_INDEXED(0)
#define KEYCODE_END KEYCODE_END_INDEXED(0)
#define KEYCODE_PGUP KEYCODE_PGUP_INDEXED(0)
#define KEYCODE_PGDN KEYCODE_PGDN_INDEXED(0)
#define KEYCODE_LEFT KEYCODE_LEFT_INDEXED(0)
#define KEYCODE_RIGHT KEYCODE_RIGHT_INDEXED(0)
#define KEYCODE_UP KEYCODE_UP_INDEXED(0)
#define KEYCODE_DOWN KEYCODE_DOWN_INDEXED(0)
#define KEYCODE_0_PAD KEYCODE_0_PAD_INDEXED(0)
#define KEYCODE_1_PAD KEYCODE_1_PAD_INDEXED(0)
#define KEYCODE_2_PAD KEYCODE_2_PAD_INDEXED(0)
#define KEYCODE_3_PAD KEYCODE_3_PAD_INDEXED(0)
#define KEYCODE_4_PAD KEYCODE_4_PAD_INDEXED(0)
#define KEYCODE_5_PAD KEYCODE_5_PAD_INDEXED(0)
#define KEYCODE_6_PAD KEYCODE_6_PAD_INDEXED(0)
#define KEYCODE_7_PAD KEYCODE_7_PAD_INDEXED(0)
#define KEYCODE_8_PAD KEYCODE_8_PAD_INDEXED(0)
#define KEYCODE_9_PAD KEYCODE_9_PAD_INDEXED(0)
#define KEYCODE_SLASH_PAD KEYCODE_SLASH_PAD_INDEXED(0)
#define KEYCODE_ASTERISK KEYCODE_ASTERISK_INDEXED(0)
#define KEYCODE_MINUS_PAD KEYCODE_MINUS_PAD_INDEXED(0)
#define KEYCODE_PLUS_PAD KEYCODE_PLUS_PAD_INDEXED(0)
#define KEYCODE_DEL_PAD KEYCODE_DEL_PAD_INDEXED(0)
#define KEYCODE_ENTER_PAD KEYCODE_ENTER_PAD_INDEXED(0)
#define KEYCODE_PRTSCR KEYCODE_PRTSCR_INDEXED(0)
#define KEYCODE_PAUSE KEYCODE_PAUSE_INDEXED(0)
#define KEYCODE_LSHIFT KEYCODE_LSHIFT_INDEXED(0)
#define KEYCODE_RSHIFT KEYCODE_RSHIFT_INDEXED(0)
#define KEYCODE_LCONTROL KEYCODE_LCONTROL_INDEXED(0)
#define KEYCODE_RCONTROL KEYCODE_RCONTROL_INDEXED(0)
#define KEYCODE_LALT KEYCODE_LALT_INDEXED(0)
#define KEYCODE_RALT KEYCODE_RALT_INDEXED(0)
#define KEYCODE_SCRLOCK KEYCODE_SCRLOCK_INDEXED(0)
#define KEYCODE_NUMLOCK KEYCODE_NUMLOCK_INDEXED(0)
#define KEYCODE_CAPSLOCK KEYCODE_CAPSLOCK_INDEXED(0)
#define KEYCODE_LWIN KEYCODE_LWIN_INDEXED(0)
#define KEYCODE_RWIN KEYCODE_RWIN_INDEXED(0)
#define KEYCODE_MENU KEYCODE_MENU_INDEXED(0)
#define KEYCODE_CANCEL KEYCODE_CANCEL_INDEXED(0)

// mouse axes as relative devices
#define MOUSECODE_X_INDEXED(n) input_code(DEVICE_CLASS_MOUSE, n, ITEM_CLASS_RELATIVE, ITEM_MODIFIER_NONE, ITEM_ID_XAXIS)
#define MOUSECODE_Y_INDEXED(n) input_code(DEVICE_CLASS_MOUSE, n, ITEM_CLASS_RELATIVE, ITEM_MODIFIER_NONE, ITEM_ID_YAXIS)
#define MOUSECODE_Z_INDEXED(n) input_code(DEVICE_CLASS_MOUSE, n, ITEM_CLASS_RELATIVE, ITEM_MODIFIER_NONE, ITEM_ID_ZAXIS)

#define MOUSECODE_X MOUSECODE_X_INDEXED(0)
#define MOUSECODE_Y MOUSECODE_Y_INDEXED(0)
#define MOUSECODE_Z MOUSECODE_Z_INDEXED(0)

// mouse axes as switches in +/- direction
#define MOUSECODE_X_POS_SWITCH_INDEXED(n) input_code(DEVICE_CLASS_MOUSE, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_POS, ITEM_ID_XAXIS)
#define MOUSECODE_X_NEG_SWITCH_INDEXED(n) input_code(DEVICE_CLASS_MOUSE, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NEG, ITEM_ID_XAXIS)
#define MOUSECODE_Y_POS_SWITCH_INDEXED(n) input_code(DEVICE_CLASS_MOUSE, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_POS, ITEM_ID_YAXIS)
#define MOUSECODE_Y_NEG_SWITCH_INDEXED(n) input_code(DEVICE_CLASS_MOUSE, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NEG, ITEM_ID_YAXIS)
#define MOUSECODE_Z_POS_SWITCH_INDEXED(n) input_code(DEVICE_CLASS_MOUSE, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_POS, ITEM_ID_ZAXIS)
#define MOUSECODE_Z_NEG_SWITCH_INDEXED(n) input_code(DEVICE_CLASS_MOUSE, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NEG, ITEM_ID_ZAXIS)

#define MOUSECODE_X_POS_SWITCH MOUSECODE_X_POS_SWITCH_INDEXED(0)
#define MOUSECODE_X_NEG_SWITCH MOUSECODE_X_NEG_SWITCH_INDEXED(0)
#define MOUSECODE_Y_POS_SWITCH MOUSECODE_Y_POS_SWITCH_INDEXED(0)
#define MOUSECODE_Y_NEG_SWITCH MOUSECODE_Y_NEG_SWITCH_INDEXED(0)
#define MOUSECODE_Z_POS_SWITCH MOUSECODE_Z_POS_SWITCH_INDEXED(0)
#define MOUSECODE_Z_NEG_SWITCH MOUSECODE_Z_NEG_SWITCH_INDEXED(0)

// mouse buttons
#define MOUSECODE_BUTTON1_INDEXED(n) input_code(DEVICE_CLASS_MOUSE, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_BUTTON1)
#define MOUSECODE_BUTTON2_INDEXED(n) input_code(DEVICE_CLASS_MOUSE, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_BUTTON2)
#define MOUSECODE_BUTTON3_INDEXED(n) input_code(DEVICE_CLASS_MOUSE, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_BUTTON3)
#define MOUSECODE_BUTTON4_INDEXED(n) input_code(DEVICE_CLASS_MOUSE, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_BUTTON4)
#define MOUSECODE_BUTTON5_INDEXED(n) input_code(DEVICE_CLASS_MOUSE, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_BUTTON5)
#define MOUSECODE_BUTTON6_INDEXED(n) input_code(DEVICE_CLASS_MOUSE, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_BUTTON6)
#define MOUSECODE_BUTTON7_INDEXED(n) input_code(DEVICE_CLASS_MOUSE, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_BUTTON7)
#define MOUSECODE_BUTTON8_INDEXED(n) input_code(DEVICE_CLASS_MOUSE, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_BUTTON8)

#define MOUSECODE_BUTTON1 MOUSECODE_BUTTON1_INDEXED(0)
#define MOUSECODE_BUTTON2 MOUSECODE_BUTTON2_INDEXED(0)
#define MOUSECODE_BUTTON3 MOUSECODE_BUTTON3_INDEXED(0)
#define MOUSECODE_BUTTON4 MOUSECODE_BUTTON4_INDEXED(0)
#define MOUSECODE_BUTTON5 MOUSECODE_BUTTON5_INDEXED(0)
#define MOUSECODE_BUTTON6 MOUSECODE_BUTTON6_INDEXED(0)
#define MOUSECODE_BUTTON7 MOUSECODE_BUTTON7_INDEXED(0)
#define MOUSECODE_BUTTON8 MOUSECODE_BUTTON8_INDEXED(0)

// gun axes as absolute devices
#define GUNCODE_X_INDEXED(n) input_code(DEVICE_CLASS_LIGHTGUN, n, ITEM_CLASS_ABSOLUTE, ITEM_MODIFIER_NONE, ITEM_ID_XAXIS)
#define GUNCODE_Y_INDEXED(n) input_code(DEVICE_CLASS_LIGHTGUN, n, ITEM_CLASS_ABSOLUTE, ITEM_MODIFIER_NONE, ITEM_ID_YAXIS)

#define GUNCODE_X GUNCODE_X_INDEXED(0)
#define GUNCODE_Y GUNCODE_Y_INDEXED(0)

// gun buttons
#define GUNCODE_BUTTON1_INDEXED(n) input_code(DEVICE_CLASS_LIGHTGUN, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_BUTTON1)
#define GUNCODE_BUTTON2_INDEXED(n) input_code(DEVICE_CLASS_LIGHTGUN, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_BUTTON2)
#define GUNCODE_BUTTON3_INDEXED(n) input_code(DEVICE_CLASS_LIGHTGUN, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_BUTTON3)
#define GUNCODE_BUTTON4_INDEXED(n) input_code(DEVICE_CLASS_LIGHTGUN, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_BUTTON4)
#define GUNCODE_BUTTON5_INDEXED(n) input_code(DEVICE_CLASS_LIGHTGUN, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_BUTTON5)
#define GUNCODE_BUTTON6_INDEXED(n) input_code(DEVICE_CLASS_LIGHTGUN, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_BUTTON6)
#define GUNCODE_BUTTON7_INDEXED(n) input_code(DEVICE_CLASS_LIGHTGUN, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_BUTTON7)
#define GUNCODE_BUTTON8_INDEXED(n) input_code(DEVICE_CLASS_LIGHTGUN, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_BUTTON8)

#define GUNCODE_BUTTON1 GUNCODE_BUTTON1_INDEXED(0)
#define GUNCODE_BUTTON2 GUNCODE_BUTTON2_INDEXED(0)
#define GUNCODE_BUTTON3 GUNCODE_BUTTON3_INDEXED(0)
#define GUNCODE_BUTTON4 GUNCODE_BUTTON4_INDEXED(0)
#define GUNCODE_BUTTON5 GUNCODE_BUTTON5_INDEXED(0)
#define GUNCODE_BUTTON6 GUNCODE_BUTTON6_INDEXED(0)
#define GUNCODE_BUTTON7 GUNCODE_BUTTON7_INDEXED(0)
#define GUNCODE_BUTTON8 GUNCODE_BUTTON8_INDEXED(0)

// joystick axes as absolute devices
#define JOYCODE_X_INDEXED(n) input_code(DEVICE_CLASS_JOYSTICK, n, ITEM_CLASS_ABSOLUTE, ITEM_MODIFIER_NONE, ITEM_ID_XAXIS)
#define JOYCODE_Y_INDEXED(n) input_code(DEVICE_CLASS_JOYSTICK, n, ITEM_CLASS_ABSOLUTE, ITEM_MODIFIER_NONE, ITEM_ID_YAXIS)
#define JOYCODE_Z_INDEXED(n) input_code(DEVICE_CLASS_JOYSTICK, n, ITEM_CLASS_ABSOLUTE, ITEM_MODIFIER_NONE, ITEM_ID_ZAXIS)
#define JOYCODE_U_INDEXED(n) input_code(DEVICE_CLASS_JOYSTICK, n, ITEM_CLASS_ABSOLUTE, ITEM_MODIFIER_NONE, ITEM_ID_RXAXIS)
#define JOYCODE_V_INDEXED(n) input_code(DEVICE_CLASS_JOYSTICK, n, ITEM_CLASS_ABSOLUTE, ITEM_MODIFIER_NONE, ITEM_ID_RYAXIS)

#define JOYCODE_X JOYCODE_X_INDEXED(0)
#define JOYCODE_Y JOYCODE_Y_INDEXED(0)
#define JOYCODE_Z JOYCODE_Z_INDEXED(0)
#define JOYCODE_U JOYCODE_U_INDEXED(0)
#define JOYCODE_V JOYCODE_V_INDEXED(0)

// joystick axes as absolute half-axes
#define JOYCODE_X_POS_ABSOLUTE_INDEXED(n) input_code(DEVICE_CLASS_JOYSTICK, n, ITEM_CLASS_ABSOLUTE, ITEM_MODIFIER_POS, ITEM_ID_XAXIS)
#define JOYCODE_X_NEG_ABSOLUTE_INDEXED(n) input_code(DEVICE_CLASS_JOYSTICK, n, ITEM_CLASS_ABSOLUTE, ITEM_MODIFIER_NEG, ITEM_ID_XAXIS)
#define JOYCODE_Y_POS_ABSOLUTE_INDEXED(n) input_code(DEVICE_CLASS_JOYSTICK, n, ITEM_CLASS_ABSOLUTE, ITEM_MODIFIER_POS, ITEM_ID_YAXIS)
#define JOYCODE_Y_NEG_ABSOLUTE_INDEXED(n) input_code(DEVICE_CLASS_JOYSTICK, n, ITEM_CLASS_ABSOLUTE, ITEM_MODIFIER_NEG, ITEM_ID_YAXIS)
#define JOYCODE_Z_POS_ABSOLUTE_INDEXED(n) input_code(DEVICE_CLASS_JOYSTICK, n, ITEM_CLASS_ABSOLUTE, ITEM_MODIFIER_POS, ITEM_ID_ZAXIS)
#define JOYCODE_Z_NEG_ABSOLUTE_INDEXED(n) input_code(DEVICE_CLASS_JOYSTICK, n, ITEM_CLASS_ABSOLUTE, ITEM_MODIFIER_NEG, ITEM_ID_ZAXIS)
#define JOYCODE_U_POS_ABSOLUTE_INDEXED(n) input_code(DEVICE_CLASS_JOYSTICK, n, ITEM_CLASS_ABSOLUTE, ITEM_MODIFIER_POS, ITEM_ID_RXAXIS)
#define JOYCODE_U_NEG_ABSOLUTE_INDEXED(n) input_code(DEVICE_CLASS_JOYSTICK, n, ITEM_CLASS_ABSOLUTE, ITEM_MODIFIER_NEG, ITEM_ID_RXAXIS)
#define JOYCODE_V_POS_ABSOLUTE_INDEXED(n) input_code(DEVICE_CLASS_JOYSTICK, n, ITEM_CLASS_ABSOLUTE, ITEM_MODIFIER_POS, ITEM_ID_RYAXIS)
#define JOYCODE_V_NEG_ABSOLUTE_INDEXED(n) input_code(DEVICE_CLASS_JOYSTICK, n, ITEM_CLASS_ABSOLUTE, ITEM_MODIFIER_NEG, ITEM_ID_RYAXIS)

#define JOYCODE_X_POS_ABSOLUTE JOYCODE_X_POS_ABSOLUTE_INDEXED(0)
#define JOYCODE_X_NEG_ABSOLUTE JOYCODE_X_NEG_ABSOLUTE_INDEXED(0)
#define JOYCODE_Y_POS_ABSOLUTE JOYCODE_Y_POS_ABSOLUTE_INDEXED(0)
#define JOYCODE_Y_NEG_ABSOLUTE JOYCODE_Y_NEG_ABSOLUTE_INDEXED(0)
#define JOYCODE_Z_POS_ABSOLUTE JOYCODE_Z_POS_ABSOLUTE_INDEXED(0)
#define JOYCODE_Z_NEG_ABSOLUTE JOYCODE_Z_NEG_ABSOLUTE_INDEXED(0)
#define JOYCODE_U_POS_ABSOLUTE JOYCODE_U_POS_ABSOLUTE_INDEXED(0)
#define JOYCODE_U_NEG_ABSOLUTE JOYCODE_U_NEG_ABSOLUTE_INDEXED(0)
#define JOYCODE_V_POS_ABSOLUTE JOYCODE_V_POS_ABSOLUTE_INDEXED(0)
#define JOYCODE_V_NEG_ABSOLUTE JOYCODE_V_NEG_ABSOLUTE_INDEXED(0)

// joystick axes as switches; X/Y are specially handled for left/right/up/down mapping
#define JOYCODE_X_LEFT_SWITCH_INDEXED(n) input_code(DEVICE_CLASS_JOYSTICK, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_LEFT, ITEM_ID_XAXIS)
#define JOYCODE_X_RIGHT_SWITCH_INDEXED(n) input_code(DEVICE_CLASS_JOYSTICK, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_RIGHT, ITEM_ID_XAXIS)
#define JOYCODE_Y_UP_SWITCH_INDEXED(n) input_code(DEVICE_CLASS_JOYSTICK, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_UP, ITEM_ID_YAXIS)
#define JOYCODE_Y_DOWN_SWITCH_INDEXED(n) input_code(DEVICE_CLASS_JOYSTICK, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_DOWN, ITEM_ID_YAXIS)
#define JOYCODE_Z_POS_SWITCH_INDEXED(n) input_code(DEVICE_CLASS_JOYSTICK, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_POS, ITEM_ID_ZAXIS)
#define JOYCODE_Z_NEG_SWITCH_INDEXED(n) input_code(DEVICE_CLASS_JOYSTICK, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NEG, ITEM_ID_ZAXIS)
#define JOYCODE_U_POS_SWITCH_INDEXED(n) input_code(DEVICE_CLASS_JOYSTICK, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_POS, ITEM_ID_RXAXIS)
#define JOYCODE_U_NEG_SWITCH_INDEXED(n) input_code(DEVICE_CLASS_JOYSTICK, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NEG, ITEM_ID_RXAXIS)
#define JOYCODE_V_POS_SWITCH_INDEXED(n) input_code(DEVICE_CLASS_JOYSTICK, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_POS, ITEM_ID_RYAXIS)
#define JOYCODE_V_NEG_SWITCH_INDEXED(n) input_code(DEVICE_CLASS_JOYSTICK, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NEG, ITEM_ID_RYAXIS)

#define JOYCODE_X_LEFT_SWITCH JOYCODE_X_LEFT_SWITCH_INDEXED(0)
#define JOYCODE_X_RIGHT_SWITCH JOYCODE_X_RIGHT_SWITCH_INDEXED(0)
#define JOYCODE_Y_UP_SWITCH JOYCODE_Y_UP_SWITCH_INDEXED(0)
#define JOYCODE_Y_DOWN_SWITCH JOYCODE_Y_DOWN_SWITCH_INDEXED(0)
#define JOYCODE_Z_POS_SWITCH JOYCODE_Z_POS_SWITCH_INDEXED(0)
#define JOYCODE_Z_NEG_SWITCH JOYCODE_Z_NEG_SWITCH_INDEXED(0)
#define JOYCODE_U_POS_SWITCH JOYCODE_U_POS_SWITCH_INDEXED(0)
#define JOYCODE_U_NEG_SWITCH JOYCODE_U_NEG_SWITCH_INDEXED(0)
#define JOYCODE_V_POS_SWITCH JOYCODE_V_POS_SWITCH_INDEXED(0)
#define JOYCODE_V_NEG_SWITCH JOYCODE_V_NEG_SWITCH_INDEXED(0)

// joystick buttons
#define JOYCODE_BUTTON1_INDEXED(n) input_code(DEVICE_CLASS_JOYSTICK, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_BUTTON1)
#define JOYCODE_BUTTON2_INDEXED(n) input_code(DEVICE_CLASS_JOYSTICK, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_BUTTON2)
#define JOYCODE_BUTTON3_INDEXED(n) input_code(DEVICE_CLASS_JOYSTICK, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_BUTTON3)
#define JOYCODE_BUTTON4_INDEXED(n) input_code(DEVICE_CLASS_JOYSTICK, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_BUTTON4)
#define JOYCODE_BUTTON5_INDEXED(n) input_code(DEVICE_CLASS_JOYSTICK, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_BUTTON5)
#define JOYCODE_BUTTON6_INDEXED(n) input_code(DEVICE_CLASS_JOYSTICK, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_BUTTON6)
#define JOYCODE_BUTTON7_INDEXED(n) input_code(DEVICE_CLASS_JOYSTICK, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_BUTTON7)
#define JOYCODE_BUTTON8_INDEXED(n) input_code(DEVICE_CLASS_JOYSTICK, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_BUTTON8)
#define JOYCODE_BUTTON9_INDEXED(n) input_code(DEVICE_CLASS_JOYSTICK, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_BUTTON9)
#define JOYCODE_BUTTON10_INDEXED(n) input_code(DEVICE_CLASS_JOYSTICK, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_BUTTON10)
#define JOYCODE_BUTTON11_INDEXED(n) input_code(DEVICE_CLASS_JOYSTICK, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_BUTTON11)
#define JOYCODE_BUTTON12_INDEXED(n) input_code(DEVICE_CLASS_JOYSTICK, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_BUTTON12)
#define JOYCODE_BUTTON13_INDEXED(n) input_code(DEVICE_CLASS_JOYSTICK, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_BUTTON13)
#define JOYCODE_BUTTON14_INDEXED(n) input_code(DEVICE_CLASS_JOYSTICK, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_BUTTON14)
#define JOYCODE_BUTTON15_INDEXED(n) input_code(DEVICE_CLASS_JOYSTICK, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_BUTTON15)
#define JOYCODE_BUTTON16_INDEXED(n) input_code(DEVICE_CLASS_JOYSTICK, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_BUTTON16)
#define JOYCODE_BUTTON17_INDEXED(n) input_code(DEVICE_CLASS_JOYSTICK, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_BUTTON17)
#define JOYCODE_BUTTON18_INDEXED(n) input_code(DEVICE_CLASS_JOYSTICK, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_BUTTON18)
#define JOYCODE_BUTTON19_INDEXED(n) input_code(DEVICE_CLASS_JOYSTICK, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_BUTTON19)
#define JOYCODE_BUTTON20_INDEXED(n) input_code(DEVICE_CLASS_JOYSTICK, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_BUTTON20)
#define JOYCODE_BUTTON21_INDEXED(n) input_code(DEVICE_CLASS_JOYSTICK, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_BUTTON21)
#define JOYCODE_BUTTON22_INDEXED(n) input_code(DEVICE_CLASS_JOYSTICK, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_BUTTON22)
#define JOYCODE_BUTTON23_INDEXED(n) input_code(DEVICE_CLASS_JOYSTICK, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_BUTTON23)
#define JOYCODE_BUTTON24_INDEXED(n) input_code(DEVICE_CLASS_JOYSTICK, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_BUTTON24)
#define JOYCODE_BUTTON25_INDEXED(n) input_code(DEVICE_CLASS_JOYSTICK, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_BUTTON25)
#define JOYCODE_BUTTON26_INDEXED(n) input_code(DEVICE_CLASS_JOYSTICK, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_BUTTON26)
#define JOYCODE_BUTTON27_INDEXED(n) input_code(DEVICE_CLASS_JOYSTICK, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_BUTTON27)
#define JOYCODE_BUTTON28_INDEXED(n) input_code(DEVICE_CLASS_JOYSTICK, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_BUTTON28)
#define JOYCODE_BUTTON29_INDEXED(n) input_code(DEVICE_CLASS_JOYSTICK, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_BUTTON29)
#define JOYCODE_BUTTON30_INDEXED(n) input_code(DEVICE_CLASS_JOYSTICK, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_BUTTON30)
#define JOYCODE_BUTTON31_INDEXED(n) input_code(DEVICE_CLASS_JOYSTICK, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_BUTTON31)
#define JOYCODE_BUTTON32_INDEXED(n) input_code(DEVICE_CLASS_JOYSTICK, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_BUTTON32)
#define JOYCODE_START_INDEXED(n) input_code(DEVICE_CLASS_JOYSTICK, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_START)
#define JOYCODE_SELECT_INDEXED(n) input_code(DEVICE_CLASS_JOYSTICK, n, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_SELECT)

#define JOYCODE_BUTTON1 JOYCODE_BUTTON1_INDEXED(0)
#define JOYCODE_BUTTON2 JOYCODE_BUTTON2_INDEXED(0)
#define JOYCODE_BUTTON3 JOYCODE_BUTTON3_INDEXED(0)
#define JOYCODE_BUTTON4 JOYCODE_BUTTON4_INDEXED(0)
#define JOYCODE_BUTTON5 JOYCODE_BUTTON5_INDEXED(0)
#define JOYCODE_BUTTON6 JOYCODE_BUTTON6_INDEXED(0)
#define JOYCODE_BUTTON7 JOYCODE_BUTTON7_INDEXED(0)
#define JOYCODE_BUTTON8 JOYCODE_BUTTON8_INDEXED(0)
#define JOYCODE_BUTTON9 JOYCODE_BUTTON9_INDEXED(0)
#define JOYCODE_BUTTON10 JOYCODE_BUTTON10_INDEXED(0)
#define JOYCODE_BUTTON11 JOYCODE_BUTTON11_INDEXED(0)
#define JOYCODE_BUTTON12 JOYCODE_BUTTON12_INDEXED(0)
#define JOYCODE_BUTTON13 JOYCODE_BUTTON13_INDEXED(0)
#define JOYCODE_BUTTON14 JOYCODE_BUTTON14_INDEXED(0)
#define JOYCODE_BUTTON15 JOYCODE_BUTTON15_INDEXED(0)
#define JOYCODE_BUTTON16 JOYCODE_BUTTON16_INDEXED(0)
#define JOYCODE_BUTTON17 JOYCODE_BUTTON17_INDEXED(0)
#define JOYCODE_BUTTON18 JOYCODE_BUTTON18_INDEXED(0)
#define JOYCODE_BUTTON19 JOYCODE_BUTTON19_INDEXED(0)
#define JOYCODE_BUTTON20 JOYCODE_BUTTON20_INDEXED(0)
#define JOYCODE_BUTTON21 JOYCODE_BUTTON21_INDEXED(0)
#define JOYCODE_BUTTON22 JOYCODE_BUTTON22_INDEXED(0)
#define JOYCODE_BUTTON23 JOYCODE_BUTTON23_INDEXED(0)
#define JOYCODE_BUTTON24 JOYCODE_BUTTON24_INDEXED(0)
#define JOYCODE_BUTTON25 JOYCODE_BUTTON25_INDEXED(0)
#define JOYCODE_BUTTON26 JOYCODE_BUTTON26_INDEXED(0)
#define JOYCODE_BUTTON27 JOYCODE_BUTTON27_INDEXED(0)
#define JOYCODE_BUTTON28 JOYCODE_BUTTON28_INDEXED(0)
#define JOYCODE_BUTTON29 JOYCODE_BUTTON29_INDEXED(0)
#define JOYCODE_BUTTON30 JOYCODE_BUTTON30_INDEXED(0)
#define JOYCODE_BUTTON31 JOYCODE_BUTTON31_INDEXED(0)
#define JOYCODE_BUTTON32 JOYCODE_BUTTON32_INDEXED(0)
#define JOYCODE_START JOYCODE_START_INDEXED(0)
#define JOYCODE_SELECT JOYCODE_SELECT_INDEXED(0)



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// joystick maps
extern const char joystick_map_8way[];
extern const char joystick_map_4way_diagonal[];


//**************************************************************************
//  INLINE FUNCTIONS
//**************************************************************************

// input_device_item helpers
inline input_manager &input_device_item::manager() const { return m_device.manager(); }
inline running_machine &input_device_item::machine() const { return m_device.machine(); }
inline  INT32 input_device_item::update_value() { return m_current = (*m_getstate)(m_device.internal(), m_internal); }

// input_device helpers
inline input_manager &input_device::manager() const { return m_class.manager(); }
inline running_machine &input_device::machine() const { return m_class.machine(); }
inline input_device_class input_device::devclass() const { return m_class.devclass(); }

// input_class helpers
inline running_machine &input_class::machine() const { return m_manager.machine(); }



#endif  // __INPUT_H__
