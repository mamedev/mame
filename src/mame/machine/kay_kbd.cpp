// license:GPL-2.0+
// copyright-holders:Juergen Buchmueller
/******************************************************************************
 *
 *  kay_kbd.c
 *
 *  Kaypro II Serial Keyboard
 *
 *  Most of this is copied from the old kaypro.c,
 *  rather than re-inventing the wheel.
 *
 *  Juergen Buchmueller, July 1998
 *  Benjamin C. W. Sittler, July 1998 (new keyboard table)
 *
 *  Converted to a serial device (as far as MESS will allow)
 *  by Robbbert, April 2009.
 *
 ******************************************************************************/

#include "emu.h"
#include "sound/beep.h"
#include "includes/kaypro.h"


/*
 * The KAYPRO keyboard has roughly the following layout:
 *
 *                                                                  [up] [down] [left] [right]
 *         [ESC] [1!] [2@] [3#] [4$] [5%] [6^] [7&] [8*] [9(] [0)] [-_] [=+] [`~] [BACK SPACE]
 *           [TAB] [qQ] [wW] [eE] [rR] [tT] [yY] [uU] [iI] [oO] [pP] [[{] []}]       [DEL]
 *[CTRL] [CAPS LOCK] [aA] [sS] [dD] [fF] [gG] [hH] [jJ] [kK] [lL] [;:] ['"] [RETURN] [\|]
 *            [SHIFT] [zZ] [xX] [cC] [vV] [bB] [nN] [mM] [,<] [.>] [/?] [SHIFT] [LINE FEED]
 *                      [                 SPACE                     ]
 *
 * [7] [8] [9] [-]
 * [4] [5] [6] [,]
 * [1] [2] [3]
 * [  0  ] [.] [ENTER]
 *
 * Notes on Appearance
 * -------------------
 * The RETURN key is shaped like a backwards "L". The keypad ENTER key is actually
 * oriented vertically, not horizontally. The alpha keys are marked with the uppercase letters
 * only. Other keys with two symbols have the shifted symbol printed above the unshifted symbol.
 * The keypad is actually located to the right of the main keyboard; it is shown separately here
 * as a convenience to users of narrow listing windows. The arrow keys are actually marked with
 * arrow graphics pointing in the appropriate directions. The F and J keys are specially shaped,
 * since they are the "home keys" for touch-typing. The CAPS LOCK key has a build-in red indicator
 * which is lit when CAPS LOCK is pressed once, and extinguished when the key is pressed again.
 *
 * Technical Notes
 * ---------------
 * The keyboard interfaces to the computer using a serial protocol. Modifier keys are handled
 * inside the keyboards, as is the CAPS LOCK key. The arrow keys and the numeric keypad send
 * non-ASCII codes which are not affected by the modifier keys. The remaining keys send the
 * appropriate ASCII values.
 *
 * The keyboard has a built-in buzzer which is activated briefly by a non-modifier keypress,
 * producing a "clicking" sound for user feedback. Additionally, this buzzer can be activated
 * for a longer period by sending a 0x04 byte to the keyboard. This is used by the ROM soft
 * terminal to alert the user in case of a BEL.
 *
 * 2008-05 FP:
 * Small note about natural keyboard support: currently,
 * - "Line Feed" is mapped to the 'End' key
 * - "Keypad ," is not mapped
 */

struct kay_kbd_t
{
	beep_device *beeper;
	UINT8 buff[16];
	UINT8 head;
	UINT8 tail;
	UINT8 beep_on;
	UINT8 control_status;
	UINT8 keyrows[10];
	int lastrow;
	int mask;
	int key;
	int repeat;
	int repeater;
};

