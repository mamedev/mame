// license:BSD-3-Clause
// copyright-holders:

#include "emu.h"

#include "cpu/c33/s1c33209.h"

#include "screen.h"
#include "speaker.h"


namespace {

class tama_mix_state : public driver_device
{
public:
	tama_mix_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen")
	{
	}

	void tama_mix(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void mem_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
};

u32 tama_mix_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void tama_mix_state::machine_start()
{
}


void tama_mix_state::mem_map(address_map &map)
{
	map(0x0c00000, 0x0ffffff).rom().region("maincpu", 0); // maybe internal ROM belongs here instead?
	map(0x2000000, 0x27fffff).rom().region("maincpu", 0);
}


static INPUT_PORTS_START(tama_mix)
INPUT_PORTS_END


void tama_mix_state::tama_mix(machine_config &config)
{
	S1C33209(config, m_maincpu, 10'000'000); // unknown model and clock
	m_maincpu->set_addrmap(AS_PROGRAM, &tama_mix_state::mem_map);

	// wrong, just so it's clear this has a screen
	SCREEN(config, m_screen, SCREEN_TYPE_LCD);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(128, 128); 
	m_screen->set_visarea(0, 128 - 1, 0, 128 - 1);
	m_screen->set_screen_update(FUNC(tama_mix_state::screen_update));

	SPEAKER(config, "speaker").front_center();
}

ROM_START( tamamixm )
	ROM_REGION( 0x800000, "maincpu", 0 )
	ROM_LOAD( "mx29lv640et.u4", 0x000000, 0x800000, CRC(6e9301d2) SHA1(25a0df83da441d18467f82e8854e195139cce4a1) )
ROM_END

} // anonymous namespace

// 2016/03/14 MODEL: WIZ249 on PCB
GAME( 2016, tamamixm, 0, tama_mix, tama_mix, tama_mix_state, empty_init, ROT0, "Bandai", "Tamagotchi m!x Melody Blue (Japan)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
