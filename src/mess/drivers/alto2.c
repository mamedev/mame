/***************************************************************************
 *   Portable Xerox AltoII driver for MESS
 *
 *   Copyright: Juergen Buchmueller <pullmoll@t-online.de>
 *
 *   Licenses: MAME, GPLv2
 ***************************************************************************/

#include "includes/alto2.h"

/* Video */

UINT32 alto2_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void alto2_state::screen_eof_alto2(screen_device &screen, bool state)
{

}

/* Input Ports */

/** @brief make an Alto key int from 1 << bit */
#define	MAKE_KEY(a,b) (1 << (b))

/** @brief no key assigned is -1 */
#define	A2_KEY_NONE	(-1)

#define	A2_KEY_5			MAKE_KEY(0,017)		//!< normal: 5    shifted: %
#define	A2_KEY_4			MAKE_KEY(0,016)		//!< normal: 4    shifted: $
#define	A2_KEY_6			MAKE_KEY(0,015)		//!< normal: 6    shifted: ~
#define	A2_KEY_E			MAKE_KEY(0,014)		//!< normal: e    shifted: E
#define	A2_KEY_7			MAKE_KEY(0,013)		//!< normal: 7    shifted: &
#define	A2_KEY_D			MAKE_KEY(0,012)		//!< normal: d    shifted: D
#define	A2_KEY_U			MAKE_KEY(0,011)		//!< normal: u    shifted: U
#define	A2_KEY_V			MAKE_KEY(0,010)		//!< normal: v    shifted: V
#define	A2_KEY_0			MAKE_KEY(0,007)		//!< normal: 0    shifted: )
#define	A2_KEY_K			MAKE_KEY(0,006)		//!< normal: k    shifted: K
#define	A2_KEY_MINUS		MAKE_KEY(0,005)		//!< normal: -    shifted: _
#define	A2_KEY_P			MAKE_KEY(0,004)		//!< normal: p    shifted: P
#define	A2_KEY_SLASH		MAKE_KEY(0,003)		//!< normal: /    shifted: ?
#define	A2_KEY_BACKSLASH	MAKE_KEY(0,002)		//!< normal: \    shifted: |
#define	A2_KEY_LF			MAKE_KEY(0,001)		//!< normal: LF   shifted: ?
#define	A2_KEY_BS			MAKE_KEY(0,000)		//!< normal: BS   shifted: ?

#define	A2_KEY_FR2			MAKE_KEY(0,002)		//!< ADL right function key 2
#define	A2_KEY_FL2			MAKE_KEY(0,001)		//!< ADL left function key 1

#define	A2_KEY_3			MAKE_KEY(1,017)		//!< normal: 3    shifted: #
#define	A2_KEY_2			MAKE_KEY(1,016)		//!< normal: 2    shifted: @
#define	A2_KEY_W			MAKE_KEY(1,015)		//!< normal: w    shifted: W
#define	A2_KEY_Q			MAKE_KEY(1,014)		//!< normal: q    shifted: Q
#define	A2_KEY_S			MAKE_KEY(1,013)		//!< normal: s    shifted: S
#define	A2_KEY_A			MAKE_KEY(1,012)		//!< normal: a    shifted: A
#define	A2_KEY_9			MAKE_KEY(1,011)		//!< normal: 9    shifted: (
#define	A2_KEY_I			MAKE_KEY(1,010)		//!< normal: i    shifted: I
#define	A2_KEY_X			MAKE_KEY(1,007)		//!< normal: x    shifted: X
#define	A2_KEY_O			MAKE_KEY(1,006)		//!< normal: o    shifted: O
#define	A2_KEY_L			MAKE_KEY(1,005)		//!< normal: l    shifted: L
#define	A2_KEY_COMMA		MAKE_KEY(1,004)		//!< normal: ,    shifted: <
#define	A2_KEY_QUOTE		MAKE_KEY(1,003)		//!< normal: '    shifted: "
#define	A2_KEY_RBRACKET		MAKE_KEY(1,002)		//!< normal: ]    shifted: }
#define	A2_KEY_BLANK_MID	MAKE_KEY(1,001)		//!< middle blank key
#define	A2_KEY_BLANK_TOP	MAKE_KEY(1,000)		//!< top blank key

