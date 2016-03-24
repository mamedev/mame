// license:BSD-3-Clause
// copyright-holders:Robbbert
/*************************************************************************************************


    Kaypro 2/83 computer - the very first Kaypro II - 2 full size floppy drives.
    Each disk was single sided, and could hold 191k. The computer had 2x pio
    and 1x sio. One of the sio ports communicated with the keyboard with a coiled
    telephone cord, complete with modular plug on each end. The keyboard carries
    its own Intel 87C51 processor and is an intelligent device.

    Kaypro 10 notes:
    - This machine comes with a 10MB hard drive, split into 2 5MB partitions. It also
      has one floppy drive. The drive letters change depending on what drive it was booted
      from. The boot drive is always A:.
      If booted from floppy:
      A: floppy
      B: HD partition 1
      C: HD partition 2
      If booted from HD (presumably partition 1)
      A: HD partition 1
      B: HD partition 2
      C: floppy

    ToDo:

    - See about getting keyboard to work as a serial device.
    - Need dump of 87C51 cpu in the keyboard.

    - Kaypro 2x, 4a: floppy not working "No operating system present on this disk"
    - Kaypro 10: Boots from floppy, but needs hard drive added.
    - Kaypro 4p88: works as a normal Kaypro 4, extra hardware not done
    - Kaypro Robie: has twin 2.6MB 5.25 floppy drives which we don't support, no software available

    - Hard Disk not emulated.
      The controller is a WD1002 (original version, for Winchester drives).

    - RTC type MM58167A to be added. Modem chips TMS99531, TMS99532 to be developed.

    - Once everything works, sort out parent and compat relationships.

**************************************************************************************************/

#include "includes/kaypro.h"
#include "formats/kaypro_dsk.h"
#include "softlist.h"

READ8_MEMBER( kaypro_state::kaypro2x_87_r ) { return 0x7f; }    /* to bypass unemulated HD controller */

/***********************************************************

    Address Maps

************************************************************/

static ADDRESS_MAP_START( kaypro_map, AS_PROGRAM, 8, kaypro_state )
	AM_RANGE(0x0000, 0x2fff) AM_READ_BANK("bankr0") AM_WRITE_BANK("bankw0")
	AM_RANGE(0x3000, 0x3fff) AM_RAMBANK("bank3")
	AM_RANGE(0x4000, 0xffff) AM_RAM AM_REGION("rambank", 0x4000)
ADDRESS_MAP_END

static ADDRESS_MAP_START( kayproii_io, AS_IO, 8, kaypro_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00, 0x03) AM_DEVWRITE("brg", com8116_device, stt_w)
	AM_RANGE(0x04, 0x07) AM_READWRITE(kaypro_sio_r, kaypro_sio_w)
	AM_RANGE(0x08, 0x0b) AM_DEVREADWRITE("z80pio_g", z80pio_device, read_alt, write_alt)
	AM_RANGE(0x0c, 0x0f) AM_DEVWRITE("brg", com8116_device, str_w)
	AM_RANGE(0x10, 0x13) AM_DEVREADWRITE("fdc", fd1793_t, read, write)
	AM_RANGE(0x1c, 0x1f) AM_DEVREADWRITE("z80pio_s", z80pio_device, read_alt, write_alt)
ADDRESS_MAP_END

static ADDRESS_MAP_START( kaypro2x_io, AS_IO, 8, kaypro_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00, 0x03) AM_DEVWRITE("brg", com8116_device, str_w)
	AM_RANGE(0x04, 0x07) AM_READWRITE(kaypro_sio_r, kaypro_sio_w)
	AM_RANGE(0x08, 0x0b) AM_DEVWRITE("brg", com8116_device, stt_w)
	AM_RANGE(0x0c, 0x0f) AM_DEVREADWRITE("z80sio_2x", z80sio0_device, ba_cd_r, ba_cd_w)
	AM_RANGE(0x10, 0x13) AM_DEVREADWRITE("fdc", fd1793_t, read, write)
	AM_RANGE(0x14, 0x17) AM_READWRITE(kaypro2x_system_port_r,kaypro2x_system_port_w)
	AM_RANGE(0x18, 0x1b) AM_DEVWRITE("cent_data_out", output_latch_device, write)
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
	8, 8,                   /* 8 x 8 characters */
	256,                    /* 256 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8                 /* every char takes 8 bytes */
};

static const gfx_layout kaypro2x_charlayout =
{
	8, 16,                  /* 8 x 16 characters */
	256,                    /* 256 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	8*16                    /* every char takes 16 bytes */
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
	{ "z80sio" },       /* sio */
	{ "z80pio_s" },     /* System pio */
	{ "z80pio_g" },     /* General purpose pio */
	{ nullptr }
};

