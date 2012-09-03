/************************************************************************
Nascom Memory map

    CPU: z80
        0000-03ff   ROM (Nascom1 Monitor)
        0400-07ff   ROM (Nascom2 Monitor extension)
        0800-0bff   RAM (Screen)
        0c00-0c7f   RAM (OS workspace)
        0c80-0cff   RAM (extended OS workspace)
        0d00-0f7f   RAM (Firmware workspace)
        0f80-0fff   RAM (Stack space)
        1000-8fff   RAM (User space)
        9000-97ff   RAM (Programmable graphics RAM/User space)
        9800-afff   RAM (Colour graphics RAM/User space)
        b000-b7ff   ROM (OS extensions)
        b800-bfff   ROM (WP/Naspen software)
        c000-cfff   ROM (Disassembler/colour graphics software)
        d000-dfff   ROM (Assembler/Basic extensions)
        e000-ffff   ROM (Nascom2 Basic)

    Interrupts:

    Ports:
        OUT (00)    0:  Increment keyboard scan
                1:  Reset keyboard scan
                2:
                3:  Read from cassette
                4:
                5:
                6:
                7:
        IN  (00)    Read keyboard
        OUT (01)    Write to cassette/serial
        IN  (01)    Read from cassette/serial
        OUT (02)    Unused
        IN  (02)    ?

    Monitors:
        Nasbug1     1K  Original Nascom1
        Nasbug2         1K
        Nasbug3     Probably non existing
        Nasbug4     2K
        Nassys1     2K  Original Nascom2
        Nassys2     Probably non existing
        Nassys3     2K
        Nassys4     2K
        T4      2K

************************************************************************/

/* Core includes */
#include "emu.h"
#include "includes/nascom1.h"

/* Components */
#include "cpu/z80/z80.h"
#include "machine/wd17xx.h"
#include "machine/ay31015.h"
#include "machine/z80pio.h"

/* Devices */
#include "imagedev/flopdrv.h"
#include "formats/basicdsk.h"
#include "imagedev/cartslot.h"
#include "imagedev/cassette.h"
#include "machine/ram.h"


/*************************************
 *
 *  Memory maps
 *
 *************************************/

static ADDRESS_MAP_START( nascom1_mem, AS_PROGRAM, 8, nascom1_state )
	AM_RANGE(0x0000, 0x07ff) AM_ROM
	AM_RANGE(0x0800, 0x0bff) AM_RAM AM_SHARE("videoram")
	AM_RANGE(0x0c00, 0x0fff) AM_RAM
	AM_RANGE(0x1000, 0x13ff) AM_RAM	/* 1Kb */
	AM_RANGE(0x1400, 0x4fff) AM_RAM	/* 16Kb */
	AM_RANGE(0x5000, 0x8fff) AM_RAM	/* 32Kb */
	AM_RANGE(0x9000, 0xafff) AM_RAM	/* 40Kb */
	AM_RANGE(0xb000, 0xffff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( nascom1_io, AS_IO, 8, nascom1_state )
	ADDRESS_MAP_GLOBAL_MASK(0x0F)
	AM_RANGE(0x00, 0x00) AM_READWRITE(nascom1_port_00_r, nascom1_port_00_w)
	AM_RANGE(0x01, 0x01) AM_READWRITE(nascom1_port_01_r, nascom1_port_01_w)
	AM_RANGE(0x02, 0x02) AM_READ(nascom1_port_02_r)
	AM_RANGE(0x04, 0x07) AM_DEVREADWRITE("z80pio", z80pio_device, read, write )
ADDRESS_MAP_END


static ADDRESS_MAP_START( nascom2_io, AS_IO, 8, nascom1_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READWRITE(nascom1_port_00_r, nascom1_port_00_w)
	AM_RANGE(0x01, 0x01) AM_READWRITE(nascom1_port_01_r, nascom1_port_01_w)
	AM_RANGE(0x02, 0x02) AM_READ(nascom1_port_02_r)
	AM_RANGE(0x04, 0x07) AM_DEVREADWRITE("z80pio", z80pio_device, read, write )
	AM_RANGE(0xe0, 0xe3) AM_DEVREADWRITE_LEGACY("wd1793", wd17xx_r, wd17xx_w)
	AM_RANGE(0xe4, 0xe4) AM_READWRITE(nascom2_fdc_select_r, nascom2_fdc_select_w)
	AM_RANGE(0xe5, 0xe5) AM_READ(nascom2_fdc_status_r)
ADDRESS_MAP_END



/*************************************
 *
 *  GFX layouts
 *
 *************************************/

static const gfx_layout nascom1_charlayout =
{
	8, 16,
	128,
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
	  8*8, 9*8,10*8,11*8,12*8,13*8,14*8,15*8 },
	8 * 16
};


static GFXDECODE_START( nascom1 )
	GFXDECODE_ENTRY("gfx1", 0x0000, nascom1_charlayout, 0, 1)
GFXDECODE_END


static const gfx_layout nascom2_charlayout =
{
	8, 14,
	256,
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8,  3*8,  4*8,  5*8,  6*8,
	  7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8 },
	8 * 16
};


