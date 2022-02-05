// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/*
    Elektronika MS 7004 keyboard (DEC LK-201 workalike with extra keys
    for Cyrillic characters).

    To do:
    - connect LEDs and speaker
*/

#include "emu.h"
#include "ms7004.h"
#include "speaker.h"


//#define LOG_GENERAL (1U <<  0) //defined in logmacro.h already
#define LOG_DEBUG     (1U <<  1)

//#define VERBOSE (LOG_GENERAL | LOG_DEBUG)
//#define LOG_OUTPUT_FUNC printf
#include "logmacro.h"

#define LOGDBG(...) LOGMASKED(LOG_DEBUG, __VA_ARGS__)


//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define MS7004_CPU_TAG   "i8035"
#define MS7004_SPK_TAG   "beeper"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(MS7004, ms7004_device, "ms7004", "MS7004 keyboard")

ROM_START( ms7004 )
	ROM_REGION (0x800, MS7004_CPU_TAG, 0)
	ROM_LOAD ("mc7004_keyboard_original.rom", 0x0000, 0x800, CRC(69fcab53) SHA1(2d7cc7cd182f2ee09ecf2c539e33db3c2195f778))
ROM_END

//-------------------------------------------------
//  ADDRESS_MAP
//-------------------------------------------------

void ms7004_device::ms7004_map(address_map &map)
{
	map(0x0000, 0x07ff).rom();
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void ms7004_device::device_add_mconfig(machine_config &config)
{
	I8035(config, m_maincpu, XTAL(4'608'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &ms7004_device::ms7004_map);
	m_maincpu->p1_out_cb().set(FUNC(ms7004_device::p1_w));
	m_maincpu->p2_out_cb().set(FUNC(ms7004_device::p2_w));
	m_maincpu->t1_in_cb().set(FUNC(ms7004_device::t1_r));
	m_maincpu->prog_out_cb().set(m_i8243, FUNC(i8243_device::prog_w));

	I8243(config, m_i8243);
	m_i8243->p4_out_cb().set(FUNC(ms7004_device::i8243_port_w<0>));
	m_i8243->p5_out_cb().set(FUNC(ms7004_device::i8243_port_w<1>));
	m_i8243->p6_out_cb().set(FUNC(ms7004_device::i8243_port_w<2>));
	m_i8243->p7_out_cb().set(FUNC(ms7004_device::i8243_port_w<3>));

	SPEAKER(config, "mono").front_center();
	BEEP(config, m_speaker, 3250).add_route(ALL_OUTPUTS, "mono", 0.50);
}

const tiny_rom_entry *ms7004_device::device_rom_region() const
{
	return ROM_NAME(ms7004);
}

//-------------------------------------------------
//  INPUT_PORTS( ms7004 )
//-------------------------------------------------
/*
bit sig XSn ????n
--- --- --- ---
0   8   16  15
1   9   15  14
2   10  14  13
3   11  13  12
4   12  19  16
5   13  12  11
6   14  11  10
7   15  10  9
8   16  9   8
9   17  8   7
10  18  7   6
11  19  6   5
12  20  3   1
13  21  1   2
14  22  4   3
15  23  5   4

0xc9    KEY_LANGLE_RANGLE   'резервная клавиша'
0xbc    KEY_DELETE  ЗБ
0xbd    KEY_RETURN  ВК
0xbf    KEY_TILDE   '; +'
0xc4    -       'Ъ'
0xca    -       '/ ?'
0xed    KEY_PERIOD  'Ю @'
0xf1    -       '_'
<...>

0x56    KEY_F1      СТОП КАДР
0x57    KEY_F2      ПЕЧАТЬ КАДРА
0x58    KEY_F3      ПАУЗА
0x59    KEY_F4      УСТ РЕЖИМА
0x5a    KEY_F5      Ф5

0x64    KEY_F6      ПРЕРЫВ
0x65    KEY_F7      ПРОДОЛЖ
0x66    KEY_F8      ОТМЕН
0x67    KEY_F9      ОСНОВН КАДР
0x69    KEY_F10     ВЫХОД

0x71    KEY_F11     Ф11 (АР2)
0x72    KEY_F12     Ф12 (ВШ)
0x73    KEY_F13     Ф13 (ПС)
0x74    KEY_F14     ДОП ВАРИАНТ

0x7c    KEY_HELP    ПМ
0x7d    KEY_MENU    ИСП

0x80    KEY_F17     Ф17
0x81    KEY_F18     Ф18
0x82    KEY_F19     Ф19
0x83    KEY_F20     Ф20

0xb0    KEY_LOCK    ФКС
0xae    KEY_SHIFT   ВР
0xaf    KEY_CTRL    СУ

0xb1    KEY_META    КМП
0xb2    -       РУС/ЛАТ

0x8a    KEY_FIND    НТ
0x8b    KEY_INSERT_HERE ВСТ
0x8c    KEY_REMOVE  УДАЛ
0x8d    KEY_SELECT  ВЫБР
0x8e    KEY_PREV_SCREEN ПРЕД КАДР
0x8f    KEY_NEXT_SCREEN СЛЕД КАДР

nothing sends '@' or '`'

`/~ sends ^/~
2/@ sends 2/"
6/^ sends 6/&
7/& sends 7/'
8 * sends 8/(
9/( sends 9/)
0/) sends 0/0
-/_ sends _/_
+/= sends -/=
;/: sends ;/+
'/" sends : *

F10 sends ^C
F11 sends ESC
F12 sends ^H
*/
INPUT_PORTS_START( ms7004 )
	PORT_START("KBD12") // vertical row 1
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F2) PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Hold Screen (F1)") PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED ) // '{' / '|'
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_TAB) PORT_CHAR('\t')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Shift Lock") PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)

	PORT_START("KBD13") // vertical row 2
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Setup (F3)") PORT_CODE(KEYCODE_F3) PORT_CHAR(UCHAR_MAMEKEY(F3))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Data / Talk (F4)") PORT_CODE(KEYCODE_F4) PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('J') PORT_CHAR('j')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('C') PORT_CHAR('c')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('F') PORT_CHAR('f')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Rus/Lat") PORT_CODE(KEYCODE_ESC) PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Compose") PORT_CODE(KEYCODE_LALT) PORT_CHAR(UCHAR_MAMEKEY(LALT))

	PORT_START("KBD14") // vertical row 3
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Break (F5)") PORT_CODE(KEYCODE_F5) PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('U') PORT_CHAR('u')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y') PORT_CHAR('y')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_TILDE) PORT_CHAR('^') PORT_CHAR('~')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q') PORT_CHAR('q')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KBD15") // vertical row 4
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('K') PORT_CHAR('k')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('W') PORT_CHAR('w')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('S') PORT_CHAR('s')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KBD11") // vertical row 5
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Interrupt (F6)") PORT_CODE(KEYCODE_F6) PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('E') PORT_CHAR('e')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('P') PORT_CHAR('p')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A') PORT_CHAR('a')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('M') PORT_CHAR('m')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KBD10") // vertical row 6
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Resume (F7)") PORT_CODE(KEYCODE_F7) PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('N') PORT_CHAR('n')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('G') PORT_CHAR('g')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('R') PORT_CHAR('r')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('T') PORT_CHAR('t')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('I') PORT_CHAR('i')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ') PORT_CHAR(' ')

	PORT_START("KBD9")  // vertical row 7
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Cancel (F8)") PORT_CODE(KEYCODE_F8) PORT_CHAR(UCHAR_MAMEKEY(F8))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Main Screen (F9)") PORT_CODE(KEYCODE_F9) PORT_CHAR(UCHAR_MAMEKEY(F9))
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('O') PORT_CHAR('o')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('X') PORT_CHAR('x')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('B') PORT_CHAR('b')

	PORT_START("KBD8")  // vertical row 8
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Exit (F10)") PORT_CODE(KEYCODE_F10) PORT_CHAR(UCHAR_MAMEKEY(F10))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z') PORT_CHAR('z')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('D') PORT_CHAR('d')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('L') PORT_CHAR('l')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH2) PORT_CHAR('@') PORT_CHAR('`')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')

	PORT_START("KBD7")  // vertical row 9
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("ESC (F11)") PORT_CODE(KEYCODE_F11) PORT_CHAR(UCHAR_MAMEKEY(F11))
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('H') PORT_CHAR('h')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('V') PORT_CHAR('v')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')

	PORT_START("KBD6")  // vertical row 10
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("BS (F12)") PORT_CODE(KEYCODE_F12) PORT_CHAR(UCHAR_MAMEKEY(F12))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("LF (F13)") PORT_CODE(KEYCODE_PRTSCR) PORT_CHAR(UCHAR_MAMEKEY(PRTSCR))
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED ) // '}'
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED ) // sends '_'
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('_') PORT_CHAR('_')

	PORT_START("KBD5")  // vertical row 11
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Additional Options (F14)") PORT_CODE(KEYCODE_PAUSE) PORT_CHAR(UCHAR_MAMEKEY(PAUSE))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Delete <X") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED ) // sends LF
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Return") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KBD3")  // vertical row 12
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Help (F15)") PORT_CODE(KEYCODE_RALT) PORT_CHAR(UCHAR_MAMEKEY(RALT))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Insert Here") PORT_CODE(KEYCODE_HOME) PORT_CHAR(UCHAR_MAMEKEY(HOME))
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Find") PORT_CODE(KEYCODE_INSERT) PORT_CHAR(UCHAR_MAMEKEY(INSERT))
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Select") PORT_CODE(KEYCODE_DEL) PORT_CHAR(UCHAR_MAMEKEY(DEL))
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KBD2")  // vertical row 13
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Remove") PORT_CODE(KEYCODE_PGUP) PORT_CHAR(UCHAR_MAMEKEY(PGUP))
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Next [v]") PORT_CODE(KEYCODE_PGDN) PORT_CHAR(UCHAR_MAMEKEY(PGDN))
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Previous [^]") PORT_CODE(KEYCODE_END) PORT_CHAR(UCHAR_MAMEKEY(END))
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KBD1")  // vertical row 14
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Do (F16)") PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_MAMEKEY(RCONTROL))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("PF1") PORT_CODE(KEYCODE_NUMLOCK) PORT_CHAR(UCHAR_MAMEKEY(NUMLOCK))
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Num 7") PORT_CODE(KEYCODE_7_PAD) PORT_CHAR(UCHAR_MAMEKEY(7_PAD))
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Num 4") PORT_CODE(KEYCODE_4_PAD) PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Num 1") PORT_CODE(KEYCODE_1_PAD) PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Num 2") PORT_CODE(KEYCODE_2_PAD) PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Num 0") PORT_CODE(KEYCODE_0_PAD) PORT_CHAR(UCHAR_MAMEKEY(0_PAD))

	PORT_START("KBD0")  // vertical row 15
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F17) PORT_CHAR(UCHAR_MAMEKEY(F17))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F18) PORT_CHAR(UCHAR_MAMEKEY(F18))
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("PF2") PORT_CODE(KEYCODE_SLASH_PAD) PORT_CHAR(UCHAR_MAMEKEY(SLASH_PAD))
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("PF3") PORT_CODE(KEYCODE_ASTERISK)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Num 8") PORT_CODE(KEYCODE_8_PAD) PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Num 5") PORT_CODE(KEYCODE_5_PAD) PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Num 6") PORT_CODE(KEYCODE_6_PAD) PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Num 3") PORT_CODE(KEYCODE_3_PAD) PORT_CHAR(UCHAR_MAMEKEY(3_PAD))

	PORT_START("KBD4")  // vertical row 16
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F19) PORT_CHAR(UCHAR_MAMEKEY(F19))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F20) PORT_CHAR(UCHAR_MAMEKEY(F20))
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("PF4") PORT_CODE(KEYCODE_MINUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD))
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Num 9") PORT_CODE(KEYCODE_9_PAD) PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Num ,") PORT_CODE(KEYCODE_PLUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD))
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Num -")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Num .") PORT_CODE(KEYCODE_DEL_PAD) PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))  // "." on num.pad
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Enter") PORT_CODE(KEYCODE_ENTER_PAD) PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD))
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor ms7004_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(ms7004);
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  ms7004_device - constructor
//-------------------------------------------------

