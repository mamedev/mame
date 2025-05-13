// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Skeleton driver for MITS Altair 8800b.

***************************************************************************/

#include "emu.h"
//#include "bus/s100/s100.h"
#include "cpu/i8085/i8085.h"

namespace {

class al8800b_state : public driver_device
{
public:
	al8800b_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void al8800b(machine_config &config);

private:
	required_device<i8080a_cpu_device> m_maincpu;
};

static INPUT_PORTS_START(al8800b)
INPUT_PORTS_END

void al8800b_state::al8800b(machine_config &config)
{
	I8080A(config, m_maincpu, 18_MHz_XTAL / 9);
}

ROM_START(al8800b)
	ROM_REGION(0x100, "panel", 0)
	// This 1702A EPROM is not mapped into the 8080 memory space. It contains custom microcode implementing front panel functions.
	ROM_LOAD("8800b front panel.bin", 0x000, 0x100, CRC(8b462c1b) SHA1(3c13b1cc941225b16580655c15a61b8e8418b052))
ROM_END

} // anonymous namespace

COMP(1976, al8800b, 0, 0, al8800b, al8800b, al8800b_state, empty_init, "MITS", "Altair 8800b", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW)
