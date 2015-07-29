// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    PROF-180X

    07/07/2009 Skeleton driver.

****************************************************************************/

/*

    TODO:

    - HD64180 CPU (DMA, MMU, SIO, etc) (Z80180 like)
    - memory banking
    - keyboard
    - floppy
    - floppy motor off timer
    - floppy index callback
    - RTC

*/


#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/ram.h"
#include "bus/centronics/ctronics.h"
#include "machine/upd765.h"
#include "includes/prof180x.h"

UINT32 prof180x_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}


READ8_MEMBER( prof180x_state::read )
{
	UINT8 data = 0;

	if (offset < 0x40000)
	{
	}
	else
	{
	}
/*
    switch ((m_mm1 << 1) | m_mm0)
    {
    case 0:
        // bank0_r = EPROM, bank0_w = RAM, bank1 = RAM
        break;

    case 1:
        // bank0_r = RAM, bank0_w = RAM, bank1 = RAM
        break;

    case 2:
        // bank0_r = UNMAP, bank0_w = UNMAP, bank1 = RAM
        break;

    case 3:
        // bank0_r = RAM, bank0_w = RAM, bank1 = UNMAP
        break;
    }
*/
	return data;
}

WRITE8_MEMBER( prof180x_state::write )
{
	if (offset < 0x40000)
	{
	}
	else
	{
	}
}

void prof180x_state::ls259_w(int flag, int value)
{
	switch (flag)
	{
	case 0: // C0
		m_c0 = value;
		break;

	case 1: // C1
		m_c1 = value;
		break;

	case 2: // C2
		m_c2 = value;
		break;

	case 3: // MINI
		break;

	case 4: // MM0
		m_mm0 = value;
		break;

	case 5: // RTC
		break;

	case 6: // PEPS
		break;

	case 7: // MM1
		m_mm1 = value;
		break;
	}
}

WRITE8_MEMBER( prof180x_state::flr_w )
{
	/*

	    bit     description

	    0       VAL
	    1       FLG0
	    2       FLG1
	    3       FLG2
	    4
	    5
	    6
	    7

	*/

	int val = BIT(data, 0);
	int flg = (data >> 1) & 0x07;

	ls259_w(flg, val);
}

READ8_MEMBER( prof180x_state::status0_r )
{
	/*

	    bit     description

	    0       BUSY
	    1
	    2
	    3
	    4       B-E
	    5       IDX
	    6
	    7       MOT

	*/

	return 0;
}

READ8_MEMBER( prof180x_state::status1_r )
{
	/*

	    bit     description

	    0       FREE
	    1
	    2
	    3
	    4       J18
	    5       J19
	    6
	    7       TDO

	*/

	return 0;
}

READ8_MEMBER( prof180x_state::status_r )
{
	return BIT(offset, 8) ? status1_r(space, offset) : status0_r(space, offset);
}

/* Address Maps */

static ADDRESS_MAP_START( prof180x_mem, AS_PROGRAM, 8, prof180x_state )
	AM_RANGE(0x00000, 0x7ffff) AM_READWRITE(read, write)
ADDRESS_MAP_END

static ADDRESS_MAP_START( prof180x_io , AS_IO, 8, prof180x_state )
	AM_RANGE(0x08, 0x08) AM_MIRROR(0xff00) AM_WRITE(flr_w)
	AM_RANGE(0x09, 0x09) AM_MASK(0xff00) AM_READ(status_r)
	AM_RANGE(0x0a, 0x0a) AM_MIRROR(0xff00) AM_DEVREADWRITE(FDC9268_TAG, upd765a_device, mdma_r, mdma_w)
	AM_RANGE(0x0b, 0x0b) AM_MIRROR(0xff00) AM_DEVWRITE("cent_data_out", output_latch_device, write)
	AM_RANGE(0x0c, 0x0d) AM_MIRROR(0xff00) AM_DEVICE(FDC9268_TAG, upd765a_device, map)
ADDRESS_MAP_END

/* Input ports */

static INPUT_PORTS_START( prof180x )
INPUT_PORTS_END

/* Video */

static SLOT_INTERFACE_START( prof180x_floppies )
	SLOT_INTERFACE( "35dd", FLOPPY_35_DD )
SLOT_INTERFACE_END

/*
static RTC8583_INTERFACE( rtc_intf )
{
    DEVCB_CPU_INPUT_LINE(HD64180_TAG, INPUT_LINE_INT2)
};
*/

