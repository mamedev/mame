// license:BSD-3-Clause
// copyright-holders:
/**************************************************************************************************

Skeleton driver for the Sharp UP-X500/X300 POS terminals.

===================================================================================================

Sharp UP-X500/UP-X300 motherboard:
- VIA Eden ESP7000 CPU;
- VIA VT8623 northbridge;
- VIA VT8235 southbridge;
- SIO10N268;
- UART PC16550DV for the contactless clerk key reader;
- H8S2138A MCU for POS, the "PCP III": magnetic card reader, drawers, clerk key and
  RV5C339A RTC. Host interface at I/O 0x7f0-0x7f3;
- VIA VT6103 Ethernet near a 25 MHz xtal;
- ICS 951001AF clock near a 14.318 MHz xtal;
- DS90C363MTD Programmable LVDS Transmitter;
- MIC2563A + Ricoh R5C486 for PC-Card;
Small subboard with a PIC16F873A and a 4 MHz xtal for the "Sharp Contactless Clerk Key
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

	// VT8235 LAN configuration
	ROM_REGION16_LE(0x80, "eeprom", 0)
	ROM_LOAD("at93c46.ic38", 0x00, 0x80, CRC(287d64ee) SHA1(153f8ac62922812ad12c6e8e3080c5d55be5e6b9))

	ROM_DEFAULT_BIOS("v101")
	ROM_SYSTEM_BIOS(0, "v101", "PCP III v1.01")
	ROM_SYSTEM_BIOS(1, "v100", "PCP III v1.00")
	ROM_REGION(0x20000, "pcp3", 0)
	ROMX_LOAD("hd64f2138afa20_v101.ic52", 0x00000, 0x20000, CRC(f87dbbcd) SHA1(b4364530d5c6d21912278e2e36707e4e500ab02e), ROM_BIOS(0))
	ROMX_LOAD("hd64f2138afa20_v100.ic52", 0x00000, 0x20000, CRC(9d2340df) SHA1(9db431629cd6216df60f11d07b609d547e185bc8), ROM_BIOS(1))

	ROM_REGION(0x4300, "clerkkey", 0)
	ROM_LOAD("pic16f873a.ic2", 0x0000, 0x4300, CRC(7111022b) SHA1(698fb167da1ecf697c2bb2493ce51c4329b31911)) // Data on 0x800–0xbff may vary (application area)
ROM_END

} // anonymous namespace


COMP(2004, upx500, 0, 0, upx500, 0, upx500_state, empty_init, "Sharp", "UP-X500", MACHINE_NO_SOUND | MACHINE_NOT_WORKING)
