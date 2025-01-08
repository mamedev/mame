// license:BSD-3-Clause
// copyright-holders:
/**************************************************************************************************

NCR / Symbios Logic / LSI Logic 53C825A

**************************************************************************************************/

#include "emu.h"
#include "ncr53c825.h"

#define LOG_WARN      (1U << 1)

#define VERBOSE (LOG_GENERAL | LOG_WARN)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

#define LOGWARN(...)            LOGMASKED(LOG_WARN, __VA_ARGS__)


DEFINE_DEVICE_TYPE(NCR53C825_PCI, ncr53c825_pci_device,   "ncr53c825_pci",   "NCR/Symbios Logic/LSI Logic 53C825A PCI")

ncr53c825_pci_device::ncr53c825_pci_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: pci_card_device(mconfig, type, tag, owner, clock)
	, m_scsi_rom(*this, "scsi_rom")
{
	// '825AE straps subvendor with 0x1000'1000, revision 0x26
	set_ids(0x10000003, 0x14, 0x010000, 0x00000000);
}

ncr53c825_pci_device::ncr53c825_pci_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ncr53c825_pci_device(mconfig, NCR53C825_PCI, tag, owner, clock)
{
}

void ncr53c825_pci_device::device_add_mconfig(machine_config &config)
{
	// TODO: SCSI controller, derives from 53C710 according to ROM
}

ROM_START( ncr53c825 )
	ROM_REGION32_LE( 0x10000, "scsi_rom", ROMREGION_ERASEFF )
	// misc/odyssey.cpp SCSI board
	ROM_SYSTEM_BIOS( 0, "odyssey", "NCR SDMS PCI-3.07.00 Lomas Optimum III" )
	ROMX_LOAD( "tms28f512a.u4", 0x0000, 0x10000, CRC(0d5a8054) SHA1(ed22a726976fdc34db6176a1384609f93d50f27a), ROM_BIOS(0) )
ROM_END

const tiny_rom_entry *ncr53c825_pci_device::device_rom_region() const
{
	return ROM_NAME(ncr53c825);
}


void ncr53c825_pci_device::device_start()
{
	pci_card_device::device_start();

	add_map( 128, M_IO, FUNC(ncr53c825_pci_device::io_map));
	add_map( 128, M_MEM, FUNC(ncr53c825_pci_device::io_map));
	add_map( 4*1024, M_MEM, FUNC(ncr53c825_pci_device::scripts_map));
	add_rom((u8 *)m_scsi_rom->base(), 0x8000);
	expansion_rom_base = 0xc8000;

	// INTA#
	intr_pin = 1;

	// TODO: min_gnt = 0x11, max_lat = 0x40
}

void ncr53c825_pci_device::device_reset()
{
	pci_card_device::device_reset();

	command = 0x0000;
	command_mask = 0x157;
	// status = 0x0010; for 'AE only
	status = 0x0000;

	remap_cb();
}

void ncr53c825_pci_device::config_map(address_map &map)
{
	pci_card_device::config_map(map);
//  map(0x40, 0x40) power management capability for 'AE
//  map(0x46, 0x46) bridge support extension for 'A
}

void ncr53c825_pci_device::io_map(address_map &map)
{
}

void ncr53c825_pci_device::scripts_map(address_map &map)
{
}
