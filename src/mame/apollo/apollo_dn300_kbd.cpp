// license:BSD-3-Clause
// copyright-holders:Hans Ostermeyer, Chris Toshok, Vladimir Vukicevic
/*
 * apollo_dn300_kbd.cpp - the Apollo DN300/DN320 keyboard and mouse
 *
 * The keyboard sends the codes of its compatibility mode (one or two bytes
 * per key press, no key releases or auto-repeat) and the mouse its
 * four-byte reports (0xdf, the buttons, dx, dy) over the 6850 line.
 * Derived from the DN3000's apollo_kbd.cpp.
 *
 *  see also:
 *  - http://www.bitsavers.org/pdf/apollo/008778-03_DOMAIN_Series_3000_4000_Technical_Reference_Aug87.pdf
 */

#include "emu.h"
#include "apollo_dn300_kbd.h"

#define LOG_KEYS  (1U << 1) // key presses and the codes sent
#define LOG_MOUSE (1U << 2) // mouse reports

#define VERBOSE (0)
#include "logmacro.h"

#define LOGKEYS(...)  LOGMASKED(LOG_KEYS, __VA_ARGS__)
#define LOGMOUSE(...) LOGMASKED(LOG_MOUSE, __VA_ARGS__)

//**************************************************************************
//  CONSTANTS
//**************************************************************************

#define NOP 0
#define No  0
#define Yes 1

