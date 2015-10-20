// license:GPL-2.0+
// copyright-holders:Juergen Buchmueller
/***************************************************************************
 *   Xerox AltoII driver for MESS
 *
 ***************************************************************************/

#include "emu.h"
#include "rendlay.h"
#include "cpu/alto2/alto2cpu.h"
#include "machine/diablo_hd.h"

class alto2_state : public driver_device
{
public:
	alto2_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_io_row0(*this, "ROW0"),
		m_io_row1(*this, "ROW1"),
		m_io_row2(*this, "ROW2"),
		m_io_row3(*this, "ROW3"),
		m_io_row4(*this, "ROW4"),
		m_io_row5(*this, "ROW5"),
		m_io_row6(*this, "ROW6"),
		m_io_row7(*this, "ROW7"),
		m_io_config(*this, "CONFIG")
	{ }

	DECLARE_DRIVER_INIT(alto2);
	DECLARE_MACHINE_RESET(alto2);

protected:
	required_device<cpu_device> m_maincpu;
	required_ioport m_io_row0;
	required_ioport m_io_row1;
	required_ioport m_io_row2;
	required_ioport m_io_row3;
	required_ioport m_io_row4;
	required_ioport m_io_row5;
	required_ioport m_io_row6;
	required_ioport m_io_row7;
	optional_ioport m_io_config;
};

/* Input Ports */

#define PORT_KEY(_bit,_code,_char1,_char2,_name) \
	PORT_BIT(_bit, IP_ACTIVE_LOW, IPT_KEYBOARD) \
	PORT_CODE(_code) PORT_NAME(_name) \
	PORT_CHAR(_char1) PORT_CHAR(_char2)
#define SPACING "     "

