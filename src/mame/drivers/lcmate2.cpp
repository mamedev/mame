// license:BSD-3-Clause
// copyright-holders:Sandro Ronco, Robbbert
/***************************************************************************

        Laser Compumate2

        17/04/2011 Initial driver by Robbbert, using info supplied by
        DMEnduro. He owns one of these machines.
        21/04/2011 Hooked up the lcd controller and added keyboard
        input. [Sandro Ronco]
        22/04/2011 Small adjustments resulting from team investigations.

        Chips: Z80 CPU, 8K of RAM, 32K ROM, 'VTCL', '8826KX',
               RP5C15 RTC.

        Display: 2 lines of 20 characters LCD.

        There are no devices for file saving or loading, is more of
        a PDA format with all programs (address book, etc) in the ROM.

        Sound: a piezo speaker.

        ToDo:
        - RTC doesn't remember the time
        - Alarm doesn't work
        - Reset/On button to be added
        - The ROM has INT and NMI code; investigate if something uses it.


****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/rp5c15.h"
#include "machine/nvram.h"
#include "video/hd44780.h"
#include "sound/speaker.h"
#include "rendlay.h"


class lcmate2_state : public driver_device
{
public:
	lcmate2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	m_lcdc(*this, "hd44780"),
	m_rtc(*this, "rtc"),
	m_speaker(*this, "speaker")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<hd44780_device> m_lcdc;
	required_device<rp5c15_device> m_rtc;
	required_device<speaker_sound_device> m_speaker;

	virtual void machine_start() override;

	DECLARE_READ8_MEMBER( key_r );
	DECLARE_WRITE8_MEMBER( speaker_w );
	DECLARE_WRITE8_MEMBER( bankswitch_w );
	DECLARE_PALETTE_INIT(lcmate2);
};

WRITE8_MEMBER( lcmate2_state::speaker_w )
{
	m_speaker->level_w(BIT(data, 6));
}

// offsets are FE,FD,FB,F7,EF,DF,BF,7F to scan a particular row, or 00 to check if any key pressed
READ8_MEMBER( lcmate2_state::key_r )
{
	UINT8 i,data = 0xff;
	char kbdrow[8];

	for (i=0; i<8; i++)
	{
		if (BIT(offset, i)==0)
		{
			sprintf(kbdrow,"LINE%d", i);
			data &= ioport(kbdrow)->read();
		}
	}

	return data;
}

WRITE8_MEMBER( lcmate2_state::bankswitch_w )
{
	membank("rombank")->set_entry(data&0x0f);
}

static ADDRESS_MAP_START(lcmate2_mem, AS_PROGRAM, 8, lcmate2_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0x0000, 0x3fff ) AM_ROM
	AM_RANGE( 0x4000, 0x7fff ) AM_ROMBANK("rombank")
	AM_RANGE( 0x8000, 0x9fff ) AM_RAM   AM_MIRROR(0x6000) AM_SHARE("nvram")
ADDRESS_MAP_END

static ADDRESS_MAP_START(lcmate2_io, AS_IO, 8, lcmate2_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x000f) AM_DEVREADWRITE("rtc", rp5c15_device, read, write)
	AM_RANGE(0x1000, 0x1000) AM_WRITE(speaker_w)
	AM_RANGE(0x1fff, 0x1fff) AM_WRITE(bankswitch_w)

	AM_RANGE(0x3000, 0x3000) AM_DEVWRITE("hd44780", hd44780_device, control_write)
	AM_RANGE(0x3001, 0x3001) AM_DEVWRITE("hd44780", hd44780_device, data_write)
	AM_RANGE(0x3002, 0x3002) AM_DEVREAD("hd44780", hd44780_device, control_read)
	AM_RANGE(0x3003, 0x3003) AM_DEVREAD("hd44780", hd44780_device, data_read)

	AM_RANGE(0x5000, 0x50ff) AM_READ(key_r)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( lcmate2 )
	PORT_START("LINE0")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("UP")     PORT_CODE(KEYCODE_UP)           PORT_CHAR(UCHAR_MAMEKEY(UP))
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ENTER")  PORT_CODE(KEYCODE_ENTER)        PORT_CHAR(13)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ESC")    PORT_CODE(KEYCODE_ESC)          PORT_CHAR(UCHAR_MAMEKEY(ESC))
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SHIFT")  PORT_CODE(KEYCODE_LSHIFT)       PORT_CHAR(UCHAR_SHIFT_1)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CTRL")   PORT_CODE(KEYCODE_LCONTROL)     PORT_CHAR(UCHAR_SHIFT_2)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Q")      PORT_CODE(KEYCODE_Q)            PORT_CHAR('q')  PORT_CHAR('Q')
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1")      PORT_CODE(KEYCODE_1)            PORT_CHAR('1')  PORT_CHAR('!')
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_UNUSED

	PORT_START("LINE1")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(",")      PORT_CODE(KEYCODE_COMMA)        PORT_CHAR(',')  PORT_CHAR('/')
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("K")      PORT_CODE(KEYCODE_K)            PORT_CHAR('k')  PORT_CHAR('K')
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CAPSLOCK")   PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Z")      PORT_CODE(KEYCODE_Z)            PORT_CHAR('z')  PORT_CHAR('Z')
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A")      PORT_CODE(KEYCODE_A)            PORT_CHAR('a')  PORT_CHAR('A')
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("W")      PORT_CODE(KEYCODE_W)            PORT_CHAR('w')  PORT_CHAR('W')
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2")      PORT_CODE(KEYCODE_2)            PORT_CHAR('2')  PORT_CHAR('@')
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_UNUSED

	PORT_START("LINE2")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(".")      PORT_CODE(KEYCODE_STOP)         PORT_CHAR('.')  PORT_CHAR('?')
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("O")      PORT_CODE(KEYCODE_O)            PORT_CHAR('o')  PORT_CHAR('O')
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("[")      PORT_CODE(KEYCODE_OPENBRACE)    PORT_CHAR('[')  PORT_CHAR('{')
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("X")      PORT_CODE(KEYCODE_X)            PORT_CHAR('x')  PORT_CHAR('X')
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S")      PORT_CODE(KEYCODE_S)            PORT_CHAR('s')  PORT_CHAR('S')
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E")      PORT_CODE(KEYCODE_E)            PORT_CHAR('e')  PORT_CHAR('E')
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3")      PORT_CODE(KEYCODE_3)            PORT_CHAR('3')  PORT_CHAR('#')
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_UNUSED

	PORT_START("LINE3")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(";")      PORT_CODE(KEYCODE_COLON)        PORT_CHAR(';')  PORT_CHAR(':')
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("-")      PORT_CODE(KEYCODE_MINUS)        PORT_CHAR('-')  PORT_CHAR('=')
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("]")      PORT_CODE(KEYCODE_CLOSEBRACE)   PORT_CHAR(']')  PORT_CHAR('}')
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C")      PORT_CODE(KEYCODE_C)            PORT_CHAR('c')  PORT_CHAR('C')
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D")      PORT_CODE(KEYCODE_D)            PORT_CHAR('d')  PORT_CHAR('D')
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("R")      PORT_CODE(KEYCODE_R)            PORT_CHAR('r')  PORT_CHAR('R')
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4")      PORT_CODE(KEYCODE_4)            PORT_CHAR('4')  PORT_CHAR('$')
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_UNUSED

	PORT_START("LINE4")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("'")      PORT_CODE(KEYCODE_QUOTE)        PORT_CHAR('\'') PORT_CHAR('\"')
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("BACKSPACE")  PORT_CODE(KEYCODE_BACKSPACE)    PORT_CHAR(UCHAR_MAMEKEY(DEL))
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SPACE")  PORT_CODE(KEYCODE_SPACE)        PORT_CHAR(' ')
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("V")      PORT_CODE(KEYCODE_V)            PORT_CHAR('v')  PORT_CHAR('V')
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F")      PORT_CODE(KEYCODE_F)            PORT_CHAR('f')  PORT_CHAR('F')
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("T")      PORT_CODE(KEYCODE_T)            PORT_CHAR('t')  PORT_CHAR('T')
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5")      PORT_CODE(KEYCODE_5)            PORT_CHAR('5')  PORT_CHAR('%')
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_UNUSED

	PORT_START("LINE5")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L")      PORT_CODE(KEYCODE_L)            PORT_CHAR('l')  PORT_CHAR('L')
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P")      PORT_CODE(KEYCODE_P)            PORT_CHAR('p')  PORT_CHAR('P')
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("LEFT")   PORT_CODE(KEYCODE_LEFT)         PORT_CHAR(UCHAR_MAMEKEY(LEFT))
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B")      PORT_CODE(KEYCODE_B)            PORT_CHAR('b')  PORT_CHAR('B')
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G")      PORT_CODE(KEYCODE_G)            PORT_CHAR('g')  PORT_CHAR('G')
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Y")      PORT_CODE(KEYCODE_Y)            PORT_CHAR('y')  PORT_CHAR('Y')
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6")      PORT_CODE(KEYCODE_6)            PORT_CHAR('6')  PORT_CHAR('^')
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_UNUSED

	PORT_START("LINE6")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_UNUSED
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0")      PORT_CODE(KEYCODE_0)            PORT_CHAR('0')  PORT_CHAR(')')
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("RIGHT")  PORT_CODE(KEYCODE_RIGHT)        PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("N")      PORT_CODE(KEYCODE_N)            PORT_CHAR('n')  PORT_CHAR('N')
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("H")      PORT_CODE(KEYCODE_H)            PORT_CHAR('h')  PORT_CHAR('H')
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("U")      PORT_CODE(KEYCODE_U)            PORT_CHAR('u')  PORT_CHAR('U')
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7")      PORT_CODE(KEYCODE_7)            PORT_CHAR('7')  PORT_CHAR('+')
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_UNUSED

	PORT_START("LINE7")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("NUMLOck") PORT_CODE(KEYCODE_NUMLOCK)     PORT_CHAR(UCHAR_MAMEKEY(NUMLOCK))
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9")      PORT_CODE(KEYCODE_9)            PORT_CHAR('9')  PORT_CHAR('(')
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("DOWN")   PORT_CODE(KEYCODE_DOWN)         PORT_CHAR(UCHAR_MAMEKEY(DOWN))
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("M")      PORT_CODE(KEYCODE_M)            PORT_CHAR('m')  PORT_CHAR('M')
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("J")      PORT_CODE(KEYCODE_J)            PORT_CHAR('j')  PORT_CHAR('J')
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("I")      PORT_CODE(KEYCODE_I)            PORT_CHAR('i')  PORT_CHAR('I')
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8")      PORT_CODE(KEYCODE_8)            PORT_CHAR('8')  PORT_CHAR('*')
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_UNUSED
INPUT_PORTS_END

PALETTE_INIT_MEMBER(lcmate2_state, lcmate2)
{
	palette.set_pen_color(0, rgb_t(138, 146, 148));
	palette.set_pen_color(1, rgb_t(92, 83, 88));
}

void lcmate2_state::machine_start()
{
	membank("rombank")->configure_entries(0, 0x10, (UINT8*)memregion("maincpu")->base(), 0x4000);
}

static const gfx_layout lcmate2_charlayout =
{
	5, 8,   /* 5 x 8 characters */
	256,    /* 256 characters */
	1,  /* 1 bits per pixel */
	{ 0 },  /* no bitplanes */
	{ 3, 4, 5, 6, 7},
	{ 0, 8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8},
	8*8 /* 8 bytes */
};

