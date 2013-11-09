// license:MAME
// copyright-holders:Juergen Buchmueller
/***************************************************************************
 *    alto2.c
 *
 *    Original driver by:
 *    Juergen Buchmueller, Nov 2013
 *
 ***************************************************************************/

#include "includes/alto2.h"
#include "cpu/alto2/alto2.h"
#include "cpu/alto2/a2roms.h"

// FIXME: Is this required? How to access the address space words?
// This has to somehow be mapped to the a2mem half DWORD accesses
// and (optionally) Hamming code and parity flag updating
READ16_MEMBER( alto2_state::alto2_ram_r )
{
	return 0;
}

// FIXME: Is this required? How to access the address space words?
// This has to somehow be mapped to the a2mem half DWORD accesses
// and (optionally) Hamming code and parity flag updating
WRITE16_MEMBER( alto2_state::alto2_ram_w )
{

}

// FIXME: Dispatch to the a2mem mmio handlers
READ16_MEMBER( alto2_state::alto2_mmio_r )
{
	return 0;
}

// FIXME: Dispatch to the a2mem mmio handlers
WRITE16_MEMBER( alto2_state::alto2_mmio_w )
{

}

/* Memory Maps */

/* micro code from ALTO2_UCODE_ROM_PAGES times PROMs and ALTO2_UCODE_RAM_PAGES times RAMs of ALTO2_UCODE_PAGE_SIZE 32-bit words each */
static ADDRESS_MAP_START( alto2_ucode_map, AS_PROGRAM, 32, alto2_state )
	AM_RANGE(0,                    ALTO2_UCODE_RAM_BASE-1) AM_ROM
	AM_RANGE(ALTO2_UCODE_RAM_BASE, ALTO2_UCODE_SIZE-1)     AM_RAM
ADDRESS_MAP_END

/* constant PROM with 256 16-bit words */
static ADDRESS_MAP_START( alto2_const_map, AS_DATA, 16, alto2_state )
	AM_RANGE(0,                    ALTO2_CONST_SIZE-1)     AM_ROM
ADDRESS_MAP_END

/* main memory and memory mapped i/o in range ALTO2_IO_PAGE_BASE ... ALTO2_IO_PAGE_BASE + ALTO2_IO_PAGE_SIZE - 1 */
static ADDRESS_MAP_START( alto2_ram_map, AS_IO, 16, alto2_state )
	AM_RANGE(0,                    ALTO2_IO_PAGE_BASE - 1) AM_READWRITE(alto2_ram_r,  alto2_ram_w)
	AM_RANGE(ALTO2_IO_PAGE_BASE,   0177777)                AM_READWRITE(alto2_mmio_r, alto2_mmio_w)
ADDRESS_MAP_END


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


/* Palette Initialization */

void alto2_state::palette_init()
{
	palette_set_color(machine(),0,RGB_WHITE); /* white */
	palette_set_color(machine(),1,RGB_BLACK); /* black */
}

