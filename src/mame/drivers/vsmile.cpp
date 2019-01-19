// license:BSD-3-Clause
// copyright-holders:David Haywood
/******************************************************************************

    V-Tech V.Smile skeleton driver

    Similar Systems:

        V.Smile Pocket
        V.Smile Cyber Pocket
        V.Smile PC Pal
        V-Motion Active Learning System
        V.Flash
        V.Baby
        Leapfrog Leapster

*******************************************************************************/

#include "emu.h"

#include "cpu/unsp/unsp.h"

#include "machine/spg2xx.h"

#include "screen.h"
#include "softlist.h"
#include "speaker.h"

class vsmile_state : public driver_device
{
public:
	vsmile_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_spg(*this, "spg")
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
	{ }

	void vsmile(machine_config &config);
	void vsmileb(machine_config &config);

protected:
	void mem_map(address_map &map);

	required_device<spg2xx_device> m_spg;
	required_device<unsp_device> m_maincpu;
	required_device<screen_device> m_screen;
};

void vsmile_state::mem_map(address_map &map)
{
	map(0x000000, 0x3fffff).rom().region("maincpu", 0);
	map(0x000000, 0x003fff).m(m_spg, FUNC(spg2xx_device::map));
}

static INPUT_PORTS_START( vsmile )
INPUT_PORTS_END

void vsmile_state::vsmile(machine_config &config)
{
	UNSP(config, m_maincpu, XTAL(27'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &vsmile_state::mem_map);
	m_maincpu->set_force_no_drc(true);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(320, 262);
	m_screen->set_visarea(0, 320-1, 0, 240-1);
	m_screen->set_screen_update("spg", FUNC(spg2xx_device::screen_update));
	m_screen->screen_vblank().set(m_spg, FUNC(spg2xx_device::vblank));

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	SPG24X(config, m_spg, XTAL(27'000'000), m_maincpu, m_screen);
	m_spg->add_route(ALL_OUTPUTS, "lspeaker", 0.5);
	m_spg->add_route(ALL_OUTPUTS, "rspeaker", 0.5);
}

ROM_START( vsmile )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "vsmilebios.bin", 0x000000, 0x200000, CRC(11f1b416) SHA1(11f77c4973d29c962567390e41879c86a759c93b) )
ROM_END

ROM_START( vsmileg )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "bios german.bin", 0x000000, 0x200000, CRC(205c5296) SHA1(7fbcf761b5885c8b1524607aabaf364b4559c8cc) )
ROM_END

ROM_START( vsmilef )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "sysrom_france", 0x000000, 0x200000, CRC(0cd0bdf5) SHA1(5c8d1eada1b6b545555b8d2b09325d7127681af8) )
ROM_END

ROM_START( vsmileb )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "vbabybios.bin", 0x000000, 0x800000, CRC(ddc7f845) SHA1(2c17d0f54200070176d03d44a40c7923636e596a) )
ROM_END

//    year, name,    parent, compat, machine, input,  class,        init,       company, fullname,            flags
CONS( 2005, vsmile,  0,      0,      vsmile,  vsmile, vsmile_state, empty_init, "VTech", "V.Smile (US)",      MACHINE_IS_SKELETON )
CONS( 2005, vsmileg, vsmile, 0,      vsmile,  vsmile, vsmile_state, empty_init, "VTech", "V.Smile (Germany)", MACHINE_IS_SKELETON )
CONS( 2005, vsmilef, vsmile, 0,      vsmile,  vsmile, vsmile_state, empty_init, "VTech", "V.Smile (France)",  MACHINE_IS_SKELETON )
CONS( 2005, vsmileb, 0,      0,      vsmile,  vsmile, vsmile_state, empty_init, "VTech", "V.Smile Baby (US)", MACHINE_IS_SKELETON )
