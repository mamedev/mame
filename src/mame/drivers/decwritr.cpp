// license:BSD-3-Clause
// copyright-holders:Jonathan Gevaryahu
/*************************************************************************

    decwritr.c
    Digital Equipment Corporation
    DECwriter III (LA120) Teletype/Teleprinter, 1978

**************************************************************************/

#include "emu.h"
#include "render.h"
#include "cpu/i8085/i8085.h"
#include "machine/i8251.h"
#include "sound/beep.h"

#define KBD_VERBOSE 1
#define LED_VERBOSE 1

//**************************************************************************
//  DRIVER STATE
//**************************************************************************

class decwriter_state : public driver_device
{
public:
	// constructor
	decwriter_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_speaker(*this, "beeper"),
		m_uart(*this, "i8251")
	{
	}
	required_device<cpu_device> m_maincpu;
	required_device<beep_device> m_speaker;
	required_device<i8251_device> m_uart;
	//required_device<generic_terminal_device> m_terminal;
	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
	{
		bitmap.fill(rgb_t::black);
		return 0;
	}
	DECLARE_READ8_MEMBER(la120_KBD_r);
	DECLARE_WRITE8_MEMBER(la120_LED_w);
	DECLARE_READ8_MEMBER(la120_NVR_r);
	DECLARE_WRITE8_MEMBER(la120_NVR_w);
	DECLARE_READ8_MEMBER(la120_DC305_r);
	DECLARE_WRITE8_MEMBER(la120_DC305_w);
private:
	virtual void machine_start() override;
	//virtual void machine_reset();
	ioport_port* m_col_array[16];
	UINT8 m_led_array;
	UINT8 m_led_7seg_counter;
	UINT8 m_led_7seg[4];
};

READ8_MEMBER( decwriter_state::la120_KBD_r )
{
	/* for reading the keyboard array, addr bits 5-11 are ignored.
	 * a15 a14 a13 a12 a11 a10  a9  a8  a7  a6  a5  a4  a3  a2  a1  a0
	 *   0   0   1   1   x   x   x   x   x   x   x   *   *   *   *   *
	 *                                               |   |   \---\---\--- column select
	 *                                               |   \--------------- kbd_banksel0
	 *                                               \------------------- kbd_banksel1
	 *      if both banks are selected, the resulting row read is a binary OR of both (LA120-TM1, page 4-5)
	 *
	 * d7 d6 d5 d4 d3 d2 d1 d0
	 *  \--\--\--\--\--\--\--\-- read from rows
	 */
	UINT8 code = 0;
	if (offset&0x8) code |= m_col_array[offset&0x7]->read();
	if (offset&0x10) code |= m_col_array[(offset&0x7)+8]->read();
#ifdef KBD_VERBOSE
	logerror("Keyboard column %X read, returning %02X\n", offset&0xF, code);
#endif
	return code;
}

WRITE8_MEMBER( decwriter_state::la120_LED_w )
{
	/* for writing the keyboard array, addr bits 5-11 are ignored.
	 * a15 a14 a13 a12 a11 a10  a9  a8  a7  a6  a5  a4  a3  a2  a1  a0
	 *   0   0   1   1   x   x   x   x   x   x   x   0   *   *   *   0    turn OFF LED # <a3:a1> and clear 7seg counter
	 *   0   0   1   1   x   x   x   x   x   x   x   0   *   *   *   1    turn ON LED # <a3:a1> and clear 7seg counter
	 *   0   0   1   1   x   x   x   x   x   x   x   1   *   *   *   *    display a digit <a3:a0> in the digit pointed to by 7seg counter and increment counter
	 * data bus is UNUSED.
	 */
	if (!(offset&0x10)) // we're updating an led state
	{
#ifdef LED_VERBOSE
		logerror("Updated LED status array: LED #%d is now %d\n", ((offset&0xe)>>1), (offset&1));
#endif
		m_led_array &= ((1<<((offset&0xe)>>1))^0xFF); // mask out the old bit
		m_led_array |= ((offset&1)<<((offset&0xe)>>1)); // OR in the new bit
		m_led_7seg_counter = 0;
	}
	else // we're updating the 7segment display
	{
		m_led_7seg_counter++;
		m_led_7seg_counter &= 0xF;
#ifdef LED_VERBOSE
		logerror("Updated 7seg display: displaying a digit of %d in position %d\n", (offset&0xF)^0xF, m_led_7seg_counter-1);
#endif
		if ((m_led_7seg_counter >= 1) && (m_led_7seg_counter <= 4))
		{
			m_led_7seg[m_led_7seg_counter-1] = (offset&0xF)^0xF;
		}
	}
	popmessage("LEDs: %c %c %c %c : %s  %s  %s  %s  %s  %s  %s  %s\n",
	m_led_7seg[3]+0x30, m_led_7seg[2]+0x30, m_led_7seg[1]+0x30, m_led_7seg[0]+0x30,
	(m_led_array&0x80)?"ON LINE":"-------",
	(m_led_array&0x40)?"LOCAL":"-----",
	(m_led_array&0x20)?"ALT CHR SET":"-----------",
	(m_led_array&0x10)?"<LED4>":"-----",
	(m_led_array&0x8)?"CTS":"---",
	(m_led_array&0x4)?"DSR":"---",
	(m_led_array&0x2)?"SET-UP":"------",
	(m_led_array&0x1)?"PAPER OUT":"---------" );
}

