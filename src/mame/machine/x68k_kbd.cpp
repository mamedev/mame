// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
#include "machine/x68k_kbd.h"

x68k_keyboard_device::x68k_keyboard_device(const machine_config& mconfig, const char* tag, device_t* owner, UINT32 clock) :
	serial_keyboard_device(mconfig, X68K_KEYBOARD, "X68k Keyboard", tag, owner, 0, "x68k_keyboard", __FILE__),
	m_io_kbd8(*this, "TERM_LINE8"),
	m_io_kbd9(*this, "TERM_LINE9"),
	m_io_kbda(*this, "TERM_LINEA"),
	m_io_kbdb(*this, "TERM_LINEB"),
	m_io_kbdd(*this, "TERM_LINED"),
	m_io_kbde(*this, "TERM_LINEE")
{
}


void x68k_keyboard_device::write(UINT8 data)
{
	/* Keyboard control commands:
	   00xxxxxx - TV Control
	              Not of much use as yet

	   01000xxy - y = Mouse control signal

	   01001xxy - y = Keyboard enable

	   010100xy - y = Sharp X1 display compatibility mode

	   010101xx - xx = LED brightness (00 = bright, 11 = dark)

	   010110xy - y = Display control enable

	   010111xy - y = Display control via the Opt. 2 key enable

	   0110xxxx - xxxx = Key delay (default 500ms)
	                     100 * (delay time) + 200ms

	   0111xxxx - xxxx = Key repeat rate  (default 110ms)
	                     (repeat rate)^2*5 + 30ms

	   1xxxxxxx - xxxxxxx = keyboard LED status
	              b6 = "full size"
	              b5 = hiragana
	              b4 = insert
	              b3 = caps
	              b2 = code input
	              b1 = romaji input
	              b0 = kana
	*/

	if(data & 0x80)  // LED status
	{
		machine().output().set_value("key_led_kana",(data & 0x01) ? 0 : 1);
		machine().output().set_value("key_led_romaji",(data & 0x02) ? 0 : 1);
		machine().output().set_value("key_led_code",(data & 0x04) ? 0 : 1);
		machine().output().set_value("key_led_caps",(data & 0x08) ? 0 : 1);
		machine().output().set_value("key_led_insert",(data & 0x10) ? 0 : 1);
		machine().output().set_value("key_led_hiragana",(data & 0x20) ? 0 : 1);
		machine().output().set_value("key_led_fullsize",(data & 0x40) ? 0 : 1);
		logerror("KB: LED status set to %02x\n",data & 0x7f);
	}

	if((data & 0xc0) == 0)  // TV control
	{
		// nothing for now
	}

	if((data & 0xf8) == 0x48)  // Keyboard enable
	{
		m_enabled = data & 0x01;
		logerror("KB: Keyboard enable bit = %i\n",m_enabled);
	}

	if((data & 0xf0) == 0x60)  // Key delay time
	{
		m_delay = data & 0x0f;
		logerror("KB: Keypress delay time is now %ims\n",(data & 0x0f)*100+200);
	}

	if((data & 0xf0) == 0x70)  // Key repeat rate
	{
		m_repeat = data & 0x0f;
		logerror("KB: Keypress repeat rate is now %ims\n",((data & 0x0f)^2)*5+30);
	}
}

