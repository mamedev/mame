// license:BSD-3-Clause
// copyright-holders:
/**************************************************************************************************

Aristocrat MK-7

Custom PC-based motherboard labeled MK7 CARRIER BOARD:
- Celeron D 1.6 GHz;
- Intel 82945GM (northbridge + Calistoga GPU);
- Intel NH82801GBM (SL8YB);
- Intel 82573L;
- 2x 512MB SODIMMs;
- CF slot, dumps are in ext2 format;
- 2 SATA slots, apparently some games uses these instead;
- SIM card slot (dongle, each game is tied with a SIM, presumably have files inside)
- USB port (2.0 x1?), 2x 8-pin socketed chips (?)
- A serial port near JTAG labels, a 2x 7-seg LEDs and x2 Panasonic BR-2/3A batteries labeled 
  BT1 / BT2;
- U57 chip near the batteries (RTC?);
- XMULTIPLE XMH-TRJG 1730AONL ethernet port;
- PCIe 2.0 x16 + x1 slot;
- Altera EPM570 CPLD, Altera Cyclone II;
- 3x onboard buttons labeled 522 T (L25, L26, L32);
- (unconfirmed) Viridian video card a.k.a. nVidia GeForce GT430 (GF108)

Apparently two PCB revisions, one is red coated.

**************************************************************************************************/


#include "emu.h"
#include "cpu/i386/i386.h"
#include "machine/pci.h"


namespace {

class aristmk7_state : public driver_device
{
public:
	aristmk7_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }


	void aristmk7(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	void aristmk7_io(address_map &map);
	void aristmk7_map(address_map &map);
};


void aristmk7_state::aristmk7_map(address_map &map)
{
	map(0x00000000, 0x0009ffff).ram();
	map(0x000e0000, 0x000fffff).rom().region("bios", 0xe0000);
	map(0xfff00000, 0xffffffff).rom().region("bios", 0);
}

void aristmk7_state::aristmk7_io(address_map &map)
{
}

static INPUT_PORTS_START(aristmk7)
INPUT_PORTS_END

void aristmk7_state::aristmk7(machine_config &config)
{
	PENTIUM3(config, m_maincpu, XTAL(33'868'800) * 5); /* Celeron D, 1.69 GHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &aristmk7_state::aristmk7_map);
	m_maincpu->set_addrmap(AS_IO, &aristmk7_state::aristmk7_io);

	PCI_ROOT(config, "pci", 0);
	// ...
}

ROM_START(aristmk7)
	ROM_REGION32_LE(0x100000, "bios", 0)
	// Phoenix based BIOS
	ROM_LOAD( "mk7-bios_590176_01_1.00.7.u1", 0x000000, 0x100000, CRC(f2fea07e) SHA1(285038473fbe3a5708045e2f8b14562f3c24b203) )
ROM_END

} // anonymous namespace


GAME(2007, aristmk7, 0, aristmk7, aristmk7, aristmk7_state, empty_init, ROT0, "Aristocrat", "Aristocrat MK-7 BIOS", MACHINE_IS_SKELETON )// | MACHINE_IS_BIOS_ROOT )
