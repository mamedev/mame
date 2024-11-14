// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*
  Rather skeleton driver for a LG GP40NW10 usb dvd writer.

  Main cpu, a MT1839, seems to be a mcs51 variant.
*/

#include "emu.h"
#include "cpu/mcs51/mcs51.h"


namespace {

class lg_dvd_state : public driver_device
{
public:
	lg_dvd_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void lg(machine_config &config);

private:
	void lg_dvd_map(address_map &map) ATTR_COLD;

	required_device<i80c52_device> m_maincpu;
};

static INPUT_PORTS_START( lg )
INPUT_PORTS_END

void lg_dvd_state::lg_dvd_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("maincpu", 0);
}

void lg_dvd_state::lg(machine_config &config)
{
	I80C52(config, m_maincpu, XTAL(16'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &lg_dvd_state::lg_dvd_map);
}

ROM_START( lggp40 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_SYSTEM_BIOS( 0, "bios0", "1.00" )
	ROMX_LOAD( "firm-1.00.bin", 0x000000, 0x100000, CRC(c7f24f3b) SHA1(c2ce96c02ab419fb7e0b38703cdaeeccb2b7f34b), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "bios1", "1.01" )
	ROMX_LOAD( "firm-1.01.bin", 0x000000, 0x100000, CRC(28820e0c) SHA1(c5f2c1e14e6cff2e57c5196cabcebfaaff7284ce), ROM_BIOS(1) )
ROM_END

} // anonymous namespace


SYST( 2011, lggp40, 0, 0, lg, lg, lg_dvd_state, empty_init, "LG", "GP40NW10 dvd writer", MACHINE_NOT_WORKING|MACHINE_NO_SOUND_HW )