static const z80_daisy_config kaypro2x_daisy_chain[] =
{
	{ "z80sio" },       /* sio for RS232C and keyboard */
	{ "z80sio_2x" },    /* sio for serial printer and inbuilt modem */
	{ nullptr }
};



//static WRITE_LINE_DEVICE_HANDLER( rx_tx_w )
//{
//  downcast<z80sio_device *>(device)->rx_clock_in();
//  downcast<z80sio_device *>(device)->tx_clock_in();
//}



/***********************************************************

    Machine Driver

************************************************************/

FLOPPY_FORMATS_MEMBER( kaypro_state::kayproii_floppy_formats )
	FLOPPY_KAYPROII_FORMAT
FLOPPY_FORMATS_END

FLOPPY_FORMATS_MEMBER( kaypro_state::kaypro2x_floppy_formats )
	FLOPPY_KAYPRO2X_FORMAT
FLOPPY_FORMATS_END

static SLOT_INTERFACE_START( kaypro_floppies )
	SLOT_INTERFACE( "drive0", FLOPPY_525_DD )
	SLOT_INTERFACE( "drive1", FLOPPY_525_DD )
SLOT_INTERFACE_END


static MACHINE_CONFIG_START( kayproii, kaypro_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_20MHz / 8)
	MCFG_CPU_PROGRAM_MAP(kaypro_map)
	MCFG_CPU_IO_MAP(kayproii_io)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", kaypro_state, kay_kbd_interrupt)  /* this doesn't actually exist, it is to run the keyboard */
	MCFG_CPU_CONFIG(kayproii_daisy_chain)

	MCFG_MACHINE_START_OVERRIDE(kaypro_state, kayproii )
	MCFG_MACHINE_RESET_OVERRIDE(kaypro_state, kaypro )

	/* video hardware */
	MCFG_SCREEN_ADD_MONOCHROME("screen", RASTER, rgb_t::green)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(80*7, 24*10)
	MCFG_SCREEN_VISIBLE_AREA(0,80*7-1,0,24*10-1)
	MCFG_VIDEO_START_OVERRIDE(kaypro_state, kaypro )
	MCFG_SCREEN_UPDATE_DRIVER(kaypro_state, screen_update_kayproii)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", kayproii)
	MCFG_PALETTE_ADD_MONOCHROME("palette")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("beeper", BEEP, 950) /* piezo-device needs to be measured */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	/* devices */
	MCFG_QUICKLOAD_ADD("quickload", kaypro_state, kaypro, "com,cpm", 3)

	MCFG_CENTRONICS_ADD("centronics", centronics_devices, "printer")
	MCFG_CENTRONICS_BUSY_HANDLER(WRITELINE(kaypro_state, write_centronics_busy))

	MCFG_CENTRONICS_OUTPUT_LATCH_ADD("cent_data_out", "centronics")

	MCFG_DEVICE_ADD("brg", COM8116, XTAL_5_0688MHz) // WD1943, SMC8116

	MCFG_DEVICE_ADD("z80pio_g", Z80PIO, XTAL_20MHz / 8)
	MCFG_Z80PIO_OUT_INT_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))
	MCFG_Z80PIO_OUT_PA_CB(DEVWRITE8("cent_data_out", output_latch_device, write))

	MCFG_DEVICE_ADD("z80pio_s", Z80PIO, XTAL_20MHz / 8)
	MCFG_Z80PIO_OUT_INT_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))
	MCFG_Z80PIO_IN_PA_CB(READ8(kaypro_state, pio_system_r))
	MCFG_Z80PIO_OUT_PA_CB(WRITE8(kaypro_state, kayproii_pio_system_w))

	MCFG_Z80SIO0_ADD("z80sio", XTAL_20MHz / 8, 0, 0, 0, 0)
	MCFG_Z80DART_OUT_INT_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))

	MCFG_FD1793_ADD("fdc", XTAL_20MHz / 20)
	MCFG_WD_FDC_INTRQ_CALLBACK(WRITELINE(kaypro_state, fdc_intrq_w))
	MCFG_WD_FDC_DRQ_CALLBACK(WRITELINE(kaypro_state, fdc_drq_w))
	MCFG_WD_FDC_FORCE_READY
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", kaypro_floppies, "drive0", kaypro_state::kayproii_floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", kaypro_floppies, "drive1", kaypro_state::kayproii_floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)
	MCFG_SOFTWARE_LIST_ADD("flop_list","kayproii")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( kaypro4, kayproii )
	MCFG_DEVICE_REMOVE("z80pio_s")
	MCFG_DEVICE_ADD("z80pio_s", Z80PIO, 2500000)
	MCFG_Z80PIO_OUT_INT_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))
	MCFG_Z80PIO_IN_PA_CB(READ8(kaypro_state, pio_system_r))
	MCFG_Z80PIO_OUT_PA_CB(WRITE8(kaypro_state, kaypro4_pio_system_w))
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( kaypro2x, kaypro_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_16MHz / 4)
	MCFG_CPU_PROGRAM_MAP(kaypro_map)
	MCFG_CPU_IO_MAP(kaypro2x_io)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", kaypro_state, kay_kbd_interrupt)
	MCFG_CPU_CONFIG(kaypro2x_daisy_chain)

	MCFG_MACHINE_RESET_OVERRIDE(kaypro_state, kaypro )

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(80*8, 25*16)
	MCFG_SCREEN_VISIBLE_AREA(0,80*8-1,0,25*16-1)
	MCFG_VIDEO_START_OVERRIDE(kaypro_state, kaypro )
	MCFG_SCREEN_UPDATE_DRIVER(kaypro_state, screen_update_kaypro2x)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", kaypro2x)
	MCFG_PALETTE_ADD("palette", 3)
	MCFG_PALETTE_INIT_OWNER(kaypro_state, kaypro)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("beeper", BEEP, 950) /* piezo-device needs to be measured */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	/* devices */
	MCFG_MC6845_ADD("crtc", MC6845, "screen", 2000000) /* comes out of ULA - needs to be measured */
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(7)
	MCFG_MC6845_UPDATE_ROW_CB(kaypro_state, kaypro2x_update_row)

	MCFG_QUICKLOAD_ADD("quickload", kaypro_state, kaypro, "com,cpm", 3)

	MCFG_CENTRONICS_ADD("centronics", centronics_devices, "printer")
	MCFG_CENTRONICS_BUSY_HANDLER(WRITELINE(kaypro_state, write_centronics_busy))

	MCFG_CENTRONICS_OUTPUT_LATCH_ADD("cent_data_out", "centronics")

	MCFG_Z80SIO0_ADD("z80sio", XTAL_16MHz / 4, 0, 0, 0, 0)
	MCFG_Z80DART_OUT_INT_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))
	MCFG_Z80SIO0_ADD("z80sio_2x", XTAL_16MHz / 4, 0, 0, 0, 0)   /* extra sio for modem and printer */
	MCFG_Z80DART_OUT_INT_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))
	MCFG_DEVICE_ADD("brg", COM8116, XTAL_5_0688MHz) // WD1943, SMC8116
	MCFG_FD1793_ADD("fdc", XTAL_16MHz / 16)
	MCFG_WD_FDC_INTRQ_CALLBACK(WRITELINE(kaypro_state, fdc_intrq_w))
	MCFG_WD_FDC_DRQ_CALLBACK(WRITELINE(kaypro_state, fdc_drq_w))
	MCFG_WD_FDC_FORCE_READY
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", kaypro_floppies, "drive0", kaypro_state::kaypro2x_floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", kaypro_floppies, "drive1", kaypro_state::kaypro2x_floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( kaypro10, kaypro2x )
	MCFG_DEVICE_REMOVE("fdc:1")  // only has 1 floppy drive
	// need to add hard drive & controller
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( omni2, kaypro4 )
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(kaypro_state, screen_update_omni2)
MACHINE_CONFIG_END

