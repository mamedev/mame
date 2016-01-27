// license:BSD-3-Clause
// copyright-holders:Hans Ostermeyer
/*
 * apollo_kbd.c - Apollo keyboard and mouse emulation
 *
 *  Created on: Dec 27, 2010
 *      Author: Hans Ostermeyer
 *
 *  see also http://www.bitsavers.org/pdf/apollo/008778-03_DOMAIN_Series_3000_4000_Technical_Reference_Aug87.pdf
 *
 */

#define VERBOSE 0

#include "machine/apollo_kbd.h"
#include "sound/beep.h"

#define LOG(x)  { m_device->logerror ("%s apollo_kbd: ", m_device->cpu_context()); m_device->logerror x; m_device->logerror ("\n"); }
#define LOG1(x) { if (VERBOSE > 0) LOG(x)}
#define LOG2(x) { if (VERBOSE > 1) LOG(x)}

#define MAP_APOLLO_KEYS 1

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

//const device_type APOLLO_KBD = apollo_kbd_device_config::static_alloc_device_config;

//**************************************************************************
//  CONSTANTS
//**************************************************************************

#define KBD_MODE_0_COMPATIBILITY 0
#define KBD_MODE_1_KEYSTATE 1
#define KBD_MODE_2_RELATIVE_CURSOR_CONTROL 2
#define KBD_MODE_3_ABSOLUTE_CURSOR_CONTROL 3

#define CODE_TABLE_DOWN_CODE 0
#define CODE_TABLE_UP_CODE 1
#define CODE_TABLE_UNSHIFTED_CODE 2
#define CODE_TABLE_SHIFTED_CODE 3
#define CODE_TABLE_CONTROL_CODE 4
#define CODE_TABLE_CAPS_LOCK_CODE 5
#define CODE_TABLE_UP_TRANS_CODE 6
#define CODE_TABLE_AUTO_REPEAT_CODE 7
#define CODE_TABLE_ENTRY_SIZE 8

#define NOP 0
#define No  0
#define Yes 1

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

// device type definition
const device_type APOLLO_KBD = &device_creator<apollo_kbd_device>;

//-------------------------------------------------
// apollo_kbd_device - constructor
//-------------------------------------------------

apollo_kbd_device::apollo_kbd_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, APOLLO_KBD, "Apollo Keyboard", tag, owner, clock, "apollo_kbd", __FILE__),
	device_serial_interface(mconfig, *this),
	m_tx_w(*this),
	m_german_r(*this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void apollo_kbd_device::device_start()
{
	m_device = this;
	LOG1(("start apollo_kbd"));

	m_tx_w.resolve_safe();
	m_german_r.resolve_safe(0);

	m_beeper.start(this);
	m_mouse.start(this);

	m_io_keyboard1 = machine().root_device().ioport("keyboard1");
	m_io_keyboard2 = machine().root_device().ioport("keyboard2");
	m_io_keyboard3 = machine().root_device().ioport("keyboard3");
	m_io_keyboard4 = machine().root_device().ioport("keyboard4");
	m_io_mouse1 = machine().root_device().ioport("mouse1");
	m_io_mouse2 = machine().root_device().ioport("mouse2");
	m_io_mouse3 = machine().root_device().ioport("mouse3");

	m_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(apollo_kbd_device::kbd_scan_timer), this));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void apollo_kbd_device::device_reset()
{
	LOG1(("reset apollo_kbd"));

	m_beeper.reset();
	m_mouse.reset();

	// init keyboard
	m_loopback_mode = 1;
	m_mode = KBD_MODE_0_COMPATIBILITY;
	m_delay = 500;
	m_repeat = 33;
	m_last_pressed = 0;
	memset(m_keytime, 0, sizeof(m_keytime));
	memset(m_keyon, 0, sizeof(m_keyon));

	// start timer
	m_timer->adjust( attotime::zero, 0, attotime::from_msec(5)); // every 5ms

	// keyboard comms is at 8E1, 1200 baud
	set_data_frame(1, 8, PARITY_EVEN, STOP_BITS_1);
	set_rcv_rate(1200);
	set_tra_rate(1200);

	m_tx_busy = false;
	m_xmit_read = m_xmit_write = 0;
}

void apollo_kbd_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	device_serial_interface::device_timer(timer, id, param, ptr);
}

/***************************************************************************
 cpu_context - return a string describing the current CPU context
 ***************************************************************************/

const char *apollo_kbd_device::cpu_context()
{
	static char statebuf[64]; /* string buffer containing state description */

	device_t *cpu = machine().firstcpu;
	osd_ticks_t t = osd_ticks();
	int s = t / osd_ticks_per_second();
	int ms = (t % osd_ticks_per_second()) / 1000;

	/* if we have an executing CPU, output data */
	if (cpu != nullptr)
	{
		sprintf(statebuf, "%d.%03d %s pc=%08x - %s", s, ms, cpu->tag(),
				cpu->safe_pcbase(), tag());
	}
	else
	{
		sprintf(statebuf, "%d.%03d", s, ms);
	}
	return statebuf;
}

//**************************************************************************
//  Beeper
//**************************************************************************

apollo_kbd_device::beeper::beeper() :
	m_device(nullptr),
	m_beeper(nullptr),
	m_timer(nullptr)
{
}

