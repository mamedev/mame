// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

	Hazeltine 1500
	original machine (c) 1977 Hazeltine Corporation

	perliminary driver by Ryan Holtz

TODO:
    - pretty much everything

References:
	[1]: Hazeltine_1500_Series_Maintenance_Manual_Dec77.pdf, on Bitsavers

****************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "machine/ay31015.h"
#include "machine/kb3600.h"
#include "machine/com8116.h"
#include "machine/keyboard.h"

#define KEYBOARD_TAG	"keyboard"
#define CPU_TAG			"maincpu"
#define UART_TAG		"uart"
#define BAUDGEN_TAG		"baudgen"
#define KBDC_TAG		"ay53600"
#define BAUDPORT_TAG	"BAUD"
#define MISCPORT_TAG	"MISC"
#define MISCKEYS_TAG	"MISC_KEYS"

#define SR2_FULL_DUPLEX	(0x01)
#define SR2_UPPER_ONLY	(0x08)

#define SR3_PB_RESET	(0x04)

#define KBD_STATUS_KBDR	(0x01)

class hazl1500_state : public driver_device
{
public:
	hazl1500_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, CPU_TAG)
		, m_uart(*this, UART_TAG)
		, m_kbdc(*this, KBDC_TAG)
		, m_baud_dips(*this, BAUDPORT_TAG)
		, m_misc_dips(*this, MISCPORT_TAG)
		, m_kbd_misc_keys(*this, MISCKEYS_TAG)
		, m_status_reg_3(0)
		, m_kbd_status_latch(0)
	{
	}

	virtual void machine_reset() override;

	uint32_t screen_update_hazl1500(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	DECLARE_WRITE_LINE_MEMBER(com5016_fr_w);

	DECLARE_READ8_MEMBER(system_test_r); // noted as "for use with auto test equip" in flowchart on pg. 30, ref[1], jumps to 0x8000 if bit 0 is unset
	DECLARE_READ8_MEMBER(status_reg_2_r);
	DECLARE_WRITE8_MEMBER(status_reg_3_w);

	DECLARE_READ8_MEMBER(kbd_status_latch_r);
	DECLARE_READ8_MEMBER(kbd_encoder_r);
	DECLARE_READ_LINE_MEMBER(ay3600_shift_r);
	DECLARE_READ_LINE_MEMBER(ay3600_control_r);
	DECLARE_WRITE_LINE_MEMBER(ay3600_data_ready_w);

private:
	required_device<cpu_device> m_maincpu;
	required_device<ay31015_device> m_uart;
	required_device<ay3600_device> m_kbdc;
	required_ioport m_baud_dips;
	required_ioport m_misc_dips;
	required_ioport m_kbd_misc_keys;

	uint8_t m_status_reg_3;
	uint8_t m_kbd_status_latch;
};

void hazl1500_state::machine_reset()
{
	m_status_reg_3 = 0;
	m_kbd_status_latch = 0;
}

WRITE_LINE_MEMBER( hazl1500_state::com5016_fr_w )
{
	m_uart->rx_process();
	m_uart->tx_process();
}

