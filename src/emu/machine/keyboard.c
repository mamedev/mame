// license:BSD-3-Clause
// copyright-holders:smf
/***************************************************************************
Generic ASCII Keyboard

Use GENERIC_KEYBOARD to attach as a generic ascii input device in
cases where either the driver isn't developed enough yet; or for testing;
or for the case of a computer with an inbuilt (not serial) ascii keyboard.

Example of usage in a driver.

In MACHINE_CONFIG

    MCFG_DEVICE_ADD(KEYBOARD_TAG, GENERIC_KEYBOARD, 0)
    MCFG_GENERIC_KEYBOARD_CB(WRITE8(xxx_state, kbd_put))

In the code:

WRITE8_MEMBER( xxx_state::kbd_put )
{
    (code to capture the key as it is pressed)
    (if your machine uses function keys, add your conversion code here)
}

***************************************************************************/

#include "machine/keyboard.h"

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/


generic_keyboard_device::generic_keyboard_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
	m_io_kbd0(*this, "TERM_LINE0"),
	m_io_kbd1(*this, "TERM_LINE1"),
	m_io_kbd2(*this, "TERM_LINE2"),
	m_io_kbd3(*this, "TERM_LINE3"),
	m_io_kbd4(*this, "TERM_LINE4"),
	m_io_kbd5(*this, "TERM_LINE5"),
	m_io_kbd6(*this, "TERM_LINE6"),
	m_io_kbd7(*this, "TERM_LINE7"),
	m_io_kbd8(*this, "TERM_LINE8"),
	m_io_kbd9(*this, "TERM_LINE9"),
	m_io_kbdc(*this, "TERM_LINEC"),
	m_keyboard_cb(*this)
{
}

generic_keyboard_device::generic_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, GENERIC_KEYBOARD, "Generic Keyboard", tag, owner, clock, "generic_keyboard", __FILE__),
	m_io_kbd0(*this, "TERM_LINE0"),
	m_io_kbd1(*this, "TERM_LINE1"),
	m_io_kbd2(*this, "TERM_LINE2"),
	m_io_kbd3(*this, "TERM_LINE3"),
	m_io_kbd4(*this, "TERM_LINE4"),
	m_io_kbd5(*this, "TERM_LINE5"),
	m_io_kbd6(*this, "TERM_LINE6"),
	m_io_kbd7(*this, "TERM_LINE7"),
	m_io_kbd8(*this, "TERM_LINE8"),
	m_io_kbd9(*this, "TERM_LINE9"),
	m_io_kbdc(*this, "TERM_LINEC"),
	m_keyboard_cb(*this)
{
}


UINT8 generic_keyboard_device::row_number(UINT8 code)
{
	if BIT(code,0) return 0;
	if BIT(code,1) return 1;
	if BIT(code,2) return 2;
	if BIT(code,3) return 3;
	if BIT(code,4) return 4;
	if BIT(code,5) return 5;
	if BIT(code,6) return 6;
	if BIT(code,7) return 7;
	return 0;
}

