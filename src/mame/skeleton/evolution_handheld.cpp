// license:BSD-3-Clause
// copyright-holders:David Haywood

// TODO: identify the CPU type! - there are vectors(?) near the start

#include "emu.h"

#include "cpu/evolution/evo.h"

#include "screen.h"
#include "speaker.h"


namespace {

class evolution_handheldgame_state : public driver_device
{
public:
	evolution_handheldgame_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void evolhh(machine_config &config);

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	required_device<evo_cpu_device> m_maincpu;

	void evolution_map(address_map &map) ATTR_COLD;
};

void evolution_handheldgame_state::machine_start()
{
}

void evolution_handheldgame_state::machine_reset()
{
}

static INPUT_PORTS_START( evolhh )
INPUT_PORTS_END


uint32_t evolution_handheldgame_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void evolution_handheldgame_state::evolution_map(address_map &map)
{
	map(0x400000, 0x41ffff).rom().region("maincpu", 0x00000);
}


void evolution_handheldgame_state::evolhh(machine_config &config)
{
	EVOLUTION_CPU(config, m_maincpu, XTAL(16'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &evolution_handheldgame_state::evolution_map);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(256, 256); // resolution not confirmed
	screen.set_visarea(0, 256-1, 0, 256-1);
	screen.set_screen_update(FUNC(evolution_handheldgame_state::screen_update));

	SPEAKER(config, "speaker").front_center();
}

ROM_START( evolhh )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD( "s29gl032m90tfir4.u4", 0x000000, 0x400000, CRC(c647ca01) SHA1(a88f512d3fe8803dadc4eb6a94b5babd40c698de) )
ROM_END

} // anonymous namespace


CONS( 2006, evolhh,      0,       0,      evolhh, evolhh, evolution_handheldgame_state, empty_init, "Kidz Delight", "Evolution Max", MACHINE_IS_SKELETON ) // from a pink 'for girls' unit, exists in other colours, software likely the same
