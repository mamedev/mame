// license:BSD-3-Clause
// copyright-holders:Antoine Mine
/**********************************************************************

  Copyright (C) Antoine Mine' 2006

  Thomson TO8 built-in keyboard & TO9 detached keyboard

**********************************************************************/

#include "emu.h"
#include "to_kbd.h"

#include "cpu/m6805/m68705.h"

#define LOG_KBD    (1U << 1)
#define LOG_ERRORS (1U << 2)

#define VERBOSE (LOG_KBD | LOG_ERRORS)
#include "logmacro.h"


// device type definitions
DEFINE_DEVICE_TYPE(TO8_KEYBOARD, to8_keyboard_device, "to8_kbd", "Thomson TO8 keyboard")
DEFINE_DEVICE_TYPE(TO9_KEYBOARD, to9_keyboard_device, "to9_kbd", "Thomson TO9 keyboard")
DEFINE_DEVICE_TYPE(TO9P_KEYBOARD, to9p_keyboard_device, "to9p_kbd", "Thomson TO9+ keyboard")

to8_keyboard_device::to8_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, TO8_KEYBOARD, tag, owner, clock)
	, m_data_cb(*this)
	, m_io_keyboard(*this, "keyboard.%u", 0)
	, m_caps_led(*this, "led0")
{
}

to9_keyboard_device::to9_keyboard_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_irq_cb(*this)
	, m_io_keyboard(*this, "keyboard.%u", 0)
	, m_io_mouse_x(*this, "mouse_x")
	, m_io_mouse_y(*this, "mouse_y")
	, m_io_mouse_button(*this, "mouse_button")
	, m_caps_led(*this, "led0")
{
}

to9_keyboard_device::to9_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: to9_keyboard_device(mconfig, TO9_KEYBOARD, tag, owner, clock)
{
}

to9p_keyboard_device::to9p_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: to9_keyboard_device(mconfig, TO9P_KEYBOARD, tag, owner, clock)
{
}

void to8_keyboard_device::device_add_mconfig(machine_config &config)
{
	//MC6804P2(config, "mcu", 11_MHz_XTAL).set_disable();
}

void to9_keyboard_device::device_add_mconfig(machine_config &config)
{
	M6805U2(config, "mcu", 4_MHz_XTAL).set_disable(); // 40 pins, actual model unknown
}

void to9p_keyboard_device::device_add_mconfig(machine_config &config)
{
	M6805P2(config, "mcu", 4_MHz_XTAL).set_disable();
}

ROM_START(to8_kbd)
	ROM_REGION(0x440, "mcu", 0)
	ROM_LOAD("ef6804p2p_clav--to8.bin", 0x000, 0x440, NO_DUMP)
ROM_END

ROM_START(to9_kbd)
	ROM_REGION(0x1000, "mcu", 0)
	ROM_LOAD("6805.bin", 0x0000, 0x1000, NO_DUMP)
ROM_END

ROM_START(to9p_kbd)
	ROM_REGION(0x800, "mcu", 0)
	ROM_LOAD("6805p2.bin", 0x000, 0x800, NO_DUMP)
ROM_END

const tiny_rom_entry *to8_keyboard_device::device_rom_region() const
{
	return ROM_NAME(to8_kbd);
}

const tiny_rom_entry *to9_keyboard_device::device_rom_region() const
{
	return ROM_NAME(to9_kbd);
}

const tiny_rom_entry *to9p_keyboard_device::device_rom_region() const
{
	return ROM_NAME(to9p_kbd);
}


/* ------------ inputs   ------------ */

