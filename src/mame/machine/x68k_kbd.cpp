// license:BSD-3-Clause
// copyright-holders:Barry Rodewald,Vas Crabb
#include "machine/x68k_kbd.h"

#include "machine/keyboard.ipp"


x68k_keyboard_device::x68k_keyboard_device(const machine_config& mconfig, const char* tag, device_t* owner, UINT32 clock)
	: buffered_rs232_device(mconfig, X68K_KEYBOARD, "X68k Keyboard", tag, owner, 0, "x68k_keyboard", __FILE__)
	, device_matrix_keyboard_interface(mconfig, *this, "LINE0", "LINE1", "LINE2", "LINE3", "LINE4", "LINE5", "LINE6", "LINE7", "LINE8", "LINE9", "LINEA", "LINEB", "LINEC", "LINED", "LINEE")
{
}


void x68k_keyboard_device::received_byte(UINT8 data)
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
	              b6 = fullwidth
	              b5 = hiragana
	              b4 = insert
	              b3 = caps
	              b2 = code input
	              b1 = romaji
	              b0 = kana
	*/

	if (data & 0x80)  // LED status
	{
		machine().output().set_value("key_led_kana", (data & 0x01) ? 0 : 1);
		machine().output().set_value("key_led_romaji", (data & 0x02) ? 0 : 1);
		machine().output().set_value("key_led_code", (data & 0x04) ? 0 : 1);
		machine().output().set_value("key_led_caps", (data & 0x08) ? 0 : 1);
		machine().output().set_value("key_led_insert", (data & 0x10) ? 0 : 1);
		machine().output().set_value("key_led_hiragana", (data & 0x20) ? 0 : 1);
		machine().output().set_value("key_led_fullsize", (data & 0x40) ? 0 : 1);
		logerror("KB: LED status set to %02x\n", data & 0x7f);
	}

	if ((data & 0xc0) == 0)  // TV control
	{
		// nothing for now
	}

	if ((data & 0xf8) == 0x48)  // Keyboard enable
	{
		m_enabled = data & 0x01;
		if (m_enabled) start_processing(attotime::from_hz(2'400));
		else stop_processing();
		logerror("KB: Keyboard enable bit = %i\n", m_enabled);
	}

	if ((data & 0xf0) == 0x60)  // Key delay time
	{
		m_delay = ((data & 0x0f) * 100) + 200;
		logerror("KB: Keypress delay time is now %ims\n", m_delay);
	}

	if ((data & 0xf0) == 0x70)  // Key repeat rate
	{
		m_repeat = (((data & 0x0f)^2) * 5) + 30;
		logerror("KB: Keypress repeat rate is now %ims\n", m_repeat);
	}
}

void x68k_keyboard_device::key_make(UINT8 row, UINT8 column)
{
	// TODO: work out which keys actually repeat (this assumes it's anything other than ctrl/opt/shift)
	if (row != 0x0eU)
		typematic_start(row, column, attotime::from_msec(m_delay), attotime::from_msec(m_repeat));
	else
		typematic_restart(attotime::from_msec(m_delay), attotime::from_msec(m_repeat));

	transmit_byte((row << 3) | column);
}

void x68k_keyboard_device::key_repeat(UINT8 row, UINT8 column)
{
	transmit_byte((row << 3) | column);
}

void x68k_keyboard_device::key_break(UINT8 row, UINT8 column)
{
	device_matrix_keyboard_interface::key_break(row, column);

	transmit_byte(0x80U | (row << 3) | column);
}

static INPUT_PORTS_START( x68k_keyboard )

	PORT_START("LINE0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED) // unused
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("ESC")                              PORT_CODE(KEYCODE_ESC)         PORT_CHAR(27)                     // Escape
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("1  !  \xE3\x81\xAC")               PORT_CODE(KEYCODE_1)           PORT_CHAR('1') PORT_CHAR('!')     // 1 ! nu
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("2  \"  \xE3\x81\xB5")              PORT_CODE(KEYCODE_2)           PORT_CHAR('2') PORT_CHAR('\"')    // 2 " fu
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("3  #  \xE3\x81\x82  \xE3\x81\x81") PORT_CODE(KEYCODE_3)           PORT_CHAR('3') PORT_CHAR('#')     // 3 # a
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("4  $  \xE3\x81\x86  \xE3\x81\x85") PORT_CODE(KEYCODE_4)           PORT_CHAR('4') PORT_CHAR('$')     // 4 $ u
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("5  %  \xE3\x81\x88  \xE3\x81\x87") PORT_CODE(KEYCODE_5)           PORT_CHAR('5') PORT_CHAR('%')     // 5 % e
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("6  &  \xE3\x81\x8A  \xE3\x81\x89") PORT_CODE(KEYCODE_6)           PORT_CHAR('6') PORT_CHAR('&')     // 6 & o

	PORT_START("LINE1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("7  \'  \xE3\x82\x84  \xE3\x82\x83") PORT_CODE(KEYCODE_7)          PORT_CHAR('7') PORT_CHAR('\'')    // 7 ' ya
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("8  (  \xE3\x82\x86  \xE3\x82\x85")  PORT_CODE(KEYCODE_8)          PORT_CHAR('8') PORT_CHAR('(')     // 8 ( yu
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("9  )  \xE3\x82\x88  \xE3\x82\x87")  PORT_CODE(KEYCODE_9)          PORT_CHAR('9') PORT_CHAR(')')     // 9 ) yo
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("0  \xE3\x82\x8F  \xE3\x82\x92")     PORT_CODE(KEYCODE_0)          PORT_CHAR('0')                    // 0 wa wo
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("-  =  \xE3\x81\xBB")                PORT_CODE(KEYCODE_MINUS)      PORT_CHAR('-') PORT_CHAR('=')     // - = ho
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("^  \xE3\x81\xB8")                   PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('^')                    // ^ he
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("\xC2\xA5  |  \xE3\x83\xBC")         PORT_CODE(KEYCODE_TILDE)      PORT_CHAR('\\') PORT_CHAR('|')    // Yen | -
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("BS")                                PORT_CODE(KEYCODE_BACKSPACE)  PORT_CHAR(8)                      // Backspace

	PORT_START("LINE2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("TAB")                               PORT_CODE(KEYCODE_TAB)        PORT_CHAR(9)                      // Tab
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Q  \xE3\x81\x9F")                   PORT_CODE(KEYCODE_Q)          PORT_CHAR('q') PORT_CHAR('Q')     // Q ta
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("W  \xE3\x81\xA6")                   PORT_CODE(KEYCODE_W)          PORT_CHAR('w') PORT_CHAR('W')     // W te
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("E  \xE3\x81\x84  \xE3\x81\x83")     PORT_CODE(KEYCODE_E)          PORT_CHAR('e') PORT_CHAR('E')     // E i
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("R  \xE3\x81\x99")                   PORT_CODE(KEYCODE_R)          PORT_CHAR('r') PORT_CHAR('R')     // R su
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("T  \xE3\x81\x8B")                   PORT_CODE(KEYCODE_T)          PORT_CHAR('t') PORT_CHAR('T')     // T ka
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Y  \xE3\x82\x93")                   PORT_CODE(KEYCODE_Y)          PORT_CHAR('y') PORT_CHAR('Y')     // Y n
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("U  \xE3\x81\xAA")                   PORT_CODE(KEYCODE_U)          PORT_CHAR('u') PORT_CHAR('U')     // U na

	PORT_START("LINE3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("I  \xE3\x81\xAB")                   PORT_CODE(KEYCODE_I)          PORT_CHAR('i') PORT_CHAR('I')     // I ni
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("O  \xE3\x82\x89")                   PORT_CODE(KEYCODE_O)          PORT_CHAR('o') PORT_CHAR('O')     // O ra
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("P  \xE3\x81\x9B")                   PORT_CODE(KEYCODE_P)          PORT_CHAR('p') PORT_CHAR('P')     // P se
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("@  `  \xE3\x82\x9B")                PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR('@') PORT_CHAR('`')     // @ ` dakuten
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("[  {  \xE3\x82\x9C \xE3\x80\x8C")   PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('[') PORT_CHAR('{')     // [ { handakuten kagikakko
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD )                                                 PORT_CODE(KEYCODE_ENTER)      PORT_CHAR(13)                     // Return
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("A  \xE3\x81\xA1")                   PORT_CODE(KEYCODE_A)          PORT_CHAR('a') PORT_CHAR('A')     // A chi
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("S  \xE3\x81\xA8")                   PORT_CODE(KEYCODE_S)          PORT_CHAR('s') PORT_CHAR('S')     // S to

	PORT_START("LINE4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("D  \xE3\x81\x97")                   PORT_CODE(KEYCODE_D)          PORT_CHAR('d') PORT_CHAR('D')     // D shi
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("F  \xE3\x81\xAF")                   PORT_CODE(KEYCODE_F)          PORT_CHAR('f') PORT_CHAR('F')     // F ha
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("G  \xE3\x81\x8D")                   PORT_CODE(KEYCODE_G)          PORT_CHAR('g') PORT_CHAR('G')     // G ki
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("H  \xE3\x81\x8F")                   PORT_CODE(KEYCODE_H)          PORT_CHAR('h') PORT_CHAR('H')     // H ku
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("J  \xE3\x81\xBE")                   PORT_CODE(KEYCODE_J)          PORT_CHAR('j') PORT_CHAR('J')     // J ma
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("K  \xE3\x81\xAE")                   PORT_CODE(KEYCODE_K)          PORT_CHAR('k') PORT_CHAR('K')     // K no
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("L  \xE3\x82\x8A")                   PORT_CODE(KEYCODE_L)          PORT_CHAR('l') PORT_CHAR('L')     // L ri
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME(";  +  \xE3\x82\x8C")                PORT_CODE(KEYCODE_COLON)      PORT_CHAR(';') PORT_CHAR('+')     // ; + re

	PORT_START("LINE5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME(":  *  \xE3\x81\x91")                PORT_CODE(KEYCODE_QUOTE)      PORT_CHAR(':') PORT_CHAR('*')     // : * ke
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("]  }  \xE3\x82\x80  \xE3\x80\x8D")  PORT_CODE(KEYCODE_BACKSLASH)  PORT_CHAR(']') PORT_CHAR('}')     // ] } mu kagikakko
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Z  \xE3\x81\xA4  \xE3\x81\xA3")     PORT_CODE(KEYCODE_Z)          PORT_CHAR('z') PORT_CHAR('Z')     // Z tsu
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("X  \xE3\x81\x95")                   PORT_CODE(KEYCODE_X)          PORT_CHAR('x') PORT_CHAR('X')     // X sa
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("C  \xE3\x81\x9D")                   PORT_CODE(KEYCODE_C)          PORT_CHAR('c') PORT_CHAR('C')     // C so
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("V  \xE3\x81\xB2")                   PORT_CODE(KEYCODE_V)          PORT_CHAR('v') PORT_CHAR('V')     // V hi
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("B  \xE3\x81\x93")                   PORT_CODE(KEYCODE_B)          PORT_CHAR('b') PORT_CHAR('B')     // B ko
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("N  \xE3\x81\xBF")                   PORT_CODE(KEYCODE_N)          PORT_CHAR('n') PORT_CHAR('N')     // N mi

	PORT_START("LINE6")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("M  \xE3\x82\x82")                   PORT_CODE(KEYCODE_M)          PORT_CHAR('m') PORT_CHAR('M')     // M mo
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME(",  <  \xE3\x81\xAD  \xE3\x80\x81")  PORT_CODE(KEYCODE_COMMA)      PORT_CHAR(',') PORT_CHAR('<')     // , < ne comma
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME(".  >  \xE3\x82\x8B  \xE3\x80\x82")  PORT_CODE(KEYCODE_STOP)       PORT_CHAR('.') PORT_CHAR('>')     // . > ru stop
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("/  ?  \xE3\x82\x81  \xE3\x83\xBB")  PORT_CODE(KEYCODE_SLASH)      PORT_CHAR('/') PORT_CHAR('?')     // / ? me interpunct
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("_  \xE3\x82\x8D")                                                 PORT_CHAR('_')                    // Underscore (shifted only?) ro
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Space")                             PORT_CODE(KEYCODE_SPACE)      PORT_CHAR(' ')                    // Space
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("HOME")                              PORT_CODE(KEYCODE_HOME)       PORT_CHAR(UCHAR_MAMEKEY(HOME))    // Home
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("DEL")                               PORT_CODE(KEYCODE_DEL)        PORT_CHAR(UCHAR_MAMEKEY(DEL))     // Del

	PORT_START("LINE7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("ROLL UP")                           PORT_CODE(KEYCODE_PGUP)       PORT_CHAR(UCHAR_MAMEKEY(PGUP))    // Roll Up
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("ROLL DOWN")                         PORT_CODE(KEYCODE_PGDN)       PORT_CHAR(UCHAR_MAMEKEY(PGDN))    // Roll Down
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("UNDO")                              PORT_CODE(KEYCODE_END)        PORT_CHAR(UCHAR_MAMEKEY(END))     // Undo
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Cursor Left")                       PORT_CODE(KEYCODE_LEFT)       PORT_CHAR(UCHAR_MAMEKEY(LEFT))    // Left
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Cursor Up")                         PORT_CODE(KEYCODE_UP)         PORT_CHAR(UCHAR_MAMEKEY(UP))      // Up
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Cursor Right")                      PORT_CODE(KEYCODE_RIGHT)      PORT_CHAR(UCHAR_MAMEKEY(RIGHT))   // Right
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Cursor Down")                       PORT_CODE(KEYCODE_DOWN)       PORT_CHAR(UCHAR_MAMEKEY(DOWN))    // Down
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Tenkey CLR")                        PORT_CODE(KEYCODE_NUMLOCK)    PORT_CHAR(UCHAR_MAMEKEY(NUMLOCK)) // Clear

	PORT_START("LINE8")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Tenkey /")                          PORT_CODE(KEYCODE_SLASH_PAD)  PORT_CHAR(UCHAR_MAMEKEY(SLASH_PAD)) // / (numpad)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Tenkey *")                          PORT_CODE(KEYCODE_ASTERISK)   PORT_CHAR(UCHAR_MAMEKEY(ASTERISK))  // * (numpad)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Tenkey -")                          PORT_CODE(KEYCODE_MINUS_PAD)  PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD)) // - (numpad)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Tenkey 7")                          PORT_CODE(KEYCODE_7_PAD)      PORT_CHAR(UCHAR_MAMEKEY(7_PAD))     // 7 (numpad)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Tenkey 8")                          PORT_CODE(KEYCODE_8_PAD)      PORT_CHAR(UCHAR_MAMEKEY(8_PAD))     // 8 (numpad)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Tenkey 9")                          PORT_CODE(KEYCODE_9_PAD)      PORT_CHAR(UCHAR_MAMEKEY(9_PAD))     // 9 (numpad)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Tenkey +")                          PORT_CODE(KEYCODE_PLUS_PAD)   PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD))  // + (numpad)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Tenkey 4")                          PORT_CODE(KEYCODE_4_PAD)      PORT_CHAR(UCHAR_MAMEKEY(4_PAD))     // 4 (numpad)

	PORT_START("LINE9")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Tenkey 5")                          PORT_CODE(KEYCODE_5_PAD)      PORT_CHAR(UCHAR_MAMEKEY(5_PAD))     // 5 (numpad)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Tenkey 6")                          PORT_CODE(KEYCODE_6_PAD)      PORT_CHAR(UCHAR_MAMEKEY(6_PAD))     // 6 (numpad)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Tenkey =")                                                                                            // = (numpad)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Tenkey 1")                          PORT_CODE(KEYCODE_1_PAD)      PORT_CHAR(UCHAR_MAMEKEY(1_PAD))     // 1 (numpad)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Tenkey 2")                          PORT_CODE(KEYCODE_2_PAD)      PORT_CHAR(UCHAR_MAMEKEY(2_PAD))     // 2 (numpad)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Tenkey 3")                          PORT_CODE(KEYCODE_3_PAD)      PORT_CHAR(UCHAR_MAMEKEY(3_PAD))     // 3 (numpad)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Tenkey ENTER")                      PORT_CODE(KEYCODE_ENTER_PAD)  PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD)) // Enter (numpad)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Tenkey 0")                          PORT_CODE(KEYCODE_0_PAD)      PORT_CHAR(UCHAR_MAMEKEY(0_PAD))     // 0 (numpad)

	PORT_START("LINEA")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Tenkey ,")                                                                                            // , (numpad)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Tenkey .")                          PORT_CODE(KEYCODE_DEL_PAD)    PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))   // . (numpad)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("\xE8\xA8\x98\xE5\x8F\xB7\xE5\x85\xA5\xE5\x8A\x9B (Symbol input)")                                     // Kigou nyuuryoku (Symbol input)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("\xE7\x99\xBB\xE9\x8C\xB2 (Register)")                                                                 // Register (babelfish translation)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Help")                                                                                                // Help
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("XF1")                               PORT_CODE(KEYCODE_F11)        PORT_CHAR(UCHAR_MAMEKEY(F11))       // XF1
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("XF2")                               PORT_CODE(KEYCODE_F12)        PORT_CHAR(UCHAR_MAMEKEY(F12))       // XF2
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("XF3")                                                                                                 // XF3

	PORT_START("LINEB")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("XF4")                                                                                                 // XF4
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("XF5")                               PORT_CODE(KEYCODE_RALT)                                           // XF5
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("\xE3\x81\x8B\xE3\x81\xAA (Kana)")                                                                     // Kana
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("\xE3\x83\xAD\xE3\x83\xBC\xE3\x83\x9E\xE5\xAD\x97 (Romaji)")                                           // Romaji
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("\xE3\x82\xB3\xE3\x83\xBC\xE3\x83\x89\xE5\x85\xA5\xE5\x8A\x9B (Code input)")                           // Code input
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("CAPS")                              PORT_CODE(KEYCODE_CAPSLOCK)  PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))   // Caps lock
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("INS")                               PORT_CODE(KEYCODE_INSERT)    PORT_CHAR(UCHAR_MAMEKEY(INSERT))     // Insert
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("\xE3\x81\xB2\xE3\x82\x89\xE3\x81\x8C\xE3\x81\xAA (Hiragana)") PORT_CODE(KEYCODE_LALT)                 // Hiragana

	PORT_START("LINEC")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("\xE5\x85\xA8\xE8\xA7\x92 (Fullwidth)") PORT_CODE(KEYCODE_RCONTROL)                                    // Zenkaku (Fullwidth)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Break")  /* Break */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("Copy")  /* Copy */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("F1")                                PORT_CODE(KEYCODE_F1)         PORT_CHAR(UCHAR_MAMEKEY(F1))        // F1
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("F2")                                PORT_CODE(KEYCODE_F2)         PORT_CHAR(UCHAR_MAMEKEY(F2))        // F2
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("F3")                                PORT_CODE(KEYCODE_F3)         PORT_CHAR(UCHAR_MAMEKEY(F3))        // F3
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("F4")                                PORT_CODE(KEYCODE_F4)         PORT_CHAR(UCHAR_MAMEKEY(F4))        // F4
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("F5")                                PORT_CODE(KEYCODE_F5)         PORT_CHAR(UCHAR_MAMEKEY(F5))        // F5

	PORT_START("LINED")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("F6")                                PORT_CODE(KEYCODE_F6)         PORT_CHAR(UCHAR_MAMEKEY(F6))        // F6
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("F7")                                PORT_CODE(KEYCODE_F7)         PORT_CHAR(UCHAR_MAMEKEY(F7))        // F7
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("F8")                                PORT_CODE(KEYCODE_F8)         PORT_CHAR(UCHAR_MAMEKEY(F8))        // F8
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("F9")                                PORT_CODE(KEYCODE_F9)         PORT_CHAR(UCHAR_MAMEKEY(F9))        // F9
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("F10")                               PORT_CODE(KEYCODE_F10)        PORT_CHAR(UCHAR_MAMEKEY(F10))       // F10
	// 0x6d reserved
	// 0x6e reserved
	// 0x6f reserved

	PORT_START("LINEE")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("SHIFT")   PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT)     PORT_CHAR(UCHAR_SHIFT_1)            // Shift
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("CTRL")                              PORT_CODE(KEYCODE_LCONTROL)                                       // Control
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("OPT.1")                             PORT_CODE(KEYCODE_PRTSCR)                                         // Opt1
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD )  PORT_NAME("OPT.2")                             PORT_CODE(KEYCODE_PAUSE)                                          // Opt2
INPUT_PORTS_END

ioport_constructor x68k_keyboard_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(x68k_keyboard);
}

void x68k_keyboard_device::device_start()
{
	buffered_rs232_device::device_start();

	save_item(NAME(m_delay));
	save_item(NAME(m_repeat));
	save_item(NAME(m_enabled));
}

void x68k_keyboard_device::device_reset()
{
	buffered_rs232_device::device_reset();

	set_data_frame(1, 8, PARITY_NONE, STOP_BITS_1);
	set_rate(38'400); // TODO: Should be 2400 but MC68901 doesn't support divide by 16
	receive_register_reset();
	transmit_register_reset();

	m_enabled = 0;
	m_delay = 500;  // 3*100+200
	m_repeat = 110;  // 4^2*5+30

	stop_processing();
	reset_key_state();
	typematic_stop();
	clear_fifo();

	output_dcd(0);
	output_dsr(0);
	output_cts(0);
	output_rxd(1);
}

void x68k_keyboard_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	device_matrix_keyboard_interface::device_timer(timer, id, param, ptr);
	buffered_rs232_device::device_timer(timer, id, param, ptr);
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
