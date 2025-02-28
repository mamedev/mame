// license:BSD-3-Clause
// copyright-holders:


/*************************************************************************************************************

    Skeleton driver for VTech Genius Color Pocket / Super Color Pocket / Genio Color Pocket.

    VTech 35-140500-100-203 PCB with MX25L3206E and N25S10 serial ROMs on one side and two globs on the other.
    Unknown CPU, program ROM seems compressed.

*************************************************************************************************************/


#include "emu.h"

#include "screen.h"
#include "speaker.h"


namespace {


class geniuscolor_state : public driver_device
{
public:
	geniuscolor_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_screen(*this, "screen")
	{ }

	void geniuscolor(machine_config &config) ATTR_COLD;

protected:
	required_device<screen_device> m_screen;

	uint32_t screen_update_geniuscolor(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};

uint32_t geniuscolor_state::screen_update_geniuscolor(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

// 45-keys "slider" keyboard, 8 activity buttons, two direction keys (right, left) and home, OK and power buttons.
INPUT_PORTS_START( geniuscolor )
INPUT_PORTS_END

void geniuscolor_state::geniuscolor(machine_config &config)
{
	// Unknown CPU

	SCREEN(config, m_screen, SCREEN_TYPE_LCD); // 104x48 color LCD screen
	m_screen->set_refresh_hz(60); // Guess
	m_screen->set_size(104, 48);
	m_screen->set_visarea(0, 104-1, 0, 48-1);
	m_screen->set_screen_update(FUNC(geniuscolor_state::screen_update_geniuscolor));

	SPEAKER(config, "mono").front_left();
}

// Spanish machine
ROM_START( geniuscps )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "internal.bin",       0x000000, 0x010000, NO_DUMP ) // Unknown CPU type, unknown internal ROM size

	ROM_REGION( 0x400000, "program", 0 )
	ROM_LOAD( "mx25l3206e.u1",      0x000000, 0x400000, CRC(fcc2e78d) SHA1(7f166256a10acfe854bac3fd2426ec4173d66518) ) // Compressed data?

	ROM_REGION( 0x010000, "soundcpu", 0 )
	ROM_LOAD( "sound_internal.bin", 0x000000, 0x010000, NO_DUMP ) // Unknown CPU type, unknown internal ROM size

	ROM_REGION( 0x20000, "user", 0 ) // Probably user data
	ROM_LOAD( "n25s10.u6",          0x000000, 0x020000, CRC(c5508360) SHA1(87c0855c90af2545a074df82411e5679e7309692) )
ROM_END

} // anonymous namespace


CONS( 2013, geniuscps, 0, 0, geniuscolor, geniuscolor, geniuscolor_state, empty_init, "VTech", "Genio Color Pocket (Spanish)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