#define KEY(pos,name,key)                   \
	PORT_BIT  ( 1<<(pos), IP_ACTIVE_LOW, IPT_KEYBOARD ) \
	PORT_NAME ( name )                  \
	PORT_CODE ( KEYCODE_##key )

static INPUT_PORTS_START ( to8_keyboard )
	PORT_START ( "keyboard.0" )
		KEY ( 0, "F2 F7", F2 )           PORT_CHAR(UCHAR_MAMEKEY(F2)) PORT_CHAR(UCHAR_MAMEKEY(F7))
		KEY ( 1, "_ 6", 6 )              PORT_CHAR('_') PORT_CHAR('6')
		KEY ( 2, "Y", Y )                PORT_CHAR('Y')
		KEY ( 3, "H", H )                PORT_CHAR('H')
		KEY ( 4, u8"\u2191", UP )        PORT_CHAR(UCHAR_MAMEKEY(UP))
		KEY ( 5, u8"\u2192", RIGHT )     PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
		KEY ( 6, "Home Clear", HOME )    PORT_CHAR(UCHAR_MAMEKEY(HOME)) PORT_CHAR(UCHAR_MAMEKEY(ESC))
		KEY ( 7, "N", N )                PORT_CHAR('N')
	PORT_START ( "keyboard.1" )
		KEY ( 0, "F3 F8", F3 )           PORT_CHAR(UCHAR_MAMEKEY(F3)) PORT_CHAR(UCHAR_MAMEKEY(F8))
		KEY ( 1, "( 5", 5 )              PORT_CHAR('(') PORT_CHAR('5')
		KEY ( 2, "T", T )                PORT_CHAR('T')
		KEY ( 3, "G", G )                PORT_CHAR('G')
		KEY ( 4, "= +", EQUALS )         PORT_CHAR('=') PORT_CHAR('+')
		KEY ( 5, "\u2190", LEFT )        PORT_CHAR(UCHAR_MAMEKEY(LEFT))
		KEY ( 6, "Insert", INSERT )      PORT_CHAR(UCHAR_MAMEKEY(INSERT))
		KEY ( 7, "B", B )                PORT_CHAR('B')
	PORT_START ( "keyboard.2" )
		KEY ( 0, "F4 F9", F4 )           PORT_CHAR(UCHAR_MAMEKEY(F4)) PORT_CHAR(UCHAR_MAMEKEY(F9))
		KEY ( 1, "' 4", 4 )              PORT_CHAR('\'') PORT_CHAR('4')
		KEY ( 2, "R", R )                PORT_CHAR('R')
		KEY ( 3, "F", F )                PORT_CHAR('F')
		KEY ( 4, "Accent", END )         PORT_CHAR(UCHAR_MAMEKEY(END))
		KEY ( 5, "Keypad 1", 1_PAD )     PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
		KEY ( 6, "Delete Backspace", DEL ) PORT_CHAR(8) PORT_CHAR(UCHAR_MAMEKEY(BACKSPACE))
		KEY ( 7, "V", V )                PORT_CHAR('V')
	PORT_START ( "keyboard.3" )
		KEY ( 0, "F5 F10", F5 )          PORT_CHAR(UCHAR_MAMEKEY(F5)) PORT_CHAR(UCHAR_MAMEKEY(F10))
		KEY ( 1, "\" 3", 3 )             PORT_CHAR('"') PORT_CHAR('3')
		KEY ( 2, "E", E )                PORT_CHAR('E')
		KEY ( 3, "D", D )                PORT_CHAR('D')
		KEY ( 4, "Keypad 7", 7_PAD )     PORT_CHAR(UCHAR_MAMEKEY(7_PAD))
		KEY ( 5, "Keypad 4", 4_PAD )     PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
		KEY ( 6, "Keypad 0", 0_PAD )     PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
		KEY ( 7, "C \136", C )           PORT_CHAR('C')
	PORT_START ( "keyboard.4" )
		KEY ( 0, "F1 F6", F1 )           PORT_CHAR(UCHAR_MAMEKEY(F1)) PORT_CHAR(UCHAR_MAMEKEY(F6))
		KEY ( 1, u8"é 2", 2 )            PORT_CHAR( 0xe9 ) PORT_CHAR('2')
		KEY ( 2, "Z", Z )                PORT_CHAR('Z')
		KEY ( 3, "S", S )                PORT_CHAR('S')
		KEY ( 4, "Keypad 8", 8_PAD )     PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
		KEY ( 5, "Keypad 2", 2_PAD )     PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
		KEY ( 6, "Keypad .", DEL_PAD )   PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))
		KEY ( 7, "X", X )                PORT_CHAR('X')
	PORT_START ( "keyboard.5" )
		KEY ( 0, "# @", TILDE )          PORT_CHAR('#') PORT_CHAR('@')
		KEY ( 1, "* 1", 1 )              PORT_CHAR('*') PORT_CHAR('1')
		KEY ( 2, "A \140", A )           PORT_CHAR('A')
		KEY ( 3, "Q", Q )                PORT_CHAR('Q')
		KEY ( 4, "[ {", QUOTE )          PORT_CHAR('[') PORT_CHAR('{')
		KEY ( 5, "Keypad 5", 5_PAD )     PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
		KEY ( 6, "Keypad 6", 6_PAD )     PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
		KEY ( 7, "W", W )                PORT_CHAR('W')
	PORT_START ( "keyboard.6" )
		KEY ( 0, "Stop", TAB )           PORT_CHAR(27)
		KEY ( 1, u8"è 7", 7 )            PORT_CHAR( 0xe8 ) PORT_CHAR('7')
		KEY ( 2, "U", U )                PORT_CHAR('U')
		KEY ( 3, "J", J )                PORT_CHAR('J')
		KEY ( 4, "Space", SPACE )        PORT_CHAR(' ')
		KEY ( 5, "Keypad 9", 9_PAD )     PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
		KEY ( 6, "Keypad Enter", ENTER_PAD ) PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD))
		KEY ( 7, ", ?", COMMA )          PORT_CHAR(',') PORT_CHAR('?')
	PORT_START ( "keyboard.7" )
		KEY ( 0, "Control", LCONTROL )   PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
		KEY ( 1, "! 8", 8 )              PORT_CHAR('!') PORT_CHAR('8')
		KEY ( 2, "I", I )                PORT_CHAR('I')
		KEY ( 3, "K", K )                PORT_CHAR('K')
		KEY ( 4, "$ &", CLOSEBRACE )     PORT_CHAR('$') PORT_CHAR('&')
		KEY ( 5, u8"\u2193", DOWN )      PORT_CHAR(UCHAR_MAMEKEY(DOWN))
		KEY ( 6, "] }", BACKSLASH )      PORT_CHAR(']') PORT_CHAR('}')
		KEY ( 7, "; .", STOP )           PORT_CHAR(';') PORT_CHAR('.')
	PORT_START ( "keyboard.8" )
		KEY ( 0, "Caps-Lock", CAPSLOCK ) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
		KEY ( 1, u8"ç 9", 9 )            PORT_CHAR( 0xe7 ) PORT_CHAR('9')
		KEY ( 2, "O", O )                PORT_CHAR('O')
		KEY ( 3, "L", L )                PORT_CHAR('L')
		KEY ( 4, "- \\", BACKSPACE )     PORT_CHAR('-') PORT_CHAR('\\')
		KEY ( 5, u8"ù %", COLON )        PORT_CHAR( 0xf9 ) PORT_CHAR('%')
		KEY ( 6, "Enter", ENTER )        PORT_CHAR(13)
		KEY ( 7, ": /", SLASH )          PORT_CHAR(':') PORT_CHAR('/')
	PORT_START ( "keyboard.9" )
		KEY ( 0, "Shift", LSHIFT )  PORT_CODE ( KEYCODE_RSHIFT ) PORT_CHAR(UCHAR_SHIFT_1)
		KEY ( 1, u8"à 0", 0 )            PORT_CHAR( 0xe0 ) PORT_CHAR('0')
		KEY ( 2, "P", P )                PORT_CHAR('P')
		KEY ( 3, "M", M )                PORT_CHAR('M')
		KEY ( 4, u8") °", MINUS )        PORT_CHAR(')') PORT_CHAR( 0xb0 )
		KEY ( 5, u8"^ ¨", OPENBRACE )    PORT_CHAR('^') PORT_CHAR( 0xa8 )
		KEY ( 6, "Keypad 3", 3_PAD )     PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
		KEY ( 7, "> <", BACKSLASH2 )     PORT_CHAR('>') PORT_CHAR('<')
