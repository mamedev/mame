/***************************************************************************

  AY3600.c

  GI AY-3600 keyboard controller device

  Rewritten 2014 by R. Belmont, thanks to earlier efforts by
  Barry Nelson and Nathan Woods.
 
***************************************************************************/

#include "emu.h"
#include "machine/ay3600.h"

/***************************************************************************
    PARAMETERS
***************************************************************************/

#ifdef MAME_DEBUG
#define VERBOSE 1
#else
#define VERBOSE 0
#endif /* MAME_DEBUG */

#define LOG(x) do { if (VERBOSE) logerror x; } while (0)

/**************************************************************************/

// bit order in the input port of the special keys
#define SPECIALKEY_CAPSLOCK     0x01
#define SPECIALKEY_SHIFTL       0x04
#define SPECIALKEY_SHIFTR       0x08
#define SPECIALKEY_CONTROL      0x08
#define SPECIALKEY_ALTL         0x10
#define SPECIALKEY_ALTR         0x20
#define SPECIALKEY_WIN          0x40
#define SPECIALKEY_RESET        0x80

static const unsigned char ay3600_key_remap_2[9*8][4] =
{
/*        norm ctrl shft both */
		{ 0x7f,0x7f,0x7f,0x7f },    /* UNUSED       */
		{ 0x08,0x08,0x08,0x08 },    /* Left         */
		{ 0x09,0x09,0x09,0x09 },    /* UNUSED       */
		{ 0x0a,0x0a,0x0a,0x0a },    /* UNUSED       */
		{ 0x0b,0x0b,0x0b,0x0b },    /* UNUSED       */
		{ 0x0d,0x0d,0x0d,0x0d },    /* Enter        */
		{ 0x15,0x15,0x15,0x15 },    /* Right        */
		{ 0x1b,0x1b,0x1b,0x1b },    /* Escape       */
		{ 0x20,0x20,0x20,0x20 },    /* Space        */
		{ 0x27,0x27,0x22,0x22 },    /* UNUSED       */
		{ 0x2c,0x2c,0x3c,0x3c },    /* , <          */
		{ 0x3a,0x3a,0x2a,0x2a },    /* : *          */
		{ 0x2e,0x2e,0x3e,0x3e },    /* . >          */
		{ 0x2f,0x2f,0x3f,0x3f },    /* / ?          */
		{ 0x30,0x30,0x30,0x30 },    /* 0            */
		{ 0x31,0x31,0x21,0x21 },    /* 1 !          */
		{ 0x32,0x32,0x22,0x22 },    /* 2 "          */
		{ 0x33,0x33,0x23,0x23 },    /* 3 #          */
		{ 0x34,0x34,0x24,0x24 },    /* 4 $          */
		{ 0x35,0x35,0x25,0x25 },    /* 5 %          */
		{ 0x36,0x36,0x26,0x26 },    /* 6 &          */
		{ 0x37,0x37,0x27,0x27 },    /* 7 '          */
		{ 0x38,0x38,0x28,0x28 },    /* 8 (          */
		{ 0x39,0x39,0x29,0x29 },    /* 9 )          */
		{ 0x3b,0x3b,0x2b,0x2b },    /* ; +          */
		{ 0x2d,0x2d,0x3d,0x3d },    /* - =          */
		{ 0x5b,0x1b,0x7b,0x1b },    /* UNUSED       */
		{ 0x5c,0x1c,0x7c,0x1c },    /* UNUSED       */
		{ 0x5d,0x1d,0x7d,0x1d },    /* UNUSED       */
		{ 0x60,0x60,0x7e,0x7e },    /* UNUSED       */
		{ 0x41,0x01,0x41,0x01 },    /* A            */
		{ 0x42,0x02,0x42,0x02 },    /* B            */
		{ 0x43,0x03,0x43,0x03 },    /* C            */
		{ 0x44,0x04,0x44,0x04 },    /* D            */
		{ 0x45,0x05,0x45,0x05 },    /* E            */
		{ 0x46,0x06,0x46,0x06 },    /* F            */
		{ 0x47,0x07,0x47,0x07 },    /* G            */
		{ 0x48,0x08,0x48,0x08 },    /* H            */
		{ 0x49,0x09,0x49,0x09 },    /* I            */
		{ 0x4a,0x0a,0x4a,0x0a },    /* J            */
		{ 0x4b,0x0b,0x4b,0x0b },    /* K            */
		{ 0x4c,0x0c,0x4c,0x0c },    /* L            */
		{ 0x4d,0x0d,0x5d,0x0d },    /* M ]          */
		{ 0x4e,0x0e,0x5e,0x1e },    /* N ^          */
		{ 0x4f,0x0f,0x4f,0x0f },    /* O            */
		{ 0x50,0x10,0x40,0x00 },    /* P @          */
		{ 0x51,0x11,0x51,0x11 },    /* Q            */
		{ 0x52,0x12,0x52,0x12 },    /* R            */
		{ 0x53,0x13,0x53,0x13 },    /* S            */
		{ 0x54,0x14,0x54,0x14 },    /* T            */
		{ 0x55,0x15,0x55,0x15 },    /* U            */
		{ 0x56,0x16,0x56,0x16 },    /* V            */
		{ 0x57,0x17,0x57,0x17 },    /* W            */
		{ 0x58,0x18,0x58,0x18 },    /* X            */
		{ 0x59,0x19,0x59,0x19 },    /* Y            */
		{ 0x5a,0x1a,0x5a,0x1a }     /* Z            */
};

