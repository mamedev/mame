/***************************************************************************
 *   Xerox AltoII driver for MESS
 *
 *   Copyright © Jürgen Buchmüller <pullmoll@t-online.de>
 *
 *   Licenses: MAME, GPLv2
 ***************************************************************************/

#include "includes/alto2.h"

/* Input Ports */

#define	PORT_KEY(_bit,_code,_char1,_char2,_name) \
	PORT_BIT(_bit, IP_ACTIVE_LOW, IPT_KEYBOARD) \
	PORT_CODE(_code) PORT_NAME(_name) \
	PORT_CHAR(_char1) PORT_CHAR(_char2) \

#define SPACING "     "

static INPUT_PORTS_START( alto2 )
	PORT_START("ROW0")
	PORT_KEY(A2_KEY_5,          KEYCODE_5,          '5',    '%',  "5" SPACING "%")  //!< normal: 5    shifted: %
	PORT_KEY(A2_KEY_4,          KEYCODE_4,          '4',    '$',  "4" SPACING "$")  //!< normal: 4    shifted: $
	PORT_KEY(A2_KEY_6,          KEYCODE_6,          '6',    '~',  "6" SPACING "~")  //!< normal: 6    shifted: ~
	PORT_KEY(A2_KEY_E,          KEYCODE_E,          'e',    'E',  "e" SPACING "E")  //!< normal: e    shifted: E
	PORT_KEY(A2_KEY_7,          KEYCODE_7,          '7',    '&',  "7" SPACING "&")  //!< normal: 7    shifted: &
	PORT_KEY(A2_KEY_D,          KEYCODE_D,          'd',    'D',  "d" SPACING "D")  //!< normal: d    shifted: D
	PORT_KEY(A2_KEY_U,          KEYCODE_U,          'u',    'U',  "u" SPACING "U")  //!< normal: u    shifted: U
	PORT_KEY(A2_KEY_V,          KEYCODE_V,          'v',    'V',  "v" SPACING "V")  //!< normal: v    shifted: V
	PORT_KEY(A2_KEY_0,          KEYCODE_0,          '0',    ')',  "0" SPACING ")")  //!< normal: 0    shifted: )
	PORT_KEY(A2_KEY_K,          KEYCODE_K,          'k',    'K',  "k" SPACING "K")  //!< normal: k    shifted: K
	PORT_KEY(A2_KEY_MINUS,      KEYCODE_MINUS,      '-',    '_',  "-" SPACING "_")  //!< normal: -    shifted: _
	PORT_KEY(A2_KEY_P,          KEYCODE_P,          'p',    'P',  "p" SPACING "P")  //!< normal: p    shifted: P
	PORT_KEY(A2_KEY_SLASH,      KEYCODE_SLASH,      '/',    '?',  "/" SPACING "?")  //!< normal: /    shifted: ?
	PORT_KEY(A2_KEY_BACKSLASH,  KEYCODE_BACKSLASH,  '\\',   '|', "\\" SPACING "|")  //!< normal: \    shifted: |
	PORT_KEY(A2_KEY_LF,         KEYCODE_DOWN,       '\012', 0,    "LF"           )  //!< normal: LF   shifted: ?
	PORT_KEY(A2_KEY_BS,         KEYCODE_BACKSPACE,  '\010', 0,    "BS"           )  //!< normal: BS   shifted: ?

	PORT_START("ROW1")
	PORT_KEY(A2_KEY_3,          KEYCODE_3,          '3',    '#',  "3" SPACING "#")  //!< normal: 3    shifted: #
	PORT_KEY(A2_KEY_2,          KEYCODE_2,          '2',    '@',  "2" SPACING "@")  //!< normal: 2    shifted: @
	PORT_KEY(A2_KEY_W,          KEYCODE_W,          'w',    'W',  "w" SPACING "W")  //!< normal: w    shifted: W
	PORT_KEY(A2_KEY_Q,          KEYCODE_Q,          'q',    'Q',  "q" SPACING "Q")  //!< normal: q    shifted: Q
	PORT_KEY(A2_KEY_S,          KEYCODE_S,          's',    'S',  "s" SPACING "S")  //!< normal: s    shifted: S
	PORT_KEY(A2_KEY_A,          KEYCODE_A,          'a',    'A',  "a" SPACING "A")  //!< normal: a    shifted: A
	PORT_KEY(A2_KEY_9,          KEYCODE_9,          '9',    '(',  "9" SPACING "(")  //!< normal: 9    shifted: (
	PORT_KEY(A2_KEY_I,          KEYCODE_I,          'i',    'I',  "i" SPACING "I")  //!< normal: i    shifted: I
	PORT_KEY(A2_KEY_X,          KEYCODE_X,          'x',    'X',  "x" SPACING "X")  //!< normal: x    shifted: X
	PORT_KEY(A2_KEY_O,          KEYCODE_O,          'o',    'O',  "o" SPACING "O")  //!< normal: o    shifted: O
	PORT_KEY(A2_KEY_L,          KEYCODE_L,          'l',    'L',  "l" SPACING "L")  //!< normal: l    shifted: L
	PORT_KEY(A2_KEY_COMMA,      KEYCODE_COMMA,      ',',    '<',  "," SPACING "<")  //!< normal: ,    shifted: <
	PORT_KEY(A2_KEY_QUOTE,		KEYCODE_QUOTE,      '\x27', '"',  "'" SPACING "\"") //!< normal: '    shifted: "
	PORT_KEY(A2_KEY_RBRACKET,   KEYCODE_CLOSEBRACE, ']',    '}',  "]" SPACING "}")  //!< normal: ]    shifted: }
	PORT_KEY(A2_KEY_BLANK_MID,  KEYCODE_END,        0,      0,    "MID"          )  //!< middle blank key
	PORT_KEY(A2_KEY_BLANK_TOP,  KEYCODE_PGUP,       0,      0,    "TOP"          )  //!< top blank key

	PORT_START("ROW2")
	PORT_KEY(A2_KEY_1,          KEYCODE_1,          '1',    '!',  "1" SPACING "!")  //!< normal: 1    shifted: !
	PORT_KEY(A2_KEY_ESCAPE,     KEYCODE_ESC,        '\x1b', 0,    "ESC"          )  //!< normal: ESC  shifted: ?
	PORT_KEY(A2_KEY_TAB,        KEYCODE_TAB,        '\011', 0,    "TAB"          )  //!< normal: TAB  shifted: ?
	PORT_KEY(A2_KEY_F,          KEYCODE_F,          'f',    'F',  "f" SPACING "F")  //!< normal: f    shifted: F
	PORT_KEY(A2_KEY_CTRL,       KEYCODE_LCONTROL,   0,      0,    "CTRL"         )  //!< CTRL
	PORT_KEY(A2_KEY_C,          KEYCODE_C,          'c',    'C',  "c" SPACING "C")  //!< normal: c    shifted: C
	PORT_KEY(A2_KEY_J,          KEYCODE_J,          'j',    'J',  "j" SPACING "J")  //!< normal: j    shifted: J
	PORT_KEY(A2_KEY_B,          KEYCODE_B,          'b',    'B',  "b" SPACING "B")  //!< normal: b    shifted: B
	PORT_KEY(A2_KEY_Z,          KEYCODE_Z,          'z',    'Z',  "z" SPACING "Z")  //!< normal: z    shifted: Z
	PORT_KEY(A2_KEY_LSHIFT,     KEYCODE_LSHIFT,     0,      0,    "LSHIFT"       )  //!< LSHIFT
	PORT_KEY(A2_KEY_PERIOD,     KEYCODE_STOP,       '.',    '>',  "." SPACING ">")  //!< normal: .    shifted: >
	PORT_KEY(A2_KEY_SEMICOLON,  KEYCODE_COLON,      ';',    ':',  ";" SPACING ":")  //!< normal: ;    shifted: :
	PORT_KEY(A2_KEY_RETURN,     KEYCODE_ENTER,      '\013', 0,    "RETURN"       )  //!< RETURN
	PORT_KEY(A2_KEY_LEFTARROW,  KEYCODE_LEFT,       0,      0,    "←" SPACING "↑")  //!< normal: left arrow   shifted: up arrow (caret)
	PORT_KEY(A2_KEY_DEL,        KEYCODE_DEL,        0,      0,    "DEL"          )  //!< normal: DEL  shifted: ?
	PORT_KEY(A2_KEY_MSW_2_17,   KEYCODE_MENU,       0,      0,    "MSW2/17"      )  //!< unused on Microswitch KDB

	PORT_START("ROW3")
	PORT_KEY(A2_KEY_R,          KEYCODE_R,          'r',    'R',  "r" SPACING "R")  //!< normal: r    shifted: R
	PORT_KEY(A2_KEY_T,          KEYCODE_T,          't',    'T',  "t" SPACING "T")  //!< normal: t    shifted: T
	PORT_KEY(A2_KEY_G,          KEYCODE_G,          'g',    'G',  "g" SPACING "G")  //!< normal: g    shifted: G
	PORT_KEY(A2_KEY_Y,          KEYCODE_Y,          'y',    'Y',  "y" SPACING "Y")  //!< normal: y    shifted: Y
	PORT_KEY(A2_KEY_H,          KEYCODE_H,          'h',    'H',  "h" SPACING "H")  //!< normal: h    shifted: H
	PORT_KEY(A2_KEY_8,          KEYCODE_8,          '8',    '*',  "8" SPACING "*")  //!< normal: 8    shifted: *
	PORT_KEY(A2_KEY_N,          KEYCODE_N,          'n',    'N',  "n" SPACING "N")  //!< normal: n    shifted: N
	PORT_KEY(A2_KEY_M,          KEYCODE_M,          'm',    'M',  "m" SPACING "M")  //!< normal: m    shifted: M
	PORT_KEY(A2_KEY_LOCK,       KEYCODE_SCRLOCK,    0,      0,    "LOCK"         )  //!< LOCK
	PORT_KEY(A2_KEY_SPACE,      KEYCODE_SPACE,      ' ',    0,    "SPACE"        )  //!< SPACE
	PORT_KEY(A2_KEY_LBRACKET,   KEYCODE_OPENBRACE,  '[',    '{',  "[" SPACING "{")  //!< normal: [    shifted: {
	PORT_KEY(A2_KEY_EQUALS,     KEYCODE_EQUALS,     '=',    '+',  "=" SPACING "+")  //!< normal: =    shifted: +
	PORT_KEY(A2_KEY_RSHIFT,     KEYCODE_RSHIFT,     0,      0,    "RSHIFT"       )  //!< RSHIFT
	PORT_KEY(A2_KEY_BLANK_BOT,  KEYCODE_PGDN,       0,      0,    "BOT"          )  //!< bottom blank key
	PORT_KEY(A2_KEY_MSW_3_16,   KEYCODE_HOME,       0,      0,    "MSW3/16"      )  //!< unused on Microswitch KDB
	PORT_KEY(A2_KEY_MSW_3_17,   KEYCODE_INSERT,     0,      0,    "MSW3/17"      )  //!< unused on Microswitch KDB

	PORT_START("ROW4")
	PORT_KEY(A2_KEY_FR2,        KEYCODE_F6,         0,      0,    "FR2"          )  //!< ADL right function key 2
	PORT_KEY(A2_KEY_FL2,        KEYCODE_F2,         0,      0,    "FL2"          )  //!< ADL left function key 2

	PORT_START("ROW5")
	PORT_KEY(A2_KEY_FR4,        KEYCODE_F8,         0,      0,    "FR4"          )  //!< ADL right funtion key 4
	PORT_KEY(A2_KEY_BW,         KEYCODE_F10,        0,      0,    "BW"           )  //!< ADL BW (?)

	PORT_START("ROW6")
	PORT_KEY(A2_KEY_FR3,        KEYCODE_F7,         0,      0,    "FR3"          )  //!< ADL right function key 3
	PORT_KEY(A2_KEY_FL1,        KEYCODE_F1,         0,      0,    "FL1"          )  //!< ADL left function key 1
	PORT_KEY(A2_KEY_FL3,        KEYCODE_F3,         0,      0,    "FL3"          )  //!< ADL left function key 3

	PORT_START("ROW7")
	PORT_KEY(A2_KEY_FR1,        KEYCODE_F5,         0,      0,    "FR1"          )  //!< ADL right function key 1
	PORT_KEY(A2_KEY_FL4,        KEYCODE_F4,         0,      0,    "FL4"          )  //!< ADL left function key 4
	PORT_KEY(A2_KEY_FR5,        KEYCODE_F9,         0,      0,    "FR5"          )  //!< ADL right function key 5

	PORT_START("mouseb")	// Mouse buttons
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Mouse RED (left)")      PORT_CODE(MOUSECODE_BUTTON1) PORT_CHANGED_MEMBER( ":maincpu", alto2_cpu_device, mouse_buttons, 0 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Mouse BLUE (right)")    PORT_CODE(MOUSECODE_BUTTON2) PORT_CHANGED_MEMBER( ":maincpu", alto2_cpu_device, mouse_buttons, 0 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("Mouse YELLOW (middel)") PORT_CODE(MOUSECODE_BUTTON3) PORT_CHANGED_MEMBER( ":maincpu", alto2_cpu_device, mouse_buttons, 0 )

	PORT_START("mousex")	// Mouse - X AXIS
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_X) PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_CHANGED_MEMBER( ":maincpu", alto2_cpu_device, mouse_motion_x, 0 )

	PORT_START("mousey")	// Mouse - Y AXIS
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_Y) PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_CHANGED_MEMBER( ":maincpu", alto2_cpu_device, mouse_motion_y, 0 )

	PORT_START("CONFIG")    /* Memory switch on AIM board */
	PORT_CONFNAME( 0x01, 0x01, "Memory switch")
	PORT_CONFSETTING( 0x00, "on")
	PORT_CONFSETTING( 0x01, "off")