INPUT_PORTS_END

static INPUT_PORTS_START ( to9_keyboard )
	PORT_INCLUDE( to8_keyboard )

	PORT_START ( "mouse_x" )
	PORT_BIT ( 0xffff, 0x00, IPT_MOUSE_X )
	PORT_NAME ( "Mouse X" )
	PORT_SENSITIVITY ( 150 )
	PORT_PLAYER (1)

	PORT_START ( "mouse_y" )
	PORT_BIT ( 0xffff, 0x00, IPT_MOUSE_Y )
	PORT_NAME ( "Mouse Y" )
	PORT_SENSITIVITY ( 150 )
	PORT_PLAYER (1)

	PORT_START ( "mouse_button" )
	PORT_BIT ( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_NAME ( "Left Mouse Button" )
	PORT_CODE( MOUSECODE_BUTTON1 )
	PORT_BIT ( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_NAME ( "Right Mouse Button" )
INPUT_PORTS_END

ioport_constructor to8_keyboard_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(to8_keyboard);
}

ioport_constructor to9_keyboard_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(to9_keyboard);
}

ioport_constructor to9p_keyboard_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(to8_keyboard);
}


/* ------------ keyboard (6804) ------------ */

/* The 6804 chip scans the keyboard and sends keycodes to the 6809.
   Data is serialized using variable pulse length encoding.
   Unlike the TO9, there is no decoding chip on the 6809 side, only
   1-bit PIA ports (6821 & 6846). The 6809 does the decoding.

   We do not emulate the 6804 but pass serialized data directly through the
   PIA ports.

   Note: if we conform to the (scarce) documentation the CPU tend to lock
   waiting for keyboard input.
   The protocol documentation is pretty scarce and does not account for these
   behaviors!
   The emulation code contains many hacks (delays, timeouts, spurious
   pulses) to improve the stability.
   This works well, but is not very accurate.
*/



/* polling interval */
#define TO8_KBD_POLL_PERIOD  attotime::from_msec( 1 )

/* first and subsequent repeat periods, in TO8_KBD_POLL_PERIOD units */
#define TO8_KBD_REPEAT_DELAY  800 /* 800 ms */
#define TO8_KBD_REPEAT_PERIOD  70 /*  70 ms */

/* timeout waiting for CPU */
#define TO8_KBD_TIMEOUT  attotime::from_msec( 100 )



