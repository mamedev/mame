// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"

#include "cpu/arm7/arm7.h"

#include "screen.h"
#include "speaker.h"


namespace {

class generalplus_gp327902_game_state : public driver_device
{
public:
	generalplus_gp327902_game_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen")
	{ }

	void gp327902(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void arm_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;

	uint32_t screen_update_gp327902(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

private:

};

void generalplus_gp327902_game_state::arm_map(address_map &map)
{
	map(0x00000000, 0x03ffffff).ram();
}

uint32_t generalplus_gp327902_game_state::screen_update_gp327902(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void generalplus_gp327902_game_state::machine_start()
{
}

void generalplus_gp327902_game_state::machine_reset()
{
}

static INPUT_PORTS_START( gp327902 )
INPUT_PORTS_END

void generalplus_gp327902_game_state::gp327902(machine_config &config)
{
	ARM9(config, m_maincpu, 240'000'000); // unknown core / frequency, but ARM based
	m_maincpu->set_addrmap(AS_PROGRAM, &generalplus_gp327902_game_state::arm_map);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(320, 262);
	m_screen->set_visarea(0, 320-1, 0, 240-1);
	m_screen->set_screen_update(FUNC(generalplus_gp327902_game_state::screen_update_gp327902));

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
}

ROM_START( sanxpet )
	ROM_REGION(  0x800000, "spi", ROMREGION_ERASE00 )
	ROM_LOAD( "25l64.u1", 0x0000, 0x800000, CRC(f28b9fd3) SHA1(8ed4668f271cbe01065bc0836e49ce70faf10834) )
ROM_END

ROM_START( sanxpeta )
	ROM_REGION(  0x800000, "spi", ROMREGION_ERASE00 )
	ROM_LOAD( "gpr25l6403f.u1", 0x0000, 0x800000, CRC(cb5dc7b6) SHA1(425c4d01b56784278b77824a354d9efa46e1a74e) )
ROM_END

ROM_START( tomyegg )
	ROM_REGION(  0x800000, "spi", ROMREGION_ERASE00 )
	ROM_LOAD( "gpr25l6403f.u1", 0x0000, 0x800000, CRC(2acd6752) SHA1(85e59546a1af4618c75c275cead7ef0f5e3faa44) )
ROM_END

} // anonymous namespace

// Tomy / San-X devices

// dates for each of these taken from back of case, are the DX versions different software or just different accessories?

// 2018 version is a square device - Sumikko Gurashi - Sumikko Atsume (すみっコぐらし すみっコあつめ)

// 2019 version is house shaped device - すみっコぐらし すみっコさがし
CONS( 2019, sanxpet,         0,        0,      gp327902, gp327902, generalplus_gp327902_game_state, empty_init,  "San-X / Tomy",        "Sumikko Gurashi - Sumikko Sagashi",   MACHINE_NO_SOUND | MACHINE_NOT_WORKING)
// or Sumikko Gurashi - Sumikko Sagashi DX (すみっコぐらし すみっコさがしDX "Sumikko Gurashi the movie" alt version)

// 2020 version is a cloud shaped device - Sumikko Gurashi - Sumikko Catch (すみっコぐらし すみっコキャッチ)
// or Sumikko Gurashi - Sumikko Catch DX (すみっコぐらし すみっコキャッチDX) = Sumikko Catch with pouch and strap
 
// 2021 version is a square device with a tiny 'mole' figure on top - すみっコぐらし すみっコみっけDX
// or Sumikko Gurashi - Sumikko Mikke (すみっコぐらし すみっコみっけ)
CONS( 2021, sanxpeta,        0,        0,      gp327902, gp327902, generalplus_gp327902_game_state, empty_init,  "San-X / Tomy",        "Sumikko Gurashi - Sumikko Mikke",   MACHINE_NO_SOUND | MACHINE_NOT_WORKING) // DX?


// other devices on the same Soc

// キラッとプリ☆チャン プリたまGO ミスティパープル
CONS( 2019, tomyegg,         0,        0,      gp327902, gp327902, generalplus_gp327902_game_state, empty_init,  "Tomy",        "Kiratto Pri-Chan - Pritama Go: Misty Purple (Japan)",   MACHINE_NO_SOUND | MACHINE_NOT_WORKING)
// these also exist, are they the same software or different versions?
// Powder Pink (パウダーピンク)
// Mint Blue (ミントブルー).
