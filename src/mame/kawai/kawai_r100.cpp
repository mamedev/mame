// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Skeleton driver for Kawai R-100 drum machine.

***************************************************************************/

#include "emu.h"
#include "cpu/m6502/m3745x.h"
#include "machine/nvram.h"

class kawai_r100_state : public driver_device
{
public:
	kawai_r100_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void r100(machine_config &config);

private:
	void main_map(address_map &map);
	//void data_map(address_map &map);

	required_device<m740_device> m_maincpu;
};


void kawai_r100_state::main_map(address_map &map)
{
	map(0x0000, 0x1fff).ram().share("nvram1");
	map(0x2000, 0x3fff).ram().share("nvram2");
	map(0x4400, 0xffff).rom().region("program", 0x4400);
}


static INPUT_PORTS_START(r100)
INPUT_PORTS_END

void kawai_r100_state::r100(machine_config &config)
{
	M37450(config, m_maincpu, 16_MHz_XTAL / 2); // FIXME: M50734SP
	m_maincpu->set_addrmap(AS_PROGRAM, &kawai_r100_state::main_map);
	//m_maincpu->set_addrmap(AS_DATA, &kawai_r100_state::data_map);

	NVRAM(config, "nvram1", nvram_device::DEFAULT_ALL_0); // MB8464-15LL-SK + battery
	NVRAM(config, "nvram2", nvram_device::DEFAULT_ALL_0); // MB8464-15LL-SK + battery

	//M60009_AGU_DGU(config, "pcm", 5_MHz_XTAL);
}

ROM_START(r100)
	ROM_REGION(0x10000, "program", 0)
	ROM_SYSTEM_BIOS(0, "c", "Revision C")
	ROMX_LOAD("kawai_6p13c.u18", 0x00000, 0x08000, CRC(3d7ed644) SHA1(d47d6c866d19390d781287ae7a69962c4503ce8e), ROM_BIOS(0))
	ROMX_LOAD("kawai_6p14c.u16", 0x08000, 0x08000, CRC(61757eca) SHA1(24331ed8d5842d65ad1b2a472fb708e8c5a140c7), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "b", "Revision B")
	ROMX_LOAD("kawai_6p13b.u18", 0x00000, 0x08000, CRC(53d288a0) SHA1(25d5bcaf4b5146d13c9d75313d3110776dd4e237), ROM_BIOS(1)) // D27C256AD-20
	ROMX_LOAD("kawai_6p14b.u16", 0x08000, 0x08000, CRC(777889bb) SHA1(30fb93afcc2e8738bcc6f991081f269953e49b15), ROM_BIOS(1)) // D27C256AD-20

	ROM_REGION(0x80000, "pcm", 0)
	ROM_LOAD("kawai_mn234001kaa.u20", 0x00000, 0x80000, CRC(aaf1805e) SHA1(5894b4cb03e17a5aa8c2b0c9b1b3d9285009a1c3))
ROM_END

SYST(1987, r100, 0, 0, r100, r100, kawai_r100_state, empty_init, "Kawai Musical Instrument Manufacturing", "R-100 Digital Drum Machine", MACHINE_IS_SKELETON)
