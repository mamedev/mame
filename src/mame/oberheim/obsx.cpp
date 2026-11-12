// license:BSD-3-Clause
// copyright-holders:

/*
Oberheim OB-SX synthesizer
Six voice with 56 programs

Oberheim 1458 B PCB

Z8400 PS CPU
lots of logic chips
*/

#include "emu.h"

#include "cpu/z80/z80.h"


namespace {

class obsx_state : public driver_device
{
public:
	obsx_state(const machine_config &mconfig, device_type type, const char *tag) ATTR_COLD
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void obsx(machine_config &config) ATTR_COLD;

private:
	required_device<z80_device> m_maincpu;

	void program_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
};


void obsx_state::program_map(address_map &map)
{
	map(0x0000, 0x0bff).rom();
	map(0x4000, 0x40ff).ram();
}

void obsx_state::io_map(address_map &map)
{
	map.unmap_value_high();
}


INPUT_PORTS_START(obsx)
INPUT_PORTS_END


void obsx_state::obsx(machine_config &config)
{
	Z80(config, m_maincpu, 4'000'000); // clock not verified
	m_maincpu->set_addrmap(AS_PROGRAM, &obsx_state::program_map);
	m_maincpu->set_addrmap(AS_IO, &obsx_state::io_map);
}


ROM_START( obsx )
	ROM_REGION( 0xc00, "maincpu", 0 )
	ROM_LOAD( "sx-c0.30", 0x000, 0x400, CRC(8d35d2d2) SHA1(94baafb1b17c00c396a98ce69e18078294fe1710) )
	ROM_LOAD( "sxc1.31",  0x400, 0x400, CRC(f6325457) SHA1(ae6749aa6c79c3bd16310616387b7d262f767b59) )
	ROM_LOAD( "sxc2.32",  0x800, 0x400, CRC(1471d3b6) SHA1(1c5ec479986e98a3ca6ea38d9c14d2d418a38f02) )
ROM_END

} // anonymous namespace


SYST( 1980, obsx, 0, 0, obsx, obsx, obsx_state, empty_init, "Oberheim", "OB-SX", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
