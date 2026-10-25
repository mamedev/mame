// license:BSD-3-Clause
// copyright-holders:
/**************************************************************************************************

Skeleton driver for the Sharp UP-X500/X300 POS terminals.

TODO:
- VT8623 + VT8235 chipset;

===================================================================================================

Sharp UP-X500/UP-X300 motherboard:
 -VIA Eden ESP7000 CPU.
 -North VT8623, South VT8235.
 -SIO10N268.
 -UART PC16550DV for the contactless clerk key reader.
 -H8S2138A MCU for POS (undumped), the "PCP III": magnetic card reader, drawers, clerk key and
  RV5C339A RTC. Host interface at I/O 0x7f0-0x7f3.
 -VIA VT6103 Ethernet near a 25 MHz xtal.
 -ICS 951001AF clock near a 14.318 MHz xtal.
 -DS90C363MTD Programmable LVDS Transmitter.
 -MIC2563A + Ricoh R5C486 for PC-Card.
Small subboard with a PIC16F873A (undumped) and a 4 MHz xtal for the "Sharp Contactless Clerk Key
Reader".
HT12X13-100 LCD screen + HTHXLS60T LCD controller.
Small subboard with a NC41120-0017 for the resistive touch panel (USB).

**************************************************************************************************/

#include "emu.h"
#include "cpu/i386/i386.h"
#include "machine/pci.h"


namespace {

class upx500_state : public driver_device
{
public:
	upx500_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void upx500(machine_config &config) ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;

	void main_map(address_map &map) ATTR_COLD;
};


void upx500_state::main_map(address_map &map)
{
	map(0x00000000, 0x0009ffff).ram();
	map(0x000e0000, 0x000fffff).rom().region("bios", 0x60000);
	map(0xfff80000, 0xffffffff).rom().region("bios", 0);
}

void upx500_state::upx500(machine_config &config)
{
	PENTIUM3(config, m_maincpu, 733'000'000); // VIA Eden ESP7000
	m_maincpu->set_addrmap(AS_PROGRAM, &upx500_state::main_map);

	PCI_ROOT(config, "pci");
}


ROM_START(upx500)
	// Phoenix NoteBIOS 4.0 Release 6.0
	ROM_REGION32_LE(0x80000, "bios", 0)
	ROM_LOAD("sst39sf040.ic54", 0x00000, 0x80000, CRC(e39d42f2) SHA1(c0a6a38b77f4323ea8645b53839298567092668f)) // First 8KB may vary (ESCD/ACFG area)

	// Ethernet MAC address
	ROM_REGION(0x80, "eeprom", 0)
	ROM_LOAD("at93c46.ic38", 0x00, 0x80, CRC(251da233) SHA1(af9c078a4c67a1038a98270c47e1bd9bce4c3acc))

	ROM_REGION(0x20000, "pcp3", 0)
	ROM_LOAD("hd64f2138afa20.ic52", 0x00000, 0x20000, NO_DUMP)

	ROM_REGION(0x4300, "clerkkey", 0)
	ROM_LOAD("pic16f873a.ic2", 0x0000, 0x4300, NO_DUMP)
ROM_END

} // anonymous namespace


COMP(2004, upx500, 0, 0, upx500, 0, upx500_state, empty_init, "Sharp", "UP-X500", MACHINE_NO_SOUND | MACHINE_NOT_WORKING)
