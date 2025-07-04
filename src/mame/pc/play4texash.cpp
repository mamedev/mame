// license:BSD-3-Clause
// copyright-holders:
/**********************************************************************************************************************************

Skeleton driver for "Play 4 Texas Hold'em", by Sleic.

Based on an Innocore / Advantech "E105MB/B" PCB (specific for embedded gaming systems):
-CPU Intel Atom N270 SLB73, 1.6 GHz / 512 / 533.
-512MB RAM.
-Safenet Sentinel USB security dongle.
-Intel NH82801GBM chipset, RTL8111CP Ethernet controller, IT8718F-S EC - LPC I/O, GL826/MX2AE12G12 USB 2.0 card reader controller.
-PIC16F54 near a 4MHz xtal, Lattice ispMACH LC4256V near a 40MHz xtal, PIC16LF747, Lattice ispMACH LC4384V, CY62157EV3 SRAM.

The machine has two anti-tamper switches directly connected to the PCB.

Linux-based operating system called "LudOS".

***********************************************************************************************************************************/

#include "emu.h"
#include "cpu/i386/i386.h"
#include "machine/pci.h"


namespace {

class play4texash_state : public driver_device
{
public:
	play4texash_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }


	void play4texash(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;

	void play4texash_io(address_map &map) ATTR_COLD;
	void play4texash_map(address_map &map) ATTR_COLD;
};


void play4texash_state::play4texash_map(address_map &map)
{
}

void play4texash_state::play4texash_io(address_map &map)
{
}

static INPUT_PORTS_START(play4texash)
INPUT_PORTS_END

void play4texash_state::play4texash(machine_config &config)
{
	PENTIUM4(config, m_maincpu, 100'000'000); // Intel Atom N270 SLB73, 1.6 GHz / 512 / 533
	m_maincpu->set_addrmap(AS_PROGRAM, &play4texash_state::play4texash_map);
	m_maincpu->set_addrmap(AS_IO, &play4texash_state::play4texash_io);

	PCI_ROOT(config, "pci", 0);
	// ...
}


ROM_START(play4texash)
	ROM_REGION32_LE(0x100000, "bios", 0)
	ROM_LOAD("e1051005_igl_49lf008a.u15", 0x000000, 0x100000, CRC(f6996b6d) SHA1(dbe8dce732ec941eb298ae016b74e49c8465783c))

	ROM_REGION32_LE(0x080, "eeprom", 0)
	ROM_LOAD("at93c46.u16", 0x000, 0x080, CRC(379af802) SHA1(60dc137ad06815feb3e9de3f59a3e486e16dd5f1)) // For storing BIOS settings?

	ROM_REGION(0x1000, "pics", 0)
	ROM_LOAD("56.21_pic16lf747.u13", 0x0000, 0x1000, NO_DUMP)
	ROM_LOAD("a5_pic16f54.u28",      0x0000, 0x0800, CRC(2901d211) SHA1(9b4bc7c490b073e95e748ae7cdf6e8e07025da02))

	ROM_REGION(0x50000, "plds", 0)
	ROM_LOAD("15-02_lc4256v.u4.jed", 0x00000, 0x2d182, CRC(954e2812) SHA1(573b17d85058512ace9a50cfee510580e6bf7d23)) // Lattice ispMACH LC4256V. Not converted to BIN because jedutil does not support it
	ROM_LOAD("81-lc4384v.u31.jed",   0x00000, 0x4be4d, CRC(c4ded582) SHA1(d24e10301338207f8342963b9330820994fb05de)) // Lattice ispMACH LC4384V. Not converted to BIN because jedutil does not support it

	DISK_REGION("ide:0:hdd")
	DISK_IMAGE("sleic_play_4_texas_holdem_v2.1.4_12-00314", 0, SHA1(6cbf515ee80a68c98a13be02b17c9d306f4e1445)) // Transcend CompactFlash 133x 1GB
ROM_END

} // anonymous namespace


GAME(2010, play4texash, 0, play4texash, play4texash, play4texash_state, empty_init, ROT0, "Sleic", "Play 4 Texas Hold'em", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