UINT8 x68k_keyboard_device::keyboard_handler(UINT8 last_code, UINT8 *scan_line)
{
	if (m_enabled)
	{
		for (int row = 0; row < 15; row++ )
		{
			UINT8 data = 0;

			if (row == 0) data = m_io_kbd0->read();
			else
			if (row == 1) data = m_io_kbd1->read();
			else
			if (row == 2) data = m_io_kbd2->read();
			else
			if (row == 3) data = m_io_kbd3->read();
			else
			if (row == 4) data = m_io_kbd4->read();
			else
			if (row == 5) data = m_io_kbd5->read();
			else
			if (row == 6) data = m_io_kbd6->read();
			else
			if (row == 7) data = m_io_kbd7->read();
			else
			if (row == 8) data = m_io_kbd8->read();
			else
			if (row == 9) data = m_io_kbd9->read();
			else
			if (row == 10) data = m_io_kbda->read();
			else
			if (row == 11) data = m_io_kbdb->read();
			else
			if (row == 12) data = m_io_kbdc->read();
			else
			if (row == 13) data = m_io_kbdd->read();
			else
			if (row == 14) data = m_io_kbde->read();

			for (int column = 0; column < 8; column++ )
			{
				int new_down = (data & (1 << column)) != 0;
				int scan_code = (row * 8) + column;
				int old_down = m_key_down[scan_code];
				m_key_down[scan_code] = new_down;

				if (new_down && !old_down)
				{
					m_repeat_code = scan_code;
					m_until_repeat = m_delay * 240;

					return scan_code;
				}
				else if(!new_down && old_down)
				{
					m_repeat_code = 0;
					return scan_code + 0x80;
				}
			}
		}

		if (m_repeat_code > 0 && m_key_down[m_repeat_code])
		{
			m_until_repeat--;
			if (m_until_repeat == 0)
			{
				m_until_repeat = m_repeat * 240;
				return m_repeat_code;
			}
		}
	}

	return 0;
}