UINT8 generic_keyboard_device::keyboard_handler(UINT8 last_code, UINT8 *scan_line)
{
	int i;
	UINT8 code = 0;
	UINT8 key_code = 0;
	UINT8 retVal = 0;
	UINT8 shift = BIT(m_io_kbdc->read(), 1);
	UINT8 caps  = BIT(m_io_kbdc->read(), 2);
	UINT8 ctrl  = BIT(m_io_kbdc->read(), 0);
	i = *scan_line;
	{
		if (i == 0) code = m_io_kbd0->read();
		else
		if (i == 1) code = m_io_kbd1->read();
		else
		if (i == 2) code = m_io_kbd2->read();
		else
		if (i == 3) code = m_io_kbd3->read();
		else
		if (i == 4) code = m_io_kbd4->read();
		else
		if (i == 5) code = m_io_kbd5->read();
		else
		if (i == 6) code = m_io_kbd6->read();
		else
		if (i == 7) code = m_io_kbd7->read();
		else
		if (i == 8) code = m_io_kbd8->read();
		else
		if (i == 9) code = m_io_kbd9->read();

		if (code != 0)
		{
			if (i==0 && shift==0) {
				key_code = 0x30 + row_number(code) + 8*i; // for numbers and some signs
			}
			if (i==0 && shift==1) {
				key_code = 0x20 + row_number(code) + 8*i; // for shifted numbers
			}
			if (i==1 && shift==0) {
				if (row_number(code) < 4) {
					key_code = 0x30 + row_number(code) + 8*i; // for numbers and some signs
				} else {
					key_code = 0x20 + row_number(code) + 8*i; // for numbers and some signs
				}
			}
			if (i==1 && shift==1) {
				if (row_number(code) < 4) {
					key_code = 0x20 + row_number(code) + 8*i; // for numbers and some signs
				} else {
					key_code = 0x30 + row_number(code) + 8*i; // for numbers and some signs
				}
			}
			if (i>=2 && i<=4 && (shift ^ caps)==0 && ctrl==0) {
				key_code = 0x60 + row_number(code) + (i-2)*8; // for small letters
			}
			if (i>=2 && i<=4 && (shift ^ caps)==1 && ctrl==0) {
				key_code = 0x40 + row_number(code) + (i-2)*8; // for big letters
			}
			if (i>=2 && i<=5 && ctrl==1) {
				key_code = 0x00 + row_number(code) + (i-2)*8; // for CTRL + letters
			}
			if (i==5 && shift==1 && ctrl==0) {
				if (row_number(code)<7) {
					if (row_number(code)<3) {
						key_code = (caps ? 0x60 : 0x40) + row_number(code) + (i-2)*8; // for big letters
					} else {
						key_code = 0x60 + row_number(code) + (i-2)*8; // for upper symbols letters
					}
				} else {
					key_code = 0x40 + row_number(code) + (i-2)*8; // for DEL it is switched
				}
			}
			if (i==5 && shift==0 && ctrl==0) {
				if (row_number(code)<7) {
					if (row_number(code)<3) {
						key_code = (caps ? 0x40 : 0x60) + row_number(code) + (i-2)*8; // for small letters
					} else {
						key_code = 0x40 + row_number(code) + (i-2)*8; // for lower symbols letters
					}
				} else {
					key_code = 0x60 + row_number(code) + (i-2)*8; // for DEL it is switched
				}
			}

			if (i==6) {
				switch(row_number(code))
				{
/*                  case 0: key_code = 0x11; break;
                    case 1: key_code = 0x12; break;
                    case 2: key_code = 0x13; break;
                    case 3: key_code = 0x14; break;*/
					case 4: key_code = 0x20; break; // Space
					case 5: key_code = 0x0A; break; // LineFeed
					case 6: key_code = 0x09; break; // TAB
					case 7: key_code = 0x0D; break; // Enter
				}
			}
			if (i==7)
			{
				switch(row_number(code))
				{
					case 0: key_code = 0x1B; break; // Escape
					case 1: key_code = 0x08; break; // Backspace
				}
			}
			else
			if (i==8)
			{
				key_code = row_number(code)+0x81;
				if (ctrl) key_code+=0x10;
				if (shift) key_code+=0x20;
			}
			else
			if (i==9)
			{
				key_code = row_number(code)+0x89;
				if (ctrl) key_code+=0x10;
				if (shift) key_code+=0x20;
			}

			retVal = key_code;
		}
		else
		{
			*scan_line += 1;
			if (*scan_line==10)
				*scan_line = 0;
		}
	}
	return retVal;
}

void generic_keyboard_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	UINT8 new_code;
	new_code = keyboard_handler(m_last_code, &m_scan_line);
	if ((m_last_code != new_code) && (new_code))
		send_key(new_code);
	m_last_code = new_code;
}

/***************************************************************************
    VIDEO HARDWARE
***************************************************************************/

static MACHINE_CONFIG_FRAGMENT( generic_keyboard )
MACHINE_CONFIG_END

machine_config_constructor generic_keyboard_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME(generic_keyboard);
}

void generic_keyboard_device::device_start()
{
	m_keyboard_cb.resolve_safe();
	m_timer = timer_alloc();
	save_item(NAME(m_last_code));
	save_item(NAME(m_scan_line));
}

void generic_keyboard_device::device_reset()
{
	m_last_code = 0;
	m_scan_line = 0;
	m_timer->adjust(attotime::from_hz(2400), 0, attotime::from_hz(2400));
}

