// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for Novation BassStation synthesizers.

****************************************************************************/

#include "emu.h"
#include "cpu/mn1880/mn1880.h"
//#include "machine/eeprompar.h"

class basssta_state : public driver_device
{
public:
	basssta_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void bassstr(machine_config &config);
	void sbasssta(machine_config &config);

private:
	void bassstr_prog(address_map &map);
	void sbasssta_prog(address_map &map);
	void bassstr_data(address_map &map);
	void sbasssta_data(address_map &map);

	required_device<mn1880_device> m_maincpu;
};


void basssta_state::bassstr_prog(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("program", 0);
}

void basssta_state::sbasssta_prog(address_map &map)
{
	map(0x0000, 0xffff).rom().region("program", 0);
}

void basssta_state::bassstr_data(address_map &map)
{
}

void basssta_state::sbasssta_data(address_map &map)
{
}


static INPUT_PORTS_START(basssta)
INPUT_PORTS_END

void basssta_state::bassstr(machine_config &config)
{
	MN1880(config, m_maincpu, 8000000); // type and clock unknown (custom silkscreen)
	m_maincpu->set_addrmap(AS_PROGRAM, &basssta_state::bassstr_prog);
	m_maincpu->set_addrmap(AS_DATA, &basssta_state::bassstr_data);
}

void basssta_state::sbasssta(machine_config &config)
{
	MN1880(config, m_maincpu, 8000000); // type and clock unknown (custom silkscreen)
	m_maincpu->set_addrmap(AS_PROGRAM, &basssta_state::sbasssta_prog);
	m_maincpu->set_addrmap(AS_DATA, &basssta_state::sbasssta_data);

	// TODO: Microchip 28C64AF EEPROM
}

ROM_START(bassstr)
	ROM_REGION(0x8000, "program", 0)
	ROM_LOAD("v1.5.bin", 0x0000, 0x8000, CRC(a8bc9721) SHA1(2c2dc3bee0fabf1e77d02cdf3e53885f2c5b913e)) // ST M27C256B (DIP28)
ROM_END

ROM_START(sbasssta)
	ROM_REGION(0x10000, "program", 0)
	ROM_LOAD("v1.9.bin", 0x00000, 0x10000, CRC(045bf9e3) SHA1(69155026c2497a4731162cadb6b441e00902d3f6))
ROM_END

SYST(1995, bassstr, 0, 0,  bassstr,  basssta, basssta_state, empty_init, "Novation", "BassStation Rack Analogue Synthesizer Module", MACHINE_IS_SKELETON)
SYST(1997, sbasssta, 0, 0, sbasssta, basssta, basssta_state, empty_init, "Novation", "Super Bass Station", MACHINE_IS_SKELETON)
