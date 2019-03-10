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
#include "includes/prof180x.h"

#include "cpu/z80/z80.h"
#include "imagedev/floppy.h"
#include "machine/ram.h"
#include "machine/upd765.h"
#include "screen.h"
#include "softlist.h"

uint32_t prof180x_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}


READ8_MEMBER( prof180x_state::read )
{
	uint8_t data = 0;

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

void prof180x_state::prof180x_mem(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(prof180x_state::read), FUNC(prof180x_state::write));
}

void prof180x_state::prof180x_io(address_map &map)
{
	map(0x08, 0x08).mirror(0xff00).w(FUNC(prof180x_state::flr_w));
	map(0x09, 0x09).select(0xff00).r(FUNC(prof180x_state::status_r));
	map(0x0a, 0x0a).mirror(0xff00).rw(FDC9268_TAG, FUNC(upd765a_device::dma_r), FUNC(upd765a_device::dma_w));
	map(0x0b, 0x0b).mirror(0xff00).w("cent_data_out", FUNC(output_latch_device::bus_w));
	map(0x0c, 0x0d).mirror(0xff00).m(FDC9268_TAG, FUNC(upd765a_device::map));
}

/* Input ports */

static INPUT_PORTS_START( prof180x )
INPUT_PORTS_END

/* Video */

static void prof180x_floppies(device_slot_interface &device)
{
	device.option_add("35dd", FLOPPY_35_DD);
}

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

void prof180x_state::prof180x(machine_config &config)
{
	/* basic machine hardware */
	z80_device &maincpu(Z80(config, HD64180_TAG, XTAL(9'216'000)));
	maincpu.set_addrmap(AS_PROGRAM, &prof180x_state::prof180x_mem);
	maincpu.set_addrmap(AS_IO, &prof180x_state::prof180x_io);

	/* video hardware */
	screen_device &screen(SCREEN(config, SCREEN_TAG, SCREEN_TYPE_RASTER));
	screen.set_screen_update(FUNC(prof180x_state::screen_update));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(640, 480);
	screen.set_visarea(0, 640-1, 0, 480-1);

	/* devices */
	UPD765A(config, FDC9268_TAG, 8'000'000, false, true);
	FLOPPY_CONNECTOR(config, FDC9268_TAG ":0", prof180x_floppies, "35dd", floppy_image_device::default_floppy_formats);
	FLOPPY_CONNECTOR(config, FDC9268_TAG ":1", prof180x_floppies, "35dd", floppy_image_device::default_floppy_formats);
	FLOPPY_CONNECTOR(config, FDC9268_TAG ":2", prof180x_floppies, "35dd", floppy_image_device::default_floppy_formats);
	FLOPPY_CONNECTOR(config, FDC9268_TAG ":3", prof180x_floppies, "35dd", floppy_image_device::default_floppy_formats);

	//RTC8583(config, MK3835_TAG, rtc_intf);

	CENTRONICS(config, m_centronics, centronics_devices, "printer");

	output_latch_device &latch(OUTPUT_LATCH(config, "cent_data_out"));
	m_centronics->set_output_latch(latch);

	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("128K").set_extra_options("256K,512K");

	/* software lists */
	SOFTWARE_LIST(config, "flop_list").set_original("prof180");
}

/* ROM definition */

