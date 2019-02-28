// license:BSD-3-Clause
// copyright-holders:
/*
    Skeleton driver for Cromptons Leisure Machines' Frantic Fruits redemption game.

    Hardware overview:
    Main CPU: TS80C32X2-MCA
    Sound: CD sound?
    Other: M48T08 timekeeper RAM
    OSC: 11.0592 MHz
    Dips: 2 x 8 dips banks

    Video: https://www.youtube.com/watch?v=89XJpor9dSQ
*/

#include "emu.h"
#include "cpu/mcs51/mcs51.h"
#include "screen.h"

class cromptons_state : public driver_device
{
public:
	cromptons_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void cromptons(machine_config &config);

protected:

private:
	required_device<cpu_device> m_maincpu;

	void prg_map(address_map &map);
	void io_map(address_map &map);
};

void cromptons_state::prg_map(address_map &map)
{
	map(0x0000, 0xffff).rom();
}

void cromptons_state::io_map(address_map &map)
{
}

static INPUT_PORTS_START( cromptons )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

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


void cromptons_state::cromptons(machine_config &config)
{
	/* basic machine hardware */
	I80C32(config, m_maincpu, 11.0592_MHz_XTAL); // TS80C32X2-MCA
	m_maincpu->set_addrmap(AS_PROGRAM, &cromptons_state::prg_map);
	m_maincpu->set_addrmap(AS_IO, &cromptons_state::io_map);

	// sound ??
}

/***************************************************************************

  Game drivers

***************************************************************************/

ROM_START( ffruits )
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "frntf5v6.ic11", 0x00000, 0x10000, CRC(ca60c557) SHA1(6f356827f0c93ec0376a7edc03963ef0748dccdb) ) // 27c512
ROM_END


GAME( 2000, ffruits,  0,   cromptons, cromptons, cromptons_state, empty_init, ROT0, "Cromptons Leisure Machines", "Frantic Fruits",  MACHINE_IS_SKELETON_MECHANICAL )