static const unsigned char ay3600_key_remap_2e[2][9*8][4] =
{
/* caps lock off  norm ctrl shft both */
	{
		{ 0x7f,0x7f,0x7f,0x7f },    /* Backspace    */
		{ 0x08,0x08,0x08,0x08 },    /* Left         */
		{ 0x09,0x09,0x09,0x09 },    /* Tab          */
		{ 0x0a,0x0a,0x0a,0x0a },    /* Down         */
		{ 0x0b,0x0b,0x0b,0x0b },    /* Up           */
		{ 0x0d,0x0d,0x0d,0x0d },    /* Enter        */
		{ 0x15,0x15,0x15,0x15 },    /* Right        */
		{ 0x1b,0x1b,0x1b,0x1b },    /* Escape       */
		{ 0x20,0x20,0x20,0x20 },    /* Space        */
		{ 0x27,0x27,0x22,0x22 },    /* ' "          */
		{ 0x2c,0x2c,0x3c,0x3c },    /* , <          */
		{ 0x2d,0x2d,0x5f,0x1f },    /* - _          */
		{ 0x2e,0x2e,0x3e,0x3e },    /* . >          */
		{ 0x2f,0x2f,0x3f,0x3f },    /* / ?          */
		{ 0x30,0x30,0x29,0x29 },    /* 0 )          */
		{ 0x31,0x31,0x21,0x21 },    /* 1 !          */
		{ 0x32,0x00,0x40,0x00 },    /* 2 @          */
		{ 0x33,0x33,0x23,0x23 },    /* 3 #          */
		{ 0x34,0x34,0x24,0x24 },    /* 4 $          */
		{ 0x35,0x35,0x25,0x25 },    /* 5 %          */
		{ 0x36,0x1e,0x5e,0x1e },    /* 6 ^          */
		{ 0x37,0x37,0x26,0x26 },    /* 7 &          */
		{ 0x38,0x38,0x2a,0x2a },    /* 8 *          */
		{ 0x39,0x39,0x28,0x28 },    /* 9 (          */
		{ 0x3b,0x3b,0x3a,0x3a },    /* ; :          */
		{ 0x3d,0x3d,0x2b,0x2b },    /* = +          */
		{ 0x5b,0x1b,0x7b,0x1b },    /* [ {          */
		{ 0x5c,0x1c,0x7c,0x1c },    /* \ |          */
		{ 0x5d,0x1d,0x7d,0x1d },    /* ] }          */
		{ 0x60,0x60,0x7e,0x7e },    /* ` ~          */
		{ 0x61,0x01,0x41,0x01 },    /* a A          */
		{ 0x62,0x02,0x42,0x02 },    /* b B          */
		{ 0x63,0x03,0x43,0x03 },    /* c C          */
		{ 0x64,0x04,0x44,0x04 },    /* d D          */
		{ 0x65,0x05,0x45,0x05 },    /* e E          */
		{ 0x66,0x06,0x46,0x06 },    /* f F          */
		{ 0x67,0x07,0x47,0x07 },    /* g G          */
		{ 0x68,0x08,0x48,0x08 },    /* h H          */
		{ 0x69,0x09,0x49,0x09 },    /* i I          */
		{ 0x6a,0x0a,0x4a,0x0a },    /* j J          */
		{ 0x6b,0x0b,0x4b,0x0b },    /* k K          */
		{ 0x6c,0x0c,0x4c,0x0c },    /* l L          */
		{ 0x6d,0x0d,0x4d,0x0d },    /* m M          */
		{ 0x6e,0x0e,0x4e,0x0e },    /* n N          */
		{ 0x6f,0x0f,0x4f,0x0f },    /* o O          */
		{ 0x70,0x10,0x50,0x10 },    /* p P          */
		{ 0x71,0x11,0x51,0x11 },    /* q Q          */
		{ 0x72,0x12,0x52,0x12 },    /* r R          */
		{ 0x73,0x13,0x53,0x13 },    /* s S          */
		{ 0x74,0x14,0x54,0x14 },    /* t T          */
		{ 0x75,0x15,0x55,0x15 },    /* u U          */
		{ 0x76,0x16,0x56,0x16 },    /* v V          */
		{ 0x77,0x17,0x57,0x17 },    /* w W          */
		{ 0x78,0x18,0x58,0x18 },    /* x X          */
		{ 0x79,0x19,0x59,0x19 },    /* y Y          */
		{ 0x7a,0x1a,0x5a,0x1a },    /* z Z          */
		{ 0x30,0x30,0x30,0x30 },    /* 0 (KP)       */
		{ 0x31,0x31,0x31,0x31 },    /* 1 (KP)       */
		{ 0x32,0x32,0x32,0x32 },    /* 2 (KP)       */
		{ 0x33,0x33,0x33,0x33 },    /* 3 (KP)       */
		{ 0x34,0x34,0x34,0x34 },    /* 4 (KP)       */
		{ 0x35,0x35,0x35,0x35 },    /* 5 (KP)       */
		{ 0x36,0x36,0x36,0x36 },    /* 6 (KP)       */
		{ 0x37,0x37,0x37,0x37 },    /* 7 (KP)       */
		{ 0x38,0x38,0x38,0x38 },    /* 8 (KP)       */
		{ 0x39,0x39,0x39,0x39 },    /* 9 (KP)       */
		{ 0x2f,0x2f,0x2f,0x2f },    /* / (KP)       */
		{ 0x2a,0x2a,0x2a,0x2a },    /* * (KP)       */
		{ 0x2d,0x2d,0x2d,0x2d },    /* - (KP)       */
		{ 0x2b,0x2b,0x2b,0x2b },    /* + (KP)       */
		{ 0x2e,0x2e,0x2e,0x2e },    /* . (KP)       */
		{ 0x0d,0x0d,0x0d,0x0d }     /* Enter (KP)   */
	},
/* caps lock on   norm ctrl shft both */
	{
		{ 0x7f,0x7f,0x7f,0x7f },    /* Backspace    */
		{ 0x08,0x08,0x08,0x08 },    /* Left         */
		{ 0x09,0x09,0x09,0x09 },    /* Tab          */
		{ 0x0a,0x0a,0x0a,0x0a },    /* Down         */
		{ 0x0b,0x0b,0x0b,0x0b },    /* Up           */
		{ 0x0d,0x0d,0x0d,0x0d },    /* Enter        */
		{ 0x15,0x15,0x15,0x15 },    /* Right        */
		{ 0x1b,0x1b,0x1b,0x1b },    /* Escape       */
		{ 0x20,0x20,0x20,0x20 },    /* Space        */
		{ 0x27,0x27,0x22,0x22 },    /* ' "          */
		{ 0x2c,0x2c,0x3c,0x3c },    /* , <          */
		{ 0x2d,0x2d,0x5f,0x1f },    /* - _          */
		{ 0x2e,0x2e,0x3e,0x3e },    /* . >          */
		{ 0x2f,0x2f,0x3f,0x3f },    /* / ?          */
		{ 0x30,0x30,0x29,0x29 },    /* 0 )          */
		{ 0x31,0x31,0x21,0x21 },    /* 1 !          */
		{ 0x32,0x00,0x40,0x00 },    /* 2 @          */
		{ 0x33,0x33,0x23,0x23 },    /* 3 #          */
		{ 0x34,0x34,0x24,0x24 },    /* 4 $          */
		{ 0x35,0x35,0x25,0x25 },    /* 5 %          */
		{ 0x36,0x1e,0x5e,0x1e },    /* 6 ^          */
		{ 0x37,0x37,0x26,0x26 },    /* 7 &          */
		{ 0x38,0x38,0x2a,0x2a },    /* 8 *          */
		{ 0x39,0x39,0x28,0x28 },    /* 9 (          */
		{ 0x3b,0x3b,0x3a,0x3a },    /* ; :          */
		{ 0x3d,0x3d,0x2b,0x2b },    /* = +          */
		{ 0x5b,0x1b,0x7b,0x1b },    /* [ {          */
		{ 0x5c,0x1c,0x7c,0x1c },    /* \ |          */
		{ 0x5d,0x1d,0x7d,0x1d },    /* ] }          */
		{ 0x60,0x60,0x7e,0x7e },    /* ` ~          */
		{ 0x41,0x01,0x61,0x01 },    /* A a          */
		{ 0x42,0x02,0x62,0x02 },    /* B b          */
		{ 0x43,0x03,0x63,0x03 },    /* C c          */
		{ 0x44,0x04,0x64,0x04 },    /* D d          */
		{ 0x45,0x05,0x65,0x05 },    /* E e          */
		{ 0x46,0x06,0x66,0x06 },    /* F f          */
		{ 0x47,0x07,0x67,0x07 },    /* G g          */
		{ 0x48,0x08,0x68,0x08 },    /* H h          */
		{ 0x49,0x09,0x69,0x09 },    /* I i          */
		{ 0x4a,0x0a,0x6a,0x0a },    /* J j          */
		{ 0x4b,0x0b,0x6b,0x0b },    /* K k          */
		{ 0x4c,0x0c,0x6c,0x0c },    /* L l          */
		{ 0x4d,0x0d,0x6d,0x0d },    /* M m          */
		{ 0x4e,0x0e,0x6e,0x0e },    /* N n          */
		{ 0x4f,0x0f,0x6f,0x0f },    /* O o          */
		{ 0x50,0x10,0x70,0x10 },    /* P p          */
		{ 0x51,0x11,0x71,0x11 },    /* Q q          */
		{ 0x52,0x12,0x72,0x12 },    /* R r          */
		{ 0x53,0x13,0x73,0x13 },    /* S s          */
		{ 0x54,0x14,0x74,0x14 },    /* T t          */
		{ 0x55,0x15,0x75,0x15 },    /* U u          */
		{ 0x56,0x16,0x76,0x16 },    /* V v          */
		{ 0x57,0x17,0x77,0x17 },    /* W w          */
		{ 0x58,0x18,0x78,0x18 },    /* X x          */
		{ 0x59,0x19,0x79,0x19 },    /* Y y          */
		{ 0x5a,0x1a,0x7a,0x1a },    /* Z z          */
		{ 0x30,0x30,0x30,0x30 },    /* 0 (KP)       */
		{ 0x31,0x31,0x31,0x31 },    /* 1 (KP)       */
		{ 0x32,0x32,0x32,0x32 },    /* 2 (KP)       */
		{ 0x33,0x33,0x33,0x33 },    /* 3 (KP)       */
		{ 0x34,0x34,0x34,0x34 },    /* 4 (KP)       */
		{ 0x35,0x35,0x35,0x35 },    /* 5 (KP)       */
		{ 0x36,0x36,0x36,0x36 },    /* 6 (KP)       */
		{ 0x37,0x37,0x37,0x37 },    /* 7 (KP)       */
		{ 0x38,0x38,0x38,0x38 },    /* 8 (KP)       */
		{ 0x39,0x39,0x39,0x39 },    /* 9 (KP)       */
		{ 0x2f,0x2f,0x2f,0x2f },    /* / (KP)       */
		{ 0x2a,0x2a,0x2a,0x2a },    /* * (KP)       */
		{ 0x2d,0x2d,0x2d,0x2d },    /* - (KP)       */
		{ 0x2b,0x2b,0x2b,0x2b },    /* + (KP)       */
		{ 0x2e,0x2e,0x2e,0x2e },    /* . (KP)       */
		{ 0x0d,0x0d,0x0d,0x0d }     /* Enter (KP)   */
	}
};