/* quick keyboard scan */
int to8_keyboard_device::ktest_r()
{
	int line, bit;
	uint8_t port;

	for ( line = 0; line < 10; line++ )
	{
		port = m_io_keyboard[line]->read();

		if ( line == 7 || line == 9 )
			port |= 1; /* shift & control */

		for ( bit = 0; bit < 8; bit++ )
		{
			if ( ! (port & (1 << bit)) )
				return 1;
		}
	}

	return 0;
}



/* keyboard scan & return keycode (or -1) */
int to8_keyboard_device::get_key()
{
	int control = (m_io_keyboard[7]->read() & 1) ? 0 : 0x100;
	int shift   = (m_io_keyboard[9]->read() & 1) ? 0 : 0x080;
	int key = -1, line, bit;
	uint8_t port;

	for ( line = 0; line < 10; line++ )
	{
		port = m_io_keyboard[line]->read();

		if ( line == 7 || line == 9 )
			port |= 1; /* shift & control */

		/* TODO: correct handling of simultaneous keystokes:
		   return the new key preferably & disable repeat
		*/
		for ( bit = 0; bit < 8; bit++ )
		{
			if ( ! (port & (1 << bit)) )
				key = line * 8 + bit;
		}
	}

	if ( key == -1 )
	{
		m_kbd_last_key = 0xff;
		m_kbd_key_count = 0;
		return -1;
	}
	else if ( key == 64 )
	{
		/* caps lock */
		if ( m_kbd_last_key == key )
			return -1; /* no repeat */
		m_kbd_last_key = key;
		m_kbd_caps = !m_kbd_caps;
		if ( m_kbd_caps )
			key |= 0x080; /* auto-shift */
		m_caps_led = !m_kbd_caps;
		return key;
	}
	else if ( key == m_kbd_last_key )
	{
		/* repeat */
		m_kbd_key_count++;
		if ( m_kbd_key_count < TO8_KBD_REPEAT_DELAY || (m_kbd_key_count - TO8_KBD_REPEAT_DELAY) % TO8_KBD_REPEAT_PERIOD )
			return -1;
		return key | shift | control;
	}
	else
	{
		m_kbd_last_key = key;
		m_kbd_key_count = 0;
		return key | shift | control;
	}
}


/* steps:
   0     = idle, key polling
   1     = wait for ack to go down (key to send)
   99-117 = key data transmit
   91-117 = signal
   255    = timeout
*/

/* keyboard automaton */
void to8_keyboard_device::timer_func()
{
	attotime d;

	LOGMASKED(LOG_KBD, "%f timer_func: step=%i ack=%i data=$%03X\n", machine().time().as_double(), m_kbd_step, m_kbd_ack, m_kbd_data);

	if( ! m_kbd_step )
	{
		/* key polling */
		int k = get_key();
		/* if not in transfer, send pulse from time to time
		   (helps avoiding CPU lock)
		*/
		if ( ! m_kbd_ack )
			m_data_cb(0);
		m_data_cb(1);

		if ( k == -1 )
			d = TO8_KBD_POLL_PERIOD;
		else
		{
			/* got key! */
			LOGMASKED(LOG_KBD, "timer_func: got key $%03X\n", k);
			m_kbd_data = k;
			m_kbd_step = 1;
			d = attotime::from_usec( 100 );
		}
	}
	else if ( m_kbd_step == 255 )
	{
		/* timeout */
		m_kbd_last_key = 0xff;
		m_kbd_key_count = 0;
		m_kbd_step = 0;
		m_data_cb(1);
		d = TO8_KBD_POLL_PERIOD;
	}
	else if ( m_kbd_step == 1 )
	{
		/* schedule timeout waiting for ack to go down */
		m_data_cb(0);
		m_kbd_step = 255;
		d = TO8_KBD_TIMEOUT;
	}
	else if ( m_kbd_step == 117 )
	{
		/* schedule timeout  waiting for ack to go up */
		m_data_cb(0);
		m_kbd_step = 255;
		d = TO8_KBD_TIMEOUT;
	}
	else if ( m_kbd_step & 1 )
	{
		/* send silence between bits */
		m_data_cb(0);
		d = attotime::from_usec( 100 );
		m_kbd_step++;
	}
	else
	{
		/* send bit */
		int bpos = 8 - ( (m_kbd_step - 100) / 2);
		int bit = (m_kbd_data >> bpos) & 1;
		m_data_cb(1);
		d = attotime::from_usec( bit ? 56 : 38 );
		m_kbd_step++;
	}
	m_kbd_timer->adjust(d);
}



TIMER_CALLBACK_MEMBER(to8_keyboard_device::timer_cb)
{
	timer_func();
}



