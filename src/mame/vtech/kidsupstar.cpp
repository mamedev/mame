// license:BSD-3-Clause
// copyright-holders:


/*********************************************************************************

    Skeleton driver for VTech Kidi SuperStar LightShow (karaoke + games).
    GeneralPlus based. VTech PCB 35-178500-200-263.
    Video from the real hardware: https://youtu.be/ru6zKr2fbTk?si=1ZAAt9a1fapj23BM

    Two big globs on the PCB back side (U4 and U8).

*********************************************************************************/


#include "emu.h"

#include "cpu/arm7/arm7.h"

#include "screen.h"
#include "speaker.h"


namespace {


class kidsupstar_state : public driver_device
{
public:
	kidsupstar_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen")
	{ }

	void kidsupstar(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;

	uint32_t screen_update_kidsupstar(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};

uint32_t kidsupstar_state::screen_update_kidsupstar(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void kidsupstar_state::machine_start()
{
}

void kidsupstar_state::machine_reset()
{
}

static INPUT_PORTS_START( kidsupstar )
INPUT_PORTS_END

void kidsupstar_state::kidsupstar(machine_config &config)
{
	ARM9(config, m_maincpu, 240'000'000); // Unknown core and frequency

	SCREEN(config, m_screen, SCREEN_TYPE_LCD); // Monochrome 48x48 LCD screen
	m_screen->set_refresh_hz(60); // Guess
	m_screen->set_size(48, 48);
	m_screen->set_visarea(0, 48-1, 0, 48-1);
	m_screen->set_screen_update(FUNC(kidsupstar_state::screen_update_kidsupstar));

	SPEAKER(config, "mono").front_left();
}

// Spanish machine, may be different between regions.
ROM_START( kidsupstar )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "internal.u4",              0x000000, 0x010000, NO_DUMP ) // Unknown CPU type, unknown internal ROM size

	ROM_REGION( 0x400000, "program", 0 )
	ROM_LOAD( "winbond_w25q32fvssiq.ic7", 0x000000, 0x400000, CRC(9233f8b4) SHA1(eb5accb9c3f0a3fe0d0141a84d7e08fc356b4959) )

	ROM_REGION( 0x010000, "soundcpu", 0 )
	ROM_LOAD( "internal.u8",              0x000000, 0x010000, NO_DUMP ) // Unknown CPU type, unknown internal ROM size

	ROM_REGION( 0x400000, "music", 0 )
	ROM_LOAD( "zbit_25vq32.ic3",          0x000000, 0x400000, CRC(45a8d68c) SHA1(c00d17527b0e585c9358e873c3449b259814f025) )

	ROM_REGION( 0x400000, "user", 0 )
	ROM_LOAD( "zbit_25vq32.ic13",         0x000000, 0x400000, CRC(9233f8b4) SHA1(eb5accb9c3f0a3fe0d0141a84d7e08fc356b4959) ) // May contain user data, needs a factory reset
ROM_END

} // anonymous namespace


CONS( 2016, kidsupstar, 0, 0, kidsupstar, kidsupstar, kidsupstar_state, empty_init, "VTech", "Kidi SuperStar LightShow", MACHINE_IS_SKELETON )
