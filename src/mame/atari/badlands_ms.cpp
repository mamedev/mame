// license:BSD-3-Clause
// copyright-holders:

/*
    Bad Lands (Modular System)

    As with most of the 'Modular System' setups, the hardware is heavily modified from the original
    and consists of a multi-board stack in a cage, hence different driver.

    For this game the Modular System cage contains 6 main boards and 1 sub board.

    MOD-6/1 - MC68000P10, 4 ROMs, RAMs, 20 MHz XTAL.
    MOD 1/4 - Sound board (Z8400BB1, 2 x YM2203C). 2 8-dips banks + small sub board with OKI M5205.
    MOD 51/1 - Sprite board, has logic + 4 empty ROM sockets. Sprite ROMs are actually on the below board.
    MODULAR SYSTEM 2 MOD 5/1 - red sprite ROM board, 12 sprite ROMs populated (maximum 24 ROMs)
    MOD 4/3 - Tilemap board, has logic + 4 tilemap ROMs, long thin sub-board (C0477 SOLD) with no chips, just routing along one edge.
    MOD 21/1 - Logic + 2 PROMs.

    TODO:
    - everything,
    - derive from badlands_state (badlands.cpp) or badlandsbl_state (badlandsbl.cpp) once all the common points and differences are verified.
*/


#include "emu.h"

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "sound/msm5205.h"
#include "sound/ymopn.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class badlands_ms_state : public driver_device
{
public:
	badlands_ms_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen")
	{ }

	void badlandsm(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map) ATTR_COLD;
};


uint32_t badlands_ms_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}


void badlands_ms_state::machine_start()
{
}


void badlands_ms_state::main_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0xfff000, 0xffffff).ram();
}


static INPUT_PORTS_START( badlandsm )
	PORT_START("DSW")
	PORT_DIPUNKNOWN_DIPLOC(0x0001, 0x0001, "SW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x0002, 0x0002, "SW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x0004, 0x0004, "SW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x0008, 0x0008, "SW1:4")
	PORT_DIPUNKNOWN_DIPLOC(0x0010, 0x0010, "SW1:5")
	PORT_DIPUNKNOWN_DIPLOC(0x0020, 0x0020, "SW1:6")
	PORT_DIPUNKNOWN_DIPLOC(0x0040, 0x0040, "SW1:7")
	PORT_DIPUNKNOWN_DIPLOC(0x0080, 0x0080, "SW1:8")
	PORT_DIPUNKNOWN_DIPLOC(0x0100, 0x0100, "SW2:1")
	PORT_DIPUNKNOWN_DIPLOC(0x0200, 0x0200, "SW2:2")
	PORT_DIPUNKNOWN_DIPLOC(0x0400, 0x0400, "SW2:3")
	PORT_DIPUNKNOWN_DIPLOC(0x0800, 0x0800, "SW2:4")
	PORT_DIPUNKNOWN_DIPLOC(0x1000, 0x1000, "SW2:5")
	PORT_DIPUNKNOWN_DIPLOC(0x2000, 0x2000, "SW2:6")
	PORT_DIPUNKNOWN_DIPLOC(0x4000, 0x4000, "SW2:7")
	PORT_DIPUNKNOWN_DIPLOC(0x8000, 0x8000, "SW2:8")
INPUT_PORTS_END


static const gfx_layout tiles_layout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0,8,16,24 },
	{ 0,1,2,3,4,5,6,7, 512,513,514,515,516,517,518,519 },
	{ STEP16(0,32) },
	16 * 16 * 4
};

static GFXDECODE_START( gfx_badlandsm ) // TODO: needs adjusting, especially sprites
	GFXDECODE_ENTRY( "tiles",   0, tiles_layout,   0, 8 )
	GFXDECODE_ENTRY( "sprites", 0, tiles_layout, 128, 8 )
GFXDECODE_END


