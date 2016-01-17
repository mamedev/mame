// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "pci-apic.h"

const device_type APIC = &device_creator<apic_device>;

apic_device::apic_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: pci_device(mconfig, APIC, "I/O Advanced Programmable Interrupt Controller", tag, owner, clock, "apic", __FILE__)
{
}

void apic_device::device_start()
{
	pci_device::device_start();
}

void apic_device::device_reset()
{
	pci_device::device_reset();
}
