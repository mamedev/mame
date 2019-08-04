// license:BSD-3-Clause
// copyright-holders:David Haywood

/*
    Toki (Modular System)

    as with most of the 'Modular System' setups, the hardware is heavily modified from the original
    and consists of a multi-board stack in a cage, hence different driver.

    TODO: PCB list


    NOTES:
    PCB lacks raster effect on title screen

*/


#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


class toki_ms_state : public driver_device
{
public:
	toki_ms_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_palette(*this, "palette"),
		m_screen(*this, "screen"),
		m_paletteram(*this, "palette"),
		m_gfxdecode(*this, "gfxdecode")
	{ }

	void tokim(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;

	required_shared_ptr<uint16_t> m_paletteram;
	required_device<gfxdecode_device> m_gfxdecode;

	virtual void machine_start() override;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void tokim_map(address_map &map);
};


void toki_ms_state::tokim_map(address_map &map)
{
	map(0x000000, 0x05ffff).rom();
	map(0x060000, 0x06d7ff).ram();
	map(0x06e000, 0x06e7ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
}

void toki_ms_state::machine_start()
{
}


uint32_t toki_ms_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}

static INPUT_PORTS_START( tokim )
INPUT_PORTS_END

static const gfx_layout tiles16x16x4_layout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0,8,16,24 },
	{ 0,1,2,3,4,5,6,7, 512,513,514,515,516,517,518,519 },
	{ STEP16(0,32) },
	16 * 16 * 4
};

static GFXDECODE_START( gfx_toki_ms )
	GFXDECODE_ENTRY( "gfx1", 0, tiles16x16x4_layout, 0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, tiles16x16x4_layout, 0, 16 )
	GFXDECODE_ENTRY( "gfx3", 0, tiles16x16x4_layout, 0, 16 )
	GFXDECODE_ENTRY( "gfx4", 0, tiles16x16x4_layout, 0, 16 )
GFXDECODE_END

void toki_ms_state::tokim(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 12_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &toki_ms_state::tokim_map);
	m_maincpu->set_vblank_int("screen", FUNC(toki_ms_state::irq1_line_hold));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500) /* not accurate */);
	m_screen->set_size(256, 256);
	m_screen->set_visarea(0, 256-1, 0, 256-32-1);
	m_screen->set_screen_update(FUNC(toki_ms_state::screen_update));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xRGB_444, 0x400);

	GFXDECODE(config, m_gfxdecode, "palette", gfx_toki_ms);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
}


