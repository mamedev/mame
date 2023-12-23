// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Microtan Keyboard (MT009)

    http://www.microtan.ukpc.net/pageProducts.html#KEYBOARDS

    The keyboard is essentially an ordinary ASCII keyboard, with 71 keys, 8
    TTL chips and a 2716 eprom labeled MON V1. We currently use a
    generic keyboard because of the lack of a schematic. Unemulated keys are
    'SHIFT LOCK' and 'REPT'. Since all commands must be uppercase, capslock
    is defaulted to on.

    TODO:
    - use ROM data to determine the returned ASCII code.

*********************************************************************/

#include "emu.h"
#include "mt009.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(MICROTAN_KBD_MT009, microtan_kbd_mt009, "microtan_kbd_mt009", "Microtan Keyboard (MT009)")


static const char keyboard[8][9][8] = {
	{ /* normal */
		{ 27,'1','2','3','4','5','6','7'},
		{'8','9','0',':','-', 12,127,'^'},
		{'q','w','e','r','t','y','u','i'},
		{'o','p','[',']', 13,  3,  0,  0},
		{'a','s','d','f','g','h','j','k'},
		{'l',';','@', 92,  0,'z','x','c'},
		{'v','b','n','m',',','.','/',  0},
		{ 10,' ','-',',', 13,'.','0','1'},
		{'2','3','4','5','6','7','8','9'},
	},
	{ /* Shift */
		{ 27,'!','"','#','$','%','&', 39},
		{'(',')','~','*','=', 12,127,'_'},
		{'Q','W','E','R','T','Y','U','I'},
		{'O','P','{','}', 13,  3,  0,  0},
		{'A','S','D','F','G','H','J','K'},
		{'L','+','`','|',  0,'Z','X','C'},
		{'V','B','N','M','<','>','?',  0},
		{ 10,' ','-',',', 13,'.','0','1'},
		{'2','3','4','5','6','7','8','9'},
	},
	{ /* Control */
		{ 27,'1','2','3','4','5','6','7'},
		{'8','9','0',':','-','`',127, 30},
		{ 17, 23,  5, 18, 20, 25, 21,  9},
		{ 15, 16, 27, 29, 13,  3,  0,  0},
		{  1, 19,  4,  6,  7,  8, 10, 11},
		{ 12,';','@', 28,  0, 26, 24,  3},
		{ 22,  2, 14, 13,',','.','/',  0},
		{ 10,' ','-',',', 13,'.','0','1'},
		{'2','3','4','5','6','7','8','9'},
	},
	{ /* Shift+Control */
		{ 27,'!','"','#','$','%','&', 39},
		{'(',')','~','*','=', 12,127, 31},
		{ 17, 23,  5, 18, 20, 25, 21,  9},
		{ 15, 16, 27, 29, 13,127,  0,  0},
		{  1, 19,  4,  6,  7,  8, 10, 11},
		{ 12,'+','`', 28,  0, 26, 24,  3},
		{ 22,  2, 14, 13,',','.','/',  0},
		{ 10,' ','-',',', 13,'.','0','1'},
		{'2','3','4','5','6','7','8','9'},
	},
	{ /* CapsLock */
		{ 27,'1','2','3','4','5','6','7'},
		{'8','9','0',':','-', 12,127,'^'},
		{'Q','W','E','R','T','Y','U','I'},
		{'O','P','[',']', 13,  3,  0,  0},
		{'A','S','D','F','G','H','J','K'},
		{'L',';','@', 92,  0,'Z','X','C'},
		{'V','B','N','M',',','.','/',  0},
		{ 10,' ','-',',', 13,'.','0','1'},
		{'2','3','4','5','6','7','8','9'},
	},
	{ /* Shift+CapsLock */
		{ 27,'!','"','#','$','%','&', 39},
		{'(',')','~','*','=', 12,127,'_'},
		{'q','w','e','r','t','y','u','i'},
		{'o','p','{','}', 13,  3,  0,  0},
		{'a','s','d','f','g','h','j','k'},
		{'l','+','`','|',  0,'z','x','c'},
		{'v','b','n','m','<','>','?',  0},
		{ 10,' ','-',',', 13,'.','0','1'},
		{'2','3','4','5','6','7','8','9'},
	},
	{ /* Control+CapsLock */
		{ 27,'1','2','3','4','5','6','7'},
		{'8','9','0',':','-', 12,127,  9},
		{ 17, 23,  5, 18, 20, 25, 21,  9},
		{ 15, 16, 27, 29, 13,127,  0,  0},
		{  1, 19,  4,  6,  7,  8, 10, 11},
		{ 12,';', 39, 28,  0, 26, 24,  3},
		{ 22,  2, 14, 13,',','.','/',  0},
		{ 10,' ','-',',', 13,'.','0','1'},
		{'2','3','4','5','6','7','8','9'},
	},
	{ /* Shift+Control+CapsLock */
		{ 27,'!','"','#','$','%','&', 39},
		{'(',')','~','*','=', 12,127,  9},
		{ 17, 23,  5, 18, 20, 25, 21,  9},
		{ 15, 16, 27, 29, 13,127,  0,  0},
		{  1, 19,  4,  6,  7,  8, 10, 11},
		{ 12,':','"', 28,  0, 26, 24,  3},
		{ 22,  2, 14, 13,',','.','/',  0},
		{ 10,' ','-',',', 13,'.','0','1'},
		{'2','3','4','5','6','7','8','9'},
	},
};


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