DRIVER_INIT_MEMBER( kaypro_state, kaypro )
{
	UINT8 *main = memregion("roms")->base();
	UINT8 *ram = memregion("rambank")->base();

	membank("bankr0")->configure_entry(1, &main[0x0000]);
	membank("bankr0")->configure_entry(0, &ram[0x0000]);
	membank("bank3")->configure_entry(1, &main[0x3000]);
	membank("bank3")->configure_entry(0, &ram[0x3000]);
	membank("bankw0")->configure_entry(0, &ram[0x0000]);
}


/***********************************************************

    Game driver

************************************************************/

/* The detested bios "universal rom" is part number 81-478 */

ROM_START(kayproii)
	/* The board could take a 2716 or 2732 */
	ROM_REGION(0x4000, "roms",0)
	ROM_SYSTEM_BIOS( 0, "149", "149")
	ROMX_LOAD("81-149.u47",   0x0000, 0x0800, CRC(28264bc1) SHA1(a12afb11a538fc0217e569bc29633d5270dfa51b), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "149b", "149B")
	ROMX_LOAD("81-149b.u47",  0x0000, 0x0800, CRC(c008549e) SHA1(b9346a16f5f9ffb6bb0eb1766c348b74056485a8), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 2, "149c", "149C")
	ROMX_LOAD("81-149c.u47",  0x0000, 0x0800, CRC(1272aa65) SHA1(027fee2f5f17ba71a4738f00188e132e326536ff), ROM_BIOS(3) )

	ROM_REGION(0x10000, "rambank", ROMREGION_ERASEFF)

	ROM_REGION(0x0800, "chargen", ROMREGION_INVERT)
	ROM_LOAD("81-146.u43",   0x0000, 0x0800, CRC(4cc7d206) SHA1(5cb880083b94bd8220aac1f87d537db7cfeb9013) )