void apollo_kbd_device::beeper::start(apollo_kbd_device *device)
{
	m_device = device;
	LOG2(("start apollo_kbd::beeper"));
	m_beeper = m_device->machine().device<beep_device>("beep");
	m_timer = m_device->machine().scheduler().timer_alloc(FUNC(static_beeper_callback), this);
}

void apollo_kbd_device::beeper::reset()
{
	LOG2(("reset apollo_kbd::beeper"));
	on();
}

void apollo_kbd_device::beeper::off()
{
	m_beeper->set_state(0);
}

void apollo_kbd_device::beeper::on()
{
	if (keyboard_has_beeper())
	{
		m_beeper->set_state(1);
		m_timer->adjust( attotime::from_msec(10), 0, attotime::zero);
	}
}

int apollo_kbd_device::beeper::keyboard_has_beeper()
{
	return true;    // driver has no facility to return false here, so go with it
}

void apollo_kbd_device::beeper::beeper_callback()
{
	off();
}

TIMER_CALLBACK( apollo_kbd_device::beeper::static_beeper_callback )
{
	reinterpret_cast<beeper*>(ptr)->beeper_callback();
}

//**************************************************************************
//  Mouse
//**************************************************************************

apollo_kbd_device::mouse::mouse() :
	m_device(nullptr)
{
}

void apollo_kbd_device::mouse::start(apollo_kbd_device *device)
{
	m_device = device;
	LOG2(("start apollo_kbd::mouse"));
}

void apollo_kbd_device::mouse::reset()
{
	LOG2(("reset apollo_kbd::mouse"));

	m_last_b = -1;
	m_last_x = 0;
	m_last_y = 0;
	m_tx_pending = 0;
}

void apollo_kbd_device::mouse::read_mouse()
{
	if (m_tx_pending > 0)
	{
		m_tx_pending -= 5; // we will be called every 5ms
	}
	else
	{
		int b = m_device->m_io_mouse1->read();
		int x = m_device->m_io_mouse2->read();
		int y = m_device->m_io_mouse3->read();

		/* sign extend values < 0 */
		if (x & 0x80)
			x |= 0xffffff00;
		if (y & 0x80)
			y |= 0xffffff00;
		y = -y;

		if (m_last_b < 0)
		{
			m_last_b = b;
			m_last_x = x;
			m_last_y = y;
		}
		else if (b != m_last_b || x != m_last_x || y != m_last_y)
		{
			UINT8 mouse_data[4];
			int mouse_data_size;

			int dx = x - m_last_x;
			int dy = y - m_last_y;

			LOG2(("read_mouse: b=%02x x=%d y=%d dx=%d dy=%d", b, x, y, dx, dy));

			if (m_device->m_mode == KBD_MODE_0_COMPATIBILITY)
			{
				mouse_data[0] = 0xdf;
				mouse_data[1] = 0xf0 ^ b;
				mouse_data[2] = dx;
				mouse_data[3] = dy;
				mouse_data_size = 4;
			}
			else
			{
				if (m_device->m_mode != KBD_MODE_2_RELATIVE_CURSOR_CONTROL)
				{
					m_device->set_mode(KBD_MODE_2_RELATIVE_CURSOR_CONTROL);
				}

				mouse_data[0] = 0xf0 ^ b;
				mouse_data[1] = dx;
				mouse_data[2] = dy;
				mouse_data_size = 3;
			}

			for (int md = 0; md < mouse_data_size; md++)
			{
				m_device->xmit_char(mouse_data[md]);
			}

			// mouse data submitted; update current mouse state
			m_last_b = b;
			m_last_x += dx;
			m_last_y += dy;
			m_tx_pending = 100; // mouse data packet will take 40 ms
		}
	}
}

/*-------------------------------------------------
 keyboard_is_german - check for german keyboard
 -------------------------------------------------*/

int apollo_kbd_device::keyboard_is_german()
{
	return (m_german_r() == ASSERT_LINE) ? true : false;
}

void apollo_kbd_device::set_mode(UINT16 mode)
{
	xmit_char(0xff);
	xmit_char(mode);
	m_mode = mode;
}

void apollo_kbd_device::tra_complete()    // Tx completed sending byte
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

void apollo_kbd_device::rcv_complete()    // Rx completed receiving byte
{
	receive_register_extract();
	UINT8 data = get_received_char();

	kgetchar(data);
}

void apollo_kbd_device::tra_callback()    // Tx send bit
{
	int bit = transmit_register_get_data_bit();
	m_tx_w(bit);
}

void apollo_kbd_device::input_callback(UINT8 state)
{
}

void apollo_kbd_device::xmit_char(UINT8 data)
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

/*-------------------------------------------------
 putdata - put keyboard data to sio
 -------------------------------------------------*/

void apollo_kbd_device::putdata(const UINT8 *data, int data_length)
{
	// send data only if no real Apollo keyboard has been connected
	if (m_mode > KBD_MODE_1_KEYSTATE)
	{
		set_mode(KBD_MODE_1_KEYSTATE);
	}

	for (int i = 0; i < data_length; i++)
	{
		xmit_char(data[i]);
	}
}

/*-------------------------------------------------
 putstring - put keyboard string to sio
 -------------------------------------------------*/

void apollo_kbd_device::putstring(const char *data)
{
	putdata((UINT8 *) data, strlen(data));
}