#define AY3600_KEY_NORMAL               0
#define AY3600_KEY_CONTROL              1
#define AY3600_KEY_SHIFT                2
#define AY3600_KEY_BOTH                 3
#define MAGIC_KEY_REPEAT_NUMBER     80

#define AY3600_KEYS_LENGTH          128

const device_type AY3600N = &device_creator<ay3600n_device>;

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  ay3600n_device - constructor
//-------------------------------------------------

ay3600n_device::ay3600n_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, AY3600N, "AY-3600 keyboard controller", tag, owner, clock, "ay3600", __FILE__)
{
	m_keypad = m_repeat = false;
}

/***************************************************************************
  AY3600_init
***************************************************************************/

void ay3600n_device::device_start()
{
	/* Init the key remapping table */
	m_ay3600_keys = auto_alloc_array_clear(machine(), unsigned int, AY3600_KEYS_LENGTH);

	/* We poll the keyboard periodically to scan the keys.  This is
	actually consistent with how the AY-3600 keyboard controller works. */
	m_timer = timer_alloc(0, NULL);
	m_timer->adjust(attotime::never);

	machine().ioport().natkeyboard().configure(
		ioport_queue_chars_delegate(FUNC(ay3600n_device::AY3600_keyboard_queue_chars), this),
		ioport_accept_char_delegate(FUNC(ay3600n_device::AY3600_keyboard_accept_char), this),
		ioport_charqueue_empty_delegate(FUNC(ay3600n_device::AY3600_keyboard_charqueue_empty), this));

	save_item(NAME(m_keycode));
	save_item(NAME(m_keycode_unmodified));
	save_item(NAME(m_keywaiting));
	save_item(NAME(m_keystilldown));
	save_item(NAME(m_keymodreg));
	save_item(NAME(m_last_key));
	save_item(NAME(m_last_key_unmodified));
	save_item(NAME(m_time_until_repeat));
}