static INPUT_PORTS_START( alto2 )
	PORT_START("ROW0")
	PORT_KEY(A2_KEY_5,          KEYCODE_5,          '5',            '%',          "5" SPACING "%")  //!< normal: 5    shifted: %
	PORT_KEY(A2_KEY_4,          KEYCODE_4,          '4',            '$',          "4" SPACING "$")  //!< normal: 4    shifted: $
	PORT_KEY(A2_KEY_6,          KEYCODE_6,          '6',            '~',          "6" SPACING "~")  //!< normal: 6    shifted: ~
	PORT_KEY(A2_KEY_E,          KEYCODE_E,          'e',            'E',          "e" SPACING "E")  //!< normal: e    shifted: E
	PORT_KEY(A2_KEY_7,          KEYCODE_7,          '7',            '&',          "7" SPACING "&")  //!< normal: 7    shifted: &
	PORT_KEY(A2_KEY_D,          KEYCODE_D,          'd',            'D',          "d" SPACING "D")  //!< normal: d    shifted: D
	PORT_KEY(A2_KEY_U,          KEYCODE_U,          'u',            'U',          "u" SPACING "U")  //!< normal: u    shifted: U
	PORT_KEY(A2_KEY_V,          KEYCODE_V,          'v',            'V',          "v" SPACING "V")  //!< normal: v    shifted: V
	PORT_KEY(A2_KEY_0,          KEYCODE_0,          '0',            ')',          "0" SPACING ")")  //!< normal: 0    shifted: )
	PORT_KEY(A2_KEY_K,          KEYCODE_K,          'k',            'K',          "k" SPACING "K")  //!< normal: k    shifted: K
	PORT_KEY(A2_KEY_MINUS,      KEYCODE_MINUS,      '-',            '_',          "-" SPACING "_")  //!< normal: -    shifted: _
	PORT_KEY(A2_KEY_P,          KEYCODE_P,          'p',            'P',          "p" SPACING "P")  //!< normal: p    shifted: P
	PORT_KEY(A2_KEY_SLASH,      KEYCODE_SLASH,      '/',            '?',          "/" SPACING "?")  //!< normal: /    shifted: ?
	PORT_KEY(A2_KEY_BACKSLASH,  KEYCODE_BACKSLASH,  '\\',           '|',         "\\" SPACING "|")  //!< normal: \    shifted: |
	PORT_KEY(A2_KEY_LF,         KEYCODE_DOWN,       10,             10,           "LF"           )  //!< normal: LF   shifted: ?
	PORT_KEY(A2_KEY_BS,         KEYCODE_BACKSPACE,  8,              8,            "BS"           )  //!< normal: BS   shifted: ?

	PORT_START("ROW1")
	PORT_KEY(A2_KEY_3,          KEYCODE_3,          '3',            '#',          "3" SPACING "#")  //!< normal: 3    shifted: #
	PORT_KEY(A2_KEY_2,          KEYCODE_2,          '2',            '@',          "2" SPACING "@")  //!< normal: 2    shifted: @
	PORT_KEY(A2_KEY_W,          KEYCODE_W,          'w',            'W',          "w" SPACING "W")  //!< normal: w    shifted: W
	PORT_KEY(A2_KEY_Q,          KEYCODE_Q,          'q',            'Q',          "q" SPACING "Q")  //!< normal: q    shifted: Q
	PORT_KEY(A2_KEY_S,          KEYCODE_S,          's',            'S',          "s" SPACING "S")  //!< normal: s    shifted: S
	PORT_KEY(A2_KEY_A,          KEYCODE_A,          'a',            'A',          "a" SPACING "A")  //!< normal: a    shifted: A
	PORT_KEY(A2_KEY_9,          KEYCODE_9,          '9',            '(',          "9" SPACING "(")  //!< normal: 9    shifted: (
	PORT_KEY(A2_KEY_I,          KEYCODE_I,          'i',            'I',          "i" SPACING "I")  //!< normal: i    shifted: I
	PORT_KEY(A2_KEY_X,          KEYCODE_X,          'x',            'X',          "x" SPACING "X")  //!< normal: x    shifted: X
	PORT_KEY(A2_KEY_O,          KEYCODE_O,          'o',            'O',          "o" SPACING "O")  //!< normal: o    shifted: O
	PORT_KEY(A2_KEY_L,          KEYCODE_L,          'l',            'L',          "l" SPACING "L")  //!< normal: l    shifted: L
	PORT_KEY(A2_KEY_COMMA,      KEYCODE_COMMA,      ',',            '<',          "," SPACING "<")  //!< normal: ,    shifted: <
	PORT_KEY(A2_KEY_QUOTE,      KEYCODE_QUOTE,      39,             34,           "'" SPACING "\"") //!< normal: '    shifted: "
	PORT_KEY(A2_KEY_RBRACKET,   KEYCODE_CLOSEBRACE, ']',            '}',          "]" SPACING "}")  //!< normal: ]    shifted: }
	PORT_KEY(A2_KEY_BLANK_MID,  KEYCODE_END,        0,              0,            "MID"          )  //!< middle blank key
	PORT_KEY(A2_KEY_BLANK_TOP,  KEYCODE_PGUP,       0,              0,            "TOP"          )  //!< top blank key

	PORT_START("ROW2")
	PORT_KEY(A2_KEY_1,          KEYCODE_1,          '1',            '!',          "1" SPACING "!")  //!< normal: 1    shifted: !
	PORT_KEY(A2_KEY_ESCAPE,     KEYCODE_ESC,        27,             0,            "ESC"          )  //!< normal: ESC  shifted: ?
	PORT_KEY(A2_KEY_TAB,        KEYCODE_TAB,        9,              0,            "TAB"          )  //!< normal: TAB  shifted: ?
	PORT_KEY(A2_KEY_F,          KEYCODE_F,          'f',            'F',          "f" SPACING "F")  //!< normal: f    shifted: F
	PORT_KEY(A2_KEY_CTRL,       KEYCODE_LCONTROL,   0,              0,            "CTRL"         )  //!< CTRL
	PORT_KEY(A2_KEY_C,          KEYCODE_C,          'c',            'C',          "c" SPACING "C")  //!< normal: c    shifted: C
	PORT_KEY(A2_KEY_J,          KEYCODE_J,          'j',            'J',          "j" SPACING "J")  //!< normal: j    shifted: J
	PORT_KEY(A2_KEY_B,          KEYCODE_B,          'b',            'B',          "b" SPACING "B")  //!< normal: b    shifted: B
	PORT_KEY(A2_KEY_Z,          KEYCODE_Z,          'z',            'Z',          "z" SPACING "Z")  //!< normal: z    shifted: Z
	PORT_KEY(A2_KEY_LSHIFT,     KEYCODE_LSHIFT,     UCHAR_SHIFT_1,  0,            "LSHIFT"       )  //!< LSHIFT
	PORT_KEY(A2_KEY_PERIOD,     KEYCODE_STOP,       '.',            '>',          "." SPACING ">")  //!< normal: .    shifted: >
	PORT_KEY(A2_KEY_SEMICOLON,  KEYCODE_COLON,      ';',            ':',          ";" SPACING ":")  //!< normal: ;    shifted: :
	PORT_KEY(A2_KEY_RETURN,     KEYCODE_ENTER,      13,             0,            "RETURN"       )  //!< RETURN
	PORT_KEY(A2_KEY_LEFTARROW,  KEYCODE_LEFT,       0,              0,            "???" SPACING "???")  //!< normal: left arrow   shifted: up arrow (caret)
	PORT_KEY(A2_KEY_DEL,        KEYCODE_DEL,        UCHAR_MAMEKEY(DEL), 0,        "DEL"          )  //!< normal: DEL  shifted: ?
	PORT_KEY(A2_KEY_MSW_2_17,   KEYCODE_MENU,       0,              0,            "MSW2/17"      )  //!< unused on Microswitch KDB

	PORT_START("ROW3")
	PORT_KEY(A2_KEY_R,          KEYCODE_R,          'r',            'R',          "r" SPACING "R")  //!< normal: r    shifted: R
	PORT_KEY(A2_KEY_T,          KEYCODE_T,          't',            'T',          "t" SPACING "T")  //!< normal: t    shifted: T
	PORT_KEY(A2_KEY_G,          KEYCODE_G,          'g',            'G',          "g" SPACING "G")  //!< normal: g    shifted: G
	PORT_KEY(A2_KEY_Y,          KEYCODE_Y,          'y',            'Y',          "y" SPACING "Y")  //!< normal: y    shifted: Y
	PORT_KEY(A2_KEY_H,          KEYCODE_H,          'h',            'H',          "h" SPACING "H")  //!< normal: h    shifted: H
	PORT_KEY(A2_KEY_8,          KEYCODE_8,          '8',            '*',          "8" SPACING "*")  //!< normal: 8    shifted: *
	PORT_KEY(A2_KEY_N,          KEYCODE_N,          'n',            'N',          "n" SPACING "N")  //!< normal: n    shifted: N
	PORT_KEY(A2_KEY_M,          KEYCODE_M,          'm',            'M',          "m" SPACING "M")  //!< normal: m    shifted: M
	PORT_KEY(A2_KEY_LOCK,       KEYCODE_SCRLOCK,    0,              0,            "LOCK"         )  //!< LOCK
	PORT_KEY(A2_KEY_SPACE,      KEYCODE_SPACE,      32,             0,            "SPACE"        )  //!< SPACE
	PORT_KEY(A2_KEY_LBRACKET,   KEYCODE_OPENBRACE,  '[',            '{',          "[" SPACING "{")  //!< normal: [    shifted: {
	PORT_KEY(A2_KEY_EQUALS,     KEYCODE_EQUALS,     '=',            '+',          "=" SPACING "+")  //!< normal: =    shifted: +
	PORT_KEY(A2_KEY_RSHIFT,     KEYCODE_RSHIFT,     UCHAR_SHIFT_2,  0,            "RSHIFT"       )  //!< RSHIFT
	PORT_KEY(A2_KEY_BLANK_BOT,  KEYCODE_PGDN,       0,              0,            "BOT"          )  //!< bottom blank key
	PORT_KEY(A2_KEY_MSW_3_16,   KEYCODE_HOME,       0,              0,            "MSW3/16"      )  //!< unused on Microswitch KDB
	PORT_KEY(A2_KEY_MSW_3_17,   KEYCODE_INSERT,     0,              0,            "MSW3/17"      )  //!< unused on Microswitch KDB

	PORT_START("ROW4")
	PORT_KEY(A2_KEY_FR2,        KEYCODE_F6,         0,              0,            "FR2"          )  //!< ADL right function key 2
	PORT_KEY(A2_KEY_FL2,        KEYCODE_F2,         0,              0,            "FL2"          )  //!< ADL left function key 2

	PORT_START("ROW5")
	PORT_KEY(A2_KEY_FR4,        KEYCODE_F8,         0,              0,            "FR4"          )  //!< ADL right funtion key 4
	PORT_KEY(A2_KEY_BW,         KEYCODE_F10,        0,              0,            "BW"           )  //!< ADL BW (?)

	PORT_START("ROW6")
	PORT_KEY(A2_KEY_FR3,        KEYCODE_F7,         0,              0,            "FR3"          )  //!< ADL right function key 3
	PORT_KEY(A2_KEY_FL1,        KEYCODE_F1,         0,              0,            "FL1"          )  //!< ADL left function key 1
	PORT_KEY(A2_KEY_FL3,        KEYCODE_F3,         0,              0,            "FL3"          )  //!< ADL left function key 3

	PORT_START("ROW7")
	PORT_KEY(A2_KEY_FR1,        KEYCODE_F5,         0,              0,            "FR1"          )  //!< ADL right function key 1
	PORT_KEY(A2_KEY_FL4,        KEYCODE_F4,         0,              0,            "FL4"          )  //!< ADL left function key 4
	PORT_KEY(A2_KEY_FR5,        KEYCODE_F9,         0,              0,            "FR5"          )  //!< ADL right function key 5

	PORT_START("mouseb0")   // Mouse button 0
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("Mouse RED (left)")      PORT_PLAYER(1) PORT_CODE(MOUSECODE_BUTTON1) PORT_CHANGED_MEMBER( ":maincpu", alto2_cpu_device, mouse_button_0, 0 )
	PORT_START("mouseb1")   // Mouse button 1
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_NAME("Mouse BLUE (right)")    PORT_PLAYER(1) PORT_CODE(MOUSECODE_BUTTON2) PORT_CHANGED_MEMBER( ":maincpu", alto2_cpu_device, mouse_button_1, 0 )
	PORT_START("mouseb2")   // Mouse button 2
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3) PORT_NAME("Mouse YELLOW (middle)") PORT_PLAYER(1) PORT_CODE(MOUSECODE_BUTTON3) PORT_CHANGED_MEMBER( ":maincpu", alto2_cpu_device, mouse_button_2, 0 )

	PORT_START("mousex")    // Mouse - X AXIS
	PORT_BIT( 0xffff, 0x00, IPT_MOUSE_X) PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_CHANGED_MEMBER( ":maincpu", alto2_cpu_device, mouse_motion_x, 0 )

	PORT_START("mousey")    // Mouse - Y AXIS
	PORT_BIT( 0xffff, 0x00, IPT_MOUSE_Y) PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_CHANGED_MEMBER( ":maincpu", alto2_cpu_device, mouse_motion_y, 0 )

	PORT_START("CONFIG")    /* Memory switch on AIM board */
	PORT_CONFNAME( 0x01, 0x01, "Memory switch")
	PORT_CONFSETTING( 0x00, "on")
	PORT_CONFSETTING( 0x01, "off")
	PORT_CONFNAME( 0x70, 0x00, "Ethernet breath-of-life")
	PORT_CONFSETTING( 0x00, "off")
	PORT_CONFSETTING( 0x10, "5 seconds")
	PORT_CONFSETTING( 0x20, "10 seconds")
	PORT_CONFSETTING( 0x30, "15 seconds")
	PORT_CONFSETTING( 0x40, "30 seconds")
	PORT_CONFSETTING( 0x50, "60 seconds")
	PORT_CONFSETTING( 0x60, "90 seconds")
	PORT_CONFSETTING( 0x70, "120 seconds")

	PORT_START("ETHERID")
	PORT_DIPNAME( 0377, 0042, "Ethernet ID")
	PORT_DIPSETTING( 0000, "No ether") PORT_DIPSETTING( 0001, "ID 001")   PORT_DIPSETTING( 0002, "ID 002")   PORT_DIPSETTING( 0003, "ID 003")
	PORT_DIPSETTING( 0004, "ID 004")   PORT_DIPSETTING( 0005, "ID 005")   PORT_DIPSETTING( 0006, "ID 006")   PORT_DIPSETTING( 0007, "ID 007")
	PORT_DIPSETTING( 0010, "ID 010")   PORT_DIPSETTING( 0011, "ID 011")   PORT_DIPSETTING( 0012, "ID 012")   PORT_DIPSETTING( 0013, "ID 013")
	PORT_DIPSETTING( 0014, "ID 014")   PORT_DIPSETTING( 0015, "ID 015")   PORT_DIPSETTING( 0016, "ID 016")   PORT_DIPSETTING( 0017, "ID 017")
	PORT_DIPSETTING( 0020, "ID 020")   PORT_DIPSETTING( 0021, "ID 021")   PORT_DIPSETTING( 0022, "ID 022")   PORT_DIPSETTING( 0023, "ID 023")
	PORT_DIPSETTING( 0024, "ID 024")   PORT_DIPSETTING( 0025, "ID 025")   PORT_DIPSETTING( 0026, "ID 026")   PORT_DIPSETTING( 0027, "ID 027")
	PORT_DIPSETTING( 0030, "ID 030")   PORT_DIPSETTING( 0031, "ID 031")   PORT_DIPSETTING( 0032, "ID 032")   PORT_DIPSETTING( 0033, "ID 033")
	PORT_DIPSETTING( 0034, "ID 034")   PORT_DIPSETTING( 0035, "ID 035")   PORT_DIPSETTING( 0036, "ID 036")   PORT_DIPSETTING( 0037, "ID 037")
	PORT_DIPSETTING( 0040, "ID 040")   PORT_DIPSETTING( 0041, "ID 041")   PORT_DIPSETTING( 0042, "ID 042")   PORT_DIPSETTING( 0043, "ID 043")
	PORT_DIPSETTING( 0044, "ID 044")   PORT_DIPSETTING( 0045, "ID 045")   PORT_DIPSETTING( 0046, "ID 046")   PORT_DIPSETTING( 0047, "ID 047")
	PORT_DIPSETTING( 0050, "ID 050")   PORT_DIPSETTING( 0051, "ID 051")   PORT_DIPSETTING( 0052, "ID 052")   PORT_DIPSETTING( 0053, "ID 053")
	PORT_DIPSETTING( 0054, "ID 054")   PORT_DIPSETTING( 0055, "ID 055")   PORT_DIPSETTING( 0056, "ID 056")   PORT_DIPSETTING( 0057, "ID 057")
	PORT_DIPSETTING( 0060, "ID 060")   PORT_DIPSETTING( 0061, "ID 061")   PORT_DIPSETTING( 0062, "ID 062")   PORT_DIPSETTING( 0063, "ID 063")
	PORT_DIPSETTING( 0064, "ID 064")   PORT_DIPSETTING( 0065, "ID 065")   PORT_DIPSETTING( 0066, "ID 066")   PORT_DIPSETTING( 0067, "ID 067")
	PORT_DIPSETTING( 0070, "ID 070")   PORT_DIPSETTING( 0071, "ID 071")   PORT_DIPSETTING( 0072, "ID 072")   PORT_DIPSETTING( 0073, "ID 073")
	PORT_DIPSETTING( 0074, "ID 074")   PORT_DIPSETTING( 0075, "ID 075")   PORT_DIPSETTING( 0076, "ID 076")   PORT_DIPSETTING( 0077, "ID 077")
	PORT_DIPSETTING( 0100, "ID 100")   PORT_DIPSETTING( 0101, "ID 101")   PORT_DIPSETTING( 0102, "ID 102")   PORT_DIPSETTING( 0103, "ID 103")
	PORT_DIPSETTING( 0104, "ID 104")   PORT_DIPSETTING( 0105, "ID 105")   PORT_DIPSETTING( 0106, "ID 106")   PORT_DIPSETTING( 0107, "ID 107")
	PORT_DIPSETTING( 0110, "ID 110")   PORT_DIPSETTING( 0111, "ID 111")   PORT_DIPSETTING( 0112, "ID 112")   PORT_DIPSETTING( 0113, "ID 113")
	PORT_DIPSETTING( 0114, "ID 114")   PORT_DIPSETTING( 0115, "ID 115")   PORT_DIPSETTING( 0116, "ID 116")   PORT_DIPSETTING( 0117, "ID 117")
	PORT_DIPSETTING( 0120, "ID 120")   PORT_DIPSETTING( 0121, "ID 121")   PORT_DIPSETTING( 0122, "ID 122")   PORT_DIPSETTING( 0123, "ID 123")
	PORT_DIPSETTING( 0124, "ID 124")   PORT_DIPSETTING( 0125, "ID 125")   PORT_DIPSETTING( 0126, "ID 126")   PORT_DIPSETTING( 0127, "ID 127")
	PORT_DIPSETTING( 0130, "ID 130")   PORT_DIPSETTING( 0131, "ID 131")   PORT_DIPSETTING( 0132, "ID 132")   PORT_DIPSETTING( 0133, "ID 133")
	PORT_DIPSETTING( 0134, "ID 134")   PORT_DIPSETTING( 0135, "ID 135")   PORT_DIPSETTING( 0136, "ID 136")   PORT_DIPSETTING( 0137, "ID 137")
	PORT_DIPSETTING( 0140, "ID 140")   PORT_DIPSETTING( 0141, "ID 141")   PORT_DIPSETTING( 0142, "ID 142")   PORT_DIPSETTING( 0143, "ID 143")
	PORT_DIPSETTING( 0144, "ID 144")   PORT_DIPSETTING( 0145, "ID 145")   PORT_DIPSETTING( 0146, "ID 146")   PORT_DIPSETTING( 0147, "ID 147")
	PORT_DIPSETTING( 0150, "ID 150")   PORT_DIPSETTING( 0151, "ID 151")   PORT_DIPSETTING( 0152, "ID 152")   PORT_DIPSETTING( 0153, "ID 153")
	PORT_DIPSETTING( 0154, "ID 154")   PORT_DIPSETTING( 0155, "ID 155")   PORT_DIPSETTING( 0156, "ID 156")   PORT_DIPSETTING( 0157, "ID 157")
	PORT_DIPSETTING( 0160, "ID 160")   PORT_DIPSETTING( 0161, "ID 161")   PORT_DIPSETTING( 0162, "ID 162")   PORT_DIPSETTING( 0163, "ID 163")
	PORT_DIPSETTING( 0164, "ID 164")   PORT_DIPSETTING( 0165, "ID 165")   PORT_DIPSETTING( 0166, "ID 166")   PORT_DIPSETTING( 0167, "ID 167")
	PORT_DIPSETTING( 0170, "ID 170")   PORT_DIPSETTING( 0171, "ID 171")   PORT_DIPSETTING( 0172, "ID 172")   PORT_DIPSETTING( 0173, "ID 173")
	PORT_DIPSETTING( 0174, "ID 174")   PORT_DIPSETTING( 0175, "ID 175")   PORT_DIPSETTING( 0176, "ID 176")   PORT_DIPSETTING( 0177, "ID 177")
	PORT_DIPSETTING( 0200, "ID 200")   PORT_DIPSETTING( 0201, "ID 201")   PORT_DIPSETTING( 0202, "ID 202")   PORT_DIPSETTING( 0203, "ID 203")
	PORT_DIPSETTING( 0204, "ID 204")   PORT_DIPSETTING( 0205, "ID 205")   PORT_DIPSETTING( 0206, "ID 206")   PORT_DIPSETTING( 0207, "ID 207")
	PORT_DIPSETTING( 0210, "ID 210")   PORT_DIPSETTING( 0211, "ID 211")   PORT_DIPSETTING( 0212, "ID 212")   PORT_DIPSETTING( 0213, "ID 213")
	PORT_DIPSETTING( 0214, "ID 214")   PORT_DIPSETTING( 0215, "ID 215")   PORT_DIPSETTING( 0216, "ID 216")   PORT_DIPSETTING( 0217, "ID 217")
	PORT_DIPSETTING( 0220, "ID 220")   PORT_DIPSETTING( 0221, "ID 221")   PORT_DIPSETTING( 0222, "ID 222")   PORT_DIPSETTING( 0223, "ID 223")
	PORT_DIPSETTING( 0224, "ID 224")   PORT_DIPSETTING( 0225, "ID 225")   PORT_DIPSETTING( 0226, "ID 226")   PORT_DIPSETTING( 0227, "ID 227")
	PORT_DIPSETTING( 0230, "ID 230")   PORT_DIPSETTING( 0231, "ID 231")   PORT_DIPSETTING( 0232, "ID 232")   PORT_DIPSETTING( 0233, "ID 233")
	PORT_DIPSETTING( 0234, "ID 234")   PORT_DIPSETTING( 0235, "ID 235")   PORT_DIPSETTING( 0236, "ID 236")   PORT_DIPSETTING( 0237, "ID 237")
	PORT_DIPSETTING( 0240, "ID 240")   PORT_DIPSETTING( 0241, "ID 241")   PORT_DIPSETTING( 0242, "ID 242")   PORT_DIPSETTING( 0243, "ID 243")
	PORT_DIPSETTING( 0244, "ID 244")   PORT_DIPSETTING( 0245, "ID 245")   PORT_DIPSETTING( 0246, "ID 246")   PORT_DIPSETTING( 0247, "ID 247")
	PORT_DIPSETTING( 0250, "ID 250")   PORT_DIPSETTING( 0251, "ID 251")   PORT_DIPSETTING( 0252, "ID 252")   PORT_DIPSETTING( 0253, "ID 253")
	PORT_DIPSETTING( 0254, "ID 254")   PORT_DIPSETTING( 0255, "ID 255")   PORT_DIPSETTING( 0256, "ID 256")   PORT_DIPSETTING( 0257, "ID 257")
	PORT_DIPSETTING( 0260, "ID 260")   PORT_DIPSETTING( 0261, "ID 261")   PORT_DIPSETTING( 0262, "ID 262")   PORT_DIPSETTING( 0263, "ID 263")
	PORT_DIPSETTING( 0264, "ID 264")   PORT_DIPSETTING( 0265, "ID 265")   PORT_DIPSETTING( 0266, "ID 266")   PORT_DIPSETTING( 0267, "ID 267")
	PORT_DIPSETTING( 0270, "ID 270")   PORT_DIPSETTING( 0271, "ID 271")   PORT_DIPSETTING( 0272, "ID 272")   PORT_DIPSETTING( 0273, "ID 273")
	PORT_DIPSETTING( 0274, "ID 274")   PORT_DIPSETTING( 0275, "ID 275")   PORT_DIPSETTING( 0276, "ID 276")   PORT_DIPSETTING( 0277, "ID 277")
	PORT_DIPSETTING( 0300, "ID 300")   PORT_DIPSETTING( 0301, "ID 301")   PORT_DIPSETTING( 0302, "ID 302")   PORT_DIPSETTING( 0303, "ID 303")
	PORT_DIPSETTING( 0304, "ID 304")   PORT_DIPSETTING( 0305, "ID 305")   PORT_DIPSETTING( 0306, "ID 306")   PORT_DIPSETTING( 0307, "ID 307")
	PORT_DIPSETTING( 0310, "ID 310")   PORT_DIPSETTING( 0311, "ID 311")   PORT_DIPSETTING( 0312, "ID 312")   PORT_DIPSETTING( 0313, "ID 313")
	PORT_DIPSETTING( 0314, "ID 314")   PORT_DIPSETTING( 0315, "ID 315")   PORT_DIPSETTING( 0316, "ID 316")   PORT_DIPSETTING( 0317, "ID 317")
	PORT_DIPSETTING( 0320, "ID 320")   PORT_DIPSETTING( 0321, "ID 321")   PORT_DIPSETTING( 0322, "ID 322")   PORT_DIPSETTING( 0323, "ID 323")
	PORT_DIPSETTING( 0324, "ID 324")   PORT_DIPSETTING( 0325, "ID 325")   PORT_DIPSETTING( 0326, "ID 326")   PORT_DIPSETTING( 0327, "ID 327")
	PORT_DIPSETTING( 0330, "ID 330")   PORT_DIPSETTING( 0331, "ID 331")   PORT_DIPSETTING( 0332, "ID 332")   PORT_DIPSETTING( 0333, "ID 333")
	PORT_DIPSETTING( 0334, "ID 334")   PORT_DIPSETTING( 0335, "ID 335")   PORT_DIPSETTING( 0336, "ID 336")   PORT_DIPSETTING( 0337, "ID 337")
	PORT_DIPSETTING( 0340, "ID 340")   PORT_DIPSETTING( 0341, "ID 341")   PORT_DIPSETTING( 0342, "ID 342")   PORT_DIPSETTING( 0343, "ID 343")
	PORT_DIPSETTING( 0344, "ID 344")   PORT_DIPSETTING( 0345, "ID 345")   PORT_DIPSETTING( 0346, "ID 346")   PORT_DIPSETTING( 0347, "ID 347")
	PORT_DIPSETTING( 0350, "ID 350")   PORT_DIPSETTING( 0351, "ID 351")   PORT_DIPSETTING( 0352, "ID 352")   PORT_DIPSETTING( 0353, "ID 353")
	PORT_DIPSETTING( 0354, "ID 354")   PORT_DIPSETTING( 0355, "ID 355")   PORT_DIPSETTING( 0356, "ID 356")   PORT_DIPSETTING( 0357, "ID 357")
	PORT_DIPSETTING( 0360, "ID 360")   PORT_DIPSETTING( 0361, "ID 361")   PORT_DIPSETTING( 0362, "ID 362")   PORT_DIPSETTING( 0363, "ID 363")
	PORT_DIPSETTING( 0364, "ID 364")   PORT_DIPSETTING( 0365, "ID 365")   PORT_DIPSETTING( 0366, "ID 366")   PORT_DIPSETTING( 0367, "ID 367")
	PORT_DIPSETTING( 0370, "ID 370")   PORT_DIPSETTING( 0371, "ID 371")   PORT_DIPSETTING( 0372, "ID 372")   PORT_DIPSETTING( 0373, "ID 373")
	PORT_DIPSETTING( 0374, "ID 374")   PORT_DIPSETTING( 0375, "ID 375")
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
							ALTO2_DISPLAY_TOTAL_HEIGHT,  0, ALTO2_DISPLAY_HEIGHT + ALTO2_FAKE_STATUS_H)
	MCFG_SCREEN_REFRESH_RATE(60)    // two interlaced fields
	MCFG_SCREEN_VBLANK_TIME(ALTO2_DISPLAY_VBLANK_TIME)
	MCFG_SCREEN_UPDATE_DEVICE("maincpu", alto2_cpu_device, screen_update)
	MCFG_SCREEN_VBLANK_DEVICE("maincpu", alto2_cpu_device, screen_eof)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_DEFAULT_LAYOUT( layout_vertical )

	MCFG_PALETTE_ADD_WHITE_AND_BLACK("palette")

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
COMP( 1977, alto2,  0,      0,      alto2,   alto2, alto2_state, alto2, "Xerox", "Alto-II", MACHINE_NO_SOUND )