uint32_t hazl1500_state::screen_update_hazl1500(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

READ8_MEMBER( hazl1500_state::system_test_r )
{
	return 0xff;
}

READ8_MEMBER( hazl1500_state::status_reg_2_r )
{
	uint8_t misc_dips = m_misc_dips->read();
	uint8_t status = 0;

	if (misc_dips & 0x10)
		status |= SR2_FULL_DUPLEX;
	if (misc_dips & 0x40)
		status |= SR2_UPPER_ONLY;

	return status ^ 0xff;
}

WRITE8_MEMBER( hazl1500_state::status_reg_3_w )
{
	m_status_reg_3 = data;
}

READ8_MEMBER( hazl1500_state::kbd_status_latch_r )
{
	return m_kbd_status_latch;
}

READ8_MEMBER(hazl1500_state::kbd_encoder_r)
{
	return m_kbdc->b_r() & 0xff; // TODO: This should go through an 8048, but we have no dump of it currently.
}

READ_LINE_MEMBER(hazl1500_state::ay3600_shift_r)
{
	// either shift key
	if (m_kbd_misc_keys->read() & 0x06)
	{
		return 1;
	}

	return 0;
}

READ_LINE_MEMBER(hazl1500_state::ay3600_control_r)
{
	if (m_kbd_misc_keys->read() & 0x08)
	{
		return 1;
	}

	return 0;
}

WRITE_LINE_MEMBER(hazl1500_state::ay3600_data_ready_w)
{
	if (state)
		m_kbd_status_latch |= KBD_STATUS_KBDR;
	else
		m_kbd_status_latch &= ~KBD_STATUS_KBDR;
}

static ADDRESS_MAP_START(hazl1500_mem, AS_PROGRAM, 8, hazl1500_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x07ff) AM_ROM
	AM_RANGE(0x3000, 0x377f) AM_RAM AM_SHARE("char_ram")
	AM_RANGE(0x3780, 0x37ff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START(hazl1500_io, AS_IO, 8, hazl1500_state)
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x7f, 0x7f) AM_READWRITE(status_reg_2_r, status_reg_3_w)
	AM_RANGE(0xdf, 0xdf) AM_READ(kbd_encoder_r)
	AM_RANGE(0xef, 0xef) AM_READ(system_test_r)
	AM_RANGE(0xf7, 0xf7) AM_READ(kbd_status_latch_r)
ADDRESS_MAP_END

	/*
	  Hazeltine 1500 key matrix (from ref[1])

	      | Y0  | Y1  | Y2  | Y3  | Y4  | Y5  | Y6  | Y7  | Y8  | Y9  |
	      |     |     |     |     |     |     |     |     |     |     |
	  ----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----|
	  X1  | TAB |  1  |  3  |  5  |  7  |BREAK| ^~  |     |     |     |
	  ----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----|
	  X2  |BAKSP|  2  |  4  |  6  |  8  |  9  | -=  |     |     |     |
	  ----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----|
	  X3  |     | ESC |  W  |  R  |  Y  |  0  | `@  | CLR |NP , |NP 9 |
	  ----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----|
	  X4  |     |  Q  |  E  |  T  |  U  |  O  | {[  | |\  |NP 7 |NP 6 |
	  ----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----|
	  X5  |HOME |  A  |  D  |  G  |  I  |  P  | *:  | LF  |NP 8 |NP 5 |
	  ----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----|
	  X6  |     |  S  |  F  |  H  |  J  | +;  | }]  | DEL |NP 4 |NP 2 |
	  ----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----|
	  X7  |     |  Z  |  C  |  B  |  K  |  L  | <,  | CR  |NP 1 |NP 3 |
	  ----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----|
	  X8  |     |  X  |  V  |  N  |SPACE|  M  | >.  | ?/  |NP 0 |NP . |
	  ----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----|
	*/

static INPUT_PORTS_START( hazl1500 )
	PORT_START("X0")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_UNUSED)

	// X1  | TAB |  1  |  3  |  5  |  7  |BREAK| ^~  |     |     |     |
	PORT_START("X1")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB) PORT_CHAR('\t') PORT_NAME("Tab")
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)  PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)  PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)  PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)  PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_PAUSE) PORT_NAME("Break")
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('^') PORT_CHAR('~')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_UNUSED)

	// X2  |BAKSP|  2  |  4  |  6  |  8  |  9  | -=  |     |     |     |
	PORT_START("X2")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE)	PORT_NAME("Backspace")
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)			PORT_CHAR('2') PORT_CHAR('\"')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)			PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)			PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)			PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)			PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)      PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_UNUSED)

	// X3  |     | ESC |  W  |  R  |  Y  |  0  | `@  | CLR |NP , |NP 9 |
	PORT_START("X3")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC)		PORT_NAME("Esc")
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)			PORT_CHAR('W') PORT_CHAR('w')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)			PORT_CHAR('R') PORT_CHAR('r')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)			PORT_CHAR('Y') PORT_CHAR('y')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)			PORT_CHAR('0')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)	PORT_CHAR('@') PORT_CHAR('`')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL)		PORT_NAME("Clr")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL_PAD)
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9_PAD)		PORT_CHAR(UCHAR_MAMEKEY(9_PAD))

	// X4  |     |  Q  |  E  |  T  |  U  |  O  | {[  | |\  |NP 7 |NP 6 |
	PORT_START("X4")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)			PORT_CHAR('Q') PORT_CHAR('q')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)			PORT_CHAR('E') PORT_CHAR('e')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)			PORT_CHAR('T') PORT_CHAR('t')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)			PORT_CHAR('U') PORT_CHAR('u')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)			PORT_CHAR('O') PORT_CHAR('o')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)	PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH)	PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7_PAD)		PORT_CHAR(UCHAR_MAMEKEY(7_PAD))
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6_PAD)		PORT_CHAR(UCHAR_MAMEKEY(6_PAD))

	// X5  |HOME |  A  |  D  |  G  |  I  |  P  | *:  | LF  |NP 8 |NP 5 |
	PORT_START("X5")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_HOME)	PORT_NAME("Home")
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)		PORT_CHAR('A') PORT_CHAR('a')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)		PORT_CHAR('D') PORT_CHAR('d')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)		PORT_CHAR('G') PORT_CHAR('g')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)		PORT_CHAR('I') PORT_CHAR('i')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)		PORT_CHAR('P') PORT_CHAR('p')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)	PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_PGDN)	PORT_NAME("Line Feed")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8_PAD)	PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5_PAD)	PORT_CHAR(UCHAR_MAMEKEY(5_PAD))

	// X6  |     |  S  |  F  |  H  |  J  | +;  | }]  | DEL |NP 4 |NP 2 |
	PORT_START("X6")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)		PORT_CHAR('S') PORT_CHAR('s')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)		PORT_CHAR('F') PORT_CHAR('f')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)		PORT_CHAR('H') PORT_CHAR('h')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)		PORT_CHAR('J') PORT_CHAR('j')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)	PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL)	PORT_NAME("Del")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4_PAD)	PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2_PAD)	PORT_CHAR(UCHAR_MAMEKEY(2_PAD))

	// X7  |     |  Z  |  C  |  B  |  K  |  L  | <,  | CR  |NP 1 |NP 3 |
	PORT_START("X7")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)		PORT_CHAR('Z') PORT_CHAR('z')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)		PORT_CHAR('C') PORT_CHAR('c')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)		PORT_CHAR('B') PORT_CHAR('b')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)		PORT_CHAR('K') PORT_CHAR('k')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)		PORT_CHAR('L') PORT_CHAR('l')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)	PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER)	PORT_CHAR(13)  PORT_NAME("Return")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1_PAD)	PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3_PAD)	PORT_CHAR(UCHAR_MAMEKEY(3_PAD))

	// X8  |     |  X  |  V  |  N  |SPACE|  M  | >.  | ?/  |NP 0 |NP . |
	PORT_START("X8")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)		PORT_CHAR('X') PORT_CHAR('x')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)		PORT_CHAR('V') PORT_CHAR('v')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)		PORT_CHAR('N') PORT_CHAR('n')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)	PORT_CHAR(' ')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)		PORT_CHAR('M') PORT_CHAR('m')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)	PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)	PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0_PAD)	PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER_PAD) PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))

	PORT_START(MISCKEYS_TAG)
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Left Shift")   PORT_CODE(KEYCODE_LSHIFT)   PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Right Shift")  PORT_CODE(KEYCODE_RSHIFT)   PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Control")      PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START(BAUDPORT_TAG)
	PORT_DIPNAME( 0xff, 0x08, "Baud Rate" )
	PORT_DIPSETTING( 0x01, "110" )
	PORT_DIPSETTING( 0x02, "300" )
	PORT_DIPSETTING( 0x04, "1200" )
	PORT_DIPSETTING( 0x08, "1800" )
	PORT_DIPSETTING( 0x10, "2400" )
	PORT_DIPSETTING( 0x20, "4800" )
	PORT_DIPSETTING( 0x40, "9600" )
	PORT_DIPSETTING( 0x80, "19.2K" )

	PORT_START(MISCPORT_TAG)
	PORT_DIPNAME( 0x0f, 0x01, "Parity" )
	PORT_DIPSETTING( 0x01, "Even" )
	PORT_DIPSETTING( 0x02, "Odd" )
	PORT_DIPSETTING( 0x04, "1" )
	PORT_DIPSETTING( 0x08, "0" )
	PORT_DIPNAME( 0x10, 0x10, "Duplex" )
	PORT_DIPSETTING( 0x00, "Half" )
	PORT_DIPSETTING( 0x10, "Full" )
	PORT_DIPNAME( 0x20, 0x20, "Auto" )
	PORT_DIPSETTING( 0x00, "LF" )
	PORT_DIPSETTING( 0x20, "CR" )
	PORT_DIPNAME( 0x40, 0x40, "Case" )
	PORT_DIPSETTING( 0x00, "Upper and Lower" )
	PORT_DIPSETTING( 0x40, "Upper" )
	PORT_DIPNAME( 0x80, 0x80, "Video" )
	PORT_DIPSETTING( 0x00, "Standard" )
	PORT_DIPSETTING( 0x80, "Reverse" )
