// license:BSD-3-Clause
// copyright-holders:Ryan Holtz, David Haywood

// Sharp Cookie units were published by JAKKS Pacific under the 'Child Guidance' brand (battery compartments etc. still have JAKKS branding)

#include "emu.h"
#include "includes/spg2xx.h"
#include "machine/nvram.h"

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
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // PAL/NTSC flag, set to PAL (not verified, based on other JAKKS units)
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

// has UK specific voice actors, US versions on YouTube have different voices not present in this set
CONS( 2007, jsc_thom, 0, 0, base_config_pal,     jak_sharp,      jakks_sharp_state, empty_init, "JAKKS Pacific Inc / Child Guidance / Pronto Games",          "Thomas & Friends - Learning Circus Express (Sharp Cookie) (PAL, UK)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS ) // uses NK keys (same as Nicktoons & Spongebob) (3 released) - The upper part of this one is pink/purple.

// The Amazing Spider-Man - Great Math Caper
// Scooby-Doo! - The Pirate's Puzzles
// Go Diego Go - Aztec ABC Adventure
// Dora the Explorer - Dora Saves the Mermaids
