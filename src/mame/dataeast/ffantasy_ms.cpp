// license:BSD-3-Clause
// copyright-holders:

/*
    Fighting Fantasy (Modular System)
    Dragon Ninja (Modular System)
    Secret Agent (Modular System)
    Automat (Modular System) [bootleg of Robocop]

    As with most of the 'Modular System' setups, the hardware is heavily modified from the original
    and consists of a multi-board stack in a cage, hence different driver.


    For Fighting Fantasy the Modular System cage contains 8 main boards and 1 sub board.

    MOD-6/1 - TSC68000CP12, 4 ROMs, RAMs, 20 MHz XTAL.
    MOD 21/1(?) - 20 MHz XTAL.
    MOD 1/5 - Sound board (Z80B, 2 x YM2203C). 2 8-dips banks + small sub board with OKI M5205.
    MOD 51/3 - Sprite board, has logic + 4 empty ROM sockets. Sprite ROMs are actually on the below board.
    MODULAR SYSTEM 2 MOD 5/1 - red sprite ROM board, 8 sprite ROMs populated (maximum 24 ROMs)
    MOD 4/3 - Tilemap board, has logic + 4 tilemap ROMs, long thin sub-board (C0467 SOLD) with no chips, just routing along one edge.
    MOD 4/3 - Tilemap board, has logic + 4 tilemap ROMs, long thin sub-board (C0468 SOLD) with no chips, just routing along one edge.
    MOD 4/3 - Tilemap board, has logic + 4 tilemap ROMs, long thin sub-board (C0469 SOLD) with no chips, just routing along one edge.

    PCBs pictures and dip listing are available at: http://www.recreativas.org/modular-system-fighting-fantasy-5694-gaelco-sa


    For Dragon Ninja the Modular System cage contains 7 main boards and 1 sub board.

    MOD-6/1 - MC68000P10, 2 ROMs, RAMs, 20 MHz XTAL.
    MOD 21/1(?) - 20 MHz XTAL.
    MOD 1/B 4 - Sound board (Z0840006PSC, 2 x YM2203C). 2 8-dips banks + small sub board with OKI M5205.
    MOD 51/3 - Sprite board, has logic + 4 ROM, short thin sub-board (C0528) with no chips, just routing along one edge.
    MOD 4/3 - Tilemap board, has logic + 4 tilemap ROMs, long thin sub-board (C0462) with no chips, just routing along one edge.
    MOD 4/3 - Tilemap board, has logic + 4 tilemap ROMs, long thin sub-board (C0463) with no chips, just routing along one edge.
    MOD 4/2 - Tilemap board, has logic + 4 tilemap ROMs, long thin sub-board (C0464) with no chips, just routing along one edge.

    PCBs pictures and dip listing are available at: https://www.recreativas.org/modular-system-dragon-ninja-4319-gaelco-sa


    For Secret Agent the Modular System cage contains 8 main boards and 1 sub board.

    MOD-6/1 - MC68000P10, 4 ROMs, RAMs, 22.1184 MHz XTAL.
    MOD 21/1(?) - 20 MHz XTAL.
    MOD 1/2 - Sound board (NEC D780C-2 Z80 compatible, 2 x YM2203C, 2 x Y3014B). 2 8-dips banks + small sub board with OKI M5205.
    MOD 51/3 - Sprite board, has logic + 4 empty ROM sockets. Sprite ROMs are actually on the below board.
    MODULAR SYSTEM 2 MOD 5/1 - red sprite ROM board, 8 sprite ROMs populated (maximum 24 ROMs)
    MOD 4/3 - Tilemap board, has logic + 4 tilemap ROMs, long thin sub-board (C0462) with no chips, just routing along one edge.
    MOD 4/3 - Tilemap board, has logic + 4 tilemap ROMs, long thin sub-board (C0463) with no chips, just routing along one edge.
    MOD 4/2 - Tilemap board, has logic + 4 tilemap ROMs, long thin sub-board (C0464) with no chips, just routing along one edge.

    PCBs pictures and dip listing are available at: https://www.recreativas.org/modular-system-secret-agent-11088-gaelco-sa


    For Automat the Modular System cage contains 6 main boards and 1 sub board.

    MOD 1/5 - Sound board (Z80B, 2 x YM2203C). Two 8 dip switches banks + small sub board with OKI M5205.
    MOD 6/1 - TSC68000CP12, 4 ROMs, RAMs, 20 MHz XTAL.
    MOD 2   - Logic + 20 MHz XTAL.
    MOD 7/8 - Logic + RAM (2 x + Sony CXK58256PM-10) + PLDs.
    MOD 8   - ROM (eight EPROMs)
    MOD 51/3 - Sprite board, has logic + 4 EPROMs.

    PCBs pictures and dip listing are available at: https://www.recreativas.org/modular-system-automat-robocop-14508-gaelco-sa
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


namespace {

class ffantasy_ms_state : public driver_device
{
public:
	ffantasy_ms_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen")
	{ }

	void ffantasym(machine_config &config);
	void secretagm(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void ffantasym_map(address_map &map) ATTR_COLD;
};


void ffantasy_ms_state::ffantasym_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0xff8000, 0xffbfff).ram();
}


void ffantasy_ms_state::machine_start()
{
}


uint32_t ffantasy_ms_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}

static INPUT_PORTS_START( ffantasym )
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

static GFXDECODE_START( gfx_ffantasy_ms )
GFXDECODE_END

void ffantasy_ms_state::ffantasym(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 20_MHz_XTAL / 2); // divisor unknown
	m_maincpu->set_addrmap(AS_PROGRAM, &ffantasy_ms_state::ffantasym_map);

	Z80(config, "audiocpu", 20_MHz_XTAL / 5).set_disable(); // divisor unknown

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER); // all wrong
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500) /* not accurate */);
	m_screen->set_size(256, 256);
	m_screen->set_visarea(0, 256-1, 0, 256-32-1);
	m_screen->set_screen_update(FUNC(ffantasy_ms_state::screen_update));
	m_screen->set_palette("palette");

	PALETTE(config, "palette").set_format(palette_device::xBRG_444, 1024);

	GFXDECODE(config, "gfxdecode", "palette", gfx_ffantasy_ms);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, "soundlatch");

	YM2203(config, "ym1", 20_MHz_XTAL / 5).add_route(ALL_OUTPUTS, "mono", 0.15); // divisor unknown

	YM2203(config, "ym2", 20_MHz_XTAL / 5).add_route(ALL_OUTPUTS, "mono", 0.15); // divisor unknown

	MSM5205(config, "msm", 20_MHz_XTAL / 5).add_route(ALL_OUTPUTS, "mono", 0.15); // divisor unknown
}