READ8_MEMBER( decwriter_state::la120_NVR_r )
{
	return 0xFF;
}

WRITE8_MEMBER( decwriter_state::la120_NVR_w )
{
}
READ8_MEMBER( decwriter_state::la120_DC305_r )
{
	return 0xFF;
}
WRITE8_MEMBER( decwriter_state::la120_DC305_w )
{
}

static ADDRESS_MAP_START(la120_mem, AS_PROGRAM, 8, decwriter_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0x0000, 0x2fff ) AM_ROM
	AM_RANGE( 0x3000, 0x3fff ) AM_READWRITE(la120_KBD_r, la120_LED_w) // keyboard read, write to status and 7seg LEDS
	AM_RANGE( 0x4000, 0x43ff ) AM_MIRROR(0x0c00) AM_RAM // 1k 'low ram'
	AM_RANGE( 0x5000, 0x53ff ) AM_MIRROR(0x0c00) AM_RAM // 1k 'high ram'
	AM_RANGE( 0x6000, 0x6fff ) AM_MIRROR(0x08fe) AM_READWRITE(la120_NVR_r, la120_NVR_w) // ER1400 EAROM
	AM_RANGE( 0x7000, 0x7003 ) AM_MIRROR(0x0ffc) AM_READWRITE(la120_DC305_r, la120_DC305_w) // DC305 printer controller ASIC stuff; since this can generate interrupts this needs to be split to its own device.
	// 8000-ffff is reserved for expansion (i.e. unused, open bus)
ADDRESS_MAP_END

/*
 * 8080 IO address map (x = ignored; * = selects address within this range)
 * (a15 to a8 are latched the same value as a7-a0 on the 8080 and 8085)
   a7  a6  a5  a4  a3  a2  a1  a0
   0   x   x   x   x   x   0   0     RW     8251 Data
   0   x   x   x   x   x   0   1     RW     8251 Status/Control
   0   x   x   x   x   x   1   x     RW     Flags Read/Write
   1   x   x   x   x   x   x   x     RW     Expansion (Open bus)
 */