/* cpu <-> keyboard hand-check */
void to8_keyboard_device::set_ack( int data )
{
	if ( data == m_kbd_ack )
		return;
	m_kbd_ack = data;

	if ( data )
	{
		double len = m_kbd_signal->elapsed( ).as_double() * 1000. - 2.;
		LOGMASKED(LOG_KBD, "%f set_ack: CPU end ack, len=%f\n", machine().time().as_double(), len);
		if ( m_kbd_data == 0xfff )
		{
			/* end signal from CPU */
			if ( len >= 0.6 && len <= 0.8 )
			{
				LOG("%f set_ack: INIT signal\n", machine().time().as_double());
				m_kbd_last_key = 0xff;
				m_kbd_key_count = 0;
				m_kbd_caps = 1;
				/* send back signal: TODO returned codes ? */
				m_kbd_data = 0;
				m_kbd_step = 0;
				m_kbd_timer->adjust(attotime::from_msec( 1 ));
			}
			else
			{
				m_kbd_step = 0;
				m_kbd_timer->adjust(TO8_KBD_POLL_PERIOD);
				if ( len >= 1.2 && len <= 1.4 )
				{
					LOG("%f set_ack: CAPS on signal\n", machine().time().as_double());
					m_kbd_caps = 1;
				}
				else if ( len >= 1.8 && len <= 2.0 )
				{
					LOG("%f set_ack: CAPS off signal\n", machine().time().as_double());
					m_kbd_caps = 0;
				}
			}
			m_caps_led = !m_kbd_caps;
		}
		else
		{
			/* end key transmission */
			m_kbd_step = 0;
			m_kbd_timer->adjust(TO8_KBD_POLL_PERIOD);
		}
	}

	else
	{
		if ( m_kbd_step == 255 )
		{
			/* CPU accepts key */
			m_kbd_step = 99;
			m_kbd_timer->adjust(attotime::from_usec( 400 ));
		}
		else
		{
			/* start signal from CPU */
			m_kbd_data = 0xfff;
			m_kbd_step = 91;
			m_kbd_timer->adjust(attotime::from_usec( 400 ));
			m_kbd_signal->adjust(attotime::never);
		}
		LOGMASKED(LOG_KBD, "%f set_ack: CPU ack, data=$%03X\n", machine().time().as_double(), m_kbd_data);
	}
}



void to8_keyboard_device::device_reset()
{
	m_kbd_last_key = 0xff;
	m_kbd_key_count = 0;
	m_kbd_step = 0;
	m_kbd_data = 0;
	m_kbd_ack = 1;
	m_kbd_caps = 1;
	m_caps_led = !m_kbd_caps;
	timer_func();
}



void to8_keyboard_device::device_start()
{
	m_caps_led.resolve();

	m_kbd_timer = timer_alloc(FUNC(to8_keyboard_device::timer_cb), this);
	m_kbd_signal = machine().scheduler().timer_alloc(timer_expired_delegate());
	save_item(NAME(m_kbd_ack));
	save_item(NAME(m_kbd_data));
	save_item(NAME(m_kbd_step));
	save_item(NAME(m_kbd_last_key));
	save_item(NAME(m_kbd_key_count));
	save_item(NAME(m_kbd_caps));
}



/* ------------ 6850 defines ------------ */

#define ACIA_6850_RDRF  0x01    /* Receive data register full */
#define ACIA_6850_TDRE  0x02    /* Transmit data register empty */
#define ACIA_6850_dcd   0x04    /* Data carrier detect, active low */
#define ACIA_6850_cts   0x08    /* Clear to send, active low */
#define ACIA_6850_FE    0x10    /* Framing error */
#define ACIA_6850_OVRN  0x20    /* Receiver overrun */
#define ACIA_6850_PE    0x40    /* Parity error */
#define ACIA_6850_irq   0x80    /* Interrupt request, active low */



/* ------------ keyboard (6850 ACIA + 6805 CPU) ------------ */

/* The 6805 chip scans the keyboard and sends ASCII codes to the 6909.
   Data between the 6809 and 6805 is serialized at 9600 bauds.
   On the 6809 side, a 6850 ACIA is used.
   We do not emulate the seral line but pass bytes directly between the
   keyboard and the 6850 registers.
   Note that the keyboard protocol uses the parity bit as an extra data bit.
*/



/* normal mode: polling interval */
#define TO9_KBD_POLL_PERIOD  attotime::from_msec( 10 )

/* peripheral mode: time between two bytes, and after last byte */
#define TO9_KBD_BYTE_SPACE   attotime::from_usec( 300 )
#define TO9_KBD_END_SPACE    attotime::from_usec( 9100 )

/* first and subsequent repeat periods, in TO9_KBD_POLL_PERIOD units */
#define TO9_KBD_REPEAT_DELAY  80 /* 800 ms */
#define TO9_KBD_REPEAT_PERIOD  7 /*  70 ms */



/* quick keyboard scan */
int to9_keyboard_device::ktest_r()
{
	int line, bit;
	uint8_t port;

	for ( line = 0; line < 10; line++ )
	{
		port = m_io_keyboard[line]->read();

		if ( line == 7 || line == 9 )
			port |= 1; /* shift & control */

		for ( bit = 0; bit < 8; bit++ )
		{
			if ( ! (port & (1 << bit)) )
				return 1;
		}
	}
	return 0;
}