static INPUT_PORTS_START( kbd_mt009 )
	PORT_START("KBD0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("ESC") PORT_CODE(KEYCODE_ESC) PORT_CHAR(27)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("1  !") PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("2  \"") PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("3  #") PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("4  $") PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("5  %") PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("6  &") PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("7  \'") PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR(39)

	PORT_START("KBD1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("8  (") PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("9  )") PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("0") PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR('~')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(":  *") PORT_CODE(KEYCODE_MINUS) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("-  =") PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("NEW PAGE") PORT_CODE(KEYCODE_TILDE) PORT_CHAR(12)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("RUB OUT") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"\u2191") PORT_CODE(KEYCODE_TAB) PORT_CHAR('^') PORT_CHAR('_') // U+2191 = ↑

	PORT_START("KBD2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q') PORT_CHAR('q')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('W') PORT_CHAR('w')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('E') PORT_CHAR('e')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('R') PORT_CHAR('r')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('T') PORT_CHAR('t')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y') PORT_CHAR('y')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('U') PORT_CHAR('u')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('I') PORT_CHAR('i')

	PORT_START("KBD3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('O') PORT_CHAR('o')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('P') PORT_CHAR('p')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("[  {") PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("]  }") PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("RETURN") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("BREAK") PORT_CODE(KEYCODE_DEL) PORT_CHAR(3)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("CTRL") PORT_CODE(KEYCODE_LCONTROL)
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_KEYBOARD ) PORT_NAME("ALFA LOCK") PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE

	PORT_START("KBD4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A') PORT_CHAR('a')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('S') PORT_CHAR('s')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('D') PORT_CHAR('d')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('F') PORT_CHAR('f')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('G') PORT_CHAR('g')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('H') PORT_CHAR('h')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('J') PORT_CHAR('j')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('K') PORT_CHAR('k')

	PORT_START("KBD5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('L') PORT_CHAR('l')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(";  +") PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("@") PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('@') PORT_CHAR('`')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("\\  |") PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("SHIFT (L)") PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z') PORT_CHAR('z')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('X') PORT_CHAR('x')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('C') PORT_CHAR('c')

	PORT_START("KBD6")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('V') PORT_CHAR('v')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('B') PORT_CHAR('b')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('N') PORT_CHAR('n')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('M') PORT_CHAR('m')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(",  <") PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(".  >") PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("/  ?") PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("SHIFT (R)") PORT_CODE(KEYCODE_RSHIFT)

	PORT_START("KBD7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("LINE FEED") PORT_CHAR(10)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("SPACE") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(32)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("- (KP)") PORT_CODE(KEYCODE_MINUS_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(", (KP)") PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("ENTER (KP)") PORT_CODE(KEYCODE_ENTER_PAD)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(". (KP)") PORT_CODE(KEYCODE_DEL_PAD)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("0 (KP)") PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("1 (KP)") PORT_CODE(KEYCODE_1_PAD)

	PORT_START("KBD8")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("2 (KP)") PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("3 (KP)") PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("4 (KP)") PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("5 (KP)") PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("6 (KP)") PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("7 (KP)") PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("8 (KP)") PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("9 (KP)") PORT_CODE(KEYCODE_9_PAD)

	PORT_START("BRK")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("RESET") PORT_CODE(KEYCODE_F12) PORT_CHAR(UCHAR_MAMEKEY(F12)) PORT_CHANGED_MEMBER(DEVICE_SELF, microtan_kbd_mt009, trigger_reset, 0)
INPUT_PORTS_END

INPUT_CHANGED_MEMBER(microtan_kbd_mt009::trigger_reset)
{
	m_slot->reset_w(newval ? ASSERT_LINE : CLEAR_LINE);
}

ioport_constructor microtan_kbd_mt009::device_input_ports() const
{
	return INPUT_PORTS_NAME( kbd_mt009 );
}


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

ROM_START( kbd_mt009 )
	ROM_REGION( 0x0800, "rom", 0 )
	ROM_LOAD("ascii_v1.bin", 0x0000, 0x0800, CRC(17a03a82) SHA1(89dcc0712745ed9ba876a78eefd55e9a54fc4fad))
ROM_END

const tiny_rom_entry *microtan_kbd_mt009::device_rom_region() const
{
	return ROM_NAME( kbd_mt009 );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  microtan_kbd_mt009 - constructor
//-------------------------------------------------

microtan_kbd_mt009::microtan_kbd_mt009(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MICROTAN_KBD_MT009, tag, owner, clock)
	, device_microtan_kbd_interface(mconfig, *this)
	, m_rom(*this, "rom")
	, m_keyboard(*this, "KBD%u", 0U)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void microtan_kbd_mt009::device_start()
{
	m_kbd_scan_timer = timer_alloc(FUNC(microtan_kbd_mt009::kbd_scan), this);

	m_kbd_scan_timer->adjust(attotime::from_hz(45), 0, attotime::from_hz(45));

	save_item(NAME(m_kbd_ascii));
	save_item(NAME(m_keyrows));
	save_item(NAME(m_lastrow));
	save_item(NAME(m_mask));
	save_item(NAME(m_key));
	save_item(NAME(m_repeat));
	save_item(NAME(m_repeater));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void microtan_kbd_mt009::device_reset()
{
	for (int i = 1; i < 10;  i++)
		m_keyrows[i] = m_keyboard[i-1]->read();
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

uint8_t microtan_kbd_mt009::read()
{
	return m_kbd_ascii;
}


void microtan_kbd_mt009::store_key(int key)
{
	m_kbd_ascii = key;
	m_slot->strobe_w(ASSERT_LINE);
}

TIMER_CALLBACK_MEMBER(microtan_kbd_mt009::kbd_scan)
{
	int mod, row, col, chg, newvar;

	if (m_repeat)
	{
		if (!--m_repeat)
			m_repeater = 4;
	}
	else if (m_repeater)
		m_repeat = m_repeater;

	row = 9;
	newvar = m_keyboard[8]->read();
	chg = m_keyrows[--row] ^ newvar;

	while (!chg && row > 0)
	{
		newvar = m_keyboard[row - 1]->read();
		chg = m_keyrows[--row] ^ newvar;
	}
	if (!chg)
		--row;

	if (row >= 0)
	{
		m_repeater = 0x00;
		m_mask = 0x00;
		m_key = 0x00;
		m_lastrow = row;

		if (newvar & chg)  /* key(s) pressed ? */
		{
			mod = 0;

			/* Shift modifier */
			if ((m_keyrows[5] & 0x10) || (m_keyrows[6] & 0x80))
				mod |= 1;

			/* Control modifier */
			if (m_keyrows[3] & 0x40)
				mod |= 2;

			/* CapsLock modifier */
			if (m_keyrows[3] & 0x80)
				mod |= 4;

			/* find newvar key */
			m_mask = 0x01;
			for (col = 0; col < 8; col++)
			{
				if (chg & m_mask)
				{
					newvar &= m_mask;
					m_key = keyboard[mod][row][col];
					break;
				}
				m_mask <<= 1;
			}
			if (m_key)   /* normal key */
			{
				m_repeater = 30;
				store_key(m_key);
			}
			else
				if ((row == 0) && (chg == 0x04)) /* Ctrl-@ (NUL) */
					store_key(0);
			m_keyrows[row] |= newvar;
		}
		else
			m_keyrows[row] = newvar;

		m_repeat = m_repeater;
	}
	else
		if (m_key && (m_keyrows[m_lastrow] & m_mask) && m_repeat == 0)
			store_key(m_key);
}
