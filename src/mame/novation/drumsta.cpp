// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for Novation Drum Station drum machine.

****************************************************************************/

#include "emu.h"
#include "cpu/adsp2100/adsp2100.h"
#include "cpu/mn1880/mn1880.h"
#include "machine/eeprompar.h"


namespace {

class drumsta_state : public driver_device
{
public:
	drumsta_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_dsp(*this, "dsp")
	{
	}

	void drumsta(machine_config &config);

private:
	void drumsta_prog(address_map &map) ATTR_COLD;
	void drumsta_data(address_map &map) ATTR_COLD;

	required_device<mn1880_device> m_maincpu;
	required_device<adsp2181_device> m_dsp;
};


void drumsta_state::drumsta_prog(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("program", 0);
}

void drumsta_state::drumsta_data(address_map &map)
{
	map(0x0001, 0x0001).noprw();
	map(0x0003, 0x0003).noprw();
	map(0x000e, 0x000f).noprw();
	map(0x0060, 0x031f).ram();
	map(0xb000, 0xb7ff).rw("eeprom", FUNC(eeprom_parallel_28xx_device::read), FUNC(eeprom_parallel_28xx_device::write));
}


static INPUT_PORTS_START(drumsta)
INPUT_PORTS_END

void drumsta_state::drumsta(machine_config &config)
{
	MN1880(config, m_maincpu, 8000000); // type and clock unknown (custom silkscreen)
	m_maincpu->set_addrmap(AS_PROGRAM, &drumsta_state::drumsta_prog);
	m_maincpu->set_addrmap(AS_DATA, &drumsta_state::drumsta_data);

	ADSP2181(config, m_dsp, 16000000).set_disable(); // clock unknown

	EEPROM_2864(config, "eeprom"); // Atmel AT28C64
}

ROM_START(drumsta)
	ROM_REGION(0x8000, "program", 0)
	ROM_LOAD("v1.2.u15", 0x0000, 0x8000, CRC(ab9a8654) SHA1(1914254c714d35031a456c71b1f9f9191df9126d))

	ROM_REGION(0x80000, "samples", 0)
	ROM_LOAD("v1.2.u28", 0x00000, 0x80000, CRC(dbbc9cfe) SHA1(61474c0bc6cfff3efe95527c57e4891f886b02aa))
ROM_END

} // anonymous namespace


SYST(1995, drumsta, 0, 0, drumsta, drumsta, drumsta_state, empty_init, "Novation", "Drum Station", MACHINE_IS_SKELETON)