ROM_END

ROM_START(kaypro4)
	ROM_REGION(0x4000, "roms",0)
	ROM_LOAD("81-232.u47",   0x0000, 0x1000, CRC(4918fb91) SHA1(cd9f45cc3546bcaad7254b92c5d501c40e2ef0b2) )

	ROM_REGION(0x10000, "rambank", ROMREGION_ERASEFF)

	ROM_REGION(0x0800, "chargen", ROMREGION_INVERT)
	ROM_LOAD("81-146.u43",   0x0000, 0x0800, CRC(4cc7d206) SHA1(5cb880083b94bd8220aac1f87d537db7cfeb9013) )
ROM_END

ROM_START(kaypro4p88) // "KAYPRO-88" board has 128k or 256k of its own ram on it
	ROM_REGION(0x4000, "roms",0)
	ROM_LOAD("81-232.u47",   0x0000, 0x1000, CRC(4918fb91) SHA1(cd9f45cc3546bcaad7254b92c5d501c40e2ef0b2) )

	ROM_REGION(0x10000, "rambank", ROMREGION_ERASEFF)

	ROM_REGION(0x0800, "chargen", ROMREGION_INVERT)
	ROM_LOAD("81-146.u43",   0x0000, 0x0800, CRC(4cc7d206) SHA1(5cb880083b94bd8220aac1f87d537db7cfeb9013) )

	ROM_REGION(0x1000, "8088cpu",0)
	ROM_LOAD("81-356.u29",   0x0000, 0x1000, CRC(948556db) SHA1(6e779866d099cc0dc8c6369bdfb37a923ac448a4) )
ROM_END

ROM_START(omni2)
	ROM_REGION(0x4000, "roms",0)
	ROM_LOAD("omni2.u47",    0x0000, 0x1000, CRC(2883f9e0) SHA1(d98c784e62853582d298bf7ca84c75872847ac9b) )

	ROM_REGION(0x10000, "rambank", ROMREGION_ERASEFF)

	ROM_REGION(0x0800, "chargen", ROMREGION_INVERT)
	ROM_LOAD("omni2.u43",    0x0000, 0x0800, CRC(049b3381) SHA1(46f1d4f038747ba9048b075dc617361be088f82a) )
ROM_END

ROM_START(kaypro2x)
	ROM_REGION(0x4000, "roms",0)
	ROM_SYSTEM_BIOS( 0, "292", "292")
	ROMX_LOAD("81-292.u34",   0x0000, 0x2000, CRC(5eb69aec) SHA1(525f955ca002976e2e30ac7ee37e4a54f279fe96), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "292a", "292A")
	ROMX_LOAD("81-292a.u34",  0x0000, 0x1000, CRC(241f27a5) SHA1(82711289d19e9b165e35324da010466d225e503a), ROM_BIOS(2) )

	ROM_REGION(0x10000, "rambank", ROMREGION_ERASEFF)

	ROM_REGION(0x1000, "chargen",0)
	ROM_LOAD("81-235.u9",    0x0000, 0x1000, CRC(5f72da5b) SHA1(8a597000cce1a7e184abfb7bebcb564c6bf24fb7) )
ROM_END

ROM_START(kaypro4a) // same as kaypro2x ??
	ROM_REGION(0x4000, "roms",0)
	ROM_LOAD("81-292.u34",   0x0000, 0x2000, CRC(5eb69aec) SHA1(525f955ca002976e2e30ac7ee37e4a54f279fe96) )

	ROM_REGION(0x10000, "rambank", ROMREGION_ERASEFF)

	ROM_REGION(0x1000, "chargen",0)
	ROM_LOAD("81-235.u9",    0x0000, 0x1000, CRC(5f72da5b) SHA1(8a597000cce1a7e184abfb7bebcb564c6bf24fb7) )
