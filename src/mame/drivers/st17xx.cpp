// license:BSD-3-Clause
// copyright-holders:Justin Kerk
/***************************************************************************

    Saturn ST-17xx series DVD players

    Skeleton driver.

****************************************************************************/

/*

    TODO:

    - everything

    Technical info:
    DVD-player-on-a-chip designs from Mediatek:
    http://www.mediatek.com/en/products/home-entertainment/consumer-dvd-blu-ray/

    MT1379:
    http://pdf.datasheetcatalog.com/datasheets/134/477326_DS.pdf

    MT1389:
    http://newage.mpeg4-players.info/mt1389/mt1389.html
    http://groups.yahoo.com/group/MEDIATEK1389/
    http://www.sigmatek-xm600.borec.cz/chip.html
    http://www.sigmatek-xm600.borec.cz/Info%20-%20MT1389%20v0.3b%20English.doc (rename to .rtf)
    Includes an 8032, ARM7TDMI, and unknown DSP

    SUNPLUS SPHE8200A:
    http://pdf.datasheetcatalog.com/datasheets/2300/499420_DS.pdf
    https://004code.googlecode.com/svn/trunk/h/regmapo_8200.h
    "32-bit RISC" architecture, but which one?

*/


#include "emu.h"
#include "cpu/arm7/arm7.h"
#include "cpu/arm7/arm7core.h"

#define SCREEN_TAG "screen"

class st17xx_state : public driver_device
{
public:
	st17xx_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	virtual void machine_start() override;

	virtual void video_start() override;
	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};

/* Memory Maps */

static ADDRESS_MAP_START( cpu_map, AS_PROGRAM, 32, st17xx_state )
	AM_RANGE(0x00000000, 0x000fffff) AM_ROM
ADDRESS_MAP_END

/* Input Ports */

static INPUT_PORTS_START( st17xx )
INPUT_PORTS_END

/* Video */

void st17xx_state::video_start()
{
}

UINT32 st17xx_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

/* Machine Initialization */

void st17xx_state::machine_start()
{
}

/* Machine Driver */

static MACHINE_CONFIG_START( st17xx, st17xx_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", ARM7, 50000000) /* speed unknown */
	MCFG_CPU_PROGRAM_MAP(cpu_map)

	/* video hardware */
	MCFG_SCREEN_ADD(SCREEN_TAG, RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_UPDATE_DRIVER(st17xx_state, screen_update)
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 480-1)

	MCFG_PALETTE_ADD("palette", 64)
MACHINE_CONFIG_END

/* ROMs */

/*
Uses MT1379 DVD player chip. There are two versions - with outputs for headphones and karaoke.
Reads: AudioCD, VCD, MP3, Pictures Kodak, Xvid
*/
ROM_START( st1700h )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD32_DWORD( "st1700headphones.bin", 0x000000, 0x100000, CRC(c219e9df) SHA1(6769bcd2c6b19a2cd4e0e36f01824508a7342e4e) )
ROM_END

ROM_START( st1701 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD32_DWORD( "st1701.bin", 0x000000, 0x100000, CRC(e6c08dae) SHA1(00ed616fc4c7955be036ca739d9c34038c0ecd58) )
ROM_END

ROM_START( st1702 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD32_DWORD( "st1702.bin", 0x000000, 0x0f67dc, CRC(ca5a1a97) SHA1(21a0632cff4bcb25f9a5f7a4e9bd8aeaa9f715c3) )
ROM_END

ROM_START( st1703 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD32_DWORD( "st1703.bin", 0x000000, 0x100000, CRC(04963c10) SHA1(ef7eb45de2fd6dab826c362939ea67370d9bd84b) )
ROM_END

/*
Uses MT1389 DVD player chip.
Reads: Audio CD, VCD, MP3, MPEG4, Pictures Kodak, Xvid
*/
ROM_START( st1704 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD32_DWORD( "st-1704.bin", 0x000000, 0x0feef0, CRC(b834d37e) SHA1(d048b793c2c838f07fdae9592b07fd4fb2ec73b8) )
ROM_END

ROM_START( st1705 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD32_DWORD( "st1705.bin", 0x000000, 0x100000, CRC(efda2beb) SHA1(d30505552fc9ffd37ac12f576d792212deb10d84) )
ROM_END

/*
SATURN
ST 1706, 1707, 1708.
Chip: SUNPLUS SPHE 8200A
Servo: SUNPLUS SPHE 6300A
Flash: EN29F040A-70P or ID39F040-70P
Eeprom: 24C02
Driver: CD5954CB
Indication & Key processor : CS16312EN
Laser head: CSQCL33EI38 (requires clarification)
*/
ROM_START( st1706 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD32_DWORD( "1706_sphe8200.bin", 0x000000, 0x080000, CRC(6a92fa45) SHA1(d40327ebfacd0c3690b170f802d8059e22848aa0) )
ROM_END

ROM_START( st1707 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD32_DWORD( "1707_sphe8200.bin", 0x000000, 0x080000, CRC(4d90d176) SHA1(7058b37413c90fd9e7f845944191f7ebe9e03250) )
ROM_END

ROM_START( st1708 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD32_DWORD( "1708_sphe8200.bin", 0x000000, 0x080000, CRC(5176c819) SHA1(2f1ae3389380be27fdd6a66da119e3ccdaa2fd59) )
ROM_END

ROM_START( st1714 )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD32_DWORD( "st1714.bin", 0x000000, 0x200000, CRC(08fc0a1b) SHA1(74dfd5595e1ab45fb9aff50a6c365fd9c9b33c33) )
ROM_END

/* System Drivers */

/*    YEAR  NAME        PARENT      COMPAT  MACHINE     INPUT     CLASS          INIT    COMPANY      FULLNAME   FLAGS */
CONS( 200?, st1700h,    0,          0,      st17xx,     st17xx,   driver_device,   0,    "Saturn",    "ST-1700 (headphone version)", MACHINE_IS_SKELETON )
CONS( 200?, st1701,     0,          0,      st17xx,     st17xx,   driver_device,   0,    "Saturn",    "ST-1701", MACHINE_IS_SKELETON )
CONS( 200?, st1702,     0,          0,      st17xx,     st17xx,   driver_device,   0,    "Saturn",    "ST-1702", MACHINE_IS_SKELETON )
CONS( 200?, st1703,     0,          0,      st17xx,     st17xx,   driver_device,   0,    "Saturn",    "ST-1703", MACHINE_IS_SKELETON )
CONS( 200?, st1704,     0,          0,      st17xx,     st17xx,   driver_device,   0,    "Saturn",    "ST-1704", MACHINE_IS_SKELETON )
CONS( 200?, st1705,     0,          0,      st17xx,     st17xx,   driver_device,   0,    "Saturn",    "ST-1705", MACHINE_IS_SKELETON )
CONS( 200?, st1706,     0,          0,      st17xx,     st17xx,   driver_device,   0,    "Saturn",    "ST-1706", MACHINE_IS_SKELETON )
CONS( 200?, st1707,     0,          0,      st17xx,     st17xx,   driver_device,   0,    "Saturn",    "ST-1707", MACHINE_IS_SKELETON )
CONS( 200?, st1708,     0,          0,      st17xx,     st17xx,   driver_device,   0,    "Saturn",    "ST-1708", MACHINE_IS_SKELETON )
CONS( 200?, st1714,     0,          0,      st17xx,     st17xx,   driver_device,   0,    "Saturn",    "ST-1714", MACHINE_IS_SKELETON )
