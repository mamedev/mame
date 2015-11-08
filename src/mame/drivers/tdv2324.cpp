// license:BSD-3-Clause
// copyright-holders:Curt Coder,Jonathan Gevaryahu
/*

    Tandberg TDV2324

    Skeleton driver
    By Curt Coder with some work by Lord Nightmare

    Status:
    * Main cpu is currently hacked to read i/o port 0xE6 as 0x10;
      it then seems to copy code to a ram area then jumps there
      (there may be some sort of overlay/banking mess going on to allow full 64kb of ram)
      The cpu gets stuck reading i/o port 0x30 in a loop.
      - interrupts and sio lines are not hooked up
    * Sub cpu does a bunch of unknown i/o accesses and also tries to
      sequentially read chunk of address space which it never writes to;
      this seems likely to be a shared ram or i/o mapped area especially since it seems
      to write to i/o port 0x60 before trying to read there.
      - interrupts and sio lines are not hooked up
    * Fdc cpu starts, does a rom checksum (which passes) and tests a ram area


    Board Notes:
    Mainboard (pictures P1010036 & P1010038)
    *28-pin: D27128, L4267096S,...(eprom, occluded by sticker: "965268 1", character set)
    *40-pin: TMS9937NL, DB 336, ENGLAND (VTAC Video Chip)
    *40-pin: P8085AH-2, F4265030, C INTEL '80 (cpus, there are 2 of these)
    *28-pin: JAPAN 8442, 00009SS0, HN4827128G-25 (eprom, sticker: "962107")
    *22-pin: ER3400, GI 8401HHA (EAROM)
    *  -pin: MOSTEK C 8424, MK3887N-4 (Z80 Serial I/O Controller)
    *20-pin: (pal, sticker: "961420 0")
    *24-pin: D2716, L3263271, INTEL '77 (eprom, sticker: "962058 1")
    *3 tiny 16-pins which look socketed (proms)
    *+B8412, DMPAL10L8NC
    *PAL... (can't remove the sticker to read the rest since there's electrical components soldered above the chip)
    *Am27S21DC, 835IDmm
    *AM27S13DC, 8402DM (x2)
    *TBP28L22N, GERMANY 406 A (x2)
    *PAL16L6CNS, 8406

    FD/HD Interface Board P/N 962013 Rev14 (pictures P1010031 & P1010033)
    *28-pin: TMS, 2764JL-25, GHP8414 (@U15, labeled "962014 // -4-", fdc cpu rom)
    *40-pin: MC68B02P, R1H 8340 (fdc cpu)
    *40-pin: WDC '79, FD1797PL-02, 8342 16 (fdc chip)
    *14-pin: MC4024P, MG 8341 (dual voltage controlled multivibrator)
    *24-pin: TMM2016AP-12 (@U14 and @U80, 120ns 2kx8 SRAM)

    Keyboard:
    *40-pin: NEC D8035LC (mcs-48 cpu)
    *24-pin: NEC D2716 (eprom)

    Main CPU:
    - PIT, SIO

    Sub CPU:
    - PIC, PIT, VTAC

*/
/*
'subcpu' (17CD): unmapped i/o memory write to 23 = 36 & FF
'subcpu' (17D1): unmapped i/o memory write to 23 = 76 & FF
'subcpu' (17D5): unmapped i/o memory write to 23 = B6 & FF
'subcpu' (17DB): unmapped i/o memory write to 20 = 1A & FF
'subcpu' (17DE): unmapped i/o memory write to 20 = 00 & FF
'subcpu' (17E0): unmapped i/o memory write to 3E = 00 & FF
'subcpu' (17E2): unmapped i/o memory write to 3A = 00 & FF
'subcpu' (17E6): unmapped i/o memory write to 30 = 74 & FF
'subcpu' (17EA): unmapped i/o memory write to 31 = 7F & FF
'subcpu' (17EE): unmapped i/o memory write to 32 = 6D & FF
'subcpu' (17F2): unmapped i/o memory write to 33 = 18 & FF
'subcpu' (17F6): unmapped i/o memory write to 34 = 49 & FF
'subcpu' (17FA): unmapped i/o memory write to 35 = 20 & FF
'subcpu' (17FE): unmapped i/o memory write to 36 = 18 & FF
'subcpu' (1801): unmapped i/o memory write to 3C = 00 & FF
'subcpu' (1803): unmapped i/o memory write to 3C = 00 & FF
'subcpu' (1805): unmapped i/o memory write to 3E = 00 & FF
'subcpu' (0884): unmapped i/o memory write to 10 = 97 & FF
'subcpu' (0888): unmapped i/o memory write to 10 = 96 & FF

'fdccpu' (E004): unmapped program memory read from 3C05 & FF  0011 1100 0000 0101
'fdccpu' (E007): unmapped program memory read from C000 & FF  1100 0000 0000 0000
'fdccpu' (E00A): unmapped program memory read from A000 & FF  1010 0000 0000 0000
'fdccpu' (E012): unmapped program memory write to F000 = D0 & FF 1111 0000 0000 0000 = 1101 0000
'fdccpu' (E015): unmapped program memory read from 3801 & FF  0011 1000 0000 0001
'fdccpu' (E018): unmapped program memory read from 3C06 & FF  0011 1100 0000 0110
'fdccpu' (E01B): unmapped program memory read from 3C04 & FF  0011 1100 0000 0100

'fdccpu' (E070): unmapped program memory write to 2101 = 01 & FF
'fdccpu' (E07C): unmapped program memory read from 6000 & FF
'fdccpu' (E07F): unmapped program memory read from 380D & FF
'fdccpu' (E082): unmapped program memory read from 380F & FF
'fdccpu' (E085): unmapped program memory read from 3803 & FF
'fdccpu' (E08B): unmapped program memory write to 6000 = 08 & FF
'fdccpu' (E08E): unmapped program memory write to 8000 = 08 & FF
'fdccpu' (E091): unmapped program memory write to 6000 = 00 & FF
'fdccpu' (E099): unmapped program memory write to F800 = 55 & FF
...
*/



