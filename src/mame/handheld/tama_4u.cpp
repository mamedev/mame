// license:BSD-3-Clause
// copyright-holders:

#include "emu.h"

#include "cpu/c33/s1c33209.h"

#include "screen.h"
#include "speaker.h"


namespace {

class tama_4u_state : public driver_device
{
public:
	tama_4u_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen")
	{
	}

	void tama4u(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void mem_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
};

u32 tama_4u_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void tama_4u_state::machine_start()
{
}


void tama_4u_state::mem_map(address_map &map)
{
	map(0x0800000, 0x0ffffff).rom().region("maincpu", 0);
	map(0x2000000, 0x27fffff).rom().region("maincpu", 0);
}


static INPUT_PORTS_START(tama4u)
INPUT_PORTS_END


void tama_4u_state::tama4u(machine_config &config)
{
	S1C33209(config, m_maincpu, 10'000'000); // unknown model and clock
	m_maincpu->set_addrmap(AS_PROGRAM, &tama_4u_state::mem_map);

	// wrong, just so it's clear this has a screen
	SCREEN(config, m_screen, SCREEN_TYPE_LCD);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(128, 128);
	m_screen->set_visarea(0, 128 - 1, 0, 128 - 1);
	m_screen->set_screen_update(FUNC(tama_4u_state::screen_update));

	SPEAKER(config, "speaker").front_center();
}

ROM_START( tama4u )
	ROM_REGION( 0x800000, "maincpu", 0 )
	// vector table(?) at 0x400000, similar to tamaid
	ROM_LOAD( "mx29lb640eb.u4", 0x000000, 0x800000, CRC(f9dcc04a) SHA1(5f5d738357219b2a4a1452835455abef25fe7dcb) )
ROM_END

} // anonymous namespace

// 2014/06/10 MODEL:WIZ245 on PCB
GAME( 2014, tama4u, 0, tama4u, tama4u, tama_4u_state, empty_init, ROT0, "Bandai", "Tamagotchi 4U (Japan)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