ms7004_device::ms7004_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MS7004, tag, owner, clock)
	, m_maincpu(*this, MS7004_CPU_TAG)
	, m_speaker(*this, MS7004_SPK_TAG)
	, m_i8243(*this, "i8243")
	, m_kbd(*this, "KBD%u", 0)
	, m_tx_handler(*this)
	, m_rts_handler(*this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ms7004_device::device_start()
{
	m_tx_handler.resolve_safe();
	m_rts_handler.resolve_safe();
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void ms7004_device::device_reset()
{
	m_rts_handler(0);
}


WRITE_LINE_MEMBER(ms7004_device::write_rxd)
{
	m_maincpu->set_input_line(MCS48_INPUT_IRQ, state ? CLEAR_LINE : ASSERT_LINE);
}

//-------------------------------------------------
//  p1_w -
//-------------------------------------------------

void ms7004_device::p1_w(uint8_t data)
{
	/*
	    bit     description

	    0       Matrix row bit 0
	    1       Matrix row bit 1
	    2       Matrix row bit 2
	    3       Speaker
	    4       -STROBE (to matrix mux)
	    5       LED "Latin"
	    6
	    7       Serial TX
	*/
	LOGDBG("p1_w %02x = send %d\n", data, BIT(data, 7));

	m_p1 = data;
	m_tx_handler(BIT(data, 7));
}


//-------------------------------------------------
//  p2_w -
//-------------------------------------------------

void ms7004_device::p2_w(uint8_t data)
{
	/*
	    bit     description

	    0       Matrix columns, to 8243 (port 4)
	    1       Matrix columns, to 8243 (port 5)
	    2       Matrix columns, to 8243 (port 6)
	    3       Matrix columns, to 8243 (port 7)
	    4       LED "Wait"
	    5       LED "Compose"
	    6       LED "Caps"
	    7       LED "Hold"
	*/
	LOGDBG("p2_w %02x = col %d\n", data, data&15);

	m_p2 = data;
	m_i8243->p2_w(data);
}


//-------------------------------------------------
//  prog_w -
//-------------------------------------------------

template<int P>
void ms7004_device::i8243_port_w(uint8_t data)
{
	int sense = 0;

	LOGDBG("8243 port %d data %02xH\n", P + 4, data);

	if (data) {
		switch(data) {
			case 0x01: sense = m_kbd[(P << 2) + 0]->read(); break;
			case 0x02: sense = m_kbd[(P << 2) + 1]->read(); break;
			case 0x04: sense = m_kbd[(P << 2) + 2]->read(); break;
			case 0x08: sense = m_kbd[(P << 2) + 3]->read(); break;
		}
		m_keylatch = BIT(sense, (m_p1 & 7));
		if (m_keylatch)
		LOG("row %d col %02x t1 %d\n",
			(m_p1 & 7), (P << 4 | data), m_keylatch);
	}
}


//-------------------------------------------------
//  t1_r -
//-------------------------------------------------

READ_LINE_MEMBER(ms7004_device::t1_r)
{
	if (!BIT(m_p1, 4))
		return m_keylatch;
	else
		return 0;
}
