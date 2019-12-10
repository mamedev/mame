// license:BSD-3-Clause
// copyright-holders:David Haywood


/*****************************************************************************

	unlike earlier SunPlus / GeneralPlus based SoCs this one seems to be
	ARM based

*****************************************************************************/


#include "emu.h"

#include "cpu/arm7/arm7.h"
#include "cpu/arm7/arm7core.h"

#include "screen.h"
#include "speaker.h"

class generalplus_gpl32612_game_state : public driver_device
{
public:
	generalplus_gpl32612_game_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen")
	{ }

	void gpl32612(machine_config &config);

private:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	required_device<cpu_device> m_maincpu;

	required_device<screen_device> m_screen;

	uint32_t screen_update_gpl32612(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};

uint32_t generalplus_gpl32612_game_state::screen_update_gpl32612(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void generalplus_gpl32612_game_state::machine_start()
{
}

void generalplus_gpl32612_game_state::machine_reset()
{
}

static INPUT_PORTS_START( gpl32612 )
INPUT_PORTS_END


void generalplus_gpl32612_game_state::gpl32612(machine_config &config)
{
	ARM9(config, m_maincpu, 240000000); // unknown core / frequency, but ARM based

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(320, 262);
	m_screen->set_visarea(0, 320-1, 0, 240-1);
	m_screen->set_screen_update(FUNC(generalplus_gpl32612_game_state::screen_update_gpl32612));

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
}

// NAND dumps, so there will be a bootloader / boot strap at least

ROM_START( jak_tmnthp )
	ROM_REGION( 0x8400000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "tmntheroportal.bin", 0x000000, 0x8400000, CRC(75ec7127) SHA1(cd05f55a1f5a7fd3d1b0658ad6805b8777857a7e) )
ROM_END

ROM_START( jak_swbstrik )
	ROM_REGION( 0x8400000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "starwarsblaster.bin", 0x000000, 0x8400000, CRC(02c3c4d6) SHA1(a6ae05a7d7b2015023113f6baad25458f3c01102) )
ROM_END

//    year, name,         parent,  compat, machine,      input,        class,              init,       company,  fullname,                             flags
CONS( 200?, jak_tmnthp,      0,       0,      gpl32612, gpl32612, generalplus_gpl32612_game_state, empty_init, "JAKKS Pacific Inc", "Teenage Mutanat Ninja Turtles Hero Portal", MACHINE_IS_SKELETON )
CONS( 200?, jak_swbstrik,    0,       0,      gpl32612, gpl32612, generalplus_gpl32612_game_state, empty_init, "JAKKS Pacific Inc", "Star Wars Blaster Strike", MACHINE_IS_SKELETON )
// Hero Portal Dreamworks Dragons 
// Hero Portal Power Rangers 
// Hero Portal DC Super Heroes 
