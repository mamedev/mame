// license:BSD-3-Clause
// copyright-holders:

/*
    Power Spikes (Modular System)

    As with most of the 'Modular System' setups, the hardware is heavily modified from the original
    and consists of a multi-board stack in a cage, hence different driver.

    For this game the Modular System cage contains 6 main boards and 1 sub board.
    More information and PCB pictures: https://www.recreativas.org/modular-system-super-volleyball-11089-gaelco-sa

    MOD-6/1  - MC68000CP12, 2 ROMs, RAMs, 20 MHz XTAL.
    MOD 1/4  - Sound board (Z8400BB1, 2 x YM2203C), one EPROM, two 8 dip switches banks, a small sub board with OKI M5205.
    MOD 21/1 - Logic, 2 PROMs, 28.000 MHz XTAL.
    MOD 4/3  - Tilemap board, has logic + 4 tilemap ROMs, long thin sub-board (C0485 SOLD) with no chips, just routing along one edge.
    MODULAR SYSTEM 2 MOD 5/1 - Red sprite ROM board, 16 sprite ROMs populated (maximum 24 ROMs)
    MOD 51/1 - Sprite board, has logic, a PROM, and 4 empty ROM sockets. Sprite ROMs are actually on the below board.

    TODO:
    - everything,
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

class pspikes_ms_state : public driver_device
{
public:
	pspikes_ms_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen")
	{ }

	void pspikesm(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map);
};


uint32_t pspikes_ms_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}


void pspikes_ms_state::machine_start()
{
}


void pspikes_ms_state::main_map(address_map &map)
{
}


static INPUT_PORTS_START( pspikesm )
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


void pspikes_ms_state::pspikesm(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 20_MHz_XTAL / 2); // divisor unknown
	m_maincpu->set_addrmap(AS_PROGRAM, &pspikes_ms_state::main_map);

	Z80(config, "audiocpu", 20_MHz_XTAL / 5).set_disable(); // divisor unknown

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER); // all wrong
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	m_screen->set_size(256, 256);
	m_screen->set_visarea(0, 256-1, 0, 256-32-1);
	m_screen->set_screen_update(FUNC(pspikes_ms_state::screen_update));
	m_screen->set_palette("palette");

	PALETTE(config, "palette").set_format(palette_device::IRGB_1555, 256).set_membits(8);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	YM2203(config, "ym1", 20_MHz_XTAL / 5).add_route(ALL_OUTPUTS, "mono", 0.15); // divisor unknown

	YM2203(config, "ym2", 20_MHz_XTAL / 5).add_route(ALL_OUTPUTS, "mono", 0.15); // divisor unknown

	MSM5205(config, "msm", 20_MHz_XTAL / 5).add_route(ALL_OUTPUTS, "mono", 0.15); // divisor unknown
}

ROM_START( pspikesm )
	ROM_REGION( 0x100000, "maincpu", 0 ) // on MOD 6/1 board
	ROM_LOAD16_BYTE( "mod_6-1_ps_617.ic17", 0x00000, 0x20000, CRC(e963c91d) SHA1(f3e5e2243d1ae5c0cbc0fa55bec099a4048c2776) )
	ROM_LOAD16_BYTE( "mod_6-1_ps_608.ic8",  0x00001, 0x20000, CRC(a6ef3123) SHA1(b2e31a1f4182b324065d06feef96f63ebcfa9daf) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // on MOD 1/4 board
	ROM_LOAD( "mod_1-4_ps_101.ic12",        0x00000, 0x10000, CRC(2f6f7f94) SHA1(49ba5b97807b9488ab485f67ca37795935b8bed0) )

	ROM_REGION( 0x80000, "tiles", ROMREGION_INVERT ) // on MOD 4/2 board
	ROM_LOAD32_BYTE( "mod_4-3_ps_401.ic17", 0x00003, 0x20000, CRC(a7e00b36) SHA1(2b5e85ec02e8893d7d730aad4d690883b1d236cc) )
	ROM_LOAD32_BYTE( "mod_4-3_ps_402.ic16", 0x00002, 0x20000, CRC(96a5c235) SHA1(dad4ef9069d3130f719a402737909bb48225b73c) )
	ROM_LOAD32_BYTE( "mod_4-3_ps_403.ic15", 0x00001, 0x20000, CRC(bfdc60f4) SHA1(2b1893fac2651ac82f5a05b8f891b20c928ced7e) )
	ROM_LOAD32_BYTE( "mod_4-3_ps_404.ic14", 0x00000, 0x20000, CRC(ea1c05a7) SHA1(adfdfeac80df287ffa6f469dc38ea94698817cf4) )

	ROM_REGION( 0x100000, "sprites", ROMREGION_ERASEFF | ROMREGION_INVERT ) // on MODULAR SYSTEM 2 MOD 5/1 board
	ROM_LOAD32_BYTE( "mod_5-1_ps_503.ic3",  0x00003, 0x10000, CRC(2200c459) SHA1(4d0c4eafc6f1d66675358b100805fe1a5d05562a) )
	ROM_LOAD32_BYTE( "mod_5-1_ps_512.ic12", 0x00002, 0x10000, CRC(e4006f38) SHA1(dda067531b112e38bc6af351123f30eddb5f13d3) )
	ROM_LOAD32_BYTE( "mod_5-1_ps_518.ic18", 0x00001, 0x10000, CRC(b02134af) SHA1(0fffdab8f93079babe36cc84c93aeb94c8b09dfb) )
	ROM_LOAD32_BYTE( "mod_5-1_ps_524.ic24", 0x00000, 0x10000, CRC(7579d958) SHA1(904cc765f7e548029cecb400b7d223ad4feccabd) )
	ROM_LOAD32_BYTE( "mod_5-1_ps_504.ic4",  0x40003, 0x10000, CRC(b79abc10) SHA1(dba32ba27b7e255ac2f6ca640295659eba095892) )
	ROM_LOAD32_BYTE( "mod_5-1_ps_513.ic13", 0x40002, 0x10000, CRC(0679a0fc) SHA1(ce316d757569e5b29f8dcd7a1b5eaa05b0175344) )
	ROM_LOAD32_BYTE( "mod_5-1_ps_519.ic19", 0x40001, 0x10000, CRC(8ce19996) SHA1(2a2952b0805d65a30ed08386787aacf7c1ac636a) )
	ROM_LOAD32_BYTE( "mod_5-1_ps_525.ic25", 0x40000, 0x10000, CRC(db5cc36d) SHA1(a466df0d5a7de2cd09e10734d58d44e5f8f59c04) )
	ROM_LOAD32_BYTE( "mod_5-1_ps_505.ic5",  0x80003, 0x10000, CRC(08d7e196) SHA1(95ea557aafddeee971364e9c6678bd2fba147b94) )
	ROM_LOAD32_BYTE( "mod_5-1_ps_514.ic14", 0x80002, 0x10000, CRC(eef21bd7) SHA1(52421c47cd3687cc36dd74bd453ec093f044d3ea) )
	ROM_LOAD32_BYTE( "mod_5-1_ps_520.ic20", 0x80001, 0x10000, CRC(12db2f4a) SHA1(9bd8c6eed080e131fc9b4cc6b6b179f27fc237ee) )
	ROM_LOAD32_BYTE( "mod_5-1_ps_526.ic26", 0x80000, 0x10000, CRC(dd0ae8d8) SHA1(4107ca4c1f580a6d27a71a9f3af94e991f519b16) )
	ROM_LOAD32_BYTE( "mod_5-1_ps_506.ic6",  0xc0003, 0x10000, CRC(df296e38) SHA1(cf190c2504c032359e327e18e1f23b3aa4905749) )
	ROM_LOAD32_BYTE( "mod_5-1_ps_515.ic15", 0xc0002, 0x10000, CRC(363a582a) SHA1(3800af6cce8f7105a47d07a1d2ab60b148dad77c) )
	ROM_LOAD32_BYTE( "mod_5-1_ps_521.ic21", 0xc0001, 0x10000, CRC(b652bb25) SHA1(87976d9ac16e2471e7b87b3509ad1ab34e0d9eaa) )
	ROM_LOAD32_BYTE( "mod_5-1_ps_527.ic27", 0xc0000, 0x10000, CRC(39d9b0a5) SHA1(1396a96fe662bf53a2e86d39518f00a3f9693918) )

	ROM_REGION( 0x320, "proms", 0 ) // PROMs (function unknown)
	ROM_LOAD( "mod_1-4_105_82s123n.ic20",   0x00000, 0x00020, CRC(14d72781) SHA1(372dc021d8aaf4aa6fd46e69a3d8f1c68113426f) )
	ROM_LOAD( "mod_51-1_p0502_8s2129.ic10", 0x00020, 0x00100, CRC(15085e44) SHA1(646e7100fcb112594023cf02be036bd3d42cc13c) )
	ROM_LOAD( "mod_21-1_209_82s129.ic12",   0x00120, 0x00100, CRC(204a7aee) SHA1(322164134aa65c37a9389024f921364a81d13e88) )
	ROM_LOAD( "mod_21-1_p0204_82s129.ic4",  0x00220, 0x00100, CRC(74470450) SHA1(40b0e0991090733f8190ad7efcb500bd109c2a7e) )

	ROM_REGION( 0x1000, "plds", ROMREGION_ERASEFF )
	ROM_LOAD( "mod_6-1_606_gal16v8.ic13",   0x00000, 0x00117, NO_DUMP )
	ROM_LOAD( "mod_6-1_649_gal16v8.ic7",    0x00000, 0x00117, NO_DUMP )
	ROM_LOAD( "mod_4-3_p0403_pal16r8.ic29", 0x00000, 0x00104, CRC(506156cc) SHA1(5560671fc2c9872ed28620491af5dc486909fc6e) )
	ROM_LOAD( "mod_51-1_503_gal16v8.ic46",  0x00000, 0x00104, NO_DUMP )
	ROM_LOAD( "mod_5-1_5149_gal16v8.ic9",   0x00000, 0x00117, NO_DUMP )
	ROM_LOAD( "mod_5-1_5249_gal16v8.ic8",   0x00000, 0x00117, NO_DUMP )
ROM_END

} // anonymous namespace


GAME( 199?, pspikesm, pspikes, pspikesm, pspikesm, pspikes_ms_state, empty_init, ROT0, "bootleg (Gaelco / Ervisa)", "Power Spikes (Modular System)", MACHINE_IS_SKELETON )