static INPUT_PORTS_START( x68k_keyboard )

	PORT_START("TERM_LINE0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED) // unused
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("ESC") PORT_CODE(KEYCODE_ESC) PORT_CHAR(27)  /* ESC */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("1  !  \xE3\x83\x8C") PORT_CODE(KEYCODE_1)  PORT_CHAR('1') PORT_CHAR('!') /* 1 ! */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("2  \"  \xE3\x83\x95") PORT_CODE(KEYCODE_2)  PORT_CHAR('2') PORT_CHAR('\"') /* 2 " */
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("3  #  \xE3\x82\xA2  \xE3\x82\xA1") PORT_CODE(KEYCODE_3)  PORT_CHAR('3') PORT_CHAR('#') /* 3 # */
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("4  $  \xE3\x82\xA6  \xE3\x82\xA5") PORT_CODE(KEYCODE_4)  PORT_CHAR('4') PORT_CHAR('$') /* 4 $ */
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("5  %  \xE3\x82\xA8  \xE3\x82\xA7") PORT_CODE(KEYCODE_5)  PORT_CHAR('5') PORT_CHAR('%') /* 5 % */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("6  &  \xE3\x82\xAA  \xE3\x82\xA9") PORT_CODE(KEYCODE_6)  PORT_CHAR('6') PORT_CHAR('&') /* 6 & */

	PORT_START("TERM_LINE1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("7  \'  \xE3\x83\xA4  \xE3\x83\xA3") PORT_CODE(KEYCODE_7)  PORT_CHAR('7') PORT_CHAR('\'') /* 7 ' */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("8  (  \xE3\x83\xA6  \xE3\x83\xA5") PORT_CODE(KEYCODE_8)  PORT_CHAR('8') PORT_CHAR('(') /* 8 ( */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("9  )  \xE3\x83\xA8  \xE3\x83\xA7") PORT_CODE(KEYCODE_9)  PORT_CHAR('9') PORT_CHAR(')') /* 9 ) */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("0  \xE3\x83\xAF  \xE3\x83\xB2") PORT_CODE(KEYCODE_0)  PORT_CHAR('0')                /* 0 */
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("-  =  \xE3\x83\x9B") PORT_CODE(KEYCODE_MINUS)  PORT_CHAR('-') PORT_CHAR('=') /* - = */
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("^  \xE3\x83\x98") PORT_CHAR('^') /* ^ */
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("\xC2\xA5  \xE3\x83\xBC  |") PORT_CODE(KEYCODE_BACKSLASH)  PORT_CHAR('\\') PORT_CHAR('|') /* Yen | */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_CODE(KEYCODE_BACKSPACE)  PORT_CHAR(8) /* Backspace */

	PORT_START("TERM_LINE2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_CODE(KEYCODE_TAB)  PORT_CHAR(9)  /* Tab */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Q  \xE3\x82\xBF") PORT_CODE(KEYCODE_Q)  PORT_CHAR('Q')  /* Q */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("W  \xE3\x83\x86") PORT_CODE(KEYCODE_W)  PORT_CHAR('W')  /* W */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("E  \xE3\x82\xA4  \xE3\x82\xA3") PORT_CODE(KEYCODE_E)  PORT_CHAR('E')  /* E */
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("R  \xE3\x82\xB9") PORT_CODE(KEYCODE_R)  PORT_CHAR('R')  /* R */
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("T  \xE3\x82\xAB") PORT_CODE(KEYCODE_T)  PORT_CHAR('T')  /* T */
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Y  \xE3\x83\xB3") PORT_CODE(KEYCODE_Y)  PORT_CHAR('Y')  /* Y */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("U  \xE3\x83\x8A") PORT_CODE(KEYCODE_U)  PORT_CHAR('U')  /* U */

	PORT_START("TERM_LINE3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("I  \xE3\x83\x8B") PORT_CODE(KEYCODE_I)  PORT_CHAR('I')  /* I */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("O  \xE3\x83\xA9") PORT_CODE(KEYCODE_O)  PORT_CHAR('O')  /* O */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("P  \xE3\x82\xBB") PORT_CODE(KEYCODE_P)  PORT_CHAR('P')  /* P */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("@  `  \xE3\x82\x9B") PORT_CHAR('@') PORT_CHAR('`')  /* @ */
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("[  {  \xE3\x82\x9C \xE3\x80\x8C") PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR('[') PORT_CHAR('{')  /* [ { */
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_CODE(KEYCODE_ENTER)  PORT_CHAR(13)  /* Return */
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("A  \xE3\x83\x81") PORT_CODE(KEYCODE_A)  PORT_CHAR('A')  /* A */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("S  \xE3\x83\x88") PORT_CODE(KEYCODE_S)  PORT_CHAR('S')  /* S */

	PORT_START("TERM_LINE4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("D  \xE3\x82\xB7") PORT_CODE(KEYCODE_D)  PORT_CHAR('D')  /* D */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("F  \xE3\x83\x8F") PORT_CODE(KEYCODE_F)  PORT_CHAR('F')  /* F */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("G  \xE3\x82\xAD") PORT_CODE(KEYCODE_G)  PORT_CHAR('G')  /* G */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("H  \xE3\x82\xAF") PORT_CODE(KEYCODE_H)  PORT_CHAR('H')  /* H */
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("J  \xE3\x83\x9E") PORT_CODE(KEYCODE_J)  PORT_CHAR('J')  /* J */
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("K  \xE3\x83\x8E") PORT_CODE(KEYCODE_K)  PORT_CHAR('K')  /* K */
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("L  \xE3\x83\xAA") PORT_CODE(KEYCODE_L)  PORT_CHAR('L')  /* L */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME(";  +  \xE3\x83\xAC") PORT_CODE(KEYCODE_COLON)  PORT_CHAR(';')  PORT_CHAR('+')  /* ; + */

	PORT_START("TERM_LINE5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME(":  *  \xE3\x82\xB1") PORT_CODE(KEYCODE_QUOTE)  PORT_CHAR(':')  PORT_CHAR('*')  /* : * */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("]  }  \xE3\x83\xA0  \xE3\x80\x8D") PORT_CODE(KEYCODE_CLOSEBRACE)  PORT_CHAR(']')  PORT_CHAR('}')  /* ] } */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Z  \xE3\x83\x84  \xE3\x83\x83") PORT_CODE(KEYCODE_Z)  PORT_CHAR('Z')  /* Z */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("X  \xE3\x82\xB5") PORT_CODE(KEYCODE_X)  PORT_CHAR('X')  /* X */
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("C  \xE3\x82\xBD") PORT_CODE(KEYCODE_C)  PORT_CHAR('C')  /* C */
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("V  \xE3\x83\x92") PORT_CODE(KEYCODE_V)  PORT_CHAR('V')  /* V */
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("B  \xE3\x82\xB3") PORT_CODE(KEYCODE_B)  PORT_CHAR('B')  /* B */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("N  \xE3\x83\x9F") PORT_CODE(KEYCODE_N)  PORT_CHAR('N')  /* N */

	PORT_START("TERM_LINE6")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("M  \xE3\x83\xA2") PORT_CODE(KEYCODE_M)  PORT_CHAR('M')  /* M */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME(",  <  \xE3\x83\x8D  \xE3\x80\x81") PORT_CODE(KEYCODE_COMMA)  PORT_CHAR(',')  PORT_CHAR('<')  /* , < */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME(".  >  \xE3\x83\xAB  \xE3\x80\x82") PORT_CODE(KEYCODE_STOP)  PORT_CHAR('.')  PORT_CHAR('>')  /* . > */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("/  ?  \xE3\x83\xA1  \xE3\x83\xBB") PORT_CODE(KEYCODE_SLASH)  PORT_CHAR('/')  PORT_CHAR('?')  /* / ? */
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("_  \xE3\x83\xAD") PORT_CHAR('_')  /* Underscore (shifted only?) */
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Space")  PORT_CODE(KEYCODE_SPACE)  PORT_CHAR(' ')  /* Space */
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Home")  PORT_CODE(KEYCODE_HOME)  /* Home */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Delete")  PORT_CODE(KEYCODE_DEL)  /* Del */

	PORT_START("TERM_LINE7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Roll Up")  PORT_CODE(KEYCODE_PGUP)  /* Roll Up */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Roll Down")  PORT_CODE(KEYCODE_PGDN)  /* Roll Down */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Undo")  PORT_CODE(KEYCODE_END)  /* Undo */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Cursor Left")  PORT_CODE(KEYCODE_LEFT)  /* Left */
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Cursor Up")  PORT_CODE(KEYCODE_UP)  /* Up */
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Cursor Right")  PORT_CODE(KEYCODE_RIGHT)  /* Right */
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Cursor Down")  PORT_CODE(KEYCODE_DOWN)  /* Down */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Tenkey CLR")  PORT_CODE(KEYCODE_NUMLOCK)  /* CLR */

	PORT_START("TERM_LINE8")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Tenkey /")  PORT_CODE(KEYCODE_SLASH_PAD)  /* / (numpad) */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Tenkey *")  PORT_CODE(KEYCODE_ASTERISK)  /* * (numpad) */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Tenkey -")  PORT_CODE(KEYCODE_MINUS_PAD)  /* - (numpad) */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Tenkey 7")  PORT_CODE(KEYCODE_7_PAD)  /* 7 (numpad) */
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Tenkey 8")  PORT_CODE(KEYCODE_8_PAD)  /* 8 (numpad) */
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Tenkey 9")  PORT_CODE(KEYCODE_9_PAD)  /* 9 (numpad) */
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Tenkey +")  PORT_CODE(KEYCODE_PLUS_PAD)  /* + (numpad) */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Tenkey 4")  PORT_CODE(KEYCODE_4_PAD)  /* 4 (numpad) */

	PORT_START("TERM_LINE9")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Tenkey 5")  PORT_CODE(KEYCODE_5_PAD)  /* 5 (numpad) */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Tenkey 6")  PORT_CODE(KEYCODE_6_PAD)  /* 6 (numpad) */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Tenkey =")  /* = (numpad) */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Tenkey 1")  PORT_CODE(KEYCODE_1_PAD)  /* 1 (numpad) */
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Tenkey 2")  PORT_CODE(KEYCODE_2_PAD)  /* 2 (numpad) */
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Tenkey 3")  PORT_CODE(KEYCODE_3_PAD)  /* 3 (numpad) */
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Tenkey Enter")  PORT_CODE(KEYCODE_ENTER_PAD)  /* Enter (numpad) */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Tenkey 0")  PORT_CODE(KEYCODE_0_PAD)  /* 0 (numpad) */

	PORT_START("TERM_LINEA")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Tenkey ,")  /* , (numpad) */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Tenkey .")  PORT_CODE(KEYCODE_DEL_PAD)  /* 2 (numpad) */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("\xE8\xA8\x98\xE5\x8F\xB7 (Symbolic input)")  /* Sign / Symbolic input (babelfish translation) */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("\xE7\x99\xBB\xE9\x8C\xB2 (Register)")  /* Register (babelfish translation) */
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Help")  /* Help */
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("XF1")  PORT_CODE(KEYCODE_F11)  /* XF1 */
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("XF2")  PORT_CODE(KEYCODE_F12)  /* XF2 */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("XF3")  /* XF3 */

	PORT_START("TERM_LINEB")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("XF4")  /* XF4 */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("XF5")  /* XF5 */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("\xe3\x81\x8b\xe3\x81\xaa (Kana)")  /* Kana */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("\xe3\x83\xad\xe3\x83\xbc\xe3\x83\x9e\xe5\xad\x97 (Romaji)")  /* Romaji */
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("\xE3\x82\xB3\xE3\x83\xBC\xE3\x83\x89 (Code input)")  /* Code input */
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Caps")  PORT_CODE(KEYCODE_CAPSLOCK)  /* Caps lock */
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Insert")  PORT_CODE(KEYCODE_INSERT)  /* Insert */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("\xE3\x81\xB2\xE3\x82\x89\xE3\x81\x8C\xE3\x81\xAA (Hiragana)")  /* Hiragana */

	PORT_START("TERM_LINEC")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("\xE5\x85\xA8\xE8\xA7\x92 (Full size)")  /* Full size (babelfish translation) */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Break")  /* Break */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Copy")  /* Copy */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("F1")  PORT_CODE(KEYCODE_F1)  /* F1 */
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("F2")  PORT_CODE(KEYCODE_F2)  /* F2 */
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("F3")  PORT_CODE(KEYCODE_F3)  /* F3 */
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("F4")  PORT_CODE(KEYCODE_F4)  /* F4 */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("F5")  PORT_CODE(KEYCODE_F5)  /* F5 */

	PORT_START("TERM_LINED")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("F6")  PORT_CODE(KEYCODE_F6)  /* F6 */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("F7")  PORT_CODE(KEYCODE_F7)  /* F7 */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("F8")  PORT_CODE(KEYCODE_F8)  /* F8 */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("F9")  PORT_CODE(KEYCODE_F9)  /* F9 */
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("F10")  PORT_CODE(KEYCODE_F10)  /* F10 */
	// 0x6d reserved
	// 0x6e reserved
	// 0x6f reserved

	PORT_START("TERM_LINEE")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Shift")  PORT_CODE(KEYCODE_LSHIFT)  /* Shift */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Ctrl")  PORT_CODE(KEYCODE_LCONTROL)  /* Ctrl */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Opt. 1")  PORT_CODE(KEYCODE_PRTSCR) /* Opt1 */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Opt. 2")  PORT_CODE(KEYCODE_PAUSE)  /* Opt2 */

	PORT_START("RS232_TXBAUD")
	PORT_CONFNAME(0xff, RS232_BAUD_38400, "TX Baud") PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, serial_keyboard_device, update_serial)
	PORT_CONFSETTING( RS232_BAUD_38400, "38400") // TODO: Should be 2400 but MC68901 doesn't support divide by 16

	PORT_START("RS232_STARTBITS")
	PORT_CONFNAME(0xff, RS232_STARTBITS_1, "Start Bits") PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, serial_keyboard_device, update_serial)
	PORT_CONFSETTING( RS232_STARTBITS_1, "1")

	PORT_START("RS232_DATABITS")
	PORT_CONFNAME(0xff, RS232_DATABITS_8, "Data Bits") PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, serial_keyboard_device, update_serial)
	PORT_CONFSETTING( RS232_DATABITS_8, "8")

	PORT_START("RS232_PARITY")
	PORT_CONFNAME(0xff, RS232_PARITY_NONE, "Parity") PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, serial_keyboard_device, update_serial)
	PORT_CONFSETTING( RS232_PARITY_NONE, "None")

	PORT_START("RS232_STOPBITS")
	PORT_CONFNAME(0xff, RS232_STOPBITS_1, "Stop Bits") PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, serial_keyboard_device, update_serial)
	PORT_CONFSETTING( RS232_STOPBITS_1, "1")