void prof180x_state::machine_start()
{
	// register for state saving
	save_item(NAME(m_c0));
	save_item(NAME(m_c1));
	save_item(NAME(m_c2));
	save_item(NAME(m_mm0));
	save_item(NAME(m_mm1));
}

void prof180x_state::machine_reset()
{
	for (int i = 0; i < 8; i++)
	{
		ls259_w(i, 0);
	}
}

static MACHINE_CONFIG_START( prof180x, prof180x_state )
	/* basic machine hardware */
	MCFG_CPU_ADD(HD64180_TAG, Z80, XTAL_9_216MHz)
	MCFG_CPU_PROGRAM_MAP(prof180x_mem)
	MCFG_CPU_IO_MAP(prof180x_io)

	/* video hardware */
	MCFG_SCREEN_ADD(SCREEN_TAG, RASTER)
	MCFG_SCREEN_UPDATE_DRIVER(prof180x_state, screen_update)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 480-1)

	/* devices */
	MCFG_UPD765A_ADD(FDC9268_TAG, false, true)
	MCFG_FLOPPY_DRIVE_ADD(FDC9268_TAG ":0", prof180x_floppies, "35dd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(FDC9268_TAG ":1", prof180x_floppies, "35dd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(FDC9268_TAG ":2", prof180x_floppies, "35dd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(FDC9268_TAG ":3", prof180x_floppies, "35dd", floppy_image_device::default_floppy_formats)

	//MCFG_RTC8583_ADD(MK3835_TAG, rtc_intf)
	MCFG_CENTRONICS_ADD(CENTRONICS_TAG, centronics_devices, "printer")

	MCFG_CENTRONICS_OUTPUT_LATCH_ADD("cent_data_out", CENTRONICS_TAG)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("128K")
	MCFG_RAM_EXTRA_OPTIONS("256K,512K")

	/* software lists */
	MCFG_SOFTWARE_LIST_ADD("flop_list", "prof180")
MACHINE_CONFIG_END

/* ROM definition */

ROM_START( prof180x )
	ROM_REGION( 0x10000, HD64180_TAG, 0 )
	ROM_SYSTEM_BIOS( 0, "pmon13", "pmon v1.3" )
	ROMX_LOAD( "pmon1_3.z16", 0x00000, 0x04000, CRC(32986688) SHA1(a6229d7e66ef699722ca3d41179fe3f1b75185d4), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "pmon14", "pmon v1.4" )
	ROMX_LOAD( "pmon1_4.z16", 0x00000, 0x04000, CRC(ed03f49f) SHA1(e016f9e0b89ab64c6203e2e46501d0b09f74ee9b), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 2, "pmon15", "pmon v1.5" )
	ROMX_LOAD( "pmon1_5.z16", 0x00000, 0x04000, CRC(f43d185c) SHA1(a7a219b3d48c74602b3116cfcd34e44d6e7bc423), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS( 3, "pmon", "pmon" )
	ROMX_LOAD( "pmon.z16",    0x00000, 0x04000, CRC(4f3732d7) SHA1(7dc27262db4e0c8f109470253b9364a216909f2c), ROM_BIOS(4) )
	ROM_SYSTEM_BIOS( 4, "eboot1", "eboot1" )
	ROMX_LOAD( "eboot1.z16",  0x00000, 0x08000, CRC(7a164b3c) SHA1(69367804b5cbc0633e3d7bbbcc256c2c8c9e7aca), ROM_BIOS(5) )
	ROM_SYSTEM_BIOS( 5, "eboot2", "eboot2" )
	ROMX_LOAD( "eboot2.z16",  0x00000, 0x08000, CRC(0c2d4301) SHA1(f1a4f457e287b19e14d8ccdbc0383f183d8a3efe), ROM_BIOS(6) )
	ROM_SYSTEM_BIOS( 6, "epmon1", "epmon1" )
	ROMX_LOAD( "epmon1.z16",  0x00000, 0x10000, CRC(27aabfb4) SHA1(41adf038c474596dbf7d387a1a7f33ed86aa7869), ROM_BIOS(7) )
	ROM_SYSTEM_BIOS( 7, "epmon2", "epmon2" )
	ROMX_LOAD( "epmon2.z16",  0x00000, 0x10000, CRC(3b8a7b59) SHA1(33741f0725e3eaa21c6881c712579b3c1fd30607), ROM_BIOS(8) )
	ROM_SYSTEM_BIOS( 8, "epmon3", "epmon3" )
	ROMX_LOAD( "epmon3.z16",  0x00000, 0x10000, CRC(51313af1) SHA1(60c293171a1c7cb9a5ff6d681e61894f44fddbd1), ROM_BIOS(9) )

	ROM_REGION( 0x157, "plds", 0 )
	ROM_LOAD( "pal14l8.z10", 0x000, 0x157, NO_DUMP )
ROM_END

ROM_START( prof181x )
	ROM_REGION( 0x10000, HD64180_TAG, 0 )
	ROM_SYSTEM_BIOS( 0, "pmon13", "pmon v1.3" )
	ROMX_LOAD( "pmon1_3.u13", 0x00000, 0x04000, CRC(32986688) SHA1(a6229d7e66ef699722ca3d41179fe3f1b75185d4), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "pmon14", "pmon v1.4" )
	ROMX_LOAD( "pmon1_4.u13", 0x00000, 0x04000, CRC(ed03f49f) SHA1(e016f9e0b89ab64c6203e2e46501d0b09f74ee9b), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 2, "pmon15", "pmon v1.5" )
	ROMX_LOAD( "pmon1_5.u13", 0x00000, 0x04000, CRC(f43d185c) SHA1(a7a219b3d48c74602b3116cfcd34e44d6e7bc423), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS( 3, "pmon", "pmon" )
	ROMX_LOAD( "pmon.u13",    0x00000, 0x04000, CRC(4f3732d7) SHA1(7dc27262db4e0c8f109470253b9364a216909f2c), ROM_BIOS(4) )
	ROM_SYSTEM_BIOS( 4, "eboot1", "eboot1" )
	ROMX_LOAD( "eboot1.u13",  0x00000, 0x08000, CRC(7a164b3c) SHA1(69367804b5cbc0633e3d7bbbcc256c2c8c9e7aca), ROM_BIOS(5) )
	ROM_SYSTEM_BIOS( 5, "eboot2", "eboot2" )
	ROMX_LOAD( "eboot2.u13",  0x00000, 0x08000, CRC(0c2d4301) SHA1(f1a4f457e287b19e14d8ccdbc0383f183d8a3efe), ROM_BIOS(6) )
	ROM_SYSTEM_BIOS( 6, "epmon1", "epmon1" )
	ROMX_LOAD( "epmon1.u13",  0x00000, 0x10000, CRC(27aabfb4) SHA1(41adf038c474596dbf7d387a1a7f33ed86aa7869), ROM_BIOS(7) )
	ROM_SYSTEM_BIOS( 7, "epmon2", "epmon2" )
	ROMX_LOAD( "epmon2.u13",  0x00000, 0x10000, CRC(3b8a7b59) SHA1(33741f0725e3eaa21c6881c712579b3c1fd30607), ROM_BIOS(8) )
	ROM_SYSTEM_BIOS( 8, "epmon3", "epmon3" )
	ROMX_LOAD( "epmon3.u13",  0x00000, 0x10000, CRC(51313af1) SHA1(60c293171a1c7cb9a5ff6d681e61894f44fddbd1), ROM_BIOS(9) )

	ROM_REGION( 0x157, "plds", 0 ) // converted from JED files
	ROM_LOAD( "pal20v8.u14", 0x000, 0x157, CRC(46da52b0) SHA1(c11362223c0d5c57c6ef970e66d674b89d8e7784) )
	ROM_LOAD( "pal20v8.u15", 0x000, 0x157, CRC(19fef936) SHA1(579ad23ee3c0b1c64c584383f9c8085c6ce3d094) )
	ROM_LOAD( "pal20v8.u19", 0x000, 0x157, CRC(69348c3b) SHA1(6eb8432660eb9b639a95b1973a54dab8b99f10ef) )
	ROM_LOAD( "pal20v8.u21", 0x000, 0x157, CRC(6df4e281) SHA1(602fa4637cd9356acc31b2adfb3a084fd5a0bfcb) )
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT    COMPANY   FULLNAME       FLAGS */
COMP( 1986, prof180x,  0,       0,  prof180x,   prof180x, driver_device,     0,  "Conitec Datensysteme",   "PROF-180X",     MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
COMP( 1992, prof181x,  prof180x,0,  prof180x,   prof180x, driver_device,     0,  "Conitec Datensysteme",   "PROF-181X",     MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