ROM_START( tokims )
	ROM_REGION( 0x080000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "6_tk_606.ic8",   0x000001, 0x010000, CRC(13ecfa14) SHA1(77b8600e41b6dc51e4ab927c626599e24f9a7853) )
	ROM_LOAD16_BYTE( "6_tk_603.ic17",  0x000000, 0x010000, CRC(714f76ff) SHA1(f2d13dcba84d5b7dd2f156d63421c511c518fc82) )
	ROM_LOAD16_BYTE( "6_tk_605.ic11",  0x020001, 0x010000, CRC(03644aa7) SHA1(4c874c0fe47213b9597690f4a4805e281fed20ad) )
	ROM_LOAD16_BYTE( "6_tk_602.ic20",  0x020000, 0x010000, CRC(71234409) SHA1(9d476ddde30b9f9ca80578a2d3ca5b94da6ee2f0) )
	ROM_LOAD16_BYTE( "6_tk_604.ic25",  0x040001, 0x010000, CRC(9879fde6) SHA1(79f8020bbc8e1545466bd12c81117f736c985afe) )
	ROM_LOAD16_BYTE( "6_tk_601.ic26",  0x040000, 0x010000, CRC(9810f8f0) SHA1(5a3fa4599058a3ff3a97d20764005e6d530187f8) )

	ROM_REGION( 0x010000, "audiocpu", 0 )    /* Z80 code */
	ROM_LOAD( "1_tk_101.c19", 0x000000, 0x10000, CRC(a447a394) SHA1(ccaa6aca5c2afc7c05035cb551b8368b18188dd6) )

	ROM_REGION( 0x100000, "gfx1", ROMREGION_ERASEFF ) // sprites (same rom subboard type as galpanic_ms.cpp)
	ROM_LOAD32_BYTE( "5_tk_501.ic3",         0x000003, 0x010000, CRC(c3cd26b6) SHA1(20d5a68eada4150642365dd61c699b7771de5372) )
	ROM_LOAD32_BYTE( "5_tk_505.ic12",        0x000002, 0x010000, CRC(ec096351) SHA1(10417266c2280b2d9c301423d8c41ed73d9654c9) )
	ROM_LOAD32_BYTE( "5_tk_509.ic18",        0x000001, 0x010000, CRC(a1a4ef7b) SHA1(92aad84f14f8257477920012bd1fe033ec96301b) )
	ROM_LOAD32_BYTE( "5_tk_513.ic24",        0x000000, 0x010000, CRC(8dfda6fa) SHA1(ee2600d6cdcb27500e61dd1beebed904fd2c3ac5) )

	ROM_LOAD32_BYTE( "5_tk_502.ic4",         0x040003, 0x010000, CRC(122d59eb) SHA1(5dc9c55667021630f49cfb70c0c70bdf3ac1e3a7) )
	ROM_LOAD32_BYTE( "5_tk_506.ic13",        0x040002, 0x010000, CRC(ed92289f) SHA1(fe612e704bf6aefdbd85f1d49a9bbc4d0fef0f95) )
	ROM_LOAD32_BYTE( "5_tk_510.ic19",        0x040001, 0x010000, CRC(56eb4876) SHA1(113d2b300d7670068e3587f63b4f0b0bd38d84a3) )
	ROM_LOAD32_BYTE( "5_tk_514.ic25",        0x040000, 0x010000, CRC(b0c7801c) SHA1(99e898bcb4a8c4dc00726908f9095df512539776) )

	ROM_LOAD32_BYTE( "5_tk_503.ic5",         0x080003, 0x010000, CRC(9201545b) SHA1(dee1736946ec781ee035714281298f2e2a48fec1) )
	ROM_LOAD32_BYTE( "5_tk_507.ic14",        0x080002, 0x010000, CRC(e61eebbd) SHA1(1f854ba98a1cde4473107b8282b88e6412094d19) )
	ROM_LOAD32_BYTE( "5_tk_511.ic20",        0x080001, 0x010000, CRC(06d9fd86) SHA1(22472905672c956941d41b3e5febb4cb57c91283) )
	ROM_LOAD32_BYTE( "5_tk_515.ic26",        0x080000, 0x010000, CRC(04b575a7) SHA1(c6c65745511e27b594818e3f7ba7313c0a6f599e) )

	ROM_LOAD32_BYTE( "5_tk_504.ic6",         0x0c0003, 0x010000, CRC(cec71122) SHA1(283d38f998b1ca4fa080bf9fac797f5ac91dd072) )
	ROM_LOAD32_BYTE( "5_tk_508.ic15",        0x0c0002, 0x010000, CRC(1873ae38) SHA1(a1633ab5c417e9851e285a6b322c06e7d2d0bccd) )
	ROM_LOAD32_BYTE( "5_tk_512.ic21",        0x0c0001, 0x010000, CRC(0228110f) SHA1(33a29f9f458ca9d0af3c8da8a5b67bab79cecdec) )
	ROM_LOAD32_BYTE( "5_tk_516.ic27",        0x0c0000, 0x010000, CRC(f4e29429) SHA1(706050b51e0afbddf6ec5c8f14d3649bb05c8550) )

	ROM_REGION( 0x040000, "gfx2", 0 )
	ROM_LOAD( "8_tk_825.ic9",      0x000000, 0x10000, CRC(6d04def0) SHA1(36f23b0893dfae6cf4c6f4414ff54bb13cfdad41) )
	ROM_LOAD( "8_tk_826.ic16",     0x010000, 0x10000, CRC(d3a2a038) SHA1(a2a020397a427f5fd401aad09048c7d4a21cd728) )
	ROM_LOAD( "8_tk_827.ic24",     0x020000, 0x10000, CRC(d254ae6c) SHA1(cdbdd7d7c6cd4de8b8a0f54e1543caba5f3d11cb) )
	ROM_LOAD( "8_tk_828.ic31",     0x030000, 0x10000, CRC(a6fae34b) SHA1(d9a276d30bdcc25d9cd299c2502cf910273890f6) )

	ROM_REGION( 0x080000, "gfx3", 0 ) // same ROMs as some of the other Toki bootlegs
	ROM_LOAD( "8_tk_809.ic13",     0x000000, 0x10000, CRC(feb13d35) SHA1(1b78ce1e48d16e58ad0721b30ab87765ded7d24e) )
	ROM_LOAD( "8_tk_813.ic12",     0x010000, 0x10000, CRC(5b365637) SHA1(434775b0614d904beaf40d7e00c1eaf59b704cb1) )
	ROM_LOAD( "8_tk_810.ic20",     0x020000, 0x10000, CRC(617c32e6) SHA1(a80f93c83a06acf836e638e4ad2453692622015d) )
	ROM_LOAD( "8_tk_814.ic19",     0x030000, 0x10000, CRC(2a11c0f0) SHA1(f9b1910c4932f5b95e5a9a8e8d5376c7210bcde7) )
	ROM_LOAD( "8_tk_811.ic28",     0x040000, 0x10000, CRC(fbc3d456) SHA1(dd10455f2e6c415fb5e39fb239904c499b38ca3e) )
	ROM_LOAD( "8_tk_815.ic27",     0x050000, 0x10000, CRC(4c2a72e1) SHA1(52a31f88e02e1689c2fffbbd86cbccd0bdab7dcc) )
	ROM_LOAD( "8_tk_812.ic35",     0x060000, 0x10000, CRC(46a1b821) SHA1(74d9762aef3891463dc100d1bc2d4fdc3c1d163f) )
	ROM_LOAD( "8_tk_816.ic34",     0x070000, 0x10000, CRC(82ce27f6) SHA1(db29396a336098664f48e3c04930b973a6ffe969) )

	ROM_REGION( 0x080000, "gfx4", 0 ) // same ROMs as some of the other Toki bootlegs
	ROM_LOAD( "8_tk_801.ic15",     0x000000, 0x10000, CRC(63026cad) SHA1(c8f3898985d99f2a61d4e17eba66b5989a23d0d7) )
	ROM_LOAD( "8_tk_805.ic14",     0x010000, 0x10000, CRC(a7f2ce26) SHA1(6b12b3bd872112b42d91ce3c0d5bc95c0fc0f5b5) )
	ROM_LOAD( "8_tk_802.ic22",     0x020000, 0x10000, CRC(48989aa0) SHA1(109c68c9f0966862194226cecc8b269d9307dd25) )
	ROM_LOAD( "8_tk_806.ic21",     0x030000, 0x10000, CRC(c2ad9342) SHA1(7c9b5c14c8061e1a57797b79677741b1b98e64fa) )
	ROM_LOAD( "8_tk_803.ic30",     0x040000, 0x10000, CRC(6cd22b18) SHA1(8281cfd46738448b6890c50c64fb72941e169bee) )
	ROM_LOAD( "8_tk_807.ic29",     0x050000, 0x10000, CRC(859e313a) SHA1(18ac471a72b3ed42ba74456789adbe323f723660) )
	ROM_LOAD( "8_tk_804.ic37",     0x060000, 0x10000, CRC(e15c1d0f) SHA1(d0d571dd1055d7307379850313216da86b0704e6) )
	ROM_LOAD( "8_tk_808.ic36",     0x070000, 0x10000, CRC(6f4b878a) SHA1(4560b1e705a0eb9fad7fdc11fadf952ff67eb264) )

	ROM_REGION( 0x100, "protpal", 0 ) // all read protected
	ROM_LOAD( "5_5140_palce16v8h-25pc.ic9", 0, 1, NO_DUMP )
	ROM_LOAD( "5_5240_palce16v8h-25pc.ic8", 0, 1, NO_DUMP )
	ROM_LOAD( "6_604_gal16v8-20hb1.ic13", 0, 1, NO_DUMP )
	ROM_LOAD( "6_640_palce16v8h-25pc.ic7", 0, 1, NO_DUMP )
	ROM_LOAD( "7_7140_gal20v8-20hb1.ic7", 0, 1, NO_DUMP )
	ROM_LOAD( "7_7240_gal20v8-20hb1.ic54", 0, 1, NO_DUMP )
	ROM_LOAD( "7_7340_palce16v8h-25pc.ic55", 0, 1, NO_DUMP )
	ROM_LOAD( "7_7440_palce16v8h-25pc.ic9", 0, 1, NO_DUMP )
	ROM_LOAD( "7_7540_palce16v8h-25pc.ic59", 0, 1, NO_DUMP )
	ROM_LOAD( "7_7640_gal20v8-20hb1.ic44", 0, 1, NO_DUMP )
	ROM_LOAD( "51_503_palce16v8h-25pc.ic46", 0, 1, NO_DUMP )

	ROM_REGION( 0x100, "proms", ROMREGION_ERASEFF )
	ROM_LOAD( "1_10110_82s123.ic20",  0x0000, 0x020, CRC(e26e680a) SHA1(9bbe30e98e952a6113c64e1171330153ddf22ce7) )
	ROM_LOAD( "51_502_82s129an.ic10", 0x0000, 0x100, CRC(15085e44) SHA1(646e7100fcb112594023cf02be036bd3d42cc13c) ) // same as every other modular bootleg
	ROM_LOAD( "21_201_82s129.ic4",    0x0000, 0x100, CRC(2697da58) SHA1(e62516b886ff6e204b718e5f0c6ce2712e4b7fc5) )
	ROM_LOAD( "21_202_82s129.ic12",   0x0000, 0x100, CRC(e434128a) SHA1(ef0f6d8daef8b25211095577a182cdf120a272c1) )
ROM_END

GAME( 1991, tokims,  toki,  tokim,  tokim,  toki_ms_state, empty_init, ROT0, "bootleg", "Toki (Modular System)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
