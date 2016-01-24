// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/*
    Elektronika MS 7004 keyboard (DEC LK-201 workalike with extra keys
    for Cyrillic characters).

    To do:
    - receive data from host (not used by KSM but used by other boards)
    - connect LEDs and speaker
*/

#include "emu.h"
#include "ms7004.h"

#define VERBOSE_DBG 1       /* general debug messages */

#define DBG_LOG(N,M,A) \
	do { \
	if(VERBOSE_DBG>=N) \
		{ \
			logerror("%11.6f at %s: ",machine().time().as_double(),machine().describe_context()); \
			logerror A; \
		} \
	} while (0)

//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define MS7004_CPU_TAG   "i8048"
#define MS7004_SPK_TAG   "beeper"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type MS7004 = &device_creator<ms7004_device>;

ROM_START( ms7004 )
	ROM_REGION (0x800, MS7004_CPU_TAG, 0)
	ROM_LOAD ("mc7004_keyboard_original.rom", 0x0000, 0x800, CRC(69fcab53) SHA1(2d7cc7cd182f2ee09ecf2c539e33db3c2195f778))
ROM_END

//-------------------------------------------------
//  ADDRESS_MAP
//-------------------------------------------------

static ADDRESS_MAP_START( ms7004_map, AS_IO, 8, ms7004_device )
	AM_RANGE(MCS48_PORT_P1, MCS48_PORT_P1) AM_WRITE(p1_w)
	AM_RANGE(MCS48_PORT_P2, MCS48_PORT_P2) AM_WRITE(p2_w)
	AM_RANGE(MCS48_PORT_T1, MCS48_PORT_T1) AM_READ(t1_r)
	AM_RANGE(MCS48_PORT_PROG, MCS48_PORT_PROG) AM_DEVWRITE("i8243", i8243_device, i8243_prog_w)
ADDRESS_MAP_END

//-------------------------------------------------
//  MACHINE_CONFIG
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( ms7004 )
	MCFG_CPU_ADD(MS7004_CPU_TAG, I8048, XTAL_4_608MHz)
	MCFG_CPU_IO_MAP(ms7004_map)

	MCFG_I8243_ADD("i8243", NOOP, WRITE8(ms7004_device, i8243_port_w))

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(MS7004_SPK_TAG, BEEP, 3250)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor ms7004_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( ms7004 );
}