INPUT_PORTS_END

ioport_constructor x68k_keyboard_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(x68k_keyboard);
}

void x68k_keyboard_device::device_start()
{
	serial_keyboard_device::device_start();
	set_rcv_rate(38400); // TODO: Should be 2400 but MC68901 doesn't support divide by 16
}

void x68k_keyboard_device::device_reset()
{
	serial_keyboard_device::device_reset();

	m_enabled = 0;
	m_delay = 500;  // 3*100+200
	m_repeat = 110;  // 4^2*5+30
	m_repeat_code = 0;

	memset(m_key_down, 0, sizeof(m_key_down));
}

void x68k_keyboard_device::rcv_complete()
{
	receive_register_extract();
	write(get_received_char());
}

const device_type X68K_KEYBOARD = &device_creator<x68k_keyboard_device>;

#if 0

void x68k_state::x68k_keyboard_push_scancode(unsigned char code)
{
	m_keynum++;
	if(m_keynum >= 1)
	{
		// keyboard buffer full
		if(m_enabled != 0)
		{
			//m_mfp.rsr |= 0x80;  // Buffer full
			//if(ioport("options")->read() & 0x01)
			//{
			//  m_current_vector[6] = 0x4c;
			//  m_maincpu->set_input_line_and_vector(6,ASSERT_LINE,0x4c);
			//  logerror("MFP: Receive buffer full IRQ sent\n");
			//}
		}
	}
	m_buffer[m_headpos++] = code;
	if(m_headpos > 15)
	{
		m_headpos = 0;
		m_current_vector[6] = 0x4b;
	}
}