void to9_keyboard_device::update_irq()
{
	if ( (m_kbd_intr & 4) && (m_kbd_status & ACIA_6850_RDRF) )
		m_kbd_status |= ACIA_6850_irq; /* byte received interrupt */

	if ( (m_kbd_intr & 4) && (m_kbd_status & ACIA_6850_OVRN) )
		m_kbd_status |= ACIA_6850_irq; /* overrun interrupt */

	if ( (m_kbd_intr & 3) == 1 && (m_kbd_status & ACIA_6850_TDRE) )
		m_kbd_status |= ACIA_6850_irq; /* ready to transmit interrupt */

	m_irq_cb( (m_kbd_status & ACIA_6850_irq) ? 1 : 0 );
}



uint8_t to9_keyboard_device::kbd_acia_r(offs_t offset)
{
	/* ACIA 6850 registers */

	switch ( offset )
	{
	case 0: /* get status */
		/* bit 0:     data received */
		/* bit 1:     ready to transmit data (always 1) */
		/* bit 2:     data carrier detect (ignored) */
		/* bit 3:     clear to send (ignored) */
		/* bit 4:     framing error (ignored) */
		/* bit 5:     overrun */
		/* bit 6:     parity error */
		/* bit 7:     interrupt */

		LOG("%s %f kbd_acia_r: status $%02X (rdrf=%i, tdre=%i, ovrn=%i, pe=%i, irq=%i)\n",
				machine().describe_context(), machine().time().as_double(), m_kbd_status,
				(m_kbd_status & ACIA_6850_RDRF) ? 1 : 0,
				(m_kbd_status & ACIA_6850_TDRE) ? 1 : 0,
				(m_kbd_status & ACIA_6850_OVRN) ? 1 : 0,
				(m_kbd_status & ACIA_6850_PE) ? 1 : 0,
				(m_kbd_status & ACIA_6850_irq) ? 1 : 0 );
		return m_kbd_status;

	case 1: /* get input data */
		if ( !machine().side_effects_disabled() )
		{
			m_kbd_status &= ~(ACIA_6850_irq | ACIA_6850_PE);
			if ( m_kbd_overrun )
				m_kbd_status |= ACIA_6850_OVRN;
			else
				m_kbd_status &= ~(ACIA_6850_OVRN | ACIA_6850_RDRF);
			m_kbd_overrun = 0;
			LOGMASKED(LOG_KBD, "%s %f kbd_acia_r: read data $%02X\n", machine().describe_context(), machine().time().as_double(), m_kbd_in);
			update_irq();
		}
		return m_kbd_in;

	default:
		LOGMASKED(LOG_ERRORS, "%s kbd_acia_r: invalid offset %i\n", machine().describe_context(), offset);
		return 0;
	}
}



void to9_keyboard_device::kbd_acia_w(offs_t offset, uint8_t data)
{
	/* ACIA 6850 registers */

	switch ( offset )
	{
	case 0: /* set control */
		/* bits 0-1: clock divide (ignored) or reset */
		if ( (data & 3) == 3 )
		{
			/* reset */
			m_kbd_overrun = 0;
			m_kbd_status = ACIA_6850_TDRE;
			m_kbd_intr = 0;
			LOGMASKED(LOG_KBD, "%s %f kbd_acia_w: reset (data=$%02X)\n", machine().describe_context(), machine().time().as_double(), data);
		}
		else
		{
			/* bits 2-4: parity */
			if ( (data & 0x18) == 0x10 )
				m_kbd_parity = 2;
			else
				m_kbd_parity = (data >> 2) & 1;
			/* bits 5-6: interrupt on transmit */
			/* bit 7:    interrupt on receive */
			m_kbd_intr = data >> 5;

			LOGMASKED(LOG_KBD, "%s %f kbd_acia_w: set control to $%02X (parity=%i, intr in=%i out=%i)\n",
					machine().describe_context(), machine().time().as_double(),
					data, m_kbd_parity, m_kbd_intr >> 2,
					(m_kbd_intr & 3) ? 1 : 0);
		}
		update_irq();
		break;

	case 1: /* output data */
		m_kbd_status &= ~(ACIA_6850_irq | ACIA_6850_TDRE);
		update_irq();
		/* TODO: 1 ms delay here ? */
		m_kbd_status |= ACIA_6850_TDRE; /* data transmit ready again */
		update_irq();

		switch ( data )
		{
		case 0xF8:
			/* reset */
			m_kbd_caps = 1;
			m_kbd_periph = 0;
			m_kbd_pad = 0;
			break;

		case 0xF9: m_kbd_caps = 1;   break;
		case 0xFA: m_kbd_caps = 0;   break;
		case 0xFB: m_kbd_pad = 1;    break;
		case 0xFC: m_kbd_pad = 0;    break;
		case 0xFD: m_kbd_periph = 1; break;
		case 0xFE: m_kbd_periph = 0; break;

		default:
			LOGMASKED(LOG_ERRORS, "%s %f kbd_acia_w: unknown kbd command %02X\n", machine().describe_context(), machine().time().as_double(), data);
		}

		m_caps_led = !m_kbd_caps;

		LOG("%s %f kbd_acia_w: kbd command %02X (caps=%i, pad=%i, periph=%i)\n",
				machine().describe_context(), machine().time().as_double(), data,
				m_kbd_caps, m_kbd_pad, m_kbd_periph);

		break;

	default:
		LOGMASKED(LOG_ERRORS, "%s kbd_acia_w: invalid offset %i (data=$%02X) \n", machine().describe_context(), offset, data);
	}
}



