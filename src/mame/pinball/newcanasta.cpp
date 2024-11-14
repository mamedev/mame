// license:BSD-3-Clause
// copyright-holders:
/*
    New Canasta, pinball with PC-based electronics, from the Spanish company Marsaplay.

    AsRock Conroe 1333-d677 Pressler P4FSB1333-650 motherboard.
    USB-DIO-96 (96 channel digital I/O module from "Access I/O Products, Inc.".
*/

#include "emu.h"
#include "cpu/i386/i386.h"
//#include "cpu/mcs51/mcs51.h"
#include "machine/pci.h"

namespace {

class newcanasta_state : public driver_device
{
public:
	newcanasta_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void newcanasta(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;

	void newcanasta_map(address_map &map) ATTR_COLD;
};



void newcanasta_state::newcanasta_map(address_map &map)
{
	map(0x00000000, 0x0009ffff).ram();
	map(0x000e0000, 0x000fffff).rom().region("bios", 0x60000);
	map(0xfff80000, 0xffffffff).rom().region("bios", 0);
}

static INPUT_PORTS_START( newcanasta )
INPUT_PORTS_END



void newcanasta_state::newcanasta(machine_config &config)
{
	// Basic machine hardware
	PENTIUM4(config, m_maincpu, 100'000'000); // 775-pin LGA "Socket T" CPU, exact model unknown
	m_maincpu->set_addrmap(AS_PROGRAM, &newcanasta_state::newcanasta_map);

	PCI_ROOT(config, "pci", 0);
	// ...

	// I/O board
	//I80C51(config, m_maincpu, 24.0000_MHz_XTAL); // USB-DIO-96 with a CY7C68013A MCU (MCS51 core)
}

/***************************************************************************

  Game drivers

***************************************************************************/

ROM_START(newcanasta)
	ROM_REGION32_LE(0x80000, "bios", 0)
	ROM_SYSTEM_BIOS( 0, "110", "v1.10")
	ROMX_LOAD("p4f136_1.10.bin", 0x00000, 0x80000, CRC(88e558e7) SHA1(ea4305cf7a6373711dad21e1de0e208b62f2d7de), ROM_BIOS(0))
	ROM_SYSTEM_BIOS( 1, "100", "v1.00")
	ROMX_LOAD("p4f136_1.00.bin", 0x00000, 0x80000, CRC(63968e55) SHA1(7dd49be078abc55422639e145c06505f48e2abb2), ROM_BIOS(1))

	// External ROM for the CY7C68013A MCU on the USB I/O board
	ROM_REGION(0x2000, "usbio", 0)
	ROM_LOAD("24lc64.u4", 0x0000, 0x2000, NO_DUMP)

	DISK_REGION("ide:0:hdd") // Seagate ST320410A
	DISK_IMAGE("newcanasta", 0, BAD_DUMP SHA1(7b18a07925cf62d0fcf25fab6e65897eddc45e4e)) // Contains players and operator data
ROM_END

} // Anonymous namespace


GAME(2010, newcanasta, 0, newcanasta, newcanasta, newcanasta_state, empty_init, ROT0, "Marsaplay", "New Canasta", MACHINE_IS_SKELETON_MECHANICAL)
