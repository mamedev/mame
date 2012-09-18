/*************************************************************************************************


    Kaypro 2/83 computer - the very first Kaypro II - 2 full size floppy drives.
    Each disk was single sided, and could hold 191k. The computer had 2x pio
    and 1x sio. One of the sio ports communicated with the keyboard with a coiled
    telephone cord, complete with modular plug on each end. The keyboard carries
    its own Intel 8748 processor and is an intelligent device.

    Things that need doing:

    - MAME's z80sio implementation is unusable at this time. Needs proper callback
      mechanism for both channels, and interface to modern standards.

    - When z80sio gets updated, then see about getting keyboard to work as a serial device.

    - Kaypro2x/4a are not booting.

    - Hard Disk not emulated.
      The controller is a WD1002 (original version, for Winchester drives).

    - Kaypro 4 plus 88 does work as a normal Kaypro, but the extra processor needs
      to be worked out.

    - RTC type MM58167A to be added. Modem chips TMS99531, TMS99532 to be developed.

    - Regression all models: Cannot read the floppy disk any more since someone modified the
      WD controller.

**************************************************************************************************/

#include "includes/kaypro.h"


READ8_MEMBER( kaypro_state::kaypro2x_87_r ) { return 0x7f; }	/* to bypass unemulated HD controller */

/***********************************************************

    Address Maps

************************************************************/

static ADDRESS_MAP_START( kaypro_map, AS_PROGRAM, 8, kaypro_state )
	AM_RANGE(0x0000, 0x0fff) AM_ROM AM_REGION("maincpu", 0x0000)
	AM_RANGE(0x3000, 0x3fff) AM_RAM AM_REGION("maincpu", 0x3000) AM_SHARE("p_videoram")
	AM_RANGE(0x4000, 0xffff) AM_RAM AM_REGION("rambank", 0x4000)
ADDRESS_MAP_END

static ADDRESS_MAP_START( kayproii_io, AS_IO, 8, kaypro_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00, 0x03) AM_DEVWRITE("brg", com8116_device, stt_w)
	AM_RANGE(0x04, 0x07) AM_DEVREADWRITE_LEGACY("z80sio", kaypro_sio_r, kaypro_sio_w)
	AM_RANGE(0x08, 0x0b) AM_DEVREADWRITE("z80pio_g", z80pio_device, read_alt, write_alt)
	AM_RANGE(0x0c, 0x0f) AM_DEVWRITE("brg", com8116_device, str_w)
	AM_RANGE(0x10, 0x13) AM_DEVREADWRITE_LEGACY("wd1793", wd17xx_r, wd17xx_w)
	AM_RANGE(0x1c, 0x1f) AM_DEVREADWRITE("z80pio_s", z80pio_device, read_alt, write_alt)
ADDRESS_MAP_END

static ADDRESS_MAP_START( kaypro2x_io, AS_IO, 8, kaypro_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00, 0x03) AM_DEVWRITE("brg", com8116_device, str_w)
	AM_RANGE(0x04, 0x07) AM_DEVREADWRITE_LEGACY("z80sio", kaypro_sio_r, kaypro_sio_w)
	AM_RANGE(0x08, 0x0b) AM_DEVWRITE("brg", com8116_device, stt_w)
	AM_RANGE(0x0c, 0x0f) AM_DEVREADWRITE("z80sio_2x", z80sio_device, read, write)
	AM_RANGE(0x10, 0x13) AM_DEVREADWRITE_LEGACY("wd1793", wd17xx_r, wd17xx_w)
	AM_RANGE(0x14, 0x17) AM_READWRITE(kaypro2x_system_port_r,kaypro2x_system_port_w)
	AM_RANGE(0x18, 0x1b) AM_DEVWRITE("centronics", centronics_device, write)
	AM_RANGE(0x1c, 0x1c) AM_READWRITE(kaypro2x_status_r,kaypro2x_index_w)
	AM_RANGE(0x1d, 0x1d) AM_DEVREAD("crtc", mc6845_device, register_r) AM_WRITE(kaypro2x_register_w)
	AM_RANGE(0x1f, 0x1f) AM_READWRITE(kaypro2x_videoram_r,kaypro2x_videoram_w)

	/* The below are not emulated */
