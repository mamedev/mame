// license:BSD-3-Clause
// copyright-holders:AJR
/*******************************************************************************

    Skeleton device for Apple external CD-ROM drives.

    PCB: 1-631-957-15

    Microcontrollers:
    - Intel S80C31-1
    - Mitsubishi M37450M8-452FP © SONY '91

    Memories:
    - National NMC27C256Q (or BQ) 32,768x8 EEPROM (for 80C31)
    - Sony CXK5864BM-12L 8,192x8 Static RAM (x2)
      (one near EEPROM, one between LC8951 and M37450M8)
    - Fujitsu MB81464-12 65,536x4 Dynamic RAM (x2: for CXD1184)
    - Fujitsu MB81C79A-45 8,192x9 Static RAM (for LC8951)
    - Sony CXK5816MS-12L 2,048x8 Static RAM (near CXD1135)

    Other major digital ICs:
    - Sony CXD1180AQ SCSI Controller
    - Sony CXD1184Q
    - Sanyo LC8951 CD-ROM Error Correction & Host Interface Processor
    - Sony CXD1135Q

    Linear ICs:
    - Toshiba TC9154AP Dual Volume Control
    - Burr-Brown PCM67U Dual Audio DAC
    - Sony CXA1182Q-Z CD Servo Signal Processor
    - Sony CXA1081M
    - Toshiba TA8406P Dual Power Operational Amplifier (x3)

    XTALs: 12A KSS2A (80C31?), 20A KSS2A (CXD1184), D167 (LC8951?)

*******************************************************************************/

#include "emu.h"
#include "applecd.h"

#include "cpu/mcs51/i80c51.h"
#include "cpu/m6502/m3745x.h"
#include "machine/ncr5380.h"

DEFINE_DEVICE_TYPE(APPLECD150, applecd150_device, "aplcd150", "AppleCD 150")

applecd150_device::applecd150_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, APPLECD150, tag, owner, clock)
	, nscsi_slot_card_interface(mconfig, *this, "scsic")
{
}

void applecd150_device::device_start()
{
}

void applecd150_device::prog_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("eeprom", 0);
}

void applecd150_device::ext_map(address_map &map)
{
	map(0xc020, 0xc027).m("scsic", FUNC(cxd1180_device::map));
}

void applecd150_device::device_add_mconfig(machine_config &config)
{
	i80c31_device &mcu1(I80C31(config, "mcu1", 12_MHz_XTAL));
	mcu1.set_addrmap(AS_PROGRAM, &applecd150_device::prog_map);
	mcu1.set_addrmap(AS_DATA, &applecd150_device::ext_map);

	M37450(config, "mcu2", 16.9344_MHz_XTAL / 2).set_disable();

	CXD1180(config, "scsic").irq_handler().set_inputline("mcu1", MCS51_INT1_LINE);

	//LC8951(config, "cdintf", 16.9344_MHz_XTAL);
}

ROM_START(aplcd150)
	ROM_REGION(0x8000, "eeprom", 0)
	ROM_LOAD("apl_1.8g_1289.ic303", 0x0000, 0x8000, CRC(54ebf81f) SHA1(e4cd656e2a433229543e874a29f1567758170fc6))

	ROM_REGION(0x4000, "mcu2", 0)
	ROM_LOAD("m37450m8-452fp.ic201", 0x0000, 0x4000, NO_DUMP)
ROM_END

const tiny_rom_entry *applecd150_device::device_rom_region() const
{
	return ROM_NAME(aplcd150);
}