#define	A2_KEY_FR4			MAKE_KEY(1,001)		//!< ADL right funtion key 4
#define	A2_KEY_BW			MAKE_KEY(1,000)		//!< ADL BW (?)

#define	A2_KEY_1			MAKE_KEY(2,017)		//!< normal: 1    shifted: !
#define	A2_KEY_ESCAPE		MAKE_KEY(2,016)		//!< normal: ESC  shifted: ?
#define	A2_KEY_TAB			MAKE_KEY(2,015)		//!< normal: TAB  shifted: ?
#define	A2_KEY_F			MAKE_KEY(2,014)		//!< normal: f    shifted: F
#define	A2_KEY_CTRL			MAKE_KEY(2,013)		//!< CTRL
#define	A2_KEY_C			MAKE_KEY(2,012)		//!< normal: c    shifted: C
#define	A2_KEY_J			MAKE_KEY(2,011)		//!< normal: j    shifted: J
#define	A2_KEY_B			MAKE_KEY(2,010)		//!< normal: b    shifted: B
#define	A2_KEY_Z			MAKE_KEY(2,007)		//!< normal: z    shifted: Z
#define	A2_KEY_LSHIFT		MAKE_KEY(2,006)		//!< LSHIFT
#define	A2_KEY_PERIOD		MAKE_KEY(2,005)		//!< normal: .    shifted: >
#define	A2_KEY_SEMICOLON	MAKE_KEY(2,004)		//!< normal: ;    shifted: :
#define	A2_KEY_RETURN		MAKE_KEY(2,003)		//!< RETURN
#define	A2_KEY_LEFTARROW	MAKE_KEY(2,002)		//!< normal: left arrow   shifted: up arrow (caret)
#define	A2_KEY_DEL			MAKE_KEY(2,001)		//!< normal: DEL  shifted: ?
#define	A2_KEY_MSW_2_17		MAKE_KEY(2,000)		//!< unused on Microswitch KDB

#define	A2_KEY_FR3			MAKE_KEY(2,002)		//!< ADL right function key 3
#define	A2_KEY_FL1			MAKE_KEY(2,001)		//!< ADL left function key 1
#define	A2_KEY_FL3			MAKE_KEY(2,000)		//!< ADL left function key 3

#define	A2_KEY_R			MAKE_KEY(3,017)		//!< normal: r    shifted: R
#define	A2_KEY_T			MAKE_KEY(3,016)		//!< normal: t    shifted: T
#define	A2_KEY_G			MAKE_KEY(3,015)		//!< normal: g    shifted: G
#define	A2_KEY_Y			MAKE_KEY(3,014)		//!< normal: y    shifted: Y
#define	A2_KEY_H			MAKE_KEY(3,013)		//!< normal: h    shifted: H
#define	A2_KEY_8			MAKE_KEY(3,012)		//!< normal: 8    shifted: *
#define	A2_KEY_N			MAKE_KEY(3,011)		//!< normal: n    shifted: N
#define	A2_KEY_M			MAKE_KEY(3,010)		//!< normal: m    shifted: M
#define	A2_KEY_LOCK			MAKE_KEY(3,007)		//!< LOCK
#define	A2_KEY_SPACE		MAKE_KEY(3,006)		//!< SPACE
#define	A2_KEY_LBRACKET		MAKE_KEY(3,005)		//!< normal: [    shifted: {
#define	A2_KEY_EQUALS		MAKE_KEY(3,004)		//!< normal: =    shifted: +
#define	A2_KEY_RSHIFT		MAKE_KEY(3,003)		//!< RSHIFT
#define	A2_KEY_BLANK_BOT	MAKE_KEY(3,002)		//!< bottom blank key
#define	A2_KEY_MSW_3_16		MAKE_KEY(3,001)		//!< unused on Microswitch KDB
#define	A2_KEY_MSW_3_17		MAKE_KEY(3,000)		//!< unused on Microswitch KDB