/* send a key to the CPU, 8-bit + parity bit (0=even, 1=odd)
   note: parity is not used as a checksum but to actually transmit a 9-th bit
   of information!
*/
void to9_keyboard_device::send( uint8_t data, int parity )
{
	if ( m_kbd_status & ACIA_6850_RDRF )
	{
		/* overrun will be set when the current valid byte is read */
		m_kbd_overrun = 1;
		LOGMASKED(LOG_KBD, "%f send: overrun => drop data=$%02X, parity=%i\n", machine().time().as_double(), data, parity);
	}
	else
	{
		/* valid byte */
		m_kbd_in = data;
		m_kbd_status |= ACIA_6850_RDRF; /* raise data received flag */
		if ( m_kbd_parity == 2 || m_kbd_parity == parity )
			m_kbd_status &= ~ACIA_6850_PE; /* parity OK */
		else
			m_kbd_status |= ACIA_6850_PE;  /* parity error */
		LOGMASKED(LOG_KBD, "%f send: data=$%02X, parity=%i, status=$%02X\n", machine().time().as_double(), data, parity, m_kbd_status);
	}
	update_irq();
}



/* keycode => TO9 code (extended ASCII), shifted and un-shifted */
static const int to9_kbd_code[80][2] =
{
	{ 145, 150 }, { '_', '6' }, { 'Y', 'Y' }, { 'H', 'H' },
	{ 11, 11 }, { 9, 9 }, { 30, 12 }, { 'N', 'N' },

	{ 146, 151 }, { '(', '5' }, { 'T', 'T' }, { 'G', 'G' },
	{ '=', '+' }, { 8, 8 }, { 28, 28 }, { 'B', 'B' },

	{ 147, 152 }, { '\'', '4' }, { 'R', 'R' }, { 'F', 'F' },
	{ 22, 22 },  { 155, 155 }, { 29, 127 }, { 'V', 'V' },

	{ 148, 153 }, { '"', '3' }, { 'E', 'E' }, { 'D', 'D' },
	{ 161, 161 }, { 158, 158 },
	{ 154, 154 }, { 'C', 'C' },

	{ 144, 149 }, { 128, '2' }, { 'Z', 'Z' }, { 'S', 'S' },
	{ 162, 162 }, { 156, 156 },
	{ 164, 164 }, { 'X', 'X' },

	{ '#', '@' }, { '*', '1' }, { 'A', 'A' }, { 'Q', 'Q' },
	{ '[', '{' }, { 159, 159 }, { 160, 160 }, { 'W', 'W' },

	{ 2, 2 }, { 129, '7' }, { 'U', 'U' }, { 'J', 'J' },
	{ ' ', ' ' }, { 163, 163 }, { 165, 165 },
	{ ',', '?' },

	{ 0, 0 }, { '!', '8' }, { 'I', 'I' }, { 'K', 'K' },
	{ '$', '&' }, { 10, 10 }, { ']', '}' },  { ';', '.' },

	{ 0, 0 }, { 130, '9' }, { 'O', 'O' }, { 'L', 'L' },
	{ '-', '\\' }, { 132, '%' }, { 13, 13 }, { ':', '/' },

	{ 0, 0 }, { 131, '0' }, { 'P', 'P' }, { 'M', 'M' },
	{ ')', 134 }, { '^', 133 }, { 157, 157 }, { '>', '<' }
};