void badlands_ms_state::badlandsm(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 20_MHz_XTAL / 2); // divisor unknown
	m_maincpu->set_addrmap(AS_PROGRAM, &badlands_ms_state::main_map);

	Z80(config, "audiocpu", 20_MHz_XTAL / 5).set_disable(); // divisor unknown

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER); // all wrong
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	m_screen->set_size(256, 256);
	m_screen->set_visarea(0, 256-1, 0, 256-32-1);
	m_screen->set_screen_update(FUNC(badlands_ms_state::screen_update));
	m_screen->set_palette("palette");

	PALETTE(config, "palette").set_format(palette_device::IRGB_1555, 256).set_membits(8);

	GFXDECODE(config, "gfxdecode", "palette", gfx_badlandsm);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, "soundlatch");

	YM2203(config, "ym1", 20_MHz_XTAL / 5).add_route(ALL_OUTPUTS, "mono", 0.15); // divisor unknown

	YM2203(config, "ym2", 20_MHz_XTAL / 5).add_route(ALL_OUTPUTS, "mono", 0.15); // divisor unknown

	MSM5205(config, "msm", 20_MHz_XTAL / 5).add_route(ALL_OUTPUTS, "mono", 0.15); // divisor unknown
}

ROM_START( badlandsm )
	ROM_REGION( 0x100000, "maincpu", 0 ) // on MOD 6/1 board, very similar to badlandsb
	ROM_LOAD16_BYTE( "mod-6-1_cpu_ba_603a.ic17", 0x00000, 0x10000, CRC(ee03753e) SHA1(42b3f214aa889cb62096016cc214ea7f5dde5e93) )
	ROM_LOAD16_BYTE( "mod-6-1_cpu_ba_606a.ic8",  0x00001, 0x10000, CRC(f22364a2) SHA1(d077bdcad3c0d301007e3fa539039b4a40bbc399) )
	ROM_LOAD16_BYTE( "mod-6-1_cpu_ba_602.ic20",  0x20000, 0x10000, CRC(0e2e807f) SHA1(5b61de066dca12c44335aa68a13c821845657866) )
	ROM_LOAD16_BYTE( "mod-6-1_cpu_ba_605.ic11",  0x20001, 0x10000, CRC(99a20c2c) SHA1(9b0a5a5dafb8816e72330d302c60339b600b49a8) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // on MOD 1/4 board
	ROM_LOAD( "mod-1-4_sound_ba_401.ic12",  0x00000, 0x10000, CRC(6cddace7) SHA1(f53c3d6fdec22bc2d39814d482db76aa0fc18af7) )

	ROM_REGION( 0x80000, "tiles", ROMREGION_INVERT ) // on MOD 4/2 board, different format from original and other bootlegs
	ROM_LOAD32_BYTE( "mod-4-3_ba_401.ic17",  0x00003, 0x20000, CRC(a943e878) SHA1(4330c5599c23818044ccba82331abe31b1d8216a) )
	ROM_LOAD32_BYTE( "mod-4-3_ba_402.ic16",  0x00002, 0x20000, CRC(cbf9f01d) SHA1(390aaed71b247099320118d8b8d59ec10f65a11c) )
	ROM_LOAD32_BYTE( "mod-4-3_ba_403.ic15",  0x00001, 0x20000, CRC(b27dc32a) SHA1(8e56a0a17d505e3a65a5d2c2c6314f21a4630485) )
	ROM_LOAD32_BYTE( "mod-4-3_ba_404.ic14",  0x00000, 0x20000, CRC(263f474f) SHA1(86b6762bcc28117e65cb201c9bc5a12c3f350e46) )

	ROM_REGION( 0xc0000, "sprites", ROMREGION_ERASEFF | ROMREGION_INVERT ) // on MODULAR SYSTEM 2 MOD 5/1 board, different format from original and other bootlegs. TODO: verify ROM loading
	ROM_LOAD32_BYTE( "mod-5-1_ba_503.ic3",   0x00003, 0x10000, CRC(4f3c9a22) SHA1(6ee806aaf663540902a372c04592d2e3030934aa) ) // xxxxxxxxxxxx1xxx = 0x00
	ROM_LOAD32_BYTE( "mod-5-1_ba_512.ic12",  0x00002, 0x10000, CRC(352f2167) SHA1(dd4aea8450e42b6f70801bbb1091df63bffbe463) ) // xxxxxxxxxxxx1xxx = 0x00
	ROM_LOAD32_BYTE( "mod-5-1_ba_518.ic18",  0x00001, 0x10000, CRC(834aa5ef) SHA1(0761110de2a27453fc654970f8a7540fcd2c2615) ) // xxxxxxxxxxxx1xxx = 0x00
	ROM_LOAD32_BYTE( "mod-5-1_ba_524.ic24",  0x00000, 0x10000, CRC(5d7f2231) SHA1(685ab5214c800ef49a6da1cdea5a672a03c281b3) ) // xxxxxxxxxxxx1xxx = 0x00
	ROM_LOAD32_BYTE( "mod-5-1_ba_504.ic4",   0x40003, 0x10000, CRC(690fc050) SHA1(5d77b674ff3fa5701d8fe0f4d4aa4d6821d4da7c) )
	ROM_LOAD32_BYTE( "mod-5-1_ba_513.ic13",  0x40002, 0x10000, CRC(d239ac97) SHA1(4809760dc4c3c3b830402450bd24ccc161bf01f1) )
	ROM_LOAD32_BYTE( "mod-5-1_ba_519.ic19",  0x40001, 0x10000, CRC(bf2cc0f9) SHA1(9175eeac707c04795e1198e8d3a4aac8e64f74d1) ) // xxxxxxxxxxxx1xxx = 0x00
	ROM_LOAD32_BYTE( "mod-5-1_ba_525.ic25",  0x40000, 0x10000, CRC(26d53fc5) SHA1(f0eebbe31fb887ce7c91f5268af2af373ecc88c2) ) // xxxxxxxxxxxx1xxx = 0x00
	ROM_LOAD32_BYTE( "mod-5-1_ba_505.ic5",   0x80003, 0x10000, CRC(a5f91533) SHA1(add36c54a6caec431434b138180d7ca38411d84e) )
	ROM_LOAD32_BYTE( "mod-5-1_ba_514.ic14",  0x80002, 0x10000, CRC(5d3e52e8) SHA1(f8bbc9067058af6ee5ca2ac79c5deba6fd6a366b) )
	ROM_LOAD32_BYTE( "mod-5-1_ba_520.ic20",  0x80001, 0x10000, CRC(f4e132ae) SHA1(c34b4a3b204035e2f4d5209aee48673f758f3ceb) )
	ROM_LOAD32_BYTE( "mod-5-1_ba_526.ic26",  0x80000, 0x10000, CRC(243a6bc8) SHA1(7f3e6031824c28f37afafc80fc9757bb5f80e660) )

	ROM_REGION( 0x320, "proms", 0 )    // PROMs (function unknown)
	ROM_LOAD( "mod-1-4_sound_105_82s123.ic20", 0x000, 0x020, CRC(14d72781) SHA1(372dc021d8aaf4aa6fd46e69a3d8f1c68113426f) )
	ROM_LOAD( "mod-51-1_502_82s129a.ic10",     0x020, 0x100, CRC(15085e44) SHA1(646e7100fcb112594023cf02be036bd3d42cc13c) )
	ROM_LOAD( "mod_21_1_205_82s129.ic12",      0x120, 0x100, CRC(204a7aee) SHA1(322164134aa65c37a9389024f921364a81d13e88) )
	ROM_LOAD( "mod_21_1_210_82s129.ic4",       0x220, 0x100, CRC(d3e676aa) SHA1(241a9f9df88e8f796ca8c773ae7930f6e7ce4db8) )

	ROM_REGION( 0x1000, "plds", ROMREGION_ERASEFF )
	ROM_LOAD( "mod-6-1_cpu_644_gal16v8.ic13", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "mod-6-1_cpu_643_gal16v8.ic7",  0x000, 0x117, CRC(d05b2c26) SHA1(51b718554bfba96a3c9e7b3a000fc80679044cb3) )
	ROM_LOAD( "mod-4-3_403_gal16v8.ic29",     0x000, 0x117, CRC(c136de93) SHA1(116f6d3b456d20621ab07a005c1421f57569915c) )
	ROM_LOAD( "mod-51-1_50503_pal16r6.ic46",  0x000, 0x104, CRC(07eb86d2) SHA1(482eb325df5bc60353bac85412cf45429cd03c6d) )
	ROM_LOAD( "mod-5-1_5143_gal16v8.ic9",     0x000, 0x117, CRC(2d9cf890) SHA1(b8e3bdfe8543a8729e0397166780036b074f4bc5) )
	ROM_LOAD( "mod-5-1_5242_gal16v8.ic8",     0x000, 0x117, NO_DUMP )
ROM_END

} // anonymous namespace


GAME( 199?, badlandsm, badlands, badlandsm, badlandsm, badlands_ms_state, empty_init, ROT0, "bootleg (Gaelco / Ervisa)", "Bad Lands (Modular System)", MACHINE_IS_SKELETON )
