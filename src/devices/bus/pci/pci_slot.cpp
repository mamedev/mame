// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Generic PCI card port

#include "emu.h"
#include "pci_slot.h"
#include "virge_pci.h"
#include "riva128.h"
#include "rivatnt.h"
#include "geforce.h"
#include "mga2064w.h"
#include "promotion.h"
#include "ds2416.h"
#include "sonicvibes.h"
#include "sw1000xg.h"
#include "rtl8029as_pci.h"

DEFINE_DEVICE_TYPE(PCI_SLOT, pci_slot_device, "pci_slot", "PCI extension motherboard port")

pci_slot_device::pci_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, PCI_SLOT, tag, owner, clock),
	device_single_card_slot_interface<pci_card_interface>(mconfig, *this),
	m_irq_cb(*this)
{
}

pci_slot_device::~pci_slot_device()
{
}

void pci_slot_device::device_start()
{
}

u8 pci_slot_device::get_slot() const
{
	return m_slot;
}

pci_card_device *pci_slot_device::get_card() const
{
	return dynamic_cast<pci_card_device *>(get_card_device());
}

pci_card_interface::pci_card_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "pci_card"),
	m_pci_slot(nullptr)
{
}

void pci_card_interface::interface_pre_start()
{
	m_pci_slot = downcast<pci_slot_device *>(device().owner());
}

void pci_card_interface::irq_w(offs_t line, u8 state)
{
}

pci_card_device::pci_card_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	pci_device(mconfig, type, tag, owner, clock),
	pci_card_interface(mconfig, *this)
{
}

pci_card_device::~pci_card_device()
{
}

void pci_cards(device_slot_interface &device)
{
	// 0x01 - mass storage controllers
	// 0x02 - network controllers
	device.option_add("rtl8029as",      RTL8029AS_PCI);

	// 0x03 - display controllers
	device.option_add("virge",          VIRGE_PCI);
	device.option_add("virgedx",        VIRGEDX_PCI);
	device.option_add("riva128",        RIVA128);
	device.option_add("riva128zx",      RIVA128ZX);
	device.option_add("rivatnt",        RIVATNT);
	device.option_add("rivatnt2",       RIVATNT2);
	device.option_add("rivatnt2_ultra", RIVATNT2_ULTRA);
	device.option_add("vanta",          VANTA);
	device.option_add("rivatnt2_m64",   RIVATNT2_M64);
	device.option_add("geforce256",     GEFORCE256);
	device.option_add("geforce256_ddr", GEFORCE256_DDR);
	device.option_add("quadro",         QUADRO);
	device.option_add("mga2064w",       MGA2064W);
	device.option_add("promotion3210",  PROMOTION3210);

	// 0x04 - multimedia controllers
	device.option_add("sw1000xg",       SW1000XG);
	device.option_add("ds2416",         DS2416);
	device.option_add("sonicvibes",     SONICVIBES);

	// 0x05 - memory controllers
	// 0x06 - bridge devices
	// 0x07 - simple communication controllers
	// 0x08 - generic system peripherals
	// 0x09 - input devices
	// 0x0a - docking stations
	// 0x0b - processors
	// 0x0c - Serial Bus controllers
	// 0x0d - wireless controllers
	// 0x0e - Intelligent I/O controllers
	// 0x0f - Satellite Communication controllers
	// 0x10 - Encryption/Decryption controllers
	// 0x11 - Data acquisition and signal processing controllers
	// 0x12 - Processing accelerators
	// 0x13 - Debug
}
