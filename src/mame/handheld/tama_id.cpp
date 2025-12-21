// license:BSD-3-Clause
// copyright-holders:

#include "emu.h"

#include "cpu/c33/s1c33209.h"

#include "screen.h"
#include "speaker.h"


namespace {

class tama_id_state : public driver_device
{
public:
	tama_id_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen")
	{
	}

	void tamaid(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void mem_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
};

u32 tama_id_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void tama_id_state::machine_start()
{
}


void tama_id_state::mem_map(address_map &map)
{
	map(0x0c00000, 0x0ffffff).rom().region("maincpu", 0);
}


static INPUT_PORTS_START(tamaid)
INPUT_PORTS_END


void tama_id_state::tamaid(machine_config &config)
{
	S1C33209(config, m_maincpu, 10'000'000); // unknown model and clock
	m_maincpu->set_addrmap(AS_PROGRAM, &tama_id_state::mem_map);

	// wrong, just so it's clear this has a screen
	SCREEN(config, m_screen, SCREEN_TYPE_LCD);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(128, 128); 
	m_screen->set_visarea(0, 128 - 1, 0, 128 - 1);
	m_screen->set_screen_update(FUNC(tama_id_state::screen_update));

	SPEAKER(config, "speaker").front_center();
}

ROM_START( tamaid )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD( "mx29lb320dt.ic4", 0x000000, 0x400000, CRC(6272c24d) SHA1(f4d67409fa15873961bda684c68f3febf501218f) )
ROM_END

} // anonymous namespace

// 2010/2/5 MODEL: WIZ229 on PCB, 2009 on case
GAME( 2010, tamaid, 0, tamaid, tamaid, tama_id_state, empty_init, ROT0, "Bandai", "Tamagotchi iD (Japan)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