TIMER_CALLBACK_MEMBER(x68k_state::x68k_keyboard_poll)
{
	int x;
	static const char *const keynames[] = { "key1", "key2", "key3", "key4" };

	for(x=0;x<0x80;x++)
	{
		// adjust delay/repeat timers
		if(m_keytime[x] > 0)
		{
			m_keytime[x] -= 5;
		}
		if(!(ioport(keynames[x / 32])->read() & (1 << (x % 32))))
		{
			if(m_keyon[x] != 0)
			{
				x68k_keyboard_push_scancode(0x80 + x);
				m_keytime[x] = 0;
				m_keyon[x] = 0;
				m_last_pressed = 0;
				logerror("KB: Released key 0x%02x\n",x);
			}
		}
		// check to see if a key is being held
		if(m_keyon[x] != 0 && m_keytime[x] == 0 && m_last_pressed == x)
		{
			if(ioport(keynames[m_last_pressed / 32])->read() & (1 << (m_last_pressed % 32)))
			{
				x68k_keyboard_push_scancode(m_last_pressed);
				m_keytime[m_last_pressed] = (m_repeat^2)*5+30;
				logerror("KB: Holding key 0x%02x\n",m_last_pressed);
			}
		}
		if((ioport(keynames[x / 32])->read() & (1 << (x % 32))))
		{
			if(m_keyon[x] == 0)
			{
				x68k_keyboard_push_scancode(x);
				m_keytime[x] = m_delay * 100 + 200;
				m_keyon[x] = 1;
				m_last_pressed = x;
				logerror("KB: Pushed key 0x%02x\n",x);
			}
		}
	}
}

	struct
	{
		unsigned char led_status;  // keyboard LED status
		unsigned char buffer[16];
		int headpos;  // scancodes are added here
		int tailpos;  // scancodes are read from here
		int keynum;  // number of scancodes in buffer
		int keytime[0x80];  // time until next keypress
		int keyon[0x80];  // is 1 if key is pressed, used to determine if the key state has changed from 1 to 0
		int last_pressed;  // last key pressed, for repeat key handling
	} m_keyboard;
	TIMER_CALLBACK_MEMBER(x68k_led_callback);
	TIMER_CALLBACK_MEMBER(x68k_keyboard_poll);
	void x68k_keyboard_ctrl_w(int data);
	int x68k_keyboard_pop_scancode();
	void x68k_keyboard_push_scancode(unsigned char code);

#endif
