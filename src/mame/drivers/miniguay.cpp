// license:BSD-3-Clause
// copyright-holders:

/*
    Mini Guay (c) 1986 Cirsa

    Slot machine.

    Main components:
    1 x 8085
    1 x AY-3-8910
    1 x 8256A (MUART, unemulated)
    2 x 8155
    2 x 8-dip banks (between AY and MUART)
    1 x 6.144MHz Osc
    battery backed RAM

    Two different PCBs were found with same components, albeit some from different producers.
    According to pics of the machine found on the net, it has leds.
*/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "machine/i8155.h"
#include "sound/ay8910.h"
#include "screen.h"
#include "speaker.h"


namespace {

class miniguay_state : public driver_device
{
public:
	miniguay_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void miniguay(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;

	void main_map(address_map &map);
};

void miniguay_state::main_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("maincpu", 0);
}

static INPUT_PORTS_START( miniguay )
	PORT_START("IN0")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW1:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW1:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW1:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW1:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW1:8")

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


void miniguay_state::miniguay(machine_config &config)
{
	// basic machine hardware
	I8085A(config, m_maincpu, 6.144_MHz_XTAL); // divider not verified
	m_maincpu->set_addrmap(AS_PROGRAM, &miniguay_state::main_map);

	I8155(config, "i8155_1", 6.144_MHz_XTAL / 2); // divider not verified

	I8155(config, "i8155_2", 6.144_MHz_XTAL / 2); // divider not verified

	// sound hardware
	SPEAKER(config, "mono").front_center();
	ay8910_device &ay(AY8910(config, "ay", 6.144_MHz_XTAL / 4)); // divider not verified
	//ay.port_a_read_callback().set_ioport("DSW1");
	//ay.port_b_read_callback().set_ioport("DSW2");
	ay.add_route(ALL_OUTPUTS, "mono", 0.30);
}


ROM_START( miniguay )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "a21t_b-82.bin", 0x0000, 0x8000, CRC(04865da9) SHA1(78cf41d8428eb67ae40e764494ac03d45762500a) )

	ROM_REGION( 0x200, "plds", 0 )
	ROM_LOAD( "pat_031_pal16r4.bin", 0x000, 0x104, NO_DUMP )
ROM_END

} // Anonymous namespace


GAME( 1986, miniguay, 0, miniguay, miniguay, miniguay_state, empty_init, ROT0, "Cirsa", "Mini Guay", MACHINE_IS_SKELETON_MECHANICAL )
