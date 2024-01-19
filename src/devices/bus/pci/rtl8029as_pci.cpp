// license:BSD-3-Clause
// copyright-holders:
/***************************************************************************************************

Realtek RTL8029AS based Network cards

List of OEM cards:
- D-Link SN-3200 (ibm5170:s3200pci)
- ...

TODO:
- Understand how it manages "config pages" (-> 4 views);
- Bridge with NE2000;
- 93C46 EEPROM;
- Optional ROMs for RTL8029AS extended modes;

***************************************************************************************************/

#include "emu.h"
#include "rtl8029as_pci.h"

#define LOG_WARN      (1U << 1)

#define VERBOSE (LOG_GENERAL | LOG_WARN)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

#define LOGWARN(...)            LOGMASKED(LOG_WARN, __VA_ARGS__)


DEFINE_DEVICE_TYPE(RTL8029AS_PCI, rtl8029as_pci_device,   "rtl8029as_pci",   "Realtek RTL8029AS PCI Full-Duplex Ethernet card")



rtl8029as_pci_device::rtl8029as_pci_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: pci_card_device(mconfig, type, tag, owner, clock)
{
	set_ids(0x10ec8029, 0x00, 0x020000, 0x10ec8029);
}

rtl8029as_pci_device::rtl8029as_pci_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: rtl8029as_pci_device(mconfig, RTL8029AS_PCI, tag, owner, clock)
{
}

void rtl8029as_pci_device::device_add_mconfig(machine_config &config)
{

}

void rtl8029as_pci_device::device_start()
{
	pci_card_device::device_start();

	// TODO: verify 16 being of the right size
	// documentation puts 3 bits in BAR0 as size but then all the "pages" are 16,
	// then one note also claims 32 bytes wtf
//  add_map( 16, M_IO, FUNC(rtl8029as_pci_device::map));

	// INTA#
	intr_pin = 1;
}

void rtl8029as_pci_device::device_reset()
{
	pci_card_device::device_reset();

	// Not indicated, assume zeroed
	command = 0x0000;
	// medium DEVSELB
	status = 0x0200;

	remap_cb();
}

void rtl8029as_pci_device::config_map(address_map &map)
{
	pci_card_device::config_map(map);
}
