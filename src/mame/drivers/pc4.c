// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/***************************************************************************

    VTech Laser PC4

    Driver by Sandro Ronco

    I found no documentations about this, if you have documentation or schematic
    please let me know.

    Notes:
    - the LCD controller seems similar to a HD44780, but with more ddram and
      uses the bit 0 and 2 of the "Function Set" (0x2?) that are unused in
      the specifications of the HD44780 datasheet.
    - INT and NMI interrupt lines cause "LOW BATTERY" signal
    - chargen are taken from other driver, a real dump is required

    More info:
        http://www.8bit-micro.com/laser.htm
        http://www.euskalnet.net/ingepal/images/Vtech_Laser_PC4_1988.jpg
        http://www.oldcomputermuseum.com/laser_pc4.html

****************************************************************************/


#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/rp5c01.h"
#include "rendlay.h"
#include "includes/pc4.h"


READ8_MEMBER( pc4_state::kb_r )
{
	UINT8 data = 0xff;

	for (int line=0; line<8; line++)
	{
		if (!(offset & (1<<line)))
		{
			data &= io_port[line]->read();
		}
	}

	return data;
}

WRITE8_MEMBER( pc4_state::bank_w )
{
	//printf("set bank %x\n", data);
	m_rombank->set_entry(data&0x07);
}

WRITE8_MEMBER( pc4_state::beep_w )
{
	m_beep->set_state(data&0x40);
}

static ADDRESS_MAP_START(pc4_mem, AS_PROGRAM, 8, pc4_state)
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x7fff) AM_ROMBANK("rombank")
	AM_RANGE(0x8000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START(pc4_io, AS_IO, 8, pc4_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x000f) AM_DEVREADWRITE("rtc", rp5c01_device, read, write)
	AM_RANGE(0x1000, 0x1000) AM_WRITE(beep_w)
	AM_RANGE(0x1fff, 0x1fff) AM_WRITE(bank_w)

	AM_RANGE(0x3000, 0x3000) AM_WRITE(lcd_control_w)
	AM_RANGE(0x3001, 0x3001) AM_WRITE(lcd_data_w)
	AM_RANGE(0x3002, 0x3002) AM_READ(lcd_control_r)
	AM_RANGE(0x3003, 0x3003) AM_READ(lcd_data_r)
	AM_RANGE(0x3005, 0x3005) AM_WRITE(lcd_offset_w)

	//keyboard read, offset used as matrix
	AM_RANGE(0x5000, 0x50ff) AM_READ(kb_r)
ADDRESS_MAP_END

static INPUT_PORTS_START( pc4 )
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

PALETTE_INIT_MEMBER(pc4_state, pc4)
{
	palette.set_pen_color(0, rgb_t(138, 146, 148));
	palette.set_pen_color(1, rgb_t(92, 83, 88));
}

static const gfx_layout pc4_charlayout =
{
	5, 8,                   /* 5 x 8 characters */
	256,                    /* 256 characters */
	1,                      /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	{ 3, 4, 5, 6, 7},
	{ 0, 8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8},
	8*8                     /* 8 bytes */
};

static GFXDECODE_START( pc4 )
	GFXDECODE_ENTRY( "charset", 0x0000, pc4_charlayout, 0, 1 )
GFXDECODE_END

void pc4_state::machine_start()
{
	static const char *const bitnames[] = {"LINE0", "LINE1", "LINE2", "LINE3", "LINE4", "LINE5", "LINE6", "LINE7"};
	UINT8* rom_base = (UINT8 *)memregion("maincpu")->base();

	m_rombank->configure_entries(0, 8, rom_base, 0x4000);
	m_rombank->set_entry(0);

	m_busy_timer = timer_alloc(BUSY_TIMER);
	m_blink_timer = timer_alloc(BLINKING_TIMER);
	m_blink_timer->adjust(attotime::from_msec(409), 0, attotime::from_msec(409));

	for (int i=0; i<8; i++)
	{
		io_port[i] = ioport(bitnames[i]);
	}

	m_ac = 0;
	m_ac_mode = 0;
	m_data_bus_flag = 0;
	m_cursor_pos = 0;
	m_display_on = 0;
	m_cursor_on = 0;
	m_shift_on = 0;
	m_blink_on = 0;
	m_direction = 1;
	m_disp_shift = 0;
	m_blink = 0;
}

static MACHINE_CONFIG_START( pc4, pc4_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_4MHz)
	MCFG_CPU_PROGRAM_MAP(pc4_mem)
	MCFG_CPU_IO_MAP(pc4_io)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", LCD)
	MCFG_SCREEN_REFRESH_RATE(72)
	MCFG_SCREEN_UPDATE_DRIVER(pc4_state, screen_update)
	MCFG_SCREEN_SIZE(240, 36)
	MCFG_SCREEN_VISIBLE_AREA(0, 240-1, 0, 36-1)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_DEFAULT_LAYOUT(layout_lcd)
	MCFG_PALETTE_ADD("palette", 2)
	MCFG_PALETTE_INIT_OWNER(pc4_state, pc4)
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", pc4)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO( "mono" )
	MCFG_SOUND_ADD( "beeper", BEEP, 0 )
	MCFG_SOUND_ROUTE( ALL_OUTPUTS, "mono", 1.00 )

	MCFG_DEVICE_ADD("rtc", RP5C01, XTAL_32_768kHz)
MACHINE_CONFIG_END

ROM_START( pc4 )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "laser pc4 v4.14a.u2", 0x00000, 0x20000, CRC(f8dabf5d) SHA1(6988517b3ccb42df2b8d6e1517ff04b24458d146) )
	ROM_REGION( 0x0860, "charset", ROMREGION_ERASE )
	ROM_LOAD( "44780a00.bin",    0x0000, 0x0860,  BAD_DUMP CRC(3a89024c) SHA1(5a87b68422a916d1b37b5be1f7ad0b3fb3af5a8d))
ROM_END

/*    YEAR  NAME        PARENT      COMPAT  MACHINE     INPUT   INIT      COMPANY                     FULLNAME                FLAGS */
COMP( 1990, pc4,       0,          0,      pc4,  pc4, driver_device, 0,      "Laser Computer",   "Laser PC4",                       MACHINE_NOT_WORKING )