#include "includes/tdv2324.h"
#include "softlist.h"

READ8_MEMBER( tdv2324_state::tdv2324_main_io_30 )
{
	return 0xff;
}

// Not sure what this is for, i/o read at 0xE6 on maincpu, post fails if it does not return bit 4 set
READ8_MEMBER( tdv2324_state::tdv2324_main_io_e6 )
{
	return 0x10; // TODO: this should actually return something meaningful, for now is enough to pass early boot test
}

WRITE8_MEMBER( tdv2324_state::tdv2324_main_io_e2 )
{
	printf("%c\n", data);
}


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

//-------------------------------------------------
//  ADDRESS_MAP( tdv2324_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( tdv2324_mem, AS_PROGRAM, 8, tdv2324_state )
	AM_RANGE(0x0000, 0x07ff) AM_MIRROR(0x0800) AM_ROM AM_REGION(P8085AH_0_TAG, 0)
	/* when copying code to 4000 area it runs right off the end of rom;
	 * I'm not sure if its supposed to mirror or read as open bus */
//  AM_RANGE(0x4000, 0x5fff) AM_RAM // 0x4000 has the boot code copied to it, 5fff and down are the stack
//  AM_RANGE(0x6000, 0x6fff) AM_RAM // used by the relocated boot code; shared?
	AM_RANGE(0x0800, 0xffff) AM_RAM
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( tdv2324_io )
//-------------------------------------------------

static ADDRESS_MAP_START( tdv2324_io, AS_IO, 8, tdv2324_state )
	//ADDRESS_MAP_GLOBAL_MASK(0xff)
	/* 0x30 is read by main code and if high bit isn't set at some point it will never get anywhere */
	/* e0, e2, e8, ea are written to */
	/* 30, e6 and e2 are readable */
	AM_RANGE(0x30, 0x30) AM_READ(tdv2324_main_io_30)
//  AM_RANGE(0xe2, 0xe2) AM_WRITE(tdv2324_main_io_e2) console output
	AM_RANGE(0xe6, 0xe6) AM_READ(tdv2324_main_io_e6)
