// license:BSD-3-Clause
// copyright-holders:
/**************************************************************************************************

VIA VT6306 generic FireWire IEEE 1394a card

- paired with USB 2.0 controller on a Sunix UFC3212V 4x USB 3x Firewire card
  (as multifunction, at .3 while USB takes .0/.1/.2)
- midway/midzeus.cpp crusnexo/thegrid uses regular IEEE 1394 ports.
- skeleton/pegasos2.cpp uses an on-board version of this.
- PHY has default vendor ID=0x004063, device ID=306000, Compliance Level=1

**************************************************************************************************/

#include "emu.h"
#include "vt6306.h"

#define LOG_WARN      (1U << 1)

#define VERBOSE (LOG_GENERAL | LOG_WARN)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

#define LOGWARN(...)            LOGMASKED(LOG_WARN, __VA_ARGS__)


DEFINE_DEVICE_TYPE(VT6306_PCI, vt6306_device,   "vt6306",   "VT6306 VIA Fire II IEEE-1394a OHCI Link Layer Controller")



vt6306_device::vt6306_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: pci_card_device(mconfig, type, tag, owner, clock)
{
	set_ids(0x11063044, 0x00, 0x0c0010, 0x11063044);
	// Sunix UFC3212V has these values
//  set_ids(0x11063044, 0x46, 0x0c0010, 0x1106e8c1);
}

vt6306_device::vt6306_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: vt6306_device(mconfig, VT6306_PCI, tag, owner, clock)
{
}

void vt6306_device::device_add_mconfig(machine_config &config)
{

}

void vt6306_device::device_start()
{
	pci_card_device::device_start();

	add_map(2048, M_MEM, FUNC(vt6306_device::ohci_mmio_map));
	add_map( 128, M_IO, FUNC(vt6306_device::vio_map));
//  add_map( 256, M_MEM, FUNC(vt6306_device::cardbus_map));

	// INTA#
	intr_pin = 1;

	// TODO: min_gnt = 0x00, max_lat = 0x20
}

void vt6306_device::device_reset()
{
	pci_card_device::device_reset();

	command = 0x0000;
	// doc claims not having an I/O space but real HW pci.exe proved otherwise
	command_mask = 7;
	// Fast Back-to-Back, medium DEVSEL#
	status = 0x0280;

	remap_cb();
}

u8 vt6306_device::capptr_r()
{
	return 0x50;
}

void vt6306_device::config_map(address_map &map)
{
	pci_card_device::config_map(map);
//  map(0x40, 0x43) PCI HCI Control (?)
	// ACPI
	map(0x50, 0x50).lr8(NAME([] () { return 0x01; }));
	map(0x51, 0x51).lr8(NAME([] () { return 0x00; })); // NULL pointer
//  map(0x52, 0x57) PCI Power Management v1.1
}

void vt6306_device::ohci_mmio_map(address_map &map)
{
}

void vt6306_device::vio_map(address_map &map)
{
}
