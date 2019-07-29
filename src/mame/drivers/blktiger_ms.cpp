// license:BSD-3-Clause
// copyright-holders:

/*
    Black Tiger (Modular System)

    As with most of the 'Modular System' setups, the hardware is heavily modified from the original
    and consists of a multi-board stack in a cage, hence different driver.

    The Modular System cage contains 6 main boards for this game.

    MOD3-4 - Z80 board (CPU + 5 ROMs + RAM + 24MHz XTAL).
    MOD21/1 - RAM board? 20 MHz XTAL.
    MOD1/5 - Sound board (Z80, 2xYM2203C). 2 8-dips banks.
    MOD51/1 - Sprite board, has logic + 4 sprite ROMs.
    MOD 4/3 - Tilemap board, has logic + 2 tilemap ROMs, long thin sub-board (CAR-0484/1 SOLD) with no chips, just routing along one edge.
    MOD 4/3 - Tilemap board, has logic + 4 tilemap ROMs, long thin sub-board (CAR-0484/1 SOLD) with no chips, just routing along one edge.
*/


#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "sound/2203intf.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


class blktiger_ms_state : public driver_device
{
public:
	blktiger_ms_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen")
	{ }

	void blktigerm(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void blktigerm_map(address_map &map);
};


void blktiger_ms_state::blktigerm_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
}


void blktiger_ms_state::machine_start()
{
}


uint32_t blktiger_ms_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}

static INPUT_PORTS_START( blktigerm )
INPUT_PORTS_END

static GFXDECODE_START( gfx_blktiger_ms )
GFXDECODE_END

void blktiger_ms_state::blktigerm(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 24_MHz_XTAL / 4); // divisor unknown
	m_maincpu->set_addrmap(AS_PROGRAM, &blktiger_ms_state::blktigerm_map);

	Z80(config, "audiocpu", 24_MHz_XTAL / 8).set_disable(); // divisor unknown, no XTAL on the PCB, might also use the 20 MHz one

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER); // all wrong
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500) /* not accurate */);
	m_screen->set_size(256, 256);
	m_screen->set_visarea(0, 256-1, 0, 256-32-1);
	m_screen->set_screen_update(FUNC(blktiger_ms_state::screen_update));
	m_screen->set_palette("palette");

	PALETTE(config, "palette").set_format(palette_device::xBRG_444, 1024);

	GFXDECODE(config, "gfxdecode", "palette", gfx_blktiger_ms);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, "soundlatch");

	YM2203(config, "ym1", 24_MHz_XTAL / 8).add_route(ALL_OUTPUTS, "mono", 0.15); // divisor unknown, no XTAL on the PCB, might also use the 20 MHz one

	YM2203(config, "ym2", 24_MHz_XTAL / 8).add_route(ALL_OUTPUTS, "mono", 0.15); // divisor unknown, no XTAL on the PCB, might also use the 20 MHz one
}

