// license:BSD-3-Clause
// copyright-holders:

/***************************************************************************************************
    Xor World (Modular System).

    This is neither a clone nor a bootleg, but the original Xor World, that was originally developed
    using the Modular System.

    For this game the Modular System cage contains 6 boards.

    MOD 5/1: (sprites) 4 x EPROM, 2 x GAL16V9
    MOD 51/1: 1 x PROM, 1 x PAL16R6, logic.
    MOD 7/4: 2 x MS62256L-10PC RAM, 3 x GAL16V8, 2 x GAL20V8
    MOD 8/2: (gfx) 12 x EPROM.
    MOD 6/1: (CPU) MC68000P10, 20 MHz xtal, 2 x EPROM, 1 x GAL16V8, 1 x PAL16V8.
    SYSTEM 2: (sound) 1 x EPROM, Z8400BB1, 28 MHz xtal, 16 MHz xtal, 1 x OKI5205, 384 kHz osc,
              1 x GAL16V8, 1 x GAL20V8, 1 x YM3812.

***************************************************************************************************/

#include "emu.h"

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"

#include "sound/msm5205.h"
#include "sound/ymopl.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

namespace {

class xorworld_ms_state : public driver_device
{
public:
	xorworld_ms_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_palette(*this, "palette")
		, m_screen(*this, "screen")
		, m_gfxdecode(*this, "gfxdecode")
	{ }

	void xorworld_ms(machine_config &config) ATTR_COLD;

	void init_xorworld_ms() ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;
	required_device<gfxdecode_device> m_gfxdecode;

	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void descramble_16x16tiles(u8 *src, int len);

	void xorworld_ms_map(address_map &map) ATTR_COLD;
};


void xorworld_ms_state::video_start()
{
}

u32 xorworld_ms_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}


void xorworld_ms_state::xorworld_ms_map(address_map &map)
{
	map(0x000000, 0x01ffff).rom();
}

static INPUT_PORTS_START( xorworld_ms )
INPUT_PORTS_END

static const gfx_layout tiles8x8x4_layout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0,8,16,24 },
	{ 0,1,2,3,4,5,6,7 },
	{ STEP8(0,32) },
	16 * 16
};

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

// Probably wrong
static GFXDECODE_START( gfx_xorworld_ms )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8x4_layout, 0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, tiles16x16x4_layout, 0, 16 )
	GFXDECODE_ENTRY( "sprites", 0, tiles16x16x4_layout, 0x000, 16 )
GFXDECODE_END

void xorworld_ms_state::machine_start()
{
}

// Reorganize graphics into something we can decode with a single pass
void xorworld_ms_state::descramble_16x16tiles(u8 *src, int len)
{
	std::vector<u8> buffer(len);
	{
		for (int i = 0; i < len; i++)
		{
			int j = bitswap<20>(i, 19,18,17,16,15,12,11,10,9,8,7,6,5,14,13,4,3,2,1,0);
			buffer[j] = src[i];
		}

		std::copy(buffer.begin(), buffer.end(), &src[0]);
	}
}

void xorworld_ms_state::init_xorworld_ms()
{
	descramble_16x16tiles(memregion("gfx2")->base(), memregion("gfx2")->bytes());
}

void xorworld_ms_state::xorworld_ms(machine_config &config)
{
	M68000(config, m_maincpu, 20_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &xorworld_ms_state::xorworld_ms_map);
	m_maincpu->set_vblank_int("screen", FUNC(xorworld_ms_state::irq4_line_hold));

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(64*8, 32*8);
	m_screen->set_visarea(0*8, 64*8-1, 0*8, 32*8-1);
	m_screen->set_screen_update(FUNC(xorworld_ms_state::screen_update));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_entries(0x400);

	GFXDECODE(config, m_gfxdecode, "palette", gfx_xorworld_ms);

	// Sound hardware

	SPEAKER(config, "mono").front_center();

	Z80(config, "soundcpu", 16_MHz_XTAL / 4); // Z8400BB1

	YM3812(config, "ymsnd", 16_MHz_XTAL / 4); // Unknown divisor

	MSM5205(config, "msm", 384_kHz_XTAL).add_route(ALL_OUTPUTS, "mono", 0.15);
}