static GFXDECODE_START( nascom2 )
	GFXDECODE_ENTRY("gfx1", 0x0000, nascom2_charlayout, 0, 1)
GFXDECODE_END



/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( nascom1 )
	PORT_START("KEY0")
	PORT_BIT(0x6f, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)

	PORT_START("KEY1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)  PORT_CHAR('H') PORT_CHAR('h')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)  PORT_CHAR('B') PORT_CHAR('b')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)  PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)  PORT_CHAR('F') PORT_CHAR('f')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)  PORT_CHAR('X') PORT_CHAR('x')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)  PORT_CHAR('T') PORT_CHAR('t')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))

	PORT_START("KEY2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)    PORT_CHAR('J') PORT_CHAR('j')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)    PORT_CHAR('N') PORT_CHAR('n')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)    PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)    PORT_CHAR('D') PORT_CHAR('d')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)    PORT_CHAR('Z') PORT_CHAR('z')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)    PORT_CHAR('Y') PORT_CHAR('y')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))

	PORT_START("KEY3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)    PORT_CHAR('K') PORT_CHAR('k')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)    PORT_CHAR('M') PORT_CHAR('m')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)    PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)    PORT_CHAR('E') PORT_CHAR('e')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)    PORT_CHAR('S') PORT_CHAR('s')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)    PORT_CHAR('U') PORT_CHAR('u')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))

	PORT_START("KEY4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)     PORT_CHAR('L') PORT_CHAR('l')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)     PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)     PORT_CHAR('W') PORT_CHAR('w')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)     PORT_CHAR('A') PORT_CHAR('a')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)     PORT_CHAR('I') PORT_CHAR('i')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))

	PORT_START("KEY5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)  PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)     PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)     PORT_CHAR('3') PORT_CHAR('\xA3')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)     PORT_CHAR('Q') PORT_CHAR('q')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)     PORT_CHAR('O') PORT_CHAR('o')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)     PORT_CHAR('0')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)     PORT_CHAR('2') PORT_CHAR('\"')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)     PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)     PORT_CHAR('P') PORT_CHAR('p')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)     PORT_CHAR('G') PORT_CHAR('g')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)     PORT_CHAR('V') PORT_CHAR('v')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)     PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)     PORT_CHAR('C') PORT_CHAR('c')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)     PORT_CHAR('R') PORT_CHAR('r')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Backspace ClearScreen") PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(8)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("New Line")              PORT_CODE(KEYCODE_ENTER)      PORT_CHAR(13)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT(0x58, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR(UCHAR_SHIFT_2) PORT_CHAR('@')
INPUT_PORTS_END

static INPUT_PORTS_START( nascom2 )
	PORT_INCLUDE(nascom1)

	PORT_MODIFY("KEY6")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD)                            PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('[') PORT_CHAR('\\')

	PORT_MODIFY("KEY7")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD)                            PORT_CODE(KEYCODE_BACKSPACE)  PORT_CHAR(']') PORT_CHAR('_')

	PORT_MODIFY("KEY8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Back CS")       PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(8)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Enter  Escape") PORT_CODE(KEYCODE_ENTER)      PORT_CHAR(13)  PORT_CHAR(27)
INPUT_PORTS_END


/*************************************
 *
 *  Machine drivers
 *
 *************************************/

static const ay31015_config nascom1_ay31015_config =
{
	AY_3_1015,
	( XTAL_16MHz / 16 ) / 256,
	( XTAL_16MHz / 16 ) / 256,
	nascom1_hd6402_si,
	nascom1_hd6402_so,
	NULL
};


static Z80PIO_INTERFACE( nascom1_z80pio_intf )
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};


static MACHINE_CONFIG_START( nascom1, nascom1_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_16MHz/8)
	MCFG_CPU_PROGRAM_MAP(nascom1_mem)
	MCFG_CPU_IO_MAP(nascom1_io)

	MCFG_MACHINE_RESET( nascom1 )

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(48 * 8, 16 * 16)
	MCFG_SCREEN_VISIBLE_AREA(0, 48 * 8 - 1, 0, 16 * 16 - 1)
	MCFG_SCREEN_UPDATE_STATIC(nascom1)

	MCFG_GFXDECODE(nascom1)
	MCFG_PALETTE_LENGTH(2)
	MCFG_PALETTE_INIT(black_and_white)

	MCFG_AY31015_ADD( "hd6402", nascom1_ay31015_config )

	MCFG_Z80PIO_ADD( "z80pio", XTAL_16MHz/8, nascom1_z80pio_intf )

	/* devices */
	MCFG_SNAPSHOT_ADD("snapshot", nascom1, "nas", 0.5)

	MCFG_CASSETTE_ADD( CASSETTE_TAG, default_cassette_interface )

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("40K")
	MCFG_RAM_EXTRA_OPTIONS("1K,16K,32K")
MACHINE_CONFIG_END

static LEGACY_FLOPPY_OPTIONS_START(nascom2)
	LEGACY_FLOPPY_OPTION(nascom2_ss, "dsk", "Nascom 2 SS disk image", basicdsk_identify_default, basicdsk_construct_default, NULL,
		HEADS([1])
		TRACKS([80])
		SECTORS([16])
		SECTOR_LENGTH([256])
		FIRST_SECTOR_ID([1]))
	LEGACY_FLOPPY_OPTION(nascom2_ds, "dsk", "Nascom 2 DS disk image", basicdsk_identify_default, basicdsk_construct_default, NULL,
		HEADS([2])
		TRACKS([80])
		SECTORS([16])
		SECTOR_LENGTH([256])
		FIRST_SECTOR_ID([1]))
LEGACY_FLOPPY_OPTIONS_END

static const floppy_interface nascom2_floppy_interface =
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	FLOPPY_STANDARD_5_25_DSHD,
	LEGACY_FLOPPY_OPTIONS_NAME(nascom2),
	NULL,
	NULL
};

static MACHINE_CONFIG_DERIVED( nascom2, nascom1 )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_IO_MAP(nascom2_io)

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_SIZE(48 * 8, 16 * 14)
	MCFG_SCREEN_VISIBLE_AREA(0, 48 * 8 - 1, 0, 16 * 14 - 1)
	MCFG_SCREEN_UPDATE_STATIC(nascom2)

	MCFG_GFXDECODE(nascom2)

	MCFG_FD1793_ADD("wd1793", nascom2_wd17xx_interface )

	MCFG_LEGACY_FLOPPY_4_DRIVES_ADD(nascom2_floppy_interface)
MACHINE_CONFIG_END



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START(nascom1)
	ROM_REGION(0x10000, "maincpu",0)
	ROM_SYSTEM_BIOS(0, "T4", "NasBug T4")
	ROMX_LOAD("nasbugt4.rom", 0x0000, 0x0800, CRC(f391df68) SHA1(00218652927afc6360c57e77d6a4fd32d4e34566), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "T1", "NasBug T1")
	ROMX_LOAD("nasbugt1.rom", 0x0000, 0x0400, CRC(8ea07054) SHA1(3f9a8632826003d6ea59d2418674d0fb09b83a4c), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(2, "T2", "NasBug T2")
	ROMX_LOAD("nasbugt2.rom", 0x0000, 0x0400, CRC(e371b58a) SHA1(485b20a560b587cf9bb4208ba203b12b3841689b), ROM_BIOS(3))
	ROM_REGION(0x0800, "gfx1", 0)
	ROM_LOAD("nascom1.chr",   0x0000, 0x0800, CRC(33e92a04) SHA1(be6e1cc80e7f95a032759f7df19a43c27ff93a52))
ROM_END


ROM_START(nascom2)
	ROM_REGION(0x10000, "maincpu",0)
	ROM_SYSTEM_BIOS( 0, "NS3", "NasSys 3")
	ROMX_LOAD("nassys3.rom", 0x0000, 0x0800, CRC(3da17373) SHA1(5fbda15765f04e4cd08cf95c8d82ce217889f240), ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 1, "NS1", "NasSys 1")
	ROMX_LOAD("nassys1.rom", 0x0000, 0x0800, CRC(b6300716) SHA1(29da7d462ba3f569f70ed3ecd93b981f81c7adfa), ROM_BIOS(2))
	ROM_LOAD("nasdos.rom",   0xd000, 0x1000, CRC(54a36f6d) SHA1(1d063d04be5024f128bd589e6edc066e9a63fc1b))
	ROM_LOAD("basic.rom",    0xe000, 0x2000, CRC(5cb5197b) SHA1(c41669c2b6d6dea808741a2738426d97bccc9b07))
	ROM_REGION(0x1000, "gfx1", 0)
	ROM_LOAD("nascom1.chr",  0x0000, 0x0800, CRC(33e92a04) SHA1(be6e1cc80e7f95a032759f7df19a43c27ff93a52))
	ROM_LOAD("nasgra.chr",   0x0800, 0x0800, CRC(2bc09d32) SHA1(d384297e9b02cbcb283c020da51b3032ff62b1ae))
ROM_END


/*************************************
 *
 *  Driver definitions
 *
 *************************************/

/*    YEAR  NAME        PARENT      COMPAT  MACHINE     INPUT       INIT       COMPANY                     FULLNAME        FLAGS */
COMP( 1978, nascom1,    0,          0,      nascom1,    nascom1, nascom1_state,    nascom1,   "Nascom Microcomputers",    "Nascom 1",     GAME_NO_SOUND )
COMP( 1979, nascom2,    nascom1,    0,      nascom2,    nascom2, nascom1_state,    nascom1,   "Nascom Microcomputers",    "Nascom 2",     GAME_NO_SOUND )