void ffantasy_ms_state::secretagm(machine_config &config)
{
	ffantasym(config);

	// Exactly the same hardware configuration as 'ffantasym', but with a 22.1184 MHz xtal on the CPU PCB.
	m_maincpu->set_clock(22.1184_MHz_XTAL);
}

ROM_START( automatm )
	ROM_REGION( 0x100000, "maincpu", 0 ) // on MOD 6/1 board
	ROM_LOAD16_BYTE( "mod_6-1_automat_2au_603.ic17", 0x00000, 0x10000, CRC(efce0f8e) SHA1(c27a2db6c5cf92b9682be12d00d4c8b2998a88ee) )
	ROM_LOAD16_BYTE( "mod_6-1_automat_2au_606.ic26", 0x00001, 0x10000, CRC(f19fc5b6) SHA1(3d27ed69a4ab4d799c4afa779ac5d5083cd73cc2) )
	ROM_LOAD16_BYTE( "mod_6-1_automat_2au_602.ic20", 0x20000, 0x10000, CRC(9d7b79e0) SHA1(e0d901b9b3cd62f7c947da04f7447ebfa88bf44a) )
	ROM_LOAD16_BYTE( "mod_6-1_automat_2au_605.ic11", 0x20001, 0x10000, CRC(e655f9c3) SHA1(d5e99d542303d009277ccfc245f877e4e28603c9) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // on MOD 1/5 board
	ROM_LOAD( "mod_1-5_automat_2au_101.ic12", 0x00000, 0x10000, CRC(fc80cac1) SHA1(0f31d6c3f2ca1aa1c6d9ac5da28b75a6fbf8444b) )

	ROM_REGION( 0x80000, "gfx1", 0 ) // on MOD 8 board
	ROM_LOAD( "mod_8_automat_2au_301.ic15", 0x00000, 0x10000, CRC(fd3b363b) SHA1(af63d57e362f159c5b54a764e2069ca24849164b) )
	ROM_LOAD( "mod_8_automat_2au_302.ic22", 0x10000, 0x10000, CRC(05755c02) SHA1(e1fa7d464eb7bbf2f22ca132e7df8e92911a0685) )
	ROM_LOAD( "mod_8_automat_2au_303.ic30", 0x20000, 0x10000, CRC(9440bb38) SHA1(aee9372bfc3b2b036a208d91a706312b77042001) )
	ROM_LOAD( "mod_8_automat_2au_304.ic37", 0x30000, 0x10000, CRC(a1c9d781) SHA1(8e062af8a9c2eef74c8e578624f3c961750ac260) )
	ROM_LOAD( "mod_8_automat_2au_305.ic14", 0x40000, 0x10000, CRC(53cb118e) SHA1(cf2329cb8f41c6ddc81dea8fa8def0e47e25b723) )
	ROM_LOAD( "mod_8_automat_2au_306.ic21", 0x50000, 0x10000, CRC(21b682f7) SHA1(1e9bdae1317f215ffdffdd2c1a3c0e08c14ab00c) )
	ROM_LOAD( "mod_8_automat_2au_307.ic29", 0x60000, 0x10000, CRC(be822d87) SHA1(d5f9bb55a16f0ab2d6d11fea5d22b7ced59f9128) )
	ROM_LOAD( "mod_8_automat_2au_308.ic36", 0x70000, 0x10000, CRC(9459159a) SHA1(20fc3f33f2e23d88b2bd21f248ed7814f03b387f) )

	ROM_REGION( 0x80000, "gfx2", 0 ) // on MOD 51/1 board
	ROM_LOAD( "mod_51-3_automat_2au_501.ic43", 0x00000, 0x20000, CRC(8e7ec3fc) SHA1(0c40cf90bb105dca2629a61ff13227b82d3b10de) )
	ROM_LOAD( "mod_51-3_automat_2au_502.ic42", 0x20000, 0x20000, CRC(d0a46eae) SHA1(3efe3b1462b442b4a16704d15e201a4d7bd402c8) )
	ROM_LOAD( "mod_51-3_automat_2au_503.ic41", 0x40000, 0x20000, CRC(e174cdaf) SHA1(e66c8adc6df08e30365bd0e360e78397b495582c) )
	ROM_LOAD( "mod_51-3_automat_2au_504.ic40", 0x60000, 0x20000, CRC(e8662060) SHA1(f59f55fe5bf79be0c66f4984a0ecab17504438c7) )

	ROM_REGION( 0x120, "proms", 0 )    // PROMs (function unknown)
	ROM_LOAD( "mod_1-5_automat_110_82s123.ic20", 0x000, 0x020, NO_DUMP )
	ROM_LOAD( "mod_51-3_a502_63s141.ic10",       0x020, 0x100, NO_DUMP )

	ROM_REGION( 0x200, "plds", ROMREGION_ERASEFF )
	ROM_LOAD( "mod_6-1_automat_604_gal16v8.ic13", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "mod_6-1_automat_633_gal16v8.ic7",  0x000, 0x117, NO_DUMP )
	ROM_LOAD( "mod_7-8_7133_gal20v8.ic7",         0x000, 0x157, NO_DUMP )
	ROM_LOAD( "mod_7-8_7233_gal20v8.ic54",        0x000, 0x157, NO_DUMP )
	ROM_LOAD( "mod_7-8_7333_gal16v8.ic55",        0x000, 0x117, NO_DUMP )
	ROM_LOAD( "mod_7-8_7433_gal16v8.ic9",         0x000, 0x117, NO_DUMP )
	ROM_LOAD( "mod_7-8_7533_gal16v8.ic59",        0x000, 0x117, NO_DUMP )
	ROM_LOAD( "mod_7-8_7633_gal20v9.ic44",        0x000, 0x157, NO_DUMP )
ROM_END

ROM_START( drgninjam )
	ROM_REGION( 0x100000, "maincpu", 0 ) // on MOD 6/1 board, extremely similar to drgninjab in dec0.cpp. TODO: ROM loading may be wrong (check 0x20000 - 0x3ffff being empty in drgninjab)
	ROM_LOAD16_BYTE( "6-1_cpu_dn_603.ic17",  0x00000, 0x20000, CRC(1c3670df) SHA1(0e287a0b4494e702080e80a911c4762bbc6a5815) )
	ROM_LOAD16_BYTE( "6-1_cpu_dn_606.ic8",   0x00001, 0x20000, CRC(07669458) SHA1(c5dcf72eba5e345228ee3f9e0228cba526e1156a) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // on MOD 1/2 board
	ROM_LOAD( "1-4_snd_dn_101.ic12",  0x00000, 0x10000, CRC(0b82c205) SHA1(0be243b19693c54914eccefbf6a8b8b513eec299) )

	ROM_REGION( 0x80000, "gfx1", 0 ) // on one of the MOD 4/3 boards
	ROM_LOAD( "4-3_dn_401.ic17",  0x0000,  0x4000, CRC(45ab2822) SHA1(0cf0f74ad5325a66f74fc20cabd6a9d3d8c42f2a) )
	ROM_LOAD( "4-3_dn_402.ic16",  0x4000,  0x4000, CRC(26f0095b) SHA1(9e568a23df2b0ca05d8b87fea445f5f98e55fac5) )
	ROM_LOAD( "4-3_dn_403.ic15",  0x8000,  0x4000, CRC(c2061c37) SHA1(b06737ecece7da4785c2e98977f6dc35986a445e) ) // 1ST AND 2ND HALF IDENTICAL
	ROM_IGNORE(                            0x4000 )
	ROM_LOAD( "4-3_dn_404.ic14",  0xc000,  0x4000, CRC(1da44e82) SHA1(ccb9464b711025a5c2c18ecb765f7f0e15f7d4a5) ) // 1ST AND 2ND HALF IDENTICAL
	ROM_IGNORE(                            0x4000 )

	ROM_REGION( 0x80000, "gfx2", 0 ) // on a second MOD 4/3 board
	ROM_LOAD( "4-3-a_dn_4a01.ic17",  0x00000, 0x8000, CRC(c89982ac) SHA1(7509322bf4dcaceefdbaf6e9f4087a576133fe39) )
	ROM_LOAD( "4-3-a_dn_4a02.ic16",  0x08000, 0x8000, CRC(144b0359) SHA1(46565c5694f0cfcf06c1ee7c131ab3c88c804d35) )
	ROM_LOAD( "4-3-a_dn_4a03.ic15",  0x10000, 0x8000, CRC(ae98a684) SHA1(d6fe7237b82125a21b9804500ba2166906475a0e) )
	ROM_LOAD( "4-3-a_dn_4a04.ic14",  0x18000, 0x8000, CRC(8292c4ad) SHA1(f83acf64b101022927b678749d754374a20f5444) )

	ROM_REGION( 0x80000, "gfx3", 0 ) // on a MOD 4/2 board
	ROM_LOAD( "4-2-b_dn_4b01.ic17",  0x00000, 0x10000, CRC(65002e5c) SHA1(e1fa0a0395d3fbcf31e15b63b7eaf478063aa971) )
	ROM_LOAD( "4-2-b_dn_4b02.ic16",  0x10000, 0x10000, CRC(dd6acd2d) SHA1(e2a0ccf49dce421b10ad0ed54d02ba1ca6525404) )
	ROM_LOAD( "4-2-b_dn_4b03.ic15",  0x20000, 0x10000, CRC(63e337c1) SHA1(ca347d6a46ee643edfdc8614760dc9b0994e1745) )
	ROM_LOAD( "4-2-b_dn_4b04.ic14",  0x30000, 0x10000, CRC(5cc7ec8c) SHA1(3f17e13af7152c51ce025ca06ece70b4a6f65f94) )

	ROM_REGION( 0x100000, "gfx4", 0 ) // on MOD 51/1 board
	ROM_LOAD( "51-3_dn_501.ic43",  0x00000, 0x20000, CRC(0fccce1f) SHA1(e2e5625b62ddd73a4363596a4b1b2d72c2fe1c38) )
	ROM_LOAD( "51-3_dn_502.ic42",  0x20000, 0x20000, CRC(361f4616) SHA1(40279c76d027bcf698b8ff694afc2afb15279381) )
	ROM_LOAD( "51-3_dn_503.ic41",  0x40000, 0x20000, CRC(b3c97ad1) SHA1(f77426e1ee347b8cae9c5aef1e31058344c019c2) )
	ROM_LOAD( "51-3_dn_504.ic40",  0x60000, 0x20000, CRC(1445dccb) SHA1(fddbb9d136ea6dbb2704ac93e5536ed0e6bfb19c) )

	ROM_REGION( 0x0400, "proms", 0 ) // PROMs (function unknown)
	ROM_LOAD( "1-4_snd_110_82s123.ic20",  0x000, 0x020, CRC(e26e680a) SHA1(9bbe30e98e952a6113c64e1171330153ddf22ce7) )
	ROM_LOAD( "21-1_p0201_tbp24s10n.ic4", 0x100, 0x100, CRC(2697da58) SHA1(e62516b886ff6e204b718e5f0c6ce2712e4b7fc5) )
	ROM_LOAD( "21-1_p0205_82s129n.ic12",  0x200, 0x100, CRC(204a7aee) SHA1(322164134aa65c37a9389024f921364a81d13e88) )
	ROM_LOAD( "51-3_502_82s129.ic10",     0x300, 0x100, CRC(15085e44) SHA1(646e7100fcb112594023cf02be036bd3d42cc13c) )

	ROM_REGION( 0xc00, "plds", ROMREGION_ERASEFF )
	ROM_LOAD( "6-1_cpu_604_gal16v8.ic13", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "6-1_cpu_630_gal16v8.ic7",  0x200, 0x117, NO_DUMP )
	ROM_LOAD( "4-3_p0403_pal16r8a.ic29",  0x400, 0x104, CRC(506156cc) SHA1(5560671fc2c9872ed28620491af5dc486909fc6e) )
	ROM_LOAD( "4-3-a_p0403.ic29",         0x600, 0x104, CRC(506156cc) SHA1(5560671fc2c9872ed28620491af5dc486909fc6e) )
	ROM_LOAD( "4-2-b_403_gal16v8.ic29",   0x800, 0x104, NO_DUMP )
	ROM_LOAD( "51-3_503_gal16v8.ic46",    0xa00, 0x117, CRC(11470ea1) SHA1(cfcafbcc7e55be717348f895df61e144fdd0cc9b) )
ROM_END

ROM_START( ffantasym )
	ROM_REGION( 0x100000, "maincpu", 0 ) // on MOD 6/1 board, extremely similar to ffantasybl in dec0.cpp
	ROM_LOAD16_BYTE( "ff_6-1_5fa_603.ic17",  0x00000, 0x10000, CRC(124ebff8) SHA1(20423990903dc3d682e2df6fb4dec0ae6b49036d) )
	ROM_LOAD16_BYTE( "ff_6-1_5fa_606.ic8",   0x00001, 0x10000, CRC(94fa4a64) SHA1(fa1405144fc525debd1b62f78b5434cf7670e1cb) )
	ROM_LOAD16_BYTE( "ff_6-1_5fa_602.ic20",  0x20000, 0x10000, CRC(68d50c5d) SHA1(d4a96c9ecb565c3c84ca76537492101264007cc1) )
	ROM_LOAD16_BYTE( "ff_6-1_5fa_605.ic11",  0x20001, 0x10000, CRC(dee5475e) SHA1(57a063834dc2fe97f4372eee06114f873a372786) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // on MOD 1/2 board
	ROM_LOAD( "ff_1-5_5fa_101.ic12",  0x00000, 0x10000, CRC(891bc223) SHA1(737248ea23c42076da9ce6c4abb9506f396892a0) )

	ROM_REGION( 0x80000, "gfx1", 0 ) // on one of the MOD 4/3 boards
	ROM_LOAD( "ff_4-3_5fa_404.ic14",  0x00000, 0x8000, CRC(819d8197) SHA1(1303d94f6b3048e1108d25662de971a77f6e3ae6) )
	ROM_LOAD( "ff_4-3_5fa_402.ic16",  0x08000, 0x8000, CRC(fd8030d7) SHA1(84a9d775c0be1d068795ae8889cb5cec2842d622) )
	ROM_LOAD( "ff_4-3_5fa_403.ic15",  0x10000, 0x8000, CRC(0c572113) SHA1(3d035f1a985d1e04e64eb124c83c44e17bc07d53) )
	ROM_LOAD( "ff_4-3_5fa_401.ic17",  0x18000, 0x8000, CRC(26f57324) SHA1(079745385f1240edf4ce712262352547b20282d2) )

	ROM_REGION( 0x80000, "gfx2", 0 ) // on a second MOD 4/3 board
	ROM_LOAD( "ff_4-3-a_5fa_4a1.ic17",  0x00000, 0x8000, CRC(4be99b3a) SHA1(d4c576dda0bd855c8f361c96cd4615815c8c36c8) )
	ROM_LOAD( "ff_4-3-a_5fa_4a2.ic16",  0x08000, 0x8000, CRC(206239f1) SHA1(f5d2f7c888f01db57c9b78eff733b22301836025) )
	ROM_LOAD( "ff_4-3-a_5fa_4a3.ic15",  0x10000, 0x8000, CRC(c3eeb8a4) SHA1(c097034e2d3c3f55c935aff1d2c14a1dacc1f5da) )
	ROM_LOAD( "ff_4-3-a_5fa_4a4.ic14",  0x18000, 0x8000, CRC(a4a8ba47) SHA1(2bf3379a33850703db49cbcdd61d5e4777006726) )

	ROM_REGION( 0x80000, "gfx3", 0 ) // on a third MOD 4/3 board
	ROM_LOAD( "ff_4-3-b_4fa_4b1.ic17",  0x00000, 0x8000, CRC(51c274f0) SHA1(145a2da91db9d26a8ecaa6d4267c9aa0e4883fdc) )
	ROM_LOAD( "ff_4-3-b_4fa_4b2.ic16",  0x08000, 0x8000, CRC(53fade55) SHA1(d7ae60c908338f746fb585bf81c89036a6a9c8e0) )
	ROM_LOAD( "ff_4-3-b_4fa_4b3.ic15",  0x10000, 0x8000, CRC(49668a54) SHA1(f6129aa46f8e766a5012fe1b90c98a78581a773b) )
	ROM_LOAD( "ff_4-3-b_4fa_4b4.ic14",  0x18000, 0x8000, BAD_DUMP CRC(bd0d2e07) SHA1(3165fb23ee9cd5912fd17e557dd0d1513bd189ef) )

	ROM_REGION( 0x100000, "gfx4", 0 ) // on MOD 51/1 board
	ROM_LOAD( "ff_5-1_5fa_505.ic5",   0x20000, 0x10000, CRC(24503b71) SHA1(4b303bbfa7f7ede1679cdf660e5cf53806ca3531) )
	ROM_LOAD( "ff_5-1_5fa_506.ic6",   0x30000, 0x10000, CRC(b47d3b99) SHA1(ee6c79a93a0640692fcc49a0c247d15f35e3a7ca) )
	ROM_LOAD( "ff_5-1_5fa_511.ic14",  0x60000, 0x10000, CRC(01b8cc18) SHA1(434989b5cf00845788d4a855fc027684e8c79ace) )
	ROM_LOAD( "ff_5-1_5fa_512.ic15",  0x70000, 0x10000, CRC(36fa650e) SHA1(8226bba7d75a5112883e30c8012d18bc7c1acd11) )
	ROM_LOAD( "ff_5-1_5fa_517.ic20",  0xa0000, 0x10000, CRC(ed7d42e5) SHA1(52a453da9bbf11846bbe8e1121b56a869b93ac65) )
	ROM_LOAD( "ff_5-1_5fa_518.ic21",  0xb0000, 0x10000, CRC(1106d842) SHA1(002b785ed575c3ae6514d866c070dab83323a8b6) )
	ROM_LOAD( "ff_5-1_5fa_523.ic26",  0xe0000, 0x10000, CRC(a541c879) SHA1(53133cb2ad5378f101025fc758d08a54e23f3c87) )
	ROM_LOAD( "ff_5-1_5fa_524.ic27",  0xf0000, 0x10000, CRC(1e7351c5) SHA1(88d83b45f25fe58726e5158b97395443cf065564) )

	ROM_REGION( 0x0400, "proms", 0 )    // PROMs (function unknown)
	ROM_LOAD( "ff_1-5_110_82s123.ic20",  0x000, 0x020, CRC(e26e680a) SHA1(9bbe30e98e952a6113c64e1171330153ddf22ce7) )
	ROM_LOAD( "ff_21-1_201_82s129.ic4",  0x100, 0x100, CRC(2697da58) SHA1(e62516b886ff6e204b718e5f0c6ce2712e4b7fc5) )
	ROM_LOAD( "ff_21-1_205_82s129.ic12", 0x200, 0x100, CRC(204a7aee) SHA1(322164134aa65c37a9389024f921364a81d13e88) )
	ROM_LOAD( "ff_51-3_502_82s129.ic10", 0x300, 0x100, CRC(15085e44) SHA1(646e7100fcb112594023cf02be036bd3d42cc13c) )

	ROM_REGION( 0x1000, "plds", ROMREGION_ERASEFF )
	ROM_LOAD( "ff_6-1_604_gal16v8.ic13",       0x000, 0x117, NO_DUMP )
	ROM_LOAD( "ff_6-1_635_gal16v8.ic7",        0x000, 0x117, NO_DUMP )
	ROM_LOAD( "ff_4-3_p0403_pal16r8a.ic29",    0x000, 0x104, CRC(506156cc) SHA1(5560671fc2c9872ed28620491af5dc486909fc6e) )
	ROM_LOAD( "ff_4-3-a_p0402_pal16r8a.ic29",  0x000, 0x104, CRC(506156cc) SHA1(5560671fc2c9872ed28620491af5dc486909fc6e) )
	ROM_LOAD( "ff_4-3-b_p0403_pal16r8a.ic29",  0x000, 0x104, CRC(506156cc) SHA1(5560671fc2c9872ed28620491af5dc486909fc6e) )
	ROM_LOAD( "ff_51-3_p0503_pal16r6a.ic46",   0x000, 0x104, CRC(07eb86d2) SHA1(482eb325df5bc60353bac85412cf45429cd03c6d) )
	ROM_LOAD( "ff_5-1_5135_gal16v8.ic9",       0x000, 0x117, NO_DUMP )
	ROM_LOAD( "ff_5-1_5235_gal16v8.ic8",       0x000, 0x117, NO_DUMP )
ROM_END

ROM_START( secretagm )
	ROM_REGION( 0x100000, "maincpu", 0 ) // on MOD 6/1 board
	ROM_LOAD16_BYTE( "mod_6-1_ag_603.ic17", 0x00000, 0x10000, CRC(4a61489b) SHA1(41f3b99bbd60ebac567477b98dc7c0158f972d60) )
	ROM_LOAD16_BYTE( "mod_6-1_ag_606.ic8",  0x00001, 0x10000, CRC(94dfe5a1) SHA1(01a4c25fa883b0a4bea5d8555e38eb40c302b6d5) )
	ROM_LOAD16_BYTE( "mod_6-1_ag_602.ic20", 0x20000, 0x10000, CRC(bc67dfa0) SHA1(0e32ab1ef9008e95c2e0b8a6916766b255681987) )
	ROM_LOAD16_BYTE( "mod_6-1_ag_605.ic11", 0x20001, 0x10000, CRC(ba149ce8) SHA1(6c405a670983a5c3b5c98248298924b87c350274) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // on MOD 1/2 board
	ROM_LOAD( "mod_1-2_ag_110.ic12", 0x00000, 0x10000, CRC(7446d17e) SHA1(be0c2c5359af20e17f76c0a5ea59de9357735ab8) )

	ROM_REGION( 0x80000, "gfx1", 0 ) // on one of the MOD 4/3 boards
	ROM_LOAD( "mod_4-3_0_ag_404.ic14", 0x00000, 0x8000, CRC(4b00d933) SHA1(dd8113f24f8a9f408fbf3e955c8bf61c89744e53) )
	ROM_LOAD( "mod_4-3_0_ag_402.ic16", 0x08000, 0x8000, CRC(7a4bf6b0) SHA1(63a1d0e10903fc2548a24126961d057e8b5806bd) )
	ROM_LOAD( "mod_4-3_0_ag_403.ic15", 0x10000, 0x8000, CRC(dec5f0dc) SHA1(6df6c0d1c3f83b0cc896083f0dce3ce93f67a49b) )
	ROM_LOAD( "mod_4-3_0_ag_401.ic17", 0x18000, 0x8000, CRC(0a56829f) SHA1(444d3ef0c701810d8f2a53fc42565d5d04073b1c) )

	ROM_REGION( 0x80000, "gfx2", 0 ) // on a second MOD 4/3 board
	ROM_LOAD( "mod_4-3_a_ag_4a1.ic17", 0x00000, 0x8000, CRC(58e496f2) SHA1(2cabd84bff6047be81f79c3cd55f480828aa3d3d) )
	ROM_LOAD( "mod_4-3_a_ag_4a2.ic16", 0x08000, 0x8000, CRC(3bbd9e82) SHA1(79926cd1f227762174de50f9af62f54060021a63) )
	ROM_LOAD( "mod_4-3_a_ag_4a3.ic15", 0x10000, 0x8000, CRC(3fd968a0) SHA1(ddecb3f01ce6227653607878c933bd17c96483e6) )
	ROM_LOAD( "mod_4-3_a_ag_4a4.ic14", 0x18000, 0x8000, CRC(960007ad) SHA1(9e7058385110acc149ba42c8ec64bd871debd0b9) )

	ROM_REGION( 0x80000, "gfx3", 0 ) // on a third MOD 4/3 board
	ROM_LOAD( "mod_4-3_b_ag_4b1.ic17", 0x00000, 0x10000, CRC(e06bc56d) SHA1(5a189c7f6e131fc6d6eebd2bb1c9c8c3cff9d6f8) )
	ROM_LOAD( "mod_4-3_b_ag_4b2.ic16", 0x10000, 0x10000, CRC(f46fd920) SHA1(925f0c9c5d2990dcff09a18363b88a69c19f0d01) )
	ROM_LOAD( "mod_4-3_b_ag_4b3.ic15", 0x20000, 0x10000, CRC(51ef5441) SHA1(eb06022174e177f6785d1b621e6e3fdaca67c122) )
	ROM_LOAD( "mod_4-3_b_ag_4b4.ic14", 0x30000, 0x10000, CRC(3beeddde) SHA1(1127d78a9b29787aaae47e0b4359be7b2528f6d9) )

	ROM_REGION( 0x100000, "gfx4", 0 ) // on MOD 51/1 board
	ROM_LOAD( "mod_5-1_ag_501.ic5",  0x20000, 0x10000, CRC(d234cae5) SHA1(0cd07bf087a4da19a5da29785385de9eee52d0fb) )
	ROM_LOAD( "mod_5-1_ag_502.ic6",  0x30000, 0x10000, CRC(dc6a38df) SHA1(9043df911389d3f085299f2f2202cab356473a32) )
	ROM_LOAD( "mod_5-1_ag_503.ic14", 0x60000, 0x10000, CRC(447e4f0b) SHA1(97db103e505a6e11eb9bdb3622e4aa3b796a9714) )
	ROM_LOAD( "mod_5-1_ag_504.ic15", 0x70000, 0x10000, CRC(d29bc22e) SHA1(ce0935d09f7e94fa32247c86e14a74b73514b29e) )
	ROM_LOAD( "mod_5-1_ag_505.ic20", 0xa0000, 0x10000, CRC(bd17ddc5) SHA1(edb847378c4a95b18bd3b49b6cbf8c06598d9d3b) )
	ROM_LOAD( "mod_5-1_ag_506.ic21", 0xb0000, 0x10000, CRC(f61972c8) SHA1(fa9ddca3473091b4879171d8f3b302e8f2b45149) )
	ROM_LOAD( "mod_5-1_ag_507.ic26", 0xe0000, 0x10000, CRC(ff72b838) SHA1(fdc48ecdd2225fc69472313f34973f6add8fb558) )
	ROM_LOAD( "mod_5-1_ag_508.ic27", 0xf0000, 0x10000, CRC(54fcbc39) SHA1(293a6799193b01424c3eac86cf90cc023aa771db) )

	ROM_REGION( 0x0400, "proms", 0 )    // PROMs (function unknown)
	ROM_LOAD( "mod_1-2_110_82s123.ic20",    0x000, 0x020, CRC(e26e680a) SHA1(9bbe30e98e952a6113c64e1171330153ddf22ce7) )
	ROM_LOAD( "mod_21-1_201_82s129.ic4",    0x100, 0x100, CRC(2697da58) SHA1(e62516b886ff6e204b718e5f0c6ce2712e4b7fc5) )
	ROM_LOAD( "mod_21-1_205_82s129.ic12",   0x200, 0x100, CRC(204a7aee) SHA1(322164134aa65c37a9389024f921364a81d13e88) )
	ROM_LOAD( "mod_51-3_p0502_82s129.ic10", 0x300, 0x100, CRC(15085e44) SHA1(646e7100fcb112594023cf02be036bd3d42cc13c) )

	ROM_REGION( 0x200, "plds", ROMREGION_ERASEFF )
	ROM_LOAD( "mod_6-1_604_gal16v8.ic13",      0x000, 0x117, NO_DUMP )
	ROM_LOAD( "mod_6-1_637_gal16v8.ic7",       0x000, 0x117, NO_DUMP )
	ROM_LOAD( "mod_4-3_0_10403_pal16r8a.ic29", 0x000, 0x104, CRC(506156cc) SHA1(5560671fc2c9872ed28620491af5dc486909fc6e) )
	ROM_LOAD( "mod_4-3_a_p0403_pal16r8.ic29",  0x000, 0x104, CRC(506156cc) SHA1(5560671fc2c9872ed28620491af5dc486909fc6e) )
	ROM_LOAD( "mod_4-3_b_403_pal16r8.ic29",    0x000, 0x104, CRC(506156cc) SHA1(5560671fc2c9872ed28620491af5dc486909fc6e) )
	ROM_LOAD( "mod_51-3_503_gal16v8.ic46",     0x000, 0x117, CRC(11470ea1) SHA1(cfcafbcc7e55be717348f895df61e144fdd0cc9b) )
	ROM_LOAD( "mod_5-1_s137_gal16v8.ic9",      0x000, 0x117, NO_DUMP )
	ROM_LOAD( "mod_5-1_s237_gal16v8.ic8",      0x000, 0x117, NO_DUMP )
ROM_END

} // anonymous namespace


GAME( 199?, automatm,  robocop,  ffantasym, ffantasym, ffantasy_ms_state, empty_init, ROT0, "bootleg (Gaelco / Ervisa)", "Automat (bootleg of Robocop, Modular System)", MACHINE_IS_SKELETON )
GAME( 199?, drgninjam, baddudes, ffantasym, ffantasym, ffantasy_ms_state, empty_init, ROT0, "bootleg (Gaelco / Ervisa)", "Dragon Ninja (Modular System)",                MACHINE_IS_SKELETON )
GAME( 199?, ffantasym, hippodrm, ffantasym, ffantasym, ffantasy_ms_state, empty_init, ROT0, "bootleg (Gaelco / Ervisa)", "Fighting Fantasy (Modular System)",            MACHINE_IS_SKELETON )
GAME( 199?, secretagm, secretag, secretagm, ffantasym, ffantasy_ms_state, empty_init, ROT0, "bootleg (Gaelco / Ervisa)", "Secret Agent (Modular System)",                MACHINE_IS_SKELETON )