static MACHINE_CONFIG_START( alto2, alto2_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", ALTO2, XTAL_20_16MHz)
	MCFG_CPU_PROGRAM_MAP(alto2_ucode_map)
	MCFG_CPU_DATA_MAP(alto2_const_map)
	MCFG_CPU_IO_MAP(alto2_ram_map)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(XTAL_20_16MHz, ALTO2_DISPLAY_TOTAL_WIDTH, 0, ALTO2_DISPLAY_WIDTH, ALTO2_DISPLAY_TOTAL_HEIGHT, 0, ALTO2_DISPLAY_HEIGHT)
	MCFG_SCREEN_UPDATE_DRIVER(alto2_state, screen_update)
	MCFG_SCREEN_VBLANK_DRIVER(alto2_state, screen_eof_alto2)

	MCFG_PALETTE_LENGTH(2)
MACHINE_CONFIG_END

/* ROMs */

ROM_START( alto2 )
	// FIXME: how to allocate initally empty regions for AS_PROGRAM, AS_DATA and AS_IO?
	ROM_REGION( 4*ALTO2_UCODE_SIZE, "maincpu", 0)
	ROM_REGION( 2*ALTO2_CONST_SIZE, "constants", 0)
	ROM_REGION( ALTO2_RAM_SIZE, "memory", 0 )

	// Alto-II micro code PROMs, 8 x 4bit
	ROM_REGION( 16 * 02000, "ucode", 0 )
	ROM_LOAD( "55x.3",     0*02000, 0x400, CRC(de870d75) SHA1(2b98cc769d8302cb39948711424d987d94e4159b) )	//!< 00000-01777 RSEL(0)',RSEL(1)',RSEL(2)',RSEL(3)'
	ROM_LOAD( "64x.3",     1*02000, 0x400, CRC(51b444c0) SHA1(8756e51f7f3253a55d75886465beb7ee1be6e1c4) )	//!< 00000-01777 RSEL(4)',ALUF(0)',ALUF(1)',ALUF(2)'
	ROM_LOAD( "65x.3",     2*02000, 0x400, CRC(741d1437) SHA1(01f7cf07c2173ac93799b2475180bfbbe7e0149b) )	//!< 00000-01777 ALUF(3)',BS(0)',BS(1)',BS(2)'
	ROM_LOAD( "63x.3",     3*02000, 0x400, CRC(f22d5028) SHA1(c65a42baef702d4aff2d9ad8e363daec27de6801) )	//!< 00000-01777 F1(0),F1(1)',F1(2)',F1(3)'
	ROM_LOAD( "53x.3",     4*02000, 0x400, CRC(3c89a740) SHA1(95d812d489b2bde03884b2f126f961caa6c8ec45) )	//!< 00000-01777 F2(0),F2(1)',F2(2)',F2(3)'
	ROM_LOAD( "60x.3",     5*02000, 0x400, CRC(a35de0bf) SHA1(7fa4aead44dcf5393bbfd1706c0ada24aa6fd3ac) )	//!< 00000-01777 LOADT',LOADL,NEXT(0)',NEXT(1)'
	ROM_LOAD( "61x.3",     6*02000, 0x400, CRC(f25bcb2d) SHA1(acb57f3104a8dc4ba750dd1bf22ccc81cce9f084) )	//!< 00000-01777 NEXT(2)',NEXT(3)',NEXT(4)',NEXT(5)'
	ROM_LOAD( "62x.3",     7*02000, 0x400, CRC(1b20a63f) SHA1(41dc86438e91c12b0fe42ffcce6b2ac2eb9e714a) )	//!< 00000-01777 NEXT(6)',NEXT(7)',NEXT(8)',NEXT(9)'

	// extended memory Mesa 5.1 micro code PROMs, 8 x 4bit
	ROM_LOAD( "xm51.u54",  8*02000, 02000, CRC(11086ae9) SHA1(c394e3fadbfb91801ddc1a70cb25dc6f606c4f76) )	//!< 00000-01777 RSEL(0)',RSEL(1)',RSEL(2)',RSEL(3)'
	ROM_LOAD( "xm51.u74",  9*02000, 02000, CRC(be8224f2) SHA1(ea9abcc3832b26a094319796901237e1e3f238b6) )	//!< 00000-01777 RSEL(4)',ALUF(0)',ALUF(1)',ALUF(2)'
	ROM_LOAD( "xm51.u75", 10*02000, 02000, CRC(dfe3e3ac) SHA1(246fd29f92150a5d5d7627fbb4f2504c7b6cd5ec) )	//!< 00000-01777 ALUF(3)',BS(0)',BS(1)',BS(2)'
	ROM_LOAD( "xm51.u73", 11*02000, 02000, CRC(6c20fa46) SHA1(a054330c65048011f12209aaed5c6da73d95f029) )	//!< 00000-01777 F1(0),F1(1)',F1(2)',F1(3)'
	ROM_LOAD( "xm51.u52", 12*02000, 02000, CRC(0a31eec8) SHA1(4e2ad5daa5e6a6f2143ee4de00c7b625d096fb02) )	//!< 00000-01777 F2(0),F2(1)',F2(2)',F2(3)'
	ROM_LOAD( "xm51.u70", 13*02000, 02000, CRC(5c64ee54) SHA1(0eb16d1b5e5967be7c1bf8c8ef6efdf0518a752c) )	//!< 00000-01777 LOADT',LOADL,NEXT(0)',NEXT(1)'
	ROM_LOAD( "xm51.u71", 14*02000, 02000, CRC(7283bf71) SHA1(819fdcc407ed0acdd8f12b02db6efbcab7bec19a) )	//!< 00000-01777 NEXT(2)',NEXT(3)',NEXT(4)',NEXT(5)'
	ROM_LOAD( "xm51.u72", 15*02000, 02000, CRC(a28e5251) SHA1(44dd8ad4ad56541b5394d30ce3521b4d1d561394) )	//!< 00000-01777 NEXT(6)',NEXT(7)',NEXT(8)',NEXT(9)'

	// constant PROMs, 4 x 4bit
	// UINT16 src = BITS(addr, 3,2,1,4,5,6,7,0);
	ROM_REGION( 4 * 0400, "const", 0 )
	ROM_LOAD( "madr.a6",   0*00400, 00400, CRC(c2c196b2) SHA1(8b2a599ac839ec2a070dbfef2f1626e645c858ca) )	//!< 0000-0377 C(00)',C(01)',C(02)',C(03)'
	ROM_LOAD( "madr.a5",   1*00400, 00400, CRC(42336101) SHA1(c77819cf40f063af3abf66ea43f17cc1a62e928b) )	//!< 0000-0377 C(04)',C(05)',C(06)',C(07)'
	ROM_LOAD( "madr.a4",   2*00400, 00400, CRC(b957e490) SHA1(c72660ad3ada4ca0ed8697c6bb6275a4fe703184) )	//!< 0000-0377 C(08)',C(09)',C(10)',C(11)'
	ROM_LOAD( "madr.a3",   3*00400, 00400, CRC(e0992757) SHA1(5c45ea824970663cb9ee672dc50861539c860249) )	//!< 0000-0377 C(12)',C(13)',C(14)',C(15)'

	// extended memory Mesa 4.1 (?) micro code PROMs, 8 x 4bit (unused)
	ROM_REGION32_BE( 8 * 02000, "xm_mesa_4.1", ROMREGION_INVERT )
	ROM_LOAD( "xm654.41",  0*02000, 02000, CRC(beace302) SHA1(0002fea03a0261f57365095c4b87385d833f7063) )	//!< 00000-01777 RSEL(0)',RSEL(1)',RSEL(2)',RSEL(3)'
	ROM_LOAD( "xm674.41",  1*02000, 02000, CRC(7db5c097) SHA1(364bc41951baa3ad274031bd49abec1cf5b7a980) )	//!< 00000-01777 RSEL(4)',ALUF(0)',ALUF(1)',ALUF(2)'
	ROM_LOAD( "xm675.41",  2*02000, 02000, CRC(26eac1e7) SHA1(9220a1386afae8de96bdb2cf084afbadeeb61d42) )	//!< 00000-01777 ALUF(3)',BS(0)',BS(1)',BS(2)'
	ROM_LOAD( "xm673.41",  3*02000, 02000, CRC(8173d7e3) SHA1(7fbacf6dccb60dfe9cef88a248c3a1660efddcf4) )	//!< 00000-01777 F1(0),F1(1)',F1(2)',F1(3)'
	ROM_LOAD( "xm652.41",  4*02000, 02000, CRC(ddfa94bb) SHA1(38625e269400aaf38cd07b5dbf36c0087a0f1b92) )	//!< 00000-01777 F2(0),F2(1)',F2(2)',F2(3)'
	ROM_LOAD( "xm670.41",  5*02000, 02000, CRC(1cd187f3) SHA1(0fd5eff7c6b5c2383aa20148a795b80286554675) )	//!< 00000-01777 LOADT',LOADL,NEXT(0)',NEXT(1)'
	ROM_LOAD( "xm671.41",  6*02000, 02000, CRC(f21b1ad7) SHA1(1e18bdb35de7802892ac373c128f900786d40886) )	//!< 00000-01777 NEXT(2)',NEXT(3)',NEXT(4)',NEXT(5)'
	ROM_LOAD( "xm672.41",  7*02000, 02000, CRC(110ee075) SHA1(bb72fceba5ce9e5e8c8a0024915006bdd011a3f3) )	//!< 00000-01777 NEXT(6)',NEXT(7)',NEXT(8)',NEXT(9)'
ROM_END

/**
 * @brief list of microcoode PROM loading options
 */
static const prom_load_t ucode_prom_list[] = {
	{	// 0000-01777 RSEL(0)',RSEL(1)',RSEL(2)',RSEL(3)'
		"55x.3",
		0,
		"de870d75",
		"2b98cc769d8302cb39948711424d987d94e4159b",
/* size */	ALTO2_UCODE_PAGE_SIZE,
/* amap */	AMAP_DEFAULT,
/* axor */	ALTO2_UCODE_PAGE_MASK,
/* dxor */	017,						// invert D0-D3
/* width */	4,
/* shift */	28,
/* dmap */	DMAP_DEFAULT,
/* dand */	ZERO,
/* type */	sizeof(UINT32)
	},
	{	// 0000-01777 RSEL(4)',ALUF(0)',ALUF(1)',ALUF(2)'
		"64x.3",
		0,
		"51b444c0",
		"8756e51f7f3253a55d75886465beb7ee1be6e1c4",
/* size */	ALTO2_UCODE_PAGE_SIZE,
/* amap */	AMAP_DEFAULT,
/* axor */	ALTO2_UCODE_PAGE_MASK,
/* dxor */	017,						// invert D0-D3
/* width */	4,
/* shift */	24,
/* dmap */	DMAP_DEFAULT,
/* dand */	KEEP,
/* type */	sizeof(UINT32)
	},
	{	// 0000-01777 ALUF(3)',BS(0)',BS(1)',BS(2)'
		"65x.3",
		0,
		"741d1437",
		"01f7cf07c2173ac93799b2475180bfbbe7e0149b",
/* size */	ALTO2_UCODE_PAGE_SIZE,
/* amap */	AMAP_DEFAULT,
/* axor */	ALTO2_UCODE_PAGE_MASK,
/* dxor */	017,						// invert D0-D3
/* width */	4,
/* shift */	20,
/* dmap */	DMAP_DEFAULT,
/* dand */	KEEP,
/* type */	sizeof(UINT32)
	},
	{	// 0000-01777 F1(0),F1(1)',F1(2)',F1(3)'
		"63x.3",
		0,
		"f22d5028",
		"c65a42baef702d4aff2d9ad8e363daec27de6801",
/* size */	ALTO2_UCODE_PAGE_SIZE,
/* amap */	AMAP_DEFAULT,
/* axor */	ALTO2_UCODE_PAGE_MASK,
/* dxor */	007,						// keep D0, invert D1-D3
/* width */	4,
/* shift */	16,
/* dmap */	DMAP_DEFAULT,
/* dand */	KEEP,
/* type */	sizeof(UINT32)
	},
	{	// 0000-01777 F2(0),F2(1)',F2(2)',F2(3)'
		"53x.3",
		0,
		"3c89a740",
		"95d812d489b2bde03884b2f126f961caa6c8ec45",
/* size */	ALTO2_UCODE_PAGE_SIZE,
/* amap */	AMAP_DEFAULT,
/* axor */	ALTO2_UCODE_PAGE_MASK,
/* dxor */	007,						// keep D0, invert D1-D3
/* width */	4,
/* shift */	12,
/* dmap */	DMAP_DEFAULT,
/* dand */	KEEP,
/* type */	sizeof(UINT32)
	},
	{	// 0000-01777 LOADT',LOADL,NEXT(0)',NEXT(1)'
		"60x.3",
		0,
		"a35de0bf",
		"7fa4aead44dcf5393bbfd1706c0ada24aa6fd3ac",
/* size */	ALTO2_UCODE_PAGE_SIZE,
/* amap */	AMAP_DEFAULT,
/* axor */	ALTO2_UCODE_PAGE_MASK,
/* dxor */	013,						// invert D0 and D2-D3
/* width */	4,
/* shift */	8,
/* dmap */	DMAP_DEFAULT,
/* dand */	KEEP,
/* type */	sizeof(UINT32)
	},
	{	// 0000-01777 NEXT(2)',NEXT(3)',NEXT(4)',NEXT(5)'
		"61x.3",
		0,
		"f25bcb2d",
		"acb57f3104a8dc4ba750dd1bf22ccc81cce9f084",
/* size */	ALTO2_UCODE_PAGE_SIZE,
/* amap */	AMAP_DEFAULT,
/* axor */	ALTO2_UCODE_PAGE_MASK,
/* dxor */	017,						// invert D0-D3
/* width */	4,
/* shift */	4,
/* dmap */	DMAP_DEFAULT,
/* dand */	KEEP,
/* type */	sizeof(UINT32)
	},
	{	// 0000-01777 NEXT(6)',NEXT(7)',NEXT(8)',NEXT(9)'
		"62x.3",
		0,
		"1b20a63f",
		"41dc86438e91c12b0fe42ffcce6b2ac2eb9e714a",
/* size */	ALTO2_UCODE_PAGE_SIZE,
/* amap */	AMAP_DEFAULT,
/* axor */	ALTO2_UCODE_PAGE_MASK,
/* dxor */	017,						// invert D0-D3
/* width */	4,
/* shift */	0,
/* dmap */	DMAP_DEFAULT,
/* dand */	KEEP,
/* type */	sizeof(UINT32)
	}

#if	(ALTO2_UCODE_ROM_PAGES > 1)
	,
	{	// 02000-03777 RSEL(0)',RSEL(1)',RSEL(2)',RSEL(3)'
		"xm51.u54",
		0,
		"11086ae9",
		"c394e3fadbfb91801ddc1a70cb25dc6f606c4f76",
/* size */	ALTO2_UCODE_PAGE_SIZE,
/* amap */	AMAP_DEFAULT,
/* axor */	ALTO2_UCODE_PAGE_MASK,
/* dxor */	017,						// invert D0-D3
/* width */	4,
/* shift */	28,
/* dmap */	DMAP_DEFAULT,
/* dand */	ZERO,
/* type */	sizeof(UINT32)
	},
	{	// 02000-03777 RSEL(4)',ALUF(0)',ALUF(1)',ALUF(2)'
		"xm51.u74",
		0,
		"be8224f2",
		"ea9abcc3832b26a094319796901237e1e3f238b6",
/* size */	ALTO2_UCODE_PAGE_SIZE,
/* amap */	AMAP_DEFAULT,
/* axor */	ALTO2_UCODE_PAGE_MASK,
/* dxor */	017,						// invert D0-D3
/* width */	4,
/* shift */	24,
/* dmap */	DMAP_DEFAULT,
/* dand */	KEEP,
/* type */	sizeof(UINT32)
	},
	{	// 02000-03777 ALUF(3)',BS(0)',BS(1)',BS(2)'
		"xm51.u75",
		0,
		"dfe3e3ac",
		"246fd29f92150a5d5d7627fbb4f2504c7b6cd5ec",
/* size */	ALTO2_UCODE_PAGE_SIZE,
/* amap */	AMAP_DEFAULT,
/* axor */	ALTO2_UCODE_PAGE_MASK,
/* dxor */	017,						// invert D0-D3
/* width */	4,
/* shift */	20,
/* dmap */	DMAP_DEFAULT,
/* dand */	KEEP,
/* type */	sizeof(UINT32)
	},
	{	// 02000-03777 F1(0),F1(1)',F1(2)',F1(3)'
		"xm51.u73",
		0,
		"6c20fa46",
		"a054330c65048011f12209aaed5c6da73d95f029",
/* size */	ALTO2_UCODE_PAGE_SIZE,
/* amap */	AMAP_DEFAULT,
/* axor */	ALTO2_UCODE_PAGE_MASK,
/* dxor */	007,						// keep D0, invert D1-D3
/* width */	4,
/* shift */	16,
/* dmap */	DMAP_DEFAULT,
/* dand */	KEEP,
/* type */	sizeof(UINT32)
	},
	{	// 02000-03777 F2(0),F2(1)',F2(2)',F2(3)'
		"xm51.u52",
		0,
		"0a31eec8",
		"4e2ad5daa5e6a6f2143ee4de00c7b625d096fb02",
/* size */	ALTO2_UCODE_PAGE_SIZE,
/* amap */	AMAP_DEFAULT,
/* axor */	ALTO2_UCODE_PAGE_MASK,
/* dxor */	007,						// keep D0, invert D1-D3
/* width */	4,
/* shift */	12,
/* dmap */	DMAP_DEFAULT,
/* dand */	KEEP,
/* type */	sizeof(UINT32)
	},
	{	// 02000-03777 LOADT',LOADL,NEXT(0)',NEXT(1)'
		"xm51.u70",
		0,
		"5c64ee54",
		"0eb16d1b5e5967be7c1bf8c8ef6efdf0518a752c",
/* size */	ALTO2_UCODE_PAGE_SIZE,
/* amap */	AMAP_DEFAULT,
/* axor */	ALTO2_UCODE_PAGE_MASK,
/* dxor */	013,						// invert D0 and D2-D3
/* width */	4,
/* shift */	8,
/* dmap */	DMAP_DEFAULT,
/* dand */	KEEP,
/* type */	sizeof(UINT32)
	},
	{	// 02000-03777 NEXT(2)',NEXT(3)',NEXT(4)',NEXT(5)'
		"xm51.u71",
		0,
		"7283bf71",
		"819fdcc407ed0acdd8f12b02db6efbcab7bec19a",
/* size */	ALTO2_UCODE_PAGE_SIZE,
/* amap */	AMAP_DEFAULT,
/* axor */	ALTO2_UCODE_PAGE_MASK,
/* dxor */	017,						// invert D0-D3
/* width */	4,
/* shift */	4,
/* dmap */	DMAP_DEFAULT,
/* dand */	KEEP,
/* type */	sizeof(UINT32)
	},
	{	// 02000-03777 NEXT(6)',NEXT(7)',NEXT(8)',NEXT(9)'
		"xm51.u72",
		0,
		"a28e5251",
		"44dd8ad4ad56541b5394d30ce3521b4d1d561394",
/* size */	ALTO2_UCODE_PAGE_SIZE,
/* amap */	AMAP_DEFAULT,
/* axor */	ALTO2_UCODE_PAGE_MASK,
/* dxor */	017,						// invert D0-D3
/* width */	4,
/* shift */	0,
/* dmap */	DMAP_DEFAULT,
/* dand */	KEEP,
/* type */	sizeof(UINT32)
	}
#endif	// (UCODE_ROM_PAGES > 1)
};

static const prom_load_t const_prom_list[] = {
	{	// constant prom D0-D3
		"madr.a6",
		"c3.3",
		"c2c196b2",
		"8b2a599ac839ec2a070dbfef2f1626e645c858ca",
/* size */	ALTO2_CONST_SIZE,
/* amap */	AMAP_CONST_PROM,			// descramble constant address
/* axor */	0,
/* dxor */	017,						// invert D0-D3
/* width */	4,
/* shift */	0,
/* dmap */	DMAP_REVERSE_0_3,			// reverse D0-D3 to D3-D0
/* dand */	ZERO,
/* type */	sizeof(UINT16)
	},
	{	// constant prom D4-D7
		"madr.a5",
		"c2.3",
		"42336101",
		"c77819cf40f063af3abf66ea43f17cc1a62e928b",
/* size */	ALTO2_CONST_SIZE,
/* amap */	AMAP_CONST_PROM,			// descramble constant address
/* axor */	0,
/* dxor */	017,						// invert D0-D3
/* width */	4,
/* shift */	4,
/* dmap */	DMAP_REVERSE_0_3,			// reverse D0-D3 to D3-D0
/* dand */	KEEP,
/* type */	sizeof(UINT16)
	},
	{	// constant prom D8-D11
		"madr.a4",
		"c1.3",
		"b957e490",
		"c72660ad3ada4ca0ed8697c6bb6275a4fe703184",
/* size */	ALTO2_CONST_SIZE,
/* amap */	AMAP_CONST_PROM,			// descramble constant address
/* axor */	0,
/* dxor */	017,						// invert D0-D3
/* width */	4,
/* shift */	8,
/* dmap */	DMAP_REVERSE_0_3,			// reverse D0-D3 to D3-D0
/* dand */	KEEP,
/* type */	sizeof(UINT16)
	},
	{	// constant PROM D12-D15
		"madr.a3",
		"c0.3",
		"e0992757",
		"5c45ea824970663cb9ee672dc50861539c860249",
/* size */	ALTO2_CONST_SIZE,
/* amap */	AMAP_CONST_PROM,			// descramble constant address
/* axor */	0,
/* dxor */	017,						// invert D0-D3
/* width */	4,
/* shift */	12,
/* dmap */	DMAP_REVERSE_0_3,			// reverse D0-D3 to D3-D0
/* dand */	KEEP,
/* type */	sizeof(UINT16)
	}
};

/* Driver Init */

DRIVER_INIT_MEMBER( alto2_state, alto2 )
{
	UINT32* maincpu = reinterpret_cast<UINT32 *>(memregion("maincpu")->base());
	for (UINT32 addr = 0; addr < ALTO2_UCODE_SIZE; addr++)
		maincpu[addr] = ALTO2_UCODE_INVERTED;

	UINT8* ucode_prom = prom_load(this, ucode_prom_list, memregion("ucode")->base(), 2, 8);
	memcpy(memregion("maincpu")->base(), ucode_prom, sizeof(UINT32)*ALTO2_UCODE_RAM_BASE);

	UINT8* const_prom = prom_load(this, const_prom_list, memregion("const")->base(), 1, 4);
	memcpy(memregion("constants")->base(), const_prom, sizeof(UINT16)*ALTO2_CONST_SIZE);
}

/* Game Drivers */

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT   COMPANY  FULLNAME   FLAGS
COMP( 1974, alto2,  0,      0,      alto2,   alto2, alto2_state, alto2, "Xerox", "Alto-II", GAME_NOT_WORKING | GAME_NO_SOUND )
