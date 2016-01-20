// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    Tektronix 4107A/4109A

    Skeleton driver.

****************************************************************************/

/*

    TODO:

    - everything

*/


#include "emu.h"
#include "cpu/i86/i186.h"

#define I80188_TAG "i80188"
#define SCREEN_TAG "screen"

class tek4107a_state : public driver_device
{
public:
	tek4107a_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	virtual void machine_start() override;

	virtual void video_start() override;
	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};

/* Memory Maps */

static ADDRESS_MAP_START( tek4107a_mem, AS_PROGRAM, 8, tek4107a_state )
	AM_RANGE(0x00000, 0xbffff) AM_RAM
	AM_RANGE(0xc0000, 0xfffff) AM_ROM AM_REGION(I80188_TAG, 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( tek4107a_io, AS_IO, 8, tek4107a_state )
ADDRESS_MAP_END

/* Input Ports */

static INPUT_PORTS_START( tek4107a )
INPUT_PORTS_END

/* Video */

void tek4107a_state::video_start()
{
}

UINT32 tek4107a_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

static const gfx_layout tek4107a_charlayout =
{
	8, 15,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ STEP8(0,1) },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8 },
	16*8
};

static GFXDECODE_START( tek4107a )
	GFXDECODE_ENTRY( "chargen", 0x0000, tek4107a_charlayout, 0, 1 )
GFXDECODE_END

/* Machine Initialization */

void tek4107a_state::machine_start()
{
}

/* Machine Driver */

static MACHINE_CONFIG_START( tek4107a, tek4107a_state )
	/* basic machine hardware */
	MCFG_CPU_ADD(I80188_TAG, I80188, 21000000)
	MCFG_CPU_PROGRAM_MAP(tek4107a_mem)
	MCFG_CPU_IO_MAP(tek4107a_io)

	/* video hardware */
	MCFG_SCREEN_ADD(SCREEN_TAG, RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_UPDATE_DRIVER(tek4107a_state, screen_update)
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 480-1)

	MCFG_PALETTE_ADD("palette", 64)
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", tek4107a)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( tek4109a, tek4107a )
	/* video hardware */
	MCFG_PALETTE_MODIFY("palette")
	MCFG_PALETTE_ENTRIES(4096)
MACHINE_CONFIG_END

/* ROMs */

ROM_START( tek4107a )
	ROM_REGION( 0x40000, I80188_TAG, 0 )
	ROM_LOAD16_BYTE( "160-2379-03.u60",  0x00000, 0x8000, NO_DUMP )
	ROM_LOAD16_BYTE( "160-2380-03.u160", 0x00001, 0x8000, NO_DUMP )
	ROM_LOAD16_BYTE( "160-2377-03.u70",  0x10000, 0x8000, NO_DUMP )
	ROM_LOAD16_BYTE( "160-2378-03.u170", 0x10001, 0x8000, CRC(feac272f) SHA1(f2018dc9bb5bd6840b2843c709cbc24689054dc0) )
	ROM_LOAD16_BYTE( "160-2375-03.u80",  0x20000, 0x8000, NO_DUMP )
	ROM_LOAD16_BYTE( "160-2376-03.u180", 0x20001, 0x8000, CRC(35a44e75) SHA1(278c7f218105ad3473409b16a8d27a0b9f7a859e) )
	ROM_LOAD16_BYTE( "160-2373-03.u90",  0x30000, 0x8000, CRC(d5e89b15) SHA1(d6eb1a0a684348a8194238d641528fb96fb29087) )
	ROM_LOAD16_BYTE( "160-2374-03.u190", 0x30001, 0x8000, CRC(a3fef76e) SHA1(902845c4aa0cbc392b62c726ac746ca62567d91c) )

	ROM_REGION( 0x1000, "chargen", 0 )
	ROM_LOAD( "160-2381 v1.0.u855", 0x0000, 0x1000, CRC(ac7ca279) SHA1(4c9de06d1c346f83eb8d0d09a0eb32c40bd8014c) )
ROM_END

ROM_START( tek4109a )
	// another set with 160-32xx-03 v10.5 labels exists: http://picasaweb.google.com/glen.slick/Tektronix4107A#5300179291078507810
	ROM_REGION( 0x40000, I80188_TAG, 0 )
	ROM_LOAD16_BYTE( "160-3283-02 v8.2.u60",  0x00000, 0x8000, CRC(2a821db6) SHA1(b4d8b74bd9fe43885dcdc4efbdd1eebb96e32060) )
	ROM_LOAD16_BYTE( "160-3284-02 v8.2.u160", 0x00001, 0x8000, CRC(ee567b01) SHA1(67b1b0648cfaa28d57473bcc45358ff2bf986acf) )
	ROM_LOAD16_BYTE( "160-3281-02 v8.2.u70",  0x10000, 0x8000, CRC(e2713328) SHA1(b0bb3471539ef24d79b18d0e33bc148ed27d0ec4) )
	ROM_LOAD16_BYTE( "160-3282-02 v8.2.u170", 0x10001, 0x8000, CRC(c109a4f7) SHA1(762019105c1f82200a9c99ccfcfd8ee81d2ac4fe) )
	ROM_LOAD16_BYTE( "160-3279-02 v8.2.u80",  0x20000, 0x8000, CRC(00822078) SHA1(a82e61dafccbaea44e67efaa5940e52ec6d07d7d) )
	ROM_LOAD16_BYTE( "160-3280-02 v8.2.u180", 0x20001, 0x8000, CRC(eec9f70f) SHA1(7b65336219f5fa0d11f8be2b37040b564a53c52f) )
	ROM_LOAD16_BYTE( "160-3277-02 v8.2.u90",  0x30000, 0x8000, CRC(cf6ebc97) SHA1(298db473874c57bf4eec788818179748030a9ad8) )
	ROM_LOAD16_BYTE( "160-3278-02 v8.2.u190", 0x30001, 0x8000, CRC(d6124cd1) SHA1(f826aee5ec07cf5ac369697d93def0259ad225bb) )

	ROM_REGION( 0x1000, "chargen", 0 )
	ROM_LOAD( "160-3087 v1.0.u855", 0x0000, 0x1000, CRC(97479528) SHA1(e9e15f1f64b3b6bd139accd51950bae71fdc2193) )
ROM_END

/* System Drivers */

/*    YEAR  NAME        PARENT      COMPAT  MACHINE     INPUT       INIT    COMPANY         FULLNAME            FLAGS */
COMP( 1983, tek4107a,   0,          0,      tek4107a,   tek4107a, driver_device,   0,    "Tektronix", "Tektronix 4107A", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
COMP( 1983, tek4109a,   tek4107a,   0,      tek4109a,   tek4107a, driver_device,   0,    "Tektronix", "Tektronix 4109A", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