ROM_START( prof180x )
	ROM_REGION( 0x10000, HD64180_TAG, 0 )
	ROM_SYSTEM_BIOS( 0, "pmon13", "pmon v1.3" )
	ROMX_LOAD( "pmon1_3.z16", 0x00000, 0x04000, CRC(32986688) SHA1(a6229d7e66ef699722ca3d41179fe3f1b75185d4), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "pmon14", "pmon v1.4" )
	ROMX_LOAD( "pmon1_4.z16", 0x00000, 0x04000, CRC(ed03f49f) SHA1(e016f9e0b89ab64c6203e2e46501d0b09f74ee9b), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "pmon15", "pmon v1.5" )
	ROMX_LOAD( "pmon1_5.z16", 0x00000, 0x04000, CRC(f43d185c) SHA1(a7a219b3d48c74602b3116cfcd34e44d6e7bc423), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 3, "pmon", "pmon" )
	ROMX_LOAD( "pmon.z16",    0x00000, 0x04000, CRC(4f3732d7) SHA1(7dc27262db4e0c8f109470253b9364a216909f2c), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS( 4, "eboot1", "eboot1" )
	ROMX_LOAD( "eboot1.z16",  0x00000, 0x08000, CRC(7a164b3c) SHA1(69367804b5cbc0633e3d7bbbcc256c2c8c9e7aca), ROM_BIOS(4) )
	ROM_SYSTEM_BIOS( 5, "eboot2", "eboot2" )
	ROMX_LOAD( "eboot2.z16",  0x00000, 0x08000, CRC(0c2d4301) SHA1(f1a4f457e287b19e14d8ccdbc0383f183d8a3efe), ROM_BIOS(5) )
	ROM_SYSTEM_BIOS( 6, "epmon1", "epmon1" )
	ROMX_LOAD( "epmon1.z16",  0x00000, 0x10000, CRC(27aabfb4) SHA1(41adf038c474596dbf7d387a1a7f33ed86aa7869), ROM_BIOS(6) )
	ROM_SYSTEM_BIOS( 7, "epmon2", "epmon2" )
	ROMX_LOAD( "epmon2.z16",  0x00000, 0x10000, CRC(3b8a7b59) SHA1(33741f0725e3eaa21c6881c712579b3c1fd30607), ROM_BIOS(7) )
	ROM_SYSTEM_BIOS( 8, "epmon3", "epmon3" )
	ROMX_LOAD( "epmon3.z16",  0x00000, 0x10000, CRC(51313af1) SHA1(60c293171a1c7cb9a5ff6d681e61894f44fddbd1), ROM_BIOS(8) )

	ROM_REGION( 0x157, "plds", 0 )
	ROM_LOAD( "pal14l8.z10", 0x000, 0x157, NO_DUMP )
ROM_END

ROM_START( prof181x )
	ROM_REGION( 0x10000, HD64180_TAG, 0 )
	ROM_SYSTEM_BIOS( 0, "pmon13", "pmon v1.3" )
	ROMX_LOAD( "pmon1_3.u13", 0x00000, 0x04000, CRC(32986688) SHA1(a6229d7e66ef699722ca3d41179fe3f1b75185d4), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "pmon14", "pmon v1.4" )
	ROMX_LOAD( "pmon1_4.u13", 0x00000, 0x04000, CRC(ed03f49f) SHA1(e016f9e0b89ab64c6203e2e46501d0b09f74ee9b), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "pmon15", "pmon v1.5" )
	ROMX_LOAD( "pmon1_5.u13", 0x00000, 0x04000, CRC(f43d185c) SHA1(a7a219b3d48c74602b3116cfcd34e44d6e7bc423), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 3, "pmon", "pmon" )
	ROMX_LOAD( "pmon.u13",    0x00000, 0x04000, CRC(4f3732d7) SHA1(7dc27262db4e0c8f109470253b9364a216909f2c), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS( 4, "eboot1", "eboot1" )
	ROMX_LOAD( "eboot1.u13",  0x00000, 0x08000, CRC(7a164b3c) SHA1(69367804b5cbc0633e3d7bbbcc256c2c8c9e7aca), ROM_BIOS(4) )
	ROM_SYSTEM_BIOS( 5, "eboot2", "eboot2" )
	ROMX_LOAD( "eboot2.u13",  0x00000, 0x08000, CRC(0c2d4301) SHA1(f1a4f457e287b19e14d8ccdbc0383f183d8a3efe), ROM_BIOS(5) )
	ROM_SYSTEM_BIOS( 6, "epmon1", "epmon1" )
	ROMX_LOAD( "epmon1.u13",  0x00000, 0x10000, CRC(27aabfb4) SHA1(41adf038c474596dbf7d387a1a7f33ed86aa7869), ROM_BIOS(6) )
	ROM_SYSTEM_BIOS( 7, "epmon2", "epmon2" )
	ROMX_LOAD( "epmon2.u13",  0x00000, 0x10000, CRC(3b8a7b59) SHA1(33741f0725e3eaa21c6881c712579b3c1fd30607), ROM_BIOS(7) )
	ROM_SYSTEM_BIOS( 8, "epmon3", "epmon3" )
	ROMX_LOAD( "epmon3.u13",  0x00000, 0x10000, CRC(51313af1) SHA1(60c293171a1c7cb9a5ff6d681e61894f44fddbd1), ROM_BIOS(8) )

	ROM_REGION( 0x157, "plds", 0 ) // converted from JED files
	ROM_LOAD( "pal20v8.u14", 0x000, 0x157, CRC(46da52b0) SHA1(c11362223c0d5c57c6ef970e66d674b89d8e7784) )
	ROM_LOAD( "pal20v8.u15", 0x000, 0x157, CRC(19fef936) SHA1(579ad23ee3c0b1c64c584383f9c8085c6ce3d094) )
	ROM_LOAD( "pal20v8.u19", 0x000, 0x157, CRC(69348c3b) SHA1(6eb8432660eb9b639a95b1973a54dab8b99f10ef) )
	ROM_LOAD( "pal20v8.u21", 0x000, 0x157, CRC(6df4e281) SHA1(602fa4637cd9356acc31b2adfb3a084fd5a0bfcb) )
ROM_END

/* Driver */

/*    YEAR  NAME      PARENT    COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY                 FULLNAME     FLAGS */
COMP( 1986, prof180x, 0,        0,      prof180x, prof180x, prof180x_state, empty_init, "Conitec Datensysteme", "PROF-180X", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
COMP( 1992, prof181x, prof180x, 0,      prof180x, prof180x, prof180x_state, empty_init, "Conitec Datensysteme", "PROF-181X", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
