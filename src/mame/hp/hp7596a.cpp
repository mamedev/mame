// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Skeleton driver for Hewlett-Packard 7596A drafting plotter.

***************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"

namespace {

class hp7596a_state : public driver_device
{
public:
	hp7596a_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void hp7596a(machine_config &config);

private:
	void main_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
};


void hp7596a_state::main_map(address_map &map)
{
	map(0x000000, 0x01ffff).rom().region("program", 0);
	map(0x37c000, 0x383fff).ram();
}


static INPUT_PORTS_START(hp7596a)
INPUT_PORTS_END

void hp7596a_state::hp7596a(machine_config &config)
{
	M68000(config, m_maincpu, 9'980'000); // SCN68000CAN64
	m_maincpu->set_addrmap(AS_PROGRAM, &hp7596a_state::main_map);

	//EEPROM_2804(config, "eeprom"); // X2804AP-45

	//Z80DART(config, "dart"); // Z8470BPS

	//I8291A(config, "gpibc");
}

ROM_START(hp7596a)
	ROM_REGION16_BE(0x20000, "program", 0)
	ROM_LOAD16_BYTE("07595-18096_u37.bin", 0x00000, 0x10000, CRC(b1df8181) SHA1(d7ef1edf3f4b9f5a7e0fdca0bf6b72a3118da9c2))
	ROM_LOAD16_BYTE("07595-18095_u28.bin", 0x00001, 0x10000, CRC(3588c029) SHA1(cc4b4838531b9309a3fc27102c19393653b08ae4))
ROM_END

} // anonymous namespace


SYST(1988, hp7596a, 0, 0, hp7596a, hp7596a, hp7596a_state, empty_init, "Hewlett-Packard", "HP 7596A DraftMaster II", MACHINE_IS_SKELETON)
