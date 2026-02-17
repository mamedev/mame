// license:BSD-3-Clause
// copyright-holders:
/**************************************************************************************************

Skeleton driver for Arachnid "Super Shuffle" (electromechanical + video).

Based on an Asus "M2N68-AM PLUS" PCB:
- AMD Sempron SDX140HBK13GQ CPU.
- Nvidia MCP68S
- Realtek ALC662 audio
- Realtek RTL8211C network
- ITE IT8712F Super I/O
- 2x PCI
- 1x PCIe x1
- 1x PCIe x16
- 1024MB DDR2 UDIMM RAM.
- GeForce 7025 / nForce 630a

Arachnid I/O PCB (P/N 42688):
- PIC18F4520 near a 40MHz crystal.
- DB9 serial port (controlled by a MAX232A).
- Buttons for reset and service.
- Two custom connectors for left (blue) and right (yellow) laser sensors.

Linux operating system.

Game starts with the following message on screen:
  "It's better to be wanted for murder that not to be wanted at all. - Marty Winch"

**************************************************************************************************/

#include "emu.h"
#include "cpu/i386/i386.h"
#include "machine/pci.h"


namespace {

class sshuffle_state : public driver_device
{
public:
	sshuffle_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }


	void sshuffle(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;

	void sshuffle_io(address_map &map) ATTR_COLD;
	void sshuffle_map(address_map &map) ATTR_COLD;
};


void sshuffle_state::sshuffle_map(address_map &map)
{
	map(0x00000000, 0x0009ffff).ram();
	map(0x000e0000, 0x000fffff).rom().region("bios", 0xe0000);
	map(0xfff00000, 0xffffffff).rom().region("bios", 0);
}

void sshuffle_state::sshuffle_io(address_map &map)
{
}

void sshuffle_state::sshuffle(machine_config &config)
{
	// Socket AM2 / AM2+ / AM3
	// TODO: doesn't map anything with ATHLONXP (wants PCI_ROOT stuff?)
	PENTIUM4(config, m_maincpu, 100'000'000); // Actually an AMD Sempron SDX140HBK13GQ
	m_maincpu->set_addrmap(AS_PROGRAM, &sshuffle_state::sshuffle_map);
	m_maincpu->set_addrmap(AS_IO, &sshuffle_state::sshuffle_io);

	PCI_ROOT(config, "pci", 0);
	// ...
}


ROM_START(sshuffle)
	ROM_REGION32_LE(0x100000, "bios", 0)
	ROM_LOAD("en25f80.bin", 0x000000, 0x100000, CRC(256357c2) SHA1(f7dc38e1d475ab4ebfddb716488b325651e4049d))

	ROM_REGION(0x8000, "io", 0)
	ROM_LOAD("iob_pic18f4520.bin", 0x0000, 0x8000, CRC(e6c32bc4) SHA1(619e30dd5d3d8da15e1dc8d03a3cb996981464a0))

	DISK_REGION("ide:0:hdd")
	DISK_IMAGE("shuffleboard_bowling", 0, BAD_DUMP SHA1(6fcc7b5e525c9a3083e29edfb6657664c9c4c9d4)) // Image tested on real hardware, but may contain operator or players data
ROM_END

} // anonymous namespace


GAME(2011, sshuffle, 0, sshuffle, 0, sshuffle_state, empty_init, ROT0, "Arachnid", "Super Shuffle", MACHINE_NO_SOUND | MACHINE_NOT_WORKING)
