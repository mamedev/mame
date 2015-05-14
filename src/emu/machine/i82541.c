// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "i82541.h"

const device_type I82541 = &device_creator<i82541_device>;

DEVICE_ADDRESS_MAP_START(registers_map, 32, i82541_device)
ADDRESS_MAP_END

DEVICE_ADDRESS_MAP_START(flash_map, 32, i82541_device)
ADDRESS_MAP_END

DEVICE_ADDRESS_MAP_START(registers_io_map, 32, i82541_device)
ADDRESS_MAP_END

i82541_device::i82541_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: pci_device(mconfig, I82541, "I82541 ethernet controller", tag, owner, clock, "i82541", __FILE__)
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