/*
Char  Dec  Oct  Hex | Char  Dec  Oct  Hex | Char  Dec  Oct  Hex | Char Dec  Oct   Hex
-------------------------------------------------------------------------------------
(nul)   0 0000 0x00 | (sp)   32 0040 0x20 | @      64 0100 0x40 | `      96 0140 0x60
(soh)   1 0001 0x01 | !      33 0041 0x21 | A      65 0101 0x41 | a      97 0141 0x61
(stx)   2 0002 0x02 | "      34 0042 0x22 | B      66 0102 0x42 | b      98 0142 0x62
(etx)   3 0003 0x03 | #      35 0043 0x23 | C      67 0103 0x43 | c      99 0143 0x63
(eot)   4 0004 0x04 | $      36 0044 0x24 | D      68 0104 0x44 | d     100 0144 0x64
(enq)   5 0005 0x05 | %      37 0045 0x25 | E      69 0105 0x45 | e     101 0145 0x65
(ack)   6 0006 0x06 | &      38 0046 0x26 | F      70 0106 0x46 | f     102 0146 0x66
(bel)   7 0007 0x07 | '      39 0047 0x27 | G      71 0107 0x47 | g     103 0147 0x67
(bs)    8 0010 0x08 | (      40 0050 0x28 | H      72 0110 0x48 | h     104 0150 0x68
(ht)    9 0011 0x09 | )      41 0051 0x29 | I      73 0111 0x49 | i     105 0151 0x69
(nl)   10 0012 0x0a | *      42 0052 0x2a | J      74 0112 0x4a | j     106 0152 0x6a
(vt)   11 0013 0x0b | +      43 0053 0x2b | K      75 0113 0x4b | k     107 0153 0x6b
(np)   12 0014 0x0c | ,      44 0054 0x2c | L      76 0114 0x4c | l     108 0154 0x6c
(cr)   13 0015 0x0d | -      45 0055 0x2d | M      77 0115 0x4d | m     109 0155 0x6d
(so)   14 0016 0x0e | .      46 0056 0x2e | N      78 0116 0x4e | n     110 0156 0x6e
(si)   15 0017 0x0f | /      47 0057 0x2f | O      79 0117 0x4f | o     111 0157 0x6f
(dle)  16 0020 0x10 | 0      48 0060 0x30 | P      80 0120 0x50 | p     112 0160 0x70
(dc1)  17 0021 0x11 | 1      49 0061 0x31 | Q      81 0121 0x51 | q     113 0161 0x71
(dc2)  18 0022 0x12 | 2      50 0062 0x32 | R      82 0122 0x52 | r     114 0162 0x72
(dc3)  19 0023 0x13 | 3      51 0063 0x33 | S      83 0123 0x53 | s     115 0163 0x73
(dc4)  20 0024 0x14 | 4      52 0064 0x34 | T      84 0124 0x54 | t     116 0164 0x74
(nak)  21 0025 0x15 | 5      53 0065 0x35 | U      85 0125 0x55 | u     117 0165 0x75
(syn)  22 0026 0x16 | 6      54 0066 0x36 | V      86 0126 0x56 | v     118 0166 0x76
(etb)  23 0027 0x17 | 7      55 0067 0x37 | W      87 0127 0x57 | w     119 0167 0x77
(can)  24 0030 0x18 | 8      56 0070 0x38 | X      88 0130 0x58 | x     120 0170 0x78
(em)   25 0031 0x19 | 9      57 0071 0x39 | Y      89 0131 0x59 | y     121 0171 0x79
(sub)  26 0032 0x1a | :      58 0072 0x3a | Z      90 0132 0x5a | z     122 0172 0x7a
(esc)  27 0033 0x1b | ;      59 0073 0x3b | [      91 0133 0x5b | {     123 0173 0x7b
(fs)   28 0034 0x1c | <      60 0074 0x3c | \      92 0134 0x5c | |     124 0174 0x7c
(gs)   29 0035 0x1d | =      61 0075 0x3d | ]      93 0135 0x5d | }     125 0175 0x7d
(rs)   30 0036 0x1e | >      62 0076 0x3e | ^      94 0136 0x5e | ~     126 0176 0x7e
(us)   31 0037 0x1f | ?      63 0077 0x3f | _      95 0137 0x5f | (del) 127 0177 0x7f

*/
INPUT_PORTS_START( generic_keyboard )
	PORT_START("TERM_LINEC")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Ctrl") PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT)  PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Caps Lock")  PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("TERM_LINE0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')

	PORT_START("TERM_LINE1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')

	PORT_START("TERM_LINE2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_END) PORT_CHAR('`') PORT_CHAR('@')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')

	PORT_START("TERM_LINE3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')

	PORT_START("TERM_LINE4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')

	PORT_START("TERM_LINE5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE) PORT_CHAR('^') PORT_CHAR('~')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("DEL") PORT_CODE(KEYCODE_DEL) PORT_CHAR(UCHAR_MAMEKEY(DEL))

	PORT_START("TERM_LINE6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Right") PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Up") PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Down") PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("LF") PORT_CODE(KEYCODE_RALT) PORT_CHAR(10)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB) PORT_CHAR(9)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)

	PORT_START("TERM_LINE7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Escape") PORT_CODE(KEYCODE_ESC) PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Backspace") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)

	PORT_START("TERM_LINE8")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F1") PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F2") PORT_CODE(KEYCODE_F2) PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F3") PORT_CODE(KEYCODE_F3) PORT_CHAR(UCHAR_MAMEKEY(F3))
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F4") PORT_CODE(KEYCODE_F4) PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F5") PORT_CODE(KEYCODE_F5) PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F6") PORT_CODE(KEYCODE_F6) PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F7") PORT_CODE(KEYCODE_F7) PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F8") PORT_CODE(KEYCODE_F8) PORT_CHAR(UCHAR_MAMEKEY(F8))

	PORT_START("TERM_LINE9")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F9") PORT_CODE(KEYCODE_F9) PORT_CHAR(UCHAR_MAMEKEY(F9))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F10") PORT_CODE(KEYCODE_F10) PORT_CHAR(UCHAR_MAMEKEY(F10))
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F11") PORT_CODE(KEYCODE_F11) PORT_CHAR(UCHAR_MAMEKEY(F11))
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F12") PORT_CODE(KEYCODE_F12) PORT_CHAR(UCHAR_MAMEKEY(F12))
INPUT_PORTS_END

ioport_constructor generic_keyboard_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(generic_keyboard);
}

const device_type GENERIC_KEYBOARD = &device_creator<generic_keyboard_device>;