INPUT_PORTS_START( kay_kbd )
	PORT_START("ROW0")
	PORT_BIT(0x01, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC)           PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT(0x02, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)             PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x04, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)             PORT_CHAR('2') PORT_CHAR('@')
	PORT_BIT(0x08, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)             PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x10, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)             PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x20, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)             PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x40, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)             PORT_CHAR('6') PORT_CHAR('^')
	PORT_BIT(0x80, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)             PORT_CHAR('7') PORT_CHAR('&')

	PORT_START("ROW1")
	PORT_BIT(0x01, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)             PORT_CHAR('8') PORT_CHAR('*')
	PORT_BIT(0x02, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)             PORT_CHAR('9') PORT_CHAR('(')
	PORT_BIT(0x04, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)             PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT(0x08, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)             PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT(0x10, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)            PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT(0x20, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE)             PORT_CHAR('`') PORT_CHAR('~')
	PORT_BIT(0x40, 0x00, IPT_KEYBOARD) PORT_NAME("BACK SPACE")          PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT(0x80, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB)           PORT_CHAR('\t')

	PORT_START("ROW2")
	PORT_BIT(0x01, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)             PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x02, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)             PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x04, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)             PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x08, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)             PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x10, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)             PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x20, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)             PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x40, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)             PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x80, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)             PORT_CHAR('i') PORT_CHAR('I')

	PORT_START("ROW3")
	PORT_BIT(0x01, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)             PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT(0x02, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)             PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x04, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)         PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x08, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE)        PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT(0x10, 0x00, IPT_KEYBOARD) PORT_NAME("RETURN")              PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x20, 0x00, IPT_KEYBOARD) PORT_NAME("DEL") PORT_CODE(KEYCODE_DEL)  PORT_CHAR(UCHAR_MAMEKEY(DEL))
	PORT_BIT(0x40, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL)          PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT(0x80, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_CAPSLOCK)          PORT_TOGGLE PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))

	PORT_START("ROW4")
	PORT_BIT(0x01, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)             PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x02, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)             PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x04, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)             PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x08, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)             PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x10, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)             PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT(0x20, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)             PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x40, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)             PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x80, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)             PORT_CHAR('k') PORT_CHAR('K')

	PORT_START("ROW5")
	PORT_BIT(0x01, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)             PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x02, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)         PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT(0x04, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)         PORT_CHAR('\'') PORT_CHAR('"')
	PORT_BIT(0x08, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH)         PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT(0x10, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT)            PORT_CHAR(UCHAR_MAMEKEY(LSHIFT))
	PORT_BIT(0x20, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)             PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x40, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)             PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x80, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)             PORT_CHAR('c') PORT_CHAR('C')

	PORT_START("ROW6")
	PORT_BIT(0x01, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)             PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x02, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)             PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x04, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)             PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x08, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)             PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x10, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)         PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x20, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)          PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x40, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)         PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x80, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_RSHIFT)            PORT_CHAR(UCHAR_MAMEKEY(RSHIFT))

	PORT_START("ROW7")
	PORT_BIT(0x01, 0x00, IPT_KEYBOARD) PORT_NAME("LINE FEED")           PORT_CODE(KEYCODE_END) PORT_CHAR(UCHAR_MAMEKEY(END))
	PORT_BIT(0x02, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)         PORT_CHAR(' ')
	PORT_BIT(0x04, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS_PAD)         PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD))
	PORT_BIT(0x08, 0x00, IPT_KEYBOARD) PORT_NAME("Keypad ,")            PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT(0x10, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER_PAD)         PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD))
	PORT_BIT(0x20, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL_PAD)           PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))
	PORT_BIT(0x40, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_0_PAD)             PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT(0x80, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_1_PAD)             PORT_CHAR(UCHAR_MAMEKEY(1_PAD))

	PORT_START("ROW8")
	PORT_BIT(0x01, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_2_PAD)             PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT(0x02, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_3_PAD)             PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT(0x04, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_4_PAD)             PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT(0x08, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_5_PAD)             PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT(0x10, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_6_PAD)             PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT(0x20, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_7_PAD)             PORT_CHAR(UCHAR_MAMEKEY(7_PAD))
	PORT_BIT(0x40, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_8_PAD)             PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT(0x80, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_9_PAD)         PORT_CHAR(UCHAR_MAMEKEY(9_PAD))

	PORT_START("ROW9")
	PORT_BIT(0x01, 0x00, IPT_KEYBOARD) PORT_NAME("\xE2\x86\x91")            PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x02, 0x00, IPT_KEYBOARD) PORT_NAME("\xE2\x86\x93")            PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x04, 0x00, IPT_KEYBOARD) PORT_NAME("\xE2\x86\x90")            PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x08, 0x00, IPT_KEYBOARD) PORT_NAME("\xE2\x86\x92")            PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0xf0, 0x00, IPT_UNUSED)