/*  AM_RANGE(0x20, 0x23) AM_DEVREADWRITE("z80pio", kaypro2x_pio_r, kaypro2x_pio_w) - for RTC and Modem
    AM_RANGE(0x24, 0x27) communicate with MM58167A RTC. Modem uses TMS99531 and TMS99532 chips.
    AM_RANGE(0x80, 0x80) Hard drive controller card I/O port - 10MB hard drive only fitted to the Kaypro 10
    AM_RANGE(0x81, 0x81) Hard Drive READ error register, WRITE precomp
    AM_RANGE(0x82, 0x82) Hard Drive Sector register count I/O
    AM_RANGE(0x83, 0x83) Hard Drive Sector register number I/O
    AM_RANGE(0x84, 0x84) Hard Drive Cylinder low register I/O
    AM_RANGE(0x85, 0x85) Hard Drive Cylinder high register I/O
    AM_RANGE(0x86, 0x86) Hard Drive Size / Drive / Head register I/O
    AM_RANGE(0x87, 0x87) Hard Drive READ status register, WRITE command register */
	AM_RANGE(0x20, 0x86) AM_NOP
	AM_RANGE(0x87, 0x87) AM_READ(kaypro2x_87_r)
ADDRESS_MAP_END


/***************************************************************

    F4 CHARACTER DISPLAYER

****************************************************************/
static const gfx_layout kayproii_charlayout =
{
	8, 8,					/* 8 x 8 characters */
	256,					/* 256 characters */
	1,					/* 1 bits per pixel */
	{ 0 },					/* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8					/* every char takes 8 bytes */
};

static const gfx_layout kaypro2x_charlayout =
{
	8, 16,					/* 8 x 16 characters */
	256,					/* 256 characters */
	1,					/* 1 bits per pixel */
	{ 0 },					/* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	8*16					/* every char takes 16 bytes */
};

static GFXDECODE_START( kayproii )
	GFXDECODE_ENTRY( "chargen", 0x0000, kayproii_charlayout, 0, 1 )
GFXDECODE_END

static GFXDECODE_START( kaypro2x )
	GFXDECODE_ENTRY( "chargen", 0x0000, kaypro2x_charlayout, 0, 1 )
GFXDECODE_END

/***************************************************************

    Interfaces

****************************************************************/

static const z80_daisy_config kayproii_daisy_chain[] =
{
	{ "z80sio" },		/* sio */
	{ "z80pio_s" },		/* System pio */
	{ "z80pio_g" },		/* General purpose pio */
	{ NULL }
};

static const z80_daisy_config kaypro2x_daisy_chain[] =
{
	{ "z80sio" },		/* sio for RS232C and keyboard */
	{ "z80sio_2x" },	/* sio for serial printer and inbuilt modem */
	{ NULL }
};

static const mc6845_interface kaypro2x_crtc = {
	"screen",			/* name of screen */
	7,				/* number of dots per character */
	NULL,
	kaypro2x_update_row,		/* handler to display a scanline */
	NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	NULL
};

//static WRITE_LINE_DEVICE_HANDLER( rx_tx_w )
//{
//  downcast<z80sio_device *>(device)->rx_clock_in();
//  downcast<z80sio_device *>(device)->tx_clock_in();
//}

static COM8116_INTERFACE( kayproii_brg_intf )
{
	DEVCB_NULL,		/* fX/4 output */
	DEVCB_NULL, //  DEVCB_DEVICE_LINE("z80sio", rx_tx_a_w), z80sio implementation has no clock pin
	DEVCB_NULL, // DEVCB_DEVICE_LINE("z80sio", rx_tx_b_w),
	{ 101376, 67584, 46080, 37686, 33792, 16896, 8448, 4224, 2816, 2534, 2112, 1408, 1056, 704, 528, 264 },			/* receiver divisor ROM */
	{ 101376, 67584, 46080, 37686, 33792, 16896, 8448, 4224, 2816, 2534, 2112, 1408, 1056, 704, 528, 264 },			/* transmitter divisor ROM */
};

static COM8116_INTERFACE( kaypro2x_brg_intf )
{
	DEVCB_NULL,		/* fX/4 output */
	DEVCB_NULL,//DEVCB_DEVICE_LINE("z80sio", rx_tx_a_w),
	DEVCB_NULL,//DEVCB_DEVICE_LINE("z80sio_2x", rx_tx_a_w),
	{ 101376, 67584, 46080, 37686, 33792, 16896, 8448, 4224, 2816, 2534, 2112, 1408, 1056, 704, 528, 264 },			/* receiver divisor ROM */
	{ 101376, 67584, 46080, 37686, 33792, 16896, 8448, 4224, 2816, 2534, 2112, 1408, 1056, 704, 528, 264 },			/* transmitter divisor ROM */
};


