// license:BSD-3-Clause
// copyright-holders:Ryan Holtz, David Haywood

// Sharp Cookie units were published by JAKKS Pacific under the 'Child Guidance' brand (battery compartments etc. still have JAKKS branding)

#include "emu.h"
#include "spg2xx.h"
#include "machine/nvram.h"


namespace {

class jakks_sharp_state : public spg2xx_game_state
{
public:
	jakks_sharp_state(const machine_config &mconfig, device_type type, const char *tag) :
		spg2xx_game_state(mconfig, type, tag)
	{ }

	void base_config(machine_config& config);
	void base_config_pal(machine_config& config);

private:
};

static INPUT_PORTS_START( jak_sharp )
	PORT_START("P1")
	PORT_BIT( 0x001f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Menu / Pause")
	PORT_BIT( 0x00c0, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )

	PORT_START("P2")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P3")
	PORT_BIT( 0x0007, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // PAL/NTSC flag, set to PAL (based on other JAKKS units, possibly not the case here as not read / no effect)
	PORT_BIT( 0xfff0, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END

void jakks_sharp_state::base_config(machine_config& config)
{
	SPG24X(config, m_maincpu, XTAL(27'000'000), m_screen);

	spg2xx_base(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &jakks_sharp_state::mem_map_1m);

	m_maincpu->porta_in().set_ioport("P1");
	m_maincpu->portb_in().set_ioport("P2");
	m_maincpu->portc_in().set_ioport("P3");
}

void jakks_sharp_state::base_config_pal(machine_config& config)
{
	base_config(config);

	m_maincpu->set_pal(true);
	m_screen->set_refresh_hz(50);
}


ROM_START( jsc_thom )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "shckthomas.bin", 0x000000, 0x200000, CRC(bc9549ed) SHA1(7925a8ac166f9c7a56bc5f9d4f9f774af1c92d05) )
ROM_END

ROM_START( jsc_thomu )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "shckthomasus.bin", 0x000000, 0x200000, CRC(28d0887d) SHA1(31caf4bb4e823a572010de9c58e76590c4346e92) )
ROM_END

ROM_START( jsc_spid )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "sharpcookiespiderman.bin", 0x000000, 0x200000, CRC(84cf58bf) SHA1(ac0be079c2469c9c0dea3decd7a7318806cc7ac0) )
ROM_END

ROM_START( jsc_gdg )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "shckdiego.bin", 0x000000, 0x200000, CRC(8069147a) SHA1(3f90dd3deff89d7d66b4f14b6246c2bf63c44586) )
ROM_END

ROM_START( jsc_dora )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "shckdora.bin", 0x000000, 0x200000, CRC(4a973046) SHA1(13b38b5db23169731ebf1a4657d95e34fc88b9b8) )
ROM_END

ROM_START( jsc_sdoo )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "shckscooby.bin", 0x000000, 0x200000, CRC(ce7039a4) SHA1(d5815149b75262253d03fac946b10c43e96945c0) )
ROM_END

} // anonymous namespace


// The UK version has UK specific voice actors
CONS( 2007, jsc_thom,  0,        0, base_config_pal,     jak_sharp,      jakks_sharp_state, empty_init, "JAKKS Pacific Inc / Child Guidance / Pronto Games",          "Thomas & Friends - Learning Circus Express (Sharp Cookie) (PAL, UK)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
// the US version appears to be an earlier build with no "Sir Topham Hatt" pre-game instruction screens, different narrator, no visible freight carriage in the 3rd game etc.
CONS( 2007, jsc_thomu, jsc_thom, 0, base_config,         jak_sharp,      jakks_sharp_state, empty_init, "JAKKS Pacific Inc / Child Guidance / Pronto Games",          "Thomas & Friends - Learning Circus Express (Sharp Cookie) (NTSC, US)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )

CONS( 2007, jsc_spid,  0,        0, base_config,         jak_sharp,      jakks_sharp_state, empty_init, "JAKKS Pacific Inc / Child Guidance / Pronto Games",          "The Amazing Spider-Man - Great Math Caper (Sharp Cookie) (NTSC, US)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )

// from a UK unit but still says 'Zee' instead of 'Zed' for 'Z'  This does not appear to be controlled by a PAL/NTSC flag in the inputs.
CONS( 2007, jsc_gdg,   0,        0, base_config_pal,     jak_sharp,      jakks_sharp_state, empty_init, "JAKKS Pacific Inc / Child Guidance / Pronto Games",          "Go, Diego, Go! Aztec ABC Adventure (Sharp Cookie) (PAL, UK)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )

CONS( 2007, jsc_dora,  0,        0, base_config_pal,     jak_sharp,      jakks_sharp_state, empty_init, "JAKKS Pacific Inc / Child Guidance / Handheld Games",        "Dora the Explorer - Dora Saves the Mermaids (Sharp Cookie) (PAL, UK)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )

CONS( 2007, jsc_sdoo,  0,        0, base_config_pal,     jak_sharp,      jakks_sharp_state, empty_init, "JAKKS Pacific Inc / Child Guidance / Handheld Games",        "Scooby-Doo! and The Pirate's Puzzles (Sharp Cookie) (PAL, UK)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
