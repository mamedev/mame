// license:BSD-3-Clause
// copyright-holders:

/*
    Fighting Fantasy (Modular System)
    Dragon Ninja (Modular System)

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

protected:
	virtual void machine_start() override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void ffantasym_map(address_map &map);
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
	/* basic machine hardware */
	M68000(config, m_maincpu, 20_MHz_XTAL / 2); // divisor unknown
	m_maincpu->set_addrmap(AS_PROGRAM, &ffantasy_ms_state::ffantasym_map);

	Z80(config, "audiocpu", 20_MHz_XTAL / 5).set_disable(); // divisor unknown

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER); // all wrong
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500) /* not accurate */);
	m_screen->set_size(256, 256);
	m_screen->set_visarea(0, 256-1, 0, 256-32-1);
	m_screen->set_screen_update(FUNC(ffantasy_ms_state::screen_update));
	m_screen->set_palette("palette");

	PALETTE(config, "palette").set_format(palette_device::xBRG_444, 1024);

	GFXDECODE(config, "gfxdecode", "palette", gfx_ffantasy_ms);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, "soundlatch");

	YM2203(config, "ym1", 20_MHz_XTAL / 5).add_route(ALL_OUTPUTS, "mono", 0.15); // divisor unknown

	YM2203(config, "ym2", 20_MHz_XTAL / 5).add_route(ALL_OUTPUTS, "mono", 0.15); // divisor unknown

	MSM5205(config, "msm", 20_MHz_XTAL / 5).add_route(ALL_OUTPUTS, "mono", 0.15); // divisor unknown
}

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
	ROM_LOAD( "51-3_dn_502.ic42",  0x40000, 0x20000, CRC(b3c97ad1) SHA1(f77426e1ee347b8cae9c5aef1e31058344c019c2) )
	ROM_LOAD( "51-3_dn_504.ic40",  0x60000, 0x20000, CRC(1445dccb) SHA1(fddbb9d136ea6dbb2704ac93e5536ed0e6bfb19c) )

	ROM_REGION( 0x0400, "proms", 0 )    // PROMs (function unknown)
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

} // anonymous namespace


GAME( 199?, ffantasym,  hippodrm,  ffantasym,  ffantasym,  ffantasy_ms_state, empty_init, ROT0, "bootleg (Gaelco / Ervisa)", "Fighting Fantasy (Modular System)", MACHINE_IS_SKELETON )
GAME( 199?, drgninjam,  baddudes,  ffantasym,  ffantasym,  ffantasy_ms_state, empty_init, ROT0, "bootleg (Gaelco / Ervisa)", "Dragon Ninja (Modular System)",     MACHINE_IS_SKELETON )
