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
	map(0x000000, 0x01ffff).mirror(0x400000).rom().region("maincpu", 0x00000);
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

ROM_START( smkatsum )
	ROM_REGION( 0x800000, "maincpu", 0 )
	ROM_LOAD( "gpr25l64.ic4", 0x000000, 0x800000, CRC(85be7517) SHA1(f9b838e09ceff9b99f3e41f010ff12f6adfa9be1) )
ROM_END

ROM_START( buttdtct )
	ROM_REGION( 0x800000, "maincpu", 0 )
	ROM_LOAD( "25l64.ic4", 0x000000, 0x800000, CRC(a70a2b0d) SHA1(fe5517d58297b737f9b6f645f76bea2a5dae1eb6) )
ROM_END


} // anonymous namespace


CONS( 2006, evolhh,      0,       0,      evolhh, evolhh, evolution_handheldgame_state, empty_init, "Kidz Delight", "Evolution Max", MACHINE_NO_SOUND | MACHINE_NOT_WORKING ) // from a pink 'for girls' unit, exists in other colours, software likely the same

CONS( 2018, smkatsum,    0,       0,      evolhh, evolhh, evolution_handheldgame_state, empty_init, "San-X / Tomy", "Sumikko Gurashi - Sumikko Atsume (Japan)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING ) // from a pink 'for girls' unit, exists in other colours, software likely the same

// おしりたんてい ププッとかいけつゲーム
CONS( 2020, buttdtct,    0,       0,      evolhh, evolhh, evolution_handheldgame_state, empty_init, "Tomy", "Oshiri Tantei - Puputto Kaiketsu Game (Japan)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING ) // from a pink 'for girls' unit, exists in other colours, software likely the same
