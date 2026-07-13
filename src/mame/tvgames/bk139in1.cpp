// license:BSD-3-Clause
// copyright-holders:David Haywood

// possibly SPG293 (S+Core) or ARM, but the code is encrypted (probably AES)
// the NES emulation seems comparable to other SPG293 platforms

#include "emu.h"
#include "cpu/score/score.h"

#include "screen.h"
#include "speaker.h"


class bk139in1_state : public driver_device
{
public:
	bk139in1_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_screen(*this, "screen")
	{
	}

	void bk139in1(machine_config &config) ATTR_COLD;

	void init_bk139in1() ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void mem_map(address_map &map) ATTR_COLD;

	required_device<screen_device> m_screen;
};

uint32_t bk139in1_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void bk139in1_state::machine_start()
{
}

void bk139in1_state::machine_reset()
{
}

void bk139in1_state::mem_map(address_map &map)
{
	map(0x00000000, 0x001fffff).rom();
}

static INPUT_PORTS_START( bk139in1 )
INPUT_PORTS_END

void bk139in1_state::bk139in1(machine_config &config)
{
	SCORE7(config, "maincpu", 27'000'000).set_addrmap(AS_PROGRAM, &bk139in1_state::mem_map);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(320, 262);
	m_screen->set_visarea(0, 320-1, 0, 240-1);
	m_screen->set_screen_update(FUNC(bk139in1_state::screen_update));

	SPEAKER(config, "speaker", 2).front();
}

ROM_START( bk139in1 )
	ROM_REGION(0x4000000, "maincpu", ROMREGION_ERASE00)
	ROM_LOAD( "25q512.bin", 0x0000, 0x4000000, CRC(0cd111a4) SHA1(70553a44c3d946e5d23c09f04e0627a5dbaa3e4d) )
ROM_END

ROM_START( lxcyrace )
	ROM_REGION(0x1000000, "maincpu", ROMREGION_ERASE00)
	ROM_LOAD( "25q128.u2", 0x0000, 0x1000000, CRC(4489c99d) SHA1(792d6d224584fe1f3349c64a59aa79a587dd8c17) )
ROM_END

ROM_START( lxcymsm )
	ROM_REGION(0x2000000, "maincpu", ROMREGION_ERASE00)
	ROM_LOAD( "25q256.bin", 0x0000, 0x2000000, CRC(96b3ee5c) SHA1(f1e6bf46a4503877074310506d1acc4607dac331) )
ROM_END

ROM_START( lxcympp )
	ROM_REGION(0x2000000, "maincpu", ROMREGION_ERASE00)
	ROM_LOAD( "25q256.bin", 0x0000, 0x2000000, CRC(570b669c) SHA1(e7fcae662c8c8cae18cf1151d6caefacfe1e9fda) )
ROM_END

ROM_START( lxcymls )
	ROM_REGION(0x2000000, "maincpu", ROMREGION_ERASE00)
	ROM_LOAD( "25q256.bin", 0x0000, 0x2000000, CRC(76c89fe5) SHA1(99668cbce2ace6ec972ee4e72fec8b93862a0ef4) )
ROM_END

ROM_START( dgun3944 )
	ROM_REGION(0x1000000, "maincpu", ROMREGION_ERASE00)
	ROM_LOAD( "by25q128as.bin", 0x0000, 0x1000000, CRC(cf2bfc98) SHA1(f8c984f0278506d74b0b6337d2e96bb9d3a58148) )
ROM_END

ROM_START( starbuck )
	ROM_REGION(0x200000, "maincpu", ROMREGION_ERASE00)
	ROM_LOAD( "by25q16est.bin", 0x0000, 0x200000,  CRC(926c1499) SHA1(edd88b11f350e0016ee7dc76e872f9ae8c00aa6c) )
ROM_END

void bk139in1_state::init_bk139in1()
{
	u32 *rom = &memregion("maincpu")->as_u32(0xc00 / 4);
	u32 len = (memregion("maincpu")->bytes() - 0xc00) / 4;
	for (u32 j = 0; j < len; j++)
	{
		rom[j] ^= 0x32383024;
		rom[j] ^= bitswap<9>(j, 0, 1, 2, 3, 4, 5, 6, 7, 8);
		rom[j] ^= bitswap<16>(j, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15) << 16;
		// TODO: probably another bitswap on top of this
	}
}


// This can be found listed as a ZHISHAN / Aojiao / Bornkid 32 Bit Preloaded 139-in-1 Handheld Game Console
// but these just seem to be brands, manufacturer is unknown.
// Various case styles are available, the unit here was styled after a Nintendo Switch
//
// Architecture is unknown, it contains many of the games in beijuehh / bornkidh (generalplus_gpl16250_rom.cpp)
// but is running from SPI flash and has 'Loading' screens between menus and after selecting a game.
//
// While those are GeneralPlus based platforms, it's possible the games were ported to something else, the SPI
// appears to contain a filesystem, but data looks to be compressed / encrypted with no obvious code.
// There is no GPspi header in the SPI ROM.
CONS(202?, bk139in1,  0, 0, bk139in1, bk139in1, bk139in1_state, init_bk139in1, "<unknown>", "BornKid 32 Bit Preloaded 139-in-1 Handheld Game Console", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
// same unknown hardware as above, fewer games
CONS(2021, lxcyrace,  0, 0, bk139in1, bk139in1, bk139in1_state, init_bk139in1, "Lexibook", "Cyber Arcade Racing (JL3150)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
CONS(2021, lxcymsm,   0, 0, bk139in1, bk139in1, bk139in1_state, init_bk139in1, "Lexibook", "Cyber Arcade Motion - Superman (JL3180SU)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
CONS(2021, lxcympp,   0, 0, bk139in1, bk139in1, bk139in1_state, init_bk139in1, "Lexibook", "Cyber Arcade Motion - Paw Patrol (JL3180PA)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
CONS(2021, lxcymls,   0, 0, bk139in1, bk139in1, bk139in1_state, init_bk139in1, "Lexibook", "Cyber Arcade Motion - Lilo & Stitch (JL3180D_01)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
CONS(2021, dgun3944,  0, 0, bk139in1, bk139in1, bk139in1_state, init_bk139in1, "dreamGEAR", "My Arcade All Star Sports (Pixel Pocket, DGUNL3944)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
// sold by Starbucks China, contains a bunch of NES hacks (including a version of Super Mario Bros) probably running on an emulator
CONS(2022, starbuck,  0, 0, bk139in1, bk139in1, bk139in1_state, init_bk139in1, "Subor",     "Starbucks x Subor (OEM Q2, China)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
