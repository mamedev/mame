// license:BSD-3-Clause
// copyright-holders:

/*
M102-C-A 05C04585A1

Games known to run on this PCB (may be only one):
Hill Climber
Electromechanical medal game
https://www.youtube.com/watch?v=vb7NiRsT_x4

Main components:

D70008AC-6 CPU
27C010A-15 ROM (for program)
2 M27C4001-12F1 ROMs (for sound data)
PAL16L8
CXK58257ASP-10L SRAM
Nanao GA20 sound chip
3.579545 MHz XTAL
2 8-DIP banks
9 connectors
*/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "sound/iremga20.h"

#include "speaker.h"


namespace {

class m102_state : public driver_device
{
public:
	m102_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{}

	void m102(machine_config &config) ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;

	void program_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
};


void m102_state::program_map(address_map &map)
{
	map(0x0000, 0xefff).rom();
	map(0xf000, 0xffff).ram();
}


// TODO: double-check everything
void m102_state::io_map(address_map &map)
{
	map.global_mask(0xff);

	map(0x00, 0x00).portr("IN0");
	map(0x01, 0x01).portr("IN1");
	map(0x02, 0x02).portr("DSW1");
	map(0x03, 0x03).portr("DSW2");
	// map(0x00, 0x10).w() Lamps, LEDs, coin counter and?
	map(0x40, 0x5f).rw("ga20", FUNC(iremga20_device::read), FUNC(iremga20_device::write));
}


static INPUT_PORTS_START( m102 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_PLAYER(1)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_PLAYER(2)

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


void m102_state::m102(machine_config &config)
{
	Z80(config, m_maincpu, 3.579545_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &m102_state::program_map);
	m_maincpu->set_addrmap(AS_IO, &m102_state::io_map);
	m_maincpu->set_periodic_int(FUNC(m102_state::irq0_line_hold), attotime::from_hz(60)); // TODO: where does this come from?

	SPEAKER(config, "mono").front_center();

	IREMGA20(config, "ga20", 3.579545_MHz_XTAL).add_route(ALL_OUTPUTS, "mono", 0.7);
}


ROM_START( hclimber )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "hc-pr-c.ic23", 0x00000, 0x20000, CRC(0b91671b) SHA1(306bde2f4e46fc9927bd659f38e233e6d533a1e4) ) // 1xxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x100000, "ga20", 0 )
	ROM_LOAD( "hc-v0-.ic13", 0x00000, 0x80000, CRC(1658e7b5) SHA1(1a554d4381db09216983ddc301c329634043f694) )
	ROM_LOAD( "hc-v1-.ic14", 0x80000, 0x80000, CRC(ae505c2e) SHA1(abfa1b3996b6960d681bb1209bed91be72a656c2) )

	ROM_REGION( 0x200, "plds", ROMREGION_ERASE00 )
	ROM_LOAD( "m102-c_pal.ic33", 0x000, 0x104, NO_DUMP ) // exact type not verified, but marked as 16L8 on PCB
ROM_END

} // anonymous namespace


GAME( 1992, hclimber, 0, m102, m102, m102_state, empty_init, ROT0, "Irem", "Hill Climber", MACHINE_IS_SKELETON_MECHANICAL )