INPUT_PORTS_END

/* ROM */
ROM_START( alto2 )
	// dummy region for the maincpu - this is not used in any way
	ROM_REGION( 0400, "maincpu", 0 )
	ROM_FILL(0, 0400, ALTO2_UCODE_INVERTED)
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
	AM_RANGE(0, 2*ALTO2_RAM_SIZE-1) AM_DEVICE16( "maincpu", alto2_cpu_device, iomem_map, 0xffffU )
ADDRESS_MAP_END


/* Palette Initialization */

void alto2_state::palette_init()
{
	palette_set_color(machine(),0,RGB_WHITE); /* white */
	palette_set_color(machine(),1,RGB_BLACK); /* black */
}

static MACHINE_CONFIG_START( alto2, alto2_state )
	/* basic machine hardware */
	// SYSCLK is Display Control part A51 (tagged 29.4MHz) divided by 5(?)
	// 5.8MHz according to de.wikipedia.org/wiki/Xerox_Alto
	MCFG_CPU_ADD("maincpu", ALTO2, XTAL_29_4912MHz/5)
	MCFG_CPU_PROGRAM_MAP(alto2_ucode_map)
	MCFG_CPU_DATA_MAP(alto2_const_map)
	MCFG_CPU_IO_MAP(alto2_iomem_map)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(XTAL_20_16MHz,
						   ALTO2_DISPLAY_TOTAL_WIDTH,   0, ALTO2_DISPLAY_WIDTH,
						   ALTO2_DISPLAY_TOTAL_HEIGHT,  0, ALTO2_DISPLAY_HEIGHT)
	MCFG_SCREEN_REFRESH_RATE(60)	// two interlaced fields
	MCFG_SCREEN_VBLANK_TIME(ALTO2_DISPLAY_VBLANK_TIME)
	MCFG_SCREEN_UPDATE_DEVICE("maincpu", alto2_cpu_device, screen_update)
	MCFG_SCREEN_VBLANK_DEVICE("maincpu", alto2_cpu_device, screen_eof)

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
}

/* Game Drivers */

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT   COMPANY  FULLNAME   FLAGS
COMP( 1977, alto2,  0,      0,      alto2,   alto2, alto2_state, alto2, "Xerox", "Alto-II", GAME_NO_SOUND )
