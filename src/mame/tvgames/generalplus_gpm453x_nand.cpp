// license:BSD-3-Clause
// copyright-holders:David Haywood


/*****************************************************************************

    ARM Cortex M4F based GPM453x series chips

    all game data is stored on a NAND

    both games here are on HDMI sticks

    These likely boot from an internal ROM so will need bootstrapping without
    a dump of it

    LeapLand Adventures - GPM4530A
    Paw Patrol - GPM4532C

*****************************************************************************/


#include "emu.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "cpu/arm7/arm7.h"

#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"


namespace {

class generalplus_gpm453x_game_state : public driver_device
{
public:
	generalplus_gpm453x_game_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen")
	{ }

	void gpm453x(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void arm_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

private:

};


void generalplus_gpm453x_game_state::arm_map(address_map &map)
{
	map(0x00000000, 0x03ffffff).ram();
}

uint32_t generalplus_gpm453x_game_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void generalplus_gpm453x_game_state::machine_start()
{
}

void generalplus_gpm453x_game_state::machine_reset()
{
}

static INPUT_PORTS_START( gpm453x )
INPUT_PORTS_END

void generalplus_gpm453x_game_state::gpm453x(machine_config &config)
{
	ARM9(config, m_maincpu, 240'000'000); // unknown core / frequency, but ARM based
	m_maincpu->set_addrmap(AS_PROGRAM, &generalplus_gpm453x_game_state::arm_map);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(320, 262);
	m_screen->set_visarea(0, 320-1, 0, 240-1);
	m_screen->set_screen_update(FUNC(generalplus_gpm453x_game_state::screen_update));

	SPEAKER(config, "speaker", 2).front();
}

ROM_START( leapland )
	ROM_REGION( 0x22000000, "nand", ROMREGION_ERASEFF )
	ROM_LOAD( "tc58nvg2s0hta00_withspare.u2", 0x000000, 0x22000000, CRC(2482c26d) SHA1(2ebaacdcc9188bcf86507ebdc9cea6e13f9b9988) )
ROM_END

ROM_START( leappawp )
	ROM_REGION( 0x11000000, "nand", ROMREGION_ERASEFF )
	ROM_LOAD( "tc58nvg1s3hta00_withspare.u2", 0x000000, 0x11000000, CRC(0d7ff9a1) SHA1(9916c3578595b89e94c0ab64f4356badc5b8a0dd) )
ROM_END

} // anonymous namespace

CONS( 2021, leapland,    0,       0,      gpm453x, gpm453x, generalplus_gpm453x_game_state, empty_init, "LeapFrog", "LeapLand Adventures (UK)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
CONS( 2021, leappawp,    0,       0,      gpm453x, gpm453x, generalplus_gpm453x_game_state, empty_init, "LeapFrog", "PAW Patrol: To The Rescue! (UK)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