const rom_entry *ms7004_device::device_rom_region() const
{
	return ROM_NAME( ms7004 );
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

0xc9    KEY_LANGLE_RANGLE   '?????????????????? ??????????????'
0xbc    KEY_DELETE  ????
0xbd    KEY_RETURN  ????
0xbf    KEY_TILDE   '; +'
0xc4    -       '??'
0xca    -       '/ ?'
0xed    KEY_PERIOD  '?? @'
0xf1    -       '_'
<...>

0x56    KEY_F1      ???????? ????????
0x57    KEY_F2      ???????????? ??????????
0x58    KEY_F3      ??????????
0x59    KEY_F4      ?????? ????????????
0x5a    KEY_F5      ??5

0x64    KEY_F6      ????????????
0x65    KEY_F7      ??????????????
0x66    KEY_F8      ??????????
0x67    KEY_F9      ???????????? ????????
0x69    KEY_F10     ??????????

0x71    KEY_F11     ??11 (????2)
0x72    KEY_F12     ??12 (????)
0x73    KEY_F13     ??13 (????)
0x74    KEY_F14     ?????? ??????????????

0x7c    KEY_HELP    ????
0x7d    KEY_MENU    ??????

0x80    KEY_F17     ??17
0x81    KEY_F18     ??18
0x82    KEY_F19     ??19
0x83    KEY_F20     ??20

0xb0    KEY_LOCK    ??????
0xae    KEY_SHIFT   ????
0xaf    KEY_CTRL    ????

0xb1    KEY_META    ??????
0xb2    -       ??????/??????

0x8a    KEY_FIND    ????
0x8b    KEY_INSERT_HERE ??????
0x8c    KEY_REMOVE  ????????
0x8d    KEY_SELECT  ????????
0x8e    KEY_PREV_SCREEN ???????? ????????
0x8f    KEY_NEXT_SCREEN ???????? ????????

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
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Print Screen (F2)") PORT_CODE(KEYCODE_F2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Hold Screen (F1)") PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED ) // '{' / '|'
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(";") PORT_CODE(KEYCODE_COLON) // '+'
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Tab") PORT_CODE(KEYCODE_TAB)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Ctrl") PORT_CODE(KEYCODE_LCONTROL)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Shift Lock") PORT_CODE(KEYCODE_CAPSLOCK)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("LShift") PORT_CODE(KEYCODE_LSHIFT)

	PORT_START("KBD13") // vertical row 2
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Setup (F3)") PORT_CODE(KEYCODE_F3)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Data / Talk (F4)") PORT_CODE(KEYCODE_F4)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("1") PORT_CODE(KEYCODE_1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("J") PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("C") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F") PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Rus/Lat") PORT_CODE(KEYCODE_ESC)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Compose") PORT_CODE(KEYCODE_LALT)

	PORT_START("KBD14") // vertical row 3
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Break (F5)") PORT_CODE(KEYCODE_F5)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("2") PORT_CODE(KEYCODE_2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("U") PORT_CODE(KEYCODE_U)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Y") PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Tilde") PORT_CODE(KEYCODE_TILDE) // ^
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Q") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KBD15") // vertical row 4
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("3") PORT_CODE(KEYCODE_3)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("4") PORT_CODE(KEYCODE_4)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("K") PORT_CODE(KEYCODE_K)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("W") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("S") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KBD11") // vertical row 5
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Interrupt (F6)") PORT_CODE(KEYCODE_F6)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("5") PORT_CODE(KEYCODE_5)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("E") PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("P") PORT_CODE(KEYCODE_P)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("A") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("M") PORT_CODE(KEYCODE_M)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KBD10") // vertical row 6
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Resume (F7)") PORT_CODE(KEYCODE_F7)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("6") PORT_CODE(KEYCODE_6)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("N") PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("G") PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("R") PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("T") PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("I") PORT_CODE(KEYCODE_I)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE)

	PORT_START("KBD9")  // vertical row 7
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Cancel (F8)") PORT_CODE(KEYCODE_F8)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Main Screen (F9)") PORT_CODE(KEYCODE_F9)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("7") PORT_CODE(KEYCODE_7)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("[") PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("]") PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("O") PORT_CODE(KEYCODE_O)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("X") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("B") PORT_CODE(KEYCODE_B)

	PORT_START("KBD8")  // vertical row 8
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Exit (F10)") PORT_CODE(KEYCODE_F10)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("9") PORT_CODE(KEYCODE_9)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("8") PORT_CODE(KEYCODE_8)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Z") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("D") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("L") PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED ) // '@'
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(",") PORT_CODE(KEYCODE_COMMA)

	PORT_START("KBD7")  // vertical row 9
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("ESC (F11)") PORT_CODE(KEYCODE_F11)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("0") PORT_CODE(KEYCODE_0)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("=") PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("H") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("V") PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("\\") PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("/") PORT_CODE(KEYCODE_SLASH)

	PORT_START("KBD6")  // vertical row 10
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("BS (F12)") PORT_CODE(KEYCODE_F12)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("LF (F13)") PORT_CODE(KEYCODE_PRTSCR)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED ) // '}'
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("'") PORT_CODE(KEYCODE_QUOTE) // ':'
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED ) // '??'
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(".") PORT_CODE(KEYCODE_STOP)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("-") PORT_CODE(KEYCODE_MINUS)

	PORT_START("KBD5")  // vertical row 11
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Additional Options (F14)") PORT_CODE(KEYCODE_PAUSE)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Delete <X") PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED ) // ???
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Return") PORT_CODE(KEYCODE_ENTER)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("RShift") PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KBD3")  // vertical row 12
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Help (F15)") PORT_CODE(KEYCODE_RALT)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Insert Here") PORT_CODE(KEYCODE_HOME)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Find") PORT_CODE(KEYCODE_INSERT)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Select") PORT_CODE(KEYCODE_DEL)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Up") PORT_CODE(KEYCODE_UP)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KBD2")  // vertical row 13
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Remove") PORT_CODE(KEYCODE_PGUP)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Next [v]") PORT_CODE(KEYCODE_PGDN)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Previous [^]") PORT_CODE(KEYCODE_END)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Right") PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Down") PORT_CODE(KEYCODE_DOWN)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KBD1")  // vertical row 14
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Do (F16)") PORT_CODE(KEYCODE_RCONTROL)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("PF1") PORT_CODE(KEYCODE_NUMLOCK)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Num 7") PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Num 4") PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Num 1") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Num 2") PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Num 0") PORT_CODE(KEYCODE_0_PAD)

	PORT_START("KBD0")  // vertical row 15
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F17")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F18")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("PF2") PORT_CODE(KEYCODE_SLASH_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("PF3") PORT_CODE(KEYCODE_ASTERISK)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Num 8") PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Num 5") PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Num 6") PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Num 3") PORT_CODE(KEYCODE_3_PAD)

	PORT_START("KBD4")  // vertical row 16
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F19")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F20")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("PF4") PORT_CODE(KEYCODE_MINUS_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Num 9") PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Num ,") PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Num -")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Num .") PORT_CODE(KEYCODE_DEL_PAD)  // "." on num.pad
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Enter") PORT_CODE(KEYCODE_ENTER_PAD)
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor ms7004_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( ms7004 );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  ms7004_device - constructor
//-------------------------------------------------