INPUT_PORTS_END

static const UINT8 keyboard[8][10][8] = {
	{ /* normal */
		{ 27,'1','2','3','4','5','6','7'},
		{'8','9','0','-','=','`',  8,  9},
		{'q','w','e','r','t','y','u','i'},
		{'o','p','[',']', 13,127,  0,  0},
		{'a','s','d','f','g','h','j','k'},
		{'l',';', 39, 92,  0,'z','x','c'},
		{'v','b','n','m',',','.','/',  0},
		{ 10, 32, 228, 211, 195, 178, 177, 192},
		{ 193, 194, 208, 209, 210, 225, 226, 227},
		{ 241, 242, 243, 244,  0,  0,  0,  0},
	},
	{ /* Shift */
		{ 27,'!','@','#','$','%','^','&'},
		{'*','(',')','_','+','~',  8,  9},
		{'Q','W','E','R','T','Y','U','I'},
		{'O','P','{','}', 13,127,  0,  0},
		{'A','S','D','F','G','H','J','K'},
		{'L',':','"','|',  0,'Z','X','C'},
		{'V','B','N','M','<','>','?',  0},
		{ 10, 32, 228, 211, 195, 178, 177, 192},
		{ 193, 194, 208, 209, 210, 225, 226, 227},
		{ 241, 242, 243, 244,  0,  0,  0,  0},
	},
	{ /* Control */
		{ 27,'1','2','3','4','5','6','7'},
		{'8','9','0','-','=','`',  8,  9},
		{ 17, 23,  5, 18, 20, 25, 21,  9},
		{ 15, 16, 27, 29, 13,127,  0,  0},
		{  1, 19,  4,  6,  7,  8, 10, 11},
		{ 12,';', 39, 28,  0, 26, 24,  3},
		{ 22,  2, 14, 13,',','.','/',  0},
		{ 10, 32, 228, 211, 195, 178, 177, 192},
		{ 193, 194, 208, 209, 210, 225, 226, 227},
		{ 241, 242, 243, 244,  0,  0,  0,  0},
	},
	{ /* Shift+Control */
		{ 27,'!',  0,'#','$','%', 30,'&'},
		{'*','(',')', 31,'+','~',  8,  9},
		{ 17, 23,  5, 18, 20, 25, 21,  9},
		{ 15, 16, 27, 29, 13,127,  0,  0},
		{  1, 19,  4,  6,  7,  8, 10, 11},
		{ 12,':','"', 28,  0, 26, 24,  3},
		{ 22,  2, 14, 13,',','.','/',  0},
		{ 10, 32, 228, 211, 195, 178, 177, 192},
		{ 193, 194, 208, 209, 210, 225, 226, 227},
		{ 241, 242, 243, 244,  0,  0,  0,  0},
	},
	{ /* CapsLock */
		{ 27,'1','2','3','4','5','6','7'},
		{'8','9','0','-','=','`',  8,  9},
		{'Q','W','E','R','T','Y','U','I'},
		{'O','P','[',']', 13,127,  0,  0},
		{'A','S','D','F','G','H','J','K'},
		{'L',';', 39, 92,  0,'Z','X','C'},
		{'V','B','N','M',',','.','/',  0},
		{ 10, 32, 228, 211, 195, 178, 177, 192},
		{ 193, 194, 208, 209, 210, 225, 226, 227},
		{ 241, 242, 243, 244,  0,  0,  0,  0},
	},
	{ /* Shift+CapsLock */
		{ 27,'!','@','#','$','%','^','&'},
		{'*','(',')','_','+','~',  8,  9},
		{'Q','W','E','R','T','Y','U','I'},
		{'O','P','{','}', 13,127,  0,  0},
		{'A','S','D','F','G','H','J','K'},
		{'L',':','"','|',  0,'Z','X','C'},
		{'V','B','N','M','<','>','?',  0},
		{ 10, 32, 228, 211, 195, 178, 177, 192},
		{ 193, 194, 208, 209, 210, 225, 226, 227},
		{ 241, 242, 243, 244,  0,  0,  0,  0},
	},
	{ /* Control+CapsLock */
		{ 27,'1','2','3','4','5','6','7'},
		{'8','9','0','-','=','`',  8,  9},
		{ 17, 23,  5, 18, 20, 25, 21,  9},
		{ 15, 16, 27, 29, 13,127,  0,  0},
		{  1, 19,  4,  6,  7,  8, 10, 11},
		{ 12,';', 39, 28,  0, 26, 24,  3},
		{ 22,  2, 14, 13,',','.','/',  0},
		{ 10, 32, 228, 211, 195, 178, 177, 192},
		{ 193, 194, 208, 209, 210, 225, 226, 227},
		{ 241, 242, 243, 244,  0,  0,  0,  0},
	},
	{ /* Shift+Control+CapsLock */
		{ 27,'!',  0,'#','$','%', 30,'&'},
		{'*','(',')', 31,'+','~',  8,  9},
		{ 17, 23,  5, 18, 20, 25, 21,  9},
		{ 15, 16, 27, 29, 13,127,  0,  0},
		{  1, 19,  4,  6,  7,  8, 10, 11},
		{ 12,':','"', 28,  0, 26, 24,  3},
		{ 22,  2, 14, 13,',','.','/',  0},
		{ 10, 32, 228, 211, 195, 178, 177, 192},
		{ 193, 194, 208, 209, 210, 225, 226, 227},
		{ 241, 242, 243, 244,  0,  0,  0,  0},
	},
};