void ay3600n_device::device_reset()
{
	/* Set Caps Lock light to ON, since that's how we default it. */
	set_led_status(machine(), 1, 1);

	m_keywaiting = 0;
	m_keycode = 0;
	m_keystilldown = 0;
	m_keymodreg = AY3600_KEYMOD_CAPSLOCK;    // caps lock on

	m_last_key = 0xff;   /* necessary for special repeat key behaviour */
	m_last_key_unmodified = 0xff;    /* necessary for special repeat key behaviour */
	m_time_until_repeat = MAGIC_KEY_REPEAT_NUMBER;

	m_timer->adjust(attotime::from_hz(60), 0, attotime::from_hz(60));
}

void ay3600n_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	int switchkey;  /* Normal, Shift, Control, or both */
	int port, num_ports, bit, data;
	int any_key_pressed = 0;   /* Have we pressed a key? True/False */
	int caps_lock = 0;
	int curkey;
	int curkey_unmodified;
	static const char *const portnames[] = 
	{
		"keyb_0", "keyb_1", "keyb_2", "keyb_3", "keyb_4", "keyb_5", "keyb_6",
		"keypad_1", "keypad_2"
	};

	/* check for these special cases because they affect the emulated key codes */
	/* does this machine have a special repeat key? */
	if (m_repeat)
		m_time_until_repeat = machine().root_device().ioport("keyb_repeat")->read() & 0x01 ? 0 : ~0;

	int special_state = machine().root_device().ioport("keyb_special")->read();

	/* check caps lock and set LED here */
	if (special_state & SPECIALKEY_CAPSLOCK)
	{
		caps_lock = 1;
		set_led_status(machine(), 1, 1);
		m_keymodreg |= AY3600_KEYMOD_CAPSLOCK;
	}
	else
	{
		caps_lock = 0;
		set_led_status(machine(), 1, 0);
		m_keymodreg &= ~AY3600_KEYMOD_CAPSLOCK;
	}

	switchkey = AY3600_KEY_NORMAL;

	/* shift key check */
	if (special_state & (SPECIALKEY_SHIFTL | SPECIALKEY_SHIFTR))
	{
		switchkey |= AY3600_KEY_SHIFT;
		m_keymodreg |= AY3600_KEYMOD_SHIFT;
	}
	else
	{
		m_keymodreg &= ~AY3600_KEYMOD_SHIFT;
	}

	/* control key check - only one control key */
	if (special_state & SPECIALKEY_CONTROL)
	{
		switchkey |= AY3600_KEY_CONTROL;
		m_keymodreg |= AY3600_KEYMOD_CONTROL;
	}
	else
	{
		m_keymodreg &= ~AY3600_KEYMOD_CONTROL;
	}

	/* apple key check */
	if (special_state & SPECIALKEY_ALTL)
	{
		m_keymodreg |= AY3600_KEYMOD_COMMAND;
	}
	else
	{
		m_keymodreg &= ~AY3600_KEYMOD_COMMAND;
	}

	/* option key check */
	if (special_state & SPECIALKEY_ALTR)
	{
		m_keymodreg |= AY3600_KEYMOD_OPTION;
	}
	else
	{
		m_keymodreg &= ~AY3600_KEYMOD_OPTION;
	}

	/* run through real keys and see what's being pressed */
	num_ports = m_keypad ? 9 : 7;

	m_keymodreg &= ~AY3600_KEYMOD_KEYPAD;

	for (port = 0; port < num_ports; port++)
	{
		data = machine().root_device().ioport(portnames[port])->read();

		for (bit = 0; bit < 8; bit++)
		{
			if (!m_repeat)
			{
				curkey = ay3600_key_remap_2e[caps_lock][port*8+bit][switchkey];
				curkey_unmodified = ay3600_key_remap_2e[caps_lock][port*8+bit][0];
			}
			else
			{
				curkey = ay3600_key_remap_2[port*8+bit][switchkey];
				curkey_unmodified = ay3600_key_remap_2[port*8+bit][0];
			}
			if (data & (1 << bit))
			{
				any_key_pressed = 1;

				if (port == 8)
				{
					m_keymodreg |= AY3600_KEYMOD_KEYPAD;
				}

				/* prevent overflow */
				if (m_ay3600_keys[port*8+bit] < 65000)
					m_ay3600_keys[port*8+bit]++;

				/* on every key press, reset the time until repeat and the key to repeat */
				if ((m_ay3600_keys[port*8+bit] == 1) && (curkey_unmodified != m_last_key_unmodified))
				{
					m_time_until_repeat = MAGIC_KEY_REPEAT_NUMBER;
					m_last_key = curkey;
					m_last_key_unmodified = curkey_unmodified;
				}
			}
			else
			{
				m_ay3600_keys[port*8+bit] = 0;
			}
		}
	}

	m_keymodreg &= ~AY3600_KEYMOD_REPEAT;

	if (!any_key_pressed)
	{
		/* If no keys have been pressed, reset the repeat time and return */
		m_time_until_repeat = MAGIC_KEY_REPEAT_NUMBER;
		m_last_key = 0xff;
		m_last_key_unmodified = 0xff;
	}
	else
	{
		/* Otherwise, count down the repeat time */
		if (m_time_until_repeat > 0)
			m_time_until_repeat--;

		/* Even if a key has been released, repeat it if it was the last key pressed */
		/* If we should output a key, set the appropriate Apple II data lines */
		if (m_time_until_repeat == 0 ||
			m_time_until_repeat == MAGIC_KEY_REPEAT_NUMBER-1)
		{
			m_keywaiting = 1;
			m_keycode = m_last_key;
			m_keycode_unmodified = m_last_key_unmodified;
			m_keymodreg |= AY3600_KEYMOD_REPEAT;
		}
	}
	m_keystilldown = (m_last_key_unmodified == m_keycode_unmodified);
}



