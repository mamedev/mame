// license:BSD-3-Clause
// copyright-holders:
/**************************************************************************************************

Adaptec AHA-2940AU PCI

Based off AIC-7860/7861

References:
- https://github.com/torvalds/linux/tree/master/drivers/scsi/aic7xxx

TODO:
- Stub, enough to make it surpass initial POST and nothing else;
- Lack specific documentation for AIC-7860, we base on AIC-7890 for now;

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
	, m_eeprom(*this, "eeprom")
	, m_scsi_rom(*this, "scsi_rom")
{
	// TODO: unknown revision
	// class code should be 0x01'0000 as per AIC-7890 manual
	set_ids(0x90046178, 0x00, 0x010000, 0x90046178);
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
	ROM_REGION16_LE( 0x80, "eeprom", ROMREGION_ERASEFF )
	ROM_LOAD( "93c46.u1", 0x00, 0x80, CRC(c82242ce) SHA1(93311b8c02e87492a40c1df6ea4eb60628bb2825) )
ROM_END

const tiny_rom_entry *aha2940au_scsi_device::device_rom_region() const
{
	return ROM_NAME(aha2940au);
}

void aha2940au_scsi_device::device_add_mconfig(machine_config &config)
{
	// AIC-7860Q

	EEPROM_93C46_16BIT(config, m_eeprom);
}

void aha2940au_scsi_device::device_start()
{
	pci_card_device::device_start();

	// TODO: BAR setup from AIC-7890, verify
	add_map( 256, M_IO, FUNC(aha2940au_scsi_device::io_map));
	// TODO: enables M_64A on demand
	add_map( 4096, M_MEM, FUNC(aha2940au_scsi_device::mem_map));

	add_rom((u8 *)m_scsi_rom->base(), 0x8000);
	expansion_rom_base = 0xc8000;

	// TODO: BIST capable

	// INTA#
	intr_pin = 1;
	// TODO: definitely uses PCI regs 0x3e/0x3f
	// max_lat = 0x19, min_gnt = 0x27 for AIC-7890
	// others may be different
}

void aha2940au_scsi_device::device_reset()
{
	pci_card_device::device_reset();

	command = 0x0000;
	// Can be bus master (bit 2), can use Memory Write and Invalidate (bit 4)
	command_mask = 0x17;
	// cap list (bit 4), fast back-to-back (bit 7), medium DEVSEL#
	status = 0x0290;

	remap_cb();
}

u8 aha2940au_scsi_device::capptr_r()
{
	return 0xdc;
}

void aha2940au_scsi_device::config_map(address_map &map)
{
	pci_card_device::config_map(map);
//	map(0x40, 0x40) DEVCONFIG
//	map(0x41, 0x41) DEVSTATUS0
//	map(0x42, 0x42) DEVSTATUS1
//	map(0x43, 0x43) PCIERRGEN

	// PME 1.0
	map(0xdc, 0xdf).lr32(NAME([] { return 0x0001'0001; }));
//	map(0xe0, 0xe1) PM_CSR
//	map(0xe2, 0xe2) PMCSR_BSE
//	map(0xe3, 0xe3) PM_DATA
//	map(0xff, 0xff) IDENREG
}

void aha2940au_scsi_device::io_map(address_map &map)
{
	// SEECTL
	map(0x1e, 0x1e).lrw8(
		NAME([this] (offs_t offset) {
			return 0x10 | m_eeprom->do_read();
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("SEECTL %02x\n", data);
			m_eeprom->di_write(BIT(data, 1));
			m_eeprom->clk_write(BIT(data, 2));
			m_eeprom->cs_write(BIT(data, 3));
		})
	);
	// HCNTRL
	map(0x87, 0x87).lr8(NAME([] () { return 0xff; }));
}

void aha2940au_scsi_device::mem_map(address_map &map)
{
}