ms7004_device::ms7004_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, MS7004, "MS7004 keyboard", tag, owner, clock, "ms7004", __FILE__),
	m_maincpu(*this, MS7004_CPU_TAG),
	m_speaker(*this, MS7004_SPK_TAG),
	m_i8243(*this, "i8243"),
	m_kbd0(*this, "KBD0"),
	m_kbd1(*this, "KBD1"),
	m_kbd2(*this, "KBD2"),
	m_kbd3(*this, "KBD3"),
	m_kbd4(*this, "KBD4"),
	m_kbd5(*this, "KBD5"),
	m_kbd6(*this, "KBD6"),
	m_kbd7(*this, "KBD7"),
	m_kbd8(*this, "KBD8"),
	m_kbd9(*this, "KBD9"),
	m_kbd10(*this, "KBD10"),
	m_kbd11(*this, "KBD11"),
	m_kbd12(*this, "KBD12"),
	m_kbd13(*this, "KBD13"),
	m_kbd14(*this, "KBD14"),
	m_kbd15(*this, "KBD15"),
	m_tx_handler(*this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ms7004_device::device_start()
{
	m_tx_handler.resolve_safe();
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void ms7004_device::device_reset()
{
}


//-------------------------------------------------
//  p1_w -
//-------------------------------------------------

WRITE8_MEMBER( ms7004_device::p1_w )
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
	DBG_LOG(2,0,( "%s: p1_w %02x = send %d\n", tag(), data, BIT(data, 7)));

	m_p1 = data;
	m_tx_handler(BIT(data, 7));
}


//-------------------------------------------------
//  p2_w -
//-------------------------------------------------

WRITE8_MEMBER( ms7004_device::p2_w )
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
	DBG_LOG(2,0,( "%s: p2_w %02x = col %d\n", tag(), data, data&15));

	m_p2 = data;
	m_i8243->i8243_p2_w(space, offset, data);
}


//-------------------------------------------------
//  prog_w -
//-------------------------------------------------

WRITE8_MEMBER( ms7004_device::i8243_port_w )
{
	int sense = 0;

	DBG_LOG(2,0,( "%s: 8243 port %d data %02xH\n",
		tag(), offset + 4, data));

	if (data) {
		switch(offset << 4 | data) {
			case 0x01: sense = m_kbd0->read(); break;
			case 0x02: sense = m_kbd1->read(); break;
			case 0x04: sense = m_kbd2->read(); break;
			case 0x08: sense = m_kbd3->read(); break;
			case 0x11: sense = m_kbd4->read(); break;
			case 0x12: sense = m_kbd5->read(); break;
			case 0x14: sense = m_kbd6->read(); break;
			case 0x18: sense = m_kbd7->read(); break;
			case 0x21: sense = m_kbd8->read(); break;
			case 0x22: sense = m_kbd9->read(); break;
			case 0x24: sense = m_kbd10->read(); break;
			case 0x28: sense = m_kbd11->read(); break;
			case 0x31: sense = m_kbd12->read(); break;
			case 0x32: sense = m_kbd13->read(); break;
			case 0x34: sense = m_kbd14->read(); break;
			case 0x38: sense = m_kbd15->read(); break;
		}
		m_keylatch = BIT(sense, (m_p1 & 7));
		if (m_keylatch)
		DBG_LOG(1,0,( "%s: row %d col %02x t1 %d\n",
			tag(), (m_p1 & 7), (offset << 4 | data), m_keylatch));
	}
}


//-------------------------------------------------
//  t1_r -
//-------------------------------------------------

READ8_MEMBER( ms7004_device::t1_r )
{
	if (!BIT(m_p1,4))
		return m_keylatch;
	else
		return 0;
}