ROM_END

ROM_START(kaypro10)
	ROM_REGION(0x4000, "roms",0)
	ROM_SYSTEM_BIOS( 0, "302", "V1.9E")
	ROMX_LOAD("81-302.u42",   0x0000, 0x1000, CRC(3f9bee20) SHA1(b29114a199e70afe46511119b77a662e97b093a0), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "188", "V1.9")
	ROMX_LOAD("81-188.u42",   0x0000, 0x1000, CRC(6cbd6aa0) SHA1(47004f8c6e17407e4f8d613c9520f9316716d9e2), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 2, "277", "V1.9E(F)")
	ROMX_LOAD("81-277.u42",   0x0000, 0x1000, CRC(e4e1831f) SHA1(1de31ed532a461ace7a4abad1f6647eeddceb3e7), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS( 3, "478", "V2.01")
	ROMX_LOAD("81-478.u42",   0x0000, 0x2000, CRC(de618380) SHA1(c8d6312e6eeb62a53e741f1ff3b878bdcb7b5aaa), ROM_BIOS(4) )

	ROM_REGION(0x10000, "rambank", ROMREGION_ERASEFF)

	ROM_REGION(0x1000, "chargen",0)
	ROM_LOAD("81-187.u31",   0x0000, 0x1000, CRC(5f72da5b) SHA1(8a597000cce1a7e184abfb7bebcb564c6bf24fb7) )
ROM_END

ROM_START(robie)
	ROM_REGION(0x4000, "roms",0)
	ROM_SYSTEM_BIOS( 0, "326", "V1.7R")
	ROMX_LOAD("81-326.u34",   0x0000, 0x2000, CRC(7f0c3f68) SHA1(54b088a1b2200f9df4b9b347bbefb0115f3a4976), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "u", "V1.4")
	ROMX_LOAD("robie_u.u34",  0x0000, 0x2000, CRC(da7248b5) SHA1(1dc053b3e44ead47255cc166b7b4b0adaeb3dd3d), ROM_BIOS(2) ) // rom number unknown

	ROM_REGION(0x10000, "rambank", ROMREGION_ERASEFF)

	ROM_REGION(0x1000, "chargen",0)
	ROM_LOAD("81-235.u9",    0x0000, 0x1000, CRC(5f72da5b) SHA1(8a597000cce1a7e184abfb7bebcb564c6bf24fb7) )
ROM_END

/*    YEAR  NAME      PARENT    COMPAT  MACHINE   INPUT    CLASS         INIT         COMPANY                 FULLNAME */
COMP( 1982, kayproii,   0,        0,    kayproii, kay_kbd, kaypro_state, kaypro, "Non Linear Systems",  "Kaypro II - 2/83" , 0 )
COMP( 1983, kaypro4,    kayproii, 0,    kaypro4,  kay_kbd, kaypro_state, kaypro, "Non Linear Systems",  "Kaypro 4 - 4/83" , 0 ) // model 81-004
COMP( 1983, kaypro4p88, kayproii, 0,    kaypro4,  kay_kbd, kaypro_state, kaypro, "Non Linear Systems",  "Kaypro 4 plus88 - 4/83" , MACHINE_NOT_WORKING ) // model 81-004 with an added 8088 daughterboard and rom
COMP( 198?, omni2,      kayproii, 0,    omni2,    kay_kbd, kaypro_state, kaypro, "Non Linear Systems",  "Omni II Logic Analyzer" , 0 )
COMP( 1984, kaypro2x,   0,        0,    kaypro2x, kay_kbd, kaypro_state, kaypro, "Non Linear Systems",  "Kaypro 2x" , MACHINE_NOT_WORKING ) // model 81-025
COMP( 1984, kaypro4a,   kaypro2x, 0,    kaypro2x, kay_kbd, kaypro_state, kaypro, "Non Linear Systems",  "Kaypro 4 - 4/84" , MACHINE_NOT_WORKING ) // model 81-015
// Kaypro 4/84 plus 88 goes here, model 81-015 with an added 8088 daughterboard and rom
COMP( 1983, kaypro10,   0,        0,    kaypro10, kay_kbd, kaypro_state, kaypro, "Non Linear Systems",  "Kaypro 10" , MACHINE_NOT_WORKING ) // model 81-005
COMP( 1984, robie,      0,        0,    kaypro2x, kay_kbd, kaypro_state, kaypro, "Non Linear Systems",  "Kaypro Robie" , MACHINE_NOT_WORKING ) // model 81-005
