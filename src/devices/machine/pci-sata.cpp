// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "emu.h"
#include "pci-sata.h"

DEFINE_DEVICE_TYPE(SATA, sata_device, "sata", "SATA AHCI interface")

sata_device::sata_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_device(mconfig, SATA, tag, owner, clock)
{
}

void sata_device::primary_command_map(address_map &map)
{
}

void sata_device::primary_control_map(address_map &map)
{
}

void sata_device::secondary_command_map(address_map &map)
{
}

void sata_device::secondary_control_map(address_map &map)
{
}

void sata_device::bus_master_map(address_map &map)
{
}

void sata_device::ide_command_posting_map(address_map &map)
{
}


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