/***********************************************************

    Machine Driver

************************************************************/
static LEGACY_FLOPPY_OPTIONS_START(kayproii)
	LEGACY_FLOPPY_OPTION(kayproii, "dsk", "Kaypro II disk image", basicdsk_identify_default, basicdsk_construct_default, NULL,
		HEADS([1])
		TRACKS([40])
		SECTORS([10])
		SECTOR_LENGTH([512])
		FIRST_SECTOR_ID([0]))
LEGACY_FLOPPY_OPTIONS_END

static LEGACY_FLOPPY_OPTIONS_START(kaypro2x)
	LEGACY_FLOPPY_OPTION(kaypro2x, "dsk", "Kaypro 2x disk image", basicdsk_identify_default, basicdsk_construct_default, NULL,
		HEADS([2])
		TRACKS([80])
		SECTORS([10])
		SECTOR_LENGTH([512])
		FIRST_SECTOR_ID([0]))
LEGACY_FLOPPY_OPTIONS_END

static const floppy_interface kayproii_floppy_interface =
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	FLOPPY_STANDARD_5_25_DSHD,
	LEGACY_FLOPPY_OPTIONS_NAME(kayproii),
	NULL,
	NULL
};

static const floppy_interface kaypro2x_floppy_interface =
{
	DEVCB_LINE(wd17xx_idx_w),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	FLOPPY_STANDARD_5_25_DSHD,
	LEGACY_FLOPPY_OPTIONS_NAME(kaypro2x),
	NULL,
	NULL
};