void apollo_kbd_device::kgetchar(UINT8 data)
{
	static const UINT8 ff1116_data[] = { 0x00, 0xff, 0x00 };

	LOG1(("getchar <- %02x", data));

	if (data == 0xff)
	{
		m_rx_message = data;
		putdata(&data, 1);
		m_loopback_mode = 1;
	}
	else if (data == 0x00)
	{
		if (m_loopback_mode)
		{
			set_mode(KBD_MODE_0_COMPATIBILITY);
			m_loopback_mode = 0;
		}
	}
	else
	{
		m_rx_message = m_rx_message << 8 | data;

		switch (m_rx_message)
		{
		case 0xff00:
			putdata(&data, 1);
			m_mode = KBD_MODE_0_COMPATIBILITY;
			m_loopback_mode = 0;
			m_rx_message = 0;
		case 0xff01:
			putdata(&data, 1);
			m_mode = KBD_MODE_1_KEYSTATE;
			m_rx_message = 0;
			break;
		case 0xff11:
			putdata(&data, 1);
			break;
		case 0xff1116:
			putdata(ff1116_data, sizeof(ff1116_data));
			m_loopback_mode = 0;
			m_rx_message = 0;
			break;
		case 0xff1117:
			m_rx_message = 0;
			break;
		case 0xff12:
			putdata(&data, 1);
			break;
		case 0xff1221: // receive ID message
			m_loopback_mode = 0;
			putdata(&data, 1);
			if (keyboard_is_german())
			{
				putstring("3-A\r2-0\rSD-03863-MS\r");
			}
			else
			{
				putstring("3-@\r2-0\rSD-03863-MS\r");
			}

			if (m_mode == KBD_MODE_0_COMPATIBILITY)
			{
				set_mode(KBD_MODE_0_COMPATIBILITY);
			}
			else
			{
				set_mode(KBD_MODE_1_KEYSTATE);
			}
			m_rx_message = 0;
			break;
		case 0xff2181: // beeper on (for 300 ms)
			putdata(&data, 1);
			m_rx_message = 0;
			m_beeper.on();
			break;
		case 0xff2182: // beeper off
			putdata(&data, 1);
			m_rx_message = 0;
			m_beeper.off();
			break;
		default:
			if (m_loopback_mode && data != 0)
			{
				putdata(&data, 1);
			}
			break;
		}
	}
}

