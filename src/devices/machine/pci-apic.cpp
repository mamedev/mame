// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "emu.h"
#include "pci-apic.h"

DEFINE_DEVICE_TYPE(APIC, apic_device, "apic", "I/O Advanced Programmable Interrupt Controller")

apic_device::apic_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_device(mconfig, APIC, tag, owner, clock)
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
