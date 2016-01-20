// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "pci-sata.h"

const device_type SATA = &device_creator<sata_device>;

sata_device::sata_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: pci_device(mconfig, SATA, "SATA AHCI interface", tag, owner, clock, "sata", __FILE__)
{
}

DEVICE_ADDRESS_MAP_START(primary_command_map, 32, sata_device)
ADDRESS_MAP_END

DEVICE_ADDRESS_MAP_START(primary_control_map, 32, sata_device)
ADDRESS_MAP_END

DEVICE_ADDRESS_MAP_START(secondary_command_map, 32, sata_device)
ADDRESS_MAP_END

DEVICE_ADDRESS_MAP_START(secondary_control_map, 32, sata_device)
ADDRESS_MAP_END

DEVICE_ADDRESS_MAP_START(bus_master_map, 32, sata_device)
ADDRESS_MAP_END

DEVICE_ADDRESS_MAP_START(ide_command_posting_map, 32, sata_device)
ADDRESS_MAP_END


void sata_device::device_start()
{
	pci_device::device_start();

	add_map(8,    M_IO,  FUNC(sata_device::primary_command_map));
	add_map(4,    M_IO,  FUNC(sata_device::primary_control_map));
	add_map(8,    M_IO,  FUNC(sata_device::secondary_command_map));
	add_map(4,    M_IO,  FUNC(sata_device::secondary_control_map));
	add_map(16,   M_IO,  FUNC(sata_device::bus_master_map));
	add_map(1024, M_MEM, FUNC(sata_device::ide_command_posting_map));
}

void sata_device::device_reset()
{
	pci_device::device_reset();
}
