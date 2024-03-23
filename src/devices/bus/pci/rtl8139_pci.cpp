// license:BSD-3-Clause
// copyright-holders:
/**************************************************************************************************

Realtek RTL8139C / RTL8139D / RTL8129 (?) PCI ethernet cards

Second gen of Realtek network cards, driving away from the NE2000 design in '8029AS.
'8139 is considerably popular, being the base of:
- QEMU Virtual Machine;
- Dreamcast BroadBand Adapter, thru a G2-to-PCI bridge codenamed GAPS (Sega ID 315-6170).
https://github.com/sizious/dcload-ip/tree/master
- And a gazillion of other OEM versions, a partial list at:
https://admin.pci-ids.ucw.cz/read/PC/10ec/8139

**************************************************************************************************/

#include "emu.h"
#include "rtl8139_pci.h"

#define LOG_WARN      (1U << 1)

#define VERBOSE (LOG_GENERAL | LOG_WARN)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

#define LOGWARN(...)            LOGMASKED(LOG_WARN, __VA_ARGS__)


// RTL8129 (?)
DEFINE_DEVICE_TYPE(RTL8139_PCI, rtl8139_pci_device,   "rtl8139_pci",   "Realtek RTL8139 PCI Fast Ethernet Adapter card")
// RTL8139A
// RTL8139A-G
// RTL8139B
// RTL8130
// RTL8139C
// RTL8139C+
// RTL8100
// RTL8100B / RTL8139D
// RTL8101

rtl8139_pci_device::rtl8139_pci_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: pci_card_device(mconfig, type, tag, owner, clock)
{
	// NOTE: later cards actually pick ups device ID from EEPROM, falling back to 0x8139 if there isn't one
	// Presumably HWVERID in I/O space tells what kind of device it really is
	// TODO: pci-ids shows 0139 as 8139 alias
	// TODO: RTL8139C datasheet claims 8129h as default, assume typo for now
	set_ids(0x10ec8139, 0x00, 0x020000, 0x10ec8139);
}

rtl8139_pci_device::rtl8139_pci_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: rtl8139_pci_device(mconfig, RTL8139_PCI, tag, owner, clock)
{
}

void rtl8139_pci_device::device_add_mconfig(machine_config &config)
{

}

void rtl8139_pci_device::device_start()
{
	pci_card_device::device_start();

//  add_map( 256, M_IO, FUNC(rtl8139_pci_device::ioar_map));
//  add_map( 256, M_MEM, FUNC(rtl8139_pci_device::memar_map));

	// INTA#
	intr_pin = 1;
}

void rtl8139_pci_device::device_reset()
{
	pci_card_device::device_reset();

	// TODO: strapping options for both command and status
	command = 0x0000;
	status = 0x0200;

	remap_cb();
}

void rtl8139_pci_device::config_map(address_map &map)
{
	pci_card_device::config_map(map);
}