//  AM_RANGE(0x, 0x) AM_DEVREADWRITE(P8253_5_0_TAG, pit8253_device, read, write)
//  AM_RANGE(0x, 0x) AM_DEVREADWRITE(MK3887N4_TAG, z80dart_device, ba_cd_r, ba_cd_w)
//  AM_RANGE(0x, 0x) AM_DEVREADWRITE(P8259A_TAG, pic8259_device, read, write)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( tdv2324_sub_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( tdv2324_sub_mem, AS_PROGRAM, 8, tdv2324_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM AM_REGION(P8085AH_1_TAG, 0)
	AM_RANGE(0x4000, 0x47ff) AM_RAM
	AM_RANGE(0x5000, 0x53ff) AM_RAM // EAROM
	AM_RANGE(0x6000, 0x7fff) AM_RAM AM_SHARE("video_ram")
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( tdv2324_sub_io )
//-------------------------------------------------

static ADDRESS_MAP_START( tdv2324_sub_io, AS_IO, 8, tdv2324_state )
	//ADDRESS_MAP_GLOBAL_MASK(0xff)
	/* 20, 23, 30-36, 38, 3a, 3c, 3e, 60, 70 are written to */
	AM_RANGE(0x20, 0x23) AM_DEVREADWRITE(P8253_5_1_TAG, pit8253_device, read, write)
	AM_RANGE(0x30, 0x3f) AM_DEVREADWRITE(TMS9937NL_TAG, tms9927_device, read, write) // TODO: this is supposed to be a 9937, which is not quite the same as 9927
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( tdv2324_fdc_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( tdv2324_fdc_mem, AS_PROGRAM, 8, tdv2324_state )
	// the following two are probably enabled/disabled via the JP2 jumper block next to the fdc cpu
	//AM_RANGE(0x0000, 0x001f) AM_RAM // on-6802-die ram (optionally battery backed)
	//AM_RANGE(0x0020, 0x007f) AM_RAM // on-6802-die ram
	AM_RANGE(0x0000, 0x07ff) AM_RAM // TMM2016AP-12 @ U14, tested with A5,5A pattern
	//AM_RANGE(0x1000, 0x17ff) AM_RAM // TMM2016AP-12 @ U80, address is wrong
	// the 3xxx area appears to be closely involved in fdc or other i/o
	// in particular, reads from 30xx, 38xx, 3Cxx may be actually writes to certain fdc registers with data xx?
	// 0x2101 is something writable
	// 0x8000 is either a read from reg 0 (status reg) of the FD1797, OR a read from some sort of status from other cpus
	// 0x8000 can also be written to
	// 0x6000 can also be read from and written to
	// Somewhere in here, the FDC chip and the hard disk interface live
	AM_RANGE(0xe000, 0xffff) AM_ROM AM_REGION(MC68B02P_TAG, 0) // rom "962014 // -4-" @U15
ADDRESS_MAP_END



//**************************************************************************
//  INPUT PORTS
//**************************************************************************

//-------------------------------------------------
//  INPUT_PORTS( tdv2324 )
//-------------------------------------------------

static INPUT_PORTS_START( tdv2324 )
INPUT_PORTS_END



//**************************************************************************
//  VIDEO
//**************************************************************************

void tdv2324_state::video_start()
{
}


UINT32 tdv2324_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  SLOT_INTERFACE( tdv2324_floppies )
//-------------------------------------------------

static SLOT_INTERFACE_START( tdv2324_floppies )
	SLOT_INTERFACE( "8dsdd", FLOPPY_8_DSDD )
SLOT_INTERFACE_END



//**************************************************************************
//  MACHINE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  MACHINE_CONFIG( tdv2324 )
//-------------------------------------------------

static MACHINE_CONFIG_START( tdv2324, tdv2324_state )
	// basic system hardware
	MCFG_CPU_ADD(P8085AH_0_TAG, I8085A, 8700000/2) // ???
	MCFG_CPU_PROGRAM_MAP(tdv2324_mem)
	MCFG_CPU_IO_MAP(tdv2324_io)

	MCFG_CPU_ADD(P8085AH_1_TAG, I8085A, 8000000/2) // ???
	MCFG_CPU_PROGRAM_MAP(tdv2324_sub_mem)
	MCFG_CPU_IO_MAP(tdv2324_sub_io)

	MCFG_CPU_ADD(MC68B02P_TAG, M6802, 8000000/2) // ???
	MCFG_CPU_PROGRAM_MAP(tdv2324_fdc_mem)

	// video hardware
	MCFG_SCREEN_ADD(SCREEN_TAG, RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_UPDATE_DRIVER(tdv2324_state, screen_update)
	MCFG_SCREEN_SIZE(800, 400)
	MCFG_SCREEN_VISIBLE_AREA(0, 800-1, 0, 400-1)

	MCFG_PALETTE_ADD_MONOCHROME_GREEN("palette")

	MCFG_DEVICE_ADD(TMS9937NL_TAG, TMS9927, XTAL_25_39836MHz)
	MCFG_TMS9927_CHAR_WIDTH(8)

	// devices
	MCFG_PIC8259_ADD(P8259A_TAG, NULL, VCC, NULL)

	MCFG_DEVICE_ADD(P8253_5_0_TAG, PIT8253, 0)

	MCFG_DEVICE_ADD(P8253_5_1_TAG, PIT8253, 0)

	MCFG_Z80SIO2_ADD(MK3887N4_TAG, 8000000/2, 0, 0, 0, 0)

	MCFG_FD1797_ADD(FD1797PL02_TAG, 8000000/4)
	MCFG_FLOPPY_DRIVE_ADD(FD1797PL02_TAG":0", tdv2324_floppies, "8dsdd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(FD1797PL02_TAG":1", tdv2324_floppies, "8dsdd", floppy_image_device::default_floppy_formats)

	// internal ram
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("64K")

	// software list
	MCFG_SOFTWARE_LIST_ADD("flop_list", "tdv2324")
MACHINE_CONFIG_END



//**************************************************************************
//  ROMS
//**************************************************************************

//-------------------------------------------------
//  ROM( tdv2324 )
//-------------------------------------------------

ROM_START( tdv2324 )
	ROM_REGION( 0x800, P8085AH_0_TAG, 0 )
	ROM_LOAD( "962058-1.21g", 0x000, 0x800, CRC(3771aece) SHA1(36d3f03235f327d6c8682e5c167aed6dddfaa6ec) )

	ROM_REGION( 0x4000, P8085AH_1_TAG, 0 )
	ROM_LOAD( "962107-1.12c", 0x0000, 0x4000, CRC(29c1a139) SHA1(f55fa9075fdbfa6a3e94e5120270179f754d0ea5) )

	ROM_REGION( 0x2000, MC68B02P_TAG, 0 )
	ROM_LOAD( "962014-4.13c", 0x0000, 0x2000, CRC(d01c32cd) SHA1(1f00f5f5ff0c035eec6af820b5acb6d0c207b6db) )

	ROM_REGION( 0x800, "keyboard_8035", 0 )
	ROM_LOAD( "961294-3.u8", 0x000, 0x800, NO_DUMP )

	ROM_REGION( 0x4000, "chargen", 0 )
	ROM_LOAD( "965268-1.4g", 0x0000, 0x4000, CRC(7222a85f) SHA1(e94074b68d90698734ab1fc38d156407846df47c) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "961487-1.3f", 0x0000, 0x0200, NO_DUMP )
	ROM_LOAD( "prom.4f", 0x0000, 0x0200, NO_DUMP )
	ROM_LOAD( "prom.8g", 0x0000, 0x0200, NO_DUMP )
	ROM_LOAD( "prom.10f", 0x0000, 0x0200, NO_DUMP )
	ROM_LOAD( "961420-0.16f", 0x0000, 0x0200, NO_DUMP )
	ROM_LOAD( "962103-0.15g", 0x0000, 0x0200, NO_DUMP )

	ROM_REGION( 0x200, "plds", 0 )
	ROM_LOAD( "962108-0.2g", 0x0000, 0x0200, NO_DUMP )
	ROM_LOAD( "pal.12d", 0x0000, 0x0200, NO_DUMP )
	ROM_LOAD( "pal.13d", 0x0000, 0x0200, NO_DUMP )
ROM_END



//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     INIT  COMPANY     FULLNAME     FLAGS
COMP( 1983, tdv2324,        0,      0,      tdv2324,        tdv2324, driver_device,     0,      "Tandberg",     "TDV 2324",     MACHINE_NOT_WORKING|MACHINE_NO_SOUND)
