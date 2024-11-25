// license:BSD-3-Clause
// copyright-holders:
/****************************************************************************************************************

  Skeleton driver for "El Circulo", from Inder.
  It's a gambling roulette built with lights (mix of leds and small light bulbs) and an crystal artwork overlay.

     ______________    ______    __________________________________    ____________    ________________
    | | | | | | | |___| | | |___|               _         _       |___| | | | | | |___| | | | | | | | |
    |                               _     LD23 (_)       (_) LD24    _                                |
    |                         LD22 (_)                              (_) LD25                          |
    |                   _                                                         _                   |
    |              L21 (_)                                                       (_) LD26             |
    |                      _________  _________  _________  _________  _________                      |
    |            _         DM81LS95N  DM81LS95N  DM74LS04N  DM74LS04N  DM74LS04N          _           |
    |      LD20 (_)                                                                      (_) LD27     |
    |                      _________  _________  _________  _________  _________                      |
    |       _              |_DIPSx8_| HM74C173N  HM74C173N  HM74C173N  HM74C173N              _       |
    | LD17 (_)                                                                               (_) LD30 |
    |
    |     _                                                            _________  _________     _     |
    LD16 (_)                                                           |74LS132N |SN74LS42N    (_) LD31
    |                                                            _                _________           |
    |                                                       IL2 (_)              HM435101P-1          |
    | _                                                                                             _ |
LD15 (_)                                                                          _________        (_) LD00
    |                                                                            HM435101P-1          |
    |                                                                                                 |
    |     _                                                                                     _     |
    LD14 (_)                                       _                             ___________   (_) LD01
    |                                         IL5 (_)                            |TMS2716C |          |
    |       _                                (unused)            _               |_________|  _       |
    | LD13 (_)                          _                   IL1 (_)              ___________ (_) LD02 |
    |                              IL4 (_)                                       | R6504P   |         |
    |                      _________  _________  _________  _________  _________ |__________|         |
    |            _         SCL4093BE  HM74C173N  HM74C173N  HM74C173N  HM74C173N          _           |
    |      LD12 (_)                                                                      (_) LD03     |
    |                                                                                                 |
    |                   _                                                         _                   |
    |             LD11 (_)                                                       (_) LD04             |
    |                               _                                _                                |
    |                         LD10 (_)                              (_) LD05                          |
    |                                           _         _                                           |
    |                                     LD07 (_)       (_) LD06 (unused)                            |
    |_________________________________________________________________________________________________|

 LD = leds
 IL = light bulbs

****************************************************************************************************************/

#include "emu.h"

#include "cpu/m6502/m6504.h"

#include "speaker.h"

namespace {

class elcirculo_state : public driver_device
{

public:
	elcirculo_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void elcirculo(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;

	INTERRUPT_GEN_MEMBER(irq_gen);

	void program_map(address_map &map) ATTR_COLD;
};

INTERRUPT_GEN_MEMBER(elcirculo_state::irq_gen)
{
	m_maincpu->set_input_line(M6504_IRQ_LINE, HOLD_LINE);
}

void elcirculo_state::program_map(address_map &map)
{
	map.global_mask(0x1fff);

	map(0x0000, 0x01ff).ram();
	map(0x0200, 0x0200).portr("IN0");
	map(0x0201, 0x0201).portr("DSW1");
	// map(0x0400, 0x0401).w();
	// map(0x0600, 0x0601).w();
	map(0x1000, 0x17ff).mirror(0x800).rom().region("maincpu", 0);
}

INPUT_PORTS_START(elcirculo)
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
INPUT_PORTS_END

void elcirculo_state::elcirculo(machine_config &config)
{
	M6504(config, m_maincpu, 1'000'000); // R6504P, internal clock
	m_maincpu->set_addrmap(AS_PROGRAM, &elcirculo_state::program_map);
	m_maincpu->set_periodic_int(FUNC(elcirculo_state::irq_gen), attotime::from_hz(60)); // TODO: source of this

	SPEAKER(config, "mono").front_center();
}

ROM_START( elcirculo )
	ROM_REGION( 0x800, "maincpu", 0 )
	ROM_LOAD( "tms2716c.ic2", 0x000, 0x800, CRC(7b5ae97f) SHA1(e7276d5d97328628889d216beac0b04216bf82c7) )
ROM_END

} // anonymous namespace

GAME(1980, elcirculo, 0, elcirculo, elcirculo, elcirculo_state, empty_init, ROT0, "Inder", "El Circulo", MACHINE_IS_SKELETON)
