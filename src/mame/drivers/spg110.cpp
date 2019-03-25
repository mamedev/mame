// license:BSD-3-Clause
// copyright-holders:David Haywood
/******************************************************************************

    Short Description:

        die markings show

        "SunPlus PA7801" ( known as Sunplus SPG110? )
        Classic Arcade Pinball
        EA Sports (NHL95 + Madden 95)
        Spiderman 5-in-1 (original release)

*******************************************************************************/

#include "emu.h"

#include "cpu/unsp/unsp.h"
#include "machine/spg110.h"
#include "screen.h"
#include "speaker.h"

class spg110_game_state : public driver_device
{
public:
	spg110_game_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_spg(*this, "spg")
	{ }

	void spg110_base(machine_config &config);

protected:

	required_device<unsp_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<spg110_device> m_spg;

	virtual void mem_map(address_map &map);
};

/*************************
*    Machine Hardware    *
*************************/

void spg110_game_state::mem_map(address_map &map)
{
	map(0x004000, 0x0fffff).rom().region("maincpu", 0x8000);
	map(0x000000, 0x003fff).m(m_spg, FUNC(spg110_device::map));
}

static INPUT_PORTS_START( spg110 )
INPUT_PORTS_END


void spg110_game_state::spg110_base(machine_config &config)
{
	UNSP(config, m_maincpu, XTAL(27'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &spg110_game_state::mem_map);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(320, 262);
	m_screen->set_visarea(0, 320-1, 0, 240-1);
	m_screen->set_screen_update("spg", FUNC(spg110_device::screen_update));
	m_screen->screen_vblank().set(m_spg, FUNC(spg110_device::vblank));

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
//  m_spg->add_route(ALL_OUTPUTS, "lspeaker", 0.5);
//  m_spg->add_route(ALL_OUTPUTS, "rspeaker", 0.5);

	SPG110(config, m_spg, XTAL(27'000'000), "maincpu");
}

ROM_START( jak_capb )
	ROM_REGION( 0x200000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "classicarcadepinball.bin", 0x000000, 0x200000, CRC(b643dab0) SHA1(f57d546758ba442e28b5f0f48b3819b2fc2eb7f7) )
ROM_END


ROM_START( jak_spdmo )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "spidermaneyes.bin", 0x000000, 0x200000, CRC(d5eaa6ae) SHA1(df226d378b41cf6ef90b9f72e48ff5e66385dcba) )
ROM_END

// JAKKS Pacific Inc TV games
CONS( 2004, jak_capb,  0,        0, spg110_base, spg110, spg110_game_state, empty_init, "JAKKS Pacific Inc / HotGen Ltd",      "Classic Arcade Pinball (JAKKS Pacific TV Game)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
CONS( 2004, jak_spdmo, jak_spdm, 0, spg110_base, spg110, spg110_game_state, empty_init, "JAKKS Pacific Inc / Digital Eclipse", "Spider-Man (JAKKS Pacific TV Game) (older hardare)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS ) // this is the smaller more 'square' style joystick that was originally released before the GameKey slot was added.
