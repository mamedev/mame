// license:BSD-3-Clause
// copyright-holders:
/**************************************************************************************************

Adaptec AHA-2940AU PCI

Based off AIC-7860/7861

TODO:
- Plays with southbridge SMI & APMCAPMS regs soon after entering in its own BIOS (PC=ccfe1),
  going off the track blanking the almost entirety of I/O space range.

**************************************************************************************************/

#include "emu.h"
#include "aha2940au.h"

#define LOG_WARN      (1U << 1)

#define VERBOSE (LOG_GENERAL | LOG_WARN)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

#define LOGWARN(...)            LOGMASKED(LOG_WARN, __VA_ARGS__)


DEFINE_DEVICE_TYPE(AHA2940AU, aha2940au_scsi_device,   "aha2940au",   "Adaptec AHA-2940AU SCSI controller")



aha2940au_scsi_device::aha2940au_scsi_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: pci_card_device(mconfig, type, tag, owner, clock)
	, m_scsi_rom(*this, "scsi_rom")
{
	// TODO: unknown revision & class code
	set_ids(0x90046178, 0x00, 0x010700, 0x90046178);
}

aha2940au_scsi_device::aha2940au_scsi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: aha2940au_scsi_device(mconfig, AHA2940AU, tag, owner, clock)
{
}

// NOTE: BIOS is FAT compressed, decompress as 8bios.bin under the hood.
ROM_START( aha2940au )
	ROM_REGION32_LE( 0x8000, "scsi_rom", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0, "v1.30", "AHA-2940AU (v1.30)" )
	// "589247-00_c_bios_2400_(C)1996_v1.30.u3"
	ROMX_LOAD( "589247-00.u3", 0x0000, 0x8000, CRC(de00492b) SHA1(a6015bcd51e51015a5710d7ac1929e28bf033db4), ROM_BIOS(0) )

	// default eeprom contents, to be verified
	ROM_REGION( 0x80, "eeprom", ROMREGION_ERASEFF )
	ROM_LOAD( "93c46.u1", 0x00, 0x80, CRC(c82242ce) SHA1(93311b8c02e87492a40c1df6ea4eb60628bb2825) )
ROM_END

const tiny_rom_entry *aha2940au_scsi_device::device_rom_region() const
{
	return ROM_NAME(aha2940au);
}

void aha2940au_scsi_device::device_add_mconfig(machine_config &config)
{
	// AIC-7860Q
	// 93C46 EEPROM
}

void aha2940au_scsi_device::device_start()
{
	pci_card_device::device_start();

	// TODO: unknown BAR setup
//  add_map( size, M_MEM, FUNC(aha2940au_scsi_device::map));

	add_rom((u8 *)m_scsi_rom->base(), 0x8000);
	expansion_rom_base = 0xc8000;

	// TODO: unknown irq pin
//  intr_pin = 1;
}

void aha2940au_scsi_device::device_reset()
{
	pci_card_device::device_reset();

	// TODO: unknown startup state
	command = 0x0000;
	status = 0x0200;

	remap_cb();
}

void aha2940au_scsi_device::config_map(address_map &map)
{
	pci_card_device::config_map(map);
	// ...
}