namespace {

INPUT_PORTS_START( apollo_dn300_kbd )
	PORT_START( "keyboard1" )
	PORT_BIT( 0x00000001, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("~ `") PORT_CODE(KEYCODE_TILDE) /* ` ~ */
	PORT_BIT( 0x00000002, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("ESC") PORT_CODE(KEYCODE_ESC) /* ESC */
	PORT_BIT( 0x00000004, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("1 !") PORT_CODE(KEYCODE_1) /* 1 ! */
	PORT_BIT( 0x00000008, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("2 @") PORT_CODE(KEYCODE_2) /* 2 " */
	PORT_BIT( 0x00000010, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("3 #") PORT_CODE(KEYCODE_3) /* 3 # */
	PORT_BIT( 0x00000020, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("4 $") PORT_CODE(KEYCODE_4) /* 4 $ */
	PORT_BIT( 0x00000040, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("5 %") PORT_CODE(KEYCODE_5) /* 5 % */
	PORT_BIT( 0x00000080, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("6 &") PORT_CODE(KEYCODE_6) /* 6 & */
	PORT_BIT( 0x00000100, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("7 \'") PORT_CODE(KEYCODE_7) /* 7 ' */
	PORT_BIT( 0x00000200, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("8 (") PORT_CODE(KEYCODE_8) /* 8 ( */
	PORT_BIT( 0x00000400, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("9 )") PORT_CODE(KEYCODE_9) /* 9 ) */
	PORT_BIT( 0x00000800, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("0") PORT_CODE(KEYCODE_0) /* 0 */
	PORT_BIT( 0x00001000, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("- _") PORT_CODE(KEYCODE_MINUS) /* - _ */
	PORT_BIT( 0x00002000, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("= +") PORT_CODE(KEYCODE_EQUALS) /* = + */
	PORT_BIT( 0x00004000, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("@ ^") PORT_CODE(KEYCODE_BACKSLASH2) /* ~ ` */
	PORT_BIT( 0x00008000, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_CODE(KEYCODE_BACKSPACE) /* Backspace */
	PORT_BIT( 0x00010000, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_CODE(KEYCODE_TAB) /* Tab */
	PORT_BIT( 0x00020000, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Q") PORT_CODE(KEYCODE_Q) /* Q */
	PORT_BIT( 0x00040000, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("W") PORT_CODE(KEYCODE_W) /* W */
	PORT_BIT( 0x00080000, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("E") PORT_CODE(KEYCODE_E) /* E */
	PORT_BIT( 0x00100000, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("R") PORT_CODE(KEYCODE_R) /* R */
	PORT_BIT( 0x00200000, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("T") PORT_CODE(KEYCODE_T) /* T */
	PORT_BIT( 0x00400000, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Y") PORT_CODE(KEYCODE_Y) /* Y */
	PORT_BIT( 0x00800000, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("U") PORT_CODE(KEYCODE_U) /* U */
	PORT_BIT( 0x01000000, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("I") PORT_CODE(KEYCODE_I) /* I */
	PORT_BIT( 0x02000000, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("O") PORT_CODE(KEYCODE_O) /* O */
	PORT_BIT( 0x04000000, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("P") PORT_CODE(KEYCODE_P) /* P */
	PORT_BIT( 0x08000000, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("[ {") PORT_CODE(KEYCODE_OPENBRACE) /* [ { */
	PORT_BIT( 0x10000000, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("] }") PORT_CODE(KEYCODE_CLOSEBRACE) /* ] } */
	PORT_BIT( 0x20000000, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_CODE(KEYCODE_ENTER) /* Return */
	PORT_BIT( 0x40000000, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("A") PORT_CODE(KEYCODE_A) /* A */
	PORT_BIT( 0x80000000, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("S") PORT_CODE(KEYCODE_S) /* S */

	PORT_START( "keyboard2" )
	PORT_BIT( 0x00000001, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("D") PORT_CODE(KEYCODE_D) /* D */
	PORT_BIT( 0x00000002, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("F") PORT_CODE(KEYCODE_F) /* F */
	PORT_BIT( 0x00000004, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("G") PORT_CODE(KEYCODE_G) /* G */
	PORT_BIT( 0x00000008, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("H") PORT_CODE(KEYCODE_H) /* H */
	PORT_BIT( 0x00000010, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("J") PORT_CODE(KEYCODE_J) /* J */
	PORT_BIT( 0x00000020, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("K") PORT_CODE(KEYCODE_K) /* K */
	PORT_BIT( 0x00000040, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("L") PORT_CODE(KEYCODE_L) /* L */
	PORT_BIT( 0x00000080, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("; +") PORT_CODE(KEYCODE_COLON) /* ; + */
	PORT_BIT( 0x00000100, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME(": *") PORT_CODE(KEYCODE_QUOTE) /* : * */
	PORT_BIT( 0x00000200, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("\\ |") PORT_CODE(KEYCODE_BACKSLASH) /* \\ | */
	PORT_BIT( 0x00000400, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Z") PORT_CODE(KEYCODE_Z) /* Z */
	PORT_BIT( 0x00000800, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("X") PORT_CODE(KEYCODE_X) /* X */
	PORT_BIT( 0x00001000, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("C") PORT_CODE(KEYCODE_C) /* C */
	PORT_BIT( 0x00002000, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("V") PORT_CODE(KEYCODE_V) /* V */
	PORT_BIT( 0x00004000, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("B") PORT_CODE(KEYCODE_B) /* B */
	PORT_BIT( 0x00008000, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("N") PORT_CODE(KEYCODE_N) /* N */
	PORT_BIT( 0x00010000, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("M") PORT_CODE(KEYCODE_M) /* M */
	PORT_BIT( 0x00020000, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME(", <") PORT_CODE(KEYCODE_COMMA) /* , < */
	PORT_BIT( 0x00040000, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME(". >") PORT_CODE(KEYCODE_STOP) /* . > */
	PORT_BIT( 0x00080000, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("/ ?") PORT_CODE(KEYCODE_SLASH) /* / ? */
	PORT_BIT( 0x00100000, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("~ '") PORT_CODE(KEYCODE_TILDE) /* Underscore (shifted only?) */
	PORT_BIT( 0x00200000, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Space")  PORT_CODE(KEYCODE_SPACE) /* Space */
	PORT_BIT( 0x00400000, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Home")  PORT_CODE(KEYCODE_HOME) /* Home */
	PORT_BIT( 0x00800000, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Delete")  PORT_CODE(KEYCODE_DEL) /* Del */
	PORT_BIT( 0x01000000, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Roll Up")  PORT_CODE(KEYCODE_PGUP) /* Roll Up */
	PORT_BIT( 0x02000000, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Roll Down")  PORT_CODE(KEYCODE_PGDN) /* Roll Down */
	PORT_BIT( 0x04000000, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("End")  PORT_CODE(KEYCODE_END) /* End */
	PORT_BIT( 0x08000000, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Cursor Left")  PORT_CODE(KEYCODE_LEFT) /* Left */
	PORT_BIT( 0x10000000, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Cursor Up")  PORT_CODE(KEYCODE_UP) /* Up */
	PORT_BIT( 0x20000000, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Cursor Right")  PORT_CODE(KEYCODE_RIGHT) /* Right */
	PORT_BIT( 0x40000000, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Cursor Down")  PORT_CODE(KEYCODE_DOWN) /* Down */
	PORT_BIT( 0x80000000, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("MENU")  PORT_CODE(KEYCODE_MENU) /* Menu = POP */

	PORT_START( "keyboard3" )
	PORT_BIT( 0x00000001, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Numpad /")  PORT_CODE(KEYCODE_SLASH_PAD) /* / (numpad) */
	PORT_BIT( 0x00000002, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Numpad *")  PORT_CODE(KEYCODE_ASTERISK) /* * (numpad) */
	PORT_BIT( 0x00000004, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Numpad -")  PORT_CODE(KEYCODE_MINUS_PAD) /* - (numpad) */
	PORT_BIT( 0x00000008, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Numpad 7")  PORT_CODE(KEYCODE_7_PAD) /* 7 (numpad) */
	PORT_BIT( 0x00000010, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Numpad 8")  PORT_CODE(KEYCODE_8_PAD) /* 8 (numpad) */
	PORT_BIT( 0x00000020, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Numpad 9")  PORT_CODE(KEYCODE_9_PAD) /* 9 (numpad) */
	PORT_BIT( 0x00000040, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Numpad +")  PORT_CODE(KEYCODE_PLUS_PAD) /* + (numpad) */
	PORT_BIT( 0x00000080, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Numpad 4")  PORT_CODE(KEYCODE_4_PAD) /* 4 (numpad) */
	PORT_BIT( 0x00000100, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Numpad 5")  PORT_CODE(KEYCODE_5_PAD) /* 5 (numpad) */
	PORT_BIT( 0x00000200, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Numpad 6")  PORT_CODE(KEYCODE_6_PAD) /* 6 (numpad) */
	PORT_BIT( 0x00000400, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Numpad =") /* = (numpad) */
	PORT_BIT( 0x00000800, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Numpad 1")  PORT_CODE(KEYCODE_1_PAD) /* 1 (numpad) */
	PORT_BIT( 0x00001000, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Numpad 2")  PORT_CODE(KEYCODE_2_PAD) /* 2 (numpad) */
	PORT_BIT( 0x00002000, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Numpad 3")  PORT_CODE(KEYCODE_3_PAD) /* 3 (numpad) */
	PORT_BIT( 0x00004000, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Numpad Enter")  PORT_CODE(KEYCODE_ENTER_PAD) /* Enter (numpad) */
	PORT_BIT( 0x00008000, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Numpad 0")  PORT_CODE(KEYCODE_0_PAD) /* 0 (numpad) */
	PORT_BIT( 0x00010000, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Numpad ,") /* , (numpad) */
	PORT_BIT( 0x00020000, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Numpad .")  PORT_CODE(KEYCODE_DEL_PAD) /* 2 (numpad) */

	PORT_BIT( 0x00040000, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("F1")  PORT_CODE(KEYCODE_F1) /* F1 */
	PORT_BIT( 0x00080000, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("F2")  PORT_CODE(KEYCODE_F2) /* F2 */
	PORT_BIT( 0x00100000, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("F3")  PORT_CODE(KEYCODE_F3) /* F3 */
	PORT_BIT( 0x00200000, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("F4")  PORT_CODE(KEYCODE_F4) /* F4 */
	PORT_BIT( 0x00400000, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("F5")  PORT_CODE(KEYCODE_F5) /* F5 */
	PORT_BIT( 0x00800000, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("F6")  PORT_CODE(KEYCODE_F6) /* F6 */
	PORT_BIT( 0x01000000, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("F7")  PORT_CODE(KEYCODE_F7) /* F7 */
	PORT_BIT( 0x02000000, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("F8")  PORT_CODE(KEYCODE_F8) /* F8 */
	PORT_BIT( 0x04000000, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("F9")  PORT_CODE(KEYCODE_F9) /* F9 */
	PORT_BIT( 0x08000000, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("F10") PORT_CODE(KEYCODE_F10) /* F10 */
	PORT_BIT( 0x10000000, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("F11") PORT_CODE(KEYCODE_F11) /* F11 */
	PORT_BIT( 0x20000000, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("F12") PORT_CODE(KEYCODE_F12) /* F12 */
	PORT_BIT( 0x40000000, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Insert") PORT_CODE(KEYCODE_INSERT) /* Insert */
	PORT_BIT( 0x80000000, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("WIN_R")  PORT_CODE(KEYCODE_RWIN) /* Windows Right = Next Window */

	PORT_START( "keyboard4" )

	PORT_BIT( 0x00000001, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Caps")  PORT_CODE(KEYCODE_CAPSLOCK) /* Caps lock */
	PORT_BIT( 0x00000002, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Shift")  PORT_CODE(KEYCODE_LSHIFT) /* Shift */
	PORT_BIT( 0x00000004, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Ctrl")  PORT_CODE(KEYCODE_LCONTROL)  PORT_CODE(KEYCODE_RCONTROL) /* Ctrl */
	PORT_BIT( 0x00000008, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("ALT_L")  PORT_CODE(KEYCODE_LALT) /* ALT */
	PORT_BIT( 0x00000010, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("ALT_R")  PORT_CODE(KEYCODE_RALT) /* ALT GR */
	PORT_BIT( 0x00000020, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Shift")  PORT_CODE(KEYCODE_RSHIFT) /* Shift */
	PORT_BIT( 0x00000040, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Num Lock")  PORT_CODE(KEYCODE_NUMLOCK) /* Num Lock */

	PORT_START("mouse1")  // mouse buttons
	PORT_BIT( 0x00000010, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Left mouse button") PORT_CODE(MOUSECODE_BUTTON1)
	PORT_BIT( 0x00000020, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("Right mouse button") PORT_CODE(MOUSECODE_BUTTON3)
	PORT_BIT( 0x00000040, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Center mouse button") PORT_CODE(MOUSECODE_BUTTON2)

	PORT_START("mouse2")  // X-axis
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_X) PORT_SENSITIVITY(50) PORT_KEYDELTA(0) PORT_PLAYER(1)

	PORT_START("mouse3")  // Y-axis
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_Y) PORT_SENSITIVITY(50) PORT_KEYDELTA(0) PORT_PLAYER(1)
INPUT_PORTS_END

} // anonymous namespace


/***************************************************************************
	IMPLEMENTATION
***************************************************************************/

// device type definition
DEFINE_DEVICE_TYPE(APOLLO_DN300_KBD, apollo_dn300_kbd_device, "apollo_dn300_kbd", "Apollo DN300 Keyboard")

//-------------------------------------------------
// apollo_dn300_kbd_device - constructor
//-------------------------------------------------

apollo_dn300_kbd_device::apollo_dn300_kbd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, APOLLO_DN300_KBD, tag, owner, clock)
	, device_serial_interface(mconfig, *this)
	, m_io_keyboard(*this, "keyboard%u", 1U)
	, m_io_mouse(*this, "mouse%u", 1U)
	, m_tx_w(*this)
{
}

//-------------------------------------------------
//  device_input_ports - device-specific ports
//-------------------------------------------------

ioport_constructor apollo_dn300_kbd_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(apollo_dn300_kbd);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void apollo_dn300_kbd_device::device_start()
{
	m_timer = timer_alloc(FUNC(apollo_dn300_kbd_device::kbd_scan_timer), this);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void apollo_dn300_kbd_device::device_reset()
{
	// init mouse
	m_mouse_last_b = -1;
	m_mouse_last_x = 0;
	m_mouse_last_y = 0;
	m_mouse_tx_pending = 0;

	// init keyboard
	memset(m_keyon, 0, sizeof(m_keyon));

	// start timer
	m_timer->adjust( attotime::zero, 0, attotime::from_msec(5)); // every 5ms

	// the kernel programs the 6850 for /16, 8E1
	set_data_frame(1, 8, PARITY_EVEN, STOP_BITS_1);
	set_tra_rate(1200);

	m_tx_busy = false;
	m_xmit_read = m_xmit_write = 0;
}


//**************************************************************************
//  Mouse
//**************************************************************************

void apollo_dn300_kbd_device::read_mouse()
{
	if (m_mouse_tx_pending > 0)
	{
		m_mouse_tx_pending -= 5; // we will be called every 5ms
	}
	else
	{
		char b = m_io_mouse[0]->read();
		char x = m_io_mouse[1]->read();
		char y = m_io_mouse[2]->read();

		y = -y;

		if (m_mouse_last_b < 0)
		{
			m_mouse_last_b = b;
			m_mouse_last_x = x;
			m_mouse_last_y = y;
		}
		else if (b != m_mouse_last_b || x != m_mouse_last_x || y != m_mouse_last_y)
		{
			int dx = x - m_mouse_last_x;
			int dy = y - m_mouse_last_y;

			LOGMOUSE("read_mouse: b=%02x x=%d y=%d dx=%d dy=%d\n", b, x, y, dx, dy);

			// the report: 0xdf, the buttons (active low), dx, dy
			xmit_char(0xdf);
			xmit_char(0xf0 ^ b);
			xmit_char(dx);
			xmit_char(dy);

			// mouse data submitted; update current mouse state
			m_mouse_last_b = b;
			m_mouse_last_x = x;
			m_mouse_last_y = y;
			m_mouse_tx_pending = 50; // mouse data packet will take 40 ms
		}
	}
}

void apollo_dn300_kbd_device::tra_complete()    // Tx completed sending byte
{
	// is there more waiting to send?
	if (m_xmit_read != m_xmit_write)
	{
		transmit_register_setup(m_xmitring[m_xmit_read++]);
		if (m_xmit_read >= XMIT_RING_SIZE)
		{
			m_xmit_read = 0;
		}
	}
	else
	{
		m_tx_busy = false;
	}
}

void apollo_dn300_kbd_device::tra_callback()    // Tx send bit
{
	int bit = transmit_register_get_data_bit();
	m_tx_w(bit);
}

void apollo_dn300_kbd_device::xmit_char(uint8_t data)
{
	// if tx is busy it'll pick this up automatically when it completes
	if (!m_tx_busy)
	{
		m_tx_busy = true;
		transmit_register_setup(data);
	}
	else
	{
		// tx is busy, it'll pick this up next time
		m_xmitring[m_xmit_write++] = data;
		if (m_xmit_write >= XMIT_RING_SIZE)
		{
			m_xmit_write = 0;
		}
	}
}

int apollo_dn300_kbd_device::push_scancode(uint8_t code)
{
	int n_chars = 0;
	uint16_t key_code = 0;
	ioport_value const modifiers = m_io_keyboard[3]->read();
	uint8_t const caps = BIT(modifiers,0);
	uint8_t const shift = BIT(modifiers,1) | BIT(modifiers,5);
	uint8_t const ctrl = BIT(modifiers,2);

	code_entry const &entry = s_code_table[code & 0x7f];
	if (ctrl)
	{
		key_code = entry.control;
	}
	else if (shift)
	{
		key_code = entry.shifted;
	}
	else if (caps)
	{
		key_code = entry.caps_lock;
	}
	else
	{
		key_code = entry.unshifted;
	}

	if (key_code != 0)
	{
		LOGKEYS("scan_code = 0x%02x key_code = 0x%04x, caps/shift/ctrl = %d/%d/%d\n", code, key_code, caps ? 1 : 0, shift ? 1 : 0, ctrl ? 1 : 0);
		if (key_code & 0xff00)
		{
			xmit_char(key_code >> 8);
			n_chars++;
		}
		xmit_char(key_code & 0xff);
		n_chars++;
	}
	return n_chars;
}

void apollo_dn300_kbd_device::scan_keyboard()
{
	for (int x = 0; x < 0x80; x++)
	{
		ioport_value v = m_io_keyboard[x / 32]->read();
		if (!(v & (1 << (x % 32))))
		{
			// no key pressed (key releases are not sent)
			m_keyon[x] = 0;
		}
		else if (m_keyon[x] == 0)
		{
			// a key has been pressed
			if (push_scancode(x) > 0)
			{
				m_keyon[x] = 1;
				LOGKEYS("pushed key 0x%02x\n", x);
			}
		}
	}
}

TIMER_CALLBACK_MEMBER(apollo_dn300_kbd_device::kbd_scan_timer)
{
	scan_keyboard();

	// in Compatibility mode the mouse's reports (0xdf, buttons, dx, dy) go
	// out with the keys, as the hardware does; the PROM never changes the
	// mode, so at the MD prompt it takes them as keystrokes
	read_mouse();
}

apollo_dn300_kbd_device::code_entry const apollo_dn300_kbd_device::s_code_table[] = {
		/* Key   | Keycap         | Down | Up  |Unshifted|Shifted|Control|Caps Lock|Up Trans|Auto  */
		/* Number| Legend         | Code | Code|Code     | Code  | Code  |Code     | Code   |Repeat*/

		/* B14     ~ '         */ { 0x24, 0xA4, 0x60,     0x7E,   0x1E,   0x60,     NOP,     No  },
		/* B1      ESC         */ { 0x17, 0x97, 0x1B,     0x1B,   NOP,    0x1B,     NOP,     No  },
		/* B2      ! 1         */ { 0x18, 0x98, 0x31,     0x21,   NOP,    0x31,     NOP,     No  },
		/* B3      @ 2         */ { 0x19, 0x99, 0x32,     0x40,   NOP,    0x32,     NOP,     No  },
		/* B4      # 3         */ { 0x1A, 0x9A, 0x33,     0x23,   NOP,    0x33,     NOP,     No  },
		/* B5      $ 4         */ { 0x1B, 0x9B, 0x34,     0x24,   NOP,    0x34,     NOP,     No  },
		/* B6      % 5         */ { 0x1C, 0x9C, 0x35,     0x25,   NOP,    0x35,     NOP,     No  },
		/* B7      ^ 6         */ { 0x1D, 0x9D, 0x36,     0x5E,   NOP,    0x36,     NOP,     No  },
		/* B8      & 7         */ { 0x1E, 0x9E, 0x37,     0x26,   NOP,    0x37,     NOP,     No  },
		/* B9      * 8         */ { 0x1F, 0x9F, 0x38,     0x2A,   NOP,    0x38,     NOP,     No  },
		/* B10     ( 9         */ { 0x20, 0xA0, 0x39,     0x28,   NOP,    0x39,     NOP,     No  },
		/* B11     ) 0         */ { 0x21, 0xA1, 0x30,     0x29,   NOP,    0x30,     NOP,     No  },
		/* B12     _ -         */ { 0x22, 0xA2, 0x2D,     0x5F,   NOP,    0x2D,     NOP,     Yes },
		/* B13     + =         */ { 0x23, 0xA3, 0x3D,     0x2B,   NOP,    0x3D,     NOP,     Yes },
		/* D14     \\ |        */ { 0x53, 0xD3, 0xC8,     0xC9,   NOP,    0xC8,     NOP,     No  },
		/* B15     BACKSPACE   */ { 0x25, 0xA5, 0xDE,     0xDE,   NOP,    0xDE,     NOP,     Yes }, // BS: SMD_$KTT[0xDE] = 0x95 (BS)

		/* C1      TAB         */ { 0x2C, 0xAC, 0xCA,     0xDA,   0xFA,   0xCA,     NOP,     No  },
		/* C2      Q           */ { 0x2D, 0xAD, 0x71,     0x51,   0x11,   0x51,     NOP,     No  },
		/* C3      W           */ { 0x2E, 0xAE, 0x77,     0x57,   0x17,   0x57,     NOP,     No  },
		/* C4      E           */ { 0x2F, 0xAF, 0x65,     0x45,   0x05,   0x45,     NOP,     No  },
		/* C5      R           */ { 0x30, 0xB0, 0x72,     0x52,   0x12,   0x52,     NOP,     No  },
		/* C6      T           */ { 0x31, 0xB1, 0x74,     0x54,   0x14,   0x54,     NOP,     No  },
		/* C7      Y           */ { 0x32, 0xB2, 0x79,     0x59,   0x19,   0x59,     NOP,     No  },
		/* C8      U           */ { 0x33, 0xB3, 0x75,     0x55,   0x15,   0x55,     NOP,     No  },
		/* C9      I           */ { 0x34, 0xB4, 0x69,     0x49,   0x09,   0x49,     NOP,     No  },
		/* C10     O           */ { 0x35, 0xB5, 0x6F,     0x4F,   0x0F,   0x4F,     NOP,     No  },
		/* C11     P           */ { 0x36, 0xB6, 0x70,     0x50,   0x10,   0x50,     NOP,     No  },
		/* C12     { [ / Ue    */ { 0x37, 0xB7, 0x7B,     0x5B,   0x1B,   0x7B,     NOP,     No  },
		/* C13     } ] / Oe    */ { 0x38, 0xB8, 0x7D,     0x5D,   0x1D,   0x7D,     NOP,     No  },
		// RETURN: SMD_$KTT[0xCB] = SMD_$KTT[0xDB] = 0x96 (the DM's CR); a plain 0x0D is not RETURN to the DM
		/* D13     RETURN      */ { 0x52, 0xD2, 0xCB,     0xDB,   NOP,    0xCB,     NOP,     No  },

		/* D2      A           */ { 0x46, 0xC6, 0x61,     0x41,   0x01,   0x41,     NOP,     No  },
		/* D3      S           */ { 0x47, 0xC7, 0x73,     0x53,   0x13,   0x53,     NOP,     No  },
		/* D4      D           */ { 0x48, 0xC8, 0x64,     0x44,   0x04,   0x44,     NOP,     No  },
		/* D5      F           */ { 0x49, 0xC9, 0x66,     0x46,   0x06,   0x46,     NOP,     No  },
		/* D6      G           */ { 0x4A, 0xCA, 0x67,     0x47,   0x07,   0x47,     NOP,     No  },
		/* D7      H           */ { 0x4B, 0xCB, 0x68,     0x48,   0x08,   0x48,     NOP,     No  },
		/* D8      J           */ { 0x4C, 0xCC, 0x6A,     0x4A,   0x0A,   0x4A,     NOP,     No  },
		/* D9      K           */ { 0x4D, 0xCD, 0x6B,     0x4B,   0x0B,   0x4B,     NOP,     No  },
		/* D10     L           */ { 0x4E, 0xCE, 0x6C,     0x4C,   0x0C,   0x4C,     NOP,     No  },
		/* D11     : ; / Oe    */ { 0x4F, 0xCF, 0x3B,     0x3A,   0xFB,   0x3B,     NOP,     No  },
		/* D12     " ' / Ae    */ { 0x50, 0xD0, 0x27,     0x22,   0xF8,   0x27,     NOP,     No  },
		// Apollo US keyboards have no hash key (#)
		// the host \ | key (KEYCODE_BACKSLASH, scan index 0x29) lands on this row: send the D14 \ | codes
		// (SMD_$KTT[0xC8] = '\\', [0xC9] = '|'); plain 0x5C / 0x7C are F0 shifted / control to the kernel
		/* D14     \\ |        */ { 0x53, 0xD3, 0xC8,     0xC9,   NOP,    0xC8,     NOP,     No  },

		/* E2      Z           */ { 0x60, 0xE0, 0x7A,     0x5A,   0x1A,   0x5A,     NOP,     No  },
		/* E3      X           */ { 0x61, 0xE1, 0x78,     0x58,   0x18,   0x58,     NOP,     No  },
		/* E4      C           */ { 0x62, 0xE2, 0x63,     0x43,   0x03,   0x43,     NOP,     No  },
		/* E5      V           */ { 0x63, 0xE3, 0x76,     0x56,   0x16,   0x56,     NOP,     No  },
		/* E6      B           */ { 0x64, 0xE4, 0x62,     0x42,   0x02,   0x42,     NOP,     No  },
		/* E7      N           */ { 0x65, 0xE5, 0x6E,     0x4E,   0x0E,   0x4E,     NOP,     No  },
		/* E8      M           */ { 0x66, 0xE6, 0x6D,     0x4D,   0x0D,   0x4D,     NOP,     No  },
		/* E9      < ,         */ { 0x67, 0xE7, 0x2C,     0x3C,   NOP,    0x2C,     NOP,     No  },
		/* E10     > .         */ { 0x68, 0xE8, 0x2E,     0x3E,   NOP,    0x2E,     NOP,     Yes },
		/* E11     ? /         */ { 0x69, 0xE9, 0xCC,     0xDC,   0xFC,   0xCC,     NOP,     No  },

		/*         _           */ { NOP,  NOP,  NOP,      NOP,    NOP,    NOP,      NOP,     NOP },
		/* F1      (space bar) */ { 0x76, 0xF6, 0x20,     0x20,   0x20,   0x20,     NOP,     Yes },
		/* LC0     Home        */ { 0x27, 0xA7, 0x84,     0x94,   0x84,   0x84,     0xA4,    No  },
		/* C14     DELETE      */ { 0x3A, 0xBA, 0x7F,     0x7F,   NOP,    0x7F,     NOP,     Yes },
		/* LF0     Roll Up     */ { 0x72, 0xF2, 0x8D,     0x9D,   0x8D,   0x8D,     0xAD,    No  },
		/* LF2     Roll Down   */ { 0x74, 0xF4, 0x8F,     0x9F,   0x8F,   0x8F,     0xAF,    No  },
		/* LC2     End         */ { 0x29, 0xA9, 0x86,     0x96,   0x86,   0x86,     0xA6,    No  },
		/* LE0     Cursor left */ { 0x59, 0xD9, 0x8A,     0x9A,   0x9A,   0x9A,     0xAA,    Yes },
		/* LD1     Cursor Up   */ { 0x41, 0xC1, 0x88,     0x98,   0x88,   0x88,     0xA8,    Yes },
		/* LE2     Cursor right*/ { 0x5B, 0xDB, 0x8C,     0x9C,   0x8C,   0xBE,     0xAC,    Yes },
		/* LF1     Cursor down */ { 0x73, 0xF3, 0x8E,     0x9E,   0x8E,   0x8E,     0xAE,    Yes },

		/*         Numpad CLR  */ { NOP,  NOP,  NOP,      NOP,    NOP,    NOP,      NOP,     NOP },
		/*         Numpad /    */ { NOP,  NOP,  NOP,      NOP,    NOP,    NOP,      NOP,     NOP },
		/*         Numpad *    */ { NOP,  NOP,  NOP,      NOP,    NOP,    NOP,      NOP,     NOP },
		/* RD4     -           */ { 0x58, 0xD8, 0xFE2D,   0xFE5F, NOP,    0xFE2D,   NOP,     No  },
		/* RC1     7           */ { 0x3C, 0xBC, 0xFE37,   0xFE26, NOP,    0xFE37,   NOP,     No  },
		/* RC2     8           */ { 0x3D, 0xBD, 0xFE38,   0xFE2A, NOP,    0xFE38,   NOP,     No  },
		/* RC3     9           */ { 0x3E, 0xBE, 0xFE39,   0xFE28, NOP,    0xFE39,   NOP,     No  },
		/* RC4     +           */ { 0x3F, 0xBF, 0xFE2B,   0xFE3D, NOP,    0xFE2B,   NOP,     No  },
		/* RD1     4           */ { 0x55, 0xD5, 0xFE34,   0xFE24, NOP,    0xFE34,   NOP,     No  },
		/* RD2     5           */ { 0x56, 0xD6, 0xFE35,   0xFE25, NOP,    0xFE35,   NOP,     No  },
		/* RD3     6           */ { 0x57, 0xD7, 0xFE36,   0xFE5E, NOP,    0xFE36,   NOP,     No  },
		/*         Numpad =    */ { NOP,  NOP,  NOP,      NOP,    NOP,    NOP,      NOP,     NOP },
		/* RE1     1           */ { 0x6E, 0xEE, 0xFE31,   0xFE21, NOP,    0xFE31,   NOP,     No  },
		/* RE2     2           */ { 0x6F, 0xEF, 0xFE32,   0xFE40, NOP,    0xFE32,   NOP,     No  },
		/* RE3     3           */ { 0x70, 0xF0, 0xFE33,   0xFE23, NOP,    0xFE33,   NOP,     No  },
		/* RF3     ENTER       */ { 0x7C, 0xFC, 0xFECB,   0xFEDB, NOP,    0xFECB,   NOP,     No  },
		/* RF1     0           */ { 0x79, 0xF9, 0xFE30,   0xFE29, NOP,    0xFE30,   NOP,     No  },
		/*         Numpad ,    */ { NOP,  NOP,  NOP,      NOP,    NOP,    NOP,      NOP,     NOP },
		/* RF2     .           */ { 0x7B, 0xFB, 0xFE2E,   0xFE2E, NOP,    0xFE2E,   NOP,     No  },
		/* A0      F0          */ { 0x04, 0x84, 0x1C,     0x5C,   0x7C,   0x1C,     0xBC,    No  },
		/* A1      F1          */ { 0x05, 0x85, 0xC0,     0xD0,   0xF0,   0xC0,     0xE0,    No  },
		/* A2      F2          */ { 0x06, 0x86, 0xC1,     0x01,   0xF1,   0xC1,     0xE1,    No  },
		/* A3      F3          */ { 0x07, 0x87, 0xC2,     0x02,   0xF2,   0xC2,     0xE2,    No  },
		/* A4      F4          */ { 0x08, 0x88, 0xC3,     0x03,   0xF3,   0xC3,     0xE3,    No  },
		/* A5      F5          */ { 0x09, 0x89, 0xC4,     0x04,   0xF4,   0xC4,     0xE4,    No  },
		/* A6      F6          */ { 0x0A, 0x8A, 0xC5,     0x05,   0xF5,   0xC5,     0xE5,    No  },
		/* A7      F7          */ { 0x0B, 0x8B, 0xC6,     0x06,   0xF6,   0xC6,     0xE6,    No  },
		/* A8      F8          */ { 0x0C, 0x8C, 0xC7,     0x07,   0xF7,   0xC7,     0xE7,    No  },
		/* A9      F9          */ { 0x0D, 0x8D, 0x1F,     0x2F,   0x3F,   0x1F,     0xBD,    No  },
		/* RA3 F11/ABORT/EXIT  */ { 0x11, 0x91, 0xDD,     0xEC,   0xD0,   0xD0,     0xFD,    No  },
		/* RA4 F12/HELP/HOLD   */ { 0x12, 0x92, 0xB3,     0xB7,   0xB3,   0xB3,     0xBB,    No  },

		/* LE1     NEXT WINDOW */ { 0x5A, 0xDA, 0x8B,     0x9B,   0x8B,   0x8B,     0xAB,    No  },
		/* LE1     NEXT WINDOW */ { 0x5A, 0xDA, 0x8B,     0x9B,   0x8B,   0x8B,     0xAB,    No  },

		/* D1      CAPS LOCK   */ { NOP,  NOP,  NOP,      NOP,    NOP,    NOP,      NOP,     NOP },
		/* E1      SHIFT       */ { 0x5E, 0xDE, NOP,      NOP,    NOP,    NOP,      NOP,     NOP },
		/* DO      CTRL        */ { 0x43, 0xC3, NOP,      NOP,    NOP,    NOP,      NOP,     NOP },
		// TODO: the two ALT keys are swapped
		/* ??      ALT_R       */ { 0x77, 0xF7, 0xfe00,   NOP,    NOP,    NOP,      0xfe01,  No  },
		/* ??      ALT_L       */ { 0x75, 0xF5, 0xfe02,   NOP,    NOP,    NOP,      0xfe03,  No  },
		/* E12     SHIFT       */ { 0x6A, 0xEA, NOP,      NOP,    NOP,    NOP,      NOP,     NOP },
		// not sent:
		/* E0      REPEAT      */ { 0x5D, 0xDD, NOP,      NOP,    NOP,    NOP,      NOP,     NOP },
		/*        CAPS LOCK LED*/ { 0x7E, 0xFE, NOP,      NOP,    NOP,    NOP,      NOP,     NOP },
		// German keyboard:
		/* 0x68 B1_DE   _      */ { 0x17, 0x97, 0x60,     0x7E,   0x1E,   0x60,     NOP,     No  },
		/* 0x69 E1a_DE  <>     */ { 0x5F, 0xDF, 0xBE,     0xBE,   NOP,    0xBE,     NOP,     No  },
		/* 0x6a B14_DE  ESC    */ { 0x16, 0x96, 0x1B,     0x1B,   NOP,    0x1B,     NOP,     No  },
		/* 0x6b D14_DE  # \    */ { 0x51, 0xD1, 0xC8,     0xC9,   NOP,    0xC8,     NOP,     No  },

		/* 0x6c NPG     NP (   */ { 0x3F, 0xBF, 0xFE28,   0xFE0E, NOP,    0xFE28,   NOP,     No  },
		/* 0x6d NPF     NP )   */ { 0x58, 0xD8, 0xFE29,   0xFE0F, NOP,    0xFE29,   NOP,     No  },
		/* 0x6e NPD     NP +   */ { 0x3B, 0xBB, 0xFE2B,   0xFE26, NOP,    0xFE2B,   NOP,     No  },
		/* 0x6f NPC     NP -   */ { 0x54, 0xD4, 0xFE2D,   0xFE7E, NOP,    0xFE2D,   NOP,     No  },
		/* 0x70 NPB     NP *   */ { 0x6D, 0xED, 0xFE2A,   0xFE21, NOP,    0xFE2A,   NOP,     No  },
		/* 0x71 NPA     NP /   */ { 0x78, 0xF8, 0xFECC,   0xFEC8, NOP,    0xFECC,   NOP,     No  },
		/* 0x72 NPP     NP .   */ { 0x7B, 0xFB, 0xFE2E,   0xFE2C, NOP,    0xFE2E,   NOP,     No  },
		/* 0x73 NPE     ENTER  */ { 0x7C, 0xFC, 0xFECB,   0xFE3D, NOP,    0xFECB,   NOP,     No  },

		/* 0x74 A0      F0     */ { 0x04, 0x84, 0x1C,     0x5C,   0x7C,   0x1C,     0xBC,    No  },
		/* 0x75 A1      F1     */ { 0x05, 0x85, 0xC0,     0xD0,   0xF0,   0xC0,     0xE0,    No  },
		/* 0x76 A2      F2     */ { 0x06, 0x86, 0xC1,     0x01,   0xF1,   0xC1,     0xE1,    No  },
		/* 0x77 A3      F3     */ { 0x07, 0x87, 0xC2,     0x02,   0xF2,   0xC2,     0xE2,    No  },
		/* 0x78 A4      F4     */ { 0x08, 0x88, 0xC3,     0x03,   0xF3,   0xC3,     0xE3,    No  },
		/* 0x79 A5      F5     */ { 0x09, 0x89, 0xC4,     0x04,   0xF4,   0xC4,     0xE4,    No  },
		/* 0x7a A6      F6     */ { 0x0A, 0x8A, 0xC5,     0x05,   0xF5,   0xC5,     0xE5,    No  },
		/* 0x7b A7      F7     */ { 0x0B, 0x8B, 0xC6,     0x06,   0xF6,   0xC6,     0xE6,    No  },
		/* 0x7c A8      F8     */ { 0x0C, 0x8C, 0xC7,     0x07,   0xF7,   0xC7,     0xE7,    No  },
		/* 0x7d A9      F9     */ { 0x0D, 0x8D, 0x1F,     0x2F,   0x3F,   0x1F,     0xBD,    No  },

		/* Key   | Keycap      | Down | Up  |Unshifted|Shifted|Control|Caps Lock|Up Trans|Auto  */
		/* Number| Legend      | Code | Code|Code     | Code  | Code  |Code     | Code   |Repeat*/
};