static GFXDECODE_START( lcmate2 )
	GFXDECODE_ENTRY( "hd44780:cgrom", 0x0000, lcmate2_charlayout, 0, 1 )
GFXDECODE_END


static MACHINE_CONFIG_START( lcmate2, lcmate2_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_3_579545MHz) // confirmed
	MCFG_CPU_PROGRAM_MAP(lcmate2_mem)
	MCFG_CPU_IO_MAP(lcmate2_io)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", LCD)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_UPDATE_DEVICE("hd44780", hd44780_device, screen_update)
	MCFG_SCREEN_SIZE(120, 18)
	MCFG_SCREEN_VISIBLE_AREA(0, 120-1, 0, 18-1)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 2)
	MCFG_PALETTE_INIT_OWNER(lcmate2_state, lcmate2)
	MCFG_DEFAULT_LAYOUT(layout_lcd)
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", lcmate2)

	MCFG_HD44780_ADD("hd44780")
	MCFG_HD44780_LCD_SIZE(2, 20)

	MCFG_NVRAM_ADD_0FILL("nvram")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	/* Devices */
	MCFG_DEVICE_ADD("rtc", RP5C15, XTAL_32_768kHz)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( lcmate2 )
	ROM_REGION( 0x40000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "u2.bin",  0x00000, 0x08000, CRC(521931b9) SHA1(743a6e2928c4365fbf5ed9a173e2c1bfe695850f) )
	ROM_LOAD( "u3.bin",  0x20000, 0x20000, CRC(84fe767a) SHA1(8dd306f203e1220f0eab1a284be3095e2642c5b6) ) // spell library
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT     COMPANY   FULLNAME       FLAGS */
COMP( 1984, lcmate2,  0,       0,   lcmate2,    lcmate2, driver_device,  0,   "Vtech",   "Laser Compumate 2", MACHINE_NOT_WORKING )