int apollo_kbd_device::push_scancode(UINT8 code, UINT8 repeat)
{
	int n_chars = 0;
	UINT16 key_code = 0;
	UINT8 caps = BIT(machine().root_device().ioport("keyboard4")->read(),0);
	UINT8 shift = BIT(machine().root_device().ioport("keyboard4")->read(),1) | BIT(machine().root_device().ioport("keyboard4")->read(),5);
	UINT8 ctrl = BIT(machine().root_device().ioport("keyboard4")->read(),2);
	UINT8 numlock = BIT(machine().root_device().ioport("keyboard4")->read(),6);
	UINT16 index;

	if (keyboard_is_german())
	{
		// map special keys for German keyboard
		switch (code)
		{
		case 0x00: code = 0x68; break; // _
		case 0x0e: code = 0x6b; break; // #
		case 0x29: code = 0x69; break; // <>
		case 0x42: code = 0x6f; break; // NP-
		case 0x46: code = 0x6e; break; // NP+
		case 0x4e: code = 0x73; break; // NP ENTER
		}
	}

#if MAP_APOLLO_KEYS
	if (numlock)
	{
		// don't map function keys to Apollo left keypad
		switch (code)
		{
		case 0x52: code = 0x75; break; // F1
		case 0x53: code = 0x76; break; // F2
		case 0x54: code = 0x77; break; // F3
		case 0x55: code = 0x78; break; // F4
		case 0x56: code = 0x79; break; // F5
		case 0x57: code = 0x7a; break; // F6
		case 0x58: code = 0x7b; break; // F7
		case 0x59: code = 0x7c; break; // F8
		case 0x5a: code = 0x7d; break; // F9
		case 0x5b: code = 0x74; break; // F0 = F10
		}
	}
#endif

	index = (code & 0x7f) * CODE_TABLE_ENTRY_SIZE;
	if (m_mode == KBD_MODE_0_COMPATIBILITY)
	{
		if (code & 0x80)
		{
			// skip up code in ASCII mode
		}
		else if (repeat > 0
				&& m_code_table[index + CODE_TABLE_AUTO_REPEAT_CODE] != Yes)
		{
			// skip repeat in ASCII mode
		}
		else if (ctrl)
		{
			key_code = m_code_table[index + CODE_TABLE_CONTROL_CODE];
		}
		else if (shift)
		{
			key_code = m_code_table[index + CODE_TABLE_SHIFTED_CODE];
		}
		else if (caps)
		{
			key_code = m_code_table[index + CODE_TABLE_CAPS_LOCK_CODE];
		}
		else
		{
			key_code = m_code_table[index + CODE_TABLE_UNSHIFTED_CODE];
		}
	}
	else
	{
		if (repeat > 0)
		{
			if (repeat == 1)
			{
				// auto repeat (but only for first scanned key)
				key_code = 0x7f;
			}
		}
		else if (code & 0x80)
		{
			key_code = m_code_table[index + CODE_TABLE_UP_CODE];
		}
		else
		{
			key_code = m_code_table[index + CODE_TABLE_DOWN_CODE];
		}
	}

	if (key_code != 0)
	{
		LOG2(("scan_code = 0x%02x key_code = 0x%04x",code, key_code));
		if (m_mode > KBD_MODE_1_KEYSTATE)
		{
			set_mode(KBD_MODE_1_KEYSTATE);
		}

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

void apollo_kbd_device::scan_keyboard()
{
	int x;
	int repeat = 0;

	static const char * const keynames[] = { "keyboard1", "keyboard2", "keyboard3", "keyboard4" };

	for (x = 0; x < 0x80; x++)
	{
		if (!(machine().root_device().ioport(keynames[x / 32])->read() & (1 << (x % 32))))
		{
			// no key pressed
			if (m_keyon[x] != 0)
			{
				// a key has been released
				push_scancode(0x80 + x, 0);
				m_keytime[x] = 0;
				m_keyon[x] = 0;
				m_last_pressed = 0;
				LOG2(("released key 0x%02x at time %d",x, m_keytime[x]));
			}
		}
		else if (m_keyon[x] == 0)
		{
			// a key has been pressed
			if (push_scancode(x, 0) > 0)
			{
				m_keytime[x] = m_mode == KBD_MODE_0_COMPATIBILITY ? m_delay : m_repeat;
				m_keyon[x] = 1;
				m_last_pressed = x;
				LOG2(("pushed key 0x%02x at time %d",x, m_keytime[x]));
			}
		}
		else if (m_last_pressed == x)
		{
			// a key is being held; adjust delay/repeat timers
			m_keytime[x] -= 5;
			if (m_keytime[x] <= 0)
			{
				push_scancode(x, ++repeat);
				m_keytime[x] = m_repeat;
			}
			LOG2(("holding key 0x%02x at time %d",x, m_keytime[x]));
		}
	}
}

TIMER_CALLBACK_MEMBER(apollo_kbd_device::kbd_scan_timer)
{
	scan_keyboard();

	// Note: we omit extra traffic while keyboard is in Compatibility mode
	if (m_device->m_mode != KBD_MODE_0_COMPATIBILITY)
	{
		m_mouse.read_mouse();
	}
}

UINT16 apollo_kbd_device::m_code_table[] = {
		/* Key   | Keycap      | Down | Up  |Unshifted|Shifted|Control|Caps Lock|Up Trans|Auto  */
		/* Number| Legend      | Code | Code|Code     | Code  | Code  |Code     | Code   |Repeat*/

		/* B14     ~ ' / ESC   */ 0x24, 0xA4, 0x60,     0x7E,   0x1E,   0x60,     NOP,     No,
		/* B1      ESC         */ 0x17, 0x97, 0x1B,     0x1B,   NOP,    0x1B,     NOP,     No,
		/* B2      ! 1         */ 0x18, 0x98, 0x31,     0x21,   NOP,    0x31,     NOP,     No,
		/* B3      @ 2         */ 0x19, 0x99, 0x32,     0x40,   NOP,    0x32,     NOP,     No,
		/* B4      # 3         */ 0x1A, 0x9A, 0x33,     0x23,   NOP,    0x33,     NOP,     No,
		/* B5      $ 4         */ 0x1B, 0x9B, 0x34,     0x24,   NOP,    0x34,     NOP,     No,
		/* B6      % 5         */ 0x1C, 0x9C, 0x35,     0x25,   NOP,    0x35,     NOP,     No,
		/* B7      ^ 6         */ 0x1D, 0x9D, 0x36,     0x5E,   NOP,    0x36,     NOP,     No,
		/* B8      & 7         */ 0x1E, 0x9E, 0x37,     0x26,   NOP,    0x37,     NOP,     No,
		/* B9      * 8         */ 0x1F, 0x9F, 0x38,     0x2A,   NOP,    0x38,     NOP,     No,
		/* B10     ( 9         */ 0x20, 0xA0, 0x39,     0x28,   NOP,    0x39,     NOP,     No,
		/* B11     ) 0         */ 0x21, 0xA1, 0x30,     0x29,   NOP,    0x30,     NOP,     No,
		/* B12     _ -         */ 0x22, 0xA2, 0x2D,     0x5F,   NOP,    0x2D,     NOP,     Yes,
		/* B13     + =         */ 0x23, 0xA3, 0x3D,     0x2B,   NOP,    0x3D,     NOP,     Yes,
		/* B14     ~ ' / BS    */ 0x24, 0xA4, 0x60,     0x7E,   0x1E,   0x60,     NOP,     No,
		/* B15     BACKSPACE   */ 0x25, 0xA5, 0xDE,     0xDE,   NOP,    0xDE,     NOP,     Yes,

		/* C1      TAB         */ 0x2C, 0xAC, 0xCA,     0xDA,   0xFA,   0xCA,     NOP,     No,
		/* C2      Q           */ 0x2D, 0xAD, 0x71,     0x51,   0x11,   0x51,     NOP,     No,
		/* C3      W           */ 0x2E, 0xAE, 0x77,     0x57,   0x17,   0x57,     NOP,     No,
		/* C4      E           */ 0x2F, 0xAF, 0x65,     0x45,   0x05,   0x45,     NOP,     No,
		/* C5      R           */ 0x30, 0xB0, 0x72,     0x52,   0x12,   0x52,     NOP,     No,
		/* C6      T           */ 0x31, 0xB1, 0x74,     0x54,   0x14,   0x54,     NOP,     No,
		/* C7      V           */ 0x32, 0xB2, 0x79,     0x59,   0x19,   0x59,     NOP,     No,
		/* C8      U           */ 0x33, 0xB3, 0x75,     0x55,   0x15,   0x55,     NOP,     No,
		/* C9      I           */ 0x34, 0xB4, 0x69,     0x49,   0x09,   0x49,     NOP,     No,
		/* C10     O           */ 0x35, 0xB5, 0x6F,     0x4F,   0x0F,   0x4F,     NOP,     No,
		/* C11     P           */ 0x36, 0xB6, 0x70,     0x50,   0x10,   0x50,     NOP,     No,
		/* C12     { [ / Ue    */ 0x37, 0xB7, 0x7B,     0x5B,   0x1B,   0x7B,     NOP,     No,
		/* C13     } ] / Oe    */ 0x38, 0xB8, 0x7D,     0x5D,   0x1D,   0x7D,     NOP,     No,
		/* D13     RETURN      */ 0x52, 0xD2, 0xCB,     0xDB,   NOP,    0xCB,     NOP,     No,

		/* D2      A           */ 0x46, 0xC6, 0x61,     0x41,   0x01,   0x41,     NOP,     No,
		/* D3      S           */ 0x47, 0xC7, 0x73,     0x53,   0x13,   0x53,     NOP,     No,
		/* D4      D           */ 0x48, 0xC8, 0x64,     0x44,   0x04,   0x44,     NOP,     No,
		/* D5      F           */ 0x49, 0xC9, 0x66,     0x46,   0x06,   0x46,     NOP,     No,
		/* D6      G           */ 0x4A, 0xCA, 0x67,     0x47,   0x07,   0x47,     NOP,     No,
		/* D7      H           */ 0x4B, 0xCB, 0x68,     0x48,   0x08,   0x48,     NOP,     No,
		/* D8      J           */ 0x4C, 0xCC, 0x6A,     0x4A,   0x0A,   0x4A,     NOP,     No,
		/* D9      K           */ 0x4D, 0xCD, 0x6B,     0x4B,   0x0B,   0x4B,     NOP,     No,
		/* D10     L           */ 0x4E, 0xCE, 0x6C,     0x4C,   0x0C,   0x4C,     NOP,     No,
		/* D11     : ;         */ 0x4F, 0xCF, 0x3B,     0x3A,   0xFB,   0x3B,     NOP,     No,
		/* D12     " ' / Ae    */ 0x50, 0xD0, 0x27,     0x22,   0xF8,   0x27,     NOP,     No,
		/* D14     ! \         */ 0x53, 0xD3, 0xC8,     0xC9,   NOP,    0xC8,     NOP,     No,

		/* E2      Z           */ 0x60, 0xE0, 0x7A,     0x5A,   0x1A,   0x5A,     NOP,     No,
		/* E3      X           */ 0x61, 0xE1, 0x78,     0x58,   0x18,   0x58,     NOP,     No,
		/* E4      C           */ 0x62, 0xE2, 0x63,     0x43,   0x03,   0x43,     NOP,     No,
		/* E5      V           */ 0x63, 0xE3, 0x76,     0x56,   0x16,   0x56,     NOP,     No,
		/* E6      8           */ 0x64, 0xE4, 0x62,     0x42,   0x02,   0x42,     NOP,     No,
		/* E7      N           */ 0x65, 0xE5, 0x6E,     0x4E,   0x0E,   0x4E,     NOP,     No,
		/* E8      M           */ 0x66, 0xE6, 0x6D,     0x4D,   0x0D,   0x4D,     NOP,     No,
		/* E9      < ,         */ 0x67, 0xE7, 0x2C,     0x3C,   NOP,    0x2C,     NOP,     No,
		/* E10     > .         */ 0x68, 0xE8, 0x2E,     0x3E,   NOP,    0x2E,     NOP,     Yes,
		/* E11     ? /         */ 0x69, 0xE9, 0xCC,     0xDC,   0xFC,   0xCC,     NOP,     No,

//      /* B14     ~ '         */ 0x24, 0xA4, 0x60,     0x7E,   0x1E,   0x60,     NOP,     No,
		/*         _           */ NOP,  NOP,  NOP,      NOP,    NOP,    NOP,      NOP,     NOP,
		/* F1      (space bar) */ 0x76, 0xF6, 0x20,     0x20,   0x20,   0x20,     NOP,     Yes,
		/* LC0     Home        */ 0x27, 0xA7, 0x84,     0x94,   0x84,   0x84,     0xA4,    No,
#if !MAP_APOLLO_KEYS
		/* C14     DELETE      */ 0x3A, 0xBA, 0x7F,     0x7F,   NOP,    0x7F,     NOP,     Yes,
#else
		/* E13     POP         */ 0x6C, 0xEC, 0x80,     0x90,   0x80,   0x80,     0xA0,    No,
#endif
		/* LF0     Roll Up     */ 0x72, 0xF2, 0x8D,     0x9D,   0x8D,   0x8D,     0xAD,    No,
		/* LF2     Roll Down   */ 0x74, 0xF4, 0x8F,     0x9F,   0x8F,   0x8F,     0xAF,    No,
		/* LC2     End         */ 0x29, 0xA9, 0x86,     0x96,   0x86,   0x86,     0xA6,    No,
		/* LE0     Cursor left */ 0x59, 0xD9, 0x8A,     0x9A,   0x9A,   0x9A,     0xAA,    Yes,
		/* LD1     Cursor Up   */ 0x41, 0xC1, 0x88,     0x98,   0x88,   0x88,     0xA8,    Yes,
		/* LE2     Cursor right*/ 0x5B, 0xDB, 0x8C,     0x9C,   0x8C,   0xBE,     0xAC,    Yes,
		/* LF1     Cursor down */ 0x73, 0xF3, 0x8E,     0x9E,   0x8E,   0x8E,     0xAE,    Yes,

#if !MAP_APOLLO_KEYS
		/*         Numpad CLR  */ NOP,  NOP,  NOP,      NOP,    NOP,    NOP,      NOP,     NOP,
		/*         Numpad /    */ NOP,  NOP,  NOP,      NOP,    NOP,    NOP,      NOP,     NOP,
		/*         Numpad *    */ NOP,  NOP,  NOP,      NOP,    NOP,    NOP,      NOP,     NOP,
		/* RD4     -           */ 0x58, 0xD8, 0xFE2D,   0xFE5F, NOP,    0xFE2D,   NOP,     No,
		/* RC1     7           */ 0x3C, 0xBC, 0xFE37,   0xFE26, NOP,    0xFE37,   NOP,     No,
		/* RC2     8           */ 0x3D, 0xBD, 0xFE38,   0xFE2A, NOP,    0xFE38,   NOP,     No,
		/* RC3     9           */ 0x3E, 0xBE, 0xFE39,   0xFE28, NOP,    0xFE39,   NOP,     No,
		/* RC4     +           */ 0x3F, 0xBF, 0xFE2B,   0xFE3D, NOP,    0xFE2B,   NOP,     No,
		/* RD1     4           */ 0x55, 0xD5, 0xFE34,   0xFE24, NOP,    0xFE34,   NOP,     No,
		/* RD2     5           */ 0x56, 0xD6, 0xFE35,   0xFE25, NOP,    0xFE35,   NOP,     No,
		/* RD3     6           */ 0x57, 0xD7, 0xFE36,   0xFE5E, NOP,    0xFE36,   NOP,     No,
		/*         Numpad =    */ NOP,  NOP,  NOP,      NOP,    NOP,    NOP,      NOP,     NOP,
		/* RE1     1           */ 0x6E, 0xEE, 0xFE31,   0xFE21, NOP,    0xFE31,   NOP,     No,
		/* RE2     2           */ 0x6F, 0xEF, 0xFE32,   0xFE40, NOP,    0xFE32,   NOP,     No,
		/* RE3     3           */ 0x70, 0xF0, 0xFE33,   0xFE23, NOP,    0xFE33,   NOP,     No,
		/* RF3     ENTER       */ 0x7C, 0xFC, 0xFECB,   0xFEDB, NOP,    0xFECB,   NOP,     No,
		/* RF1     0           */ 0x79, 0xF9, 0xFE30,   0xFE29, NOP,    0xFE30,   NOP,     No,
		/*         Numpad ,    */ NOP,  NOP,  NOP,      NOP,    NOP,    NOP,      NOP,     NOP,
		/* RF2     .           */ 0x7B, 0xFB, 0xFE2E,   0xFE2E, NOP,    0xFE2E,   NOP,     No,
		/* A0      F0          */ 0x04, 0x84, 0x1C,     0x5C,   0x7C,   0x1C,     0xBC,    No,
		/* A1      F1          */ 0x05, 0x85, 0xC0,     0xD0,   0xF0,   0xC0,     0xE0,    No,
		/* A2      F2          */ 0x06, 0x86, 0xC1,     0x01,   0xF1,   0xC1,     0xE1,    No,
		/* A3      F3          */ 0x07, 0x87, 0xC2,     0x02,   0xF2,   0xC2,     0xE2,    No,
		/* A4      F4          */ 0x08, 0x88, 0xC3,     0x03,   0xF3,   0xC3,     0xE3,    No,
		/* A5      F5          */ 0x09, 0x89, 0xC4,     0x04,   0xF4,   0xC4,     0xE4,    No,
		/* A6      F6          */ 0x0A, 0x8A, 0xC5,     0x05,   0xF5,   0xC5,     0xE5,    No,
		/* A7      F7          */ 0x0B, 0x8B, 0xC6,     0x06,   0xF6,   0xC6,     0xE6,    No,
		/* A8      F8          */ 0x0C, 0x8C, 0xC7,     0x07,   0xF7,   0xC7,     0xE7,    No,
		/* A9      F9          */ 0x0D, 0x8D, 0x1F,     0x2F,   0x3F,   0x1F,     0xBD,    No,
#else
		/* E13     POP         */ 0x6C, 0xEC, 0x80,     0x90,   0x80,   0x80,     0xA0,    No,
		/* LDO     [<-]        */ 0x40, 0xC0, 0x87,     0x97,   0x87,   0x87,     0xA7,    No,
		/* LD2     [->]        */ 0x42, 0xC2, 0x89,     0x99,   0x89,   0x89,     0xA9,    No,
		/* RD4     Numpad -    */ 0x58, 0xD8, 0xFE2D,   0xFE5F, NOP,    0xFE2D,   NOP,     No,
		/* LC0   7 Home        */ 0x27, 0xA7, 0x84,     0x94,   0x84,   0x84,     0xA4,    No,
		/* LD1   8 Cursor Up   */ 0x41, 0xC1, 0x88,     0x98,   0x88,   0x88,     0xA8,    Yes,
		/* LF0   9 Roll Up     */ 0x72, 0xF2, 0x8D,     0x9D,   0x8D,   0x8D,     0xAD,    No,
		/* RC4     Numpad +    */ 0x3F, 0xBF, 0xFE2B,   0xFE3D, NOP,    0xFE2B,   NOP,     No,
		/* LE0   4 Cursor left */ 0x59, 0xD9, 0x8A,     0x9A,   0x9A,   0x9A,     0xAA,    Yes,
		/* LE1     NEXT WINDOW */ 0x5A, 0xDA, 0x8B,     0x9B,   0x8B,   0x8B,     0xAB,    No,
		/* LE2   6 Cursor right*/ 0x5B, 0xDB, 0x8C,     0x9C,   0x8C,   0xBE,     0xAC,    Yes,
		/*         Numpad =    */ 0x7C, 0xFC, 0xFEC8,   0xFED8, NOP,    0xFECB,   NOP,     No,
		/* LC2   1 End         */ 0x29, 0xA9, 0x86,     0x96,   0x86,   0x86,     0xA6,    No,
		/* LF1   2 Cursor down */ 0x73, 0xF3, 0x8E,     0x9E,   0x8E,   0x8E,     0xAE,    Yes,
		/* LF2   3 Roll Down   */ 0x74, 0xF4, 0x8F,     0x9F,   0x8F,   0x8F,     0xAF,    No,
		/* RF3     ENTER       */ 0x7C, 0xFC, 0xFECB,   0xFEDB, NOP,    0xFECB,   NOP,     No,
		/* LE1     NEXT WINDOW */ 0x5A, 0xDA, 0x8B,     0x9B,   0x8B,   0x8B,     0xAB,    No,
		/*         Numpad ,    */ NOP,  NOP,  NOP,      NOP,    NOP,    NOP,      NOP,     NOP,
		/* E13     POP         */ 0x6C, 0xEC, 0x80,     0x90,   0x80,   0x80,     0xA0,    No,

		/* LC1  F1/SHELL/CMD   */ 0x28, 0xA8, 0x85,     0x95,   0x85,   0x85,     0xA5,    No,
		/* LB0  F2/CUT/COPY    */ 0x13, 0x93, 0xB0,     0xB4,   0xB0,   0xB0,     0xB8,    No,
		/* LB1  F3/UNDO/PASTE  */ 0x14, 0x94, 0xB1,     0xB5,   0xB1,   0xB1,     0xB9,    No,
		/* LB2  F4/MOVE/GROW   */ 0x15, 0x95, 0xB2,     0xB6,   0xB2,   0xB2,     0xBA,    No,

		/* LAO  F5/INS/MARK    */ 0x01, 0x81, 0x81,     0x91,   0x81,   0x81,     0xA1,    No,
		/* LA1  F6/LINE DEL    */ 0x02, 0x82, 0x82,     0x92,   0x82,   0x82,     0xA2,    No,
		/* LA2  F7/CHAR DEL    */ 0x03, 0x83, 0x83,     0x93,   0x83,   0x83,     0xA3,    Yes,
		/* RA0  F8/AGAIN       */ 0x0E, 0x8E, 0xCD,     0xE9,   0xCD,   0xCD,     0xED,    No,

		/* RA1  F9/READ        */ 0x0F, 0x8F, 0xCE,     0xEA,   0xCE,   0xCE,     0xEE,    No,
		/* RA2 F10/SAVE/EDIT   */ 0x10, 0x90, 0xCF,     0xEB,   0xCF,   0xCF,     0xEF,    No,
#endif
		/* RA3 F11/ABORT/EXIT  */ 0x11, 0x91, 0xDD,     0xEC,   0xD0,   0xD0,     0xFD,    No,
		/* RA4 F12/HELP/HOLD   */ 0x12, 0x92, 0xB3,     0xB7,   0xB3,   0xB3,     0xBB,    No,

		/* LE1     NEXT WINDOW */ 0x5A, 0xDA, 0x8B,     0x9B,   0x8B,   0x8B,     0xAB,    No,
		/* LE1     NEXT WINDOW */ 0x5A, 0xDA, 0x8B,     0x9B,   0x8B,   0x8B,     0xAB,    No,

		/* D1      CAPS LOCK   */ NOP,  NOP,  NOP,      NOP,    NOP,    NOP,      NOP,     NOP,
		/* E1      SHIFT       */ 0x5E, 0xDE, NOP,      NOP,    NOP,    NOP,      NOP,     NOP,
		/* DO      CTRL        */ 0x43, 0xC3, NOP,      NOP,    NOP,    NOP,      NOP,     NOP,
// FIXME: ALT swapped!
		/* ??      ALT_R       */ 0x77, 0xF7, 0xfe00,   NOP,    NOP,    NOP,      0xfe01,  No,
		/* ??      ALT_L       */ 0x75, 0xF5, 0xfe02,   NOP,    NOP,    NOP,      0xfe03,  No,
		/* E12     SHIFT       */ 0x6A, 0xEA, NOP,      NOP,    NOP,    NOP,      NOP,     NOP,
// not yet used:
		/* E0      REPEAT      */ 0x5D, 0xDD, NOP,      NOP,    NOP,    NOP,      NOP,     NOP,
		/*        CAPS LOCK LED*/ 0x7E, 0xFE, NOP,      NOP,    NOP,    NOP,      NOP,     NOP,
// german kbd:
		/* 0x68 B1_DE   _      */ 0x17, 0x97, 0x60,     0x7E,   0x1E,   0x60,     NOP,     No,
		/* 0x69 E1a_DE  <>     */ 0x5F, 0xDF, 0xBE,     0xBE,   NOP,    0xBE,     NOP,     No,
		/* 0x6a B14_DE  ESC    */ 0x16, 0x96, 0x1B,     0x1B,   NOP,    0x1B,     NOP,     No,
		/* 0x6b D14_DE  # \    */ 0x51, 0xD1, 0xC8,     0xC9,   NOP,    0xC8,     NOP,     No,

		/* 0x6c NPG     NP (   */ 0x3F, 0xBF, 0xFE28,   0xFE0E, NOP,    0xFE28,   NOP,     No,
		/* 0x6d NPF     NP )   */ 0x58, 0xD8, 0xFE29,   0xFE0F, NOP,    0xFE29,   NOP,     No,
		/* 0x6e NPD     NP +   */ 0x3B, 0xBB, 0xFE2B,   0xFE26, NOP,    0xFE2B,   NOP,     No,
		/* 0x6f NPC     NP -   */ 0x54, 0xD4, 0xFE2D,   0xFE7E, NOP,    0xFE2D,   NOP,     No,
		/* 0x70 NPB     NP *   */ 0x6D, 0xED, 0xFE2A,   0xFE21, NOP,    0xFE2A,   NOP,     No,
		/* 0x71 NPA     NP /   */ 0x78, 0xF8, 0xFECC,   0xFEC8, NOP,    0xFECC,   NOP,     No,
		/* 0x72 NPP     NP .   */ 0x7B, 0xFB, 0xFE2E,   0xFE2C, NOP,    0xFE2E,   NOP,     No,
		/* 0x73 NPE     ENTER  */ 0x7C, 0xFC, 0xFECB,   0xFE3D, NOP,    0xFECB,   NOP,     No,

		/* 0x74 A0      F0     */ 0x04, 0x84, 0x1C,     0x5C,   0x7C,   0x1C,     0xBC,    No,
		/* 0x75 A1      F1     */ 0x05, 0x85, 0xC0,     0xD0,   0xF0,   0xC0,     0xE0,    No,
		/* 0x76 A2      F2     */ 0x06, 0x86, 0xC1,     0x01,   0xF1,   0xC1,     0xE1,    No,
		/* 0x77 A3      F3     */ 0x07, 0x87, 0xC2,     0x02,   0xF2,   0xC2,     0xE2,    No,
		/* 0x78 A4      F4     */ 0x08, 0x88, 0xC3,     0x03,   0xF3,   0xC3,     0xE3,    No,
		/* 0x79 A5      F5     */ 0x09, 0x89, 0xC4,     0x04,   0xF4,   0xC4,     0xE4,    No,
		/* 0x7a A6      F6     */ 0x0A, 0x8A, 0xC5,     0x05,   0xF5,   0xC5,     0xE5,    No,
		/* 0x7b A7      F7     */ 0x0B, 0x8B, 0xC6,     0x06,   0xF6,   0xC6,     0xE6,    No,
		/* 0x7c A8      F8     */ 0x0C, 0x8C, 0xC7,     0x07,   0xF7,   0xC7,     0xE7,    No,
		/* 0x7d A9      F9     */ 0x0D, 0x8D, 0x1F,     0x2F,   0x3F,   0x1F,     0xBD,    No,

		/* Key   | Keycap      | Down | Up  |Unshifted|Shifted|Control|Caps Lock|Up Trans|Auto  */
		/* Number| Legend      | Code | Code|Code     | Code  | Code  |Code     | Code   |Repeat*/
};

INPUT_PORTS_START( apollo_kbd )

	PORT_START( "keyboard1" )
	PORT_BIT( 0x00000001, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("~ '") PORT_CODE(KEYCODE_TILDE) /* ESC */
	PORT_BIT( 0x00000002, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("ESC") PORT_CODE(KEYCODE_ESC) /* ESC */
	PORT_BIT( 0x00000004, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("1 !") PORT_CODE(KEYCODE_1) /* 1 ! */
	PORT_BIT( 0x00000008, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("2 \"") PORT_CODE(KEYCODE_2) /* 2 " */
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
//??
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
//  PORT_BIT( 0x80000000, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Numpad CLR")  PORT_CODE(KEYCODE_NUMLOCK) /* CLR */
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
//  PORT_BIT( 0x80000000, 0x0000, IPT_UNUSED )
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

// FIXME:
// PORT_SENSITIVITY(50) would be perfect for SDL2
// PORT_SENSITIVITY(200) would be perfect for SDL1.2

	PORT_START("mouse2")  // X-axis
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_X) PORT_SENSITIVITY(100) PORT_KEYDELTA(0) PORT_PLAYER(1)

	PORT_START("mouse3")  // Y-axis
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_Y) PORT_SENSITIVITY(100) PORT_KEYDELTA(0) PORT_PLAYER(1)

INPUT_PORTS_END