/***************************************************************************
  AY3600_keydata_strobe_r
***************************************************************************/

int ay3600n_device::keydata_strobe_r()
{
	int rc;
	rc = m_keycode | (m_keywaiting ? 0x80 : 0x00);
	LOG(("AY3600_keydata_strobe_r(): rc=0x%02x\n", rc));
	return rc;
}



/***************************************************************************
  AY3600_anykey_clearstrobe_r
***************************************************************************/

int ay3600n_device::anykey_clearstrobe_r()
{
	int rc;
	m_keywaiting = 0;
	rc = m_keycode | (m_keystilldown ? 0x80 : 0x00);
	LOG(("AY3600_anykey_clearstrobe_r(): rc=0x%02x\n", rc));
	return rc;
}


/***************************************************************************
  AY3600_keymod_r
***************************************************************************/

int ay3600n_device::keymod_r()
{
	return m_keymodreg;
}

/***************************************************************************
  Natural keyboard support
***************************************************************************/

UINT8 ay3600n_device::AY3600_get_keycode(unicode_char ch)
{
	UINT8 result;

	switch(ch)
	{
		case UCHAR_MAMEKEY(UP):
			result = 0x0B;
			break;
		case UCHAR_MAMEKEY(DOWN):
			result = 0x0A;
			break;
		case UCHAR_MAMEKEY(LEFT):
			result = 0x08;
			break;
		case UCHAR_MAMEKEY(RIGHT):
			result = 0x15;
			break;

		default:
			if (ch <= 0x7F)
				result = (UINT8) ch;
			else
				result = 0;
			break;
	}

	return result;
}



int ay3600n_device::AY3600_keyboard_queue_chars(const unicode_char *text, size_t text_len)
{
	if (m_keywaiting)
		return 0;
	m_keycode = AY3600_get_keycode(text[0]);
	m_keywaiting = 1;
	return 1;
}

bool ay3600n_device::AY3600_keyboard_accept_char(unicode_char ch)
{
	return AY3600_get_keycode(ch) != 0;
}

bool ay3600n_device::AY3600_keyboard_charqueue_empty()
{
	return true;
}

void ay3600n_device::set_runtime_config(bool keypad, bool repeat)
{
	m_keypad = keypad;
	m_repeat = repeat;
}