static MACHINE_CONFIG_START( kayproii, kaypro_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_2_5MHz)
	MCFG_CPU_PROGRAM_MAP(kaypro_map)
	MCFG_CPU_IO_MAP(kayproii_io)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", kaypro_state,  kay_kbd_interrupt)	/* this doesn't actually exist, it is to run the keyboard */
	MCFG_CPU_CONFIG(kayproii_daisy_chain)

	MCFG_MACHINE_START_OVERRIDE(kaypro_state, kayproii )
	MCFG_MACHINE_RESET_OVERRIDE(kaypro_state, kayproii )

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(80*7, 24*10)
	MCFG_SCREEN_VISIBLE_AREA(0,80*7-1,0,24*10-1)
	MCFG_VIDEO_START_OVERRIDE(kaypro_state, kaypro )
	MCFG_SCREEN_UPDATE_DRIVER(kaypro_state, screen_update_kayproii)
	MCFG_GFXDECODE(kayproii)
	MCFG_PALETTE_LENGTH(2)
	MCFG_PALETTE_INIT(monochrome_green)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(BEEPER_TAG, BEEP, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	/* devices */
	MCFG_QUICKLOAD_ADD("quickload", kayproii, "com,cpm", 3)
	MCFG_FD1793_ADD("wd1793", kaypro_wd1793_interface )
	MCFG_CENTRONICS_PRINTER_ADD("centronics", standard_centronics)
	MCFG_COM8116_ADD("brg", XTAL_5_0688MHz, kayproii_brg_intf)	// WD1943, SMC8116
	MCFG_Z80PIO_ADD( "z80pio_g", 2500000, kayproii_pio_g_intf )
	MCFG_Z80PIO_ADD( "z80pio_s", 2500000, kayproii_pio_s_intf )
	MCFG_Z80SIO_ADD( "z80sio", 4800, kaypro_sio_intf )	/* start at 300 baud */

	MCFG_LEGACY_FLOPPY_2_DRIVES_ADD(kayproii_floppy_interface)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( kaypro4, kayproii )
	MCFG_DEVICE_REMOVE("z80pio_s")
	MCFG_Z80PIO_ADD( "z80pio_s", 2500000, kaypro4_pio_s_intf )
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( kaypro2x, kaypro_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_4MHz)
	MCFG_CPU_PROGRAM_MAP(kaypro_map)
	MCFG_CPU_IO_MAP(kaypro2x_io)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", kaypro_state,  kay_kbd_interrupt)
	MCFG_CPU_CONFIG(kaypro2x_daisy_chain)

	MCFG_MACHINE_RESET_OVERRIDE(kaypro_state, kaypro2x )

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(80*8, 25*16)
	MCFG_SCREEN_VISIBLE_AREA(0,80*8-1,0,25*16-1)
	MCFG_VIDEO_START_OVERRIDE(kaypro_state, kaypro )
	MCFG_SCREEN_UPDATE_DRIVER(kaypro_state, screen_update_kaypro2x)
	MCFG_GFXDECODE(kaypro2x)
	MCFG_PALETTE_LENGTH(3)
	MCFG_PALETTE_INIT_OVERRIDE(kaypro_state,kaypro)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(BEEPER_TAG, BEEP, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	/* devices */
	MCFG_MC6845_ADD("crtc", MC6845, 2000000, kaypro2x_crtc) /* comes out of ULA - needs to be measured */
	MCFG_QUICKLOAD_ADD("quickload", kaypro2x, "com,cpm", 3)
	MCFG_FD1793_ADD("wd1793", kaypro_wd1793_interface )
	MCFG_CENTRONICS_PRINTER_ADD("centronics", standard_centronics)
	MCFG_COM8116_ADD("brg", XTAL_5_0688MHz, kaypro2x_brg_intf)	// WD1943, SMC8116
	MCFG_Z80SIO_ADD( "z80sio", 4800, kaypro_sio_intf )
	MCFG_Z80SIO_ADD( "z80sio_2x", 4800, kaypro_sio_intf )	/* extra sio for modem and printer */

	MCFG_LEGACY_FLOPPY_2_DRIVES_ADD(kaypro2x_floppy_interface)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( omni2, kaypro4 )
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(kaypro_state, screen_update_omni2)
MACHINE_CONFIG_END

/***********************************************************

    Game driver

************************************************************/

/* The detested bios "universal rom" is part number 81-478 */

ROM_START(kayproii)
	/* The board could take a 2716 or 2732 */
	ROM_REGION(0x4000, "maincpu",0)
	ROM_LOAD("81-149.u47",   0x0000, 0x0800, CRC(28264bc1) SHA1(a12afb11a538fc0217e569bc29633d5270dfa51b) )

	ROM_REGION(0x10000, "rambank",0)
	ROM_FILL( 0, 0x10000, 0xff)

	ROM_REGION(0x0800, "chargen", ROMREGION_INVERT)
	ROM_LOAD("81-146.u43",   0x0000, 0x0800, CRC(4cc7d206) SHA1(5cb880083b94bd8220aac1f87d537db7cfeb9013) )
ROM_END

ROM_START(kaypro4)
	ROM_REGION(0x4000, "maincpu",0)
	ROM_LOAD("81-232.u47",   0x0000, 0x1000, CRC(4918fb91) SHA1(cd9f45cc3546bcaad7254b92c5d501c40e2ef0b2) )

	ROM_REGION(0x10000, "rambank",0)
	ROM_FILL( 0, 0x10000, 0xff)

	ROM_REGION(0x0800, "chargen", ROMREGION_INVERT)
	ROM_LOAD("81-146.u43",   0x0000, 0x0800, CRC(4cc7d206) SHA1(5cb880083b94bd8220aac1f87d537db7cfeb9013) )
ROM_END

ROM_START(kaypro4p88) // "KAYPRO-88" board has 128k or 256k of its own ram on it
	ROM_REGION(0x4000, "maincpu",0)
	ROM_LOAD("81-232.u47",   0x0000, 0x1000, CRC(4918fb91) SHA1(cd9f45cc3546bcaad7254b92c5d501c40e2ef0b2) )

	ROM_REGION(0x10000, "rambank",0)
	ROM_FILL( 0, 0x10000, 0xff)

	ROM_REGION(0x0800, "chargen", ROMREGION_INVERT)
	ROM_LOAD("81-146.u43",   0x0000, 0x0800, CRC(4cc7d206) SHA1(5cb880083b94bd8220aac1f87d537db7cfeb9013) )

	ROM_REGION(0x1000, "8088cpu",0)
	ROM_LOAD("81-356.u29",   0x0000, 0x1000, CRC(948556db) SHA1(6e779866d099cc0dc8c6369bdfb37a923ac448a4) )
ROM_END

ROM_START(omni2)
	ROM_REGION(0x4000, "maincpu",0)
	ROM_LOAD("omni2.u47",    0x0000, 0x1000, CRC(2883f9e0) SHA1(d98c784e62853582d298bf7ca84c75872847ac9b) )

	ROM_REGION(0x10000, "rambank",0)
	ROM_FILL( 0, 0x10000, 0xff)

	ROM_REGION(0x0800, "chargen", ROMREGION_INVERT)
	ROM_LOAD("omni2.u43",    0x0000, 0x0800, CRC(049b3381) SHA1(46f1d4f038747ba9048b075dc617361be088f82a) )
ROM_END

ROM_START(kaypro2x)
	ROM_REGION(0x4000, "maincpu",0)
	ROM_LOAD("81-292.u34",   0x0000, 0x2000, CRC(5eb69aec) SHA1(525f955ca002976e2e30ac7ee37e4a54f279fe96) )

	ROM_REGION(0x10000, "rambank",0)
	ROM_FILL( 0, 0x10000, 0xff)

	ROM_REGION(0x1000, "chargen",0)
	ROM_LOAD("81-817.u9",    0x0000, 0x1000, CRC(5f72da5b) SHA1(8a597000cce1a7e184abfb7bebcb564c6bf24fb7) )
ROM_END

ROM_START(kaypro4a) // same as kaypro2x ??
	ROM_REGION(0x4000, "maincpu",0)
	ROM_LOAD("81-292.u34",   0x0000, 0x2000, CRC(5eb69aec) SHA1(525f955ca002976e2e30ac7ee37e4a54f279fe96) )

	ROM_REGION(0x10000, "rambank",0)
	ROM_FILL( 0, 0x10000, 0xff)

	ROM_REGION(0x1000, "chargen",0)
	ROM_LOAD("81-817.u9",    0x0000, 0x1000, CRC(5f72da5b) SHA1(8a597000cce1a7e184abfb7bebcb564c6bf24fb7) )
ROM_END

ROM_START(kaypro10)
	ROM_REGION(0x4000, "maincpu",0)
	ROM_LOAD("81-302.u42",   0x0000, 0x1000, CRC(3f9bee20) SHA1(b29114a199e70afe46511119b77a662e97b093a0) )
//Note: the older rom 81-187 is also allowed here for kaypro 10, but we don't have a dump of it yet.
	ROM_REGION(0x10000, "rambank",0)
	ROM_FILL( 0, 0x10000, 0xff)

	ROM_REGION(0x1000, "chargen",0)
	ROM_LOAD("81-817.u31",   0x0000, 0x1000, CRC(5f72da5b) SHA1(8a597000cce1a7e184abfb7bebcb564c6bf24fb7) )
ROM_END

/*    YEAR  NAME      PARENT    COMPAT  MACHINE   INPUT    INIT         COMPANY                 FULLNAME */
COMP( 1982, kayproii,   0,        0,    kayproii, kay_kbd, driver_device, 0,      "Non Linear Systems",  "Kaypro II - 2/83" , 0 )
COMP( 1983, kaypro4,    kayproii, 0,    kaypro4,  kay_kbd, driver_device, 0,      "Non Linear Systems",  "Kaypro 4 - 4/83" , 0 ) // model 81-004
COMP( 1983, kaypro4p88, kayproii, 0,    kaypro4,  kay_kbd, driver_device, 0,      "Non Linear Systems",  "Kaypro 4 plus88 - 4/83" , GAME_NOT_WORKING ) // model 81-004 with an added 8088 daughterboard and rom
COMP( 198?, omni2,      kayproii, 0,    omni2,    kay_kbd, driver_device, 0,      "Non Linear Systems",  "Omni II" , 0 )
COMP( 1984, kaypro2x,   0,        0,    kaypro2x, kay_kbd, driver_device, 0,      "Non Linear Systems",  "Kaypro 2x" , GAME_NOT_WORKING ) // model 81-025
COMP( 1984, kaypro4a,   kaypro2x, 0,    kaypro2x, kay_kbd, driver_device, 0,      "Non Linear Systems",  "Kaypro 4 - 4/84" , GAME_NOT_WORKING ) // model 81-015
// Kaypro 4/84 plus 88 goes here, model 81-015 with an added 8088 daughterboard and rom
COMP( 1983, kaypro10,   0,        0,    kaypro2x, kay_kbd, driver_device, 0,      "Non Linear Systems",  "Kaypro 10" , GAME_NOT_WORKING ) // model 81-005