ROM_START( xorworldm )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "mod_6-1_xo_608a_27c512.ic8",  0x000000, 0x010000, CRC(cebcd3e7) SHA1(ee6d107dd13e4faa8d5b6ba9815c57919a69568e) )
	ROM_LOAD16_BYTE( "mod_6-1_xo_617a_27c512.ic17", 0x000001, 0x010000, CRC(47bae292) SHA1(f36e2f80d4da31d7edc7dc8c07abb158ef21cb20) )

	ROM_REGION( 0x40000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "mod_8-2_27c512.ic15", 0x00003, 0x10000, CRC(01d16cac) SHA1(645b3380e66ab158392f8161485ae957ab2dfa44) )
	ROM_LOAD32_BYTE( "mod_8-2_27c512.ic22", 0x00002, 0x10000, CRC(3aadacaf) SHA1(f640a1d6a32774cb70e180c49965f70f5b79ba6a) )
	ROM_LOAD32_BYTE( "mod_8-2_27c512.ic30", 0x00001, 0x10000, CRC(fa75826e) SHA1(1db9cc8ec5811b9d497b060bfc3245c6e082e98c) )
	ROM_LOAD32_BYTE( "mod_8-2_27c512.ic37", 0x00000, 0x10000, CRC(157832ed) SHA1(5dec1c7046d2449d3d4330654554f1a9430c8057) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD32_BYTE( "mod_8-2_27c512.ic13", 0x00003, 0x10000, CRC(56a683fc) SHA1(986a6b26e38308dd3230dd13a1a2f5cfc7b1dda8) )
	ROM_LOAD32_BYTE( "mod_8-2_27c512.ic20", 0x00002, 0x10000, CRC(950090e6) SHA1(cdec2eb93884c5a759af39d02f5cc0fa25011103) )
	ROM_LOAD32_BYTE( "mod_8-2_27c512.ic28", 0x00001, 0x10000, CRC(ca950a11) SHA1(cef2a65d1f5562556fc28a95f6ba8f3ff1a3678d) )
	ROM_LOAD32_BYTE( "mod_8-2_27c512.ic35", 0x00000, 0x10000, CRC(5e7582f9) SHA1(fdb3ddb2a224fdea610f6bd55a3e3efe1a5515a2) )

	ROM_REGION( 0x80000, "gfx3", 0 )
	ROM_LOAD32_BYTE( "mod_8-2_27c512.ic11", 0x00003, 0x10000, CRC(93373f1f) SHA1(06718a09c76060914a1b2a4c57d0328a9f59eb99) )
	ROM_LOAD32_BYTE( "mod_8-2_27c512.ic33", 0x00002, 0x10000, CRC(c29cd83b) SHA1(e8833b6ab8f7f6ce4c01937e94332a86908353db) )
	ROM_LOAD32_BYTE( "mod_8-2_27c512.ic18", 0x00001, 0x10000, CRC(7015d8ff) SHA1(83d40ef2ca1cf8dfb2229b8f47c08a303b8490ea) )
	ROM_LOAD32_BYTE( "mod_8-2_27c512.ic26", 0x00000, 0x10000, CRC(4be8db92) SHA1(8983893570587de9cdd63bc645d1ae2d8c3400a2) )

	ROM_REGION( 0x80000, "sprites", ROMREGION_ERASEFF | ROMREGION_INVERT )
	ROM_LOAD32_BYTE( "mod_5-1_27c512.ic3",  0x00003, 0x10000, CRC(26b2181e) SHA1(03cc280a4e9d9d8ac1c04da7d90c684d83fc2444) )
	ROM_LOAD32_BYTE( "mod_5-1_27c512.ic12", 0x00002, 0x10000, CRC(b2bdb8d1) SHA1(eeace8dc4e6c192ad7b2f53cd01fcc2e92125f35) )
	ROM_LOAD32_BYTE( "mod_5-1_27c512.ic18", 0x00001, 0x10000, CRC(d0c7a07b) SHA1(698883f2a91a34c802c1bc9a86bec0a77f4de7fe) )
	ROM_LOAD32_BYTE( "mod_5-1_27c512.ic24", 0x00000, 0x10000, CRC(fd2cbaed) SHA1(037523a3eae058e848763bbfc067d8271c70074b) )

	ROM_REGION( 0x010000, "soundcpu", 0 )
	ROM_LOAD( "mod_9-2_27c512.ic6", 0x00000, 0x10000, CRC(c722165b) SHA1(b1cd3ac80521036059579e71b6cb9bd4b97da4c2) )

	ROM_REGION( 0x100, "proms", ROMREGION_ERASE00 )
	ROM_LOAD( "mod_51-3_502_82s129a.ic10", 0x000, 0x100, CRC(15085e44) SHA1(646e7100fcb112594023cf02be036bd3d42cc13c) ) // same as every other modular system

	ROM_REGION( 0x157, "plds", ROMREGION_ERASE00 )
	ROM_LOAD( "mod_5-1_52fx_gal16v8.ic8",  0x0000, 0x117, CRC(f3d686c9) SHA1(6bc1bd40f49663e776c0d40b2f146338a2057097) )
	ROM_LOAD( "mod_5-1_51fx_gal16v8.ic9",  0x0000, 0x117, CRC(0070e8b9) SHA1(8a3ef9445599dc88c001a919e01f47844eec81eb) )
	ROM_LOAD( "mod_6-1_gal16v8.ic7",       0x0000, 0x117, BAD_DUMP CRC(470a0194) SHA1(810c51fca7d2430b7c2292f3e1dd02c86d74d64b) ) // Bruteforced
	ROM_LOAD( "mod_6-1_palce16v8.ic13",    0x0000, 0x117, BAD_DUMP CRC(0b90f141) SHA1(4b0f2b98073d52b6a49941031e8e37b5aeb6d507) ) // Bruteforced
	ROM_LOAD( "mod_7-4_gal20v8.ic7",       0x0000, 0x157, NO_DUMP )
	ROM_LOAD( "mod_7-4_gal16v8.ic9",       0x0000, 0x117, NO_DUMP )
	ROM_LOAD( "mod_7-4_gal20v8.ic44",      0x0000, 0x157, NO_DUMP )
	ROM_LOAD( "mod_7-4_gal20v8.ic54",      0x0000, 0x157, NO_DUMP )
	ROM_LOAD( "mod_7-4_gal16v8.ic55",      0x0000, 0x117, NO_DUMP )
	ROM_LOAD( "mod_7-4_gal16v8.ic59",      0x0000, 0x117, BAD_DUMP CRC(55b86dd5) SHA1(4a05d86002f929f38171282d2aad5d986283c45b) ) // Bruteforced
	ROM_LOAD( "mod_9-2_gal16v8.ic10",      0x0000, 0x117, BAD_DUMP CRC(23070765) SHA1(3c87f7ab299fcd62da366fc9c1985ab21ee14960) ) // Bruteforced
	ROM_LOAD( "mod_9-2_gal20v8.ic18",      0x0000, 0x157, NO_DUMP )
	ROM_LOAD( "mod_9-2_gal16v8.ic42",      0x0000, 0x117, BAD_DUMP CRC(d62f0a0d) SHA1(76bd995c8b71b12b752ed517d87a78e55a8bab00) ) // Bruteforced
	ROM_LOAD( "mod_51-3_503_gal16v8.ic46", 0x0000, 0x117, NO_DUMP )
ROM_END

ROM_START( xorworldma )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "xo_608_27c512.bin", 0x000000, 0x010000, CRC(12ed0ab0) SHA1(dcdd6dccc367fe1084af71001afaa26c4879817e) )
	ROM_LOAD16_BYTE( "xo_617_27c512.bin", 0x000001, 0x010000, CRC(fea2750f) SHA1(b6ed781514a9c1901372e489ba46a36120bde528) )

	ROM_REGION( 0x40000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "mod_8-2_27c512.ic15", 0x00003, 0x10000, CRC(01d16cac) SHA1(645b3380e66ab158392f8161485ae957ab2dfa44) )
	ROM_LOAD32_BYTE( "mod_8-2_27c512.ic22", 0x00002, 0x10000, CRC(3aadacaf) SHA1(f640a1d6a32774cb70e180c49965f70f5b79ba6a) )
	ROM_LOAD32_BYTE( "mod_8-2_27c512.ic30", 0x00001, 0x10000, CRC(fa75826e) SHA1(1db9cc8ec5811b9d497b060bfc3245c6e082e98c) )
	ROM_LOAD32_BYTE( "mod_8-2_27c512.ic37", 0x00000, 0x10000, CRC(157832ed) SHA1(5dec1c7046d2449d3d4330654554f1a9430c8057) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD32_BYTE( "mod_8-2_27c512.ic13", 0x00003, 0x10000, CRC(56a683fc) SHA1(986a6b26e38308dd3230dd13a1a2f5cfc7b1dda8) )
	ROM_LOAD32_BYTE( "mod_8-2_27c512.ic20", 0x00002, 0x10000, CRC(950090e6) SHA1(cdec2eb93884c5a759af39d02f5cc0fa25011103) )
	ROM_LOAD32_BYTE( "mod_8-2_27c512.ic28", 0x00001, 0x10000, CRC(ca950a11) SHA1(cef2a65d1f5562556fc28a95f6ba8f3ff1a3678d) )
	ROM_LOAD32_BYTE( "mod_8-2_27c512.ic35", 0x00000, 0x10000, CRC(5e7582f9) SHA1(fdb3ddb2a224fdea610f6bd55a3e3efe1a5515a2) )

	ROM_REGION( 0x80000, "gfx3", 0 )
	ROM_LOAD32_BYTE( "mod_8-2_27c512.ic11", 0x00003, 0x10000, CRC(93373f1f) SHA1(06718a09c76060914a1b2a4c57d0328a9f59eb99) )
	ROM_LOAD32_BYTE( "mod_8-2_27c512.ic33", 0x00002, 0x10000, CRC(c29cd83b) SHA1(e8833b6ab8f7f6ce4c01937e94332a86908353db) )
	ROM_LOAD32_BYTE( "mod_8-2_27c512.ic18", 0x00001, 0x10000, CRC(7015d8ff) SHA1(83d40ef2ca1cf8dfb2229b8f47c08a303b8490ea) )
	ROM_LOAD32_BYTE( "mod_8-2_27c512.ic26", 0x00000, 0x10000, CRC(4be8db92) SHA1(8983893570587de9cdd63bc645d1ae2d8c3400a2) )

	ROM_REGION( 0x80000, "sprites", ROMREGION_ERASEFF | ROMREGION_INVERT )
	ROM_LOAD32_BYTE( "mod_5-1_27c512.ic3",  0x00003, 0x10000, CRC(26b2181e) SHA1(03cc280a4e9d9d8ac1c04da7d90c684d83fc2444) )
	ROM_LOAD32_BYTE( "mod_5-1_27c512.ic12", 0x00002, 0x10000, CRC(b2bdb8d1) SHA1(eeace8dc4e6c192ad7b2f53cd01fcc2e92125f35) )
	ROM_LOAD32_BYTE( "mod_5-1_27c512.ic18", 0x00001, 0x10000, CRC(d0c7a07b) SHA1(698883f2a91a34c802c1bc9a86bec0a77f4de7fe) )
	ROM_LOAD32_BYTE( "mod_5-1_27c512.ic24", 0x00000, 0x10000, CRC(fd2cbaed) SHA1(037523a3eae058e848763bbfc067d8271c70074b) )

	ROM_REGION( 0x010000, "soundcpu", 0 )
	ROM_LOAD( "mod_9-2_27c512.ic6", 0x00000, 0x10000, CRC(c722165b) SHA1(b1cd3ac80521036059579e71b6cb9bd4b97da4c2) )

	ROM_REGION( 0x100, "proms", ROMREGION_ERASE00 )
	ROM_LOAD( "mod_51-3_502_82s129a.ic10", 0x000, 0x100, CRC(15085e44) SHA1(646e7100fcb112594023cf02be036bd3d42cc13c) ) // same as every other modular system

	ROM_REGION( 0x157, "plds", ROMREGION_ERASE00 )
	ROM_LOAD( "mod_5-1_52fx_gal16v8.ic8",  0x0000, 0x117, CRC(f3d686c9) SHA1(6bc1bd40f49663e776c0d40b2f146338a2057097) )
	ROM_LOAD( "mod_5-1_51fx_gal16v8.ic9",  0x0000, 0x117, CRC(0070e8b9) SHA1(8a3ef9445599dc88c001a919e01f47844eec81eb) )
	ROM_LOAD( "mod_6-1_gal16v8.ic7",       0x0000, 0x117, BAD_DUMP CRC(470a0194) SHA1(810c51fca7d2430b7c2292f3e1dd02c86d74d64b) ) // Bruteforced
	ROM_LOAD( "mod_6-1_palce16v8.ic13",    0x0000, 0x117, BAD_DUMP CRC(0b90f141) SHA1(4b0f2b98073d52b6a49941031e8e37b5aeb6d507) ) // Bruteforced
	ROM_LOAD( "mod_7-4_gal20v8.ic7",       0x0000, 0x157, NO_DUMP )
	ROM_LOAD( "mod_7-4_gal16v8.ic9",       0x0000, 0x117, NO_DUMP )
	ROM_LOAD( "mod_7-4_gal20v8.ic44",      0x0000, 0x157, NO_DUMP )
	ROM_LOAD( "mod_7-4_gal20v8.ic54",      0x0000, 0x157, NO_DUMP )
	ROM_LOAD( "mod_7-4_gal16v8.ic55",      0x0000, 0x117, NO_DUMP )
	ROM_LOAD( "mod_7-4_gal16v8.ic59",      0x0000, 0x117, BAD_DUMP CRC(55b86dd5) SHA1(4a05d86002f929f38171282d2aad5d986283c45b) ) // Bruteforced
	ROM_LOAD( "mod_9-2_gal16v8.ic10",      0x0000, 0x117, BAD_DUMP CRC(23070765) SHA1(3c87f7ab299fcd62da366fc9c1985ab21ee14960) ) // Bruteforced
	ROM_LOAD( "mod_9-2_gal20v8.ic18",      0x0000, 0x157, NO_DUMP )
	ROM_LOAD( "mod_9-2_gal16v8.ic42",      0x0000, 0x117, BAD_DUMP CRC(d62f0a0d) SHA1(76bd995c8b71b12b752ed517d87a78e55a8bab00) ) // Bruteforced
	ROM_LOAD( "mod_51-3_503_gal16v8.ic46", 0x0000, 0x117, NO_DUMP )
ROM_END

} // anonymous namespace

// Xor World was originally developed using the Modular System, so this isn't a bootleg

GAME( 1990, xorworldm,  xorworld, xorworld_ms, xorworld_ms, xorworld_ms_state, init_xorworld_ms, ROT0, "Gaelco", "Xor World (Modular System, set 1)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 1990, xorworldma, xorworld, xorworld_ms, xorworld_ms, xorworld_ms_state, init_xorworld_ms, ROT0, "Gaelco", "Xor World (Modular System, set 2)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
