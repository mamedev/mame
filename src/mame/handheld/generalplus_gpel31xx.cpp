// license:BSD-3-Clause
// copyright-holders:David Haywood

// GPEL31xx chips can have OTP ROM

#include "emu.h"

#include "cpu/arm7/arm7.h"

#include "screen.h"
#include "speaker.h"

namespace {

class generalplus_gpel31xx_game_state : public driver_device
{
public:
	generalplus_gpel31xx_game_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen")
	{ }

	void gpel31xx(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void arm_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;

	uint32_t screen_update_gpel31xx(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

private:
};

uint32_t generalplus_gpel31xx_game_state::screen_update_gpel31xx(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void generalplus_gpel31xx_game_state::arm_map(address_map &map)
{
}

void generalplus_gpel31xx_game_state::machine_start()
{
}

void generalplus_gpel31xx_game_state::machine_reset()
{
}


static INPUT_PORTS_START( gpel31xx )
INPUT_PORTS_END

void generalplus_gpel31xx_game_state::gpel31xx(machine_config &config)
{
	ARM7(config, m_maincpu, 96'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &generalplus_gpel31xx_game_state::arm_map);

	SCREEN(config, m_screen, SCREEN_TYPE_LCD);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(320, 262);
	m_screen->set_visarea(0, 320-1, 0, 240-1);
	m_screen->set_screen_update(FUNC(generalplus_gpel31xx_game_state::screen_update_gpel31xx));

	SPEAKER(config, "speaker", 2).front();
}

ROM_START( tamameet )
	ROM_REGION(  0x800000, "spi", ROMREGION_ERASE00 )
	ROM_LOAD( "25q64.u3", 0x0000, 0x800000, CRC(f15507f8) SHA1(356cb1bd68169eb747898325eacfd7590dbe9f9c) )
ROM_END

} // anonymous namespace

// uses GPEL3101A
// たまごっち みーつ  (there appear to be many units in this series, ROM data could differ, this was from a light blue 'hearts and rainbows' themed unit with no subtitles)
CONS( 2018, tamameet,           0,        0,      gpel31xx, gpel31xx, generalplus_gpel31xx_game_state, empty_init,  "Bandai",        "Tamagotchi Meets (Japan)",   MACHINE_NO_SOUND | MACHINE_NOT_WORKING)
