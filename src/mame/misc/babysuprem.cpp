// license:BSD-3-Clause
// copyright-holders:

/*
    Baby Suprem (c) 198? Andra / Vifico

    Slot machine.

    Main components:
     1 x Z80
     2 x AY-3-8910
     1 x 5517 RAM
     1 x 2.4576MHz Osc
     1 x 8-dip + 1 x 4 dip banks

    There's a complete manual with schematics at https://www.recreativas.org/manuales
*/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "sound/ay8910.h"

#include "speaker.h"


namespace {

class bsuprem_state : public driver_device
{
public:
	bsuprem_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_psg(*this, "psg%u", 1U)
	{
	}

	void bsuprem(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	required_device_array<ay8910_device, 2> m_psg;
};

static INPUT_PORTS_START( bsuprem )
	PORT_START("DSW1")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW1:4")

	PORT_START("DSW2")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW2:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW2:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW2:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW2:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW2:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW2:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW2:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW2:8")
INPUT_PORTS_END

void bsuprem_state::bsuprem(machine_config &config)
{
	// Basic machine hardware
	Z80(config, m_maincpu, 2.4576_MHz_XTAL); // divider not verified

	// Sound hardware
	SPEAKER(config, "mono").front_center();
	AY8910(config, m_psg[0], 2.4576_MHz_XTAL); // divider not verified
	m_psg[0]->add_route(ALL_OUTPUTS, "mono", 0.30);
	AY8910(config, m_psg[1], 2.4576_MHz_XTAL); // divider not verified
	m_psg[1]->add_route(ALL_OUTPUTS, "mono", 0.30);
}

ROM_START( bsuprem )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "andra_baby_suprem.u4", 0x0000, 0x4000, CRC(71e4ae62) SHA1(3aa4e7125ea03464c58187eb85fb706b5392b9b5) )
ROM_END

} // Anonymous namespace


//    YEAR  NAME     PARENT  MACHINE  INPUT    CLASS          INIT        ROT   COMPANY           FULLNAME       FLAGS
GAME( 198?, bsuprem, 0,      bsuprem, bsuprem, bsuprem_state, empty_init, ROT0, "Andra / Vifico", "Baby Suprem", MACHINE_IS_SKELETON_MECHANICAL )