static ADDRESS_MAP_START(la120_io, AS_IO, 8, decwriter_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00, 0x00) AM_MIRROR(0x7C) AM_DEVREADWRITE("i8251", i8251_device, data_r, data_w) // 8251 Data
	AM_RANGE(0x01, 0x01) AM_MIRROR(0x7C) AM_DEVREADWRITE("i8251", i8251_device, status_r, control_w) // 8251 Status/Control
	//AM_RANGE(0x02, 0x02) AM_MIRROR(0x7D) // other io ports
	// 0x80-0xff are reserved for expansion (i.e. unused, open bus)
	ADDRESS_MAP_GLOBAL_MASK(0xff)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( la120 )
	PORT_START("COL0")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Num .") PORT_CODE(KEYCODE_DEL_PAD)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("PF4") PORT_CODE(KEYCODE_F4)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Local LF") PORT_CODE(KEYCODE_F12)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Break") PORT_CODE(KEYCODE_F6)
		PORT_BIT(0x60, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_START("COL1")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("PF3") PORT_CODE(KEYCODE_F3)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Local FF") PORT_CODE(KEYCODE_F11)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("\\ |") PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\') PORT_CHAR('|')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Delete") PORT_CODE(KEYCODE_DEL) PORT_CHAR(127)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Backspace") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
		PORT_BIT(0x60, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED) // this is actually the cover open interlock sensor
	PORT_START("COL2")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("PF2") PORT_CODE(KEYCODE_F2)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("HERE IS") PORT_CODE(KEYCODE_F10)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(alternate) Opt LF") PORT_CODE(KEYCODE_RALT)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("` ~") PORT_CODE(KEYCODE_TILDE) PORT_CHAR('`') PORT_CHAR('~')
		PORT_BIT(0x60, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED) // this is actually the paper out interlock sensor
	PORT_START("COL3")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("PF1") PORT_CODE(KEYCODE_F1)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("LF") PORT_CODE(KEYCODE_LALT) PORT_CHAR(10)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("LINE LOCAL") PORT_CODE(KEYCODE_F9)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("] }") PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("= +") PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=') PORT_CHAR('+')
		PORT_BIT(0x60, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_START("COL4")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Num -") PORT_CODE(KEYCODE_MINUS_PAD)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Num ,") PORT_CODE(KEYCODE_PLUS_PAD)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("' \"") PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('\'') PORT_CHAR('"')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("[ {") PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_CHAR('{')
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("- _") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('_')
		PORT_BIT(0x60, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_TOGGLE PORT_NAME("Caps lock") PORT_CODE(KEYCODE_CAPSLOCK) // TODO: does the physical switch toggle?
	PORT_START("COL5")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Num 0") PORT_CODE(KEYCODE_0_PAD)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("/ ?") PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("; :") PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR(':')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("0 )") PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR(')')
		PORT_BIT(0x60, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Ctrl") PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_START("COL6")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Num 9") PORT_CODE(KEYCODE_9_PAD)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(". >") PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("9 (") PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR('(')
		PORT_BIT(0x60, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Return") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_START("COL7")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Num 8") PORT_CODE(KEYCODE_8_PAD)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(", <") PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("8 *") PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('*')
		PORT_BIT(0x60, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Set Up") PORT_CODE(KEYCODE_F5)
	PORT_START("COL8")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Num 7") PORT_CODE(KEYCODE_7_PAD)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("7 &") PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('&')
		PORT_BIT(0xe0, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_START("COL9")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Num 6") PORT_CODE(KEYCODE_6_PAD)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("6 ^") PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('^')
		PORT_BIT(0xe0, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_START("COLA")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Num 5") PORT_CODE(KEYCODE_5_PAD)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("5 %") PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
		PORT_BIT(0xe0, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_START("COLB")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Num 4") PORT_CODE(KEYCODE_4_PAD)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("4 $") PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
		PORT_BIT(0xe0, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_START("COLC")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Num 3") PORT_CODE(KEYCODE_3_PAD)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("3 #") PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
		PORT_BIT(0xe0, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_START("COLD")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Num 2") PORT_CODE(KEYCODE_2_PAD)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("2 @") PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('@')
		PORT_BIT(0xe0, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_START("COLE")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Num 1") PORT_CODE(KEYCODE_1_PAD)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("1 !") PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
		PORT_BIT(0xe0, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_START("COLF")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Num Enter") PORT_CODE(KEYCODE_ENTER_PAD)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED) // this is actually the RO flag
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("View") PORT_CODE(KEYCODE_LWIN)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Tab") PORT_CODE(KEYCODE_TAB) PORT_CHAR(9)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Esc") PORT_CODE(KEYCODE_ESC) PORT_CHAR(27)
		PORT_BIT(0xe0, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END

void decwriter_state::machine_start()
{
	m_speaker->set_frequency(786); // TODO: LA120 speaker is controlled by asic; VT100 has: 7.945us per serial clock = ~125865.324hz, / 160 clocks per char = ~ 786 hz
#if 0
	output().set_value("online_led",1);
	output().set_value("local_led", 0);
	output().set_value("noscroll_led",1);
	output().set_value("basic_led", 1);
	output().set_value("hardcopy_led", 1);
	output().set_value("l1_led", 1);
	output().set_value("l2_led", 1);
#endif
	char kbdcol[8];
	// look up all 16 tags 'the slow way' but only once on reset
	for (int i = 0; i < 16; i++)
	{
		sprintf(kbdcol,"COL%X", i);
		m_col_array[i] = ioport(kbdcol);
	}
	m_led_array = 0;
	m_led_7seg_counter = 0;
	m_led_7seg[0] = m_led_7seg[1] = m_led_7seg[2] = m_led_7seg[3] = 0xF;
}

/*
void decwriter_state::machine_reset()
{
}
*/

//**************************************************************************
//  MACHINE DRIVERS
//**************************************************************************

static MACHINE_CONFIG_START( la120, decwriter_state )

	MCFG_CPU_ADD("maincpu",I8080, XTAL_18MHz / 9) // 18Mhz xtal on schematics, using an i8224 clock divider/reset sanitizer IC
	MCFG_CPU_PROGRAM_MAP(la120_mem)
	MCFG_CPU_IO_MAP(la120_io)

	/* video hardware */
	//TODO: no actual screen! has 8 leds above the keyboard (similar to vt100/vk100) and has 4 7segment leds for showing an error code.
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_UPDATE_DRIVER(decwriter_state, screen_update)
	MCFG_SCREEN_SIZE(640,480)
	MCFG_SCREEN_VISIBLE_AREA(0,639, 0,479)
	MCFG_SCREEN_REFRESH_RATE(30)

	//MCFG_DEFAULT_LAYOUT( layout_la120 )

	/* audio hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("beeper", BEEP, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	/* i8251 */
	MCFG_DEVICE_ADD("i8251", I8251, 0)
	/*
	MCFG_I8251_TXD_HANDLER(DEVWRITELINE(RS232_TAG, rs232_port_device, write_txd))
	MCFG_I8251_DTR_HANDLER(DEVWRITELINE(RS232_TAG, rs232_port_device, write_dtr))
	MCFG_I8251_RTS_HANDLER(DEVWRITELINE(RS232_TAG, rs232_port_device, write_rts))

	MCFG_RS232_PORT_ADD(RS232_TAG, default_rs232_devices, NULL)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("i8251", i8251_device, write_rxd))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE("i8251", i8251_device, write_dsr))

	MCFG_DEVICE_ADD(COM5016T_TAG, COM8116, XTAL_5_0688MHz)
	MCFG_COM8116_FR_HANDLER(DEVWRITELINE("i8251", i8251_device, write_rxc))
	MCFG_COM8116_FT_HANDLER(DEVWRITELINE("i8251", i8251_device, write_txc))
	*/
MACHINE_CONFIG_END



//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( la120 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	// later romset, with 23-003e2.e6, 23-004e2.e8, 23-005e2.e12, 23-006e2.e17 replaced by one rom, 23-038e4.e6 which may be a concatenation of the old roms, unclear.
	ROM_LOAD( "23-038e4-00.e6", 0x0000, 0x2000, CRC(cad4eb09) SHA1(d5db117da363d36817476f906251ea4ee1cb14b8))
	ROM_LOAD( "23-007e2-00.e4", 0x2000, 0x0800, CRC(41eaebf1) SHA1(c7d05417b24b853280d1636776d399a0aea34720)) // used by both earlier and later romset
	// there is an optional 3 roms, european and APL (and BOTH) rom which goes from 2000-2fff in e4, all undumped.
	// there is another romset used on the Bell Teleprinter 1000 (Model LAS12) which I believe is 23-004e4.e6 and 23-086e2.e4
ROM_END


//**************************************************************************
//  DRIVERS
//**************************************************************************
/*    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS          INIT  COMPANY                          FULLNAME       FLAGS */
COMP( 1978, la120, 0,      0,      la120,   la120, driver_device, 0,    "Digital Equipment Corporation", "DECwriter III (LA120)", MACHINE_NO_SOUND | MACHINE_IS_SKELETON | MACHINE_NOT_WORKING )
