// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Generic PCI card port

#include "emu.h"
#include "pci_slot.h"

#include "aha2940au.h"
#include "audiowerk2.h"
#include "clgd5446.h"
#include "clgd546x_laguna.h"
#include "ds2416.h"
#include "ess_maestro.h"
#include "geforce.h"
#include "mga2064w.h"
#include "ncr53c825.h"
#include "neon250.h"
#include "opti82c861.h"
#include "oti_spitfire.h"
#include "pdc20262.h"
#include "promotion.h"
#include "riva128.h"
#include "rivatnt.h"
#include "rtl8029as_pci.h"
#include "rtl8139_pci.h"
#include "sis6326.h"
#include "sonicvibes.h"
#include "sw1000xg.h"
#include "virge_pci.h"
#include "vision.h"
#include "vt6306.h"
#include "wd9710_pci.h"
#include "zr36057.h"


DEFINE_DEVICE_TYPE(PCI_SLOT, pci_slot_device, "pci_slot", "PCI extension motherboard port")

pci_slot_device::pci_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, PCI_SLOT, tag, owner, clock),
	device_single_card_slot_interface<pci_card_interface>(mconfig, *this)
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
	m_pci_slot(dynamic_cast<pci_slot_device *>(device.owner())) // Beware, the owner may not be a pci_slot_device, in which case the cast returns nullptr
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

void pci_card_device::device_start()
{
	pci_device::device_start();

	if(m_pci_slot)
		m_pci_slot->get_irq_map(m_irq_map);

	m_pin_state = 0;
	save_item(NAME(m_pin_state));
}

void pci_card_device::device_reset()
{
	pci_device::device_reset();

	for(int i=0; i != 4; i++)
		if(m_pin_state & (1 << i))
			m_pci_root->irq_pin_w(m_irq_map[i], 0);
	m_pin_state = 0;
}

void pci_card_device::irq_pin_w(offs_t line, int state)
{
	state = state != 0;
	int pstate = (m_pin_state >> line) & 1;
	if(pstate == state)
		return;
	m_pin_state = (m_pin_state & ~(1 << line)) | (state << line);

	m_pci_root->irq_pin_w(m_irq_map[line], state);
}

void pci_cards(device_slot_interface &device)
{
	// 0x00 - backward compatible pre-class code
//  device.option_add("voodoo1",        VOODOO_1_PCI);
	device.option_add("vision864",      VISION864_PCI);
	device.option_add("vision964",      VISION964_PCI);

	// 0x01 - mass storage controllers
	device.option_add("aha2940au",      AHA2940AU);
	device.option_add("ncr53c825",      NCR53C825_PCI);
	device.option_add("pdc20262",       PDC20262);

	// 0x02 - network controllers
	device.option_add("rtl8029as",      RTL8029AS_PCI);
	device.option_add("rtl8139",        RTL8139_PCI);

	// 0x03 - display controllers
	device.option_add("vision968",      VISION968_PCI);
	device.option_add("virge",          VIRGE_PCI);
	device.option_add("virgedx",        VIRGEDX_PCI);
	device.option_add("mga2064w",       MGA2064W);
	device.option_add("promotion3210",  PROMOTION3210);
	device.option_add("gd5446",         GD5446_PCI);
	device.option_add("oti64111",       OTI64111_PCI);
	device.option_add("wd9710",         WD9710_PCI);

	// 0x04 - multimedia controllers
	device.option_add("sw1000xg",       SW1000XG);
	device.option_add("ds2416",         DS2416);
	device.option_add("sonicvibes",     SONICVIBES);
	device.option_add("ess_solo1",      ES1946_SOLO1E);
	device.option_add("zr36057",        ZR36057_PCI);
	device.option_add("audiowerk2",     AUDIOWERK2);

	// 0x05 - memory controllers
	// 0x06 - bridge devices
	// 0x07 - simple communication controllers
	// 0x08 - generic system peripherals
	// 0x09 - input devices
	// 0x0a - docking stations
	// 0x0b - processors
	// 0x0c - Serial Bus controllers
	device.option_add("vt6306",         VT6306_PCI);
	device.option_add("opti82c861",     OPTI_82C861);

	// 0x0d - wireless controllers
	// 0x0e - Intelligent I/O controllers
	// 0x0f - Satellite Communication controllers
	// 0x10 - Encryption/Decryption controllers
	// 0x11 - Data acquisition and signal processing controllers
	// 0x12 - Processing accelerators
	// 0x13 - Debug
}

// assume all natively with class code 03
void agp_cards(device_slot_interface &device)
{
	// nVidia
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
	// Cirrus Logic
	device.option_add("laguna3d",       GD5465_LAGUNA3D);
	// PowerVR VideoLogic
	device.option_add("neon250",        NEON250);
	// SiS
	device.option_add("sis6326_agp",    SIS6326_AGP);
}