MACHINE_RESET_MEMBER(kaypro_state,kay_kbd)
{
	kay_kbd_t *kbd = m_kbd = auto_alloc_clear(machine(), <kay_kbd_t>());

	/* disable CapsLock LED initially */
	output().set_led_value(1, 1);
	output().set_led_value(1, 0);
	kbd->beeper = machine().device<beep_device>("beeper");
	kbd->beep_on = 1;
	kbd->control_status = 0x14;
	kbd->beeper->set_state(0);
	kbd->head = kbd->tail = 0;          /* init buffer */
}

/******************************************************
 * vertical blank interrupt
 * used to scan keyboard; newly pressed keys
 * are stuffed into a keyboard buffer;
 * also drives keyboard LEDs and
 * and handles autorepeating keys
 ******************************************************/
INTERRUPT_GEN_MEMBER(kaypro_state::kay_kbd_interrupt)
{
	kay_kbd_t *kbd = m_kbd;
	int mod, row, col, chg, newval;
	UINT8 *keyrows = kbd->keyrows;

	if( kbd->repeat )
	{
		if( !--kbd->repeat )
			kbd->repeater = 4;
	}
	else
	if( kbd->repeater )
	{
		kbd->repeat = kbd->repeater;
	}

	row = 9;
	newval = machine().root_device().ioport("ROW9")->read();
	chg = keyrows[row] ^ newval;

	if (!chg) { newval = machine().root_device().ioport("ROW8")->read(); chg = keyrows[--row] ^ newval; }
	if (!chg) { newval = machine().root_device().ioport("ROW7")->read(); chg = keyrows[--row] ^ newval; }
	if (!chg) { newval = machine().root_device().ioport("ROW6")->read(); chg = keyrows[--row] ^ newval; }
	if (!chg) { newval = machine().root_device().ioport("ROW5")->read(); chg = keyrows[--row] ^ newval; }
	if (!chg) { newval = machine().root_device().ioport("ROW4")->read(); chg = keyrows[--row] ^ newval; }
	if (!chg) { newval = machine().root_device().ioport("ROW3")->read(); chg = keyrows[--row] ^ newval; }
	if (!chg) { newval = machine().root_device().ioport("ROW2")->read(); chg = keyrows[--row] ^ newval; }
	if (!chg) { newval = machine().root_device().ioport("ROW1")->read(); chg = keyrows[--row] ^ newval; }
	if (!chg) { newval = machine().root_device().ioport("ROW0")->read(); chg = keyrows[--row] ^ newval; }
	if (!chg) --row;

	if (row >= 0)
	{
		kbd->repeater = 0x00;
		kbd->mask = 0x00;
		kbd->key = 0x00;
		kbd->lastrow = row;
		/* CapsLock LED */
		if( row == 3 && chg == 0x80 )
			output().set_led_value(1, (kbd->keyrows[3] & 0x80) ? 0 : 1);

		if (newval & chg)   /* key(s) pressed ? */
		{
			mod = 0;

			/* Shift modifier */
			if ( (keyrows[5] & 0x10) || (keyrows[6] & 0x80) )
				mod |= 1;

			/* Control modifier */
			if (keyrows[3] & 0x40)
				mod |= 2;

			/* CapsLock modifier */
			if (keyrows[3] & 0x80)
				mod |= 4;

			/* find newval key */
			kbd->mask = 0x01;
			for (col = 0; col < 8; col ++)
			{
				if (chg & kbd->mask)
				{
					newval &= kbd->mask;
					kbd->key = keyboard[mod][row][col];
					break;
				}
				kbd->mask <<= 1;
			}
			if( kbd->key )  /* normal key */
			{
				kbd->repeater = 30;
				kay_kbd_in(kbd->key);
			}
			else
			if( (row == 0) && (chg == 0x04) ) /* Ctrl-@ (NUL) */
				kay_kbd_in(0);
			keyrows[row] |= newval;
		}
		else
		{
			kbd->keyrows[row] = newval;
		}
		kbd->repeat = kbd->repeater;
	}
	else if ( kbd->key && (keyrows[kbd->lastrow] & kbd->mask) && kbd->repeat == 0 )
	{
		kay_kbd_in(kbd->key);
	}
}

