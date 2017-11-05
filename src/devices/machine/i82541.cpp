// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "emu.h"
#include "i82541.h"

DEFINE_DEVICE_TYPE(I82541, i82541_device, "i82541_device", "Intel 82541 Ethernet controller")

DEVICE_ADDRESS_MAP_START(registers_map, 32, i82541_device)
ADDRESS_MAP_END

DEVICE_ADDRESS_MAP_START(flash_map, 32, i82541_device)
ADDRESS_MAP_END

DEVICE_ADDRESS_MAP_START(registers_io_map, 32, i82541_device)
ADDRESS_MAP_END

i82541_device::i82541_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_device(mconfig, I82541, tag, owner, clock)
{
}

void i82541_device::device_start()
{
	pci_device::device_start();
	add_map(128*1024, M_MEM, FUNC(i82541_device::registers_map));
	add_map(128*1024, M_MEM, FUNC(i82541_device::flash_map));
	add_map(32,       M_IO,  FUNC(i82541_device::registers_io_map));
}

void i82541_device::device_reset()
{
	pci_device::device_reset();
}