ROM_START( blktigerm )
	ROM_REGION( 0x50000, "maincpu", 0 ) // on MOD 3/4 board
	ROM_LOAD( "3_bl_301.ic14",   0x00000, 0x08000, CRC(b4525312) SHA1(958ce9a7f41422f3011412e4a16dd3ad65019733) ) // encrypted / scrambled?
	ROM_LOAD( "3_bl_302_c.ic13", 0x10000, 0x10000, CRC(c943415f) SHA1(08e82d2557de01c979a0a58b5433e9bdcdb118cb) )
	ROM_LOAD( "3_bl_303_b.ic12", 0x20000, 0x10000, CRC(72114f6b) SHA1(049e747431cd95db2e7414467754424f68a8d757) )
	ROM_LOAD( "3_bl_304.ic18",   0x30000, 0x10000, CRC(36f669c6) SHA1(e91fd222919904d4d4b3ef70a22ab599a7cfa263) )
	ROM_LOAD( "3_bl_305.ic19",   0x40000, 0x10000, CRC(b30a99af) SHA1(19d6dbe11ffeb9a21dbfe75eb39ce9ceaef5eb31) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) //
	ROM_LOAD( "1_bl_101.ic12",  0x0000, 0x8000, CRC(14028686) SHA1(64dc219d906f1bdd0c2bc05aff5aa73e001a6901) )

	ROM_REGION( 0x08000, "gfx1", 0 ) // on one of the MOD 4/3 boards, both ROMs 0xxxxxxxxxxxxxx = 0xFF
	ROM_LOAD( "4_bl_401.ic17",  0x00000, 0x04000, CRC(ab1afb3d) SHA1(555332ccfb69e65b776f94ffac9a4a051fb6f09e) ) /* characters */
	ROM_CONTINUE(               0x00000, 0x04000)
	ROM_LOAD( "4_bl_402.ic16",  0x04000, 0x04000, CRC(89445b11) SHA1(7d9ab6e88d7de3a0e31b3b8a5e57dd39afd7940d) )
	ROM_CONTINUE(               0x04000, 0x04000)

	ROM_REGION( 0x40000, "gfx2", 0 ) // on the other MOD 4/3 board
	ROM_LOAD( "4_a_bl_4a01.ic17",  0x00000, 0x10000, CRC(fc00fd6c) SHA1(066714421fa782c6cfad9f695f05e6e3551ffdc7) ) /* tiles */
	ROM_LOAD( "4_a_bl_4a02.ic16",  0x10000, 0x10000, CRC(49cd8afa) SHA1(147b65dad4f05b940d1f7a4028f5748ba1b7daa5) )
	ROM_LOAD( "4_a_bl_4a03.ic15",  0x20000, 0x10000, CRC(50207dd4) SHA1(bee343fbefc817fa14b8b303f013bf5b89e911f8) )
	ROM_LOAD( "4_a_bl_4a04.ic14",  0x30000, 0x10000, CRC(4bb0c1ba) SHA1(9433c78f0de7cb1f92b22612b9f3c51d813b025a) )

	ROM_REGION( 0x40000, "gfx3", 0 ) // on MOD 51/1 board
	ROM_LOAD( "51_bl_501.ic43",   0x00000, 0x10000, CRC(ace32b94) SHA1(4a09e8dc73bd16f7d378aefbe5d433dd1396f97d) )    /* sprites */
	ROM_LOAD( "51_bl_502.ic42",   0x10000, 0x10000, CRC(f6c4cc0b) SHA1(678fd71e90237f8e19eae78fe150a4c5d3494c6c) )
	ROM_LOAD( "51_bl_503.ic41",   0x20000, 0x10000, CRC(61e3ae0b) SHA1(cbb83827f027acd6b905f4210476810bcd4b03a9) )
	ROM_LOAD( "51_bl_504.ic40",   0x30000, 0x10000, CRC(2cb45034) SHA1(b2fd3b7b7b9d68b6ff1985166b106e65b17dbb23) )

	ROM_REGION( 0x0700, "proms", 0 )    /* PROMs (function unknown) */
	ROM_LOAD( "1_p0101_82s123.ic20",       0x0000, 0x0020, CRC(3fd60d3a) SHA1(8100fa7453638ac40193b5d92404f41b101ed2cc) )
	ROM_LOAD( "21_p0201_82s129.ic4",       0x0100, 0x0100, CRC(2697da58) SHA1(e62516b886ff6e204b718e5f0c6ce2712e4b7fc5) )
	ROM_LOAD( "21_p0202_82s129.ic12",      0x0200, 0x0100, CRC(e434128a) SHA1(ef0f6d8daef8b25211095577a182cdf120a272c1) )
	ROM_LOAD( "3_subcpu_p0322_82s147.bin", 0x0300, 0x0200, CRC(6d427336) SHA1(6ce1ef41df2b450fafe11c3021991618bdb771ed) )
	ROM_LOAD( "4_a_bl_82s123.ic13",        0x0500, 0x0020, CRC(671f1183) SHA1(dbc21c3922c5e69340daa9008e1200f1304b3e8f) )
	ROM_LOAD( "51_p0502_82s129.ic10",      0x0600, 0x0100, CRC(15085e44) SHA1(646e7100fcb112594023cf02be036bd3d42cc13c) )

	ROM_REGION( 0x1000, "plds", ROMREGION_ERASEFF )
	ROM_LOAD( "4_p0403_pal16r8a-2cn.ic29",      0x000, 0x104, CRC(506156cc) SHA1(5560671fc2c9872ed28620491af5dc486909fc6e) )
	ROM_LOAD( "4_a_p0403_pal16r8a-2cn.ic29",    0x000, 0x104, CRC(506156cc) SHA1(5560671fc2c9872ed28620491af5dc486909fc6e) ) // identical to the above one (same PCB type)
	ROM_LOAD( "51_p0503_pal16r8a.ic56",         0x000, 0x104, CRC(07eb86d2) SHA1(482eb325df5bc60353bac85412cf45429cd03c6d) )
ROM_END

GAME( 1991, blktigerm,  blktiger,  blktigerm,  blktigerm,  blktiger_ms_state, empty_init, ROT0, "bootleg (Gaelco / Ervisa)", "Black Tiger (Modular System)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