#define	A2_KEY_FR1			MAKE_KEY(3,002)		//!< ADL right function key 4
#define	A2_KEY_FL4			MAKE_KEY(3,001)		//!< ADL left function key 4
#define	A2_KEY_FR5			MAKE_KEY(3,000)		//!< ADL right function key 5

static INPUT_PORTS_START( alto2 )
	PORT_START("ROW0")
	PORT_BIT(A2_KEY_5,			IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5 %") PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')	//!< normal: 5    shifted: %
	PORT_BIT(A2_KEY_4,			IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4 $") PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')	//!< normal: 4    shifted: $
	PORT_BIT(A2_KEY_6,			IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6 ~") PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('~')	//!< normal: 6    shifted: ~
	PORT_BIT(A2_KEY_E,			IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("e E") PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')	//!< normal: e    shifted: E
	PORT_BIT(A2_KEY_7,			IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7 &") PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('&')	//!< normal: 7    shifted: &
	PORT_BIT(A2_KEY_D,			IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("d D") PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')	//!< normal: d    shifted: D
	PORT_BIT(A2_KEY_U,			IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("u U") PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')	//!< normal: u    shifted: U
	PORT_BIT(A2_KEY_V,			IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("v V") PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')	//!< normal: v    shifted: V
	PORT_BIT(A2_KEY_0,			IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0 )") PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR(')')	//!< normal: 0    shifted: )
	PORT_BIT(A2_KEY_K,			IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("k K") PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')	//!< normal: k    shifted: K
	PORT_BIT(A2_KEY_MINUS,		IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("- _") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('_')	//!< normal: -    shifted: _
	PORT_BIT(A2_KEY_P,			IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("p P") PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')	//!< normal: p    shifted: P
	PORT_BIT(A2_KEY_SLASH,		IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("/ ?") PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')	//!< normal: /    shifted: ?
	PORT_BIT(A2_KEY_BACKSLASH,	IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\\ |") PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\') PORT_CHAR('|')	//!< normal: \    shifted: |
	PORT_BIT(A2_KEY_LF,			IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("LF") PORT_CODE(KEYCODE_DOWN) PORT_CHAR('\012')				//!< normal: LF   shifted: ?
	PORT_BIT(A2_KEY_BS,			IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("BS") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR('\010')			//!< normal: BS   shifted: ?

	PORT_START("ROW1")
	PORT_BIT(A2_KEY_3,			IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3 #") PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')	//!< normal: 3    shifted: #
	PORT_BIT(A2_KEY_2,			IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2 @") PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('@')	//!< normal: 2    shifted: @
	PORT_BIT(A2_KEY_W,			IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("w W") PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')	//!< normal: w    shifted: W
	PORT_BIT(A2_KEY_Q,			IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("q Q") PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')	//!< normal: q    shifted: Q
	PORT_BIT(A2_KEY_S,			IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("s S") PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')	//!< normal: s    shifted: S
	PORT_BIT(A2_KEY_A,			IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("a A") PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')	//!< normal: a    shifted: A
	PORT_BIT(A2_KEY_9,			IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9 (") PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR('(')	//!< normal: 9    shifted: (
	PORT_BIT(A2_KEY_I,			IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("i I") PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')	//!< normal: i    shifted: I
	PORT_BIT(A2_KEY_X,			IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("x X") PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')	//!< normal: x    shifted: X
	PORT_BIT(A2_KEY_O,			IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("o O") PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')	//!< normal: o    shifted: O
	PORT_BIT(A2_KEY_L,			IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("l L") PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')	//!< normal: l    shifted: L
	PORT_BIT(A2_KEY_COMMA,		IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(", <") PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')	//!< normal: ,    shifted: <
	PORT_BIT(A2_KEY_QUOTE,		IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("' \"") PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('\x27') PORT_CHAR('"')	//!< normal: '    shifted: "
	PORT_BIT(A2_KEY_RBRACKET,	IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("] }") PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')	//!< normal: ]    shifted: }
	PORT_BIT(A2_KEY_BLANK_MID,	IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("M[ ]") PORT_CODE(KEYCODE_END)								//!< middle blank key
	PORT_BIT(A2_KEY_BLANK_TOP,	IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("T[ ]") PORT_CODE(KEYCODE_PGUP)								//!< top blank key

	PORT_START("ROW2")
	PORT_BIT(A2_KEY_1,			IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1 !") PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')	//!< normal: 1    shifted: !
	PORT_BIT(A2_KEY_ESCAPE,		IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ESC") PORT_CODE(KEYCODE_ESC) PORT_CHAR('\x1b')				//!< normal: ESC  shifted: ?
	PORT_BIT(A2_KEY_TAB,		IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("TAB") PORT_CODE(KEYCODE_TAB) PORT_CHAR('\011')				//!< normal: TAB  shifted: ?
	PORT_BIT(A2_KEY_F,			IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("f F") PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')	//!< normal: f    shifted: F
	PORT_BIT(A2_KEY_CTRL,		IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CTRL") PORT_CODE(KEYCODE_LCONTROL)							//!< CTRL
	PORT_BIT(A2_KEY_C,			IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("c C") PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')	//!< normal: c    shifted: C
	PORT_BIT(A2_KEY_J,			IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("j J") PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')	//!< normal: j    shifted: J
	PORT_BIT(A2_KEY_B,			IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("b B") PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')	//!< normal: b    shifted: B
	PORT_BIT(A2_KEY_Z,			IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("z Z") PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')	//!< normal: z    shifted: Z
	PORT_BIT(A2_KEY_LSHIFT,		IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("LSHIFT") PORT_CODE(KEYCODE_LSHIFT)							//!< LSHIFT
	PORT_BIT(A2_KEY_PERIOD,		IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(". >") PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')	//!< normal: .    shifted: >
	PORT_BIT(A2_KEY_SEMICOLON,	IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("; :") PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR(':')//!< normal: ;    shifted: :
	PORT_BIT(A2_KEY_RETURN,		IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("RETURN") PORT_CODE(KEYCODE_ENTER) PORT_CHAR('\013')			//!< RETURN
	PORT_BIT(A2_KEY_LEFTARROW,	IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("← ↑") PORT_CODE(KEYCODE_LEFT)								//!< normal: left arrow   shifted: up arrow (caret)
	PORT_BIT(A2_KEY_DEL,		IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("DEL") PORT_CODE(KEYCODE_DEL)								//!< normal: DEL  shifted: ?
	PORT_BIT(A2_KEY_MSW_2_17,	IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2/17") PORT_CODE(KEYCODE_MENU)								//!< unused on Microswitch KDB

	PORT_START("ROW3")
	PORT_BIT(A2_KEY_R,			IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("r R") PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')	//!< normal: r    shifted: R
	PORT_BIT(A2_KEY_T,			IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("t T") PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')	//!< normal: t    shifted: T
	PORT_BIT(A2_KEY_G,			IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("g G") PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')	//!< normal: g    shifted: G
	PORT_BIT(A2_KEY_Y,			IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("y Y") PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')	//!< normal: y    shifted: Y
	PORT_BIT(A2_KEY_H,			IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("h H") PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')	//!< normal: h    shifted: H
	PORT_BIT(A2_KEY_8,			IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8 *") PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('*')	//!< normal: 8    shifted: *
	PORT_BIT(A2_KEY_N,			IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("n N") PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')	//!< normal: n    shifted: N
	PORT_BIT(A2_KEY_M,			IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("m M") PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')	//!< normal: m    shifted: M
	PORT_BIT(A2_KEY_LOCK,		IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("LOCK") PORT_CODE(KEYCODE_SCRLOCK)							//!< LOCK
	PORT_BIT(A2_KEY_SPACE,		IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SPACE") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')					//!< SPACE
	PORT_BIT(A2_KEY_L,			IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("[ {") PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_CHAR('{')	//!< normal: [    shifted: {
	PORT_BIT(A2_KEY_EQUALS,		IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("= +") PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=') PORT_CHAR('+')	//!< normal: =    shifted: +
	PORT_BIT(A2_KEY_RSHIFT,		IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("RSHIFT") PORT_CODE(KEYCODE_RSHIFT)							//!< RSHIFT
	PORT_BIT(A2_KEY_BLANK_BOT,	IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B[ ]") PORT_CODE(KEYCODE_PGDN)								//!< bottom blank key
	PORT_BIT(A2_KEY_MSW_3_16,	IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3/16") PORT_CODE(KEYCODE_HOME)								//!< unused on Microswitch KDB
	PORT_BIT(A2_KEY_MSW_3_17,	IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3/17") PORT_CODE(KEYCODE_INSERT)							//!< unused on Microswitch KDB

	PORT_START("ROW4")
	PORT_BIT(A2_KEY_FR2,		IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("FR2") PORT_CODE(KEYCODE_F6)									//!< ADL right function key 2
	PORT_BIT(A2_KEY_FL2,		IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("FL2") PORT_CODE(KEYCODE_F2)									//!< ADL left function key 2

	PORT_START("ROW5")
	PORT_BIT(A2_KEY_FR4,		IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("FR4") PORT_CODE(KEYCODE_F8)									//!< ADL right funtion key 4
	PORT_BIT(A2_KEY_BW,			IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("BW")  PORT_CODE(KEYCODE_F10)								//!< ADL BW (?)

	PORT_START("ROW6")
	PORT_BIT(A2_KEY_FR3,		IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("FR3") PORT_CODE(KEYCODE_F7)									//!< ADL right function key 3
	PORT_BIT(A2_KEY_FL1,		IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("FL1") PORT_CODE(KEYCODE_F1)									//!< ADL left function key 1
	PORT_BIT(A2_KEY_FL3,		IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("FL3") PORT_CODE(KEYCODE_F3)									//!< ADL left function key 3

	PORT_START("ROW7")
	PORT_BIT(A2_KEY_FR1,		IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("FR1") PORT_CODE(KEYCODE_F5)									//!< ADL right function key 1
	PORT_BIT(A2_KEY_FL4,		IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("FL4") PORT_CODE(KEYCODE_F4)									//!< ADL left function key 4
	PORT_BIT(A2_KEY_FR5,		IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("FR5") PORT_CODE(KEYCODE_F9)									//!< ADL right function key 5

	PORT_START("CONFIG")    /* config diode on main board */
	PORT_CONFNAME( 0x40, 0x40, "TV system")
	PORT_CONFSETTING(    0x00, "NTSC")
	PORT_CONFSETTING(    0x40, "PAL")
INPUT_PORTS_END

/* ROM */
ROM_START( alto2 )
	// dummy region for the maincpu
	ROM_REGION( 1, "maincpu", 0 )
	ROM_FILL(0, 1, ALTO2_UCODE_INVERTED)
ROM_END

//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

ADDRESS_MAP_START( alto2_ucode_map, AS_0, 32, alto2_state )
	AM_RANGE(0, ALTO2_UCODE_SIZE-1) AM_DEVICE32( "maincpu", alto2_cpu_device, ucode_map, 0xffffffffUL )
ADDRESS_MAP_END

ADDRESS_MAP_START( alto2_const_map, AS_1, 16, alto2_state )
	AM_RANGE(0, ALTO2_CONST_SIZE-1) AM_DEVICE16( "maincpu", alto2_cpu_device, const_map, 0xffffU )
ADDRESS_MAP_END

ADDRESS_MAP_START( alto2_iomem_map, AS_2, 16, alto2_state )
	AM_RANGE(0, ALTO2_RAM_SIZE-1)   AM_DEVICE16( "maincpu", alto2_cpu_device, iomem_map, 0xffffU )
ADDRESS_MAP_END


/* Palette Initialization */

void alto2_state::palette_init()
{
	palette_set_color(machine(),0,RGB_WHITE); /* white */
	palette_set_color(machine(),1,RGB_BLACK); /* black */
}

static MACHINE_CONFIG_START( alto2, alto2_state )
	/* basic machine hardware */
	// SYSCLK is Display Control part A51 (tagged 29.4MHz) divided by 8
	MCFG_CPU_ADD("maincpu", ALTO2, XTAL_29_4912MHz/8)
	MCFG_CPU_PROGRAM_MAP(alto2_ucode_map)
	MCFG_CPU_DATA_MAP(alto2_const_map)
	MCFG_CPU_IO_MAP(alto2_iomem_map)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(XTAL_20_16MHz, ALTO2_DISPLAY_TOTAL_WIDTH, 0, ALTO2_DISPLAY_WIDTH, ALTO2_DISPLAY_TOTAL_HEIGHT, 0, ALTO2_DISPLAY_HEIGHT)
	MCFG_SCREEN_UPDATE_DRIVER(alto2_state, screen_update)
	MCFG_SCREEN_VBLANK_DRIVER(alto2_state, screen_eof_alto2)

	MCFG_PALETTE_LENGTH(2)

	MCFG_DIABLO_DRIVES_ADD()
MACHINE_CONFIG_END

/* Driver Init */

DRIVER_INIT_MEMBER( alto2_state, alto2 )
{
	// make the diablo drives known to the CPU core
	alto2_cpu_device* cpu = downcast<alto2_cpu_device *>(m_maincpu.target());
	cpu->set_diablo(0, downcast<diablo_hd_device *>(machine().device(DIABLO_HD_0)));
	cpu->set_diablo(1, downcast<diablo_hd_device *>(machine().device(DIABLO_HD_1)));

#define	UNICODE_TESTING	0
#if	UNICODE_TESTING
	const char* filename = "docs/UnicodeData.txt";
	int res = unicode_data_load(filename);
	logerror("%s: unicode_data_load(\"%s\") result %d\n", filename, __FUNCTION__, res);
	for (unicode_char uchar = 0; uchar < UNICODE_PLANESIZE; uchar++) {
		if (UNICODE_NOT_DECIMAL != unicode_decimal(uchar))
			logerror("%s: U+%04x (%s) is decimal %u\n", __FUNCTION__, uchar, unicode_name(uchar), unicode_decimal(uchar));
		if (UNICODE_NOT_DIGIT != unicode_digit(uchar))
			logerror("%s: U+%04x (%s) is digit %u\n", __FUNCTION__, uchar, unicode_name(uchar), unicode_digit(uchar));
		if (UNICODE_NOT_NUMERIC != unicode_digit(uchar))
			logerror("%s: U+%04x (%s) is numeric %u\n", __FUNCTION__, uchar, unicode_name(uchar), unicode_numeric(uchar));
		if (uchar != unicode_lcase(uchar))
			logerror("%s: U+%04x (%s) lower chase is U+%04x (%s)\n", __FUNCTION__, uchar, unicode_name(uchar), unicode_lcase(uchar), unicode_name(unicode_lcase(uchar)));
	}
	unicode_data_free();
#endif
}

/* Game Drivers */

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT   COMPANY  FULLNAME   FLAGS
COMP( 1974, alto2,  0,      0,      alto2,   alto2, alto2_state, alto2, "Xerox", "Alto-II", GAME_NOT_WORKING | GAME_NO_SOUND )