INPUT_PORTS_END

/* F4 Character Displayer */
static const gfx_layout hazl1500_charlayout =
{
	8, 16,
	128,
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	8*16
};

static GFXDECODE_START( hazl1500 )
	GFXDECODE_ENTRY( "chargen", 0x0000, hazl1500_charlayout, 0, 1 )
GFXDECODE_END

static MACHINE_CONFIG_START( hazl1500, hazl1500_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",I8080, XTAL_1MHz) //unknown clock
	MCFG_CPU_PROGRAM_MAP(hazl1500_mem)
	MCFG_CPU_IO_MAP(hazl1500_io)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(640, 384)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 384-1)
	MCFG_SCREEN_UPDATE_DRIVER(hazl1500_state, screen_update_hazl1500)

	MCFG_PALETTE_ADD_MONOCHROME("palette")
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", hazl1500)

	MCFG_DEVICE_ADD(BAUDGEN_TAG, COM8116, XTAL_5_0688MHz)
	MCFG_COM8116_FR_HANDLER(WRITELINE(hazl1500_state, com5016_fr_w))

	MCFG_DEVICE_ADD(UART_TAG, AY51013, 0)

	/* keyboard controller */
	MCFG_DEVICE_ADD(KBDC_TAG, AY3600, 0)
	MCFG_AY3600_MATRIX_X0(IOPORT("X0"))
	MCFG_AY3600_MATRIX_X1(IOPORT("X1"))
	MCFG_AY3600_MATRIX_X2(IOPORT("X2"))
	MCFG_AY3600_MATRIX_X3(IOPORT("X3"))
	MCFG_AY3600_MATRIX_X4(IOPORT("X4"))
	MCFG_AY3600_MATRIX_X5(IOPORT("X5"))
	MCFG_AY3600_MATRIX_X6(IOPORT("X6"))
	MCFG_AY3600_MATRIX_X7(IOPORT("X7"))
	MCFG_AY3600_MATRIX_X8(IOPORT("X8"))
	MCFG_AY3600_SHIFT_CB(READLINE(hazl1500_state, ay3600_shift_r))
	MCFG_AY3600_CONTROL_CB(READLINE(hazl1500_state, ay3600_control_r))
	MCFG_AY3600_DATA_READY_CB(WRITELINE(hazl1500_state, ay3600_data_ready_w))
MACHINE_CONFIG_END


ROM_START( hazl1500 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "h15s-00I-10-3.bin", 0x0000, 0x0800, CRC(a2015f72) SHA1(357cde517c3dcf693de580881add058c7b26dfaa))

	ROM_REGION( 0x800, "chargen", ROMREGION_ERASEFF )
	ROM_LOAD( "u83_chr.bin", 0x0000, 0x0800, CRC(e0c6b734) SHA1(7c42947235c66c41059fd4384e09f4f3a17c9857))
ROM_END

/* Driver */

/*    YEAR  NAME      PARENT	COMPAT   MACHINE   INPUT     CLASS			INIT    COMPANY                		FULLNAME     		FLAGS */
COMP( 1977, hazl1500, 0,		0,       hazl1500, hazl1500, driver_device,	0,		"Hazeltine Corporation",	"Hazeltine 1500",	MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW)
