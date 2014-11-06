#include "pci-smbus.h"

const device_type SMBUS = &device_creator<smbus_device>;

DEVICE_ADDRESS_MAP_START(map, 32, smbus_device)
ADDRESS_MAP_END

smbus_device::smbus_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: pci_device(mconfig, SMBUS, "SMBUS interface", tag, owner, clock, "smbus", __FILE__)
{
}

void smbus_device::device_start()
{
	pci_device::device_start();
	add_map(32, M_IO, FUNC(smbus_device::map));
}

void smbus_device::device_reset()
{
	pci_device::device_reset();
}