/* returns the ASCII code for the key, or 0 for no key */
int to9_keyboard_device::get_key()
{
	int control = ! (m_io_keyboard[7]->read() & 1);
	int shift   = ! (m_io_keyboard[9]->read() & 1);
	int key = -1, line, bit;
	uint8_t port;

	for ( line = 0; line < 10; line++ )
	{
		port = m_io_keyboard[line]->read();

		if ( line == 7 || line == 9 )
			port |= 1; /* shift & control */

		/* TODO: correct handling of simultaneous keystokes:
		   return the new key preferably & disable repeat
		*/
		for ( bit = 0; bit < 8; bit++ )
		{
			if ( ! (port & (1 << bit)) )
				key = line * 8 + bit;
		}
	}

	if ( key == -1 )
	{
		m_kbd_last_key = 0xff;
		m_kbd_key_count = 0;
		return 0;
	}
	else if ( key == 64 )
	{
		/* caps lock */
		if ( m_kbd_last_key == key )
			return 0; /* no repeat */

		m_kbd_last_key = key;
		m_kbd_caps = !m_kbd_caps;
		m_caps_led = !m_kbd_caps;
		return 0;
	}
	else
	{
		int asc;
		asc = to9_kbd_code[key][shift];
		if ( ! asc ) return 0;

		/* keypad */
		if ( ! m_kbd_pad ) {
			if ( asc >= 154 && asc <= 163 )
				asc += '0' - 154;
			else if ( asc == 164 )
				asc = '.';
			else if ( asc == 165 )
				asc = 13;
		}

		/* shifted letter */
		if ( asc >= 'A' && asc <= 'Z' && ( ! m_kbd_caps ) && ( ! shift ) )
			asc += 'a' - 'A';

		/* control */
		if ( control )
			asc &= ~0x40;

		if ( key == m_kbd_last_key )
		{
			/* repeat */
			m_kbd_key_count++;
			if ( m_kbd_key_count < TO9_KBD_REPEAT_DELAY || (m_kbd_key_count - TO9_KBD_REPEAT_DELAY) % TO9_KBD_REPEAT_PERIOD )
				return 0;
			LOGMASKED(LOG_KBD, "to9_kbd_get_key: repeat key $%02X '%c'\n", asc, asc);
			return asc;
		}
		else
		{
			m_kbd_last_key = key;
			m_kbd_key_count = 0;
			LOGMASKED(LOG_KBD, "to9_kbd_get_key: key down $%02X '%c'\n", asc, asc);
			return asc;
		}
	}
}



TIMER_CALLBACK_MEMBER(to9_keyboard_device::timer_cb)
{
	if ( m_kbd_periph )
	{
		/* peripheral mode: every 10 ms we send 4 bytes */

		switch ( m_kbd_byte_count )
		{
		case 0: /* key */
			send( get_key(), 0 );
			break;

		case 1: /* x axis */
		{
			int newx = m_io_mouse_x.read_safe(0);
			uint8_t data = ( (newx - m_mouse_x) & 0xf ) - 8;
			send( data, 1 );
			m_mouse_x = newx;
			break;
		}

		case 2: /* y axis */
		{
			int newy = m_io_mouse_y.read_safe(0);
			uint8_t data = ( (newy - m_mouse_y) & 0xf ) - 8;
			send( data, 1 );
			m_mouse_y = newy;
			break;
		}

		case 3: /* axis overflow & buttons */
		{
			int b = m_io_mouse_button.read_safe(~0);
			uint8_t data = 0;
			if ( b & 1 ) data |= 1;
			if ( b & 2 ) data |= 4;
			send( data, 1 );
			break;
		}

		}

		m_kbd_byte_count = ( m_kbd_byte_count + 1 ) & 3;
		m_kbd_timer->adjust(m_kbd_byte_count ? TO9_KBD_BYTE_SPACE : TO9_KBD_END_SPACE);
	}
	else
	{
		int key = get_key();
		/* keyboard mode: send a byte only if a key is down */
		if ( key )
			send( key, 0 );
		m_kbd_timer->adjust(TO9_KBD_POLL_PERIOD);
	}
}



void to9_keyboard_device::device_reset()
{
	m_kbd_overrun = 0;  /* no byte lost */
	m_kbd_status = ACIA_6850_TDRE;  /* clear to transmit */
	m_kbd_intr = 0;     /* interrupt disabled */
	m_kbd_caps = 1;
	m_kbd_periph = 0;
	m_kbd_pad = 0;
	m_kbd_byte_count = 0;
	m_caps_led = !m_kbd_caps;
	m_kbd_key_count = 0;
	m_kbd_last_key = 0xff;
	update_irq();
	m_kbd_timer->adjust(TO9_KBD_POLL_PERIOD);
}



void to9_keyboard_device::device_start()
{
	m_caps_led.resolve();

	m_kbd_timer = timer_alloc(FUNC(to9_keyboard_device::timer_cb), this);
	save_item(NAME(m_kbd_parity));
	save_item(NAME(m_kbd_intr));
	save_item(NAME(m_kbd_in));
	save_item(NAME(m_kbd_status));
	save_item(NAME(m_kbd_overrun));
	save_item(NAME(m_kbd_last_key));
	save_item(NAME(m_kbd_key_count));
	save_item(NAME(m_kbd_caps));
	save_item(NAME(m_kbd_periph));
	save_item(NAME(m_kbd_pad));
	save_item(NAME(m_kbd_byte_count));
	save_item(NAME(m_mouse_x));
	save_item(NAME(m_mouse_y));
}