#if 0

/******************************************************
 *  kaypro_const_w (write console status ;)
 *  bit
 *  0   flush keyboard buffer
 ******************************************************/
static WRITE8_HANDLER ( kaypro2_const_w )
{
	if (data & 1)
		kbd->head = kbd->tail = 0;
}

#endif

/******************************************************
 *  stuff character into the keyboard buffer
 *  releases CPU if it was waiting for a key
 *  sounds bell if buffer would overflow
 ******************************************************/
void kaypro_state::kay_kbd_in(UINT8 data )
{
	kay_kbd_t *kbd = m_kbd;
	UINT8 kbd_head_old;

	kbd_head_old = kbd->head;
	kbd->buff[kbd->head] = data;
	kbd->head = (kbd->head + 1) % sizeof(kbd->buff);
	/* will the buffer overflow ? */
	if (kbd->head == kbd->tail)
	{
		kbd->head = kbd_head_old;
		kay_kbd_d_w(4);
	}
	else
		kay_kbd_d_w(1);
}


UINT8 kaypro_state::kay_kbd_c_r()
{
/*  d4 transmit buffer empty - 1=ok to send
    d2 appears to be receive buffer empty - 1=ok to receive
    d0 keyboard buffer empty - 1=key waiting to be used */

	kay_kbd_t *kbd = m_kbd;
	UINT8 data = kbd->control_status;

	if( kbd->head != kbd->tail )
		data++;

	return data;
}

UINT8 kaypro_state::kay_kbd_d_r()
{
/* return next key in buffer */

	kay_kbd_t *kbd = m_kbd;
	UINT8 data = 0;

	if (kbd->tail != kbd->head)
	{
		data = kbd->buff[kbd->tail];
		kbd->tail = (kbd->tail + 1) % sizeof(kbd->buff);
	}

	return data;
}

TIMER_CALLBACK_MEMBER( kaypro_state::kay_kbd_beepoff )
{
	kay_kbd_t *kbd = m_kbd;
	kbd->beeper->set_state(0);
	kbd->control_status |= 4;
}

void kaypro_state::kay_kbd_d_w( UINT8 data )
{
/* Beeper control - lengths need verifying
    01 - keyclick
    02 - short beep
    04 - standard bell beep
    08 - mute
    16 - unmute */

	kay_kbd_t *kbd = m_kbd;
	UINT16 length = 0;

	if (data & 0x10)
		kbd->beep_on = 1;
	else
	if (data & 0x08)
		kbd->beep_on = 0;
	else
	if (kbd->beep_on)
	{
		if (data & 0x04)
			length = 400;
		else
		if (data & 0x02)
			length = 200;
		else
		if (data & 0x01)
			length = 4;

		if (length)
		{
			kbd->control_status &= 0xfb;
			machine().scheduler().timer_set(attotime::from_msec(length), timer_expired_delegate(FUNC(kaypro_state::kay_kbd_beepoff),this));
			kbd->beeper->set_state(1);
		}
	}
}
