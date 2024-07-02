// license:BSD-3-Clause
// copyright-holders:
/***********************************************************************************************************

Skeleton driver for "Cut The Rope", by Innovative Concepts in Entertainment, Inc. (ICE).
Redemption game with a vertical touch sceen and ticket payout, from 2016.
ICE also produced a "Cut the Rope" electromechanical crane arcade machine in 2013, with a suction
cup rather than a claw.

Standard PC with Intel DH61CR motherboard (IT8892E PCIe to PCI Bridge, Nuvoton W83677HG-i Super I/O, etc.):
-Unknown RAM, unknown exact CPU type.
-No security dongle needed (at least on the dumped machine).

And an external PCB for I/O, connected by USB to the motherboard:
-ColdFire MCF51JM32VLK.
-Two banks of 8 DIP switches.

***********************************************************************************************************/

#include "emu.h"
#include "cpu/i386/i386.h"
#include "machine/pci.h"


namespace {

class cuttherope_state : public driver_device
{
public:
	cuttherope_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }


	void cuttherope(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;

	void cuttherope_io(address_map &map);
	void cuttherope_map(address_map &map);
};


void cuttherope_state::cuttherope_map(address_map &map)
{
}

void cuttherope_state::cuttherope_io(address_map &map)
{
}

static INPUT_PORTS_START(cuttherope)
INPUT_PORTS_END

void cuttherope_state::cuttherope(machine_config &config)
{
	PENTIUM4(config, m_maincpu, 100'000'000); // Exact CPU and frequency unknown
	m_maincpu->set_addrmap(AS_PROGRAM, &cuttherope_state::cuttherope_map);
	m_maincpu->set_addrmap(AS_IO, &cuttherope_state::cuttherope_io);

	PCI_ROOT(config, "pci", 0);
	// ...
}


ROM_START(cuttherope)
	ROM_REGION32_LE(0x582020, "bios", 0)
	ROM_LOAD("be0118.bin", 0x000000, 0x582020, BAD_DUMP CRC(c6b94784) SHA1(f689d45309f2b6825f660623414bcb2ca118c6b1)) // BIOS update from Internet, not dumped from the actual machine

	ROM_REGION(0x40000, "iomcu", 0)
	ROM_LOAD("mcf51jm32vlk", 0x00000, 0x40000, NO_DUMP) // On the I/O PCB

	DISK_REGION("ide:0:hdd")
	DISK_IMAGE("cuttherope", 0, SHA1(25be8285ca75d759962f4b59b313a97d9077747e))
ROM_END

} // anonymous namespace


GAME(2016, cuttherope, 0, cuttherope, cuttherope, cuttherope_state, empty_init, ROT90, "ICE", "Cut The Rope", MACHINE_IS_SKELETON )
